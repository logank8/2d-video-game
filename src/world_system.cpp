// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>

#include "physics_system.hpp"
#include "animation_system.hpp"
#include "player_controller.hpp"

// Game configuration
//const size_t MAX_NUM_CONTACT_SLOW = 2;
//const size_t MAX_NUM_CONTACT_FAST = 5;
const size_t CONTACT_SLOW_SPAWN_DELAY_MS = 5000 * 3;
const size_t CONTACT_FAST_SPAWN_DELAY_MS = 2000 * 3;
//const size_t MAX_NUM_RANGED_ENEMY = 1;
const size_t RANGED_ENEMY_SPAWN_DELAY_MS = 5000 * 3;
const size_t RANGED_ENEMY_PROJECTILE_DELAY_MS = 3000;

const int TILE_SIZE = 100;
std::vector<vec2> tile_vec;
std::vector<vec2> spawnable_tiles;
const int LIGHT_FLICKER_RATE = 2000 * 10;
const int FPS_COUNTER_MS = 1000;

int lightflicker_counter_ms;
int fps_counter_ms;
int fps = 0;
bool display_fps = false;
bool WorldSystem::is_paused = false;

bool is_tutorial_on = false;

PhysicsSystem physics;

void windowMinimizedCallback(GLFWwindow* window, int iconified) {
	if (iconified)
		WorldSystem::is_paused = true;
}

void windowFocusCallback(GLFWwindow* window, int focused) {
	if (!focused) {
		WorldSystem::is_paused = true;
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
	if (salmon_dead_sound != nullptr)
		Mix_FreeChunk(salmon_dead_sound);
	if (salmon_eat_sound != nullptr)
		Mix_FreeChunk(salmon_eat_sound);

	Mix_CloseAudio();

	// Destroy all created components
	registry.clear_all_components();

	// Close the window
	glfwDestroyWindow(window);
}

// Debugging
namespace
{
	void glfw_err_cb(int error, const char *desc)
	{
		fprintf(stderr, "%d: %s", error, desc);
	}
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
	window = glfwCreateWindow(window_width_px, window_height_px, "Salmon Game Assignment", nullptr, nullptr);
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
	salmon_dead_sound = Mix_LoadWAV(audio_path("death_sound.wav").c_str());
	salmon_eat_sound = Mix_LoadWAV(audio_path("eat_sound.wav").c_str());

	if (background_music == nullptr || salmon_dead_sound == nullptr || salmon_eat_sound == nullptr)
	{
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
				audio_path("music.wav").c_str(),
				audio_path("death_sound.wav").c_str(),
				audio_path("eat_sound.wav").c_str());
		return nullptr;
	}

	return window;
}

void WorldSystem::init(RenderSystem *renderer_arg)
{
	this->renderer = renderer_arg;
	// Playing background music indefinitely
	Mix_PlayMusic(background_music, -1);
	fprintf(stderr, "Loaded music\n");
	fps_counter_ms = FPS_COUNTER_MS;
	
	current_map = map1;

	// Set all states to default
	restart_game();
}

// Create an HP bar for an enemy
void createHPBar(Entity enemy)
{
	// Check to avoid errors since collisions occur after world step
	
	if (registry.swarms.has(enemy)) {
		return;
	}
	if (registry.healths.has(enemy) && registry.deadlys.get(enemy).enemy_type != ENEMY_TYPES::PROJECTILE)
	{
		float &hp = registry.healths.get(enemy).hit_points;
		float &max_hp = registry.healths.get(enemy).max_hp;
		Motion &enemy_motion = registry.motions.get(enemy);
		// Shift centre of line to left as the hp bar decreases
		Entity hp_bar = createLine({enemy_motion.position.x - ((100.f) * (1 - (hp / max_hp))) / 2,
									enemy_motion.position.y - enemy_motion.scale.y * 0.75f},
								   {(100.f) * hp / max_hp, 15.f});
		vec3 &color = registry.colors.emplace(hp_bar);
		color = vec3(0.f, 5.f, 0.f);
	}
}

void WorldSystem::mapSwitch(int map) {
	switch (map) {
		case 1:
			current_map = map1;
			break;
		case 2:
			current_map = map2;
			break;
		default:
			current_map = map1;
	}
	restart_game();
}


// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update)
{
	// Updating window title with points
	std::stringstream title_ss;
	title_ss << "Points: " << points;
	glfwSetWindowTitle(window, title_ss.str().c_str());



	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
		registry.remove_all_components_of(registry.debugComponents.entities.back());

	// Removing out of screen entities
	auto &motions_registry = registry.motions;

	vec2 player_pos = motions_registry.get(my_player).position;
	vec2 world_origin = vec2(-1860, -3760);

	if (display_fps)
	{
		fps_counter_ms -= elapsed_ms_since_last_update;
		if (fps_counter_ms <= 0.f)
		{
			fps = (int)(1000 / elapsed_ms_since_last_update);
			fps_counter_ms = FPS_COUNTER_MS;
		}
		createText({1000.f, 650.f}, 1.f, "FPS: " + std::to_string(fps), glm::vec3(1.0f, 0.f, 0.f));
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

	/*** UPDATING MAP ***/
	// given player position in world coords, convert to map:
	// origin: [-1860, -3760]
	// current screen pos + origin
	//  minus modulo tilesize and divide by tilesize
	vec2 player_pos_map = vec2((int)((player_pos.x - world_origin.x) - ((int)(player_pos.x - world_origin.x) % TILE_SIZE)) / 100, (int)((player_pos.y - world_origin.y) - ((int)(player_pos.y - world_origin.y) % TILE_SIZE)) / TILE_SIZE);

	for (int i = player_pos_map.x - 8; i <= player_pos_map.x + 8; i++)
	{
		for (int j = player_pos_map.y - 8; j <= player_pos_map.y + 8; j++)
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
					createWalls(renderer, world_pos, false);
					tile_vec.push_back(vec2(i, j));
				}
				continue;
			}
			if (std::find(tile_vec.begin(), tile_vec.end(), vec2(i, j)) != tile_vec.end())
			{
				continue;
			}
			// continue on if tiles/objects have already been processed

			if (current_map[j][i] == 0) {
				createWalls(renderer, world_pos, false);
				tile_vec.push_back(vec2(i, j));
			}

			// furniture spawning
			if (current_map[j][i] == 2 || (current_map[j][i] >= 9 && current_map[j][i] <= 14)) {
				if ((current_map[j][i] == 2 || current_map[j][i] == 9) && current_map[j - 1][i] == 1 && current_map[j][i-1] == 1) {
					// add 2 while loops here to find furniture size
					int horiz_idx = 1;
					int vert_idx = 1;
					while (current_map[j + vert_idx][i] == 2) {
						vert_idx += 1;
					}
					while (current_map[j][i + horiz_idx] == 2) {
						horiz_idx += 1;
					}
					
					// pos {world_pos.x + (TILE_SIZE * ((horiz_idx - 1) / 2)), world_pos.y + (TILE_SIZE * ((vert_idx - 1) / 2))}
					createFurniture(renderer, {world_pos.x, world_pos.y}, current_map[j][i]);
				} else if (current_map[j][i] != 2 && current_map[j][i] != 9) {
					createFurniture(renderer, {world_pos.x, world_pos.y}, current_map[j][i]);
				}

				tile_vec.push_back(vec2(i, j));
				
			}
			

			if (current_map[j][i] == 3) {
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

					createContactSlow(renderer, vec2(world_pos.x, world_pos.y + TILE_SIZE));
					createContactSlow(renderer, vec2(world_pos.x + TILE_SIZE, world_pos.y));
				}
				tile_vec.push_back(vec2(i, j));
			}
			if (current_map[j][i] == 4) {
				int encounter = rand() % 3;
				if (encounter == 0)
				{

					createContactSlow(renderer, vec2(world_pos.x, world_pos.y - TILE_SIZE));
					createContactFast(renderer, vec2(world_pos.x + TILE_SIZE, world_pos.y));
				}
				else if (encounter == 1)
				{

					createContactFast(renderer, vec2(world_pos.x, world_pos.y - TILE_SIZE));
					createContactFast(renderer, vec2(world_pos.x + TILE_SIZE, world_pos.y));
				}
				else
				{
					createRangedEnemy(renderer, vec2(world_pos.x - TILE_SIZE, world_pos.y + TILE_SIZE));
					createContactSlow(renderer, vec2(world_pos.x, world_pos.y + TILE_SIZE));
					createContactSlow(renderer, vec2(world_pos.x - TILE_SIZE, world_pos.y));
				}
				tile_vec.push_back(vec2(i, j));
			}
			if (current_map[j][i] == 5) {
				int encounter = rand() % 3;
				if (encounter == 0)
				{
					createContactFast(renderer, vec2(world_pos.x, world_pos.y));
					createRangedEnemy(renderer, vec2(world_pos.x, world_pos.y + TILE_SIZE));
					createRangedEnemy(renderer, vec2(world_pos.x - TILE_SIZE, world_pos.y));
				} else if (encounter == 1)
				{
					createContactSlow(renderer, vec2(world_pos.x, world_pos.y));
					createContactFast(renderer, vec2(world_pos.x, world_pos.y + TILE_SIZE));
					createContactFast(renderer, vec2(world_pos.x - TILE_SIZE, world_pos.y));
				}
				else
				{
					createRangedEnemy(renderer, vec2(world_pos.x, world_pos.y + TILE_SIZE));
					createContactFast(renderer, vec2(world_pos.x, world_pos.y + TILE_SIZE));
					createContactSlow(renderer, vec2(world_pos.x - TILE_SIZE, world_pos.y));
					createContactSlow(renderer, vec2(world_pos.x + TILE_SIZE, world_pos.y));
				}
				tile_vec.push_back(vec2(i, j));
			}

			
			if (current_map[j][i] == 6) {
				//BUFF_TYPE type = static_cast<BUFF_TYPE>((int) (rand() % 3));
				//createBuff(renderer, world_pos, type);
				tile_vec.push_back(vec2(i, j));
			}
			
			if (current_map[j][i] == 8) {
				Entity swarm_leader = createSwarm(renderer, world_pos, 0.55f, 0.05f, 0.00005f);
				Motion& swarm_motion = motions_registry.get(swarm_leader);
				tile_vec.push_back(vec2(i, j));
			}
			
		}
	}

	// create hp bars for enemies
	for (auto &enemy : registry.deadlys.entities)
	{
		createHPBar(enemy);
	}

	//TODO: spawn frequencies and spawn radius to be adjusted
	// Spawn Level 1 type enemy: slow with contact damage
	next_contact_slow_spawn -= elapsed_ms_since_last_update * current_speed;
	if (next_contact_slow_spawn < 0.f) {
		next_contact_slow_spawn = (CONTACT_SLOW_SPAWN_DELAY_MS / 2) + uniform_dist(rng) * (CONTACT_SLOW_SPAWN_DELAY_MS / 2);
		vec2 contact_slow_pos;
		float distance_to_player;
		float index;
		do {
			index = static_cast<int>(uniform_dist(rng) * spawnable_tiles.size());
			contact_slow_pos = { (640 - (25 * 100)) + (spawnable_tiles[index].y * TILE_SIZE) + (TILE_SIZE / 2), (640 - (44 * 100)) + (spawnable_tiles[index].x * TILE_SIZE) + (TILE_SIZE / 2) };
			distance_to_player = sqrt(pow(contact_slow_pos.x - player_pos.x, 2) + pow(contact_slow_pos.y - player_pos.y, 2));
		} while (distance_to_player < 300.f);

		createContactSlow(renderer, contact_slow_pos);
		//tile_vec.push_back(spawnable_tiles[index]);
	}

	// Spawn Level 2 type enemy: fast with contact damage
	next_contact_fast_spawn -= elapsed_ms_since_last_update * current_speed;
	if (next_contact_fast_spawn < 0.f) {
		next_contact_fast_spawn = (CONTACT_FAST_SPAWN_DELAY_MS / 2) + uniform_dist(rng) * (CONTACT_FAST_SPAWN_DELAY_MS / 2);
		vec2 contact_fast_pos;
		float distance_to_player;
		float index;
		do {
			index = static_cast<int>(uniform_dist(rng) * spawnable_tiles.size());
			contact_fast_pos = { (640 - (25 * 100)) + (spawnable_tiles[index].y * TILE_SIZE) + (TILE_SIZE / 2), (640 - (44 * 100)) + (spawnable_tiles[index].x * TILE_SIZE) + (TILE_SIZE / 2) };
			distance_to_player = sqrt(pow(contact_fast_pos.x - player_pos.x, 2) + pow(contact_fast_pos.y - player_pos.y, 2));
		} while (distance_to_player < 300.f);

		createContactFast(renderer, contact_fast_pos);
		//tile_vec.push_back(spawnable_tiles[index]);
	}

	// Spawn Level 3 type enemy: slow ranged enemy
	next_ranged_spawn -= elapsed_ms_since_last_update * current_speed;
	if (next_ranged_spawn < 0.f) {
		next_ranged_spawn = (RANGED_ENEMY_SPAWN_DELAY_MS / 2) + uniform_dist(rng) * (RANGED_ENEMY_SPAWN_DELAY_MS / 2);
		vec2 ranged_pos;
		float distance_to_player;
		float index;
		do {
			index = static_cast<int>(uniform_dist(rng) * spawnable_tiles.size());
			ranged_pos = { (640 - (25 * 100)) + (spawnable_tiles[index].y * TILE_SIZE) + (TILE_SIZE / 2), (640 - (44 * 100)) + (spawnable_tiles[index].x * TILE_SIZE) + (TILE_SIZE / 2) };
			distance_to_player = sqrt(pow(ranged_pos.x - player_pos.x, 2) + pow(ranged_pos.y - player_pos.y, 2));
		} while (distance_to_player < 300.f);

		createRangedEnemy(renderer, ranged_pos);
		//tile_vec.push_back(spawnable_tiles[index]);
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
				Entity projectile = createRangedProjectile(renderer, registry.motions.get(ranged).position);
				Motion &projectile_motion = registry.motions.get(projectile);
				Motion &player_motion = registry.motions.get(my_player);
				projectile_motion.angle = atan2(projectile_motion.position.y - player_motion.position.y, projectile_motion.position.x - player_motion.position.x);
				projectile_motion.velocity = {200.f, 200.f};
			}
		}
	}

	// Processing the salmon state
	assert(registry.screenStates.components.size() <= 1);
	ScreenState &screen = registry.screenStates.components[0];


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
			if (registry.players.has(entity)) {
				screen.darken_screen_factor = 0;
				restart_game();
				return true;
			}
			// Enemy death - animation finished, remove entity
			if (registry.deadlys.has(entity)) {
				if (registry.animationSets.has(entity)) {
					AnimationSet animSet_enemy = registry.animationSets.get(entity);
					Animation anim = animSet_enemy.animations[animSet_enemy.current_animation];
				}

				float roll = uniform_dist(rng);
				Deadly& deadly = registry.deadlys.get(entity);
				Motion& enemy_motion = registry.motions.get(entity);

				if (roll <= deadly.drop_chance && deadly.enemy_type != ENEMY_TYPES::PROJECTILE)
				{
					createExperience(renderer, enemy_motion.position, deadly.experience);
				}

				// replace dead leader with new swarm member
				if (registry.swarms.has(entity)) {
					if (registry.swarms.get(entity).leader_id == entity) {
						for (int i = 0; i < registry.swarms.entities.size(); i++) {
							Entity e = registry.swarms.entities[i];
							SwarmMember& s = registry.swarms.get(e);
							if (s.leader_id == entity && e != entity) {
								s.leader_id = e;
								for (int j = 0; j < registry.swarms.components.size(); j++) {
									SwarmMember& swarm = registry.swarms.components[j];
									if (swarm.leader_id == entity) {
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

				registry.remove_all_components_of(entity);
				enemies_killed++;
				if (enemies_killed >= enemy_kill_goal) {
					mapSwitch(2);
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
		lightflicker_counter_ms += elapsed_ms_since_last_update;
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

	Player& player = registry.players.get(my_player);

	
	// handle dash cooldown
    if (!player.is_dash_up) {
		player.curr_dash_cooldown_ms -= elapsed_ms_since_last_update;
		AnimationSet& stamina_anim = registry.animationSets.get(stamina_bar);

		// Player able to dash again
		if (player.curr_dash_cooldown_ms < 0.f) {
			player.curr_dash_cooldown_ms = player.dash_cooldown_ms;
			registry.motions.get(my_player).speed = 300.f;
			player.is_dash_up = true;
			stamina_anim.current_animation = "staminabar_full";

		// Player done dashing
		} else if (player.curr_dash_cooldown_ms < 2900.f) {
			registry.motions.get(my_player).speed = 300.f;
			registry.lightUps.remove(my_player);
			
			stamina_anim.current_animation = "staminabar_regen";
		
		// Player dashing
		} else {
			registry.motions.get(my_player).speed = 2000.f;
			if (!registry.lightUps.has(my_player)) {
				registry.lightUps.emplace(my_player);
			}
			for (int i = 0; i < 5; i++) {
				int bound = 2900 + (i * 20);
				if (player.curr_dash_cooldown_ms < bound && player.curr_dash_cooldown_ms + elapsed_ms_since_last_update > bound) {
					// TODO: modify this a little bit to make dash shadows fully even
					// TODO: make it so player is invulnerable during dash ?
					vec2 effectPos = registry.motions.get(my_player).position;
					createEffect(renderer, effectPos, 500, EFFECT_TYPE::DASH);
				}
			}
		}
	}

	player_controller.step(elapsed_ms_since_last_update);


	for (Entity& e : registry.deadlys.entities) {
		Deadly& enemy = registry.deadlys.get(e);
		if (enemy.enemy_type == ENEMY_TYPES::PROJECTILE) {
			continue;
		}

		AnimationSet& animSet_enemy = registry.animationSets.get(e);
		std::string enemy_name = "";
		switch (enemy.enemy_type) {
			case ENEMY_TYPES::PROJECTILE:
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
			default:
				break;
		}
		switch (enemy.state) {
			case ENEMY_STATE::IDLE:
				animSet_enemy.current_animation = enemy_name + "enemy_idle_f";
				break;
			case ENEMY_STATE::RUN:
				animSet_enemy.current_animation = enemy_name + "enemy_run_f";
				break;
			case ENEMY_STATE::DEAD:
			// seems fine
				if (animSet_enemy.current_animation != enemy_name + "enemy_die") {
					animSet_enemy.current_animation = enemy_name + "enemy_die";
					animSet_enemy.current_frame = 0;
				}
				break;
			default:
				animSet_enemy.current_animation = enemy_name + "enemy_idle_f";
		}
	}

	

	for (Entity entity : registry.lightUps.entities) {
		LightUp& counter = registry.lightUps.get(entity);
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
	// Debugging for memory/component leaks
	registry.list_all_components();
	printf("Restarting\n");

	// Reset the game speed
	current_speed = 1.f;
  
	WorldSystem::is_paused = false;
	
	enemies_killed = 0;

	// Remove all entities that we created
	// All that have a motion, we could also iterate over all fish, eels, ... but that would be more cumbersome
	while (registry.motions.entities.size() > 0)
		registry.remove_all_components_of(registry.motions.entities.back());

	// Debugging for memory/component leaks
	registry.list_all_components();

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
			else if (current_map[i][j] != 0) {
				spawnable_tiles.push_back(vec2(i, j));
			}
		}
	}

	// create a new Player
	my_player = createPlayer(renderer, {window_width_px / 2, window_height_px - 200});
	registry.colors.insert(my_player, {1, 0.8f, 0.8f});
	player_controller.set_player_reference(&my_player);
	player_controller.set_renderer(renderer);

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
		for (int j = playerPos_init.y - 8; j <= playerPos_init.y + 8; j++)
		{
			if ((i < 0) || (j < 0) || (i >= current_map[0].size()) || j >= current_map.size())
			{
				createWalls(renderer, {(640 - (25 * 100)) + (i * TILE_SIZE) + (sign(i) * TILE_SIZE / 2), (640 - (44 * 100)) + (j * TILE_SIZE) + (sign(j) * TILE_SIZE / 2)}, false);
				tile_vec.push_back(vec2(i, j));
			}
			else if (current_map[j][i] == 0)
			{
				createWalls(renderer, {(640 - (25 * 100)) + (i * TILE_SIZE) + (TILE_SIZE / 2), (640 - (44 * 100)) + (j * TILE_SIZE) + (TILE_SIZE / 2)}, false);
				tile_vec.push_back(vec2(i, j));
			}
		}
	}

	// create health bar
	vec2 hp_bar_pos = {-0.75, 0.85f};
	hp_bar = createHPBar(renderer, hp_bar_pos);
	
	vec2 stamina_bar_pos = { -0.74f, 0.7f };
	stamina_bar = createStaminaBar(renderer, stamina_bar_pos);

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
				if (!player.invulnerable && !registry.deathTimers.has(entity_other))
				{
					// player takes damage
					player_hp -= registry.damages.get(entity_other).damage;
					// avoid negative hp values for hp bar
					player_hp = max(0.f, player_hp);
					// modify hp bar
					// std::cout << "Player hp: " << player_hp << "\n";
					if (player_hp <= 200 && player_hp >= 0)
					{
						// Motion& motion = registry.motions.get(hp_bar);
						// motion.scale.x = HPBAR_BB_WIDTH * (player_hp / 100);
						RenderRequest &hp_bar_render = registry.renderRequests.get(hp_bar);
						if (hp_bar_render.used_texture != TEXTURE_ASSET_ID::HP_BAR_0)
						{
							hp_bar_render.used_texture = static_cast<TEXTURE_ASSET_ID>(static_cast<int>(hp_bar_render.used_texture) - 1);
						}

						// motion.position.x += HPBAR_BB_WIDTH * (player_hp / 400);
					}
					player.invulnerable = true;
					player.invulnerable_duration_ms = 1000.f;
				}
				if (registry.deadlys.get(entity_other).enemy_type == ENEMY_TYPES::PROJECTILE)
				{
					registry.remove_all_components_of(entity_other);
				}
				if (!registry.deathTimers.has(entity) && player_hp <= 0.f) {
					registry.deathTimers.emplace(entity);
					//Mix_PlayChannel(-1, salmon_dead_sound, 0);

					// Control what happens when the player dies here
					Motion &motion = registry.motions.get(my_player);
					motion.velocity[0] = 0.0f;
					motion.velocity[1] = 0.0f;
				}
			}
			// Checking Player - Eatable collisions
			else if (registry.eatables.has(entity_other))
			{
				if (!registry.deathTimers.has(entity))
				{
					// chew, count points, and set the LightUp timer
					registry.remove_all_components_of(entity_other);
					Mix_PlayChannel(-1, salmon_eat_sound, 0);
					++points;
				}
			}
			// Checking player collision with solid object
			if (registry.solidObjs.has(entity_other) || registry.walls.has(entity_other))
			{
				Motion &motion_moving = registry.motions.get(entity);
				Motion &motion_solid = registry.motions.get(entity_other);

				// Temp solution to prevent player from sticking to solid objects - may not work if solid object is really long or tall

				float x_diff = motion_moving.position.x - motion_solid.position.x;
				float y_diff = motion_moving.position.y - motion_solid.position.y;

				if (x_diff < 0 && abs(x_diff) > abs(y_diff) && motion_moving.velocity.x > 0)
				{
					motion_moving.velocity.x = 0.f;
					motion_moving.position.x = registry.players.get(entity).last_pos.x;
				}
				if (x_diff > 0 && abs(x_diff) > abs(y_diff) && motion_moving.velocity.x < 0)
				{
					motion_moving.velocity.x = 0.f;
					motion_moving.position.x = registry.players.get(entity).last_pos.x;
				}
				if (y_diff < 0 && abs(y_diff) > abs(x_diff) && motion_moving.velocity.y > 0)
				{
					motion_moving.velocity.y = 0.f;
					motion_moving.position.y = registry.players.get(entity).last_pos.y;
				}
				if (y_diff > 0 && abs(y_diff) > abs(x_diff) && motion_moving.velocity.y < 0)
				{
					motion_moving.velocity.y = 0.f;
					motion_moving.position.y = registry.players.get(entity).last_pos.y;
				}

				/*
				float left_1 = motion_moving.position.x - (abs(motion_moving.scale.x) / 2);
				float right_1 = motion_moving.position.x + (abs(motion_moving.scale.x) / 2);

				float left_2 = motion_solid.position.x - (abs(motion_solid.scale.x) / 2);
				float right_2 = motion_solid.position.x + (abs(motion_solid.scale.x) / 2);

				float bottom_1 = motion_moving.position.y - (abs(motion_moving.scale.y) / 2);
				float top_1 = motion_moving.position.y + (abs(motion_moving.scale.y) / 2);

				float bottom_2 = motion_solid.position.y - (abs(motion_solid.scale.y) / 2);
				float top_2 = motion_solid.position.y + (abs(motion_solid.scale.y) / 2);

				// float x_diff = max(abs(right_2 - left_1), abs(right_1 - left_2));
				// float y_diff = max(abs(bottom_2 - top_1), abs(bottom_1 - top_2));

				float x_diff = 2 * (motion_moving.position.x - motion_solid.position.x) / (motion_moving.scale.x + motion_solid.scale.x);
				float y_diff = 2 * (motion_moving.position.y - motion_solid.position.y) / (motion_moving.scale.y + motion_solid.scale.y);



				if (abs(x_diff) > abs(y_diff)) {

					if (((left_1 <= right_2) && (left_1 >= left_2))) {
						// player bounding box left bound overlaps with object box
						//motion_moving.velocity.x = 0.f;
						motion_moving.position.x += (right_2 - left_1) + 1;

					}
					if ((left_2 <= right_1) && (left_2 >= left_1)) {
						// player bounding box right bound overlaps with object box
						//motion_moving.velocity.x = 0.f;
						motion_moving.position.x -= (right_1 - left_2) + 1;
					}
				} else {


					if ((top_1 >= bottom_2) && (top_1 <= top_2)) {
						// player bounding box top bound overlaps with object box
						motion_moving.position.y += (bottom_2 - top_1) + 1;
						//motion_moving.velocity.y = 0.f;
					}
					if ((top_2 >= bottom_1) && (top_2 <= top_1)) {
						// player bounding box bottom bound overlaps with object box
						motion_moving.position.y -= (bottom_1 - top_2) + 1;
						//motion_moving.velocity.y = 0.f;

					}

				}
				*/
			}
			if (registry.stickies.has(entity_other))
			{
				Motion &player_motion = registry.motions.get(my_player);
				player_motion.speed = 120.f;
				unstick_player = false;
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
			if (registry.deadlys.has(entity_other) && registry.healths.has(entity_other) && registry.motions.has(entity_other))
			{
				Health &deadly_health = registry.healths.get(entity_other);
				Damage &damage = registry.damages.get(entity);
				Motion &enemy_motion = registry.motions.get(entity_other);
				Motion &pmotion = registry.motions.get(my_player);
				Player &player = registry.players.get(my_player);
				Deadly &deadly = registry.deadlys.get(entity_other);


				deadly_health.hit_points = std::max(0.0f, deadly_health.hit_points - damage.damage);

				vec2 diff = enemy_motion.position - pmotion.position;

				if (deadly.enemy_type != ENEMY_TYPES::PROJECTILE && deadly_health.hit_points > 0)
				{
					vec2 kockback_pos = enemy_motion.position + (diff * player.knockback_strength);
					int grid_x = static_cast<int>((kockback_pos.x - (640 - (25 * TILE_SIZE))) / TILE_SIZE);
					int grid_y = static_cast<int>((kockback_pos.y - (640 - (44 * TILE_SIZE))) / TILE_SIZE);
					int adjust_x = 0;
					int adjust_y = 0;
					if (diff.x < 0 && diff.y < 0) {
						if (abs(diff.x) < 50 && abs(diff.y) < 50) {
							adjust_x -= 1;
							adjust_y -= 1;
						}
						else {
							if (abs(diff.x) > 50) {
								adjust_x -= 1;
							}
							if (abs(diff.y) > 50) {
								adjust_y -= 1;
							}
						}
					} else if (diff.x > 0 && diff.y < 0) {
						if (abs(diff.x) < 50 && abs(diff.y) < 50) {
							adjust_x += 1;
							adjust_y -= 1;
						}
						else {
							if (abs(diff.x) > 50) {
								adjust_x += 1;
							}
							if (abs(diff.y) > 50) {
								adjust_y -= 1;
							}
						}
					}
					else if (diff.x > 0 && diff.y > 0) {
						if (abs(diff.x) < 50 && abs(diff.y) < 50) {
							adjust_x += 1;
							adjust_y += 1;
						}
						else {
							if (abs(diff.x) > 50) {
								adjust_x += 1;
							}
							if (abs(diff.y) > 50) {
								adjust_y += 1;
							}
						}
					}
					else if (diff.x < 0 && diff.y > 0) {
						if (abs(diff.x) < 50 && abs(diff.y) < 50) {
							adjust_x -= 1;
							adjust_y += 1;
						}
						else {
							if (abs(diff.x) > 50) {
								adjust_x -= 1;
							}
							if (abs(diff.y) > 50) {
								adjust_y += 1;
							}
						}
					}

					vec2 adjust = adjust_knockback_coordinates(grid_x, grid_y, adjust_x, adjust_y);

					if (!(adjust_x == 0 && adjust_y == 0) && registry.motions.has(entity_other)) {
						vec2 grid_kockback_pos = { (640 - (25 * 100)) + ((grid_x + adjust.x) * TILE_SIZE), (640 - (44 * 100)) + ((grid_y + adjust.y) * TILE_SIZE) };
						enemy_motion.position = grid_kockback_pos;
						physics.update_enemy_movement(entity_other, step_seconds);
						if (registry.pathTimers.has(entity_other)) {
							registry.pathTimers.get(entity_other).timer = -1.f;
						}
						
					}
				}

				if (registry.lightUps.has(entity_other))
				{
					registry.lightUps.remove(entity_other);
				}
				registry.lightUps.emplace(entity_other);

				
				if (deadly_health.hit_points <= 0.0f && (!registry.deathTimers.has(entity_other))) {
					Deadly& d = registry.deadlys.get(entity_other);
					d.state = ENEMY_STATE::DEAD;
					DeathTimer& death = registry.deathTimers.emplace(entity_other);
					death.counter_ms = 550.4f;
				}

				registry.playerAttacks.get(entity).has_hit = true;
			}
		}
	}

	if (unstick_player)
	{
		Motion &player_motion = registry.motions.get(my_player);
		player_motion.speed = 300.f;
	}
	
	
	// Remove all collisions from this simulation step
	registry.collisions.clear();
}

vec2 WorldSystem::adjust_knockback_coordinates(int grid_x, int grid_y, int adjust_x, int adjust_y) {
	if (current_map[grid_y + adjust_y][grid_x + adjust_x] != 0) {
		return vec2(adjust_x, adjust_y);
	}
	if (current_map[grid_y + adjust_y][grid_x] != 0) {
		return vec2(0, adjust_y);
	}
	if (current_map[grid_y][grid_x + adjust_x] != 0) {
		return vec2(adjust_x, 0);
	}
	return vec2(0, 0);
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
	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R)
	{
		int w, h;
		glfwGetWindowSize(window, &w, &h);

		restart_game();
	}

	if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE)
	{
		WorldSystem::is_paused = !WorldSystem::is_paused;
		ScreenState &screen = registry.screenStates.components[0];
	}

	if (action == GLFW_RELEASE && key == GLFW_KEY_F)
	{
		display_fps = !display_fps;
	}

	// Tutorial
	if (action == GLFW_RELEASE && key == GLFW_KEY_T)
	{
		ScreenState& screen = registry.screenStates.components[0];
		// if the game is already paused then this shouldn't work
		if (!WorldSystem::is_paused)
		{
			WorldSystem::is_paused = true;
			is_tutorial_on = true;
			screen.darken_screen_factor = 0.9;
			float tutorial_header_x = window_width_px / 2 - 120;
			float tutorial_header_y = 620;
			glm::vec3 white = glm::vec3(1.f, 1.f, 1.f);
			createText({ tutorial_header_x, tutorial_header_y }, 1.0, "TUTORIAL", white);
			std::vector<std::string> lines = read_file(PROJECT_SOURCE_DIR + std::string("data/tutorial/tutorial.txt"));
			float y_spacer = 80.f;
			for (std::string line : lines)
			{
				// Render each line
				createText({ tutorial_header_x - 400.f, tutorial_header_y - y_spacer }, 0.6f, line, white);
				y_spacer += 70.f;
			}
		}
		else if (is_tutorial_on)
		{
			WorldSystem::is_paused = false;
			is_tutorial_on = false;
			screen.darken_screen_factor = 0.0;
		}
	}

	// Debugging
	if (key == GLFW_KEY_P)
	{
		if (action == GLFW_RELEASE)
			debugging.in_debug_mode = !debugging.in_debug_mode;
	}

	// player key stuff starts here


	// Player controller
	Motion& pmotion = registry.motions.get(my_player);
	Player& player = registry.players.get(my_player);

	if (key == GLFW_KEY_SPACE) {
		if (action == GLFW_PRESS && !registry.deathTimers.has(my_player) && (!is_paused) && player.is_dash_up) {
			pmotion.speed = 5500.f;
			player.is_dash_up = false;
			registry.lightUps.emplace(my_player);
			AnimationSet& stamina_anim = registry.animationSets.get(stamina_bar);
			stamina_anim.current_animation = "staminabar_depleting";
		}
	}
  
  if (!WorldSystem::is_paused)
	  player_controller.on_key(key, action, mod);


	// if (key == GLFW_KEY_SPACE) {
	// 	if (action == GLFW_PRESS && !registry.deathTimers.has(my_player)) {
	// 		if (!registry.dashing.has(my_player)) {

	// 			vec2 dashtarget = registry.motions.get(my_player).position + (player.last_direction * vec2(100.f,100.f));
	// 			if (dashtarget.x > window_width_px - 100.f) {
	// 				dashtarget.x = window_width_px - 100.f;
	// 			}

	// 			Dash new_dash = {dashtarget, registry.motions.get(my_player).position - (registry.motions.get(my_player).position + vec2(0, 5)), 1000};

	// 			registry.dashing.insert(my_player, new_dash);
	// 		}
	// 	}
	// }

	// // Dash movement
	// if (registry.dashing.has(my_player)) {
	// 	if (registry.dashing.get(my_player).diff != vec2(0.f, 0.f)) {
	// 		motion.velocity = lerp(vec2(0, 0), registry.dashing.get(my_player).target - motion.position, 0.1);
	// 		motion.velocity = motion.speed * motion.velocity;
	// 	}
	// }

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
	player_controller.on_mouse_move(mouse_position);
}

void WorldSystem::on_mouse_button(int button, int action, int mods)
{
	player_controller.on_mouse_button(button, action, mods);
}

std::vector<std::vector<int>> WorldSystem::get_current_map() {
	return current_map;
}
