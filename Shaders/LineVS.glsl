#version 410 core

uniform vec2 lineWidth;

layout(location = 0) in vec4 inPos;
layout(location = 1) in vec4 inColor;
layout(location = 2) in float inDepth;

out vec4 v_color;

void main() 
{
	vec2 forward = vec2(inPos.zw - inPos.xy);
	forward = forward / length(forward);
	vec2 perpendicular = vec2(-forward.y, forward.x);
	
	// alignment 1 = X Width, alignment 0 = Y Width
	float alignment = abs(dot(forward, vec2(0, 1)));
	
	// Calculate width based on rotation + window ratio
	float widthDiff = lineWidth.x - lineWidth.y;
	float adjustedWidth = lineWidth.y + (widthDiff * alignment);
	
	vec2 linePoints[4];	
	linePoints[0] = inPos.xy + (perpendicular * adjustedWidth);
	linePoints[1] = inPos.xy;
	linePoints[2] = inPos.zw + (perpendicular * adjustedWidth);
	linePoints[3] = inPos.zw;
	
	gl_Position = vec4(linePoints[gl_VertexID].xy, inDepth, 1.0);	
	v_color = inColor;
}