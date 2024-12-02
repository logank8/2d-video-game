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
    int sprite_index;
    int tier;
    OnClickCallback onClick; // Lambda function for handling the click
};

class PlayerController
{
public:
    PlayerController();

    void step(float elapsed_ms_since_last_update);
    void displayUpgradeCards();
    void displayStatCard();
    void handleUpgradeInput();

    // Handle input
    void on_key(int key, int action, int mods);
    void on_mouse_move(vec2 mouse_position);
    void on_mouse_button(int button, int action, int mod);

    void set_player_reference(Entity *player);
    void set_renderer(RenderSystem *renderer);
    void set_world(WorldSystem *world);

    std::vector<Upgrade> upgradePool = {
        {0, "KNIFE", "+5% damage", 0, 1, [this]()
         {
             Player &player = registry.players.get(*my_player);
             player.damage_multiplier += 0.5;
         }},
        {1, "MAGNET", "+50 collect range", 3, 1, [this]()
         {
             Player &player = registry.players.get(*my_player);
             player.collection_distance += 50.0f;
         }},
        {2, "ROCK", "+0.5 knockback", 4, 1, [this]()
         {
             Player &player = registry.players.get(*my_player);
             player.knockback_strength += .5f;
         }},
        {3, "BOOK", "+25% exp gain", 1, 1, [this]()
         {
             Player &player = registry.players.get(*my_player);
             player.experience_multiplier += 0.25f;
         }},
        {4, "\"SALT\"", "+25% attack size", 2, 1, [this]()
         {
             Player &player = registry.players.get(*my_player);
             player.attack_size += 25.f;
         }}};

    // DEAD

    // ,
    //         {5, "BOOTS", "-0.5s dash cd", 2, 1, [this]()
    //          {
    //              Player &player = registry.players.get(*my_player);
    //              player.dash_cooldown_ms -= 500.f;
    //          }}
private:
    Entity *my_player; // Pointer to the player instance to control
    RenderSystem *renderer;
    WorldSystem *world;
};