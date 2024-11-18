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

uniform float time;

//uniform vec2 offsets[100];

void main()
{
	// we split oscillation by fourths
	// later this will be passed as a uniform - but rn just doing a constant thingy
	int circumfrence = 24;

	float layer = gl_InstanceID / circumfrence;

	float rotate_angle = gl_InstanceID % (circumfrence * 2);
	rotate_angle = rotate_angle * (3.14 / 12);

	vec2 offset;

	// movement function based on time and mod 11 


	mat2 rotation_mx = mat2(cos(rotate_angle), - sin(rotate_angle), 
						sin(rotate_angle), cos(rotate_angle));

	vec2 dir = vec2(1, 1);

	float spread_factor = ((time / 200));
	

	offset = spread_factor * layer * normalize(rotation_mx * dir);

	offset += 0.01 * (gl_InstanceID % 34);


	texcoord = (in_texcoord * uv_scale) + uv_offset;
	vec3 pos = projection * transform * vec3(in_position.xy + offset, 1.0);
	gl_Position = vec4(pos.xy, in_position.z, 1.0);
}