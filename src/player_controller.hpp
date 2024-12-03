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
        {0, "KNIFE", "+50% damage", 0, 1, [this]()
         {
             Player &player = registry.players.get(*my_player);
             player.damage_multiplier += 0.5;
         }},
        {1, "MAGNET", "+100 collect dist", 3, 1, [this]()
         {
             Player &player = registry.players.get(*my_player);
             player.collection_distance += 100.0f;
         }},
        {2, "ROCK", "+0.5 knockback", 4, 1, [this]()
         {
             Player &player = registry.players.get(*my_player);
             player.knockback_strength += .5f;
         }},
        {3, "BOOK", "+100% exp gain", 1, 1, [this]()
         {
             Player &player = registry.players.get(*my_player);
             player.experience_multiplier += 1.0f;
         }},
        {4, "SALT", "+25% attack size", 2, 1, [this]()
         {
             Player &player = registry.players.get(*my_player);
             player.attack_size += 25.f;
         }},
        {5, "SALT", "-20% attack cost", 2, 1, [this]()
         {
             Player &player = registry.players.get(*my_player);
             player.attackCost = std::max(0.f, player.attackCost - 5.f);
         }},
        {6, "SALT", "-25% dash cost", 2, 1, [this]()
         {
             Player &player = registry.players.get(*my_player);
             player.dashCost = std::max(0.f, player.dashCost - 12.5f);
         }},
        {7, "SALT", "+1% lifesteal", 2, 1, [this]()
         {
             Player &player = registry.players.get(*my_player);
             player.lifesteal += 0.1f;
         }},
        {8, "SALT", "+20% crit chance", 2, 1, [this]()
         {
             Player &player = registry.players.get(*my_player);
             player.crit_chance = std::min(1.f, player.crit_chance + .2f);
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