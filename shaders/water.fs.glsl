#version 330

uniform sampler2D screen_texture;
uniform float time;
uniform float darken_screen_factor;
uniform vec3 viewPos;
uniform int paused;

uniform int darkenedmode;

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

	// viewPos
	vec3 viewPos1 = 2 * viewPos;

	// ambient calculation
	float lightStrength = 0.1;
	if (darkenedmode == 1) {
		lightStrength = 0.5;
	}
	vec3 ambient = lightStrength * vec3(1.0, 1.0, 1.0);
	// ambient done

	// norm coming off of point
	vec3 norm = normalize(normal);
	// light direction - from fragment position to viewpoint position
	vec3 lightDir = normalize(viewPos1 + fragPos);
	
	// calculating diffuse component
	float diff = max((dot(norm, lightDir)), 0.0);
	

	vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);

	float specularStrength = 0.01;

	vec3 viewDir = normalize(viewPos1 - fragPos);
	vec3 reflectDir = reflect(-lightDir, norm);

	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specularStrength * spec * vec3(1.0, 1.0, 1.0);

	color = vec4((ambient + diffuse + specular) * color.xyz, 1.0);
	color = fade_color(color);

	// excluding UI from lighting effects (unless it is paused):

	if (paused != 1) {
		// HP Bar
		if ((fragPos.y >= 0.75) && (fragPos.y <= 0.95)) {
			if ((fragPos.x >= -0.98) && (fragPos.x <= -0.52)) {
				if ((fragPos.x <= -0.97) && (fragPos.y >= 0.9 || fragPos.y <= 0.84)) {
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

		// Stamina Bar
		if ((fragPos.x >= -0.96) && (fragPos.x <= -0.52)) {
			if ((fragPos.y >= 0.65) && (fragPos.y <= 0.735)) {
				if ((fragPos.x <= -0.94) && (fragPos.y >= 0.72)) {
					color = color;
				} else if ((fragPos.x <= -0.95) && (fragPos.y >=0.71)) {
					color = color;
				} else if ((fragPos.x >= -0.54) && (fragPos.y >= 0.72)) {
					color = color;
				} else if ((fragPos.x >= -0.55) && (fragPos.y >= 0.71)) {
					color = color;
				}
				else {
					color = in_color;
				}
			}
		}

		// XP Bar

		if ((fragPos.x >= -0.96) && (fragPos.x <= -0.52)) {
			if ((fragPos.y >= 0.5) && (fragPos.y <= 0.585)) {
				if ((fragPos.x <= -0.94) && (fragPos.y >= 0.57)) {
					color = color;
				} else if ((fragPos.x <= -0.95) && (fragPos.y >=0.56)) {
					color = color;
				} else if ((fragPos.x >= -0.54) && (fragPos.y >= 0.57)) {
					color = color;
				} else if ((fragPos.x >= -0.55) && (fragPos.y >= 0.56)) {
					color = color;
				}
				else {
					color = in_color;
				}
			}
		}

		// Holy Artifact
		if ((fragPos.y >= 0.73) && (fragPos.y <= 0.97)) {
			if ((fragPos.x <= 0.965) && (fragPos.x >= 0.835)) {
				if ((fragPos.x >= 0.925 || fragPos.x <= 0.875) && (fragPos.y >= 0.955)) {
					color = color;
				} else if ((fragPos.x >= 0.945 || fragPos.x <= 0.865) && (fragPos.y >= 0.935)) {
					color = color;
				} else if ((fragPos.x >= 0.955 || fragPos.x <= 0.855) && (fragPos.y >= 0.915)) {
					color = color;
				}else if ((fragPos.x >= 0.955 || fragPos.x <= 0.845) && (fragPos.y <= 0.854)) {
					color = color;
				} else if ((fragPos.x >= 0.945 || fragPos.x <= 0.855) && (fragPos.y <= 0.82)) {
					color = color;
				} else if ((fragPos.x >= 0.935 || fragPos.x <= 0.865) && (fragPos.y <= 0.8)) {
					color = color;
				} else if ((fragPos.x >= 0.925 || fragPos.x <= 0.875) && (fragPos.y <= 0.78)) {
					color = color;
				} else if ((fragPos.x >= 0.915 || fragPos.x <= 0.885) && (fragPos.y <= 0.76)) {
					color = color;
				} else if ((fragPos.x >= 0.905 || fragPos.x <= 0.895) && (fragPos.y <= 0.74)) {
					color = color;
				} else if ((fragPos.x >= 0.99) && (fragPos.y <= 0.8)) {
					color = color;
				} else if ((fragPos.x >= 0.99) && (fragPos.y >= 0.99)) {
					color = color;
				} else if ((fragPos.x >= 0.99) && (fragPos.y >= 0.9)) {
					color = color;
				} else if ((fragPos.x >= 0.99) && (fragPos.y >= 0.99)) {
					color = color;
				} 
				else {
					color = in_color;		
				}
			}
		}
	}
	
}