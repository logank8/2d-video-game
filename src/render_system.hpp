#pragma once

#include <array>
#include <utility>
#include <unordered_map>

#include "common.hpp"
#include "components.hpp"
#include "tiny_ecs.hpp"
#include <map>
// fonts
#include <ft2build.h>
#include FT_FREETYPE_H

// System responsible for setting up OpenGL and for rendering all the
// visual entities in the game
class RenderSystem
{
	/**
	 * The following arrays store the assets the game will use. They are loaded
	 * at initialization and are assumed to not be modified by the render loop.
	 *
	 * Whenever possible, add to these lists instead of creating dynamic state
	 * it is easier to debug and faster to execute for the computer.
	 */
	std::array<GLuint, texture_count> texture_gl_handles;
	std::array<ivec2, texture_count> texture_dimensions;
	std::unordered_map<SPRITE_ASSET_ID, SpriteSheetInfo> sprite_sheets;

	// Make sure these paths remain in sync with the associated enumerators.
	// Associated id with .obj path
	const std::vector<std::pair<GEOMETRY_BUFFER_ID, std::string>> mesh_paths =
		{
			std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::SALMON, mesh_path("slime_patch.obj"))
			// specify meshes of other assets here
	};

	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, texture_count> texture_paths = {
		textures_path("Slime.png"),
		textures_path("Skeleton.png"),
		textures_path("adventurer.png"),
		textures_path("mini_golem.png"),
		textures_path("projectile.png"), // ranged projectile
		textures_path("homing_projectile.png"),
		textures_path("boss_run.png"),
		textures_path("boss_death.png"),
		textures_path("boss_attack.png"),
		textures_path("hp_bar.png"),
		textures_path("plant.png"),
		textures_path("long_table.png"),
		textures_path("Coat_Rack.png"),
		textures_path("Chair_Front.png"),
		textures_path("Chair_Back.png"),
		textures_path("Chair_Side.png"),
		textures_path("Kitchen_Counter_1.png"),
		textures_path("Kitchen_Counter_2.png"),
		textures_path("Fridge.png"),
		textures_path("Stove.png"),
		textures_path("Book_Case.png"),
		textures_path("Coffee_Table.png"),
		textures_path("Couch.png"),
		textures_path("Dresser.png"),
		textures_path("Grandfather_Clock.png"),
		textures_path("Lamp.png"),
		textures_path("Round_Table.png"),
		textures_path("Side_Table.png"),
		textures_path("walls.png"),
		textures_path("inner_wall.png"),
		textures_path("player.png"),
		textures_path("heart.png"),
		textures_path("smoke_particle.png"),
		textures_path("dash.png"),
		textures_path("stamina_regen.png"),
		textures_path("coins.png"),
		textures_path("beetle.png"),
		textures_path("powerup.png"),
		textures_path("card.png"),
		textures_path("grey_cat.png"),
		textures_path("orange_cat.png"),
		textures_path("slash.png"),
		textures_path("door.png"),
		textures_path("start_screen.png"),
		textures_path("game_over_screen.png"),
		textures_path("elevator.png"),
		textures_path("menu_level_button.png"),
		textures_path("menu_exit_button.png"),
		textures_path("floor.png"),
		textures_path("tutorial_sprites/wasd.png"),
		textures_path("tutorial_sprites/dash.png"),
		textures_path("tutorial_sprites/attack_cursor.png"),
		textures_path("tutorial_sprites/interact.png"),
		textures_path("tutorial_sprites/pause.png"),
		textures_path("upgrade_icons.png"),
		textures_path("homing_enemy.png"),
		textures_path("fast_enemy_2.png"),
		textures_path("slowing_enemy.png"),
		textures_path("dialogue_box.png"),
		textures_path("tutorial_sprites/tutorial_toggle.png"),
		textures_path("bars.png"),
		textures_path("elevator_display.png"),
		textures_path("tenant_1.png"),
		textures_path("tenant_2.png"),
		textures_path("tenant_3.png"),
		textures_path("tenant_4.png"),
		textures_path("levelup_confirm.png"),
		textures_path("title.png"), 
		textures_path("progress_circle.png"),
		textures_path("sigil_1.png")};

	std::array<GLuint, effect_count> effects;
	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, effect_count> effect_paths = {
		shader_path("coloured"),
		shader_path("egg"),
		shader_path("salmon"),
		shader_path("textured"),
		shader_path("water"),
		shader_path("font"),
		shader_path("dash"),
		shader_path("smoke")};

	std::array<GLuint, geometry_count> vertex_buffers;
	std::array<GLuint, geometry_count> index_buffers;
	std::array<Mesh, geometry_count> meshes;
	std::map<char, Character> m_ftCharacters;
	GLuint m_font_vao;
	GLuint m_font_vbo;
	GLuint vao;
	GLuint vbo;
	FT_Face face;

public:
	// Initialize the window
	bool init(GLFWwindow *window);
	bool fontInit(const std::string &font_filename, unsigned int font_default_size);

	template <class T>
	void bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices, std::vector<uint16_t> indices);

	// For sprite sheets

	void initializeSpriteSheets();

	void getUVCoordinates(SPRITE_ASSET_ID sid, int spriteIndex, float &u0, float &v0, float &u1, float &v1);

	void initializeGlTextures();

	void initializeGlEffects();

	void initializeGlMeshes();
	Mesh &getMesh(GEOMETRY_BUFFER_ID id) { return meshes[(int)id]; };

	void initializeGlGeometryBuffers();
	// Initialize the screen texture used as intermediate render target
	// The draw loop first renders to this texture, then it is used for the wind
	// shader
	bool initScreenTexture();

	// Destroy resources associated to one or all entities created by the system
	~RenderSystem();

	// Draw all entities
	void draw();

	mat3 createProjectionMatrix();
	mat3 createPlayerProjectionMatrix(vec2 position);

private:
	// Internal drawing functions for each entity type
	void drawTexturedMesh(Entity entity, const mat3 &projection);
	void drawScreenSpaceObject(Entity entity);
	void drawToScreen();
	void renderText();

	// Window handle
	GLFWwindow *window;

	// Screen texture handles
	GLuint frame_buffer;
	GLuint off_screen_render_buffer_color;
	GLuint off_screen_render_buffer_depth;

	Entity screen_state_entity;
};

bool loadEffectFromFile(
	const std::string &vs_path, const std::string &fs_path, GLuint &out_program);
