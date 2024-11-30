#include "world_init.hpp"
#include "tiny_ecs_registry.hpp"

#include <iostream>
#include <random>

Entity createPlayer(RenderSystem *renderer, vec2 pos)
{
	auto entity = Entity();
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto &motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = {0, 0};
	motion.position = pos;
	motion.scale = vec2({PLAYER_BB_WIDTH, PLAYER_BB_HEIGHT});

	Player &player = registry.players.emplace(entity);
	player.dash_cooldown_ms = PLAYER_DASH_SEC * 1000.f;
	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::TEXTURE_COUNT,
		 SPRITE_ASSET_ID::PLAYER,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 1, // Sprite index  => 0 INDEXED (L->R, T->B)
		 RENDER_LAYER::CREATURES});

	// Initialize animations
	std::vector<int> run_s_vec = {24, 25, 26, 27, 28, 29};
	Animation run_s = {
		"player_run_s",
		15,
		SPRITE_ASSET_ID::PLAYER,
		run_s_vec};
	std::vector<int> run_f_vec = {18, 19, 20, 21, 22, 23};
	Animation run_f = {
		"player_run_f",
		15,
		SPRITE_ASSET_ID::PLAYER,
		run_f_vec};
	std::vector<int> run_b_vec = {30, 31, 32, 33, 34, 35};
	Animation run_b = {
		"player_run_b",
		15,
		SPRITE_ASSET_ID::PLAYER,
		run_b_vec};

	std::vector<int> idle_f_vec = {0, 1, 2, 3, 4, 5};
	Animation idle_f = {
		"player_idle_f",
		15,
		SPRITE_ASSET_ID::PLAYER,
		idle_f_vec};
	std::vector<int> idle_s_vec = {6, 7, 8, 9, 10, 11};
	Animation idle_s = {
		"player_idle_s",
		15,
		SPRITE_ASSET_ID::PLAYER,
		idle_s_vec};
	std::vector<int> idle_b_vec = {12, 13, 14, 15, 16, 17};
	Animation idle_b = {
		"player_idle_b",
		15,
		SPRITE_ASSET_ID::PLAYER,
		idle_b_vec};
	std::vector<int> attack_f_vec = {36, 37, 38, 39};
	Animation attack_f = {
		"player_attack_f",
		15,
		SPRITE_ASSET_ID::PLAYER,
		attack_f_vec};
	std::vector<int> attack_s_vec = {42, 43, 44, 45};
	Animation attack_s = {
		"player_attack_s",
		15,
		SPRITE_ASSET_ID::PLAYER,
		attack_s_vec};
	std::vector<int> attack_b_vec = {48, 49, 50, 51};
	Animation attack_b = {
		"player_attack_b",
		15,
		SPRITE_ASSET_ID::PLAYER,
		attack_b_vec};
	std::vector<int> die_vec = {54, 55, 56};
	Animation die = {
		"player_die",
		15,
		SPRITE_ASSET_ID::PLAYER,
		die_vec};

	auto &animSet = registry.animationSets.emplace(entity);
	animSet.animations[run_s.name] = run_s;
	animSet.animations[run_f.name] = run_f;
	animSet.animations[run_b.name] = run_b;
	animSet.animations[idle_f.name] = idle_f;
	animSet.animations[idle_s.name] = idle_s;
	animSet.animations[idle_b.name] = idle_b;
	animSet.animations[attack_f.name] = attack_f;
	animSet.animations[attack_b.name] = attack_b;
	animSet.animations[attack_s.name] = attack_s;
	animSet.animations[die.name] = die;
	animSet.current_animation = idle_f.name;

	// Add damage to player
	Damage &damage = registry.damages.emplace(entity);
	// Add health to player
	Health &health = registry.healths.emplace(entity);

	return entity;
}

Entity createHPBar(RenderSystem *renderer, vec2 pos)
{
	auto entity = Entity();
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto &ui = registry.userInterfaces.emplace(entity);
	ui.angle = 0.f;
	ui.position = pos;
	ui.scale = vec2({HPBAR_BB_WIDTH, -HPBAR_BB_HEIGHT});

	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::TEXTURE_COUNT,
		 SPRITE_ASSET_ID::HP_BAR,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 0});

	std::vector<int> full_vec = {0};
	Animation full = {
		"hpbar_8",
		15,
		SPRITE_ASSET_ID::HP_BAR,
		full_vec};

	std::vector<int> seven_vec = {1};
	Animation seven = {
		"hpbar_7",
		15,
		SPRITE_ASSET_ID::HP_BAR,
		seven_vec};

	std::vector<int> six_vec = {2};
	Animation six = {
		"hpbar_6",
		15,
		SPRITE_ASSET_ID::HP_BAR,
		six_vec};

	std::vector<int> five_vec = {3};
	Animation five = {
		"hpbar_5",
		15,
		SPRITE_ASSET_ID::HP_BAR,
		five_vec};

	std::vector<int> four_vec = {4};
	Animation four = {
		"hpbar_4",
		15,
		SPRITE_ASSET_ID::HP_BAR,
		four_vec};

	std::vector<int> three_vec = {5};
	Animation three = {
		"hpbar_3",
		15,
		SPRITE_ASSET_ID::HP_BAR,
		three_vec};

	std::vector<int> two_vec = {6};
	Animation two = {
		"hpbar_2",
		15,
		SPRITE_ASSET_ID::HP_BAR,
		two_vec};

	std::vector<int> one_vec = {7};
	Animation one = {
		"hpbar_1",
		15,
		SPRITE_ASSET_ID::HP_BAR,
		one_vec};

	std::vector<int> empty_vec = {8};
	Animation empty = {
		"hpbar_0",
		15,
		SPRITE_ASSET_ID::HP_BAR,
		empty_vec};

	auto &animSet = registry.animationSets.emplace(entity);
	animSet.animations[full.name] = full;
	animSet.animations[seven.name] = seven;
	animSet.animations[six.name] = six;
	animSet.animations[five.name] = five;
	animSet.animations[four.name] = four;
	animSet.animations[three.name] = three;
	animSet.animations[two.name] = two;
	animSet.animations[one.name] = one;
	animSet.animations[empty.name] = empty;

	return entity;
}

Entity createText(vec2 pos, float scale, std::string content, glm::vec3 color)
{
	auto entity = Entity();

	registry.renderRequests.insert(
		entity, {TEXTURE_ASSET_ID::TEXTURE_COUNT,
				 SPRITE_ASSET_ID::SPRITE_COUNT,
				 EFFECT_ASSET_ID::FONT,
				 GEOMETRY_BUFFER_ID::DEBUG_LINE});

	// Create motion
	Motion &motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = {0, 0};
	motion.position = pos;
	motion.scale = {1.f, 1.f};

	// Create text component
	Text &text = registry.texts.emplace(entity);
	text.content = content;
	text.color = color;
	text.scale = scale;

	registry.debugComponents.emplace(entity);
	return entity;
}

