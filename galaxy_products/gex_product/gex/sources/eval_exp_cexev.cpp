/* 
   CExEv - C Expression Evaluator library

   Copyright (C) 2002 Janos Pap

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  
*/
//--------------------------------------------------------------------------------
#include <QtCore>

#include "eval_exp_cexev.h"
#include <gqtl_global.h>
#include <math.h>

//--------------------------------------------------------------------------------

//#define DEBUG_CEXEV 1
#define DEBUG_CEXEV_MAIN 1

namespace cexev
{

/*****************************************************************************************
 */

CExEvCtx::CExEvCtx()
{
}

CExEvCtx::~CExEvCtx()
{
	funcFree();
}

void CExEvCtx::funcFree()
{
}

bool CExEvCtx::funcDefine(const char* /*cFuncName*/,
						  const char* /*cSignature*/,
						  ExEvFunction /*func*/,
						  int /*coll*/)
{
	return false;
}

bool CExEvCtx::isValidIdentStartChar(int ch)
{
	const char *cValidStartChars = "_"; // and Aa..Zz
	if (('A' <= ch && ch <= 'Z') ||
		('a' <= ch && ch <= 'z') ||
		strchr(cValidStartChars, ch) != NULL) {
		return true;
	}

	return false;
}

bool CExEvCtx::isValidIdentChar(int ch)
{
	const char *cValidChars = "0123456789.:?"; // and all valid start chars!
	if (isValidIdentStartChar(ch) ||
		strchr(cValidChars, ch) != NULL) {
		return true;
	}

	return false;
}

bool CExEvCtx::getValue(const char *cIdent, Variant& result)
{
	// Predefined identifier pi
	if (qstricmp(cIdent, "pi") == 0)
	{
		result.setReal(3.141592653);
		return true;
	}

	result.setNull();
	return false;
}

/**
	Calls a function
*/
bool CExEvCtx::callFunction(const char *cFunction,
											   int nArgc, Variant **vArgs,
												Variant& result)
{
	return doCallFunction(m_stdFuncDef, cFunction, nArgc, vArgs, result);
}

/**
	Calls an operator
*/
bool CExEvCtx::callOperator(const char *cOperator,
											   int nArgc, Variant **vArgs,
												Variant& result)
{
	return doCallFunction(m_stdOpDef, cOperator, nArgc, vArgs, result);
}

bool CExEvCtx::doCallFunction(SFunctionDef *pFnDefs,
							  const char *cFunction,
							  int nArgc, Variant **vArgs,
							  Variant& result)
{
	bool bMatch;
	char *cSig = new char [nArgc + 2], cs;
	int i, j;

	// Create signature for the function to be called
	cSig[0] = '?';
	for (i = 0; i < nArgc; i++)
	{
		cSig[i+1] = vArgs[i]->getTypeChar();
	}
	cSig[i+1] = '\0';

	// Find function by name and signature (the first match is selected)
	bMatch = false;
	for (i = 0; pFnDefs[i].cFuncName != NULL; i++)
	{
		if (strcmp(cFunction, pFnDefs[i].cFuncName) != 0)
			continue;
		bMatch = true;
		for (j = 0; j < nArgc+1; j++)
		{
			cs = pFnDefs[i].cSignature[j];
			if (cs == '*')
				break;
			if (cs == '\0')
			{
				if (cSig[j] != '\0')
					bMatch = false;
				break;
			}
			if (cs == '?' || cSig[j] == '?' || cs == cSig[j])
				continue;
			bMatch = false;
			break;
		}
		if (bMatch)
			break;
	}
	delete [] cSig;

	result.setNull();

	if (bMatch)
	{
		return pFnDefs[i].func(nArgc, vArgs, result);
	}

	return false;
}

// Built-in operators
CExEvCtx::SFunctionDef CExEvCtx::m_stdOpDef[] =
{
	// +v
	{ "+",			"??",		"Unary plus",	CExEvCtx::opUnaryPlus },
	// -v
	{ "-",			"??",		"Unary minus",	CExEvCtx::opUnaryMinus },
	// ~v
	{ "~",			"??",		"Unary bitnot",	CExEvCtx::opUnaryBitNot },
	// !v
	{ "!",			"??",		"Unary not",	CExEvCtx::opUnaryLogNot },

	// v1 + s
	{ "+",			"S?S",		"Append string",	CExEvCtx::opPlusS_S },
	// s + v2
	{ "+",			"SS?",		"Append to string",	CExEvCtx::opPlusSS_ },

	// v1 * v2
	{ "*",			"???",		"Mul",	CExEvCtx::opMul },
	// v1 / v2
	{ "/",			"???",		"Div",	CExEvCtx::opDiv },
	// v1 % v2
	{ "%",			"???",		"Mod",	CExEvCtx::opMod },
	// v1 + v2
	{ "+",			"???",		"Add",	CExEvCtx::opAdd },
	// v1 - v2
	{ "-",			"???",		"Sub",	CExEvCtx::opSub },
	// v1 << v2
	{ "<<",			"???",		"Bitleft",	CExEvCtx::opBitLeft },
	// v1 >> v2
	{ ">>",			"???",		"Bitright",	CExEvCtx::opBitRight },
	// v1 < v2
	{ "<",			"???",		"LT",	CExEvCtx::opLT },
	// v1 > v2
	{ ">",			"???",		"GT",	CExEvCtx::opGT },
	// v1 <= v2
	{ "<=",			"???",		"LE",	CExEvCtx::opLE },
	// v1 >= v2
	{ ">=",			"???",		"GE",	CExEvCtx::opGE },
	// v1 == v2
	{ "==",			"???",		"EQ",	CExEvCtx::opEQ },
	// v1 != v2
	{ "!=",			"???",		"NE",	CExEvCtx::opNE },
	// v1 & v2
	{ "&",			"???",		"BitAnd",	CExEvCtx::opBitAnd },
	// v1 ^ v2
	{ "^",			"???",		"BitXor",	CExEvCtx::opBitXor },
	// v1 | v2
	{ "|",			"???",		"BitOr",	CExEvCtx::opBitOr },
	// v1 && v2
	{ "&&",			"???",		"LogAnd",	CExEvCtx::opLogAnd },
	// v1 || v2
	{ "||",			"???",		"LogOr",	CExEvCtx::opLogOr },

	{ NULL,			NULL,		NULL,			NULL }
};

// Built-in functions
CExEvCtx::SFunctionDef CExEvCtx::m_stdFuncDef[] =
{
	// sin
	{ "sin",		"R?",		"Sinus",	CExEvCtx::fnSin },
	
	// max
	{ "max",		"V?V",	"Max",		CExEvCtx::fnMax },
	
	// min
	{ "min",		"V?V",	"Min",		CExEvCtx::fnMin },
	
	// log10
	{ "log10",		"V?",		"Log10",	CExEvCtx::fnLog10 },
	
	// ln
	{ "ln",			"V?",		"Ln",		CExEvCtx::fnLn },
	
	// exp
	{ "exp",		"V?",		"Exp",		CExEvCtx::fnExp },
	
	// power
	{ "power",		"B?E",		"Power",	CExEvCtx::fnPower },

    // abs
    { "abs",		"V?",		"Abs",	CExEvCtx::fnAbs },

	{ NULL,			NULL,		NULL,			NULL }
};

bool CExEvCtx::opUnaryPlus(int /*nArgc*/, Variant **vArgs, Variant& result)
{
	if (vArgs[0]->isInt() ||
		vArgs[0]->isDWORD() ||
		vArgs[0]->isReal() ||
		vArgs[0]->isBool())
	{
		result = *vArgs[0];
		return true;
	}

	return false;
}

bool CExEvCtx::opUnaryMinus(int /*nArgc*/, Variant **vArgs, Variant& result)
{
	if (vArgs[0]->isInt())
	{
		result.setInt(-vArgs[0]->getInt());
	}
	else if (vArgs[0]->isDWORD())
	{
		result.setDWORD((long)(-(long)vArgs[0]->getDWORD()));
	}
	else if (vArgs[0]->isReal())
	{
		result.setReal(-vArgs[0]->getReal());
	}
	else if (vArgs[0]->isBool())
	{
		result.setInt(-(int)(vArgs[0]->getBool()));
	}
	else
		return false;

	return true;
}

bool CExEvCtx::opUnaryBitNot(int /*nArgc*/, Variant **vArgs, Variant& result)
{
	if (vArgs[0]->isInt())
	{
		result.setInt(~vArgs[0]->getInt());
	}
	else if (vArgs[0]->isDWORD())
	{
		result.setDWORD(~vArgs[0]->getDWORD());
	}
	else
		return false;

	return true;
}

bool CExEvCtx::opUnaryLogNot(int /*nArgc*/, Variant **vArgs, Variant& result)
{
	if (vArgs[0]->isInt())
	{
		result.setBool(vArgs[0]->getInt() == 0);
	}
	else if (vArgs[0]->isDWORD())
	{
		result.setBool(vArgs[0]->getDWORD() == 0);
	}
	else if (vArgs[0]->isReal())
	{
		result.setBool(vArgs[0]->getReal() == 0.0);
	}
	else if (vArgs[0]->isBool())
	{
		result.setBool(!vArgs[0]->getBool());
	}
	else
		return false;

	return true;
}

bool CExEvCtx::opPlusS_S(int /*nArgc*/, Variant **vArgs, Variant& result)
{
	Variant &v1 = *vArgs[0],
			&v2 = *vArgs[1];

	String s;
	v1.asString(s);
	s += v2.getString();
	result.setString(s.c_str());

	return  true;
}

bool CExEvCtx::opPlusSS_(int /*nArgc*/, Variant **vArgs, Variant& result)
{
	Variant &v1 = *vArgs[0],
			&v2 = *vArgs[1];

	String s1, s2;
	v1.asString(s1);
	v2.asString(s2);
	s1 += s2;
	result.setString(s1.c_str());

	return  true;
}

bool CExEvCtx::opMul(int /*nArgc*/, Variant **vArgs, Variant& result)
{
	bool bRes = Variant::MUL(*vArgs[0], *vArgs[1], result);

	return  bRes;
}

bool CExEvCtx::opDiv(int /*nArgc*/, Variant **vArgs, Variant& result)
{
	bool bRes = Variant::DIV(*vArgs[0], *vArgs[1], result);

	return  bRes;
}

bool CExEvCtx::opMod(int /*nArgc*/, Variant **vArgs, Variant& result)
{
	bool bRes = Variant::MOD(*vArgs[0], *vArgs[1], result);

	return  bRes;
}

bool CExEvCtx::opAdd(int /*nArgc*/, Variant **vArgs, Variant& result)
{
	bool bRes = Variant::ADD(*vArgs[0], *vArgs[1], result);

	return  bRes;
}

bool CExEvCtx::opSub(int /*nArgc*/, Variant **vArgs, Variant& result)
{
	bool bRes = Variant::SUB(*vArgs[0], *vArgs[1], result);

	return  bRes;
}

bool CExEvCtx::opBitLeft(int /*nArgc*/, Variant **vArgs, Variant& result)
{
	bool bRes = Variant::BITLEFT(*vArgs[0], *vArgs[1], result);

	return  bRes;
}

bool CExEvCtx::opBitRight(int /*nArgc*/, Variant **vArgs, Variant& result)
{
	bool bRes = Variant::BITRIGHT(*vArgs[0], *vArgs[1], result);

	return  bRes;
}

bool CExEvCtx::opLT(int /*nArgc*/, Variant **vArgs, Variant& result)
{
	bool bRes = Variant::LT(*vArgs[0], *vArgs[1], result);

	return  bRes;
}

bool CExEvCtx::opGT(int /*nArgc*/, Variant **vArgs, Variant& result)
{
	bool bRes = Variant::GT(*vArgs[0], *vArgs[1], result);

	return  bRes;
}

bool CExEvCtx::opLE(int /*nArgc*/, Variant **vArgs, Variant& result)
{
	bool bRes = Variant::LE(*vArgs[0], *vArgs[1], result);

	return  bRes;
}

bool CExEvCtx::opGE(int /*nArgc*/, Variant **vArgs, Variant& result)
{
	bool bRes = Variant::GE(*vArgs[0], *vArgs[1], result);

	return  bRes;
}

bool CExEvCtx::opEQ(int /*nArgc*/, Variant **vArgs, Variant& result)
{
	bool bRes = Variant::EQ(*vArgs[0], *vArgs[1], result);

	return  bRes;
}

bool CExEvCtx::opNE(int /*nArgc*/, Variant **vArgs, Variant& result)
{
	bool bRes = Variant::NE(*vArgs[0], *vArgs[1], result);

	return  bRes;
}

bool CExEvCtx::opBitAnd(int /*nArgc*/, Variant **vArgs, Variant& result)
{
	bool bRes = Variant::BITAND(*vArgs[0], *vArgs[1], result);

	return  bRes;
}

bool CExEvCtx::opBitXor(int /*nArgc*/, Variant **vArgs, Variant& result)
{
	bool bRes = Variant::BITXOR(*vArgs[0], *vArgs[1], result);

	return  bRes;
}

bool CExEvCtx::opBitOr(int /*nArgc*/, Variant **vArgs, Variant& result)
{
	bool bRes = Variant::BITOR(*vArgs[0], *vArgs[1], result);

	return  bRes;
}

bool CExEvCtx::opLogAnd(int /*nArgc*/, Variant **vArgs, Variant& result)
{
	bool bRes = Variant::LOGAND(*vArgs[0], *vArgs[1], result);

	return  bRes;
}

bool CExEvCtx::opLogOr(int /*nArgc*/, Variant **vArgs, Variant& result)
{
	bool bRes = Variant::LOGOR(*vArgs[0], *vArgs[1], result);

	return  bRes;
}

/**
	double sin(double r)
*/
bool CExEvCtx::fnSin(int /*nArgc*/, Variant **vArgs, Variant& result)
{
	double r;

	if (vArgs[0]->asReal(r))
	{
		result.setReal(sin(r));
		return true;
	}

	return false;
}

/**
	double max(double v1, double v2)
*/
bool CExEvCtx::fnMax(int /*nArgc*/, Variant **vArgs, Variant& result)
{
	double v1, v2;
	
	if (vArgs[0]->asReal(v1) && vArgs[1]->asReal(v2))
	{
		result.setReal(std::max(v1, v2));
		return true;
	}

	return false;
}

/**
	double min(double v1, double v2)
*/
bool CExEvCtx::fnMin(int /*nArgc*/, Variant **vArgs, Variant& result)
{
	double v1, v2;
	
	if (vArgs[0]->asReal(v1) && vArgs[1]->asReal(v2))
	{
		result.setReal(std::min(v1, v2));
		return true;
	}

	return false;
}

/**
	double log(double v)
*/
bool CExEvCtx::fnLog10(int /*nArgc*/, Variant **vArgs, Variant& result)
{
	double v;
	
	if (vArgs[0]->asReal(v))
	{
		result.setReal(log10(v));
		return true;
	}

	return false;
}

/**
	double ln(double v)
*/
bool CExEvCtx::fnLn(int /*nArgc*/, Variant **vArgs, Variant& result)
{
	double v;
	
	if (vArgs[0]->asReal(v))
	{
		result.setReal(log(v));
		return true;
	}

	return false;
}

/**
	double exp(double v)
*/
bool CExEvCtx::fnExp(int /*nArgc*/, Variant **vArgs, Variant& result)
{
	double v;
	
	if (vArgs[0]->asReal(v))
	{
		result.setReal(exp(v));
		return true;
	}

	return false;
}

/**
	double power(double b, double e)
*/
bool CExEvCtx::fnPower(int /*nArgc*/, Variant **vArgs, Variant& result)
{
	double v1, v2;
	
	if (vArgs[0]->asReal(v1) && vArgs[1]->asReal(v2))
	{
        result.setReal(GS_POW(v1, v2));
		return true;
	}

	return false;
}

/**
    double abs(double v)
*/
bool CExEvCtx::fnAbs(int /*nArgc*/, Variant **vArgs, Variant& result)
{
    double v;

    if (vArgs[0]->asReal(v))
    {
        result.setReal(fabs(v));
        return true;
    }

    return false;
}
/*****************************************************************************************
 */

class CExEv_Token : public Variant
{
public:
	enum
	{
	  TOK_NULL     = 0x0000,	// NULL (empty) token
	  TOK_UNKNOWN  = 0x0001,	// unknown token

