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
	float dash_cooldown_ms = 1000.f;
	float curr_dash_cooldown_ms = dash_cooldown_ms;
	bool is_dash_up = true;

	// For attacking
	bool is_attacking = false;
	float damage_multiplier = 2.0f;
	float attack_duration_ms = 230.f;
	float curr_attack_duration_ms = attack_duration_ms;
	float knockback_strength = 0.3f;

	// Other
	float collection_distance = 100.f;
	int experience = 0;
};

enum class ENEMY_TYPES
{
	CONTACT_DMG = 0,
	CONTACT_DMG_2 = CONTACT_DMG + 1,
	RANGED = CONTACT_DMG_2 + 1,
	PROJECTILE = RANGED + 1
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
	int experience = 5;
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
	float speed = 300.0f; // To control player speed
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

// Sets the brightness of the screen
struct ScreenState
{
	float darken_screen_factor = -1;
};

struct UserInterface
{
	vec2 position = {0, 0};
	float angle = 0;
	vec2 scale = {10, 10};
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
	HP_BAR = RANGED_PROJECTILE + 1,
	HP_BAR_0 = HP_BAR + 1,
	HP_BAR_1 = HP_BAR_0 + 1,
	HP_BAR_2 = HP_BAR_1 + 1,
	HP_BAR_3 = HP_BAR_2 + 1,
	HP_BAR_4 = HP_BAR_3 + 1,
	HP_BAR_5 = HP_BAR_4 + 1,
	HP_BAR_6 = HP_BAR_5 + 1,
	HP_BAR_7 = HP_BAR_6 + 1,
	HP_BAR_FULL = HP_BAR_7 + 1,
	FURNITURE = HP_BAR_FULL + 1,
	WALL = FURNITURE + 1,
	SIDE_WALL = WALL + 1,
	PLAYERS = SIDE_WALL + 1,
	COINS = PLAYERS + 1,
	TEXTURE_COUNT = COINS + 1
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class SPRITE_ASSET_ID
{
	PLAYER = 0,
	SKELETON = PLAYER + 1,
	SLIME = SKELETON + 1,
	RANGED_ENEMY = SLIME + 1,
	COIN = RANGED_ENEMY + 1,
	SPRITE_COUNT = COIN + 1,
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
	EFFECT_COUNT = FONT + 1
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