Entity createBossEnemy(RenderSystem *renderer, vec2 position)
{
	// Reserve en entity
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto &motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = {75.f, 75.f};
	motion.position = position;

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({BOSS_BB_WIDTH, BOSS_BB_HEIGHT});

	// Create an (empty) Bug component to be able to refer to all bug
	auto &enemy = registry.deadlys.emplace(entity);
	enemy.enemy_type = ENEMY_TYPES::FINAL_BOSS;

	auto &health = registry.healths.emplace(entity);
	health.hit_points = 2000.f;
	health.max_hp = 2000.f;

	auto &damage = registry.damages.emplace(entity);
	damage.damage = 50.f;

	registry.bosses.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::TEXTURE_COUNT,
		 SPRITE_ASSET_ID::FINAL_BOSS,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 0});

	std::vector<int> idle_f_vec = {3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17};
	Animation idle_f = {
		"final_boss_enemy_idle_f",
		12,
		SPRITE_ASSET_ID::FINAL_BOSS,
		idle_f_vec};

	std::vector<int> run_f_vec = {0, 1, 2, 3};
	Animation run_f = {
		"final_boss_enemy_run_f",
		10,
		SPRITE_ASSET_ID::FINAL_BOSS,
		run_f_vec};

	std::vector<int> die_vec = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	Animation die = {
		"final_boss_enemy_die",
		10,
		SPRITE_ASSET_ID::FINAL_BOSS_DEATH,
		die_vec};

	std::vector<int> attack_f_vec = {0, 1, 2, 3, 4, 5, 6, 7};
	Animation attack_f = {
		"final_boss_enemy_attack_f",
		10,
		SPRITE_ASSET_ID::FINAL_BOSS_ATTACK,
		attack_f_vec};

	auto &animSet = registry.animationSets.emplace(entity);
	animSet.animations[idle_f.name] = idle_f;
	animSet.animations[run_f.name] = run_f;
	animSet.animations[die.name] = die;
	animSet.animations[attack_f.name] = attack_f;
	animSet.current_animation = idle_f.name;

	return entity;
}

Entity createContactSlow(RenderSystem *renderer, vec2 position)
{
	// Reserve en entity
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto &motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = {100.f, 100.f};
	motion.position = position;

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ENEMY_1_BB_WIDTH * sign(motion.velocity.x), ENEMY_1_BB_HEIGHT});

	// Create an (empty) Bug component to be able to refer to all bug
	Deadly &deadly = registry.deadlys.emplace(entity);
	deadly.enemy_type = ENEMY_TYPES::CONTACT_DMG;
	registry.healths.emplace(entity);
	registry.damages.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::TEXTURE_COUNT,
		 SPRITE_ASSET_ID::SKELETON,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 1,
		 RENDER_LAYER::CREATURES});

	std::vector<int> idle_f_vec = {0, 1, 2, 3, 4, 5};
	Animation idle_f = {
		"slowenemy_idle_f",
		15,
		SPRITE_ASSET_ID::SKELETON,
		idle_f_vec};

	std::vector<int> run_f_vec = {18, 19, 20, 21, 22, 23};
	Animation run_f = {
		"slowenemy_run_f",
		10,
		SPRITE_ASSET_ID::SKELETON,
		run_f_vec};
	std::vector<int> die_vec = {36, 37, 38, 39, 39};
	Animation die = {
		"slowenemy_die",
		7,
		SPRITE_ASSET_ID::PLAYER,
		die_vec};

	auto &animSet = registry.animationSets.emplace(entity);
	animSet.animations[idle_f.name] = idle_f;
	animSet.animations[run_f.name] = run_f;
	animSet.animations[die.name] = die;
	animSet.current_animation = idle_f.name;

	return entity;
}

Entity createContactFast(RenderSystem *renderer, vec2 position)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto &motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = {200.f, 200.f};
	motion.position = position;

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({EEL_BB_WIDTH * sign(motion.velocity.x), EEL_BB_HEIGHT});

	// create an empty Eel component to be able to refer to all eels
	Deadly &deadly = registry.deadlys.emplace(entity);
	deadly.enemy_type = ENEMY_TYPES::CONTACT_DMG_2;
	registry.healths.emplace(entity);
	auto &damage = registry.damages.emplace(entity);
	// TODO: adjust	 damage amounts
	damage.damage = 25.0;
	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::TEXTURE_COUNT,
		 SPRITE_ASSET_ID::SLIME,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 1,
		 RENDER_LAYER::CREATURES});

	std::vector<int> idle_f_vec = {0, 1};
	Animation idle_f = {
		"fastenemy_idle_f",
		2,
		SPRITE_ASSET_ID::SLIME,
		idle_f_vec};

	std::vector<int> run_f_vec = {10, 11, 12, 13, 14};
	Animation run_f = {
		"fastenemy_run_f",
		8,
		SPRITE_ASSET_ID::SLIME,
		run_f_vec};

	std::vector<int> die_vec = {1, 2, 3, 4, 4};
	Animation die = {
		"fastenemy_die",
		7,
		SPRITE_ASSET_ID::SLIME,
		die_vec};

	auto &animSet = registry.animationSets.emplace(entity);
	animSet.animations[idle_f.name] = idle_f;
	animSet.animations[run_f.name] = run_f;
	animSet.animations[die.name] = die;
	animSet.current_animation = idle_f.name;

	return entity;
}

Entity createRangedEnemy(RenderSystem *renderer, vec2 position)
{
	// Reserve en entity
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto &motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = {50.f, 50.f};
	motion.position = position;

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({RANGED_BB_WIDTH, RANGED_BB_HEIGHT});

	// Create an (empty) Bug component to be able to refer to all bug
	auto &enemy = registry.deadlys.emplace(entity);
	enemy.enemy_type = ENEMY_TYPES::RANGED;
	registry.healths.emplace(entity);
	registry.damages.emplace(entity);
	registry.ranged.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::TEXTURE_COUNT,
		 SPRITE_ASSET_ID::RANGED_ENEMY,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 0,
		 RENDER_LAYER::CREATURES});

	std::vector<int> idle_f_vec = {0, 1, 2, 3, 4};
	Animation idle_f = {
		"rangedenemy_idle_f",
		12,
		SPRITE_ASSET_ID::RANGED_ENEMY,
		idle_f_vec};

	std::vector<int> run_f_vec = {8, 9, 10, 11, 12, 13, 14, 15};
	Animation run_f = {
		"rangedenemy_run_f",
		10,
		SPRITE_ASSET_ID::RANGED_ENEMY,
		run_f_vec};

	std::vector<int> die_vec = {32, 33, 34, 35, 36};
	Animation die = {
		"rangedenemy_die",
		7,
		SPRITE_ASSET_ID::RANGED_ENEMY,
		die_vec};

	auto &animSet = registry.animationSets.emplace(entity);
	animSet.animations[idle_f.name] = idle_f;
	animSet.animations[run_f.name] = run_f;
	animSet.animations[die.name] = die;
	animSet.current_animation = idle_f.name;

	return entity;
}

Entity createRangedProjectile(RenderSystem *renderer, vec2 position)
{
	// Reserve en entity
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto &motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = {0, 0};
	motion.position = position;

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({-PROJ_SIZE, PROJ_SIZE});

	// Create an (empty) Bug component to be able to refer to all bug
	auto &enemy = registry.deadlys.emplace(entity);
	enemy.enemy_type = ENEMY_TYPES::PROJECTILE;
	auto &damage = registry.damages.emplace(entity);
	damage.damage = 25.f;
	auto &health = registry.healths.emplace(entity);
	health.hit_points = 1.f;
	registry.projectiles.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::RANGED_PROJECTILE,
		 SPRITE_ASSET_ID::SPRITE_COUNT,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 -1,
		 RENDER_LAYER::CREATURES});

	return entity;
}

