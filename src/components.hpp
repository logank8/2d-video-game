#pragma once
#include "common.hpp"
#include <vector>
#include <unordered_map>
#include "../ext/stb_image/stb_image.h"

enum class PLAYER_STATE
{
	IDLE = 0,
	RUN = IDLE + 1,
	ATTACK = RUN + 1,
	DASH = ATTACK + 1,
	DEAD = DASH + 1
};

enum class ENEMY_STATE
{
	IDLE = 0,
	RUN = IDLE + 1,
	ATTACK = RUN + 1,
	DEAD = ATTACK + 1,
	KNOCKED_BACK = DEAD + 1
};

enum class BUFF_TYPE
{
	HEALTH = 0,
	SPEED = HEALTH + 1,
	ATTACK = SPEED + 1
};

enum class EFFECT_TYPE
{
	HEART = 0,
	SMOKE = HEART + 1,
	DASH = SMOKE + 1
};

enum class RENDER_LAYER
{
	FLOOR = 0,
	FLOOR_DECOR = FLOOR + 1,
	EFFECTS = FLOOR_DECOR + 1,
	CREATURES = EFFECTS + 1,
	OBSTACLES = CREATURES + 1,
	DEFAULT_LAYER = OBSTACLES + 1,
	UI_LAYER_1 = DEFAULT_LAYER + 1,
	UI_LAYER_2 = UI_LAYER_1 + 1
};

// Player component
struct Player
{
	// General
	bool is_moving = false;
	vec2 move_direction = {0, 0};
	vec2 attack_direction = {0, 0};
	vec2 last_direction = {0, 1};
	PLAYER_STATE state = PLAYER_STATE::IDLE;

	// For dashing and after taking damage;
	bool invulnerable = false;
	float invulnerable_duration_ms = 2000.f;
	vec2 last_pos = {0, 0};
	float dash_cooldown_ms = 250.f;
	float dash_time = 100.f;
	float curr_dash_cooldown_ms = dash_cooldown_ms;
	bool is_dash_up = true;
	float slowed_duration_ms = 0.f;
	float slowed_amount = 0.f;


	// For attacking
	bool is_attacking = false;
	float damage_multiplier = 2.0f;
	float attack_duration_ms = 230.f;
	float curr_attack_duration_ms = attack_duration_ms;
	float knockback_strength = 0.08;
	float attack_size = 100.f;
	float crit_chance = 0.05f;
	float crit_multiplier = 2.0f;
	float lifesteal = 0.0f;

	// Other
	float collection_distance = 100.f;
	float experience_multiplier = 1.0f;
	int experience = 0;
	int toNextLevel = 2;
	int level = 0;
	std::vector<int> levels_unlocked = {1};

	// stamina
	float totalStamina = 100.f;
	float currentStamina = 100.f;
	float attackCost = 25.f;
	float dashCost = 50.f;
	float staminaRegen = 5.f; // per 1/4 second
	float curr_stamina_elapsed_ms = 250.f;
};

enum class ENEMY_TYPES
{
	CONTACT_DMG = 0,
	CONTACT_DMG_2 = CONTACT_DMG + 1,
	RANGED = CONTACT_DMG_2 + 1,
	PROJECTILE = RANGED + 1,
	SWARM = PROJECTILE + 1,
	SLOWING_CONTACT = SWARM + 1,
	RANGED_HOMING = SLOWING_CONTACT + 1,
	HOMING_PROJECTILE = RANGED_HOMING + 1,
	FINAL_BOSS = HOMING_PROJECTILE + 1,
	DASHING = FINAL_BOSS + 1
};

struct PlayerAttack
{
	float duration_ms = 100.f;
	bool has_hit = false;
};

// anything that is deadly to the player
struct Deadly
{
	ENEMY_TYPES enemy_type = ENEMY_TYPES::CONTACT_DMG;
	float movement_timer = 0.f;
	float drop_chance = 1.0f;
	int experience = 1;
	ENEMY_STATE state = ENEMY_STATE::IDLE;
	vec2 knocked_back_pos = {INFINITY, INFINITY};
};

