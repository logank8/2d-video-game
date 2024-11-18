#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"

// These are hardcoded to the dimensions of the entity texture
// BB = bounding box
const float ENEMY_1_BB_WIDTH = 0.6f * 75.f;
const float ENEMY_1_BB_HEIGHT = 0.6f * 132.f;

const float PROJ_SIZE = 0.6f * 30.f;

const float PLAYER_BB_WIDTH = 0.7f * 90.f;
const float PLAYER_BB_HEIGHT = 0.7f * 110.f;

const float EEL_BB_WIDTH = 0.6f * 80.f;  // 1001
const float EEL_BB_HEIGHT = 0.6f * 70.f; // 870

const float RANGED_BB_WIDTH = 0.6 * 130.f;
const float RANGED_BB_HEIGHT = 0.6 * 90.f;

const float PLANT_BB_HEIGHT = 72.f;
const float PLANT_BB_WIDTH = 60.f;

const float HPBAR_BB_WIDTH = 0.46f;
const float HPBAR_BB_HEIGHT = 0.20f;

const float TILE_PX_SIZE = 16.f;
const float BASIC_ATTACK_WIDTH = 100.f;
const float BASIC_ATTACK_HEIGHT = 100.f;

// the player
Entity createPlayer(RenderSystem *renderer, vec2 pos);

// the hp bar
Entity createHPBar(RenderSystem *renderer, vec2 pos);

// slow contact damage enemy
Entity createContactSlow(RenderSystem *renderer, vec2 position);

// fast contact damage enemy
Entity createContactFast(RenderSystem *renderer, vec2 position);

Entity createRangedEnemy(RenderSystem *renderer, vec2 position);

Entity createRangedProjectile(RenderSystem *renderer, vec2 position);

Entity createBasicAttackHitbox(RenderSystem *renderer, vec2 position, Entity player_entity);

// a red line for debugging purposes
Entity createLine(vec2 position, vec2 size);

// a egg
Entity createEgg(vec2 pos, vec2 size);

// Text
Entity createText(vec2 pos, float scale, std::string text, glm::vec3 color);

// a wall
Entity createWalls(RenderSystem *renderer, vec2 pos, bool side_wall);

// a piece of furniture
Entity createFurniture(RenderSystem *renderer, vec2 pos);

// a slime patch
Entity createSlimePatch(RenderSystem *renderer, vec2 pos);


void createSmoke(RenderSystem* renderer, vec2 pos);

Entity createEffect(RenderSystem* renderer, vec2 pos, float lifespan_ms, EFFECT_TYPE type);

Entity createStaminaBar(RenderSystem* renderer, vec2 pos);
// a experience
Entity createExperience(RenderSystem *renderer, vec2 pos, int experience);

Entity createSwarm(RenderSystem* renderer, vec2 pos, float separation, float alignment, float cohesion);

Entity createSwarmMember(RenderSystem* renderer, vec2 pos, float separation, float alignment, float cohesion, int leader);

Entity createTempPowerup(RenderSystem* renderer, vec2 pos, PowerupType type, float multiplier, float timer);