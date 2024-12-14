#pragma once
#include "Core.h"

#ifdef _DEBUG
#define START_PROFILE(x) FrameProfiler::StartCategory(x);
#define END_PROFILE(x) FrameProfiler::EndCategory(x);

struct ProfileCategory
{
	const char* categoryName;
	u64 startTimestamp{ 0 };
	float durationMs{ 0.f };
		
	ProfileCategory* parentCategory{ nullptr };
	std::vector<ProfileCategory> childCategories;
};

class FrameProfiler
{
	NO_CONSTRUCT(FrameProfiler);

public:
	static void StartFrame(u64 curTime);
	static void StartCategory(const char* catName);
	static void EndCategory(const char* catName);

	static const std::vector<ProfileCategory>& GetData();
	static float GetCurrentFrameMs();

private:
	static u64 s_frameStart;
	static std::vector<ProfileCategory> s_categoryTrees;
	static ProfileCategory* s_activeCategory;
};

#else
#define START_PROFILE(x);
#define END_PROFILE(x);
#endif