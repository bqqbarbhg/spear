--
-- emscripten.lua
--

	local p = premake
	p.tools.emscripten = {}
	local emscripten = p.tools.emscripten
	local clang = p.tools.clang
	local config = p.config

	local api = p.api

	api.addAllowed("architecture", "wasm")

--
-- Build a list of flags for the C preprocessor corresponding to the
-- settings in a particular project configuration.
--
-- @param cfg
--    The project configuration.
-- @return
--    An array of C preprocessor flags.
--

	function emscripten.getcppflags(cfg)

		-- Just pass through to clang for now
		local flags = clang.getcppflags(cfg)
		return flags

	end


--
-- Build a list of C compiler flags corresponding to the settings in
-- a particular project configuration. These flags are exclusive
-- of the C++ compiler flags, there is no overlap.
--
-- @param cfg
--    The project configuration.
-- @return
--    An array of C compiler flags.
--

	emscripten.shared = table.merge(clang.shared, {
	})

	emscripten.cflags = table.merge(clang.cflags, {
	})

	function emscripten.getcflags(cfg)
		local shared = config.mapFlags(cfg, emscripten.shared)
		local cflags = config.mapFlags(cfg, emscripten.cflags)

		local flags = table.join(shared, cflags)
		flags = table.join(flags, emscripten.getwarnings(cfg))

		return flags
	end

	function emscripten.getwarnings(cfg)
		return clang.getwarnings(cfg)
	end


--
-- Build a list of C++ compiler flags corresponding to the settings
-- in a particular project configuration. These flags are exclusive
-- of the C compiler flags, there is no overlap.
--
-- @param cfg
--    The project configuration.
-- @return
--    An array of C++ compiler flags.
--

	emscripten.cxxflags = table.merge(clang.cxxflags, {
	})

	function emscripten.getcxxflags(cfg)
		local shared = config.mapFlags(cfg, emscripten.shared)
		local cxxflags = config.mapFlags(cfg, emscripten.cxxflags)
		local flags = table.join(shared, cxxflags)
		flags = table.join(flags, emscripten.getwarnings(cfg))
		return flags
	end


--
-- Returns a list of defined preprocessor symbols, decorated for
-- the compiler command line.
--
-- @param defines
--    An array of preprocessor symbols to define; as an array of
--    string values.
-- @return
--    An array of symbols with the appropriate flag decorations.
--

	function emscripten.getdefines(defines)

		-- Just pass through to clang for now
		local flags = clang.getdefines(defines)
		return flags

	end

	function emscripten.getundefines(undefines)

		-- Just pass through to clang for now
		local flags = clang.getundefines(undefines)
		return flags

	end



--
-- Returns a list of forced include files, decorated for the compiler
-- command line.
--
-- @param cfg
--    The project configuration.
-- @return
--    An array of force include files with the appropriate flags.
--

	function emscripten.getforceincludes(cfg)

		-- Just pass through to clang for now
		local flags = clang.getforceincludes(cfg)
		return flags

	end


--
-- Returns a list of include file search directories, decorated for
-- the compiler command line.
--
-- @param cfg
--    The project configuration.
-- @param dirs
--    An array of include file search directories; as an array of
--    string values.
-- @return
--    An array of symbols with the appropriate flag decorations.
--

	function emscripten.getincludedirs(cfg, dirs, sysdirs)

		-- Just pass through to clang for now
		local flags = clang.getincludedirs(cfg, dirs, sysdirs)
		return flags

	end

	emscripten.getrunpathdirs = clang.getrunpathdirs

--
-- get the right output flag.
--
	function emscripten.getsharedlibarg(cfg)
		return clang.getsharedlibarg(cfg)
	end

--
-- Build a list of linker flags corresponding to the settings in
-- a particular project configuration.
--
-- @param cfg
--    The project configuration.
-- @return
--    An array of linker flags.
--

	emscripten.ldflags = {
		architecture = {
			wasm = "",
		},
		flags = {
			LinkTimeOptimization = "-flto",
		},
		kind = {
			SharedLib = function(cfg)
				local r = { emscripten.getsharedlibarg(cfg) }
				return r
			end,
		},
		system = {
			wii = "$(MACHDEP)",
		},
		optimize = {
			On = "-O2",
			Size = "-Os",
		},
	}

	function emscripten.getldflags(cfg)
		local flags = config.mapFlags(cfg, emscripten.ldflags)
		return flags
	end



--
-- Build a list of additional library directories for a particular
-- project configuration, decorated for the tool command line.
--
-- @param cfg
--    The project configuration.
-- @return
--    An array of decorated additional library directories.
--

	function emscripten.getLibraryDirectories(cfg)

		-- Just pass through to clang for now
		local flags = clang.getLibraryDirectories(cfg)
		return flags

	end


--
-- Build a list of libraries to be linked for a particular project
-- configuration, decorated for the linker command line.
--
-- @param cfg
--    The project configuration.
-- @param systemOnly
--    Boolean flag indicating whether to link only system libraries,
--    or system libraries and sibling projects as well.
-- @return
--    A list of libraries to link, decorated for the linker.
--

	function emscripten.getlinks(cfg, systemonly, nogroups)
		return clang.getlinks(cfg, systemonly, nogroups)
	end


--
-- Return a list of makefile-specific configuration rules. This will
-- be going away when I get a chance to overhaul these adapters.
--
-- @param cfg
--    The project configuration.
-- @return
--    A list of additional makefile rules.
--

	function emscripten.getmakesettings(cfg)

		-- Just pass through to clang for now
		local flags = clang.getmakesettings(cfg)
		return flags

	end


--
-- Retrieves the executable command name for a tool, based on the
-- provided configuration and the operating environment. I will
-- be moving these into global configuration blocks when I get
-- the chance.
--
-- @param cfg
--    The configuration to query.
-- @param tool
--    The tool to fetch, one of "cc" for the C compiler, "cxx" for
--    the C++ compiler, or "ar" for the static linker.
-- @return
--    The executable command name for a tool, or nil if the system's
--    default value should be used.
--

	emscripten.tools = {
		cc = "emcc",
		cxx = "em++",
		ar = "emar"
	}

	function emscripten.gettoolname(cfg, tool)
		return emscripten.tools[tool]
	end


	filter { "architecture:wasm" }
		toolset "emscripten"

	filter { "architecture:wasm" }
		targetextension ".js"

	filter { "architecture:wasm", "kind:StaticLib" }
		targetextension ".bc"

	filter {}
