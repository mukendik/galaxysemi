#define ZC_BUILDING_ZCSL

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "zcsl.hpp"
#include "zfile.hpp"
#include <gqtl_log.h>

#include <sstream>

#if ZC_UNIXFAM
  #include <sys/stat.h>
#else
  #include <sys\stat.h>
#endif

#include "internal.h"

using namespace std;

// Initialisation functions for each library.
extern void ZCslInitLib_Sys(ZCsl*);			// System library
extern void ZCslInitLib_String(ZCsl*);		// String library
extern void ZCslInitLib_Math(ZCsl*);		// Math library
extern void ZCslInitLib_File(ZCsl*);		// File library
extern void ZCslInitLib_Gex(ZCsl*);			// GEX library

// Clean Reset of libraries
extern void ZCslCleanupLib_Sys(ZCsl*);		// System library
extern void ZCslCleanupLib_String(ZCsl*);	// String library
extern void ZCslCleanupLib_Math(ZCsl*);		// Math library
extern void ZCslCleanupLib_File(ZCsl*);		// File library
extern void ZCslCleanupLib_Gex(ZCsl*);		// GEX library

ZExport0 ZCsl::ZCsl(int aFlags) :
   iStats(0),
   iCalls(0),
   iTraceMode(traceNone),
   iTraceLevel(0),
   iFlags(aFlags),
   iState(isNormal),
   iList(zFalse),
   iInDirective(zFalse),
   iInput(0)
{
   ZFUNCTRACE_DEVELOP("ZCsl::ZCsl(int aFlags)");
   iBlock = new Block(this,0);
   iStack = iTos = new ZString[iStackSize = STACKCHUNK];
   iStackLimit = &iStack[iStackSize-1];
   *iTos = "<stack bottom>";
   openStat("ZCsl");

   iBlock->addVar("true", "1", zTrue, zFalse, zTrue);
   iBlock->addVar("false", "0", zTrue, zFalse, zTrue);

   iBlock->addVar("MAXLONG", ZString((long)ZBase::maxLong), zTrue, zFalse, zTrue);
   iBlock->addVar("PATHSEPARATOR", ZString(ZC_PATHSEPARATOR), zTrue, zFalse, zTrue);

   char buf[10];
   ZString str;
   sprintf(buf, "%d.%02d", CSL_MAJOR_VERSION, CSL_MINOR_VERSION);
   str = buf;
   iBlock->addVar("cslVersion", str, zTrue, zFalse, zTrue);
   iBlock->addVar("cslCompiler", ZC_COMPILER, zTrue, zFalse, zTrue);
   iBlock->addVar("cslBuilt", ZString(__DATE__)+" "+__TIME__, zTrue, zFalse, zTrue);
#ifdef ZC_CSS_COMPATIBLE
   // these are for compatibility with CSS
   iBlock->addVar("cssVersion", str, zTrue, zFalse, zTrue);
   iBlock->addVar("cssCompiler", ZC_COMPILER, zTrue, zFalse, zTrue);
   iBlock->addVar("cssBuilt", ZString(__DATE__)+" "+__TIME__, zTrue, zFalse, zTrue);
#endif

   
   // Jan2002, Phil: Load all Libabries...no longer need to list them in the .CSL
	ZCslInitLib_Sys(this);		// System library
	ZCslInitLib_String(this);	// String library
	ZCslInitLib_Math(this);		// Math library
	ZCslInitLib_File(this);		// File library
	ZCslInitLib_Gex(this);		// GEX library
} // ZCsl

ZExport0 ZCsl::~ZCsl()
{
   ZFUNCTRACE_DEVELOP("ZCsl::~ZCsl()");
   
   // Jan2002, Phil: Clear reset all Libabries...
	ZCslCleanupLib_Sys(this);		// System library
	ZCslCleanupLib_String(this);	// String library
	ZCslCleanupLib_Math(this);		// Math library
	ZCslCleanupLib_File(this);		// File library
	ZCslCleanupLib_Gex(this);		// GEX library

   delete iBlock;
   if (iStats) delete iStats;
   if (iCalls) delete iCalls;
   if (iInput) delete iInput;
   delete [] iStack;
} // ~ZCsl

