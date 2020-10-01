/****************************************************************************************/
/* Copyright Galaxy                                                                     */
/* This computer program is protected by copyright law                                  */
/* and international treaties. Unauthorized reproduction or                             */
/* distribution of this program, or any portion of it,may                               */
/* result in severe civil and criminal penalties, and will be                           */
/* prosecuted to the maximum extent possible under the law.                             */
/****************************************************************************************/
/*                                                                                      */
/* Notes :                                                                              */
/*                                                                                      */
/****************************************************************************************/

///////////////////////////////////////////////////////////
// CSpektraRecords_V3 class IMPLEMENTATION :
// This class contains routines to read records from a
// spektra file
///////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <QString>
#include <QStringList>
#include <QDateTime>

#include "cspektrarecords_v3.h"
#include <gqtl_log.h>

// Field manipulation macros
#define _FIELD_CHECKREAD(readfunc,pos)\
	{\
		if(readfunc != CSpektra::NoError)\
		{\
			if(pos < eposFIRST_OPTIONAL)	return false;\
			else return true;\
		}\
	}

#define _FIELD_SET(setfunc,validationfunc,pos)\
	{\
		setfunc;\
		m_pFieldFlags[pos]	|= FieldFlag_Present;\
		if(validationfunc)	m_pFieldFlags[pos]	|= FieldFlag_Valid;\
	}

#define _LIST_ADDFIELD_ASCII(list,field,val,fieldselection,pos)\
	{\
		if((m_pFieldFlags[pos] & fieldselection) == fieldselection)\
		{\
			if(m_pFieldFlags[pos] & FieldFlag_Valid)\
			{\
				m_strTmp_macro = field;\
				m_strTmp_macro += ";";\
				m_strTmp_macro += val;\
			}\
			else if(m_pFieldFlags[pos] & FieldFlag_Present)\
			{\
				m_strTmp_macro = field;\
				m_strTmp_macro += ";<not valid> (";\
				m_strTmp_macro += val;\
				m_strTmp_macro += ")";\
			}\
			else\
			{\
				m_strTmp_macro = field;\
				m_strTmp_macro += ";<not present>";\
			}\
			list.append(m_strTmp_macro);\
		}\
	}

#define _STR_ADDFIELD_ASCII(string,field,val,fieldselection,pos)\
	{\
		if((m_pFieldFlags[pos] & fieldselection) == fieldselection)\
		{\
			if(m_pFieldFlags[pos] & FieldFlag_Valid)\
			{\
				m_strTmp2_macro = field;\
				m_strTmp_macro = m_strTmp2_macro.leftJustified(30);\
				m_strTmp_macro += " = ";\
				m_strTmp_macro += val;\
			}\
			else if(m_pFieldFlags[pos] & FieldFlag_Present)\
			{\
				m_strTmp2_macro = field;\
				m_strTmp_macro = m_strTmp2_macro.leftJustified(30);\
				m_strTmp_macro += " = <not valid> (";\
				m_strTmp_macro += val;\
				m_strTmp_macro += ")";\
			}\
			else\
			{\
				m_strTmp2_macro = field;\
				m_strTmp_macro = m_strTmp2_macro.leftJustified(30);\
				m_strTmp_macro += " = <not present>";\
			}\
			string += m_strTmp_macro;\
			string += "\n";\
		}\
	}

#define _STR_ADDFIELD_XML(string,tabs,field,val,fieldselection,pos)\
	{\
		if(((m_pFieldFlags[pos] & fieldselection) == fieldselection) && (m_pFieldFlags[pos] & FieldFlag_Present))\
		{\
			m_nTabs_macro = tabs;\
			m_strTmp_macro = "";\
			while(m_nTabs_macro-- > 0)\
				m_strTmp_macro += "\t";\
			m_strTmp_macro += "<";\
			m_strTmp_macro += field;\
			m_strTmp_macro += ">";\
			m_strTmp_macro += val;\
			m_strTmp_macro += "</";\
			m_strTmp_macro += field;\
			m_strTmp_macro += ">";\
			m_strTmp_macro += "\n";\
			string += m_strTmp_macro;\
		}\
	}

#define _STR_ADDLIST_XML(string,count,tabs,field,val,fieldselection,pos)\
	{\
		if(((m_pFieldFlags[pos] & fieldselection) == fieldselection) && (m_pFieldFlags[pos] & FieldFlag_Present))\
		{\
			m_nTabs_macro = tabs;\
			m_strTmp_macro = "";\
			while(m_nTabs_macro-- > 0)\
				m_strTmp_macro += "\t";\
			m_strTmp_macro += "<";\
			m_strTmp_macro += field;\
			m_strTmp_macro += " count=\"";\
			m_strTmp_macro += QString::number(count);\
			m_strTmp_macro += "\"";\
			m_strTmp_macro += ">";\
			m_strTmp_macro += val;\
			m_strTmp_macro += "</";\
			m_strTmp_macro += field;\
			m_strTmp_macro += ">";\
			m_strTmp_macro += "\n";\
			string += m_strTmp_macro;\
		}\
	}


#define _CREATEHEXFIELD_FROM_U1_ASCII(data)\
	{\
		sprintf(m_szTmp_macro, "0x%02X", (int)data & 0x0ff);\
		m_strFieldValue_macro = m_szTmp_macro;\
	}

#define _CREATEHEXFIELD_FROM_U2_ASCII(data)\
	{\
		sprintf(m_szTmp_macro, "0x%04X", (int)data & 0x0ffff);\
		m_strFieldValue_macro = m_szTmp_macro;\
	}

#define _CREATEFIELD_FROM_KUi_ASCII(count,data,pos)\
	{\
		m_strFieldValue_macro = "";\
		if(m_pFieldFlags[pos] & FieldFlag_Present)\
		{\
			if(count > 0)\
			{\
				sprintf(m_szTmp_macro, "\n        [000] = %u", data[0]);\
				m_strFieldValue_macro = m_szTmp_macro;\
			}\
			for(m_uiIndex_macro=1; m_uiIndex_macro<(unsigned int)count; m_uiIndex_macro++)\
			{\
				sprintf(m_szTmp_macro, "\n        [%03u] = %u", m_uiIndex_macro, data[m_uiIndex_macro]);\
				m_strFieldValue_macro += m_szTmp_macro;\
			}\
		}\
	}

#define _CREATEFIELD_FROM_KUi_XML(count,data,pos)\
	{\
		m_strFieldValue_macro = "";\
		if(m_pFieldFlags[pos] & FieldFlag_Present)\
		{\
			if(count > 0)\
			{\
				sprintf(m_szTmp_macro, "%u", data[0]);\
				m_strFieldValue_macro = m_szTmp_macro;\
			}\
			for(m_uiIndex_macro=1; m_uiIndex_macro<(unsigned int)count; m_uiIndex_macro++)\
			{\
				sprintf(m_szTmp_macro, " %u", data[m_uiIndex_macro]);\
				m_strFieldValue_macro += m_szTmp_macro;\
			}\
		}\
	}

#define _CREATEFIELD_FROM_KIi_ASCII(count,data,pos)\
	{\
		m_strFieldValue_macro = "";\
		if(m_pFieldFlags[pos] & FieldFlag_Present)\
		{\
			if(count > 0)\
			{\
				sprintf(m_szTmp_macro, "\n        [000] = %d", data[0]);\
				m_strFieldValue_macro = m_szTmp_macro;\
			}\
			for(m_uiIndex_macro=1; m_uiIndex_macro<(unsigned int)count; m_uiIndex_macro++)\
			{\
				sprintf(m_szTmp_macro, "\n        [%03u] = %d", m_uiIndex_macro, data[m_uiIndex_macro]);\
				m_strFieldValue_macro += m_szTmp_macro;\
			}\
		}\
	}

#define _CREATEFIELD_FROM_KIi_XML(count,data,pos)\
	{\
		m_strFieldValue_macro = "";\
		if(m_pFieldFlags[pos] & FieldFlag_Present)\
		{\
			if(count > 0)\
			{\
				sprintf(m_szTmp_macro, "%d", data[0]);\
				m_strFieldValue_macro = m_szTmp_macro;\
			}\
			for(m_uiIndex_macro=1; m_uiIndex_macro<(unsigned int)count; m_uiIndex_macro++)\
			{\
				sprintf(m_szTmp_macro, " %d", data[m_uiIndex_macro]);\
				m_strFieldValue_macro += m_szTmp_macro;\
			}\
		}\
	}

#define _CREATEFIELD_FROM_KRi_ASCII(count,data,pos)\
	{\
		m_strFieldValue_macro = "";\
		if(m_pFieldFlags[pos] & FieldFlag_Present)\
		{\
			if(count > 0)\
			{\
				sprintf(m_szTmp_macro, "\n        [000] = %g", data[0]);\
				m_strFieldValue_macro = m_szTmp_macro;\
			}\
			for(m_uiIndex_macro=1; m_uiIndex_macro<(unsigned int)count; m_uiIndex_macro++)\
			{\
				sprintf(m_szTmp_macro, "\n        [%03u] = %g", m_uiIndex_macro, data[m_uiIndex_macro]);\
				m_strFieldValue_macro += m_szTmp_macro;\
			}\
		}\
	}

