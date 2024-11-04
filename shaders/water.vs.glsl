#version 330

in vec3 in_position;

out vec2 texcoord;
out vec3 normal;
out vec3 fragPos;

uniform vec3 viewPos; // position of camera - might change from uniform later - going to use as viewPos and lightPos

layout (location = 0) in vec3 aNormal;

void main()
{
    gl_Position = vec4(in_position.xy, 0, 1.0);
	texcoord = (in_position.xy + 1) / 2.f;

    // pass normal and fragment position to fragment shader
    normal = (viewPos - vec3(in_position.xy, 0));
    fragPos = vec3(in_position.xy, 0); // not sure about the values we're passing here...
}
