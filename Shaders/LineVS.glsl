#version 410 core

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec4 inColor;

out vec4 v_color;

//uniform float lineWidth;

void main() 
{
	gl_Position = vec4(inPosition, 0.0, 1.0);
	//gl_PointSize = lineWidth;
	
	v_color = inColor;
}