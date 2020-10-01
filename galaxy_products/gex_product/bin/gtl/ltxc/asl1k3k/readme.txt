The ASL 1k3k using Microsoft Visual 6 on WinNT has to use the usual win32 libs available in gtl/lib/win32.

1 - add the path to GTL header in Project settings : "Additional ressource include dirs"
2 - add these lines to link with the correct gtl: 
#ifndef DYNGTL
	// static linking : no need of dll
	#pragma comment (lib, "c:/galaxy/lib/libgtl_core.a")		
	#pragma comment (lib, "c:/galaxy/lib/libgstdl.a")		
#else
	// Dynamic linking: needs gtl.dll in a registered folder 
	#pragma comment (lib, "c:/galaxy/lib/libgtl.a")
#endif	
3 - add DYNGTL in Project settings "Preprocessor definitions"
4 - add these few minimalist lines to open GTL somewhere in your code:
	gtl_set(GTL_KEY_OUTPUT_FOLDER, "c:/Galaxy");
	gtl_set(GTL_KEY_MAX_NUMBER_OF_ACTIVE_SITES, "2");
	gtl_set(GTL_KEY_SITES_NUMBERS, "0 1 2 3");
	gtl_set(GTL_KEY_CONFIG_FILE, "c:/Galaxy/gtl.conf");
	gtl_set(GTL_KEY_RECIPE_FILE, "c:/Galaxy/myrecipe.csv");
	int nStatus = gtl_command("open");

5 - Fix MSVC: It seems that Visual Studio 6 has no C implementation of  these 2 functions : copy/paste the code above in a c/cpp file of your project to bypass any build error: 
	// to prevent some link issues on VisualC 6
	extern "C"
	{
		int snprintf(char *s1, size_t size, const char * fmt, ...)
		{
			va_list ap;
			va_start( ap, fmt );
			int r=_snprintf(s1, size, fmt, ap);
			va_end( ap );
			return r;
		}
		int vsnprintf(char *s1, size_t size, const char *fmt, va_list ap)
		{
			int r=_vsnprintf(s1, size, fmt, ap);
			return r;
		}
	}

6 - Build: 
7 - copy paste gtl.dll and libwinpthread.dll into for example c:/asl_nt/system/bin
8 - Run: GTL should try to open/initialize and returns 5 if GTM server not found.
