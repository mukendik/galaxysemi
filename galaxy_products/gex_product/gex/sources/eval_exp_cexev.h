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

#if !defined(CEXEV_H)
#define CEXEV_H 1

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "eval_exp_variant.h"

namespace cexev
{

typedef bool (*ExEvFunction)(int nArgc, Variant **vArgs, Variant& result);
class CExEvCtx;
class CExEv;
class CExEv_Token;

/*****************************************************************************************
	CExEvCtx class: provides the virtual space of variables, operators and functions
	needed by CExEv when evaluating an expression. Also defines valid chars for
	identifiers (variable and function names).

*/
class CExEvCtx
{
public:
	CExEvCtx();
	virtual ~CExEvCtx();

	// Override to define valid identifiers
	virtual bool isValidIdentStartChar(int ch);
	virtual bool isValidIdentChar(int ch);

	// Override to return values for variables
	virtual bool getValue(const char *cIdent, Variant& result);

	// Override to define new functions (or override default functions)
	virtual bool callFunction(const char *cFunction, int nArgc, Variant **vArgs,
								Variant& result);
	// Override to override default operators (!?)
	virtual bool callOperator(const char *cOperator, int nArgc, Variant **vArgs,
								Variant& result);


	/** TODO: implement
	*/
	bool funcDefine(const char *cFuncName, const char *cSignature, ExEvFunction func) {
		return funcDefine(cFuncName, cSignature, func, 1);
	}

protected:
	/** TODO: implement
	*/
	bool funcDefine(const char *cFuncName, const char *cSignature, ExEvFunction func, int coll);
	void funcFree();

protected:
	struct SFunctionDef;
	bool doCallFunction(SFunctionDef *pFnDefs, const char *cFunction,
						int nArgc, Variant **vArgs, Variant& result);

	// Default operators (return false on failure)
	static bool opUnaryPlus(int nArgc, Variant **vArgs, Variant& result);
	static bool opUnaryMinus(int nArgc, Variant **vArgs, Variant& result);
	static bool opUnaryBitNot(int nArgc, Variant **vArgs, Variant& result);
	static bool opUnaryLogNot(int nArgc, Variant **vArgs, Variant& result);
	static bool opPlusSS_(int nArgc, Variant **vArgs, Variant& result);
	static bool opPlusS_S(int nArgc, Variant **vArgs, Variant& result);
	static bool opMul(int nArgc, Variant **vArgs, Variant& result);
	static bool opDiv(int nArgc, Variant **vArgs, Variant& result);
	static bool opMod(int nArgc, Variant **vArgs, Variant& result);
	static bool opAdd(int nArgc, Variant **vArgs, Variant& result);
	static bool opSub(int nArgc, Variant **vArgs, Variant& result);
	static bool opBitLeft(int nArgc, Variant **vArgs, Variant& result);
	static bool opBitRight(int nArgc, Variant **vArgs, Variant& result);
	static bool opLT(int nArgc, Variant **vArgs, Variant& result);
	static bool opGT(int nArgc, Variant **vArgs, Variant& result);
	static bool opLE(int nArgc, Variant **vArgs, Variant& result);
	static bool opGE(int nArgc, Variant **vArgs, Variant& result);
	static bool opEQ(int nArgc, Variant **vArgs, Variant& result);
	static bool opNE(int nArgc, Variant **vArgs, Variant& result);
	static bool opBitAnd(int nArgc, Variant **vArgs, Variant& result);
	static bool opBitXor(int nArgc, Variant **vArgs, Variant& result);
	static bool opBitOr(int nArgc, Variant **vArgs, Variant& result);
	static bool opLogAnd(int nArgc, Variant **vArgs, Variant& result);
	static bool opLogOr(int nArgc, Variant **vArgs, Variant& result);

	// Built-in functions (return false on failure)
	static bool fnSin(int nArgc, Variant **vArgs, Variant& result);
	static bool fnMax(int nArgc, Variant **vArgs, Variant& result);
	static bool fnMin(int nArgc, Variant **vArgs, Variant& result);
	static bool fnLog10(int nArgc, Variant **vArgs, Variant& result);
	static bool fnLn(int nArgc, Variant **vArgs, Variant& result);
	static bool fnExp(int nArgc, Variant **vArgs, Variant& result);
	static bool fnPower(int nArgc, Variant **vArgs, Variant& result);
    static bool fnAbs(int nArgc, Variant **vArgs, Variant& result);

protected:
	struct SFunctionDef
	{
		const char *cFuncName;
		const char *cSignature;
		const char *cFuncDesc;
		ExEvFunction func;
	};
	static SFunctionDef m_stdOpDef[];	// Built-in operators
	static SFunctionDef m_stdFuncDef[];	// Built-in functions
};

/*****************************************************************************************
	CExEv class: this is the class to be used to evaluate a C syntax expression string.
	Handles bool,int,dword,real(double),string data (see the Variant class).
	If you need to provide your variables, define a subclass of CExEvCtx, override the 
	getValue() method to return the values when required.
	To define your own functions override the callFunction() method to call your function
	based on the function name and arguments passed.
	To redefine the default operators override the callOperator() method. Anyway, the
	operators' precedence cannot be changed. See th eimplementation for details.
	To make CExEv use your context instantiate your context class and pass the address to
	the CExEv contructor or setContext() method. Further calls to evalExpression() will
	use that context. By passing NULL resets the context to the default.
	NOTE: CExEv stores the address of your context object, so you MUST NOT destroy it
	while CExEv might use it.
*/
class CExEv :
	private CExEvCtx
{
public:
	CExEv();
	CExEv(CExEvCtx *pEECtx);
	virtual ~CExEv();

	/**
		Evaluates the expression in 'cExprText' and places the result in 'result'.
		return: E_OK(=0), on success, error code on failure
	*/
	int evalExpression(QString strExprText, Variant& result);

	/**
		Changes the default context used for evaluation
	*/
	void setContext(CExEvCtx *pEECtx) {
		m_pEECtx = pEECtx;
	}

	/**
		Used for debugging
	*/
	void getDump(String& sdump) {
		sdump = m_sDump;
	}

	// Return the error code. //
	int getError() {
		return m_iError;
	}

	// Return error message
	QString getErrorMessage(void);

public:
	/**
		Evaluation return codes
	*/
	enum {
		E_OK		= 0,	// no error
		E_ERROR		= 1,	// generic error
		E_NOMEM		= 2,	// not enough memory
		E_EMPTY		= 3,	// empty expression string
		E_TOKEN     = 4,	// invalid token in expression
		E_VARIABLE  = 5,	// unknown variable
		E_SYNTAX    = 6,	// syntax error
		E_FUNCTION  = 7,	// function call error
		E_OPERATOR  = 8,	// operator call error
		E_EOE		= 9,	// premature end of expression encountered
	};

protected:
	void clear();
	int setError(int iErrorCode);
	void dumpStack();
	CExEv_Token *pop();
	CExEv_Token *push();
	CExEv_Token *swapTop();
	CExEv_Token *newToken();
	int  getNextToken();
	int reduce(int iOpToken);
	int getOperatorPriority(const char *cOp);

protected:
	char *m_cExprText, *m_cExprPtr;
	CExEv_Token *pTokenStack, *pTokenDel, *pCurrToken;
	int    m_iError;
	CExEvCtx *m_pEECtx;
	String m_sDump;
};


} // namespace cexev

using namespace cexev;

#endif // CEXEV_H
