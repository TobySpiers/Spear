#version 430 core
// Compute shader for Raycasting multiple planes

// TODO: Investigate moving these structs into a 'header' file we can reuse for a RaycastPlanes compute shader
// May require our C++ ShaderCompiler class somehow 'detecting' includes and inserting them itself
struct GridNode
{
	int texIdRoof[2];
	int texIdWall;
	int texIdFloor[2];
	
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
	int rayEncounterLimit;
	float scale2D;
	bool highlightCorrectivePixels;
	float correctivePixelDepthTolerance;
};

struct FrameData
{
	float viewPitch;
	float viewHeight;
	float fov;
	float fovWallMultiplier;
	
	vec2 viewPos;
	vec2 viewForward;
	
	vec2 screenPlaneEdgeL;
	vec2 screenPlaneEdgeR;
	vec2 screenPlane;
	
	vec2 fovMinAngle;
	vec2 fovMaxAngle;
		
	vec2 raySpacingDir;	
	float raySpacingLength;
	
	float fovSpriteMultiplier;
	vec2 planeHeights;
};

struct SpriteData
{
	ivec2 spriteStart;
	ivec2 spriteEnd;
	int spriteTex;
	float spriteDepth;
};

// UBOs
layout(std140) uniform raycasterConfig { RaycasterConfig rayConfig; };
layout(std140) uniform frameData { FrameData frame; };

// BINDINGS
layout(rgba8, binding = 0) uniform image2D outTexture;
layout(r32f, binding = 1) uniform image2D outDepth;
layout(std430, binding = 2) buffer GridNodes { GridNode nodes[]; };
layout(binding = 3) uniform sampler2DArray worldTextures;
layout(binding = 4) uniform sampler2DArray spriteTextures;
layout(std430, binding = 5) buffer SpritesData { SpriteData spriteData[]; };

// UNIFORMS
layout(location = 0) uniform ivec2 gridDimensions;
layout(location = 1) uniform int spriteCount;

// eLevelTexture definitions
const int TEX_NONE = -1;

// eRayHit definitions
const int RAY_NOHIT = 0;
const int RAY_HIT_FRONT = 1;
const int RAY_HIT_SIDE = 2;

// eDrawFlags definitions
const int DRAW_DEFAULT = 0;
const int DRAW_N = 1;
const int DRAW_E = 2;
const int DRAW_S = 4;
const int DRAW_W = 8;

vec2 Projection(vec2 vecToProject, vec2 vecTarget)
{
	return normalize(vecTarget) * dot(vecToProject, normalize(vecTarget));
}

