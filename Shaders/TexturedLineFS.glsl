#version 410 core

uniform sampler2DArray textureSampler;

in vec3 v_UV;

out vec4 color;

void main() 
{
	color = texture(textureSampler, v_UV);
}