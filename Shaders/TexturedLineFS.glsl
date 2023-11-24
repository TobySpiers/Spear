#version 410 core

uniform sampler2DArray textureSampler;

in vec3 v_UV;
in float depth;

out vec4 color;

void main() 
{
	gl_FragDepth = depth;
	// temporary solution. ideally we should use the VS depth calculation and adjust BackgroundFS.glsl to the same range
	
	color = texture(textureSampler, v_UV);
	color.rgb *= (1 - gl_FragDepth);
}