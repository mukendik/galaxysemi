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
#include <math.h>
#include "eval_exp_variant.h"

//--------------------------------------------------------------------------------

namespace variant
{

/*****************************************************************************************
 */

const char *Variant::m_cTypeChar = "0BIDRSXP";

Variant::Variant()
{
	m_type = T_NULL;
	m_size = 0;
}

Variant::~Variant()
{
	free();
}

void Variant::operator=(const Variant& v)
{
	free();

	if (v.m_type == T_STRING)
	{
		this->setString(v.m_d.data_string);
	}
	else if (v.m_type == T_BINARY)
	{
		this->setBinary(v.m_d.data_binary, v.m_size);
	}
	else
	{
		m_type = v.m_type;
		m_d = v.m_d;
		m_size = v.m_size;
	}
}

void Variant::setNull()
{
	free();

	m_type = T_NULL;
	m_size = 0;
}

void Variant::setBool(bool bValue)
{
	free();

	m_type = T_BOOL;
	m_size = sizeof(m_d.data_bool);

	m_d.data_bool = bValue;
}

void Variant::setInt(int iValue)
{
	free();

	m_type = T_INT;
	m_size = sizeof(m_d.data_int);

	m_d.data_int = iValue;
}

void Variant::setDWORD(long dwValue)
{
	free();

	m_type = T_DWORD;
	m_size = sizeof(m_d.data_DWORD);

	m_d.data_DWORD = dwValue;
}

void Variant::setReal(double dReal)
{
	free();

	m_type = T_REAL;
	m_size = sizeof(m_d.data_real);

	m_d.data_real = dReal;
}

void Variant::setString(const char *cValue)
{
	free();

	m_type = T_STRING;
	m_size = (cValue ? strlen(cValue) + 1: 0);

	if (m_size) {
		m_d.data_string = (char *) new char[m_size];
		memcpy(m_d.data_string, cValue, m_size);
	}
	else {
		m_d.data_string = NULL;
	}
}

void Variant::appendToString(const char *cStringToAppend)
{
	if (cStringToAppend)
	{
		String s;
		asString(s);
		s += cStringToAppend;
		setString(s.c_str());
	}
}

void Variant::appendToString(int iCharToAppend)
{
	if (iCharToAppend)
	{
		String s;
		asString(s);
		s += (char)iCharToAppend;
		setString(s.c_str());
	}
}

void Variant::setBinary(const unsigned char* bValue, int nBytes)
{
	free();

	m_type = T_BINARY;
	m_size = (nBytes > 0) ? nBytes : 0;

	if (m_size > 0) {
		m_d.data_binary = new unsigned char[m_size];
		memcpy(m_d.data_binary, bValue, m_size);
	}
	else {
		m_d.data_binary = NULL;
	}
}

void Variant::setPtr(void *ptr)
{
	free();

	m_type = T_PTR;
	m_size = sizeof(m_d.data_ptr);

	m_d.data_ptr = ptr;
}

bool Variant::getBool()
{
	if (isBool()) {
		return m_d.data_bool;
	}

	return false;
}

int  Variant::getInt()
{
	if (isInt()) {
		return m_d.data_int;
	}

	return 0;
}

long Variant::getDWORD()
{
	if (isDWORD()) {
		return m_d.data_DWORD;
	}

	return 0Lu;
}

double Variant::getReal()
{
	if (isReal()) {
		return m_d.data_real;
	}

	return 0.0;
}

const char *Variant::getString()
{
	if (isString()) {
		return m_d.data_string;
	}

	return NULL;
}

const unsigned char* Variant::getBinary()
{
	if (isBinary()) {
		return m_d.data_binary;
	}

	return NULL;
}

void *Variant::getPtr()
{
	if (isPtr()) {
		return m_d.data_ptr;
	}

	return NULL;
}

int Variant::getData(void *pBuf, int iBufSize)
{
	int n = m_size;
	if (n > 0) {
		if (iBufSize < n)
			n = iBufSize;
		memcpy(pBuf, &m_d.data_DWORD, n);
	}
	return n;
}

bool Variant::asString(String& s, const char *cFormat)
{
	char cb[20];
	switch (m_type) {

	case T_NULL:
		s = "";
		break;

	case T_BOOL:
		s = m_d.data_bool ? "1" : "0";
		break;

	case T_INT:
		sprintf(cb, (cFormat ? cFormat : "%d"), m_d.data_int);
		s = cb;
		break;

	case T_DWORD:
		sprintf(cb, (cFormat ? cFormat : "%lu"), m_d.data_DWORD);
		s = cb;
		break;

	case T_REAL:
		sprintf(cb, (cFormat ? cFormat : "%.4lf"), m_d.data_real);
		s = cb;
		break;

	case T_STRING:
		s = m_d.data_string ? m_d.data_string : "";
		break;

	case T_BINARY: {
		s = "";
		for (int i = 0; i < m_size; i++) {
			sprintf(cb, cFormat ? cFormat : "%02X",
				0xff & (int) m_d.data_binary[i]);
			s += cb;
		}
		break;
	}

	case T_PTR:
		sprintf(cb, "%p", m_d.data_ptr);
		s = cb;
		break;

	default:
		s = "";
		break;
	}

	return true;
}

bool Variant::asInt(int& iRes)
{
	bool bOk = true;

	switch (m_type) {

	case T_INT:
		iRes = m_d.data_int;
		break;

	case T_DWORD:
		iRes = (int)m_d.data_DWORD;
		break;

	case T_REAL:
		iRes = (int)floor(m_d.data_real);
		break;

	case T_BOOL:
		iRes = m_d.data_bool ? 1 : 0;
		break;

	default:
		bOk = false;
		break;
	}

	return bOk;
}

bool Variant::asDWORD(long& dwRes)
{
	bool bOk = true;

	switch (m_type) {

	case T_INT:
		dwRes = (long)m_d.data_int;
		break;

	case T_DWORD:
		dwRes = m_d.data_DWORD;
		break;

	case T_REAL:
		dwRes = (long)floor(m_d.data_real);
		break;

	case T_BOOL:
		dwRes = m_d.data_bool ? 1Lu : 0Lu;
		break;

	default:
		bOk = false;
		break;
	}

	return bOk;
}

bool Variant::asReal(double& rRes)
{
	bool bOk = true;

	switch (m_type) {

	case T_INT:
		rRes = (double)m_d.data_int;
		break;

	case T_DWORD:
		rRes = (double)m_d.data_DWORD;
		break;

	case T_REAL:
		rRes = m_d.data_real;
		break;

	case T_BOOL:
		rRes = m_d.data_bool ? 1.0 : 0.0;
		break;

	default:
		bOk = false;
		break;
	}

	return bOk;
}

bool Variant::asBool(bool& bRes)
{
	bool bOk = true;

	switch (m_type) {

	case T_INT:
		bRes = (m_d.data_int != 0);
		break;

	case T_DWORD:
		bRes = (m_d.data_DWORD != 0);
		break;

	case T_REAL:
		bRes = (m_d.data_real != 0.0);
		break;

	case T_BOOL:
		bRes = m_d.data_bool ? true : false;
		break;

	default:
		bOk = false;
		break;
	}

	return bOk;
}


void Variant::free()
{
	switch (m_type) {

	case T_STRING:
		delete m_d.data_string;
		m_d.data_string = NULL;
		m_size = 0;
		break;

	case T_BINARY:
		delete m_d.data_binary;
		m_d.data_binary = NULL;
		m_size = 0;
		break;

	default:
		break;
	}
}

bool Variant::compare(Variant& v1, Variant& v2, int& rel)
{
	bool bOk = true;

	if (v1.isString())
	{
		if (v2.isString())
			rel = strcmp(v1.getString(), v2.getString());
		else
			bOk = false;
	}
	else if (v2.isString())
	{
		bOk = false;
	}
	else if (v1.isReal() || v2.isReal())
	{
		double r1, r2;
		if (v1.asReal(r1) && v2.asReal(r2))
			rel = (r1 < r2) ? -1 : (r1 > r2) ? 1 : 0;
		else
			bOk = false;
	}
	else if (v1.isInt())
	{
		int i1 = v1.getInt();
		if (v2.isInt())
			rel = i1 - v2.getInt();
		else if (v2.isDWORD())
		{
			if (i1 < 0)
				rel = -1; // v2 is always >= 0 (unsigned)
			else
			{
				long dw1 = (long)i1,
					dw2 = (long)v2.getDWORD();
				rel = (dw1 < dw2) ? -1 : (dw1 == dw2) ? 0 : 1;
			}
		}
		else
		{
			int i2;
			if (v2.asInt(i2))
				rel = (i1 < i2) ? -1 : (i1 == i2) ? 0 : 1;
			else
				bOk = false;
		}
	}
	else if (v1.isDWORD())
	{
		long dw1 = v1.getDWORD();
		if (v2.isDWORD())
		{
			long dw2 = (long)v2.getDWORD();
			rel = (dw1 < dw2) ? -1 : (dw1 == dw2) ? 0 : 1;
		}
		else if (v2.isInt())
		{
			if (v2.getInt() < 0)
				rel = 1; // v1 is always >= 0 (unsigned)
			else
			{
				long dw2 = (long)v2.getInt();
				rel = (dw1 < dw2) ? -1 : (dw1 == dw2) ? 0 : 1;
			}
		}
		else
		{
			long dw2;
			if (v2.asDWORD(dw2))
				rel = (dw1 < dw2) ? -1 : (dw1 == dw2) ? 0 : 1;
			else
				bOk = false;
		}
	}
	else
	{
		int i1, i2;
		if (v1.asInt(i1) && v2.asInt(i2))
			rel = (i1 < i2) ? -1 : (i1 > i2) ? 1 : 0;
		else
			bOk = false;
	}

	return bOk;
}

bool Variant::MUL(Variant& v1, Variant& v2, Variant& vres)
{
	bool bOk = true;

	if (v2.isInt())
	{
		if (v1.isInt())
			vres.setInt(v2.getInt() * v1.getInt());
		else if (v1.isDWORD())
			vres.setDWORD((long)v2.getInt() * v1.getDWORD());
		else if (v1.isReal())
			vres.setReal((double)v2.getInt() * v1.getReal());
		else if (v1.isBool())
			vres.setInt(v1.getBool() ? v2.getInt() : 0);
		else
			bOk = false;
	}
	else if (v2.isDWORD())
	{
		if (v1.isInt())
			vres.setDWORD(v2.getDWORD() * (long)v1.getInt());
		else if (v1.isDWORD())
			vres.setDWORD(v2.getDWORD() * v1.getDWORD());
		else if (v1.isReal())
			vres.setReal((double)v2.getDWORD() * v1.getReal());
		else if (v1.isBool())
			vres.setDWORD(v1.getBool() ? v2.getDWORD() : 0Lu);
		else
			bOk = false;
	}
	else if (v2.isReal())
	{
		if (v1.isInt())
			vres.setReal(v2.getReal() * (double)v1.getInt());
		else if (v1.isDWORD())
			vres.setReal(v2.getReal() * (double)v1.getDWORD());
		else if (v1.isReal())
			vres.setReal(v2.getReal() * v1.getReal());
		else if (v1.isBool())
			vres.setReal(v1.getBool() ? v2.getReal() : 0.0);
		else
			bOk = false;
	}
	else if (v2.isBool())
	{
		if (v1.isInt())
			vres.setInt(v2.getBool() ? v1.getInt() : 0);
		else if (v1.isDWORD())
			vres.setDWORD(v2.getBool() ? v1.getDWORD() : 0);
		else if (v1.isReal())
			vres.setReal(v2.getBool() ? v1.getReal() : 0);
		else if (v1.isBool())
			vres.setInt(v2.getBool() && v1.getBool());
		else
			bOk = false;
	}
	else
	{
		bOk = false;
	}

	return bOk;
}

bool Variant::DIV(Variant& v1, Variant& v2, Variant& vres)
{
	bool bOk = true;

	if (v2.isInt())
	{
		if (v2.getInt() == 0)
			bOk = false;
		else if (v1.isInt())
			vres.setInt(v1.getInt() / v2.getInt());
		else if (v1.isDWORD())
			vres.setDWORD(v1.getDWORD() / (long)v2.getInt());
		else if (v1.isReal())
			vres.setReal(v1.getReal() / (double)v2.getInt());
		else if (v1.isBool())
			vres.setReal(v1.getBool() ? 1.0 / (double)v2.getInt() : 0.0);
		else
			bOk = false;
	}
	else if (v2.isDWORD())
	{
		if (v2.getDWORD() == 0Lu)
			bOk = false;
		else if (v1.isInt())
			vres.setInt(v1.getInt() / (int)v2.getDWORD());
		else if (v1.isDWORD())
			vres.setDWORD(v1.getDWORD() / v2.getDWORD());
		else if (v1.isReal())
			vres.setReal(v1.getReal() / (double)v2.getDWORD());
		else if (v1.isBool())
			vres.setReal(v1.getBool() ? 1.0 / (double)v2.getDWORD() : 0.0);
		else
			bOk = false;
	}
	else if (v2.isReal())
	{
		const double eps = 1e-12;
		if (v2.getReal() < eps && v2.getReal() > -eps)
			bOk = false;
		else if (v1.isInt())
			vres.setReal((double)v1.getInt() / v2.getReal());
		else if (v1.isDWORD())
			vres.setReal((double)v1.getDWORD() / v2.getReal());
		else if (v1.isReal())
			vres.setReal(v1.getReal() / v2.getReal());
		else if (v1.isBool())
			vres.setReal(v1.getBool() ? 1.0 / v2.getReal() : 0.0);
		else
			bOk = false;
	}
	else
	{
		bOk = false;
	}

	return bOk;
}

bool Variant::MOD(Variant& v1, Variant& v2, Variant& vres)
{
	bool bOk = true;

	if (v2.isInt())
	{
		if (v2.getInt() == 0)
			bOk = false;
		else if (v1.isInt())
			vres.setInt(v1.getInt() % v2.getInt());
		else if (v1.isDWORD())
			vres.setDWORD(v1.getDWORD() % (long)v2.getInt());
		else
			bOk = false;
	}
	else if (v2.isDWORD())
	{
		if (v2.getDWORD() == 0Lu)
			bOk = false;
		else if (v1.isInt())
			vres.setInt(v1.getInt() % (int)v2.getDWORD());
		else if (v1.isDWORD())
			vres.setDWORD(v1.getDWORD() % v2.getDWORD());
		else
			bOk = false;
	}
	else
	{
		bOk = false;
	}

	return bOk;
}

bool Variant::ADD(Variant& v1, Variant& v2, Variant& vres)
{
	bool bOk = true;

	if (v2.isInt())
	{
		if (v1.isInt())
			vres.setInt(v1.getInt() + v2.getInt());
		else if (v1.isDWORD())
			vres.setDWORD(v1.getDWORD() + (long)v2.getInt());
		else if (v1.isReal())
			vres.setReal(v1.getReal() + (double)v2.getInt());
		else if (v1.isBool())
			vres.setInt((v1.getBool() ? 1 : 0) + v2.getInt());
		else
			bOk = false;
	}
	else if (v2.isDWORD())
	{
		if (v1.isInt())
			vres.setDWORD((long)v1.getInt() + v2.getDWORD());
		else if (v1.isDWORD())
			vres.setDWORD(v1.getDWORD() + v2.getDWORD());
		else if (v1.isReal())
			vres.setReal(v1.getReal() + (double)v2.getDWORD());
		else if (v1.isBool())
			vres.setDWORD((long)(v1.getBool() ? 1 : 0) + v2.getDWORD());
		else
			bOk = false;
	}
	else if (v2.isReal())
	{
		if (v1.isInt())
			vres.setReal((double)v1.getInt() + v2.getReal());
		else if (v1.isDWORD())
			vres.setReal((double)v1.getDWORD() + v2.getReal());
		else if (v1.isReal())
			vres.setReal(v1.getReal() + v2.getReal());
		else if (v1.isBool())
			vres.setReal((v1.getBool() ? 1.0 : 0.0) + v2.getReal());
		else
			bOk = false;
	}
	else if (v2.isBool())
	{
		if (v1.isInt())
			vres.setInt(v1.getInt() + (v2.getBool() ? 1 : 0));
		else if (v1.isDWORD())
			vres.setDWORD(v1.getDWORD() + (v2.getBool() ? 1 : 0));
		else if (v1.isReal())
			vres.setReal(v1.getReal() + (v2.getBool() ? 1.0 : 0.0));
		else if (v1.isBool())
			vres.setInt((int)v1.getBool() + (int)v2.getBool());
		else
			bOk = false;
	}
	else
	{
		bOk = false;
	}

	return bOk;
}

bool Variant::SUB(Variant& v1, Variant& v2, Variant& vres)
{
	bool bOk = true;

	if (v2.isInt())
	{
		if (v1.isInt())
			vres.setInt(v1.getInt() - v2.getInt());
		else if (v1.isDWORD())
			vres.setDWORD(v1.getDWORD() - (long)v2.getInt());
		else if (v1.isReal())
			vres.setReal(v1.getReal() - (double)v2.getInt());
		else if (v1.isBool())
			vres.setInt((v1.getBool() ? 1 : 0) - v2.getInt());
		else
			bOk = false;
	}
	else if (v2.isDWORD())
	{
		if (v1.isInt())
			vres.setDWORD((long)v1.getInt() - v2.getDWORD());
		else if (v1.isDWORD())
			vres.setDWORD(v1.getDWORD() - v2.getDWORD());
		else if (v1.isReal())
			vres.setReal(v1.getReal() - (double)v2.getDWORD());
		else if (v1.isBool())
			vres.setDWORD((long)(v1.getBool() ? 1 : 0) - v2.getDWORD());
		else
			bOk = false;
	}
	else if (v2.isReal())
	{
		if (v1.isInt())
			vres.setReal((double)v1.getInt() - v2.getReal());
		else if (v1.isDWORD())
			vres.setReal((double)v1.getDWORD() - v2.getReal());
		else if (v1.isReal())
			vres.setReal(v1.getReal() - v2.getReal());
		else if (v1.isBool())
			vres.setReal((v1.getBool() ? 1.0 : 0.0) - v2.getReal());
		else
			bOk = false;
	}
	else if (v2.isBool())
	{
		if (v1.isInt())
			vres.setInt(v1.getInt() - (v2.getBool() ? 1 : 0));
		else if (v1.isDWORD())
			vres.setDWORD(v1.getDWORD() - (v2.getBool() ? 1 : 0));
		else if (v1.isReal())
			vres.setReal(v1.getReal() - (v2.getBool() ? 1.0 : 0.0));
		else if (v1.isBool())
			vres.setInt((int)v1.getBool() - (int)v2.getBool());
		else
			bOk = false;
	}
	else
	{
		bOk = false;
	}

	return bOk;
}

bool Variant::BITLEFT(Variant& v1, Variant& v2, Variant& vres)
{
	bool bOk = true;

	int nb = 0;
	if (v2.isInt())
		nb = v2.getInt();
	else if (v2.isDWORD())
		nb = v2.getDWORD();
	else
		bOk = false;

	if (bOk)
	{
		if (v1.isInt())
			vres.setInt(v1.getInt() << nb);
		else if (v1.isDWORD())
			vres.setDWORD(v1.getDWORD() << nb);
		else
			bOk = false;
	}

	return bOk;
}

bool Variant::BITRIGHT(Variant& v1, Variant& v2, Variant& vres)
{
	bool bOk = true;

	int nb = 0;
	if (v2.isInt())
		nb = v2.getInt();
	else if (v2.isDWORD())
		nb = v2.getDWORD();
	else
		bOk = false;

	if (bOk)
	{
		if (v1.isInt())
			vres.setInt(v1.getInt() >> nb);
		else if (v1.isDWORD())
			vres.setDWORD(v1.getDWORD() >> nb);
		else
			bOk = false;
	}

	return bOk;
}

bool Variant::LT(Variant& v1, Variant& v2, Variant& vres)
{
	int rel;
	if (compare(v1, v2, rel))
	{
		vres.setBool(rel < 0);
		return true;
	}
	return false;
}

bool Variant::GT(Variant& v1, Variant& v2, Variant& vres)
{
	int rel;
	if (compare(v1, v2, rel))
	{
		vres.setBool(rel > 0);
		return true;
	}
	return false;
}

bool Variant::LE(Variant& v1, Variant& v2, Variant& vres)
{
	int rel;
	if (compare(v1, v2, rel))
	{
		vres.setBool(rel <= 0);
		return true;
	}
	return false;
}

bool Variant::GE(Variant& v1, Variant& v2, Variant& vres)
{
	int rel;
	if (compare(v1, v2, rel))
	{
		vres.setBool(rel >= 0);
		return true;
	}
	return false;
}

bool Variant::EQ(Variant& v1, Variant& v2, Variant& vres)
{
	int rel;
	if (compare(v1, v2, rel))
	{
		vres.setBool(rel == 0);
		return true;
	}
	return false;
}

bool Variant::NE(Variant& v1, Variant& v2, Variant& vres)
{
	int rel;
	if (compare(v1, v2, rel))
	{
		vres.setBool(rel != 0);
		return true;
	}
	return false;
}

bool Variant::BITAND(Variant& v1, Variant& v2, Variant& vres)
{
	bool bOk = true;

	if (v2.isInt())
	{
		if (v1.isInt())
			vres.setInt(v1.getInt() & v2.getInt());
		else if (v1.isDWORD())
			vres.setDWORD(v1.getDWORD() & (long)v2.getInt());
		else
			bOk = false;
	}
	else if (v2.isDWORD())
	{
		if (v1.isInt())
			vres.setDWORD((long)v1.getInt() & v2.getDWORD());
		else if (v1.isDWORD())
			vres.setDWORD(v1.getDWORD() & v2.getDWORD());
		else
			bOk = false;
	}
	else
	{
		bOk = false;
	}

	return bOk;
}

bool Variant::BITXOR(Variant& v1, Variant& v2, Variant& vres)
{
	bool bOk = true;

	if (v2.isInt())
	{
		if (v1.isInt())
			vres.setInt(v1.getInt() ^ v2.getInt());
		else if (v1.isDWORD())
			vres.setDWORD(v1.getDWORD() ^ (long)v2.getInt());
		else
			bOk = false;
	}
	else if (v2.isDWORD())
	{
		if (v1.isInt())
			vres.setDWORD((long)v1.getInt() ^ v2.getDWORD());
		else if (v1.isDWORD())
			vres.setDWORD(v1.getDWORD() ^ v2.getDWORD());
		else
			bOk = false;
	}
	else
	{
		bOk = false;
	}

	return bOk;
}

bool Variant::BITOR(Variant& v1, Variant& v2, Variant& vres)
{
	bool bOk = true;

	if (v2.isInt())
	{
		if (v1.isInt())
			vres.setInt(v1.getInt() | v2.getInt());
		else if (v1.isDWORD())
			vres.setDWORD(v1.getDWORD() | (long)v2.getInt());
		else
			bOk = false;
	}
	else if (v2.isDWORD())
	{
		if (v1.isInt())
			vres.setDWORD((long)v1.getInt() | v2.getDWORD());
		else if (v1.isDWORD())
			vres.setDWORD(v1.getDWORD() | v2.getDWORD());
		else
			bOk = false;
	}
	else
	{
		bOk = false;
	}

	return bOk;
}

bool Variant::LOGAND(Variant& v1, Variant& v2, Variant& vres)
{
	bool b1, b2;
	if (v1.asBool(b1) && v2.asBool(b2))
	{
		vres.setBool(b1 && b2);
		return true;
	}

	return false;
}

bool Variant::LOGOR(Variant& v1, Variant& v2, Variant& vres)
{
	bool b1, b2;
	if (v1.asBool(b1) && v2.asBool(b2))
	{
		vres.setBool(b1 || b2);
		return true;
	}

	return false;
}


} // namespace variant

