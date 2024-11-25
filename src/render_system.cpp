// internal
#include "render_system.hpp"
#include <SDL.h>

#include <iostream>

#include "tiny_ecs_registry.hpp"
#include <glm/gtc/type_ptr.hpp>

void RenderSystem::drawTexturedMesh(Entity entity,
									const mat3 &projection)
{
	Motion &motion = registry.motions.get(entity);
	// Transformation code, see Rendering and Transformation in the template
	// specification for more info Incrementally updates transformation matrix,
	// thus ORDER IS IMPORTANT
	Transform transform;
	transform.translate(motion.position + motion.renderPositionOffset);
	transform.scale(motion.scale * motion.renderScale);
	transform.rotate(motion.angle);

	// adjusting for discrepancies in texture vs. bb size
	if (registry.players.has(entity))
	{
		transform.scale(vec2(2.6f, 2.f));
	}

	if (registry.deadlys.has(entity))
	{
		Deadly &enemy = registry.deadlys.get(entity);
		if (enemy.enemy_type == ENEMY_TYPES::CONTACT_DMG)
		{
			transform.scale(vec2(2.5f, 1.6f));
		}
		else if (enemy.enemy_type == ENEMY_TYPES::CONTACT_DMG_2 || enemy.enemy_type == ENEMY_TYPES::SLOWING_CONTACT)
		{
			transform.scale(vec2(2.4f, 2.2f));
		}
		else if (enemy.enemy_type == ENEMY_TYPES::RANGED || enemy.enemy_type == ENEMY_TYPES::RANGED_HOMING)
		{
			transform.scale(vec2(1.5, 2.7));
		}
		else if (registry.projectiles.has(entity))
		{
			transform.scale(vec2(5, 5));
		}
		else if (enemy.enemy_type == ENEMY_TYPES::FINAL_BOSS)
		{
			if (enemy.state == ENEMY_STATE::DEAD)
			{
				transform.scale(vec2(1.3, 1));
			}
			else if (enemy.state == ENEMY_STATE::ATTACK)
			{
				transform.scale(vec2(4, 1));
			}
			else
			{
				transform.scale(vec2(1.5, 1.5));
			}
		}
	}

	if (registry.healthBuffs.has(entity))
	{
		transform.scale(vec2(2, 2));
	}

	assert(registry.renderRequests.has(entity));
	const RenderRequest &render_request = registry.renderRequests.get(entity);

	const GLuint used_effect_enum = (GLuint)render_request.used_effect;
	assert(used_effect_enum != (GLuint)EFFECT_ASSET_ID::EFFECT_COUNT);
	const GLuint program = (GLuint)effects[used_effect_enum];

	// Setting shaders
	glUseProgram(program);
	gl_has_errors();

	assert(render_request.used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT);
	const GLuint vbo = vertex_buffers[(GLuint)render_request.used_geometry];
	const GLuint ibo = index_buffers[(GLuint)render_request.used_geometry];

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	// Input data location as in the vertex buffer
	if (render_request.used_effect == EFFECT_ASSET_ID::TEXTURED)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
		GLint light_up_uloc = glGetUniformLocation(program, "light_up");

		if (registry.lightUps.has(entity))
		{
			glUniform1i(light_up_uloc, 1);
			assert(light_up_uloc >= 0);
		}
		else
		{
			glUniform1i(light_up_uloc, 0);
		}

		gl_has_errors();

		GLuint time_passed_uloc = glGetUniformLocation(program, "time_passed");
		GLuint lifespan_uloc = glGetUniformLocation(program, "lifespan");
		float ms_passed = 1.f;
		float lifespan = 1.f;

		if (registry.effects.has(entity))
		{
			if (registry.effects.get(entity).type == EFFECT_TYPE::DASH)
			{
				ms_passed = registry.effects.get(entity).ms_passed;
				lifespan = registry.effects.get(entity).lifespan_ms;
			}
		}

		glUniform1f(time_passed_uloc, ms_passed);
		glUniform1f(lifespan_uloc, lifespan);

		gl_has_errors();

		assert(in_texcoord_loc >= 0);

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(TexturedVertex), (void *)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(
			in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
			(void *)sizeof(
				vec3)); // note the stride to skip the preceeding vertex position

		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		gl_has_errors();

		assert(registry.renderRequests.has(entity));
		GLuint texture_index = (GLuint)registry.renderRequests.get(entity).used_texture - 1;
		if (texture_index == -1)
		{
			texture_index = 0;
		}
		GLuint texture_id = texture_gl_handles[texture_index];
		;
		if (render_request.used_sprite == SPRITE_ASSET_ID::SPRITE_COUNT || render_request.sprite_index == -1)
		{
			texture_id =
				texture_gl_handles[(GLuint)registry.renderRequests.get(entity).used_texture];

			glBindTexture(GL_TEXTURE_2D, texture_id);
			gl_has_errors();

			GLuint uv_offset_loc = glGetUniformLocation(program, "uv_offset");
			glUniform2f(uv_offset_loc, 0.0f, 0.0f);

			GLuint uv_scale_loc = glGetUniformLocation(program, "uv_scale");
			glUniform2f(uv_scale_loc, 1.0f, 1.0f);
		}
		else
		{
			texture_id = texture_gl_handles[(GLuint)sprite_sheets[registry.renderRequests.get(entity).used_sprite].texture_id];

			glBindTexture(GL_TEXTURE_2D, texture_id);
			gl_has_errors();

			float u0, v0, u1, v1;
			getUVCoordinates(registry.renderRequests.get(entity).used_sprite, registry.renderRequests.get(entity).sprite_index, u0, v0, u1, v1);

			GLuint uv_offset_loc = glGetUniformLocation(program, "uv_offset");
			glUniform2f(uv_offset_loc, u0, v0);

			GLuint uv_scale_loc = glGetUniformLocation(program, "uv_scale");
			glUniform2f(uv_scale_loc, (u1 - u0), (v1 - v0));
		}
	}
	else if (render_request.used_effect == EFFECT_ASSET_ID::SALMON || render_request.used_effect == EFFECT_ASSET_ID::EGG)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_color_loc = glGetAttribLocation(program, "in_color");
		gl_has_errors();

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(ColoredVertex), (void *)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_color_loc);
		glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(ColoredVertex), (void *)sizeof(vec3));
		gl_has_errors();
	}
	else if (render_request.used_effect == EFFECT_ASSET_ID::SMOKE)
	{

		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		gl_has_errors();

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(ColoredVertex), (void *)0);
		gl_has_errors();

		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
		assert(in_texcoord_loc >= 0);

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(TexturedVertex), (void *)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(
			in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
			(void *)sizeof(
				vec3)); // note the stride to skip the preceeding vertex position

		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		gl_has_errors();

		assert(registry.renderRequests.has(entity));
		GLuint texture_index = (GLuint)registry.renderRequests.get(entity).used_texture - 1;
		if (texture_index == -1)
		{
			texture_index = 0;
		}
		GLuint texture_id = texture_gl_handles[texture_index];
		;

		texture_id =
			texture_gl_handles[(GLuint)registry.renderRequests.get(entity).used_texture];

		glBindTexture(GL_TEXTURE_2D, texture_id);
		gl_has_errors();

		GLuint uv_offset_loc = glGetUniformLocation(program, "uv_offset");
		glUniform2f(uv_offset_loc, 0.0f, 0.0f);

		GLuint uv_scale_loc = glGetUniformLocation(program, "uv_scale");
		glUniform2f(uv_scale_loc, 1.0f, 1.0f);

		GLuint time_uloc = glGetUniformLocation(program, "time");
		glUniform1f(time_uloc, registry.effects.get(entity).ms_passed);
	}
	else
	{
		assert(false && "Type of render request not supported");
	}

	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(program, "fcolor");
	vec3 color = registry.colors.has(entity) ? registry.colors.get(entity) : vec3(1);

	if (registry.colors.has(entity))
	{
		
		if (registry.players.has(entity) && registry.players.get(entity).invulnerable)
		{
			Player& player = registry.players.get(entity);
			if (!(!player.is_dash_up && player.curr_dash_cooldown_ms >= (player.dash_cooldown_ms - player.dash_time)))
			{
				color = vec3(1, 0, 0);
			}
		}
		else
		{
			color = registry.colors.get(entity);
		}
	}

	glUniform3fv(color_uloc, 1, (float *)&color);
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();

	GLsizei num_indices = size / sizeof(uint16_t);
	// GLsizei num_triangles = num_indices / 3;

	GLint currProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
	// Setting uniform values to the currently bound program
	GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
	glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float *)&transform.mat);
	GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float *)&projection);
	gl_has_errors();
	// Drawing of num_indices/3 triangles specified in the index buffer
	if (render_request.used_effect == EFFECT_ASSET_ID::SMOKE)
	{
		glDrawElementsInstanced(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, 0, 120);
	}
	else
	{
		glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	}

	gl_has_errors();
}

