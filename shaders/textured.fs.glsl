#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;
uniform int light_up;

// Output color
layout(location = 0) out  vec4 color;

void main()
{
	color = vec4(fcolor, 1.0) * texture(sampler0, vec2(texcoord.x, texcoord.y));

	if (light_up == 1) {
		color = vec4(fcolor, 1.0) * 10000.f * texture(sampler0, vec2(texcoord.x, texcoord.y));
	}
}