#define _CREATEFIELD_FROM_KRi_XML(count,data,pos)\
	{\
		m_strFieldValue_macro = "";\
		if(m_pFieldFlags[pos] & FieldFlag_Present)\
		{\
			if(count > 0)\
			{\
				sprintf(m_szTmp_macro, "%g", data[0]);\
				m_strFieldValue_macro = m_szTmp_macro;\
			}\
			for(m_uiIndex_macro=1; m_uiIndex_macro<(unsigned int)count; m_uiIndex_macro++)\
			{\
				sprintf(m_szTmp_macro, " %g", data[m_uiIndex_macro]);\
				m_strFieldValue_macro += m_szTmp_macro;\
			}\
		}\
	}

#define _CREATEFIELD_FROM_CN_XML(data)\
	{\
		m_strFieldValue_macro = data;\
		m_strFieldValue_macro.replace("<", "&lt;");\
		m_strFieldValue_macro.replace(">", "&gt;");\
	}

#define _CREATEFIELD_FROM_KCN_ASCII(count,data,pos)\
	{\
		m_strFieldValue_macro = "";\
		if(m_pFieldFlags[pos] & FieldFlag_Present)\
		{\
			if(count > 0)\
			{\
				sprintf(m_szTmp_macro, "\n        [000] = %s", data[0].toLatin1().constData());\
				m_strFieldValue_macro = m_szTmp_macro;\
			}\
			for(m_uiIndex_macro=1; m_uiIndex_macro<(unsigned int)count; m_uiIndex_macro++)\
			{\
				sprintf(m_szTmp_macro, "\n        [%03u] = %s", m_uiIndex_macro, data[m_uiIndex_macro].toLatin1().constData());\
				m_strFieldValue_macro += m_szTmp_macro;\
			}\
		}\
	}

#define _CREATEFIELD_FROM_KCN_XML(count,data,pos)\
	{\
		m_strFieldValue_macro = "";\
		if(m_pFieldFlags[pos] & FieldFlag_Present)\
		{\
			if(count > 0)\
			{\
				m_strFieldValue_macro = data[0];\
				m_strFieldValue_macro.replace("<", "&lt;");\
				m_strFieldValue_macro.replace(">", "&gt;");\
			}\
			for(m_uiIndex_macro=1; m_uiIndex_macro<(unsigned int)count; m_uiIndex_macro++)\
			{\
				m_strTmp_macro = data[m_uiIndex_macro];\
				m_strTmp_macro.replace("<", "&lt;");\
				m_strTmp_macro.replace(">", "&gt;");\
				m_strFieldValue_macro += " ";\
				m_strFieldValue_macro += m_strTmp_macro;\
			}\
		}\
	}

#define _CREATEFIELD_FROM_N1_ASCII(data)\
	{\
		sprintf(m_szTmp_macro, "0x%02x", (int)data & 0x0ff);\
		m_strFieldValue_macro = m_szTmp_macro;\
	}

#define _CREATEFIELD_FROM_N1_XML(data)\
	{\
		sprintf(m_szTmp_macro, "%02x", (int)data & 0x0ff);\
		m_strFieldValue_macro = m_szTmp_macro;\
	}

#define _CREATEFIELD_FROM_B1_ASCII(data)\
	{\
		sprintf(m_szTmp_macro, "0x%02x", (int)data & 0x0ff);\
		m_strFieldValue_macro = m_szTmp_macro;\
	}

#define _CREATEFIELD_FROM_B1_XML(data)\
	{\
		sprintf(m_szTmp_macro, "%02x", (int)data & 0x0ff);\
		m_strFieldValue_macro = m_szTmp_macro;\
	}

#define _CREATEFIELD_FROM_BN_ASCII(data)\
	{\
		m_strFieldValue_macro = "0x";\
		if(data.m_bLength > 0)\
		{\
			m_ptChar_macro = data.m_pBitField;\
			for(m_uiIndex_macro=0; m_uiIndex_macro<data.m_bLength; m_uiIndex_macro++)\
			{\
				sprintf(m_szTmp_macro, "%02x", (int)m_ptChar_macro[m_uiIndex_macro] & 0x0ff);\
				m_strFieldValue_macro += m_szTmp_macro;\
			}\
		}\
	}

#define _CREATEFIELD_FROM_BN_XML(data)\
	{\
		m_strFieldValue_macro = "";\
		if(data.m_bLength > 0)\
		{\
			m_ptChar_macro = data.m_pBitField;\
			for(m_uiIndex_macro=0; m_uiIndex_macro<data.m_bLength; m_uiIndex_macro++)\
			{\
				sprintf(m_szTmp_macro, "%02x", (int)m_ptChar_macro[m_uiIndex_macro] & 0x0ff);\
				m_strFieldValue_macro += m_szTmp_macro;\
			}\
		}\
	}

#define _CREATEFIELD_FROM_DN_ASCII(data)\
	{\
		m_strFieldValue_macro = "0x";\
		if(data.m_uiLength > 0)\
		{\
			m_ptChar_macro = data.m_pBitField;\
			for(m_uiIndex_macro=0; m_uiIndex_macro<data.m_uiLength; m_uiIndex_macro++)\
			{\
				sprintf(m_szTmp_macro, "%02x", (int)m_ptChar_macro[m_uiIndex_macro] & 0x0ff);\
				m_strFieldValue_macro += m_szTmp_macro;\
			}\
		}\
	}

#define _CREATEFIELD_FROM_DN_XML(data)\
	{\
		m_strFieldValue_macro = "";\
		if(data.m_uiLength > 0)\
		{\
			m_ptChar_macro = data.m_pBitField;\
			for(m_uiIndex_macro=0; m_uiIndex_macro<data.m_uiLength; m_uiIndex_macro++)\
			{\
				sprintf(m_szTmp_macro, "%02x", (int)m_ptChar_macro[m_uiIndex_macro] & 0x0ff);\
				m_strFieldValue_macro += m_szTmp_macro;\
			}\
		}\
	}

#define _CREATEFIELD_FROM_KN1_ASCII(count,data,pos)\
	{\
		m_strFieldValue_macro = "";\
		if(m_pFieldFlags[pos] & FieldFlag_Present)\
		{\
			if(count > 0)\
			{\
				sprintf(m_szTmp_macro, "\n        [000] = 0x%01x", (int)data[0] & 0x0f);\
				m_strFieldValue_macro = m_szTmp_macro;\
			}\
			for(m_uiIndex_macro=1; m_uiIndex_macro<(unsigned int)count; m_uiIndex_macro++)\
			{\
				sprintf(m_szTmp_macro, "\n        [%03u] = 0x%01x", m_uiIndex_macro, ((int)data[m_uiIndex_macro/2] >> (4 * (m_uiIndex_macro%2))) & 0x0f);\
				m_strFieldValue_macro += m_szTmp_macro;\
			}\
		}\
	}

#define _CREATEFIELD_FROM_KN1_XML(count,data,pos)\
	{\
		m_strFieldValue_macro = "";\
		if(m_pFieldFlags[pos] & FieldFlag_Present)\
		{\
			if(count > 0)\
			{\
				sprintf(m_szTmp_macro, "%01x", (int)data[0] & 0x0f);\
				m_strFieldValue_macro = m_szTmp_macro;\
			}\
			for(m_uiIndex_macro=1; m_uiIndex_macro<(unsigned int)count; m_uiIndex_macro++)\
			{\
				sprintf(m_szTmp_macro, " %01x", m_uiIndex_macro, ((int)data[m_uiIndex_macro/2] >> (4 * (m_uiIndex_macro%2))) & 0x0f);\
				m_strFieldValue_macro += m_szTmp_macro;\
			}\
		}\
	}

///////////////////////////////////////////////////////////
// GENERIC RECORD
///////////////////////////////////////////////////////////
CSpektra_Record_V3::CSpektra_Record_V3()
{
}

CSpektra_Record_V3::~CSpektra_Record_V3()
{
	// This destructor is virtual and must be implemented in derived classes
}

void CSpektra_Record_V3::Reset(void)
{
	// This function is virtual and must be implemented in derived classes
	GEX_ASSERT(false);
	return;
}

QString CSpektra_Record_V3::GetRecordShortName(void)
{
	// This function is virtual and must be implemented in derived classes
	GEX_ASSERT(false);
	return "";
}

QString CSpektra_Record_V3::GetRecordLongName(void)
{
	// This function is virtual and must be implemented in derived classes
	GEX_ASSERT(false);
	return "";
}

int CSpektra_Record_V3::GetRecordType(void)
{
	// This function is virtual and must be implemented in derived classes
	GEX_ASSERT(false);
	return Rec_UNKNOWN;
}

bool CSpektra_Record_V3::Read(CSpektra& /*clSpektra*/)
{
	// This function is virtual and must be implemented in derived classes
	GEX_ASSERT(false);
	return true;
}

void CSpektra_Record_V3::GetAsciiFieldList(QStringList& /*listFields*/,
										   int /*nFieldSelection = 0*/)
{
	// This function is virtual and must be implemented in derived classes
	GEX_ASSERT(false);
}

void CSpektra_Record_V3::GetAsciiString(QString& /*strAsciiString*/,
										int /*nFieldSelection = 0*/)
{
	// This function is virtual and must be implemented in derived classes
	GEX_ASSERT(false);
}

void CSpektra_Record_V3::GetXMLString(QString& /*strXmlString*/,
									  const int /*nIndentationLevel*/,
									  int /*nFieldSelection = 0*/)
{
	// This function is virtual and must be implemented in derived classes
	GEX_ASSERT(false);
}

