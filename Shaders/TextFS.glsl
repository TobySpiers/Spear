#version 410 core

uniform sampler2DArray textureSampler;

in vec3 v_uvPos;
in vec3 v_color;

out vec4 color;

void main() 
{
	color = texture(textureSampler, v_uvPos);
	color.r *= v_color.r;
	color.y *= v_color.y;
	color.z *= v_color.z;
}