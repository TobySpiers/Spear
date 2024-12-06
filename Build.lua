-- premake5.lua
workspace "Spear"
	architecture "x64"
	configurations { "Debug", "Release", "Shipping" }
	startproject "SpearGame"

	-- Workspace-wide build options for MSVC
	filter "system:windows"
		buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }
	
	-- Workspace-wide ThirdParty includes (as these are used by both Engine & Game, makes sense to declare these here)
	includedirs
	{
		"SpearEngine/ThirdParty/Imgui",
		"SpearEngine/ThirdParty/glad/include",
		"SpearEngine/ThirdParty/SDL2-2.28.3/include",
		"SpearEngine/ThirdParty/openal-soft-1.23.1/include",
		"SpearEngine/ThirdParty/libsndfile-1.2.2/include",
	}

OutputDir = "%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"

group "Engine"
	include "SpearEngine/Build-Engine.lua"
group ""

include "SpearGame/Build-Game.lua"