void CSpektra_Record_V3::GetAsciiRecord(QString & strRecord, int nFieldSelection /*= 0*/)
{
	QString		strAsciiString, strField, strTmp;
	
	// Get fields
	GetAsciiString(strAsciiString, nFieldSelection);

	// Write record name
	strRecord.sprintf("** %s (%s) **\n", GetRecordLongName().toLatin1().constData(), GetRecordShortName().toLatin1().constData());

	if((nFieldSelection & FieldFlag_None) == 0)
	{
		// Write all fields
		strRecord += strAsciiString;
	}
}

///////////////////////////////////////////////////////////
// Label RECORD
///////////////////////////////////////////////////////////
CSpektra_Label_V3::CSpektra_Label_V3() : CSpektra_Record_V3()
{
	Reset();
}

CSpektra_Label_V3::~CSpektra_Label_V3()
{
	Reset();
}

void CSpektra_Label_V3::Reset(void)
{
	// Reset field flags
	for(int i=0; i<eposEND; i++)
		m_pFieldFlags[i] = FieldFlag_Empty;

	// Select fields for reduced list

	// Reset Data
	QDate	clDate(1900, 1, 1);
	QTime	clTime(0, 0);
	m_dtDATETIME.setDate(clDate);							// Label.DATETIME
	m_dtDATETIME.setTime(clTime);
	m_cnFILENAME		= "";								// Label.FILENAME
	m_cnDEVICENAME		= "";								// Label.DEVICENAME
	m_cnOPERATOR		= "";								// Label.OPERATOR
	m_b1TESTINGMODE		= 0;								// Label.TESTINGMODE
	m_c1STATIONNAME		= 0;								// Label.STATIONNAME
	m_cnLOTNAME			= "";								// Label.LOTNAME
	m_cnCOMMENT			= "";								// Label.COMMENT
	m_u2TIMEPOINT		= 0;								// Label.TIMEPOINT
	m_u2SETQUANTITY		= 0;								// Label.SETQUANTITY
	m_u2LOGGINGRATE		= 0;								// Label.LOGGINGRATE
	m_u2TESTMAX			= 0;								// Label.TESTMAX
	m_u2DATABLOCKNUMBER	= 0;								// Label.DATABLOCKNUMBER
	m_u2LOGGEDQUANTITY	= 0;								// Label.LOGGEDQUANTITY
	m_u2INDEXMAX		= 0;								// Label.INDEXMAX
	m_cnRESERVED		= "";								// Label.RESERVED
}

QString CSpektra_Label_V3::GetRecordShortName(void)
{
	return "Label";
}

QString CSpektra_Label_V3::GetRecordLongName(void)
{
	return "Label";
}

int CSpektra_Label_V3::GetRecordType(void)
{
	return Rec_Label;
}

bool CSpektra_Label_V3::Read(CSpektra & clSpektra)
{
	BYTE	bData1, bData2, bData3;
	WORD	wData;
	char	szString[SPEKTRA_MAX_U1+1];

	// First reset data
	Reset();

	// Label.DATETIME
	_FIELD_CHECKREAD(clSpektra.ReadByte(&bData1), eposDATETIME);
	_FIELD_CHECKREAD(clSpektra.ReadByte(&bData2), eposDATETIME);
	_FIELD_CHECKREAD(clSpektra.ReadByte(&bData3), eposDATETIME);
	_FIELD_SET(m_u1DATETIME_Year = spektra_type_u1(bData1) , true, eposDATETIME);
	_FIELD_SET(m_u1DATETIME_Month= spektra_type_u1(bData2) , true, eposDATETIME);
	_FIELD_SET(m_u1DATETIME_Day = spektra_type_u1(bData3) , true, eposDATETIME);
	QDate clDate(1900+bData1, 1+bData2, 1+bData3);
	_FIELD_SET(m_dtDATETIME.setDate(clDate) , true, eposDATETIME);
	_FIELD_CHECKREAD(clSpektra.ReadByte(&bData1), eposDATETIME);
	_FIELD_CHECKREAD(clSpektra.ReadByte(&bData2), eposDATETIME);
	_FIELD_CHECKREAD(clSpektra.ReadByte(&bData3), eposDATETIME);
	_FIELD_SET(m_u1DATETIME_Hour = spektra_type_u1(bData1) , true, eposDATETIME);
	_FIELD_SET(m_u1DATETIME_Minute= spektra_type_u1(bData2) , true, eposDATETIME);
	_FIELD_SET(m_u1DATETIME_Second= spektra_type_u1(bData3) , true, eposDATETIME);
	QTime clTime(bData1, bData2, bData3);
	_FIELD_SET(m_dtDATETIME.setTime(clTime) , true, eposDATETIME);

	// Label.FILENAME
	_FIELD_CHECKREAD(clSpektra.ReadString(szString, 8), eposFILENAME);
	_FIELD_SET(m_cnFILENAME = szString, true, eposFILENAME);

	// Label.DEVICENAME
	_FIELD_CHECKREAD(clSpektra.ReadString(szString, 10), eposDEVICENAME);
	_FIELD_SET(m_cnDEVICENAME = szString, true, eposDEVICENAME);

	// Label.OPERATOR
	_FIELD_CHECKREAD(clSpektra.ReadString(szString, 10), eposOPERATOR);
	_FIELD_SET(m_cnOPERATOR = szString, true, eposOPERATOR);

	// Label.TESTINGMODE
	_FIELD_CHECKREAD(clSpektra.ReadByte(&bData1), eposTESTINGMODE);
	_FIELD_SET(m_b1TESTINGMODE = spektra_type_b1(bData1), true, eposTESTINGMODE);
	
	// Label.STATIONNAME
	_FIELD_CHECKREAD(clSpektra.ReadByte(&bData1), eposSTATIONNAME);
	_FIELD_SET(m_c1STATIONNAME = spektra_type_c1(bData1), true, eposSTATIONNAME);
	
	// Label.LOTNAME
	_FIELD_CHECKREAD(clSpektra.ReadString(szString, 10), eposLOTNAME);
	_FIELD_SET(m_cnLOTNAME = szString, true, eposLOTNAME);

	// Label.COMMENT
	_FIELD_CHECKREAD(clSpektra.ReadString(szString, 50), eposCOMMENT);
	_FIELD_SET(m_cnCOMMENT = szString, true, eposCOMMENT);

	// Label.TIMEPOINT
	_FIELD_CHECKREAD(clSpektra.ReadWord(&wData), eposTIMEPOINT);
	_FIELD_SET(m_u2TIMEPOINT = spektra_type_u2(wData), true, eposTIMEPOINT);
	
	// Label.SETQUANTITY
	_FIELD_CHECKREAD(clSpektra.ReadWord(&wData), eposSETQUANTITY);
	_FIELD_SET(m_u2SETQUANTITY = spektra_type_u2(wData), true, eposSETQUANTITY);
	
	// Label.LOGGINGRATE
	_FIELD_CHECKREAD(clSpektra.ReadWord(&wData), eposLOGGINGRATE);
	_FIELD_SET(m_u2LOGGINGRATE = spektra_type_u2(wData), true, eposLOGGINGRATE);
	
	// Label.TESTMAX
	_FIELD_CHECKREAD(clSpektra.ReadWord(&wData), eposTESTMAX);
	_FIELD_SET(m_u2TESTMAX = spektra_type_u2(wData), true, eposTESTMAX);
	
	// Label.DATABLOCKNUMBER
	_FIELD_CHECKREAD(clSpektra.ReadWord(&wData), eposDATABLOCKNUMBER);
	_FIELD_SET(m_u2DATABLOCKNUMBER = spektra_type_u2(wData), true, eposDATABLOCKNUMBER);
	
	// Label.LOGGEDQUANTITY
	_FIELD_CHECKREAD(clSpektra.ReadWord(&wData), eposLOGGEDQUANTITY);
	_FIELD_SET(m_u2LOGGEDQUANTITY = spektra_type_u2(wData), true, eposLOGGEDQUANTITY);
	
	// Label.INDEXMAX
	_FIELD_CHECKREAD(clSpektra.ReadWord(&wData), eposINDEXMAX);
	_FIELD_SET(m_u2INDEXMAX = spektra_type_u2(wData), true, eposINDEXMAX);
	
	// Label.RESERVED
	_FIELD_CHECKREAD(clSpektra.ReadString(szString, 18), eposRESERVED);
	_FIELD_SET(m_cnRESERVED = szString, true, eposRESERVED);

	return true;
}

