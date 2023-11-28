#version 410 core

uniform vec2 spriteSize;

layout(location = 0) in vec4 inPos;
// xy: 2d pos, zw: scaleX, scaleY

layout(location = 1) in vec2 inColor;
// inColor.x = opacity, inColor.y = texture depth

out vec4 v_data;

void main() 
{	
	vec2 quadPoints[4];	
	quadPoints[0] = vec2(inPos.x - (spriteSize.x * inPos.z), inPos.y - (spriteSize.y * inPos.w));
	quadPoints[1] = vec2(inPos.x + (spriteSize.x * inPos.z), inPos.y - (spriteSize.y * inPos.w));
	quadPoints[2] = vec2(inPos.x - (spriteSize.x * inPos.z), inPos.y + (spriteSize.y * inPos.w));
	quadPoints[3] = vec2(inPos.x + (spriteSize.x * inPos.z), inPos.y + (spriteSize.y * inPos.w));
		
	vec4 uvCoords[4];
	uvCoords[0] = vec4(0, 1, inColor.y, inColor.x);
	uvCoords[1] = vec4(1, 1, inColor.y, inColor.x);
	uvCoords[2] = vec4(0, 0, inColor.y, inColor.x);
	uvCoords[3] = vec4(1, 0, inColor.y, inColor.x);
	
	gl_Position = vec4(quadPoints[gl_VertexID].xy, 0.1, 1.0);
	v_data = uvCoords[gl_VertexID];
}