void RenderSystem::drawScreenSpaceObject(Entity entity)
{
	int w, h;
	glfwGetFramebufferSize(window, &w, &h);

	UserInterface &userInterface = registry.userInterfaces.get(entity);

	mat3 screen_projection = mat3(1.0f); // Identity for 2D rendering
	screen_projection[0][0] = userInterface.scale.x;
	screen_projection[1][1] = userInterface.scale.y;
	screen_projection[2][0] = userInterface.position.x;
	screen_projection[2][1] = userInterface.position.y;

	assert(registry.renderRequests.has(entity));
	const RenderRequest &render_request = registry.renderRequests.get(entity);

	const GLuint used_effect_enum = (GLuint)render_request.used_effect;
	assert(used_effect_enum != (GLuint)EFFECT_ASSET_ID::EFFECT_COUNT);
	const GLuint program = (GLuint)effects[used_effect_enum];

	glUseProgram(program);
	gl_has_errors();

	assert(render_request.used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT);
	const GLuint vbo = vertex_buffers[(GLuint)render_request.used_geometry];
	const GLuint ibo = index_buffers[(GLuint)render_request.used_geometry];

	mat3 transform = mat3(1.0f);

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	if (render_request.used_effect == EFFECT_ASSET_ID::TEXTURED)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
		gl_has_errors();
		assert(in_texcoord_loc >= 0);

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(TexturedVertex), (void *)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(
			in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
			(void *)sizeof(
				vec3));

		glActiveTexture(GL_TEXTURE0);
		gl_has_errors();

		assert(registry.renderRequests.has(entity));
		GLuint texture_index = (GLuint)registry.renderRequests.get(entity).used_texture - 1;
		if (texture_index == -1)
		{
			texture_index = 0;
		}
		GLuint texture_id = texture_gl_handles[texture_index];

		GLuint time_passed_uloc = glGetUniformLocation(program, "time_passed");
		GLuint lifespan_uloc = glGetUniformLocation(program, "lifespan");

		float ms_passed = 1.f;
		float lifespan = 1.f;

		glUniform1f(time_passed_uloc, ms_passed);
		glUniform1f(lifespan_uloc, lifespan);
		

		if (render_request.used_sprite == SPRITE_ASSET_ID::SPRITE_COUNT || render_request.sprite_index == -1)
		{
			texture_id =
				texture_gl_handles[(GLuint)registry.renderRequests.get(entity).used_texture];

			glBindTexture(GL_TEXTURE_2D, texture_id);
			gl_has_errors();

			GLuint uv_offset_loc = glGetUniformLocation(program, "uv_offset");
			glUniform2f(uv_offset_loc, 0.0f, 0.0f);

			GLuint uv_scale_loc = glGetUniformLocation(program, "uv_scale");
			glUniform2f(uv_scale_loc, 1.0f, 1.0f);
		}
		else
		{
			texture_id = texture_gl_handles[(GLuint)sprite_sheets[registry.renderRequests.get(entity).used_sprite].texture_id];

			glBindTexture(GL_TEXTURE_2D, texture_id);
			gl_has_errors();

			float u0, v0, u1, v1;
			getUVCoordinates(registry.renderRequests.get(entity).used_sprite, registry.renderRequests.get(entity).sprite_index, u0, v0, u1, v1);

			GLuint uv_offset_loc = glGetUniformLocation(program, "uv_offset");
			glUniform2f(uv_offset_loc, u0, v0);

			GLuint uv_scale_loc = glGetUniformLocation(program, "uv_scale");
			glUniform2f(uv_scale_loc, (u1 - u0), (v1 - v0));
		}
	}

	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();

	GLsizei num_indices = size / sizeof(uint16_t);

	GLuint transform_loc = glGetUniformLocation(program, "transform");
	glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float *)&transform);

	GLuint projection_loc = glGetUniformLocation(program, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float *)&screen_projection);

	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	gl_has_errors();
}

