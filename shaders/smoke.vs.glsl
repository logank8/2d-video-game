#version 330

// Input attributes
in vec3 in_color;
in vec3 in_position;

out vec3 vcolor;

// Application data
uniform mat3 transform;
uniform mat3 projection;
uniform vec2 offsets[100]; // 100 offset vectors

void main()
{
	vcolor = in_color;
    vec2 offset = offsets[gl_InstanceID]; // edit position by id offset
	vec3 pos = projection * transform * vec3(in_position.xy, 1.0);
	gl_Position = vec4(pos.xy + offset, in_position.z, 1.0);
}