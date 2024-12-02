// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include "../ext/nlohmann/json.hpp"
// mac config: run  brew install nlohmann-json

#include "physics_system.hpp"
#include "animation_system.hpp"
#include "player_controller.hpp"
#include <iomanip>

// Game configuration
// const size_t MAX_NUM_CONTACT_SLOW = 2;
// const size_t MAX_NUM_CONTACT_FAST = 5;
const size_t CONTACT_SLOW_SPAWN_DELAY_MS = 5000 * 3;
const size_t CONTACT_FAST_SPAWN_DELAY_MS = 2000 * 3;
// const size_t MAX_NUM_RANGED_ENEMY = 1;
const size_t RANGED_ENEMY_SPAWN_DELAY_MS = 5000 * 3;
const size_t RANGED_ENEMY_PROJECTILE_DELAY_MS = 3000;
const size_t DASHING_ENEMY_SPAWN_DELAY_MS = 5000 * 3;

const int TILE_SIZE = 100;
std::vector<vec2> tile_vec;
std::vector<vec2> spawnable_tiles;
const int LIGHT_FLICKER_RATE = 2000 * 10;
const int FPS_COUNTER_MS = 1000;
const float POWERUP_DROP_CHANCE = 0.35;
const float POWERUP_TIMER = 15000;

int lightflicker_counter_ms;
int fps_counter_ms;
int fps = 0;
bool display_fps = false;
int frames = 0;

bool is_tutorial_on = false;
bool WorldSystem::is_paused = false;
bool WorldSystem::is_level_up = false;

size_t max_num_enemies = 50;

// For testing purposes only
int map_counter = 1;

std::vector<int> current_door_pos;

std::vector<int> current_tenant_pos;

PhysicsSystem physics;

using json = nlohmann::json;

void pauseMenuText()
{
	while (registry.debugComponents.entities.size() > 0)
		registry.remove_all_components_of(registry.debugComponents.entities.back());
	createText(vec2(475, 450), 0.8f, "PRESS P TO UNPAUSE", vec3(1.0f, 1.0f, 1.0f));
	createText(vec2(410, 370), 0.8f, "PRESS ESC TO EXIT TO MENU", vec3(1.0f, 1.0f, 1.0f));
	createText(vec2(245, 300), 0.6f, "(If you exit to menu your progress in the level will be lost!)", vec3(1.0f, 1.0f, 1.0f));
}

void windowMinimizedCallback(GLFWwindow *window, int iconified)
{
	if (iconified)
	{
		WorldSystem::is_paused = true;
		if (registry.screenStates.components.size() != 0)
		{

			ScreenState &screen = registry.screenStates.components[0];
			if (screen.state == GameState::GAME)
			{
				screen.state = GameState::PAUSED;
				screen.darken_screen_factor = 0.9f;
				pauseMenuText();
			}
			if (screen.state != GameState::PAUSED)
			{
				WorldSystem::is_paused = false;
			}
		}
	}
}

void windowFocusCallback(GLFWwindow *window, int focused)
{
	if (!focused)
	{
		WorldSystem::is_paused = true;
		if (registry.screenStates.components.size() != 0)
		{
			ScreenState &screen = registry.screenStates.components[0];
			if (screen.state == GameState::GAME)
			{
				screen.state = GameState::PAUSED;
				screen.darken_screen_factor = 0.9f;
				pauseMenuText();
			}
			if (screen.state != GameState::PAUSED)
			{
				WorldSystem::is_paused = false;
			}
		}
	}
}

void WorldSystem::stateSwitch(GameState new_state)
{
	// need to assert size
	ScreenState &screen = registry.screenStates.components[0];

	switch (new_state)
	{
	case (GameState::GAME_OVER):
		// game over - we are guaranteed to be coming from GameState::GAME
		screen.state = GameState::GAME_OVER;
		while (registry.userInterfaces.entities.size() > 0)
		{
			registry.remove_all_components_of(registry.userInterfaces.entities.back());
		}
		while (registry.motions.entities.size() > 0)
		{
			registry.list_all_components_of(registry.motions.entities.back());
			registry.remove_all_components_of(registry.motions.entities.back());
		}
		is_paused = false;
		screen.darken_screen_factor = 0.0f;
		createGameOverScreen(renderer);
		restart_world();
		// currently the screen won't darken, maybe just make it a big background screen like the menu or whatever and remove all motion components
		break;
	case (GameState::MENU):
		// guaranteed to be coming from GAME_OVER or PAUSED
		while (registry.userInterfaces.entities.size() > 0)
		{
			registry.remove_all_components_of(registry.userInterfaces.entities.back());
		}
		while (registry.motions.entities.size() > 0)
		{
			registry.remove_all_components_of(registry.motions.entities.back());
		}
		restart_world();
		is_paused = false;
		screen.state = GameState::MENU;
		screen.darken_screen_factor = 0.0f;

		createMenuScreen(renderer);
		createElevatorButtons(renderer, 3);
		break;
	case (GameState::GAME):
		while (registry.userInterfaces.entities.size() > 0)
		{
			registry.remove_all_components_of(registry.userInterfaces.entities.back());
		}
		while (registry.motions.entities.size() > 0)
		{
			registry.remove_all_components_of(registry.motions.entities.back());
		}
		is_paused = false;
		restart_game();
		break;
	default:
		return;
	}
}

// create the underwater world
WorldSystem::WorldSystem()
	: points(0), next_contact_fast_spawn(0.f), next_contact_slow_spawn(0.f), next_ranged_spawn(0.f)
{
	// Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
}

WorldSystem::~WorldSystem()
{

	// destroy music components
	if (background_music != nullptr)
		Mix_FreeMusic(background_music);
	if (button_click_sound != nullptr)
		Mix_FreeChunk(button_click_sound);
	if (salmon_eat_sound != nullptr)
		Mix_FreeChunk(salmon_eat_sound);
	if (player_damage_sound != nullptr)
		Mix_FreeChunk(player_damage_sound);
	if (enemy_damage_sound != nullptr)
		Mix_FreeChunk(enemy_damage_sound);
	if (level_up_sound != nullptr)
		Mix_FreeChunk(level_up_sound);
	if (door_sound != nullptr)
		Mix_FreeChunk(door_sound);

	Mix_CloseAudio();

	// Destroy all created components
	registry.clear_all_components();

	// Close the window
	glfwDestroyWindow(window);

	// glfwTerminate();
}

// Debugging
namespace
{
	void glfw_err_cb(int error, const char *desc)
	{
		fprintf(stderr, "%d: %s", error, desc);
	}
}

// Returns an int between 0 and n
int WorldSystem::randomInt(int n)
{
	double randomFloat = uniform_dist(rng);
	return static_cast<int>(std::floor(randomFloat * (n + 1)));
}

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the renderer
GLFWwindow *WorldSystem::create_window()
{
	///////////////////////////////////////
	// Initialize GLFW
	glfwSetErrorCallback(glfw_err_cb);
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW");
		return nullptr;
	}

	//-------------------------------------------------------------------------
	// If you are on Linux or Windows, you can change these 2 numbers to 4 and 3 and
	// enable the glDebugMessageCallback to have OpenGL catch your mistakes for you.
	// GLFW / OGL Initialization
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_RESIZABLE, 0);

	// Create the main window (for rendering, keyboard, and mouse input)
	window = glfwCreateWindow(window_width_px, window_height_px, "Eviction of the Damned", nullptr, nullptr);
	if (window == nullptr)
	{
		fprintf(stderr, "Failed to glfwCreateWindow");
		return nullptr;
	}

	// Minimized callback
	glfwSetWindowIconifyCallback(window, windowMinimizedCallback);

	// Not in focus callback
	glfwSetWindowFocusCallback(window, windowFocusCallback);

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this);
	auto key_redirect = [](GLFWwindow *wnd, int _0, int _1, int _2, int _3)
	{ ((WorldSystem *)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow *wnd, double _0, double _1)
	{ ((WorldSystem *)glfwGetWindowUserPointer(wnd))->on_mouse_move({_0, _1}); };
	auto mouse_button_redirect = [](GLFWwindow *wnd, int _0, int _1, int _2)
	{ ((WorldSystem *)glfwGetWindowUserPointer(wnd))->on_mouse_button(_0, _1, _2); };
	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);
	glfwSetMouseButtonCallback(window, mouse_button_redirect);

	//////////////////////////////////////
	// Loading music and sounds with SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0)
	{
		fprintf(stderr, "Failed to initialize SDL Audio");
		return nullptr;
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1)
	{
		fprintf(stderr, "Failed to open audio device");
		return nullptr;
	}

	background_music = Mix_LoadMUS(audio_path("music.wav").c_str());
	button_click_sound = Mix_LoadWAV(audio_path("click.wav").c_str());
	salmon_eat_sound = Mix_LoadWAV(audio_path("eat_sound.wav").c_str());
	player_damage_sound = Mix_LoadWAV(audio_path("damage.wav").c_str());
	enemy_damage_sound = Mix_LoadWAV(audio_path("enemy_damage.wav").c_str());
	level_up_sound = Mix_LoadWAV(audio_path("level_up_select.wav").c_str());
	door_sound = Mix_LoadWAV(audio_path("door.wav").c_str());

	if (background_music == nullptr || button_click_sound == nullptr || salmon_eat_sound == nullptr || player_damage_sound == nullptr || enemy_damage_sound == nullptr || level_up_sound == nullptr)
	{
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
				audio_path("music.wav").c_str(),
				audio_path("death_sound.wav").c_str(),
				audio_path("eat_sound.wav").c_str());
		return nullptr;
	}

	return window;
}

vec2 WorldSystem::mousePosToNormalizedDevice(vec2 mouse_position)
{
	float normalized_x = (2.0f * mouse_position.x) / window_width_px - 1.0f;
	float normalized_y = 1.0f - (2.0f * mouse_position.y) / window_height_px;
	return vec2(normalized_x, normalized_y);
}

std::vector<std::string> WorldSystem::textToDialogueMode(std::string text)
{
	std::vector<std::string> result;

	std::string current = "";

	for (int i = 0; i < text.size(); i++)
	{
		std::string c = {text[i]};
		if (c == "\n")
		{
			result.push_back(current);
			current = "";
		}
		else
		{
			current.push_back(text[i]);
		}
	}
	if (current.size() != 0)
	{
		result.push_back(current);
	}
	return result;
}

void WorldSystem::restart_world()
{
	// Debugging for memory/component leaks
	registry.list_all_components();
	printf("Restarting\n");

	// Reset the game speed
	current_speed = 1.f;
	is_level_up = false;

	camera = createCamera(renderer, vec2(window_width_px / 2, window_height_px / 2));
}

