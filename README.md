
## ADDING SPRITE SHEETS
1. Add spritesheet as a basic texture
- update texture_paths (render_system.hpp)
- update TEXTURE_ASSET_ID enum (components.hpp)
2. Setup sprite_sheet data (render_system_init.cpp => initializeSpriteSheets)
- create mapping => texture_id, #rows, #cols, sprite_width, sprite_height
e.g. sprite_sheets[SPRITE_ASSET_ID::PLAYER] = {TEXTURE_ASSET_ID::PLAYERS, 10, 6, 64, 64};
3. Update SPRITE_ASSET_ID enum (components.hpp)
4. Use in renderRequest (world_init.cpp ??)

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
    - if using spriteSheet
        - set TEXTURE_ASSET_ID to TEXTURE_COUNT
        - specify SPRITE_ASSET_ID
        - specify INDEX
    - otherwise:
        - sprite index can be left out (defaults to -1)
        - set SPRITE_ASSET_ID to SPRITE_COUNT


Credits:
- Penzilla (https://penzilla.itch.io/top-down-retro-interior) - Various furniture sprites

