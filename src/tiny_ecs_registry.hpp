#pragma once
#include <vector>

#include "tiny_ecs.hpp"
#include "components.hpp"

class ECSRegistry
{
	// Callbacks to remove a particular or all entities in the system
	std::vector<ContainerInterface *> registry_list;

public:
	// Manually created list of all components this game has
	// TODO: A1 add a LightUp component
	ComponentContainer<DeathTimer> deathTimers;
	ComponentContainer<BlockedTimer> blockedTimers;
	ComponentContainer<AttackTimer> attackTimers;
	ComponentContainer<Motion> motions;
	ComponentContainer<Collision> collisions;
	ComponentContainer<Player> players;
	ComponentContainer<Mesh *> meshPtrs;
	ComponentContainer<RenderRequest> renderRequests;
	ComponentContainer<ScreenState> screenStates;
	ComponentContainer<Eatable> eatables;
	ComponentContainer<Deadly> deadlys;
	ComponentContainer<FinalBoss> bosses;
	ComponentContainer<DebugComponent> debugComponents;
	ComponentContainer<vec3> colors;
	ComponentContainer<Health> healths;
	ComponentContainer<Damage> damages;
	ComponentContainer<Wall> walls;
	ComponentContainer<Ground> groundTiles;
	ComponentContainer<Sticky> stickies;
	ComponentContainer<Solid> solidObjs;
	ComponentContainer<Slows> slows;
	ComponentContainer<Ranged> ranged;
	ComponentContainer<Projectile> projectiles;
	ComponentContainer<Dash> dashing;
	ComponentContainer<UserInterface> userInterfaces;
	ComponentContainer<AnimationSet> animationSets;
	ComponentContainer<Path> paths;
	ComponentContainer<PathTimer> pathTimers;
	ComponentContainer<Text> texts;
	ComponentContainer<PlayerAttack> playerAttacks;
	ComponentContainer<LightUp> lightUps;
	ComponentContainer<Experience> experiences;
	ComponentContainer<Effect> effects;
	ComponentContainer<Collectible> collectibles;
	ComponentContainer<SwarmMember> swarms;
	ComponentContainer<Powerup> powerups;
	ComponentContainer<UpgradeCard> upgradeCards;
	ComponentContainer<SelectedCard> selectedCards;
	ComponentContainer<UpgradeConfirm> upgradeConfirms;
	ComponentContainer<HealthBuff> healthBuffs;
	ComponentContainer<Camera> cameras;
	ComponentContainer<Door> doors;
	ComponentContainer<DamageIndicator> damageIndicators;
	ComponentContainer<TutorialIcon> tutorialIcons;
	ComponentContainer<EnemyDash> enemyDashes;
	ComponentContainer<ElevatorButton> elevatorButtons;
	ComponentContainer<Tenant> tenants;
	ComponentContainer<DialogueBox> dialogueBoxes;
	ComponentContainer<ElevatorDisplay> elevatorDisplays;
	ComponentContainer<KillTracker> killTrackers;

	// constructor that adds all containers for looping over them
	// IMPORTANT: Don't forget to add any newly added containers!
	ECSRegistry()
	{
		// TODO: A1 add a LightUp component
		registry_list.push_back(&deathTimers);
		registry_list.push_back(&blockedTimers);
		registry_list.push_back(&attackTimers);
		registry_list.push_back(&motions);
		registry_list.push_back(&collisions);
		registry_list.push_back(&players);
		registry_list.push_back(&meshPtrs);
		registry_list.push_back(&renderRequests);
		registry_list.push_back(&screenStates);
		registry_list.push_back(&eatables);
		registry_list.push_back(&deadlys);
		registry_list.push_back(&bosses);
		registry_list.push_back(&debugComponents);
		registry_list.push_back(&colors);
		registry_list.push_back(&healths);
		registry_list.push_back(&damages);
		registry_list.push_back(&walls);
		registry_list.push_back(&groundTiles);
		registry_list.push_back(&stickies);
		registry_list.push_back(&solidObjs);
		registry_list.push_back(&slows);
		registry_list.push_back(&ranged);
		registry_list.push_back(&projectiles);
		registry_list.push_back(&dashing);
		registry_list.push_back(&userInterfaces);
		registry_list.push_back(&animationSets);
		registry_list.push_back(&paths);
		registry_list.push_back(&pathTimers);
		registry_list.push_back(&texts);
		registry_list.push_back(&playerAttacks);
		registry_list.push_back(&lightUps);
		registry_list.push_back(&effects);
		registry_list.push_back(&collectibles);
		registry_list.push_back(&experiences);
		registry_list.push_back(&swarms);
		registry_list.push_back(&powerups);
		registry_list.push_back(&upgradeCards);
		registry_list.push_back(&selectedCards);
		registry_list.push_back(&upgradeConfirms);
		registry_list.push_back(&healthBuffs);
		registry_list.push_back(&cameras);
		registry_list.push_back(&doors);
		registry_list.push_back(&damageIndicators);
		registry_list.push_back(&tutorialIcons);
		registry_list.push_back(&enemyDashes);
		registry_list.push_back(&elevatorButtons);
		registry_list.push_back(&tenants);
		registry_list.push_back(&dialogueBoxes);
		registry_list.push_back(&elevatorDisplays);
		registry_list.push_back(&killTrackers);
	}

	void clear_all_components()
	{
		for (ContainerInterface *reg : registry_list)
			reg->clear();
	}

	void list_all_components()
	{
		printf("Debug info on all registry entries:\n");
		for (ContainerInterface *reg : registry_list)
			if (reg->size() > 0)
				printf("%4d components of type %s\n", (int)reg->size(), typeid(*reg).name());
	}

	void list_all_components_of(Entity e)
	{
		printf("Debug info on components of entity %u:\n", (unsigned int)e);
		for (ContainerInterface *reg : registry_list)
			if (reg->has(e))
				printf("type %s\n", typeid(*reg).name());
	}

	void remove_all_components_of(Entity e)
	{
		for (ContainerInterface *reg : registry_list)
			reg->remove(e);
	}
};

extern ECSRegistry registry;