struct EnemyKnockback {
	vec2 dir;
	float time_elapsed_ms;
	float force;
};

struct EnemyDash
{
	float charge_time = 3000.f;
	float current_charge_timer = 0.f;
	vec2 target_pos = {0, 0};
};

enum FinalLevelStage
{
	NOT_FINAL_LEVEL = 0,
	STAGE1 = NOT_FINAL_LEVEL + 1,
	STAGE2 = STAGE1 + 1,
	STAGE3 = STAGE2 + 1
};

struct FinalBoss
{
	FinalLevelStage stage = STAGE1;
	float attack_timer_ms = 0;
	float attack_frequency_ms = 10000;

	float stage_1_change_hp = 3500;
	float stage_2_change_hp = 1000;
};

struct Effect
{
	float ms_passed = 0.f;
	float lifespan_ms = 1000.f;
	float width_init;
	float height_init;
	EFFECT_TYPE type;
};

struct SwarmMember
{
	int leader_id;
	float separation_factor;
	float alignment_factor;
	float cohesion_factor;
	float max_speed = 180.f;
	float min_speed = 150.f;
};

struct Ranged
{
	float projectile_delay = 1000.f;
};

struct Projectile
{
};

struct Experience
{
	int experience;
};

struct Collectible
{
	bool is_collected = false;
};

struct Path
{
	std::vector<vec2> points;
	size_t current_index = 0;
};

struct PathTimer
{
	float timer = 0.f;
};

// Walls component
struct Wall
{
};

// Ground component
struct Ground
{
};

// Door component
struct Door
{
	bool touching = false;
};

// Ground component
struct Sticky
{
};

// player can't move through
struct Solid
{
};

// slows player down when walked on - applied to specific ground tiles
struct Slows
{
	float duration = 1000.f;
	float speed_dec = 20;
};

// anything the player can eat
struct Eatable
{
};

// anything that can cause damage
struct Damage
{
	float damage = 25.f;
};

// anything that has health
struct Health
{
	float hit_points = 200.f;
	float max_hp = 200.f;
};

// light up damage effect
struct LightUp
{
	float duration_ms = 100.f;
};

// All data relevant to the shape and motion of entities
struct Motion
{
	vec2 position = {0, 0};
	float angle = 0;
	vec2 velocity = {0, 0};
	vec2 scale = {10, 10};
	float speed = 30.0f; // To control player speed

	vec2 renderScale = {1, 1};
	vec2 renderPositionOffset = {0, 0};
};

// Stucture to store collision information
struct Collision
{
	// Note, the first object is stored in the ECS container.entities
	Entity other; // the second object involved in the collision
	Collision(Entity &other) { this->other = other; };
};

// Data structure for toggling debug mode
struct Debug
{
	bool in_debug_mode = 0;
	bool in_freeze_mode = 0;
};
extern Debug debugging;

enum GameState
{
	START = 0,
	PAUSED = START + 1,
	GAME = PAUSED + 1,
	MENU = GAME + 1,
	GAME_OVER = MENU + 1
};

// Sets the brightness of the screen
struct ScreenState
{
	float darken_screen_factor = -1;
	GameState state = START;
	int lights_on = 0;
};

struct UserInterface
{
	vec2 position = {0, 0};
	float angle = 0;
	vec2 scale = {10, 10};
};

struct HealthBuff
{
	float factor = 1;
};

struct Sigil 
{
	
};

// Any object you have to hold a key down for some time to interact with
struct HoldInteract
{
	bool touching = false;
	bool interacting = false;
	bool activated = false;
	float touch_time_ms = 0;
	float activate_ms = 1500;
	std::function<void(Entity e)> onInteractCallback;
};

// A struct to refer to debugging graphics in the ECS
struct DebugComponent
{
	// Note, an empty struct has size 1
};

// A timer that will be associated to dying salmon
struct DeathTimer
{
	float counter_ms = 3000;
};

// A timer that will be associated to dying salmon
struct AttackTimer
{
	float counter_ms = 700;
};

// Movement mechanic
struct Dash
{
	vec2 target;
	vec2 diff;
	float stamina_timer_ms = 1000;
};