void WorldSystem::init(RenderSystem *renderer_arg)
{
	this->renderer = renderer_arg;
	// Playing background music indefinitely
	// Mix_PlayMusic(background_music, -1);
	fprintf(stderr, "Loaded music\n");
	fps_counter_ms = FPS_COUNTER_MS;

	current_map = map1;
	current_door_pos = door_positions[0];
	current_tenant_pos = tenant_positions[0];

	ScreenState &screen = registry.screenStates.components[0];
	screen.state = GameState::START;

	createStartScreen(renderer);

	// Set all states to default
	restart_world();
}

// Create an HP bar for an enemy
void createHPBar(Entity enemy)
{
	// Check to avoid errors since collisions occur after world step

	if (registry.swarms.has(enemy))
	{
		return;
	}
	if (registry.healths.has(enemy) && !registry.projectiles.has(enemy))
	{
		float &hp = registry.healths.get(enemy).hit_points;
		float &max_hp = registry.healths.get(enemy).max_hp;
		Motion &enemy_motion = registry.motions.get(enemy);
		// Shift centre of line to left as the hp bar decreases
		Entity hp_bar = createLine({enemy_motion.position.x - ((100.f) * (1 - (hp / max_hp))) / 2,
									enemy_motion.position.y - abs(enemy_motion.scale.y) * 0.75f},
								   {(100.f) * hp / max_hp, 10.f});
		vec3 &color = registry.colors.emplace(hp_bar);
		if (registry.bosses.has(enemy))
		{
			color = vec3(1.f, 0.f, 0.f);
		}
		else
		{
			color = vec3(0.f, 5.f, 0.f);
		}
	}
}

void WorldSystem::update_experience_bar()
{
	auto &player = registry.players.get(my_player);
	auto &pmotion = registry.motions.get(my_player);

	// vec2 exp_bar_pos = {-0.74f, 0.55f};

	float progress = std::min((float)player.experience / player.toNextLevel, 1.0f);

	float bar_offset = (progress * 0.2f);
	vec2 bar_pos = vec2(-0.94f + bar_offset, 0.525f);

	auto &ui = registry.userInterfaces.get(experience_in);
	ui.scale = vec2(progress * 0.4f, 0.205f);
	ui.position = bar_pos;
}

void WorldSystem::save_player_data(const std::string &filename)
{
	auto &player = registry.players.get(my_player);

	json j = {
		{"dash_cooldown_ms", player.dash_cooldown_ms},
		{"damage_multiplier", player.damage_multiplier},
		{"attack_duration_ms", player.attack_duration_ms},
		{"knockback_strength", player.knockback_strength},
		{"attack_size", player.attack_size},
		{"collection_distance", player.collection_distance},
		{"experience_multiplier", player.experience_multiplier},
		{"experience", player.experience},
		{"toNextLevel", player.toNextLevel},
		{"level", player.level}};

	std::ofstream file(filename);
	file << j.dump(4);

	std::cout << "saved" << std::endl;
}

void WorldSystem::load_player_data(const std::string &filename)
{
	std::ifstream file(filename);

	if (!file.is_open())
	{
		std::cout << "failed to open file for player data" << std::endl;
		return;
	}

	json j;
	file >> j;

	auto &player = registry.players.get(my_player);

	player.dash_cooldown_ms = j["dash_cooldown_ms"];
	player.damage_multiplier = j["damage_multiplier"];
	player.knockback_strength = j["knockback_strength"];
	player.attack_size = j["attack_size"];
	player.collection_distance = j["collection_distance"];
	player.experience_multiplier = j["experience_multiplier"];
	player.experience = j["experience"];
	player.toNextLevel = j["toNextLevel"];
	player.level = j["level"];

	std::cout << "loaded" << std::endl;
}

void WorldSystem::mapSwitch(int map)
{
	if (map < 3)
	{
		current_door_pos = door_positions[map - 1];
		current_tenant_pos = tenant_positions[map - 1];
	}
	else
	{
		current_door_pos = {-1, -1};
		current_tenant_pos = {-1, -1};
	}
	switch (map)
	{
	case 1:
		current_map = map1;
		break;
	case 2:
		current_map = map2;
		break;
	case 3:
		current_map = map3;
		break;
	default:
		current_map = map1;
	}
	restart_game();
}

std::string floatToString1DP(double value)
{
	std::ostringstream out;
	out << std::fixed << std::setprecision(1) << value;
	return out.str();
}

