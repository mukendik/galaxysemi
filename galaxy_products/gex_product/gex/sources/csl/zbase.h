// Galaxy: 30-April-2002:


#ifndef _ZBASEDEFS_
#define _ZBASEDEFS_

#define ZC_MAJOR_VERSION 4
#define ZC_MINOR_VERSION 3

// Makes sure compilation is not a DLL
#define ZC_NATIVECSLLIB

/* compiler determination */

#if defined(__IBMC__) || defined(__IBMCPP__)
   #define ZC_IBM 1          /* IBM VisualAge C++ */
   #define ZC_COMPILER "IBM"
#endif /* IBM */

#ifdef _MSC_VER
   #define ZC_MICROSOFT 1    /* Microsoft Visual C++ */
   #define ZC_COMPILER "MICROSOFT"
#endif /* MICROSOFT */

#ifdef __BORLANDC__
   #define ZC_BORLAND 1      /* Borland C++ */
   #define ZC_COMPILER "BORLAND"
#endif /* BORLAND */

#ifdef __GNUC__
   #define ZC_GNU 1          /* GNU gcc/g++ */
   #define ZC_COMPILER "GNU"
#endif /* GNU */

#if !defined(ZC_COMPILER) && defined(unix)
   #define ZC_COMPILER "UNIX"
#endif

/* platform determination */

#if ZC_IBM
   #ifdef __OS2__
      #define ZC_OS2 1
      #define ZC_PATHSEPARATOR '\\'
   #endif
   #ifdef __WINDOWS__
      #define ZC_WIN 1
      #define ZC_PATHSEPARATOR '\\'
   #endif
#endif

#if ZC_MICROSOFT
   #define ZC_WIN 1
   #define ZC_PATHSEPARATOR '\\'
#endif

#if ZC_BORLAND
   #define ZC_WIN 1
   #define ZC_PATHSEPARATOR '\\'
#endif

#if unix || __MACH__
   #if defined(__WIN32__) || defined(__CYGWIN32__)
      #define ZC_WIN 1
      #define ZC_PATHSEPARATOR '\\'
   #else
      #if defined(__linux__)
         #define ZC_LINUX 1
      #elif defined(__NetBSD__)
         #define ZC_NETBSD 1
         #define ZC_BSD 1
      #elif defined(__FreeBSD__)
         #define ZC_FREEBSD 1
         #define ZC_BSD 1
      #elif defined(sparc) && defined(sun) && defined(unix)
         #define ZC_SOLARIS 1
      #else
         #define ZC_UNIX 1
      #endif
      #define ZC_UNIXFAM 1
      #define ZC_PATHSEPARATOR '/'
      #define _MAX_PATH 2048
   #endif
#endif

/* import/export macros */

#if ZC_IBM
  #define ZImport0
  #define ZExport0 _Export
  #define ZImport(type) type
  #define ZExport(type) type _Export

  #if ZC_OS2
    #define ZFuncptrAPI * _System
    #define ZImportAPI(type) type _System
    #define ZExportAPI(type) type _System
  #endif /* ZC_OS2 */

  #ifdef ZC_WIN
    #define ZFuncptrAPI * __stdcall
    #define ZImportAPI(type) type __stdcall
    #define ZExportAPI(type) type __stdcall _Export
  #endif /* ZC_WIN */
#endif /* ZC_IBM */

#if ZC_MICROSOFT
  #define ZImport0
  #define ZExport0 __declspec(dllexport)
  #define ZImport(type) type
  #define ZExport(type) __declspec(dllexport) type
  #define ZFuncptrAPI __stdcall *
  #define ZImportAPI(type) type __stdcall
  #define ZExportAPI(type) __declspec(dllexport) type __stdcall
#endif /* ZC_MICROSOFT */

#if ZC_BORLAND
  #define ZImport0
  #define ZExport0 __declspec(dllexport)
  #define ZImport(type) type
  #define ZExport(type) __declspec(dllexport) type
  #define ZFuncptrAPI __stdcall *
  #define ZImportAPI(type) type __stdcall
  #define ZExportAPI(type) __declspec(dllexport) type __stdcall
#endif /* ZC_BORLAND */

#if unix || __MACH__
  #if ZC_WIN
    #define ZImport0
    #define ZExport0 __declspec(dllexport)
    #define ZImport(type) type
    #define ZExport(type) __declspec(dllexport) type
    #ifdef __cplusplus
      #define ZFuncptrAPI *
    #else
      #define ZFuncptrAPI * __attribute__((stdcall))
    #endif
    #define ZImportAPI(type) type __attribute__((stdcall))
    #define ZExportAPI(type) __declspec(dllexport) type __attribute__((stdcall))
  #endif /* ZC_WIN */

  #if ZC_UNIXFAM
    #define ZImport0
    #define ZExport0
    #define ZImport(type) type
    #define ZExport(type) type
    #define ZFuncptrAPI *
    #define ZImportAPI(type) type
    #define ZExportAPI(type) type
  #endif /* ZC_UNIXFAM */
#endif /* ZC_GNU */

#if _WIN32
  #if ZC_GNU
	#define ZImport0
	#define ZExport0 __declspec(dllexport)
	#define ZImport(type) type
	#define ZExport(type) __declspec(dllexport) type
	#define ZFuncptrAPI __stdcall *
	#define ZImportAPI(type) type __stdcall
	#define ZExportAPI(type) __declspec(dllexport) type __stdcall
  #endif
  #define ZC_WIN 1
  #define ZC_PATHSEPARATOR '\\'
#endif

#endif /* _ZBASEDEFS_ */