Entity createRangedHomingEnemy(RenderSystem *renderer, vec2 position)
{
	// Reserve en entity
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto &motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = {50.f, 50.f};
	motion.position = position;

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({RANGED_BB_WIDTH, RANGED_BB_HEIGHT});

	// Create an (empty) Bug component to be able to refer to all bug
	auto &enemy = registry.deadlys.emplace(entity);
	enemy.enemy_type = ENEMY_TYPES::RANGED_HOMING;
	registry.healths.emplace(entity);
	registry.damages.emplace(entity);
	registry.ranged.emplace(entity);
	auto &color = registry.colors.emplace(entity);
	color = vec3(1.f, 0, 0);
	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::TEXTURE_COUNT,
		 SPRITE_ASSET_ID::RANGED_ENEMY,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 0,
		 RENDER_LAYER::CREATURES});

	std::vector<int> idle_f_vec = {0, 1, 2, 3, 4};
	Animation idle_f = {
		"rangedenemy_idle_f",
		12,
		SPRITE_ASSET_ID::RANGED_ENEMY,
		idle_f_vec};

	std::vector<int> run_f_vec = {8, 9, 10, 11, 12, 13, 14, 15};
	Animation run_f = {
		"rangedenemy_run_f",
		10,
		SPRITE_ASSET_ID::RANGED_ENEMY,
		run_f_vec};

	std::vector<int> die_vec = {32, 33, 34, 35, 36};
	Animation die = {
		"rangedenemy_die",
		7,
		SPRITE_ASSET_ID::RANGED_ENEMY,
		die_vec};

	auto &animSet = registry.animationSets.emplace(entity);
	animSet.animations[idle_f.name] = idle_f;
	animSet.animations[run_f.name] = run_f;
	animSet.animations[die.name] = die;
	animSet.current_animation = idle_f.name;

	return entity;
}

Entity createRangedHomingProjectile(RenderSystem *renderer, vec2 position)
{
	// Reserve en entity
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto &motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = {0, 0};
	motion.position = position;

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({-PROJ_SIZE, PROJ_SIZE});

	// Create an (empty) Bug component to be able to refer to all bug
	auto &enemy = registry.deadlys.emplace(entity);
	enemy.enemy_type = ENEMY_TYPES::HOMING_PROJECTILE;
	auto &damage = registry.damages.emplace(entity);
	damage.damage = 25.f;
	auto &health = registry.healths.emplace(entity);
	health.hit_points = 1.f;
	registry.projectiles.emplace(entity);
	auto &color = registry.colors.emplace(entity);
	color = vec3(1.f, 0, 0);
	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::RANGED_PROJECTILE,
		 SPRITE_ASSET_ID::SPRITE_COUNT,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 -1,
		 RENDER_LAYER::CREATURES});

	return entity;
}

Entity createSlowingEnemy(RenderSystem *renderer, vec2 position)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto &motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = {200.f, 200.f};
	motion.position = position;

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({EEL_BB_WIDTH * sign(motion.velocity.x), EEL_BB_HEIGHT});

	// create an empty Eel component to be able to refer to all eels
	Deadly &deadly = registry.deadlys.emplace(entity);
	deadly.enemy_type = ENEMY_TYPES::SLOWING_CONTACT;
	registry.healths.emplace(entity);
	auto &damage = registry.damages.emplace(entity);
	// TODO: adjust	 damage amounts
	damage.damage = 25.0;
	auto &color = registry.colors.emplace(entity);
	color = vec3(0, 0, 50.f);
	auto &slows = registry.slows.emplace(entity);
	slows.speed_dec = 0.5;
	slows.duration = 1000.f;
	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::TEXTURE_COUNT,
		 SPRITE_ASSET_ID::SLIME,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 1,
		 RENDER_LAYER::CREATURES});

	std::vector<int> idle_f_vec = {0, 1};
	Animation idle_f = {
		"fastenemy_idle_f",
		2,
		SPRITE_ASSET_ID::SLIME,
		idle_f_vec};

	std::vector<int> run_f_vec = {10, 11, 12, 13, 14};
	Animation run_f = {
		"fastenemy_run_f",
		8,
		SPRITE_ASSET_ID::SLIME,
		run_f_vec};

	std::vector<int> die_vec = {1, 2, 3, 4, 4};
	Animation die = {
		"fastenemy_die",
		7,
		SPRITE_ASSET_ID::SLIME,
		die_vec};

	auto &animSet = registry.animationSets.emplace(entity);
	animSet.animations[idle_f.name] = idle_f;
	animSet.animations[run_f.name] = run_f;
	animSet.animations[die.name] = die;
	animSet.current_animation = idle_f.name;

	return entity;
}

Entity createDashingEnemy(RenderSystem* renderer, vec2 position)
{
	// Reserve en entity
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 100.f, 100.f };
	motion.position = position;

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ ENEMY_1_BB_WIDTH * sign(motion.velocity.x), ENEMY_1_BB_HEIGHT });

	// Create an (empty) Bug component to be able to refer to all bug
	Deadly& deadly = registry.deadlys.emplace(entity);
	deadly.enemy_type = ENEMY_TYPES::DASHING;
	registry.enemyDashes.emplace(entity);
	registry.healths.emplace(entity);
	registry.damages.emplace(entity);
	auto& color = registry.colors.emplace(entity);
	color = vec3(0, 0.3f, 0);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
		 SPRITE_ASSET_ID::SKELETON,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 1,
		 RENDER_LAYER::CREATURES });

	std::vector<int> idle_f_vec = { 0, 1, 2, 3, 4, 5 };
	Animation idle_f = {
		"slowenemy_idle_f",
		15,
		SPRITE_ASSET_ID::SKELETON,
		idle_f_vec };

	std::vector<int> run_f_vec = { 18, 19, 20, 21, 22, 23 };
	Animation run_f = {
		"slowenemy_run_f",
		10,
		SPRITE_ASSET_ID::SKELETON,
		run_f_vec };
	std::vector<int> die_vec = { 36, 37, 38, 39, 39 };
	Animation die = {
		"slowenemy_die",
		7,
		SPRITE_ASSET_ID::PLAYER,
		die_vec };

	auto& animSet = registry.animationSets.emplace(entity);
	animSet.animations[idle_f.name] = idle_f;
	animSet.animations[run_f.name] = run_f;
	animSet.animations[die.name] = die;
	animSet.current_animation = idle_f.name;

	return entity;
}

