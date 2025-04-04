
#define GL3W_IMPLEMENTATION
#include <gl3w.h>

// stlib
#include <chrono>

// internal
#include "physics_system.hpp"
#include "render_system.hpp"
#include "world_system.hpp"
#include "animation_system.hpp"
#include "damage_indicator_system.hpp"

using Clock = std::chrono::high_resolution_clock;

// Entry point
int main()
{
	// Global systems
	WorldSystem world;
	RenderSystem renderer;
	PhysicsSystem physics;
	AnimationSystem animations;
	DamageIndicatorSystem damages;

	// Initializing window
	GLFWwindow *window = world.create_window();
	if (!window)
	{
		// Time to read the error message
		printf("Press any key to exit");
		getchar();
		return EXIT_FAILURE;
	}

	// initialize the main systems
	renderer.init(window);
	world.init(&renderer);

	// variable timestep loop
	auto t = Clock::now();
	while (!world.is_over())
	{
		// Processes system messages, if this wasn't present the window would become unresponsive
		glfwPollEvents();

		// Calculating elapsed times in milliseconds from the previous iteration
		auto now = Clock::now();
		float elapsed_ms =
			(float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
		t = now;

		ScreenState &screen = registry.screenStates.components[0];

		if (!world.is_level_up && (screen.state != GameState::PAUSED))
		{
			world.step(elapsed_ms);
			if (screen.state == GameState::GAME)
			{
				physics.step(elapsed_ms, world.get_current_map());
				
			}
			animations.step(elapsed_ms);
			if (screen.state == GameState::GAME) {
				damages.step(elapsed_ms);
			}
			
		}
		else
		{
			// Darken screen if game is paused
			if (WorldSystem::is_paused)
			{
				screen.darken_screen_factor = 0.9;
			}
			else
			{
				screen.darken_screen_factor = 0;
			}
		}
		if (screen.state == GAME)
		{
			world.handle_collisions(elapsed_ms);
		}

		renderer.draw();
	}

	return EXIT_SUCCESS;
}
