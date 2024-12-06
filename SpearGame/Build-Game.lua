project "SpearGame"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	targetdir "Binaries/%{cfg.buildcfg}"
	staticruntime "off"

	files 
	{
		"Source/**.h",
		"Source/**.cpp"
	}

	includedirs
	{
		"Source",
		"../SpearEngine/Source",
	}

	links
	{
		"SDL2",
		"SDL2main",
		"SDL2_image",
		"OpenAL32",
		"sndfile",
		"SpearEngine"
	}

	libdirs
	{
		"../SpearEngine/ThirdParty/SDL2-2.28.3/lib/x64",
		"../SpearEngine/ThirdParty/openal-soft-1.23.1/lib/x64",
		"../SpearEngine/ThirdParty/libsndfile-1.2.2/lib/x64",
	}
	
	-- Necessary to enable GameObject's self-registration features when compiled in static library (see: https://www.cppstories.com/2018/02/static-vars-static-lib/)
	linkoptions {"/WHOLEARCHIVE:SpearEngine"}
	
	-- Copy any dlls necessary in build output directories
    postbuildcommands
	{
        '{COPY} ../SpearEngine/ThirdParty/dlls/x64/SDL2.dll "%{cfg.buildtarget.directory}"',
		'{COPY} ../SpearEngine/ThirdParty/dlls/x64/SDL2_image.dll "%{cfg.buildtarget.directory}"',
		'{COPY} ../SpearEngine/ThirdParty/dlls/x64/OpenAL32.dll "%{cfg.buildtarget.directory}"',
		'{COPY} ../SpearEngine/ThirdParty/dlls/x64/sndfile.dll "%{cfg.buildtarget.directory}"'
    }

	targetdir ("../Binaries/" .. OutputDir .. "/%{prj.name}")
	objdir ("../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

	filter "system:windows"
		systemversion "latest"
		defines { "WINDOWS" }

	filter "configurations:Debug"
		defines { "DEBUG" }
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines { "RELEASE" }
		runtime "Release"
		optimize "On"
		symbols "On"

	filter "configurations:Shipping"
		defines { "DIST" }
		runtime "Release"
		optimize "On"
		symbols "Off"
		kind "WindowedApp"