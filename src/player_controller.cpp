#include "world_init.hpp"

#include "player_controller.hpp"
#include "animation_system.hpp"
#include "world_system.hpp"

#include <iostream>
#include <random>

PlayerController::PlayerController() : my_player(nullptr) {}

// Snap direction to closest axis
vec2 snapToClosestAxis(vec2 direction)
{
    vec2 normalized = glm::normalize(direction);

    float absX = std::abs(normalized.x);
    float absY = std::abs(normalized.y);

    if (absX >= absY)
    {
        return vec2(normalized.x > 0 ? 1 : -1, 0);
    }
    else
    {
        return vec2(0, normalized.y > 0 ? 1 : -1);
    }
}

// Print player state for debugging
std::string toString(PLAYER_STATE status)
{
    switch (status)
    {
    case PLAYER_STATE::IDLE:
        return "Idle";
    case PLAYER_STATE::RUN:
        return "Running";
    case PLAYER_STATE::ATTACK:
        return "Attacking";
    case PLAYER_STATE::DASH:
        return "Dashiong";
    default:
        return "Unknown";
    }
}

void PlayerController::set_player_reference(Entity *player_ref)
{
    my_player = player_ref;
}

void PlayerController::set_renderer(RenderSystem *renderer)
{
    this->renderer = renderer;
}

void PlayerController::set_world(WorldSystem *world)
{
    this->world = world;
}

void PlayerController::step(float elapsed_ms_since_last_update)
{
    // Handle Player Attacks
    auto &attackRegistry = registry.playerAttacks;
    for (Entity entity : attackRegistry.entities)
    {
        auto &attack = attackRegistry.get(entity);

        attack.duration_ms -= elapsed_ms_since_last_update;
        if (attack.duration_ms < 0.0f || attack.has_hit)
        {
            registry.remove_all_components_of(entity);
        }
    }

    Motion &pmotion = registry.motions.get(*my_player);
    AnimationSet &animSet = registry.animationSets.get(*my_player);
    Player &player = registry.players.get(*my_player);

    if (registry.deathTimers.has(*my_player))
    {
        player.state = PLAYER_STATE::DEAD;
    }

    // handle player state switch (ew)
    switch (player.state)
    {
    case PLAYER_STATE::DEAD:
        player.is_moving = false;
        player.move_direction = vec2(0, 0);
        break;
    case PLAYER_STATE::DASH:
        break;
    case PLAYER_STATE::ATTACK:
        player.curr_attack_duration_ms -= elapsed_ms_since_last_update;

        if (player.curr_attack_duration_ms < 0.0f)
        {
            player.is_attacking = false;
            player.curr_attack_duration_ms = player.attack_duration_ms;
        }

        if (!player.is_attacking)
        {
            if (!player.is_moving)
            {
                player.state = PLAYER_STATE::IDLE;
            }
            else
            {
                player.state = PLAYER_STATE::RUN;
            }
        }
        break;
    case PLAYER_STATE::RUN:
        player.is_moving = true;
        if (player.move_direction != vec2(0, 0))
        {
            if (player.is_moving && !player.is_attacking)
            {
                player.last_direction = snapToClosestAxis(player.move_direction);
                pmotion.velocity = glm::normalize(player.move_direction) * pmotion.speed;
            }
            else
            {
                pmotion.velocity = vec2(0, 0);
                player.is_moving = false;
            }
        }
        else
        {
            pmotion.velocity = vec2(0, 0);
            player.is_moving = false;
        }

        if (player.is_attacking)
        {
            player.is_moving = false;
            player.state = PLAYER_STATE::ATTACK;
        }
        else if (player.move_direction == vec2(0, 0))
        {
            player.state = PLAYER_STATE::IDLE;
        }
        break;
    case PLAYER_STATE::IDLE:
        if (player.is_attacking)
        {
            player.state = PLAYER_STATE::ATTACK;
        }
        else if (player.move_direction != vec2(0, 0))
        {
            player.state = PLAYER_STATE::RUN;
        }
        break;
    default:
        std::cout << "player state not found" << std::endl;
        break;
    }

    // handle animations
    switch (player.state)
    {
    case PLAYER_STATE::DEAD:
        animSet.current_animation = "player_die";
        animSet.current_frame = 0;
        break;
    case PLAYER_STATE::DASH:
        break;
    case PLAYER_STATE::ATTACK:
        if (player.last_direction == vec2(0, 1))
        {
            if (animSet.current_animation != "player_attack_f")
            {
                animSet.current_frame = 0;
            }

            animSet.current_animation = "player_attack_f";
        }
        else if (player.last_direction == vec2(1, 0))
        {
            if (animSet.current_animation != "player_attack_s")
            {
                animSet.current_frame = 0;
            }

            animSet.current_animation = "player_attack_s";
        }
        else if (player.last_direction == vec2(-1, 0))
        {
            if (animSet.current_animation != "player_attack_s")
            {
                animSet.current_frame = 0;
            }

            animSet.current_animation = "player_attack_s";
        }
        else
        {
            if (animSet.current_animation != "player_attack_b")
            {
                animSet.current_frame = 0;
            }

            animSet.current_animation = "player_attack_b";
        }
        break;
    case PLAYER_STATE::RUN:
        if (player.last_direction == vec2(1, 0))
        {
            animSet.current_animation = "player_run_s";
        }
        else if (player.last_direction == vec2(-1, 0))
        {
            animSet.current_animation = "player_run_s";
        }
        else if (player.last_direction == vec2(0, -1))
        {
            animSet.current_animation = "player_run_b";
        }
        else
        {
            animSet.current_animation = "player_run_f";
        }
        break;
    case PLAYER_STATE::IDLE:
        if (player.last_direction == vec2(1, 0))
        {
            animSet.current_animation = "player_idle_s";
        }
        else if (player.last_direction == vec2(-1, 0))
        {
            animSet.current_animation = "player_idle_s";
        }
        else if (player.last_direction == vec2(0, -1))
        {
            animSet.current_animation = "player_idle_b";
        }
        else
        {
            animSet.current_animation = "player_idle_f";
        }
        break;
    default:
        std::cout << "player state not found" << std::endl;
        break;
    }

    // check direction
    if (player.last_direction.x < 0)
    {
        pmotion.scale.x = -std::abs(pmotion.scale.x);
    }
    else
    {
        pmotion.scale.x = std::abs(pmotion.scale.x);
    }

    // Check if player is invulnerable
    if (player.invulnerable)
    {
        player.invulnerable_duration_ms -= elapsed_ms_since_last_update;
        if (player.invulnerable_duration_ms < 0.f)
        {
            // std::cout << "Invuln ended" << std::endl;
            player.invulnerable = false;
            player.invulnerable_duration_ms = 1000.f;
        }
    }

    // collectibles

    for (Entity entity : registry.collectibles.entities)
    {
        if (!registry.motions.has(entity))
            continue;
        Collectible &collectible = registry.collectibles.get(entity);
        Motion &collectible_motion = registry.motions.get(entity);
        auto &animation = registry.animationSets.get(entity);

        if (glm::distance(collectible_motion.position, pmotion.position) < player.collection_distance && !collectible.is_collected)
        {
            collectible_motion.position = glm::mix(collectible_motion.position, pmotion.position, 0.1f);

            if (glm::distance(collectible_motion.position, pmotion.position) < 25.f)
            {
                collectible.is_collected = true;

                if (registry.experiences.has(entity))
                {
                    animation.current_animation = "experience_collect";
                    animation.current_frame = 0;
                }
            }
        }

        // Remove entity when animation is finished playing
        if (collectible.is_collected && animation.current_frame == animation.animations[animation.current_animation].sprite_indices.size() - 1)
        {
            auto &collectible_experience = registry.experiences.get(entity);

            player.experience += collectible_experience.experience;

            if (player.experience >= player.toNextLevel)
            {
                player.level++;

                world->set_level_up_state(true);

                displayUpgradeCards();

                std::cout << "Player levelled up: " << player.level << std::endl;
                player.experience -= player.toNextLevel;

                // temporary increase
                player.toNextLevel += 5;
            }

            registry.remove_all_components_of(entity);
        }
    }
}