void CSpektra_Label_V3::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
	// Empty string first
	strAsciiString = "";

	// Label.DATETIME
    m_strFieldValue_macro.sprintf(	"%s (Y=%d, M=%d, D=%d, h=%d, m=%d, s=%d)",
                                    m_dtDATETIME.toString("ddd dd MMM yyyy h:mm:ss").toLatin1().constData(),
									m_u1DATETIME_Year, m_u1DATETIME_Month, m_u1DATETIME_Day, m_u1DATETIME_Hour, m_u1DATETIME_Minute, m_u1DATETIME_Second);
	_STR_ADDFIELD_ASCII(strAsciiString, "Label.DATETIME", m_strFieldValue_macro, nFieldSelection, eposDATETIME);

	// Label.FILENAME
	_STR_ADDFIELD_ASCII(strAsciiString, "Label.FILENAME", m_cnFILENAME, nFieldSelection, eposFILENAME);

	// Label.DEVICENAME
	_STR_ADDFIELD_ASCII(strAsciiString, "Label.DEVICENAME", m_cnDEVICENAME, nFieldSelection, eposDEVICENAME);

	// Label.OPERATOR
	_STR_ADDFIELD_ASCII(strAsciiString, "Label.OPERATOR", m_cnOPERATOR, nFieldSelection, eposOPERATOR);

	// Label.TESTINGMODE
	_CREATEFIELD_FROM_B1_ASCII(m_b1TESTINGMODE)
	_STR_ADDFIELD_ASCII(strAsciiString, "Label.TESTINGMODE", m_strFieldValue_macro, nFieldSelection, eposTESTINGMODE);

	// Label.STATIONNAME
	_STR_ADDFIELD_ASCII(strAsciiString, "Label.STATIONNAME", m_c1STATIONNAME, nFieldSelection, eposSTATIONNAME);

	// Label.LOTNAME
	_STR_ADDFIELD_ASCII(strAsciiString, "Label.LOTNAME", m_cnLOTNAME, nFieldSelection, eposLOTNAME);

	// Label.COMMENT
	_STR_ADDFIELD_ASCII(strAsciiString, "Label.COMMENT", m_cnCOMMENT, nFieldSelection, eposCOMMENT);

	// Label.TIMEPOINT
	_STR_ADDFIELD_ASCII(strAsciiString, "Label.TIMEPOINT", QString::number(m_u2TIMEPOINT), nFieldSelection, eposTIMEPOINT);

	// Label.SETQUANTITY
	_STR_ADDFIELD_ASCII(strAsciiString, "Label.SETQUANTITY", QString::number(m_u2SETQUANTITY), nFieldSelection, eposSETQUANTITY);

	// Label.LOGGINGRATE
	_STR_ADDFIELD_ASCII(strAsciiString, "Label.LOGGINGRATE", QString::number(m_u2LOGGINGRATE), nFieldSelection, eposLOGGINGRATE);

	// Label.TESTMAX
	_STR_ADDFIELD_ASCII(strAsciiString, "Label.TESTMAX", QString::number(m_u2TESTMAX), nFieldSelection, eposTESTMAX);

	// Label.DATABLOCKNUMBER
	_STR_ADDFIELD_ASCII(strAsciiString, "Label.DATABLOCKNUMBER", QString::number(m_u2DATABLOCKNUMBER), nFieldSelection, eposDATABLOCKNUMBER);

	// Label.LOGGEDQUANTITY
	_STR_ADDFIELD_ASCII(strAsciiString, "Label.LOGGEDQUANTITY", QString::number(m_u2LOGGEDQUANTITY), nFieldSelection, eposLOGGEDQUANTITY);

	// Label.INDEXMAX
	_STR_ADDFIELD_ASCII(strAsciiString, "Label.INDEXMAX", QString::number(m_u2INDEXMAX), nFieldSelection, eposINDEXMAX);

	// Label.RESERVED
	_STR_ADDFIELD_ASCII(strAsciiString, "Label.RESERVED", m_cnRESERVED, nFieldSelection, eposRESERVED);
}

void CSpektra_Label_V3::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
	// Empty string list first
	listFields.empty();

	// Label.DATETIME
    m_strFieldValue_macro.sprintf(	"%s (Y=%d, M=%d, D=%d, h=%d, m=%d, s=%d)",
                                    m_dtDATETIME.toString("ddd dd MMM yyyy h:mm:ss").toLatin1().constData(),
									m_u1DATETIME_Year, m_u1DATETIME_Month, m_u1DATETIME_Day, m_u1DATETIME_Hour, m_u1DATETIME_Minute, m_u1DATETIME_Second);
	_LIST_ADDFIELD_ASCII(listFields, "Label.DATETIME", m_strFieldValue_macro, nFieldSelection, eposDATETIME);

	// Label.FILENAME
	_LIST_ADDFIELD_ASCII(listFields, "Label.FILENAME", m_cnFILENAME, nFieldSelection, eposFILENAME);

	// Label.DEVICENAME
	_LIST_ADDFIELD_ASCII(listFields, "Label.DEVICENAME", m_cnDEVICENAME, nFieldSelection, eposDEVICENAME);

	// Label.OPERATOR
	_LIST_ADDFIELD_ASCII(listFields, "Label.OPERATOR", m_cnOPERATOR, nFieldSelection, eposOPERATOR);

	// Label.TESTINGMODE
	_CREATEFIELD_FROM_B1_ASCII(m_b1TESTINGMODE)
	_LIST_ADDFIELD_ASCII(listFields, "Label.TESTINGMODE", m_strFieldValue_macro, nFieldSelection, eposTESTINGMODE);

	// Label.STATIONNAME
	_LIST_ADDFIELD_ASCII(listFields, "Label.STATIONNAME", m_c1STATIONNAME, nFieldSelection, eposSTATIONNAME);

	// Label.LOTNAME
	_LIST_ADDFIELD_ASCII(listFields, "Label.LOTNAME", m_cnLOTNAME, nFieldSelection, eposLOTNAME);

	// Label.COMMENT
	_LIST_ADDFIELD_ASCII(listFields, "Label.COMMENT", m_cnCOMMENT, nFieldSelection, eposCOMMENT);

	// Label.TIMEPOINT
	_LIST_ADDFIELD_ASCII(listFields, "Label.TIMEPOINT", QString::number(m_u2TIMEPOINT), nFieldSelection, eposTIMEPOINT);

	// Label.SETQUANTITY
	_LIST_ADDFIELD_ASCII(listFields, "Label.SETQUANTITY", QString::number(m_u2SETQUANTITY), nFieldSelection, eposSETQUANTITY);

	// Label.LOGGINGRATE
	_LIST_ADDFIELD_ASCII(listFields, "Label.LOGGINGRATE", QString::number(m_u2LOGGINGRATE), nFieldSelection, eposLOGGINGRATE);

	// Label.TESTMAX
	_LIST_ADDFIELD_ASCII(listFields, "Label.TESTMAX", QString::number(m_u2TESTMAX), nFieldSelection, eposTESTMAX);

	// Label.DATABLOCKNUMBER
	_LIST_ADDFIELD_ASCII(listFields, "Label.DATABLOCKNUMBER", QString::number(m_u2DATABLOCKNUMBER), nFieldSelection, eposDATABLOCKNUMBER);

	// Label.LOGGEDQUANTITY
	_LIST_ADDFIELD_ASCII(listFields, "Label.LOGGEDQUANTITY", QString::number(m_u2LOGGEDQUANTITY), nFieldSelection, eposLOGGEDQUANTITY);

	// Label.INDEXMAX
	_LIST_ADDFIELD_ASCII(listFields, "Label.INDEXMAX", QString::number(m_u2INDEXMAX), nFieldSelection, eposINDEXMAX);

	// Label.RESERVED
	_LIST_ADDFIELD_ASCII(listFields, "Label.RESERVED", m_cnRESERVED, nFieldSelection, eposRESERVED);
}

void CSpektra_Label_V3::GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection /*= 0*/)
{
	QString strTabs="";

	// Init XML string
	for(int nIndex=nIndentationLevel; nIndex>0; nIndex--)
		strTabs += "\t";
	strXmlString = strTabs;
	strXmlString += "<label>\n";
	
	// Label.DATETIME
	m_strFieldValue_macro.sprintf("%s", m_dtDATETIME.toString("ddd dd MMM yyyy h:mm:ss").toLatin1().constData());
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "datetime", m_strFieldValue_macro, nFieldSelection, eposDATETIME);

	// Label.FILENAME
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "filename", m_cnFILENAME, nFieldSelection, eposFILENAME);

	// Label.DEVICENAME
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "devicename", m_cnDEVICENAME, nFieldSelection, eposDEVICENAME);

	// Label.OPERATOR
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "operator", m_cnOPERATOR, nFieldSelection, eposOPERATOR);

	// Label.TESTINGMODE
	_CREATEFIELD_FROM_B1_XML(m_b1TESTINGMODE)
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "testingmode", m_strFieldValue_macro, nFieldSelection, eposTESTINGMODE);

	// Label.STATIONNAME
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "stationname", m_c1STATIONNAME, nFieldSelection, eposSTATIONNAME);

	// Label.LOTNAME
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "lotname", m_cnLOTNAME, nFieldSelection, eposLOTNAME);

	// Label.COMMENT
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "comment", m_cnCOMMENT, nFieldSelection, eposCOMMENT);

	// Label.TIMEPOINT
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "timepoint", QString::number(m_u2TIMEPOINT), nFieldSelection, eposTIMEPOINT);

	// Label.SETQUANTITY
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "setquantity", QString::number(m_u2SETQUANTITY), nFieldSelection, eposSETQUANTITY);

	// Label.LOGGINGRATE
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "loggingrate", QString::number(m_u2LOGGINGRATE), nFieldSelection, eposLOGGINGRATE);

	// Label.TESTMAX
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "testmax", QString::number(m_u2TESTMAX), nFieldSelection, eposTESTMAX);

	// Label.DATABLOCKNUMBER
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "datablocknumber", QString::number(m_u2DATABLOCKNUMBER), nFieldSelection, eposDATABLOCKNUMBER);

	// Label.LOGGEDQUANTITY
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "loggedquantity", QString::number(m_u2LOGGEDQUANTITY), nFieldSelection, eposLOGGEDQUANTITY);

	// Label.INDEXMAX
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "indexmax", QString::number(m_u2INDEXMAX), nFieldSelection, eposINDEXMAX);

	// Label.RESERVED
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "reserved", m_cnRESERVED, nFieldSelection, eposRESERVED);

	strXmlString += strTabs;
	strXmlString += "</label>\n";
}

