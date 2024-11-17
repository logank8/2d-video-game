#version 330

// From Vertex Shader
in vec3 vcolor;

// Application data
uniform vec3 fcolor;
uniform float time_passed;
uniform float lifespan;

// Output color
layout(location = 0) out vec4 color;

void main()
{
	color = vec4(fcolor * vcolor, time_passed / lifespan);
}