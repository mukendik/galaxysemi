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

#if !defined(VARIANT_H)
#define VARIANT_H 1

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>

namespace variant
{

typedef std::string String;
//typedef std::basic_string<char> String;


/*****************************************************************************************
	Variant class: to hold a simple data value (bool,int,dword,double,string,binary,ptr)

*/
class Variant
{
public:
	// Data types
	enum {
		T_NULL,   // 0: no data
		T_BOOL,   // B: boolean data
		T_INT,    // I: int data
		T_DWORD,  // D: long data
		T_REAL,   // R: double precision data
		T_STRING, // S: character string data
		T_BINARY, // X: binary data
		T_PTR,    // P: pointer
	};

public:
	// CTOR, DTOR
	Variant();
	virtual ~Variant();

	// Get data type
	int getType() {
		return m_type;
	}
	// Get data type as char (see above)
	char getTypeChar() {
		return m_cTypeChar[m_type];
	}

	// Copy operator
	void operator=(const Variant& v);

	// These set methods change the data type AND value!
	void setNull();
	void setBool(bool bValue);
	void setInt(int iValue);
	void setDWORD(long dwValue);
	void setReal(double dReal);
	void setString(const char *cValue);
	void appendToString(const char *cStringToAppend);
	void appendToString(int iCharToAppend);
	void setBinary(const unsigned char* bValue, int nBytes);
	void setPtr(void *ptr);

	// Test data type
	bool isNull()		{ return m_type == T_NULL; }
	bool isBool()		{ return m_type == T_BOOL; }
	bool isInt()		{ return m_type == T_INT; }
	bool isDWORD()		{ return m_type == T_DWORD; }
	bool isReal()		{ return m_type == T_REAL; }
	bool isString()		{ return m_type == T_STRING; }
	bool isBinary()		{ return m_type == T_BINARY; }
	bool isPtr()		{ return m_type == T_PTR; }

	/**
		Access data value.
		Be sure to use the method that matches the data type, otherwise a default value
		is returned (false, 0, 0.0, NULL) and NOT a converted one!!!
	*/
	bool getBool();
	int  getInt();
	long getDWORD();
	double getReal();
	const char *getString();
	const unsigned char* getBinary();
	void *getPtr();

	/**
		Return the data size. It is the sizeof() for the simple data types (int,...)
		For string it's 0 for NULL string, strlen()+1 for valid string.
		For binary it's 0 for NULL data, or >0 the number of bytes.
		For ptr it's sizeof(void *)!!!
	*/
	int getSize() {
		return m_size;
	}
	/**
		Get the data in buffer. Copies no more than the least of iBufSize and getSize().
		Return: the number of bytes copied.
		NOTE: For string, if iBufSize < getSize() the ending '\0' is not copied!!!
	*/
	int getData(void *pBuf, int iBufSize);

	/**
		These conversion function do not always succeed, so check the return value!
		Return: 0 on failure, <>0 on success
	*/
	bool asString(String& s) { return asString(s, NULL); } // ok always
	bool asString(String& s, const char *cFormat);			// ok always
	bool asInt(int& iRes);		// ok for: int,dword,real,bool
	bool asDWORD(long& dwRes);	// ok for: int,dword,real,bool
	bool asReal(double& rRes);	// ok for: int,dword,real,bool
	bool asBool(bool& bRes);	// ok for: int,dword,real,bool

public:
	// Operators (return false if the operation could not be performed)
	static bool compare(Variant& v1, Variant& v2, int& rel); // fails if not comparable!
	static bool MUL(Variant& v1, Variant& v2, Variant& vres);
	static bool DIV(Variant& v1, Variant& v2, Variant& vres);
	static bool MOD(Variant& v1, Variant& v2, Variant& vres);
	static bool ADD(Variant& v1, Variant& v2, Variant& vres);
	static bool SUB(Variant& v1, Variant& v2, Variant& vres);
	static bool BITLEFT(Variant& v1, Variant& v2, Variant& vres);
	static bool BITRIGHT(Variant& v1, Variant& v2, Variant& vres);
	static bool LT(Variant& v1, Variant& v2, Variant& vres);
	static bool GT(Variant& v1, Variant& v2, Variant& vres);
	static bool LE(Variant& v1, Variant& v2, Variant& vres);
	static bool GE(Variant& v1, Variant& v2, Variant& vres);
	static bool EQ(Variant& v1, Variant& v2, Variant& vres);
	static bool NE(Variant& v1, Variant& v2, Variant& vres);
	static bool BITAND(Variant& v1, Variant& v2, Variant& vres);
	static bool BITXOR(Variant& v1, Variant& v2, Variant& vres);
	static bool BITOR(Variant& v1, Variant& v2, Variant& vres);
	static bool LOGAND(Variant& v1, Variant& v2, Variant& vres);
	static bool LOGOR(Variant& v1, Variant& v2, Variant& vres);

protected:
	// Free the dynamic data (for string and binary)
	void free();

protected:
	int m_type;
	union {
		bool data_bool;
		int  data_int;
		long data_DWORD;
		double data_real;
		char *data_string;
		unsigned char* data_binary;
		void *data_ptr;
	} m_d;
	int m_size;
	static const char *m_cTypeChar;
};



} // namespace variant

using namespace variant;

#endif // VARIANT_H
