#pragma once
#include <SDL.h>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <iostream>

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

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

class SDL_Window;
class SDL_Renderer;
namespace Spear
{
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
		static void LaunchGameloop();
		static void SignalShutdown();
		static void Cleanup();

	private:
		static bool m_shutdown;
		static SDL_Window* m_window;
		static SDL_Renderer* m_renderer;
	};
}