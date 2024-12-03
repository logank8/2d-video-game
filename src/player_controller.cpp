#include "world_init.hpp"

#include "player_controller.hpp"
#include "animation_system.hpp"
#include "world_system.hpp"

#include <iostream>
#include <random>
#include <iomanip>
#include <sstream>

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
        auto &animSet = registry.animationSets.get(entity);

        attack.duration_ms -= elapsed_ms_since_last_update;

        if (animSet.current_frame == animSet.animations[animSet.current_animation].sprite_indices.size() - 1)
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
                if (player.slowed_duration_ms <= 0)
                {
                    pmotion.velocity = glm::normalize(player.move_direction) * pmotion.speed;
                }
                else
                {
                    pmotion.velocity = glm::normalize(player.move_direction) * pmotion.speed * player.slowed_amount;
                }
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

    // Check if player is slowed
    if (player.slowed_duration_ms > 0)
    {
        player.slowed_duration_ms -= elapsed_ms_since_last_update;
        if (player.invulnerable_duration_ms < 0.f)
        {
            player.slowed_amount = 1.f;
        }
    }

    //  Powerup magnet
    for (Entity entity : registry.powerups.entities)
    {
        Motion &motion = registry.motions.get(entity);
        if (glm::distance(motion.position, pmotion.position) < player.collection_distance)
        {
            motion.position = glm::mix(motion.position, pmotion.position, 0.1f);
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
                    Mix_PlayChannel(-1, world->exp_sound, 0);
                    animation.current_frame = 0;
                }
            }
        }

        // Remove entity when animation is finished playing
        if (collectible.is_collected && animation.current_frame == animation.animations[animation.current_animation].sprite_indices.size() - 1)
        {
            auto &collectible_experience = registry.experiences.get(entity);

            player.experience += (collectible_experience.experience * player.experience_multiplier);

            if (player.experience >= player.toNextLevel && !registry.deathTimers.has(*my_player))
            {
                player.level++;

                world->set_level_up_state(true);

                displayUpgradeCards();

                std::cout << "Player levelled up: " << player.level << std::endl;
                player.experience -= player.toNextLevel;

                // temporary increase
                player.toNextLevel += 1;
                world->update_experience_bar();
            }

            registry.remove_all_components_of(entity);
        }
    }

    // handle stamina regen
    player.curr_stamina_elapsed_ms -= elapsed_ms_since_last_update;

    if (player.curr_stamina_elapsed_ms < 0.f)
    {
        player.currentStamina = std::min(player.totalStamina, player.currentStamina + player.staminaRegen);
        player.curr_stamina_elapsed_ms = 250.f;
    }

    world->update_stamina_bar();
    world->update_experience_bar();
}

const float STAT_FONT_SIZE = 0.5f;
const float FONT_LINE_SPACE = 25.f;
const vec3 STAT_TEXT_COLOR = vec3{1.f, 1.f, 1.f};

void PlayerController::displayStatCard()
{
    auto &player = registry.players.get(*my_player);

    float x = 40;
    float y = 475;

    std::vector<std::pair<std::string, float>> attack_stats = {
        {"Attack: ", player.damage_multiplier},
        {"Attack Area: ", player.attack_size},
        {"Crit chance: ", player.crit_chance},
        {"Crit damage: ", player.crit_multiplier},
        {"Lifesteal: ", player.lifesteal}};

    std::vector<std::pair<std::string, float>> utility_stats = {
        {"Collection Range: ", player.collection_distance},
        {"Experience multiplier: ", player.experience_multiplier},
        {"Knockback: ", player.knockback_strength}};

    createText({x, y}, STAT_FONT_SIZE, "LEVEL: " + std::to_string((int)player.level), STAT_TEXT_COLOR);
    y -= (FONT_LINE_SPACE) * 2;

    for (const auto &stat : attack_stats)
    {
        std::stringstream stream;
        stream << std::fixed << std::setprecision(2) << (float)stat.second;
        std::string formattedNumber = stream.str();

        createText({x, y}, STAT_FONT_SIZE, stat.first + formattedNumber, STAT_TEXT_COLOR);
        y -= FONT_LINE_SPACE;
    }
    y -= (FONT_LINE_SPACE);

    for (const auto &stat : utility_stats)
    {
        std::stringstream stream;
        stream << std::fixed << std::setprecision(2) << (float)stat.second;
        std::string formattedNumber = stream.str();

        createText({x, y}, STAT_FONT_SIZE, stat.first + formattedNumber, STAT_TEXT_COLOR);
        y -= FONT_LINE_SPACE;
    }
}

