#pragma once
//third party libraries
#include <SDL.h>
#include <glad/glad.h>

// common includes
#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <iostream>

// common custom classes
#include "MathVector.h"
#include "Colour.h"
#include "EventID.h"

#ifdef _DEBUG
#define GLCheck(x) GLClearErrors(); x; GLPrintErrors(__FILE__, #x, __LINE__);
#define GLDumpErrors(x) GLPrintErrors(__FILE__, x, 0);
#define LOG(mssg) std::cout << "\n" << mssg << std::endl;
#else
#define GLCheck(x) x;
#define GLDumpErrors(x)
#define LOG(mssg)
#endif

// Remove ability for class/struct to be constructed (only static members allowed)
#define NO_CONSTRUCT(classname)						\
classname() = delete;                               \
classname(const classname&) = delete;               \
classname& operator=(const classname&) = delete;    \
~classname() = delete;

// Remove ability for class/struct to be copied
#define NO_COPY(classname)							\
classname(const classname&) = delete;               \
classname& operator=(const classname&) = delete;

// Custom assert
#ifdef _DEBUG
#define ASSERT(x)			\
if (!(x))					\
{							\
	__debugbreak();			\
}		
#else
#define ASSERT(x)
#endif

#ifdef _DEBUG
#define AL_CATCH_ERROR()\
{\
const ALenum alError = alGetError();\
if(alError)\
{\
	std::cout << alError << std::endl;\
	__debugbreak();\
}\
}
#else
#define AL_CATCH_ERROR()
#endif

#define PI 3.1415927f
#define TO_RADIANS(x) ((x) * (PI / 180))

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

struct SDL_Window;
struct SDL_Renderer;
namespace Spear
{
	void GLClearErrors();
	void GLPrintErrors(const char* file, const char* function, int line);
	
	struct WindowParams
	{
		float scale{1.f};
		const char* title;
		int xpos, ypos;
		int width, height;
		bool fullscreen;
	};

	class Core
	{
		NO_CONSTRUCT(Core);

	public:
		static void Initialise(const WindowParams& params);
		static void RunGameloop(int targetFPS = 0);
		static void SignalShutdown();
		static void Cleanup();

		static Vector2i GetWindowSize();
		static float GetWindowScale();

		static Vector2f GetNormalizedDeviceCoordinate(const Vector2f& inCoord);
		static Vector2f GetNormalizedDeviceCoordinate(const Vector2f& inCoord, const Vector2f& viewport);

	private:
		static bool m_shutdown;
		static WindowParams m_windowParams;
	};
}