void RenderSystem::renderText()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	GLuint m_font_shaderProgram = effects[(GLuint)EFFECT_ASSET_ID::FONT];
	glUseProgram(m_font_shaderProgram);
	gl_has_errors();
	for (Entity &entity : registry.texts.entities)
	{
		Motion &motion_component = registry.motions.get(entity);
		float x = motion_component.position.x;
		float y = motion_component.position.y;

		Text &text_component = registry.texts.get(entity);
		glm::vec3 color = text_component.color;
		std::string text = text_component.content;
		float scale = text_component.scale;

		// get shader uniforms
		GLint textColor_location =
			glGetUniformLocation(m_font_shaderProgram, "textColor");
		glUniform3f(textColor_location, color.x, color.y, color.z);
		gl_has_errors();
		GLint transformLoc =
			glGetUniformLocation(m_font_shaderProgram, "transform");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
		gl_has_errors();
		glBindVertexArray(m_font_vao);
		gl_has_errors();
		// iterate through all characters
		std::string::const_iterator c;
		for (c = text.begin(); c != text.end(); c++)
		{
			Character ch = m_ftCharacters[*c];

			float xpos = x + ch.Bearing.x * scale;
			float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

			float w = ch.Size.x * scale;
			float h = ch.Size.y * scale;
			// update VBO for each character
			float vertices[6][4] = {
				{xpos, ypos + h, 0.0f, 0.0f},
				{xpos, ypos, 0.0f, 1.0f},
				{xpos + w, ypos, 1.0f, 1.0f},

				{xpos, ypos + h, 0.0f, 0.0f},
				{xpos + w, ypos, 1.0f, 1.0f},
				{xpos + w, ypos + h, 1.0f, 0.0f}};

			// render glyph texture over quad
			glBindTexture(GL_TEXTURE_2D, ch.TextureID);

			// update content of VBO memory
			glBindBuffer(GL_ARRAY_BUFFER, m_font_vbo);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
			gl_has_errors();
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			// render quad
			glDrawArrays(GL_TRIANGLES, 0, 6);
			gl_has_errors();

			// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
			x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
		}
	}
	glBindVertexArray(vao);
	glBindTexture(GL_TEXTURE_2D, 0);
}

