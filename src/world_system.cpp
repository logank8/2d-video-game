// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <iostream>

#include "physics_system.hpp"

// Game configuration
const size_t MAX_NUM_EELS = 5;
const size_t MAX_NUM_FISH = 2;
const size_t EEL_SPAWN_DELAY_MS = 2000 * 3;
const size_t FISH_SPAWN_DELAY_MS = 5000 * 3;
const size_t MAX_NUM_RANGED_ENEMY = 1;
const size_t RANGED_ENEMY_SPAWN_DELAY_MS = 5000 * 3;
const size_t RANGED_ENEMY_PROJECTILE_DELAY_MS = 2000 * 3;

bool rstuck = false;
bool lstuck = false;
bool ustuck = false;
bool dstuck = false;

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
	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);

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

	// Remove entities that leave the screen on the left side
	// Iterate backwards to be able to remove without unterfering with the next object to visit
	// (the containers exchange the last element with the current)
	for (int i = (int)motions_registry.components.size()-1; i>=0; --i) {
		Motion& motion = motions_registry.components[i];
		if (motion.position.x + abs(motion.scale.x) < 0.f) {
			if(!registry.players.has(motions_registry.entities[i])) // don't remove the player
				registry.remove_all_components_of(motions_registry.entities[i]);
		}
	}


	vec2 player_pos = motions_registry.get(my_player).position;
	//TODO: spawn frequencies and spawn radius to be adjusted
	// Spawn Level 1 type enemy: slow with contact damage
	next_fish_spawn -= elapsed_ms_since_last_update * current_speed;
	if (registry.eatables.components.size() <= MAX_NUM_FISH && next_fish_spawn < 0.f) {
		next_fish_spawn = (FISH_SPAWN_DELAY_MS / 2) + uniform_dist(rng) * (FISH_SPAWN_DELAY_MS / 2);
		vec2 fish_pos;
		float distance_to_player;
		do {
			fish_pos = { 50.0f + uniform_dist(rng) * (window_width_px - 100.f), 50.f + uniform_dist(rng) * (window_height_px - 100.f) };
			distance_to_player = sqrt(pow(fish_pos.x - player_pos.x, 2) + pow(fish_pos.y - player_pos.y, 2));
		} while (distance_to_player < 500.f);
		Entity fish = createFish(renderer, fish_pos);
		registry.motions.get(fish).velocity = { 50.f, 50.f };
	}

	// Spawn Level 2 type enemy: fast with contact damage
	next_eel_spawn -= elapsed_ms_since_last_update * current_speed;
	if (registry.deadlys.components.size() <= MAX_NUM_EELS && next_eel_spawn < 0.f) {
		// reset timer
		next_eel_spawn = (EEL_SPAWN_DELAY_MS / 2) + uniform_dist(rng) * (EEL_SPAWN_DELAY_MS / 2);
		vec2 eel_pos;
		float distance_to_player;
		do {
			eel_pos = { 100.0f + uniform_dist(rng) * (window_width_px - 150.f), 50.f + uniform_dist(rng) * (window_height_px - 100.f) };
			distance_to_player = sqrt(pow(eel_pos.x - player_pos.x, 2) + pow(eel_pos.y - player_pos.y, 2));
		} while (distance_to_player < 500.f);
		Entity eel = createEel(renderer, eel_pos);
		registry.motions.get(eel).velocity = { 100.f, 100.f };
	}

	// Spawn Level 3 type enemy: slow ranged enemy
	next_ranged_spawn -= elapsed_ms_since_last_update * current_speed;
	if (registry.ranged.components.size() <= MAX_NUM_RANGED_ENEMY && next_ranged_spawn < 0.f) {
		// reset timer
		next_ranged_spawn = (RANGED_ENEMY_SPAWN_DELAY_MS / 2) + uniform_dist(rng) * (RANGED_ENEMY_SPAWN_DELAY_MS / 2);
		vec2 ranged_enemy_pos;
		float distance_to_player;
		do {
			ranged_enemy_pos = { 10.0f + uniform_dist(rng) * (window_width_px - 100.f), 50.f + uniform_dist(rng) * (window_height_px - 100.f) };
			distance_to_player = sqrt(pow(ranged_enemy_pos.x - player_pos.x, 2) + pow(ranged_enemy_pos.y - player_pos.y, 2));
		} while (distance_to_player < 500.f);
		Entity ranged_enemy = createRangedEnemy(renderer, ranged_enemy_pos);
		registry.motions.get(ranged_enemy).velocity = { 10.f, 10.f };
	}

	// Spawn projectiles for ranged enemies
	for (auto& ranged : registry.ranged.entities) {
		float& projectile_delay = registry.ranged.get(ranged).projectile_delay;
		projectile_delay -= elapsed_ms_since_last_update * current_speed;
		if (projectile_delay < 0.f) {
			// reset timer
			projectile_delay = (RANGED_ENEMY_PROJECTILE_DELAY_MS / 2) + uniform_dist(rng) * (RANGED_ENEMY_PROJECTILE_DELAY_MS / 2);
			Entity projectile = createRangedProjectile(renderer, registry.motions.get(ranged).position);
			Motion& projectile_motion = registry.motions.get(projectile);
			projectile_motion.velocity = { 200.f, 200.f };
			projectile_motion.angle = registry.motions.get(ranged).angle;
		}

	}

	// Check if player is invulnerable
	Player& player = registry.players.get(my_player);
	if (player.invulnerable) {
		player.invulnerable_duration_ms -= elapsed_ms_since_last_update;
		if (player.invulnerable_duration_ms < 0.f) {
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

		// std::cout << player_dash.stamina_timer_ms << std::endl;
		if (player_dash.stamina_timer_ms < 0) {
			registry.dashing.remove(my_player);
			continue;
		}

	}


	// reduce window brightness if the salmon is dying
	screen.darken_screen_factor = 1 - min_counter_ms / 3000;

	// !!! TODO A1: update LightUp timers and remove if time drops below zero, similar to the death counter

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

	// create furniture/table (for testing)
	createFurniture(renderer, { window_width_px / 4, window_height_px * 3 / 4 });

	// create walls on screen boundary (top & bottom)
	for (int i = 0; i < window_width_px + FURNITURE_WIDTH * 3; i += FURNITURE_WIDTH * 3) {
		createWalls(renderer, { i, 0 }, false);
		createWalls(renderer, { i, window_height_px }, false);
	}
	// create walls on screen boundary (sides)
	for (int j = 0; j < window_height_px + FURNITURE_WIDTH * 3; j += FURNITURE_WIDTH * 3) {
		createWalls(renderer, { 0, j }, true);
		createWalls(renderer, { window_width_px, j }, true);
	}

	// create health bar
	vec2 hp_bar_pos = { -0.75, 0.8f };
	createHPBarEmpty(renderer, hp_bar_pos);
	hp_bar = createHPBar(renderer, hp_bar_pos);

	// !! TODO A2: Enable static eggs on the ground, for reference
	// Create eggs on the floor, use this for reference
	/*
	for (uint i = 0; i < 20; i++) {
		int w, h;
		glfwGetWindowSize(window, &w, &h);
		float radius = 30 * (uniform_dist(rng) + 0.3f); // range 0.3 .. 1.3
		Entity egg = createEgg({ uniform_dist(rng) * w, h - uniform_dist(rng) * 20 },
					 { radius, radius });
		float brightness = uniform_dist(rng) * 0.5 + 0.5;
		registry.colors.insert(egg, { brightness, brightness, brightness});
	}
	*/
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

		// for now, we are only interested in collisions that involve the salmon
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
					std::cout << "Player hp: " << player_hp << "\n";
					if (player_hp <= 100 && player_hp >= 0) {
						// Motion& motion = registry.motions.get(hp_bar);
						// motion.scale.x = HPBAR_BB_WIDTH * (player_hp / 100);
						UserInterface& userInterface = registry.userInterfaces.get(hp_bar);
						userInterface.scale.x = HPBAR_BB_WIDTH * (player_hp / 100);
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

					// !!! TODO A1: change the salmon's orientation and color on death
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
			if (registry.solidObjs.has(entity_other)) {
				Motion& motion_moving = registry.motions.get(entity);
				Motion& motion_solid = registry.motions.get(entity_other);
				if (motion_moving.position.x < motion_solid.position.x - (motion_solid.scale.x / 2) && motion_moving.velocity.x > 0) {
					motion_moving.velocity.x = 0.f;
					rstuck = true;
				} else if (motion_moving.position.x > motion_solid.position.x + (motion_solid.scale.x / 2) && motion_moving.velocity.x < 0)
				{
					motion_moving.velocity.x = 0.f;
					lstuck = true;
				} else if (motion_moving.position.y < motion_solid.position.y - (motion_solid.scale.y / 2) && motion_moving.velocity.y > 0)
				{
					motion_moving.velocity.y = 0.f;
					dstuck = true;
				} else if (motion_moving.position.y > motion_solid.position.y + (motion_solid.scale.y / 2) && motion_moving.velocity.y < 0)
				{
					motion_moving.velocity.y = 0.f;
					ustuck = true;
				}
			}
		} else if (registry.deadlys.has(entity)) {
			if (registry.solidObjs.has(entity_other) && registry.deadlys.get(entity).enemy_type == ENEMY_TYPES::PROJECTILE) {
				registry.remove_all_components_of(entity);
			} else if (registry.solidObjs.has(entity_other)) {
				if (!registry.blockedTimers.has(entity)) registry.blockedTimers.emplace(entity);

				Motion& motion_moving = registry.motions.get(entity);
				Motion& motion_solid = registry.motions.get(entity_other);

				if (motion_moving.position.x < motion_solid.position.x - ((motion_moving.scale.x + motion_solid.scale.x) / 2) && motion_moving.velocity.x > 0) {
					motion_moving.velocity.x = 0.f;
				} else if (motion_moving.position.x > motion_solid.position.x + ((motion_moving.scale.x + motion_solid.scale.x) / 2) && motion_moving.velocity.x < 0)
				{
					motion_moving.velocity.x = 0.f;
				} else if (motion_moving.position.y < motion_solid.position.y - ((motion_moving.scale.y + motion_solid.scale.y) / 2) && motion_moving.velocity.y > 0)
				{
					motion_moving.velocity.y = 0.f;
				} else if (motion_moving.position.y > motion_solid.position.y + ((motion_moving.scale.y + motion_solid.scale.y) / 2) && motion_moving.velocity.y < 0)
				{
					motion_moving.velocity.y = 0.f;
				}
			}
		}
		// Checking collision with solid object
		// if (registry.solidObjs.has(entity_other)) {
		// 	Motion& motion_moving = registry.motions.get(entity);
		// 	Motion& motion_solid = registry.motions.get(entity_other);

		// 	if (registry.players.has(entity)) {
		// 		if (motion_moving.position.x < motion_solid.position.x - (motion_solid.scale.x / 2) && motion_moving.velocity.x > 0) {
		// 			motion_moving.velocity.x = 0.f;
		// 			rstuck = true;
		// 		} else if (motion_moving.position.x > motion_solid.position.x + (motion_solid.scale.x / 2) && motion_moving.velocity.x < 0)
		// 		{
		// 			motion_moving.velocity.x = 0.f;
		// 			lstuck = true;
		// 		} else if (motion_moving.position.y < motion_solid.position.y - (motion_solid.scale.y / 2) && motion_moving.velocity.y > 0)
		// 		{
		// 			motion_moving.velocity.y = 0.f;
		// 			dstuck = true;
		// 		} else if (motion_moving.position.y > motion_solid.position.y + (motion_solid.scale.y / 2) && motion_moving.velocity.y < 0)
		// 		{
		// 			motion_moving.velocity.y = 0.f;
		// 			ustuck = true;
		// 		}
		// 	} else if (registry.deadlys.has(entity) && registry.deadlys.get(entity).enemy_type == ENEMY_TYPES::PROJECTILE) {
		// 		registry.remove_all_components_of(entity);
		// 	} else if (registry.deadlys.has(entity)) {
		// 		if (!registry.blockedTimers.has(entity)) registry.blockedTimers.emplace(entity);

		// 		if (motion_moving.position.x < motion_solid.position.x - (motion_solid.scale.x / 2) && motion_moving.velocity.x > 0) {
		// 			motion_moving.velocity.x = 0.f;
		// 		} else if (motion_moving.position.x > motion_solid.position.x + (motion_solid.scale.x / 2) && motion_moving.velocity.x < 0)
		// 		{
		// 			motion_moving.velocity.x = 0.f;
		// 		} else if (motion_moving.position.y < motion_solid.position.y - (motion_solid.scale.y / 2) && motion_moving.velocity.y > 0)
		// 		{
		// 			motion_moving.velocity.y = 0.f;
		// 		} else if (motion_moving.position.y > motion_solid.position.y + (motion_solid.scale.y / 2) && motion_moving.velocity.y < 0)
		// 		{
		// 			motion_moving.velocity.y = 0.f;
		// 		}
		// 	}
		// }
	}

	// Remove all collisions from this simulation step
	registry.collisions.clear();
}

