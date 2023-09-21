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
#include "MathVector.h"

#ifdef _DEBUG
#define GLCheck(x) GLClearErrors(); x; GLPrintErrors(__FILE__, #x, __LINE__);
#define GLDumpErrors(x) GLPrintErrors(__FILE__, x, 0);
#define LOG(mssg) std::cout << mssg << std::endl;
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
#endif

#define PI 3.14159265359f

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
		static void RunGameloop(int targetFPS);
		static void SignalShutdown();
		static void Cleanup();

		static Vector2D GetWindowSize();
		static float GetWindowScale();

		static Vector2D GetNormalizedDeviceCoordinate(const Vector2D& inCoord);

	private:
		static bool m_shutdown;
		static WindowParams m_windowParams;
	};
}