// draw the intermediate texture to the screen, with some distortion to simulate
// water
void RenderSystem::drawToScreen()
{
	// Setting shaders
	// get the water texture, sprite mesh, and program
	glUseProgram(effects[(GLuint)EFFECT_ASSET_ID::WATER]);
	gl_has_errors();
	// Clearing backbuffer
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, w, h);
	glDepthRange(0, 10);
	glClearColor(1.f, 0, 0, 1.0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl_has_errors();
	// Enabling alpha channel for textures
	glDisable(GL_BLEND);
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	// Draw the screen texture on the quad geometry
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]);
	glBindBuffer(
		GL_ELEMENT_ARRAY_BUFFER,
		index_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]); // Note, GL_ELEMENT_ARRAY_BUFFER associates
																	 // indices to the bound GL_ARRAY_BUFFER
	gl_has_errors();
	const GLuint water_program = effects[(GLuint)EFFECT_ASSET_ID::WATER];
	// Set clock
	GLuint time_uloc = glGetUniformLocation(water_program, "time");
	GLuint dead_timer_uloc = glGetUniformLocation(water_program, "darken_screen_factor");
	glUniform1f(time_uloc, (float)(glfwGetTime() * 10.0f));
	ScreenState &screen = registry.screenStates.get(screen_state_entity);
	glUniform1f(dead_timer_uloc, screen.darken_screen_factor);

	// Pause uniform 0-1 switch
	GLuint paused_uloc = glGetUniformLocation(water_program, "paused");
	int pause = (screen.state != GameState::GAME) ? 1 : 0;
	glUniform1i(paused_uloc, pause);

	// Pass lighting variables
	GLuint view_pos_uloc = glGetUniformLocation(water_program, "viewPos");

	glUniform3f(view_pos_uloc, 0, 0, 1.0);

	gl_has_errors();
	// Set the vertex position and vertex texture coordinates (both stored in the
	// same VBO)
	GLint in_position_loc = glGetAttribLocation(water_program, "in_position");
	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void *)0);

	// Set up vertex normal location
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void *)0);
	glEnableVertexAttribArray(0);
	gl_has_errors();

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, off_screen_render_buffer_color);
	gl_has_errors();
	// Draw
	glDrawElements(
		GL_TRIANGLES, 3, GL_UNSIGNED_SHORT,
		nullptr); // one triangle = 3 vertices; nullptr indicates that there is
				  // no offset from the bound index buffer
	gl_has_errors();
}

// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void RenderSystem::draw()
{
	// Getting size of window
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays

	// First render to the custom framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	gl_has_errors();
	// Clearing backbuffer
	glViewport(0, 0, w, h);
	glDepthRange(0.00001, 10);
	glClearColor(GLfloat(120 / 255.0), GLfloat(120 / 255.0), GLfloat(117 / 255.0), 1.0);
	glClearDepth(10.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST); // native OpenGL does not work with a depth buffer
							  // and alpha blending, one would have to sort
							  // sprites back to front
	gl_has_errors();

	Entity camera_entity = registry.cameras.entities.front();
	vec2 camera_position = registry.motions.get(camera_entity).position;
	mat3 projection_2D = createPlayerProjectionMatrix(camera_position);
	// mat3 projection_2D = createProjectionMatrix();
	// Draw all textured meshes that have a position and size component
	std::vector<Entity> uiEntities;

	// separate by layer - visible_layers establishes order, inclusion
	std::vector<RENDER_LAYER> visible_layers = {RENDER_LAYER::FLOOR, RENDER_LAYER::EFFECTS, RENDER_LAYER::CREATURES, RENDER_LAYER::OBSTACLES, RENDER_LAYER::DEFAULT_LAYER, RENDER_LAYER::UI_LAYER};

	for  (RENDER_LAYER layer : visible_layers) {
		for (Entity entity : registry.renderRequests.entities)
		{
			if (registry.userInterfaces.has(entity) && layer == RENDER_LAYER::UI_LAYER)
			{
				uiEntities.push_back(entity);
				continue;
			}
			if (registry.renderRequests.get(entity).layer != layer) {
				continue;
			}
			
			if (!registry.motions.has(entity))
				continue;

			if (registry.texts.has(entity))
				continue;
			// Note, its not very efficient to access elements indirectly via the entity
			// albeit iterating through all Sprites in sequence. A good point to optimize
			drawTexturedMesh(entity, projection_2D);
		}
	}
	

	for (Entity entity : uiEntities)
	{
	 	drawScreenSpaceObject(entity);
	}

	// Truely render to the screen
	drawToScreen();

	renderText();
	// flicker-free display with a double buffer
	glfwSwapBuffers(window);
	gl_has_errors();
}

mat3 RenderSystem::createPlayerProjectionMatrix(vec2 position)
{
	// Fake projection matrix, scales with respect to window coordinates
	float left = 0.f;
	float top = 0.f;

	gl_has_errors();
	float right = (float)window_width_px;
	float bottom = (float)window_height_px;

	float sx = 2.f / (right - left);
	float sy = 2.f / (top - bottom);
	float tx = -(right + left) / (right - left) - (position.x - window_width_px / 2.f) * sx;
	float ty = -(top + bottom) / (top - bottom) - (position.y - window_height_px / 2.f) * sy;
	return {{sx, 0.f, 0.f}, {0.f, sy, 0.f}, {tx, ty, 1.f}};
}

mat3 RenderSystem::createProjectionMatrix()
{
	// Fake projection matrix, scales with respect to window coordinates
	float left = 0.f;
	float top = 0.f;

	gl_has_errors();
	float right = (float)window_width_px;
	float bottom = (float)window_height_px;

	float sx = 2.f / (right - left);
	float sy = 2.f / (top - bottom);
	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);
	return {{sx, 0.f, 0.f}, {0.f, sy, 0.f}, {tx, ty, 1.f}};
}

void RenderSystem::getUVCoordinates(SPRITE_ASSET_ID sid, int spriteIndex, float &u0, float &v0, float &u1, float &v1)
{
	SpriteSheetInfo info = sprite_sheets[sid];
	int numCols = info.cols;
	int numRows = info.rows;

	int col = spriteIndex % numCols;
	int row = spriteIndex / numCols;

	// std::cout << "COL: "<< col << std::endl;
	// std::cout << "ROW: " << row << std::endl;

	int textureWidth = numCols * info.sprite_width;
	int textureHeight = numRows * info.sprite_height;

	u0 = col * (float)info.sprite_width / textureWidth;
	v0 = row * (float)info.sprite_height / textureHeight;
	u1 = (col + 1) * (float)info.sprite_width / textureWidth;
	v1 = (row + 1) * (float)info.sprite_height / textureHeight;
}