const float CARD_PADDING = -0.6f;
const float CARD_DISTANCE = 0.6f;
const int UPGRADE_CARD_COUNT = 3;

bool operator==(const Upgrade &up1, const Upgrade &up2)
{
    return up1.id == up2.id;
}

void PlayerController::displayUpgradeCards()
{
    std::vector<Upgrade> availableUpgrades = {};
    static std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<int> dist(0, upgradePool.size() - 1);

    for (int i = 0; i < UPGRADE_CARD_COUNT; i++)
    {
        int cardIndex = dist(rng);

        // Ensure no duplicates (codes a little sus but should be fine)
        while (std::find(availableUpgrades.begin(), availableUpgrades.end(), upgradePool[cardIndex]) != availableUpgrades.end())
        {
            cardIndex = dist(rng);
        }

        Upgrade currCard = upgradePool[cardIndex];

        availableUpgrades.push_back(currCard);

        Entity card = createUpgradeCard(renderer,
                                        vec2(CARD_PADDING + (i * CARD_DISTANCE), 0.2f),
                                        vec2(0.1f, 0.1f),
                                        currCard.tier,
                                        TEXTURE_ASSET_ID::BEETLE,
                                        currCard.title,
                                        currCard.description,
                                        currCard.onClick);
    }
}

void PlayerController::on_key(int key, int action, int mod)
{
    Motion &pmotion = registry.motions.get(*my_player);
    Player &player = registry.players.get(*my_player);

    if (!registry.deathTimers.has(*my_player) && (player.is_dash_up || (!player.is_dash_up && (player.curr_dash_cooldown_ms < 2900.f))))
    {
        if (key == GLFW_KEY_RIGHT || key == GLFW_KEY_D)
        {
            if ((action == GLFW_PRESS || action == GLFW_REPEAT))
            {
                player.move_direction[0] = 1.f;
                player.is_moving = true;
            }
            else if (action == GLFW_RELEASE && player.move_direction[0] > 0)
            {
                player.move_direction[0] = 0.f;
            }
        }

        if (key == GLFW_KEY_LEFT || key == GLFW_KEY_A)
        {
            if ((action == GLFW_PRESS || action == GLFW_REPEAT))
            {
                player.move_direction[0] = -1.f;
                player.is_moving = true;
            }
            else if (action == GLFW_RELEASE && player.move_direction[0] < 0)
            {
                player.move_direction[0] = 0.f;
            }
        }

        if (key == GLFW_KEY_UP || key == GLFW_KEY_W)
        {
            if ((action == GLFW_PRESS || action == GLFW_REPEAT))
            {
                player.move_direction[1] = -1.f;
                player.is_moving = true;
            }
            else if (action == GLFW_RELEASE && player.move_direction[1] < 0)
            {
                player.move_direction[1] = 0.f;
            }
        }

        if (key == GLFW_KEY_DOWN || key == GLFW_KEY_S)
        {
            if ((action == GLFW_PRESS || action == GLFW_REPEAT))
            {
                player.move_direction[1] = 1.f;
                player.is_moving = true;
            }
            else if (action == GLFW_RELEASE && player.move_direction[1] > 0)
            {
                player.move_direction[1] = 0.f;
            }
        }
    }
}

