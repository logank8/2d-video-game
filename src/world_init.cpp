#include "world_init.hpp"
#include "tiny_ecs_registry.hpp"

Entity createPlayer(RenderSystem* renderer, vec2 pos)
{
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = pos;
	motion.scale = vec2({ PLAYER_BB_WIDTH, PLAYER_BB_HEIGHT });

	registry.players.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::TEXTURE_COUNT,
			SPRITE_ASSET_ID::PLAYER,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			1 // Sprite index  => 0 INDEXED (L->R, T->B)
		}
	);

	// Initialize animations
	std::vector<int> run_f_vec = {24,25,26,27,28,29};
	Animation run_f = {
			"player_run_f",
			15,
			SPRITE_ASSET_ID::PLAYER,
			run_f_vec
		};

	std::vector<int> idle_f_vec = {0,1,2,3,4,5};
	Animation idle_f = {
			"player_idle_f",
			15,
			SPRITE_ASSET_ID::PLAYER,
			idle_f_vec
		};
	std::vector<int> attack_f_vec = {36, 37, 38, 39};
	Animation attack_f = {
			"player_attack_f",
			15,
			SPRITE_ASSET_ID::PLAYER,
			attack_f_vec
		};
	std::vector<int> attack_s_vec = {42, 43, 44, 45};
	Animation attack_s = {
			"player_attack_s",
			15,
			SPRITE_ASSET_ID::PLAYER,
			attack_s_vec
		};
	std::vector<int> attack_b_vec = {48, 49, 50, 51};
	Animation attack_b = {
			"player_attack_b",
			15,
			SPRITE_ASSET_ID::PLAYER,
			attack_b_vec
		};


	auto& animSet = registry.animationSets.emplace(entity);
	animSet.animations[run_f.name] = run_f;
	animSet.animations[idle_f.name] = idle_f;
	animSet.animations[attack_f.name] = attack_f;
	animSet.animations[attack_b.name] = attack_b;
	animSet.animations[attack_s.name] = attack_s;
	animSet.current_animation=idle_f.name;


	// Add damage to player
	Damage& damage = registry.damages.emplace(entity);
	// Add health to player
	Health& health = registry.healths.emplace(entity);

	return entity;
}

Entity createHPBar(RenderSystem* renderer, vec2 pos)
{
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	// auto& motion = registry.motions.emplace(entity);
	// motion.angle = 0.f;
	// motion.velocity = { 0, 0 };
	// motion.position = pos;
	// motion.scale = vec2({ HPBAR_BB_WIDTH, HPBAR_BB_HEIGHT });

	auto& ui = registry.userInterfaces.emplace(entity);
	ui.angle = 0.f;
	ui.position = pos;
	ui.scale = vec2({ HPBAR_BB_WIDTH, -HPBAR_BB_HEIGHT });

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::HP_BAR_FULL,
			SPRITE_ASSET_ID::SPRITE_COUNT,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	return entity;
}

Entity createFish(RenderSystem* renderer, vec2 position)
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
	motion.scale = vec2({ TILE_PX_SIZE * (100 / TILE_PX_SIZE), TILE_PX_SIZE * (100 / TILE_PX_SIZE) });

	// Create an (empty) Bug component to be able to refer to all bug
	registry.deadlys.emplace(entity);
	registry.healths.emplace(entity);
	registry.damages.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::FISH,
			SPRITE_ASSET_ID::SPRITE_COUNT,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		});

	return entity;
}

Entity createEel(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 200.f, 200.f };
	motion.position = position;

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ TILE_PX_SIZE * (100 / TILE_PX_SIZE), TILE_PX_SIZE * (100 / TILE_PX_SIZE) });

	// create an empty Eel component to be able to refer to all eels
	registry.deadlys.emplace(entity);
	registry.healths.emplace(entity);
	auto& damage = registry.damages.emplace(entity);
	//TODO: adjust	 damage amounts
	damage.damage = 25.0;
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::EEL,
			SPRITE_ASSET_ID::SPRITE_COUNT,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		});

	return entity;
}