void WorldSystem::spawn_nearby_tile(vec2 curr_tile, std::vector<ENEMY_TYPES> &enemy_types)
{
	const int dx[] = {-1, -1, -1, 0, 0, 1, 1, 1};
	const int dy[] = {-1, 0, 1, -1, 1, -1, 0, 1};

	// Assuming i,j is your current position
	for (int dir = 0; dir < 8; dir++)
	{
		if (enemy_types.size() == 0)
		{
			break;
		}
		int new_i = curr_tile.x + dx[dir];
		int new_j = curr_tile.y + dy[dir];

		// Bounds checking to avoid array out of bounds
		if (new_i >= 0 && new_i < current_map[0].size() &&
			new_j >= 0 && new_j < current_map.size())
		{
			if (current_map[new_j][new_i] == 1 || (current_map[new_j][new_i] >= 3 && current_map[new_j][new_i] <= 8))
			{
				vec2 world_pos = {(640 - (25 * 100)) + (new_i * TILE_SIZE) + (TILE_SIZE / 2), (640 - (44 * 100)) + (new_j * TILE_SIZE) + (TILE_SIZE / 2)};
				ENEMY_TYPES enemy_type = enemy_types.back();
				switch (enemy_type)
				{
				case ENEMY_TYPES::CONTACT_DMG:
					createContactSlow(renderer, world_pos);
					break;
				case ENEMY_TYPES::CONTACT_DMG_2:
					createContactFast(renderer, world_pos);
					break;
				case ENEMY_TYPES::RANGED:
					createRangedEnemy(renderer, world_pos);
					break;
				default:
					break;
				}
				enemy_types.pop_back();
			}
		}
	}
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update)
{
	// Processing the screen state
	assert(registry.screenStates.components.size() <= 1);
	ScreenState &screen = registry.screenStates.components[0];

	if (screen.state == GameState::START)
	{
		glfwSetWindowTitle(window, "Eviction of the Damned");
		return true;
	}

	if (screen.state == GameState::GAME_OVER)
	{
		glfwSetWindowTitle(window, "GAME OVER");
		return true;
	}

	if (screen.state == GameState::MENU)
	{
		glfwSetWindowTitle(window, "Main Menu");

		if (registry.elevatorDisplays.entities.size() == 0) {
			createElevatorDisplay(renderer, {window_width_px / 2, window_height_px / 2});
		} 

		Entity e = registry.elevatorDisplays.entities[0];
		ElevatorDisplay &display = registry.elevatorDisplays.get(e);

		if (display.selection_made) {
			display.current_ms += elapsed_ms_since_last_update;
			std::cout << registry.animationSets.get(e).current_animation << std::endl;

			// if display is done - either reset (if locked) or go to level
			if (display.current_ms >= display.lasting_ms) {
				
				if (display.message != 0) {
					stateSwitch(GameState::GAME);
					mapSwitch(display.message);
				} else {
					AnimationSet &animSet = registry.animationSets.get(e);
					animSet.current_animation = "elevator_empty";
					display.selection_made = false;
				}
			}
			
		}

		return true;
	}

	// Updating window title with points
	std::stringstream title_ss;
	if (current_map != map3)
	{
		title_ss << "Number of Enemies Until Next Level: " << (max(0, enemy_kill_goal - enemies_killed));
	}
	else
	{
		title_ss << "Final Boss";
	}
	glfwSetWindowTitle(window, title_ss.str().c_str());

	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
		registry.remove_all_components_of(registry.debugComponents.entities.back());

	// Removing out of screen entities
	auto &motions_registry = registry.motions;

	vec2 player_pos = motions_registry.get(my_player).position;
	vec2 world_origin = vec2(-1860, -3760);

	// Camera follows player
	auto &cameraMotion = motions_registry.get(camera);
	cameraMotion.position = player_pos;

	// Remove a spawned power up if it has been around for more than 10 seconds
	if (registry.powerups.entities.size() > 0) {
		assert(registry.powerups.size() == 1);
		Entity& powerupEntity = registry.powerups.entities[0];
		Powerup& powerup = registry.powerups.get(powerupEntity);
		if (!powerup.equipped) {
			powerup.timer -= elapsed_ms_since_last_update;
			if (powerup.timer < 0) {
				registry.remove_all_components_of(powerupEntity);
			}
		}
	}

	if (display_fps)
	{
		frames += 1;
		fps_counter_ms -= elapsed_ms_since_last_update;
		if (fps_counter_ms <= 0.f)
		{
			fps = (int) ((frames * 1000) / (FPS_COUNTER_MS + std::abs(fps_counter_ms)));
			fps_counter_ms = FPS_COUNTER_MS;
			frames = 0;
		}
		createText({1000.f, 650.f}, 1.f, "FPS: " + std::to_string(fps), glm::vec3(1.0f, 0.f, 0.f));
	}

	if (registry.doors.components.size() == 0 && goal_reached)
	{
		vec2 world_pos = {(640 - (25 * 100)) + (current_door_pos[0] * TILE_SIZE) + (TILE_SIZE / 2), (640 - (44 * 100)) + (current_door_pos[1] * TILE_SIZE) + (TILE_SIZE / 2)};

		createDoor(renderer, world_pos);
	}

	for (int i = (int)motions_registry.components.size() - 1; i >= 0; --i)
	{
		Motion &motion = motions_registry.components[i];
		Entity entity = motions_registry.entities[i];

		if (registry.solidObjs.has(entity))
		{
			if ((abs(player_pos.x - motion.position.x) > 800) || (abs(player_pos.y - motion.position.y) > 800))
			{
				vec2 obj_pos = motion.position;
				vec2 obj_pos_map = vec2((int)((obj_pos.x - world_origin.x) - ((int)abs(obj_pos.x - world_origin.x) % TILE_SIZE)) / TILE_SIZE, (int)((obj_pos.y - world_origin.y) - ((int)abs(obj_pos.y - world_origin.y) % TILE_SIZE)) / TILE_SIZE);

				if (std::find(tile_vec.begin(), tile_vec.end(), vec2(obj_pos_map.x, obj_pos_map.y)) != tile_vec.end())
				{
					tile_vec.erase(std::find(tile_vec.begin(), tile_vec.end(), vec2(obj_pos_map.x, obj_pos_map.y)));
					registry.remove_all_components_of(entity);
				}
			}
		}
	}

	// Update timer on equipped powerups, if any
	// Also show active powerup
	if (registry.powerups.has(my_player) && !is_paused)
	{
		Powerup &powerup = registry.powerups.get(my_player);
		powerup.timer -= elapsed_ms_since_last_update;

		float x = 30;
		float y = 480;

		std::string powerup_name = "";
		if (powerup.type == PowerupType::DAMAGE_BOOST)
			powerup_name = "Damage Boost";
		else if (powerup.type == PowerupType::SPEED_BOOST)
			powerup_name = "Speed Boost";
		else if (powerup.type == PowerupType::INVINCIBILITY)
			powerup_name = "Invincibility";
		
		createText({x, y}, 0.5f, "Powerup active: " + powerup_name + (powerup.multiplier < 1.02f ? "" : " with multiplier x" + floatToString1DP(powerup.multiplier)) + " for " + std::to_string((int)std::ceil(powerup.timer / 1000)) + "s", {1.f, 1.f, 1.f});
		if (powerup.timer < 0 || goal_reached)
		{
			registry.powerups.remove(my_player);
		}
	}

	/*** UPDATING MAP ***/
	// given player position in world coords, convert to map:
	// origin: [-1860, -3760]
	// current screen pos + origin
	//  minus modulo tilesize and divide by tilesize
	vec2 player_pos_map = vec2((int)((player_pos.x - world_origin.x) - ((int)(player_pos.x - world_origin.x) % TILE_SIZE)) / 100, (int)((player_pos.y - world_origin.y) - ((int)(player_pos.y - world_origin.y) % TILE_SIZE)) / TILE_SIZE);

	for (int i = player_pos_map.x - 8; i <= player_pos_map.x + 8; i++)
	{
		for (int j = player_pos_map.y - 6; j <= player_pos_map.y + 6; j++)
		{
			vec2 world_pos = {(640 - (25 * 100)) + (i * TILE_SIZE) + (TILE_SIZE / 2), (640 - (44 * 100)) + (j * TILE_SIZE) + (TILE_SIZE / 2)};

			// deciding on wall sprites
			// if it is only adjacent to one wall sprite - use 3-sided thingy and rotate accordingly
			// if it is adjacent to two walls - if on same axis use parallel sided wall and rotate accordingly
			//  							  - if not on same axis use corner wall and rotate accordingly
			// if it is adjacent to three walls - use single side wall and rotate accordingly
			// if it is adjacent to all four walls - default sprite

			if ((i < 0) || (j < 0) || (i >= current_map[0].size()) || j >= current_map.size())
			{
				if ((std::find(tile_vec.begin(), tile_vec.end(), vec2(i, j)) == tile_vec.end()))
				{
					createWalls(renderer, world_pos, current_map, {i, j});
					tile_vec.push_back(vec2(i, j));
				}
				continue;
			}
			if (std::find(tile_vec.begin(), tile_vec.end(), vec2(i, j)) != tile_vec.end())
			{
				continue;
			}
			// continue on if tiles/objects have already been processed

			if (current_map[j][i] == 0)
			{
				if (i == current_door_pos[0] && j == current_door_pos[1] && goal_reached)
				{
					createDoor(renderer, world_pos);
				}
				createWalls(renderer, world_pos, current_map, {i, j});
				tile_vec.push_back(vec2(i, j));
			}

			if (current_map[j][i] == 1)
			{
				// createFloor(renderer, world_pos);
				tile_vec.push_back(vec2(i, j));
			}

			// furniture spawning
			if (current_map[j][i] >= 20 && current_map[j][i] <= 38)
			{
				// if (current_map[j][i] == 20 && current_map[j - 1][i] != current_map[j][i] && current_map[j][i-1] != current_map[j][i]) {
				//	// add 2 while loops here to find furniture size
				//	int horiz_idx = 1;
				//	int vert_idx = 1;
				//	while (current_map[j + vert_idx][i] == 20)
				//	{
				//		vert_idx += 1;
				//	}
				//	while (current_map[j][i + horiz_idx] == 20)
				//	{
				//		horiz_idx += 1;
				//	}

				//	// pos {world_pos.x + (TILE_SIZE * ((horiz_idx - 1) / 2)), world_pos.y + (TILE_SIZE * ((vert_idx - 1) / 2))}
				//	createFurniture(renderer, {world_pos.x, world_pos.y}, current_map[j][i]);
				//} else if (current_map[j][i] != 2) {
				//	createFurniture(renderer, {world_pos.x, world_pos.y}, current_map[j][i]);
				//}
				createFurniture(renderer, {world_pos.x, world_pos.y}, current_map[j][i]);
				tile_vec.push_back(vec2(i, j));
			}

			if (current_map[j][i] == 3 && registry.deadlys.entities.size() < max_num_enemies)
			{
				int encounter = rand() % 3;
				if (encounter == 0)
				{
					createContactSlow(renderer, world_pos);
				}
				else if (encounter == 1)
				{

					createContactFast(renderer, world_pos);
				}
				else
				{

					createContactSlow(renderer, world_pos);
					std::vector<ENEMY_TYPES> additional_enemies = {ENEMY_TYPES::CONTACT_DMG};
					spawn_nearby_tile(vec2(i, j), additional_enemies);
				}
				tile_vec.push_back(vec2(i, j));
			}
			if (current_map[j][i] == 4 && registry.deadlys.entities.size() < max_num_enemies)
			{
				int encounter = rand() % 3;
				if (encounter == 0)
				{

					createContactSlow(renderer, world_pos);
					std::vector<ENEMY_TYPES> additional_enemies = {ENEMY_TYPES::CONTACT_DMG_2};
					spawn_nearby_tile(vec2(i, j), additional_enemies);
				}
				else if (encounter == 1)
				{

					createContactFast(renderer, world_pos);
					std::vector<ENEMY_TYPES> additional_enemies = {ENEMY_TYPES::CONTACT_DMG_2};
					spawn_nearby_tile(vec2(i, j), additional_enemies);
				}
				else
				{
					createRangedEnemy(renderer, world_pos);
					std::vector<ENEMY_TYPES> additional_enemies = {ENEMY_TYPES::CONTACT_DMG, ENEMY_TYPES::CONTACT_DMG};
					spawn_nearby_tile(vec2(i, j), additional_enemies);
				}
				tile_vec.push_back(vec2(i, j));
			}
			if (current_map[j][i] == 5 && registry.deadlys.entities.size() < max_num_enemies)
			{
				int encounter = rand() % 3;
				if (encounter == 0)
				{
					createContactFast(renderer, world_pos);
					std::vector<ENEMY_TYPES> additional_enemies = {ENEMY_TYPES::RANGED, ENEMY_TYPES::RANGED};
					spawn_nearby_tile(vec2(i, j), additional_enemies);
				}
				else if (encounter == 1)
				{
					createContactSlow(renderer, world_pos);
					std::vector<ENEMY_TYPES> additional_enemies = {ENEMY_TYPES::CONTACT_DMG_2, ENEMY_TYPES::CONTACT_DMG_2};
					spawn_nearby_tile(vec2(i, j), additional_enemies);
				}
				else
				{
					createRangedEnemy(renderer, world_pos);
					std::vector<ENEMY_TYPES> additional_enemies = {ENEMY_TYPES::CONTACT_DMG_2, ENEMY_TYPES::CONTACT_DMG, ENEMY_TYPES::CONTACT_DMG};
					spawn_nearby_tile(vec2(i, j), additional_enemies);
				}
				tile_vec.push_back(vec2(i, j));
			}

			if (current_map[j][i] == 6)
			{
				// BUFF_TYPE type = static_cast<BUFF_TYPE>((int) (rand() % 3));
				createHealthBuff(renderer, world_pos);
				tile_vec.push_back(vec2(i, j));
			}

			if (current_map[j][i] == 8 && registry.deadlys.entities.size() < max_num_enemies)
			{
				Entity swarm_leader = createSwarm(renderer, world_pos, 0.55f, 0.05f, 0.00005f);
				Motion &swarm_motion = motions_registry.get(swarm_leader);
				tile_vec.push_back(vec2(i, j));
			}
		}
	}

	// create hp bars for enemies
	for (auto &enemy : registry.deadlys.entities)
	{
		createHPBar(enemy);
	}

	// Managing tenant appearance and interaction ability stuff
	if (goal_reached)
	{
		if (registry.tenants.entities.size() == 0)
		{
			std::cout << "creating tenant" << std::endl;
			vec2 world_pos = {(640 - (25 * 100)) + (current_tenant_pos[0] * TILE_SIZE) + (TILE_SIZE / 2), (640 - (44 * 100)) + (current_tenant_pos[1] * TILE_SIZE) + (TILE_SIZE / 2)};
			createTenant(renderer, world_pos, 1); // TODO: vary level input based on map
		}
		Entity tenant = registry.tenants.entities[0];
		if (registry.tutorialIcons.size() == 0 && registry.tenants.get(tenant).player_in_radius && !cutscene)
		{
			createInteractKey(renderer, {registry.motions.get(tenant).position.x, registry.motions.get(tenant).position.y + 60});
		}
		if (!registry.tenants.get(tenant).player_in_radius)
		{
			while (registry.tutorialIcons.entities.size() != 0)
			{
				registry.remove_all_components_of(registry.tutorialIcons.entities.back());
			}
		}
	}

	// TODO: spawn frequencies and spawn radius to be adjusted
	//  Spawn Level 1 type enemy: slow with contact damage
	next_contact_slow_spawn -= elapsed_ms_since_last_update * current_speed;
	if (next_contact_slow_spawn < 0.f && registry.deadlys.entities.size() < max_num_enemies)
	{
		next_contact_slow_spawn = (CONTACT_SLOW_SPAWN_DELAY_MS / 2) + uniform_dist(rng) * (CONTACT_SLOW_SPAWN_DELAY_MS / 2);
		vec2 contact_slow_pos;
		float distance_to_player;
		float index;
		do
		{
			index = static_cast<int>(uniform_dist(rng) * spawnable_tiles.size());
			contact_slow_pos = {(640 - (25 * 100)) + (spawnable_tiles[index].y * TILE_SIZE) + (TILE_SIZE / 2), (640 - (44 * 100)) + (spawnable_tiles[index].x * TILE_SIZE) + (TILE_SIZE / 2)};
			distance_to_player = sqrt(pow(contact_slow_pos.x - player_pos.x, 2) + pow(contact_slow_pos.y - player_pos.y, 2));
		} while (distance_to_player < 300.f);

		createContactSlow(renderer, contact_slow_pos);
		// tile_vec.push_back(spawnable_tiles[index]);
	}

	// Spawn Level 2 type enemy: fast with contact damage
	next_contact_fast_spawn -= elapsed_ms_since_last_update * current_speed;
	if (next_contact_fast_spawn < 0.f && registry.deadlys.entities.size() < max_num_enemies)
	{
		next_contact_fast_spawn = (CONTACT_FAST_SPAWN_DELAY_MS / 2) + uniform_dist(rng) * (CONTACT_FAST_SPAWN_DELAY_MS / 2);
		vec2 contact_fast_pos;
		float distance_to_player;
		float index;
		do
		{
			index = static_cast<int>(uniform_dist(rng) * spawnable_tiles.size());
			contact_fast_pos = {(640 - (25 * 100)) + (spawnable_tiles[index].y * TILE_SIZE) + (TILE_SIZE / 2), (640 - (44 * 100)) + (spawnable_tiles[index].x * TILE_SIZE) + (TILE_SIZE / 2)};
			distance_to_player = sqrt(pow(contact_fast_pos.x - player_pos.x, 2) + pow(contact_fast_pos.y - player_pos.y, 2));
		} while (distance_to_player < 300.f);
		if (uniform_dist(rng) > 0.5)
		{
			createContactFast(renderer, contact_fast_pos);
		}
		else
		{
			createSlowingEnemy(renderer, contact_fast_pos);
		}
		// tile_vec.push_back(spawnable_tiles[index]);
	}

	// Spawn dashing enemy
	next_dashing_spawn -= elapsed_ms_since_last_update * current_speed;
	if (next_dashing_spawn < 0.f && registry.deadlys.entities.size() < max_num_enemies)
	{
		next_dashing_spawn = (DASHING_ENEMY_SPAWN_DELAY_MS / 2) + uniform_dist(rng) * (DASHING_ENEMY_SPAWN_DELAY_MS / 2);
		vec2 dashing_pos;
		float distance_to_player;
		float index;
		do
		{
			index = static_cast<int>(uniform_dist(rng) * spawnable_tiles.size());
			dashing_pos = {(640 - (25 * 100)) + (spawnable_tiles[index].y * TILE_SIZE) + (TILE_SIZE / 2), (640 - (44 * 100)) + (spawnable_tiles[index].x * TILE_SIZE) + (TILE_SIZE / 2)};
			distance_to_player = sqrt(pow(dashing_pos.x - player_pos.x, 2) + pow(dashing_pos.y - player_pos.y, 2));
		} while (distance_to_player < 300.f);
		createDashingEnemy(renderer, dashing_pos);
		// tile_vec.push_back(spawnable_tiles[index]);
	}

	// Spawn projectiles for ranged enemies
	for (auto &ranged : registry.ranged.entities)
	{
		// Don't shoot if there's no los
		if (physics.has_los(registry.motions.get(ranged).position, player_pos))
		{
			float &projectile_delay = registry.ranged.get(ranged).projectile_delay;
			projectile_delay -= elapsed_ms_since_last_update * current_speed;
			if (projectile_delay < 0.f)
			{
				// reset timer
				projectile_delay = (RANGED_ENEMY_PROJECTILE_DELAY_MS / 2) + uniform_dist(rng) * (RANGED_ENEMY_PROJECTILE_DELAY_MS / 2);
				Entity projectile;
				if (registry.deadlys.get(ranged).enemy_type == ENEMY_TYPES::RANGED)
				{
					projectile = createRangedProjectile(renderer, registry.motions.get(ranged).position);
				}
				else if (registry.deadlys.get(ranged).enemy_type == ENEMY_TYPES::RANGED_HOMING)
				{
					projectile = createRangedHomingProjectile(renderer, registry.motions.get(ranged).position);
				}
				Motion &projectile_motion = registry.motions.get(projectile);
				Motion &player_motion = registry.motions.get(my_player);
				projectile_motion.angle = atan2(projectile_motion.position.y - player_motion.position.y, projectile_motion.position.x - player_motion.position.x);
				projectile_motion.velocity = {200.f, 200.f};
			}
		}
	}

	float attack_counter_ms = 800.f;
	for (Entity entity : registry.attackTimers.entities)
	{
		// progress timer
		AttackTimer &counter = registry.attackTimers.get(entity);
		counter.counter_ms -= elapsed_ms_since_last_update;
		if (counter.counter_ms < attack_counter_ms)
		{
			attack_counter_ms = counter.counter_ms;
		}

		// remove timer and reset final bosses enemy state if timer expires
		if (counter.counter_ms < 0)
		{
			registry.attackTimers.remove(entity);
			if (registry.bosses.has(entity))
			{
				registry.deadlys.get(entity).state = ENEMY_STATE::IDLE;
			}
		}
	}

	float min_counter_ms = 3000.f;
	for (Entity entity : registry.deathTimers.entities)
	{
		// progress timer
		DeathTimer &counter = registry.deathTimers.get(entity);
		counter.counter_ms -= elapsed_ms_since_last_update;
		if (counter.counter_ms < min_counter_ms)
		{
			min_counter_ms = counter.counter_ms;
		}

		// restart the game once the death timer expired
		if (counter.counter_ms < 0)
		{
			registry.deathTimers.remove(entity);
			if (registry.players.has(entity))
			{
				screen.darken_screen_factor = 1.0;
				stateSwitch(GameState::GAME_OVER);
				return true;
			}
			// Enemy death - animation finished, remove entity
			if (registry.deadlys.has(entity))
			{
				if (registry.animationSets.has(entity))
				{
					AnimationSet animSet_enemy = registry.animationSets.get(entity);
					Animation anim = animSet_enemy.animations[animSet_enemy.current_animation];
				}

				// might need to move enemy kill count stuff here to stop powerup drop - kind of useless at last enemey kill

				float roll = uniform_dist(rng);
				Deadly &deadly = registry.deadlys.get(entity);
				Motion &enemy_motion = registry.motions.get(entity);

				if (roll <= deadly.drop_chance && !registry.projectiles.has(entity) && !registry.swarms.has(entity) && !goal_reached)
				{
					createExperience(renderer, enemy_motion.position, deadly.experience);
				}

				float roll_powerup = uniform_dist(rng);
				if (roll_powerup <= POWERUP_DROP_CHANCE && !registry.projectiles.has(entity) && !registry.swarms.has(entity) && !goal_reached && registry.powerups.entities.size() == 0)
				{
					PowerupType type = (PowerupType)randomInt(2); // Change this if more powerups
					float multiplier = 1.f;
					if (type == PowerupType::DAMAGE_BOOST || type == PowerupType::SPEED_BOOST)
					{
						int multiplier_roll = randomInt(10);
						if (multiplier_roll < 7)
							multiplier *= 1.5f;
						else
							multiplier *= 1.3f;
					}
					createTempPowerup(renderer, enemy_motion.position, type, multiplier, 10000); // spawned powerup clears after 10 seconds if not picked up
				}

				// replace dead leader with new swarm member
				if (registry.swarms.has(entity))
				{
					if (registry.swarms.get(entity).leader_id == entity)
					{
						for (int i = 0; i < registry.swarms.entities.size(); i++)
						{
							Entity e = registry.swarms.entities[i];
							SwarmMember &s = registry.swarms.get(e);
							if (s.leader_id == entity && e != entity)
							{
								s.leader_id = e;
								for (int j = 0; j < registry.swarms.components.size(); j++)
								{
									SwarmMember &swarm = registry.swarms.components[j];
									if (swarm.leader_id == entity)
									{
										swarm.leader_id = e;
									}
								}
								// erase later - for debugging
								registry.motions.get(e).velocity = vec2(1, 1);
								registry.motions.get(e).speed = 150.f;
								break;
							}
						}
					}
				}

				// Check if final boss is dead then stop game
				if (registry.bosses.has(entity))
				{
					screen.state = GameState::GAME_OVER;
					createText({window_width_px / 2 - 100, window_height_px / 2}, 1, "!!!You Win!!!", glm::vec3(1.f, 1.f, 1.f));
				}

				registry.remove_all_components_of(entity);
				enemies_killed++;
				if (enemies_killed >= enemy_kill_goal && current_map != map3)
				{
					for (Entity enemy : registry.deadlys.entities)
					{
						registry.healths.get(enemy).hit_points = 0;
						registry.deadlys.get(enemy).state = ENEMY_STATE::DEAD;

						if (!registry.deathTimers.has(enemy))
						{
							DeathTimer &death = registry.deathTimers.emplace(enemy);
							death.counter_ms = 440.4f;
						}
					}
					max_num_enemies = 0;
					goal_reached = true;

					return true;
				}
			}
		}
	}

	// reduce window brightness if the salmon is dying
	screen.darken_screen_factor = 1 - min_counter_ms / 3000;

	// Lights flickering
	//  if salmon is not dying - let lights effect on screen
	if (!registry.deathTimers.has(my_player))
	{
		if (!goal_reached)
		{
			lightflicker_counter_ms += elapsed_ms_since_last_update;
		}

		if (lightflicker_counter_ms >= LIGHT_FLICKER_RATE)
		{
			screen.darken_screen_factor = 0.4;
			lightflicker_counter_ms = 0;
		}
		else if (lightflicker_counter_ms < 400)
		{
			if ((lightflicker_counter_ms - (lightflicker_counter_ms % 10)) % 20 < 10)
			{
				screen.darken_screen_factor = 0;
			}
			else
			{
				screen.darken_screen_factor = 0.4;
			}
		}
		else
		{
			screen.darken_screen_factor = 0;
		}
	}

	if (debugging.in_debug_mode == true)
	{
		for (Motion &motion : motions_registry.components)
		{
			createLine(motion.position, motion.scale);
		}
	}

	Player &player = registry.players.get(my_player);

	// handle dash cooldown
	if (!player.is_dash_up)
	{
		player.curr_dash_cooldown_ms -= elapsed_ms_since_last_update;
		AnimationSet &stamina_anim = registry.animationSets.get(stamina_bar);

		// Player able to dash again
		if (player.curr_dash_cooldown_ms < 0.f)
		{
			player.curr_dash_cooldown_ms = player.dash_cooldown_ms;
			registry.motions.get(my_player).speed = min(registry.motions.get(my_player).speed, 300.f);
			player.is_dash_up = true;
			stamina_anim.current_animation = "staminabar_full";

		} // Player done dashing
		else if (player.curr_dash_cooldown_ms < (player.dash_cooldown_ms - player.dash_time))
		{
			registry.motions.get(my_player).speed = min(registry.motions.get(my_player).speed, 300.f);
			registry.motions.get(my_player).velocity = {0, 0};
			registry.lightUps.remove(my_player);

			stamina_anim.current_animation = "staminabar_regen";

		} // Player dashing
		else
		{
			if (!tutorial.dash)
			{
				tutorial.dash = true;
				// add a thing to check for type here
				for (Entity icon : registry.tutorialIcons.entities)
				{
					registry.remove_all_components_of(icon);
				}
			}
			registry.motions.get(my_player).speed = 2000.f;
			// TODO: add a thing to counter speed boost here or something .. idk
			if (!registry.lightUps.has(my_player))
			{
				registry.lightUps.emplace(my_player);
			}
			for (int i = 0; i < 5; i++)
			{
				int bound = (player.dash_cooldown_ms - player.dash_time) + (i * (player.dash_time / 5));
				if (player.curr_dash_cooldown_ms < bound && player.curr_dash_cooldown_ms + elapsed_ms_since_last_update > bound)
				{
					// TODO: modify this a little bit to make dash shadows fully even
					// TODO: make it so player is invulnerable during dash ?
					vec2 effectPos = registry.motions.get(my_player).position;
					createEffect(renderer, effectPos, 500, EFFECT_TYPE::DASH);
				}
			}
		}
	}

	player_controller.step(elapsed_ms_since_last_update);

	if (tutorial.toggle_show_ms_passed < tutorial.toggle_show_ms)
	{
		if (tutorial.toggle_key == 1)
		{
			createText({800, 680}, 0.6, "Press T to disable tutorial mode", vec3(1.0, 1.0, 1.0));
		}
		tutorial.toggle_show_ms_passed += elapsed_ms_since_last_update;
		if (tutorial.toggle_show_ms_passed >= tutorial.toggle_show_ms)
		{
			tutorial.toggle_key = 0;
		}
	}

	if (!tutorial.movement)
	{
		if (registry.tutorialIcons.entities.size() == 0)
		{
			createMovementKeys(renderer, {registry.motions.get(my_player).position.x + 80.f, registry.motions.get(my_player).position.y - 80.f});
		}
	}
	else if (!tutorial.attack)
	{
		if (registry.tutorialIcons.entities.size() == 0)
		{
			vec2 attack_direction = registry.players.get(my_player).attack_direction;
			if (registry.players.get(my_player).attack_direction == vec2(0, 0))
			{
				attack_direction = vec2(1, 0);
			}
			createAttackCursor(renderer, {registry.motions.get(my_player).position.x + (80.f * attack_direction.x), registry.motions.get(my_player).position.y + (80.f * attack_direction.y)});
		}
		else
		{
			Entity attack_cursor = registry.tutorialIcons.entities[0];
			vec2 attack_direction = registry.players.get(my_player).attack_direction;
			registry.motions.get(attack_cursor).position = {registry.motions.get(my_player).position.x + (80.f * attack_direction.x), registry.motions.get(my_player).position.y + (80.f * attack_direction.y)};
		}
	}
	else if (!tutorial.dash)
	{
		if (tutorial.dash_tut_cur_wait_ms < tutorial.dash_tut_wait_ms)
		{
			tutorial.dash_tut_cur_wait_ms += elapsed_ms_since_last_update;
		}
		else
		{
			if (registry.tutorialIcons.entities.size() == 0)
			{
				if (player.is_moving && !player.is_attacking)
				{
					// maybe only create when not a certain radius from any enemies... but this would take a bit to check
					createDashKey(renderer, {registry.motions.get(my_player).position.x + 80.f, registry.motions.get(my_player).position.y - 80.f});
				}
			}
			else
			{
				// guaranteed to only have one tutorial icon at a time
				Entity dash = registry.tutorialIcons.entities[0];
				registry.motions.get(dash).position = {registry.motions.get(my_player).position.x + 80.f, registry.motions.get(my_player).position.y - 80.f};
			}
		}
	}
	else if (!tutorial.health_buff)
	{
		for (Entity e : registry.healthBuffs.entities)
		{
			HealthBuff &hb = registry.healthBuffs.get(e);
			if (hb.touching && registry.tutorialIcons.entities.size() == 0)
			{
				createInteractKey(renderer, {registry.motions.get(e).position.x + 50, registry.motions.get(e).position.y - 50});
				break;
			}
		}
	}
	else if (!tutorial.pause)
	{
		if (registry.tutorialIcons.entities.size() == 0)
		{
			createPauseKey(renderer, {registry.motions.get(my_player).position.x + 100, registry.motions.get(my_player).position.y - 100});
		}
		else
		{
			Entity pause_key = registry.tutorialIcons.entities[0];
			registry.motions.get(pause_key).position = {registry.motions.get(my_player).position.x + 100, registry.motions.get(my_player).position.y - 100};
		}
	}
	if (!tutorial.door)
	{
		if (registry.tutorialIcons.entities.size() == 0 && registry.doors.entities.size() != 0)
		{
			if (registry.doors.components[0].touching)
			{
				createInteractKey(renderer, {registry.motions.get(my_player).position.x + 60, registry.motions.get(my_player).position.y});
			}
		}
	}

	for (Entity &e : registry.deadlys.entities)
	{
		Deadly &enemy = registry.deadlys.get(e);
		if (registry.projectiles.has(e))
		{
			continue;
		}

		AnimationSet &animSet_enemy = registry.animationSets.get(e);
		std::string enemy_name = "";
		switch (enemy.enemy_type)
		{
		case ENEMY_TYPES::PROJECTILE:
			continue;
		case ENEMY_TYPES::HOMING_PROJECTILE:
			continue;
		case ENEMY_TYPES::RANGED:
			enemy_name = "ranged";
			break;
		case ENEMY_TYPES::CONTACT_DMG:
			enemy_name = "slow";
			break;
		case ENEMY_TYPES::CONTACT_DMG_2:
			enemy_name = "fast";
			break;
		case ENEMY_TYPES::SWARM:
			enemy_name = "swarm";
			break;
		case ENEMY_TYPES::SLOWING_CONTACT:
			enemy_name = "fast";
			break;
		case ENEMY_TYPES::RANGED_HOMING:
			enemy_name = "ranged";
			break;
		case ENEMY_TYPES::FINAL_BOSS:
			enemy_name = "final_boss_";
			break;
		case ENEMY_TYPES::DASHING:
			enemy_name = "slow";
			break;
		default:
			break;
		}
		switch (enemy.state)
		{
		case ENEMY_STATE::IDLE:
			animSet_enemy.current_animation = enemy_name + "enemy_idle_f";
			break;
		case ENEMY_STATE::RUN:
			if (registry.bosses.has(e))
			{
				auto &render_rqst = registry.renderRequests.get(e);
				render_rqst.used_sprite = SPRITE_ASSET_ID::FINAL_BOSS;
			}

			animSet_enemy.current_animation = enemy_name + "enemy_run_f";
			break;
		case ENEMY_STATE::DEAD:
			if (registry.bosses.has(e))
			{
				auto &render_rqst = registry.renderRequests.get(e);
				render_rqst.used_sprite = SPRITE_ASSET_ID::FINAL_BOSS_DEATH;
			}

			// seems fine
			if (animSet_enemy.current_animation != enemy_name + "enemy_die")
			{
				animSet_enemy.current_animation = enemy_name + "enemy_die";
				animSet_enemy.current_frame = 0;
			}
			break;
		case ENEMY_STATE::ATTACK:
			if (registry.bosses.has(e))
			{
				auto &render_rqst = registry.renderRequests.get(e);
				render_rqst.used_sprite = SPRITE_ASSET_ID::FINAL_BOSS_ATTACK;

				animSet_enemy.current_animation = enemy_name + "enemy_attack_f";
			}
			break;
		default:
			animSet_enemy.current_animation = enemy_name + "enemy_idle_f";
		}
	}

	for (Entity entity : registry.lightUps.entities)
	{
		LightUp &counter = registry.lightUps.get(entity);
		counter.duration_ms -= elapsed_ms_since_last_update;
		if (counter.duration_ms < 0)
		{
			registry.lightUps.remove(entity);
		}
	}

	return true;
}