Entity createBasicAttackHitbox(RenderSystem *renderer, vec2 position, Entity player_entity)
{
	auto entity = Entity();

	auto &motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = {0, 0};
	motion.position = position;

	auto &player = registry.players.get(player_entity);

	motion.scale = vec2({player.attack_size, player.attack_size});
	float new_scale = 1.5f + ((250.f - player.attack_size) / 300.f) * (1.75f - 1.4f);
	motion.renderScale = vec2(new_scale, new_scale);
	motion.renderPositionOffset = player.attack_direction * vec2(-50, -50);

	auto &damage = registry.damages.emplace(entity);
	damage.damage = 25.f * player.damage_multiplier;

	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::TEXTURE_COUNT,
		 SPRITE_ASSET_ID::SLASH,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 0});
	std::vector<int> u_vec = {0, 1, 2, 3};
	Animation up = {
		"up_attack",
		15,
		SPRITE_ASSET_ID::SLASH,
		u_vec};
	std::vector<int> s_vec = {5, 6, 7, 8};
	Animation side = {
		"s_attack",
		15,
		SPRITE_ASSET_ID::SLASH,
		s_vec};
	std::vector<int> d_vec = {9, 10, 11, 12};
	Animation down = {
		"d_attack",
		15,
		SPRITE_ASSET_ID::SLASH,
		d_vec};

	auto &animSet = registry.animationSets.emplace(entity);
	animSet.animations[up.name] = up;
	animSet.animations[down.name] = down;
	animSet.animations[side.name] = side;

	if (player.attack_direction == vec2{0, -1})
	{
		animSet.current_animation = up.name;
	}
	else if (player.attack_direction == vec2{0, 1})
	{
		animSet.current_animation = down.name;
	}
	else
	{
		animSet.current_animation = side.name;
		motion.scale.x *= player.attack_direction.x;
	}

	registry.playerAttacks.emplace(entity);

	return entity;
}

Entity createLine(vec2 position, vec2 scale)
{
	Entity entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	registry.renderRequests.insert(
		entity, {TEXTURE_ASSET_ID::TEXTURE_COUNT,
				 SPRITE_ASSET_ID::SPRITE_COUNT,
				 EFFECT_ASSET_ID::EGG,
				 GEOMETRY_BUFFER_ID::DEBUG_LINE});

	// Create motion
	Motion &motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = {0, 0};
	motion.position = position;
	motion.scale = scale;

	registry.debugComponents.emplace(entity);
	return entity;
}

Entity createUILine(vec2 position, vec2 scale)
{
	Entity entity = Entity();

	registry.renderRequests.insert(
		entity, {TEXTURE_ASSET_ID::TEXTURE_COUNT,
				 SPRITE_ASSET_ID::SPRITE_COUNT,
				 EFFECT_ASSET_ID::COLOURED,
				 GEOMETRY_BUFFER_ID::DEBUG_LINE});

	auto &uiComponent = registry.userInterfaces.emplace(entity);
	uiComponent.position = position;
	uiComponent.scale = scale;

	registry.debugComponents.emplace(entity);

	return entity;
}

Entity createText(vec2 position, vec2 scale)
{
	Entity entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	registry.renderRequests.insert(
		entity, {TEXTURE_ASSET_ID::TEXTURE_COUNT,
				 SPRITE_ASSET_ID::SPRITE_COUNT,
				 EFFECT_ASSET_ID::EGG,
				 GEOMETRY_BUFFER_ID::DEBUG_LINE});

	// Create motion
	Motion &motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = {0, 0};
	motion.position = position;
	motion.scale = scale;

	return entity;
}

// take 4 extra inputs: cardinal directions and whether there is a wall adjacent
Entity createWalls(RenderSystem *renderer, vec2 pos, std::vector<std::vector<int>> current_map, vec2 map_pos)
{
	auto entity = Entity();
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	float sprite_rotate = 0;

	// Setting initial motion values
	Motion &motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = (M_PI / 180) * sprite_rotate;
	motion.velocity = {0.f, 0.f};
	motion.scale = vec2({100, 100});

	int sprite_idx = -1;
	TEXTURE_ASSET_ID texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;

	std::vector<std::vector<bool>> adjacent_walls = {{false, false, false}, {false, false, false}, {false, false, false}};

	int wall_count = 0;

	// check all 8 squares around and decide on sprite
	for (int i = -1; i <= 1; i++)
	{
		for (int j = -1; j <= 1; j++)
		{
			vec2 checked_pos = {map_pos.x + i, map_pos.y + j};

			if ((checked_pos.x < 0) || (checked_pos.y < 0) || (checked_pos.x >= current_map[0].size()) || checked_pos.y >= current_map.size())
			{
				adjacent_walls[i + 1][j + 1] = true;
				if (i != 0 || j != 0)
				{
					wall_count++;
				}

				continue;
			}

			if (current_map[checked_pos.y][checked_pos.x] == 0)
			{
				adjacent_walls[i + 1][j + 1] = true;
				if (i != 0 || j != 0)
				{
					wall_count++;
				}
			}
		}
	}

	// create an empty component for the walls
	registry.walls.emplace(entity);
	if (!adjacent_walls[1][2]) // painted walls
	{
		sprite_idx = 2;

		if (!adjacent_walls[1][0])
		{
			sprite_idx = 0;

			if (!adjacent_walls[0][1])
			{
				sprite_idx = 1;
			}
			else if (!adjacent_walls[2][1])
			{
				sprite_idx = 1;
				motion.scale.x = -1 * motion.scale.x;
			}
		}
		else if (!adjacent_walls[0][1] && !adjacent_walls[2][1])
		{
			sprite_idx = 5;
		}
		else if (!adjacent_walls[0][1])
		{
			if (!adjacent_walls[2][0])
			{
				sprite_idx = 14;
			}
			else
			{
				sprite_idx = 3;
			}
		}
		else if (!adjacent_walls[2][1])
		{
			if (!adjacent_walls[0][0])
			{
				sprite_idx = 14;
			}
			else
			{
				sprite_idx = 3;
			}
			motion.scale.x = -1 * motion.scale.x;
		}
		else
		{
			if (!adjacent_walls[2][0])
			{
				sprite_idx = 15;
			}
			else if (!adjacent_walls[0][0])
			{
				sprite_idx = 15;
				motion.scale.x = -1 * motion.scale.x;
			}
		}
	}
	else
	{ // inner walls, mostly blacked out
		if (adjacent_walls[1][0])
		{
			// side walls
			if (!adjacent_walls[0][1] && adjacent_walls[2][1])
			{
				if (adjacent_walls[2][2])
				{
					if (adjacent_walls[2][0])
					{
						sprite_idx = 4;
					}
					else
					{
						sprite_idx = 17;
					}
				}
				else if (adjacent_walls[2][0])
				{
					sprite_idx = 13;
				}
			}
			else if (!adjacent_walls[2][1] && adjacent_walls[0][1])
			{
				if (adjacent_walls[0][2])
				{
					if (adjacent_walls[0][0])
					{
						sprite_idx = 4;
					}
					else
					{
						sprite_idx = 17;
					}
				}
				else if (adjacent_walls[0][0])
				{
					sprite_idx = 13;
				}
				motion.scale.x = -1 * motion.scale.x;
			}
			else if (adjacent_walls[0][1] && adjacent_walls[2][0] && adjacent_walls[2][1] && adjacent_walls[2][2] && !adjacent_walls[0][2])
			{
				if (adjacent_walls[0][0])
				{
					sprite_idx = 6;
				}
				else
				{
					sprite_idx = 10;
				}
			}
			else if (adjacent_walls[2][1] && adjacent_walls[0][0] && adjacent_walls[0][1] && adjacent_walls[0][2] && !adjacent_walls[2][2])
			{
				if (adjacent_walls[2][0])
				{
					sprite_idx = 6;
				}
				else
				{
					sprite_idx = 10;
				}
				motion.scale.x = -1 * motion.scale.x;
			}
			else if (!adjacent_walls[0][1] && !adjacent_walls[2][1])
			{
				sprite_idx = 9;
			}
		}
		else
		{
			sprite_idx = 8;
			if (!adjacent_walls[0][1])
			{
				if (!adjacent_walls[2][1])
				{
					sprite_idx = 12;
				}
				else
				{
					sprite_idx = 7;
				}
			}
			else if (!adjacent_walls[2][1])
			{
				sprite_idx = 7;
				motion.scale.x = -1 * motion.scale.x;
			}
			else if (!adjacent_walls[2][2])
			{
				sprite_idx = 16;
			}
			else if (!adjacent_walls[0][2])
			{
				sprite_idx = 16;
				motion.scale.x = -1 * motion.scale.x;
			}
		}
	}

	if (wall_count == 7 && sprite_idx == -1 && texture == TEXTURE_ASSET_ID::TEXTURE_COUNT)
	{
		sprite_idx = 11;
		if (!adjacent_walls[2][0])
		{
			motion.scale.x = -1 * motion.scale.x;
		}
	}

	if (wall_count == 8 || (sprite_idx == -1 && texture == TEXTURE_ASSET_ID::TEXTURE_COUNT))
	{
		sprite_idx = -1;
		texture = TEXTURE_ASSET_ID::INNER_WALL;
	}

	registry.renderRequests.insert(
		entity, {texture,
				 SPRITE_ASSET_ID::WALL,
				 EFFECT_ASSET_ID::TEXTURED,
				 GEOMETRY_BUFFER_ID::SPRITE,
				 sprite_idx,
				 RENDER_LAYER::OBSTACLES});

	// Add wall to solid objects - player can't move through walls
	registry.solidObjs.emplace(entity);

	return entity;
}

