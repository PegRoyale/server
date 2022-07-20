workspace "server"
	startproject "server"
	location "../build/"
	targetdir "%{wks.location}/bin/%{cfg.buildcfg}-%{cfg.platform}/"
	objdir "%{wks.location}/obj/%{prj.name}/%{cfg.buildcfg}-%{cfg.platform}/"
	buildlog "%{wks.location}/obj/%{cfg.platform}/%{cfg.buildcfg}-%{prj.name}.log"

	largeaddressaware "on"
	editandcontinue "off"
	staticruntime "on"

	systemversion "latest"
	characterset "unicode"
	warnings "extra"

	flags {
		"noincrementallink",
		"no64bitchecks",
		"shadowedvariables",
		"undefinedidentifiers",
		"multiprocessorcompile",
	}

	syslibdirs {
		"../deps/enet-1.3.17/",
	}

	platforms {
		"x86",
	}

	configurations {
		"Release",
		"Debug",
	}

	defines {
		"NOMINMAX",
		"WIN32_LEAN_AND_MEAN",
		"_CRT_SECURE_NO_WARNINGS",
		"_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS",
	}

	--x86
	filter "platforms:x86"	
		architecture "x86"
	--end

	filter "Release"
		defines "NDEBUG"
		optimize "full"
		runtime "debug"
		symbols "off"

	filter "Debug"
		defines "DEBUG"
		optimize "debug"
		runtime "debug"
		symbols "on"

	project "server"
		targetname "server"
		language "c++"
		cppdialect "c++17"
		kind "consoleapp"
		warnings "off"

		pchheader "stdafx.hpp"
		pchsource "../src/server/stdafx.cpp"
		forceincludes "stdafx.hpp"

		links {
			"enet",
			"ws2_32",
			"winmm",
		}

		includedirs {
			"../src/server/",
			"../deps/enet-1.3.17/include/",
		}

		files {
			"../src/server/**",
		}

	project "test-client"
		targetname "test-client"
		language "c++"
		cppdialect "c++17"
		kind "consoleapp"
		warnings "off"

		pchheader "stdafx.hpp"
		pchsource "../src/test-client/stdafx.cpp"
		forceincludes "stdafx.hpp"

		links {
			"enet",
			"ws2_32",
			"winmm",
		}

		includedirs {
			"../src/test-client/",
			"../deps/enet-1.3.17/include/",
		}

		files {
			"../src/test-client/**",
		}