// Reset the world state to its initial state
void WorldSystem::restart_game()
{
	restart_world();

	enemies_killed = 0;
	goal_reached = false;
	if (current_map == map3)
	{
		max_num_enemies = 15;
	}
	else
	{
		max_num_enemies = 50;
	}

	while (registry.upgradeCards.entities.size() > 0)
	{
		registry.remove_all_components_of(registry.upgradeCards.entities.back());
	}

	// Remove all entities that we created
	// All that have a motion, we could also iterate over all fish, eels, ... but that would be more cumbersome
	while (registry.motions.entities.size() > 0)
		registry.remove_all_components_of(registry.motions.entities.back());

	// Debugging for memory/component leaks
	registry.list_all_components();

	// clear spawnable_tiles on map switch
	spawnable_tiles.clear();

	createFloor(renderer);

	// create a slime patches and create spawnable tiles vector
	for (int i = 0; i < current_map.size(); i++)
	{
		for (int j = 0; j < current_map[0].size(); j++)
		{
			vec2 world_pos = {(640 - (25 * 100)) + (j * TILE_SIZE) + (TILE_SIZE / 2), (640 - (44 * 100)) + (i * TILE_SIZE) + (TILE_SIZE / 2)};

			// create slime patches on ground
			if (current_map[i][j] == 7)
			{
				createSlimePatch(renderer, world_pos);
				// tile_vec.push_back(vec2(i, j));
			}
			// create vector of spawnable tiles
			else if (current_map[i][j] == 1 || (current_map[i][j] >= 3 && current_map[i][j] <= 8))
			{
				spawnable_tiles.push_back(vec2(i, j));
			}
		}
	}

	// Create final boss
	for (int i = 0; i < current_map.size(); i++)
	{
		for (int j = 0; j < current_map[0].size(); j++)
		{
			vec2 world_pos = {(640 - (25 * 100)) + (j * TILE_SIZE) + (TILE_SIZE / 2), (640 - (44 * 100)) + (i * TILE_SIZE) + (TILE_SIZE / 2)};

			if (current_map[i][j] == 10)
			{
				createBossEnemy(renderer, world_pos);
			}
		}
	}

	// create a new Player
	my_player = createPlayer(renderer, {window_width_px / 2, window_height_px - 200});
	registry.colors.insert(my_player, {1, 0.8f, 0.8f});
	player_controller.set_player_reference(&my_player);
	player_controller.set_renderer(renderer);
	player_controller.set_world(this);

	load_player_data(SAVE_FILENAME);

	registry.cameras.clear();
	camera = createCamera(renderer, vec2(window_width_px / 2, window_height_px / 2));

	lightflicker_counter_ms = 1000;
	tile_vec.clear();

	// player pos: [25, 44]
	// player pos on screen: [640, 640]
	// tile size: 48
	// origin: [-560 + 50, -1472 + 50]
	// to spawn at [0, 0] -> [-536, -1448]

	vec2 playerPos_init = vec2(25, 44);
	// screen bounds will always be [ceil(640 / 100), ceil(720/100)]
	// generously rounding it up to [8, 10]
	vec2 playerpos_world = vec2(640, 640);
	for (int i = playerPos_init.x - 8; i <= playerPos_init.x + 8; i++)
	{
		for (int j = playerPos_init.y - 6; j <= playerPos_init.y + 6; j++)
		{
			if ((i < 0) || (j < 0) || (i >= current_map[0].size()) || j >= current_map.size())
			{
				createWalls(renderer, {(640 - (25 * 100)) + (i * TILE_SIZE) + (sign(i) * TILE_SIZE / 2), (640 - (44 * 100)) + (j * TILE_SIZE) + (sign(j) * TILE_SIZE / 2)}, current_map, {i, j});
				tile_vec.push_back(vec2(i, j));
			}
			else if (current_map[j][i] == 0)
			{
				vec2 world_pos = {(640 - (25 * 100)) + (i * TILE_SIZE) + (TILE_SIZE / 2), (640 - (44 * 100)) + (j * TILE_SIZE) + (TILE_SIZE / 2)};

				createWalls(renderer, world_pos, current_map, {i, j});
				tile_vec.push_back(vec2(i, j));
			}
		}
	}

	// create health bar
	vec2 hp_bar_pos = {-0.75, 0.85f};
	hp_bar = createHPBar(renderer, hp_bar_pos);

	vec2 stamina_bar_pos = {-0.74f, 0.7f};
	stamina_bar = createStaminaBar(renderer, stamina_bar_pos);

	// set pause correctly
	is_paused = true;
	unpause();
	// create experience bar
	vec2 exp_bar_pos = {-0.74f, 0.55f};
	experience_bar = createExperienceBar(renderer, exp_bar_pos);

	auto &player = registry.players.get(my_player);
	float progress = std::min((float)player.experience / player.toNextLevel, 1.0f);
	float bar_offset = (progress * 0.2f);
	vec2 bar_pos = vec2(-0.94f + bar_offset, 0.525f);
	experience_in = createUIBar(bar_pos, vec2(progress * 0.4f, 0.205f), 0);
}

