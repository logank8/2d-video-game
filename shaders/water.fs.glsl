#version 330

uniform sampler2D screen_texture;
uniform float time;
uniform float darken_screen_factor;
uniform vec3 viewPos;

in vec2 texcoord;
in vec3 normal;
in vec3 fragPos; // range [-1, 1]

layout(location = 0) out vec4 color;


vec4 fade_color(vec4 in_color) 
{
	if (darken_screen_factor > 0)
		in_color = darken_screen_factor * vec4(0.0, 0.0, 0.0, 1.0) + (1 - darken_screen_factor) * in_color;
	return in_color;
}

// position bounds should be [-1, 1]

void main()
{
	vec2 coord = texcoord;

    vec4 in_color = texture(screen_texture, coord);
    color = in_color;

	// general lighting
	// calculate ambient component

	// viewPos: center of screen
	vec3 viewPos1 = vec3(0, 0, 2);

	// ambient calculation
	float lightStrength = 0.45;
	vec3 ambient = lightStrength * vec3(1.0, 1.0, 0.9);
	// ambient done

	// norm coming off of point
	vec3 norm = normalize(normal);
	// light direction - from fragment position to viewpoint position
	vec3 lightDir = normalize(viewPos1 + fragPos);
	
	// calculating diffuse component
	float diff = max((dot(norm, lightDir)), 0.0);

	vec3 diffuse = diff * vec3(1.0, 1.0, 0.9);

	color = vec4((ambient + diffuse) * color.xyz, 1.0);
	color = fade_color(color);
}