	  TOK_CONST    = 0x0101,	// const token (bool,int,dword,double,string)
	  TOK_IDENT    = 0x0102,	// ident token (identifier)
//		  TOK_PARAM    = 0x0103,	//

	  TOK_OPAREN   = 0x0201,	// Open parenthese
	  TOK_CPAREN   = 0x0202,	// Closing parenthese

	  TOK_OPERATOR = 0x0401,	// Operator
	  TOK_FUNCTION = 0x0402,	// Function

	  TOK_COMMA    = 0x0301,	// Comma
	};

  CExEv_Token();
  ~CExEv_Token();
  void clear();
  int getTokenType() { return m_iType; }
  int setTokenType(int iType) { return m_iType = iType; }
  CExEv_Token& operator=(CExEv_Token& t);
  void dump(String& s);

  static bool isTokNULL(int iTok) { return iTok == TOK_NULL; }
  static bool isTokUNKNOWN(int iTok) { return iTok == TOK_UNKNOWN; }
  static bool isTokVALUE(int iTok) { return (iTok & 0xFF00) == 0x0100; }
  static bool isTokPAREN(int iTok) { return (iTok & 0xFF00) == 0x0200; }
  static bool isTokACTION(int iTok) { return (iTok & 0xFF00) == 0x0400; }