vec2 Normal(vec2 inputVec)
{
	return vec2(-inputVec.y, inputVec.x);
}

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{
	ivec2 screen = ivec2(gl_GlobalInvocationID.xy);
	
	// Clear last pixel
	imageStore(outTexture, screen, vec4(1,0,0,1));
	imageStore(outDepth, screen, vec4(rayConfig.farClip, 0, 0, 1));
	
	bool bIsFloor = screen.y < (float(rayConfig.yResolution) / 2) + frame.viewPitch;
	
	// Current y position compared to the center of the screen (the horizon)
	int rayPitch;
	if(bIsFloor)
	{		
		rayPitch = int(((imageSize(outTexture).y - screen.y - 1) - (rayConfig.yResolution / 2)) + frame.viewPitch);
	}
	else
	{
		rayPitch = int((screen.y - (rayConfig.yResolution / 2)) - frame.viewPitch);
	}
		
	// Horizontal distance from the camera to the floor for the current row.
	// 0.5 is the z position exactly in the middle between floor and ceiling.
	float rowDistance[2];
	const float defaultDistance = frame.viewHeight / rayPitch;
	rowDistance[0] = defaultDistance * frame.planeHeights.x;
	rowDistance[1] = defaultDistance * frame.planeHeights.y;
	
	// vector representing position offset equivalent to 1 pixel right (imagine topdown 2D view, this 'jumps' horizontally by 1 ray)
	vec2 rayStep[2];
	const vec2 rayPixelWidth = (frame.fovMaxAngle - frame.fovMinAngle) / rayConfig.xResolution;
	rayStep[0] = rowDistance[0] * rayPixelWidth;
	rayStep[1] = rowDistance[1] * rayPixelWidth;
	
	// endpoint of first ray in row (left)
	// essentially the 'left-most ray' along a length (depth) of rowDistance
	vec2 rayEnd[2];
	rayEnd[0] = frame.viewPos + rowDistance[0] * frame.fovMinAngle;
	rayEnd[1] = frame.viewPos + rowDistance[1] * frame.fovMinAngle;
		
	// calculate 'depth' for this strip of floor (same depth will be shared for all drawn pixels in one row of X)
	vec2 rayStart[2];
	float rayDepth[2];
	rayStart[0] = frame.viewPos + Projection(rowDistance[0] * frame.fovMinAngle, Normal(frame.viewForward));
	rayStart[1] = frame.viewPos + Projection(rowDistance[1] * frame.fovMinAngle, Normal(frame.viewForward));
	rayDepth[0] = length(rayEnd[0] - rayStart[0]) / rayConfig.farClip;
	rayDepth[1] = length(rayEnd[1] - rayStart[1]) / rayConfig.farClip;
	
	// Step along X to the correct rays for this invocation's pixel
	rayEnd[0] += rayStep[0] * screen.x;
	rayEnd[1] += rayStep[1] * screen.x;
	
	// Prevent drawing textures behind camera
	float curDepth = rayConfig.farClip;
	if (rowDistance[0] > 0)
	{
		for (int layer = 0; layer < 2; layer++)
		{
			const int cellX = int(rayEnd[layer].x);
			const int cellY = int(rayEnd[layer].y);

			if(cellX < gridDimensions.x && cellY < gridDimensions.y && cellX >= 0 && cellY >= 0)
			{
				GridNode node = nodes[cellX + (cellY * gridDimensions.x)];
			
				// Tex sampling
				if ((bIsFloor && node.texIdFloor[layer] != TEX_NONE) || (!bIsFloor && node.texIdRoof[layer] != TEX_NONE))
				{				
					int texX = int((rayEnd[layer].x - cellX) * textureSize(worldTextures, 0).x);
					int texY = int((rayEnd[layer].y - cellY) * textureSize(worldTextures, 0).y);
					if (texX < 0)
					{
						texX += textureSize(worldTextures, 0).x;
					}
					if (texY < 0)
					{
						texY += textureSize(worldTextures, 0).y;
					}
					vec3 texCoord = vec3(float(texX) / textureSize(worldTextures, 0).x, float(texY) / textureSize(worldTextures, 0).y, bIsFloor ? node.texIdFloor[layer] : node.texIdRoof[layer]);

					vec4 texel = texture(worldTextures, texCoord);
					if(texel.a == 0)
					{
						continue;
					}
										
					imageStore(outTexture, screen, texel);
					imageStore(outDepth, screen, vec4(rayDepth[layer], 0, 0, 0));
					curDepth = rayDepth[layer];

					// We're drawing the planes nearest-first, so break this loop as soon as we draw a pixel (ie. no need to calculate pixels BEHIND this)
					break;
				}
			}
		}
	}
	
	// Draw any sprite data needed at this position
	ivec3 spriteTexSize = textureSize(spriteTextures, 0);
	for(int i = 0; i < spriteCount; i++)
	{
		if(spriteData[i].spriteStart.x <= screen.x && spriteData[i].spriteEnd.x >= screen.x)
		{
			if(spriteData[i].spriteStart.y <= screen.y && spriteData[i].spriteEnd.y >= screen.y)
			{
				if(spriteData[i].spriteDepth < curDepth)
				{			
					float texX = spriteTexSize.x * (float(screen.x - spriteData[i].spriteStart.x) / (spriteData[i].spriteEnd.x - spriteData[i].spriteStart.x));
					float texY = spriteTexSize.y - ((spriteTexSize.y - 1) * (float(screen.y - spriteData[i].spriteStart.y) / (spriteData[i].spriteEnd.y - spriteData[i].spriteStart.y)));
					
					vec3 texCoord = vec3(texX / spriteTexSize.x, texY / spriteTexSize.y, spriteData[i].spriteTex);
					vec4 texel = texture(spriteTextures, texCoord);
					
					// If we want to support transparency in sprites, we'll need to move them until AFTER the WallShader has run
					if(texel.a <= 0.1f)
					{
						continue;
					}
				
					imageStore(outTexture, screen, texel);
					imageStore(outDepth, screen, vec4(spriteData[i].spriteDepth, 0, 0, 0));
					curDepth = spriteData[i].spriteDepth;
				}
			}
		}
	}
}