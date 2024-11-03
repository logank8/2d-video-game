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
	vec3 ambient = lightStrength * vec3(1.0, 1.0, 1.0);
	// ambient done

	// norm coming off of point
	vec3 norm = normalize(normal);
	// light direction - from fragment position to viewpoint position
	vec3 lightDir = normalize(viewPos1 + fragPos);
	
	// calculating diffuse component
	float diff = max((dot(norm, lightDir)), 0.0);

	vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);

	color = vec4((ambient + diffuse) * color.xyz, 1.0);
	color = fade_color(color);

	// excluding UI from lighting effects:

	// HP Bar
	if ((fragPos.y >= 0.75) && (fragPos.y <= 0.95)) {
		if ((fragPos.x >= -0.98) && (fragPos.x <= -0.52)) {
			if ((fragPos.x <= -0.97) && (fragPos.y >= 0.9 || fragPos.y <= 0.837)) {
				color = color;
			} else if ((fragPos.x <= -0.965) && (fragPos.y >= 0.91 || fragPos.y <= 0.827)) {
				color = color;
			} else if ((fragPos.x <= -0.95) && (fragPos.y >= 0.925 || fragPos.y <= 0.817)) {
				color = color;
			} else if ((fragPos.x <= -0.942) && (fragPos.y <= 0.79)){
				color = color;
			} else if ((fragPos.x <= -0.937) && (fragPos.y <= 0.78)) {
				color = color;
			} else if ((fragPos.x <= -0.93) && (fragPos.y <= 0.77)) {
				color = color;
			} else if ((fragPos.x <= -0.91) && (fragPos.y >= 0.935 || (fragPos.y <= 0.78))) {
				color = color;
			} else if ((fragPos.x >= -0.895) && (fragPos.y >= 0.935)) {
				color = color;
			} else if ((fragPos.x >= -0.95) && (fragPos.y <= 0.775)) {
				color = color;
			} else if ((fragPos.x >= -0.91) && (fragPos.y <= 0.8)) {
				color = color;
			} else if ((fragPos.x >= -0.887) && (fragPos.y >= 0.912)) {
				color = color;
			} else if ((fragPos.x >= -0.875) && (fragPos.y >= 0.9)) {
				color = color;
			} else if ((fragPos.x >= -0.53) && (fragPos.y >= 0.887)) {
				color = color;
			} 
			else {
				color = in_color;
			}
		}
	}


	// points to cut out of box
	// x=-0.98 -> y = [0.95, 0.91], [0.79, 0.75]
	// x=-0.97 -> y = [0.95, 0.92], [0.78, 0.75]
	// x=-0.96 -> y = [0.95, 0.93], [0.77, 0.75]
	// x=-0.95 -> y = [0.95, 0.94], [0.76, 0.75]
	// x=-0.94 -> y = 0.94, 0.75
}