// utility functions for dash mvmnt implementation
vec2 lerp(vec2 start, vec2 end, float t)
{
	return start * (1 - t) + end * t;
}

float distance(vec2 coord1, vec2 coord2)
{
	return sqrt(powf(coord2.x - coord1.x, 2.f) + powf(coord2.y - coord1.y, 2.f));
}

vec2 norm(vec2 vec)
{
	return (vec / (vec.x + vec.y));
}

// Compute collisions between entities
void WorldSystem::handle_collisions(float step_seconds)
{
	// Loop over all collisions detected by the physics system
	auto &collisionsRegistry = registry.collisions;
	bool unstick_player = true;
	for (uint i = 0; i < collisionsRegistry.components.size(); i++)
	{
		// The entity and its collider
		Entity entity = collisionsRegistry.entities[i];
		Entity entity_other = collisionsRegistry.components[i].other;

		if (registry.players.has(entity))
		{
			// Player& player = registry.players.get(entity);

			// Checking Player - Deadly collisions
			if (registry.deadlys.has(entity_other))
			{
				float &player_hp = registry.healths.get(entity).hit_points;
				Player &player = registry.players.get(entity);
				if (!player.invulnerable && !registry.deathTimers.has(entity_other) && !player.god_mode)
				{
					if (registry.powerups.has(entity))
					{
						Powerup &powerup = registry.powerups.get(entity);
						if (powerup.type == PowerupType::INVINCIBILITY)
							// In this case, player does not receive damage
							continue;
					}

					if (registry.bosses.has(entity_other))
					{
						registry.deadlys.get(entity_other).state = ENEMY_STATE::ATTACK;
						registry.attackTimers.emplace(entity_other);
					}

					// player takes damage
					player_hp -= registry.damages.get(entity_other).damage;
					// damage sound
					Mix_PlayChannel(-1, player_damage_sound, 0);
					// avoid negative hp values for hp bar
					player_hp = max(0.f, player_hp);
					// modify hp bar
					// std::cout << "Player hp: " << player_hp << "\n";
					if (player_hp <= 200 && player_hp >= 0)
					{
						// Motion& motion = registry.motions.get(hp_bar);
						// motion.scale.x = HPBAR_BB_WIDTH * (player_hp / 100);
						RenderRequest &hp_bar_render = registry.renderRequests.get(hp_bar);
						// Total HP bar is 200
						int hp_level = int(player_hp / 25);
						// set current animation to hpbar_[hp_level]
						std::string new_anim = "hpbar_" + std::to_string(hp_level);

						registry.animationSets.get(hp_bar).current_animation = new_anim;
					}

					if (registry.deadlys.get(entity_other).enemy_type == ENEMY_TYPES::SLOWING_CONTACT)
					{
						Slows &slows = registry.slows.get(entity_other);
						player.slowed_amount = slows.speed_dec;
						player.slowed_duration_ms = slows.duration;
					}
					player.invulnerable = true;
					player.invulnerable_duration_ms = 1000.f;
				}
				if (registry.projectiles.has(entity_other))
				{
					registry.remove_all_components_of(entity_other);
				}
				if (!registry.deathTimers.has(entity) && player_hp <= 0.f)
				{
					registry.deathTimers.emplace(entity);
					// Mix_PlayChannel(-1, salmon_dead_sound, 0);

					// Control what happens when the player dies here
					Motion &motion = registry.motions.get(my_player);
					motion.velocity[0] = 0.0f;
					motion.velocity[1] = 0.0f;
				}
			}
			// Checking Player - Eatable collisions (e.g. powerups)
			else if (registry.eatables.has(entity_other))
			{
				if (!registry.deathTimers.has(entity) && !registry.powerups.has(entity))
				{
					Powerup &powerup = registry.powerups.get(entity_other);
					PowerupType type = powerup.type;
					float timer = POWERUP_TIMER;
					float multiplier = powerup.multiplier;

					registry.remove_all_components_of(entity_other);

					Powerup &player_powerup = registry.powerups.emplace(entity);

					player_powerup.type = type;

					player_powerup.multiplier = multiplier;
					player_powerup.timer = timer;
					player_powerup.equipped = true;

					Mix_PlayChannel(-1, salmon_eat_sound, 0);
				}
			}
			if (registry.stickies.has(entity_other))
			{
				Motion &player_motion = registry.motions.get(my_player);
				player_motion.speed = 120.f;
				unstick_player = false;
			}

			// touching health buffs
			if (registry.healthBuffs.has(entity_other))
			{
				HealthBuff &hb = registry.healthBuffs.get(entity_other);
				hb.touching = true;
			}

			if (registry.doors.has(entity_other))
			{
				Door &door = registry.doors.get(entity_other);
				door.touching = true;
			}
		}
		else if (registry.deadlys.has(entity))
		{
			if (registry.solidObjs.has(entity_other) && registry.projectiles.has(entity))
			{
				registry.remove_all_components_of(entity);
			}
		}
		else if (registry.playerAttacks.has(entity))
		{
			auto &playerAttacks = registry.playerAttacks.get(entity);

			if (registry.deadlys.has(entity_other) && registry.healths.has(entity_other) && registry.motions.has(entity_other) && !playerAttacks.has_hit)
			{
				Health &deadly_health = registry.healths.get(entity_other);
				Damage &damage = registry.damages.get(entity);
				Motion &enemy_motion = registry.motions.get(entity_other);
				Motion &pmotion = registry.motions.get(my_player);
				Player &player = registry.players.get(my_player);
				Deadly &deadly = registry.deadlys.get(entity_other);

				if (registry.enemyDashes.has(entity_other))
				{
					registry.enemyDashes.get(entity_other).current_charge_timer = 0.f;
				}

				float temp_multiplier = 1.f;
				if (registry.powerups.has(my_player))
				{
					Powerup &powerup = registry.powerups.get(my_player);
					if (powerup.type == PowerupType::DAMAGE_BOOST)
					{
						temp_multiplier *= powerup.multiplier;
					}
				}

				// damage scales between -0.75 and 1.25
				float damage_rng = (uniform_dist(rng) / 2) - 0.25;

				float damage_dealt = (damage.damage + (damage.damage * damage_rng)) * temp_multiplier;

				deadly_health.hit_points = std::max(0.0f, deadly_health.hit_points - damage_dealt);

				// std::cout << damage_rng << std::endl;

				createDamageIndicator(renderer, damage_dealt, enemy_motion.position, damage_rng, temp_multiplier);

				// play enemy damage sound
				Mix_PlayChannel(-1, enemy_damage_sound, 0);

				vec2 diff = registry.motions.get(entity_other).position - pmotion.position;

				if (!registry.projectiles.has(entity_other) && deadly_health.hit_points > 0)
				{
					vec2 kockback_pos = registry.motions.get(entity_other).position + (diff * player.knockback_strength);
					int grid_x = static_cast<int>((kockback_pos.x - (640 - (25 * TILE_SIZE)) - TILE_SIZE / 2) / TILE_SIZE);
					int grid_y = static_cast<int>((kockback_pos.y - (640 - (44 * TILE_SIZE)) - TILE_SIZE / 2) / TILE_SIZE);
					int adjust_x = 0;
					int adjust_y = 0;
					if (diff.x < 0 && diff.y < 0)
					{
						if (abs(diff.x) < 50 && abs(diff.y) < 50)
						{
							adjust_x = -1;
							adjust_y = -1;
						}
						else
						{
							if (abs(diff.x) > 50)
							{
								adjust_x = -1;
							}
							if (abs(diff.y) > 50)
							{
								adjust_y = -1;
							}
						}
					}
					else if (diff.x > 0 && diff.y < 0)
					{
						if (abs(diff.x) < 50 && abs(diff.y) < 50)
						{
							adjust_x = 1;
							adjust_y = -1;
						}
						else
						{
							if (abs(diff.x) > 50)
							{
								adjust_x = 2;
							}
							if (abs(diff.y) > 50)
							{
								adjust_y = -1;
							}
						}
					}
					else if (diff.x > 0 && diff.y > 0)
					{
						if (abs(diff.x) < 50 && abs(diff.y) < 50)
						{
							adjust_x = 1;
							adjust_y = 1;
						}
						else
						{
							if (abs(diff.x) > 50)
							{
								adjust_x = 2;
							}
							if (abs(diff.y) > 50)
							{
								adjust_y = 2;
							}
						}
					}
					else if (diff.x < 0 && diff.y > 0)
					{
						if (abs(diff.x) < 50 && abs(diff.y) < 50)
						{
							adjust_x = -1;
							adjust_y = 1;
						}
						else
						{
							if (abs(diff.x) > 50)
							{
								adjust_x = -1;
							}
							if (abs(diff.y) > 50)
							{
								adjust_y = 2;
							}
						}
					}

					vec2 adjust = adjust_knockback_coordinates(grid_x, grid_y, adjust_x, adjust_y);

					int adjusted_tile = current_map[grid_y + adjust.y][grid_x + adjust.x];

					if (registry.motions.has(entity_other) && (adjusted_tile == 1 || (adjusted_tile >= 3 && adjusted_tile <= 8)))
					{
						vec2 grid_knockback_pos = {(640 - (25 * 100)) + ((grid_x + adjust.x) * TILE_SIZE) + (TILE_SIZE / 2), (640 - (44 * 100)) + ((grid_y + adjust.y) * TILE_SIZE) + (TILE_SIZE / 2)};
						vec2 new_diff = grid_knockback_pos - registry.motions.get(my_player).position;
						if (length(new_diff) > length(diff))
						{
							enemy_motion.position = grid_knockback_pos;
							deadly.knocked_back_pos = grid_knockback_pos;
							physics.update_enemy_movement(entity_other, step_seconds);
							if (registry.pathTimers.has(entity_other))
							{
								registry.pathTimers.get(entity_other).timer = -1.f;
							}
						}
					}
				}
				if (registry.lightUps.has(entity_other))
				{
					registry.lightUps.remove(entity_other);
				}
				registry.lightUps.emplace(entity_other);

				if (deadly_health.hit_points <= 0.0f && (!registry.deathTimers.has(entity_other)))
				{
					Deadly &d = registry.deadlys.get(entity_other);
					d.state = ENEMY_STATE::DEAD;
					DeathTimer &death = registry.deathTimers.emplace(entity_other);
					death.counter_ms = 550.4f;
					if (!(registry.projectiles.has(entity_other) || d.enemy_type == ENEMY_TYPES::SWARM))
					{
						createSmoke(renderer, {enemy_motion.position.x, enemy_motion.position.y});
					}
				}
			}
		}
	}

	if (unstick_player)
	{
		Motion &player_motion = registry.motions.get(my_player);
		player_motion.speed = 300.f;
	}

	for (Entity entity : registry.playerAttacks.entities)
	{
		auto &attackComponent = registry.playerAttacks.get(entity);
		attackComponent.has_hit = true;
	}

	// Remove all collisions from this simulation step
	registry.collisions.clear();
}

