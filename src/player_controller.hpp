#pragma once
#include "common.hpp"   // Include any necessary headers
#include <GLFW/glfw3.h> // Include GLFW for key and mouse input

class PlayerController
{
public:
    PlayerController();

    void step(float elapsed_ms_since_last_update);

    // Handle input
    void on_key(int key, int action, int mods);
    void on_mouse_move(vec2 mouse_position);
    void on_mouse_button(int button, int action, int mod);

    void set_player_reference(Entity *player);
    void set_renderer(RenderSystem *renderer);

private:
    Entity *my_player; // Pointer to the player instance to control
    RenderSystem *renderer;
};