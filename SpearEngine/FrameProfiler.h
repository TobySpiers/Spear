#pragma once
#include "Core.h"

#ifdef _DEBUG
#define START_PROFILE(x) FrameProfiler::StartCategory(x);
#define END_PROFILE(x) FrameProfiler::EndCategory(x);
#else
#define START_PROFILE(x)
#define END_PROFILE(x)
#endif

class FrameProfiler
{
	NO_CONSTRUCT(FrameProfiler);

public:
	static void Initialise();
	static void SaveProfile(const char* outputFilename = "profile.csv");

	static void StartFrame(u64 curTime);
	static void EndFrame(u64 curTime);

	static void StartCategory(const char* catName);
	static void EndCategory(const char* catName);

};