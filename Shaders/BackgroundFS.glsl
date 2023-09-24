#version 410 core

uniform sampler2D textureSampler;

in vec2 v_texCoord;

out vec4 color;

void main() 
{
	color = texture(textureSampler, v_texCoord);
}