ZExport(ZCsl&) ZCsl::set(const ZString& aVarName, const ZString& aValue)
{
   ZFUNCTRACE_DEVELOP("ZCsl::set(const ZString& aVarName, const ZString& aValue)");
   Variable *v;
   if (iCalls)
      v = iCalls->findVar(aVarName);
   else
      v = iBlock->findVar(aVarName);
   v->set(aValue);
   return *this;
} // set

ZExport(ZString) ZCsl::get(const ZString& aVarName)
{
   ZFUNCTRACE_DEVELOP("ZCsl::get(const ZString& aVarName)");
   Variable *v;
   if (iCalls)
      v = iCalls->findVar(aVarName);
   else
      v = iBlock->findVar(aVarName);
   return v->value();
} // get

ZExport(long) ZCsl::varSizeof(const ZString& aVarName)
{
   ZFUNCTRACE_DEVELOP("ZCsl::varSizeof(const ZString& aVarName)");
   Variable *v;
   if (iCalls)
      v = iCalls->findVar(aVarName);
   else
      v = iBlock->findVar(aVarName);
   return v->iSize;
} // varSizeOf

ZExport(ZCsl&) ZCsl::varResize(const ZString& aVarName)
{
   ZFUNCTRACE_DEVELOP("ZCsl::varResize(const ZString& aVarName)");
   Variable *v;
   if (iCalls)
      v = iCalls->findVar(pureVarName(aVarName));
   else
      v = iBlock->findVar(pureVarName(aVarName));
   v->resize(aVarName);
   return *this;
} // varResize

ZExport(ZBoolean) ZCsl::varExists(const ZString& aVarName)
{
   Variable *v;
   if (iCalls)
      v = iCalls->findVar(aVarName,zFalse);
   else
      v = iBlock->findVar(aVarName,zFalse);
   return v != 0;
} // varExists

static ZString findFile(const ZString& aFile, const ZString& aExt)
{
   ZFUNCTRACE_DEVELOP("findFile(const ZString& aFile, const ZString& aExt, char *path)");
   const char *env;
#if ZC_WIN || ZC_OS2
   if (ZString::lowerCase(aExt)==".cmd" || ZString::lowerCase(aExt)==".bat")
#endif
#if ZC_UNIXFAM
   if (aExt.size()==0)
#endif
      env = "PATH";
   else
      env = "CSLPATH";
#ifdef ZC_CSS_COMPATIBLE
   ZString isEnv = env;
   ZString fn = ZFile::locateFile(aFile+aExt, isEnv);
   if (!fn.length() && isEnv == "CSLPATH") {
	  isEnv = "CSSPATH";
      fn = ZFile::locateFile(aFile+aExt, isEnv);
   } // if
   return fn;
#else
   return ZFile::locateFile(aFile+aExt, env);
#endif
} // findFile

ZExport(ZCsl&) ZCsl::loadScript(const ZString& aFileName)
{
   ZFUNCTRACE_DEVELOP("ZCsl::loadScript(const ZString& aFileName)");
   GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("ZCsl::loadScript(aFileName=%1)")
         .arg(aFileName.constBuffer()).toLatin1().constData());


   last_loaded_file=aFileName;

   // separate extention from filename
   ZString fname, ext;
   fname = ZFile::stripExtension(aFileName, ext);

   // search file
   ZString path;
   if (ext.size())
      path = findFile(fname, ext);
   else {
      path = findFile(fname, ".csl");
#ifdef ZC_CSS_COMPATIBLE
      if (path.size()==0)
         path = findFile(fname, ".css");
#endif
#if ZC_WIN || ZC_OS2
      if (path.size()==0) {
         path = findFile(fname, ".bat");
         if (path.size()==0)
            path = findFile(fname, ".cmd");
      } // if
#endif
#if ZC_UNIXFAM
      if (path.size()==0)
         path = findFile(fname, "");
#endif
   } // if
   if (path.size()==0)
      throwExcept(msgFileNotFound, aFileName);

   // check if allready loaded