Entity createGround(RenderSystem *renderer, vec2 pos, vec2 size)
{
	auto entity = Entity();
	// TODO: Add mesh for ground
	// Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	// registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion &motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = {0.f, 0.f};
	motion.scale = size; // Will likely change this to a constant size for all tiles

	// create an empty component for the ground tile
	registry.groundTiles.emplace(entity);
	// TODO: get sprite for the ground and complete below
	/*
	registry.renderRequests.insert(
		entity, {
			TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::EGG,
			GEOMETRY_BUFFER_ID::EGG
		});
	*/

	return entity;
}

Entity createFurniture(RenderSystem *renderer, vec2 pos, int type)
{
	auto entity = Entity();
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion &motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = {0.f, 0.f};

	TEXTURE_ASSET_ID texture;

	// Selecting asset and scale based on furniture type
	if (type == 2)
	{
		motion.scale = vec2({PLANT_BB_WIDTH, PLANT_BB_HEIGHT});
		texture = TEXTURE_ASSET_ID::PLANT;
	}
	else if (type == 9)
	{
		motion.scale = vec2({COAT_RACK_BB_WIDTH, COAT_RACK_BB_HEIGHT});
		motion.position.y += 50.f;
		texture = TEXTURE_ASSET_ID::COAT_RACK;
	}
	else if (type == 10)
	{
		motion.scale = vec2({LONG_TABLE_BB_WIDTH, LONG_TABLE_BB_HEIGHT});
		texture = TEXTURE_ASSET_ID::FURNITURE;
	}
	else if (type == 11)
	{
		motion.scale = vec2({CHAIR_FRONT_BB_WIDTH, CHAIR_FRONT_BB_HEIGHT});
		texture = TEXTURE_ASSET_ID::CHAIR_FRONT;
	}
	else if (type == 12)
	{
		motion.scale = vec2({CHAIR_BACK_BB_WIDTH, CHAIR_BACK_BB_HEIGHT});
		texture = TEXTURE_ASSET_ID::CHAIR_BACK;
		motion.position.y -= 20;
	}
	else if (type == 13)
	{
		motion.scale = vec2({CHAIR_SIDE_BB_WIDTH, CHAIR_SIDE_BB_HEIGHT});
		texture = TEXTURE_ASSET_ID::CHAIR_SIDE;
	}
	else if (type == 14)
	{
		motion.scale = vec2({-CHAIR_SIDE_BB_WIDTH, CHAIR_SIDE_BB_HEIGHT});
		texture = TEXTURE_ASSET_ID::CHAIR_SIDE;
	}
	else if (type == 15)
	{
		motion.scale = vec2({KITCHEN_COUNTER_1_BB_WIDTH, KITCHEN_COUNTER_1_BB_HEIGHT});
		motion.position.x += 50;
		motion.position.y += 17;
		texture = TEXTURE_ASSET_ID::KITCHEN_COUNTER_1;
	}
	else if (type == 16)
	{
		motion.scale = vec2({KITCHEN_COUNTER_2_BB_WIDTH, KITCHEN_COUNTER_2_BB_HEIGHT});
		motion.position.y -= 50;
		texture = TEXTURE_ASSET_ID::KITCHEN_COUNTER_2;
	}
	else if (type == 17)
	{
		motion.scale = vec2({FRIDGE_BB_WIDTH, FRIDGE_BB_HEIGHT});
		motion.position.y -= 50;
		texture = TEXTURE_ASSET_ID::FRIDGE;
	}
	else if (type == 18)
	{
		motion.scale = vec2({STOVE_BB_WIDTH, STOVE_BB_HEIGHT});
		motion.position.y += 17;
		texture = TEXTURE_ASSET_ID::STOVE;
	}
	else if (type == 19)
	{
		motion.scale = vec2({BOOK_CASE_BB_WIDTH, BOOK_CASE_BB_HEIGHT});
		motion.position.x += 50;
		motion.position.y -= 50;
		texture = TEXTURE_ASSET_ID::BOOK_CASE;
	}
	else if (type == 20)
	{
		motion.scale = vec2({COFFEE_TABLE_BB_WIDTH, COFFEE_TABLE_BB_HEIGHT});
		texture = TEXTURE_ASSET_ID::COFFEE_TABLE;
	}
	else if (type == 21)
	{
		motion.scale = vec2({COUCH_BB_WIDTH, COUCH_BB_HEIGHT});
		motion.position.x += 50;
		motion.position.y -= 50;
		texture = TEXTURE_ASSET_ID::COUCH;
	}
	else if (type == 22)
	{
		motion.scale = vec2({DRESSER_BB_WIDTH, DRESSER_BB_HEIGHT});
		motion.position.y -= 50;
		texture = TEXTURE_ASSET_ID::DRESSER;
	}
	else if (type == 23)
	{
		motion.scale = vec2({GRANDFATHER_CLOCK_BB_WIDTH, GRANDFATHER_CLOCK_BB_HEIGHT});
		motion.position.y -= 50;
		texture = TEXTURE_ASSET_ID::GRANDFATHER_CLOCK;
	}
	else if (type == 24)
	{
		motion.scale = vec2({LAMP_BB_WIDTH, LAMP_BB_HEIGHT});
		motion.position.y -= 50;
		texture = TEXTURE_ASSET_ID::LAMP;
	}
	else if (type == 25)
	{
		motion.scale = vec2({ROUND_TABLE_BB_WIDTH, ROUND_TABLE_BB_HEIGHT});
		texture = TEXTURE_ASSET_ID::ROUND_TABLE;
	}
	else if (type == 26)
	{
		motion.scale = vec2({SIDE_TABLE_BB_WIDTH, SIDE_TABLE_BB_HEIGHT});
		texture = TEXTURE_ASSET_ID::SIDE_TABLE;
	}

	// create an empty component for the furniture as a solid object
	registry.solidObjs.emplace(entity);
	registry.renderRequests.insert(
		entity, {texture,
				 SPRITE_ASSET_ID::SPRITE_COUNT,
				 EFFECT_ASSET_ID::TEXTURED,
				 GEOMETRY_BUFFER_ID::SPRITE,
				 -1,
				 RENDER_LAYER::OBSTACLES});

	return entity;
}