///////////////////////////////////////////////////////////
// TestItem RECORD
///////////////////////////////////////////////////////////
CSpektra_TestItem_V3::CSpektra_TestItem_V3() : CSpektra_Record_V3()
{
	Reset();
}

CSpektra_TestItem_V3::~CSpektra_TestItem_V3()
{
	Reset();
}

void CSpektra_TestItem_V3::Reset(void)
{
	// Reset field flags
	for(int i=0; i<eposEND; i++)
		m_pFieldFlags[i] = FieldFlag_Empty;

	// Select fields for reduced list

	// Reset Data
	m_bIsHighLimit			= true;		// true if limit is a HL, false if LL
	m_bIsStrictLimit		= true;		// true if limit is strict, false else
	m_bHaslimit				= true;		// true if test has a valid limit, false else

	m_u1TESTNUMBER			= 0;		// TestItem.TESTNUMBER
	m_u1ITEMGROUPCODE		= 0;		// TestItem.ITEMGROUPCODE
	m_u2ITEMCODE			= 0;		// TestItem.ITEMCODE
	m_b1BECONDITIONANDFLAGS	= 0;		// TestItem.BECONDITIONANDFLAGS
	m_b1RESULTASBIASFLAGS	= 0;		// TestItem.RESULTASBIASFLAGS
	m_cnLIMITITEMNAME		= "";		// TestItem.LIMITITEMNAME
	m_cnLIMITUNIT			= "";		// TestItem.LIMITUNIT
	m_r4LIMITVALUE			= 0.0;		// TestItem.LIMITVALUE
	m_r4LIMITMIN			= 0.0;		// TestItem.LIMITMIN
	m_r4LIMITMAX			= 0.0;		// TestItem.LIMITMAX
	m_cnBIAS1NAME			= "";		// TestItem.BIAS1NAME
	m_cnBIAS1UNIT			= "";		// TestItem.BIAS1UNIT
	m_r4BIAS1VALUE			= 0.0;		// TestItem.BIAS1VALUE
	m_cnBIAS2NAME			= "";		// TestItem.BIAS2NAME
	m_cnBIAS2UNIT			= "";		// TestItem.BIAS2UNIT
	m_r4BIAS2VALUE			= 0.0;		// TestItem.BIAS2VALUE
	m_cnTIMECONDITIONNAME	= "";		// TestItem.TIMECONDITIONNAME
	m_cnTIMEUNIT			= "";		// TestItem.TIMEUNIT
	m_r4TIMEVALUE			= 0.0;		// TestItem.TIMEVALUE
	m_b1DLINHIBITFLAGS		= 0;		// TestItem.DLINHIBITFLAGS
	m_cnRESERVED			= "";		// TestItem.RESERVED
}

QString CSpektra_TestItem_V3::GetRecordShortName(void)
{
	return "TestItem";
}

QString CSpektra_TestItem_V3::GetRecordLongName(void)
{
	return "TestItem";
}

int CSpektra_TestItem_V3::GetRecordType(void)
{
	return Rec_TestItem;
}

bool CSpektra_TestItem_V3::Read(CSpektra & clSpektra)
{
	BYTE	bData;
	WORD	wData;
	FLOAT	fData;
	char	szString[SPEKTRA_MAX_U1+1];

	// First reset data
	Reset();

	// TestItem.TESTNUMBER
	_FIELD_CHECKREAD(clSpektra.ReadByte(&bData), eposTESTNUMBER);
	_FIELD_SET(m_u1TESTNUMBER = spektra_type_u1(bData), true, eposTESTNUMBER);

	// TestItem.ITEMGROUPCODE
	_FIELD_CHECKREAD(clSpektra.ReadByte(&bData), eposITEMGROUPCODE);
	_FIELD_SET(m_u1ITEMGROUPCODE = spektra_type_u1(bData), true, eposITEMGROUPCODE);

	// TestItem.ITEMCODE
	_FIELD_CHECKREAD(clSpektra.ReadWord(&wData), eposITEMCODE);
	_FIELD_SET(m_u2ITEMCODE = spektra_type_u2(wData), true, eposITEMCODE);

	// TestItem.BECONDITIONANDFLAGS
	_FIELD_CHECKREAD(clSpektra.ReadByte(&bData), eposBECONDITIONANDFLAGS);
	_FIELD_SET(m_b1BECONDITIONANDFLAGS = spektra_type_b1(bData), true, eposBECONDITIONANDFLAGS);
	
	// TestItem.RESULTASBIASFLAGS
	_FIELD_CHECKREAD(clSpektra.ReadByte(&bData), eposRESULTASBIASFLAGS);
	_FIELD_SET(m_b1RESULTASBIASFLAGS = spektra_type_b1(bData), true, eposRESULTASBIASFLAGS);
	// Check if dynamic test limit (result of previous test). In this case, consider test has no limit
	if(m_b1RESULTASBIASFLAGS & 0x80)
		m_bHaslimit = false;
	
	// TestItem.LIMITITEMNAME
	_FIELD_CHECKREAD(clSpektra.ReadString(szString, 6), eposLIMITITEMNAME);
	_FIELD_SET(m_cnLIMITITEMNAME = szString, true, eposLIMITITEMNAME);

	// TestItem.LIMITUNIT
	_FIELD_CHECKREAD(clSpektra.ReadString(szString, 2), eposLIMITUNIT);
	szString[1] = '\0'; // Second byte is not used
	_FIELD_SET(m_cnLIMITUNIT = szString, true, eposLIMITUNIT);

	// TestItem.LIMITVALUE
	_FIELD_CHECKREAD(clSpektra.ReadFloat(&fData), eposLIMITVALUE);
	_FIELD_SET(m_r4LIMITVALUE = spektra_type_r4(fData), true, eposLIMITVALUE);

	// TestItem.LIMITMIN
	_FIELD_CHECKREAD(clSpektra.ReadFloat(&fData), eposLIMITMIN);
	_FIELD_SET(m_r4LIMITMIN = spektra_type_r4(fData), true, eposLIMITMIN);

	// TestItem.LIMITMAX
	_FIELD_CHECKREAD(clSpektra.ReadFloat(&fData), eposLIMITMAX);
	_FIELD_SET(m_r4LIMITMAX = spektra_type_r4(fData), true, eposLIMITMAX);

	// TestItem.BIAS1NAME
	_FIELD_CHECKREAD(clSpektra.ReadString(szString, 4), eposBIAS1NAME);
	_FIELD_SET(m_cnBIAS1NAME = szString, true, eposBIAS1NAME);

	// TestItem.BIAS1UNIT
	_FIELD_CHECKREAD(clSpektra.ReadString(szString, 2), eposBIAS1UNIT);
	szString[1] = '\0'; // Second byte is not used
	_FIELD_SET(m_cnBIAS1UNIT = szString, true, eposBIAS1UNIT);

	// TestItem.BIAS1VALUE
	_FIELD_CHECKREAD(clSpektra.ReadFloat(&fData), eposBIAS1VALUE);
	_FIELD_SET(m_r4BIAS1VALUE = spektra_type_r4(fData), true, eposBIAS1VALUE);

	// TestItem.BIAS2NAME
	_FIELD_CHECKREAD(clSpektra.ReadString(szString, 4), eposBIAS2NAME);
	_FIELD_SET(m_cnBIAS2NAME = szString, true, eposBIAS2NAME);

	// TestItem.BIAS2UNIT
	_FIELD_CHECKREAD(clSpektra.ReadString(szString, 2), eposBIAS2UNIT);
	szString[1] = '\0'; // Second byte is not used
	_FIELD_SET(m_cnBIAS2UNIT = szString, true, eposBIAS2UNIT);

	// TestItem.BIAS2VALUE
	_FIELD_CHECKREAD(clSpektra.ReadFloat(&fData), eposBIAS2VALUE);
	_FIELD_SET(m_r4BIAS2VALUE = spektra_type_r4(fData), true, eposBIAS2VALUE);

	// TestItem.TIMECONDITIONNAME
	_FIELD_CHECKREAD(clSpektra.ReadString(szString, 4), eposTIMECONDITIONNAME);
	_FIELD_SET(m_cnTIMECONDITIONNAME = szString, true, eposTIMECONDITIONNAME);

	// TestItem.TIMEUNIT
	_FIELD_CHECKREAD(clSpektra.ReadString(szString, 2), eposTIMEUNIT);
	szString[1] = '\0'; // Second byte is not used
	_FIELD_SET(m_cnTIMEUNIT = szString, true, eposTIMEUNIT);

	// TestItem.TIMEVALUE
	_FIELD_CHECKREAD(clSpektra.ReadFloat(&fData), eposTIMEVALUE);
	_FIELD_SET(m_r4TIMEVALUE = spektra_type_r4(fData), true, eposTIMEVALUE);

	// TestItem.DLINHIBITFLAGS
	_FIELD_CHECKREAD(clSpektra.ReadByte(&bData), eposDLINHIBITFLAGS);
	_FIELD_SET(m_b1DLINHIBITFLAGS = spektra_type_b1(bData), true, eposDLINHIBITFLAGS);
	
	// TestItem.RESERVED
	_FIELD_CHECKREAD(clSpektra.ReadString(szString, 7), eposRESERVED);
	_FIELD_SET(m_cnRESERVED = szString, true, eposRESERVED);

	return true;
}

