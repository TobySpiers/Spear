#version 410 core

uniform vec2 spriteSize;

layout(location = 0) in vec4 inPos;
// xy: 2d pos, zw: scaleX, scaleY

layout(location = 1) in vec4 inColor;
// xyz: rgb, w: textureDepth

out vec3 v_uvPos;
out vec3 v_color;

void main() 
{	
	vec2 quadPoints[4];	
	quadPoints[0] = vec2(inPos.x - (spriteSize.x * inPos.z), inPos.y - (spriteSize.y * inPos.w));
	quadPoints[1] = vec2(inPos.x + (spriteSize.x * inPos.z), inPos.y - (spriteSize.y * inPos.w));
	quadPoints[2] = vec2(inPos.x - (spriteSize.x * inPos.z), inPos.y + (spriteSize.y * inPos.w));
	quadPoints[3] = vec2(inPos.x + (spriteSize.x * inPos.z), inPos.y + (spriteSize.y * inPos.w));
		
	vec3 uvCoords[4];
	uvCoords[0] = vec3(0, 1, inColor.w);
	uvCoords[1] = vec3(1, 1, inColor.w);
	uvCoords[2] = vec3(0, 0, inColor.w);
	uvCoords[3] = vec3(1, 0, inColor.w);
	
	// Outputs
	gl_Position = vec4(quadPoints[gl_VertexID].xy, 0.0, 1.0);
	v_uvPos = uvCoords[gl_VertexID];
	v_color = inColor.xyz;
}