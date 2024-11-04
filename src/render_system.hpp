#pragma once

#include <array>
#include <utility>
#include <unordered_map>

#include "common.hpp"
#include "components.hpp"
#include "tiny_ecs.hpp"

// System responsible for setting up OpenGL and for rendering all the
// visual entities in the game
class RenderSystem {
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
	const std::vector < std::pair<GEOMETRY_BUFFER_ID, std::string>> mesh_paths =
	{
		  std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::SALMON, mesh_path("salmon.obj"))
		  // specify meshes of other assets here
	};

	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, texture_count> texture_paths = {
			textures_path("Slime.png"),
			textures_path("Skeleton.png"),
			textures_path("adventurer.png"),
			textures_path("mini_golem.png"),
			textures_path("bubble_closed.png"), // ranged projectile
			textures_path("health_bar.png"),
			textures_path("hp_bar/hp_bar_0.png"), // empty
			textures_path("hp_bar/hp_bar_1.png"), // 1
			textures_path("hp_bar/hp_bar_2.png"), // 2
			textures_path("hp_bar/hp_bar_3.png"), // 3
			textures_path("hp_bar/hp_bar_4.png"), // 4
			textures_path("hp_bar/hp_bar_5.png"), // 5
			textures_path("hp_bar/hp_bar_6.png"), // 6
			textures_path("hp_bar/hp_bar_7.png"), // 7
			textures_path("hp_bar/hp_bar_full.png"), // full
			textures_path("table.png"),
			textures_path("wall.png"),
			textures_path("side_wall.png"),
			textures_path("player.png")
			
	};

	std::array<GLuint, effect_count> effects;
	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, effect_count> effect_paths = {
		shader_path("coloured"),
		shader_path("egg"),
		shader_path("salmon"),
		shader_path("textured"),
		shader_path("water") };

	std::array<GLuint, geometry_count> vertex_buffers;
	std::array<GLuint, geometry_count> index_buffers;
	std::array<Mesh, geometry_count> meshes;

public:
	// Initialize the window
	bool init(GLFWwindow* window);

	template <class T>
	void bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices, std::vector<uint16_t> indices);

	// For sprite sheets

	void initializeSpriteSheets();

	void getUVCoordinates(SPRITE_ASSET_ID sid, int spriteIndex, float& u0, float& v0, float& u1, float& v1);

	void initializeGlTextures();

	void initializeGlEffects();

	void initializeGlMeshes();
	Mesh& getMesh(GEOMETRY_BUFFER_ID id) { return meshes[(int)id]; };

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
	void drawTexturedMesh(Entity entity, const mat3& projection);
	void drawScreenSpaceObject(Entity entity);
	void drawToScreen();

	// Window handle
	GLFWwindow* window;

	// Screen texture handles
	GLuint frame_buffer;
	GLuint off_screen_render_buffer_color;
	GLuint off_screen_render_buffer_depth;

	Entity screen_state_entity;
};

bool loadEffectFromFile(
	const std::string& vs_path, const std::string& fs_path, GLuint& out_program);