const float CARD_PADDING_LEFT = -0.2f;
const float CARD_PADDING_TOP = 0.2f;
const float CARD_DISTANCE = 0.45f;
const int UPGRADE_CARD_COUNT = 3;
const vec2 CARD_SIZE = vec2({0.4f, -1.f});

bool operator==(const Upgrade &up1, const Upgrade &up2)
{
    return up1.id == up2.id;
}

void PlayerController::displayUpgradeCards()
{
    Entity test = createLine(vec2(0, 0), vec2(10000, 10000));
    vec3 &color = registry.colors.emplace(test);
    color = vec3(0.0f, 0.0f, 0.0f);
    displayStatCard();
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
                                        vec2(CARD_PADDING_LEFT + (i * CARD_DISTANCE), CARD_PADDING_TOP),
                                        CARD_SIZE,
                                        currCard.tier,
                                        currCard.sprite_index,
                                        currCard.title,
                                        currCard.description,
                                        currCard.onClick);
    }

    createUpgradeConfirm(renderer, vec2(0.25f, -0.55f), vec2(1.5f, 1.5f));
}

void PlayerController::on_key(int key, int action, int mod)
{
    Motion &pmotion = registry.motions.get(*my_player);
    Player &player = registry.players.get(*my_player);

    if (!registry.deathTimers.has(*my_player) && (player.is_dash_up || (!player.is_dash_up && (player.curr_dash_cooldown_ms < (player.dash_cooldown_ms - player.dash_time)))))
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

    if (key == GLFW_KEY_0 && action == GLFW_PRESS)
    {
        player.attack_size += 50.f;
    }

    if (key == GLFW_KEY_1 && action == GLFW_PRESS)
    {
        player.attack_size -= std::max(0.f, player.attack_size - 50.f);
    }
}

// button must have ui component
bool mouseOverButton(vec2 mouse_position, Entity button)
{
    if (!registry.userInterfaces.has(button))
        return false;

    auto &ui = registry.userInterfaces.get(button);
    vec2 mouse_position_ndc = WorldSystem::mousePosToNormalizedDevice(mouse_position);

    return (mouse_position_ndc.x >= ui.position.x - (ui.scale.x / 2) && mouse_position_ndc.x <= ui.position.x + (ui.scale.x / 2) &&
            mouse_position_ndc.y >= ui.position.y - (-ui.scale.y / 2) && mouse_position_ndc.y <= ui.position.y + (-ui.scale.y / 2));
}

// dont know why i need two of these and why this function doesnt work properly for the upgradecards
bool mouseOverConfirmButton(vec2 mouse_position, Entity button)
{
    if (!registry.userInterfaces.has(button))
        return false;

    auto &ui = registry.userInterfaces.get(button);
    vec2 mouse_position_ndc = WorldSystem::mousePosToNormalizedDevice(mouse_position);

    return (mouse_position_ndc.x >= ui.position.x - (ui.scale.x / 2) && mouse_position_ndc.x <= ui.position.x + (ui.scale.x / 2) &&
            mouse_position_ndc.y >= ui.position.y - (ui.scale.y / 2) && mouse_position_ndc.y <= ui.position.y + (ui.scale.y / 2));
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
            auto &upgradeCard = registry.upgradeCards.get(entity);

            if (mouseOverButton(mouse_position, entity))
            {

                upgradeCard.hovering = true;
            }
            else
            {
                upgradeCard.hovering = false;
            }
        }

        for (Entity entity : registry.upgradeConfirms.entities)
        {
            auto &ui = registry.userInterfaces.get(entity);
            auto &upgradeConfirm = registry.upgradeConfirms.get(entity);

            if (mouseOverConfirmButton(mouse_position, entity))
            {
                upgradeConfirm.hovering = true;
            }
            else
            {
                upgradeConfirm.hovering = false;
            }
        }
    }
}

