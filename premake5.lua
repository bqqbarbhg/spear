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

workspace "spear"
	configurations { "debug", "develop", "release" }
	platforms { "x86", "x64", "js" }
	location "proj"
	includedirs { "src" }
	targetdir "build/%{cfg.buildcfg}_%{cfg.platform}"
    startproject "test"
    flags { "MultiProcessorCompile" }

	if cppdialect ~= nil then
		cppdialect "C++11"
	else
		flags { "C++11" }
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

	filter "configurations:relase"
		defines { "NDEBUG" }
		optimize "On"
        flags { "LinkTimeOptimization" }

	filter "platforms:x86"
		architecture "x86"

	filter "platforms:x64"
		architecture "x86_64"

	filter "platforms:js"
		architecture "js"

project "spear"
	kind "WindowedApp"
	language "C++"
    files { "src/**.h", "src/**.cpp", "src/**.c" }
    files { "misc/*.natvis" }

