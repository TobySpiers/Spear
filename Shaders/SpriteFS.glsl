#version 410 core

uniform sampler2DArray textureSampler;

in vec4 v_data;

out vec4 color;

void main() 
{
	color = texture(textureSampler, v_data.xyz);
	color.a *= v_data.w;
}