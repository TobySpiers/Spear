#include "FrameProfiler.h"

#ifdef _DEBUG
#include <unordered_map>
#include <fstream>
#include <iomanip>
#include <cstdio>
#include "Panels/PanelFrameProfiler.h"

static PanelFrameProfiler debugPanel;

std::vector<ProfileCategory> FrameProfiler::s_categoryTrees;
ProfileCategory* FrameProfiler::s_activeCategory{ nullptr };
u64 FrameProfiler::s_frameStart{0};

void FrameProfiler::StartFrame(u64 curTime)
{
	ASSERT(!s_activeCategory);
	s_frameStart = curTime;
	s_categoryTrees.clear();
}

void FrameProfiler::StartCategory(const char* newCategoryName)
{
	ProfileCategory* newCategory{ nullptr };
	if (!s_activeCategory)
	{
		s_categoryTrees.emplace_back();
		newCategory = &s_categoryTrees.back();
	}
	else
	{
		s_activeCategory->childCategories.emplace_back();
		newCategory = &s_activeCategory->childCategories.back();
		newCategory->parentCategory = s_activeCategory;
	}

	s_activeCategory = newCategory;
	newCategory->categoryName = newCategoryName;
	newCategory->startTimestamp = SDL_GetPerformanceCounter();
}

void FrameProfiler::EndCategory(const char* categoryName)
{
	// Forbid nested categories from lasting longer than their parent category
	// This makes it easy to sum overall frame contributions and spot missing time
	ASSERT(s_activeCategory->categoryName == categoryName);
	s_activeCategory->durationMs = static_cast<float>(SDL_GetPerformanceCounter() - s_activeCategory->startTimestamp) / SDL_GetPerformanceFrequency();
	s_activeCategory->durationMs *= 1000;
	s_activeCategory = s_activeCategory->parentCategory;
}

const std::vector<ProfileCategory>& FrameProfiler::GetData()
{
	return s_categoryTrees;
}

float FrameProfiler::GetCurrentFrameMs()
{
	return 1000 * (static_cast<float>(SDL_GetPerformanceCounter() - s_frameStart) / SDL_GetPerformanceFrequency());
}

#endif