#if ZC_WIN || ZC_OS2
   ZString sName(ZFile::stripPath(path).lowerCase());
#endif
#if ZC_UNIXFAM
   ZString sName(ZFile::stripPath(path));
#endif
   Block* b = iStats;
   while (b) {
      if (b->findVar("cslFileName")->value()==sName)
         return *this;
      b = b->iPrev;
   } // while

   // open ifstream
   ifstream iStr(path.constBuffer());
   if (!iStr.good()) throwExcept(msgFileOpenErr, path);

   return loadScript(sName, &iStr);
} // loadScript

ZExport(ZCsl&) ZCsl::loadLibrary(const ZString& /*aDllName*/)
{
	// Phil jan-2002: We no longer need to support this!
   return *this;
} // loadLibrary

ZExport(ZString) ZCsl::call(
   const ZString& aFileName,
   const ZString& aFuncName,
   long aArgCount,
   char** aArgs,
   long* aSize)
{
   ZFUNCTRACE_DEVELOP("ZCsl::call(const ZString&, const ZString&, long, char**, long*)");
   State        saveState(iState);
   Block*       saveStat = iStat;
   ZString      ret;
   ZString		isArg;

   try {
      iState = isNormal;

      Function* func = findFunc(aFuncName); // function available?

      // count mandatory and optional params
      int mandParams(0);
      int totParams(0);
      Parameter* par = func->iParams;
      while (par) {
         if (!par->iIsOptional) mandParams++;
         totParams++;
         par = par->iNext;
      } // while

      // check argument count against parameter list
      if (aArgCount < mandParams || aArgCount > totParams)
         throwExcept(msgInvArgCnt, func->iName);

      // everything ok, so push the arguments in reverse order
      openStat(aFileName);
      for (int i = aArgCount-1; i >= 0; i--)
         if (!aSize || aSize[i] < 0)
		 {
			 isArg = aArgs[i];
            push(isArg);
		 }
         else
            push(ZString(aArgs[i], aSize[i]));

      // if variable parameter count, push argCount
      if (totParams > mandParams) push(ZString(aArgCount));

      // off we go...
      iState = isInterpreting;
      exec(func);
      ret = pop();
   } // try
   catch (const ZException& exc) {
      iStat = saveStat;
      iState = saveState;
      throw;
   } // catch
   iStat = saveStat;
   iState = saveState;
   return ret;
} // call

ZExport(ZString) ZCsl::call(
   const ZString& aFileName,
   const ZString& aFuncName,
   int aArgCount, ...)
{
   ZFUNCTRACE_DEVELOP("ZCsl::call(const ZString&, const ZString&, int, ...)");
   char** args = 0;

   if (aArgCount) {
      args = new char*[aArgCount];
      va_list argPtr;
      va_start(argPtr, aArgCount);
      for (int argc = 0; argc < aArgCount; argc++)
         args[argc] = va_arg(argPtr, char*);
      va_end(argPtr);
   } // if
   return call(aFileName, aFuncName, (long)aArgCount, args);
} // call

ZExport(ZString) ZCsl::callEx(
   const ZString& aFileName,
   const ZString& aFuncName,
   int aArgCount, ...)
{
   ZFUNCTRACE_DEVELOP("ZCsl::callEx(const ZString&, const ZString&, int, ...)");
   char** args = 0;
   long* size = 0;

   if (aArgCount) {
      args = new char*[aArgCount];
      size = new long[aArgCount];
      va_list argPtr;
      va_start(argPtr, aArgCount);
      for (int argc = 0; argc < aArgCount; argc++) {
         args[argc] = va_arg(argPtr, char*);
         size[argc] = va_arg(argPtr, long);
      } // for
      va_end(argPtr);
   } // if
   return call(aFileName, aFuncName, (long)aArgCount, args, size);
} // callEx

