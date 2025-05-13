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
	
	// compares circular bounding box to rectangular bounding box
	bool static circle_rect_collision(float rad, Motion circle_motion, Motion rect_motion) {
		vec2 circle_pos = circle_motion.position;
		vec2 rect_pos = rect_motion.position;

		std::vector<vec2> corners = {
			{rect_pos.x + rect_motion.scale.x, rect_pos.y + rect_motion.scale.y},
			{rect_pos.x - rect_motion.scale.x, rect_pos.y + rect_motion.scale.y},
			{rect_pos.x + rect_motion.scale.x, rect_pos.y - rect_motion.scale.y},
			{rect_pos.x - rect_motion.scale.x, rect_pos.y - rect_motion.scale.y}
		};

		if ((circle_pos.x < rect_pos.x + rect_motion.scale.x) && (circle_pos.x > rect_pos.x - rect_motion.scale.x)) {
			// check if center y is between top - rad and bottom + rad
			if (circle_pos.y < rect_pos.y + rect_motion.scale.y + rad && circle_pos.y > rect_pos.y - rect_motion.scale.y - rad) {
				return true;
			}
		}
		if ((circle_pos.y < rect_pos.y + rect_motion.scale.y) && (circle_pos.y > rect_pos.y - rect_motion.scale.y)) {
			// check if center x is between left - rad and right + rad
			if (circle_pos.x < rect_pos.x + rect_motion.scale.x + rad && circle_pos.x > rect_pos.x - rect_motion.scale.x - rad) {
				return true;
			}
		}

		for (int i = 0; i < corners.size(); i++) {
			vec2 c = corners[i];

			if (pow(circle_pos.x - c.x, 2) + pow(circle_pos.y - c.y, 2) <= pow(rad, 2)) {
				return true;
			}
		}

		return false;
	}
};