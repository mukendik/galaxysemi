
#define ZC_BUILDING_ZCSL

#include "zcsl.hpp"

ZCsl::Function::Function(ZCsl* aParent, const ZString& aName, Function* aPrev) :
   iName(aName),
   iPrev(aPrev),
   iFunc(0),
   iInst(new Instruction),
   iPcode(0),
   iPsize(0),
   iText(0),
   iTsize(0),
   iBlocks(0),
   iStat(aParent->iStat),
   iParams(0),
   iParent(aParent)
{
   ZFUNCTRACE_DEVELOP("ZCsl::Function::Function(ZCsl*, const ZString&, Function*)");
} // Function

ZCsl::Function::Function(ZCsl* aParent, const ZString& aName, ZString (*aFunc)(ZCsl* aCsl), Function* aPrev) :
   iName(aName),
   iPrev(aPrev),
   iFunc(aFunc),
   iInst(0),
   iPcode(0),
   iPsize(0),
   iText(0),
   iTsize(0),
   iBlocks(0),
   iStat(aParent->iStat),
   iParams(0),
   iParent(aParent)
{
   ZFUNCTRACE_DEVELOP("ZCsl::Function::Function(ZCsl*, const ZString&, ZString (*)(ZCsl*), Function*)");
} // Function

ZCsl::Function::Function(Function* aFunction, Function* aPrev) :
   iName(aFunction->iName),
   iPrev(aPrev),
   iFunc(aFunction->iFunc),
   iInst(aFunction->iInst),
   iPcode(aFunction->iPcode),
   iPsize(aFunction->iPsize),
   iText(aFunction->iText),
   iTsize(aFunction->iTsize),
   iBlocks(0),
   iStat(aFunction->iStat),
   iParams(aFunction->iParams),
   iParent(aFunction->iParent)
{
   ZFUNCTRACE_DEVELOP("ZCsl::Function::Function(Function* aFunction, Function* aPrev)");
} // Function

ZCsl::Function::~Function()
{
   ZFUNCTRACE_DEVELOP("ZCsl::Function::~Function()");
   if (iInst)   delete iInst;
   if (iPcode)  delete [] iPcode;
   if (iText)   delete [] iText;
   if (iParams) delete iParams;
   if (iPrev)   delete iPrev;
   if (iBlocks) delete iBlocks;
} // ~Function

void ZCsl::Function::openBlock(const ZString& aName)
{
   ZFUNCTRACE_DEVELOP("ZCsl::Function::openBlock(const ZString& aName)");
   ZBoolean outmost(iBlocks == 0);
   iBlocks = new Block(iParent, iBlocks, aName);
   if (aName.length()) {
      if (outmost) {
         if (iParent->iTraceMode & ZCsl::traceFuncs)
            iParent->trace('+', aName);
      } else {
         if (iParent->iTraceMode & ZCsl::traceBlks)
            iParent->trace('+', aName);
      } // if
   } // if
} // openBlock

void ZCsl::Function::closeBlock()
{
   ZFUNCTRACE_DEVELOP("ZCsl::Function::closeBlock()");
   if (iBlocks->iName.length()) {
      if (iBlocks->iPrev == 0) {
         if (iParent->iTraceMode & ZCsl::traceFuncs)
            iParent->trace('-', iBlocks->iName.constBuffer());
      } else {
         if (iParent->iTraceMode & ZCsl::traceBlks)
            iParent->trace('-', iBlocks->iName.constBuffer());
      } // if
   } // if
   Block* oldBlock = iBlocks;
   iBlocks = iBlocks->iPrev;
   oldBlock->iPrev = 0;
   delete oldBlock;
} // closeBlock

int ZCsl::Function::paramCnt()
{
   ZFUNCTRACE_DEVELOP("ZCsl::Function::paramCnt()");
   Parameter* par = iParams;
   int cnt(0);
   while (par) { cnt++; par = par->iNext; }
   return cnt;
} // parCnt

ZCsl::Parameter* ZCsl::Function::param(int iIndex)
{
   ZFUNCTRACE_DEVELOP("ZCsl::Function::param(int iIndex)");
   Parameter* par = iParams;
   for (int i = 0; i < iIndex; i++) par = par->iNext;
   return par;
} // param

ZCsl::Function& ZCsl::Function::addPar(
   const ZString& aName,
   ZBoolean aIsConst,
   ZBoolean aIsRef,
   ZBoolean aIsOptional)
{
   ZFUNCTRACE_DEVELOP("ZCsl::Function::addPar(const ZString&, ZBoolean, ZBoolean, ZBoolean)");
   Parameter* par = iParams;
   if (par) {
      while (par->iNext) par = par->iNext;
      par->iNext = new Parameter(aName, aIsConst, aIsRef, aIsOptional);
   } else
      iParams = new Parameter(aName, aIsConst, aIsRef, aIsOptional);
   return *this;
} // addPar

ZCsl::Variable* ZCsl::Function::findVar(const ZString& aVarName, ZBoolean aFail)
{
   ZFUNCTRACE_DEVELOP("ZCsl::Function::findVar(const ZString& aVarName, ZBoolean aFail)");
   Block* b = iBlocks;
   Variable *v;
   while (b) {
      v = b->findVar(aVarName, zFalse);
      if (v) return v;
      b = b->iPrev;
   } // while
   v = iStat->findVar(aVarName, zFalse);
   if (v) return v;
   return iParent->iBlock->findVar(aVarName, aFail);
} // findVar