vec2 WorldSystem::adjust_knockback_coordinates(int grid_x, int grid_y, int adjust_x, int adjust_y)
{
	int x = adjust_x;
	int y = adjust_y;
	if (grid_y + y > (int)current_map.size())
	{
		y = current_map.size() - grid_y - 1;
	}
	else if (grid_y + y < 0)
	{
		y = 0;
	}

	if (grid_x + x > (int)current_map[0].size())
	{
		x = current_map[0].size() - grid_x - 1;
	}
	else if (grid_x + x < 0)
	{
		x = 0;
	}

	vec2 player_pos = registry.motions.get(my_player).position;
	do
	{
		if (current_map[grid_y + y][grid_x + x] == 1 || (current_map[grid_y + y][grid_x + x] >= 3 && current_map[grid_y + y][grid_x + x] <= 8))
		{
			vec2 pos = {(640 - (25 * 100)) + ((grid_x + x) * TILE_SIZE) + (TILE_SIZE / 2), (640 - (44 * 100)) + ((grid_y + y) * TILE_SIZE) + (TILE_SIZE / 2)};
			if (knockback_through_wall_check(vec2(grid_x + x, grid_y + y), player_pos))
			{
				return vec2(x, y);
			}
		}
		int temp_x = x;
		if (temp_x > 0)
		{
			temp_x--;
		}
		else if (temp_x < 0)
		{
			temp_x++;
		}
		if (current_map[grid_y + y][grid_x + temp_x] == 1 || (current_map[grid_y + y][grid_x + temp_x] >= 3 && current_map[grid_y + y][grid_x + temp_x] <= 8))
		{
			vec2 pos = {(640 - (25 * 100)) + ((grid_x + temp_x) * TILE_SIZE) + (TILE_SIZE / 2), (640 - (44 * 100)) + ((grid_y + y) * TILE_SIZE) + (TILE_SIZE / 2)};
			if (knockback_through_wall_check(vec2(grid_x + temp_x, grid_y + y), player_pos))
			{
				return vec2(temp_x, y);
			}
		}
		int temp_y = y;
		if (temp_y > 0)
		{
			temp_y--;
		}
		else if (temp_y < 0)
		{
			temp_y++;
		}
		if (current_map[grid_y + temp_y][grid_x + x] == 1 || (current_map[grid_y + temp_y][grid_x + x] >= 3 && current_map[grid_y + temp_y][grid_x + x] <= 8))
		{
			vec2 pos = {(640 - (25 * 100)) + ((grid_x + x) * TILE_SIZE) + (TILE_SIZE / 2), (640 - (44 * 100)) + ((grid_y + temp_y) * TILE_SIZE) + (TILE_SIZE / 2)};
			if (knockback_through_wall_check(vec2(grid_x + x, grid_y + temp_y), player_pos))
			{
				return vec2(x, temp_y);
			}
		}
		if (x > 0)
		{
			x--;
		}
		else if (x < 0)
		{
			x++;
		}
		if (y > 0)
		{
			y--;
		}
		else if (y < 0)
		{
			y++;
		}
	} while (x != 0 && y != 0);

	return vec2(0, 0);
}

