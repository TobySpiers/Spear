#version 410 core

uniform sampler2D textureSampler;
uniform sampler2D depthSampler;

in vec2 v_texCoord;

out vec4 color;

void main() 
{	
	gl_FragDepth = texture(depthSampler, v_texCoord).r;
	// note: assigning to depthbuffer like this disables early depth testing, worsens performance 
	// currently using this approach to keep resolution easily adaptable at runtime
	// more performant solution is to create FrameBufferObject same size as texture and assign DepthTexture directly
	
	color = texture(textureSampler, v_texCoord);
}