void CSpektra_TestItem_V3::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
	// Empty string first
	strAsciiString = "";

	// TestItem.TESTNUMBER
	_STR_ADDFIELD_ASCII(strAsciiString, "TestItem.TESTNUMBER", QString::number(m_u1TESTNUMBER), nFieldSelection, eposTESTNUMBER);

	// TestItem.ITEMGROUPCODE
	_CREATEHEXFIELD_FROM_U1_ASCII(m_u1ITEMGROUPCODE)
	_STR_ADDFIELD_ASCII(strAsciiString, "TestItem.ITEMGROUPCODE", m_strFieldValue_macro, nFieldSelection, eposITEMGROUPCODE);

	// TestItem.ITEMCODE
	_CREATEHEXFIELD_FROM_U2_ASCII(m_u2ITEMCODE)
	_STR_ADDFIELD_ASCII(strAsciiString, "TestItem.ITEMCODE", m_strFieldValue_macro, nFieldSelection, eposITEMCODE);
	
	// TestItem.BECONDITIONANDFLAGS
	_CREATEFIELD_FROM_B1_ASCII(m_b1BECONDITIONANDFLAGS)
	_STR_ADDFIELD_ASCII(strAsciiString, "TestItem.BECONDITIONANDFLAGS", m_strFieldValue_macro, nFieldSelection, eposBECONDITIONANDFLAGS);

	// TestItem.RESULTASBIASFLAGS
	_CREATEFIELD_FROM_B1_ASCII(m_b1RESULTASBIASFLAGS)
	_STR_ADDFIELD_ASCII(strAsciiString, "TestItem.RESULTASBIASFLAGS", m_strFieldValue_macro, nFieldSelection, eposRESULTASBIASFLAGS);

	// TestItem.LIMITITEMNAME
	_STR_ADDFIELD_ASCII(strAsciiString, "TestItem.LIMITITEMNAME", m_cnLIMITITEMNAME, nFieldSelection, eposLIMITITEMNAME);

	// TestItem.LIMITUNIT
	_STR_ADDFIELD_ASCII(strAsciiString, "TestItem.LIMITUNIT", m_cnLIMITUNIT, nFieldSelection, eposLIMITUNIT);
	
	// TestItem.LIMITVALUE
	_STR_ADDFIELD_ASCII(strAsciiString, "TestItem.LIMITVALUE", QString::number(m_r4LIMITVALUE), nFieldSelection, eposLIMITVALUE);

	// TestItem.LIMITMIN
	_STR_ADDFIELD_ASCII(strAsciiString, "TestItem.LIMITMIN", QString::number(m_r4LIMITMIN), nFieldSelection, eposLIMITMIN);

	// TestItem.LIMITMAX
	_STR_ADDFIELD_ASCII(strAsciiString, "TestItem.LIMITMAX", QString::number(m_r4LIMITMAX), nFieldSelection, eposLIMITMAX);

	// TestItem.BIAS1NAME
	_STR_ADDFIELD_ASCII(strAsciiString, "TestItem.BIAS1NAME", m_cnBIAS1NAME, nFieldSelection, eposBIAS1NAME);

	// TestItem.BIAS1UNIT
	_STR_ADDFIELD_ASCII(strAsciiString, "TestItem.BIAS1UNIT", m_cnBIAS1UNIT, nFieldSelection, eposBIAS1UNIT);
	
	// TestItem.BIAS1VALUE
	_STR_ADDFIELD_ASCII(strAsciiString, "TestItem.BIAS1VALUE", QString::number(m_r4BIAS1VALUE), nFieldSelection, eposBIAS1VALUE);

	// TestItem.BIAS2NAME
	_STR_ADDFIELD_ASCII(strAsciiString, "TestItem.BIAS2NAME", m_cnBIAS2NAME, nFieldSelection, eposBIAS2NAME);

	// TestItem.BIAS2UNIT
	_STR_ADDFIELD_ASCII(strAsciiString, "TestItem.BIAS2UNIT", m_cnBIAS2UNIT, nFieldSelection, eposBIAS2UNIT);
	
	// TestItem.BIAS2VALUE
	_STR_ADDFIELD_ASCII(strAsciiString, "TestItem.BIAS2VALUE", QString::number(m_r4BIAS2VALUE), nFieldSelection, eposBIAS2VALUE);

	// TestItem.TIMECONDITIONNAME
	_STR_ADDFIELD_ASCII(strAsciiString, "TestItem.TIMECONDITIONNAME", m_cnTIMECONDITIONNAME, nFieldSelection, eposTIMECONDITIONNAME);

	// TestItem.TIMEUNIT
	_STR_ADDFIELD_ASCII(strAsciiString, "TestItem.TIMEUNIT", m_cnTIMEUNIT, nFieldSelection, eposTIMEUNIT);
	
	// TestItem.TIMEVALUE
	_STR_ADDFIELD_ASCII(strAsciiString, "TestItem.TIMEVALUE", QString::number(m_r4TIMEVALUE), nFieldSelection, eposTIMEVALUE);

	// TestItem.DLINHIBITFLAGS
	_CREATEFIELD_FROM_B1_ASCII(m_b1DLINHIBITFLAGS)
	_STR_ADDFIELD_ASCII(strAsciiString, "TestItem.DLINHIBITFLAGS", m_strFieldValue_macro, nFieldSelection, eposDLINHIBITFLAGS);

	// TestItem.RESERVED
	_STR_ADDFIELD_ASCII(strAsciiString, "TestItem.RESERVED", m_cnRESERVED, nFieldSelection, eposRESERVED);
}

void CSpektra_TestItem_V3::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
	// Empty string list first
	listFields.empty();

	// TestItem.TESTNUMBER
	_LIST_ADDFIELD_ASCII(listFields, "TestItem.TESTNUMBER", QString::number(m_u1TESTNUMBER), nFieldSelection, eposTESTNUMBER);

	// TestItem.ITEMGROUPCODE
	_CREATEHEXFIELD_FROM_U1_ASCII(m_u1ITEMGROUPCODE)
	_LIST_ADDFIELD_ASCII(listFields, "TestItem.ITEMGROUPCODE", m_strFieldValue_macro, nFieldSelection, eposITEMGROUPCODE);

	// TestItem.ITEMCODE
	_CREATEHEXFIELD_FROM_U2_ASCII(m_u2ITEMCODE)
	_LIST_ADDFIELD_ASCII(listFields, "TestItem.ITEMCODE", m_strFieldValue_macro, nFieldSelection, eposITEMCODE);
	
	// TestItem.BECONDITIONANDFLAGS
	_CREATEFIELD_FROM_B1_ASCII(m_b1BECONDITIONANDFLAGS)
	_LIST_ADDFIELD_ASCII(listFields, "TestItem.BECONDITIONANDFLAGS", m_strFieldValue_macro, nFieldSelection, eposBECONDITIONANDFLAGS);

	// TestItem.RESULTASBIASFLAGS
	_CREATEFIELD_FROM_B1_ASCII(m_b1RESULTASBIASFLAGS)
	_LIST_ADDFIELD_ASCII(listFields, "TestItem.RESULTASBIASFLAGS", m_strFieldValue_macro, nFieldSelection, eposRESULTASBIASFLAGS);

	// TestItem.LIMITITEMNAME
	_LIST_ADDFIELD_ASCII(listFields, "TestItem.LIMITITEMNAME", m_cnLIMITITEMNAME, nFieldSelection, eposLIMITITEMNAME);

	// TestItem.LIMITUNIT
	_LIST_ADDFIELD_ASCII(listFields, "TestItem.LIMITUNIT", m_cnLIMITUNIT, nFieldSelection, eposLIMITUNIT);
	
	// TestItem.LIMITVALUE
	_LIST_ADDFIELD_ASCII(listFields, "TestItem.LIMITVALUE", QString::number(m_r4LIMITVALUE), nFieldSelection, eposLIMITVALUE);

	// TestItem.LIMITMIN
	_LIST_ADDFIELD_ASCII(listFields, "TestItem.LIMITMIN", QString::number(m_r4LIMITMIN), nFieldSelection, eposLIMITMIN);

	// TestItem.LIMITMAX
	_LIST_ADDFIELD_ASCII(listFields, "TestItem.LIMITMAX", QString::number(m_r4LIMITMAX), nFieldSelection, eposLIMITMAX);

	// TestItem.BIAS1NAME
	_LIST_ADDFIELD_ASCII(listFields, "TestItem.BIAS1NAME", m_cnBIAS1NAME, nFieldSelection, eposBIAS1NAME);

	// TestItem.BIAS1UNIT
	_LIST_ADDFIELD_ASCII(listFields, "TestItem.BIAS1UNIT", m_cnBIAS1UNIT, nFieldSelection, eposBIAS1UNIT);
	
	// TestItem.BIAS1VALUE
	_LIST_ADDFIELD_ASCII(listFields, "TestItem.BIAS1VALUE", QString::number(m_r4BIAS1VALUE), nFieldSelection, eposBIAS1VALUE);

	// TestItem.BIAS2NAME
	_LIST_ADDFIELD_ASCII(listFields, "TestItem.BIAS2NAME", m_cnBIAS2NAME, nFieldSelection, eposBIAS2NAME);

	// TestItem.BIAS2UNIT
	_LIST_ADDFIELD_ASCII(listFields, "TestItem.BIAS2UNIT", m_cnBIAS2UNIT, nFieldSelection, eposBIAS2UNIT);
	
	// TestItem.BIAS2VALUE
	_LIST_ADDFIELD_ASCII(listFields, "TestItem.BIAS2VALUE", QString::number(m_r4BIAS2VALUE), nFieldSelection, eposBIAS2VALUE);

	// TestItem.TIMECONDITIONNAME
	_LIST_ADDFIELD_ASCII(listFields, "TestItem.TIMECONDITIONNAME", m_cnTIMECONDITIONNAME, nFieldSelection, eposTIMECONDITIONNAME);

	// TestItem.TIMEUNIT
	_LIST_ADDFIELD_ASCII(listFields, "TestItem.TIMEUNIT", m_cnTIMEUNIT, nFieldSelection, eposTIMEUNIT);
	
	// TestItem.TIMEVALUE
	_LIST_ADDFIELD_ASCII(listFields, "TestItem.TIMEVALUE", QString::number(m_r4TIMEVALUE), nFieldSelection, eposTIMEVALUE);

	// TestItem.DLINHIBITFLAGS
	_CREATEFIELD_FROM_B1_ASCII(m_b1DLINHIBITFLAGS)
	_LIST_ADDFIELD_ASCII(listFields, "TestItem.DLINHIBITFLAGS", m_strFieldValue_macro, nFieldSelection, eposDLINHIBITFLAGS);

	// TestItem.RESERVED
	_LIST_ADDFIELD_ASCII(listFields, "TestItem.RESERVED", m_cnRESERVED, nFieldSelection, eposRESERVED);
}