void PlayerController::on_mouse_button(int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        Player &player = registry.players.get(*my_player);

        if (player.is_attacking == false && !world->is_level_up && !world->is_paused && player.currentStamina >= player.attackCost)
        {
            vec2 player_pos = registry.motions.get(*my_player).position;
            vec2 attack_direction = player.attack_direction;
            player.last_direction = attack_direction;
            player.is_attacking = true;
            player.currentStamina -= player.attackCost;
            vec2 attack_offset = vec2(player.attack_size / 2, player.attack_size / 2);
            createBasicAttackHitbox(renderer, player_pos + (attack_direction * attack_offset), *my_player);
        }
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        world->set_level_up_state(true);

        displayUpgradeCards();

        // debugging.in_debug_mode = !debugging.in_debug_mode;
    }

    if (world->is_level_up && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        for (Entity entity : registry.upgradeConfirms.entities)
        {
            auto &upgradeConfirm = registry.upgradeConfirms.get(entity);

            if (upgradeConfirm.hovering && registry.selectedCards.entities.size() > 0)
            {
                Entity selectedCardEntity = registry.selectedCards.entities[0];

                auto &upgradeCardComponent = registry.upgradeCards.get(selectedCardEntity);

                upgradeCardComponent.onClick();

                // Play upgrade sound
                Mix_PlayChannel(-1, world->level_up_sound, 0);

                for (Entity entity : registry.upgradeCards.entities)
                {
                    auto &upgradeCardComponent = registry.upgradeCards.get(entity);

                    registry.remove_all_components_of(upgradeCardComponent.icon);
                    registry.remove_all_components_of(upgradeCardComponent.name);
                    registry.remove_all_components_of(upgradeCardComponent.description);
                    registry.remove_all_components_of(entity);
                }

                for (Entity entity : registry.upgradeConfirms.entities)
                {
                    registry.remove_all_components_of(entity);
                }
                world->save_player_data(SAVE_FILENAME);
                world->set_level_up_state(false);
                break;

                return;
            }
        }

        for (Entity entity : registry.upgradeCards.entities)
        {
            auto &upgradeCardComponent = registry.upgradeCards.get(entity);
            auto &uiComponent = registry.userInterfaces.get(entity);

            if (upgradeCardComponent.hovering && !registry.selectedCards.has(entity))
            {
                // std::cout << "selected a card" << std::endl;
                // Play click button sound
                Mix_PlayChannel(-1, world->button_click_sound, 0);

                registry.selectedCards.emplace(entity);
                uiComponent.scale = upgradeCardComponent.original_scale * vec2(1.03f, 1.03f);
            }
            else
            {
                if (registry.selectedCards.has(entity) && registry.selectedCards.entities.size() > 1)
                {
                    registry.selectedCards.remove(entity);
                    uiComponent.scale = upgradeCardComponent.original_scale;
                }
            }
        }

        // ensures only one
        for (Entity entity : registry.selectedCards.entities)
        {
            auto &upgradeCardComponent = registry.upgradeCards.get(entity);
            auto &uiComponent = registry.userInterfaces.get(entity);

            if (!upgradeCardComponent.hovering && registry.selectedCards.entities.size() > 1)
            {
                registry.selectedCards.remove(entity);
                uiComponent.scale = upgradeCardComponent.original_scale;
            }
        }
    }
}