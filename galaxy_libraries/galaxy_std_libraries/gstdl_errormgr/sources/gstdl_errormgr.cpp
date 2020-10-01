// GErrorMgr.cpp : Implementation file
// ----------------------------------------------------------------------------------------------------------
// Copyright Galaxy
// This computer program is protected by copyright law
// and international treaties. Unauthorized reproduction or 
// distribution of this program, or any portion of it,may 
// result in severe civil and criminal penalties, and will be 
// prosecuted to the maximum extent possible under the low.
// ----------------------------------------------------------------------------------------------------------
//
// Notes:
//
// ----------------------------------------------------------------------------------------------------------

#define _GALAXY_ERRORMGR_EXPORTS_

#include "gstdl_errormgrdll.h"
#include "gstdl_errormgr.h"

using namespace std;

// ----------------------------------------------------------------------------------------------------------
// Class CGErrorMgr

CGErrorMgr::CGErrorMgr() : m_nMessageLevel(3)
{
}

CGErrorMgr::CGErrorMgr(const CGErrorMgr& error)
{
	CopyContent(error);
}

CGErrorMgr::~CGErrorMgr()
{

}

string CGErrorMgr::GetErrorMessage(int nMsgLevel /* = -1 */) const
{
	int	nMsgLevelToUse = nMsgLevel > -1 ?  nMsgLevel : m_nMessageLevel;
	int	nCurrentLevel = 0;

	string		strErrorMsg;

	for(int i = m_lstErrorString.size()-1; i >= 0 ; i--) 
	{
		if(nCurrentLevel == nMsgLevelToUse)
			break;

		string	strBuffer(m_lstErrorString[i]);
		if(strBuffer.empty() == false)
		{
			if(nCurrentLevel != 0)
				strErrorMsg += "\n";
			strErrorMsg += strBuffer;
		}
		nCurrentLevel++;
	}

	return strErrorMsg;
}

void CGErrorMgr::GetErrorMessage(string & strErrorMsg, int nMsgLevel/* = -1*/)
{
	int	nMsgLevelToUse = nMsgLevel > -1 ?  nMsgLevel : m_nMessageLevel;
	int	nCurrentLevel = 0;

	strErrorMsg = "";

	for(int i = m_lstErrorString.size()-1; i >= 0 ; i--) 
	{
		if(nCurrentLevel == nMsgLevelToUse)
			break;

		string	strBuffer(m_lstErrorString[i]);
		if(strBuffer.empty() == false)
		{
			if(nCurrentLevel != 0)
				strErrorMsg += "\n";
			strErrorMsg += strBuffer;
		}
		nCurrentLevel++;
	}
}

// Assignment operator
CGErrorMgr& CGErrorMgr::operator =(const CGErrorMgr& error)
{
	CopyContent(error);
	return *this;
}

// Addition assignment operator (version 1)
CGErrorMgr& CGErrorMgr::operator+=(PCSTR lpcszErrorMessage)
{
	m_lstErrorString.push_back(string(lpcszErrorMessage));
	return *this;
}

// Addition assignment operator (version 2)
CGErrorMgr& CGErrorMgr::operator+=(const string& strErrorMessage)
{
	m_lstErrorString.push_back(strErrorMessage);
	return *this;
}

// Remove all strings from the internal list
void CGErrorMgr::Reset()
{
	m_lstErrorString.clear();
}

// Copy the content of the specified CGErrorMgr object to this one
void CGErrorMgr::CopyContent(const CGErrorMgr& error)
{
	m_nMessageLevel = error.m_nMessageLevel;

	Reset();

	for(int i = 0; i < int(error.m_lstErrorString.size()); i++) 
		m_lstErrorString.push_back(error.m_lstErrorString[i]);
}
