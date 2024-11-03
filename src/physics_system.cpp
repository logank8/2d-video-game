// internal
#include "physics_system.hpp"
#include "world_init.hpp"

// Returns the local bounding coordinates scaled by the current size of the entity
vec2 get_bounding_box(const Motion& motion)
{
	// abs is to avoid negative scale due to the facing direction.
	return  { abs(motion.scale.x), abs(motion.scale.y) };
}

bool collides(const Motion& motion1, const Motion& motion2) {
	vec2 box1 = get_bounding_box(motion1) / 2.f;
	vec2 box2 = get_bounding_box(motion2) / 2.f;

	// Approximate a circle for each entity in the quadrant
	float radius1 = length(box1) / 2.f; 
	float radius2 = length(box2) / 2.f;

	vec2 dp = motion1.position - motion2.position;
	float distance_squared = dot(dp, dp);

	// Check if the distances between centers is less than the sum of radii
	if (distance_squared < (radius1 + radius2) * (radius1 + radius2)) {
		return true;
	}

	return false;
}
			

void PhysicsSystem::step(float elapsed_ms)
{
	// Move fish based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.
	auto& motion_registry = registry.motions;
	for(uint i = 0; i< motion_registry.size(); i++)
	{
		// !!! TODO A1: update motion.position based on step_seconds and motion.velocity
		Motion& motion = motion_registry.components[i];
		Entity entity = motion_registry.entities[i];
		float step_seconds = elapsed_ms / 1000.f;
    if (registry.players.has(entity)) {
		motion.position[0] += motion.velocity[0] * step_seconds;
		motion.position[1] += motion.velocity[1] * step_seconds;
    } else {
			//Handle contact damage enemies
			if (registry.deadlys.has(entity)) {
				if (registry.deadlys.get(entity).enemy_type != ENEMY_TYPES::PROJECTILE) {
					if ((motion.velocity.x == 0 || motion.velocity.y == 0 ) && !registry.blockedTimers.has(entity)) {
						float damage = registry.damages.get(entity).damage;

						if (registry.deadlys.get(entity).enemy_type == ENEMY_TYPES::RANGED) { // Level 3 type enemy: slow ranged enemy
							motion.velocity = { 10.f, 10.f };
						} else if (damage == 10) {	// Level 1 type enemy: slow with contact damage
							motion.velocity = { 50.f, 50.f };
						} else if (damage == 25) {	// Level 2 type enemy: fast with contact damage
							motion.velocity = { 100.f, 100.f };
						}
					}

					Motion& player_motion = registry.motions.get(registry.players.entities[0]);
					motion.angle = atan2(motion.position.y - player_motion.position.y, motion.position.x - player_motion.position.x);
					motion.position.x -= cos(motion.angle) * motion.velocity.x * step_seconds;
					motion.position.y -= sin(motion.angle) * motion.velocity.y * step_seconds;
				}
				else {
					motion.position.x -= cos(motion.angle) * motion.velocity.x * step_seconds;
					motion.position.y -= sin(motion.angle) * motion.velocity.y * step_seconds;
				}
			}
		}
	}

	for (Entity entity : registry.blockedTimers.entities) {
		// progress timer
		BlockedTimer& counter = registry.blockedTimers.get(entity);
		counter.counter_ms -= elapsed_ms;
		if (counter.counter_ms < 0){
			registry.blockedTimers.remove(entity);
		}
	}

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A2: HANDLE EGG UPDATES HERE
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 2
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	// Check for collisions between all moving entities
    ComponentContainer<Motion> &motion_container = registry.motions;
	for(uint i = 0; i<motion_container.components.size(); i++)
	{
		Motion& motion_i = motion_container.components[i];
		Entity entity_i = motion_container.entities[i];
		
		// note starting j at i+1 to compare all (i,j) pairs only once (and to not compare with itself)
		for(uint j = i+1; j<motion_container.components.size(); j++)
		{
			Motion& motion_j = motion_container.components[j];
			if (collides(motion_i, motion_j))
			{
				Entity entity_j = motion_container.entities[j];
				// Create a collisions event
				// We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
				registry.collisions.emplace_with_duplicates(entity_i, entity_j);
				registry.collisions.emplace_with_duplicates(entity_j, entity_i);
			}
		}
	}
}