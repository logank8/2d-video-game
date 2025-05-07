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
	void update_boss_movement(Entity enemy, float step_seconds);

	PhysicsSystem()
	{
	}

private:
	
	// compares elliptical bounding box to rectangular bounding box
	bool static ellipse_rect_collision(float x_rad, float y_rad, vec2 circle_pos, vec2 rect_pos) {
		vec2 pos = rect_pos - circle_pos;

		// constructing elliptical formula given x and y radii:
		// (1/xrad^2)x^2 + (1/yrad^2)y^2 <= 1 confirms if point is within bounding box
		// TODO: add check for if edge intersects ellipse ? or just rename this to point intersects

		if (((pow((1/x_rad) * pos.x, 2)) + pow((1/x_rad) * pos.y, 2)) <= 1) {
			return true;
		}
		return false;
	}
};