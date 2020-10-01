#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include "zexcept.hpp"
#include <sstream>

#ifdef ZC_NATIVECSLLIB
  #include "zcsl.hpp"
#else
  #include "ZCslWrap.hpp"
#endif

static ZString mathAbs(ZCsl* csl)
{
   double x(csl->get("x").asDouble());
   if (x < 0) x = -x;
   return x;
} // mathAbs

static ZString mathAcos(ZCsl* csl)
{
   double x(csl->get("x").asDouble());
   if (x < -1 || x > 1)
      ZTHROWEXC("x must be in -1...1");
   return acos(x);
} // mathAcos

static ZString mathAsin(ZCsl* csl)
{
   double x(csl->get("x").asDouble());
   if (x < -1 || x > 1)
      ZTHROWEXC("x must be in -1...1");
   return asin(x);
} // mathAsin

static ZString mathAtan(ZCsl* csl)
{
   return atan(csl->get("x").asDouble());
} // mathAtan

static ZString mathCeil(ZCsl* csl)
{
   return ceil(csl->get("x").asDouble());
} // mathCeil

static ZString mathCos(ZCsl* csl)
{
   return cos(csl->get("x").asDouble());
} // mathCos

static ZString mathCosh(ZCsl* csl)
{
   return cosh(csl->get("x").asDouble());
} // mathCosh

static ZString mathExp(ZCsl* csl)
{
   return exp(csl->get("x").asDouble());
} // mathExp

static ZString mathFloor(ZCsl* csl)
{
   return floor(csl->get("x").asDouble());
} // mathFloor

static ZString mathLog(ZCsl* csl)
{
   double x(csl->get("x").asDouble());
   if (x <= 0)
      ZTHROWEXC("x must be positive");
   return log(x);
} // mathLog

static ZString mathLog10(ZCsl* csl)
{
   double x(csl->get("x").asDouble());
   if (x <= 0)
      ZTHROWEXC("x must be positive");
   return log10(x);
} // mathLog10

static ZString mathMax(ZCsl* csl)
{
   double x(csl->get("x").asDouble());
   double y(csl->get("y").asDouble());
   return y > x ? y : x;
} // mathMax

static ZString mathMin(ZCsl* csl)
{
   double x(csl->get("x").asDouble());
   double y(csl->get("y").asDouble());
   return y < x ? y : x;
} // mathMin

static ZString mathPow(ZCsl* csl)
{
   errno = 0;
   double p(pow(
      csl->get("x").asDouble(),
      csl->get("y").asDouble()
   ));
   if ( errno == EDOM )
      ZTHROWEXC("invalid arguments for x and y");
   if ( errno == ERANGE )
      ZTHROWEXC("result underrun/overrun");
   return p;
} // mathPow

static ZString mathSqrt(ZCsl* csl)
{
   double x(csl->get("x").asDouble());
   if (x < 0)
      ZTHROWEXC("x must be >= 0");
   return sqrt(x);
} // mathSqrt

static ZString mathSin(ZCsl* csl)
{
   return sin(csl->get("x").asDouble());
} // mathSin

static ZString mathSinh(ZCsl* csl)
{
   return sinh(csl->get("x").asDouble());
} // mathSinh

static ZString mathTan(ZCsl* csl)
{
   return tan(csl->get("x").asDouble());
} // mathTan

static ZString mathTanh(ZCsl* csl)
{
   return tanh(csl->get("x").asDouble());
} // mathTanh

void ZCslInitLib_Math(ZCsl* aCsl)
{
   ZString iFile("MathLib");
   ZString init(ZString(
      "const mathVersion = '3.00';\n"
      "const mathCompiler = '")+ZC_COMPILER+"';\n"
      "const mathLibtype = '"+ZC_CSLLIBTYPE+"';\n"
      "const mathBuilt = '"+ZString(__DATE__)+" "+__TIME__+"';\n"
   );
   std::istringstream str((char*)init);
   aCsl->loadScript(iFile, &str);
   (*aCsl)
      .addFunc(iFile, "abs(const x)",          mathAbs)
      .addFunc(iFile, "acos(const x)",         mathAcos)
      .addFunc(iFile, "asin(const x)",         mathAsin)
      .addFunc(iFile, "atan(const x)",         mathAtan)
      .addFunc(iFile, "ceil(const x)",         mathCeil)
      .addFunc(iFile, "cos(const x)",          mathCos)
      .addFunc(iFile, "cosh(const x)",         mathCosh)
      .addFunc(iFile, "exp(const x)",          mathExp)
      .addFunc(iFile, "floor(const x)",        mathFloor)
      .addFunc(iFile, "log(const x)",          mathLog)
      .addFunc(iFile, "log10(const x)",        mathLog10)
      .addFunc(iFile, "max(const x, const y)", mathMax)
      .addFunc(iFile, "min(const x, const y)", mathMin)
      .addFunc(iFile, "pow(const x, const y)", mathPow)
      .addFunc(iFile, "sqrt(const x)",         mathSqrt)
      .addFunc(iFile, "sin(const x)",          mathSin)
      .addFunc(iFile, "sinh(const x)",         mathSinh)
      .addFunc(iFile, "tan(const x)",          mathTan)
      .addFunc(iFile, "tanh(const x)",         mathTanh);
} // ZCslInitLib

void ZCslCleanupLib_Math(ZCsl* /*aCsl*/)
{
} // ZCslCleanupLib
