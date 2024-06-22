#version 410 core

uniform sampler2D textureSampler;

in vec2 v_texCoord;
out vec4 color;

void main() 
{	
	color = texture(textureSampler, v_texCoord);
	color.r = color.r;
	color.g = color.g;
	color.b = color.b;
}