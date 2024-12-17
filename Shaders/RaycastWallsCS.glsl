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
	float fov;
	float fovWallMultiplier;
	
	vec2 viewPos;
	vec2 viewForward;
	
	vec2 screenPlaneEdgeL;
	vec2 screenPlaneEdgeR;
	vec2 screenPlane;
	
	vec2 raySpacingDir;
	float raySpacingLength;
	
	vec2 fovMinAngle;
	vec2 fovMaxAngle;
};

// UBOs
layout(std140) uniform raycasterConfig { RaycasterConfig rayConfig; };
layout(std140) uniform frameData { FrameData frame; };

// BINDINGS
layout(rgba8, binding = 0) uniform image2D outTexture;
layout(r8, binding = 1) uniform image2D outDepth;
layout(std430, binding = 2) buffer GridNodes { GridNode nodes[]; };
layout(binding = 3) uniform sampler2DArray worldTextures;

// UNIFORMS
layout(location = 0) uniform ivec2 texResolution;
layout(location = 1) uniform ivec2 gridDimensions;
layout(location = 2) uniform ivec3 worldTexturesSize;
//layout(location = 0) uniform sampler2DArray textureArray;

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

const float FLT_MAX = 3.402823466e+38;

vec2 Projection(vec2 vecToProject, vec2 vecTarget)
{
	return normalize(vecTarget) * dot(vecToProject, normalize(vecTarget));
}

int CalcTexX(int rayHit, vec2 intersection, ivec2 mapCheck)
{
	int result = int((rayHit == RAY_HIT_FRONT ? (intersection.x - mapCheck.x) : (intersection.y - mapCheck.y)) * (worldTexturesSize.x - 1));
	if (result < 0)
	{
		result += worldTexturesSize.x;
	}
	return result;
}

