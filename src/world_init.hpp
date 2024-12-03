#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"

// These are hardcoded to the dimensions of the entity texture
// BB = bounding box
const float ENEMY_1_BB_WIDTH = 0.6f * 75.f;
const float ENEMY_1_BB_HEIGHT = 0.6f * 132.f;

const float PROJ_SIZE = 0.1f * 30.f;

const float PLAYER_BB_WIDTH = 0.7f * 90.f;
const float PLAYER_BB_HEIGHT = 0.7f * 110.f;

const float BOSS_BB_WIDTH = 4.f * 14.f;
const float BOSS_BB_HEIGHT = 4.f * 36.f;

const float EEL_BB_WIDTH = 0.6f * 80.f;  // 1001
const float EEL_BB_HEIGHT = 0.6f * 70.f; // 870

const float RANGED_BB_WIDTH = 0.6 * 130.f;
const float RANGED_BB_HEIGHT = 0.6 * 90.f;

const float PLANT_BB_HEIGHT = 72.f;
const float PLANT_BB_WIDTH = 60.f;

const float COAT_RACK_BB_HEIGHT = 3.5f * 44.f;
const float COAT_RACK_BB_WIDTH = 4.f * 16.f;

const float LONG_TABLE_BB_WIDTH = 154.f;
const float LONG_TABLE_BB_HEIGHT = 117.f;

const float CHAIR_FRONT_BB_WIDTH = 3.f * 16.f;
const float CHAIR_FRONT_BB_HEIGHT = 3.f * 27.f;

const float CHAIR_BACK_BB_WIDTH = 3.f * 16.f;
const float CHAIR_BACK_BB_HEIGHT = 3.f * 21.f;

const float CHAIR_SIDE_BB_WIDTH = 3.f * 15.f;
const float CHAIR_SIDE_BB_HEIGHT = 3.f * 28.f;

const float KITCHEN_COUNTER_1_BB_WIDTH = 3.125f * 64.f;
const float KITCHEN_COUNTER_1_BB_HEIGHT = 3.125f * 43.f;

const float KITCHEN_COUNTER_2_BB_WIDTH = 3.125f * 32.f;
const float KITCHEN_COUNTER_2_BB_HEIGHT = 3.125f * 64.f;

const float FRIDGE_BB_WIDTH = 3.636f * 26.f;
const float FRIDGE_BB_HEIGHT = 3.636f * 55.f;

const float STOVE_BB_WIDTH = 3.125f * 30.f;
const float STOVE_BB_HEIGHT = 3.125f * 43.f;

const float BOOK_CASE_BB_WIDTH = 4.3f * 46.f;
const float BOOK_CASE_BB_HEIGHT = 4.3f * 45.f;

const float COFFEE_TABLE_BB_WIDTH = 4.f * 24.f;
const float COFFEE_TABLE_BB_HEIGHT = 4.f * 19.f;

const float COUCH_BB_WIDTH = 4.f * 49.f;
const float COUCH_BB_HEIGHT = 4.f * 31.f;

const float DRESSER_BB_WIDTH = 4.f * 25.f;
const float DRESSER_BB_HEIGHT = 4.f * 35.f;

const float GRANDFATHER_CLOCK_BB_WIDTH = 4.f * 21.f;
const float GRANDFATHER_CLOCK_BB_HEIGHT = 4.f * 46.f;

const float LAMP_BB_WIDTH = 4.f * 15.f;
const float LAMP_BB_HEIGHT = 4.f * 46.f;

const float ROUND_TABLE_BB_WIDTH = 4.f * 16.f;
const float ROUND_TABLE_BB_HEIGHT = 4.f * 24.f;

const float SIDE_TABLE_BB_WIDTH = 4.f * 20.f;
const float SIDE_TABLE_BB_HEIGHT = 4.f * 23.f;

const float HPBAR_BB_WIDTH = 0.46f;
const float HPBAR_BB_HEIGHT = 0.20f;

const float TILE_PX_SIZE = 16.f;
const float BASIC_ATTACK_WIDTH = 100.f;
const float BASIC_ATTACK_HEIGHT = 100.f;

const float UPGRADE_CARD_TITLE_Y = 0.2f;

const float PLAYER_DASH_SEC = 3.f;

const std::vector<std::string> tenant_dialogue_1 = {
    "Thank you so much for saving me!",
    "Do you think you could help my friend up on the next floor?",
    "I'm sure with all the strength you've gained from fighting here, you'll have \n no problem up there. The exit's at the end of the hallway."};

const std::vector<std::string> tenant_extra_dialogue_1 = {
    "Those monsters were so scary!",
    "You're so brave!"};

const std::vector<std::string> tenant_dialogue_2 = {
    "They're all gone! Thank you so much!",
    "I think the apartment above mine is still getting torn up... do you think you \n could go up there and help?",
    "I'm honestly too spooked to go up there myself, but you don't seem afraid \n of anything!"};

const std::vector<std::string> tenant_extra_dialogue_2 = {
    "I'd make you brownies to thank you, but I think my oven is covered in \n guts right now..."};

const std::vector<std::string> tenant_dialogue_3 = {
    "Oh my! Thank you so much for getting them out of here, I don't know what I \n would've done without you.",
    "The boy living upstairs must be terrified! Could you go help him get a handle \n on things?",
    "I'm a bit frail these days, but surely someone big and strong like you won't \n have any trouble!"};