// Should the game be over ?
bool WorldSystem::is_over() const {
	return bool(glfwWindowShouldClose(window));
}

// On key callback
void WorldSystem::on_key(int key, int, int action, int mod) {
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A1: HANDLE SALMON MOVEMENT HERE
	// key is of 'type' GLFW_KEY_
	// action can be GLFW_PRESS GLFW_RELEASE GLFW_REPEAT
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
		int w, h;
		glfwGetWindowSize(window, &w, &h);

		restart_game();
	}

	if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE) {
		is_paused = !is_paused;
		std::cout << is_paused << std::endl;
	}

	// Debugging
	/*if (key == GLFW_KEY_D) {
		if (action == GLFW_RELEASE)
			debugging.in_debug_mode = false;
		else
			debugging.in_debug_mode = true;
	}*/

	if (key == GLFW_KEY_RIGHT || key == GLFW_KEY_D) {
		Motion& motion = registry.motions.get(my_player);
		if ((action == GLFW_PRESS || action == GLFW_REPEAT) && !registry.deathTimers.has(my_player) && !rstuck) {
			motion.velocity[0] = motion.speed;
			lstuck = false;
		}
		else if (action == GLFW_RELEASE && !registry.deathTimers.has(my_player)) {
			motion.velocity[0] = 0.f;
		}
	}

	if (key == GLFW_KEY_LEFT || key == GLFW_KEY_A) {
		Motion& motion = registry.motions.get(my_player);
		if ((action == GLFW_PRESS || action == GLFW_REPEAT) && !registry.deathTimers.has(my_player) && !lstuck) {
			motion.velocity[0] = -1.0 * motion.speed;
			rstuck = false;
		}
		else if (action == GLFW_RELEASE && !registry.deathTimers.has(my_player)) {
			motion.velocity[0] = 0.f;
		}
	}

	if (key == GLFW_KEY_UP || key == GLFW_KEY_W) {
		Motion& motion = registry.motions.get(my_player);
		if ((action == GLFW_PRESS || action == GLFW_REPEAT) && !registry.deathTimers.has(my_player) && !ustuck) {
			motion.velocity[1] = -1.0 * motion.speed;
			dstuck = false;
		}
		else if (action == GLFW_RELEASE && !registry.deathTimers.has(my_player)) {
			motion.velocity[1] = 0.f;
		}
	}

	if (key == GLFW_KEY_DOWN || key == GLFW_KEY_S) {
		Motion& motion = registry.motions.get(my_player);
		if ((action == GLFW_PRESS || action == GLFW_REPEAT) && !registry.deathTimers.has(my_player) && !dstuck) {
			motion.velocity[1] = motion.speed;
			ustuck = false;
		}
		else if (action == GLFW_RELEASE && !registry.deathTimers.has(my_player)) {
			motion.velocity[1] = 0.f;
		}
	}

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

	

	// Dash movement
	if (registry.dashing.has(my_player)) {
		if (registry.dashing.get(my_player).diff != vec2(0.f, 0.f)) {
			motion.velocity = lerp(vec2(0, 0), registry.dashing.get(my_player).target - motion.position, 0.1);
			motion.velocity = motion.speed * motion.velocity;
		}
	}
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A1: HANDLE SALMON ROTATION HERE
	// xpos and ypos are relative to the top-left of the window, the salmon's
	// default facing direction is (1, 0)
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	(vec2)mouse_position; // dummy to avoid compiler warning
}
