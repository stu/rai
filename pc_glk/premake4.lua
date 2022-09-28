solution "interp"
	configurations { "Debug", "Release" }
		configuration "Debug"
			targetdir "debug"
			defines { "DEBUG", "MEMWATCH"}
			flags { "Symbols" }

		configuration "Release"
			targetdir "release"
			defines { "NDEBUG" }
			flags { "Optimize" }

   project "rai"
		kind "WindowedApp"
		language "C"
		targetdir "."
		files { "main.h", "main.c" }
	 	includedirs { "." }
		objdir "obj"

		configuration "linux"
            buildoptions{"-U_FORTIFY_SOURCE", "-D_FORTIFY_SOURCE=0"}
			defines { "BUILD_UNIXLIKE" }
			--"pthread","gtk-x11-2.0","atk-1.0","gio-2.0","pangoft2-1.0","freetype","gdk-x11-2.0","pangocairo-1.0","gdk_pixbuf-2.0","m","pango-1.0","cairo","gmodule-2.0","png16","gobject-2.0","gthread-2.0","rt","glib-2.0","fontconfig"
			links { "garglkmain", "garglk","stdc++"  }
			files { "unix_glkterm.c", "unix_glkterm.h" }
			excludes { "win32_glk.c", "win32_glk.h", "win32_glkstart.c" }
			includedirs { "/usr/local/include/garglk" }
            		--libdirs { "/usrlocal/lib/garglk/" }

		configuration "windows"
			--kind "WindowedApp"
			defines { "BUILD_WIN32", "_WINDOWS" }
			links { "winglk", "iberty", "kernel32", "z" }

			files { "win32_glk.c", "win32_glkstart.c" }
			includedirs { "/usr/local/include/winglk" }
			libdirs { "/usr/local/lib/winglk" }
			excludes { "unix_glkterm.c", "unix_glkterm.h" }

	project "decomp"
		kind "ConsoleApp"
		language "C"
		targetdir "."
		files { "decompile.c" }

if _ACTION == "clean" then
	os.rmdir "obj"
	os.rmdir "debug"
	os.rmdir "release"
end
