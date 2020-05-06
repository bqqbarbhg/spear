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

workspace "spear"
	configurations { "debug", "develop", "release" }
	platforms { "x86", "x64", "wasm" }
	location "proj"
	includedirs { "src" }
	targetdir "build/%{cfg.buildcfg}_%{cfg.platform}"
    startproject "test"
	flags { "MultiProcessorCompile" }
	staticruntime "on"

	if cppdialect ~= nil then
		cppdialect "C++14"
	else
		flags { "C++14" }
	end

	filter "action:vs*"
		systemversion(os.winSdkVersion() .. ".0")

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

	filter "platforms:x86"
		architecture "x86"

	filter "platforms:x64"
		architecture "x86_64"

	filter { "platforms:not wasm",  "system:windows" }
		libdirs { "dep/lib/windows_%{cfg.platform}" }
		includedirs { "dep/include/windows" }
		links {
			"ws2_32.lib",
			"crypt32.lib",
			"libcurl",
			"zlib",
		}

	filter { "platforms:not wasm",  "system:linux" }
		linkoptions "-pthread"
		links {
			"curl",
			"z",
			"dl",
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
		linkoptions { "-s INITIAL_MEMORY=268435456" }
		defines { "SP_USE_WEBGL2=1" }

	filter { "platforms:wasm", "options:wasm-simd" }
		buildoptions { "-msimd128" }
		targetsuffix "-simd"
		defines { "SF_USE_WASM_SIMD=1" }

	filter { "platforms:wasm", "options:wasm-threads" }
		buildoptions { "-s USE_PTHREADS" }
		linkoptions { "-s USE_PTHREADS" }
		buildoptions { "-pthread" }
		linkoptions { "-pthread" }
		targetsuffix "-threads"

	filter { "platforms:wasm", "options:wasm-simd", "options:wasm-threads" }
		targetsuffix "-simd-threads"

project "spear"
	kind "WindowedApp"
	language "C++"
    files { "src/**.h", "src/**.cpp", "src/**.c" }
    files { "misc/*.natvis" }
	debugdir "."

	debugdir "."
