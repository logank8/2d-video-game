#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"

// These are hardcoded to the dimensions of the entity texture
// BB = bounding box
const float FISH_BB_WIDTH  = 0.5f * 105.f;
const float PLAYER_BB_WIDTH = 0.7f * 90.f;
const float PLAYER_BB_HEIGHT = 0.7f * 110.f;
const float FISH_BB_HEIGHT = 0.5f * 165.f;
const float EEL_BB_WIDTH   = 0.4f * 130.f;	// 1001
const float EEL_BB_HEIGHT  = 0.4f * 130.f;	// 870
const float RANGED_BB_WIDTH = 0.5 * 150.f;
const float RANGED_BB_HEIGHT = 0.5 * 100.f;
const float HPBAR_BB_WIDTH = 0.46f;
const float HPBAR_BB_HEIGHT = 0.20f;
const float TILE_PX_SIZE = 16.f;


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

// a red line for debugging purposes
Entity createLine(vec2 position, vec2 size);

// a egg
Entity createEgg(vec2 pos, vec2 size);

// a wall
Entity createWalls(RenderSystem* renderer, vec2 pos, bool wall_above, bool wall_right, bool wall_below, bool wall_left);

// a piece of furniture
Entity createFurniture(RenderSystem* renderer, vec2 pos);