ZExport(ZCsl&) ZCsl::addFunc(
   const ZString& aFileName,
   const ZString& aFuncHeader,
   ZString (*aFunc)(ZCsl*))
{
   ZFUNCTRACE_DEVELOP("ZCsl::addFunc(const ZString&, const ZString&, ZString (*)(ZCsl*))");
   State        saveState(iState);
   Block*       saveStat = iStat;
   InputStream* saveInput = iInput;
   Symbol       saveSym(iSym);
   ZString      saveIdent(iIdent);
   double       saveValue(iValue);
   Function*    iFunc = 0;

   try {
      iState = isNormal;
      openStat(aFileName);
	  istringstream iStr((const char*)aFuncHeader);
      iInput = new InputStream(this, aFileName, &iStr, 0);
      iState = isCompiling;
      getSym();
      ZBoolean iStatic(zFalse);
      if (iSym == symStatic) {
         iStatic = zTrue;
         getSym();
      } // if
      if (iSym != symIdent) throwExcept(msgIdentExpct);
      iFunc = new Function(this, iIdent, aFunc, 0);
      getSym();
      getParamList(iFunc);
      if (iSym != symEof) throwExcept(msgUnexpSymbol);
      addFunc(iFunc, iStatic);
   } // try
   catch (const ZException& exc) {
      if (iInput) delete iInput;
      if (iFunc) delete iFunc;
      iStat  = saveStat;
      iInput = saveInput;
      iSym   = saveSym;
      iIdent = saveIdent;
      iValue = saveValue;
      iState = saveState;
      throw;
   } // catch

   iStat  = saveStat;
   iInput = saveInput;
   iSym   = saveSym;
   iIdent = saveIdent;
   iValue = saveValue;
   iState = saveState;
   return *this;
} // addFunc

ZExport(ZCsl&) ZCsl::addVar(
   const ZString& aVarName,
   const ZString& aInitValue,
   ZBoolean aIsConst)
{
   ZFUNCTRACE_DEVELOP("ZCsl::addVar(const ZString&, const ZString&, ZBoolean)");
   if (!iCalls || !iCalls->iBlocks) throwExcept(msgIllGlobAddVar);
   iCalls->iBlocks->addVar(aVarName, aInitValue, aIsConst);
   return *this;
} // addVar

ZExport(ZDateTime) ZCsl::startDateTime() const
{
   ZFUNCTRACE_DEVELOP("ZCsl::startDateTime() const");
   return iStartDateTime;
} // startDateTime

ZExport(ZCsl::TraceMode) ZCsl::traceMode() const
{
   ZFUNCTRACE_DEVELOP("ZCsl::traceMode() const");
   return iTraceMode;
} // traceMode

ZExport(ZCsl&) ZCsl::setTraceMode(TraceMode aMode)
{
   ZFUNCTRACE_DEVELOP("ZCsl::setTraceMode(TraceMode aMode)");
   iTraceMode = aMode;
   return *this;
} // setTraceMode

ZExport(ZCsl&) ZCsl::trace(const ZString& aMessage)
{
   ZFUNCTRACE_DEVELOP("ZCsl::trace(const ZString& aMessage)");
   if (iTraceMode & traceMsgs)
      trace('>', aMessage);
   return *this;
} // trace

ZExport(ZCsl&) ZCsl::show(ShowMode aMode, long aDepth)
{
   ZFUNCTRACE_DEVELOP("ZCsl::show(ShowMode aMode, long aDepth)");
   switch (aMode) {
      case showFunctions: {
         showFuncs(iBlock->iFuncs, zFalse, aDepth);
         Block* ss = iStats;
         while (ss) {
            showFuncs(ss->iFuncs, zFalse, aDepth);
            ss = ss->iPrev;
         } // while
         break;
      }
      case showCallStack:
         showFuncs(iCalls, zFalse, aDepth);
         break;
      case showFullStack:
         showFuncs(iCalls, zTrue, aDepth);
         break;
      case showGlobals: {
         showIdents(iBlock, "", aDepth);
         Block* ss = iStats;
         while (ss) {
            showIdents(ss, ss->findVar("cslFileName")->value()+": static ", aDepth);
            ss = ss->iPrev;
         } // while
         break;
      }
      case showLibraries:
         showLibs(aDepth);
         break;
      default:;
   } // switch
   return *this;
} // show
