workspace "ShadowMappingWorkspace"
	configurations	{ "Debug", "Release" }
	platforms		{ "x64" }
	location		"build"
	characterset	"MBCS"
	startproject	"ShadowMapping"

	filter "configurations:Debug"
		symbols		"on"

project "ShadowMapping"
	kind		"WindowedApp"
	location	"build/ShadowMapping"
	debugdir	"ShadowMapping"

	files {
		"ShadowMapping/Data/**.vert",
		"ShadowMapping/Data/**.frag",
		"ShadowMapping/Source/**.cpp",
		"ShadowMapping/Source/**.h",
		"Libraries/imgui/*.cpp",
		"Libraries/imgui/*.h",
		"premake5.lua",
		".gitignore",
		"GenerateProjectFiles.bat",
	}

	includedirs {
		"ShadowMapping/Source",
	}

	links {
		"opengl32",
	}

	pchheader "PCH.h"
	pchsource "ShadowMapping/Source/PCH.cpp"

	filter "files:Libraries/imgui/*.cpp"
		flags { "NoPCH" }
