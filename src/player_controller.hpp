#pragma once
#include "common.hpp"   // Include any necessary headers
#include <GLFW/glfw3.h> // Include GLFW for key and mouse input

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"

class WorldSystem;

using OnClickCallback = std::function<void()>;

struct Upgrade
{
    int id;
    std::string title;
    std::string description;
    TEXTURE_ASSET_ID texture_id;
    int tier;
    OnClickCallback onClick; // Lambda function for handling the click
};

class PlayerController
{
public:
    PlayerController();

    void step(float elapsed_ms_since_last_update);
    void displayUpgradeCards();
    void handleUpgradeInput();

    // Handle input
    void on_key(int key, int action, int mods);
    void on_mouse_move(vec2 mouse_position);
    void on_mouse_button(int button, int action, int mod);

    void set_player_reference(Entity *player);
    void set_renderer(RenderSystem *renderer);
    void set_world(WorldSystem *world);

    std::vector<Upgrade> upgradePool = {
        {0, "+1 damage", "+10 damage", TEXTURE_ASSET_ID::BEETLE, 1, [this]()
         {
             Player &player = registry.players.get(*my_player);
             player.damage_multiplier += 1.0;
         }},
        {1, "+1 collect range", "+100 collection distance", TEXTURE_ASSET_ID::BEETLE, 1, [this]()
         {
             Player &player = registry.players.get(*my_player);
             player.collection_distance += 100.0f;
         }},
        {2, "+1 knockback", "+1 knockback", TEXTURE_ASSET_ID::BEETLE, 1, [this]()
         {
             Player &player = registry.players.get(*my_player);
             player.collection_distance += 0.1f;
         }}};

private:
    Entity *my_player; // Pointer to the player instance to control
    RenderSystem *renderer;
    WorldSystem *world;
};