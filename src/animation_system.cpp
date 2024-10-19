// internal
#include "animation_system.hpp"
#include "world_init.hpp"

#include <iostream>


void AnimationSystem::step(float elapsed_ms)
{
	auto& animation_set_registry = registry.animationSets;

    for(uint i = 0; i< animation_set_registry.size(); i++) {
        AnimationSet& animationSet = animation_set_registry.components[i];
        Entity entity = animation_set_registry.entities[i];
        float step_seconds = elapsed_ms / 1000.f;

        Animation& current_animation = animationSet.animations[animationSet.current_animation];

        animationSet.elapsed_time += step_seconds;

        float frame_duration = 1.f / current_animation.frameRate;

        if (animationSet.elapsed_time > frame_duration) {
            if (registry.renderRequests.has(entity)) {
                animationSet.current_frame = (animationSet.current_frame + 1) % current_animation.sprite_indices.size();
                registry.renderRequests.get(entity).sprite_index = current_animation.sprite_indices[animationSet.current_frame];
            }

            animationSet.elapsed_time = 0.0f;
        }


    }
}