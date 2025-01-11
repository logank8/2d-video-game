#version 330

// Input attributes
in vec3 in_position;
in vec2 in_texcoord;

// Passed to fragment shader
out vec2 texcoord;
out float scale;

// Application data
uniform mat3 transform;
uniform mat3 projection;
uniform vec2 uv_offset = vec2(0.0, 0.0);  
uniform vec2 uv_scale = vec2(1.0, 1.0);  

uniform float time;

uniform vec2 offsets[300];
uniform float scales[300];
uniform float lifespans[300];

void main()
{

	texcoord = (in_texcoord * uv_scale) + uv_offset;


	float x = offsets[gl_InstanceID][0];
	float y = offsets[gl_InstanceID][1] + (0.001 * time);

	scale = scales[gl_InstanceID];


	vec3 pos = projection * transform * vec3(in_position.xy + vec2(x, y), 1.0);
	gl_Position = vec4(pos.xy, in_position.z, 1.0);
}