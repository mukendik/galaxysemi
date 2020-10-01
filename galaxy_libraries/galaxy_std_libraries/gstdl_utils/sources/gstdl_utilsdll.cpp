// gstdutilsdll.cpp : Implementation file
// ----------------------------------------------------------------------------------------------------------
// Copyright Galaxy
// This computer program is protected by copyright law
// and international treaties. Unauthorized reproduction or 
// distribution of this program, or any portion of it,may 
// result in severe civil and criminal penalties, and will be 
// prosecuted to the maximum extent possible under the low.
// ----------------------------------------------------------------------------------------------------------
//
// Notes: This file contains the Dll main entry point
//
// ----------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------
// Only declare a DLL entry point under Windows.
// Under UNIX system, nothing needed for a shared library.
// ----------------------------------------------------------------------------------------------------------
#if defined(_WIN32)
#	define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#	include <windows.h>

/*BOOL APIENTRY DllMain( HANDLE hModule,
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}*/
#endif // !defined(_CREATE_GSTDL_STATIC_)
