#version 330

// Input attributes
in vec3 in_position;
in vec2 in_texcoord;

// Passed to fragment shader
out vec2 texcoord;

// Application data
uniform mat3 transform;
uniform mat3 projection;
uniform vec2 uv_offset = vec2(0.0, 0.0);  
uniform vec2 uv_scale = vec2(1.0, 1.0);  

void main()
{
	texcoord = (in_texcoord * uv_scale) + uv_offset;
	vec3 pos = projection * transform * vec3(in_position.xy, 1.0);
	gl_Position = vec4(pos.xy, in_position.z, 1.0);
}