Entity createSlimePatch(RenderSystem *renderer, vec2 pos)
{
	auto entity = Entity();
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SALMON);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion &motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = {0.f, 0.f};
	motion.scale = mesh.original_size * 400.f;
	motion.scale.y *= -1;

	registry.stickies.emplace(entity);
	registry.renderRequests.insert(
		entity, {TEXTURE_ASSET_ID::TEXTURE_COUNT,
				 SPRITE_ASSET_ID::SPRITE_COUNT,
				 EFFECT_ASSET_ID::SALMON,
				 GEOMETRY_BUFFER_ID::SALMON,
				 -1,
				 RENDER_LAYER::FLOOR_DECOR});

	return entity;
}

Entity createExperience(RenderSystem *renderer, vec2 pos, int experience)
{
	auto entity = Entity();
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion &motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = {0.f, 0.f};
	motion.scale = vec2({TILE_PX_SIZE * (75 / TILE_PX_SIZE), TILE_PX_SIZE * (75 / TILE_PX_SIZE)});

	registry.collectibles.emplace(entity);
	auto &experienceComponent = registry.experiences.emplace(entity);
	experienceComponent.experience = experience;
	registry.renderRequests.insert(
		entity, {TEXTURE_ASSET_ID::TEXTURE_COUNT,
				 SPRITE_ASSET_ID::COIN,
				 EFFECT_ASSET_ID::TEXTURED,
				 GEOMETRY_BUFFER_ID::SPRITE,
				 1});

	std::vector<int> idle_vec = {0, 1, 2, 3, 4, 5};
	Animation idle = {
		"experience_idle",
		15,
		SPRITE_ASSET_ID::COIN,
		idle_vec};
	std::vector<int> collect_vec = {6, 7, 8, 8};
	Animation collect = {
		"experience_collect",
		10,
		SPRITE_ASSET_ID::COIN,
		collect_vec};

	auto &animSet = registry.animationSets.emplace(entity);
	animSet.animations[idle.name] = idle;
	animSet.animations[collect.name] = collect;
	animSet.current_animation = idle.name;

	return entity;
}

Entity createSmoke(RenderSystem *renderer, vec2 pos)
{
	auto entity = Entity();
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion &motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = {0.f, 0.f};
	motion.scale = vec2({10, 10});

	registry.effects.insert(entity, {0.f,
									 400.f,
									 motion.scale.x,
									 motion.scale.y,
									 EFFECT_TYPE::SMOKE});

	registry.renderRequests.insert(
		entity, {TEXTURE_ASSET_ID::SMOKE_PARTICLE,
				 SPRITE_ASSET_ID::SPRITE_COUNT,
				 EFFECT_ASSET_ID::SMOKE,
				 GEOMETRY_BUFFER_ID::SPRITE});

	return entity;
}

// plan for effects - used for particles, heart popups, whatever:
// give lifespan
// i guess probably hardcode fps relative to lifespan ? so it gets smaller until it dies
// what if we had no sprite animation and just edited scale ?
Entity createEffect(RenderSystem *renderer, vec2 pos, float lifespan_ms, EFFECT_TYPE type)
{
	auto entity = Entity();
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion &motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = {0.f, 0.f};
	motion.scale = vec2({60, 60});

	TEXTURE_ASSET_ID texture = TEXTURE_ASSET_ID::HEART;
	EFFECT_ASSET_ID effect = EFFECT_ASSET_ID::TEXTURED;

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> distrib(0, 1);

	switch (type)
	{
	case (EFFECT_TYPE::SMOKE):
		// change velocity stuff so it's in a circle around

		motion.velocity = {pow(-1, rand() % 2) * distrib(gen), (pow(-1, rand() % 2)) * (distrib(gen))};
		motion.velocity *= 6.f * distrib(gen);

		motion.scale = vec2(15, 15);
		texture = TEXTURE_ASSET_ID::SMOKE_PARTICLE;
		break;
	case EFFECT_TYPE::DASH:
		texture = TEXTURE_ASSET_ID::DASH;
		effect = EFFECT_ASSET_ID::DASH;
		motion.scale = vec2(0.85 * PLAYER_BB_WIDTH, 0.85 * PLAYER_BB_HEIGHT);
	default:
		break;
	}

	registry.effects.insert(entity, {0.f,
									 lifespan_ms,
									 motion.scale.x,
									 motion.scale.y,
									 type});
	registry.renderRequests.insert(
		entity, {texture,
				 SPRITE_ASSET_ID::SPRITE_COUNT,
				 EFFECT_ASSET_ID::TEXTURED,
				 GEOMETRY_BUFFER_ID::SPRITE,
				 -1,
				 RENDER_LAYER::EFFECTS});

	return entity;
}

Entity createStaminaBar(RenderSystem *renderer, vec2 pos)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto &ui = registry.userInterfaces.emplace(entity);
	ui.angle = 0.f;
	ui.position = pos;
	ui.scale = vec2({HPBAR_BB_WIDTH, -HPBAR_BB_HEIGHT});

	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::TEXTURE_COUNT,
		 SPRITE_ASSET_ID::STAMINA_BAR,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 14});

	// 3 states:
	// regenerating (0-14) - might need to adjust amounts for time
	// full (14)
	// dashing (14, 11, 12, 13, 0) - might need to adjust amounts for time

	std::vector<int> full_vec = {14};
	Animation full = {
		"staminabar_full",
		15,
		SPRITE_ASSET_ID::STAMINA_BAR,
		full_vec};

	std::vector<int> regen_vec = {0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 11, 12, 13, 14};
	// edit here ? idk
	Animation regen = {
		"staminabar_regen",
		int(regen_vec.size() / int(PLAYER_DASH_SEC)),
		SPRITE_ASSET_ID::STAMINA_BAR,
		regen_vec};

	std::vector<int> depleting_vec = {14, 11, 12, 13, 13, 13, 13, 13, 0};
	Animation depleting = {
		"staminabar_depleting",
		15,
		SPRITE_ASSET_ID::STAMINA_BAR,
		depleting_vec};

	auto &animSet = registry.animationSets.emplace(entity);
	animSet.animations[full.name] = full;
	animSet.animations[regen.name] = regen;
	animSet.animations[depleting.name] = depleting;
	animSet.current_animation = full.name;

	return entity;
}

Entity createExperienceBar(RenderSystem *renderer, vec2 pos)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto &ui = registry.userInterfaces.emplace(entity);
	ui.angle = 0.f;
	ui.position = pos;
	ui.scale = vec2({HPBAR_BB_WIDTH, -HPBAR_BB_HEIGHT});

	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::TEXTURE_COUNT,
		 SPRITE_ASSET_ID::STAMINA_BAR,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 0});

	return entity;
}

