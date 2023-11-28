#include "FrameProfiler.h"

#ifdef _DEBUG
#include <unordered_map>
#include <fstream>
#include <iomanip>
#include <cstdio>

static constexpr const char* TEMP_FILENAME{"temp.csv"};

using TimingData = std::pair<u64, float>;

static std::vector<const char*> s_vCategoryEntries;
static std::unordered_map<const char*, TimingData> s_timingData;
static std::fstream s_tempFile;
static u64 s_frameStart{0};

static bool s_writeHeader{0};

void FrameProfiler::Initialise()
{
	s_writeHeader = true;

	// must create file via ofstream before opening via read/write stream
	std::ofstream createdFile(TEMP_FILENAME);
	ASSERT(createdFile.is_open());
	createdFile.close();

	s_tempFile.open(TEMP_FILENAME, std::ios::out | std::ios::in);
	ASSERT(s_tempFile.is_open())

	s_tempFile << std::fixed << std::setprecision(10);
}

void FrameProfiler::StartFrame(u64 curTime)
{
	s_frameStart = curTime;
}

void FrameProfiler::StartCategory(const char* catName)
{
	// Create new entry if new category, otherwise update existing category
	if(s_timingData.count(catName) == 0)
	{
		s_timingData.insert(std::pair<const char*, TimingData>(
			catName,
			TimingData(SDL_GetPerformanceCounter(), 0.f)
		));

		s_vCategoryEntries.push_back(catName);
	}
	else
	{
		s_timingData.at(catName) = TimingData(SDL_GetPerformanceCounter(), 0.f);
	}
}

void FrameProfiler::EndCategory(const char* catName)
{
	ASSERT(s_timingData.count(catName) == 1 && s_timingData.at(catName).first != 0);

	// Calculate and store ms since category began
	s_timingData.at(catName).second = static_cast<float>(SDL_GetPerformanceCounter() - s_timingData.at(catName).first) / SDL_GetPerformanceFrequency();
}

void FrameProfiler::EndFrame(u64 curTime)
{
	ASSERT(s_tempFile.is_open());

	// Write values for this frame to TEMP_FILE in order categories have been first created, zeroing out as we go
	s_tempFile << static_cast<float>(curTime - s_frameStart) / SDL_GetPerformanceFrequency() << ",";
	for (const char* category : s_vCategoryEntries)
	{
		TimingData& td = s_timingData.at(category);
		s_tempFile << td.second << ",";
		td = TimingData(0, 0.f);
	}

	s_tempFile << "\n";
}


void FrameProfiler::SaveProfile(const char* outputFilename)
{
	LOG(std::string("\nSaving profiling data to ") + outputFilename);

	std::string filepath = "../";
	std::ofstream saveFile(filepath + outputFilename);
	ASSERT(saveFile.is_open());

	saveFile << std::fixed << std::setprecision(10);
	
	// Write category headers into top of SaveFile
	saveFile << "Frame Total" << ",";
	for (const char* category : s_vCategoryEntries)
	{
		saveFile << category << ",";
	}
	saveFile << "\n";

	// Copy temp data into SaveFile with corrected row widths
	std::string line;
	s_tempFile.seekg(0, std::ios::beg);
	while (std::getline(s_tempFile, line))
	{
		saveFile << line;
		
		int values = std::count(line.begin(), line.end(), ',');
		while (values < s_timingData.size() + 1) // plus 1 for Frame Total category
		{
			saveFile << 0.f << ',';
			values++;
		}
		saveFile << '\n';
	}

	saveFile.close();
	s_tempFile.close();
	ASSERT(std::remove(TEMP_FILENAME) == 0); // ensure temp file is deleted

	LOG("...Profiling data succesfully saved");
}

#endif