bool WorldSystem::knockback_through_wall_check(const vec2 &start, const vec2 &end)
{
	const float GRID_OFFSET_X = (640 - (25 * TILE_SIZE)) - TILE_SIZE / 2;
	const float GRID_OFFSET_Y = (640 - (44 * TILE_SIZE)) - TILE_SIZE / 2;

	// Convert to grid coordinates
	int x2 = static_cast<int>((end.x - GRID_OFFSET_X) / TILE_SIZE);
	int y2 = static_cast<int>((end.y - GRID_OFFSET_Y) / TILE_SIZE);

	int dx = abs(x2 - start.x);
	int dy = abs(y2 - start.y);
	int x = start.x;
	int y = start.y;

	// Determine step direction
	int x_step = (start.x < x2) ? 1 : -1;
	int y_step = (start.y < y2) ? 1 : -1;

	int err;

	// Choose which axis to move along
	if (dx > dy)
	{
		err = dx / 2;
		while (x != x2)
		{
			if (y < 0 || x < 0 || y >= current_map.size() || x >= current_map[0].size())
			{
				return false;
			}

			if (!(current_map[y][x] == 1 || (current_map[y][x] >= 3 && current_map[y][x] <= 8)))
			{
				return false;
			}

			err -= dy;
			if (err < 0)
			{
				y += y_step;
				err += dx;
			}
			x += x_step;
		}
	}
	else
	{
		err = dy / 2;
		while (y != y2)
		{
			if (y < 0 || x < 0 || y >= current_map.size() || x >= current_map[0].size())
			{
				return false;
			}

			if (!(current_map[y][x] == 1 || (current_map[y][x] >= 3 && current_map[y][x] <= 8)))
			{
				return false;
			}

			err -= dx;
			if (err < 0)
			{
				x += x_step;
				err += dy;
			}
			y += y_step;
		}
	}

	if (y < 0 || x < 0 || y >= current_map.size() || x >= current_map[0].size())
	{
		return false;
	}
	return current_map[y][x] == 1 || (current_map[y][x] >= 3 && current_map[y][x] <= 8);
}

// Should the game be over ?
bool WorldSystem::is_over() const
{
	return bool(glfwWindowShouldClose(window));
}

// Helper function to read a file line by line
std::vector<std::string> read_file(std::string filepath)
{
	std::ifstream ReadFile(filepath);
	std::string text;
	std::vector<std::string> lines;
	while (std::getline(ReadFile, text))
	{
		lines.push_back(text);
	}
	ReadFile.close();
	return lines;
}