// A timer that will be associated to enemy blocked by a solid object
struct BlockedTimer
{
	float counter_ms = 100;
};

// Single Vertex Buffer element for non-textured meshes (coloured.vs.glsl & salmon.vs.glsl)
struct ColoredVertex
{
	vec3 position;
	vec3 color;
};

// Single Vertex Buffer element for textured sprites (textured.vs.glsl)
struct TexturedVertex
{
	vec3 position;
	vec2 texcoord;
};

// Mesh datastructure for storing vertex and index buffers
struct Mesh
{
	static bool loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex> &out_vertices, std::vector<uint16_t> &out_vertex_indices, vec2 &out_size);
	vec2 original_size = {1, 1};
	std::vector<ColoredVertex> vertices;
	std::vector<uint16_t> vertex_indices;
};

// for all Text components
struct Text
{
	std::string content = "";
	glm::vec3 color;
	float scale;
};

enum PowerupType
{
	DAMAGE_BOOST = 0,
	INVINCIBILITY = DAMAGE_BOOST + 1,
	SPEED_BOOST = INVINCIBILITY + 1
};

// Current temporary powerup held by the player
struct Powerup
{
	PowerupType type;
	float multiplier = 1; // Only in case of damage and speed boost
	float timer = 10000;
	bool equipped = false;
};

struct TutorialIcon
{
};

struct ElevatorButton
{
	int level = 0;
	bool hovering = false;
};

struct ElevatorDisplay
{
	bool selection_made = false;
	float lasting_ms = 1700.f;
	float current_ms = 0.f;
	int message = 0;
	/*
	messages
	 - 0    -> not available
	 - >= 1 -> level
	*/
};

struct DialogueBox
{
};

struct UpgradeCard
{
	int tier = 1;
	int icon_sprite_index;
	Entity icon;
	Entity name;
	Entity description;
	std::function<void()> onClick;
	bool hovering = false;
	vec2 original_scale;
};

struct SelectedCard
{
	vec2 scale;
};

struct KillTracker
{
	int goal;
	int killed = 0;
};

struct ProgressCircle 
{
	Entity connected;
};

struct Tenant
{
	std::vector<std::string> dialogues;
	int dialogue_progress = -1;
	std::vector<std::string> extra_dialogues;
	bool player_in_radius = false;
	bool on_path = false;
	std::vector<vec2> path = {};
};

struct UpgradeConfirm
{
	bool hovering = false;
};

struct Camera
{
};

struct DamageIndicator
{
	float time_total_ms = 1000.0f;
	float time_elapsed_ms = time_total_ms;
	int damage;
	float rng;
	float multiplier;
	Entity text;
};

struct BarIn
{
};

struct Particle {
	float time_elapsed_ms;
	float lifespan_ms;
	vec2 pos;
	vec2 dir;
};

struct ParticleEmitter {
	vec2 directon;
	int particle_count;
	int emits_per_frame;
	int emission_variance;
	float time_elapsed_ms;
	float lifespan_ms;
	int emitted_count = 0;
	std::vector<Particle> particles;
};

struct Spike {
	float follow_ms;
};



/**
 * The following enumerators represent global identifiers refering to graphic
 * assets. For example TEXTURE_ASSET_ID are the identifiers of each texture
 * currently supported by the system.
 *
 * So, instead of referring to a game asset directly, the game logic just
 * uses these enumerators and the RenderRequest struct to inform the renderer
 * how to structure the next draw command.
 *
 * There are 2 reasons for this:
 *
 * First, game assets such as textures and meshes are large and should not be
 * copied around as this wastes memory and runtime. Thus separating the data
 * from its representation makes the system faster.
 *
 * Second, it is good practice to decouple the game logic from the render logic.
 * Imagine, for example, changing from OpenGL to Vulkan, if the game logic
 * depends on OpenGL semantics it will be much harder to do the switch than if
 * the renderer encapsulates all asset data and the game logic is agnostic to it.
 *
 * The final value in each enumeration is both a way to keep track of how many
 * enums there are, and as a default value to represent uninitialized fields.
 */

