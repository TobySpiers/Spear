#version 410 core
in vec3 v_vertexColours;
in vec2 v_vertexTexcoord;

uniform sampler2D textureSampler;

out vec4 color;

void main()
{
	color = texture(textureSampler, v_vertexTexcoord);
	//color = vec4(v_vertexColours.r, v_vertexColours.g, v_vertexColours.b, 1.0f);
}