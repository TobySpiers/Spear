#version 430 core
// Compute shader for Raycasting walls

// TODO: Investigate moving these structs into a 'header' file we can reuse for a RaycastPlanes compute shader
// May require our C++ ShaderCompiler class somehow 'detecting' includes and inserting them itself
struct GridNode
{
	int texIdRoofA; // primary roof
	int texIdRoofB; // additional elevated roof
	int texIdWall;
	int texIdFloorA; // primary floor
	int texIdFloorB; // additional sunk floor
	
	int drawFlags;
	
	int extendUp;
	int extendDown;
	int collisionMask;
};

struct RaycasterConfig
{
	float fieldOfView;
	float farClip;
	int xResolution;
	int yResolution;
	int threads;
	int rayEncounterLimit;
	float scale2D;
	bool highlightCorrectivePixels;
	float correctivePixelDepthTolerance;
};

struct FrameData
{
	float viewPitch;
	float viewHeight;
	vec2 viewPos;
	vec2 viewForward;
	
	vec2 screenPlaneEdgeL;
	vec2 screenPlaneEdgeR;
	vec2 screenPlane;
	
	vec2 raySpacingDir;
	float raySpacingLength;
	
	vec2 fovMinAngle;
	vec2 fovMaxAngle;
	
	float fov;
	float fovWallMultiplier;
};

// UBOs
layout(std140) uniform raycasterConfig { RaycasterConfig rayConfig; };
layout(std140) uniform frameData { FrameData frame; };

// BINDINGS
layout(rgba8, binding = 0) uniform image2D outTexture;
layout(r8, binding = 1) uniform image2D outDepth;
layout(std430, binding = 2) buffer GridNodes { GridNode nodes[]; };

// UNIFORMS
layout(location = 0) uniform ivec2 texResolution;
layout(location = 1) uniform ivec2 gridDimensions;
//layout(location = 0) uniform sampler2DArray textureArray;

// Each thread = 1 Ray = 1 Vertical Strip of Pixels
layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{
    ivec2 pixelPosition = ivec2(gl_GlobalInvocationID.xy);
	vec4 value = vec4(1.0, 0.0, 0.0, 1.0);
	
	for(int yPos = 0; yPos < texResolution.y; yPos++)
	{
		pixelPosition.y = yPos;
		
		int nodeId = (pixelPosition.x % gridDimensions.x) + ((pixelPosition.y % gridDimensions.y) * gridDimensions.x);
		value.x = nodes[nodeId].texIdFloorA;
		if(value.x != -1)
		{
			value.x = 1;
		}
		else
		{
			value.x = 0;
		}
		
		if(rayConfig.highlightCorrectivePixels)
		{
			value.y = 1;
		}
		
		value.z = frame.viewForward.x;
		
		imageStore(outTexture, pixelPosition, value);
		imageStore(outDepth, pixelPosition, vec4(0));
	}    
}