// On key callback
void WorldSystem::on_key(int key, int, int action, int mod)
{
	ScreenState &screen = registry.screenStates.components[0];

	// Press any key to start
	if (screen.state == GameState::START)
	{
		if (action == GLFW_RELEASE)
		{
			stateSwitch(GameState::GAME);
			restart_game();
		}
		return;
	}

	if (screen.state == GameState::MENU)
	{
		return;
	}

	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R)
	{
		if (screen.state == GameState::GAME_OVER)
		{
			int w, h;
			glfwGetWindowSize(window, &w, &h);
			restart_game();
		}
	}

	if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE)
	{
		// if game over or paused, player is able to exit to menu
		if (screen.state == GameState::PAUSED || screen.state == GameState::GAME_OVER)
		{
			stateSwitch(GameState::MENU);
			return;
		}
	}

	if (screen.state == GameState::GAME_OVER)
	{
		return;
	}

	if (action == GLFW_RELEASE && key == GLFW_KEY_P)
	{

		if (screen.state != GameState::PAUSED)
		{
			if (!tutorial.pause)
			{
				tutorial.pause = true;
				if (tutorial.movement && tutorial.attack && tutorial.dash && tutorial.health_buff)
				{
					registry.remove_all_components_of(registry.tutorialIcons.entities[0]);
				}
			}
			pause();
		}
		else
		{
			unpause();
		}
	}

	if (screen.state == GameState::PAUSED)
	{
		return;
	}

	if (action == GLFW_RELEASE && key == GLFW_KEY_F)
	{
		display_fps = !display_fps;
	}

	// Tutorial
	if (action == GLFW_RELEASE && key == GLFW_KEY_T)
	{
		tutorial = {
			4000.f,
			4000.f,
			0,
			true,
			true,
			4000.f,
			4000.f,
			true,
			true,
			true,
			true};
	}

	// Debugging
	/*
	if (key == GLFW_KEY_X)
	{
		if (action == GLFW_RELEASE)
			debugging.in_debug_mode = !debugging.in_debug_mode;
	}
	*/

	// player key stuff starts here

	// Player controller
	Motion &pmotion = registry.motions.get(my_player);
	Player &player = registry.players.get(my_player);

	if (key == GLFW_KEY_SPACE)
	{
		if (action == GLFW_PRESS && !registry.deathTimers.has(my_player) && (screen.state == GameState::GAME) && player.is_dash_up)
		{
			pmotion.speed = 5500.f;
			if (pmotion.velocity == vec2(0.f, 0.f))
			{
				pmotion.velocity = player.last_direction;
			}
			player.is_dash_up = false;
			registry.lightUps.emplace(my_player);
			AnimationSet &stamina_anim = registry.animationSets.get(stamina_bar);
			stamina_anim.current_animation = "staminabar_depleting";

			player.invulnerable = true;
			player.invulnerable_duration_ms = player.dash_time + 50.f;
		}
	}

	if (key == GLFW_KEY_E && action == GLFW_RELEASE)
	{
		for (Entity e : registry.healthBuffs.entities)
		{
			HealthBuff &buff = registry.healthBuffs.get(e);
			if (buff.touching)
			{
				// first interaction with health buff obj - if the interact key is up get rid of it
				// if any other keys are up don't get rid of them
				if (tutorial.movement && tutorial.attack && tutorial.dash && !tutorial.health_buff)
				{
					tutorial.health_buff = true;
					for (Entity e : registry.tutorialIcons.entities)
					{
						registry.remove_all_components_of(e);
					}
				}
				Health &p_health = registry.healths.get(my_player);
				RenderRequest &hp_bar_render = registry.renderRequests.get(hp_bar);
				createEffect(renderer, {registry.motions.get(e).position.x + 5.f, registry.motions.get(e).position.y - 45.f}, 1400.f, EFFECT_TYPE::HEART);
				if (p_health.hit_points < 200)
				{
					std::cout << "player regained health" << std::endl;
					p_health.hit_points += min(buff.factor * 25.f, 200.f - p_health.hit_points);

					// Total HP bar is 200
					int hp_level = int(p_health.hit_points / 25);
					// set current animation to hpbar_[hp_level]
					std::string new_anim = "hpbar_" + std::to_string(hp_level);

					registry.animationSets.get(hp_bar).current_animation = new_anim;
				}
			}
		}

		for (Entity e : registry.doors.entities)
		{
			Door &door = registry.doors.get(e);
			if (door.touching)
			{
				Mix_PlayChannel(-1, door_sound, 0);
				tutorial.door = true;
				if (current_map == map2)
				{
					mapSwitch(3);
				}
				else
				{
					mapSwitch(2);
				}
				return;
			}
		}

		// tenant dialogue
		for (Entity e : registry.tenants.entities)
		{
			// if there is already dialogue up, progress index
			// 	if the current dialogue is the last -> unfreeze player movement and remove current dialogue
			//  else -> take down curent dialogue and put up next

			// extra dialogue:
			//  if already dialogue up -> close out of it and progress index
			//  else -> display next dialogue

			Tenant &tenant = registry.tenants.get(e);
			if (tenant.player_in_radius)
			{
				while (registry.tutorialIcons.entities.size() != 0)
				{
					registry.remove_all_components_of(registry.tutorialIcons.entities.back());
				}
				// cutscene dialogue
				if (tenant.dialogue_progress < int(tenant.dialogues.size()))
				{
					// player's first time speaking to tenant - freeze player movement/attack
					if (tenant.dialogue_progress == -1)
					{
						cutscene = true;
						createDialogueBox(renderer);
					}
					tenant.dialogue_progress += 1;

					while (registry.texts.size() != 0)
					{
						registry.remove_all_components_of(registry.texts.entities.back());
					}
					if (tenant.dialogue_progress < int(tenant.dialogues.size()))
					{
						// Create dialogue
						std::vector<std::string> text_lines = textToDialogueMode(tenant.dialogues[tenant.dialogue_progress]);
						for (int i = 0; i < text_lines.size(); i++)
						{
							std::string line = text_lines[i];
							float offset = 40 * i;

							Entity text = createText({40, 130 - offset}, 0.7, line, vec3(1.0, 1.0, 1.0));
							registry.debugComponents.remove(text);
						}
					}
					else
					{
						cutscene = false;
						while (registry.dialogueBoxes.size() != 0)
						{
							registry.remove_all_components_of(registry.dialogueBoxes.entities.back());
						}
					}
				}
				else if (tenant.dialogue_progress < int(tenant.dialogues.size() + tenant.extra_dialogues.size()))
				{
					while (registry.texts.size() != 0)
					{
						registry.remove_all_components_of(registry.texts.entities.back());
					}

					if (cutscene)
					{
						cutscene = false;
						while (registry.dialogueBoxes.size() != 0)
						{
							registry.remove_all_components_of(registry.dialogueBoxes.entities.back());
						}
						tenant.dialogue_progress += 1;
					}
					else
					{
						// Create dialogue
						std::vector<std::string> text_lines = textToDialogueMode(tenant.extra_dialogues[tenant.dialogue_progress - int(tenant.dialogues.size())]);
						for (int i = 0; i < text_lines.size(); i++)
						{
							std::string line = text_lines[i];
							float offset = 40 * i;

							Entity text = createText({40, 130 - offset}, 0.7, line, vec3(1.0, 1.0, 1.0));
							registry.debugComponents.remove(text);
						}

						cutscene = true;
						createDialogueBox(renderer);
					}
				}
				else
				{
					createEffect(renderer, {registry.motions.get(e).position.x, registry.motions.get(e).position.y - 45.f}, 1400.f, EFFECT_TYPE::HEART);
				}
			}
		}
	}

	// Toggle god mode
	// For testing purposes only
	if (key == GLFW_KEY_G && action == GLFW_RELEASE)
	{
		player.god_mode = !player.god_mode;
	}

	// Switch to next map
	// For testing purposes only
	if (key == GLFW_KEY_M && action == GLFW_RELEASE)
	{
		map_counter++;
		if (map_counter == 4)
		{
			map_counter = 1;
		}
		mapSwitch(map_counter);
	}

	if (screen.state != GameState::PAUSED && !cutscene)
		player_controller.on_key(key, action, mod);

	// Manage tutorial input responses

	if (player.is_moving && !tutorial.movement)
	{
		tutorial.movement = true;
		for (Entity icon : registry.tutorialIcons.entities)
		{
			registry.remove_all_components_of(icon);
		}
	}

	// Control the current speed with `<` `>`
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_COMMA)
	{
		current_speed -= 0.1f;
		printf("Current speed = %f\n", current_speed);
	}
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_PERIOD)
	{
		current_speed += 0.1f;
		printf("Current speed = %f\n", current_speed);
	}
	current_speed = fmax(0.f, current_speed);
}

void WorldSystem::on_mouse_move(vec2 mouse_position)
{
	ScreenState &screen = registry.screenStates.components[0];
	if (registry.players.entities.size() > 0)
	{
		player_controller.on_mouse_move(mouse_position);
	}

	if (screen.state == GameState::MENU)
	{
		for (Entity button : registry.elevatorButtons.entities)
		{
			vec2 mouse_position_ndc = mousePosToNormalizedDevice(mouse_position);
			UserInterface &ui = registry.userInterfaces.get(button);

			if (mouse_position_ndc.x >= ui.position.x - (ui.scale.x / 2) && mouse_position_ndc.x <= ui.position.x + (ui.scale.x / 2) &&
				mouse_position_ndc.y >= ui.position.y - (-ui.scale.y / 2) && mouse_position_ndc.y <= ui.position.y + (-ui.scale.y / 2))
			{

				registry.elevatorButtons.get(button).hovering = true;
			}
			else
			{
				registry.elevatorButtons.get(button).hovering = false;
			}
		}
	}
}

void WorldSystem::on_mouse_button(int button, int action, int mods)
{
	ScreenState &screen = registry.screenStates.components[0];
	if (registry.players.entities.size() > 0 && !cutscene)
	{
		player_controller.on_mouse_button(button, action, mods);

		// Attack tutorial done
		if (registry.players.get(my_player).is_attacking && !tutorial.attack)
		{
			tutorial.attack = true;
			for (Entity icon : registry.tutorialIcons.entities)
			{
				// should probably check for type
				registry.remove_all_components_of(icon);
			}
		}
	}

	if (screen.state == GameState::MENU)
	{
		for (ElevatorButton button : registry.elevatorButtons.components)
		{
			if (button.hovering)
			{
				Mix_PlayChannel(-1, button_click_sound, 0);
				if (button.level == 0)
				{
					exit(0);
				} else {
					if (registry.elevatorDisplays.entities.size() == 0) {
						createElevatorDisplay(renderer, {window_width_px / 2, window_height_px / 2});
					} 
					Entity e = registry.elevatorDisplays.entities[0];

					// TODO: need to more thoroughly track which levels have been unlocked to player
					

					if (button.level > 2) {
						registry.animationSets.get(e).current_animation = "elevator_locked";
						registry.elevatorDisplays.get(e).message = 0;
					} else {
						std::cout << "animation changed for level " << button.level << std::endl;
						registry.animationSets.get(e).current_animation = "elevator_level" + std::to_string(button.level);
						registry.elevatorDisplays.get(e).message = button.level;
					}

					registry.elevatorDisplays.get(e).selection_made = true;
					registry.elevatorDisplays.get(e).current_ms = 0.f;
					return;
				}
			}
		}
	}
}

std::vector<std::vector<int>> WorldSystem::get_current_map()
{
	return current_map;
}

void WorldSystem::pause()
{
	if (!is_paused)
	{
		ScreenState &screen = registry.screenStates.components[0];
		screen.darken_screen_factor = 0.9;
		screen.state = GameState::PAUSED;
		is_paused = true;
		std::cout << "Game paused" << std::endl;
		pauseMenuText();
	}
}

void WorldSystem::unpause()
{
	if (is_paused)
	{
		ScreenState &screen = registry.screenStates.components[0];
		screen.darken_screen_factor = 0.0;
		screen.state = GameState::GAME;
		is_paused = false;
		registry.players.get(my_player).is_moving = false;
		registry.players.get(my_player).move_direction = vec2(0, 0);
		std::cout << "Game unpaused" << std::endl;
	}
}

void WorldSystem::set_level_up_state(bool state)
{
	is_level_up = state;

	ScreenState &screen = registry.screenStates.components[0];
	if (state)
	{ // TODO: can we change this to pause function ?
		screen.darken_screen_factor = 0.0;
		screen.state = GameState::GAME;
		std::cout << "Game paused due to level up" << std::endl;
	}
	else
	{
		screen.darken_screen_factor = 0.0;
		screen.state = GameState::GAME;
		std::cout << "Game unpaused from level up" << std::endl;
	}
}