vec2 mousePosToNormalizedDevice(vec2 mouse_position)
{
    float normalized_x = (2.0f * mouse_position.x) / window_width_px - 1.0f;
    float normalized_y = 1.0f - (2.0f * mouse_position.y) / window_height_px;
    return vec2(normalized_x, normalized_y);
}
void PlayerController::on_mouse_move(vec2 mouse_position)
{
    // Update player attack direction
    vec2 screen_center = vec2(window_width_px / 2.0f, window_height_px / 2.0f);
    vec2 direction = mouse_position - screen_center;
    direction = normalize(direction);

    Player &player = registry.players.get(*my_player);
    player.attack_direction = snapToClosestAxis(direction);

    if (world->is_level_up)
    {
        for (Entity entity : registry.upgradeCards.entities)
        {
            auto &ui = registry.userInterfaces.get(entity);

            vec2 mouse_position_ndc = mousePosToNormalizedDevice(mouse_position);

            // std::cout << mouse_position_ndc.x << ',' << mouse_position_ndc.y << std::endl;

            if (mouse_position_ndc.x >= ui.position.x - (ui.scale.x / 2) && mouse_position_ndc.x <= ui.position.x + (ui.scale.x / 2) &&
                mouse_position_ndc.y >= ui.position.y - (-ui.scale.y / 2) && mouse_position_ndc.y <= ui.position.y + (-ui.scale.y / 2))
            {
                if (!registry.selectedCards.has(entity))
                {
                    auto &selected = registry.selectedCards.emplace(entity);
                    auto &upgradeCard = registry.upgradeCards.get(entity);
                    auto &titleText = registry.motions.get(upgradeCard.name);
                    selected.scale = ui.scale;
                    ui.scale = selected.scale * vec2(1.02f, 1.02f);
                }
            }
            else
            {
                if (registry.selectedCards.has(entity))
                {
                    auto &selected = registry.selectedCards.get(entity);
                    ui.scale = selected.scale;

                    auto &upgradeCard = registry.upgradeCards.get(entity);
                    registry.selectedCards.remove(entity);
                }
            }
        }
    }
}

void PlayerController::on_mouse_button(int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        Player &player = registry.players.get(*my_player);

        if (player.is_attacking == false)
        {
            vec2 player_pos = registry.motions.get(*my_player).position;
            vec2 attack_direction = player.attack_direction;

            createBasicAttackHitbox(renderer, player_pos + (attack_direction * vec2(75, 75)), *my_player);
            player.last_direction = attack_direction;
            player.is_attacking = true;
        }
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        world->set_level_up_state(true);

        displayUpgradeCards();
    }

    if (world->is_level_up && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        for (Entity entity : registry.selectedCards.entities)
        {
            auto &upgradeCardComponent = registry.upgradeCards.get(entity);

            std::cout << upgradeCardComponent.name << std::endl;
            upgradeCardComponent.onClick();

            for (Entity entity : registry.upgradeCards.entities)
            {
                registry.remove_all_components_of(entity);
            }

            world->set_level_up_state(false);
            break;
        }
    }
}