const std::vector<std::string> tenant_extra_dialogue_3 = {
    "I would've left a long time ago, but I've been living here long enough for \n rent control.",
    "I hope those demons didn't get into my medicine cabinet!"};

const std::vector<std::string> tenant_dialogue_4 = {
    "Wow, thanks for cleaning up in here. It was such a mess!",
    "The landlord lives on the floor above me. If you really want to bring \n peace to this building, you're going to have to take this fight to the top.",
    "Good luck!"};

const std::vector<std::string> tenant_extra_dialogue_4 = {
    "I don't know what the landlord is up to, but I hope it ends soon..."};

// the player
Entity createPlayer(RenderSystem *renderer, vec2 pos);

// the hp bar
Entity createHPBar(RenderSystem *renderer, vec2 pos);

// Final boss enemy
Entity createBossEnemy(RenderSystem *renderer, vec2 position);

// slow contact damage enemy
Entity createContactSlow(RenderSystem *renderer, vec2 position);

// fast contact damage enemy
Entity createContactFast(RenderSystem *renderer, vec2 position);

Entity createRangedEnemy(RenderSystem *renderer, vec2 position);

Entity createSlowingEnemy(RenderSystem *renderer, vec2 position);

Entity createDashingEnemy(RenderSystem *renderer, vec2 position);

Entity createRangedProjectile(RenderSystem *renderer, vec2 position);

Entity createRangedHomingEnemy(RenderSystem *renderer, vec2 position);

Entity createRangedHomingProjectile(RenderSystem *renderer, vec2 position);

Entity createBasicAttackHitbox(RenderSystem *renderer, vec2 position, Entity player_entity);

// a red line for debugging purposes
Entity createLine(vec2 position, vec2 size);

// line in screen space
Entity createUIBar(vec2 position, vec2 size, int index);

// a egg
Entity createEgg(vec2 pos, vec2 size);

// Text
Entity createText(vec2 pos, float scale, std::string text, glm::vec3 color);

// a wall
Entity createWalls(RenderSystem *renderer, vec2 pos, std::vector<std::vector<int>> current_map, vec2 map_pos);

// a piece of furniture
Entity createFurniture(RenderSystem *renderer, vec2 pos, int type);

// a slime patch
Entity createSlimePatch(RenderSystem *renderer, vec2 pos);

Entity createSmoke(RenderSystem *renderer, vec2 pos);

Entity createEffect(RenderSystem *renderer, vec2 pos, float lifespan_ms, EFFECT_TYPE type);

Entity createStaminaBar(RenderSystem *renderer, vec2 pos);

Entity createEmptyBar(RenderSystem *renderer, vec2 pos);

// a experience
Entity createExperience(RenderSystem *renderer, vec2 pos, int experience);

Entity createSwarm(RenderSystem *renderer, vec2 pos, float separation, float alignment, float cohesion);

Entity createSwarmMember(RenderSystem *renderer, vec2 pos, float separation, float alignment, float cohesion, int leader);

using OnClickCallback = std::function<void()>;

Entity createTempPowerup(RenderSystem *renderer, vec2 pos, PowerupType type, float multiplier, float timer);

Entity createUpgradeCard(RenderSystem *renderer, vec2 pos, vec2 size, int tier, int sprite_index, std::string title, std::string description, OnClickCallback onClick);

Entity createUpgradeIcon(RenderSystem *renderer, vec2 pos, vec2 scale, int sprite);

Entity createUpgradeConfirm(RenderSystem *renderer, vec2 pos, vec2 scale);

Entity createHealthBuff(RenderSystem *renderer, vec2 pos);

Entity createCamera(RenderSystem *renderer, vec2 pos);

Entity createDoor(RenderSystem *renderer, vec2 pos);

Entity createStartScreen(RenderSystem *renderer);

Entity createGameOverScreen(RenderSystem *renderer);

Entity createMenuScreen(RenderSystem *renderer);

void createElevatorButtons(RenderSystem *renderer, int num_levels);

Entity createLevelButton(RenderSystem *renderer, vec2 pos, int level);

Entity createExitButton(RenderSystem *renderer, vec2 pos);

Entity createDamageIndicator(RenderSystem *renderer, int damage, vec2 pos, float rng, float multiplier);

Entity createFloor(RenderSystem *renderer);

Entity createMovementKeys(RenderSystem *renderer, vec2 pos);

Entity createDashKey(RenderSystem *renderer, vec2 pos);

Entity createInteractKey(RenderSystem *renderer, vec2 pos);

Entity createAttackCursor(RenderSystem *renderer, vec2 pos);

Entity createTutorialToggleKey(RenderSystem *renderer, vec2 pos);

Entity createPauseKey(RenderSystem *renderer, vec2 pos);

Entity createTenant(RenderSystem *renderer, vec2 pos, int level);

Entity createDialogueBox(RenderSystem *renderer);

Entity createElevatorDisplay(RenderSystem *renderer, vec2 pos);

Entity createHolyArtifact(RenderSystem *renderer);

Entity createArtifactBackground(RenderSystem *renderer);

Entity createArtifactCenter(RenderSystem *renderer);