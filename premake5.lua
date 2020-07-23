require("./emscripten")

function os.winSdkVersion()
	local reg_arch = iif( os.is64bit(), "\\Wow6432Node\\", "\\" )
	local sdk_version = os.getWindowsRegistry( "HKLM:SOFTWARE" .. reg_arch .."Microsoft\\Microsoft SDKs\\Windows\\v10.0\\ProductVersion" )
	if sdk_version ~= nil then return sdk_version end
	return ""
end

function symbols_on()
	if symbols ~= nil then
		symbols "On"
	else
		flags { "Symbols" }
	end
end

newoption {
   trigger     = "opengl",
   description = "OpenGL 3.3 instead of a native API"
}

newoption {
   trigger     = "wasm-simd",
   description = "Use WASM 128-bit SIMD extension"
}

newoption {
   trigger     = "wasm-threads",
   description = "Use WASM threads extension"
}

newoption {
   trigger     = "dedicated-processor",
   description = "Build a dedicated resource processor"
}

newoption {
   trigger     = "dedicated-server",
   description = "Build a dedicated server"
}

workspace "spear"
	configurations { "debug", "develop", "release" }

	location "proj"
	includedirs { "src" }
	sysincludedirs { "src/ext" }
	targetdir "build/%{cfg.buildcfg}_%{cfg.platform}"
    startproject "test"
	flags { "MultiProcessorCompile" }
	staticruntime "on"
	exceptionhandling "Off"
	rtti "Off"

	if cppdialect ~= nil then
		cppdialect "C++14"
	else
		flags { "C++14" }
	end

	filter "not action:xcode4"
		platforms { "x86", "x64", "wasm" }

	filter "action:xcode4"
		links {
			"Foundation.framework",
			"Security.framework",
			"Metal.framework",
			"MetalKit.framework",
			"QuartzCore.framework",
			"CFNetwork.framework",
		}
		buildoptions { "-fobjc-arc" }

	filter { "action:xcode4", "system:macosx" }
		links {
			"AppKit.framework",
		}

	filter { "action:xcode4", "system:ios" }
		links {
			"UIKit.framework",
		}

	filter "action:vs*"
		systemversion(os.winSdkVersion() .. ".0")

	filter { "action:vs*", "configurations:release" }
		flags { "NoBufferSecurityCheck" }

	filter "not action:vs*"
		buildoptions { "-Wno-invalid-offsetof" }

	filter "configurations:debug"
		defines { "DEBUG" }
		symbols_on()

	filter "configurations:develop"
		defines { "NDEBUG" }
		optimize "On"
		symbols_on()

	filter "configurations:release"
		defines { "NDEBUG" }
		optimize "On"
        flags { "LinkTimeOptimization" }

	filter { "configurations:debug", "platforms:wasm" }
		linkoptions { "-g" }

	filter "platforms:x86"
		architecture "x86"
		vectorextensions "SSE4.1"

	filter "platforms:x64"
		architecture "x86_64"
		vectorextensions "SSE4.1"

	filter { "platforms:not wasm",  "system:windows" }
		libdirs { "dep/lib/windows_%{cfg.platform}" }
		includedirs { "dep/include/windows" }
		defines { "BQWS_PT_USE_OPENSSL=1" }
		links {
			"ws2_32.lib",
			"crypt32.lib",
			"libcurl",
			"zlib",
			"ssleay32",
			"libeay32",
		}

	filter { "platforms:not wasm", "system:linux" }
		linkoptions "-pthread"
		toolset "clang"

	filter { "platforms:not wasm", "system:linux", "not options:dedicated-processor" }
		links {
			"curl",
			"z",
			"dl",
			"ssl",
			"crypto",
		}

	filter { "platforms:not wasm", "system:linux", "not options:dedicated-processor", "not options:dedicated-server" }
		links {
			"GL",
			"X11",
		}

	filter "options:opengl"
		defines { "SP_USE_OPENGL=1" }

	filter "platforms:wasm"
		architecture "wasm"
		buildoptions { "-s MIN_WEBGL_VERSION=2" }
		buildoptions { "-s MAX_WEBGL_VERSION=2" }
		linkoptions { "-s MIN_WEBGL_VERSION=2" }
		linkoptions { "-s MAX_WEBGL_VERSION=2" }
		linkoptions { "-s WASM=1" }
		linkoptions { "-s INITIAL_MEMORY=536870912" }
		defines { "SP_USE_WEBGL2=1" }
		objdir "proj/obj/wasm/%{cfg.buildcfg}"

	filter { "platforms:wasm", "options:wasm-simd" }
		buildoptions { "-msimd128" }
		targetsuffix "-simd"
		defines { "SF_USE_WASM_SIMD=1" }
		objdir "proj/obj/wasm-simd/%{cfg.buildcfg}"

	filter { "platforms:wasm", "options:wasm-threads" }
		buildoptions { "-s USE_PTHREADS" }
		linkoptions { "-s USE_PTHREADS" }
		buildoptions { "-pthread" }
		linkoptions { "-pthread" }
		targetsuffix "-threads"
		objdir "proj/obj/wasm-threads/%{cfg.buildcfg}"

	filter { "platforms:wasm", "options:wasm-simd", "options:wasm-threads" }
		targetsuffix "-simd-threads"
		objdir "proj/obj/wasm-simd-threads/%{cfg.buildcfg}"

	filter { "options:dedicated-processor" }
		defines { "SP_DEDICATED_PROCESSOR=1", "SP_NO_APP=1" }
		targetsuffix "-processor"
		objdir "proj/obj/processor/%{cfg.platform}_%{cfg.buildcfg}"

	filter { "options:dedicated-server" }
		defines { "SP_DEDICATED_SERVER=1", "SP_NO_APP=1" }
		targetsuffix "-server"
		objdir "proj/obj/server/%{cfg.platform}_%{cfg.buildcfg}"

project "spear"
	kind "WindowedApp"
	language "C++"
    files { "src/**.h", "src/**.cpp", "src/**.c" }
    files { "misc/*.natvis" }
	debugdir "../dealers-dungeon"
	filter { "action:xcode4" }
		symbolspath "build/%{cfg.configuration}"
		files { "src/**.m" }
	filter { "system:ios" }
		files { "misc/ios/Info.plist" }
	filter { "system:macosx" }
		files { "misc/macos/Info.plist" }
	filter { "options:dedicated-processor" }
		kind "ConsoleApp"
	filter { "options:dedicated-server" }
		kind "ConsoleApp"

