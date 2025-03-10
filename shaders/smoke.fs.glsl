#version 330

// From Vertex Shader
in vec3 vcolor;
in vec2 texcoord;
in float scale;

// Application data
uniform vec3 fcolor;
uniform sampler2D sampler0;
uniform float time;

// Output color
layout(location = 0) out vec4 color;

void main()
{

	color = vec4(fcolor, 1.0 - scale) * texture(sampler0, vec2(texcoord.x, texcoord.y));

	
}