void CSpektra_TestItem_V3::GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection /*= 0*/)
{
	QString strTabs="";

	// Init XML string
	for(int nIndex=nIndentationLevel; nIndex>0; nIndex--)
		strTabs += "\t";
	strXmlString = strTabs;
	strXmlString += "<testitem>\n";
	
	// TestItem.TESTNUMBER
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "testnumber", QString::number(m_u1TESTNUMBER), nFieldSelection, eposTESTNUMBER);

	// TestItem.ITEMGROUPCODE
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "itemgroupcode", QString::number(m_u1ITEMGROUPCODE), nFieldSelection, eposITEMGROUPCODE);

	// TestItem.ITEMCODE
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "itemcode", QString::number(m_u2ITEMCODE), nFieldSelection, eposITEMCODE);
	
	// TestItem.BECONDITIONANDFLAGS
	_CREATEFIELD_FROM_B1_XML(m_b1BECONDITIONANDFLAGS)
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "beconditionandflags", m_strFieldValue_macro, nFieldSelection, eposBECONDITIONANDFLAGS);

	// TestItem.RESULTASBIASFLAGS
	_CREATEFIELD_FROM_B1_XML(m_b1RESULTASBIASFLAGS)
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "resultasbiasflags", m_strFieldValue_macro, nFieldSelection, eposRESULTASBIASFLAGS);

	// TestItem.LIMITITEMNAME
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "limititemname", m_cnLIMITITEMNAME, nFieldSelection, eposLIMITITEMNAME);

	// TestItem.LIMITUNIT
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "limitunit", m_cnLIMITUNIT, nFieldSelection, eposLIMITUNIT);
	
	// TestItem.LIMITVALUE
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "limitvalue", QString::number(m_r4LIMITVALUE), nFieldSelection, eposLIMITVALUE);

	// TestItem.LIMITMIN
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "limitmin", QString::number(m_r4LIMITMIN), nFieldSelection, eposLIMITMIN);

	// TestItem.LIMITMAX
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "limitmax", QString::number(m_r4LIMITMAX), nFieldSelection, eposLIMITMAX);

	// TestItem.BIAS1NAME
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "bias1name", m_cnBIAS1NAME, nFieldSelection, eposBIAS1NAME);

	// TestItem.BIAS1UNIT
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "bias1unit", m_cnBIAS1UNIT, nFieldSelection, eposBIAS1UNIT);
	
	// TestItem.BIAS1VALUE
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "bias1value", QString::number(m_r4BIAS1VALUE), nFieldSelection, eposBIAS1VALUE);

	// TestItem.BIAS2NAME
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "bias2name", m_cnBIAS2NAME, nFieldSelection, eposBIAS2NAME);

	// TestItem.BIAS2UNIT
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "bias2unit", m_cnBIAS2UNIT, nFieldSelection, eposBIAS2UNIT);
	
	// TestItem.BIAS2VALUE
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "bias2value", QString::number(m_r4BIAS2VALUE), nFieldSelection, eposBIAS2VALUE);

	// TestItem.TIMECONDITIONNAME
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "timeconditionname", m_cnTIMECONDITIONNAME, nFieldSelection, eposTIMECONDITIONNAME);

	// TestItem.TIMEUNIT
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "timeunit", m_cnTIMEUNIT, nFieldSelection, eposTIMEUNIT);
	
	// TestItem.TIMEVALUE
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "timevalue", QString::number(m_r4TIMEVALUE), nFieldSelection, eposTIMEVALUE);

	// TestItem.DLINHIBITFLAGS
	_CREATEFIELD_FROM_B1_XML(m_b1DLINHIBITFLAGS)
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "dlinhibitflags", m_strFieldValue_macro, nFieldSelection, eposDLINHIBITFLAGS);

	// TestItem.RESERVED
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "reserved", m_cnRESERVED, nFieldSelection, eposRESERVED);

	strXmlString += strTabs;
	strXmlString += "</testitem>\n";
}

///////////////////////////////////////////////////////////
// WaferID RECORD
///////////////////////////////////////////////////////////
CSpektra_WaferID_V3::CSpektra_WaferID_V3() : CSpektra_Record_V3()
{
	Reset();
}

CSpektra_WaferID_V3::~CSpektra_WaferID_V3()
{
	Reset();
}

void CSpektra_WaferID_V3::Reset(void)
{
	// Reset field flags
	for(int i=0; i<eposEND; i++)
		m_pFieldFlags[i] = FieldFlag_Empty;

	// Select fields for reduced list

	// Reset Data
	m_u1WAFERNUMBER			= 0;		// WaferID.WAFERNUMBER
	m_u2NUMBEROFDEVICES		= 0;		// WaferID.NUMBEROFDEVICES
	m_cnRESERVED			= "";		// WaferID.RESERVED
}

QString CSpektra_WaferID_V3::GetRecordShortName(void)
{
	return "WaferID";
}

QString CSpektra_WaferID_V3::GetRecordLongName(void)
{
	return "WaferID";
}

int CSpektra_WaferID_V3::GetRecordType(void)
{
	return Rec_WaferID;
}

bool CSpektra_WaferID_V3::Read(CSpektra & clSpektra)
{
	BYTE	bData;
	WORD	wData;
	char	szString[SPEKTRA_MAX_U1+1];

	// First reset data
	Reset();

	// WaferID record must start with a 'W'
	if(clSpektra.ReadByte(&bData) != CSpektra::NoError)		return false;
	if(bData != 'W')	return false;

	// WaferID.WAFERNUMBER
	_FIELD_CHECKREAD(clSpektra.ReadByte(&bData), eposWAFERNUMBER);
	_FIELD_SET(m_u1WAFERNUMBER = spektra_type_u1(bData), true, eposWAFERNUMBER);

	// WaferID.NUMBEROFDEVICES
	_FIELD_CHECKREAD(clSpektra.ReadWord(&wData), eposNUMBEROFDEVICES);
	_FIELD_SET(m_u2NUMBEROFDEVICES = spektra_type_u2(wData), true, eposNUMBEROFDEVICES);

	// WaferID.RESERVED
	_FIELD_CHECKREAD(clSpektra.ReadString(szString, 2), eposRESERVED);
	_FIELD_SET(m_cnRESERVED = szString, true, eposRESERVED);

	return true;
}

void CSpektra_WaferID_V3::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
	// Empty string first
	strAsciiString = "";

	// WaferID.WAFERNUMBER
	_STR_ADDFIELD_ASCII(strAsciiString, "WaferID.WAFERNUMBER", QString::number(m_u1WAFERNUMBER), nFieldSelection, eposWAFERNUMBER);

	// WaferID.NUMBEROFDEVICES
	_STR_ADDFIELD_ASCII(strAsciiString, "WaferID.NUMBEROFDEVICES", QString::number(m_u2NUMBEROFDEVICES), nFieldSelection, eposNUMBEROFDEVICES);

	// WaferID.RESERVED
	_STR_ADDFIELD_ASCII(strAsciiString, "WaferID.RESERVED", m_cnRESERVED, nFieldSelection, eposRESERVED);
}

void CSpektra_WaferID_V3::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
	// Empty string list first
	listFields.empty();

	// WaferID.WAFERNUMBER
	_LIST_ADDFIELD_ASCII(listFields, "WaferID.WAFERNUMBER", QString::number(m_u1WAFERNUMBER), nFieldSelection, eposWAFERNUMBER);

	// WaferID.NUMBEROFDEVICES
	_LIST_ADDFIELD_ASCII(listFields, "WaferID.NUMBEROFDEVICES", QString::number(m_u2NUMBEROFDEVICES), nFieldSelection, eposNUMBEROFDEVICES);

	// WaferID.RESERVED
	_LIST_ADDFIELD_ASCII(listFields, "WaferID.RESERVED", m_cnRESERVED, nFieldSelection, eposRESERVED);
}

