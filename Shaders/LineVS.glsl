#version 410 core

layout(location = 0) in vec4 inPos;
layout(location = 1) in vec4 inColor;

out vec4 v_color;

void main() 
{
	vec2 linePoints[2];
	linePoints[0] = inPos.xy;
	linePoints[1] = inPos.zw;

	gl_Position = vec4(linePoints[gl_VertexID].xy, 0.0, 1.0);	
	v_color = inColor;
}