Entity createRangedEnemy(RenderSystem* renderer, vec2 position)
{
	// Reserve en entity
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 50.f, 50.f };
	motion.position = position;

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ TILE_PX_SIZE * (100 / TILE_PX_SIZE), TILE_PX_SIZE * (100 / TILE_PX_SIZE) });

	// Create an (empty) Bug component to be able to refer to all bug
	auto& enemy = registry.deadlys.emplace(entity);
	enemy.enemy_type = ENEMY_TYPES::RANGED;
	registry.healths.emplace(entity);
	registry.damages.emplace(entity);
	registry.ranged.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::RANGED_ENEMY,
			SPRITE_ASSET_ID::SPRITE_COUNT,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		});

	return entity;
}

Entity createRangedProjectile(RenderSystem* renderer, vec2 position)
{
	// Reserve en entity
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ -FISH_BB_WIDTH, FISH_BB_HEIGHT });

	// Create an (empty) Bug component to be able to refer to all bug
	auto& enemy = registry.deadlys.emplace(entity);
	enemy.enemy_type = ENEMY_TYPES::PROJECTILE;
	registry.healths.emplace(entity);
	auto& damage = registry.damages.emplace(entity);
	damage.damage = 25.f;
	registry.projectiles.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::RANGED_PROJECTILE,
			SPRITE_ASSET_ID::SPRITE_COUNT,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		});

	return entity;
}

Entity createBasicAttackHitbox(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;

	motion.scale = vec2({ BASIC_ATTACK_WIDTH, BASIC_ATTACK_HEIGHT});

	auto& damage = registry.damages.emplace(entity);
	damage.damage = 25.f;

	registry.playerAttacks.emplace(entity);

	return entity;
}

Entity createLine(vec2 position, vec2 scale)
{
	Entity entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	registry.renderRequests.insert(
		entity, {
			TEXTURE_ASSET_ID::TEXTURE_COUNT,
			SPRITE_ASSET_ID::SPRITE_COUNT,
			EFFECT_ASSET_ID::EGG,
			GEOMETRY_BUFFER_ID::DEBUG_LINE
		});

	// Create motion
	Motion& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = scale;

	registry.debugComponents.emplace(entity);
	return entity;
}

Entity createText(vec2 position, vec2 scale)
{
	Entity entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	registry.renderRequests.insert(
		entity, {
			TEXTURE_ASSET_ID::TEXTURE_COUNT,
			SPRITE_ASSET_ID::SPRITE_COUNT,
			EFFECT_ASSET_ID::EGG,
			GEOMETRY_BUFFER_ID::DEBUG_LINE
		});

	// Create motion
	Motion& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = scale;

	return entity;
}

Entity createWalls(RenderSystem* renderer, vec2 pos, bool is_side_wall)
{
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2({ TILE_PX_SIZE * (100/ TILE_PX_SIZE), TILE_PX_SIZE * (100/ TILE_PX_SIZE) });

	// create an empty component for the walls
	registry.walls.emplace(entity);
	if (is_side_wall) {
		registry.renderRequests.insert(
			entity, {
				TEXTURE_ASSET_ID::SIDE_WALL,
				SPRITE_ASSET_ID::SPRITE_COUNT,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE
			}
		);
	} else {
		registry.renderRequests.insert(
			entity, {
				TEXTURE_ASSET_ID::WALL,
				SPRITE_ASSET_ID::SPRITE_COUNT,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE
			}
		);
	}
	
	
	// Add wall to solid objects - player can't move through walls
	registry.solidObjs.emplace(entity);

	return entity;
}

Entity createGround(RenderSystem* renderer, vec2 pos, vec2 size)
{
	auto entity = Entity();
	// TODO: Add mesh for ground
	// Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	// registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
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

Entity createFurniture(RenderSystem* renderer, vec2 pos)
{
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2({ TILE_PX_SIZE * 3, TILE_PX_SIZE * 3 });

	// create an empty component for the furniture as a solid object
	registry.solidObjs.emplace(entity);
	registry.renderRequests.insert(
		entity, {
			TEXTURE_ASSET_ID::FURNITURE,
			SPRITE_ASSET_ID::SPRITE_COUNT,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	return entity;
}
