#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"

// These are hardcoded to the dimensions of the entity texture
// BB = bounding box
const float FISH_BB_WIDTH  = 0.6f * 165.f;
const float FISH_BB_HEIGHT = 0.6f * 165.f;
const float EEL_BB_WIDTH   = 0.6f * 300.f;	// 1001
const float EEL_BB_HEIGHT  = 0.6f * 202.f;	// 870

// the player
Entity createSalmon(RenderSystem* renderer, vec2 pos);

// the prey
Entity createFish(RenderSystem* renderer, vec2 position);

// the enemy
Entity createEel(RenderSystem* renderer, vec2 position);

// a red line for debugging purposes
Entity createLine(vec2 position, vec2 size);

// a egg
Entity createEgg(vec2 pos, vec2 size);
