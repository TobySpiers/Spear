#version 410 core

out vec2 v_texCoord;

void main() 
{
	vec2 linePoints[4];	
	linePoints[0] = vec2(-1, -1);
	linePoints[1] = vec2(1, -1);
	linePoints[2] = vec2(-1, 1);
	linePoints[3] = vec2(1, 1);

	vec2 uvCoords[4];
	uvCoords[0] = vec2(0, 0);
	uvCoords[1] = vec2(1, 0);
	uvCoords[2] = vec2(0, 1);
	uvCoords[3] = vec2(1, 1);
	
	gl_Position = vec4(linePoints[gl_VertexID].xy, 0.0, 1.0);	
	v_texCoord = uvCoords[gl_VertexID];
}