void CSpektra_WaferID_V3::GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection /*= 0*/)
{
	QString strTabs="";

	// Init XML string
	for(int nIndex=nIndentationLevel; nIndex>0; nIndex--)
		strTabs += "\t";
	strXmlString = strTabs;
	strXmlString += "<waferid>\n";
	
	// WaferID.WAFERNUMBER
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "wafernumber", QString::number(m_u1WAFERNUMBER), nFieldSelection, eposWAFERNUMBER);

	// WaferID.NUMBEROFDEVICES
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "numberofdevices", QString::number(m_u2NUMBEROFDEVICES), nFieldSelection, eposNUMBEROFDEVICES);

	// WaferID.RESERVED
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "reserved", m_cnRESERVED, nFieldSelection, eposRESERVED);

	strXmlString += strTabs;
	strXmlString += "</waferid>\n";
}

///////////////////////////////////////////////////////////
// DeviceID RECORD
///////////////////////////////////////////////////////////
CSpektra_DeviceID_V3::CSpektra_DeviceID_V3() : CSpektra_Record_V3()
{
	Reset();
}

CSpektra_DeviceID_V3::~CSpektra_DeviceID_V3()
{
	Reset();
}

void CSpektra_DeviceID_V3::Reset(void)
{
	// Reset field flags
	for(int i=0; i<eposEND; i++)
		m_pFieldFlags[i] = FieldFlag_Empty;

	// Select fields for reduced list

	// Reset Data
	m_u2SERIALNUMBER	= 0;			// DeviceID.SERIALNUMBER
	m_u2BINNUMBER		= 0;			// DeviceID.BINNUMBER
}

QString CSpektra_DeviceID_V3::GetRecordShortName(void)
{
	return "DeviceID";
}

QString CSpektra_DeviceID_V3::GetRecordLongName(void)
{
	return "DeviceID";
}

int CSpektra_DeviceID_V3::GetRecordType(void)
{
	return Rec_DeviceID;
}

bool CSpektra_DeviceID_V3::Read(CSpektra & clSpektra)
{
	BYTE	bData;
	WORD	wData;

	// First reset data
	Reset();

	// DeviceID record must start with a 'D'
	if(clSpektra.ReadByte(&bData) != CSpektra::NoError)		return false;
	if(bData != 'D')	return false;

	// Second byte of DeviceID record must be 00
	if(clSpektra.ReadByte(&bData) != CSpektra::NoError)		return false;
	if(bData != 0)	return false;

	// DeviceID.SERIALNUMBER
	_FIELD_CHECKREAD(clSpektra.ReadWord(&wData), eposSERIALNUMBER);
	_FIELD_SET(m_u2SERIALNUMBER = spektra_type_u2(wData), true, eposSERIALNUMBER);

	// DeviceID.BINNUMBER
	_FIELD_CHECKREAD(clSpektra.ReadWord(&wData), eposBINNUMBER);
	_FIELD_SET(m_u2BINNUMBER = spektra_type_u2(wData), true, eposBINNUMBER);

	return true;
}

void CSpektra_DeviceID_V3::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
	// Empty string first
	strAsciiString = "";

	// DeviceID.SERIALNUMBER
	_STR_ADDFIELD_ASCII(strAsciiString, "DeviceID.SERIALNUMBER", QString::number(m_u2SERIALNUMBER), nFieldSelection, eposSERIALNUMBER);

	// DeviceID.BINNUMBER
	_STR_ADDFIELD_ASCII(strAsciiString, "DeviceID.BINNUMBER", QString::number(m_u2BINNUMBER), nFieldSelection, eposBINNUMBER);
}

void CSpektra_DeviceID_V3::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
	// Empty string list first
	listFields.empty();

	// DeviceID.SERIALNUMBER
	_LIST_ADDFIELD_ASCII(listFields, "DeviceID.SERIALNUMBER", QString::number(m_u2SERIALNUMBER), nFieldSelection, eposSERIALNUMBER);

	// DeviceID.BINNUMBER
	_LIST_ADDFIELD_ASCII(listFields, "DeviceID.BINNUMBER", QString::number(m_u2BINNUMBER), nFieldSelection, eposBINNUMBER);
}

void CSpektra_DeviceID_V3::GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection /*= 0*/)
{
	QString strTabs="";

	// Init XML string
	for(int nIndex=nIndentationLevel; nIndex>0; nIndex--)
		strTabs += "\t";
	strXmlString = strTabs;
	strXmlString += "<deviceid>\n";
	
	// DeviceID.SERIALNUMBER
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "serialnumber", QString::number(m_u2SERIALNUMBER), nFieldSelection, eposSERIALNUMBER);

	// DeviceID.BINNUMBER
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "binnumber", QString::number(m_u2BINNUMBER), nFieldSelection, eposBINNUMBER);

	strXmlString += strTabs;
	strXmlString += "</deviceid>\n";
}

///////////////////////////////////////////////////////////
// TestData RECORD
///////////////////////////////////////////////////////////
CSpektra_TestData_V3::CSpektra_TestData_V3() : CSpektra_Record_V3()
{
	Reset();
}

CSpektra_TestData_V3::~CSpektra_TestData_V3()
{
	Reset();
}

void CSpektra_TestData_V3::Reset(void)
{
	// Reset field flags
	for(int i=0; i<eposEND; i++)
		m_pFieldFlags[i] = FieldFlag_Empty;

	// Select fields for reduced list

	// Reset Data
	m_u1TESTNUMBER		= 0;			// TestData.TESTNUMBER
	m_b1FLAGS			= 0;			// TestData.FLAGS
	m_r4TESTRESULTVALUE	= 0.0;			// TestData.TESTRESULTVALUE
}

QString CSpektra_TestData_V3::GetRecordShortName(void)
{
	return "TestData";
}

QString CSpektra_TestData_V3::GetRecordLongName(void)
{
	return "TestData";
}

int CSpektra_TestData_V3::GetRecordType(void)
{
	return Rec_TestData;
}

bool CSpektra_TestData_V3::Read(CSpektra & clSpektra)
{
	BYTE	bData;
	FLOAT	fData;

	// First reset data
	Reset();

	// TestData.TESTNUMBER
	_FIELD_CHECKREAD(clSpektra.ReadByte(&bData), eposTESTNUMBER);
	_FIELD_SET(m_u1TESTNUMBER = spektra_type_u1(bData), true, eposTESTNUMBER);

	// TestData.FLAGS
	_FIELD_CHECKREAD(clSpektra.ReadByte(&bData), eposFLAGS);
	_FIELD_SET(m_b1FLAGS = spektra_type_b1(bData), true, eposFLAGS);

	// TestData.TESTRESULTVALUE
	_FIELD_CHECKREAD(clSpektra.ReadFloat(&fData), eposTESTRESULTVALUE);
	_FIELD_SET(m_r4TESTRESULTVALUE = spektra_type_r4(fData), true, eposTESTRESULTVALUE);

	return true;
}

void CSpektra_TestData_V3::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
	// Empty string first
	strAsciiString = "";

	// TestData.TESTNUMBER
	_STR_ADDFIELD_ASCII(strAsciiString, "TestData.TESTNUMBER", QString::number(m_u1TESTNUMBER), nFieldSelection, eposTESTNUMBER);

	// TestData.FLAGS
	_CREATEFIELD_FROM_B1_ASCII(m_b1FLAGS)
	_STR_ADDFIELD_ASCII(strAsciiString, "TestData.FLAGS", m_strFieldValue_macro, nFieldSelection, eposFLAGS);

	// TestData.TESTRESULTVALUE
	_STR_ADDFIELD_ASCII(strAsciiString, "TestData.TESTRESULTVALUE", QString::number(m_r4TESTRESULTVALUE), nFieldSelection, eposTESTRESULTVALUE);
}

void CSpektra_TestData_V3::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
	// Empty string list first
	listFields.empty();

	// TestData.TESTNUMBER
	_LIST_ADDFIELD_ASCII(listFields, "TestData.TESTNUMBER", QString::number(m_u1TESTNUMBER), nFieldSelection, eposTESTNUMBER);

	// TestData.FLAGS
	_CREATEFIELD_FROM_B1_ASCII(m_b1FLAGS)
	_LIST_ADDFIELD_ASCII(listFields, "TestData.FLAGS", m_strFieldValue_macro, nFieldSelection, eposFLAGS);

	// TestData.TESTRESULTVALUE
	_LIST_ADDFIELD_ASCII(listFields, "TestData.TESTRESULTVALUE", QString::number(m_r4TESTRESULTVALUE), nFieldSelection, eposTESTRESULTVALUE);
}

void CSpektra_TestData_V3::GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection /*= 0*/)
{
	QString strTabs="";

	// Init XML string
	for(int nIndex=nIndentationLevel; nIndex>0; nIndex--)
		strTabs += "\t";
	strXmlString = strTabs;
	strXmlString += "<testdata>\n";
	
	// TestData.TESTNUMBER
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "testnumber", QString::number(m_u1TESTNUMBER), nFieldSelection, eposTESTNUMBER);

	// TestData.FLAGS
	_CREATEFIELD_FROM_B1_XML(m_b1FLAGS)
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "flags", m_strFieldValue_macro, nFieldSelection, eposFLAGS);

	// TestData.TESTRESULTVALUE
	_STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "limitvalue", QString::number(m_r4TESTRESULTVALUE), nFieldSelection, eposTESTRESULTVALUE);

	strXmlString += strTabs;
	strXmlString += "</testdata>\n";
}

