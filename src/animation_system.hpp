#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"

class AnimationSystem
{
public:
	void step(float elapsed_ms);

	AnimationSystem()
	{
	}
};