Entity createSwarm(RenderSystem *renderer, vec2 pos, float separation, float alignment, float cohesion)
{

	Entity leader = createSwarmMember(renderer, pos, separation, alignment, cohesion, -1);

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> distrib(-100, 100);

	for (int i = 0; i < 40; i++)
	{
		float dist_diff_x = distrib(gen);
		float dist_diff_y = distrib(gen);

		vec2 new_pos = {pos.x + dist_diff_x, pos.y + dist_diff_y};
		createSwarmMember(renderer, new_pos, separation, alignment, cohesion, leader);
	}

	return leader;
}

Entity createSwarmMember(RenderSystem *renderer, vec2 pos, float separation, float alignment, float cohesion, int leader)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	Deadly &deadly = registry.deadlys.emplace(entity);
	deadly.enemy_type = ENEMY_TYPES::SWARM;
	Health &health = registry.healths.emplace(entity);
	health.hit_points = 50.f;
	health.max_hp = 50.f;
	registry.damages.emplace(entity);

	// Setting initial motion values
	Motion &motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = {0.f, 0.f};
	motion.scale = vec2({20, 20});
	motion.speed = 150.f;

	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::TEXTURE_COUNT,
		 SPRITE_ASSET_ID::BEETLE,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 1});

	std::vector<int> idle_vec = {0, 1};
	Animation idle = {
		"swarmenemy_idle_f",
		10,
		SPRITE_ASSET_ID::BEETLE,
		idle_vec};

	auto &animSet = registry.animationSets.emplace(entity);

	animSet.animations[idle.name] = idle;
	animSet.current_animation = idle.name;

	int lead_boid = (leader == -1) ? entity : leader;

	registry.swarms.insert(entity, {lead_boid,
									separation,
									alignment,
									cohesion});

	return entity;
}

using OnClickCallback = std::function<void()>;

vec2 screenToNDC(vec2 pos)
{
	float ndc_x = (pos.x + 1.0f) / 2.0f * window_width_px;
	float ndc_y = (1.0f - (pos.y + 1.0f) / 2.0f) * window_height_px;
	return vec2(ndc_x, ndc_y);
}

Entity createUpgradeCard(RenderSystem *renderer, vec2 pos, vec2 size, int tier, TEXTURE_ASSET_ID texture_id, std::string title, std::string description, OnClickCallback onClick)
{
	auto entity = Entity();

	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto &ui = registry.userInterfaces.emplace(entity);
	ui.angle = 0.f;
	ui.position = pos;
	ui.scale = vec2({0.5, -1.25});

	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::CARD,
		 SPRITE_ASSET_ID::SPRITE_COUNT,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 1});

	auto &upgradeCardComponent = registry.upgradeCards.emplace(entity);

	upgradeCardComponent.onClick = onClick;

	vec2 screen_pos = screenToNDC(pos + (ui.scale / vec2(2.0f, 2.0f)) - vec2(ui.scale.x - 0.05f, UPGRADE_CARD_TITLE_Y));
	upgradeCardComponent.name = createText(screen_pos, 0.65f, title, vec3(0.9f, 0.9f, 0.9f));

	return entity;
}

Entity createTempPowerup(RenderSystem *renderer, vec2 pos, PowerupType type, float multiplier, float timer)
{
	auto entity = Entity();

	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	Motion &motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.velocity = {0.f, 0.f};
	motion.scale = vec2(32, 32);

	registry.eatables.emplace(entity);

	Powerup &powerup = registry.powerups.emplace(entity);

	powerup.type = type;
	powerup.timer = timer;

	if (type == PowerupType::DAMAGE_BOOST || type == PowerupType::SPEED_BOOST)
	{
		powerup.multiplier = multiplier;
	}

	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::TEXTURE_COUNT,
		 SPRITE_ASSET_ID::POWERUP,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 0});

	return entity;
}

// random color sprite cat
Entity createHealthBuff(RenderSystem *renderer, vec2 pos)
{
	auto entity = Entity();
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion &motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = {0.f, 0.f};
	motion.scale = vec2({PLANT_BB_WIDTH, PLANT_BB_HEIGHT});

	SPRITE_ASSET_ID sprite = SPRITE_ASSET_ID::GREY_CAT;
	std::string sprite_name = "";

	int type = rand() % 2;

	switch (type)
	{
	case 0:
		sprite = SPRITE_ASSET_ID::GREY_CAT;
		sprite_name = "greycat";

		break;
	default:
		sprite = SPRITE_ASSET_ID::ORANGE_CAT;
		sprite_name = "orangecat";
	}

	registry.healthBuffs.emplace(entity);

	// create an empty component for the furniture as a solid object
	registry.renderRequests.insert(
		entity, {TEXTURE_ASSET_ID::TEXTURE_COUNT,
				 sprite,
				 EFFECT_ASSET_ID::TEXTURED,
				 GEOMETRY_BUFFER_ID::SPRITE,
				 0});

	std::vector<int> idle_f_vec = {0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 16, 17, 18, 19};
	Animation idle_f = {
		sprite_name + "_idle_f",
		10,
		sprite,
		idle_f_vec};

	auto &animSet = registry.animationSets.emplace(entity);
	animSet.animations[idle_f.name] = idle_f;
	animSet.current_animation = idle_f.name;

	return entity;
}

Entity createCamera(RenderSystem *renderer, vec2 pos)
{
	auto entity = Entity();

	Camera &camera = registry.cameras.emplace(entity);

	Motion &motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.velocity = {0.f, 0.f};
	motion.scale = vec2(32, 32);

	return entity;
}

Entity createDoor(RenderSystem *renderer, vec2 pos)
{
	auto entity = Entity();

	Camera &camera = registry.cameras.emplace(entity);

	Motion &motion = registry.motions.emplace(entity);
	motion.position = {pos.x, pos.y - 25};
	motion.velocity = {0.f, 0.f};
	motion.scale = vec2(150, 150);

	registry.doors.emplace(entity);

	registry.renderRequests.insert(
		entity, {TEXTURE_ASSET_ID::DOOR,
				 SPRITE_ASSET_ID::SPRITE_COUNT,
				 EFFECT_ASSET_ID::TEXTURED,
				 GEOMETRY_BUFFER_ID::SPRITE,
				 -1,
				 RENDER_LAYER::DEFAULT_LAYER});

	return entity;
}

Entity createStartScreen(RenderSystem *renderer)
{
	auto entity = Entity();

	Motion &motion = registry.motions.emplace(entity);
	motion.position = {window_width_px / 2, window_height_px / 2};
	motion.velocity = {0.f, 0.f};
	motion.scale = vec2(0.1, 0.1);

	UserInterface &ui = registry.userInterfaces.emplace(entity);
	ui.angle = 0.f;
	ui.position = {0, 0};
	ui.scale = vec2({2.0, -2.0});

	registry.renderRequests.insert(
		entity, {TEXTURE_ASSET_ID::START_SCREEN,
				 SPRITE_ASSET_ID::SPRITE_COUNT,
				 EFFECT_ASSET_ID::TEXTURED,
				 GEOMETRY_BUFFER_ID::SPRITE,
				 -1,
				 RENDER_LAYER::DEFAULT_LAYER});

	return entity;
}

