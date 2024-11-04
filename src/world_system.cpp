// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <iostream>
#include <fstream> 

#include "physics_system.hpp"
#include "animation_system.hpp"

// Game configuration
const size_t MAX_NUM_EELS = 5;
const size_t MAX_NUM_FISH = 2;
const size_t EEL_SPAWN_DELAY_MS = 2000 * 3;
const size_t FISH_SPAWN_DELAY_MS = 5000 * 3;
const size_t MAX_NUM_RANGED_ENEMY = 1;
const size_t RANGED_ENEMY_SPAWN_DELAY_MS = 5000 * 3;
const size_t RANGED_ENEMY_PROJECTILE_DELAY_MS = 2000 * 3;

const int TILE_SIZE = 100;
std::vector<vec2> tile_vec;
const int LIGHT_FLICKER_RATE = 2000 * 10;
const int FPS_COUNTER_MS = 1000;

int lightflicker_counter_ms;
int fps_counter_ms;
int fps = 0;
bool display_fps = false;

bool is_tutorial_on = false;

// create the underwater world
WorldSystem::WorldSystem()
	: points(0)
	, next_eel_spawn(0.f)
	, next_fish_spawn(0.f)
	, next_ranged_spawn(0.f) {
	// Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
}

WorldSystem::~WorldSystem() {

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
namespace {
	void glfw_err_cb(int error, const char *desc) {
		fprintf(stderr, "%d: %s", error, desc);
	}
}

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the renderer
GLFWwindow* WorldSystem::create_window() {
	///////////////////////////////////////
	// Initialize GLFW
	glfwSetErrorCallback(glfw_err_cb);
	if (!glfwInit()) {
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
	if (window == nullptr) {
		fprintf(stderr, "Failed to glfwCreateWindow");
		return nullptr;
	}

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this);
	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 }); };
	auto mouse_button_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_button( _0, _1, _2); };
	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);
	glfwSetMouseButtonCallback(window, mouse_button_redirect);

	//////////////////////////////////////
	// Loading music and sounds with SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "Failed to initialize SDL Audio");
		return nullptr;
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
		fprintf(stderr, "Failed to open audio device");
		return nullptr;
	}

	background_music = Mix_LoadMUS(audio_path("music.wav").c_str());
	salmon_dead_sound = Mix_LoadWAV(audio_path("death_sound.wav").c_str());
	salmon_eat_sound = Mix_LoadWAV(audio_path("eat_sound.wav").c_str());

	if (background_music == nullptr || salmon_dead_sound == nullptr || salmon_eat_sound == nullptr) {
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
			audio_path("music.wav").c_str(),
			audio_path("death_sound.wav").c_str(),
			audio_path("eat_sound.wav").c_str());
		return nullptr;
	}

	return window;
}