  int m_iType;
  CExEv_Token *pNext;

};

CExEv_Token::CExEv_Token() :
	Variant()
{
	m_iType = TOK_NULL;
	pNext = NULL;
}

CExEv_Token::~CExEv_Token()
{
	clear();
}

void CExEv_Token::clear()
{
  m_iType = TOK_NULL;
  pNext = NULL;
  Variant::setNull();
}

CExEv_Token& CExEv_Token::operator=(CExEv_Token& t)
{
  m_iType = t.m_iType;
  *(Variant *)this = *(Variant *)&t;

  return *this;
}

void CExEv_Token::dump(String& s)
{
  String s1;
  s = "";

  switch (m_iType)
  {
    case TOK_NULL:
      s = "<NULL>";
      break;
    case TOK_UNKNOWN:
      s = "<UNKNOWN>";
      break;
    case TOK_CONST:
	  Variant::asString(s1);
      s = "<CONST>[";
	  s += s1;
	  s += "]";
      break;
    case TOK_IDENT:
      Variant::asString(s1);
      s = "<IDENT>[";
	  s += s1;
	  s += "]";
      break;
    case TOK_OPAREN:
      s = "<OPAREN>(";
      break;
    case TOK_CPAREN:
      s = "<CPAREN>)";
      break;
    case TOK_OPERATOR:
      Variant::asString(s1);
	  s = "<OPERATOR>";
	  s += s1;
      break;
    case TOK_FUNCTION:
	{
	  String s1;
	  Variant::asString(s1);
      s = "<FUNC>";
	  s += s1;
      break;
	}
    case TOK_COMMA:
      s = "<COMMA>,";
      break;
    default:
      s = "<Impossible!>";
      break;
  }
}

/*****************************************************************************************
 */

CExEv::CExEv() :
	CExEvCtx()
{
	m_pEECtx = this;

	m_cExprText = m_cExprPtr = NULL;
	pTokenStack = pTokenDel = NULL;
	pCurrToken = NULL;
}

CExEv::CExEv(CExEvCtx *pEECtx) :
	CExEvCtx()
{
	m_pEECtx = pEECtx;

	m_cExprText = m_cExprPtr = NULL;
	pTokenStack = pTokenDel = NULL;
	pCurrToken = NULL;
}

CExEv::~CExEv()
{
  clear();
}

void CExEv::clear()
{
  CExEv_Token *pToken;

  delete [] m_cExprText;
  m_cExprText = m_cExprPtr = NULL;

  for (pToken = pTokenStack; pToken != NULL; pToken = pTokenStack)
  {
    pTokenStack = pTokenStack->pNext;
	delete pToken; pToken=0;
  }
  pTokenStack = NULL;

  for (pToken = pTokenDel; pToken != NULL; pToken = pTokenDel)
  {
    pTokenDel = pTokenDel->pNext;
	delete pToken; pToken=0;
  }
  pTokenDel = NULL;

  delete pCurrToken; pCurrToken = NULL;

  m_sDump = "";

  m_iError = 0;
}

int CExEv::setError(int iErrorCode)
{
	if (m_iError == 0)
	{
		m_iError = iErrorCode;
	}

	return m_iError;
}

CExEv_Token *CExEv::pop()
{
  CExEv_Token *pTopToken = pTokenStack;
  if (pTokenStack != NULL)
  {
    pTokenStack = pTokenStack->pNext;
    pTopToken->pNext = pTokenDel;
    pTokenDel = pTopToken;
  }

#if DEBUG_CEXEV
dumpStack();
#endif

  return pTopToken;
}

CExEv_Token *CExEv::push()
{
	if (pCurrToken != NULL)
	{
		pCurrToken->pNext = pTokenStack;
		pTokenStack = pCurrToken;
		pCurrToken = NULL;

#if DEBUG_CEXEV
dumpStack();
#endif

		return pTokenStack;
	}

	return NULL;
}

CExEv_Token *CExEv::swapTop()
{
  if (pTokenStack != NULL && pTokenStack->pNext != NULL)
  {
    CExEv_Token *pFirst  = pTokenStack;
    CExEv_Token *pSecond = pTokenStack->pNext;

    pTokenStack = pSecond;
    pFirst->pNext = pSecond->pNext;
    pTokenStack->pNext = pFirst;
  }
  return pTokenStack;
}

CExEv_Token *CExEv::newToken()
{
  CExEv_Token *pToken;
  if (pCurrToken != NULL)
  {
	  pToken = pCurrToken;
	  pToken->clear();
  }
  else
  {
	  if (pTokenDel != NULL && pTokenDel->pNext != NULL)
	  {
		pToken = pTokenDel->pNext;
		pTokenDel->pNext = pToken->pNext;
		pToken->pNext = NULL;
		pToken->clear(); //pToken->Set(TOKEN_NULL);
	  }
	  else
	  {
		pToken = new CExEv_Token;
	  }
	  pCurrToken = pToken;
  }

  return pCurrToken;
}

int CExEv::getNextToken()
{
#define C (*m_cExprPtr)
#define NEXT_C m_cExprPtr++
#define PREV_C m_cExprPtr--

#if DEBUG_CEXEV
String sd;
sd = "STR("; sd += m_cExprPtr; sd += ")\r\n";
m_sDump += sd.c_str();
#endif

  if (newToken() == NULL)
  {
	  setError(E_NOMEM);
	  return -1;
  }

  while (C == ' '  || C == '\t' ||
         C == '\n' || C == '\r')
    NEXT_C;

  // End of input string?
  if (C == 0)
  {
    pCurrToken->clear(); //Set(TOKEN_NULL);
	return pCurrToken->getTokenType();
  }

  // '('
  if (C == '(')
  {
	NEXT_C;
	return pCurrToken->setTokenType(CExEv_Token::TOK_OPAREN);
  }

  // ')'
  if (C == ')')
  {
    NEXT_C;
    return pCurrToken->setTokenType(CExEv_Token::TOK_CPAREN);
  }

  // ','
  if (C == ',')
  {
	NEXT_C;
	return pCurrToken->setTokenType(CExEv_Token::TOK_COMMA);
  }

  // TODO: handle octal and hexa numbers, too
  // Number
  if (strchr("0123456789", C))
  {
    char *start, cTemp;
    int decimal, len, numlen;
	bool bFloat = false;

    start = m_cExprPtr;
    len = 0;
    decimal = false;
    while (isdigit(C) ||
          (C == '.' && !decimal))
    {
	  if (C == '.')
	  {
        decimal = true;
		bFloat = true;
	  }
      NEXT_C;
      len++;
    }
    if (len == 1 && start[0] == '.')
    {
      setError(E_ERROR);
      return pCurrToken->setTokenType(CExEv_Token::TOK_UNKNOWN);
    }
    if (C == 'E' || C == 'e')
    {
	  bFloat = true;

      NEXT_C;
      len++;
      if (C == '+' || C == '-')
      {
        NEXT_C;
        len++;
      }
      numlen = 0;
      while (isdigit(C) && ++numlen <= 3)
      {
        NEXT_C;
        len++;
      }
    }
    cTemp = start[len];
    start[len] = '\0';
	if (bFloat)
	{
		double d;
		d = atof(start);
#if 0 // FIXME: 
		if (errno == ERANGE)
		{
			setError(E_ERROR);
			start[len] = cTemp;
			return pCurrToken->setTokenType(TOK_UNKNOWN);
		}
#endif
		pCurrToken->Variant::setReal(d);
	}
	else
	{
		long dw;
		dw = (long)atol(start);
		if ((dw & 0x80000000) != 0)
		{
			pCurrToken->Variant::setDWORD(dw);
		}
		else
		{
			pCurrToken->Variant::setInt((int)dw);
		}
	}
	start[len] = cTemp;

	return pCurrToken->setTokenType(CExEv_Token::TOK_CONST);
  }

	// Character
	if (C == '\'')
	{
		const char *escapes = "\\\'\"tnrba";
		const char *escaped = "\\\'\"\t\n\r\b\a";
		int ch;
		NEXT_C;
		if (C == '\\')
		{
			const char *cp;
			NEXT_C;
			if ((cp = strchr(escapes, C)) == NULL)
			{
				setError(E_ERROR);
				return pCurrToken->setTokenType(CExEv_Token::TOK_UNKNOWN);
			}
			ch = escaped[cp-escapes];
		}
		else
		{
			ch = C;
		}
		NEXT_C;
		if (C != '\'')
		{
			setError(E_ERROR);
			return pCurrToken->setTokenType(CExEv_Token::TOK_UNKNOWN);
		}
		NEXT_C;
		// Characters are integers for us!
		pCurrToken->Variant::setInt(ch);
		return pCurrToken->setTokenType(CExEv_Token::TOK_CONST);
	}

	// String
	if (C == '\"')
	{
		const char *escapes = "\\\'\"tnrba";
		const char *escaped = "\\\'\"\t\n\r\b\a";
		int ch;
		String s;

		while (1)
		{
			NEXT_C;
			// end of string
			if (C == '\"')
				break;
			// unterminated string
			if (C == 0)
			{
				setError(E_ERROR);
				return pCurrToken->setTokenType(CExEv_Token::TOK_UNKNOWN);
			}
			// is it an escaped char?
			if (C == '\\')
			{
				const char *cp;
				NEXT_C;
				// no such escape char
				if ((cp = strchr(escapes, C)) == NULL)
				{
					setError(E_ERROR);
					return pCurrToken->setTokenType(CExEv_Token::TOK_UNKNOWN);
				}
				ch = escaped[cp-escapes];
			}
			// normal char
			else
			{
				ch = C;
			}
			s += ch;
		}
		NEXT_C;
		pCurrToken->Variant::setString(s.c_str());
		return pCurrToken->setTokenType(CExEv_Token::TOK_CONST);
	}

	// Identifier (variable or function name)
	if (m_pEECtx->isValidIdentStartChar(C))
	{
		String s;
		s = C;
		NEXT_C;
		while (C != 0 && m_pEECtx->isValidIdentChar(C))
		{
			s += C;
			NEXT_C;
		}
		pCurrToken->Variant::setString(s.c_str());
		return pCurrToken->setTokenType(CExEv_Token::TOK_IDENT);
	}

	// Operator
	char c = C;
	NEXT_C;
	pCurrToken->setTokenType(CExEv_Token::TOK_OPERATOR);
	switch (c)
	{
	case '+' : pCurrToken->setString("+"); break;
	case '-' : pCurrToken->setString("-"); break;
	case '~' : pCurrToken->setString("~"); break;

	case '*' : pCurrToken->setString("*"); break;
	case '/' : pCurrToken->setString("/"); break;
	case '%' : pCurrToken->setString("%"); break;

	case '^' : pCurrToken->setString("^"); break;

	case '<' :
	{
		if (C == '<')
		{
			NEXT_C;
			pCurrToken->setString("<<");
		}
		else if (C == '=')
		{
			NEXT_C;
			pCurrToken->setString("<=");
		}
		else
			pCurrToken->setString("<");
		break;
	}
	case '>' :
	{
		if (C == '>')
		{
			NEXT_C;
			pCurrToken->setString(">>");
		}
		else if (C == '=')
		{
			NEXT_C;
			pCurrToken->setString(">=");
		}
		else
			pCurrToken->setString(">");
		break;
	}
	case '=' :
	{
		if (C == '=')
		{
			NEXT_C;
			pCurrToken->setString("==");
		}
		else
		{
			PREV_C;
			pCurrToken->setTokenType(CExEv_Token::TOK_UNKNOWN);
		}
		break;
	}
	case '!' :
	{
		if (C == '=')
		{
			NEXT_C;
			pCurrToken->setString("!=");
		}
		else
			pCurrToken->setString("!");
		break;
	}
	case '&' :
	{
		if (C == '&')
		{
			NEXT_C;
			pCurrToken->setString("&&");
		}
		else
			pCurrToken->setString("&");
		break;
	}
	case '|' :
	{
		if (C == '|')
		{
			NEXT_C;
			pCurrToken->setString("||");
		}
		else
			pCurrToken->setString("|");
		break;
	}
	default  :
		pCurrToken->setTokenType(CExEv_Token::TOK_UNKNOWN);
		break;
	}
	if (pCurrToken->getTokenType() == CExEv_Token::TOK_OPERATOR)
	{
		return CExEv_Token::TOK_OPERATOR;
	}

	setError(E_ERROR);
	PREV_C;

	return pCurrToken->setTokenType(CExEv_Token::TOK_UNKNOWN);
#undef C
#undef NEXT_C
#undef PREV_C
}

static struct _SPriority
{
	int iPri;
	const char *cOp;
} _opPriority[] =
{
	{ 10, "*" }, // mul
	{ 10, "/" }, // div
  	{ 10, "%" }, // mod

  	{ 9, "+" }, // add
  	{ 9, "-" }, // sub

  	{ 8, "<<" }, // bitleft
  	{ 8, ">>" }, // bitright

  	{ 7, "<" }, // LT
  	{ 7, ">" }, // GT
  	{ 7, "<=" }, // LE
  	{ 7, ">=" }, // GE

  	{ 6, "==" }, // EQ
  	{ 6, "!=" }, // NE

  	{ 5, "&" }, // bitand

  	{ 4, "^" }, // bitxor

  	{ 3, "|" }, // bitor

  	{ 2, "&&" }, // logand

  	{ 1, "||" }, // logor

	{ 0, NULL },
};

int CExEv::getOperatorPriority(const char *cOp)
{
	int i;
	for (i = 0; _opPriority[i].cOp != NULL; i++)
	{
		if (strcmp(cOp, _opPriority[i].cOp) == 0)
			return _opPriority[i].iPri;
	}

	return 0;
}

int CExEv::reduce(int iOpToken)
{
	int iPri;
	if (iOpToken == CExEv_Token::TOK_CPAREN)
		iPri = 0;
	else if(iOpToken == CExEv_Token::TOK_NULL)
		iPri = 0;
	else if (iOpToken == CExEv_Token::TOK_OPERATOR)
		iPri = getOperatorPriority(pCurrToken->getString());
	else
		return setError(E_ERROR);

	CExEv_Token  *t1, *t2, *t3, *t4;
	Variant result, *pArgs[2];

	while (!m_iError)
	{
		t1 = pTokenStack;
		// Who ate the bottom of the stack?
		if (t1 == NULL)
		{
			return setError(E_ERROR);
		}

		t2 = pTokenStack->pNext;
		// The stack is empty?
		if (t2 == NULL)
		{
			// An unary operator is valid as the first token
			if (iOpToken == CExEv_Token::TOK_OPERATOR)
			{
				push();
				return 0;
			}

			return setError(E_ERROR);
		}

		// The top of the stack might be a TOK_FUNCTION (function call without parameters)
		if (t1->getTokenType() == CExEv_Token::TOK_FUNCTION)
		{
			// The new token must be a closing parenthese
			if (iOpToken == CExEv_Token::TOK_CPAREN)
			{
				if (!m_pEECtx->callFunction(t1->getString(), 0, NULL, result))
					return setError(E_FUNCTION);
				// Replace the function by its return value
				pTokenStack->setTokenType(CExEv_Token::TOK_CONST);
				*(Variant *)pTokenStack = result;
			}
			else
				return setError(E_SYNTAX);

			return 0;
		}

		// The top of the stack must be a TOK_CONST
		if (t1->getTokenType() != CExEv_Token::TOK_CONST)
		{
			return setError(E_SYNTAX);
		}

		// The second token is an operator
		if (t2->getTokenType() == CExEv_Token::TOK_OPERATOR)
		{
			t3 = t2->pNext;

			// The third is a constant
			if (t3->getTokenType() == CExEv_Token::TOK_CONST)
			{
				// This is a higher priority operator
				if (iPri > getOperatorPriority(t2->getString()))
				{
					push();
					// Go fetch the next token
					return 0;
				}
				// Otherwise, evaluate all preceeding operators with higher or same priority
				pArgs[0] = (Variant *)t3;
				pArgs[1] = (Variant *)t1;
				if (!m_pEECtx->callOperator(t2->getString(), 2, pArgs, result))
					return setError(E_FUNCTION);

				// Put the result on the stack
				pop();
				pop();
				//pop();
				pTokenStack->setTokenType(CExEv_Token::TOK_CONST);
				*(Variant *)pTokenStack = result;
				continue;
			}

			// t2 must be an unary operator (highest precedence so evaluate it)
			pArgs[0] = (Variant *)t1;
			if (!m_pEECtx->callOperator(t2->getString(), 1, pArgs, result))
				return setError(E_OPERATOR);

			// Put the result on the stack
			pop();
			//pop();
			pTokenStack->setTokenType(CExEv_Token::TOK_CONST);
			*(Variant *)pTokenStack = result;
			continue;
		}

		// The second token is an '('
		if (t2->getTokenType() == CExEv_Token::TOK_OPAREN)
		{
			// Wow, it's successfully evaluated subexpression
			if (iOpToken == CExEv_Token::TOK_CPAREN)
			{
				// Put the result on the stack
				swapTop();
				pop();
				return 0;
			}
			// Premature end of expression string
			if (iOpToken == CExEv_Token::TOK_NULL)
			{
				return setError(E_EOE);
			}

			push();
			// Go on
			return 0;
		}

		// The second token is a TOK_FUNCTION (function call with 1 parameter)
		if (t2->getTokenType() == CExEv_Token::TOK_FUNCTION)
		{
			// The new token must be a closing parenthese
			if (iOpToken == CExEv_Token::TOK_CPAREN)
			{
				Variant *pArgs = (Variant *)t1;
				if (!m_pEECtx->callFunction(t2->getString(), 1, &pArgs, result))
					return setError(E_FUNCTION);
				// pop parameter
				pop();
				// Replace the function by its return value
				pTokenStack->setTokenType(CExEv_Token::TOK_CONST);
				*(Variant *)pTokenStack = result;
			}
			else
				return setError(E_SYNTAX);

			return 0;
		}

		// The second token is a TOK_COMMA (function call with >= 2 parameters)
		if (t2->getTokenType() == CExEv_Token::TOK_COMMA)
		{
			// The new token must be a closing parenthese
			if (iOpToken == CExEv_Token::TOK_CPAREN)
			{
				t3 = t2->pNext;
				t4 = t3->pNext;
				// The third is a constant
				if (t3->getTokenType() == CExEv_Token::TOK_CONST)
				{
					if (t4->getTokenType() == CExEv_Token::TOK_FUNCTION)
					{
						pArgs[0] = (Variant *)t3;
						pArgs[1] = (Variant *)t1;
						if (!m_pEECtx->callFunction(t4->getString(), 1, pArgs, result))
							return setError(E_FUNCTION);
						// pop parameter
						pop();
						pop();
						pop();
						// Replace the function by its return value
						pTokenStack->setTokenType(CExEv_Token::TOK_CONST);
						*(Variant *)pTokenStack = result;
					}
				}
			}
			else
				return setError(E_SYNTAX);

			return 0;
		}

		// The second token is the NULL token
		if (t2->getTokenType() == CExEv_Token::TOK_NULL)
		{
			if (iOpToken == CExEv_Token::TOK_CPAREN)
				return setError(E_SYNTAX);
			if (iOpToken == CExEv_Token::TOK_OPERATOR)
				push();
			// Go on
			return 0;
		}

		setError(E_SYNTAX);
	}

	return m_iError;
}

QString CExEv::getErrorMessage(void)
{
	switch(m_iError)
	{
	default:
		case E_OK:
			return "";		// no error
		case E_ERROR:		// generic error
			return "Generic Error";
		case E_NOMEM:		// not enough memory
			return "Not enough memory";
		case E_EMPTY:		// empty expression string
			return "Empty expression string";
		case E_TOKEN:		// invalid token in expression
			return "invalid token in expression";
		case E_VARIABLE:	// unknown variable
			return "Unknown variable";
		case E_SYNTAX:	// syntax error
			return "Syntax error";
		case E_FUNCTION:	// function call error
			return "Function call error";
		case E_OPERATOR:	// operator call error
			return "Operator call error";
		case E_EOE:		// premature end of expression encountered
			return "Premature end of expression encountered";
	}
}

int CExEv::evalExpression(QString strExprText, Variant& result)
{
	char    bEnd = false;
	int     iTokenType, iTokTop;

	// Do some cleanup
	clear();
	result.setNull();

	// If not specified a context, use our own (default operators, functions, no variables)
	if (m_pEECtx == NULL)
		m_pEECtx = this;

	// Keep a copy of the expression string for ourselves
	if(strExprText.isEmpty())
		strExprText = "";

	// Map operators
    strExprText.replace(" AND ","&&", Qt::CaseInsensitive);
    strExprText.replace(" OR ","||", Qt::CaseInsensitive);


	m_cExprText = m_cExprPtr = new char [strExprText.length()+1];
	if (m_cExprText != NULL)
		strcpy(m_cExprText, (char *)strExprText.toLatin1().constData());
	else
		setError(E_NOMEM);

	// Push a first token on the stack (TOK_NULL)
	newToken();
	push();

	while (!bEnd && !m_iError)
	{
//		dumpStack();

		// The top token type
		iTokTop = pTokenStack->getTokenType();

		// The next token
		iTokenType = getNextToken();

		// A token could not be allocated
		if (iTokenType < 0)
		{
			setError(E_NOMEM);
			continue;
		}

#if DEBUG_CEXEV
String sd;
pCurrToken->dump(sd);
m_sDump += "<TOKEN>";
m_sDump += sd.c_str();
m_sDump += "\r\n";
if (iTokenType == CExEv_Token::TOK_NULL)
{
	bEnd = true;
}
#endif

		// Unknown token
		if (iTokenType == CExEv_Token::TOK_UNKNOWN)
		{
			setError(E_TOKEN);
			continue;
		}

		// Change ident into function if needed
		if (iTokTop == CExEv_Token::TOK_IDENT)
		{
			if (iTokenType == CExEv_Token::TOK_OPAREN)
			{
				pTokenStack->setTokenType(CExEv_Token::TOK_FUNCTION);
				continue;
			}
			else
			{
				Variant result1;
				// Replace variable by its value
				if (m_pEECtx->getValue(pTokenStack->getString(), result1))
				{
					pTokenStack->setTokenType(CExEv_Token::TOK_CONST);
					*(Variant *)pTokenStack = result1;
					// The top token type has changed
					iTokTop = pTokenStack->getTokenType();
				}
				// Sorry: no such variable
				else
				{
					setError(E_VARIABLE);
					continue;
				}
			}
		}

		// Got a value
		if (iTokenType == CExEv_Token::TOK_CONST ||		// A constant
			iTokenType == CExEv_Token::TOK_IDENT)		// A literal
		{
			if (iTokTop == CExEv_Token::TOK_NULL ||		// first token
				iTokTop == CExEv_Token::TOK_OPAREN ||	// after opening parenthese
				iTokTop == CExEv_Token::TOK_OPERATOR ||	// after operator
				iTokTop == CExEv_Token::TOK_FUNCTION ||	// function first argument
				iTokTop == CExEv_Token::TOK_COMMA)		// function non-first argument
			{
				push();
			}
			else
			{
				setError(E_SYNTAX);
			}
			continue;
		}

		// Got an open parenthese
		if (iTokenType == CExEv_Token::TOK_OPAREN)
		{
			// Change from TOK_IDENT to TOK_FUNCTION
			if (iTokTop == CExEv_Token::TOK_IDENT)
			{
				pTokenStack->setTokenType(CExEv_Token::TOK_FUNCTION);
			}
			else
			{
				if (iTokTop == CExEv_Token::TOK_NULL ||		// first token
					iTokTop == CExEv_Token::TOK_OPAREN ||	// after opening parenthese
					iTokTop == CExEv_Token::TOK_OPERATOR ||	// after operator
					iTokTop == CExEv_Token::TOK_FUNCTION ||	// function first argument
					iTokTop == CExEv_Token::TOK_COMMA)		// function non-first argument
				{
					push();
				}
				else
				{
					setError(E_SYNTAX);
				}
			}
			continue;
		}

		// Got a closing parenthese
		if (iTokenType == CExEv_Token::TOK_CPAREN)
		{
			if (iTokTop == CExEv_Token::TOK_CONST ||		// after constant
				iTokTop == CExEv_Token::TOK_IDENT ||		// after variable
				iTokTop == CExEv_Token::TOK_FUNCTION)	// function closing parenthese
			{
				reduce(iTokenType);
			}
			else
			{
				setError(E_SYNTAX);
			}
			continue;
		}

		// Got an operator
		if (iTokenType == CExEv_Token::TOK_OPERATOR)
		{
			// Check if unary operator
			if (iTokTop == CExEv_Token::TOK_NULL ||		// first
				iTokTop == CExEv_Token::TOK_OPAREN ||	// after (
				iTokTop == CExEv_Token::TOK_FUNCTION ||	// function arg
				iTokTop == CExEv_Token::TOK_COMMA)		// function arg
			{
				push();
			}
			else
			{
				if (iTokTop == CExEv_Token::TOK_CONST)
				{
					if (pTokenStack->pNext->getTokenType() == CExEv_Token::TOK_OPERATOR &&
						pTokenStack->pNext->pNext->getTokenType() == CExEv_Token::TOK_CONST)
					{
						reduce(iTokenType);
					}
					else
					{
						push();
					}
				}
				else
				{
					setError(E_SYNTAX);
				}
			}
			continue;
		}

		// Got a comma
		if (iTokenType == CExEv_Token::TOK_COMMA)
		{
			if (iTokTop == CExEv_Token::TOK_CONST &&
				(pTokenStack->pNext->getTokenType() == CExEv_Token::TOK_COMMA ||
				 pTokenStack->pNext->getTokenType() == CExEv_Token::TOK_FUNCTION))
			{
				push();
			}
			else
			{
				setError(E_SYNTAX);
			}
			continue;
		}

		// End of expression string
		if (iTokenType == CExEv_Token::TOK_NULL)
		{
			if (iTokTop == CExEv_Token::TOK_CONST)
			{
				reduce(iTokenType);
				bEnd = true;
			}
			else
			{
				setError(E_SYNTAX);
			}
			continue;
		}

		setError(E_SYNTAX);
	}

	if (!m_iError)
	{
		if (pTokenStack == NULL ||
			pTokenStack->pNext == NULL ||
			pTokenStack->pNext->pNext != NULL ||
			pTokenStack->getTokenType() != CExEv_Token::TOK_CONST)
		{
			setError(E_ERROR);
		}
	}

#if DEBUG_CEXEV
dumpStack();
#endif

	if (!m_iError)
	{
		result = *(Variant *)pTokenStack;
	}

	return m_iError;
}

void CExEv::dumpStack()
{
#if DEBUG_CEXEV
  String s;
  CExEv_Token *pToken;
  m_sDump += "CExEv::Stack:\r\n";
  for (pToken = pTokenStack; pToken != NULL; pToken = pToken->pNext)
  {
    pToken->dump(s);
	m_sDump += s.c_str();
	m_sDump += "\r\n";
  }
  m_sDump += "\r\n";
#endif
}


} // namespace cexev

