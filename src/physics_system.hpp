#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"

// A simple physics system that moves rigid bodies and checks for collision
class PhysicsSystem
{
public:
	void step(float elapsed_ms, std::vector<std::vector<int>> current_map);
	bool has_los(const vec2& start, const vec2& end);
	void update_enemy_movement(Entity enemy, float step_seconds);
	void update_swarm_movement(Entity leader, float step_seconds);
	Path findDFSPath(vec2 start, vec2 target, std::vector<vec2>);

	PhysicsSystem()
	{
	}
};