enum class TEXTURE_ASSET_ID
{
	SLIME = 0,
	SKELETON = SLIME + 1,
	PLAYER = SKELETON + 1,
	RANGED_ENEMY = PLAYER + 1,
	RANGED_PROJECTILE = RANGED_ENEMY + 1,
	HOMING_PROJECTILE = RANGED_PROJECTILE + 1,
	FINAL_BOSS = HOMING_PROJECTILE + 1,
	FINAL_BOSS_DEATH = FINAL_BOSS + 1,
	FINAL_BOSS_ATTACK = FINAL_BOSS_DEATH + 1,
	HP_BAR = FINAL_BOSS_ATTACK + 1,
	PLANT = HP_BAR + 1,
	FURNITURE = PLANT + 1,
	COAT_RACK = FURNITURE + 1,
	CHAIR_FRONT = COAT_RACK + 1,
	CHAIR_BACK = CHAIR_FRONT + 1,
	CHAIR_SIDE = CHAIR_BACK + 1,
	KITCHEN_COUNTER_1 = CHAIR_SIDE + 1,
	KITCHEN_COUNTER_2 = KITCHEN_COUNTER_1 + 1,
	FRIDGE = KITCHEN_COUNTER_2 + 1,
	STOVE = FRIDGE + 1,
	BOOK_CASE = STOVE + 1,
	COFFEE_TABLE = BOOK_CASE + 1,
	COUCH = COFFEE_TABLE + 1,
	DRESSER = COUCH + 1,
	GRANDFATHER_CLOCK = DRESSER + 1,
	LAMP = GRANDFATHER_CLOCK + 1,
	ROUND_TABLE = LAMP + 1,
	SIDE_TABLE = ROUND_TABLE + 1,
	WALL = SIDE_TABLE + 1,
	INNER_WALL = WALL + 1,
	PLAYERS = INNER_WALL + 1,
	HEART = PLAYERS + 1,
	SMOKE_PARTICLE = HEART + 1,
	DASH = SMOKE_PARTICLE + 1,
	STAMINA_BAR = DASH + 1,
	COINS = STAMINA_BAR + 1,
	BEETLE = COINS + 1,
	POWERUP = BEETLE + 1,
	CARD = POWERUP + 1,
	GREY_CAT = CARD + 1,
	ORANGE_CAT = GREY_CAT + 1,
	SLASH = ORANGE_CAT + 1,
	DOOR = SLASH + 1,
	START_SCREEN = DOOR + 1,
	GAME_OVER_SCREEN = START_SCREEN + 1,
	MENU_SCREEN = GAME_OVER_SCREEN + 1,
	LEVEL_BUTTON = MENU_SCREEN + 1,
	EXIT_BUTTON = LEVEL_BUTTON + 1,
	FLOOR = EXIT_BUTTON + 1,
	WASD_KEYS = FLOOR + 1,
	DASH_KEYS = WASD_KEYS + 1,
	ATTACK_CURSOR = DASH_KEYS + 1,
	INTERACT_KEY = ATTACK_CURSOR + 1,
	PAUSE_KEY = INTERACT_KEY + 1,
	UPGRADE_ICONS = PAUSE_KEY + 1,
	HOMING_ENEMY = UPGRADE_ICONS + 1,
	DASHING_ENEMY = HOMING_ENEMY + 1,
	SLOWING_ENEMY = DASHING_ENEMY + 1,
	DIALOGUE_BOX = SLOWING_ENEMY + 1,
	TUTORIAL_TOGGLE_KEY = DIALOGUE_BOX + 1,
	BARS = TUTORIAL_TOGGLE_KEY + 1,
	ELEVATOR_DISPLAY = BARS + 1,
	TENANT_1 = ELEVATOR_DISPLAY + 1,
	TENANT_2 = TENANT_1 + 1,
	TENANT_3 = TENANT_2 + 1,
	TENANT_4 = TENANT_3 + 1,
	LEVELUP_CONFIRM = TENANT_4 + 1,
	TITLE = LEVELUP_CONFIRM + 1,
	PROGRESS_CIRCLE = TITLE + 1,
	SIGIL = PROGRESS_CIRCLE + 1,
	SPIKE = SIGIL + 1,
	TEXTURE_COUNT = SPIKE + 1
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class SPRITE_ASSET_ID
{
	PLAYER = 0,
	SKELETON = PLAYER + 1,
	SLIME = SKELETON + 1,
	RANGED_ENEMY = SLIME + 1,
	FINAL_BOSS = RANGED_ENEMY + 1,
	FINAL_BOSS_DEATH = FINAL_BOSS + 1,
	FINAL_BOSS_ATTACK = FINAL_BOSS_DEATH + 1,
	GREY_CAT = FINAL_BOSS_ATTACK + 1,
	ORANGE_CAT = GREY_CAT + 1,
	STAMINA_BAR = ORANGE_CAT + 1,
	COIN = STAMINA_BAR + 1,
	BEETLE = COIN + 1,
	POWERUP = BEETLE + 1,
	SLASH = POWERUP + 1,
	HP_BAR = SLASH + 1,
	WALL = HP_BAR + 1,
	WASD_KEYS = WALL + 1,
	DASH_KEYS = WASD_KEYS + 1,
	INTERACT_KEY = DASH_KEYS + 1,
	PAUSE_KEY = INTERACT_KEY + 1,
	UPGRADE_ICONS = PAUSE_KEY + 1,
	HOMING_ENEMY = UPGRADE_ICONS + 1,
	DASHING_ENEMY = HOMING_ENEMY + 1,
	SLOWING_ENEMY = DASHING_ENEMY + 1,
	TUTORIAL_TOGGLE_KEY = SLOWING_ENEMY + 1,
	BARS = TUTORIAL_TOGGLE_KEY + 1,
	ELEVATOR_DISPLAY = BARS + 1,
	TENANT_1 = ELEVATOR_DISPLAY + 1,
	TENANT_2 = TENANT_1 + 1,
	TENANT_3 = TENANT_2 + 1,
	TENANT_4 = TENANT_3 + 1,
	PROGRESS_CIRCLE = TENANT_4 + 1,
	SPIKE = PROGRESS_CIRCLE + 1,
	SPRITE_COUNT = SPIKE + 1
};
const int sprite_count = (int)SPRITE_ASSET_ID::SPRITE_COUNT;

enum class EFFECT_ASSET_ID
{
	COLOURED = 0,
	EGG = COLOURED + 1,
	SALMON = EGG + 1,
	TEXTURED = SALMON + 1,
	WATER = TEXTURED + 1,
	FONT = WATER + 1,
	DASH = FONT + 1,
	SMOKE = DASH + 1,
	EFFECT_COUNT = SMOKE + 1
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID
{
	SALMON = 0,
	SPRITE = SALMON + 1,
	EGG = SPRITE + 1,
	DEBUG_LINE = EGG + 1,
	SCREEN_TRIANGLE = DEBUG_LINE + 1,
	GEOMETRY_COUNT = SCREEN_TRIANGLE + 1
};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

struct SpriteSheetInfo
{
	TEXTURE_ASSET_ID texture_id;
	int rows;
	int cols;
	int sprite_width;
	int sprite_height;
};

struct RenderRequest
{
	TEXTURE_ASSET_ID used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	SPRITE_ASSET_ID used_sprite = SPRITE_ASSET_ID::SPRITE_COUNT;
	EFFECT_ASSET_ID used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
	GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
	int sprite_index = -1;
	RENDER_LAYER layer = RENDER_LAYER::DEFAULT_LAYER;
};

struct Animation
{
	std::string name;
	int frameRate;												 // fps
	SPRITE_ASSET_ID used_sprite = SPRITE_ASSET_ID::SPRITE_COUNT; // sprite sheet to grab sprites from
	std::vector<int> sprite_indices;							 // list of indices used in animation
};

struct AnimationSet
{
	std::unordered_map<std::string, Animation> animations;
	std::string current_animation;
	int current_frame = 0;
	float elapsed_time = 0.0f;
};