Entity createGameOverScreen(RenderSystem *renderer)
{
	auto entity = Entity();

	Motion &motion = registry.motions.emplace(entity);
	motion.position = {window_width_px / 2, window_height_px / 2};
	motion.velocity = {0.f, 0.f};
	motion.scale = vec2(0.1, 0.1);

	UserInterface &ui = registry.userInterfaces.emplace(entity);
	ui.angle = 0.f;
	ui.position = {0, 0};
	ui.scale = vec2({2.0, -2.0});

	registry.renderRequests.insert(
		entity, {TEXTURE_ASSET_ID::GAME_OVER_SCREEN,
				 SPRITE_ASSET_ID::SPRITE_COUNT,
				 EFFECT_ASSET_ID::TEXTURED,
				 GEOMETRY_BUFFER_ID::SPRITE,
				 -1,
				 RENDER_LAYER::DEFAULT_LAYER});

	return entity;
}

Entity createMenuScreen(RenderSystem *renderer)
{
	auto entity = Entity();

	UserInterface &ui = registry.userInterfaces.emplace(entity);
	ui.angle = 0.f;
	ui.position = {0, 0};
	ui.scale = vec2({2.0, -2.0});

	registry.renderRequests.insert(
		entity, {TEXTURE_ASSET_ID::MENU_SCREEN,
				 SPRITE_ASSET_ID::SPRITE_COUNT,
				 EFFECT_ASSET_ID::TEXTURED,
				 GEOMETRY_BUFFER_ID::SPRITE,
				 -1,
				 RENDER_LAYER::FLOOR});

	return entity;
}

void createElevatorButtons(RenderSystem *renderer, int num_levels)
{

	vec2 top_pos = {0.3f, 0.4f};
	vec2 top_pos_world = {window_width_px * 0.7, window_height_px * 0.3};

	for (int i = 1; i <= num_levels + 1; i++)
	{
		if (i > num_levels)
		{
			createExitButton(renderer, vec2(top_pos.x + 0.2, top_pos.y - (0.25 * i) - 0.1));
			return;
		}

		createLevelButton(renderer, vec2(top_pos.x, top_pos.y - (0.25 * i)), i);
		// figure out motion pos and pass to text
		createText({top_pos_world.x, top_pos_world.y + (90 * (num_levels - i))}, 0.9f, "Level " + std::to_string(num_levels - i + 1), vec3(0.2, 0.2, 0.2));
	}
}

Entity createLevelButton(RenderSystem *renderer, vec2 pos, int level)
{
	auto entity = Entity();

	// add to motions as well - check for collisions with mouse or something ?

	UserInterface &ui = registry.userInterfaces.emplace(entity);
	ui.angle = 0.f;
	ui.position = pos;
	ui.scale = vec2({0.09, -0.15});

	registry.renderRequests.insert(
		entity, {TEXTURE_ASSET_ID::LEVEL_BUTTON,
				 SPRITE_ASSET_ID::SPRITE_COUNT,
				 EFFECT_ASSET_ID::TEXTURED,
				 GEOMETRY_BUFFER_ID::SPRITE,
				 -1,
				 RENDER_LAYER::DEFAULT_LAYER});

	return entity;
}

Entity createExitButton(RenderSystem *renderer, vec2 pos)
{
	auto entity = Entity();

	UserInterface &ui = registry.userInterfaces.emplace(entity);
	ui.angle = 0.f;
	ui.position = pos;
	ui.scale = vec2({0.2, -0.2});

	registry.renderRequests.insert(
		entity, {TEXTURE_ASSET_ID::EXIT_BUTTON,
				 SPRITE_ASSET_ID::SPRITE_COUNT,
				 EFFECT_ASSET_ID::TEXTURED,
				 GEOMETRY_BUFFER_ID::SPRITE,
				 -1,
				 RENDER_LAYER::DEFAULT_LAYER});

	return entity;
}

Entity createDamageIndicator(RenderSystem *renderer, int damage, vec2 pos, float rng, float multiplier)
{
	auto entity = Entity();

	Motion &motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.velocity = {0.f, 0.f};
	motion.scale = vec2(32, 32);

	auto &indicator = registry.damageIndicators.emplace(entity);
	indicator.damage = damage;
	indicator.rng = rng;
	indicator.multiplier = multiplier;

	return entity;
}

Entity createFloor(RenderSystem *renderer, vec2 pos)
{
	auto entity = Entity();
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion &motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0;
	motion.velocity = {0.f, 0.f};
	motion.scale = vec2({100, 100});

	registry.renderRequests.insert(
		entity, {TEXTURE_ASSET_ID::FLOOR,
				 SPRITE_ASSET_ID::SPRITE_COUNT,
				 EFFECT_ASSET_ID::TEXTURED,
				 GEOMETRY_BUFFER_ID::SPRITE,
				 -1,
				 RENDER_LAYER::FLOOR});

	return entity;
}

Entity createMovementKeys(RenderSystem *renderer, vec2 pos)
{
	auto entity = Entity();
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion &motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0;
	motion.velocity = {0.f, 0.f};
	motion.scale = vec2({100, 100});

	registry.renderRequests.insert(
		entity, {TEXTURE_ASSET_ID::TEXTURE_COUNT,
				 SPRITE_ASSET_ID::WASD_KEYS,
				 EFFECT_ASSET_ID::TEXTURED,
				 GEOMETRY_BUFFER_ID::SPRITE,
				 0,
				 RENDER_LAYER::EFFECTS});

	registry.tutorialIcons.emplace(entity);

	std::vector<int> idle_vec = {0, 1};
	Animation idle = {
		"idle",
		2,
		SPRITE_ASSET_ID::WASD_KEYS,
		idle_vec};

	auto &animSet = registry.animationSets.emplace(entity);
	animSet.animations[idle.name] = idle;
	animSet.current_animation = idle.name;

	return entity;
}

Entity createDashKey(RenderSystem *renderer, vec2 pos)
{
	auto entity = Entity();
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion &motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0;
	motion.velocity = {0.f, 0.f};
	motion.scale = vec2({100, 50});

	registry.renderRequests.insert(
		entity, {TEXTURE_ASSET_ID::TEXTURE_COUNT,
				 SPRITE_ASSET_ID::DASH_KEYS,
				 EFFECT_ASSET_ID::TEXTURED,
				 GEOMETRY_BUFFER_ID::SPRITE,
				 0,
				 RENDER_LAYER::EFFECTS});

	registry.tutorialIcons.emplace(entity);

	std::vector<int> idle_vec = {0, 1};
	Animation idle = {
		"idle",
		2,
		SPRITE_ASSET_ID::WASD_KEYS,
		idle_vec};

	auto &animSet = registry.animationSets.emplace(entity);
	animSet.animations[idle.name] = idle;
	animSet.current_animation = idle.name;

	return entity;
}

Entity createAttackCursor(RenderSystem *renderer, vec2 pos)
{
	auto entity = Entity();
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion &motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0;
	motion.velocity = {0.f, 0.f};
	motion.scale = vec2({50, 50});

	registry.renderRequests.insert(
		entity, {TEXTURE_ASSET_ID::ATTACK_CURSOR,
				 SPRITE_ASSET_ID::SPRITE_COUNT,
				 EFFECT_ASSET_ID::TEXTURED,
				 GEOMETRY_BUFFER_ID::SPRITE,
				 -1,
				 RENDER_LAYER::EFFECTS});

	registry.tutorialIcons.emplace(entity);

	return entity;
}