// Each thread = 1 Ray = 1 Vertical Strip of Pixels
layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{	
	uint screenX = gl_GlobalInvocationID.x;
	for(int screenY = 0; screenY < texResolution.y; screenY++)
	{
		imageStore(outTexture, ivec2(screenX, screenY), vec4(0,0,0,0));
		imageStore(outDepth, ivec2(screenX, screenY), vec4(FLT_MAX, 0, 0, 0));
	}
	
	
	vec2 rayStart = frame.viewPos;
	vec2 rayEnd = frame.screenPlaneEdgeL - (frame.raySpacingDir * frame.raySpacingLength * screenX);
	vec2 rayDir = normalize(rayEnd - rayStart);

	vec2 rayStepSize = vec2(sqrt(1 + (rayDir.y / rayDir.x) * (rayDir.y / rayDir.x)),	// length required to travel 1 full X unit (edge-to-edge) in Ray Direction
							sqrt(1 + (rayDir.x / rayDir.y) * (rayDir.x / rayDir.y)));	// Length required to travel 1 full Y unit (edge-to-edge) in Ray Direction
							
	ivec2 mapCheck = ivec2(rayStart); // mapcheck is based on indexes - we don't need float/fractional data
	vec2 rayLength1D; // length of ray along each axis: via x units, via y units (maybe rename this to rayUnitsPerAxis)
	ivec2 rayStep;
	
	// CALCULATE INITIAL RAY LENGTH/DIRECTION TO FIRST EDGE
	// First 'edge' (X)
	if (rayDir.x < 0)
	{
		rayStep.x = -1;

		// calculate ray length needed to reach left edge. Example: rayStart position of (7.33) - 7 = 0.33 (ie. we are 33% across this tile)
		// 0.33 * rayStepSize = length needed to reach the edge of the DDA tile from our current position
		rayLength1D.x = (rayStart.x - mapCheck.x) * rayStepSize.x;
	}
	else
	{
		rayStep.x = 1;
		rayLength1D.x = ((mapCheck.x + 1) - rayStart.x) * rayStepSize.x;
	}
	// First 'edge' (Y)
	if (rayDir.y < 0)
	{
		rayStep.y = -1;
		rayLength1D.y = (rayStart.y - mapCheck.y) * rayStepSize.y;
	}
	else
	{
		rayStep.y = 1;
		rayLength1D.y = ((mapCheck.y + 1) - rayStart.y) * rayStepSize.y;
	}
	
	// DETERMINE RAY LENGTH UNTIL HIT (DDA)
	int rayEncounters = 0; // track how many walls this ray has encountered (used to render tall walls behind short walls)
	int rayHit = RAY_NOHIT;
	float rayDistance = 0;
	int wallNodeIndex = 0;
	while (rayEncounters < rayConfig.rayEncounterLimit && rayDistance < rayConfig.farClip)
	{
		rayHit = RAY_NOHIT;
		GridNode node;
		while (rayHit == RAY_NOHIT && rayDistance < rayConfig.farClip)
		{
			bool side = false;
			if (rayLength1D.x < rayLength1D.y)
			{
				// X distance is currently shortest, increase X
				mapCheck.x += rayStep.x;
				rayDistance = rayLength1D.x;
				rayLength1D.x += rayStepSize.x; // increase ray by 1 X unit

				side = true;
			}
			else
			{
				// Y distance is currently shortest, increase Y
				mapCheck.y += rayStep.y;
				rayDistance = rayLength1D.y;
				rayLength1D.y += rayStepSize.y; // increase ray by 1 Y unit
			}

			// Check position is within range of array
			if(mapCheck.x < gridDimensions.x && mapCheck.y < gridDimensions.y && mapCheck.x >= 0 && mapCheck.y >= 0)
			{
				wallNodeIndex = mapCheck.x + (mapCheck.y * gridDimensions.x);
				node = nodes[wallNodeIndex];
					
				// if tile has a wall texture and is tall enough to be visible...
				const bool wallExists = node.texIdWall != TEX_NONE || (node.extendUp != 0 && node.texIdRoofA != TEX_NONE) || (node.extendDown != 0 && node.texIdFloorA != TEX_NONE);
				if (wallExists)
				{
					rayHit = side ? RAY_HIT_SIDE : RAY_HIT_FRONT;
					rayEncounters++;
				}
			}
		}
		
		if (rayHit != RAY_NOHIT)
		{
			vec2 intersection = rayStart + rayDir * rayDistance;
			float depth = length(Projection(intersection - frame.viewPos, frame.viewForward * rayConfig.farClip));
			float renderDepth = depth / FLT_MAX;
			const int halfHeight = int((1 + (rayConfig.yResolution / 2) / depth) * frame.fovWallMultiplier);
			const int fullHeight = halfHeight * 2;

			//int mid = int(frame.viewPitch + (rayConfig.yResolution / 2));
			int mid = int(frame.viewPitch + (texResolution.y / 2));
			int bottom = mid - halfHeight;
			int top = mid + halfHeight;

			// Draw textured vertical line segments for wall
			bool bWallFinished = false;
			int renderingUp = 0;
			int renderingDown = 0;

			int texX = -1;
			int wallTexture = TEX_NONE;

			if (node.texIdWall != TEX_NONE)
			{				
				// X Index into WallTexture = x position inside cell
				texX = CalcTexX(rayHit, intersection, mapCheck);
				wallTexture = node.texIdWall;
			}
			
			while (!bWallFinished)
			{
				// Loop to draw various wall strips. First used to draw 'core' wall strip. Then, upper wall strips, followed by lower wall strips.
				for (int screenY = max(0, bottom); screenY <= top; screenY++)
				{
					// Respect any drawFlags specified in editor 
					if (node.drawFlags != DRAW_DEFAULT)
					{
						if (rayHit == RAY_HIT_SIDE)
						{
							if (rayDir.x > 0 && (node.drawFlags & DRAW_W) == 0)
							{
								break;
							}
							if (rayDir.x < 0 && (node.drawFlags & DRAW_E) == 0)
							{
								break;
							}
						}
						else
						{
							if (rayDir.y > 0 && (node.drawFlags & DRAW_N) == 0)
							{
								break;
							}
							if (rayDir.y < 0 && (node.drawFlags & DRAW_S) == 0)
							{
								break;
							}
						}
					}

					// Walls can be drawn based on Floor/Wall/Roof. As such, certain visits to this loop may have no texture active and should be skipped.
					if (wallTexture == -1)
					{
						break;
					}

					// If we go off the bottom of the screen, skip any pixels remaining in wall strip.
					//if (screenY >= rayConfig.yResolution)
					if (screenY >= texResolution.y)
					{
						break;
					}

					// Draw only if our depth is nearer than any existing pixel
					float existingDepth = vec4(imageLoad(outDepth, ivec2(screenX, screenY))).r;
					if (renderDepth < existingDepth)
					{
						// Y Index into WallTexture = percentage through current Y forloop
						int texY = (worldTexturesSize.y - 1) - int((float(screenY - bottom) / (top - bottom)) * (worldTexturesSize.y - 1));
						
						vec3 texCoord = vec3(float(texX) / textureSize(worldTextures, 0).x,
											float(texY) / textureSize(worldTextures, 0).y,
											float(wallTexture));
						vec4 texel = texture(worldTextures, texCoord);
						
						// No sense updating the depth array for an invisible pixel, so early out when opacity is 0 - useful for cutout textures (fences, chainlink, etc.)
						if(texel.a == 0)
						{
							continue;
						}
						
						
						imageStore(outTexture, ivec2(screenX, screenY), texel);
						imageStore(outDepth, ivec2(screenX, screenY), vec4(renderDepth, 0, 0, 0));
					}
				}
				
				// Prepare data for next iteration to render upper wall strips.
				if (node.extendUp > renderingUp++)
				{
					bottom = top + 1;
					top = bottom + fullHeight;
					
					if (node.texIdRoofA != TEX_NONE)
					{
						wallTexture = node.texIdRoofA;
					}
					else if (node.texIdWall != TEX_NONE)
					{
						wallTexture = node.texIdWall;
					}
					else
					{
						wallTexture = TEX_NONE;
					}

					if (texX == -1 && wallTexture != TEX_NONE)
					{
						texX = CalcTexX(rayHit, intersection, mapCheck);
					}

					if (wallTexture != TEX_NONE && renderingUp == 1 && node.texIdWall == TEX_NONE)
					{
						// TODO: Seam Correction logic
						//const Uint32* correctivePixel = reinterpret_cast<Uint32*>(static_cast<Uint8*>(pWallTexture->pixels) + ((pWallTexture->w - 1) * pWallTexture->pitch) + (texX * pWallTexture->format->BytesPerPixel));
						//Uint8 r, g, b, a;
						//SDL_GetRGBA(*correctivePixel, pWallTexture->format, &r, &g, &b, &a);
						//FixSeams(bottom, -1, r | (g << 8) | (b << 16) | (a << 24));
					}
				}
				// Prepare data for next iteration to render lower wall strips.
				else if (node.extendDown > renderingDown++)
				{
					if (renderingDown == 1)
					{
						bottom = mid - halfHeight;
						top = mid + halfHeight;
					}
					top = bottom - 1;
					bottom = top - fullHeight;
					
					if (node.texIdFloorA != TEX_NONE)
					{
						wallTexture = node.texIdFloorA;
					}
					else if (node.texIdWall != TEX_NONE)
					{
						wallTexture = node.texIdWall;
					}
					else
					{
						wallTexture = TEX_NONE;
					}

					if (texX == -1 && wallTexture != TEX_NONE)
					{
						texX = CalcTexX(rayHit, intersection, mapCheck);
					}

					if (wallTexture != TEX_NONE && renderingDown == 1 && node.texIdWall == TEX_NONE)
					{
						// TODO: Seam Correction logic
						//const Uint32* correctivePixel = reinterpret_cast<Uint32*>(static_cast<Uint8*>(pWallTexture->pixels) + (texX * pWallTexture->format->BytesPerPixel));
						//Uint8 r, g, b, a;
						//SDL_GetRGBA(*correctivePixel, pWallTexture->format, &r, &g, &b, &a);
						//FixSeams(top, 1, r | (g << 8) | (b << 16) | (a << 24));
					}
				}
				else
				{
					bWallFinished = true;
				}
			}
		}
	}
}