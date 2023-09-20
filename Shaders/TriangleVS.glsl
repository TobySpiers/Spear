#version 410 core

layout(location=0) in vec3 position;
layout(location=1) in vec3 vertexColour;
layout(location=2) in vec2 texCoord;

out vec3 v_vertexColours;
out vec2 v_vertexTexcoord;

void main()
{
	gl_Position = vec4(position.x, position.y, position.z, 1.0f);
	
	v_vertexColours = vertexColour;
	v_vertexTexcoord = texCoord;
}