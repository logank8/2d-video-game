#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;
uniform int light_up;
uniform float time_passed;
uniform float lifespan;

// Output color
layout(location = 0) out  vec4 color;

void main()
{
	float alpha = 1.0;
	if (lifespan != 1.0) {
		alpha = (lifespan - time_passed) / lifespan;
	}
	color = vec4(fcolor, alpha) * texture(sampler0, vec2(texcoord.x, texcoord.y));

	if (light_up == 1) {
		color = vec4(fcolor, alpha) * 10000.f * texture(sampler0, vec2(texcoord.x, texcoord.y));
	}
}
