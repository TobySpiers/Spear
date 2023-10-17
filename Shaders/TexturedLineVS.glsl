#version 410 core

uniform vec2 lineWidth;

layout(location = 0) in vec4 inPos;
layout(location = 1) in vec2 inUv;

out vec3 v_UV;

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
	
	gl_Position = vec4(linePoints[gl_VertexID].xy, 0.0, 1.0);	
	
	vec3 uvCoords[4];
	uvCoords[0] = vec3(inUv.x, 0, inUv.y);
	uvCoords[1] = vec3(inUv.x, 0, inUv.y);
	uvCoords[2] = vec3(inUv.x, 1, inUv.y);
	uvCoords[3] = vec3(inUv.x, 1, inUv.y);
	v_UV = uvCoords[gl_VertexID];
}