// For debugging a demo
#if 0
using namespace variant;
using namespace cexev;

//	A sample context.
class CMyExEvCtx : public CExEvCtx
{
public:
	CMyExEvCtx() : CExEvCtx() {}
	virtual ~CMyExEvCtx() {}

	// Override to return values for variables
	virtual bool getValue(const char *cIdent, Variant& result);
};

bool CMyExEvCtx::getValue(const char *cIdent, Variant& result)
{
	if(!qstricmp(cIdent,"Cpk"))
	{
		result.setReal(1.33);
		return true;
	}

	return CExEvCtx::getValue(cIdent, result);
}

void test_main(void)
{
	QString strString;
	CMyExEvCtx eectx;
	CExEv cExpression;
	Variant vres;
	String sres, sdump;

	QString strExpression = "Cpk > 1 || R_R < 3";

	cExpression.setContext(&eectx);
	int iStatus = cExpression.evalExpression(strExpression, vres);

	cExpression.getDump(sdump);

	if(iStatus == CExEv::E_OK)
	{
		vres.asString(sres);
		strString = "Evaluation OK: '";
		strString += strExpression + "' = ";
		strString += sres.c_str();
	}
	else
	{
		strString = "Evaluation FAILED: '";
		strString += strExpression + "' = ";
		strString += cExpression.getErrorMessage();
	}
}


#endif 

