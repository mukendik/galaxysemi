Using gtl with Microsoft VisualStudio :

- static GTL : WARNING: MSVC cannot link with static gtl (libgtl_core.a) anymore.

- dynamic GTL : MSVC can usually link with the included gcc based dll interface libraries "libgtl.a": just add #pragma comment(lib,"libgtl.a") and define DYNGTL
	If not :
	- create a tiny .lib static interface library. For that, Microsoft VisualStudio provides a tool called 'LIB' tool. The provided scripts ('create_msvc_lib.bat') should create this lib with correct arguments (i386 for 32bit and x64 for 64bit). 
	- in your program, you simply have to link with this tiny library file in order for the executable to use the dll version of the GTL.  Example: #pragma comment(lib,"gtl.lib").
	- Sadly, MSVC does not have some C wrappers of some std functions (snprintf,...). Add msvc_fix.cpp to your project in order to bypass it.  
	- copy/paste gtl.dll in your exec folder or in your PATH. 
 
Steps to integrate GTL to a MSVC project:
- add the GTL header path to MSVC settings: in Project "Properties": "C/C++" : "Additional Include Dirs": add a folder pointing to the (GTM install foder)/gtl/include
- test by adding somewhere in your sources: #include <gtl_core.h>
- check that compilation is ok
- linking with gtl.lib : go to : Your project > Properties > Linker > Input > Additional deps > add 'gtl.lib'
- add the path to the lib file : Your project > Properties > Linker > General > Additional Lib dirs : (GTM INSTALL DIR)>/gtl/lib/wg1
- add DYNGTL : Your project > Properties > C/C++ > Preprocessor > add 'DYNGTL'