void WorldSystem::init(RenderSystem* renderer_arg) {
	this->renderer = renderer_arg;
	// Playing background music indefinitely
	Mix_PlayMusic(background_music, -1);
	fprintf(stderr, "Loaded music\n");
	fps_counter_ms = FPS_COUNTER_MS;

	// Set all states to default
	restart_game();
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {
	// Updating window title with points
	std::stringstream title_ss;
	title_ss << "Points: " << points;
	glfwSetWindowTitle(window, title_ss.str().c_str());

	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
		registry.remove_all_components_of(registry.debugComponents.entities.back());

	// Removing out of screen entities
	auto& motions_registry = registry.motions;

	vec2 player_pos = motions_registry.get(my_player).position;
	vec2 world_origin = vec2(-1860, -3760);

	if (display_fps) {
		fps_counter_ms -= elapsed_ms_since_last_update;
		if (fps_counter_ms <= 0.f) {
			fps = (int)(1000 / elapsed_ms_since_last_update);
			fps_counter_ms = FPS_COUNTER_MS;
		}
		createText({ 1000.f, 650.f }, 1.f, "FPS: " + std::to_string(fps), glm::vec3(1.0f, 0.f, 0.f));
	}
	
	for (int i = (int) motions_registry.components.size()-1; i>=0; --i) {
		Motion& motion = motions_registry.components[i];
		Entity entity = motions_registry.entities[i];

		if (registry.solidObjs.has(entity)) {
			if ((abs(player_pos.x - motion.position.x) > 800) || (abs(player_pos.y - motion.position.y) > 800)) {
				vec2 obj_pos = motion.position;
				vec2 obj_pos_map = vec2((int) ((obj_pos.x - world_origin.x) - ((int) abs(obj_pos.x - world_origin.x) % TILE_SIZE)) / TILE_SIZE, (int) ((obj_pos.y - world_origin.y) - ((int) abs(obj_pos.y - world_origin.y) % TILE_SIZE)) / TILE_SIZE);
				
				if (std::find(tile_vec.begin(), tile_vec.end(), vec2(obj_pos_map.x, obj_pos_map.y)) != tile_vec.end()) {
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
	vec2 player_pos_map = vec2((int) ((player_pos.x - world_origin.x) - ((int) (player_pos.x - world_origin.x) % TILE_SIZE)) / 100, (int) ((player_pos.y - world_origin.y) - ((int) (player_pos.y - world_origin.y) % TILE_SIZE)) / TILE_SIZE);
	
	for (int i = player_pos_map.x - 8; i <= player_pos_map.x + 8; i++) {
		for (int j = player_pos_map.y - 8; j <= player_pos_map.y + 8; j++) {
			vec2 world_pos = {(640 - (25*100)) + (i * TILE_SIZE) + (TILE_SIZE/2), (640 - (44*100)) + (j * TILE_SIZE) + (TILE_SIZE/2)};
			
			if ((i < 0) || (j < 0) || (i >= map1[0].size()) || j >= map1.size()) {
				if ((std::find(tile_vec.begin(), tile_vec.end(), vec2(i, j)) == tile_vec.end())) {
					createWalls(renderer, world_pos, false);
					tile_vec.push_back(vec2(i, j));
				}
				continue;
			} 
			if (std::find(tile_vec.begin(), tile_vec.end(), vec2(i, j)) != tile_vec.end()) {
				continue;
			}
			// continue on if tiles/objects have already been processed

			if (map1[j][i] == 0) {
				createWalls(renderer, world_pos, false);
				tile_vec.push_back(vec2(i, j));
			}
			

			// spawn enemies in any new tiles
			/*
			ENCOUNTER LEVELS OUTLINE - level hardcoded into map, encounter details randomly picked
			level 1:
			- 1 contact based slow
			- 1 contact based fast
			- 2 contact based slow

			level 2: 
			- 1 contact based slow 1 contact based fast
			- 2 contact based fast
			- 2 contact based slow 1 ranged

			level 3:
			- 2 ranged 1 contact based fast
			- 2 contact based fast 1 contact based slow
			- 1 ranged 1 contact based fast 2 contact based slow

			*/
			if (map1[j][i] == 3) {
				int encounter = rand() % 3;
				if (encounter == 0) {
					std::cout << "encounter 1 generated, level 1" << std::endl;
					createFish(renderer, world_pos);
				} else if (encounter == 1) {
					std::cout << "encounter 2 generated, level 1" << std::endl;

					createEel(renderer, world_pos);
				} else {
					std::cout << "encounter 3 generated, level 1" << std::endl;

					createFish(renderer, vec2(world_pos.x, world_pos.y + TILE_SIZE));
					createFish(renderer, vec2(world_pos.x + TILE_SIZE, world_pos.y));
				}
				tile_vec.push_back(vec2(i, j));
			}
			if (map1[j][i] == 4) {
				int encounter = rand() % 3;
				if (encounter == 0) {
					std::cout << "encounter 1 generated, level 2" << std::endl;

					createFish(renderer, vec2(world_pos.x, world_pos.y - TILE_SIZE));
					createEel(renderer, vec2(world_pos.x + TILE_SIZE, world_pos.y));
				} else if (encounter == 1) {
					std::cout << "encounter 2 generated, level 2" << std::endl;

					createEel(renderer, vec2(world_pos.x, world_pos.y - TILE_SIZE));
					createEel(renderer, vec2(world_pos.x + TILE_SIZE, world_pos.y));
				} else {
					std::cout << "encounter 3 generated, level 2" << std::endl;
					createRangedEnemy(renderer, vec2(world_pos.x - TILE_SIZE, world_pos.y + TILE_SIZE));
					createFish(renderer, vec2(world_pos.x, world_pos.y + TILE_SIZE));
					createFish(renderer, vec2(world_pos.x - TILE_SIZE, world_pos.y));
				}
				tile_vec.push_back(vec2(i, j));
			}
			if (map1[j][i] == 5) {
				int encounter = rand() % 3;
				if (encounter == 0) {
					createEel(renderer, vec2(world_pos.x, world_pos.y));
					createRangedEnemy(renderer, vec2(world_pos.x, world_pos.y + TILE_SIZE));
					createRangedEnemy(renderer, vec2(world_pos.x - TILE_SIZE, world_pos.y));
				} else if (encounter == 1) {
					createFish(renderer, vec2(world_pos.x, world_pos.y));
					createEel(renderer, vec2(world_pos.x, world_pos.y + TILE_SIZE));
					createEel(renderer, vec2(world_pos.x - TILE_SIZE, world_pos.y));
				} else {
					createRangedEnemy(renderer, vec2(world_pos.x, world_pos.y + TILE_SIZE));
					createEel(renderer, vec2(world_pos.x, world_pos.y + TILE_SIZE));
					createFish(renderer, vec2(world_pos.x - TILE_SIZE, world_pos.y));
					createFish(renderer, vec2(world_pos.x + TILE_SIZE, world_pos.y));
				}
				tile_vec.push_back(vec2(i, j));
			}


		}
	}



	/*
	//TODO: spawn frequencies and spawn radius to be adjusted
	// Spawn Level 1 type enemy: slow with contact damage
	next_fish_spawn -= elapsed_ms_since_last_update * current_speed;
	if (registry.eatables.components.size() <= MAX_NUM_FISH && next_fish_spawn < 0.f) {
		next_fish_spawn = (FISH_SPAWN_DELAY_MS / 2) + uniform_dist(rng) * (FISH_SPAWN_DELAY_MS / 2);
		vec2 fish_pos;
		float distance_to_player;
		do {
			//fish_pos = { 50.0f + uniform_dist(rng) * (window_width_px - 100.f), 50.f + uniform_dist(rng) * (window_height_px - 100.f) };
			int i;
			int j;
			do {
				i = static_cast<int>(uniform_dist(rng) * map1[0].size());
				j = static_cast<int>(uniform_dist(rng) * map1.size());
			} while (map1[j][i] != 1);
			int tile_size = 100;
			fish_pos = { (640 - (25 * 100)) + (i * tile_size) + (tile_size / 2), (640 - (44 * 100)) + (j * tile_size) + (tile_size / 2) };
			distance_to_player = sqrt(pow(fish_pos.x - player_pos.x, 2) + pow(fish_pos.y - player_pos.y, 2));
		} while (distance_to_player < 500.f);
		Entity fish = createFish(renderer, fish_pos);
		registry.motions.get(fish).velocity = { 2.5f, 2.5f };
	}

	// Spawn Level 2 type enemy: fast with contact damage
	next_eel_spawn -= elapsed_ms_since_last_update * current_speed;
	if (registry.deadlys.components.size() <= MAX_NUM_EELS && next_eel_spawn < 0.f) {
		// reset timer
		next_eel_spawn = (EEL_SPAWN_DELAY_MS / 2) + uniform_dist(rng) * (EEL_SPAWN_DELAY_MS / 2);
		vec2 eel_pos;
		float distance_to_player;
		do {
			int i;
			int j;
			do {
				i = static_cast<int>(uniform_dist(rng) * map1[0].size());
				j = static_cast<int>(uniform_dist(rng) * map1.size());
			} while (map1[j][i] != 1);
			int tile_size = 100;
			eel_pos = { (640 - (25 * 100)) + (i * tile_size) + (tile_size / 2), (640 - (44 * 100)) + (j * tile_size) + (tile_size / 2) };
			distance_to_player = sqrt(pow(eel_pos.x - player_pos.x, 2) + pow(eel_pos.y - player_pos.y, 2));
		} while (distance_to_player < 500.f);
		Entity eel = createEel(renderer, eel_pos);
		registry.motions.get(eel).velocity = { 5.f, 5.f };
	}

	// Spawn Level 3 type enemy: slow ranged enemy
	next_ranged_spawn -= elapsed_ms_since_last_update * current_speed;
	if (registry.ranged.components.size() <= MAX_NUM_RANGED_ENEMY && next_ranged_spawn < 0.f) {
		// reset timer
		next_ranged_spawn = (RANGED_ENEMY_SPAWN_DELAY_MS / 2) + uniform_dist(rng) * (RANGED_ENEMY_SPAWN_DELAY_MS / 2);
		vec2 ranged_enemy_pos;
		float distance_to_player;
		do {
			int i;
			int j;
			do {
				i = static_cast<int>(uniform_dist(rng) * map1[0].size());
				j = static_cast<int>(uniform_dist(rng) * map1.size());
			} while (map1[j][i] != 1);
			int tile_size = 100;
			ranged_enemy_pos = { (640 - (25 * 100)) + (i * tile_size) + (tile_size / 2), (640 - (44 * 100)) + (j * tile_size) + (tile_size / 2) };
			distance_to_player = sqrt(pow(ranged_enemy_pos.x - player_pos.x, 2) + pow(ranged_enemy_pos.y - player_pos.y, 2));
		} while (distance_to_player < 500.f);
		Entity ranged_enemy = createRangedEnemy(renderer, ranged_enemy_pos);
		registry.motions.get(ranged_enemy).velocity = { 1.f, 1.f };
	}
	*/

	// Spawn projectiles for ranged enemies
	for (auto& ranged : registry.ranged.entities) {
		float& projectile_delay = registry.ranged.get(ranged).projectile_delay;
		projectile_delay -= elapsed_ms_since_last_update * current_speed;
		if (projectile_delay < 0.f) {
			// reset timer
			projectile_delay = (RANGED_ENEMY_PROJECTILE_DELAY_MS / 2) + uniform_dist(rng) * (RANGED_ENEMY_PROJECTILE_DELAY_MS / 2);
			Entity projectile = createRangedProjectile(renderer, registry.motions.get(ranged).position);
			Motion& projectile_motion = registry.motions.get(projectile);
			Motion& player_motion = registry.motions.get(my_player);
			projectile_motion.angle = atan2(projectile_motion.position.y - player_motion.position.y, projectile_motion.position.x - player_motion.position.x);
			projectile_motion.velocity = { 200.f, 200.f };
		}
	}
	

	// Check if player is invulnerable
	Player& player = registry.players.get(my_player);
	if (player.invulnerable) {
		player.invulnerable_duration_ms -= elapsed_ms_since_last_update;
		if (player.invulnerable_duration_ms < 0.f) {
			// std::cout << "Invuln ended" << std::endl;
			player.invulnerable = false;
			player.invulnerable_duration_ms = 1000.f;
		}
	}

	// Processing the salmon state
	assert(registry.screenStates.components.size() <= 1);
	ScreenState &screen = registry.screenStates.components[0];

	float min_counter_ms = 3000.f;
	for (Entity entity : registry.deathTimers.entities) {
		// progress timer
		DeathTimer& counter = registry.deathTimers.get(entity);
		counter.counter_ms -= elapsed_ms_since_last_update;
		if(counter.counter_ms < min_counter_ms){
			min_counter_ms = counter.counter_ms;
		}

		// restart the game once the death timer expired
		if (counter.counter_ms < 0) {
			registry.deathTimers.remove(entity);
			screen.darken_screen_factor = 0;
			restart_game();
			return true;
		}
	}

	// Updating dash components

	// distance tracking in dash:
	// min distance between player pos and target will be 10.0 for standard
	vec2 min_dash_diff = vec2(50.f, 50.f);
	float min_counter_ms_stamina = 1000.f;
	for (Entity entity : registry.dashing.entities) {
		// progress diff value
		Dash& player_dash = registry.dashing.get(entity);
		if (player_dash.diff != vec2(0.f, 0.f)) {

			player_dash.diff = player_dash.target - registry.motions.get(my_player).position;

			min_dash_diff = player_dash.diff;

			if ((player_dash.diff.x <= 0) && (player_dash.diff.y <= 0)) {
				// target reached - change velocity
				player_dash.diff = vec2(0.f, 0.f);
				registry.motions.get(my_player).velocity = vec2(0,0);
			} else if (player_dash.diff.x <= 0) {
				player_dash.diff = vec2(0.f, player_dash.diff.y);
			} else if (player_dash.diff.y <= 0) {
				player_dash.diff = vec2(player_dash.diff.x, 0.f);
			}
		}

		player_dash.stamina_timer_ms -= elapsed_ms_since_last_update;
		if(player_dash.stamina_timer_ms < min_counter_ms_stamina){
		    min_counter_ms_stamina = player_dash.stamina_timer_ms;
		}

		if (player_dash.stamina_timer_ms < 0) {
			registry.dashing.remove(my_player);
			continue;
		}

	}


	// reduce window brightness if the salmon is dying
	screen.darken_screen_factor = 1 - min_counter_ms / 3000;


	//Lights flickering
	// if salmon is not dying - let lights effect on screen
	if (!registry.deathTimers.has(my_player)) {
		lightflicker_counter_ms += elapsed_ms_since_last_update;
		if (lightflicker_counter_ms >= LIGHT_FLICKER_RATE) {
			screen.darken_screen_factor = 0.25;
			lightflicker_counter_ms = 0;
		} else if (lightflicker_counter_ms < 400) {
			if ((lightflicker_counter_ms - (lightflicker_counter_ms % 15)) % 30 < 15) {
				screen.darken_screen_factor = 0;
			} else {
				screen.darken_screen_factor = 0.25;
			}
		} else {
			screen.darken_screen_factor = 0;
		}
	}

		
	auto& attackRegistry = registry.playerAttacks;
	for (Entity entity : attackRegistry.entities) {
		auto& attack = attackRegistry.get(entity);

		attack.duration_ms -= elapsed_ms_since_last_update;
		if (attack.duration_ms < 0.0f || attack.has_hit) {
			registry.remove_all_components_of(entity);
		}
	}


	if (debugging.in_debug_mode == true) {
		for (Motion& motion : motions_registry.components) {
			createLine(motion.position, motion.scale);
		}
		
	}

	return true;
}

// Reset the world state to its initial state
void WorldSystem::restart_game() {
	// Debugging for memory/component leaks
	registry.list_all_components();
	printf("Restarting\n");

	// Reset the game speed
	current_speed = 1.f;
	is_paused = false;

	// Remove all entities that we created
	// All that have a motion, we could also iterate over all fish, eels, ... but that would be more cumbersome
	while (registry.motions.entities.size() > 0)
		registry.remove_all_components_of(registry.motions.entities.back());

	// Debugging for memory/component leaks
	registry.list_all_components();


	// create a new Player
	my_player = createPlayer(renderer, { window_width_px/2, window_height_px - 200 });
	registry.colors.insert(my_player, {1, 0.8f, 0.8f});

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
	for (int i = playerPos_init.x - 8; i <= playerPos_init.x + 8; i++) {
		for (int j = playerPos_init.y - 8; j <= playerPos_init.y + 8; j++) {
			if ((i < 0) || (j < 0) || (i >= map1[0].size()) || j >= map1.size()) {
				createWalls(renderer, {(640 - (25*100)) + (i * TILE_SIZE) + (sign(i) * TILE_SIZE/2), (640 - (44*100)) + (j * TILE_SIZE) + (sign(j) * TILE_SIZE/2)}, false);
				tile_vec.push_back(vec2(i, j));
			} else if (map1[j][i] == 0) {
				createWalls(renderer, {(640 - (25*100)) + (i * TILE_SIZE) + (TILE_SIZE/2), (640 - (44*100)) + (j * TILE_SIZE) + (TILE_SIZE/2)}, false);
				tile_vec.push_back(vec2(i, j));
			}
		}
	}
	
	

	// create health bar
	vec2 hp_bar_pos = { -0.75, 0.85f };
	hp_bar = createHPBar(renderer, hp_bar_pos);
	//createHPBarEmpty(renderer, hp_bar_pos);
	


}

// utility functions for dash mvmnt implementation
vec2 lerp(vec2 start, vec2 end, float t) {
	return start * (1-t) + end*t;
}

float distance(vec2 coord1, vec2 coord2) {	
	return sqrt(powf(coord2.x - coord1.x, 2.f) + powf(coord2.y - coord1.y, 2.f));
}

vec2 norm(vec2 vec) {
	return (vec / (vec.x + vec.y));
}

// Compute collisions between entities
void WorldSystem::handle_collisions() {
	// Loop over all collisions detected by the physics system
	auto& collisionsRegistry = registry.collisions;
	for (uint i = 0; i < collisionsRegistry.components.size(); i++) {
		// The entity and its collider
		Entity entity = collisionsRegistry.entities[i];
		Entity entity_other = collisionsRegistry.components[i].other;

		if (registry.players.has(entity)) {
			//Player& player = registry.players.get(entity);

			// Checking Player - Deadly collisions
			if (registry.deadlys.has(entity_other)) {
				float& player_hp = registry.healths.get(entity).hit_points;
				Player& player = registry.players.get(entity);
				if (!player.invulnerable) {
					// player takes damage
					player_hp -= registry.damages.get(entity_other).damage;
					// avoid negative hp values for hp bar
					player_hp = max(0.f, player_hp);
					// modify hp bar
					// std::cout << "Player hp: " << player_hp << "\n";
					if (player_hp <= 200 && player_hp >= 0) {
						// Motion& motion = registry.motions.get(hp_bar);
						// motion.scale.x = HPBAR_BB_WIDTH * (player_hp / 100);
						RenderRequest& hp_bar_render = registry.renderRequests.get(hp_bar);
						if (hp_bar_render.used_texture != TEXTURE_ASSET_ID::HP_BAR_0) {
							hp_bar_render.used_texture = static_cast<TEXTURE_ASSET_ID>(static_cast<int>(hp_bar_render.used_texture) - 1);
						}
						
						
						// motion.position.x += HPBAR_BB_WIDTH * (player_hp / 400);
					}
					player.invulnerable = true;
					player.invulnerable_duration_ms = 1000.f;
				}
				if (registry.deadlys.get(entity_other).enemy_type == ENEMY_TYPES::PROJECTILE) {
					registry.remove_all_components_of(entity_other);
				}
				if (!registry.deathTimers.has(entity) && player_hp <= 0.f) {
					// Scream, reset timer, and make the salmon sink
					registry.deathTimers.emplace(entity);
					Mix_PlayChannel(-1, salmon_dead_sound, 0);

					// Control what happens when the player dies here
					Motion& motion = registry.motions.get(my_player);
					motion.velocity[0] = 0.0f;
					motion.velocity[1] = 0.0f;
				}
			}
			// Checking Player - Eatable collisions
			else if (registry.eatables.has(entity_other)) {
				if (!registry.deathTimers.has(entity)) {
					// chew, count points, and set the LightUp timer
					registry.remove_all_components_of(entity_other);
					Mix_PlayChannel(-1, salmon_eat_sound, 0);
					++points;

				}
			}
			// Checking player collision with solid object
			if (registry.solidObjs.has(entity_other) || registry.walls.has(entity_other)) {
				Motion& motion_moving = registry.motions.get(entity);
				Motion& motion_solid = registry.motions.get(entity_other);

				// Temp solution to prevent player from sticking to solid objects - may not work if solid object is really long or tall
				
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
						std::cout << "left bound overlap" << std::endl;

					}
					if ((left_2 <= right_1) && (left_2 >= left_1)) {
						// player bounding box right bound overlaps with object box
						//motion_moving.velocity.x = 0.f;
						motion_moving.position.x -= (right_1 - left_2) + 1;
						std::cout << "right bound overlap" << std::endl;
					}
				} else {
					

					if ((top_1 >= bottom_2) && (top_1 <= top_2)) {
						// player bounding box top bound overlaps with object box
						motion_moving.position.y += (bottom_2 - top_1) + 1;
						//motion_moving.velocity.y = 0.f;
						std::cout << "top bound overlap" << std::endl;
					}
					if ((top_2 >= bottom_1) && (top_2 <= top_1)) {
						// player bounding box bottom bound overlaps with object box
						motion_moving.position.y -= (bottom_1 - top_2) + 1;
						//motion_moving.velocity.y = 0.f;
						std::cout << "bottom bound overlap" << std::endl;

					}

				}
				
				
				

				/*
				float x_diff = motion_moving.position.x - motion_solid.position.x;
				float y_diff = motion_moving.position.y - motion_solid.position.y;

				if (x_diff < 0 && abs(x_diff) > abs(y_diff) && motion_moving.velocity.x > 0) {
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
				*/
			}
		} else if (registry.deadlys.has(entity)) {
			if (registry.solidObjs.has(entity_other) && registry.projectiles.has(entity)) {
				registry.remove_all_components_of(entity);
			}
		} else if (registry.playerAttacks.has(entity)) {
			if (registry.deadlys.has(entity_other) && registry.healths.has(entity_other)) {
				Health& deadly_health = registry.healths.get(entity_other);
				Damage& damage = registry.damages.get(entity);

				deadly_health.hit_points = std::max(0.0f, deadly_health.hit_points - damage.damage);

				std::cout << "entity " << entity_other << " hitpoints: " << deadly_health.hit_points << std::endl;

				if (deadly_health.hit_points <= 0.0f) {
					registry.remove_all_components_of(entity_other);
					std::cout << "entity " << entity_other << " died" << std::endl;
				}

				registry.playerAttacks.get(entity).has_hit = true;
			}
		}
	}

	// Remove all collisions from this simulation step
	registry.collisions.clear();
}

// Should the game be over ?
bool WorldSystem::is_over() const {
	return bool(glfwWindowShouldClose(window));
}

// Helper function to read a file line by line
std::vector<std::string> read_file(std::string filepath) {
	std::ifstream ReadFile(filepath);
	std::string text;
	std::vector <std::string> lines;
	while (std::getline(ReadFile, text)) {
		lines.push_back(text);
	}
	ReadFile.close();
	return lines;
}

// On key callback
void WorldSystem::on_key(int key, int, int action, int mod) {
	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
		int w, h;
		glfwGetWindowSize(window, &w, &h);

		restart_game();
	}

	if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE) {
		is_paused = !is_paused;
		ScreenState& screen = registry.screenStates.components[0];
		if (is_paused) {
			screen.darken_screen_factor = 0.9;
		}
		else {
			screen.darken_screen_factor = 0;
		}
		std::cout << is_paused << std::endl;
	}

	if (action == GLFW_RELEASE && key == GLFW_KEY_F) {
		display_fps = !display_fps;
	}

	// Debugging
	if (key == GLFW_KEY_P) {
		if (action == GLFW_RELEASE)
			debugging.in_debug_mode = !debugging.in_debug_mode;
	}


	// Player controller
	Motion& pmotion = registry.motions.get(my_player);
	AnimationSet& animSet = registry.animationSets.get(my_player);
	Player& player = registry.players.get(my_player);

	if (!registry.deathTimers.has(my_player)) {
		if (key == GLFW_KEY_RIGHT || key == GLFW_KEY_D) {
			Motion& motion = registry.motions.get(my_player);
			if ((action == GLFW_PRESS || action == GLFW_REPEAT) && !registry.deathTimers.has(my_player)) {
				motion.velocity[0] = motion.speed;
			}
			else if (action == GLFW_RELEASE && !registry.deathTimers.has(my_player) && motion.velocity[0] > 0) {
				motion.velocity[0] = 0.f;
			}
		}

		if (key == GLFW_KEY_LEFT || key == GLFW_KEY_A) {
			Motion& motion = registry.motions.get(my_player);
			if ((action == GLFW_PRESS || action == GLFW_REPEAT) && !registry.deathTimers.has(my_player)) {
				motion.velocity[0] = -1.0 * motion.speed;
			}
			else if (action == GLFW_RELEASE && !registry.deathTimers.has(my_player) && motion.velocity[0] < 0) {
				motion.velocity[0] = 0.f;
			}
		}

		if (key == GLFW_KEY_UP || key == GLFW_KEY_W) {
			Motion& motion = registry.motions.get(my_player);
			if ((action == GLFW_PRESS || action == GLFW_REPEAT) && !registry.deathTimers.has(my_player)) {
				motion.velocity[1] = -1.0 * motion.speed;
			}
			else if (action == GLFW_RELEASE && !registry.deathTimers.has(my_player) && motion.velocity[1] < 0) {
				motion.velocity[1] = 0.f;
			}
		}

		if (key == GLFW_KEY_DOWN || key == GLFW_KEY_S) {
			Motion& motion = registry.motions.get(my_player);
			if ((action == GLFW_PRESS || action == GLFW_REPEAT) && !registry.deathTimers.has(my_player)) {
				motion.velocity[1] = motion.speed;
			}
			else if (action == GLFW_RELEASE && !registry.deathTimers.has(my_player) && motion.velocity[1] > 0) {
				motion.velocity[1] = 0.f;
			}
		}

		// player.move_direction = glm::normalize(player.move_direction);
		// pmotion.velocity = player.move_direction * pmotion.speed;
	}
	// TEMPORARY animation state handler (TO BE CHANGED)
	// *** NOT PERMANENT ***

	if (pmotion.velocity != vec2(0,0)) {
		player.state = PLAYER_STATE::RUN;
	} else {
		player.state = PLAYER_STATE::IDLE;
	}

	switch(player.state) {
		case PLAYER_STATE::DASH:
			break;
		case PLAYER_STATE::ATTACK:	
			break;
		case PLAYER_STATE::RUN:
			animSet.current_animation = "player_run_f";
			break;
		case PLAYER_STATE::IDLE:
			animSet.current_animation = "player_idle_f";
			break;
		default:
			std::cout << "player state not found" << std::endl;
			break;
	}

	if (player.move_direction.x < 0) {
			pmotion.scale.x = -std::abs(pmotion.scale.x);
		} else {
			pmotion.scale.x = std::abs(pmotion.scale.x);
		}

	// if (pmotion.velocity != vec2(0,0)) {
	// 	player.state = PLAYER_STATE::RUN;
	// 	animSet.current_animation = "player_run_f";
		
	// 	if (pmotion.velocity.x < 0) {
	// 		pmotion.scale.x = -std::abs(pmotion.scale.x);
	// 	} else {
	// 		pmotion.scale.x = std::abs(pmotion.scale.x);
	// 	}
	// } else {
	// 	std::cout << "IDLING" << std::endl;
	// 	// animSet.current_animation = "player_idle_f";
	// }

	// Control the current speed with `<` `>`
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_COMMA) {
		current_speed -= 0.1f;
		printf("Current speed = %f\n", current_speed);
	}
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_PERIOD) {
		current_speed += 0.1f;
		printf("Current speed = %f\n", current_speed);
	}
	current_speed = fmax(0.f, current_speed);

	Motion& motion = registry.motions.get(my_player);

	if (key == GLFW_KEY_X) {
		if (action == GLFW_PRESS && !registry.deathTimers.has(my_player)) {
			if (!registry.dashing.has(my_player)) {

				vec2 dashtarget = registry.motions.get(my_player).position + vec2(200.f, 0.f);
				if (dashtarget.x > window_width_px - 100.f) {
					dashtarget.x = window_width_px - 100.f;
				}
				
				Dash new_dash = {dashtarget, registry.motions.get(my_player).position - (registry.motions.get(my_player).position + vec2(0, 5)), 1000};

				registry.dashing.insert(my_player, new_dash);
			} 
		}
	}

	// Tutorial
	if (action == GLFW_RELEASE && key == GLFW_KEY_T) {
		ScreenState& screen = registry.screenStates.components[0];
		// if the game is already paused then this shouldn't work
		if (!is_paused) {
			is_paused = true;
			is_tutorial_on = true;
			screen.darken_screen_factor = 0.9;
			float tutorial_header_x = window_width_px / 2 - 120;
			float tutorial_header_y = 620;
			glm::vec3 white = glm::vec3(1.f, 1.f, 1.f);
			createText({ tutorial_header_x, tutorial_header_y }, 1.0, "TUTORIAL", white);
			std::vector<std::string> lines = read_file(PROJECT_SOURCE_DIR + std::string("data/tutorial/tutorial.txt"));
			float y_spacer = 80.f;
			for (std::string line : lines) {
				// Render each line
				createText({ tutorial_header_x - 400.f, tutorial_header_y - y_spacer }, 0.6f, line, white);
				y_spacer += 70.f;
			}
		}
		else if (is_tutorial_on) {
			is_paused = false;
			is_tutorial_on = false;
			screen.darken_screen_factor = 0.0;
		}
	}

	// Dash movement
	if (registry.dashing.has(my_player)) {
		if (registry.dashing.get(my_player).diff != vec2(0.f, 0.f)) {
			motion.velocity = lerp(vec2(0, 0), registry.dashing.get(my_player).target - motion.position, 0.1);
			motion.velocity = motion.speed * motion.velocity;
		}
	}
}

vec2 snapToClosestAxis(vec2 direction) {
	vec2 normalized = glm::normalize(direction);

	float absX = std::abs(normalized.x);
	float absY = std::abs(normalized.y);

    if (absX >= absY) {
        return vec2(normalized.x > 0 ? 1 : -1, 0);
    } else {
        return vec2(0, normalized.y > 0 ? 1 : -1);
    }
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {
	// Update player attack direction
	vec2 screen_center = vec2(window_width_px / 2.0f, window_height_px / 2.0f);
	vec2 direction = mouse_position - screen_center;
	direction = normalize(direction);

	Player& player = registry.players.get(my_player);
	player.attack_direction = snapToClosestAxis(direction);
}

void WorldSystem::on_mouse_button(int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		Player& player = registry.players.get(my_player);
		vec2 player_pos = registry.motions.get(my_player).position;
		vec2 attack_direction = player.attack_direction;

		createBasicAttackHitbox(renderer, player_pos + (attack_direction * vec2(100, 100)));
	}
}

// TODO: update to work with multiple maps
std::vector<std::vector<int>> WorldSystem::get_current_map() {
	return map1;
}
