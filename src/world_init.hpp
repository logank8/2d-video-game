#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"

// These are hardcoded to the dimensions of the entity texture
// BB = bounding box
const float ENEMY_1_BB_WIDTH  = 0.6f * 75.f;
const float ENEMY_1_BB_HEIGHT = 0.6f * 132.f;

const float PROJ_SIZE = 0.6f * 30.f;

const float PLAYER_BB_WIDTH = 0.7f * 90.f;
const float PLAYER_BB_HEIGHT = 0.7f * 110.f;

const float EEL_BB_WIDTH   = 0.6f * 80.f;	// 1001
const float EEL_BB_HEIGHT  = 0.6f * 70.f;	// 870

const float RANGED_BB_WIDTH = 0.6 * 130.f;
const float RANGED_BB_HEIGHT = 0.6 * 90.f;

const float HPBAR_BB_WIDTH = 0.46f;
const float HPBAR_BB_HEIGHT = 0.20f;

const float TILE_PX_SIZE = 16.f;
const float BASIC_ATTACK_WIDTH = 150.f;
const float BASIC_ATTACK_HEIGHT = 150.f;


// the player
Entity createPlayer(RenderSystem* renderer, vec2 pos);

// the hp bar
Entity createHPBar(RenderSystem* renderer, vec2 pos);

// the prey
Entity createFish(RenderSystem* renderer, vec2 position);

// the enemy
Entity createEel(RenderSystem* renderer, vec2 position);

Entity createRangedEnemy(RenderSystem* renderer, vec2 position);

Entity createRangedProjectile(RenderSystem* renderer, vec2 position);

Entity createBasicAttackHitbox(RenderSystem* renderer, vec2 position, Entity player_entity);

// a red line for debugging purposes
Entity createLine(vec2 position, vec2 size);

// a egg
Entity createEgg(vec2 pos, vec2 size);

// Text
Entity createText(vec2 pos, float scale, std::string text, glm::vec3 color);

// a wall
Entity createWalls(RenderSystem* renderer, vec2 pos, bool side_wall);

// a piece of furniture
Entity createFurniture(RenderSystem* renderer, vec2 pos);

// a slime patch
Entity createSlimePatch(RenderSystem* renderer, vec2 pos);
