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
// StdfParse class IMPLEMENTATION :
// This class contains routines to read records from an
// stdf file
///////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

#include <gqtl_log.h>

#include "stdfrecords_v4.h"

// For MemoryLeak detection: ALWAYS have to be LAST #include in file
//#include "DebugMemory.h"


namespace GQTL_STDF
{
///////////////////////////////////////////////////////////
// RESERVED_IMAGE RECORD
///////////////////////////////////////////////////////////
Stdf_RESERVED_IMAGE_V4::Stdf_RESERVED_IMAGE_V4() : Stdf_Record()
{
    Reset();
}

Stdf_RESERVED_IMAGE_V4::~Stdf_RESERVED_IMAGE_V4()
{
    Reset();
}

void Stdf_RESERVED_IMAGE_V4::Reset(void)
{
    // Reset field flags

    // Select fields for reduced list

    // Reset Data

    // Call Reset base method
    Stdf_Record::Reset();
}

QString Stdf_RESERVED_IMAGE_V4::GetRecordShortName(void)
{
    return "RESERVED_IMAGE";
}

QString Stdf_RESERVED_IMAGE_V4::GetRecordLongName(void)
{
    return "Reserved Image Record";
}

int Stdf_RESERVED_IMAGE_V4::GetRecordType(void)
{
    return Rec_RESERVED_IMAGE;
}

bool Stdf_RESERVED_IMAGE_V4::Read(GS::StdLib::Stdf& /*clStdf*/)
{
    // First reset data
    Reset();

    return true;
}

bool Stdf_RESERVED_IMAGE_V4::Write(GS::StdLib::Stdf& /*clStdf*/)
{
    return true;
}

void Stdf_RESERVED_IMAGE_V4::GetAsciiString(QString& strAsciiString,
                                             int /*nFieldSelection = 0*/)
{
    // Empty string first
    strAsciiString = "";
}

void Stdf_RESERVED_IMAGE_V4::GetAsciiFieldList(QStringList& listFields,
                                                int /*nFieldSelection = 0*/)
{
    // Empty string list first
    listFields.empty();
}

void Stdf_RESERVED_IMAGE_V4::GetXMLString(QString& strXmlString,
                                           const int nIndentationLevel,
                                           int /*nFieldSelection = 0*/)
{
    QString strTabs="";

    // Init XML string
    for(int nIndex=nIndentationLevel; nIndex>0; nIndex--)
        strTabs += "\t";
    strXmlString = strTabs;
    strXmlString += "<reserved_image>\n";

    strXmlString += strTabs;
    strXmlString += "</reserved_image>\n";
}

void Stdf_RESERVED_IMAGE_V4::GetAtdfString(QString & strAtdfString)
{
    // Empty string first
    strAtdfString = "";
}

///////////////////////////////////////////////////////////
// RESERVED_IG900 RECORD
///////////////////////////////////////////////////////////
Stdf_RESERVED_IG900_V4::Stdf_RESERVED_IG900_V4() : Stdf_Record()
{
    Reset();
}

Stdf_RESERVED_IG900_V4::~Stdf_RESERVED_IG900_V4()
{
    Reset();
}

void Stdf_RESERVED_IG900_V4::Reset(void)
{
    // Reset field flags

    // Select fields for reduced list

    // Reset Data

    // Call Reset base method
    Stdf_Record::Reset();
}

QString Stdf_RESERVED_IG900_V4::GetRecordShortName(void)
{
    return "RESERVED_IG900";
}

QString Stdf_RESERVED_IG900_V4::GetRecordLongName(void)
{
    return "Reserved IG900 Record";
}

int Stdf_RESERVED_IG900_V4::GetRecordType(void)
{
    return Rec_RESERVED_IG900;
}

bool Stdf_RESERVED_IG900_V4::Read(GS::StdLib::Stdf& /*clStdf*/)
{
    // First reset data
    Reset();

    return true;
}

bool Stdf_RESERVED_IG900_V4::Write(GS::StdLib::Stdf& /*clStdf*/)
{
    return true;
}

void Stdf_RESERVED_IG900_V4::GetAsciiString(QString& strAsciiString,
                                             int /*nFieldSelection = 0*/)
{
    // Empty string first
    strAsciiString = "";
}

void Stdf_RESERVED_IG900_V4::GetAsciiFieldList(QStringList& listFields,
                                                int /*nFieldSelection = 0*/)
{
    // Empty string list first
    listFields.empty();
}

void Stdf_RESERVED_IG900_V4::GetXMLString(QString& strXmlString,
                                           const int nIndentationLevel,
                                           int /*nFieldSelection = 0*/)
{
    QString strTabs="";

    // Init XML string
    for(int nIndex=nIndentationLevel; nIndex>0; nIndex--)
        strTabs += "\t";
    strXmlString = strTabs;
    strXmlString += "<reserved_ig900>\n";

    strXmlString += strTabs;
    strXmlString += "</reserved_ig900>\n";
}

void Stdf_RESERVED_IG900_V4::GetAtdfString(QString & strAtdfString)
{
    // Empty string first
    strAtdfString = "";
}

///////////////////////////////////////////////////////////
// UNKNOWN RECORD
///////////////////////////////////////////////////////////
Stdf_UNKNOWN_V4::Stdf_UNKNOWN_V4() : Stdf_Record()
{
    Reset();
}

Stdf_UNKNOWN_V4::~Stdf_UNKNOWN_V4()
{
    Reset();
}

void Stdf_UNKNOWN_V4::Reset(void)
{
    // Reset field flags

    // Select fields for reduced list

    // Reset Data

    // Call Reset base method
    Stdf_Record::Reset();
}

QString Stdf_UNKNOWN_V4::GetRecordShortName(void)
{
    return "UNKNOWN";
}

QString Stdf_UNKNOWN_V4::GetRecordLongName(void)
{
    return "Unknown Record";
}

int Stdf_UNKNOWN_V4::GetRecordType(void)
{
    return Rec_UNKNOWN;
}

bool Stdf_UNKNOWN_V4::Read(GS::StdLib::Stdf& /*clStdf*/)
{
    // First reset data
    Reset();

    return true;
}

bool Stdf_UNKNOWN_V4::Write(GS::StdLib::Stdf& /*clStdf*/)
{
    return true;
}

void Stdf_UNKNOWN_V4::GetAsciiString(QString& strAsciiString,
                                      int /*nFieldSelection = 0*/)
{
    // Empty string first
    strAsciiString = "";
}

void Stdf_UNKNOWN_V4::GetAsciiFieldList(QStringList& listFields,
                                         int /*nFieldSelection = 0*/)
{
    // Empty string list first
    listFields.empty();
}

void Stdf_UNKNOWN_V4::GetXMLString(QString& strXmlString,
                                    const int nIndentationLevel,
                                    int /*nFieldSelection = 0*/)
{
    QString strTabs="";

    // Init XML string
    for(int nIndex=nIndentationLevel; nIndex>0; nIndex--)
        strTabs += "\t";
    strXmlString = strTabs;
    strXmlString += "<unknown>\n";

    strXmlString += strTabs;
    strXmlString += "</unknown>\n";
}

void Stdf_UNKNOWN_V4::GetAtdfString(QString & strAtdfString)
{
    // Empty string first
    strAtdfString = "";
}

///////////////////////////////////////////////////////////
// FAR RECORD
///////////////////////////////////////////////////////////
Stdf_FAR_V4::Stdf_FAR_V4() : Stdf_Record()
{
    Reset();
}

Stdf_FAR_V4::~Stdf_FAR_V4()
{
    Reset();
}

void Stdf_FAR_V4::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Select fields for reduced list
    m_pFieldFlags[eposCPU_TYPE]	|= FieldFlag_ReducedList;
    m_pFieldFlags[eposSTDF_VER]	|= FieldFlag_ReducedList;

    // Reset Data
    m_u1CPU_TYPE		= 0;		// FAR.CPU_TYPE
    m_u1STDF_VER		= 0;		// FAR.STDF_VER

    // Call Reset base method
    Stdf_Record::Reset();
}

bool Stdf_FAR_V4::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;

    return false;
}

QString Stdf_FAR_V4::GetRecordShortName(void)
{
    return "FAR";
}

QString Stdf_FAR_V4::GetRecordLongName(void)
{
    return "File Attributes Record";
}

int Stdf_FAR_V4::GetRecordType(void)
{
    return Rec_FAR;
}

bool Stdf_FAR_V4::Read(GS::StdLib::Stdf & clStdf)
{
    BYTE	bData;

    // First reset data
    Reset();

    // FAR.CPU_TYPE
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposCPU_TYPE);
    _FIELD_SET(m_u1CPU_TYPE = stdf_type_u1(bData), true, eposCPU_TYPE);

    // FAR.STDF_VER
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposSTDF_VER);
    _FIELD_SET(m_u1STDF_VER = stdf_type_u1(bData), true, eposSTDF_VER);

    return true;
}

bool Stdf_FAR_V4::Write(GS::StdLib::Stdf & clStdf)
{
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;

    RecordReadInfo.iRecordType = STDF_FAR_TYPE;
    RecordReadInfo.iRecordSubType = STDF_FAR_STYPE;
    clStdf.WriteHeader(&RecordReadInfo);

    // FAR.CPU_TYPE
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_u1CPU_TYPE)), eposCPU_TYPE, clStdf.WriteRecord());

    // FAR.STDF_VER
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_u1STDF_VER)), eposSTDF_VER, clStdf.WriteRecord());

    clStdf.WriteRecord();

    return true;
}

void Stdf_FAR_V4::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    // Empty string first
    strAsciiString = "";

    // FAR.CPU_TYPE
    _STR_ADDFIELD_ASCII(strAsciiString, "FAR.cpu_type", QString::number(m_u1CPU_TYPE), nFieldSelection, eposCPU_TYPE);

    // FAR.STDF_VER
    _STR_ADDFIELD_ASCII(strAsciiString, "FAR.stdf_ver", QString::number(m_u1STDF_VER), nFieldSelection, eposSTDF_VER);
}

void Stdf_FAR_V4::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
    // Empty string list first
    listFields.empty();

    // FAR.CPU_TYPE
    _LIST_ADDFIELD_ASCII(listFields, "FAR.cpu_type", QString::number(m_u1CPU_TYPE), nFieldSelection, eposCPU_TYPE);

    // FAR.STDF_VER
    _LIST_ADDFIELD_ASCII(listFields, "FAR.stdf_ver", QString::number(m_u1STDF_VER), nFieldSelection, eposSTDF_VER);
}

void Stdf_FAR_V4::GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection /*= 0*/)
{
    QString strTabs="";

    // Init XML string
    for(int nIndex=nIndentationLevel; nIndex>0; nIndex--)
        strTabs += "\t";
    strXmlString = strTabs;
    strXmlString += "<far>\n";

    // FAR.CPU_TYPE
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "cpu_type", QString::number(m_u1CPU_TYPE), nFieldSelection, eposCPU_TYPE);

    // FAR.STDF_VER
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "stdf_ver", QString::number(m_u1STDF_VER), nFieldSelection, eposSTDF_VER);

    strXmlString += strTabs;
    strXmlString += "</far>\n";
}

void Stdf_FAR_V4::GetAtdfString(QString & strAtdfString)
{
    // Write FAR
    strAtdfString = "FAR:A|4|2|S";

    _STR_FINISHFIELD_ATDF(strAtdfString);

}

///////////////////////////////////////////////////////////
// ATR RECORD
///////////////////////////////////////////////////////////
Stdf_ATR_V4::Stdf_ATR_V4() : Stdf_Record()
{
    Reset();
}

Stdf_ATR_V4::~Stdf_ATR_V4()
{
    Reset();
}

void Stdf_ATR_V4::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Select fields for reduced list
    m_pFieldFlags[eposMOD_TIM]	|= FieldFlag_ReducedList;
    m_pFieldFlags[eposCMD_LINE]	|= FieldFlag_ReducedList;

    // Reset Data
    m_u4MOD_TIM		= 0;		// ATR.MOD_TIM
    m_cnCMD_LINE	= "";		// ATR.CMD_LINE

    // Call Reset base method
    Stdf_Record::Reset();
}

bool Stdf_ATR_V4::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;

    return false;
}

QString Stdf_ATR_V4::GetRecordShortName(void)
{
    return "ATR";
}

QString Stdf_ATR_V4::GetRecordLongName(void)
{
    return "Audit Trail Record";
}

int Stdf_ATR_V4::GetRecordType(void)
{
    return Rec_ATR;
}

bool Stdf_ATR_V4::Read(GS::StdLib::Stdf & clStdf)
{
    long	dwData=0;
    char	szString[STDF_MAX_U1+1];

    // First reset data
    Reset();

    // ATR.MOD_TIM
    _FIELD_CHECKREAD(clStdf.ReadDword(&dwData), eposMOD_TIM);
    _FIELD_SET(m_u4MOD_TIM = stdf_type_u4(dwData), true, eposMOD_TIM);

    // ATR.CMD_LINE
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposCMD_LINE);
    _FIELD_SET(m_cnCMD_LINE = szString, true, eposCMD_LINE);

    return true;
}

bool Stdf_ATR_V4::Write(GS::StdLib::Stdf & clStdf)
{
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;

    RecordReadInfo.iRecordType = STDF_ATR_TYPE;
    RecordReadInfo.iRecordSubType = STDF_ATR_STYPE;
    clStdf.WriteHeader(&RecordReadInfo);

    // ATR.MOD_TIM
    _FIELD_CHECKWRITE(clStdf.WriteDword(DWORD(m_u4MOD_TIM)), eposMOD_TIM, clStdf.WriteRecord());

    // ATR.CMD_LINE
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnCMD_LINE.toLatin1().constData()), eposCMD_LINE, clStdf.WriteRecord());

    clStdf.WriteRecord();

    return true;
}

void Stdf_ATR_V4::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    // Empty string first
    strAsciiString = "";

    // ATR.MOD_TIM
    _STR_ADDFIELD_ASCII(strAsciiString, "ATR.mod_tim", QString::number(m_u4MOD_TIM), nFieldSelection, eposMOD_TIM);

    // ATR.CMD_LINE
    _STR_ADDFIELD_ASCII(strAsciiString, "ATR.cmd_line", m_cnCMD_LINE, nFieldSelection, eposCMD_LINE);
}

void Stdf_ATR_V4::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
    // Empty string list first
    listFields.empty();

    // ATR.MOD_TIM
    _LIST_ADDFIELD_ASCII(listFields, "ATR.mod_tim", QString::number(m_u4MOD_TIM), nFieldSelection, eposMOD_TIM);

    // ATR.CMD_LINE
    _LIST_ADDFIELD_ASCII(listFields, "ATR.cmd_line", m_cnCMD_LINE, nFieldSelection, eposCMD_LINE);
}

void Stdf_ATR_V4::GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection /*= 0*/)
{
    QString strTabs="";

    // Init XML string
    for(int nIndex=nIndentationLevel; nIndex>0; nIndex--)
        strTabs += "\t";
    strXmlString = strTabs;
    strXmlString += "<atr>\n";

    // ATR.MOD_TIM
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "mod_tim", QString::number(m_u4MOD_TIM), nFieldSelection, eposMOD_TIM);

    // ATR.CMD_LINE
    _CREATEFIELD_FROM_CN_XML(m_cnCMD_LINE);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "cmd_line", m_strFieldValue_macro, nFieldSelection, eposCMD_LINE);

    strXmlString += strTabs;
    strXmlString += "</atr>\n";
}

void Stdf_ATR_V4::GetAtdfString(QString & strAtdfString)
{
    // Reset field index, and line Nb
    m_uiFieldIndex = 0;
    m_uiLineNb = 1;

    // Init string
    strAtdfString = "ATR:";

    // Convert TimeStamp to date
    QDateTime cDateTime;
    cDateTime.setTimeSpec(Qt::UTC);
    cDateTime.setTime_t(m_u4MOD_TIM);

    QString strDate = cDateTime.toString("hh:mm:ss dd-MM-yyyy");
    strDate = strDate.replace("-01-","-Jan-").replace("-02-","-Feb-").replace("-03-","-Mar-").replace("-04-","-Apr-").replace("-05-","-May-").replace("-06-","-Jun-").replace("-07-","-Jul-").replace("-08-","-Aug-").replace("-09-","-Sep-").replace("-10-","-Oct-").replace("-11-","-Nov-").replace("-12-","-Dec-");

    // ATR.MOD_TIM
    _STR_ADDFIELD_ATDF(strAtdfString, strDate, eposMOD_TIM);

    // ATR.CMD_LINE
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnCMD_LINE, eposCMD_LINE);

    _STR_FINISHFIELD_ATDF(strAtdfString);
}

///////////////////////////////////////////////////////////
// MIR RECORD
///////////////////////////////////////////////////////////
Stdf_MIR_V4::Stdf_MIR_V4() : Stdf_Record()
{
    Reset();
}

Stdf_MIR_V4::~Stdf_MIR_V4()
{
    Reset();
}

void Stdf_MIR_V4::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Select fields for reduced list
    m_pFieldFlags[eposSETUP_T]	|= FieldFlag_ReducedList;
    m_pFieldFlags[eposSTART_T]	|= FieldFlag_ReducedList;
    m_pFieldFlags[eposSTAT_NUM]	|= FieldFlag_ReducedList;
    m_pFieldFlags[eposEXEC_TYP]	|= FieldFlag_ReducedList;
    m_pFieldFlags[eposUSER_TXT]	|= FieldFlag_ReducedList;

    // Reset Data
    m_u4SETUP_T			= 0;			// MIR.SETUP_T
    m_u4START_T			= 0;			// MIR.START_T
    m_u1STAT_NUM		= 0;			// MIR.STAT_NUM
    m_c1MODE_COD		= 0;			// MIR.MODE_COD
    m_c1RTST_COD		= 0;			// MIR.RTST_COD
    m_c1PROT_COD		= 0;			// MIR.PROT_COD
    m_u2BURN_TIM		= 0;			// MIR.BURN_TIM
    m_c1CMOD_COD		= 0;			// MIR.CMOD_COD
    m_cnLOT_ID			= "";			// MIR.LOT_ID
    m_cnPART_TYP		= "";			// MIR.PART_TYP
    m_cnNODE_NAM		= "";			// MIR.NODE_NAM
    m_cnTSTR_TYP		= "";			// MIR.TSTR_TYP
    m_cnJOB_NAM			= "";			// MIR.JOB_NAM
    m_cnJOB_REV			= "";			// MIR.JOB_REV
    m_cnSBLOT_ID		= "";			// MIR.SBLOT_ID
    m_cnOPER_NAM		= "";			// MIR.OPER_NAM
    m_cnEXEC_TYP		= "";			// MIR.EXEC_TYP
    m_cnEXEC_VER		= "";			// MIR.EXEC_VER
    m_cnTEST_COD		= "";			// MIR.TEST_COD
    m_cnTST_TEMP		= "";			// MIR.TST_TEMP
    m_cnUSER_TXT		= "";			// MIR.USER_TXT
    m_cnAUX_FILE		= "";			// MIR.AUX_FILE
    m_cnPKG_TYP			= "";			// MIR.PKG_TYP
    m_cnFAMLY_ID		= "";			// MIR.FAMLY_ID
    m_cnDATE_COD		= "";			// MIR.DATE_COD
    m_cnFACIL_ID		= "";			// MIR.FACIL_ID
    m_cnFLOOR_ID		= "";			// MIR.FLOOR_ID
    m_cnPROC_ID			= "";			// MIR.PROC_ID
    m_cnOPER_FRQ		= "";			// MIR.OPER_FRQ
    m_cnSPEC_NAM		= "";			// MIR.SPEC_NAM
    m_cnSPEC_VER		= "";			// MIR.SPEC_VER
    m_cnFLOW_ID			= "";			// MIR.FLOW_ID
    m_cnSETUP_ID		= "";			// MIR.SETUP_ID
    m_cnDSGN_REV		= "";			// MIR.DSGN_REV
    m_cnENG_ID			= "";			// MIR.ENG_ID
    m_cnROM_COD			= "";			// MIR.ROM_COD
    m_cnSERL_NUM		= "";			// MIR.SERL_NUM
    m_cnSUPR_NAM		= "";			// MIR.SUPR_NAM

    // Call Reset base method
    Stdf_Record::Reset();
}

bool Stdf_MIR_V4::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;

    return false;
}

QString Stdf_MIR_V4::GetRecordShortName(void)
{
    return "MIR";
}

QString Stdf_MIR_V4::GetRecordLongName(void)
{
    return "Master Information Record";
}

int Stdf_MIR_V4::GetRecordType(void)
{
    return Rec_MIR;
}

bool Stdf_MIR_V4::Read(GS::StdLib::Stdf & clStdf)
{
    char	szString[STDF_MAX_U1+1];
    long	dwData;
    int		wData;
    BYTE	bData;

    // First reset data
    Reset();

    // MIR.SETUP_T
    _FIELD_CHECKREAD(clStdf.ReadDword(&dwData), eposSETUP_T);
    _FIELD_SET(m_u4SETUP_T = stdf_type_u4(dwData), true, eposSETUP_T);

    // MIR.START_T
    _FIELD_CHECKREAD(clStdf.ReadDword(&dwData), eposSTART_T);
    _FIELD_SET(m_u4START_T = stdf_type_u4(dwData), true, eposSTART_T);

    // MIR.STAT_NUM
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposSTAT_NUM);
    _FIELD_SET(m_u1STAT_NUM = stdf_type_u1(bData), true, eposSTAT_NUM);

    // MIR.MODE_COD
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposMODE_COD);
    _FIELD_SET(m_c1MODE_COD = stdf_type_c1(bData), m_c1MODE_COD != ' ', eposMODE_COD);

    // MIR.RTST_COD
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposRTST_COD);
    _FIELD_SET(m_c1RTST_COD = stdf_type_c1(bData), m_c1RTST_COD != ' ', eposRTST_COD);

    // MIR.PROT_COD
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposPROT_COD);
    _FIELD_SET(m_c1PROT_COD = stdf_type_c1(bData), m_c1PROT_COD != ' ', eposPROT_COD);

    // MIR.BURN_TIM
    _FIELD_CHECKREAD(clStdf.ReadWord(&wData), eposBURN_TIM);
    _FIELD_SET(m_u2BURN_TIM = stdf_type_u2(wData), m_u2BURN_TIM != 65535, eposBURN_TIM);

    // MIR.CMOD_COD
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposCMOD_COD);
    _FIELD_SET(m_c1CMOD_COD = stdf_type_c1(bData), m_c1CMOD_COD != ' ', eposCMOD_COD);

    // MIR.LOT_ID
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposLOT_ID);
    _FIELD_SET(m_cnLOT_ID = szString, true, eposLOT_ID);

    // MIR.PART_TYP
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposPART_TYP);
    _FIELD_SET(m_cnPART_TYP = szString, true, eposPART_TYP);

    // MIR.NODE_NAM
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposNODE_NAM);
    _FIELD_SET(m_cnNODE_NAM = szString, true, eposNODE_NAM);

    // MIR.TSTR_TYP
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposTSTR_TYP);
    _FIELD_SET(m_cnTSTR_TYP = szString, true, eposTSTR_TYP);

    // MIR.JOB_NAM
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposJOB_NAM);
    _FIELD_SET(m_cnJOB_NAM = szString, true, eposJOB_NAM);

    // MIR.JOB_REV
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposJOB_REV);
    _FIELD_SET(m_cnJOB_REV = szString, !m_cnJOB_REV.isEmpty(), eposJOB_REV);

    // MIR.SBLOT_ID
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposSBLOT_ID);
    _FIELD_SET(m_cnSBLOT_ID = szString, !m_cnSBLOT_ID.isEmpty(), eposSBLOT_ID);

    // MIR.OPER_NAM
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposOPER_NAM);
    _FIELD_SET(m_cnOPER_NAM = szString, !m_cnOPER_NAM.isEmpty(), eposOPER_NAM);

    // MIR.EXEC_TYP
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposEXEC_TYP);
    _FIELD_SET(m_cnEXEC_TYP = szString, !m_cnEXEC_TYP.isEmpty(), eposEXEC_TYP);

    // MIR.EXEC_VER
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposEXEC_VER);
    _FIELD_SET(m_cnEXEC_VER = szString, !m_cnEXEC_VER.isEmpty(), eposEXEC_VER);

    // MIR.TEST_COD
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposTEST_COD);
    _FIELD_SET(m_cnTEST_COD = szString, !m_cnTEST_COD.isEmpty(), eposTEST_COD);

    // MIR.TST_TEMP
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposTST_TEMP);
    _FIELD_SET(m_cnTST_TEMP = szString, !m_cnTST_TEMP.isEmpty(), eposTST_TEMP);

    // MIR.USER_TXT
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposUSER_TXT);
    _FIELD_SET(m_cnUSER_TXT = szString, !m_cnUSER_TXT.isEmpty(), eposUSER_TXT);

    // MIR.AUX_FILE
    _FIELD_CHECKREAD(clStdf.ReadString(szString),eposAUX_FILE );
    _FIELD_SET(m_cnAUX_FILE = szString, !m_cnAUX_FILE.isEmpty(), eposAUX_FILE);

    // MIR.PKG_TYP
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposPKG_TYP);
    _FIELD_SET(m_cnPKG_TYP = szString, !m_cnPKG_TYP.isEmpty(), eposPKG_TYP);

    // MIR.FAMLY_ID
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposFAMLY_ID);
    _FIELD_SET(m_cnFAMLY_ID = szString, !m_cnFAMLY_ID.isEmpty(), eposFAMLY_ID);

    // MIR.DATE_COD
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposDATE_COD);
    _FIELD_SET(m_cnDATE_COD = szString, !m_cnDATE_COD.isEmpty(), eposDATE_COD);

    // MIR.FACIL_ID
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposFACIL_ID);
    _FIELD_SET(m_cnFACIL_ID = szString, !m_cnFACIL_ID.isEmpty(), eposFACIL_ID);

    // MIR.FLOOR_ID
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposFLOOR_ID);
    _FIELD_SET(m_cnFLOOR_ID = szString, !m_cnFLOOR_ID.isEmpty(), eposFLOOR_ID);

    // MIR.PROC_ID
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposPROC_ID);
    _FIELD_SET(m_cnPROC_ID = szString, !m_cnPROC_ID.isEmpty(), eposPROC_ID);

    // MIR.OPER_FRQ
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposOPER_FRQ);
    _FIELD_SET(m_cnOPER_FRQ = szString, !m_cnOPER_FRQ.isEmpty(), eposOPER_FRQ);

    // MIR.SPEC_NAM
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposSPEC_NAM);
    _FIELD_SET(m_cnSPEC_NAM = szString, !m_cnSPEC_NAM.isEmpty(), eposSPEC_NAM);

    // MIR.SPEC_VER
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposSPEC_VER);
    _FIELD_SET(m_cnSPEC_VER = szString, !m_cnSPEC_VER.isEmpty(), eposSPEC_VER);

    // MIR.FLOW_ID
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposFLOW_ID);
    _FIELD_SET(m_cnFLOW_ID = szString, !m_cnFLOW_ID.isEmpty(), eposFLOW_ID);

    // MIR.SETUP_ID
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposSETUP_ID);
    _FIELD_SET(m_cnSETUP_ID = szString, !m_cnSETUP_ID.isEmpty(), eposSETUP_ID);

    // MIR.DSGN_REV
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposDSGN_REV);
    _FIELD_SET(m_cnDSGN_REV = szString, !m_cnDSGN_REV.isEmpty(), eposDSGN_REV);

    // MIR.ENG_ID
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposENG_ID);
    _FIELD_SET(m_cnENG_ID = szString, !m_cnENG_ID.isEmpty(), eposENG_ID);

    // MIR.ROM_COD
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposROM_COD);
    _FIELD_SET(m_cnROM_COD = szString, !m_cnROM_COD.isEmpty(), eposROM_COD);

    // MIR.SERL_NUM
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposSERL_NUM);
    _FIELD_SET(m_cnSERL_NUM = szString, !m_cnSERL_NUM.isEmpty(), eposSERL_NUM);

    // MIR.SUPR_NAM
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposSUPR_NAM);
    _FIELD_SET(m_cnSUPR_NAM = szString, !m_cnSUPR_NAM.isEmpty(), eposSUPR_NAM);

    return true;
}

bool Stdf_MIR_V4::Write(GS::StdLib::Stdf & clStdf)
{
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;

    RecordReadInfo.iRecordType = STDF_MIR_TYPE;
    RecordReadInfo.iRecordSubType = STDF_MIR_STYPE;
    clStdf.WriteHeader(&RecordReadInfo);

    // MIR.SETUP_T
    _FIELD_CHECKWRITE(clStdf.WriteDword(DWORD(m_u4SETUP_T)), eposSETUP_T, clStdf.WriteRecord());

    // MIR.START_T
    _FIELD_CHECKWRITE(clStdf.WriteDword(DWORD(m_u4START_T)), eposSTART_T, clStdf.WriteRecord());

    // MIR.STAT_NUM
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_u1STAT_NUM)), eposSTAT_NUM, clStdf.WriteRecord());

    // MIR.MODE_COD
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_c1MODE_COD)), eposMODE_COD, clStdf.WriteRecord());

    // MIR.RTST_COD
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_c1RTST_COD)), eposRTST_COD, clStdf.WriteRecord());

    // MIR.PROT_COD
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_c1PROT_COD)), eposPROT_COD, clStdf.WriteRecord());

    // MIR.BURN_TIM
    _FIELD_CHECKWRITE(clStdf.WriteWord(WORD(m_u2BURN_TIM)), eposBURN_TIM, clStdf.WriteRecord());

    // MIR.CMOD_COD
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_c1CMOD_COD)), eposCMOD_COD, clStdf.WriteRecord());

    // MIR.LOT_ID
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnLOT_ID.toLatin1().constData()), eposLOT_ID, clStdf.WriteRecord());

    // MIR.PART_TYP
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnPART_TYP.toLatin1().constData()), eposPART_TYP, clStdf.WriteRecord());

    // MIR.NODE_NAM
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnNODE_NAM.toLatin1().constData()), eposNODE_NAM, clStdf.WriteRecord());

    // MIR.TSTR_TYP
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnTSTR_TYP.toLatin1().constData()), eposTSTR_TYP, clStdf.WriteRecord());

    // MIR.JOB_NAM
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnJOB_NAM.toLatin1().constData()), eposJOB_NAM, clStdf.WriteRecord());

    // MIR.JOB_REV
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnJOB_REV.toLatin1().constData()), eposJOB_REV, clStdf.WriteRecord());

    // MIR.SBLOT_ID
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnSBLOT_ID.toLatin1().constData()), eposSBLOT_ID, clStdf.WriteRecord());

    // MIR.OPER_NAM
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnOPER_NAM.toLatin1().constData()), eposOPER_NAM, clStdf.WriteRecord());

    // MIR.EXEC_TYP
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnEXEC_TYP.toLatin1().constData()), eposEXEC_TYP, clStdf.WriteRecord());

    // MIR.EXEC_VER
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnEXEC_VER.toLatin1().constData()), eposEXEC_VER, clStdf.WriteRecord());

    // MIR.TEST_COD
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnTEST_COD.toLatin1().constData()), eposTEST_COD, clStdf.WriteRecord());

    // MIR.TST_TEMP
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnTST_TEMP.toLatin1().constData()), eposTST_TEMP, clStdf.WriteRecord());

    // MIR.USER_TXT
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnUSER_TXT.toLatin1().constData()), eposUSER_TXT, clStdf.WriteRecord());

    // MIR.AUX_FILE
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnAUX_FILE.toLatin1().constData()),eposAUX_FILE , clStdf.WriteRecord());

    // MIR.PKG_TYP
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnPKG_TYP.toLatin1().constData()), eposPKG_TYP, clStdf.WriteRecord());

    // MIR.FAMLY_ID
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnFAMLY_ID.toLatin1().constData()), eposFAMLY_ID, clStdf.WriteRecord());

    // MIR.DATE_COD
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnDATE_COD.toLatin1().constData()), eposDATE_COD, clStdf.WriteRecord());

    // MIR.FACIL_ID
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnFACIL_ID.toLatin1().constData()), eposFACIL_ID, clStdf.WriteRecord());

    // MIR.FLOOR_ID
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnFLOOR_ID.toLatin1().constData()), eposFLOOR_ID, clStdf.WriteRecord());

    // MIR.PROC_ID
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnPROC_ID.toLatin1().constData()), eposPROC_ID, clStdf.WriteRecord());

    // MIR.OPER_FRQ
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnOPER_FRQ.toLatin1().constData()), eposOPER_FRQ, clStdf.WriteRecord());

    // MIR.SPEC_NAM
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnSPEC_NAM.toLatin1().constData()), eposSPEC_NAM, clStdf.WriteRecord());

    // MIR.SPEC_VER
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnSPEC_VER.toLatin1().constData()), eposSPEC_VER, clStdf.WriteRecord());

    // MIR.FLOW_ID
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnFLOW_ID.toLatin1().constData()), eposFLOW_ID, clStdf.WriteRecord());

    // MIR.SETUP_ID
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnSETUP_ID.toLatin1().constData()), eposSETUP_ID, clStdf.WriteRecord());

    // MIR.DSGN_REV
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnDSGN_REV.toLatin1().constData()), eposDSGN_REV, clStdf.WriteRecord());

    // MIR.ENG_ID
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnENG_ID.toLatin1().constData()), eposENG_ID, clStdf.WriteRecord());

    // MIR.ROM_COD
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnROM_COD.toLatin1().constData()), eposROM_COD, clStdf.WriteRecord());

    // MIR.SERL_NUM
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnSERL_NUM.toLatin1().constData()), eposSERL_NUM, clStdf.WriteRecord());

    // MIR.SUPR_NAM
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnSUPR_NAM.toLatin1().constData()), eposSUPR_NAM, clStdf.WriteRecord());

    clStdf.WriteRecord();

    return true;
}

void Stdf_MIR_V4::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    QDateTime	clDateTime;
    time_t		tDateTime;

    // Set QDateTime object to UTC mode
    clDateTime.setTimeSpec(Qt::UTC);

    // Empty string first
    strAsciiString = "";

    // MIR.SETUP_T
    tDateTime = (time_t)m_u4SETUP_T;
    clDateTime.setTime_t(tDateTime);
    // GCORE-859
    m_strFieldValue_macro.sprintf("%s (0x%lx)",
                                  ( GS::StdLib::Stdf::GetDaysOfWeekString(clDateTime.date().dayOfWeek()-1)
                                    + clDateTime.toString(" dd ")
                                    + GS::StdLib::Stdf::GetMonthName(clDateTime.date().month()-1)
                                    + clDateTime.toString(" yyyy h:mm:ss") )
                                  .toLatin1().constData(), (long unsigned int)m_u4SETUP_T);
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.setup_t", m_strFieldValue_macro, nFieldSelection, eposSETUP_T);

    // MIR.START_T
    tDateTime = (time_t)m_u4START_T;
    clDateTime.setTime_t(tDateTime);
    m_strFieldValue_macro.sprintf("%s (0x%lx)",
                                  ( GS::StdLib::Stdf::GetDaysOfWeekString(clDateTime.date().dayOfWeek()-1)
                                    + clDateTime.toString(" dd ")
                                    + GS::StdLib::Stdf::GetMonthName(clDateTime.date().month()-1)
                                    + clDateTime.toString(" yyyy h:mm:ss") )
                                  .toLatin1().constData(), (long unsigned int)m_u4START_T);
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.start_t", m_strFieldValue_macro, nFieldSelection, eposSTART_T);

    // MIR.STAT_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.stat_num", QString::number(m_u1STAT_NUM), nFieldSelection, eposSTAT_NUM);

    // MIR.MODE_COD
    _CREATEFIELD_FROM_C1_ASCII(m_c1MODE_COD);
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.mode_cod", m_strFieldValue_macro, nFieldSelection, eposMODE_COD);

    // MIR.RTST_COD
    _CREATEFIELD_FROM_C1_ASCII(m_c1RTST_COD);
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.rtst_cod", m_strFieldValue_macro, nFieldSelection, eposRTST_COD);

    // MIR.PROT_COD
    _CREATEFIELD_FROM_C1_ASCII(m_c1PROT_COD);
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.prot_cod", m_strFieldValue_macro, nFieldSelection, eposPROT_COD);

    // MIR.BURN_TIM
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.burn_tim", QString::number(m_u2BURN_TIM), nFieldSelection, eposBURN_TIM);

    // MIR.CMOD_COD
    _CREATEFIELD_FROM_C1_ASCII(m_c1CMOD_COD);
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.cmod_cod", m_strFieldValue_macro, nFieldSelection, eposCMOD_COD);

    // MIR.LOT_ID
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.lot_id", m_cnLOT_ID, nFieldSelection, eposLOT_ID);

    // MIR.PART_TYP
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.part_typ", m_cnPART_TYP, nFieldSelection, eposPART_TYP);

    // MIR.NODE_NAM
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.node_nam", m_cnNODE_NAM, nFieldSelection, eposNODE_NAM);

    // MIR.TSTR_TYP
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.tstr_typ", m_cnTSTR_TYP, nFieldSelection, eposTSTR_TYP);

    // MIR.JOB_NAM
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.job_nam", m_cnJOB_NAM, nFieldSelection, eposJOB_NAM);

    // MIR.JOB_REV
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.job_rev", m_cnJOB_REV, nFieldSelection, eposJOB_REV);

    // MIR.SBLOT_ID
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.sblot_id", m_cnSBLOT_ID, nFieldSelection, eposSBLOT_ID);

    // MIR.OPER_NAM
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.oper_nam", m_cnOPER_NAM, nFieldSelection, eposOPER_NAM);

    // MIR.EXEC_TYP
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.exec_typ", m_cnEXEC_TYP, nFieldSelection, eposEXEC_TYP);

    // MIR.EXEC_VER
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.exec_ver", m_cnEXEC_VER, nFieldSelection, eposEXEC_VER);

    // MIR.TEST_COD
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.test_cod", m_cnTEST_COD, nFieldSelection, eposTEST_COD);

    // MIR.TST_TEMP
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.tst_temp", m_cnTST_TEMP, nFieldSelection, eposTST_TEMP);

    // MIR.USER_TXT
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.user_txt", m_cnUSER_TXT, nFieldSelection, eposUSER_TXT);

    // MIR.AUX_FILE
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.aux_file", m_cnAUX_FILE, nFieldSelection, eposAUX_FILE);

    // MIR.PKG_TYP
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.pkg_typ", m_cnPKG_TYP, nFieldSelection, eposPKG_TYP);

    // MIR.FAMLY_ID
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.famly_id", m_cnFAMLY_ID, nFieldSelection, eposFAMLY_ID);

    // MIR.DATE_COD
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.date_cod", m_cnDATE_COD, nFieldSelection, eposDATE_COD);

    // MIR.FACIL_ID
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.facil_id", m_cnFACIL_ID, nFieldSelection, eposFACIL_ID);

    // MIR.FLOOR_ID
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.floor_id", m_cnFLOOR_ID, nFieldSelection, eposFLOOR_ID);

    // MIR.PROC_ID
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.proc_id", m_cnPROC_ID, nFieldSelection, eposPROC_ID);

    // MIR.OPER_FRQ
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.oper_frq", m_cnOPER_FRQ, nFieldSelection, eposOPER_FRQ);

    // MIR.SPEC_NAM
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.spec_nam", m_cnSPEC_NAM, nFieldSelection, eposSPEC_NAM);

    // MIR.SPEC_VER
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.spec_ver", m_cnSPEC_VER, nFieldSelection, eposSPEC_VER);

    // MIR.FLOW_ID
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.flow_id", m_cnFLOW_ID, nFieldSelection, eposFLOW_ID);

    // MIR.SETUP_ID
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.setup_id", m_cnSETUP_ID, nFieldSelection, eposSETUP_ID);

    // MIR.DSGN_REV
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.dsgn_rev", m_cnDSGN_REV, nFieldSelection, eposDSGN_REV);

    // MIR.ENG_ID
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.eng_id", m_cnENG_ID, nFieldSelection, eposENG_ID);

    // MIR.ROM_COD
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.rom_cod", m_cnROM_COD, nFieldSelection, eposROM_COD);

    // MIR.SERL_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.serl_num", m_cnSERL_NUM, nFieldSelection, eposSERL_NUM);

    // MIR.SUPR_NAM
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.supr_nam", m_cnSUPR_NAM, nFieldSelection, eposSUPR_NAM);
}

void Stdf_MIR_V4::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
    QDateTime	clDateTime;
    time_t		tDateTime;

    // Set QDateTime object to UTC mode
    clDateTime.setTimeSpec(Qt::UTC);

    // Empty string list first
    listFields.empty();

    // MIR.SETUP_T
    tDateTime = (time_t)m_u4SETUP_T;
    clDateTime.setTime_t(tDateTime);
    m_strFieldValue_macro.sprintf("%s (0x%lx)",
                                  ( GS::StdLib::Stdf::GetDaysOfWeekString(clDateTime.date().dayOfWeek()-1)
                                     + clDateTime.toString(" dd ")
                                     + GS::StdLib::Stdf::GetMonthName(clDateTime.date().month()-1)
                                     + clDateTime.toString(" yyyy h:mm:ss") )
                                  .toLatin1().constData(), (long unsigned int)m_u4SETUP_T);
    _LIST_ADDFIELD_ASCII(listFields, "MIR.setup_t", m_strFieldValue_macro, nFieldSelection, eposSETUP_T);

    // MIR.START_T
    tDateTime = (time_t)m_u4START_T;
    clDateTime.setTime_t(tDateTime);
    m_strFieldValue_macro.sprintf("%s (0x%lx)", ( GS::StdLib::Stdf::GetDaysOfWeekString(clDateTime.date().dayOfWeek()-1)
                                                  + clDateTime.toString(" dd MMM yyyy h:mm:ss") )
                                  .toLatin1().constData(), (long unsigned int)m_u4START_T);
    _LIST_ADDFIELD_ASCII(listFields, "MIR.start_t", m_strFieldValue_macro, nFieldSelection, eposSTART_T);

    // MIR.STAT_NUM
    _LIST_ADDFIELD_ASCII(listFields, "MIR.stat_num", QString::number(m_u1STAT_NUM), nFieldSelection, eposSTAT_NUM);

    // MIR.MODE_COD
    _CREATEFIELD_FROM_C1_ASCII(m_c1MODE_COD);
    _LIST_ADDFIELD_ASCII(listFields, "MIR.mode_cod", m_strFieldValue_macro, nFieldSelection, eposMODE_COD);

    // MIR.RTST_COD
    _CREATEFIELD_FROM_C1_ASCII(m_c1RTST_COD);
    _LIST_ADDFIELD_ASCII(listFields, "MIR.rtst_cod", m_strFieldValue_macro, nFieldSelection, eposRTST_COD);

    // MIR.PROT_COD
    _CREATEFIELD_FROM_C1_ASCII(m_c1PROT_COD);
    _LIST_ADDFIELD_ASCII(listFields, "MIR.prot_cod", m_strFieldValue_macro, nFieldSelection, eposPROT_COD);

    // MIR.BURN_TIM
    _LIST_ADDFIELD_ASCII(listFields, "MIR.burn_tim", QString::number(m_u2BURN_TIM), nFieldSelection, eposBURN_TIM);

    // MIR.CMOD_COD
    _CREATEFIELD_FROM_C1_ASCII(m_c1CMOD_COD);
    _LIST_ADDFIELD_ASCII(listFields, "MIR.cmod_cod", m_strFieldValue_macro, nFieldSelection, eposCMOD_COD);

    // MIR.LOT_ID
    _LIST_ADDFIELD_ASCII(listFields, "MIR.lot_id", m_cnLOT_ID, nFieldSelection, eposLOT_ID);

    // MIR.PART_TYP
    _LIST_ADDFIELD_ASCII(listFields, "MIR.part_typ", m_cnPART_TYP, nFieldSelection, eposPART_TYP);

    // MIR.NODE_NAM
    _LIST_ADDFIELD_ASCII(listFields, "MIR.node_nam", m_cnNODE_NAM, nFieldSelection, eposNODE_NAM);

    // MIR.TSTR_TYP
    _LIST_ADDFIELD_ASCII(listFields, "MIR.tstr_typ", m_cnTSTR_TYP, nFieldSelection, eposTSTR_TYP);

    // MIR.JOB_NAM
    _LIST_ADDFIELD_ASCII(listFields, "MIR.job_nam", m_cnJOB_NAM, nFieldSelection, eposJOB_NAM);

    // MIR.JOB_REV
    _LIST_ADDFIELD_ASCII(listFields, "MIR.job_rev", m_cnJOB_REV, nFieldSelection, eposJOB_REV);

    // MIR.SBLOT_ID
    _LIST_ADDFIELD_ASCII(listFields, "MIR.sblot_id", m_cnSBLOT_ID, nFieldSelection, eposSBLOT_ID);

    // MIR.OPER_NAM
    _LIST_ADDFIELD_ASCII(listFields, "MIR.oper_nam", m_cnOPER_NAM, nFieldSelection, eposOPER_NAM);

    // MIR.EXEC_TYP
    _LIST_ADDFIELD_ASCII(listFields, "MIR.exec_typ", m_cnEXEC_TYP, nFieldSelection, eposEXEC_TYP);

    // MIR.EXEC_VER
    _LIST_ADDFIELD_ASCII(listFields, "MIR.exec_ver", m_cnEXEC_VER, nFieldSelection, eposEXEC_VER);

    // MIR.TEST_COD
    _LIST_ADDFIELD_ASCII(listFields, "MIR.test_cod", m_cnTEST_COD, nFieldSelection, eposTEST_COD);

    // MIR.TST_TEMP
    _LIST_ADDFIELD_ASCII(listFields, "MIR.tst_temp", m_cnTST_TEMP, nFieldSelection, eposTST_TEMP);

    // MIR.USER_TXT
    _LIST_ADDFIELD_ASCII(listFields, "MIR.user_txt", m_cnUSER_TXT, nFieldSelection, eposUSER_TXT);

    // MIR.AUX_FILE
    _LIST_ADDFIELD_ASCII(listFields, "MIR.aux_file", m_cnAUX_FILE, nFieldSelection, eposAUX_FILE);

    // MIR.PKG_TYP
    _LIST_ADDFIELD_ASCII(listFields, "MIR.pkg_typ", m_cnPKG_TYP, nFieldSelection, eposPKG_TYP);

    // MIR.FAMLY_ID
    _LIST_ADDFIELD_ASCII(listFields, "MIR.famly_id", m_cnFAMLY_ID, nFieldSelection, eposFAMLY_ID);

    // MIR.DATE_COD
    _LIST_ADDFIELD_ASCII(listFields, "MIR.date_cod", m_cnDATE_COD, nFieldSelection, eposDATE_COD);

    // MIR.FACIL_ID
    _LIST_ADDFIELD_ASCII(listFields, "MIR.facil_id", m_cnFACIL_ID, nFieldSelection, eposFACIL_ID);

    // MIR.FLOOR_ID
    _LIST_ADDFIELD_ASCII(listFields, "MIR.floor_id", m_cnFLOOR_ID, nFieldSelection, eposFLOOR_ID);

    // MIR.PROC_ID
    _LIST_ADDFIELD_ASCII(listFields, "MIR.proc_id", m_cnPROC_ID, nFieldSelection, eposPROC_ID);

    // MIR.OPER_FRQ
    _LIST_ADDFIELD_ASCII(listFields, "MIR.oper_frq", m_cnOPER_FRQ, nFieldSelection, eposOPER_FRQ);

    // MIR.SPEC_NAM
    _LIST_ADDFIELD_ASCII(listFields, "MIR.spec_nam", m_cnSPEC_NAM, nFieldSelection, eposSPEC_NAM);

    // MIR.SPEC_VER
    _LIST_ADDFIELD_ASCII(listFields, "MIR.spec_ver", m_cnSPEC_VER, nFieldSelection, eposSPEC_VER);

    // MIR.FLOW_ID
    _LIST_ADDFIELD_ASCII(listFields, "MIR.flow_id", m_cnFLOW_ID, nFieldSelection, eposFLOW_ID);

    // MIR.SETUP_ID
    _LIST_ADDFIELD_ASCII(listFields, "MIR.setup_id", m_cnSETUP_ID, nFieldSelection, eposSETUP_ID);

    // MIR.DSGN_REV
    _LIST_ADDFIELD_ASCII(listFields, "MIR.dsgn_rev", m_cnDSGN_REV, nFieldSelection, eposDSGN_REV);

    // MIR.ENG_ID
    _LIST_ADDFIELD_ASCII(listFields, "MIR.eng_id", m_cnENG_ID, nFieldSelection, eposENG_ID);

    // MIR.ROM_COD
    _LIST_ADDFIELD_ASCII(listFields, "MIR.rom_cod", m_cnROM_COD, nFieldSelection, eposROM_COD);

    // MIR.SERL_NUM
    _LIST_ADDFIELD_ASCII(listFields, "MIR.serl_num", m_cnSERL_NUM, nFieldSelection, eposSERL_NUM);

    // MIR.SUPR_NAM
    _LIST_ADDFIELD_ASCII(listFields, "MIR.supr_nam", m_cnSUPR_NAM, nFieldSelection, eposSUPR_NAM);
}

void Stdf_MIR_V4::GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection /*= 0*/)
{
    QString strTabs="";

    // Init XML string
    for(int nIndex=nIndentationLevel; nIndex>0; nIndex--)
        strTabs += "\t";
    strXmlString = strTabs;
    strXmlString += "<mir>\n";

    // MIR.SETUP_T
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "setup_t", QString::number(m_u4SETUP_T), nFieldSelection, eposSETUP_T);

    // MIR.START_T
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "start_t", QString::number(m_u4START_T), nFieldSelection, eposSTART_T);

    // MIR.STAT_NUM
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "stat_num", QString::number(m_u1STAT_NUM), nFieldSelection, eposSTAT_NUM);

    // MIR.MODE_COD
    _CREATEFIELD_FROM_C1_XML(m_c1MODE_COD);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "mode_cod", m_strFieldValue_macro, nFieldSelection, eposMODE_COD);

    // MIR.RTST_COD
    _CREATEFIELD_FROM_C1_XML(m_c1RTST_COD);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "rtst_cod", m_strFieldValue_macro, nFieldSelection, eposRTST_COD);

    // MIR.PROT_COD
    _CREATEFIELD_FROM_C1_XML(m_c1PROT_COD);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "prot_cod", m_strFieldValue_macro, nFieldSelection, eposPROT_COD);

    // MIR.BURN_TIM
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "burn_tim", QString::number(m_u2BURN_TIM), nFieldSelection, eposBURN_TIM);

    // MIR.CMOD_COD
    _CREATEFIELD_FROM_C1_XML(m_c1CMOD_COD);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "cmod_cod", m_strFieldValue_macro, nFieldSelection, eposCMOD_COD);

    // MIR.LOT_ID
    _CREATEFIELD_FROM_CN_XML(m_cnLOT_ID);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "lot_id", m_strFieldValue_macro, nFieldSelection, eposLOT_ID);

    // MIR.PART_TYP
    _CREATEFIELD_FROM_CN_XML(m_cnPART_TYP);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "part_typ", m_strFieldValue_macro, nFieldSelection, eposPART_TYP);

    // MIR.NODE_NAM
    _CREATEFIELD_FROM_CN_XML(m_cnNODE_NAM);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "node_nam", m_strFieldValue_macro, nFieldSelection, eposNODE_NAM);

    // MIR.TSTR_TYP
    _CREATEFIELD_FROM_CN_XML(m_cnTSTR_TYP);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "tstr_typ", m_strFieldValue_macro, nFieldSelection, eposTSTR_TYP);

    // MIR.JOB_NAM
    _CREATEFIELD_FROM_CN_XML(m_cnJOB_NAM);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "job_nam", m_strFieldValue_macro, nFieldSelection, eposJOB_NAM);

    // MIR.JOB_REV
    _CREATEFIELD_FROM_CN_XML(m_cnJOB_REV);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "job_rev", m_strFieldValue_macro, nFieldSelection, eposJOB_REV);

    // MIR.SBLOT_ID
    _CREATEFIELD_FROM_CN_XML(m_cnSBLOT_ID);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "sblot_id", m_strFieldValue_macro, nFieldSelection, eposSBLOT_ID);

    // MIR.OPER_NAM
    _CREATEFIELD_FROM_CN_XML(m_cnOPER_NAM);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "oper_nam", m_strFieldValue_macro, nFieldSelection, eposOPER_NAM);

    // MIR.EXEC_TYP
    _CREATEFIELD_FROM_CN_XML(m_cnEXEC_TYP);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "exec_typ", m_strFieldValue_macro, nFieldSelection, eposEXEC_TYP);

    // MIR.EXEC_VER
    _CREATEFIELD_FROM_CN_XML(m_cnEXEC_VER);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "exec_ver", m_strFieldValue_macro, nFieldSelection, eposEXEC_VER);

    // MIR.TEST_COD
    _CREATEFIELD_FROM_CN_XML(m_cnTEST_COD);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "test_cod", m_strFieldValue_macro, nFieldSelection, eposTEST_COD);

    // MIR.TST_TEMP
    _CREATEFIELD_FROM_CN_XML(m_cnTST_TEMP);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "tst_temp", m_strFieldValue_macro, nFieldSelection, eposTST_TEMP);

    // MIR.USER_TXT
    _CREATEFIELD_FROM_CN_XML(m_cnUSER_TXT);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "user_txt", m_strFieldValue_macro, nFieldSelection, eposUSER_TXT);

    // MIR.AUX_FILE
    _CREATEFIELD_FROM_CN_XML(m_cnAUX_FILE);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "aux_file", m_strFieldValue_macro, nFieldSelection, eposAUX_FILE);

    // MIR.PKG_TYP
    _CREATEFIELD_FROM_CN_XML(m_cnPKG_TYP);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "pkg_typ", m_strFieldValue_macro, nFieldSelection, eposPKG_TYP);

    // MIR.FAMLY_ID
    _CREATEFIELD_FROM_CN_XML(m_cnFAMLY_ID);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "famly_id", m_strFieldValue_macro, nFieldSelection, eposFAMLY_ID);

    // MIR.DATE_COD
    _CREATEFIELD_FROM_CN_XML(m_cnDATE_COD);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "date_cod", m_strFieldValue_macro, nFieldSelection, eposDATE_COD);

    // MIR.FACIL_ID
    _CREATEFIELD_FROM_CN_XML(m_cnFACIL_ID);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "facil_id", m_strFieldValue_macro, nFieldSelection, eposFACIL_ID);

    // MIR.FLOOR_ID
    _CREATEFIELD_FROM_CN_XML(m_cnFLOOR_ID);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "floor_id", m_strFieldValue_macro, nFieldSelection, eposFLOOR_ID);

    // MIR.PROC_ID
    _CREATEFIELD_FROM_CN_XML(m_cnPROC_ID);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "proc_id", m_strFieldValue_macro, nFieldSelection, eposPROC_ID);

    // MIR.OPER_FRQ
    _CREATEFIELD_FROM_CN_XML(m_cnOPER_FRQ);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "oper_frq", m_strFieldValue_macro, nFieldSelection, eposOPER_FRQ);

    // MIR.SPEC_NAM
    _CREATEFIELD_FROM_CN_XML(m_cnSPEC_NAM);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "spec_nam", m_strFieldValue_macro, nFieldSelection, eposSPEC_NAM);

    // MIR.SPEC_VER
    _CREATEFIELD_FROM_CN_XML(m_cnSPEC_VER);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "spec_ver", m_strFieldValue_macro, nFieldSelection, eposSPEC_VER);

    // MIR.FLOW_ID
    _CREATEFIELD_FROM_CN_XML(m_cnFLOW_ID);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "flow_id", m_strFieldValue_macro, nFieldSelection, eposFLOW_ID);

    // MIR.SETUP_ID
    _CREATEFIELD_FROM_CN_XML(m_cnSETUP_ID);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "setup_id", m_strFieldValue_macro, nFieldSelection, eposSETUP_ID);

    // MIR.DSGN_REV
    _CREATEFIELD_FROM_CN_XML(m_cnDSGN_REV);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "dsgn_rev", m_strFieldValue_macro, nFieldSelection, eposDSGN_REV);

    // MIR.ENG_ID
    _CREATEFIELD_FROM_CN_XML(m_cnENG_ID);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "eng_id", m_strFieldValue_macro, nFieldSelection, eposENG_ID);

    // MIR.ROM_COD
    _CREATEFIELD_FROM_CN_XML(m_cnROM_COD);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "rom_cod", m_strFieldValue_macro, nFieldSelection, eposROM_COD);

    // MIR.SERL_NUM
    _CREATEFIELD_FROM_CN_XML(m_cnSERL_NUM);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "serl_num", m_strFieldValue_macro, nFieldSelection, eposSERL_NUM);

    // MIR.SUPR_NAM
    _CREATEFIELD_FROM_CN_XML(m_cnSUPR_NAM);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "supr_nam", m_strFieldValue_macro, nFieldSelection, eposSUPR_NAM);

    strXmlString += strTabs;
    strXmlString += "</mir>\n";
}

void Stdf_MIR_V4::GetAtdfString(QString & strAtdfString)
{
    QDateTime	clDateTime;
    time_t		tDateTime;

    // Set QDateTime object to UTC mode
    clDateTime.setTimeSpec(Qt::UTC);

    // Reset field index, and line Nb
    m_uiFieldIndex = 0;
    m_uiLineNb = 1;

    // Init string
    strAtdfString = "MIR:";

    // MIR.LOT_ID
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnLOT_ID, eposLOT_ID);

    // MIR.PART_TYP
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnPART_TYP, eposPART_TYP);

    // MIR.JOB_NAM
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnJOB_NAM, eposJOB_NAM);

    // MIR.NODE_NAM
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnNODE_NAM, eposNODE_NAM);

    // MIR.TSTR_TYP
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnTSTR_TYP, eposTSTR_TYP);

    // MIR.SETUP_T
    tDateTime = (time_t)m_u4SETUP_T;
    clDateTime.setTime_t(tDateTime);
    QString strDate = clDateTime.toString("hh:mm:ss dd-MM-yyyy");
    strDate = strDate.replace("-01-","-Jan-").replace("-02-","-Feb-").replace("-03-","-Mar-").replace("-04-","-Apr-").replace("-05-","-May-").replace("-06-","-Jun-").replace("-07-","-Jul-").replace("-08-","-Aug-").replace("-09-","-Sep-").replace("-10-","-Oct-").replace("-11-","-Nov-").replace("-12-","-Dec-");
    m_strFieldValue_macro.sprintf("%s", strDate.toLatin1().constData());
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposSETUP_T);

    // MIR.START_T
    tDateTime = (time_t)m_u4START_T;
    clDateTime.setTime_t(tDateTime);
    strDate = clDateTime.toString("hh:mm:ss dd-MM-yyyy");
    strDate = strDate.replace("-01-","-Jan-").replace("-02-","-Feb-").replace("-03-","-Mar-").replace("-04-","-Apr-").replace("-05-","-May-").replace("-06-","-Jun-").replace("-07-","-Jul-").replace("-08-","-Aug-").replace("-09-","-Sep-").replace("-10-","-Oct-").replace("-11-","-Nov-").replace("-12-","-Dec-");
    m_strFieldValue_macro.sprintf("%s", strDate.toLatin1().constData());
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposSTART_T);

    // MIR.OPER_NAM
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnOPER_NAM, eposOPER_NAM);

    // MIR.MODE_COD
    _CREATEFIELD_FROM_C1_ASCII(m_c1MODE_COD);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposMODE_COD);

    // MIR.STAT_NUM
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u1STAT_NUM), eposSTAT_NUM);

    // MIR.SBLOT_ID
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnSBLOT_ID, eposSBLOT_ID);

    // MIR.TEST_COD
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnTEST_COD, eposTEST_COD);

    // MIR.RTST_COD
    _CREATEFIELD_FROM_C1_ASCII(m_c1RTST_COD);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposRTST_COD);

    // MIR.JOB_REV
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnJOB_REV, eposJOB_REV);

    // MIR.EXEC_TYP
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnEXEC_TYP, eposEXEC_TYP);

    // MIR.EXEC_VER
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnEXEC_VER, eposEXEC_VER);

    // MIR.PROT_COD
    _CREATEFIELD_FROM_C1_ASCII(m_c1PROT_COD);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposPROT_COD);

    // MIR.CMOD_COD
    _CREATEFIELD_FROM_C1_ASCII(m_c1CMOD_COD);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposCMOD_COD);

    // MIR.BURN_TIM
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u2BURN_TIM), eposBURN_TIM);

    // MIR.TST_TEMP
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnTST_TEMP, eposTST_TEMP);

    // MIR.USER_TXT
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnUSER_TXT, eposUSER_TXT);

    // MIR.AUX_FILE
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnAUX_FILE, eposAUX_FILE);

    // MIR.PKG_TYP
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnPKG_TYP, eposPKG_TYP);

    // MIR.FAMLY_ID
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnFAMLY_ID, eposFAMLY_ID);

    // MIR.DATE_COD
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnDATE_COD, eposDATE_COD);

    // MIR.FACIL_ID
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnFACIL_ID, eposFACIL_ID);

    // MIR.FLOOR_ID
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnFLOOR_ID, eposFLOOR_ID);

    // MIR.PROC_ID
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnPROC_ID, eposPROC_ID);

    // MIR.OPER_FRQ
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnOPER_FRQ, eposOPER_FRQ);

    // MIR.SPEC_NAM
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnSPEC_NAM, eposSPEC_NAM);

    // MIR.SPEC_VER
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnSPEC_VER, eposSPEC_VER);

    // MIR.FLOW_ID
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnFLOW_ID, eposFLOW_ID);

    // MIR.SETUP_ID
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnSETUP_ID, eposSETUP_ID);

    // MIR.DSGN_REV
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnDSGN_REV, eposDSGN_REV);

    // MIR.ENG_ID
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnENG_ID, eposENG_ID);

    // MIR.ROM_COD
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnROM_COD, eposROM_COD);

    // MIR.SERL_NUM
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnSERL_NUM, eposSERL_NUM);

    // MIR.SUPR_NAM
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnSUPR_NAM, eposSUPR_NAM);

    _STR_FINISHFIELD_ATDF(strAtdfString);
}

///////////////////////////////////////////////////////////
// MRR RECORD
///////////////////////////////////////////////////////////
Stdf_MRR_V4::Stdf_MRR_V4() : Stdf_Record()
{
    Reset();
}

Stdf_MRR_V4::~Stdf_MRR_V4()
{
    Reset();
}

void Stdf_MRR_V4::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Select fields for reduced list
    m_pFieldFlags[eposFINISH_T]	|= FieldFlag_ReducedList;

    // Reset Data
    m_u4FINISH_T		= 0;		// MRR.FINISH_T
    m_c1DISP_COD		= 0;		// MRR.DISP_COD
    m_cnUSR_DESC		= "";		// MRR.USR_DESC
    m_cnEXC_DESC		= "";		// MRR.EXC_DESC

    // Call Reset base method
    Stdf_Record::Reset();
}

bool Stdf_MRR_V4::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;

    return false;
}

QString Stdf_MRR_V4::GetRecordShortName(void)
{
    return "MRR";
}

QString Stdf_MRR_V4::GetRecordLongName(void)
{
    return "Master Results Record";
}

int Stdf_MRR_V4::GetRecordType(void)
{
    return Rec_MRR;
}

bool Stdf_MRR_V4::Read(GS::StdLib::Stdf & clStdf)
{
    char	szString[STDF_MAX_U1+1];
    long	dwData;
    BYTE	bData;

    // First reset data
    Reset();

    // MRR.FINISH_T
    _FIELD_CHECKREAD(clStdf.ReadDword(&dwData), eposFINISH_T);
    _FIELD_SET(m_u4FINISH_T = stdf_type_u4(dwData), true, eposFINISH_T);

    // MRR.DISP_COD
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposDISP_COD);
    _FIELD_SET(m_c1DISP_COD = stdf_type_c1(bData), m_c1DISP_COD != ' ', eposDISP_COD);

    // MRR.USR_DESC
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposUSR_DESC);
    _FIELD_SET(m_cnUSR_DESC = szString, !m_cnUSR_DESC.isEmpty(), eposUSR_DESC);

    // MRR.EXC_DESC
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposEXC_DESC);
    _FIELD_SET(m_cnEXC_DESC = szString, !m_cnEXC_DESC.isEmpty(), eposEXC_DESC);

    return true;
}

bool Stdf_MRR_V4::Write(GS::StdLib::Stdf & clStdf)
{
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;

    RecordReadInfo.iRecordType = STDF_MRR_TYPE;
    RecordReadInfo.iRecordSubType = STDF_MRR_STYPE;
    clStdf.WriteHeader(&RecordReadInfo);

    // MRR.FINISH_T
    _FIELD_CHECKWRITE(clStdf.WriteDword(DWORD(m_u4FINISH_T)), eposFINISH_T, clStdf.WriteRecord());

    // MRR.DISP_COD
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_c1DISP_COD)), eposDISP_COD, clStdf.WriteRecord());

    // MRR.USR_DESC
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnUSR_DESC.toLatin1().constData()), eposUSR_DESC, clStdf.WriteRecord());

    // MRR.EXC_DESC
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnEXC_DESC.toLatin1().constData()), eposEXC_DESC, clStdf.WriteRecord());

    clStdf.WriteRecord();

    return true;
}

void Stdf_MRR_V4::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    QDateTime	clDateTime;
    time_t		tDateTime;

    // Set QDateTime object to UTC mode
    clDateTime.setTimeSpec(Qt::UTC);

    // Empty string first
    strAsciiString = "";

    // MRR.FINISH_T
    tDateTime = (time_t)m_u4FINISH_T;
    clDateTime.setTime_t(tDateTime);
    m_strFieldValue_macro.sprintf("%s (0x%lx)",
                                  ( GS::StdLib::Stdf::GetDaysOfWeekString(clDateTime.date().dayOfWeek()-1)
                                    + clDateTime.toString(" dd ")
                                    + GS::StdLib::Stdf::GetMonthName(clDateTime.date().month()-1)
                                    + clDateTime.toString(" yyyy h:mm:ss") )
                                  .toLatin1().constData(),
                                  (long unsigned int)m_u4FINISH_T);
    _STR_ADDFIELD_ASCII(strAsciiString, "MRR.finish_t", m_strFieldValue_macro, nFieldSelection, eposFINISH_T);

    // MRR.DISP_COD
    _CREATEFIELD_FROM_C1_ASCII(m_c1DISP_COD);
    _STR_ADDFIELD_ASCII(strAsciiString, "MRR.disp_cod", m_strFieldValue_macro, nFieldSelection, eposDISP_COD);

    // MRR.USR_DESC
    _STR_ADDFIELD_ASCII(strAsciiString, "MRR.usr_desc", m_cnUSR_DESC, nFieldSelection, eposUSR_DESC);

    // MRR.EXC_DESC
    _STR_ADDFIELD_ASCII(strAsciiString, "MRR.exc_desc", m_cnEXC_DESC, nFieldSelection, eposEXC_DESC);
}

void Stdf_MRR_V4::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
    QDateTime	clDateTime;
    time_t		tDateTime;

    // Set QDateTime object to UTC mode
    clDateTime.setTimeSpec(Qt::UTC);

    // Empty string list first
    listFields.empty();

    // MRR.FINISH_T
    tDateTime = (time_t)m_u4FINISH_T;
    clDateTime.setTime_t(tDateTime);
    m_strFieldValue_macro.sprintf("%s (0x%lx)",
                                  ( GS::StdLib::Stdf::GetDaysOfWeekString(clDateTime.date().dayOfWeek()-1)
                                    + clDateTime.toString(" dd ")
                                    + GS::StdLib::Stdf::GetMonthName(clDateTime.date().month()-1)
                                    + clDateTime.toString(" yyyy h:mm:ss") )
                                  .toLatin1().constData(), (long unsigned int)m_u4FINISH_T);
    _LIST_ADDFIELD_ASCII(listFields, "MRR.finish_t", m_strFieldValue_macro, nFieldSelection, eposFINISH_T);

    // MRR.DISP_COD
    _CREATEFIELD_FROM_C1_ASCII(m_c1DISP_COD);
    _LIST_ADDFIELD_ASCII(listFields, "MRR.disp_cod", m_strFieldValue_macro, nFieldSelection, eposDISP_COD);

    // MRR.USR_DESC
    _LIST_ADDFIELD_ASCII(listFields, "MRR.usr_desc", m_cnUSR_DESC, nFieldSelection, eposUSR_DESC);

    // MRR.EXC_DESC
    _LIST_ADDFIELD_ASCII(listFields, "MRR.exc_desc", m_cnEXC_DESC, nFieldSelection, eposEXC_DESC);
}

void Stdf_MRR_V4::GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection /*= 0*/)
{
    QString strTabs="";

    // Init XML string
    for(int nIndex=nIndentationLevel; nIndex>0; nIndex--)
        strTabs += "\t";
    strXmlString = strTabs;
    strXmlString += "<mrr>\n";

    // MRR.FINISH_T
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "finish_t", QString::number(m_u4FINISH_T), nFieldSelection, eposFINISH_T);

    // MRR.DISP_COD
    _CREATEFIELD_FROM_CN_XML(m_c1DISP_COD);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "disp_cod", m_strFieldValue_macro, nFieldSelection, eposDISP_COD);

    // MRR.USR_DESC
    _CREATEFIELD_FROM_CN_XML(m_cnUSR_DESC);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "usr_desc", m_strFieldValue_macro, nFieldSelection, eposUSR_DESC);

    // MRR.EXC_DESC
    _CREATEFIELD_FROM_CN_XML(m_cnEXC_DESC);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "exc_desc", m_strFieldValue_macro, nFieldSelection, eposEXC_DESC);

    strXmlString += strTabs;
    strXmlString += "</mrr>\n";
}

void Stdf_MRR_V4::GetAtdfString(QString & strAtdfString)
{
    QDateTime	clDateTime;
    time_t		tDateTime;

    // Set QDateTime object to UTC mode
    clDateTime.setTimeSpec(Qt::UTC);

    // Reset field index, and line Nb
    m_uiFieldIndex = 0;
    m_uiLineNb = 1;

    // Init string
    strAtdfString = "MRR:";

    // MRR.FINISH_T
    tDateTime = (time_t)m_u4FINISH_T;
    clDateTime.setTime_t(tDateTime);
    QString strDate = clDateTime.toString("hh:mm:ss dd-MM-yyyy");
    strDate = strDate.replace("-01-","-Jan-").replace("-02-","-Feb-").replace("-03-","-Mar-").replace("-04-","-Apr-").replace("-05-","-May-").replace("-06-","-Jun-").replace("-07-","-Jul-").replace("-08-","-Aug-").replace("-09-","-Sep-").replace("-10-","-Oct-").replace("-11-","-Nov-").replace("-12-","-Dec-");
    m_strFieldValue_macro.sprintf("%s", strDate.toLatin1().constData());
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposFINISH_T);

    // MRR.DISP_COD
    _CREATEFIELD_FROM_C1_ASCII(m_c1DISP_COD);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposDISP_COD);

    // MRR.USR_DESC
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnUSR_DESC, eposUSR_DESC);

    // MRR.EXC_DESC
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnEXC_DESC, eposEXC_DESC);

    _STR_FINISHFIELD_ATDF(strAtdfString);
}

///////////////////////////////////////////////////////////
// PCR RECORD
///////////////////////////////////////////////////////////
Stdf_PCR_V4::Stdf_PCR_V4() : Stdf_Record()
{
    Reset();
}

Stdf_PCR_V4::~Stdf_PCR_V4()
{
    Reset();
}

void Stdf_PCR_V4::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Select fields for reduced list

    // Reset Data
    m_u1HEAD_NUM = 255;
    m_u1SITE_NUM = 1;
    m_u4PART_CNT = 0;
    m_u4RTST_CNT = m_u4ABRT_CNT = m_u4GOOD_CNT = m_u4FUNC_CNT = 0xFFFFFFFF;

    // Call Reset base method
    Stdf_Record::Reset();
}

bool Stdf_PCR_V4::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;

    return false;
}

QString Stdf_PCR_V4::GetRecordShortName(void)
{
    return "PCR";
}

QString Stdf_PCR_V4::GetRecordLongName(void)
{
    return "Part Count Record";
}

int Stdf_PCR_V4::GetRecordType(void)
{
    return Rec_PCR;
}

bool Stdf_PCR_V4::Read(GS::StdLib::Stdf & clStdf)
{
    long	dwData;
    BYTE	bData;

    // First reset data
    Reset();

    // PCR.HEAD_NUM
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposHEAD_NUM);
    _FIELD_SET(m_u1HEAD_NUM = stdf_type_u1(bData), true, eposHEAD_NUM);

    // PCR.SITE_NUM
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposSITE_NUM);
    _FIELD_SET(m_u1SITE_NUM = stdf_type_u1(bData), true, eposSITE_NUM);

    // PCR.PART_CNT
    _FIELD_CHECKREAD(clStdf.ReadDword(&dwData), eposPART_CNT);
    _FIELD_SET(m_u4PART_CNT = stdf_type_u4(dwData), true, eposPART_CNT);

    // PCR.RTST_CNT
    _FIELD_CHECKREAD(clStdf.ReadDword(&dwData), eposRTST_CNT);
    _FIELD_SET(m_u4RTST_CNT = stdf_type_u4(dwData), m_u4RTST_CNT != 4294967295UL, eposRTST_CNT);

    // PCR.ABRT_CNT
    _FIELD_CHECKREAD(clStdf.ReadDword(&dwData), eposABRT_CNT);
    _FIELD_SET(m_u4ABRT_CNT = stdf_type_u4(dwData), m_u4ABRT_CNT != 4294967295UL, eposABRT_CNT);

    // PCR.GOOD_CNT
    _FIELD_CHECKREAD(clStdf.ReadDword(&dwData), eposGOOD_CNT);
    _FIELD_SET(m_u4GOOD_CNT = stdf_type_u4(dwData), m_u4GOOD_CNT != 4294967295UL, eposGOOD_CNT);

    // PCR.FUNC_CNT
    _FIELD_CHECKREAD(clStdf.ReadDword(&dwData), eposFUNC_CNT);
    _FIELD_SET(m_u4FUNC_CNT = stdf_type_u4(dwData), m_u4FUNC_CNT != 4294967295UL, eposFUNC_CNT);

    return true;
}

bool Stdf_PCR_V4::Write(GS::StdLib::Stdf & clStdf)
{
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;

    RecordReadInfo.iRecordType = STDF_PCR_TYPE;
    RecordReadInfo.iRecordSubType = STDF_PCR_STYPE;
    clStdf.WriteHeader(&RecordReadInfo);

    // PCR.HEAD_NUM
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_u1HEAD_NUM)), eposHEAD_NUM, clStdf.WriteRecord());

    // PCR.SITE_NUM
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_u1SITE_NUM)), eposSITE_NUM, clStdf.WriteRecord());

    // PCR.PART_CNT
    _FIELD_CHECKWRITE(clStdf.WriteDword(DWORD(m_u4PART_CNT)), eposPART_CNT, clStdf.WriteRecord());

    // PCR.RTST_CNT
    _FIELD_CHECKWRITE(clStdf.WriteDword(DWORD(m_u4RTST_CNT)), eposRTST_CNT, clStdf.WriteRecord());

    // PCR.ABRT_CNT
    _FIELD_CHECKWRITE(clStdf.WriteDword(DWORD(m_u4ABRT_CNT)), eposABRT_CNT, clStdf.WriteRecord());

    // PCR.GOOD_CNT
    _FIELD_CHECKWRITE(clStdf.WriteDword(DWORD(m_u4GOOD_CNT)), eposGOOD_CNT, clStdf.WriteRecord());

    // PCR.FUNC_CNT
    _FIELD_CHECKWRITE(clStdf.WriteDword(DWORD(m_u4FUNC_CNT)), eposFUNC_CNT, clStdf.WriteRecord());

    clStdf.WriteRecord();

    return true;
}

void Stdf_PCR_V4::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    // Empty string first
    strAsciiString = "";

    // PCR.HEAD_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "PCR.head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // PCR.SITE_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "PCR.site_num", QString::number(m_u1SITE_NUM), nFieldSelection, eposSITE_NUM);

    // PCR.PART_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "PCR.part_cnt", QString::number(m_u4PART_CNT), nFieldSelection, eposPART_CNT);

    // PCR.RTST_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "PCR.rtst_cnt", QString::number(m_u4RTST_CNT), nFieldSelection, eposRTST_CNT);

    // PCR.ABRT_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "PCR.abrt_cnt", QString::number(m_u4ABRT_CNT), nFieldSelection, eposABRT_CNT);

    // PCR.GOOD_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "PCR.good_cnt", QString::number(m_u4GOOD_CNT), nFieldSelection, eposGOOD_CNT);

    // PCR.FUNC_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "PCR.func_cnt", QString::number(m_u4FUNC_CNT), nFieldSelection, eposFUNC_CNT);
}

void Stdf_PCR_V4::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
    // Empty string list first
    listFields.empty();

    // PCR.HEAD_NUM
    _LIST_ADDFIELD_ASCII(listFields, "PCR.head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // PCR.SITE_NUM
    _LIST_ADDFIELD_ASCII(listFields, "PCR.site_num", QString::number(m_u1SITE_NUM), nFieldSelection, eposSITE_NUM);

    // PCR.PART_CNT
    _LIST_ADDFIELD_ASCII(listFields, "PCR.part_cnt", QString::number(m_u4PART_CNT), nFieldSelection, eposPART_CNT);

    // PCR.RTST_CNT
    _LIST_ADDFIELD_ASCII(listFields, "PCR.rtst_cnt", QString::number(m_u4RTST_CNT), nFieldSelection, eposRTST_CNT);

    // PCR.ABRT_CNT
    _LIST_ADDFIELD_ASCII(listFields, "PCR.abrt_cnt", QString::number(m_u4ABRT_CNT), nFieldSelection, eposABRT_CNT);

    // PCR.GOOD_CNT
    _LIST_ADDFIELD_ASCII(listFields, "PCR.good_cnt", QString::number(m_u4GOOD_CNT), nFieldSelection, eposGOOD_CNT);

    // PCR.FUNC_CNT
    _LIST_ADDFIELD_ASCII(listFields, "PCR.func_cnt", QString::number(m_u4FUNC_CNT), nFieldSelection, eposFUNC_CNT);
}

void Stdf_PCR_V4::GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection /*= 0*/)
{
    QString strTabs="";

    // Init XML string
    for(int nIndex=nIndentationLevel; nIndex>0; nIndex--)
        strTabs += "\t";
    strXmlString = strTabs;
    strXmlString += "<pcr>\n";

    // PCR.HEAD_NUM
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // PCR.SITE_NUM
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "site_num", QString::number(m_u1SITE_NUM), nFieldSelection, eposSITE_NUM);

    // PCR.PART_CNT
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "part_cnt", QString::number(m_u4PART_CNT), nFieldSelection, eposPART_CNT);

    // PCR.RTST_CNT
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "rtst_cnt", QString::number(m_u4RTST_CNT), nFieldSelection, eposRTST_CNT);

    // PCR.ABRT_CNT
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "abrt_cnt", QString::number(m_u4ABRT_CNT), nFieldSelection, eposABRT_CNT);

    // PCR.GOOD_CNT
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "good_cnt", QString::number(m_u4GOOD_CNT), nFieldSelection, eposGOOD_CNT);

    // PCR.FUNC_CNT
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "func_cnt", QString::number(m_u4FUNC_CNT), nFieldSelection, eposFUNC_CNT);

    strXmlString += strTabs;
    strXmlString += "</pcr>\n";
}

void Stdf_PCR_V4::GetAtdfString(QString & strAtdfString)
{
    // Reset field index, and line Nb
    m_uiFieldIndex = 0;
    m_uiLineNb = 1;

    // Init string
    strAtdfString = "PCR:";

    // PCR.HEAD_NUM
    // PCR.SITE_NUM
    if(m_u1HEAD_NUM == 255)
    {
        _STR_ADDFIELD_ATDF(strAtdfString, QString(""), eposHEAD_NUM);
        _STR_ADDFIELD_ATDF(strAtdfString, QString(""), eposSITE_NUM);
    }
    else
    {
        _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u1HEAD_NUM), eposHEAD_NUM);
        _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u1SITE_NUM), eposSITE_NUM);
    }

    // PCR.PART_CNT
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u4PART_CNT), eposPART_CNT);

    // PCR.RTST_CNT
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u4RTST_CNT), eposRTST_CNT);

    // PCR.ABRT_CNT
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u4ABRT_CNT), eposABRT_CNT);

    // PCR.GOOD_CNT
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u4GOOD_CNT), eposGOOD_CNT);

    // PCR.FUNC_CNT
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u4FUNC_CNT), eposFUNC_CNT);

    _STR_FINISHFIELD_ATDF(strAtdfString);
}

///////////////////////////////////////////////////////////
// HBR RECORD
///////////////////////////////////////////////////////////
Stdf_HBR_V4::Stdf_HBR_V4() : Stdf_Record()
{
    Reset();
}

Stdf_HBR_V4::~Stdf_HBR_V4()
{
    Reset();
}

void Stdf_HBR_V4::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Select fields for reduced list

    // Reset Data
    m_u1HEAD_NUM	= 255;		// HBR.HEAD_NUM
    m_u1SITE_NUM	= 1;		// HBR.SITE_NUM
    m_u2HBIN_NUM	= 0;		// HBR.HBIN_NUM
    m_u4HBIN_CNT	= 0;		// HBR.HBIN_CNT
    m_c1HBIN_PF		= 0;		// HBR.HBIN_PF
    m_cnHBIN_NAM	= "";		// HBR.HBIN_NAM

    // Call Reset base method
    Stdf_Record::Reset();
}

bool Stdf_HBR_V4::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;

    return false;
}

QString Stdf_HBR_V4::GetRecordShortName(void)
{
    return "HBR";
}

QString Stdf_HBR_V4::GetRecordLongName(void)
{
    return "Hardware Bin Record";
}

int Stdf_HBR_V4::GetRecordType(void)
{
    return Rec_HBR;
}

bool Stdf_HBR_V4::Read(GS::StdLib::Stdf & clStdf)
{
    char	szString[STDF_MAX_U1+1];
    long	dwData;
    int		wData;
    BYTE	bData;

    // First reset data
    Reset();

    // HBR.HEAD_NUM
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposHEAD_NUM);
    _FIELD_SET(m_u1HEAD_NUM = stdf_type_u1(bData), true, eposHEAD_NUM);

    // HBR.SITE_NUM
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposSITE_NUM);
    _FIELD_SET(m_u1SITE_NUM = stdf_type_u1(bData), true, eposSITE_NUM);

    // HBR.HBIN_NUM
    _FIELD_CHECKREAD(clStdf.ReadWord(&wData), eposHBIN_NUM);
    _FIELD_SET(m_u2HBIN_NUM = stdf_type_u2(wData), true, eposHBIN_NUM);

    // HBR.HBIN_CNT
    _FIELD_CHECKREAD(clStdf.ReadDword(&dwData), eposHBIN_CNT);
    _FIELD_SET(m_u4HBIN_CNT = stdf_type_u4(dwData), true, eposHBIN_CNT);

    // HBR.HBIN_PF
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposHBIN_PF);
    _FIELD_SET(m_c1HBIN_PF = stdf_type_c1(bData), m_c1HBIN_PF != ' ', eposHBIN_PF);

    // HBR.HBIN_NAM
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposHBIN_NAM);
    _FIELD_SET(m_cnHBIN_NAM = szString, !m_cnHBIN_NAM.isEmpty(), eposHBIN_NAM);

    return true;
}

bool Stdf_HBR_V4::Write(GS::StdLib::Stdf & clStdf)
{
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;

    RecordReadInfo.iRecordType = STDF_HBR_TYPE;
    RecordReadInfo.iRecordSubType = STDF_HBR_STYPE;
    clStdf.WriteHeader(&RecordReadInfo);

    // HBR.HEAD_NUM
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_u1HEAD_NUM)), eposHEAD_NUM, clStdf.WriteRecord());

    // HBR.SITE_NUM
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_u1SITE_NUM)), eposSITE_NUM, clStdf.WriteRecord());

    // HBR.HBIN_NUM
    _FIELD_CHECKWRITE(clStdf.WriteWord(WORD(m_u2HBIN_NUM)), eposHBIN_NUM, clStdf.WriteRecord());

    // HBR.HBIN_CNT
    _FIELD_CHECKWRITE(clStdf.WriteDword(DWORD(m_u4HBIN_CNT)), eposHBIN_CNT, clStdf.WriteRecord());

    // HBR.HBIN_PF
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_c1HBIN_PF)), eposHBIN_PF, clStdf.WriteRecord());

    // HBR.HBIN_NAM
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnHBIN_NAM.toLatin1().constData()), eposHBIN_NAM, clStdf.WriteRecord());

    clStdf.WriteRecord();

    return true;
}

void Stdf_HBR_V4::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    // Empty string first
    strAsciiString = "";

    // HBR.HEAD_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "HBR.head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // HBR.SITE_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "HBR.site_num", QString::number(m_u1SITE_NUM), nFieldSelection, eposSITE_NUM);

    // HBR.HBIN_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "HBR.hbin_num", QString::number(m_u2HBIN_NUM), nFieldSelection, eposHBIN_NUM);

    // HBR.HBIN_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "HBR.hbin_cnt", QString::number(m_u4HBIN_CNT), nFieldSelection, eposHBIN_CNT);

    // HBR.HBIN_PF
    _CREATEFIELD_FROM_C1_ASCII(m_c1HBIN_PF);
    _STR_ADDFIELD_ASCII(strAsciiString, "HBR.hbin_pf", m_strFieldValue_macro, nFieldSelection, eposHBIN_PF);

    // HBR.HBIN_NAM
    _STR_ADDFIELD_ASCII(strAsciiString, "HBR.hbin_nam", m_cnHBIN_NAM, nFieldSelection, eposHBIN_NAM);
}

void Stdf_HBR_V4::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
    // Empty string list first
    listFields.empty();

    // HBR.HEAD_NUM
    _LIST_ADDFIELD_ASCII(listFields, "HBR.head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // HBR.SITE_NUM
    _LIST_ADDFIELD_ASCII(listFields, "HBR.site_num", QString::number(m_u1SITE_NUM), nFieldSelection, eposSITE_NUM);

    // HBR.HBIN_NUM
    _LIST_ADDFIELD_ASCII(listFields, "HBR.hbin_num", QString::number(m_u2HBIN_NUM), nFieldSelection, eposHBIN_NUM);

    // HBR.HBIN_CNT
    _LIST_ADDFIELD_ASCII(listFields, "HBR.hbin_cnt", QString::number(m_u4HBIN_CNT), nFieldSelection, eposHBIN_CNT);

    // HBR.HBIN_PF
    _CREATEFIELD_FROM_C1_ASCII(m_c1HBIN_PF);
    _LIST_ADDFIELD_ASCII(listFields, "HBR.hbin_pf", m_strFieldValue_macro, nFieldSelection, eposHBIN_PF);

    // HBR.HBIN_NAM
    _LIST_ADDFIELD_ASCII(listFields, "HBR.hbin_nam", m_cnHBIN_NAM, nFieldSelection, eposHBIN_NAM);
}

void Stdf_HBR_V4::GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection /*= 0*/)
{
    QString strTabs="";

    // Init XML string
    for(int nIndex=nIndentationLevel; nIndex>0; nIndex--)
        strTabs += "\t";
    strXmlString = strTabs;
    strXmlString += "<hbr>\n";

    // HBR.HEAD_NUM
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // HBR.SITE_NUM
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "site_num", QString::number(m_u1SITE_NUM), nFieldSelection, eposSITE_NUM);

    // HBR.HBIN_NUM
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "hbin_num", QString::number(m_u2HBIN_NUM), nFieldSelection, eposHBIN_NUM);

    // HBR.HBIN_CNT
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "hbin_cnt", QString::number(m_u4HBIN_CNT), nFieldSelection, eposHBIN_CNT);

    // HBR.HBIN_PF
    _CREATEFIELD_FROM_C1_XML(m_c1HBIN_PF);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "hbin_pf", m_strFieldValue_macro, nFieldSelection, eposHBIN_PF);

    // HBR.HBIN_NAM
    _CREATEFIELD_FROM_CN_XML(m_cnHBIN_NAM);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "hbin_nam", m_strFieldValue_macro, nFieldSelection, eposHBIN_NAM);

    strXmlString += strTabs;
    strXmlString += "</hbr>\n";
}

void Stdf_HBR_V4::GetAtdfString(QString & strAtdfString)
{
    // Reset field index, and line Nb
    m_uiFieldIndex = 0;
    m_uiLineNb = 1;

    // Init string
    strAtdfString = "HBR:";

    // HBR.HEAD_NUM
    // HBR.SITE_NUM
    if(m_u1HEAD_NUM == 255)
    {
        _STR_ADDFIELD_ATDF(strAtdfString, QString(""), eposHEAD_NUM);
        _STR_ADDFIELD_ATDF(strAtdfString, QString(""), eposSITE_NUM);
    }
    else
    {
        _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u1HEAD_NUM), eposHEAD_NUM);
        _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u1SITE_NUM), eposSITE_NUM);
    }

    // HBR.HBIN_NUM
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u2HBIN_NUM), eposHBIN_NUM);

    // HBR.HBIN_CNT
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u4HBIN_CNT), eposHBIN_CNT);

    // HBR.HBIN_PF
    _CREATEFIELD_FROM_C1_ASCII(m_c1HBIN_PF);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposHBIN_PF);

    // HBR.HBIN_NAM
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnHBIN_NAM, eposHBIN_NAM);

    _STR_FINISHFIELD_ATDF(strAtdfString);
}

///////////////////////////////////////////////////////////
// SBR RECORD
///////////////////////////////////////////////////////////
Stdf_SBR_V4::Stdf_SBR_V4() : Stdf_Record()
{
    Reset();
}

Stdf_SBR_V4::~Stdf_SBR_V4()
{
    Reset();
}

void Stdf_SBR_V4::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Select fields for reduced list

    // Reset Data
    m_u1HEAD_NUM	= 255;		// SBR.HEAD_NUM
    m_u1SITE_NUM	= 1;		// SBR.SITE_NUM
    m_u2SBIN_NUM	= 0;		// SBR.SBIN_NUM
    m_u4SBIN_CNT	= 0;		// SBR.SBIN_CNT
    m_c1SBIN_PF		= 0;		// SBR.SBIN_PF
    m_cnSBIN_NAM	= "";		// SBR.SBIN_NAM

    // Call Reset base method
    Stdf_Record::Reset();
}

bool Stdf_SBR_V4::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;

    return false;
}

QString Stdf_SBR_V4::GetRecordShortName(void)
{
    return "SBR";
}

QString Stdf_SBR_V4::GetRecordLongName(void)
{
    return "Software Bin Record";
}

int Stdf_SBR_V4::GetRecordType(void)
{
    return Rec_SBR;
}

bool Stdf_SBR_V4::Read(GS::StdLib::Stdf & clStdf)
{
    char	szString[STDF_MAX_U1+1];
    long	dwData;
    int		wData;
    BYTE	bData;

    // First reset data
    Reset();

    // SBR.HEAD_NUM
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposHEAD_NUM);
    _FIELD_SET(m_u1HEAD_NUM = stdf_type_u1(bData), true, eposHEAD_NUM);

    // SBR.SITE_NUM
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposSITE_NUM);
    _FIELD_SET(m_u1SITE_NUM = stdf_type_u1(bData), true, eposSITE_NUM);

    // SBR.SBIN_NUM
    _FIELD_CHECKREAD(clStdf.ReadWord(&wData), eposSBIN_NUM);
    _FIELD_SET(m_u2SBIN_NUM = stdf_type_u2(wData), true, eposSBIN_NUM);

    // SBR.SBIN_CNT
    _FIELD_CHECKREAD(clStdf.ReadDword(&dwData), eposSBIN_CNT);
    _FIELD_SET(m_u4SBIN_CNT = stdf_type_u4(dwData), true, eposSBIN_CNT);

    // SBR.SBIN_PF
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposSBIN_PF);
    _FIELD_SET(m_c1SBIN_PF = stdf_type_c1(bData), m_c1SBIN_PF != ' ', eposSBIN_PF);

    // SBR.SBIN_NAM
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposSBIN_NAM);
    _FIELD_SET(m_cnSBIN_NAM = szString, !m_cnSBIN_NAM.isEmpty(), eposSBIN_NAM);

    return true;
}

bool Stdf_SBR_V4::Write(GS::StdLib::Stdf & clStdf)
{
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;

    RecordReadInfo.iRecordType = STDF_SBR_TYPE;
    RecordReadInfo.iRecordSubType = STDF_SBR_STYPE;
    clStdf.WriteHeader(&RecordReadInfo);

    // SBR.HEAD_NUM
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_u1HEAD_NUM)), eposHEAD_NUM, clStdf.WriteRecord());

    // SBR.SITE_NUM
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_u1SITE_NUM)), eposSITE_NUM, clStdf.WriteRecord());

    // SBR.SBIN_NUM
    _FIELD_CHECKWRITE(clStdf.WriteWord(WORD(m_u2SBIN_NUM)), eposSBIN_NUM, clStdf.WriteRecord());

    // SBR.SBIN_CNT
    _FIELD_CHECKWRITE(clStdf.WriteDword(DWORD(m_u4SBIN_CNT)), eposSBIN_CNT, clStdf.WriteRecord());

    // SBR.SBIN_PF
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_c1SBIN_PF)), eposSBIN_PF, clStdf.WriteRecord());

    // SBR.SBIN_NAM
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnSBIN_NAM.toLatin1().constData()), eposSBIN_NAM, clStdf.WriteRecord());

    clStdf.WriteRecord();

    return true;
}

void Stdf_SBR_V4::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    // Empty string first
    strAsciiString = "";

    // SBR.HEAD_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "SBR.head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // SBR.SITE_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "SBR.site_num", QString::number(m_u1SITE_NUM), nFieldSelection, eposSITE_NUM);

    // SBR.SBIN_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "SBR.sbin_num", QString::number(m_u2SBIN_NUM), nFieldSelection, eposSBIN_NUM);

    // SBR.SBIN_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "SBR.sbin_cnt", QString::number(m_u4SBIN_CNT), nFieldSelection, eposSBIN_CNT);

    // SBR.SBIN_PF
    _CREATEFIELD_FROM_C1_ASCII(m_c1SBIN_PF);
    _STR_ADDFIELD_ASCII(strAsciiString, "SBR.sbin_pf", m_strFieldValue_macro, nFieldSelection, eposSBIN_PF);

    // SBR.SBIN_NAM
    _STR_ADDFIELD_ASCII(strAsciiString, "SBR.sbin_nam", m_cnSBIN_NAM, nFieldSelection, eposSBIN_NAM);
}

void Stdf_SBR_V4::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
    // Empty string list first
    listFields.empty();

    // SBR.HEAD_NUM
    _LIST_ADDFIELD_ASCII(listFields, "SBR.head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // SBR.SITE_NUM
    _LIST_ADDFIELD_ASCII(listFields, "SBR.site_num", QString::number(m_u1SITE_NUM), nFieldSelection, eposSITE_NUM);

    // SBR.SBIN_NUM
    _LIST_ADDFIELD_ASCII(listFields, "SBR.sbin_num", QString::number(m_u2SBIN_NUM), nFieldSelection, eposSBIN_NUM);

    // SBR.SBIN_CNT
    _LIST_ADDFIELD_ASCII(listFields, "SBR.sbin_cnt", QString::number(m_u4SBIN_CNT), nFieldSelection, eposSBIN_CNT);

    // SBR.SBIN_PF
    _CREATEFIELD_FROM_C1_ASCII(m_c1SBIN_PF);
    _LIST_ADDFIELD_ASCII(listFields, "SBR.sbin_pf", m_strFieldValue_macro, nFieldSelection, eposSBIN_PF);

    // SBR.SBIN_NAM
    _LIST_ADDFIELD_ASCII(listFields, "SBR.sbin_nam", m_cnSBIN_NAM, nFieldSelection, eposSBIN_NAM);
}

void Stdf_SBR_V4::GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection /*= 0*/)
{
    QString strTabs="";

    // Init XML string
    for(int nIndex=nIndentationLevel; nIndex>0; nIndex--)
        strTabs += "\t";
    strXmlString = strTabs;
    strXmlString += "<sbr>\n";

    // SBR.HEAD_NUM
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // SBR.SITE_NUM
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "site_num", QString::number(m_u1SITE_NUM), nFieldSelection, eposSITE_NUM);

    // SBR.SBIN_NUM
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "sbin_num", QString::number(m_u2SBIN_NUM), nFieldSelection, eposSBIN_NUM);

    // SBR.SBIN_CNT
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "sbin_cnt", QString::number(m_u4SBIN_CNT), nFieldSelection, eposSBIN_CNT);

    // SBR.SBIN_PF
    _CREATEFIELD_FROM_C1_XML(m_c1SBIN_PF);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "sbin_pf", m_strFieldValue_macro, nFieldSelection, eposSBIN_PF);

    // SBR.SBIN_NAM
    _CREATEFIELD_FROM_CN_XML(m_cnSBIN_NAM);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "sbin_nam", m_strFieldValue_macro, nFieldSelection, eposSBIN_NAM);

    strXmlString += strTabs;
    strXmlString += "</sbr>\n";
}

void Stdf_SBR_V4::GetAtdfString(QString & strAtdfString)
{
    // Reset field index, and line Nb
    m_uiFieldIndex = 0;
    m_uiLineNb = 1;

    // Init string
    strAtdfString = "SBR:";

    // SBR.HEAD_NUM
    // SBR.SITE_NUM
    if(m_u1HEAD_NUM == 255)
    {
        _STR_ADDFIELD_ATDF(strAtdfString, QString(""), eposHEAD_NUM);
        _STR_ADDFIELD_ATDF(strAtdfString, QString(""), eposSITE_NUM);
    }
    else
    {
        _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u1HEAD_NUM), eposHEAD_NUM);
        _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u1SITE_NUM), eposSITE_NUM);
    }

    // SBR.SBIN_NUM
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u2SBIN_NUM), eposSBIN_NUM);

    // SBR.SBIN_CNT
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u4SBIN_CNT), eposSBIN_CNT);

    // SBR.SBIN_PF
    _CREATEFIELD_FROM_C1_ASCII(m_c1SBIN_PF);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposSBIN_PF);

    // SBR.SBIN_NAM
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnSBIN_NAM, eposSBIN_NAM);

    _STR_FINISHFIELD_ATDF(strAtdfString);
}

///////////////////////////////////////////////////////////
// PMR RECORD
///////////////////////////////////////////////////////////
Stdf_PMR_V4::Stdf_PMR_V4() : Stdf_Record()
{
    Reset();
}

Stdf_PMR_V4::~Stdf_PMR_V4()
{
    Reset();
}

void Stdf_PMR_V4::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Select fields for reduced list

    // Reset Data
    m_u2PMR_INDX	= 0;		// PMR.PMR_INDX
    m_u2CHAN_TYP	= 0;		// PMR.CHAN_TYP
    m_cnCHAN_NAM	= "";		// PMR.CHAN_NAM
    m_cnPHY_NAM		= "";		// PMR.PHY_NAM
    m_cnLOG_NAM		= "";		// PMR.LOG_NAM
    m_u1HEAD_NUM	= 1;		// PMR.HEAD_NUM
    m_u1SITE_NUM	= 1;		// PMR.SITE_NUM

    // Call Reset base method
    Stdf_Record::Reset();
}

bool Stdf_PMR_V4::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;

    return false;
}

QString Stdf_PMR_V4::GetRecordShortName(void)
{
    return "PMR";
}

QString Stdf_PMR_V4::GetRecordLongName(void)
{
    return "Pin Map Record";
}

int Stdf_PMR_V4::GetRecordType(void)
{
    return Rec_PMR;
}

bool Stdf_PMR_V4::Read(GS::StdLib::Stdf & clStdf)
{
    char	szString[STDF_MAX_U1+1];
    int		wData;
    BYTE	bData;

    // First reset data
    Reset();

    // PMR.PMR_INDX
    _FIELD_CHECKREAD(clStdf.ReadWord(&wData), eposPMR_INDX);
    _FIELD_SET(m_u2PMR_INDX = stdf_type_u2(wData), true, eposPMR_INDX);

    // PMR.CHAN_TYP
    _FIELD_CHECKREAD(clStdf.ReadWord(&wData), eposCHAN_TYP);
    _FIELD_SET(m_u2CHAN_TYP = stdf_type_u2(wData), m_u2CHAN_TYP != 0, eposCHAN_TYP);

    // PMR.CHAN_NAM
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposCHAN_NAM);
    _FIELD_SET(m_cnCHAN_NAM = szString, !m_cnCHAN_NAM.isEmpty(), eposCHAN_NAM);

    // PMR.PHY_NAM
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposPHY_NAM);
    _FIELD_SET(m_cnPHY_NAM = szString, !m_cnPHY_NAM.isEmpty(), eposPHY_NAM);

    // PMR.LOG_NAM
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposLOG_NAM);
    _FIELD_SET(m_cnLOG_NAM = szString, !m_cnLOG_NAM.isEmpty(), eposLOG_NAM);

    // PMR.HEAD_NUM
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposHEAD_NUM);
    _FIELD_SET(m_u1HEAD_NUM = stdf_type_u1(bData), true, eposHEAD_NUM);

    // PMR.SITE_NUM
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposSITE_NUM);
    _FIELD_SET(m_u1SITE_NUM = stdf_type_u1(bData), true, eposSITE_NUM);

    return true;
}

bool Stdf_PMR_V4::Write(GS::StdLib::Stdf & clStdf)
{
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;

    RecordReadInfo.iRecordType = STDF_PMR_TYPE;
    RecordReadInfo.iRecordSubType = STDF_PMR_STYPE;
    clStdf.WriteHeader(&RecordReadInfo);

    // PMR.PMR_INDX
    _FIELD_CHECKWRITE(clStdf.WriteWord(WORD(m_u2PMR_INDX)), eposPMR_INDX, clStdf.WriteRecord());

    // PMR.CHAN_TYP
    _FIELD_CHECKWRITE(clStdf.WriteWord(WORD(m_u2CHAN_TYP)), eposCHAN_TYP, clStdf.WriteRecord());

    // PMR.CHAN_NAM
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnCHAN_NAM.toLatin1().constData()), eposCHAN_NAM, clStdf.WriteRecord());

    // PMR.PHY_NAM
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnPHY_NAM.toLatin1().constData()), eposPHY_NAM, clStdf.WriteRecord());

    // PMR.LOG_NAM
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnLOG_NAM.toLatin1().constData()), eposLOG_NAM, clStdf.WriteRecord());

    // PMR.HEAD_NUM
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_u1HEAD_NUM)), eposHEAD_NUM, clStdf.WriteRecord());

    // PMR.SITE_NUM
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_u1SITE_NUM)), eposSITE_NUM, clStdf.WriteRecord());

    clStdf.WriteRecord();

    return true;
}

void Stdf_PMR_V4::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    // Empty string first
    strAsciiString = "";

    // PMR.PMR_INDX
    _STR_ADDFIELD_ASCII(strAsciiString, "PMR.pmr_indx", QString::number(m_u2PMR_INDX), nFieldSelection, eposPMR_INDX);

    // PMR.CHAN_TYP
    _STR_ADDFIELD_ASCII(strAsciiString, "PMR.chan_typ", QString::number(m_u2CHAN_TYP), nFieldSelection, eposCHAN_TYP);

    // PMR.CHAN_NAM
    _STR_ADDFIELD_ASCII(strAsciiString, "PMR.chan_nam", m_cnCHAN_NAM, nFieldSelection, eposCHAN_NAM);

    // PMR.PHY_NAM
    _STR_ADDFIELD_ASCII(strAsciiString, "PMR.phy_nam", m_cnPHY_NAM, nFieldSelection, eposPHY_NAM);

    // PMR.LOG_NAM
    _STR_ADDFIELD_ASCII(strAsciiString, "PMR.log_nam", m_cnLOG_NAM, nFieldSelection, eposLOG_NAM);

    // PMR.HEAD_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "PMR.head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // PMR.SITE_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "PMR.site_num", QString::number(m_u1SITE_NUM), nFieldSelection, eposSITE_NUM);
}

void Stdf_PMR_V4::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
    // Empty string list first
    listFields.empty();

    // PMR.PMR_INDX
    _LIST_ADDFIELD_ASCII(listFields, "PMR.pmr_indx", QString::number(m_u2PMR_INDX), nFieldSelection, eposPMR_INDX);

    // PMR.CHAN_TYP
    _LIST_ADDFIELD_ASCII(listFields, "PMR.chan_typ", QString::number(m_u2CHAN_TYP), nFieldSelection, eposCHAN_TYP);

    // PMR.CHAN_NAM
    _LIST_ADDFIELD_ASCII(listFields, "PMR.chan_nam", m_cnCHAN_NAM, nFieldSelection, eposCHAN_NAM);

    // PMR.PHY_NAM
    _LIST_ADDFIELD_ASCII(listFields, "PMR.phy_nam", m_cnPHY_NAM, nFieldSelection, eposPHY_NAM);

    // PMR.LOG_NAM
    _LIST_ADDFIELD_ASCII(listFields, "PMR.log_nam", m_cnLOG_NAM, nFieldSelection, eposLOG_NAM);

    // PMR.HEAD_NUM
    _LIST_ADDFIELD_ASCII(listFields, "PMR.head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // PMR.SITE_NUM
    _LIST_ADDFIELD_ASCII(listFields, "PMR.site_num", QString::number(m_u1SITE_NUM), nFieldSelection, eposSITE_NUM);
}

void Stdf_PMR_V4::GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection /*= 0*/)
{
    QString strTabs="";

    // Init XML string
    for(int nIndex=nIndentationLevel; nIndex>0; nIndex--)
        strTabs += "\t";
    strXmlString = strTabs;
    strXmlString += "<pmr>\n";

    // PMR.PMR_INDX
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "pmr_indx", QString::number(m_u2PMR_INDX), nFieldSelection, eposPMR_INDX);

    // PMR.CHAN_TYP
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "chan_typ", QString::number(m_u2CHAN_TYP), nFieldSelection, eposCHAN_TYP);

    // PMR.CHAN_NAM
    _CREATEFIELD_FROM_CN_XML(m_cnCHAN_NAM);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "chan_nam", m_strFieldValue_macro, nFieldSelection, eposCHAN_NAM);

    // PMR.PHY_NAM
    _CREATEFIELD_FROM_CN_XML(m_cnPHY_NAM);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "phy_nam", m_strFieldValue_macro, nFieldSelection, eposPHY_NAM);

    // PMR.LOG_NAM
    _CREATEFIELD_FROM_CN_XML(m_cnLOG_NAM);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "log_nam", m_strFieldValue_macro, nFieldSelection, eposLOG_NAM);

    // PMR.HEAD_NUM
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // PMR.SITE_NUM
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "site_num", QString::number(m_u1SITE_NUM), nFieldSelection, eposSITE_NUM);

    strXmlString += strTabs;
    strXmlString += "</pmr>\n";
}

void Stdf_PMR_V4::GetAtdfString(QString & strAtdfString)
{
    // Reset field index, and line Nb
    m_uiFieldIndex = 0;
    m_uiLineNb = 1;

    // Init string
    strAtdfString = "PMR:";

    // PMR.PMR_INDX
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u2PMR_INDX), eposPMR_INDX);

    // PMR.CHAN_TYP
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u2CHAN_TYP), eposCHAN_TYP);

    // PMR.CHAN_NAM
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnCHAN_NAM, eposCHAN_NAM);

    // PMR.PHY_NAM
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnPHY_NAM, eposPHY_NAM);

    // PMR.LOG_NAM
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnLOG_NAM, eposLOG_NAM);

    // PMR.HEAD_NUM
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u1HEAD_NUM), eposHEAD_NUM);

    // PMR.SITE_NUM
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u1SITE_NUM), eposSITE_NUM);

    _STR_FINISHFIELD_ATDF(strAtdfString);
}

///////////////////////////////////////////////////////////
// PGR RECORD
///////////////////////////////////////////////////////////
Stdf_PGR_V4::Stdf_PGR_V4() : Stdf_Record()
{
    m_ku2PMR_INDX = NULL;
    Reset();
}

Stdf_PGR_V4::~Stdf_PGR_V4()
{
    Reset();
}

void Stdf_PGR_V4::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Select fields for reduced list

    // Reset Data
    m_u2GRP_INDX	= 0;			// PGR.GRP_INDX
    m_cnGRP_NAM		= "";			// PGR.GRP_NAM
    m_u2INDX_CNT	= 0;			// PGR.INDX_CNT
    if(m_ku2PMR_INDX != NULL)		// PGR.PMR_INDX
    {
        delete [] m_ku2PMR_INDX;
        m_ku2PMR_INDX = NULL;
    }

    // Call Reset base method
    Stdf_Record::Reset();
}

bool Stdf_PGR_V4::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;

    return false;
}

QString Stdf_PGR_V4::GetRecordShortName(void)
{
    return "PGR";
}

QString Stdf_PGR_V4::GetRecordLongName(void)
{
    return "Pin Group Record";
}

int Stdf_PGR_V4::GetRecordType(void)
{
    return Rec_PGR;
}

bool Stdf_PGR_V4::Read(GS::StdLib::Stdf & clStdf)
{
    char			szString[STDF_MAX_U1+1];
    int				wData;
    unsigned int	i;

    // First reset data
    Reset();

    // PGR.GRP_INDX
    _FIELD_CHECKREAD(clStdf.ReadWord(&wData), eposGRP_INDX);
    _FIELD_SET(m_u2GRP_INDX = stdf_type_u2(wData), true, eposGRP_INDX);

    // PGR.GRP_NAM
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposGRP_NAM);
    _FIELD_SET(m_cnGRP_NAM = szString, !m_cnGRP_NAM.isEmpty(), eposGRP_NAM);

    // PGR.INDX_CNT
    _FIELD_CHECKREAD(clStdf.ReadWord(&wData), eposINDX_CNT);
    _FIELD_SET(m_u2INDX_CNT = stdf_type_u2(wData), true, eposINDX_CNT);

    // PGR.PMR_INDX
    // Set the field as present, even if count is 0. We use a dummy instruction as parameter 1 to avoid warning under gcc.
    _FIELD_SET(m_uiIndex_macro = 0, true, eposPMR_INDX);
    if(m_u2INDX_CNT > 0)
        m_ku2PMR_INDX = new stdf_type_u2[m_u2INDX_CNT];
    for(i=0; i<(unsigned int)m_u2INDX_CNT; i++)
    {
        _FIELD_CHECKREAD(clStdf.ReadWord(&wData), eposPMR_INDX);
        _FIELD_SET(m_ku2PMR_INDX[i] = stdf_type_u2(wData), true, eposPMR_INDX);
    }

    return true;
}

bool Stdf_PGR_V4::Write(GS::StdLib::Stdf & clStdf)
{
    GS::StdLib::StdfRecordReadInfo	RecordReadInfo;
    unsigned int		i;

    RecordReadInfo.iRecordType = STDF_PGR_TYPE;
    RecordReadInfo.iRecordSubType = STDF_PGR_STYPE;
    clStdf.WriteHeader(&RecordReadInfo);

    // PGR.GRP_INDX
    _FIELD_CHECKWRITE(clStdf.WriteWord(WORD(m_u2GRP_INDX)), eposGRP_INDX, clStdf.WriteRecord());

    // PGR.GRP_NAM
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnGRP_NAM.toLatin1().constData()), eposGRP_NAM, clStdf.WriteRecord());

    // PGR.INDX_CNT
    _FIELD_CHECKWRITE(clStdf.WriteWord(WORD(m_u2INDX_CNT)), eposINDX_CNT, clStdf.WriteRecord());

    // PGR.PMR_INDX
    for(i=0; i<(unsigned int)m_u2INDX_CNT; i++)
    _FIELD_CHECKWRITE(clStdf.WriteWord(WORD(m_ku2PMR_INDX[i])), eposPMR_INDX, clStdf.WriteRecord());

    clStdf.WriteRecord();

    return true;
}

void Stdf_PGR_V4::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    // Empty string first
    strAsciiString = "";

    // PGR.GRP_INDX
    _STR_ADDFIELD_ASCII(strAsciiString, "PGR.grp_indx", QString::number(m_u2GRP_INDX), nFieldSelection, eposGRP_INDX);

    // PGR.GRP_NAM
    _STR_ADDFIELD_ASCII(strAsciiString, "PGR.grp_nam", m_cnGRP_NAM, nFieldSelection, eposGRP_NAM);

    // PGR.INDX_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "PGR.indx_cnt", QString::number(m_u2INDX_CNT), nFieldSelection, eposINDX_CNT);

    // PGR.PMR_INDX
    _CREATEFIELD_FROM_KUi_ASCII(m_u2INDX_CNT, m_ku2PMR_INDX, eposPMR_INDX);
    _STR_ADDFIELD_ASCII(strAsciiString, "PGR.pmr_indx", m_strFieldValue_macro, nFieldSelection, eposPMR_INDX);
}

void Stdf_PGR_V4::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
    // Empty string list first
    listFields.empty();

    // PGR.GRP_INDX
    _LIST_ADDFIELD_ASCII(listFields, "PGR.grp_indx", QString::number(m_u2GRP_INDX), nFieldSelection, eposGRP_INDX);

    // PGR.GRP_NAM
    _LIST_ADDFIELD_ASCII(listFields, "PGR.grp_nam", m_cnGRP_NAM, nFieldSelection, eposGRP_NAM);

    // PGR.INDX_CNT
    _LIST_ADDFIELD_ASCII(listFields, "PGR.indx_cnt", QString::number(m_u2INDX_CNT), nFieldSelection, eposINDX_CNT);

    // PGR.PMR_INDX
    _CREATEFIELD_FROM_KUi_ASCII(m_u2INDX_CNT, m_ku2PMR_INDX, eposPMR_INDX);
    _LIST_ADDFIELD_ASCII(listFields, "PGR.pmr_indx", m_strFieldValue_macro, nFieldSelection, eposPMR_INDX);
}

void Stdf_PGR_V4::GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection /*= 0*/)
{
    QString strTabs="";

    // Init XML string
    for(int nIndex=nIndentationLevel; nIndex>0; nIndex--)
        strTabs += "\t";
    strXmlString = strTabs;
    strXmlString += "<pgr>\n";

    // PGR.GRP_INDX
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "grp_indx", QString::number(m_u2GRP_INDX), nFieldSelection, eposGRP_INDX);

    // PGR.GRP_NAM
    _CREATEFIELD_FROM_CN_XML(m_cnGRP_NAM);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "grp_nam", m_strFieldValue_macro, nFieldSelection, eposGRP_NAM);

    // PGR.PMR_INDX
    _CREATEFIELD_FROM_KUi_XML(m_u2INDX_CNT, m_ku2PMR_INDX, eposPMR_INDX);
    _STR_ADDLIST_XML(strXmlString, m_u2INDX_CNT, nIndentationLevel+1, "pmr_indx", m_strFieldValue_macro, nFieldSelection, eposPMR_INDX);

    strXmlString += strTabs;
    strXmlString += "</pgr>\n";
}

void Stdf_PGR_V4::GetAtdfString(QString & strAtdfString)
{
    // Reset field index, and line Nb
    m_uiFieldIndex = 0;
    m_uiLineNb = 1;

    // Init string
    strAtdfString = "PGR:";

    // PGR.GRP_INDX
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u2GRP_INDX), eposGRP_INDX);

    // PGR.GRP_NAM
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnGRP_NAM, eposGRP_NAM);

    // PGR.PMR_INDX
    _CREATEFIELD_FROM_KUi_ATDF(m_u2INDX_CNT, m_ku2PMR_INDX, eposPMR_INDX);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposPMR_INDX);

    _STR_FINISHFIELD_ATDF(strAtdfString);
}

///////////////////////////////////////////////////////////
// PLR RECORD
///////////////////////////////////////////////////////////
Stdf_PLR_V4::Stdf_PLR_V4() : Stdf_Record()
{
    m_ku2GRP_INDX = NULL;
    m_ku2GRP_MODE = NULL;
    m_ku1GRP_RADX = NULL;
    m_kcnPGM_CHAR = NULL;
    m_kcnRTN_CHAR = NULL;
    m_kcnPGM_CHAL = NULL;
    m_kcnRTN_CHAL = NULL;
    Reset();
}

Stdf_PLR_V4::~Stdf_PLR_V4()
{
    Reset();
}

void Stdf_PLR_V4::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Select fields for reduced list

    // Reset Data
    m_u2GRP_CNT	= 0;				// PLR.GRP_CNT
    if(m_ku2GRP_INDX != NULL)		// PLR.GRP_INDX
    {
        delete [] m_ku2GRP_INDX;
        m_ku2GRP_INDX = NULL;
    }
    if(m_ku2GRP_MODE != NULL)		// PLR.GRP_MODE
    {
        delete [] m_ku2GRP_MODE;
        m_ku2GRP_MODE = NULL;
    }
    if(m_ku1GRP_RADX != NULL)		// PLR.GRP_RADX
    {
        delete [] m_ku1GRP_RADX;
        m_ku1GRP_RADX = NULL;
    }
    if(m_kcnPGM_CHAR != NULL)		// PLR.PGM_CHAR
    {
        delete [] m_kcnPGM_CHAR;
        m_kcnPGM_CHAR = NULL;
    }
    if(m_kcnRTN_CHAR != NULL)		// PLR.RTN_CHAR
    {
        delete [] m_kcnRTN_CHAR;
        m_kcnRTN_CHAR = NULL;
    }
    if(m_kcnPGM_CHAL != NULL)		// PLR.PGM_CHAL
    {
        delete [] m_kcnPGM_CHAL;
        m_kcnPGM_CHAL = NULL;
    }
    if(m_kcnRTN_CHAL != NULL)		// PLR.RTN_CHAL
    {
        delete [] m_kcnRTN_CHAL;
        m_kcnRTN_CHAL = NULL;
    }

    // Call Reset base method
    Stdf_Record::Reset();
}

bool Stdf_PLR_V4::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;

    return false;
}

QString Stdf_PLR_V4::GetRecordShortName(void)
{
    return "PLR";
}

QString Stdf_PLR_V4::GetRecordLongName(void)
{
    return "Pin List Record";
}

int Stdf_PLR_V4::GetRecordType(void)
{
    return Rec_PLR;
}

bool Stdf_PLR_V4::Read(GS::StdLib::Stdf & clStdf)
{
    char			szString[STDF_MAX_U1+1];
    int				wData;
    BYTE			bData;
    unsigned int	i;

    // First reset data
    Reset();

    // PLR.GRP_CNT
    _FIELD_CHECKREAD(clStdf.ReadWord(&wData), eposGRP_CNT);
    _FIELD_SET(m_u2GRP_CNT = stdf_type_u2(wData), true, eposGRP_CNT);

    // PLR.GRP_INDX
    // Set the field as present, even if count is 0. We use a dummy instruction as parameter 1 to avoid warning under gcc.
    _FIELD_SET(m_uiIndex_macro = 0, true, eposGRP_INDX);
    if(m_u2GRP_CNT > 0)
        m_ku2GRP_INDX = new stdf_type_u2[m_u2GRP_CNT];
    for(i=0; i<(unsigned int)m_u2GRP_CNT; i++)
    {
        _FIELD_CHECKREAD(clStdf.ReadWord(&wData), eposGRP_INDX);
        _FIELD_SET(m_ku2GRP_INDX[i] = stdf_type_u2(wData), true, eposGRP_INDX);
    }

    // PLR.GRP_MODE
    // Set the field as present, even if count is 0. We use a dummy instruction as parameter 1 to avoid warning under gcc.
    _FIELD_SET(m_uiIndex_macro = 0, true, eposGRP_MODE);
    if(m_u2GRP_CNT > 0)
        m_ku2GRP_MODE = new stdf_type_u2[m_u2GRP_CNT];
    for(i=0; i<(unsigned int)m_u2GRP_CNT; i++)
    {
        _FIELD_CHECKREAD(clStdf.ReadWord(&wData), eposGRP_MODE);
        _FIELD_SET(m_ku2GRP_MODE[i] = stdf_type_u2(wData), true, eposGRP_MODE);
    }

    // PLR.GRP_RADX
    // Set the field as present, even if count is 0. We use a dummy instruction as parameter 1 to avoid warning under gcc.
    _FIELD_SET(m_uiIndex_macro = 0, true, eposGRP_RADX);
    if(m_u2GRP_CNT > 0)
        m_ku1GRP_RADX = new stdf_type_u1[m_u2GRP_CNT];
    for(i=0; i<(unsigned int)m_u2GRP_CNT; i++)
    {
        _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposGRP_RADX);
        _FIELD_SET(m_ku1GRP_RADX[i] = stdf_type_u1(bData), true, eposGRP_RADX);
    }

    // PLR.PGM_CHAR
    // Set the field as present, even if count is 0. We use a dummy instruction as parameter 1 to avoid warning under gcc.
    _FIELD_SET(m_uiIndex_macro = 0, true, eposPGM_CHAR);
    if(m_u2GRP_CNT > 0)
        m_kcnPGM_CHAR = new stdf_type_cn[m_u2GRP_CNT];
    for(i=0; i<(unsigned int)m_u2GRP_CNT; i++)
    {
        _FIELD_CHECKREAD(clStdf.ReadString(szString), eposPGM_CHAR);
        _FIELD_SET(m_kcnPGM_CHAR[i] = szString, true, eposPGM_CHAR);
    }

    // PLR.RTN_CHAR
    // Set the field as present, even if count is 0. We use a dummy instruction as parameter 1 to avoid warning under gcc.
    _FIELD_SET(m_uiIndex_macro = 0, true, eposRTN_CHAR);
    if(m_u2GRP_CNT > 0)
        m_kcnRTN_CHAR = new stdf_type_cn[m_u2GRP_CNT];
    for(i=0; i<(unsigned int)m_u2GRP_CNT; i++)
    {
        _FIELD_CHECKREAD(clStdf.ReadString(szString), eposRTN_CHAR);
        _FIELD_SET(m_kcnRTN_CHAR[i] = szString, true, eposRTN_CHAR);
    }

    // PLR.PGM_CHAL
    // Set the field as present, even if count is 0. We use a dummy instruction as parameter 1 to avoid warning under gcc.
    _FIELD_SET(m_uiIndex_macro = 0, true, eposPGM_CHAL);
    if(m_u2GRP_CNT > 0)
        m_kcnPGM_CHAL = new stdf_type_cn[m_u2GRP_CNT];
    for(i=0; i<(unsigned int)m_u2GRP_CNT; i++)
    {
        _FIELD_CHECKREAD(clStdf.ReadString(szString), eposPGM_CHAL);
        _FIELD_SET(m_kcnPGM_CHAL[i] = szString, true, eposPGM_CHAL);
    }

    // PLR.RTN_CHAL
    // Set the field as present, even if count is 0. We use a dummy instruction as parameter 1 to avoid warning under gcc.
    _FIELD_SET(m_uiIndex_macro = 0, true, eposRTN_CHAL);
    if(m_u2GRP_CNT > 0)
        m_kcnRTN_CHAL = new stdf_type_cn[m_u2GRP_CNT];
    for(i=0; i<(unsigned int)m_u2GRP_CNT; i++)
    {
        _FIELD_CHECKREAD(clStdf.ReadString(szString), eposRTN_CHAL);
        _FIELD_SET(m_kcnRTN_CHAL[i] = szString, true, eposRTN_CHAL);
    }

    return true;
}

bool Stdf_PLR_V4::Write(GS::StdLib::Stdf & clStdf)
{
    GS::StdLib::StdfRecordReadInfo	RecordReadInfo;
    unsigned int		i;

    RecordReadInfo.iRecordType = STDF_PLR_TYPE;
    RecordReadInfo.iRecordSubType = STDF_PLR_STYPE;
    clStdf.WriteHeader(&RecordReadInfo);

    // PLR.GRP_CNT
    _FIELD_CHECKWRITE(clStdf.WriteWord(WORD(m_u2GRP_CNT)), eposGRP_CNT, clStdf.WriteRecord());

    // PLR.GRP_INDX
    for(i=0; i<(unsigned int)m_u2GRP_CNT; i++)
    _FIELD_CHECKWRITE(clStdf.WriteWord(WORD(m_ku2GRP_INDX[i])), eposGRP_INDX, clStdf.WriteRecord());

    // PLR.GRP_MODE
    for(i=0; i<(unsigned int)m_u2GRP_CNT; i++)
    _FIELD_CHECKWRITE(clStdf.WriteWord(WORD(m_ku2GRP_MODE[i])), eposGRP_MODE, clStdf.WriteRecord());

    // PLR.GRP_RADX
    for(i=0; i<(unsigned int)m_u2GRP_CNT; i++)
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_ku1GRP_RADX[i])), eposGRP_RADX, clStdf.WriteRecord());

    // PLR.PGM_CHAR
    for(i=0; i<(unsigned int)m_u2GRP_CNT; i++)
    _FIELD_CHECKWRITE(clStdf.WriteString(m_kcnPGM_CHAR[i].toLatin1().constData()), eposPGM_CHAR, clStdf.WriteRecord());

    // PLR.RTN_CHAR
    for(i=0; i<(unsigned int)m_u2GRP_CNT; i++)
    _FIELD_CHECKWRITE(clStdf.WriteString(m_kcnRTN_CHAR[i].toLatin1().constData()), eposRTN_CHAR, clStdf.WriteRecord());

    // PLR.PGM_CHAL
    for(i=0; i<(unsigned int)m_u2GRP_CNT; i++)
    _FIELD_CHECKWRITE(clStdf.WriteString(m_kcnPGM_CHAL[i].toLatin1().constData()), eposPGM_CHAL, clStdf.WriteRecord());

    // PLR.RTN_CHAL
    for(i=0; i<(unsigned int)m_u2GRP_CNT; i++)
    _FIELD_CHECKWRITE(clStdf.WriteString(m_kcnRTN_CHAL[i].toLatin1().constData()), eposRTN_CHAL, clStdf.WriteRecord());

    clStdf.WriteRecord();

    return true;
}

void Stdf_PLR_V4::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    // Empty string first
    strAsciiString = "";

    // PLR.GRP_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "PLR.grp_cnt", QString::number(m_u2GRP_CNT), nFieldSelection, eposGRP_CNT);

    // PLR.GRP_INDX
    _CREATEFIELD_FROM_KUi_ASCII(m_u2GRP_CNT, m_ku2GRP_INDX, eposGRP_INDX);
    _STR_ADDFIELD_ASCII(strAsciiString, "PLR.grp_indx", m_strFieldValue_macro, nFieldSelection, eposGRP_INDX);

    // PLR.GRP_MODE
    _CREATEFIELD_FROM_KUi_ASCII(m_u2GRP_CNT, m_ku2GRP_MODE, eposGRP_MODE);
    _STR_ADDFIELD_ASCII(strAsciiString, "PLR.grp_mode", m_strFieldValue_macro, nFieldSelection, eposGRP_MODE);

    // PLR.GRP_RADX
    _CREATEFIELD_FROM_KUi_ASCII(m_u2GRP_CNT, m_ku1GRP_RADX, eposGRP_RADX);
    _STR_ADDFIELD_ASCII(strAsciiString, "PLR.grp_radx", m_strFieldValue_macro, nFieldSelection, eposGRP_RADX);

    // PLR.PGM_CHAR
    _CREATEFIELD_FROM_KCN_ASCII(m_u2GRP_CNT, m_kcnPGM_CHAR, eposPGM_CHAR);
    _STR_ADDFIELD_ASCII(strAsciiString, "PLR.pgm_char", m_strFieldValue_macro, nFieldSelection, eposPGM_CHAR);

    // PLR.RTN_CHAR
    _CREATEFIELD_FROM_KCN_ASCII(m_u2GRP_CNT, m_kcnRTN_CHAR, eposRTN_CHAR);
    _STR_ADDFIELD_ASCII(strAsciiString, "PLR.rtn_char", m_strFieldValue_macro, nFieldSelection, eposRTN_CHAR);

    // PLR.PGM_CHAL
    _CREATEFIELD_FROM_KCN_ASCII(m_u2GRP_CNT, m_kcnPGM_CHAL, eposPGM_CHAL);
    _STR_ADDFIELD_ASCII(strAsciiString, "PLR.pgm_chal", m_strFieldValue_macro, nFieldSelection, eposPGM_CHAL);

    // PLR.RTN_CHAL
    _CREATEFIELD_FROM_KCN_ASCII(m_u2GRP_CNT, m_kcnRTN_CHAL, eposRTN_CHAL);
    _STR_ADDFIELD_ASCII(strAsciiString, "PLR.rtn_chal", m_strFieldValue_macro, nFieldSelection, eposRTN_CHAL);
}

void Stdf_PLR_V4::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
    // Empty string list first
    listFields.empty();

    // PLR.GRP_CNT
    _LIST_ADDFIELD_ASCII(listFields, "PLR.grp_cnt", QString::number(m_u2GRP_CNT), nFieldSelection, eposGRP_CNT);

    // PLR.GRP_INDX
    _CREATEFIELD_FROM_KUi_ASCII(m_u2GRP_CNT, m_ku2GRP_INDX, eposGRP_INDX);
    _LIST_ADDFIELD_ASCII(listFields, "PLR.grp_indx", m_strFieldValue_macro, nFieldSelection, eposGRP_INDX);

    // PLR.GRP_MODE
    _CREATEFIELD_FROM_KUi_ASCII(m_u2GRP_CNT, m_ku2GRP_MODE, eposGRP_MODE);
    _LIST_ADDFIELD_ASCII(listFields, "PLR.grp_mode", m_strFieldValue_macro, nFieldSelection, eposGRP_MODE);

    // PLR.GRP_RADX
    _CREATEFIELD_FROM_KUi_ASCII(m_u2GRP_CNT, m_ku1GRP_RADX, eposGRP_RADX);
    _LIST_ADDFIELD_ASCII(listFields, "PLR.grp_radx", m_strFieldValue_macro, nFieldSelection, eposGRP_RADX);

    // PLR.PGM_CHAR
    _CREATEFIELD_FROM_KCN_ASCII(m_u2GRP_CNT, m_kcnPGM_CHAR, eposPGM_CHAR);
    _LIST_ADDFIELD_ASCII(listFields, "PLR.pgm_char", m_strFieldValue_macro, nFieldSelection, eposPGM_CHAR);

    // PLR.RTN_CHAR
    _CREATEFIELD_FROM_KCN_ASCII(m_u2GRP_CNT, m_kcnRTN_CHAR, eposRTN_CHAR);
    _LIST_ADDFIELD_ASCII(listFields, "PLR.rtn_char", m_strFieldValue_macro, nFieldSelection, eposRTN_CHAR);

    // PLR.PGM_CHAL
    _CREATEFIELD_FROM_KCN_ASCII(m_u2GRP_CNT, m_kcnPGM_CHAL, eposPGM_CHAL);
    _LIST_ADDFIELD_ASCII(listFields, "PLR.pgm_chal", m_strFieldValue_macro, nFieldSelection, eposPGM_CHAL);

    // PLR.RTN_CHAL
    _CREATEFIELD_FROM_KCN_ASCII(m_u2GRP_CNT, m_kcnRTN_CHAL, eposRTN_CHAL);
    _LIST_ADDFIELD_ASCII(listFields, "PLR.rtn_chal", m_strFieldValue_macro, nFieldSelection, eposRTN_CHAL);
}

void Stdf_PLR_V4::GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection /*= 0*/)
{
    QString strTabs="";

    // Init XML string
    for(int nIndex=nIndentationLevel; nIndex>0; nIndex--)
        strTabs += "\t";
    strXmlString = strTabs;
    strXmlString += "<plr>\n";

    // PLR.GRP_INDX
    _CREATEFIELD_FROM_KUi_XML(m_u2GRP_CNT, m_ku2GRP_INDX, eposGRP_INDX);
    _STR_ADDLIST_XML(strXmlString,m_u2GRP_CNT,nIndentationLevel+1,"grp_indx",m_strFieldValue_macro,nFieldSelection,eposGRP_INDX);

    // PLR.GRP_MODE
    _CREATEFIELD_FROM_KUi_XML(m_u2GRP_CNT, m_ku2GRP_MODE, eposGRP_MODE);
    _STR_ADDLIST_XML(strXmlString,m_u2GRP_CNT,nIndentationLevel+1,"grp_mode",m_strFieldValue_macro,nFieldSelection,eposGRP_MODE);

    // PLR.GRP_RADX
    _CREATEFIELD_FROM_KUi_XML(m_u2GRP_CNT, m_ku1GRP_RADX, eposGRP_RADX);
    _STR_ADDLIST_XML(strXmlString,m_u2GRP_CNT,nIndentationLevel+1,"grp_radx",m_strFieldValue_macro,nFieldSelection,eposGRP_RADX);

    // PLR.PGM_CHAR
    _CREATEFIELD_FROM_KCN_XML(m_u2GRP_CNT, m_kcnPGM_CHAR, eposPGM_CHAR);
    _STR_ADDLIST_XML(strXmlString,m_u2GRP_CNT,nIndentationLevel+1,"pgm_char",m_strFieldValue_macro,nFieldSelection,eposPGM_CHAR);

    // PLR.RTN_CHAR
    _CREATEFIELD_FROM_KCN_XML(m_u2GRP_CNT, m_kcnRTN_CHAR, eposRTN_CHAR);
    _STR_ADDLIST_XML(strXmlString,m_u2GRP_CNT,nIndentationLevel+1,"rtn_char",m_strFieldValue_macro,nFieldSelection,eposRTN_CHAR);

    // PLR.PGM_CHAL
    _CREATEFIELD_FROM_KCN_XML(m_u2GRP_CNT, m_kcnPGM_CHAL, eposPGM_CHAL);
    _STR_ADDLIST_XML(strXmlString,m_u2GRP_CNT,nIndentationLevel+1,"pgm_chal",m_strFieldValue_macro,nFieldSelection,eposPGM_CHAL);

    // PLR.RTN_CHAL
    _CREATEFIELD_FROM_KCN_XML(m_u2GRP_CNT, m_kcnRTN_CHAL, eposRTN_CHAL);
    _STR_ADDLIST_XML(strXmlString,m_u2GRP_CNT,nIndentationLevel+1,"rtn_chal",m_strFieldValue_macro,nFieldSelection,eposRTN_CHAL);

    strXmlString += strTabs;
    strXmlString += "</plr>\n";
}

void Stdf_PLR_V4::GetAtdfString(QString & strAtdfString)
{
    // Reset field index, and line Nb
    m_uiFieldIndex = 0;
    m_uiLineNb = 1;

    // Init string
    strAtdfString = "PLR:";

    // PLR.GRP_INDX
    _CREATEFIELD_FROM_KUi_ATDF(m_u2GRP_CNT, m_ku2GRP_INDX, eposGRP_INDX);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposGRP_INDX);

    // PLR.GRP_MODE
    _CREATEFIELD_FROM_KUi_ATDF(m_u2GRP_CNT, m_ku2GRP_MODE, eposGRP_MODE);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposGRP_MODE);

    // PLR.GRP_RADX
    _CREATEFIELD_FROM_KUi_ATDF_PLR_GRP_RADX(m_u2GRP_CNT, m_ku1GRP_RADX, eposGRP_RADX);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposGRP_RADX);

    // PLR.PGM_CHAL/PGM_CHAR
    QStringList qslCharList, qslChalList;
    m_strFieldValue_macro.clear();
    _CONVERT_kCN_TO_STRINGLIST(m_u2GRP_CNT, m_kcnPGM_CHAR, qslCharList, eposPGM_CHAR);
    _CONVERT_kCN_TO_STRINGLIST(m_u2GRP_CNT, m_kcnPGM_CHAL, qslChalList, eposPGM_CHAL);
    _CONVERT_CHAR_CHAL_STRINGLISTS_TO_ATDF(qslCharList, qslChalList, m_strFieldValue_macro);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposPGM_CHAR);

    // PLR.RTN_CHAL/RTN_CHAR
    m_strFieldValue_macro.clear();
    _CONVERT_kCN_TO_STRINGLIST(m_u2GRP_CNT, m_kcnRTN_CHAR, qslCharList, eposRTN_CHAR);
    _CONVERT_kCN_TO_STRINGLIST(m_u2GRP_CNT, m_kcnRTN_CHAL, qslChalList, eposRTN_CHAL);
    _CONVERT_CHAR_CHAL_STRINGLISTS_TO_ATDF(qslCharList, qslChalList, m_strFieldValue_macro);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposRTN_CHAR);

    _STR_FINISHFIELD_ATDF(strAtdfString);
}

///////////////////////////////////////////////////////////
// RDR RECORD
///////////////////////////////////////////////////////////
Stdf_RDR_V4::Stdf_RDR_V4() : Stdf_Record()
{
    m_ku2RTST_BIN = NULL;
    Reset();
}

Stdf_RDR_V4::~Stdf_RDR_V4()
{
    Reset();
}

void Stdf_RDR_V4::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Select fields for reduced list

    // Reset Data
    m_u2NUM_BINS = 0;				// RDR.NUM_BINS

    if(m_ku2RTST_BIN != NULL)		// RDR.RTST_BIN
    {
        delete [] m_ku2RTST_BIN;
        m_ku2RTST_BIN = NULL;
    }

    // Call Reset base method
    Stdf_Record::Reset();
}

bool Stdf_RDR_V4::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;

    return false;
}

QString Stdf_RDR_V4::GetRecordShortName(void)
{
    return "RDR";
}

QString Stdf_RDR_V4::GetRecordLongName(void)
{
    return "Retest Data Record";
}

int Stdf_RDR_V4::GetRecordType(void)
{
    return Rec_RDR;
}

bool Stdf_RDR_V4::Read(GS::StdLib::Stdf & clStdf)
{
    int				wData;
    unsigned int	i;

    // First reset data
    Reset();

    // RDR.NUM_BINS
    _FIELD_CHECKREAD(clStdf.ReadWord(&wData), eposNUM_BINS);
    _FIELD_SET(m_u2NUM_BINS = stdf_type_u2(wData), true, eposNUM_BINS);

    // RDR.RTST_BIN
    // Set the field as present, even if count is 0. We use a dummy instruction as parameter 1 to avoid warning under gcc.
    _FIELD_SET(m_uiIndex_macro = 0, true, eposRTST_BIN);
    if(m_u2NUM_BINS > 0)
        m_ku2RTST_BIN = new stdf_type_u2[m_u2NUM_BINS];
    for(i=0; i<(unsigned int)m_u2NUM_BINS; i++)
    {
        _FIELD_CHECKREAD(clStdf.ReadWord(&wData), eposRTST_BIN);
        _FIELD_SET(m_ku2RTST_BIN[i] = stdf_type_u2(wData), true, eposRTST_BIN);
    }

    return true;
}

bool Stdf_RDR_V4::Write(GS::StdLib::Stdf & clStdf)
{
    GS::StdLib::StdfRecordReadInfo	RecordReadInfo;
    unsigned int		i;

    RecordReadInfo.iRecordType = STDF_RDR_TYPE;
    RecordReadInfo.iRecordSubType = STDF_RDR_STYPE;
    clStdf.WriteHeader(&RecordReadInfo);

    // RDR.NUM_BINS
    _FIELD_CHECKWRITE(clStdf.WriteWord(WORD(m_u2NUM_BINS)), eposNUM_BINS, clStdf.WriteRecord());

    // RDR.RTST_BIN
    for(i=0; i<(unsigned int)m_u2NUM_BINS; i++)
    _FIELD_CHECKWRITE(clStdf.WriteWord(WORD(m_ku2RTST_BIN[i])), eposRTST_BIN, clStdf.WriteRecord());

    clStdf.WriteRecord();

    return true;
}

void Stdf_RDR_V4::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    // Empty string first
    strAsciiString = "";

    // RDR.NUM_BINS
    _STR_ADDFIELD_ASCII(strAsciiString, "RDR.num_bins", QString::number(m_u2NUM_BINS), nFieldSelection, eposNUM_BINS);

    // RDR.RTST_BIN
    _CREATEFIELD_FROM_KUi_ASCII(m_u2NUM_BINS, m_ku2RTST_BIN, eposRTST_BIN);
    _STR_ADDFIELD_ASCII(strAsciiString, "RDR.rtst_bin", m_strFieldValue_macro, nFieldSelection, eposRTST_BIN);
}

void Stdf_RDR_V4::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
    // Empty string list first
    listFields.empty();

    // RDR.NUM_BINS
    _LIST_ADDFIELD_ASCII(listFields, "RDR.num_bins", QString::number(m_u2NUM_BINS), nFieldSelection, eposNUM_BINS);

    // RDR.RTST_BIN
    _CREATEFIELD_FROM_KUi_ASCII(m_u2NUM_BINS, m_ku2RTST_BIN, eposRTST_BIN);
    _LIST_ADDFIELD_ASCII(listFields, "RDR.rtst_bin", m_strFieldValue_macro, nFieldSelection, eposRTST_BIN);
}

void Stdf_RDR_V4::GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection /*= 0*/)
{
    QString strTabs="";

    // Init XML string
    for(int nIndex=nIndentationLevel; nIndex>0; nIndex--)
        strTabs += "\t";
    strXmlString = strTabs;
    strXmlString += "<rdr>\n";

    // RDR.RTST_BIN
    _CREATEFIELD_FROM_KUi_XML(m_u2NUM_BINS, m_ku2RTST_BIN, eposRTST_BIN);
    _STR_ADDLIST_XML(strXmlString, m_u2NUM_BINS, nIndentationLevel+1, "rtst_bin", m_strFieldValue_macro, nFieldSelection, eposRTST_BIN);

    strXmlString += strTabs;
    strXmlString += "</rdr>\n";
}

void Stdf_RDR_V4::GetAtdfString(QString & strAtdfString)
{
    // Reset field index, and line Nb
    m_uiFieldIndex = 0;
    m_uiLineNb = 1;

    // Init string
    strAtdfString = "RDR:";

    // RDR.RTST_BIN
    _CREATEFIELD_FROM_KUi_ATDF(m_u2NUM_BINS, m_ku2RTST_BIN, eposRTST_BIN);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposRTST_BIN);

    _STR_FINISHFIELD_ATDF(strAtdfString);
}

///////////////////////////////////////////////////////////
// SDR RECORD
///////////////////////////////////////////////////////////
Stdf_SDR_V4::Stdf_SDR_V4() : Stdf_Record()
{
    m_ku1SITE_NUM = NULL;
    Reset();
}

// -------------------------------------------------------------------------- //
// Copy constructor
// -------------------------------------------------------------------------- //
Stdf_SDR_V4::Stdf_SDR_V4(const Stdf_SDR_V4& cssv4Other) : Stdf_Record(cssv4Other)
{
    m_ku1SITE_NUM = NULL;
    Reset();
    *this = cssv4Other;
}

Stdf_SDR_V4& Stdf_SDR_V4::operator=(const Stdf_SDR_V4& cssv4Other)
{
    if (this != &cssv4Other)
    {
        Reset();

        Stdf_Record::operator =(cssv4Other);

        SetHEAD_NUM(cssv4Other.m_u1HEAD_NUM);
        SetSITE_GRP(cssv4Other.m_u1SITE_GRP);
        SetSITE_CNT(cssv4Other.m_u1SITE_CNT);       // reallocate memory
        for(int ii=0; ii<m_u1SITE_CNT; ii++)
            SetSITE_NUM( ii , cssv4Other.m_ku1SITE_NUM[ii]);
        SetHAND_TYP(cssv4Other.m_cnHAND_TYP);
        SetHAND_ID(cssv4Other.m_cnHAND_ID);
        SetCARD_TYP(cssv4Other.m_cnCARD_TYP);
        SetCARD_ID(cssv4Other.m_cnCARD_ID);
        SetLOAD_TYP(cssv4Other.m_cnLOAD_TYP);
        SetLOAD_ID(cssv4Other.m_cnLOAD_ID);
        SetDIB_TYP(cssv4Other.m_cnDIB_TYP);
        SetDIB_ID(cssv4Other.m_cnDIB_ID);
        SetCABL_TYP(cssv4Other.m_cnCABL_TYP);
        SetCABL_ID(cssv4Other.m_cnCABL_ID);
        SetCONT_TYP(cssv4Other.m_cnCONT_TYP);
        SetCONT_ID( cssv4Other.m_cnCONT_ID);
        SetLASR_TYP(cssv4Other.m_cnLASR_TYP);
        SetLASR_ID(cssv4Other.m_cnLASR_ID);
        SetEXTR_TYP(cssv4Other.m_cnEXTR_TYP);
        SetEXTR_ID(cssv4Other.m_cnEXTR_ID);
    }

    return *this;
}



Stdf_SDR_V4::~Stdf_SDR_V4()
{
    Reset();
}

void Stdf_SDR_V4::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Select fields for reduced list

    // Reset Data
    m_u1HEAD_NUM	= 0;		// SDR.HEAD_NUM
    m_u1SITE_GRP	= 0;		// SDR.SITE_GRP
    m_u1SITE_CNT	= 0;		// SDR.SITE_CNT

    if(m_ku1SITE_NUM != NULL)	// SDR.SITE_NUM
    {
        delete [] m_ku1SITE_NUM;
        m_ku1SITE_NUM = NULL;
    }

    m_cnHAND_TYP	= "";		// SDR.HAND_TYP
    m_cnHAND_ID		= "";		// SDR.HAND_ID
    m_cnCARD_TYP	= "";		// SDR.CARD_TYP
    m_cnCARD_ID		= "";		// SDR.CARD_ID
    m_cnLOAD_TYP	= "";		// SDR.LOAD_TYP
    m_cnLOAD_ID		= "";		// SDR.LOAD_ID
    m_cnDIB_TYP		= "";		// SDR.DIB_TYP
    m_cnDIB_ID		= "";		// SDR.DIB_ID
    m_cnCABL_TYP	= "";		// SDR.CABL_TYP
    m_cnCABL_ID		= "";		// SDR.CABL_ID
    m_cnCONT_TYP	= "";		// SDR.CONT_TYP
    m_cnCONT_ID		= "";		// SDR.CONT_ID
    m_cnLASR_TYP	= "";		// SDR.LASR_TYP
    m_cnLASR_ID		= "";		// SDR.LASR_ID
    m_cnEXTR_TYP	= "";		// SDR.EXTR_TYP
    m_cnEXTR_ID		= "";		// SDR.EXTR_ID

    // Call Reset base method
    Stdf_Record::Reset();
}

bool Stdf_SDR_V4::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;

    return false;
}

QString Stdf_SDR_V4::GetRecordShortName(void)
{
    return "SDR";
}

QString Stdf_SDR_V4::GetRecordLongName(void)
{
    return "Site Description Record";
}

int Stdf_SDR_V4::GetRecordType(void)
{
    return Rec_SDR;
}

bool Stdf_SDR_V4::Read(GS::StdLib::Stdf & clStdf)
{
    char			szString[STDF_MAX_U1+1];
    BYTE			bData;
    unsigned int	i;

    // First reset data
    Reset();

    // SDR.HEAD_NUM
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposHEAD_NUM);
    _FIELD_SET(m_u1HEAD_NUM = stdf_type_u1(bData), true, eposHEAD_NUM);

    // SDR.SITE_GRP
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposSITE_GRP);
    _FIELD_SET(m_u1SITE_GRP = stdf_type_u1(bData), true, eposSITE_GRP);

    // SDR.SITE_CNT
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposSITE_CNT);
    _FIELD_SET(m_u1SITE_CNT = stdf_type_u1(bData), true, eposSITE_CNT);

    // SDR.SITE_NUM
    // Set the field as present, even if count is 0. We use a dummy instruction as parameter 1 to avoid warning under gcc.
    _FIELD_SET(m_uiIndex_macro = 0, true, eposSITE_NUM);
    if(m_u1SITE_CNT > 0)
        m_ku1SITE_NUM = new stdf_type_u1[m_u1SITE_CNT];
    for(i=0; i<(unsigned int)m_u1SITE_CNT; i++)
    {
        _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposSITE_NUM);
        _FIELD_SET(m_ku1SITE_NUM[i] = stdf_type_u1(bData), true, eposSITE_NUM);
    }

    // SDR.HAND_TYP
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposHAND_TYP);
    _FIELD_SET(m_cnHAND_TYP = szString, !m_cnHAND_TYP.isEmpty(), eposHAND_TYP);

    // SDR.HAND_ID
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposHAND_ID);
    _FIELD_SET(m_cnHAND_ID = szString, !m_cnHAND_ID.isEmpty(), eposHAND_ID);

    // SDR.CARD_TYP
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposCARD_TYP);
    _FIELD_SET(m_cnCARD_TYP = szString, !m_cnCARD_TYP.isEmpty(), eposCARD_TYP);

    // SDR.CARD_ID
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposCARD_ID);
    _FIELD_SET(m_cnCARD_ID = szString, !m_cnCARD_ID.isEmpty(), eposCARD_ID);

    // SDR.LOAD_TYP
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposLOAD_TYP);
    _FIELD_SET(m_cnLOAD_TYP = szString, !m_cnLOAD_TYP.isEmpty(), eposLOAD_TYP);

    // SDR.LOAD_ID
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposLOAD_ID);
    _FIELD_SET(m_cnLOAD_ID = szString, !m_cnLOAD_ID.isEmpty(), eposLOAD_ID);

    // SDR.DIB_TYP
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposDIB_TYP);
    _FIELD_SET(m_cnDIB_TYP = szString, !m_cnDIB_TYP.isEmpty(), eposDIB_TYP);

    // SDR.DIB_ID
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposDIB_ID);
    _FIELD_SET(m_cnDIB_ID = szString, !m_cnDIB_ID.isEmpty(), eposDIB_ID);

    // SDR.CABL_TYP
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposCABL_TYP);
    _FIELD_SET(m_cnCABL_TYP = szString, !m_cnCABL_TYP.isEmpty(), eposCABL_TYP);

    // SDR.CABL_ID
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposCABL_ID);
    _FIELD_SET(m_cnCABL_ID = szString, !m_cnCABL_ID.isEmpty(), eposCABL_ID);

    // SDR.CONT_TYP
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposCONT_TYP);
    _FIELD_SET(m_cnCONT_TYP = szString, !m_cnCONT_TYP.isEmpty(), eposCONT_TYP);

    // SDR.CONT_ID
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposCONT_ID);
    _FIELD_SET(m_cnCONT_ID = szString, !m_cnCONT_ID.isEmpty(), eposCONT_ID);

    // SDR.LASR_TYP
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposLASR_TYP);
    _FIELD_SET(m_cnLASR_TYP = szString, !m_cnLASR_TYP.isEmpty(), eposLASR_TYP);

    // SDR.LASR_ID
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposLASR_ID);
    _FIELD_SET(m_cnLASR_ID = szString, !m_cnLASR_ID.isEmpty(), eposLASR_ID);

    // SDR.EXTR_TYP
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposEXTR_TYP);
    _FIELD_SET(m_cnEXTR_TYP = szString, !m_cnEXTR_TYP.isEmpty(), eposEXTR_TYP);

    // SDR.EXTR_ID
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposEXTR_ID);
    _FIELD_SET(m_cnEXTR_ID = szString, !m_cnEXTR_ID.isEmpty(), eposEXTR_ID);

    return true;
}

bool Stdf_SDR_V4::Write(GS::StdLib::Stdf & clStdf)
{
    GS::StdLib::StdfRecordReadInfo	RecordReadInfo;
    unsigned int		i;

    RecordReadInfo.iRecordType = STDF_SDR_TYPE;
    RecordReadInfo.iRecordSubType = STDF_SDR_STYPE;
    clStdf.WriteHeader(&RecordReadInfo);

    // SDR.HEAD_NUM
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_u1HEAD_NUM)), eposHEAD_NUM, clStdf.WriteRecord());

    // SDR.SITE_GRP
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_u1SITE_GRP)), eposSITE_GRP, clStdf.WriteRecord());

    // SDR.SITE_CNT
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_u1SITE_CNT)), eposSITE_CNT, clStdf.WriteRecord());

    // SDR.SITE_NUM
    for(i=0; i<(unsigned int)m_u1SITE_CNT; i++)
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_ku1SITE_NUM[i])), eposSITE_NUM, clStdf.WriteRecord());

    // SDR.HAND_TYP
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnHAND_TYP.toLatin1().constData()), eposHAND_TYP, clStdf.WriteRecord());

    // SDR.HAND_ID
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnHAND_ID.toLatin1().constData()), eposHAND_ID, clStdf.WriteRecord());

    // SDR.CARD_TYP
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnCARD_TYP.toLatin1().constData()), eposCARD_TYP, clStdf.WriteRecord());

    // SDR.CARD_ID
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnCARD_ID.toLatin1().constData()), eposCARD_ID, clStdf.WriteRecord());

    // SDR.LOAD_TYP
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnLOAD_TYP.toLatin1().constData()), eposLOAD_TYP, clStdf.WriteRecord());

    // SDR.LOAD_ID
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnLOAD_ID.toLatin1().constData()), eposLOAD_ID, clStdf.WriteRecord());

    // SDR.DIB_TYP
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnDIB_TYP.toLatin1().constData()), eposDIB_TYP, clStdf.WriteRecord());

    // SDR.DIB_ID
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnDIB_ID.toLatin1().constData()), eposDIB_ID, clStdf.WriteRecord());

    // SDR.CABL_TYP
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnCABL_TYP.toLatin1().constData()), eposCABL_TYP, clStdf.WriteRecord());

    // SDR.CABL_ID
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnCABL_ID.toLatin1().constData()), eposCABL_ID, clStdf.WriteRecord());

    // SDR.CONT_TYP
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnCONT_TYP.toLatin1().constData()), eposCONT_TYP, clStdf.WriteRecord());

    // SDR.CONT_ID
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnCONT_ID.toLatin1().constData()), eposCONT_ID, clStdf.WriteRecord());

    // SDR.LASR_TYP
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnLASR_TYP.toLatin1().constData()), eposLASR_TYP, clStdf.WriteRecord());

    // SDR.LASR_ID
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnLASR_ID.toLatin1().constData()), eposLASR_ID, clStdf.WriteRecord());

    // SDR.EXTR_TYP
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnEXTR_TYP.toLatin1().constData()), eposEXTR_TYP, clStdf.WriteRecord());

    // SDR.EXTR_ID
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnEXTR_ID.toLatin1().constData()), eposEXTR_ID, clStdf.WriteRecord());

    clStdf.WriteRecord();

    return true;
}

void Stdf_SDR_V4::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    // Empty string first
    strAsciiString = "";

    // SDR.HEAD_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "SDR.head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // SDR.SITE_GRP
    _STR_ADDFIELD_ASCII(strAsciiString, "SDR.site_grp", QString::number(m_u1SITE_GRP), nFieldSelection, eposSITE_GRP);

    // SDR.SITE_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "SDR.site_cnt", QString::number(m_u1SITE_CNT), nFieldSelection, eposSITE_CNT);

    // SDR.SITE_NUM
    _CREATEFIELD_FROM_KUi_ASCII(m_u1SITE_CNT, m_ku1SITE_NUM, eposSITE_NUM);
    _STR_ADDFIELD_ASCII(strAsciiString, "SDR.site_num", m_strFieldValue_macro, nFieldSelection, eposSITE_NUM);

    // SDR.HAND_TYP
    _STR_ADDFIELD_ASCII(strAsciiString, "SDR.hand_typ", m_cnHAND_TYP, nFieldSelection, eposHAND_TYP);

    // SDR.HAND_ID
    _STR_ADDFIELD_ASCII(strAsciiString, "SDR.hand_id", m_cnHAND_ID, nFieldSelection, eposHAND_ID);

    // SDR.CARD_TYP
    _STR_ADDFIELD_ASCII(strAsciiString, "SDR.card_typ", m_cnCARD_TYP, nFieldSelection, eposCARD_TYP);

    // SDR.CARD_ID
    _STR_ADDFIELD_ASCII(strAsciiString, "SDR.card_id", m_cnCARD_ID, nFieldSelection, eposCARD_ID);

    // SDR.LOAD_TYP
    _STR_ADDFIELD_ASCII(strAsciiString, "SDR.load_typ", m_cnLOAD_TYP, nFieldSelection, eposLOAD_TYP);

    // SDR.LOAD_ID
    _STR_ADDFIELD_ASCII(strAsciiString, "SDR.load_id", m_cnLOAD_ID, nFieldSelection, eposLOAD_ID);

    // SDR.DIB_TYP
    _STR_ADDFIELD_ASCII(strAsciiString, "SDR.dib_typ", m_cnDIB_TYP, nFieldSelection, eposDIB_TYP);

    // SDR.DIB_ID
    _STR_ADDFIELD_ASCII(strAsciiString, "SDR.dib_id", m_cnDIB_ID, nFieldSelection, eposDIB_ID);

    // SDR.CABL_TYP
    _STR_ADDFIELD_ASCII(strAsciiString, "SDR.cabl_typ", m_cnCABL_TYP, nFieldSelection, eposCABL_TYP);

    // SDR.CABL_ID
    _STR_ADDFIELD_ASCII(strAsciiString, "SDR.cabl_id", m_cnCABL_ID, nFieldSelection, eposCABL_ID);

    // SDR.CONT_TYP
    _STR_ADDFIELD_ASCII(strAsciiString, "SDR.cont_typ", m_cnCONT_TYP, nFieldSelection, eposCONT_TYP);

    // SDR.CONT_ID
    _STR_ADDFIELD_ASCII(strAsciiString, "SDR.cont_id", m_cnCONT_ID, nFieldSelection, eposCONT_ID);

    // SDR.LASR_TYP
    _STR_ADDFIELD_ASCII(strAsciiString, "SDR.lasr_typ", m_cnLASR_TYP, nFieldSelection, eposLASR_TYP);

    // SDR.LASR_ID
    _STR_ADDFIELD_ASCII(strAsciiString, "SDR.lasr_id", m_cnLASR_ID, nFieldSelection, eposLASR_ID);

    // SDR.EXTR_TYP
    _STR_ADDFIELD_ASCII(strAsciiString, "SDR.extr_typ", m_cnEXTR_TYP, nFieldSelection, eposEXTR_TYP);

    // SDR.EXTR_ID
    _STR_ADDFIELD_ASCII(strAsciiString, "SDR.extr_id", m_cnEXTR_ID, nFieldSelection, eposEXTR_ID);
}

void Stdf_SDR_V4::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
    // Empty string list first
    listFields.empty();

    // SDR.HEAD_NUM
    _LIST_ADDFIELD_ASCII(listFields, "SDR.head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // SDR.SITE_GRP
    _LIST_ADDFIELD_ASCII(listFields, "SDR.site_grp", QString::number(m_u1SITE_GRP), nFieldSelection, eposSITE_GRP);

    // SDR.SITE_CNT
    _LIST_ADDFIELD_ASCII(listFields, "SDR.site_cnt", QString::number(m_u1SITE_CNT), nFieldSelection, eposSITE_CNT);

    // SDR.SITE_NUM
    _CREATEFIELD_FROM_KUi_ASCII(m_u1SITE_CNT, m_ku1SITE_NUM, eposSITE_NUM);
    _LIST_ADDFIELD_ASCII(listFields, "SDR.site_num", m_strFieldValue_macro, nFieldSelection, eposSITE_NUM);

    // SDR.HAND_TYP
    _LIST_ADDFIELD_ASCII(listFields, "SDR.hand_typ", m_cnHAND_TYP, nFieldSelection, eposHAND_TYP);

    // SDR.HAND_ID
    _LIST_ADDFIELD_ASCII(listFields, "SDR.hand_id", m_cnHAND_ID, nFieldSelection, eposHAND_ID);

    // SDR.CARD_TYP
    _LIST_ADDFIELD_ASCII(listFields, "SDR.card_typ", m_cnCARD_TYP, nFieldSelection, eposCARD_TYP);

    // SDR.CARD_ID
    _LIST_ADDFIELD_ASCII(listFields, "SDR.card_id", m_cnCARD_ID, nFieldSelection, eposCARD_ID);

    // SDR.LOAD_TYP
    _LIST_ADDFIELD_ASCII(listFields, "SDR.load_typ", m_cnLOAD_TYP, nFieldSelection, eposLOAD_TYP);

    // SDR.LOAD_ID
    _LIST_ADDFIELD_ASCII(listFields, "SDR.load_id", m_cnLOAD_ID, nFieldSelection, eposLOAD_ID);

    // SDR.DIB_TYP
    _LIST_ADDFIELD_ASCII(listFields, "SDR.dib_typ", m_cnDIB_TYP, nFieldSelection, eposDIB_TYP);

    // SDR.DIB_ID
    _LIST_ADDFIELD_ASCII(listFields, "SDR.dib_id", m_cnDIB_ID, nFieldSelection, eposDIB_ID);

    // SDR.CABL_TYP
    _LIST_ADDFIELD_ASCII(listFields, "SDR.cabl_typ", m_cnCABL_TYP, nFieldSelection, eposCABL_TYP);

    // SDR.CABL_ID
    _LIST_ADDFIELD_ASCII(listFields, "SDR.cabl_id", m_cnCABL_ID, nFieldSelection, eposCABL_ID);

    // SDR.CONT_TYP
    _LIST_ADDFIELD_ASCII(listFields, "SDR.cont_typ", m_cnCONT_TYP, nFieldSelection, eposCONT_TYP);

    // SDR.CONT_ID
    _LIST_ADDFIELD_ASCII(listFields, "SDR.cont_id", m_cnCONT_ID, nFieldSelection, eposCONT_ID);

    // SDR.LASR_TYP
    _LIST_ADDFIELD_ASCII(listFields, "SDR.lasr_typ", m_cnLASR_TYP, nFieldSelection, eposLASR_TYP);

    // SDR.LASR_ID
    _LIST_ADDFIELD_ASCII(listFields, "SDR.lasr_id", m_cnLASR_ID, nFieldSelection, eposLASR_ID);

    // SDR.EXTR_TYP
    _LIST_ADDFIELD_ASCII(listFields, "SDR.extr_typ", m_cnEXTR_TYP, nFieldSelection, eposEXTR_TYP);

    // SDR.EXTR_ID
    _LIST_ADDFIELD_ASCII(listFields, "SDR.extr_id", m_cnEXTR_ID, nFieldSelection, eposEXTR_ID);
}

void Stdf_SDR_V4::GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection /*= 0*/)
{
    QString strTabs="";

    // Init XML string
    for(int nIndex=nIndentationLevel; nIndex>0; nIndex--)
        strTabs += "\t";
    strXmlString = strTabs;
    strXmlString += "<sdr>\n";

    // SDR.HEAD_NUM
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // SDR.SITE_GRP
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "site_grp", QString::number(m_u1SITE_GRP), nFieldSelection, eposSITE_GRP);

    // SDR.SITE_NUM
    _CREATEFIELD_FROM_KUi_XML(m_u1SITE_CNT, m_ku1SITE_NUM, eposSITE_NUM);
    _STR_ADDLIST_XML(strXmlString, m_u1SITE_CNT, nIndentationLevel+1, "site_num", m_strFieldValue_macro, nFieldSelection, eposSITE_NUM);

    // SDR.HAND_TYP
    _CREATEFIELD_FROM_CN_XML(m_cnHAND_TYP);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "hand_typ", m_strFieldValue_macro, nFieldSelection, eposHAND_TYP);

    // SDR.HAND_ID
    _CREATEFIELD_FROM_CN_XML(m_cnHAND_ID);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "hand_id", m_strFieldValue_macro, nFieldSelection, eposHAND_ID);

    // SDR.CARD_TYP
    _CREATEFIELD_FROM_CN_XML(m_cnCARD_TYP);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "card_typ", m_strFieldValue_macro, nFieldSelection, eposCARD_TYP);

    // SDR.CARD_ID
    _CREATEFIELD_FROM_CN_XML(m_cnCARD_ID);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "card_id", m_strFieldValue_macro, nFieldSelection, eposCARD_ID);

    // SDR.LOAD_TYP
    _CREATEFIELD_FROM_CN_XML(m_cnLOAD_TYP);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "load_typ", m_strFieldValue_macro, nFieldSelection, eposLOAD_TYP);

    // SDR.LOAD_ID
    _CREATEFIELD_FROM_CN_XML(m_cnLOAD_ID);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "load_id", m_strFieldValue_macro, nFieldSelection, eposLOAD_ID);

    // SDR.DIB_TYP
    _CREATEFIELD_FROM_CN_XML(m_cnDIB_TYP);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "dib_typ", m_strFieldValue_macro, nFieldSelection, eposDIB_TYP);

    // SDR.DIB_ID
    _CREATEFIELD_FROM_CN_XML(m_cnDIB_ID);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "dib_id", m_strFieldValue_macro, nFieldSelection, eposDIB_ID);

    // SDR.CABL_TYP
    _CREATEFIELD_FROM_CN_XML(m_cnCABL_TYP);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "cabl_typ", m_strFieldValue_macro, nFieldSelection, eposCABL_TYP);

    // SDR.CABL_ID
    _CREATEFIELD_FROM_CN_XML(m_cnCABL_ID);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "cabl_id", m_strFieldValue_macro, nFieldSelection, eposCABL_ID);

    // SDR.CONT_TYP
    _CREATEFIELD_FROM_CN_XML(m_cnCONT_TYP);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "cont_typ", m_strFieldValue_macro, nFieldSelection, eposCONT_TYP);

    // SDR.CONT_ID
    _CREATEFIELD_FROM_CN_XML(m_cnCONT_ID);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "cont_id", m_strFieldValue_macro, nFieldSelection, eposCONT_ID);

    // SDR.LASR_TYP
    _CREATEFIELD_FROM_CN_XML(m_cnLASR_TYP);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "lasr_typ", m_strFieldValue_macro, nFieldSelection, eposLASR_TYP);

    // SDR.LASR_ID
    _CREATEFIELD_FROM_CN_XML(m_cnLASR_ID);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "lasr_id", m_strFieldValue_macro, nFieldSelection, eposLASR_ID);

    // SDR.EXTR_TYP
    _CREATEFIELD_FROM_CN_XML(m_cnEXTR_TYP);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "extr_typ", m_strFieldValue_macro, nFieldSelection, eposEXTR_TYP);

    // SDR.EXTR_ID
    _CREATEFIELD_FROM_CN_XML(m_cnEXTR_ID);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "extr_id", m_strFieldValue_macro, nFieldSelection, eposEXTR_ID);

    strXmlString += strTabs;
    strXmlString += "</sdr>\n";
}

void Stdf_SDR_V4::GetAtdfString(QString & strAtdfString)
{
    // Reset field index, and line Nb
    m_uiFieldIndex = 0;
    m_uiLineNb = 1;

    // Init string
    strAtdfString = "SDR:";

    // SDR.HEAD_NUM
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u1HEAD_NUM), eposHEAD_NUM);

    // SDR.SITE_GRP
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u1SITE_GRP), eposSITE_GRP);

    // SDR.SITE_NUM
    _CREATEFIELD_FROM_KUi_ATDF(m_u1SITE_CNT, m_ku1SITE_NUM, eposSITE_NUM);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposSITE_NUM);

    // SDR.HAND_TYP
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnHAND_TYP, eposHAND_TYP);

    // SDR.HAND_ID
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnHAND_ID, eposHAND_ID);

    // SDR.CARD_TYP
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnCARD_TYP, eposCARD_TYP);

    // SDR.CARD_ID
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnCARD_ID, eposCARD_ID);

    // SDR.LOAD_TYP
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnLOAD_TYP, eposLOAD_TYP);

    // SDR.LOAD_ID
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnLOAD_ID, eposLOAD_ID);

    // SDR.DIB_TYP
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnDIB_TYP, eposDIB_TYP);

    // SDR.DIB_ID
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnDIB_ID, eposDIB_ID);

    // SDR.CABL_TYP
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnCABL_TYP, eposCABL_TYP);

    // SDR.CABL_ID
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnCABL_ID, eposCABL_ID);

    // SDR.CONT_TYP
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnCONT_TYP, eposCONT_TYP);

    // SDR.CONT_ID
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnCONT_ID, eposCONT_ID);

    // SDR.LASR_TYP
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnLASR_TYP, eposLASR_TYP);

    // SDR.LASR_ID
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnLASR_ID, eposLASR_ID);

    // SDR.EXTR_TYP
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnEXTR_TYP, eposEXTR_TYP);

    // SDR.EXTR_ID
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnEXTR_ID, eposEXTR_ID);

    _STR_FINISHFIELD_ATDF(strAtdfString);
}

///////////////////////////////////////////////////////////
// WIR RECORD
///////////////////////////////////////////////////////////
Stdf_WIR_V4::Stdf_WIR_V4() : Stdf_Record()
{
    Reset();
}

Stdf_WIR_V4::~Stdf_WIR_V4()
{
    Reset();
}

void Stdf_WIR_V4::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Select fields for reduced list

    // Reset Data
    m_u1HEAD_NUM	= 0;		// WIR.HEAD_NUM
    m_u1SITE_GRP	= 0;		// WIR.SITE_GRP
    m_u4START_T		= 0;		// WIR.START_T
    m_cnWAFER_ID	= "";		// WIR.WAFER_ID

    // Call Reset base method
    Stdf_Record::Reset();
}

bool Stdf_WIR_V4::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;

    return false;
}

QString Stdf_WIR_V4::GetRecordShortName(void)
{
    return "WIR";
}

QString Stdf_WIR_V4::GetRecordLongName(void)
{
    return "Wafer Information Record";
}

int Stdf_WIR_V4::GetRecordType(void)
{
    return Rec_WIR;
}

bool Stdf_WIR_V4::Read(GS::StdLib::Stdf & clStdf)
{
    char	szString[STDF_MAX_U1+1];
    long	dwData;
    BYTE	bData;

    // First reset data
    Reset();

    // WIR.HEAD_NUM
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposHEAD_NUM);
    _FIELD_SET(m_u1HEAD_NUM = stdf_type_u1(bData), true, eposHEAD_NUM);

    // WIR.SITE_GRP
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposSITE_GRP);
    _FIELD_SET(m_u1SITE_GRP = stdf_type_u1(bData), m_u1SITE_GRP != 255, eposSITE_GRP);

    // WIR.START_T
    _FIELD_CHECKREAD(clStdf.ReadDword(&dwData), eposSTART_T);
    _FIELD_SET(m_u4START_T = stdf_type_u4(dwData), true, eposSTART_T);

    // WIR.WAFER_ID
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposWAFER_ID);
    _FIELD_SET(m_cnWAFER_ID = szString, !m_cnWAFER_ID.isEmpty(), eposWAFER_ID);

    return true;
}

bool Stdf_WIR_V4::Write(GS::StdLib::Stdf & clStdf)
{
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;

    RecordReadInfo.iRecordType = STDF_WIR_TYPE;
    RecordReadInfo.iRecordSubType = STDF_WIR_STYPE;
    clStdf.WriteHeader(&RecordReadInfo);

    // WIR.HEAD_NUM
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_u1HEAD_NUM)), eposHEAD_NUM, clStdf.WriteRecord());

    // WIR.SITE_GRP
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_u1SITE_GRP)), eposSITE_GRP, clStdf.WriteRecord());

    // WIR.START_T
    _FIELD_CHECKWRITE(clStdf.WriteDword(DWORD(m_u4START_T)), eposSTART_T, clStdf.WriteRecord());

    // WIR.WAFER_ID
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnWAFER_ID.toLatin1().constData()), eposWAFER_ID, clStdf.WriteRecord());

    clStdf.WriteRecord();

    return true;
}

void Stdf_WIR_V4::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    QString		strString;
    QDateTime	clDateTime;
    time_t		tDateTime;

    // Set QDateTime object to UTC mode
    clDateTime.setTimeSpec(Qt::UTC);

    // Empty string first
    strAsciiString = "";

    // WIR.HEAD_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "WIR.head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // WIR.SITE_GRP
    _STR_ADDFIELD_ASCII(strAsciiString, "WIR.site_grp", QString::number(m_u1SITE_GRP), nFieldSelection, eposSITE_GRP);

    // WIR.START_T
    tDateTime = (time_t)m_u4START_T;
    clDateTime.setTime_t(tDateTime);
    m_strFieldValue_macro.sprintf("%s (0x%lx)",
                                  ( GS::StdLib::Stdf::GetDaysOfWeekString(clDateTime.date().dayOfWeek()-1)
                                    + clDateTime.toString(" dd ")
                                    + GS::StdLib::Stdf::GetMonthName(clDateTime.date().month()-1)
                                    + clDateTime.toString(" yyyy h:mm:ss") )
                                  .toLatin1().constData(), m_u4START_T);
    _STR_ADDFIELD_ASCII(strAsciiString, "WIR.start_t", m_strFieldValue_macro, nFieldSelection, eposSTART_T);

    // WIR.WAFER_ID
    _STR_ADDFIELD_ASCII(strAsciiString, "WIR.wafer_id", m_cnWAFER_ID, nFieldSelection, eposWAFER_ID);
}

void Stdf_WIR_V4::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
    QString		strString;
    QDateTime	clDateTime;
    time_t		tDateTime;

    // Set QDateTime object to UTC mode
    clDateTime.setTimeSpec(Qt::UTC);

    // Empty string list first
    listFields.empty();

    // WIR.HEAD_NUM
    _LIST_ADDFIELD_ASCII(listFields, "WIR.head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // WIR.SITE_GRP
    _LIST_ADDFIELD_ASCII(listFields, "WIR.site_grp", QString::number(m_u1SITE_GRP), nFieldSelection, eposSITE_GRP);

    // WIR.START_T
    tDateTime = (time_t)m_u4START_T;
    clDateTime.setTime_t(tDateTime);
    m_strFieldValue_macro.sprintf("%s (0x%lx)",
                                  ( GS::StdLib::Stdf::GetDaysOfWeekString(clDateTime.date().dayOfWeek()-1)
                                    + clDateTime.toString(" dd ")
                                    + GS::StdLib::Stdf::GetMonthName(clDateTime.date().month()-1)
                                    + clDateTime.toString(" yyyy h:mm:ss") )
                                  .toLatin1().constData(), m_u4START_T);
    _LIST_ADDFIELD_ASCII(listFields, "WIR.start_t", m_strFieldValue_macro, nFieldSelection, eposSTART_T);

    // WIR.WAFER_ID
    _LIST_ADDFIELD_ASCII(listFields, "WIR.wafer_id", m_cnWAFER_ID, nFieldSelection, eposWAFER_ID);
}

void Stdf_WIR_V4::GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection /*= 0*/)
{
    QString strTabs="";

    // Init XML string
    for(int nIndex=nIndentationLevel; nIndex>0; nIndex--)
        strTabs += "\t";
    strXmlString = strTabs;
    strXmlString += "<wir>\n";

    // WIR.HEAD_NUM
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // WIR.SITE_GRP
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "site_grp", QString::number(m_u1SITE_GRP), nFieldSelection, eposSITE_GRP);

    // WIR.START_T
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "start_t", QString::number(m_u4START_T), nFieldSelection, eposSTART_T);

    // WIR.WAFER_ID
    _CREATEFIELD_FROM_CN_XML(m_cnWAFER_ID);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "wafer_id", m_strFieldValue_macro, nFieldSelection, eposWAFER_ID);

    strXmlString += strTabs;
    strXmlString += "</wir>\n";
}

void Stdf_WIR_V4::GetAtdfString(QString & strAtdfString)
{
    QString		strString;
    QDateTime	clDateTime;
    time_t		tDateTime;

    // Set QDateTime object to UTC mode
    clDateTime.setTimeSpec(Qt::UTC);

    // Reset field index, and line Nb
    m_uiFieldIndex = 0;
    m_uiLineNb = 1;

    // Init string
    strAtdfString = "WIR:";

    // WIR.HEAD_NUM
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u1HEAD_NUM), eposHEAD_NUM);

    // WIR.START_T
    tDateTime = (time_t)m_u4START_T;
    clDateTime.setTime_t(tDateTime);
    QString strDate = clDateTime.toString("hh:mm:ss dd-MM-yyyy");
    strDate = strDate.replace("-01-","-Jan-").replace("-02-","-Feb-").replace("-03-","-Mar-").replace("-04-","-Apr-").replace("-05-","-May-").replace("-06-","-Jun-").replace("-07-","-Jul-").replace("-08-","-Aug-").replace("-09-","-Sep-").replace("-10-","-Oct-").replace("-11-","-Nov-").replace("-12-","-Dec-");
    m_strFieldValue_macro.sprintf("%s", strDate.toLatin1().constData());
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposSTART_T);

    // WIR.SITE_GRP
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u1SITE_GRP), eposSITE_GRP);

    // WIR.WAFER_ID
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnWAFER_ID, eposWAFER_ID);

    _STR_FINISHFIELD_ATDF(strAtdfString);
}

///////////////////////////////////////////////////////////
// WRR RECORD
///////////////////////////////////////////////////////////
Stdf_WRR_V4::Stdf_WRR_V4() : Stdf_Record()
{
    Reset();
}

Stdf_WRR_V4::Stdf_WRR_V4(const Stdf_WRR_V4 &other) : Stdf_Record(other)
{
    *this = other;
}

Stdf_WRR_V4::~Stdf_WRR_V4()
{
    Reset();
}

Stdf_WRR_V4 &Stdf_WRR_V4::operator=(const Stdf_WRR_V4 &other)
{
    if (this != &other)
    {
        // Clean up this object
        Reset();

        // Copy base class
        Stdf_Record::operator =(other);

        // Copy class members
        for(int i=0; i<eposEND; i++)
            m_pFieldFlags[i] = other.m_pFieldFlags[i];

        // Reset Data
        m_u1HEAD_NUM	= other.m_u1HEAD_NUM;		// WRR.HEAD_NUM
        m_u1SITE_GRP	= other.m_u1SITE_GRP;		// WRR.SITE_GRP
        m_u4FINISH_T	= other.m_u4FINISH_T;		// WRR.FINISH_T
        m_u4PART_CNT	= other.m_u4PART_CNT;		// WRR.PART_CNT
        m_u4RTST_CNT	= other.m_u4RTST_CNT;		// WRR.RTST_CNT
        m_u4ABRT_CNT	= other.m_u4ABRT_CNT;		// WRR.ABRT_CNT
        m_u4GOOD_CNT	= other.m_u4GOOD_CNT;		// WRR.GOOD_CNT
        m_u4FUNC_CNT	= other.m_u4FUNC_CNT;		// WRR.FUNC_CNT
        m_cnWAFER_ID	= other.m_cnWAFER_ID;		// WRR.WAFER_ID
        m_cnFABWF_ID	= other.m_cnFABWF_ID;		// WRR.FABWF_ID
        m_cnFRAME_ID	= other.m_cnFRAME_ID;		// WRR.FRAME_ID
        m_cnMASK_ID		= other.m_cnMASK_ID;		// WRR.MASK_ID
        m_cnUSR_DESC	= other.m_cnUSR_DESC;		// WRR.USR_DESC
        m_cnEXC_DESC	= other.m_cnEXC_DESC;		// WRR.EXC_DESC
    }

    return *this;
}

void Stdf_WRR_V4::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Select fields for reduced list

    // Reset Data
    m_u1HEAD_NUM	= 0;            // WRR.HEAD_NUM
    m_u1SITE_GRP	= 255;          // WRR.SITE_GRP
    m_u4FINISH_T	= 0;            // WRR.FINISH_T
    m_u4PART_CNT	= 0;            // WRR.PART_CNT
    m_u4RTST_CNT	= INVALID_INT;	// WRR.RTST_CNT
    m_u4ABRT_CNT	= INVALID_INT;	// WRR.ABRT_CNT
    m_u4GOOD_CNT	= INVALID_INT;	// WRR.GOOD_CNT
    m_u4FUNC_CNT	= INVALID_INT;	// WRR.FUNC_CNT
    m_cnWAFER_ID	= "";           // WRR.WAFER_ID
    m_cnFABWF_ID	= "";           // WRR.FABWF_ID
    m_cnFRAME_ID	= "";           // WRR.FRAME_ID
    m_cnMASK_ID		= "";           // WRR.MASK_ID
    m_cnUSR_DESC	= "";           // WRR.USR_DESC
    m_cnEXC_DESC	= "";           // WRR.EXC_DESC

    // Call Reset base method
    Stdf_Record::Reset();
}

bool Stdf_WRR_V4::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;

    return false;
}

void Stdf_WRR_V4::invalidateField(int nFieldPos){
    if((nFieldPos >= 0) && (nFieldPos < eposEND)){
        m_pFieldFlags[nFieldPos] &= !(FieldFlag_Valid);
        m_pFieldFlags[nFieldPos] |= FieldFlag_Present;
    }
}

QString Stdf_WRR_V4::GetRecordShortName(void)
{
    return "WRR";
}

QString Stdf_WRR_V4::GetRecordLongName(void)
{
    return "Wafer Results Record";
}

int Stdf_WRR_V4::GetRecordType(void)
{
    return Rec_WRR;
}

bool Stdf_WRR_V4::Read(GS::StdLib::Stdf & clStdf)
{
    char	szString[STDF_MAX_U1+1];
    long	dwData;
    BYTE	bData;

    // First reset data
    Reset();

    // WRR.HEAD_NUM
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposHEAD_NUM);
    _FIELD_SET(m_u1HEAD_NUM = stdf_type_u1(bData), true, eposHEAD_NUM);

    // WRR.SITE_GRP
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposSITE_GRP);
    _FIELD_SET(m_u1SITE_GRP = stdf_type_u1(bData), m_u1SITE_GRP != 255, eposSITE_GRP);

    // WRR.FINISH_T
    _FIELD_CHECKREAD(clStdf.ReadDword(&dwData), eposFINISH_T);
    _FIELD_SET(m_u4FINISH_T = stdf_type_u4(dwData), true, eposFINISH_T);

    // WRR.PART_CNT
    _FIELD_CHECKREAD(clStdf.ReadDword(&dwData), eposPART_CNT);
    _FIELD_SET(m_u4PART_CNT = stdf_type_u4(dwData), true, eposPART_CNT);

    // WRR.RTST_CNT
    _FIELD_CHECKREAD(clStdf.ReadDword(&dwData), eposRTST_CNT);
    _FIELD_SET(m_u4RTST_CNT = stdf_type_u4(dwData), m_u4RTST_CNT != 4294967295UL, eposRTST_CNT);

    // WRR.ABRT_CNT
    _FIELD_CHECKREAD(clStdf.ReadDword(&dwData), eposABRT_CNT);
    _FIELD_SET(m_u4ABRT_CNT = stdf_type_u4(dwData), m_u4ABRT_CNT != 4294967295UL, eposABRT_CNT);

    // WRR.GOOD_CNT
    _FIELD_CHECKREAD(clStdf.ReadDword(&dwData), eposGOOD_CNT);
    _FIELD_SET(m_u4GOOD_CNT = stdf_type_u4(dwData), m_u4GOOD_CNT != 4294967295UL, eposGOOD_CNT);

    // WRR.FUNC_CNT
    _FIELD_CHECKREAD(clStdf.ReadDword(&dwData), eposFUNC_CNT);
    _FIELD_SET(m_u4FUNC_CNT = stdf_type_u4(dwData), m_u4FUNC_CNT != 4294967295UL, eposFUNC_CNT);

    // WRR.WAFER_ID
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposWAFER_ID);
    _FIELD_SET(m_cnWAFER_ID = szString, !m_cnWAFER_ID.isEmpty(), eposWAFER_ID);

    // WRR.FABWF_ID
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposFABWF_ID);
    _FIELD_SET(m_cnFABWF_ID = szString, !m_cnFABWF_ID.isEmpty(), eposFABWF_ID);

    // WRR.FRAME_ID
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposFRAME_ID);
    _FIELD_SET(m_cnFRAME_ID = szString, !m_cnFRAME_ID.isEmpty(), eposFRAME_ID);

    // WRR.MASK_ID
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposMASK_ID);
    _FIELD_SET(m_cnMASK_ID = szString, !m_cnMASK_ID.isEmpty(), eposMASK_ID);

    // WRR.USR_DESC
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposUSR_DESC);
    _FIELD_SET(m_cnUSR_DESC = szString, !m_cnUSR_DESC.isEmpty(), eposUSR_DESC);

    // WRR.EXC_DESC
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposEXC_DESC);
    _FIELD_SET(m_cnEXC_DESC = szString, !m_cnEXC_DESC.isEmpty(), eposEXC_DESC);

    return true;
}

bool Stdf_WRR_V4::Write(GS::StdLib::Stdf & clStdf)
{
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;

    RecordReadInfo.iRecordType = STDF_WRR_TYPE;
    RecordReadInfo.iRecordSubType = STDF_WRR_STYPE;
    clStdf.WriteHeader(&RecordReadInfo);

    // WRR.HEAD_NUM
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_u1HEAD_NUM)), eposHEAD_NUM, clStdf.WriteRecord());

    // WRR.SITE_GRP
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_u1SITE_GRP)), eposSITE_GRP, clStdf.WriteRecord());

    // WRR.FINISH_T
    _FIELD_CHECKWRITE(clStdf.WriteDword(DWORD(m_u4FINISH_T)), eposFINISH_T, clStdf.WriteRecord());

    // WRR.PART_CNT
    _FIELD_CHECKWRITE(clStdf.WriteDword(DWORD(m_u4PART_CNT)), eposPART_CNT, clStdf.WriteRecord());

    // WRR.RTST_CNT
    _FIELD_CHECKWRITE(clStdf.WriteDword(DWORD(m_u4RTST_CNT)), eposRTST_CNT, clStdf.WriteRecord());

    // WRR.ABRT_CNT
    _FIELD_CHECKWRITE(clStdf.WriteDword(DWORD(m_u4ABRT_CNT)), eposABRT_CNT, clStdf.WriteRecord());

    // WRR.GOOD_CNT
    _FIELD_CHECKWRITE(clStdf.WriteDword(DWORD(m_u4GOOD_CNT)), eposGOOD_CNT, clStdf.WriteRecord());

    // WRR.FUNC_CNT
    _FIELD_CHECKWRITE(clStdf.WriteDword(DWORD(m_u4FUNC_CNT)), eposFUNC_CNT, clStdf.WriteRecord());

    // WRR.WAFER_ID
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnWAFER_ID.toLatin1().constData()), eposWAFER_ID, clStdf.WriteRecord());

    // WRR.FABWF_ID
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnFABWF_ID.toLatin1().constData()), eposFABWF_ID, clStdf.WriteRecord());

    // WRR.FRAME_ID
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnFRAME_ID.toLatin1().constData()), eposFRAME_ID, clStdf.WriteRecord());

    // WRR.MASK_ID
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnMASK_ID.toLatin1().constData()), eposMASK_ID, clStdf.WriteRecord());

    // WRR.USR_DESC
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnUSR_DESC.toLatin1().constData()), eposUSR_DESC, clStdf.WriteRecord());

    // WRR.EXC_DESC
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnEXC_DESC.toLatin1().constData()), eposEXC_DESC, clStdf.WriteRecord());

    clStdf.WriteRecord();

    return true;
}

void Stdf_WRR_V4::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    QDateTime	clDateTime;
    time_t		tDateTime;

    // Set QDateTime object to UTC mode
    clDateTime.setTimeSpec(Qt::UTC);

    // Empty string first
    strAsciiString = "";

    // WRR.HEAD_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "WRR.head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // WRR.SITE_GRP
    _STR_ADDFIELD_ASCII(strAsciiString, "WRR.site_grp", QString::number(m_u1SITE_GRP), nFieldSelection, eposSITE_GRP);

    // WRR.FINISH_T
    tDateTime = (time_t)m_u4FINISH_T;
    clDateTime.setTime_t(tDateTime);
    m_strFieldValue_macro.sprintf("%s (0x%lx)",
                                  ( GS::StdLib::Stdf::GetDaysOfWeekString(clDateTime.date().dayOfWeek()-1)
                                    + clDateTime.toString(" dd ")
                                    + GS::StdLib::Stdf::GetMonthName(clDateTime.date().month()-1)
                                    + clDateTime.toString(" yyyy h:mm:ss") )
                                  .toLatin1().constData(), m_u4FINISH_T);
    _STR_ADDFIELD_ASCII(strAsciiString, "WRR.finish_t", m_strFieldValue_macro, nFieldSelection, eposFINISH_T);

    // WRR.PART_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "WRR.part_cnt", QString::number(m_u4PART_CNT), nFieldSelection, eposPART_CNT);

    // WRR.RTST_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "WRR.rtst_cnt", QString::number(m_u4RTST_CNT), nFieldSelection, eposRTST_CNT);

    // WRR.ABRT_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "WRR.abrt_cnt", QString::number(m_u4ABRT_CNT), nFieldSelection, eposABRT_CNT);

    // WRR.GOOD_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "WRR.good_cnt", QString::number(m_u4GOOD_CNT), nFieldSelection, eposGOOD_CNT);

    // WRR.FUNC_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "WRR.func_cnt", QString::number(m_u4FUNC_CNT), nFieldSelection, eposFUNC_CNT);

    // WRR.WAFER_ID
    _STR_ADDFIELD_ASCII(strAsciiString, "WRR.wafer_id", m_cnWAFER_ID, nFieldSelection, eposWAFER_ID);

    // WRR.FABWF_ID
    _STR_ADDFIELD_ASCII(strAsciiString, "WRR.fabwf_id", m_cnFABWF_ID, nFieldSelection, eposFABWF_ID);

    // WRR.FRAME_ID
    _STR_ADDFIELD_ASCII(strAsciiString, "WRR.frame_id", m_cnFRAME_ID, nFieldSelection, eposFRAME_ID);

    // WRR.MASK_ID
    _STR_ADDFIELD_ASCII(strAsciiString, "WRR.mask_id", m_cnMASK_ID, nFieldSelection, eposMASK_ID);

    // WRR.USR_DESC
    _STR_ADDFIELD_ASCII(strAsciiString, "WRR.usr_desc", m_cnUSR_DESC, nFieldSelection, eposUSR_DESC);

    // WRR.EXC_DESC
    _STR_ADDFIELD_ASCII(strAsciiString, "WRR.exc_desc", m_cnEXC_DESC, nFieldSelection, eposEXC_DESC);
}

void Stdf_WRR_V4::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
    QDateTime	clDateTime;
    time_t		tDateTime;

    // Set QDateTime object to UTC mode
    clDateTime.setTimeSpec(Qt::UTC);

    // Empty string list first
    listFields.empty();

    // WRR.HEAD_NUM
    _LIST_ADDFIELD_ASCII(listFields, "WRR.head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // WRR.SITE_GRP
    _LIST_ADDFIELD_ASCII(listFields, "WRR.site_grp", QString::number(m_u1SITE_GRP), nFieldSelection, eposSITE_GRP);

    // WRR.FINISH_T
    tDateTime = (time_t)m_u4FINISH_T;
    clDateTime.setTime_t(tDateTime);
    m_strFieldValue_macro.sprintf("%s (0x%lx)",
                                  ( GS::StdLib::Stdf::GetDaysOfWeekString(clDateTime.date().dayOfWeek()-1)
                                    + clDateTime.toString(" dd ")
                                    + GS::StdLib::Stdf::GetMonthName(clDateTime.date().month()-1)
                                    + clDateTime.toString(" yyyy h:mm:ss") )
                                  .toLatin1().constData(), m_u4FINISH_T);
    _LIST_ADDFIELD_ASCII(listFields, "WRR.finish_t", m_strFieldValue_macro, nFieldSelection, eposFINISH_T);

    // WRR.PART_CNT
    _LIST_ADDFIELD_ASCII(listFields, "WRR.part_cnt", QString::number(m_u4PART_CNT), nFieldSelection, eposPART_CNT);

    // WRR.RTST_CNT
    _LIST_ADDFIELD_ASCII(listFields, "WRR.rtst_cnt", QString::number(m_u4RTST_CNT), nFieldSelection, eposRTST_CNT);

    // WRR.ABRT_CNT
    _LIST_ADDFIELD_ASCII(listFields, "WRR.abrt_cnt", QString::number(m_u4ABRT_CNT), nFieldSelection, eposABRT_CNT);

    // WRR.GOOD_CNT
    _LIST_ADDFIELD_ASCII(listFields, "WRR.good_cnt", QString::number(m_u4GOOD_CNT), nFieldSelection, eposGOOD_CNT);

    // WRR.FUNC_CNT
    _LIST_ADDFIELD_ASCII(listFields, "WRR.func_cnt", QString::number(m_u4FUNC_CNT), nFieldSelection, eposFUNC_CNT);

    // WRR.WAFER_ID
    _LIST_ADDFIELD_ASCII(listFields, "WRR.wafer_id", m_cnWAFER_ID, nFieldSelection, eposWAFER_ID);

    // WRR.FABWF_ID
    _LIST_ADDFIELD_ASCII(listFields, "WRR.fabwf_id", m_cnFABWF_ID, nFieldSelection, eposFABWF_ID);

    // WRR.FRAME_ID
    _LIST_ADDFIELD_ASCII(listFields, "WRR.frame_id", m_cnFRAME_ID, nFieldSelection, eposFRAME_ID);

    // WRR.MASK_ID
    _LIST_ADDFIELD_ASCII(listFields, "WRR.mask_id", m_cnMASK_ID, nFieldSelection, eposMASK_ID);

    // WRR.USR_DESC
    _LIST_ADDFIELD_ASCII(listFields, "WRR.usr_desc", m_cnUSR_DESC, nFieldSelection, eposUSR_DESC);

    // WRR.EXC_DESC
    _LIST_ADDFIELD_ASCII(listFields, "WRR.exc_desc", m_cnEXC_DESC, nFieldSelection, eposEXC_DESC);
}

void Stdf_WRR_V4::GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection /*= 0*/)
{
    QString strTabs="";

    // Init XML string
    for(int nIndex=nIndentationLevel; nIndex>0; nIndex--)
        strTabs += "\t";
    strXmlString = strTabs;
    strXmlString += "<wrr>\n";

    // WRR.HEAD_NUM
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // WRR.SITE_GRP
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "site_grp", QString::number(m_u1SITE_GRP), nFieldSelection, eposSITE_GRP);

    // WRR.FINISH_T
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "finish_t", QString::number(m_u4FINISH_T), nFieldSelection, eposFINISH_T);

    // WRR.PART_CNT
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "part_cnt", QString::number(m_u4PART_CNT), nFieldSelection, eposPART_CNT);

    // WRR.RTST_CNT
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "rtst_cnt", QString::number(m_u4RTST_CNT), nFieldSelection, eposRTST_CNT);

    // WRR.ABRT_CNT
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "abrt_cnt", QString::number(m_u4ABRT_CNT), nFieldSelection, eposABRT_CNT);

    // WRR.GOOD_CNT
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "good_cnt", QString::number(m_u4GOOD_CNT), nFieldSelection, eposGOOD_CNT);

    // WRR.FUNC_CNT
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "func_cnt", QString::number(m_u4FUNC_CNT), nFieldSelection, eposFUNC_CNT);

    // WRR.WAFER_ID
    _CREATEFIELD_FROM_CN_XML(m_cnWAFER_ID);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "wafer_id", m_strFieldValue_macro, nFieldSelection, eposWAFER_ID);

    // WRR.FABWF_ID
    _CREATEFIELD_FROM_CN_XML(m_cnFABWF_ID);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "fabwf_id", m_strFieldValue_macro, nFieldSelection, eposFABWF_ID);

    // WRR.FRAME_ID
    _CREATEFIELD_FROM_CN_XML(m_cnFRAME_ID);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "frame_id", m_strFieldValue_macro, nFieldSelection, eposFRAME_ID);

    // WRR.MASK_ID
    _CREATEFIELD_FROM_CN_XML(m_cnMASK_ID);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "mask_id", m_strFieldValue_macro, nFieldSelection, eposMASK_ID);

    // WRR.USR_DESC
    _CREATEFIELD_FROM_CN_XML(m_cnUSR_DESC);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "usr_desc", m_strFieldValue_macro, nFieldSelection, eposUSR_DESC);

    // WRR.EXC_DESC
    _CREATEFIELD_FROM_CN_XML(m_cnEXC_DESC);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "exc_desc", m_strFieldValue_macro, nFieldSelection, eposEXC_DESC);

    strXmlString += strTabs;
    strXmlString += "</wrr>\n";
}

void Stdf_WRR_V4::GetAtdfString(QString & strAtdfString)
{
    QString		strString;
    QDateTime	clDateTime;
    time_t		tDateTime;

    // Set QDateTime object to UTC mode
    clDateTime.setTimeSpec(Qt::UTC);

    // Reset field index, and line Nb
    m_uiFieldIndex = 0;
    m_uiLineNb = 1;

    // Init string
    strAtdfString = "WRR:";

    // WRR.HEAD_NUM
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u1HEAD_NUM), eposHEAD_NUM);

    // WRR.FINISH_T
    tDateTime = (time_t)m_u4FINISH_T;
    clDateTime.setTime_t(tDateTime);
    QString strDate = clDateTime.toString("hh:mm:ss dd-MM-yyyy");
    strDate = strDate.replace("-01-","-Jan-").replace("-02-","-Feb-").replace("-03-","-Mar-").replace("-04-","-Apr-").replace("-05-","-May-").replace("-06-","-Jun-").replace("-07-","-Jul-").replace("-08-","-Aug-").replace("-09-","-Sep-").replace("-10-","-Oct-").replace("-11-","-Nov-").replace("-12-","-Dec-");
    m_strFieldValue_macro.sprintf("%s", strDate.toLatin1().constData());
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposFINISH_T);

    // WRR.PART_CNT
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u4PART_CNT), eposPART_CNT);

    // WRR.WAFER_ID
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnWAFER_ID, eposWAFER_ID);

    // WRR.SITE_GRP
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u1SITE_GRP), eposSITE_GRP);

    // WRR.RTST_CNT
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u4RTST_CNT), eposRTST_CNT);

    // WRR.ABRT_CNT
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u4ABRT_CNT), eposABRT_CNT);

    // WRR.GOOD_CNT
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u4GOOD_CNT), eposGOOD_CNT);

    // WRR.FUNC_CNT
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u4FUNC_CNT), eposFUNC_CNT);

    // WRR.FABWF_ID
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnFABWF_ID, eposFABWF_ID);

    // WRR.FRAME_ID
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnFRAME_ID, eposFRAME_ID);

    // WRR.MASK_ID
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnMASK_ID, eposMASK_ID);

    // WRR.USR_DESC
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnUSR_DESC, eposUSR_DESC);

    // WRR.EXC_DESC
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnEXC_DESC, eposEXC_DESC);

    _STR_FINISHFIELD_ATDF(strAtdfString);
}

///////////////////////////////////////////////////////////
// WCR RECORD
///////////////////////////////////////////////////////////
Stdf_WCR_V4::Stdf_WCR_V4() : Stdf_Record()
{
    Reset();
}

Stdf_WCR_V4::~Stdf_WCR_V4()
{
    Reset();
}

void Stdf_WCR_V4::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Select fields for reduced list

    // Reset Data
    m_r4WAFR_SIZ	= 0.0;					// WCR.WAFR_SIZ
    m_r4DIE_HT		= 0.0;					// WCR.DIE_HT
    m_r4DIE_WID		= 0.0;					// WCR.DIE_WID
    m_u1WF_UNITS	= 0;					// WCR.WF_UNITS
    m_c1WF_FLAT		= 0;					// WCR.WF_FLAT
    m_i2CENTER_X	= INVALID_SMALLINT;		// WCR.CENTER_X
    m_i2CENTER_Y	= INVALID_SMALLINT;		// WCR.CENTER_Y
    m_c1POS_X		= 0;					// WCR.POS_X
    m_c1POS_Y		= 0;					// WCR.POS_Y

    // Call Reset base method
    Stdf_Record::Reset();
}

bool Stdf_WCR_V4::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;

    return false;
}

QString Stdf_WCR_V4::GetRecordShortName(void)
{
    return "WCR";
}

QString Stdf_WCR_V4::GetRecordLongName(void)
{
    return "Wafer Configuration Record";
}

int Stdf_WCR_V4::GetRecordType(void)
{
    return Rec_WCR;
}

bool Stdf_WCR_V4::Read(GS::StdLib::Stdf & clStdf)
{
    float	fData;
    int		wData;
    BYTE	bData;

    // First reset data
    Reset();

    // WCR.WAFR_SIZ
    _FIELD_CHECKREAD(clStdf.ReadFloat(&fData), eposWAFR_SIZ);
    _FIELD_SET(m_r4WAFR_SIZ = stdf_type_r4(fData), m_r4WAFR_SIZ != 0.0, eposWAFR_SIZ);

    // WCR.DIE_HT
    _FIELD_CHECKREAD(clStdf.ReadFloat(&fData), eposDIE_HT);
    _FIELD_SET(m_r4DIE_HT = stdf_type_r4(fData), m_r4DIE_HT != 0.0, eposDIE_HT);

    // WCR.DIE_WID
    _FIELD_CHECKREAD(clStdf.ReadFloat(&fData), eposDIE_WID);
    _FIELD_SET(m_r4DIE_WID = stdf_type_r4(fData), m_r4DIE_WID != 0.0, eposDIE_WID);

    // WCR.WF_UNITS
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposWF_UNITS);
    _FIELD_SET(m_u1WF_UNITS = stdf_type_u1(bData), m_u1WF_UNITS != 0, eposWF_UNITS);

    // WCR.WF_FLAT
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposWF_FLAT);
    _FIELD_SET(m_c1WF_FLAT = stdf_type_c1(bData), m_c1WF_FLAT != ' ', eposWF_FLAT);

    // WCR.CENTER_X
    _FIELD_CHECKREAD(clStdf.ReadWord(&wData), eposCENTER_X);
    _FIELD_SET(m_i2CENTER_X = stdf_type_i2(wData), m_i2CENTER_X != -32768, eposCENTER_X);

    // WCR.CENTER_Y
    _FIELD_CHECKREAD(clStdf.ReadWord(&wData), eposCENTER_Y);
    _FIELD_SET(m_i2CENTER_Y = stdf_type_i2(wData), m_i2CENTER_Y != -32768, eposCENTER_Y);

    // WCR.POS_X
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposPOS_X);
    _FIELD_SET(m_c1POS_X = stdf_type_c1(bData), m_c1POS_X != ' ', eposPOS_X);

    // WCR.POS_Y
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposPOS_Y);
    _FIELD_SET(m_c1POS_Y = stdf_type_c1(bData), m_c1POS_Y != ' ', eposPOS_Y);

    return true;
}

bool Stdf_WCR_V4::Write(GS::StdLib::Stdf & clStdf)
{
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;

    RecordReadInfo.iRecordType = STDF_WCR_TYPE;
    RecordReadInfo.iRecordSubType = STDF_WCR_STYPE;
    clStdf.WriteHeader(&RecordReadInfo);

    // WCR.WAFR_SIZ
    _FIELD_CHECKWRITE(clStdf.WriteFloat(FLOAT(m_r4WAFR_SIZ)), eposWAFR_SIZ, clStdf.WriteRecord());

    // WCR.DIE_HT
    _FIELD_CHECKWRITE(clStdf.WriteFloat(FLOAT(m_r4DIE_HT)), eposDIE_HT, clStdf.WriteRecord());

    // WCR.DIE_WID
    _FIELD_CHECKWRITE(clStdf.WriteFloat(FLOAT(m_r4DIE_WID)), eposDIE_WID, clStdf.WriteRecord());

    // WCR.WF_UNITS
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_u1WF_UNITS)), eposWF_UNITS, clStdf.WriteRecord());

    // WCR.WF_FLAT
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_c1WF_FLAT)), eposWF_FLAT, clStdf.WriteRecord());

    // WCR.CENTER_X
    _FIELD_CHECKWRITE(clStdf.WriteWord(WORD(m_i2CENTER_X)), eposCENTER_X, clStdf.WriteRecord());

    // WCR.CENTER_Y
    _FIELD_CHECKWRITE(clStdf.WriteWord(WORD(m_i2CENTER_Y)), eposCENTER_Y, clStdf.WriteRecord());

    // WCR.POS_X
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_c1POS_X)), eposPOS_X, clStdf.WriteRecord());

    // WCR.POS_Y
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_c1POS_Y)), eposPOS_Y, clStdf.WriteRecord());

    clStdf.WriteRecord();

    return true;
}

void Stdf_WCR_V4::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    QString		strString;

    // Empty string first
    strAsciiString = "";

    // WCR.WAFR_SIZ
    _STR_ADDFIELD_ASCII(strAsciiString, "WCR.wafr_siz", QString::number(m_r4WAFR_SIZ), nFieldSelection, eposWAFR_SIZ);

    // WCR.DIE_HT
    _STR_ADDFIELD_ASCII(strAsciiString, "WCR.die_ht", QString::number(m_r4DIE_HT), nFieldSelection, eposDIE_HT);

    // WCR.DIE_WID
    _STR_ADDFIELD_ASCII(strAsciiString, "WCR.die_wid", QString::number(m_r4DIE_WID), nFieldSelection, eposDIE_WID);

    // WCR.WF_UNITS
    _STR_ADDFIELD_ASCII(strAsciiString, "WCR.wf_units", QString::number(m_u1WF_UNITS), nFieldSelection, eposWF_UNITS);

    // WCR.WF_FLAT
    _CREATEFIELD_FROM_C1_ASCII(m_c1WF_FLAT);
    _STR_ADDFIELD_ASCII(strAsciiString, "WCR.wf_flat", m_strFieldValue_macro, nFieldSelection, eposWF_FLAT);

    // WCR.CENTER_X
    _STR_ADDFIELD_ASCII(strAsciiString, "WCR.center_x", QString::number(m_i2CENTER_X), nFieldSelection, eposCENTER_X);

    // WCR.CENTER_Y
    _STR_ADDFIELD_ASCII(strAsciiString, "WCR.center_y", QString::number(m_i2CENTER_Y), nFieldSelection, eposCENTER_Y);

    // WCR.POS_X
    _CREATEFIELD_FROM_C1_ASCII(m_c1POS_X);
    _STR_ADDFIELD_ASCII(strAsciiString, "WCR.pos_x", m_strFieldValue_macro, nFieldSelection, eposPOS_X);

    // WCR.POS_Y
    _CREATEFIELD_FROM_C1_ASCII(m_c1POS_Y);
    _STR_ADDFIELD_ASCII(strAsciiString, "WCR.pos_y", m_strFieldValue_macro, nFieldSelection, eposPOS_Y);
}

void Stdf_WCR_V4::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
    QString		strString;

    // Empty string list first
    listFields.empty();

    // WCR.WAFR_SIZ
    _LIST_ADDFIELD_ASCII(listFields, "WCR.wafr_siz", QString::number(m_r4WAFR_SIZ), nFieldSelection, eposWAFR_SIZ);

    // WCR.DIE_HT
    _LIST_ADDFIELD_ASCII(listFields, "WCR.die_ht", QString::number(m_r4DIE_HT), nFieldSelection, eposDIE_HT);

    // WCR.DIE_WID
    _LIST_ADDFIELD_ASCII(listFields, "WCR.die_wid", QString::number(m_r4DIE_WID), nFieldSelection, eposDIE_WID);

    // WCR.WF_UNITS
    _LIST_ADDFIELD_ASCII(listFields, "WCR.wf_units", QString::number(m_u1WF_UNITS), nFieldSelection, eposWF_UNITS);

    // WCR.WF_FLAT
    _CREATEFIELD_FROM_C1_ASCII(m_c1WF_FLAT);
    _LIST_ADDFIELD_ASCII(listFields, "WCR.wf_flat", m_strFieldValue_macro, nFieldSelection, eposWF_FLAT);

    // WCR.CENTER_X
    _LIST_ADDFIELD_ASCII(listFields, "WCR.center_x", QString::number(m_i2CENTER_X), nFieldSelection, eposCENTER_X);

    // WCR.CENTER_Y
    _LIST_ADDFIELD_ASCII(listFields, "WCR.center_y", QString::number(m_i2CENTER_Y), nFieldSelection, eposCENTER_Y);

    // WCR.POS_X
    _CREATEFIELD_FROM_C1_ASCII(m_c1POS_X);
    _LIST_ADDFIELD_ASCII(listFields, "WCR.pos_x", m_strFieldValue_macro, nFieldSelection, eposPOS_X);

    // WCR.POS_Y
    _CREATEFIELD_FROM_C1_ASCII(m_c1POS_Y);
    _LIST_ADDFIELD_ASCII(listFields, "WCR.pos_y", m_strFieldValue_macro, nFieldSelection, eposPOS_Y);
}

void Stdf_WCR_V4::GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection /*= 0*/)
{
    QString strTabs="";

    // Init XML string
    for(int nIndex=nIndentationLevel; nIndex>0; nIndex--)
        strTabs += "\t";
    strXmlString = strTabs;
    strXmlString += "<wcr>\n";

    // WCR.WAFR_SIZ
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "wafr_siz", QString::number(m_r4WAFR_SIZ), nFieldSelection, eposWAFR_SIZ);

    // WCR.DIE_HT
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "die_ht", QString::number(m_r4DIE_HT), nFieldSelection, eposDIE_HT);

    // WCR.DIE_WID
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "die_wid", QString::number(m_r4DIE_WID), nFieldSelection, eposDIE_WID);

    // WCR.WF_UNITS
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "wf_units", QString::number(m_u1WF_UNITS), nFieldSelection, eposWF_UNITS);

    // WCR.WF_FLAT
    _CREATEFIELD_FROM_C1_XML(m_c1WF_FLAT);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "wf_flat", m_strFieldValue_macro, nFieldSelection, eposWF_FLAT);

    // WCR.CENTER_X
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "center_x", QString::number(m_i2CENTER_X), nFieldSelection, eposCENTER_X);

    // WCR.CENTER_Y
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "center_y", QString::number(m_i2CENTER_Y), nFieldSelection, eposCENTER_Y);

    // WCR.POS_X
    _CREATEFIELD_FROM_C1_XML(m_c1POS_X);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "pos_x", m_strFieldValue_macro, nFieldSelection, eposPOS_X);

    // WCR.POS_Y
    _CREATEFIELD_FROM_C1_XML(m_c1POS_Y);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "pos_y", m_strFieldValue_macro, nFieldSelection, eposPOS_Y);

    strXmlString += strTabs;
    strXmlString += "</wcr>\n";
}

void Stdf_WCR_V4::GetAtdfString(QString & strAtdfString)
{
    QString		strString;

    // Reset field index, and line Nb
    m_uiFieldIndex = 0;
    m_uiLineNb = 1;

    // Init string
    strAtdfString = "WCR:";

    // WCR.WF_FLAT
    _CREATEFIELD_FROM_C1_ASCII(m_c1WF_FLAT);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposWF_FLAT);

    // WCR.POS_X
    _CREATEFIELD_FROM_C1_ASCII(m_c1POS_X);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposPOS_X);

    // WCR.POS_Y
    _CREATEFIELD_FROM_C1_ASCII(m_c1POS_Y);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposPOS_Y);

    // WCR.WAFR_SIZ
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_r4WAFR_SIZ), eposWAFR_SIZ);

    // WCR.DIE_HT
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_r4DIE_HT), eposDIE_HT);

    // WCR.DIE_WID
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_r4DIE_WID), eposDIE_WID);

    // WCR.WF_UNITS
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u1WF_UNITS), eposWF_UNITS);

    // WCR.CENTER_X
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_i2CENTER_X), eposCENTER_X);

    // WCR.CENTER_Y
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_i2CENTER_Y), eposCENTER_Y);

    _STR_FINISHFIELD_ATDF(strAtdfString);
}

///////////////////////////////////////////////////////////
// PIR RECORD
///////////////////////////////////////////////////////////
Stdf_PIR_V4::Stdf_PIR_V4() : Stdf_Record()
{
    Reset();
}

Stdf_PIR_V4::Stdf_PIR_V4(const Stdf_PIR_V4 &other) : Stdf_Record(other)
{
    *this = other;
}

Stdf_PIR_V4::~Stdf_PIR_V4()
{
    Reset();
}

Stdf_PIR_V4 &Stdf_PIR_V4::operator=(const Stdf_PIR_V4 &other)
{
    if (this != &other)
    {
        // Clean up this object
        Reset();

        // Copy the base class
        Stdf_Record::operator =(other);

        // Copy class members
        for(int i=0; i<eposEND; i++)
            m_pFieldFlags[i] = other.m_pFieldFlags[i];

        // Reset Data
        m_u1HEAD_NUM	= other.m_u1HEAD_NUM;		// PIR.HEAD_NUM
        m_u1SITE_NUM	= other.m_u1SITE_NUM;		// PIR.SITE_NUM
    }

    return *this;
}

void Stdf_PIR_V4::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Select fields for reduced list

    // Reset Data
    m_u1HEAD_NUM	= 1;		// PIR.HEAD_NUM
    m_u1SITE_NUM	= 1;		// PIR.SITE_NUM

    // Call Reset base method
    Stdf_Record::Reset();
}

bool Stdf_PIR_V4::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;

    return false;
}

QString Stdf_PIR_V4::GetRecordShortName(void)
{
    return "PIR";
}

QString Stdf_PIR_V4::GetRecordLongName(void)
{
    return "Part Information Record";
}

int Stdf_PIR_V4::GetRecordType(void)
{
    return Rec_PIR;
}

bool Stdf_PIR_V4::Read(GS::StdLib::Stdf & clStdf)
{
    BYTE	bData;

    // First reset data
    Reset();

    // PIR.HEAD_NUM
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposHEAD_NUM);
    _FIELD_SET(m_u1HEAD_NUM = stdf_type_u1(bData), true, eposHEAD_NUM);

    // PIR.SITE_NUM
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposSITE_NUM);
    _FIELD_SET(m_u1SITE_NUM = stdf_type_u1(bData), true, eposSITE_NUM);

    return true;
}

bool Stdf_PIR_V4::Write(GS::StdLib::Stdf & clStdf)
{
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;

    RecordReadInfo.iRecordType = STDF_PIR_TYPE;
    RecordReadInfo.iRecordSubType = STDF_PIR_STYPE;
    clStdf.WriteHeader(&RecordReadInfo);

    // PIR.HEAD_NUM
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_u1HEAD_NUM)), eposHEAD_NUM, clStdf.WriteRecord());

    // PIR.SITE_NUM
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_u1SITE_NUM)), eposSITE_NUM, clStdf.WriteRecord());

    clStdf.WriteRecord();

    return true;
}

void Stdf_PIR_V4::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    QString		strString;

    // Empty string first
    strAsciiString = "";

    // PIR.HEAD_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "PIR.head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // PIR.SITE_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "PIR.site_num", QString::number(m_u1SITE_NUM), nFieldSelection, eposSITE_NUM);
}

void Stdf_PIR_V4::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
    QString		strString;

    // Empty string list first
    listFields.empty();

    // PIR.HEAD_NUM
    _LIST_ADDFIELD_ASCII(listFields, "PIR.head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // PIR.SITE_NUM
    _LIST_ADDFIELD_ASCII(listFields, "PIR.site_num", QString::number(m_u1SITE_NUM), nFieldSelection, eposSITE_NUM);
}

void Stdf_PIR_V4::GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection /*= 0*/)
{
    QString strTabs="";

    // Init XML string
    for(int nIndex=nIndentationLevel; nIndex>0; nIndex--)
        strTabs += "\t";
    strXmlString = strTabs;
    strXmlString += "<pir>\n";

    // PIR.HEAD_NUM
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // PIR.SITE_NUM
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "site_num", QString::number(m_u1SITE_NUM), nFieldSelection, eposSITE_NUM);

    strXmlString += strTabs;
    strXmlString += "</pir>\n";
}

void Stdf_PIR_V4::GetAtdfString(QString & strAtdfString)
{
    QString		strString;

    // Reset field index, and line Nb
    m_uiFieldIndex = 0;
    m_uiLineNb = 1;

    // Init string
    strAtdfString = "PIR:";

    // PIR.HEAD_NUM
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u1HEAD_NUM), eposHEAD_NUM);

    // PIR.SITE_NUM
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u1SITE_NUM), eposSITE_NUM);

    _STR_FINISHFIELD_ATDF(strAtdfString);
}

///////////////////////////////////////////////////////////
// PRR RECORD
///////////////////////////////////////////////////////////
Stdf_PRR_V4::Stdf_PRR_V4() : Stdf_Record()
{
    Reset();
}

Stdf_PRR_V4::Stdf_PRR_V4(const Stdf_PRR_V4 &other) : Stdf_Record(other)
{
    *this = other;
}

Stdf_PRR_V4::~Stdf_PRR_V4()
{
    Reset();
}

Stdf_PRR_V4 &Stdf_PRR_V4::operator=(const Stdf_PRR_V4 &other)
{
    if (this != &other)
    {
        Reset();

        // Reset field flags
        for(int i=0; i<eposEND; i++)
            m_pFieldFlags[i] = other.m_pFieldFlags[i];

        m_u1HEAD_NUM	= other.m_u1HEAD_NUM;					// PRR.HEAD_NUM
        m_u1SITE_NUM	= other.m_u1SITE_NUM;					// PRR.SITE_NUM
        m_b1PART_FLG	= other.m_b1PART_FLG;					// PRR.PART_FLG
        m_u2NUM_TEST	= other.m_u2NUM_TEST;					// PRR.NUM_TEST
        m_u2HARD_BIN	= other.m_u2HARD_BIN;					// PRR.HARD_BIN
        m_u2SOFT_BIN	= other.m_u2SOFT_BIN;					// PRR.SOFT_BIN
        m_i2X_COORD		= other.m_i2X_COORD;                    // PRR.X_COORD
        m_i2Y_COORD		= other.m_i2Y_COORD;                    // PRR.Y_COORD
        m_u4TEST_T		= other.m_u4TEST_T;                     // PRR.TEST_T
        m_cnPART_ID		= other.m_cnPART_ID;					// PRR.PART_ID
        m_cnPART_TXT	= other.m_cnPART_TXT;					// PRR.PART_TXT
        m_bnPART_FIX    = other.m_bnPART_FIX;

        Stdf_Record::operator =(other);
    }

    return *this;
}

void Stdf_PRR_V4::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Select fields for reduced list

    // Reset Data
    m_u1HEAD_NUM	= 1;					// PRR.HEAD_NUM
    m_u1SITE_NUM	= 1;					// PRR.SITE_NUM
    m_b1PART_FLG	= 0;					// PRR.PART_FLG
    m_u2NUM_TEST	= 0;					// PRR.NUM_TEST
    m_u2HARD_BIN	= 0;					// PRR.HARD_BIN
    m_u2SOFT_BIN	= STDF_MAX_U2;			// PRR.SOFT_BIN
    m_i2X_COORD		= INVALID_SMALLINT;		// PRR.X_COORD
    m_i2Y_COORD		= INVALID_SMALLINT;		// PRR.Y_COORD
    m_u4TEST_T		= 0;					// PRR.TEST_T
    m_cnPART_ID		= "";					// PRR.PART_ID
    m_cnPART_TXT	= "";					// PRR.PART_TXT
    // PRR.PART_FIX
    m_bnPART_FIX.m_bLength = 0;
    for(int i=0; i<STDF_MAX_U1+1; i++)
        m_bnPART_FIX.m_pBitField[i] = 0;

    // Call Reset base method
    Stdf_Record::Reset();
}

bool Stdf_PRR_V4::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;

    return false;
}

QString Stdf_PRR_V4::GetRecordShortName(void)
{
    return "PRR";
}

QString Stdf_PRR_V4::GetRecordLongName(void)
{
    return "Part Results Record";
}

int Stdf_PRR_V4::GetRecordType(void)
{
    return Rec_PRR;
}

bool Stdf_PRR_V4::Read(GS::StdLib::Stdf & clStdf)
{
    char	szString[STDF_MAX_U1+1];
    long	dwData;
    int		wData;
    BYTE	bData;

    // First reset data
    Reset();

    // PRR.HEAD_NUM
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposHEAD_NUM);
    _FIELD_SET(m_u1HEAD_NUM = stdf_type_u1(bData), true, eposHEAD_NUM);

    // PRR.SITE_NUM
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposSITE_NUM);
    _FIELD_SET(m_u1SITE_NUM = stdf_type_u1(bData), true, eposSITE_NUM);

    // PRR.PART_FLG
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposPART_FLG);
    _FIELD_SET(m_b1PART_FLG = stdf_type_b1(bData), true, eposPART_FLG);

    // PRR.NUM_TEST
    _FIELD_CHECKREAD(clStdf.ReadWord(&wData), eposNUM_TEST);
    _FIELD_SET(m_u2NUM_TEST = stdf_type_u2(wData), true, eposNUM_TEST);

    // PRR.HARD_BIN
    _FIELD_CHECKREAD(clStdf.ReadWord(&wData), eposHARD_BIN);
    _FIELD_SET(m_u2HARD_BIN = stdf_type_u2(wData), true, eposHARD_BIN);

    // PRR.SOFT_BIN
    _FIELD_CHECKREAD(clStdf.ReadWord(&wData), eposSOFT_BIN);
    _FIELD_SET(m_u2SOFT_BIN = stdf_type_u2(wData), m_u2SOFT_BIN != 65535, eposSOFT_BIN);

    // PRR.X_COORD
    _FIELD_CHECKREAD(clStdf.ReadWord(&wData), eposX_COORD);
    _FIELD_SET(m_i2X_COORD = stdf_type_i2(wData), m_i2X_COORD != -32768, eposX_COORD);

    // PRR.Y_COORD
    _FIELD_CHECKREAD(clStdf.ReadWord(&wData), eposY_COORD);
    _FIELD_SET(m_i2Y_COORD = stdf_type_i2(wData), m_i2Y_COORD != -32768, eposY_COORD);

    // PRR.TEST_T
    _FIELD_CHECKREAD(clStdf.ReadDword(&dwData), eposTEST_T);
    _FIELD_SET(m_u4TEST_T = stdf_type_u4(dwData), m_u4TEST_T != 0, eposTEST_T);

    // PRR.PART_ID
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposPART_ID);
    _FIELD_SET(m_cnPART_ID = szString, !m_cnPART_ID.isEmpty(), eposPART_ID);

    // PRR.PART_TXT
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposPART_TXT);
    _FIELD_SET(m_cnPART_TXT = szString, !m_cnPART_TXT.isEmpty(), eposPART_TXT);

    // PRR.PART_FIX
    _FIELD_CHECKREAD(clStdf.ReadBitField(&(m_bnPART_FIX.m_bLength), m_bnPART_FIX.m_pBitField), eposPART_FIX);
    // The field has already been set by the ReadBitField function. We use a dummy instruction as parameter 1 to avoid warning under gcc.
    _FIELD_SET(m_uiIndex_macro = 0, m_bnPART_FIX.m_bLength != 0, eposPART_FIX);

    return true;
}

bool Stdf_PRR_V4::Write(GS::StdLib::Stdf & clStdf)
{
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;

    RecordReadInfo.iRecordType = STDF_PRR_TYPE;
    RecordReadInfo.iRecordSubType = STDF_PRR_STYPE;
    clStdf.WriteHeader(&RecordReadInfo);

    // PRR.HEAD_NUM
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_u1HEAD_NUM)), eposHEAD_NUM, clStdf.WriteRecord());

    // PRR.SITE_NUM
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_u1SITE_NUM)), eposSITE_NUM, clStdf.WriteRecord());

    // PRR.PART_FLG
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_b1PART_FLG)), eposPART_FLG, clStdf.WriteRecord());

    // PRR.NUM_TEST
    _FIELD_CHECKWRITE(clStdf.WriteWord(WORD(m_u2NUM_TEST)), eposNUM_TEST, clStdf.WriteRecord());

    // PRR.HARD_BIN
    _FIELD_CHECKWRITE(clStdf.WriteWord(WORD(m_u2HARD_BIN)), eposHARD_BIN, clStdf.WriteRecord());

    // PRR.SOFT_BIN
    _FIELD_CHECKWRITE(clStdf.WriteWord(WORD(m_u2SOFT_BIN)), eposSOFT_BIN, clStdf.WriteRecord());

    // PRR.X_COORD
    _FIELD_CHECKWRITE(clStdf.WriteWord(WORD(m_i2X_COORD)), eposX_COORD, clStdf.WriteRecord());

    // PRR.Y_COORD
    _FIELD_CHECKWRITE(clStdf.WriteWord(WORD(m_i2Y_COORD)), eposY_COORD, clStdf.WriteRecord());

    // PRR.TEST_T
    _FIELD_CHECKWRITE(clStdf.WriteDword(DWORD(m_u4TEST_T)), eposTEST_T, clStdf.WriteRecord());

    // PRR.PART_ID
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnPART_ID.toLatin1().constData()), eposPART_ID, clStdf.WriteRecord());

    // PRR.PART_TXT
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnPART_TXT.toLatin1().constData()), eposPART_TXT, clStdf.WriteRecord());

    // PRR.PART_FIX
    _FIELD_CHECKWRITE(clStdf.WriteBitField(BYTE(m_bnPART_FIX.m_bLength), m_bnPART_FIX.m_pBitField), eposPART_FIX, clStdf.WriteRecord());

    clStdf.WriteRecord();

    return true;
}

void Stdf_PRR_V4::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    // Empty string first
    strAsciiString = "";

    // PRR.HEAD_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "PRR.head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // PRR.SITE_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "PRR.site_num", QString::number(m_u1SITE_NUM), nFieldSelection, eposSITE_NUM);

    // PRR.PART_FLG
    _CREATEFIELD_FROM_B1_ASCII(m_b1PART_FLG)
    _STR_ADDFIELD_ASCII(strAsciiString, "PRR.part_flg", m_strFieldValue_macro, nFieldSelection, eposPART_FLG);

    // PRR.NUM_TEST
    _STR_ADDFIELD_ASCII(strAsciiString, "PRR.num_test", QString::number(m_u2NUM_TEST), nFieldSelection, eposNUM_TEST);

    // PRR.HARD_BIN
    _STR_ADDFIELD_ASCII(strAsciiString, "PRR.hard_bin", QString::number(m_u2HARD_BIN), nFieldSelection, eposHARD_BIN);

    // PRR.SOFT_BIN
    _STR_ADDFIELD_ASCII(strAsciiString, "PRR.soft_bin", QString::number(m_u2SOFT_BIN), nFieldSelection, eposSOFT_BIN);

    // PRR.X_COORD
    _STR_ADDFIELD_ASCII(strAsciiString, "PRR.x_coord", QString::number(m_i2X_COORD), nFieldSelection, eposX_COORD);

    // PRR.Y_COORD
    _STR_ADDFIELD_ASCII(strAsciiString, "PRR.y_coord", QString::number(m_i2Y_COORD), nFieldSelection, eposY_COORD);

    // PRR.TEST_T
    _STR_ADDFIELD_ASCII(strAsciiString, "PRR.test_t", QString::number(m_u4TEST_T), nFieldSelection, eposTEST_T);

    // PRR.PART_ID
    _STR_ADDFIELD_ASCII(strAsciiString, "PRR.part_id", m_cnPART_ID, nFieldSelection, eposPART_ID);

    // PRR.PART_TXT
    _STR_ADDFIELD_ASCII(strAsciiString, "PRR.part_txt", m_cnPART_TXT, nFieldSelection, eposPART_TXT);

    // PRR.PART_FIX
    _CREATEFIELD_FROM_BN_ASCII(m_bnPART_FIX);
    _STR_ADDFIELD_ASCII(strAsciiString, "PRR.part_fix", m_strFieldValue_macro, nFieldSelection, eposPART_FIX);
}

void Stdf_PRR_V4::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
    // Empty string list first
    listFields.empty();

    // PRR.HEAD_NUM
    _LIST_ADDFIELD_ASCII(listFields, "PRR.head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // PRR.SITE_NUM
    _LIST_ADDFIELD_ASCII(listFields, "PRR.site_num", QString::number(m_u1SITE_NUM), nFieldSelection, eposSITE_NUM);

    // PRR.PART_FLG
    _CREATEFIELD_FROM_B1_ASCII(m_b1PART_FLG)
    _LIST_ADDFIELD_ASCII(listFields, "PRR.part_flg", m_strFieldValue_macro, nFieldSelection, eposPART_FLG);

    // PRR.NUM_TEST
    _LIST_ADDFIELD_ASCII(listFields, "PRR.num_test", QString::number(m_u2NUM_TEST), nFieldSelection, eposNUM_TEST);

    // PRR.HARD_BIN
    _LIST_ADDFIELD_ASCII(listFields, "PRR.hard_bin", QString::number(m_u2HARD_BIN), nFieldSelection, eposHARD_BIN);

    // PRR.SOFT_BIN
    _LIST_ADDFIELD_ASCII(listFields, "PRR.soft_bin", QString::number(m_u2SOFT_BIN), nFieldSelection, eposSOFT_BIN);

    // PRR.X_COORD
    _LIST_ADDFIELD_ASCII(listFields, "PRR.x_coord", QString::number(m_i2X_COORD), nFieldSelection, eposX_COORD);

    // PRR.Y_COORD
    _LIST_ADDFIELD_ASCII(listFields, "PRR.y_coord", QString::number(m_i2Y_COORD), nFieldSelection, eposY_COORD);

    // PRR.TEST_T
    _LIST_ADDFIELD_ASCII(listFields, "PRR.test_t", QString::number(m_u4TEST_T), nFieldSelection, eposTEST_T);

    // PRR.PART_ID
    _LIST_ADDFIELD_ASCII(listFields, "PRR.part_id", m_cnPART_ID, nFieldSelection, eposPART_ID);

    // PRR.PART_TXT
    _LIST_ADDFIELD_ASCII(listFields, "PRR.part_txt", m_cnPART_TXT, nFieldSelection, eposPART_TXT);

    // PRR.PART_FIX
    _CREATEFIELD_FROM_BN_ASCII(m_bnPART_FIX);
    _LIST_ADDFIELD_ASCII(listFields, "PRR.part_fix", m_strFieldValue_macro, nFieldSelection, eposPART_FIX);
}

void Stdf_PRR_V4::GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection /*= 0*/)
{
    QString strTabs="";

    // Init XML string
    for(int nIndex=nIndentationLevel; nIndex>0; nIndex--)
        strTabs += "\t";
    strXmlString = strTabs;
    strXmlString += "<prr>\n";

    // PRR.HEAD_NUM
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // PRR.SITE_NUM
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "site_num", QString::number(m_u1SITE_NUM), nFieldSelection, eposSITE_NUM);

    // PRR.PART_FLG
    _CREATEFIELD_FROM_B1_XML(m_b1PART_FLG)
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "part_flg", m_strFieldValue_macro, nFieldSelection, eposPART_FLG);

    // PRR.NUM_TEST
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "num_test", QString::number(m_u2NUM_TEST), nFieldSelection, eposNUM_TEST);

    // PRR.HARD_BIN
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "hard_bin", QString::number(m_u2HARD_BIN), nFieldSelection, eposHARD_BIN);

    // PRR.SOFT_BIN
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "soft_bin", QString::number(m_u2SOFT_BIN), nFieldSelection, eposSOFT_BIN);

    // PRR.X_COORD
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "x_coord", QString::number(m_i2X_COORD), nFieldSelection, eposX_COORD);

    // PRR.Y_COORD
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "y_coord", QString::number(m_i2Y_COORD), nFieldSelection, eposY_COORD);

    // PRR.TEST_T
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "test_t", QString::number(m_u4TEST_T), nFieldSelection, eposTEST_T);

    // PRR.PART_ID
    _CREATEFIELD_FROM_CN_XML(m_cnPART_ID);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "part_id", m_strFieldValue_macro, nFieldSelection, eposPART_ID);

    // PRR.PART_TXT
    _CREATEFIELD_FROM_CN_XML(m_cnPART_TXT);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "part_txt", m_strFieldValue_macro, nFieldSelection, eposPART_TXT);

    // PRR.PART_FIX
    _CREATEFIELD_FROM_BN_XML(m_bnPART_FIX);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "part_fix", m_strFieldValue_macro, nFieldSelection, eposPART_FIX);

    strXmlString += strTabs;
    strXmlString += "</prr>\n";
}

void Stdf_PRR_V4::GetAtdfString(QString & strAtdfString)
{
    // Reset field index, and line Nb
    m_uiFieldIndex = 0;
    m_uiLineNb = 1;

    // Init string
    strAtdfString = "PRR:";

    // PRR.HEAD_NUM
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u1HEAD_NUM), eposHEAD_NUM);

    // PRR.SITE_NUM
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u1SITE_NUM), eposSITE_NUM);

    // PRR.PART_ID
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnPART_ID, eposPART_ID);

    // PRR.NUM_TEST
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u2NUM_TEST), eposNUM_TEST);

    // PRR.Pass/Fail Code
    m_strFieldValue_macro = " ";
    if((m_b1PART_FLG & 0x10) == 0)
    {
        if(m_b1PART_FLG & 0x08)
            m_strFieldValue_macro = "F";
        else
            m_strFieldValue_macro= "P";
    }
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposPART_FLG);

    // PRR.HARD_BIN
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u2HARD_BIN), eposHARD_BIN);

    // PRR.SOFT_BIN
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u2SOFT_BIN), eposSOFT_BIN);

    // PRR.X_COORD
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_i2X_COORD), eposX_COORD);

    // PRR.Y_COORD
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_i2Y_COORD), eposY_COORD);

    // PRR.Retest Code
    m_strFieldValue_macro = " ";
    if(m_b1PART_FLG & 0x01)
        m_strFieldValue_macro = "I";
    if(m_b1PART_FLG & 0x02)
        m_strFieldValue_macro = "C";
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposPART_FLG);

    // PRR.Abort Code
    m_strFieldValue_macro = " ";
    if(m_b1PART_FLG & 0x04)
        m_strFieldValue_macro = "Y";
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposPART_FLG);

    // PRR.TEST_T
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u4TEST_T), eposTEST_T);

    // PRR.PART_TXT
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnPART_TXT, eposPART_TXT);

    // PRR.PART_FIX
    _CREATEFIELD_FROM_BN_ATDF(m_bnPART_FIX);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposPART_FIX);

    _STR_FINISHFIELD_ATDF(strAtdfString);
}

///////////////////////////////////////////////////////////
// TSR RECORD
///////////////////////////////////////////////////////////
Stdf_TSR_V4::Stdf_TSR_V4() : Stdf_Record()
{
    Reset();
}

Stdf_TSR_V4::~Stdf_TSR_V4()
{
    Reset();
}

void Stdf_TSR_V4::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Select fields for reduced list

    // Reset Data
    m_u1HEAD_NUM	= 255;				// TSR.HEAD_NUM
    m_u1SITE_NUM	= 1;				// TSR.SITE_NUM
    m_c1TEST_TYP	= 0;				// TSR.TEST_TYP
    m_u4TEST_NUM	= 0;				// TSR.TEST_NUM
    m_u4EXEC_CNT	= INVALID_INT;		// TSR.EXEC_CNT
    m_u4FAIL_CNT	= INVALID_INT;		// TSR.FAIL_CNT
    m_u4ALRM_CNT	= INVALID_INT;		// TSR.ALRM_CNT
    m_cnTEST_NAM	= "";				// TSR.TEST_NAM
    m_cnSEQ_NAME	= "";				// TSR.SEQ_NAME
    m_cnTEST_LBL	= "";				// TSR.TEST_LBL
    m_b1OPT_FLAG	= 0;				// TSR.OPT_FLAG
    m_r4TEST_TIM	= 0.0;				// TSR.TEST_TIM
    m_r4TEST_MIN	= 0.0;				// TSR.TEST_MIN
    m_r4TEST_MAX	= 0.0;				// TSR.TEST_MAX
    m_r4TST_SUMS	= 0.0;				// TSR.TST_SUMS
    m_r4TST_SQRS	= 0.0;				// TSR.TST_SQRS

    // Call Reset base method
    Stdf_Record::Reset();
}

bool Stdf_TSR_V4::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;

    return false;
}

QString Stdf_TSR_V4::GetRecordShortName(void)
{
    return "TSR";
}

QString Stdf_TSR_V4::GetRecordLongName(void)
{
    return "Test Synopsis Record";
}

int Stdf_TSR_V4::GetRecordType(void)
{
    return Rec_TSR;
}

bool Stdf_TSR_V4::Read(GS::StdLib::Stdf & clStdf)
{
    char	szString[STDF_MAX_U1+1];
    long	dwData;
    float	fData;
    BYTE	bData;

    // First reset data
    Reset();

    // TSR.HEAD_NUM
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposHEAD_NUM);
    _FIELD_SET(m_u1HEAD_NUM = stdf_type_u1(bData), true, eposHEAD_NUM);

    // TSR.SITE_NUM
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposSITE_NUM);
    _FIELD_SET(m_u1SITE_NUM = stdf_type_u1(bData), true, eposSITE_NUM);

    // TSR.TEST_TYP
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposTEST_TYP);
    _FIELD_SET(m_c1TEST_TYP = stdf_type_c1(bData), m_c1TEST_TYP != ' ', eposTEST_TYP);

    // TSR.TEST_NUM
    _FIELD_CHECKREAD(clStdf.ReadDword(&dwData), eposTEST_NUM);
    _FIELD_SET(m_u4TEST_NUM = stdf_type_u4(dwData), true, eposTEST_NUM);

    // TSR.EXEC_CNT
    _FIELD_CHECKREAD(clStdf.ReadDword(&dwData), eposEXEC_CNT);
    _FIELD_SET(m_u4EXEC_CNT = stdf_type_u4(dwData), m_u4EXEC_CNT != 4294967295UL, eposEXEC_CNT);

    // TSR.FAIL_CNT
    _FIELD_CHECKREAD(clStdf.ReadDword(&dwData), eposFAIL_CNT);
    _FIELD_SET(m_u4FAIL_CNT = stdf_type_u4(dwData), m_u4FAIL_CNT != 4294967295UL, eposFAIL_CNT);

    // TSR.ALRM_CNT
    _FIELD_CHECKREAD(clStdf.ReadDword(&dwData), eposALRM_CNT);
    _FIELD_SET(m_u4ALRM_CNT = stdf_type_u4(dwData), m_u4ALRM_CNT != 4294967295UL, eposALRM_CNT);

    // TSR.TEST_NAM
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposTEST_NAM);
    _FIELD_SET(m_cnTEST_NAM = szString, !m_cnTEST_NAM.isEmpty(), eposTEST_NAM);

    // TSR.SEQ_NAME
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposSEQ_NAME);
    _FIELD_SET(m_cnSEQ_NAME = szString, !m_cnSEQ_NAME.isEmpty(), eposSEQ_NAME);

    // TSR.TEST_LBL
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposTEST_LBL);
    _FIELD_SET(m_cnTEST_LBL = szString, !m_cnTEST_LBL.isEmpty(), eposTEST_LBL);

    // TSR.OPT_FLAG
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposOPT_FLAG);
    _FIELD_SET(m_b1OPT_FLAG = stdf_type_b1(bData), true, eposOPT_FLAG);

    // TSR.TEST_TIM
    _FIELD_CHECKREAD(clStdf.ReadFloat(&fData), eposTEST_TIM);
    _FIELD_SET(m_r4TEST_TIM = stdf_type_r4(fData), (m_b1OPT_FLAG & STDF_MASK_BIT2) == 0, eposTEST_TIM);

    // TSR.TEST_MIN
    _FIELD_CHECKREAD(clStdf.ReadFloat(&fData), eposTEST_MIN);
    _FIELD_SET(m_r4TEST_MIN = stdf_type_r4(fData), (m_b1OPT_FLAG & STDF_MASK_BIT0) == 0, eposTEST_MIN);

    // TSR.TEST_MAX
    _FIELD_CHECKREAD(clStdf.ReadFloat(&fData), eposTEST_MAX);
    _FIELD_SET(m_r4TEST_MAX = stdf_type_r4(fData), (m_b1OPT_FLAG & STDF_MASK_BIT1) == 0, eposTEST_MAX);

    // TSR.TST_SUMS
    _FIELD_CHECKREAD(clStdf.ReadFloat(&fData), eposTST_SUMS);
    _FIELD_SET(m_r4TST_SUMS = stdf_type_r4(fData), (m_b1OPT_FLAG & STDF_MASK_BIT4) == 0, eposTST_SUMS);

    // TSR.TST_SQRS
    _FIELD_CHECKREAD(clStdf.ReadFloat(&fData), eposTST_SQRS);
    _FIELD_SET(m_r4TST_SQRS = stdf_type_r4(fData), (m_b1OPT_FLAG & STDF_MASK_BIT5) == 0, eposTST_SQRS);

    return true;
}

bool Stdf_TSR_V4::Write(GS::StdLib::Stdf & clStdf)
{
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;

    RecordReadInfo.iRecordType = STDF_TSR_TYPE;
    RecordReadInfo.iRecordSubType = STDF_TSR_STYPE;
    clStdf.WriteHeader(&RecordReadInfo);

    // TSR.HEAD_NUM
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_u1HEAD_NUM)), eposHEAD_NUM, clStdf.WriteRecord());

    // TSR.SITE_NUM
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_u1SITE_NUM)), eposSITE_NUM, clStdf.WriteRecord());

    // TSR.TEST_TYP
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_c1TEST_TYP)), eposTEST_TYP, clStdf.WriteRecord());

    // TSR.TEST_NUM
    _FIELD_CHECKWRITE(clStdf.WriteDword(DWORD(m_u4TEST_NUM)), eposTEST_NUM, clStdf.WriteRecord());

    // TSR.EXEC_CNT
    _FIELD_CHECKWRITE(clStdf.WriteDword(DWORD(m_u4EXEC_CNT)), eposEXEC_CNT, clStdf.WriteRecord());

    // TSR.FAIL_CNT
    _FIELD_CHECKWRITE(clStdf.WriteDword(DWORD(m_u4FAIL_CNT)), eposFAIL_CNT, clStdf.WriteRecord());

    // TSR.ALRM_CNT
    _FIELD_CHECKWRITE(clStdf.WriteDword(DWORD(m_u4ALRM_CNT)), eposALRM_CNT, clStdf.WriteRecord());

    // TSR.TEST_NAM
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnTEST_NAM.toLatin1().constData()), eposTEST_NAM, clStdf.WriteRecord());

    // TSR.SEQ_NAME
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnSEQ_NAME.toLatin1().constData()), eposSEQ_NAME, clStdf.WriteRecord());

    // TSR.TEST_LBL
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnTEST_LBL.toLatin1().constData()), eposTEST_LBL, clStdf.WriteRecord());

    // TSR.OPT_FLAG
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_b1OPT_FLAG)), eposOPT_FLAG, clStdf.WriteRecord());

    // TSR.TEST_TIM
    _FIELD_CHECKWRITE(clStdf.WriteFloat(FLOAT(m_r4TEST_TIM)), eposTEST_TIM, clStdf.WriteRecord());

    // TSR.TEST_MIN
    _FIELD_CHECKWRITE(clStdf.WriteFloat(FLOAT(m_r4TEST_MIN)), eposTEST_MIN, clStdf.WriteRecord());

    // TSR.TEST_MAX
    _FIELD_CHECKWRITE(clStdf.WriteFloat(FLOAT(m_r4TEST_MAX)), eposTEST_MAX, clStdf.WriteRecord());

    // TSR.TST_SUMS
    _FIELD_CHECKWRITE(clStdf.WriteFloat(FLOAT(m_r4TST_SUMS)), eposTST_SUMS, clStdf.WriteRecord());

    // TSR.TST_SQRS
    _FIELD_CHECKWRITE(clStdf.WriteFloat(FLOAT(m_r4TST_SQRS)), eposTST_SQRS, clStdf.WriteRecord());

    clStdf.WriteRecord();

    return true;
}

void Stdf_TSR_V4::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    QString			strString;

    // Empty string first
    strAsciiString = "";

    // TSR.HEAD_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "TSR.head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // TSR.SITE_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "TSR.site_num", QString::number(m_u1SITE_NUM), nFieldSelection, eposSITE_NUM);

    // TSR.TEST_TYP
    _CREATEFIELD_FROM_C1_ASCII(m_c1TEST_TYP);
    _STR_ADDFIELD_ASCII(strAsciiString, "TSR.test_typ", m_strFieldValue_macro, nFieldSelection, eposTEST_TYP);

    // TSR.TEST_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "TSR.test_num", QString::number(m_u4TEST_NUM), nFieldSelection, eposTEST_NUM);

    // TSR.EXEC_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "TSR.exec_cnt", QString::number(m_u4EXEC_CNT), nFieldSelection, eposEXEC_CNT);

    // TSR.FAIL_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "TSR.fail_cnt", QString::number(m_u4FAIL_CNT), nFieldSelection, eposFAIL_CNT);

    // TSR.ALRM_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "TSR.alrm_cnt", QString::number(m_u4ALRM_CNT), nFieldSelection, eposALRM_CNT);

    // TSR.TEST_NAM
    _STR_ADDFIELD_ASCII(strAsciiString, "TSR.test_nam", m_cnTEST_NAM, nFieldSelection, eposTEST_NAM);

    // TSR.SEQ_NAME
    _STR_ADDFIELD_ASCII(strAsciiString, "TSR.seq_name", m_cnSEQ_NAME, nFieldSelection, eposSEQ_NAME);

    // TSR.TEST_LBL
    _STR_ADDFIELD_ASCII(strAsciiString, "TSR.test_lbl", m_cnTEST_LBL, nFieldSelection, eposTEST_LBL);

    // TSR.OPT_FLAG
    _CREATEFIELD_FROM_B1_ASCII(m_b1OPT_FLAG)
    _STR_ADDFIELD_ASCII(strAsciiString, "TSR.opt_flag", m_strFieldValue_macro, nFieldSelection, eposOPT_FLAG);

    // TSR.TEST_TIM
    _STR_ADDFIELD_ASCII(strAsciiString, "TSR.test_tim", QString::number(m_r4TEST_TIM), nFieldSelection, eposTEST_TIM);

    // TSR.TEST_MIN
    _STR_ADDFIELD_ASCII(strAsciiString, "TSR.test_min", QString::number(m_r4TEST_MIN), nFieldSelection, eposTEST_MIN);

    // TSR.TEST_MAX
    _STR_ADDFIELD_ASCII(strAsciiString, "TSR.test_max", QString::number(m_r4TEST_MAX), nFieldSelection, eposTEST_MAX);

    // TSR.TST_SUMS
    _STR_ADDFIELD_ASCII(strAsciiString, "TSR.tst_sums", QString::number(m_r4TST_SUMS), nFieldSelection, eposTST_SUMS);

    // TSR.TST_SQRS
    _STR_ADDFIELD_ASCII(strAsciiString, "TSR.tst_sqrs", QString::number(m_r4TST_SQRS), nFieldSelection, eposTST_SQRS);
}

void Stdf_TSR_V4::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
    QString			strString;

    // Empty string list first
    listFields.empty();

    // TSR.HEAD_NUM
    _LIST_ADDFIELD_ASCII(listFields, "TSR.head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // TSR.SITE_NUM
    _LIST_ADDFIELD_ASCII(listFields, "TSR.site_num", QString::number(m_u1SITE_NUM), nFieldSelection, eposSITE_NUM);

    // TSR.TEST_TYP
    _CREATEFIELD_FROM_C1_ASCII(m_c1TEST_TYP);
    _LIST_ADDFIELD_ASCII(listFields, "TSR.test_typ", m_strFieldValue_macro, nFieldSelection, eposTEST_TYP);

    // TSR.TEST_NUM
    _LIST_ADDFIELD_ASCII(listFields, "TSR.test_num", QString::number(m_u4TEST_NUM), nFieldSelection, eposTEST_NUM);

    // TSR.EXEC_CNT
    _LIST_ADDFIELD_ASCII(listFields, "TSR.exec_cnt", QString::number(m_u4EXEC_CNT), nFieldSelection, eposEXEC_CNT);

    // TSR.FAIL_CNT
    _LIST_ADDFIELD_ASCII(listFields, "TSR.fail_cnt", QString::number(m_u4FAIL_CNT), nFieldSelection, eposFAIL_CNT);

    // TSR.ALRM_CNT
    _LIST_ADDFIELD_ASCII(listFields, "TSR.alrm_cnt", QString::number(m_u4ALRM_CNT), nFieldSelection, eposALRM_CNT);

    // TSR.TEST_NAM
    _LIST_ADDFIELD_ASCII(listFields, "TSR.test_nam", m_cnTEST_NAM, nFieldSelection, eposTEST_NAM);

    // TSR.SEQ_NAME
    _LIST_ADDFIELD_ASCII(listFields, "TSR.seq_name", m_cnSEQ_NAME, nFieldSelection, eposSEQ_NAME);

    // TSR.TEST_LBL
    _LIST_ADDFIELD_ASCII(listFields, "TSR.test_lbl", m_cnTEST_LBL, nFieldSelection, eposTEST_LBL);

    // TSR.OPT_FLAG
    _CREATEFIELD_FROM_B1_ASCII(m_b1OPT_FLAG)
    _LIST_ADDFIELD_ASCII(listFields, "TSR.opt_flag", m_strFieldValue_macro, nFieldSelection, eposOPT_FLAG);

    // TSR.TEST_TIM
    _LIST_ADDFIELD_ASCII(listFields, "TSR.test_tim", QString::number(m_r4TEST_TIM), nFieldSelection, eposTEST_TIM);

    // TSR.TEST_MIN
    _LIST_ADDFIELD_ASCII(listFields, "TSR.test_min", QString::number(m_r4TEST_MIN), nFieldSelection, eposTEST_MIN);

    // TSR.TEST_MAX
    _LIST_ADDFIELD_ASCII(listFields, "TSR.test_max", QString::number(m_r4TEST_MAX), nFieldSelection, eposTEST_MAX);

    // TSR.TST_SUMS
    _LIST_ADDFIELD_ASCII(listFields, "TSR.tst_sums", QString::number(m_r4TST_SUMS), nFieldSelection, eposTST_SUMS);

    // TSR.TST_SQRS
    _LIST_ADDFIELD_ASCII(listFields, "TSR.tst_sqrs", QString::number(m_r4TST_SQRS), nFieldSelection, eposTST_SQRS);
}

void Stdf_TSR_V4::GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection /*= 0*/)
{
    QString strTabs="";

    // Init XML string
    for(int nIndex=nIndentationLevel; nIndex>0; nIndex--)
        strTabs += "\t";
    strXmlString = strTabs;
    strXmlString += "<tsr>\n";

    // TSR.HEAD_NUM
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // TSR.SITE_NUM
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "site_num", QString::number(m_u1SITE_NUM), nFieldSelection, eposSITE_NUM);

    // TSR.TEST_TYP
    _CREATEFIELD_FROM_C1_XML(m_c1TEST_TYP);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "test_typ", m_strFieldValue_macro, nFieldSelection, eposTEST_TYP);

    // TSR.TEST_NUM
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "test_num", QString::number(m_u4TEST_NUM), nFieldSelection, eposTEST_NUM);

    // TSR.EXEC_CNT
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "exec_cnt", QString::number(m_u4EXEC_CNT), nFieldSelection, eposEXEC_CNT);

    // TSR.FAIL_CNT
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "fail_cnt", QString::number(m_u4FAIL_CNT), nFieldSelection, eposFAIL_CNT);

    // TSR.ALRM_CNT
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "alrm_cnt", QString::number(m_u4ALRM_CNT), nFieldSelection, eposALRM_CNT);

    // TSR.TEST_NAM
    _CREATEFIELD_FROM_CN_XML(m_cnTEST_NAM);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "test_nam", m_strFieldValue_macro, nFieldSelection, eposTEST_NAM);

    // TSR.SEQ_NAME
    _CREATEFIELD_FROM_CN_XML(m_cnSEQ_NAME);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "seq_name", m_strFieldValue_macro, nFieldSelection, eposSEQ_NAME);

    // TSR.TEST_LBL
    _CREATEFIELD_FROM_CN_XML(m_cnTEST_LBL);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "test_lbl", m_strFieldValue_macro, nFieldSelection, eposTEST_LBL);

    // TSR.OPT_FLAG
    _CREATEFIELD_FROM_B1_XML(m_b1OPT_FLAG)
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "opt_flag", m_strFieldValue_macro, nFieldSelection, eposOPT_FLAG);

    // TSR.TEST_TIM
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "test_tim", QString::number(m_r4TEST_TIM), nFieldSelection, eposTEST_TIM);

    // TSR.TEST_MIN
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "test_min", QString::number(m_r4TEST_MIN), nFieldSelection, eposTEST_MIN);

    // TSR.TEST_MAX
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "test_max", QString::number(m_r4TEST_MAX), nFieldSelection, eposTEST_MAX);

    // TSR.TST_SUMS
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "tst_sums", QString::number(m_r4TST_SUMS), nFieldSelection, eposTST_SUMS);

    // TSR.TST_SQRS
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "tst_sqrs", QString::number(m_r4TST_SQRS), nFieldSelection, eposTST_SQRS);

    strXmlString += strTabs;
    strXmlString += "</tsr>\n";
}

void Stdf_TSR_V4::GetAtdfString(QString & strAtdfString)
{
    QString			strString;

    // Reset field index, and line Nb
    m_uiFieldIndex = 0;
    m_uiLineNb = 1;

    // Init string
    strAtdfString = "TSR:";

    // TSR.HEAD_NUM
    // TSR.SITE_NUM
    if(m_u1HEAD_NUM == 255)
    {
        _STR_ADDFIELD_ATDF(strAtdfString, QString(""), eposHEAD_NUM);
        _STR_ADDFIELD_ATDF(strAtdfString, QString(""), eposSITE_NUM);
    }
    else
    {
        _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u1HEAD_NUM), eposHEAD_NUM);
        _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u1SITE_NUM), eposSITE_NUM);
    }

    // TSR.TEST_NUM
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u4TEST_NUM), eposTEST_NUM);

    // TSR.TEST_NAM
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnTEST_NAM, eposTEST_NAM);

    // TSR.TEST_TYP
    _CREATEFIELD_FROM_C1_ASCII(m_c1TEST_TYP);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposTEST_TYP);

    // TSR.EXEC_CNT
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u4EXEC_CNT), eposEXEC_CNT);

    // TSR.FAIL_CNT
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u4FAIL_CNT), eposFAIL_CNT);

    // TSR.ALRM_CNT
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u4ALRM_CNT), eposALRM_CNT);

    // TSR.SEQ_NAME
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnSEQ_NAME, eposSEQ_NAME);

    // TSR.TEST_LBL
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnTEST_LBL, eposTEST_LBL);

    // TSR.TEST_TIM
    if(m_b1OPT_FLAG & 0x04)
        _STR_ADDFIELD_ATDF(strAtdfString, QString(" "), eposTEST_TIM)
    else
        _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_r4TEST_TIM), eposTEST_TIM)

    // TSR.TEST_MIN
    if(m_b1OPT_FLAG & 0x01)
        _STR_ADDFIELD_ATDF(strAtdfString, QString(" "), eposTEST_MIN)
    else
        _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_r4TEST_MIN), eposTEST_MIN)

    // TSR.TEST_MAX
    if(m_b1OPT_FLAG & 0x02)
        _STR_ADDFIELD_ATDF(strAtdfString, QString(" "), eposTEST_MAX)
    else
        _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_r4TEST_MAX), eposTEST_MAX)

    // TSR.TST_SUMS
    if(m_b1OPT_FLAG & 0x10)
        _STR_ADDFIELD_ATDF(strAtdfString, QString(" "), eposTST_SUMS)
    else
        _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_r4TST_SUMS), eposTST_SUMS)

    // TSR.TST_SQRS
    if(m_b1OPT_FLAG & 0x20)
        _STR_ADDFIELD_ATDF(strAtdfString, QString(" "), eposTST_SQRS)
    else
        _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_r4TST_SQRS), eposTST_SQRS);

    _STR_FINISHFIELD_ATDF(strAtdfString);
}

///////////////////////////////////////////////////////////
// PTR RECORD
///////////////////////////////////////////////////////////
Stdf_PTR_V4::Stdf_PTR_V4() : Stdf_Record()
{
    Reset();
}

Stdf_PTR_V4::Stdf_PTR_V4(const Stdf_PTR_V4 & other) : Stdf_Record(other)
{
    *this = other;
}

Stdf_PTR_V4::~Stdf_PTR_V4()
{
    Reset();
}

Stdf_PTR_V4 &Stdf_PTR_V4::operator=(const Stdf_PTR_V4 &other)
{
    if (this != &other)
    {
        // Clean up members
        Reset();

        // Copy base class members
        Stdf_Record::operator =(other);

        // Copy class members
        // Reset field flags
        for(int i=0; i<eposEND; i++)
            m_pFieldFlags[i] = other.m_pFieldFlags[i];

        m_u4TEST_NUM		= other.m_u4TEST_NUM;		// PTR.TEST_NUM
        m_u1HEAD_NUM		= other.m_u1HEAD_NUM;		// PTR.HEAD_NUM
        m_u1SITE_NUM		= other.m_u1SITE_NUM;		// PTR.SITE_NUM
        m_b1TEST_FLG		= other.m_b1TEST_FLG;		// PTR.TEST_FLG
        m_b1PARM_FLG		= other.m_b1PARM_FLG;		// PTR.PARM_FLG
        m_r4RESULT			= other.m_r4RESULT;         // PTR.RESULT
        m_bRESULT_IsNAN		= other.m_bRESULT_IsNAN;
        m_cnTEST_TXT		= other.m_cnTEST_TXT;		// PTR.TEST_TXT
        m_cnALARM_ID		= other.m_cnALARM_ID;		// PTR.ALARM_ID
        m_b1OPT_FLAG		= other.m_b1OPT_FLAG;		// PTR.OPT_FLAG
        m_i1RES_SCAL		= other.m_i1RES_SCAL;		// PTR.RES_SCAL
        m_i1LLM_SCAL		= other.m_i1LLM_SCAL;		// PTR.LLM_SCAL
        m_i1HLM_SCAL		= other.m_i1HLM_SCAL;		// PTR.HLM_SCAL
        m_r4LO_LIMIT		= other.m_r4LO_LIMIT;		// PTR.LO_LIMIT
        m_r4HI_LIMIT		= other.m_r4HI_LIMIT;		// PTR.HI_LIMIT
        m_cnUNITS			= other.m_cnUNITS;          // PTR.UNITS
        m_cnC_RESFMT		= other.m_cnC_RESFMT;		// PTR.C_RESFMT
        m_cnC_LLMFMT		= other.m_cnC_LLMFMT;		// PTR.C_LLMFMT
        m_cnC_HLMFMT		= other.m_cnC_HLMFMT;		// PTR.C_HLMFMT
        m_r4LO_SPEC			= other.m_r4LO_SPEC;		// PTR.LO_SPEC
        m_r4HI_SPEC			= other.m_r4HI_SPEC;		// PTR.HI_SPEC
    }


    return *this;

}

void Stdf_PTR_V4::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Select fields for reduced list

    // Reset Data
    m_u4TEST_NUM		= 0;		// PTR.TEST_NUM
    m_u1HEAD_NUM		= 1;		// PTR.HEAD_NUM
    m_u1SITE_NUM		= 1;		// PTR.SITE_NUM
    m_b1TEST_FLG		= 0;		// PTR.TEST_FLG
    m_b1PARM_FLG		= 0;		// PTR.PARM_FLG
    m_r4RESULT			= 0.0;		// PTR.RESULT
    m_bRESULT_IsNAN		= false;
    m_cnTEST_TXT		= "";		// PTR.TEST_TXT
    m_cnALARM_ID		= "";		// PTR.ALARM_ID
    m_b1OPT_FLAG		= eOPT_FLAG_ALL;		// PTR.OPT_FLAG
    m_i1RES_SCAL		= 0;		// PTR.RES_SCAL
    m_i1LLM_SCAL		= 0;		// PTR.LLM_SCAL
    m_i1HLM_SCAL		= 0;		// PTR.HLM_SCAL
    m_r4LO_LIMIT		= 0.0;		// PTR.LO_LIMIT
    m_r4HI_LIMIT		= 0.0;		// PTR.HI_LIMIT
    m_cnUNITS			= "";		// PTR.UNITS
    m_cnC_RESFMT		= "";		// PTR.C_RESFMT
    m_cnC_LLMFMT		= "";		// PTR.C_LLMFMT
    m_cnC_HLMFMT		= "";		// PTR.C_HLMFMT
    m_r4LO_SPEC			= 0.0;		// PTR.LO_SPEC
    m_r4HI_SPEC			= 0.0;		// PTR.HI_SPEC
    mLastFieldAtdf      = (int)eposFIRST_OPTIONAL;

    // Call Reset base method
    Stdf_Record::Reset();
}

bool Stdf_PTR_V4::IsFieldValid(int nFieldPos) const
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;

    return false;
}

bool Stdf_PTR_V4::IsTestExecuted()
{
    return ((m_b1TEST_FLG & STDF_MASK_BIT4) == 0);
}

bool Stdf_PTR_V4::IsTestFail()
{
    return IsTestFail(*this);
}

bool Stdf_PTR_V4::IsTestFail(Stdf_PTR_V4 & clRefPTR)
{
    // Check if test executed
    if(m_b1TEST_FLG & STDF_MASK_BIT4)
        return false;

    // Check if Pass/Fail flag valid
    if((m_b1TEST_FLG & STDF_MASK_BIT6) == 0)
        return (m_b1TEST_FLG & STDF_MASK_BIT7);

    // Pass/Fail flag not valid, check if test result is valid/reliable
    if((m_b1TEST_FLG & STDF_MASK_BIT1) || (m_b1TEST_FLG & STDF_MASK_BIT2))
        return false;

    // Test result valid, compare to limits
    if(clRefPTR.IsFieldValid(eposLO_LIMIT))
    {
        // Low limit
        if(m_b1PARM_FLG & STDF_MASK_BIT6)
        {
            if(m_r4RESULT < clRefPTR.m_r4LO_LIMIT)
                return true;
        }
        else
        {
            if(m_r4RESULT <= clRefPTR.m_r4LO_LIMIT)
                return true;
        }
    }
    if(clRefPTR.IsFieldValid(eposHI_LIMIT))
    {
        // High limit
        if(m_b1PARM_FLG & STDF_MASK_BIT7)
        {
            if(m_r4RESULT > clRefPTR.m_r4HI_LIMIT)
                return true;
        }
        else
        {
            if(m_r4RESULT >= clRefPTR.m_r4HI_LIMIT)
                return true;
        }
    }

    return false;
}

void Stdf_PTR_V4::UpdatePassFailInfo(Stdf_PTR_V4 & clRefPTR)
{
    // Check if test executed
    if(m_b1TEST_FLG & STDF_MASK_BIT4)
        return;

    // Check if test result is valid/reliable
    if((m_b1TEST_FLG & STDF_MASK_BIT1) || (m_b1TEST_FLG & STDF_MASK_BIT2))
        return;

    // Test result valid
    m_b1TEST_FLG &= ~STDF_MASK_BIT6;				// Pass/Fail flag is valid
    m_b1TEST_FLG &= ~STDF_MASK_BIT7;				// Test is Pass
    m_b1PARM_FLG &= ~STDF_MASK_BIT3;				// Measured value not high
    m_b1PARM_FLG &= ~STDF_MASK_BIT4;				// Measured value not low
    m_b1PARM_FLG &= ~STDF_MASK_BIT5;				// Test failed or passed standard limits

    // Check result against limits
    if(clRefPTR.IsFieldValid(eposLO_LIMIT))
    {
        // Low limit
        if(m_b1PARM_FLG & STDF_MASK_BIT6)
        {
            if(m_r4RESULT < clRefPTR.m_r4LO_LIMIT)
            {
                m_b1TEST_FLG |= STDF_MASK_BIT7;		// Test is Fail
                m_b1PARM_FLG |= STDF_MASK_BIT4;		// Measured value low
                return;
            }
        }
        else
        {
            if(m_r4RESULT <= clRefPTR.m_r4LO_LIMIT)
            {
                m_b1TEST_FLG |= STDF_MASK_BIT7;		// Test is Fail
                m_b1PARM_FLG |= STDF_MASK_BIT4;		// Measured value low
                return;
            }
        }
    }
    if(clRefPTR.IsFieldValid(eposHI_LIMIT))
    {
        // High limit
        if(m_b1PARM_FLG & STDF_MASK_BIT7)
        {
            if(m_r4RESULT > clRefPTR.m_r4HI_LIMIT)
            {
                m_b1TEST_FLG |= STDF_MASK_BIT7;		// Test is Fail
                m_b1PARM_FLG |= STDF_MASK_BIT3;		// Measured value high
                return;
            }
        }
        else
        {
            if(m_r4RESULT >= clRefPTR.m_r4HI_LIMIT)
            {
                m_b1TEST_FLG |= STDF_MASK_BIT7;		// Test is Fail
                m_b1PARM_FLG |= STDF_MASK_BIT3;		// Measured value high
                return;
            }
        }
    }
}

QString Stdf_PTR_V4::GetRecordShortName(void)
{
    return "PTR";
}

QString Stdf_PTR_V4::GetRecordLongName(void)
{
    return "Parametric Test Record";
}

int Stdf_PTR_V4::GetRecordType(void)
{
    return Rec_PTR;
}

bool Stdf_PTR_V4::Read(GS::StdLib::Stdf & clStdf)
{
    char	szString[STDF_MAX_U1+1];
    long	dwData;
    float	fData;
    BYTE	bData;

    // First reset data
    Reset();

    // PTR.TEST_NUM
    _FIELD_CHECKREAD(clStdf.ReadDword(&dwData), eposTEST_NUM);
    _FIELD_SET(m_u4TEST_NUM = stdf_type_u4(dwData), true, eposTEST_NUM);

    // PTR.HEAD_NUM
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposHEAD_NUM);
    _FIELD_SET(m_u1HEAD_NUM = stdf_type_u1(bData), true, eposHEAD_NUM);

    // PTR.SITE_NUM
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposSITE_NUM);
    _FIELD_SET(m_u1SITE_NUM = stdf_type_u1(bData), true, eposSITE_NUM);

    // PTR.TEST_FLG
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposTEST_FLG);
    _FIELD_SET(m_b1TEST_FLG = stdf_type_b1(bData), true, eposTEST_FLG);

    // PTR.PARM_FLG
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposPARM_FLG);
    _FIELD_SET(m_b1PARM_FLG = stdf_type_b1(bData), true, eposPARM_FLG);

    // PTR.RESULT
    _FIELD_CHECKREAD(clStdf.ReadFloat(&fData, &m_bRESULT_IsNAN), eposRESULT);
    _FIELD_SET(m_r4RESULT = stdf_type_r4(fData), (m_b1TEST_FLG & STDF_MASK_BIT1) == 0, eposRESULT);

    // PTR.TEST_TXT
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposTEST_TXT);
    _FIELD_SET(m_cnTEST_TXT = szString, !m_cnTEST_TXT.isEmpty(), eposTEST_TXT);

    // PTR.ALARM_ID
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposALARM_ID);
    _FIELD_SET(m_cnALARM_ID = szString, !m_cnALARM_ID.isEmpty(), eposALARM_ID);

    // PTR.OPT_FLAG
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposOPT_FLAG);
    _FIELD_SET(m_b1OPT_FLAG = stdf_type_b1(bData), true, eposOPT_FLAG);

    // PTR.RES_SCAL
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposRES_SCAL);
    _FIELD_SET(m_i1RES_SCAL = stdf_type_i1(bData), (m_b1OPT_FLAG & STDF_MASK_BIT0) == 0, eposRES_SCAL);

    // PTR.LLM_SCAL
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposLLM_SCAL);
    _FIELD_SET(m_i1LLM_SCAL = stdf_type_i1(bData), (m_b1OPT_FLAG & (STDF_MASK_BIT4 | STDF_MASK_BIT6)) == 0, eposLLM_SCAL);

    // PTR.HLM_SCAL
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposHLM_SCAL);
    _FIELD_SET(m_i1HLM_SCAL = stdf_type_i1(bData), (m_b1OPT_FLAG & (STDF_MASK_BIT5 | STDF_MASK_BIT7)) == 0, eposHLM_SCAL);

    // PTR.LO_LIMIT
    _FIELD_CHECKREAD(clStdf.ReadFloat(&fData), eposLO_LIMIT);
    _FIELD_SET(m_r4LO_LIMIT = stdf_type_r4(fData), (m_b1OPT_FLAG & (STDF_MASK_BIT4 | STDF_MASK_BIT6)) == 0, eposLO_LIMIT);

    // PTR.HI_LIMIT
    _FIELD_CHECKREAD(clStdf.ReadFloat(&fData), eposHI_LIMIT);
    _FIELD_SET(m_r4HI_LIMIT = stdf_type_r4(fData), (m_b1OPT_FLAG & (STDF_MASK_BIT5 | STDF_MASK_BIT7)) == 0, eposHI_LIMIT);

    // PTR.UNITS
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposUNITS);
    _FIELD_SET(m_cnUNITS = szString, !m_cnUNITS.isEmpty(), eposUNITS);

    // PTR.C_RESFMT
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposC_RESFMT);
    _FIELD_SET(m_cnC_RESFMT = szString, !m_cnC_RESFMT.isEmpty(), eposC_RESFMT);

    // PTR.C_LLMFMT
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposC_LLMFMT);
    _FIELD_SET(m_cnC_LLMFMT = szString, !m_cnC_LLMFMT.isEmpty(), eposC_LLMFMT);

    // PTR.C_HLMFMT
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposC_HLMFMT);
    _FIELD_SET(m_cnC_HLMFMT = szString, !m_cnC_HLMFMT.isEmpty(), eposC_HLMFMT);

    // PTR.LO_SPEC
    _FIELD_CHECKREAD(clStdf.ReadFloat(&fData), eposLO_SPEC);
    _FIELD_SET(m_r4LO_SPEC = stdf_type_r4(fData), (m_b1OPT_FLAG & STDF_MASK_BIT2) == 0, eposLO_SPEC);

    // PTR.HI_SPEC
    _FIELD_CHECKREAD(clStdf.ReadFloat(&fData), eposHI_SPEC);
    _FIELD_SET(m_r4HI_SPEC = stdf_type_r4(fData), (m_b1OPT_FLAG & STDF_MASK_BIT3) == 0, eposHI_SPEC);

    return true;
}

bool Stdf_PTR_V4::Write(GS::StdLib::Stdf & clStdf)
{
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;

    RecordReadInfo.iRecordType = STDF_PTR_TYPE;
    RecordReadInfo.iRecordSubType = STDF_PTR_STYPE;
    clStdf.WriteHeader(&RecordReadInfo);

    // PTR.TEST_NUM
    _FIELD_CHECKWRITE(clStdf.WriteDword(DWORD(m_u4TEST_NUM)), eposTEST_NUM, clStdf.WriteRecord());

    // PTR.HEAD_NUM
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_u1HEAD_NUM)), eposHEAD_NUM, clStdf.WriteRecord());

    // PTR.SITE_NUM
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_u1SITE_NUM)), eposSITE_NUM, clStdf.WriteRecord());

    // PTR.TEST_FLG
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_b1TEST_FLG)), eposTEST_FLG, clStdf.WriteRecord());

    // PTR.PARM_FLG
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_b1PARM_FLG)), eposPARM_FLG, clStdf.WriteRecord());

    // PTR.RESULT
    _FIELD_CHECKWRITE(clStdf.WriteFloat(FLOAT(m_r4RESULT)), eposRESULT, clStdf.WriteRecord());

    // PTR.TEST_TXT
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnTEST_TXT.toLatin1().constData()), eposTEST_TXT, clStdf.WriteRecord());

    // PTR.ALARM_ID
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnALARM_ID.toLatin1().constData()), eposALARM_ID, clStdf.WriteRecord());

    // PTR.OPT_FLAG
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_b1OPT_FLAG)), eposOPT_FLAG, clStdf.WriteRecord());

    // PTR.RES_SCAL
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_i1RES_SCAL)), eposRES_SCAL, clStdf.WriteRecord());

    // PTR.LLM_SCAL
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_i1LLM_SCAL)), eposLLM_SCAL, clStdf.WriteRecord());

    // PTR.HLM_SCAL
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_i1HLM_SCAL)), eposHLM_SCAL, clStdf.WriteRecord());

    // PTR.LO_LIMIT
    _FIELD_CHECKWRITE(clStdf.WriteFloat(FLOAT(m_r4LO_LIMIT)), eposLO_LIMIT, clStdf.WriteRecord());

    // PTR.HI_LIMIT
    _FIELD_CHECKWRITE(clStdf.WriteFloat(FLOAT(m_r4HI_LIMIT)), eposHI_LIMIT, clStdf.WriteRecord());

    // PTR.UNITS
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnUNITS.toLatin1().constData()), eposUNITS, clStdf.WriteRecord());

    // PTR.C_RESFMT
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnC_RESFMT.toLatin1().constData()), eposC_RESFMT, clStdf.WriteRecord());

    // PTR.C_LLMFMT
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnC_LLMFMT.toLatin1().constData()), eposC_LLMFMT, clStdf.WriteRecord());

    // PTR.C_HLMFMT
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnC_HLMFMT.toLatin1().constData()), eposC_HLMFMT, clStdf.WriteRecord());

    // PTR.LO_SPEC
    _FIELD_CHECKWRITE(clStdf.WriteFloat(FLOAT(m_r4LO_SPEC)), eposLO_SPEC, clStdf.WriteRecord());

    // PTR.HI_SPEC
    _FIELD_CHECKWRITE(clStdf.WriteFloat(FLOAT(m_r4HI_SPEC)), eposHI_SPEC, clStdf.WriteRecord());

    clStdf.WriteRecord();

    return true;
}

void Stdf_PTR_V4::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    QString	strString;

    // Empty string first
    strAsciiString = "";

    // PTR.TEST_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "PTR.test_num", QString::number(m_u4TEST_NUM), nFieldSelection, eposTEST_NUM);

    // PTR.HEAD_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "PTR.head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // PTR.SITE_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "PTR.site_num", QString::number(m_u1SITE_NUM), nFieldSelection, eposSITE_NUM);

    // PTR.TEST_FLG
    _CREATEFIELD_FROM_B1_ASCII(m_b1TEST_FLG)
    _STR_ADDFIELD_ASCII(strAsciiString, "PTR.test_flg", m_strFieldValue_macro, nFieldSelection, eposTEST_FLG);

    // PTR.PARM_FLG
    _CREATEFIELD_FROM_B1_ASCII(m_b1PARM_FLG)
    _STR_ADDFIELD_ASCII(strAsciiString, "PTR.parm_flg", m_strFieldValue_macro, nFieldSelection, eposPARM_FLG);

    // PTR.RESULT
    if(m_bRESULT_IsNAN)
        strString = "NaN";
    else
    {
        // todo : use %a as soon as we support C99
        char lBuffer[256]; lBuffer[0]='\0';
        // GCORE-859
        switch(mFormat)
        {
            case 'f': strString = QString::number(m_r4RESULT,'f', mPrecision);
            break;
            case 'a':
                // With precision 12: exple: 0x0.000000000000p+0
                // Let s move to precision level 4
                // PTR.result    = 0x1.480f0c0000000000000000000000p-9 | PTR.result    = 0x1.480f0e0000000000000000000000p-9
                // Let s move to precision level 3
                // exple: PTR.result    = 0x1.7103p+0				      |	PTR.result    = 0x1.7104p+0
                // Let s move to precision level 2
                // PTR.result    = -0x1.4aep-1      |	PTR.result    = -0x1.4afp-1
                // PTR.result    = 0x1.3dp+0				      |	PTR.result    = 0x1.3ep+0
                sprintf(lBuffer, "%.1f", m_r4RESULT);
                strString=lBuffer;
                //strString = strString.sprintf("%a", m_r4RESULT); // QString sprintf is deprecated...
                break;
            case 'q': // special QA output
                strString = QString::number(m_r4RESULT,'e', mPrecision);
                strString.resize(4);
            break;
            default:
            case 'e': strString = QString::number(m_r4RESULT,'e', mPrecision);
            break;

        }
    }
    _STR_ADDFIELD_ASCII(strAsciiString, "PTR.result", strString, nFieldSelection, eposRESULT);

    // PTR.TEST_TXT
    _STR_ADDFIELD_ASCII(strAsciiString, "PTR.test_txt", m_cnTEST_TXT, nFieldSelection, eposTEST_TXT);

    // PTR.ALARM_ID
    _STR_ADDFIELD_ASCII(strAsciiString, "PTR.alarm_id", m_cnALARM_ID, nFieldSelection, eposALARM_ID);

    // PTR.OPT_FLAG
    _CREATEFIELD_FROM_B1_ASCII(m_b1OPT_FLAG)
    _STR_ADDFIELD_ASCII(strAsciiString, "PTR.opt_flag", m_strFieldValue_macro, nFieldSelection, eposOPT_FLAG);

    // PTR.RES_SCAL
    _STR_ADDFIELD_ASCII(strAsciiString, "PTR.res_scal", QString::number(m_i1RES_SCAL), nFieldSelection, eposRES_SCAL);

    // PTR.LLM_SCAL
    _STR_ADDFIELD_ASCII(strAsciiString, "PTR.llm_scal", QString::number(m_i1LLM_SCAL), nFieldSelection, eposLLM_SCAL);

    // PTR.HLM_SCAL
    _STR_ADDFIELD_ASCII(strAsciiString, "PTR.hlm_scal", QString::number(m_i1HLM_SCAL), nFieldSelection, eposHLM_SCAL);

    // PTR.LO_LIMIT
    strString = QString::number(m_r4LO_LIMIT,'e', mPrecision);
    _STR_ADDFIELD_ASCII(strAsciiString, "PTR.lo_limit", strString, nFieldSelection, eposLO_LIMIT);

    // PTR.HI_LIMIT
    strString = QString::number(m_r4HI_LIMIT,'e', mPrecision);
    _STR_ADDFIELD_ASCII(strAsciiString, "PTR.hi_limit", strString, nFieldSelection, eposHI_LIMIT);

    // PTR.UNITS
    _STR_ADDFIELD_ASCII(strAsciiString, "PTR.units", m_cnUNITS, nFieldSelection, eposUNITS);

    // PTR.C_RESFMT
    _STR_ADDFIELD_ASCII(strAsciiString, "PTR.c_resfmt", m_cnC_RESFMT, nFieldSelection, eposC_RESFMT);

    // PTR.C_LLMFMT
    _STR_ADDFIELD_ASCII(strAsciiString, "PTR.c_llmfmt", m_cnC_LLMFMT, nFieldSelection, eposC_LLMFMT);

    // PTR.C_HLMFMT
    _STR_ADDFIELD_ASCII(strAsciiString, "PTR.c_hlmfmt", m_cnC_HLMFMT, nFieldSelection, eposC_HLMFMT);

    // PTR.LO_SPEC
    strString = QString::number(m_r4LO_SPEC,'e', mPrecision);
    _STR_ADDFIELD_ASCII(strAsciiString, "PTR.lo_spec", strString, nFieldSelection, eposLO_SPEC);

    // PTR.HI_SPEC
    strString = QString::number(m_r4HI_SPEC,'e', mPrecision);
    _STR_ADDFIELD_ASCII(strAsciiString, "PTR.hi_spec", strString, nFieldSelection, eposHI_SPEC);
}

void Stdf_PTR_V4::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
    QString			strString;

    // Empty string list first
    listFields.empty();

    // PTR.TEST_NUM
    _LIST_ADDFIELD_ASCII(listFields, "PTR.test_num", QString::number(m_u4TEST_NUM), nFieldSelection, eposTEST_NUM);

    // PTR.HEAD_NUM
    _LIST_ADDFIELD_ASCII(listFields, "PTR.head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // PTR.SITE_NUM
    _LIST_ADDFIELD_ASCII(listFields, "PTR.site_num", QString::number(m_u1SITE_NUM), nFieldSelection, eposSITE_NUM);

    // PTR.TEST_FLG
    _CREATEFIELD_FROM_B1_ASCII(m_b1TEST_FLG)
    _LIST_ADDFIELD_ASCII(listFields, "PTR.test_flg", m_strFieldValue_macro, nFieldSelection, eposTEST_FLG);

    // PTR.PARM_FLG
    _CREATEFIELD_FROM_B1_ASCII(m_b1PARM_FLG)
    _LIST_ADDFIELD_ASCII(listFields, "PTR.parm_flg", m_strFieldValue_macro, nFieldSelection, eposPARM_FLG);

    // PTR.RESULT
    if(m_bRESULT_IsNAN)
        strString = "NaN";
    else
        strString = QString::number(m_r4RESULT);
    _LIST_ADDFIELD_ASCII(listFields, "PTR.result", strString, nFieldSelection, eposRESULT);

    // PTR.TEST_TXT
    _LIST_ADDFIELD_ASCII(listFields, "PTR.test_txt", m_cnTEST_TXT, nFieldSelection, eposTEST_TXT);

    // PTR.ALARM_ID
    _LIST_ADDFIELD_ASCII(listFields, "PTR.alarm_id", m_cnALARM_ID, nFieldSelection, eposALARM_ID);

    // PTR.OPT_FLAG
    _CREATEFIELD_FROM_B1_ASCII(m_b1OPT_FLAG)
    _LIST_ADDFIELD_ASCII(listFields, "PTR.opt_flag", m_strFieldValue_macro, nFieldSelection, eposOPT_FLAG);

    // PTR.RES_SCAL
    _LIST_ADDFIELD_ASCII(listFields, "PTR.res_scal", QString::number(m_i1RES_SCAL), nFieldSelection, eposRES_SCAL);

    // PTR.LLM_SCAL
    _LIST_ADDFIELD_ASCII(listFields, "PTR.llm_scal", QString::number(m_i1LLM_SCAL), nFieldSelection, eposLLM_SCAL);

    // PTR.HLM_SCAL
    _LIST_ADDFIELD_ASCII(listFields, "PTR.hlm_scal", QString::number(m_i1HLM_SCAL), nFieldSelection, eposHLM_SCAL);

    // PTR.LO_LIMIT
    _LIST_ADDFIELD_ASCII(listFields, "PTR.lo_limit", QString::number(m_r4LO_LIMIT), nFieldSelection, eposLO_LIMIT);

    // PTR.HI_LIMIT
    _LIST_ADDFIELD_ASCII(listFields, "PTR.hi_limit", QString::number(m_r4HI_LIMIT), nFieldSelection, eposHI_LIMIT);

    // PTR.UNITS
    _LIST_ADDFIELD_ASCII(listFields, "PTR.units", m_cnUNITS, nFieldSelection, eposUNITS);

    // PTR.C_RESFMT
    _LIST_ADDFIELD_ASCII(listFields, "PTR.c_resfmt", m_cnC_RESFMT, nFieldSelection, eposC_RESFMT);

    // PTR.C_LLMFMT
    _LIST_ADDFIELD_ASCII(listFields, "PTR.c_llmfmt", m_cnC_LLMFMT, nFieldSelection, eposC_LLMFMT);

    // PTR.C_HLMFMT
    _LIST_ADDFIELD_ASCII(listFields, "PTR.c_hlmfmt", m_cnC_HLMFMT, nFieldSelection, eposC_HLMFMT);

    // PTR.LO_SPEC
    _LIST_ADDFIELD_ASCII(listFields, "PTR.lo_spec", QString::number(m_r4LO_SPEC), nFieldSelection, eposLO_SPEC);

    // PTR.HI_SPEC
    _LIST_ADDFIELD_ASCII(listFields, "PTR.hi_spec", QString::number(m_r4HI_SPEC), nFieldSelection, eposHI_SPEC);
}

void Stdf_PTR_V4::GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection /*= 0*/)
{
    QString strTabs="", strString;

    // Init XML string
    for(int nIndex=nIndentationLevel; nIndex>0; nIndex--)
        strTabs += "\t";
    strXmlString = strTabs;
    strXmlString += "<ptr>\n";

    // PTR.TEST_NUM
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "test_num", QString::number(m_u4TEST_NUM), nFieldSelection, eposTEST_NUM);

    // PTR.HEAD_NUM
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // PTR.SITE_NUM
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "site_num", QString::number(m_u1SITE_NUM), nFieldSelection, eposSITE_NUM);

    // PTR.TEST_FLG
    _CREATEFIELD_FROM_B1_XML(m_b1TEST_FLG)
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "test_flg", m_strFieldValue_macro, nFieldSelection, eposTEST_FLG);

    // PTR.PARM_FLG
    _CREATEFIELD_FROM_B1_XML(m_b1PARM_FLG)
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "parm_flg", m_strFieldValue_macro, nFieldSelection, eposPARM_FLG);

    // PTR.RESULT
    if(m_bRESULT_IsNAN)
        strString = "NaN";
    else
        strString = QString::number(m_r4RESULT);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "result", strString, nFieldSelection, eposRESULT);

    // PTR.TEST_TXT
    _CREATEFIELD_FROM_CN_XML(m_cnTEST_TXT);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "test_txt", m_strFieldValue_macro, nFieldSelection, eposTEST_TXT);

    // PTR.ALARM_ID
    _CREATEFIELD_FROM_CN_XML(m_cnALARM_ID);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "alarm_id", m_strFieldValue_macro, nFieldSelection, eposALARM_ID);

    // PTR.OPT_FLAG
    _CREATEFIELD_FROM_B1_XML(m_b1OPT_FLAG)
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "opt_flag", m_strFieldValue_macro, nFieldSelection, eposOPT_FLAG);

    // PTR.RES_SCAL
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "res_scal", QString::number(m_i1RES_SCAL), nFieldSelection, eposRES_SCAL);

    // PTR.LLM_SCAL
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "llm_scal", QString::number(m_i1LLM_SCAL), nFieldSelection, eposLLM_SCAL);

    // PTR.HLM_SCAL
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "hlm_scal", QString::number(m_i1HLM_SCAL), nFieldSelection, eposHLM_SCAL);

    // PTR.LO_LIMIT
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "lo_limit", QString::number(m_r4LO_LIMIT), nFieldSelection, eposLO_LIMIT);

    // PTR.HI_LIMIT
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "hi_limit", QString::number(m_r4HI_LIMIT), nFieldSelection, eposHI_LIMIT);

    // PTR.UNITS
    _CREATEFIELD_FROM_CN_XML(m_cnUNITS);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "units", m_strFieldValue_macro, nFieldSelection, eposUNITS);

    // PTR.C_RESFMT
    _CREATEFIELD_FROM_CN_XML(m_cnC_RESFMT);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "c_resfmt", m_strFieldValue_macro, nFieldSelection, eposC_RESFMT);

    // PTR.C_LLMFMT
    _CREATEFIELD_FROM_CN_XML(m_cnC_LLMFMT);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "c_llmfmt", m_strFieldValue_macro, nFieldSelection, eposC_LLMFMT);

    // PTR.C_HLMFMT
    _CREATEFIELD_FROM_CN_XML(m_cnC_HLMFMT);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "c_hlmfmt", m_strFieldValue_macro, nFieldSelection, eposC_HLMFMT);

    // PTR.LO_SPEC
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "lo_spec", QString::number(m_r4LO_SPEC), nFieldSelection, eposLO_SPEC);

    // PTR.HI_SPEC
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "hi_spec", QString::number(m_r4HI_SPEC), nFieldSelection, eposHI_SPEC);

    strXmlString += strTabs;
    strXmlString += "</ptr>\n";
}

void Stdf_PTR_V4::GetAtdfString(QString & strAtdfString)
{
    QString	strString;

    // Reset field index, and line Nb
    m_uiFieldIndex = 0;
    m_uiLineNb = 1;

    // Init string
    strAtdfString = "PTR:";

    // PTR.TEST_NUM
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u4TEST_NUM), eposTEST_NUM);

    // PTR.HEAD_NUM
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u1HEAD_NUM), eposHEAD_NUM);

    // PTR.SITE_NUM
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u1SITE_NUM), eposSITE_NUM);

    // PTR.RESULT
    if(m_bRESULT_IsNAN)
        strString = "";
    else
    {
        if (mFormat=='q') // special QA output
        {   strString = QString::number(m_r4RESULT, 'f', mPrecision);
            //strString.resize(mPrecision);
        }
        else
        {
            strString = QString::number(m_r4RESULT,'e', mPrecision);
        }
    }
    _STR_ADDFIELD_ATDF(strAtdfString, strString, eposRESULT);

    // PTR.Pass/Fail Flag
    m_strFieldValue_macro = " ";
    if((m_b1TEST_FLG & 0x40) == 0)
    {
        if((m_b1PARM_FLG & 0x20))
            m_strFieldValue_macro = "A";
        else
        {
            if(m_b1TEST_FLG & 0x80)
                m_strFieldValue_macro = "F";
            else
                m_strFieldValue_macro = "P";
        }
    }
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposTEST_FLG);

    // PTR.Alarm Flags
    m_strFieldValue_macro = "";
    if(m_b1TEST_FLG & 0x01)
        m_strFieldValue_macro += "A";
    if(m_b1TEST_FLG & 0x04)
        m_strFieldValue_macro += "U";
    if(m_b1TEST_FLG & 0x08)
        m_strFieldValue_macro += "T";
    if(m_b1TEST_FLG & 0x10)
        m_strFieldValue_macro += "N";
    if(m_b1TEST_FLG & 0x20)
        m_strFieldValue_macro += "X";
    if(m_b1PARM_FLG & 0x01)
        m_strFieldValue_macro += "S";
    if(m_b1PARM_FLG & 0x02)
        m_strFieldValue_macro += "D";
    if(m_b1PARM_FLG & 0x04)
        m_strFieldValue_macro += "O";
    if(m_b1PARM_FLG & 0x08)
        m_strFieldValue_macro += "H";
    if(m_b1PARM_FLG & 0x10)
        m_strFieldValue_macro += "L";
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposTEST_FLG);

    // PTR.TEST_TXT
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnTEST_TXT, eposTEST_TXT);

    // PTR.ALARM_ID
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnALARM_ID, eposALARM_ID);

    // PTR.Limit Compare
    m_strFieldValue_macro = "";
    if(m_b1PARM_FLG & 0x40)
        m_strFieldValue_macro += "L";
    if(m_b1PARM_FLG & 0x80)
        m_strFieldValue_macro += "H";
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposPARM_FLG);

    // PTR.UNITS
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnUNITS, eposUNITS);

    // PTR.LO_LIMIT
    m_strFieldValue_macro = "";
    if((m_b1OPT_FLAG & 0x50) == 0)
        m_strFieldValue_macro = QString::number(m_r4LO_LIMIT,'e', mPrecision);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposLO_LIMIT);

    // PTR.HI_LIMIT
    m_strFieldValue_macro = "";
    if((m_b1OPT_FLAG & 0xa0) == 0)
        m_strFieldValue_macro = QString::number(m_r4HI_LIMIT,'e', mPrecision);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposHI_LIMIT);

    // PTR.C_RESFMT
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnC_RESFMT, eposC_RESFMT);

    // PTR.C_LLMFMT
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnC_LLMFMT, eposC_LLMFMT);

    // PTR.C_HLMFMT
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnC_HLMFMT, eposC_HLMFMT);

    // PTR.LO_SPEC
    m_strFieldValue_macro = "";
    if((m_b1OPT_FLAG & 0x04) == 0)
        m_strFieldValue_macro = QString::number(m_r4LO_SPEC,'e', mPrecision);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposLO_SPEC);

    // PTR.HI_SPEC
    m_strFieldValue_macro = "";
    if((m_b1OPT_FLAG & 0x08) == 0)
        m_strFieldValue_macro = QString::number(m_r4HI_SPEC,'e', mPrecision);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposHI_SPEC);

    // PTR.RES_SCAL
    m_strFieldValue_macro = "";
    if((m_b1OPT_FLAG & 0x01) == 0)
        m_strFieldValue_macro = QString::number(m_i1RES_SCAL);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposRES_SCAL);

    // PTR.LLM_SCAL
    m_strFieldValue_macro = "";
    if((m_b1OPT_FLAG & 0x50) == 0)
        m_strFieldValue_macro = QString::number(m_i1LLM_SCAL);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposLLM_SCAL);

    // PTR.HLM_SCAL
    m_strFieldValue_macro = "";
    if((m_b1OPT_FLAG & 0xa0) == 0)
        m_strFieldValue_macro = QString::number(m_i1HLM_SCAL);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposHLM_SCAL);

    _STR_FINISHFIELD_ATDF(strAtdfString);
}

///////////////////////////////////////////////////////////
// MPR RECORD
///////////////////////////////////////////////////////////
Stdf_MPR_V4::Stdf_MPR_V4() : Stdf_Record()
{
    m_kn1RTN_STAT		= NULL;
    m_kr4RTN_RSLT		= NULL;
    m_kbRTN_RSLT_IsNan	= NULL;
    m_ku2RTN_INDX		= NULL;
    Reset();
}

Stdf_MPR_V4::Stdf_MPR_V4(const Stdf_MPR_V4& other) : Stdf_Record(other)
{
    m_kn1RTN_STAT		= NULL;
    m_kr4RTN_RSLT		= NULL;
    m_kbRTN_RSLT_IsNan	= NULL;
    m_ku2RTN_INDX		= NULL;

    *this = other;
}

Stdf_MPR_V4::~Stdf_MPR_V4()
{
    Reset();
}

void Stdf_MPR_V4::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Select fields for reduced list

    // Reset Data
    m_u4TEST_NUM	= 0;		// MPR.TEST_NUM
    m_u1HEAD_NUM	= 1;		// MPR.HEAD_NUM
    m_u1SITE_NUM	= 1;		// MPR.SITE_NUM
    m_b1TEST_FLG	= 0;		// MPR.TEST_FLG
    m_b1PARM_FLG	= 0;		// MPR.PARM_FLG
    m_u2RTN_ICNT	= 0;		// MPR.RTN_ICNT
    m_u2RSLT_CNT	= 0;		// MPR.RSLT_CNT
    if(m_kn1RTN_STAT != NULL)	// MPR.RTN_STAT
    {
        delete [] m_kn1RTN_STAT;
        m_kn1RTN_STAT = NULL;
    }
    if(m_kr4RTN_RSLT != NULL)	// MPR.RTN_RSLT
    {
        delete [] m_kr4RTN_RSLT;
        m_kr4RTN_RSLT = NULL;
    }
    if(m_kbRTN_RSLT_IsNan != NULL)
    {
        delete [] m_kbRTN_RSLT_IsNan;
        m_kbRTN_RSLT_IsNan = NULL;
    }
    m_cnTEST_TXT	= "";		// MPR.TEST_TXT
    m_cnALARM_ID	= "";		// MPR.ALARM_ID
    m_b1OPT_FLAG	= eOPT_FLAG_ALL;		// MPR.OPT_FLAG
    m_i1RES_SCAL	= 0;		// MPR.RES_SCAL
    m_i1LLM_SCAL	= 0;		// MPR.LLM_SCAL
    m_i1HLM_SCAL	= 0;		// MPR.HLM_SCAL
    m_r4LO_LIMIT	= 0.0;		// MPR.LO_LIMIT
    m_r4HI_LIMIT	= 0.0;		// MPR.HI_LIMIT
    m_r4START_IN	= 0.0;		// MPR.START_IN
    m_r4INCR_IN		= 0.0;		// MPR.INCR_IN
    if(m_ku2RTN_INDX != NULL)	// MPR.RTN_INDX
    {
        delete [] m_ku2RTN_INDX;
        m_ku2RTN_INDX = NULL;
    }
    m_cnUNITS		= "";		// MPR.UNITS
    m_cnUNITS_IN	= "";		// MPR.UNITS_IN
    m_cnC_RESFMT	= "";		// MPR.C_RESFMT
    m_cnC_LLMFMT	= "";		// MPR.C_LLMFMT
    m_cnC_HLMFMT	= "";		// MPR.C_HLMFMT
    m_r4LO_SPEC		= 0.0;		// MPR.LO_SPEC
    m_r4HI_SPEC		= 0.0;		// MPR.HI_SPEC
    mLastFieldAtdf  = eposFIRST_OPTIONAL;

    // Call Reset base method
    Stdf_Record::Reset();
}

Stdf_MPR_V4& Stdf_MPR_V4::operator=(const Stdf_MPR_V4& other)
{
    if (this != &other)
    {
        Reset();

        // Reset field flags
        for(int i=0; i<eposEND; i++)
            m_pFieldFlags[i] = other.m_pFieldFlags[i];

        // Reset Data
        m_u4TEST_NUM	= other.m_u4TEST_NUM;		// MPR.TEST_NUM
        m_u1HEAD_NUM	= other.m_u1HEAD_NUM;		// MPR.HEAD_NUM
        m_u1SITE_NUM	= other.m_u1SITE_NUM;		// MPR.SITE_NUM
        m_b1TEST_FLG	= other.m_b1TEST_FLG;		// MPR.TEST_FLG
        m_b1PARM_FLG	= other.m_b1TEST_FLG;		// MPR.PARM_FLG
        m_u2RTN_ICNT	= other.m_u2RTN_ICNT;		// MPR.RTN_ICNT
        m_u2RSLT_CNT	= other.m_u2RSLT_CNT;		// MPR.RSLT_CNT
        m_cnTEST_TXT	= other.m_cnTEST_TXT;		// MPR.TEST_TXT
        m_cnALARM_ID	= other.m_cnALARM_ID;		// MPR.ALARM_ID
        m_b1OPT_FLAG	= other.m_b1OPT_FLAG;		// MPR.OPT_FLAG
        m_i1RES_SCAL	= other.m_i1RES_SCAL;		// MPR.RES_SCAL
        m_i1LLM_SCAL	= other.m_i1LLM_SCAL;		// MPR.LLM_SCAL
        m_i1HLM_SCAL	= other.m_i1HLM_SCAL;		// MPR.HLM_SCAL
        m_r4LO_LIMIT	= other.m_r4LO_LIMIT;		// MPR.LO_LIMIT
        m_r4HI_LIMIT	= other.m_r4HI_LIMIT;		// MPR.HI_LIMIT
        m_r4START_IN	= other.m_r4START_IN;		// MPR.START_IN
        m_r4INCR_IN		= other.m_r4INCR_IN;		// MPR.INCR_IN
        m_cnUNITS		= other.m_cnUNITS;		// MPR.UNITS
        m_cnUNITS_IN	= other.m_cnUNITS_IN;		// MPR.UNITS_IN
        m_cnC_RESFMT	= other.m_cnC_RESFMT;		// MPR.C_RESFMT
        m_cnC_LLMFMT	= other.m_cnC_LLMFMT;		// MPR.C_LLMFMT
        m_cnC_HLMFMT	= other.m_cnC_HLMFMT;		// MPR.C_HLMFMT
        m_r4LO_SPEC		= other.m_r4LO_SPEC;		// MPR.LO_SPEC
        m_r4HI_SPEC		= other.m_r4HI_SPEC;		// MPR.HI_SPEC

        if (m_u2RTN_ICNT > 0)
        {
            if(other.m_kn1RTN_STAT != NULL)	// MPR.RTN_STAT
            {
                m_kn1RTN_STAT = new stdf_type_n1[m_u2RTN_ICNT] ;

                for (int lIdx = 0; lIdx < m_u2RTN_ICNT; ++lIdx)
                    m_kn1RTN_STAT[lIdx] = other.m_kn1RTN_STAT[lIdx];
            }

            if(other.m_ku2RTN_INDX != NULL)	// MPR.RTN_INDX
            {
                m_ku2RTN_INDX = new stdf_type_u2[m_u2RTN_ICNT] ;

                for (int lIdx = 0; lIdx < m_u2RTN_ICNT; ++lIdx)
                    m_ku2RTN_INDX[lIdx] = other.m_ku2RTN_INDX[lIdx];
            }
        }

        if (m_u2RSLT_CNT > 0)
        {
            if(other.m_kr4RTN_RSLT != NULL )	// MPR.RTN_RSLT
            {
                m_kr4RTN_RSLT = new stdf_type_r4[m_u2RSLT_CNT];

                for (int lIdx = 0; lIdx < m_u2RSLT_CNT; ++lIdx)
                    m_kr4RTN_RSLT[lIdx] = other.m_kr4RTN_RSLT[lIdx];
            }

            if(other.m_kbRTN_RSLT_IsNan != NULL)
            {
                m_kbRTN_RSLT_IsNan = new bool[m_u2RSLT_CNT];

                for (int lIdx = 0; lIdx < m_u2RSLT_CNT; ++lIdx)
                    m_kbRTN_RSLT_IsNan[lIdx] = other.m_kbRTN_RSLT_IsNan[lIdx];
            }
        }

        Stdf_Record::operator =(other);
    }

    return *this;
}

bool Stdf_MPR_V4::IsFieldValid(int nFieldPos) const
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;

    return false;
}

bool Stdf_MPR_V4::IsTestExecuted()
{
    return ((m_b1TEST_FLG & STDF_MASK_BIT4) == 0);
}

bool Stdf_MPR_V4::IsTestFail()
{
    return IsTestFail(*this);
}

bool Stdf_MPR_V4::IsTestFail(Stdf_MPR_V4 & clRefMPR)
{
    // Check if test executed
    if(m_b1TEST_FLG & STDF_MASK_BIT4)
        return false;

    // Check if Pass/Fail flag valid
    if((m_b1TEST_FLG & STDF_MASK_BIT6) == 0)
        return (m_b1TEST_FLG & STDF_MASK_BIT7);

    // Pass/Fail flag not valid, check if test results are reliable
    if(m_b1TEST_FLG & STDF_MASK_BIT2)
        return false;

    // Loop through all pins
    for(int lIndex=0; lIndex<m_u2RSLT_CNT; ++lIndex)
    {
        // Test result valid, compare to limits
        if(clRefMPR.IsFieldValid(eposLO_LIMIT))
        {
            // Low limit
            if(m_b1PARM_FLG & STDF_MASK_BIT6)
            {
                if((!m_kbRTN_RSLT_IsNan[lIndex]) && (m_kr4RTN_RSLT[lIndex] < clRefMPR.m_r4LO_LIMIT))
                    return true;
            }
            else
            {
                if((!m_kbRTN_RSLT_IsNan[lIndex]) && (m_kr4RTN_RSLT[lIndex] <= clRefMPR.m_r4LO_LIMIT))
                    return true;
            }
        }
        if(clRefMPR.IsFieldValid(eposHI_LIMIT))
        {
            // High limit
            if(m_b1PARM_FLG & STDF_MASK_BIT7)
            {
                if((!m_kbRTN_RSLT_IsNan[lIndex]) && (m_kr4RTN_RSLT[lIndex] > clRefMPR.m_r4HI_LIMIT))
                    return true;
            }
            else
            {
                if((!m_kbRTN_RSLT_IsNan[lIndex]) && (m_kr4RTN_RSLT[lIndex] >= clRefMPR.m_r4HI_LIMIT))
                    return true;
            }
        }
    }

    return false;
}

void Stdf_MPR_V4::UpdatePassFailInfo(Stdf_MPR_V4 & clRefMPR)
{
    // Check if test executed
    if(m_b1TEST_FLG & STDF_MASK_BIT4)
        return;

    // Check if test results are reliable
    if(m_b1TEST_FLG & STDF_MASK_BIT1)
        return;

    // Test result valid
    m_b1TEST_FLG &= ~STDF_MASK_BIT6;				// Pass/Fail flag is valid
    m_b1TEST_FLG &= ~STDF_MASK_BIT7;				// Test is Pass
    m_b1PARM_FLG &= ~STDF_MASK_BIT3;				// Measured value not high
    m_b1PARM_FLG &= ~STDF_MASK_BIT4;				// Measured value not low
    m_b1PARM_FLG &= ~STDF_MASK_BIT5;				// Test failed or passed standard limits

    // Check result against limits
    // Loop through pin results
    for(int lIndex=0; lIndex<m_u2RSLT_CNT; ++lIndex)
    {
        if(clRefMPR.IsFieldValid(eposLO_LIMIT))
        {
            // Low limit
            if(m_b1PARM_FLG & STDF_MASK_BIT6)
            {
                if((!m_kbRTN_RSLT_IsNan[lIndex]) && (m_kr4RTN_RSLT[lIndex] < clRefMPR.m_r4LO_LIMIT))
                {
                    m_b1TEST_FLG |= STDF_MASK_BIT7;		// Test is Fail
                    m_b1PARM_FLG |= STDF_MASK_BIT4;		// Measured value low
                    return;
                }
            }
            else
            {
                if((!m_kbRTN_RSLT_IsNan[lIndex]) && (m_kr4RTN_RSLT[lIndex] <= clRefMPR.m_r4LO_LIMIT))
                {
                    m_b1TEST_FLG |= STDF_MASK_BIT7;		// Test is Fail
                    m_b1PARM_FLG |= STDF_MASK_BIT4;		// Measured value low
                    return;
                }
            }
        }
        if(clRefMPR.IsFieldValid(eposHI_LIMIT))
        {
            // High limit
            if(m_b1PARM_FLG & STDF_MASK_BIT7)
            {
                if((!m_kbRTN_RSLT_IsNan[lIndex]) && (m_kr4RTN_RSLT[lIndex] > clRefMPR.m_r4HI_LIMIT))
                {
                    m_b1TEST_FLG |= STDF_MASK_BIT7;		// Test is Fail
                    m_b1PARM_FLG |= STDF_MASK_BIT3;		// Measured value high
                    return;
                }
            }
            else
            {
                if((!m_kbRTN_RSLT_IsNan[lIndex]) && (m_kr4RTN_RSLT[lIndex] >= clRefMPR.m_r4HI_LIMIT))
                {
                    m_b1TEST_FLG |= STDF_MASK_BIT7;		// Test is Fail
                    m_b1PARM_FLG |= STDF_MASK_BIT3;		// Measured value high
                    return;
                }
            }
        }
    }
}

QString Stdf_MPR_V4::GetRecordShortName(void)
{
    return "MPR";
}

QString Stdf_MPR_V4::GetRecordLongName(void)
{
    return "Multiple-Result Parametric Record";
}

int Stdf_MPR_V4::GetRecordType(void)
{
    return Rec_MPR;
}

bool Stdf_MPR_V4::Read(GS::StdLib::Stdf & clStdf)
{
    char			szString[STDF_MAX_U1+1];
    int				wData;
    long			dwData;
    float			fData;
    BYTE			bData;
    unsigned int	i;

    // First reset data
    Reset();

    // MPR.TEST_NUM
    _FIELD_CHECKREAD(clStdf.ReadDword(&dwData), eposTEST_NUM);
    _FIELD_SET(m_u4TEST_NUM = stdf_type_u4(dwData), true, eposTEST_NUM);

    // MPR.HEAD_NUM
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposHEAD_NUM);
    _FIELD_SET(m_u1HEAD_NUM = stdf_type_u1(bData), true, eposHEAD_NUM);

    // MPR.SITE_NUM
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposSITE_NUM);
    _FIELD_SET(m_u1SITE_NUM = stdf_type_u1(bData), true, eposSITE_NUM);

    // MPR.TEST_FLG
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposTEST_FLG);
    _FIELD_SET(m_b1TEST_FLG = stdf_type_b1(bData), true, eposTEST_FLG);

    // MPR.PARM_FLG
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposPARM_FLG);
    _FIELD_SET(m_b1PARM_FLG = stdf_type_b1(bData), true, eposPARM_FLG);

    // MPR.RTN_ICNT
    _FIELD_CHECKREAD(clStdf.ReadWord(&wData), eposRTN_ICNT);
    _FIELD_SET(m_u2RTN_ICNT = stdf_type_u2(wData), true, eposRTN_ICNT);

    // MPR.RSLT_CNT
    _FIELD_CHECKREAD(clStdf.ReadWord(&wData), eposRSLT_CNT);
    _FIELD_SET(m_u2RSLT_CNT = stdf_type_u2(wData), true, eposRSLT_CNT);

    // MPR.RTN_STAT
    // Set the field as present, even if count is 0. We use a dummy instruction as parameter 1 to avoid warning under gcc.
    _FIELD_SET(m_uiIndex_macro = 0, true, eposRTN_STAT);
    if(m_u2RTN_ICNT > 0)
        m_kn1RTN_STAT = new stdf_type_n1[m_u2RTN_ICNT];
    for(i=0; i<(unsigned int)((m_u2RTN_ICNT+1)/2); i++)
    {
        _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposRTN_STAT);
        _FIELD_SET(m_kn1RTN_STAT[i*2] = stdf_type_n1(bData & 0x0f), true, eposRTN_STAT);
        if(i*2+1 < m_u2RTN_ICNT)
            _FIELD_SET(m_kn1RTN_STAT[i*2+1] = stdf_type_n1((bData >> 4) & 0x0f), true, eposRTN_STAT);
    }

    // MPR.RTN_RSLT
    // Set the field as present, even if count is 0. We use a dummy instruction as parameter 1 to avoid warning under gcc.
    _FIELD_SET(m_uiIndex_macro = 0, true, eposRTN_RSLT);
    if(m_u2RSLT_CNT > 0)
    {
        m_kr4RTN_RSLT = new stdf_type_r4[m_u2RSLT_CNT];
        m_kbRTN_RSLT_IsNan = new bool[m_u2RSLT_CNT];
    }
    for(i=0; i<(unsigned int)m_u2RSLT_CNT; i++)
        m_kbRTN_RSLT_IsNan[i] = false;
    for(i=0; i<(unsigned int)m_u2RSLT_CNT; i++)
    {
        _FIELD_CHECKREAD(clStdf.ReadFloat(&fData, m_kbRTN_RSLT_IsNan+i), eposRTN_RSLT);
        _FIELD_SET(m_kr4RTN_RSLT[i] = stdf_type_r4(fData), true, eposRTN_RSLT);
    }

    // MPR.TEST_TXT
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposTEST_TXT);
    _FIELD_SET(m_cnTEST_TXT = szString, !m_cnTEST_TXT.isEmpty(), eposTEST_TXT);

    // MPR.ALARM_ID
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposALARM_ID);
    _FIELD_SET(m_cnALARM_ID = szString, !m_cnALARM_ID.isEmpty(), eposALARM_ID);

    // MPR.OPT_FLAG
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposOPT_FLAG);
    _FIELD_SET(m_b1OPT_FLAG = stdf_type_b1(bData), true, eposOPT_FLAG);

    // MPR.RES_SCAL
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposRES_SCAL);
    _FIELD_SET(m_i1RES_SCAL = stdf_type_i1(bData), (m_b1OPT_FLAG & STDF_MASK_BIT0) == 0, eposRES_SCAL);

    // MPR.LLM_SCAL
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposLLM_SCAL);
    _FIELD_SET(m_i1LLM_SCAL = stdf_type_i1(bData), (m_b1OPT_FLAG & (STDF_MASK_BIT4 | STDF_MASK_BIT6)) == 0, eposLLM_SCAL);

    // MPR.HLM_SCAL
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposHLM_SCAL);
    _FIELD_SET(m_i1HLM_SCAL = stdf_type_i1(bData), (m_b1OPT_FLAG & (STDF_MASK_BIT5 | STDF_MASK_BIT7)) == 0, eposHLM_SCAL);

    // MPR.LO_LIMIT
    _FIELD_CHECKREAD(clStdf.ReadFloat(&fData), eposLO_LIMIT);
    _FIELD_SET(m_r4LO_LIMIT = stdf_type_r4(fData), (m_b1OPT_FLAG & (STDF_MASK_BIT4 | STDF_MASK_BIT6)) == 0, eposLO_LIMIT);

    // MPR.HI_LIMIT
    _FIELD_CHECKREAD(clStdf.ReadFloat(&fData), eposHI_LIMIT);
    _FIELD_SET(m_r4HI_LIMIT = stdf_type_r4(fData), (m_b1OPT_FLAG & (STDF_MASK_BIT5 | STDF_MASK_BIT7)) == 0, eposHI_LIMIT);

    // MPR.START_IN
    _FIELD_CHECKREAD(clStdf.ReadFloat(&fData), eposSTART_IN);
    _FIELD_SET(m_r4START_IN = stdf_type_r4(fData), true, eposSTART_IN);

    // MPR.INCR_IN
    _FIELD_CHECKREAD(clStdf.ReadFloat(&fData), eposINCR_IN);
    _FIELD_SET(m_r4INCR_IN = stdf_type_r4(fData), true, eposINCR_IN);

    // MPR.RTN_INDX (cf. line 7714)
    if(m_u2RTN_ICNT > 0)
    {
        _FIELD_CHECKREAD(clStdf.ReadWord(&wData), eposRTN_INDX);
        m_ku2RTN_INDX = new stdf_type_u2[m_u2RTN_ICNT];
        _FIELD_SET(m_ku2RTN_INDX[0] = stdf_type_u2(wData), true, eposRTN_INDX);
    }
    for(i=1; i<(unsigned int)m_u2RTN_ICNT; i++)
    {
        _FIELD_CHECKREAD(clStdf.ReadWord(&wData), eposRTN_INDX);
        _FIELD_SET(m_ku2RTN_INDX[i] = stdf_type_u2(wData), true, eposRTN_INDX);
    }

    // MPR.UNITS
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposUNITS);
    _FIELD_SET(m_cnUNITS = szString, true, eposUNITS);

    // MPR.RTN_INDX: this is to handle the situation where m_u2RTN_ICNT=0, but the STDF record is
    // not finished. So we have to consider that the RTN_INDX field is valid and set, even if
    // it has no results.
    if(m_u2RTN_ICNT == 0)
        // Set the field as present, even if count is 0. We use a dummy instruction as parameter 1 to avoid warning under gcc.
        _FIELD_SET(m_uiIndex_macro = 0, true, eposRTN_INDX);

    // MPR.UNITS_IN
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposUNITS_IN);
    _FIELD_SET(m_cnUNITS_IN = szString, true, eposUNITS_IN);

    // MPR.C_RESFMT
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposC_RESFMT);
    _FIELD_SET(m_cnC_RESFMT = szString, true, eposC_RESFMT);

    // MPR.C_LLMFMT
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposC_LLMFMT);
    _FIELD_SET(m_cnC_LLMFMT = szString, true, eposC_LLMFMT);

    // MPR.C_HLMFMT
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposC_HLMFMT);
    _FIELD_SET(m_cnC_HLMFMT = szString, true, eposC_HLMFMT);

    // MPR.LO_SPEC
    _FIELD_CHECKREAD(clStdf.ReadFloat(&fData), eposLO_SPEC);
    _FIELD_SET(m_r4LO_SPEC = stdf_type_r4(fData), (m_b1OPT_FLAG & STDF_MASK_BIT2) == 0, eposLO_SPEC);

    // MPR.HI_SPEC
    _FIELD_CHECKREAD(clStdf.ReadFloat(&fData), eposHI_SPEC);
    _FIELD_SET(m_r4HI_SPEC = stdf_type_r4(fData), (m_b1OPT_FLAG & STDF_MASK_BIT3) == 0, eposHI_SPEC);

    return true;
}

bool Stdf_MPR_V4::Write(GS::StdLib::Stdf & clStdf)
{
    GS::StdLib::StdfRecordReadInfo	RecordReadInfo;
    unsigned int		i;
    stdf_type_u1		u1Temp;

    RecordReadInfo.iRecordType = STDF_MPR_TYPE;
    RecordReadInfo.iRecordSubType = STDF_MPR_STYPE;
    clStdf.WriteHeader(&RecordReadInfo);

    // MPR.TEST_NUM
    _FIELD_CHECKWRITE(clStdf.WriteDword(DWORD(m_u4TEST_NUM)), eposTEST_NUM, clStdf.WriteRecord());

    // MPR.HEAD_NUM
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_u1HEAD_NUM)), eposHEAD_NUM, clStdf.WriteRecord());

    // MPR.SITE_NUM
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_u1SITE_NUM)), eposSITE_NUM, clStdf.WriteRecord());

    // MPR.TEST_FLG
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_b1TEST_FLG)), eposTEST_FLG, clStdf.WriteRecord());

    // MPR.PARM_FLG
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_b1PARM_FLG)), eposPARM_FLG, clStdf.WriteRecord());

    // MPR.RTN_ICNT
    _FIELD_CHECKWRITE(clStdf.WriteWord(WORD(m_u2RTN_ICNT)), eposRTN_ICNT, clStdf.WriteRecord());

    // MPR.RSLT_CNT
    _FIELD_CHECKWRITE(clStdf.WriteWord(WORD(m_u2RSLT_CNT)), eposRSLT_CNT, clStdf.WriteRecord());

    // MPR.RTN_STAT
    for(i=0; i<(unsigned int)((m_u2RTN_ICNT+1)/2); i++)
    {
        u1Temp = 0;
        if(i*2+1 < m_u2RTN_ICNT)
            u1Temp = (m_kn1RTN_STAT[i*2+1] << 4) & 0xf0;
        u1Temp |= (m_kn1RTN_STAT[i*2] & 0x0f);
        _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(u1Temp)), eposRTN_STAT, clStdf.WriteRecord());
    }

    // MPR.RTN_RSLT
    for(i=0; i<(unsigned int)m_u2RSLT_CNT; i++)
        _FIELD_CHECKWRITE(clStdf.WriteFloat(FLOAT(m_kr4RTN_RSLT[i])), eposRTN_RSLT, clStdf.WriteRecord());

    // MPR.TEST_TXT
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnTEST_TXT.toLatin1().constData()), eposTEST_TXT, clStdf.WriteRecord());

    // MPR.ALARM_ID
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnALARM_ID.toLatin1().constData()), eposALARM_ID, clStdf.WriteRecord());

    // MPR.OPT_FLAG
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_b1OPT_FLAG)), eposOPT_FLAG, clStdf.WriteRecord());

    // MPR.RES_SCAL
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_i1RES_SCAL)), eposRES_SCAL, clStdf.WriteRecord());

    // MPR.LLM_SCAL
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_i1LLM_SCAL)), eposLLM_SCAL, clStdf.WriteRecord());

    // MPR.HLM_SCAL
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_i1HLM_SCAL)), eposHLM_SCAL, clStdf.WriteRecord());

    // MPR.LO_LIMIT
    _FIELD_CHECKWRITE(clStdf.WriteFloat(FLOAT(m_r4LO_LIMIT)), eposLO_LIMIT, clStdf.WriteRecord());

    // MPR.HI_LIMIT
    _FIELD_CHECKWRITE(clStdf.WriteFloat(FLOAT(m_r4HI_LIMIT)), eposHI_LIMIT, clStdf.WriteRecord());

    // MPR.START_IN
    _FIELD_CHECKWRITE(clStdf.WriteFloat(FLOAT(m_r4START_IN)), eposSTART_IN, clStdf.WriteRecord());

    // MPR.INCR_IN
    _FIELD_CHECKWRITE(clStdf.WriteFloat(FLOAT(m_r4INCR_IN)), eposINCR_IN, clStdf.WriteRecord());

    // MPR.RTN_INDX
    for(i=0; i<(unsigned int)m_u2RTN_ICNT; i++)
        _FIELD_CHECKWRITE(clStdf.WriteWord(WORD(m_ku2RTN_INDX[i])), eposRTN_INDX, clStdf.WriteRecord());

    // MPR.UNITS
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnUNITS.toLatin1().constData()), eposUNITS, clStdf.WriteRecord());

    // MPR.UNITS_IN
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnUNITS_IN.toLatin1().constData()), eposUNITS_IN, clStdf.WriteRecord());

    // MPR.C_RESFMT
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnC_RESFMT.toLatin1().constData()), eposC_RESFMT, clStdf.WriteRecord());

    // MPR.C_LLMFMT
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnC_LLMFMT.toLatin1().constData()), eposC_LLMFMT, clStdf.WriteRecord());

    // MPR.C_HLMFMT
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnC_HLMFMT.toLatin1().constData()), eposC_HLMFMT, clStdf.WriteRecord());

    // MPR.LO_SPEC
    _FIELD_CHECKWRITE(clStdf.WriteFloat(FLOAT(m_r4LO_SPEC)), eposLO_SPEC, clStdf.WriteRecord());

    // MPR.HI_SPEC
    _FIELD_CHECKWRITE(clStdf.WriteFloat(FLOAT(m_r4HI_SPEC)), eposHI_SPEC, clStdf.WriteRecord());

    clStdf.WriteRecord();

    return true;
}

void Stdf_MPR_V4::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    // Empty string first
    strAsciiString = "";

    // MPR.TEST_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "MPR.test_num", QString::number(m_u4TEST_NUM), nFieldSelection, eposTEST_NUM);

    // MPR.HEAD_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "MPR.head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // MPR.SITE_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "MPR.site_num", QString::number(m_u1SITE_NUM), nFieldSelection, eposSITE_NUM);

    // MPR.TEST_FLG
    _CREATEFIELD_FROM_B1_ASCII(m_b1TEST_FLG)
    _STR_ADDFIELD_ASCII(strAsciiString, "MPR.test_flg", m_strFieldValue_macro, nFieldSelection, eposTEST_FLG);

    // MPR.PARM_FLG
    _CREATEFIELD_FROM_B1_ASCII(m_b1PARM_FLG)
    _STR_ADDFIELD_ASCII(strAsciiString, "MPR.parm_flg", m_strFieldValue_macro, nFieldSelection, eposPARM_FLG);

    // MPR.RTN_ICNT
    _STR_ADDFIELD_ASCII(strAsciiString, "MPR.rtn_icnt", QString::number(m_u2RTN_ICNT), nFieldSelection, eposRTN_ICNT);

    // MPR.RSLT_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "MPR.rslt_cnt", QString::number(m_u2RSLT_CNT), nFieldSelection, eposRSLT_CNT);

    // MPR.RTN_STAT
    _CREATEFIELD_FROM_KN1_ASCII(m_u2RTN_ICNT, m_kn1RTN_STAT, eposRTN_STAT);
    _STR_ADDFIELD_ASCII(strAsciiString, "MPR.rtn_stat", m_strFieldValue_macro, nFieldSelection, eposRTN_STAT);

    // MPR.RTN_RSLT
    //_CREATEFIELD_FROM_KRi_ASCII_NANCHECK(m_u2RSLT_CNT, m_kr4RTN_RSLT, m_kbRTN_RSLT_IsNan, eposRTN_RSLT);
        m_strFieldValue_macro = "";
        if(m_pFieldFlags[eposRTN_RSLT] & FieldFlag_Present)
        {
            if(m_u2RSLT_CNT > 0)\
            {
                if(m_kbRTN_RSLT_IsNan[0])
                    sprintf(m_szTmp_macro, "\n        [000] = NaN");
                else
                    sprintf(m_szTmp_macro, "\n        [000] = %s",
                        QString::number( (float)((float) m_kr4RTN_RSLT[0]),'f',mPrecision).toLatin1().data());
                m_strFieldValue_macro = m_szTmp_macro;
            }
            for(m_uiIndex_macro=1; m_uiIndex_macro<(unsigned int)m_u2RSLT_CNT; m_uiIndex_macro++)
            {
                if(m_kbRTN_RSLT_IsNan[m_uiIndex_macro])
                    sprintf(m_szTmp_macro, "\n        [%03u] = NaN", m_uiIndex_macro);
                else
                    sprintf(m_szTmp_macro, "\n        [%03u] = %s", m_uiIndex_macro,
                        QString::number((float) m_kr4RTN_RSLT[m_uiIndex_macro],'f',mPrecision).toLatin1().data() );
                    m_strFieldValue_macro += m_szTmp_macro;
            }
        }

    _STR_ADDFIELD_ASCII(strAsciiString, "MPR.rtn_rslt", m_strFieldValue_macro, nFieldSelection, eposRTN_RSLT);

    // MPR.TEST_TXT
    _STR_ADDFIELD_ASCII(strAsciiString, "MPR.test_txt", m_cnTEST_TXT, nFieldSelection, eposTEST_TXT);

    // MPR.ALARM_ID
    _STR_ADDFIELD_ASCII(strAsciiString, "MPR.alarm_id", m_cnALARM_ID, nFieldSelection, eposALARM_ID);

    // MPR.OPT_FLAG
    _CREATEFIELD_FROM_B1_ASCII(m_b1OPT_FLAG)
    _STR_ADDFIELD_ASCII(strAsciiString, "MPR.opt_flag", m_strFieldValue_macro, nFieldSelection, eposOPT_FLAG);

    // MPR.RES_SCAL
    _STR_ADDFIELD_ASCII(strAsciiString, "MPR.res_scal", QString::number(m_i1RES_SCAL), nFieldSelection, eposRES_SCAL);

    // MPR.LLM_SCAL
    _STR_ADDFIELD_ASCII(strAsciiString, "MPR.llm_scal", QString::number(m_i1LLM_SCAL), nFieldSelection, eposLLM_SCAL);

    // MPR.HLM_SCAL
    _STR_ADDFIELD_ASCII(strAsciiString, "MPR.hlm_scal", QString::number(m_i1HLM_SCAL), nFieldSelection, eposHLM_SCAL);

    // MPR.LO_LIMIT
    _STR_ADDFIELD_ASCII(strAsciiString, "MPR.lo_limit", QString::number(m_r4LO_LIMIT), nFieldSelection, eposLO_LIMIT);

    // MPR.HI_LIMIT
    _STR_ADDFIELD_ASCII(strAsciiString, "MPR.hi_limit", QString::number(m_r4HI_LIMIT), nFieldSelection, eposHI_LIMIT);

    // MPR.START_IN
    _STR_ADDFIELD_ASCII(strAsciiString, "MPR.start_in", QString::number(m_r4START_IN), nFieldSelection, eposSTART_IN);

    // MPR.INCR_IN
    _STR_ADDFIELD_ASCII(strAsciiString, "MPR.incr_in", QString::number(m_r4INCR_IN), nFieldSelection, eposINCR_IN);

    // MPR.RTN_INDX
    _CREATEFIELD_FROM_KUi_ASCII(m_u2RTN_ICNT, m_ku2RTN_INDX, eposRTN_INDX);
    _STR_ADDFIELD_ASCII(strAsciiString, "MPR.rtn_indx", m_strFieldValue_macro, nFieldSelection, eposRTN_INDX);

    // MPR.UNITS
    _STR_ADDFIELD_ASCII(strAsciiString, "MPR.units", m_cnUNITS, nFieldSelection, eposUNITS);

    // MPR.UNITS_IN
    _STR_ADDFIELD_ASCII(strAsciiString, "MPR.units_in", m_cnUNITS_IN, nFieldSelection, eposUNITS_IN);

    // MPR.C_RESFMT
    _STR_ADDFIELD_ASCII(strAsciiString, "MPR.c_resfmt", m_cnC_RESFMT, nFieldSelection, eposC_RESFMT);

    // MPR.C_LLMFMT
    _STR_ADDFIELD_ASCII(strAsciiString, "MPR.c_llmfmt", m_cnC_LLMFMT, nFieldSelection, eposC_LLMFMT);

    // MPR.C_HLMFMT
    _STR_ADDFIELD_ASCII(strAsciiString, "MPR.c_hlmfmt", m_cnC_HLMFMT, nFieldSelection, eposC_HLMFMT);

    // MPR.LO_SPEC
    _STR_ADDFIELD_ASCII(strAsciiString, "MPR.lo_spec", QString::number(m_r4LO_SPEC), nFieldSelection, eposLO_SPEC);

    // MPR.HI_SPEC
    _STR_ADDFIELD_ASCII(strAsciiString, "MPR.hi_spec", QString::number(m_r4HI_SPEC), nFieldSelection, eposHI_SPEC);
}

void Stdf_MPR_V4::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
    // Empty string list first
    listFields.empty();

    // MPR.TEST_NUM
    _LIST_ADDFIELD_ASCII(listFields, "MPR.test_num", QString::number(m_u4TEST_NUM), nFieldSelection, eposTEST_NUM);

    // MPR.HEAD_NUM
    _LIST_ADDFIELD_ASCII(listFields, "MPR.head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // MPR.SITE_NUM
    _LIST_ADDFIELD_ASCII(listFields, "MPR.site_num", QString::number(m_u1SITE_NUM), nFieldSelection, eposSITE_NUM);

    // MPR.TEST_FLG
    _CREATEFIELD_FROM_B1_ASCII(m_b1TEST_FLG)
    _LIST_ADDFIELD_ASCII(listFields, "MPR.test_flg", m_strFieldValue_macro, nFieldSelection, eposTEST_FLG);

    // MPR.PARM_FLG
    _CREATEFIELD_FROM_B1_ASCII(m_b1PARM_FLG)
    _LIST_ADDFIELD_ASCII(listFields, "MPR.parm_flg", m_strFieldValue_macro, nFieldSelection, eposPARM_FLG);

    // MPR.RTN_ICNT
    _LIST_ADDFIELD_ASCII(listFields, "MPR.rtn_icnt", QString::number(m_u2RTN_ICNT), nFieldSelection, eposRTN_ICNT);

    // MPR.RSLT_CNT
    _LIST_ADDFIELD_ASCII(listFields, "MPR.rslt_cnt", QString::number(m_u2RSLT_CNT), nFieldSelection, eposRSLT_CNT);

    // MPR.RTN_STAT
    _CREATEFIELD_FROM_KN1_ASCII(m_u2RTN_ICNT, m_kn1RTN_STAT, eposRTN_STAT);
    _LIST_ADDFIELD_ASCII(listFields, "MPR.rtn_stat", m_strFieldValue_macro, nFieldSelection, eposRTN_STAT);

    // MPR.RTN_RSLT
    _CREATEFIELD_FROM_KRi_ASCII_NANCHECK(m_u2RSLT_CNT, m_kr4RTN_RSLT, m_kbRTN_RSLT_IsNan, eposRTN_RSLT);
    _LIST_ADDFIELD_ASCII(listFields, "MPR.rtn_rslt", m_strFieldValue_macro, nFieldSelection, eposRTN_RSLT);

    // MPR.TEST_TXT
    _LIST_ADDFIELD_ASCII(listFields, "MPR.test_txt", m_cnTEST_TXT, nFieldSelection, eposTEST_TXT);

    // MPR.ALARM_ID
    _LIST_ADDFIELD_ASCII(listFields, "MPR.alarm_id", m_cnALARM_ID, nFieldSelection, eposALARM_ID);

    // MPR.OPT_FLAG
    _CREATEFIELD_FROM_B1_ASCII(m_b1OPT_FLAG)
    _LIST_ADDFIELD_ASCII(listFields, "MPR.opt_flag", m_strFieldValue_macro, nFieldSelection, eposOPT_FLAG);

    // MPR.RES_SCAL
    _LIST_ADDFIELD_ASCII(listFields, "MPR.res_scal", QString::number(m_i1RES_SCAL), nFieldSelection, eposRES_SCAL);

    // MPR.LLM_SCAL
    _LIST_ADDFIELD_ASCII(listFields, "MPR.llm_scal", QString::number(m_i1LLM_SCAL), nFieldSelection, eposLLM_SCAL);

    // MPR.HLM_SCAL
    _LIST_ADDFIELD_ASCII(listFields, "MPR.hlm_scal", QString::number(m_i1HLM_SCAL), nFieldSelection, eposHLM_SCAL);

    // MPR.LO_LIMIT
    _LIST_ADDFIELD_ASCII(listFields, "MPR.lo_limit", QString::number(m_r4LO_LIMIT), nFieldSelection, eposLO_LIMIT);

    // MPR.HI_LIMIT
    _LIST_ADDFIELD_ASCII(listFields, "MPR.hi_limit", QString::number(m_r4HI_LIMIT), nFieldSelection, eposHI_LIMIT);

    // MPR.START_IN
    _LIST_ADDFIELD_ASCII(listFields, "MPR.start_in", QString::number(m_r4START_IN), nFieldSelection, eposSTART_IN);

    // MPR.INCR_IN
    _LIST_ADDFIELD_ASCII(listFields, "MPR.incr_in", QString::number(m_r4INCR_IN), nFieldSelection, eposINCR_IN);

    // MPR.RTN_INDX
    _CREATEFIELD_FROM_KUi_ASCII(m_u2RTN_ICNT, m_ku2RTN_INDX, eposRTN_INDX);
    _LIST_ADDFIELD_ASCII(listFields, "MPR.rtn_indx", m_strFieldValue_macro, nFieldSelection, eposRTN_INDX);

    // MPR.UNITS
    _LIST_ADDFIELD_ASCII(listFields, "MPR.units", m_cnUNITS, nFieldSelection, eposUNITS);

    // MPR.UNITS_IN
    _LIST_ADDFIELD_ASCII(listFields, "MPR.units_in", m_cnUNITS_IN, nFieldSelection, eposUNITS_IN);

    // MPR.C_RESFMT
    _LIST_ADDFIELD_ASCII(listFields, "MPR.c_resfmt", m_cnC_RESFMT, nFieldSelection, eposC_RESFMT);

    // MPR.C_LLMFMT
    _LIST_ADDFIELD_ASCII(listFields, "MPR.c_llmfmt", m_cnC_LLMFMT, nFieldSelection, eposC_LLMFMT);

    // MPR.C_HLMFMT
    _LIST_ADDFIELD_ASCII(listFields, "MPR.c_hlmfmt", m_cnC_HLMFMT, nFieldSelection, eposC_HLMFMT);

    // MPR.LO_SPEC
    _LIST_ADDFIELD_ASCII(listFields, "MPR.lo_spec", QString::number(m_r4LO_SPEC), nFieldSelection, eposLO_SPEC);

    // MPR.HI_SPEC
    _LIST_ADDFIELD_ASCII(listFields, "MPR.hi_spec", QString::number(m_r4HI_SPEC), nFieldSelection, eposHI_SPEC);
}

void Stdf_MPR_V4::GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection /*= 0*/)
{
    QString strTabs="";

    // Init XML string
    for(int nIndex=nIndentationLevel; nIndex>0; nIndex--)
        strTabs += "\t";
    strXmlString = strTabs;
    strXmlString += "<mpr>\n";

    // MPR.TEST_NUM
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "test_num", QString::number(m_u4TEST_NUM), nFieldSelection, eposTEST_NUM);

    // MPR.HEAD_NUM
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // MPR.SITE_NUM
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "site_num", QString::number(m_u1SITE_NUM), nFieldSelection, eposSITE_NUM);

    // MPR.TEST_FLG
    _CREATEFIELD_FROM_B1_XML(m_b1TEST_FLG)
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "test_flg", m_strFieldValue_macro, nFieldSelection, eposTEST_FLG);

    // MPR.PARM_FLG
    _CREATEFIELD_FROM_B1_XML(m_b1PARM_FLG)
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "parm_flg", m_strFieldValue_macro, nFieldSelection, eposPARM_FLG);

    // MPR.RTN_STAT
    _CREATEFIELD_FROM_KN1_XML(m_u2RTN_ICNT, m_kn1RTN_STAT, eposRTN_STAT);
    _STR_ADDLIST_XML(strXmlString, m_u2RTN_ICNT, nIndentationLevel+1, "rtn_stat", m_strFieldValue_macro, nFieldSelection, eposRTN_STAT);

    // MPR.RTN_RSLT
    _CREATEFIELD_FROM_KRi_XML_NANCHECK(m_u2RSLT_CNT, m_kr4RTN_RSLT, m_kbRTN_RSLT_IsNan, eposRTN_RSLT);
    _STR_ADDLIST_XML(strXmlString, m_u2RSLT_CNT, nIndentationLevel+1, "rtn_rslt", m_strFieldValue_macro, nFieldSelection, eposRTN_RSLT);

    // MPR.TEST_TXT
    _CREATEFIELD_FROM_CN_XML(m_cnTEST_TXT);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "test_txt", m_strFieldValue_macro, nFieldSelection, eposTEST_TXT);

    // MPR.ALARM_ID
    _CREATEFIELD_FROM_CN_XML(m_cnALARM_ID);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "alarm_id", m_strFieldValue_macro, nFieldSelection, eposALARM_ID);

    // MPR.OPT_FLAG
    _CREATEFIELD_FROM_B1_XML(m_b1OPT_FLAG)
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "opt_flag", m_strFieldValue_macro, nFieldSelection, eposOPT_FLAG);

    // MPR.RES_SCAL
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "res_scal", QString::number(m_i1RES_SCAL), nFieldSelection, eposRES_SCAL);

    // MPR.LLM_SCAL
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "llm_scal", QString::number(m_i1LLM_SCAL), nFieldSelection, eposLLM_SCAL);

    // MPR.HLM_SCAL
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "hlm_scal", QString::number(m_i1HLM_SCAL), nFieldSelection, eposHLM_SCAL);

    // MPR.LO_LIMIT
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "lo_limit", QString::number(m_r4LO_LIMIT), nFieldSelection, eposLO_LIMIT);

    // MPR.HI_LIMIT
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "hi_limit", QString::number(m_r4HI_LIMIT), nFieldSelection, eposHI_LIMIT);

    // MPR.START_IN
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "start_in", QString::number(m_r4START_IN), nFieldSelection, eposSTART_IN);

    // MPR.INCR_IN
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "incr_in", QString::number(m_r4INCR_IN), nFieldSelection, eposINCR_IN);

    // MPR.RTN_INDX
    _CREATEFIELD_FROM_KUi_ASCII(m_u2RTN_ICNT, m_ku2RTN_INDX, eposRTN_INDX);
    _STR_ADDLIST_XML(strXmlString, m_u2RTN_ICNT, nIndentationLevel+1, "rtn_indx", m_strFieldValue_macro, nFieldSelection, eposRTN_INDX);

    // MPR.UNITS
    _CREATEFIELD_FROM_CN_XML(m_cnUNITS);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "units", m_strFieldValue_macro, nFieldSelection, eposUNITS);

    // MPR.UNITS_IN
    _CREATEFIELD_FROM_CN_XML(m_cnUNITS_IN);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "units_in", m_strFieldValue_macro, nFieldSelection, eposUNITS_IN);

    // MPR.C_RESFMT
    _CREATEFIELD_FROM_CN_XML(m_cnC_RESFMT);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "c_resfmt", m_strFieldValue_macro, nFieldSelection, eposC_RESFMT);

    // MPR.C_LLMFMT
    _CREATEFIELD_FROM_CN_XML(m_cnC_LLMFMT);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "c_llmfmt", m_strFieldValue_macro, nFieldSelection, eposC_LLMFMT);

    // MPR.C_HLMFMT
    _CREATEFIELD_FROM_CN_XML(m_cnC_HLMFMT);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "c_hlmfmt", m_strFieldValue_macro, nFieldSelection, eposC_HLMFMT);

    // MPR.LO_SPEC
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "lo_spec", QString::number(m_r4LO_SPEC), nFieldSelection, eposLO_SPEC);

    // MPR.HI_SPEC
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "hi_spec", QString::number(m_r4HI_SPEC), nFieldSelection, eposHI_SPEC);

    strXmlString += strTabs;
    strXmlString += "</mpr>\n";
}

void Stdf_MPR_V4::GetAtdfString(QString & strAtdfString)
{
    // Reset field index, and line Nb
    m_uiFieldIndex = 0;
    m_uiLineNb = 1;

    // Init string
    strAtdfString = "MPR:";

    // MPR.TEST_NUM
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u4TEST_NUM), eposTEST_NUM);

    // MPR.HEAD_NUM
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u1HEAD_NUM), eposHEAD_NUM);

    // MPR.SITE_NUM
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u1SITE_NUM), eposSITE_NUM);

    // MPR.RTN_STAT
    _CREATEFIELD_FROM_KN1_ATDF(m_u2RTN_ICNT, m_kn1RTN_STAT, eposRTN_STAT);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposRTN_STAT);

    // MPR.RTN_RSLT
    _CREATEFIELD_FROM_KRi_ATDF(m_u2RSLT_CNT,m_kr4RTN_RSLT,eposRTN_RSLT);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposRTN_RSLT);

    // MPR.Pass/Fail Flag
    m_strFieldValue_macro = " ";
    if((m_b1TEST_FLG & 0x40) == 0)
    {
        if((m_b1PARM_FLG & 0x20))
            m_strFieldValue_macro = "A";
        else
        {
            if(m_b1TEST_FLG & 0x80)
                m_strFieldValue_macro = "F";
            else
                m_strFieldValue_macro = "P";
        }
    }
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposTEST_FLG);

    // MPR.Alarm Flags
    m_strFieldValue_macro = "";
    if(m_b1TEST_FLG & 0x01)
        m_strFieldValue_macro += "A";
    if(m_b1TEST_FLG & 0x04)
        m_strFieldValue_macro += "U";
    if(m_b1TEST_FLG & 0x08)
        m_strFieldValue_macro += "T";
    if(m_b1TEST_FLG & 0x10)
        m_strFieldValue_macro += "N";
    if(m_b1TEST_FLG & 0x20)
        m_strFieldValue_macro += "X";
    if(m_b1PARM_FLG & 0x01)
        m_strFieldValue_macro += "S";
    if(m_b1PARM_FLG & 0x02)
        m_strFieldValue_macro += "D";
    if(m_b1PARM_FLG & 0x04)
        m_strFieldValue_macro += "O";
    if(m_b1PARM_FLG & 0x08)
        m_strFieldValue_macro += "H";
    if(m_b1PARM_FLG & 0x10)
        m_strFieldValue_macro += "L";
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposTEST_FLG);

    // MPR.TEST_TXT
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnTEST_TXT, eposTEST_TXT);

    // MPR.ALARM_ID
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnALARM_ID, eposALARM_ID);

    // MPR.limit compare
    m_strFieldValue_macro = "";
    if(m_b1PARM_FLG & 0x40)
        m_strFieldValue_macro += "L";
    if(m_b1PARM_FLG & 0x80)
        m_strFieldValue_macro += "H";
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposPARM_FLG);

    // MPR.UNITS
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnUNITS, eposUNITS);

    // MPR.LO_LIMIT
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_r4LO_LIMIT), eposLO_LIMIT);

    // MPR.HI_LIMIT
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_r4HI_LIMIT), eposHI_LIMIT);

    // MPR.START_IN
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_r4START_IN), eposSTART_IN);

    // MPR.INCR_IN
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_r4INCR_IN), eposINCR_IN);

    // MPR.UNITS_IN
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnUNITS_IN, eposUNITS_IN);

    // MPR.RTN_INDX
    _CREATEFIELD_FROM_KUi_ATDF(m_u2RTN_ICNT, m_ku2RTN_INDX, eposRTN_INDX);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposRTN_INDX);

    // MPR.C_RESFMT
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnC_RESFMT, eposC_RESFMT);

    // MPR.C_LLMFMT
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnC_LLMFMT, eposC_LLMFMT);

    // MPR.C_HLMFMT
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnC_HLMFMT, eposC_HLMFMT);

    // MPR.LO_SPEC
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_r4LO_SPEC), eposLO_SPEC);

    // MPR.HI_SPEC
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_r4HI_SPEC), eposHI_SPEC);

    // MPR.RES_SCAL
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_i1RES_SCAL), eposRES_SCAL);

    // MPR.LLM_SCAL
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_i1LLM_SCAL), eposLLM_SCAL);

    // MPR.HLM_SCAL
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_i1HLM_SCAL), eposHLM_SCAL);

    _STR_FINISHFIELD_ATDF(strAtdfString);
}

///////////////////////////////////////////////////////////
// FTR RECORD
///////////////////////////////////////////////////////////
Stdf_FTR_V4::Stdf_FTR_V4() : Stdf_Record(), m_pFilter(NULL)
{
    m_ku2RTN_INDX = NULL;
    m_kn1RTN_STAT = NULL;
    m_ku2PGM_INDX = NULL;
    m_kn1PGM_STAT = NULL;
    Reset();
}

Stdf_FTR_V4::~Stdf_FTR_V4()
{
    Reset();
}

void Stdf_FTR_V4::SetFilter(Stdf_FTR_V4 * pFilter)
{
    m_pFilter = pFilter;
}

void Stdf_FTR_V4::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Select fields for reduced list

    // Reset Data
    m_u4TEST_NUM	= 0;		// FTR.TEST_NUM
    m_u1HEAD_NUM	= 1;		// FTR.HEAD_NUM
    m_u1SITE_NUM	= 1;		// FTR.SITE_NUM
    m_b1TEST_FLG	= 0;		// FTR.TEST_FLG
    m_b1OPT_FLAG	= 0;		// FTR.OPT_FLAG
    m_u4CYCL_CNT	= 0;		// FTR.CYCL_CNT
    m_u4REL_VADR	= 0;		// FTR.REL_VADR
    m_u4REPT_CNT	= 0;		// FTR.REPT_CNT
    m_u4NUM_FAIL	= 0;		// FTR.NUM_FAIL
    m_i4XFAIL_AD	= 0;		// FTR.XFAIL_AD
    m_i4YFAIL_AD	= 0;		// FTR.YFAIL_AD
    m_i2VECT_OFF	= 0;		// FTR.VECT_OFF
    m_u2RTN_ICNT	= 0;		// FTR.RTN_ICNT
    m_u2PGM_ICNT	= 0;		// FTR.PGM_ICNT
    if(m_ku2RTN_INDX != NULL)	// FTR.RTN_INDX
    {
        delete [] m_ku2RTN_INDX;
        m_ku2RTN_INDX = NULL;
    }
    if(m_kn1RTN_STAT != NULL)	// FTR.RTN_STAT
    {
        delete [] m_kn1RTN_STAT;
        m_kn1RTN_STAT = NULL;
    }
    if(m_ku2PGM_INDX != NULL)	// FTR.PGM_INDX
    {
        delete [] m_ku2PGM_INDX;
        m_ku2PGM_INDX = NULL;
    }
    if(m_kn1PGM_STAT != NULL)	// FTR.PGM_STAT
    {
        delete [] m_kn1PGM_STAT;
        m_kn1PGM_STAT = NULL;
    }
    // FTR.FAIL_PIN
    m_dnFAIL_PIN.m_uiLength = 0;
    for(int i=0; i<(STDF_MAX_U2+1)/8; i++)
        m_dnFAIL_PIN.m_pBitField[i] = 0;
    m_cnVECT_NAM	= "";		// FTR.VECT_NAM
    m_cnTIME_SET	= "";		// FTR.TIME_SET
    m_cnOP_CODE		= "";		// FTR.OP_CODE
    m_cnTEST_TXT	= "";		// FTR.TEST_TXT
    m_cnALARM_ID	= "";		// FTR.ALARM_ID
    m_cnPROG_TXT	= "";		// FTR.PROG_TXT
    m_cnRSLT_TXT	= "";		// FTR.RSLT_TXT
    m_u1PATG_NUM	= 0;		// FTR.PATG_NUM
    // FTR.SPIN_MAP
    m_dnSPIN_MAP.m_uiLength = 0;
    for(int i=0; i<(STDF_MAX_U2+1)/8; i++)
        m_dnSPIN_MAP.m_pBitField[i] = 0;

    // Call Reset base method
    Stdf_Record::Reset();
}

bool Stdf_FTR_V4::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;

    return false;
}

bool Stdf_FTR_V4::IsTestExecuted()
{
    return ((m_b1TEST_FLG & STDF_MASK_BIT4) == 0);
}

bool Stdf_FTR_V4::IsTestFail()
{
    // Check if test executed
    if(m_b1TEST_FLG & STDF_MASK_BIT4)
        return false;

    // Check if Pass/Fail flag valid
    if((m_b1TEST_FLG & STDF_MASK_BIT6) == 0)
        return (m_b1TEST_FLG & STDF_MASK_BIT7);

    // Pass/Fail flag not valid
    return false;
}

QString Stdf_FTR_V4::GetRecordShortName(void)
{
    return "FTR";
}

QString Stdf_FTR_V4::GetRecordLongName(void)
{
    return "Functional Test Record";
}

int Stdf_FTR_V4::GetRecordType(void)
{
    return Rec_FTR;
}

bool Stdf_FTR_V4::Read(GS::StdLib::Stdf & clStdf)
{
    char			szString[STDF_MAX_U1+1];
    int				wData;
    long			dwData;
    BYTE			bData;
    unsigned int	i;

    // First reset data
    Reset();

    // FTR.TEST_NUM
    _FIELD_CHECKREAD(clStdf.ReadDword(&dwData), eposTEST_NUM);
    _FIELD_SET(m_u4TEST_NUM = stdf_type_u4(dwData), true, eposTEST_NUM);

    // FTR.HEAD_NUM
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposHEAD_NUM);
    _FIELD_SET(m_u1HEAD_NUM = stdf_type_u1(bData), true, eposHEAD_NUM);

    // FTR.SITE_NUM
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposSITE_NUM);
    _FIELD_CHECKFILTER(stdf_type_u1(bData), m_u1SITE_NUM, eposSITE_NUM);
    _FIELD_SET(m_u1SITE_NUM = stdf_type_u1(bData), true, eposSITE_NUM);

    // FTR.TEST_FLG
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposTEST_FLG);
    _FIELD_SET(m_b1TEST_FLG = stdf_type_b1(bData), true, eposTEST_FLG);

    // FTR.OPT_FLAG
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposOPT_FLAG);
    _FIELD_SET(m_b1OPT_FLAG = stdf_type_b1(bData), true, eposOPT_FLAG);

    // FTR.CYCL_CNT
    _FIELD_CHECKREAD(clStdf.ReadDword(&dwData), eposCYCL_CNT);
    _FIELD_SET(m_u4CYCL_CNT = stdf_type_u4(dwData), (m_b1OPT_FLAG & STDF_MASK_BIT0) == 0, eposCYCL_CNT);

    // FTR.REL_VADR
    _FIELD_CHECKREAD(clStdf.ReadDword(&dwData), eposREL_VADR);
    _FIELD_SET(m_u4REL_VADR = stdf_type_u4(dwData), (m_b1OPT_FLAG & STDF_MASK_BIT1) == 0, eposREL_VADR);

    // FTR.REPT_CNT
    _FIELD_CHECKREAD(clStdf.ReadDword(&dwData), eposREPT_CNT);
    _FIELD_SET(m_u4REPT_CNT = stdf_type_u4(dwData), (m_b1OPT_FLAG & STDF_MASK_BIT2) == 0, eposREPT_CNT);

    // FTR.NUM_FAIL
    _FIELD_CHECKREAD(clStdf.ReadDword(&dwData), eposNUM_FAIL);
    _FIELD_SET(m_u4NUM_FAIL = stdf_type_u4(dwData), (m_b1OPT_FLAG & STDF_MASK_BIT3) == 0, eposNUM_FAIL);

    // FTR.XFAIL_AD
    _FIELD_CHECKREAD(clStdf.ReadDword(&dwData), eposXFAIL_AD);
    _FIELD_SET(m_i4XFAIL_AD = stdf_type_i4(dwData), (m_b1OPT_FLAG & STDF_MASK_BIT4) == 0, eposXFAIL_AD);

    // FTR.YFAIL_AD
    _FIELD_CHECKREAD(clStdf.ReadDword(&dwData), eposYFAIL_AD);
    _FIELD_SET(m_i4YFAIL_AD = stdf_type_i4(dwData), (m_b1OPT_FLAG & STDF_MASK_BIT4) == 0, eposYFAIL_AD);

    // FTR.VECT_OFF
    _FIELD_CHECKREAD(clStdf.ReadWord(&wData), eposVECT_OFF);
    _FIELD_SET(m_i2VECT_OFF = stdf_type_i2(wData), (m_b1OPT_FLAG & STDF_MASK_BIT5) == 0, eposVECT_OFF);

    // FTR.RTN_ICNT
    _FIELD_CHECKREAD(clStdf.ReadWord(&wData), eposRTN_ICNT);
    _FIELD_SET(m_u2RTN_ICNT = stdf_type_u2(wData), true, eposRTN_ICNT);

    // FTR.PGM_ICNT
    _FIELD_CHECKREAD(clStdf.ReadWord(&wData), eposPGM_ICNT);
    _FIELD_SET(m_u2PGM_ICNT = stdf_type_u2(wData), true, eposPGM_ICNT);

    // FTR.RTN_INDX
    // Set the field as present, even if count is 0. We use a dummy instruction as parameter 1 to avoid warning under gcc.
    _FIELD_SET(m_uiIndex_macro = 0, true, eposRTN_INDX);
    if(m_u2RTN_ICNT > 0)
        m_ku2RTN_INDX = new stdf_type_u2[m_u2RTN_ICNT];
    for(i=0; i<(unsigned int)m_u2RTN_ICNT; i++)
    {
        _FIELD_CHECKREAD(clStdf.ReadWord(&wData), eposRTN_INDX);
        _FIELD_SET(m_ku2RTN_INDX[i] = stdf_type_u2(wData), true, eposRTN_INDX);
    }

    // FTR.RTN_STAT
    // Set the field as present, even if count is 0. We use a dummy instruction as parameter 1 to avoid warning under gcc.
    _FIELD_SET(m_uiIndex_macro = 0, true, eposRTN_STAT);
    if(m_u2RTN_ICNT > 0)
        m_kn1RTN_STAT = new stdf_type_n1[m_u2RTN_ICNT];
    for(i=0; i<(unsigned int)((m_u2RTN_ICNT+1)/2); i++)
    {
        _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposRTN_STAT);
        _FIELD_SET(m_kn1RTN_STAT[i*2] = stdf_type_n1(bData & 0x0f), true, eposRTN_STAT);
        if(i*2+1 < m_u2RTN_ICNT)
            _FIELD_SET(m_kn1RTN_STAT[i*2+1] = stdf_type_n1((bData >> 4) & 0x0f), true, eposRTN_STAT);
    }

    // FTR.PGM_INDX
    // Set the field as present, even if count is 0. We use a dummy instruction as parameter 1 to avoid warning under gcc.
    _FIELD_SET(m_uiIndex_macro = 0, true, eposPGM_INDX);
    if(m_u2PGM_ICNT > 0)
        m_ku2PGM_INDX = new stdf_type_u2[m_u2PGM_ICNT];
    for(i=0; i<(unsigned int)m_u2PGM_ICNT; i++)
    {
        _FIELD_CHECKREAD(clStdf.ReadWord(&wData), eposPGM_INDX);
        _FIELD_SET(m_ku2PGM_INDX[i] = stdf_type_u2(wData), true, eposPGM_INDX);
    }

    // FTR.PGM_STAT
    // Set the field as present, even if count is 0. We use a dummy instruction as parameter 1 to avoid warning under gcc.
    _FIELD_SET(m_uiIndex_macro = 0, true, eposPGM_STAT);
    if(m_u2PGM_ICNT > 0)
        m_kn1PGM_STAT = new stdf_type_n1[m_u2PGM_ICNT];
    for(i=0; i<(unsigned int)((m_u2PGM_ICNT+1)/2); i++)
    {
        _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposPGM_STAT);
        _FIELD_SET(m_kn1PGM_STAT[i*2] = stdf_type_n1(bData & 0x0f), true, eposPGM_STAT);
        if(i*2+1 < m_u2PGM_ICNT)
            _FIELD_SET(m_kn1PGM_STAT[i*2+1] = stdf_type_n1((bData >> 4) & 0x0f), true, eposPGM_STAT);
    }

    // FTR.FAIL_PIN
    _FIELD_CHECKREAD(clStdf.ReadDBitField(&(m_dnFAIL_PIN.m_uiLength), m_dnFAIL_PIN.m_pBitField), eposFAIL_PIN);
    // The field has already been set by the ReadBitField function. We use a dummy instruction as parameter 1 to avoid warning under gcc.
    _FIELD_SET(m_uiIndex_macro = 0, m_dnFAIL_PIN.m_uiLength != 0, eposFAIL_PIN);

    // FTR.VECT_NAM
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposVECT_NAM);
    _FIELD_SET(m_cnVECT_NAM = szString, !m_cnVECT_NAM.isEmpty(), eposVECT_NAM);

    // FTR.TIME_SET
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposTIME_SET);
    _FIELD_SET(m_cnTIME_SET = szString, !m_cnTIME_SET.isEmpty(), eposTIME_SET);

    // FTR.OP_CODE
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposOP_CODE);
    _FIELD_SET(m_cnOP_CODE = szString, !m_cnOP_CODE.isEmpty(), eposOP_CODE);

    // FTR.TEST_TXT
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposTEST_TXT);
    _FIELD_SET(m_cnTEST_TXT = szString, !m_cnTEST_TXT.isEmpty(), eposTEST_TXT);

    // FTR.ALARM_ID
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposALARM_ID);
    _FIELD_SET(m_cnALARM_ID = szString, !m_cnALARM_ID.isEmpty(), eposALARM_ID);

    // FTR.PROG_TXT
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposPROG_TXT);
    _FIELD_SET(m_cnPROG_TXT = szString, !m_cnPROG_TXT.isEmpty(), eposPROG_TXT);

    // FTR.RSLT_TXT
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposRSLT_TXT);
    _FIELD_SET(m_cnRSLT_TXT = szString, !m_cnRSLT_TXT.isEmpty(), eposRSLT_TXT);

    // FTR.PATG_NUM
    _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposPATG_NUM);
    _FIELD_SET(m_u1PATG_NUM = stdf_type_u1(bData), m_u1PATG_NUM != 255, eposPATG_NUM);

    // FTR.SPIN_MAP
    _FIELD_CHECKREAD(clStdf.ReadDBitField(&(m_dnSPIN_MAP.m_uiLength), m_dnSPIN_MAP.m_pBitField), eposSPIN_MAP);
    // The field has already been set by the ReadBitField function. We use a dummy instruction as parameter 1 to avoid warning under gcc.
    _FIELD_SET(m_uiIndex_macro = 0, m_dnSPIN_MAP.m_uiLength != 0, eposSPIN_MAP);

    return true;
}

bool Stdf_FTR_V4::Write(GS::StdLib::Stdf & clStdf)
{
    GS::StdLib::StdfRecordReadInfo	RecordReadInfo;
    unsigned int		i;
    stdf_type_u1		u1Temp;

    RecordReadInfo.iRecordType = STDF_FTR_TYPE;
    RecordReadInfo.iRecordSubType = STDF_FTR_STYPE;
    clStdf.WriteHeader(&RecordReadInfo);

    // FTR.TEST_NUM
    _FIELD_CHECKWRITE(clStdf.WriteDword(DWORD(m_u4TEST_NUM)), eposTEST_NUM, clStdf.WriteRecord());

    // FTR.HEAD_NUM
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_u1HEAD_NUM)), eposHEAD_NUM, clStdf.WriteRecord());

    // FTR.SITE_NUM
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_u1SITE_NUM)), eposSITE_NUM, clStdf.WriteRecord());

    // FTR.TEST_FLG
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_b1TEST_FLG)), eposTEST_FLG, clStdf.WriteRecord());

    // FTR.OPT_FLAG
    _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_b1OPT_FLAG)), eposOPT_FLAG, clStdf.WriteRecord());

    // FTR.CYCL_CNT
    _FIELD_CHECKWRITE(clStdf.WriteDword(DWORD(m_u4CYCL_CNT)), eposCYCL_CNT, clStdf.WriteRecord());

    // FTR.REL_VADR
    _FIELD_CHECKWRITE(clStdf.WriteDword(DWORD(m_u4REL_VADR)), eposREL_VADR, clStdf.WriteRecord());

    // FTR.REPT_CNT
    _FIELD_CHECKWRITE(clStdf.WriteDword(DWORD(m_u4REPT_CNT)), eposREPT_CNT, clStdf.WriteRecord());

    // FTR.NUM_FAIL
    _FIELD_CHECKWRITE(clStdf.WriteDword(DWORD(m_u4NUM_FAIL)), eposNUM_FAIL, clStdf.WriteRecord());

    // FTR.XFAIL_AD
    _FIELD_CHECKWRITE(clStdf.WriteDword(DWORD(m_i4XFAIL_AD)), eposXFAIL_AD, clStdf.WriteRecord());

    // FTR.YFAIL_AD
    _FIELD_CHECKWRITE(clStdf.WriteDword(DWORD(m_i4YFAIL_AD)), eposYFAIL_AD, clStdf.WriteRecord());

    // FTR.VECT_OFF
    _FIELD_CHECKWRITE(clStdf.WriteWord(WORD(m_i2VECT_OFF)), eposVECT_OFF, clStdf.WriteRecord());

    // FTR.RTN_ICNT
    _FIELD_CHECKWRITE(clStdf.WriteWord(WORD(m_u2RTN_ICNT)), eposRTN_ICNT, clStdf.WriteRecord());

    // FTR.PGM_ICNT
    _FIELD_CHECKWRITE(clStdf.WriteWord(WORD(m_u2PGM_ICNT)), eposPGM_ICNT, clStdf.WriteRecord());

    // FTR.RTN_INDX
    for(i=0; i<(unsigned int)m_u2RTN_ICNT; i++)
    _FIELD_CHECKWRITE(clStdf.WriteWord(WORD(m_ku2RTN_INDX[i])), eposRTN_INDX, clStdf.WriteRecord());

    // FTR.RTN_STAT
    for(i=0; i<(unsigned int)((m_u2RTN_ICNT+1)/2); i++)
    {
        u1Temp = 0;
        if(i*2+1 < m_u2RTN_ICNT)
            u1Temp = (m_kn1RTN_STAT[i*2+1] << 4) & 0xf0;
        u1Temp |= (m_kn1RTN_STAT[i*2] & 0x0f);
        _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(u1Temp)), eposRTN_STAT, clStdf.WriteRecord());
    }

    // FTR.PGM_INDX
    for(i=0; i<(unsigned int)m_u2PGM_ICNT; i++)
    _FIELD_CHECKWRITE(clStdf.WriteWord(WORD(m_ku2PGM_INDX[i])), eposPGM_INDX, clStdf.WriteRecord());

    // FTR.PGM_STAT
    for(i=0; i<(unsigned int)((m_u2PGM_ICNT+1)/2); i++)
    {
        u1Temp = 0;
        if(i*2+1 < m_u2PGM_ICNT)
            u1Temp = (m_kn1PGM_STAT[i*2+1] << 4) & 0xf0;
        u1Temp |= (m_kn1PGM_STAT[i*2] & 0x0f);
        _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(u1Temp)), eposPGM_STAT, clStdf.WriteRecord());
    }

    // FTR.FAIL_PIN
    _FIELD_CHECKWRITE(clStdf.WriteDBitField(BYTE(m_dnFAIL_PIN.m_uiLength), m_dnFAIL_PIN.m_pBitField), eposFAIL_PIN, clStdf.WriteRecord());

    // FTR.VECT_NAM
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnVECT_NAM.toLatin1().constData()), eposVECT_NAM, clStdf.WriteRecord());

    // FTR.TIME_SET
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnTIME_SET.toLatin1().constData()), eposTIME_SET, clStdf.WriteRecord());

    // FTR.OP_CODE
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnOP_CODE.toLatin1().constData()), eposOP_CODE, clStdf.WriteRecord());

    // FTR.TEST_TXT
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnTEST_TXT.toLatin1().constData()), eposTEST_TXT, clStdf.WriteRecord());

    // FTR.ALARM_ID
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnALARM_ID.toLatin1().constData()), eposALARM_ID, clStdf.WriteRecord());

    // FTR.PROG_TXT
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnPROG_TXT.toLatin1().constData()), eposPROG_TXT, clStdf.WriteRecord());

    // FTR.RSLT_TXT
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnRSLT_TXT.toLatin1().constData()), eposRSLT_TXT, clStdf.WriteRecord());

    // FTR.PATG_NUM
    _FIELD_CHECKWRITE(clStdf.WriteByte(m_u1PATG_NUM), eposPATG_NUM, clStdf.WriteRecord());

    // FTR.SPIN_MAP
    _FIELD_CHECKWRITE(clStdf.WriteDBitField(BYTE(m_dnSPIN_MAP.m_uiLength), m_dnSPIN_MAP.m_pBitField), eposSPIN_MAP, clStdf.WriteRecord());

    clStdf.WriteRecord();
    return true;
}

void Stdf_FTR_V4::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    // Empty string first
    strAsciiString = "";

    // FTR.TEST_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "FTR.test_num", QString::number(m_u4TEST_NUM), nFieldSelection, eposTEST_NUM);

    // FTR.HEAD_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "FTR.head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // FTR.SITE_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "FTR.site_num", QString::number(m_u1SITE_NUM), nFieldSelection, eposSITE_NUM);

    // FTR.TEST_FLG
    _CREATEFIELD_FROM_B1_ASCII(m_b1TEST_FLG)
    _STR_ADDFIELD_ASCII(strAsciiString, "FTR.test_flg", m_strFieldValue_macro, nFieldSelection, eposTEST_FLG);

    // FTR.OPT_FLAG
    _CREATEFIELD_FROM_B1_ASCII(m_b1OPT_FLAG)
    _STR_ADDFIELD_ASCII(strAsciiString, "FTR.opt_flag", m_strFieldValue_macro, nFieldSelection, eposOPT_FLAG);

    // FTR.CYCL_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "FTR.cycl_cnt", QString::number(m_u4CYCL_CNT), nFieldSelection, eposCYCL_CNT);

    // FTR.REL_VADR
    _STR_ADDFIELD_ASCII(strAsciiString, "FTR.rel_vadr", QString::number(m_u4REL_VADR), nFieldSelection, eposREL_VADR);

    // FTR.REPT_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "FTR.rept_cnt", QString::number(m_u4REPT_CNT), nFieldSelection, eposREPT_CNT);

    // FTR.NUM_FAIL
    _STR_ADDFIELD_ASCII(strAsciiString, "FTR.num_fail", QString::number(m_u4NUM_FAIL), nFieldSelection, eposNUM_FAIL);

    // FTR.XFAIL_AD
    _STR_ADDFIELD_ASCII(strAsciiString, "FTR.xfail_ad", QString::number(m_i4XFAIL_AD), nFieldSelection, eposXFAIL_AD);

    // FTR.YFAIL_AD
    _STR_ADDFIELD_ASCII(strAsciiString, "FTR.yfail_ad", QString::number(m_i4YFAIL_AD), nFieldSelection, eposYFAIL_AD);

    // FTR.VECT_OFF
    _STR_ADDFIELD_ASCII(strAsciiString, "FTR.vect_off", QString::number(m_i2VECT_OFF), nFieldSelection, eposVECT_OFF);

    // FTR.RTN_ICNT
    _STR_ADDFIELD_ASCII(strAsciiString, "FTR.rtn_icnt", QString::number(m_u2RTN_ICNT), nFieldSelection, eposRTN_ICNT);

    // FTR.PGM_ICNT
    _STR_ADDFIELD_ASCII(strAsciiString, "FTR.pgm_icnt", QString::number(m_u2PGM_ICNT), nFieldSelection, eposPGM_ICNT);

    // FTR.RTN_INDX
    _CREATEFIELD_FROM_KUi_ASCII(m_u2RTN_ICNT, m_ku2RTN_INDX, eposRTN_INDX);
    _STR_ADDFIELD_ASCII(strAsciiString, "FTR.rtn_indx", m_strFieldValue_macro, nFieldSelection, eposRTN_INDX);

    // FTR.RTN_STAT
    _CREATEFIELD_FROM_KN1_ASCII(m_u2RTN_ICNT, m_kn1RTN_STAT, eposRTN_STAT);
    _STR_ADDFIELD_ASCII(strAsciiString, "FTR.rtn_stat", m_strFieldValue_macro, nFieldSelection, eposRTN_STAT);

    // FTR.PGM_INDX
    _CREATEFIELD_FROM_KUi_ASCII(m_u2PGM_ICNT, m_ku2PGM_INDX, eposPGM_INDX);
    _STR_ADDFIELD_ASCII(strAsciiString, "FTR.pgm_indx", m_strFieldValue_macro, nFieldSelection, eposPGM_INDX);

    // FTR.PGM_STAT
    _CREATEFIELD_FROM_KN1_ASCII(m_u2PGM_ICNT, m_kn1PGM_STAT, eposPGM_STAT);
    _STR_ADDFIELD_ASCII(strAsciiString, "FTR.pgm_stat", m_strFieldValue_macro, nFieldSelection, eposPGM_STAT);

    // FTR.FAIL_PIN
    _CREATEFIELD_FROM_DN_ASCII(m_dnFAIL_PIN);
    _STR_ADDFIELD_ASCII(strAsciiString, "FTR.fail_pin", m_strFieldValue_macro, nFieldSelection, eposFAIL_PIN);

    // FTR.VECT_NAM
    _STR_ADDFIELD_ASCII(strAsciiString, "FTR.vect_nam", m_cnVECT_NAM, nFieldSelection, eposVECT_NAM);

    // FTR.TIME_SET
    _STR_ADDFIELD_ASCII(strAsciiString, "FTR.time_set", m_cnTIME_SET, nFieldSelection, eposTIME_SET);

    // FTR.OP_CODE
    _STR_ADDFIELD_ASCII(strAsciiString, "FTR.op_code", m_cnOP_CODE, nFieldSelection, eposOP_CODE);

    // FTR.TEST_TXT
    _STR_ADDFIELD_ASCII(strAsciiString, "FTR.test_txt", m_cnTEST_TXT, nFieldSelection, eposTEST_TXT);

    // FTR.ALARM_ID
    _STR_ADDFIELD_ASCII(strAsciiString, "FTR.alarm_id", m_cnALARM_ID, nFieldSelection, eposALARM_ID);

    // FTR.PROG_TXT
    _STR_ADDFIELD_ASCII(strAsciiString, "FTR.prog_txt", m_cnPROG_TXT, nFieldSelection, eposPROG_TXT);

    // FTR.RSLT_TXT
    _STR_ADDFIELD_ASCII(strAsciiString, "FTR.rslt_txt", m_cnRSLT_TXT, nFieldSelection, eposRSLT_TXT);

    // FTR.PATG_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "FTR.patg_num", QString::number(m_u1PATG_NUM), nFieldSelection, eposPATG_NUM);

    // FTR.SPIN_MAP
    _CREATEFIELD_FROM_DN_ASCII(m_dnSPIN_MAP);
    _STR_ADDFIELD_ASCII(strAsciiString, "FTR.spin_map", m_strFieldValue_macro, nFieldSelection, eposSPIN_MAP);
}

void Stdf_FTR_V4::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
    // Empty string list first
    listFields.empty();

    // FTR.TEST_NUM
    _LIST_ADDFIELD_ASCII(listFields, "FTR.test_num", QString::number(m_u4TEST_NUM), nFieldSelection, eposTEST_NUM);

    // FTR.HEAD_NUM
    _LIST_ADDFIELD_ASCII(listFields, "FTR.head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // FTR.SITE_NUM
    _LIST_ADDFIELD_ASCII(listFields, "FTR.site_num", QString::number(m_u1SITE_NUM), nFieldSelection, eposSITE_NUM);

    // FTR.TEST_FLG
    _CREATEFIELD_FROM_B1_ASCII(m_b1TEST_FLG)
    _LIST_ADDFIELD_ASCII(listFields, "FTR.test_flg", m_strFieldValue_macro, nFieldSelection, eposTEST_FLG);

    // FTR.OPT_FLAG
    _CREATEFIELD_FROM_B1_ASCII(m_b1OPT_FLAG)
    _LIST_ADDFIELD_ASCII(listFields, "FTR.opt_flag", m_strFieldValue_macro, nFieldSelection, eposOPT_FLAG);

    // FTR.CYCL_CNT
    _LIST_ADDFIELD_ASCII(listFields, "FTR.cycl_cnt", QString::number(m_u4CYCL_CNT), nFieldSelection, eposCYCL_CNT);

    // FTR.REL_VADR
    _LIST_ADDFIELD_ASCII(listFields, "FTR.rel_vadr", QString::number(m_u4REL_VADR), nFieldSelection, eposREL_VADR);

    // FTR.REPT_CNT
    _LIST_ADDFIELD_ASCII(listFields, "FTR.rept_cnt", QString::number(m_u4REPT_CNT), nFieldSelection, eposREPT_CNT);

    // FTR.NUM_FAIL
    _LIST_ADDFIELD_ASCII(listFields, "FTR.num_fail", QString::number(m_u4NUM_FAIL), nFieldSelection, eposNUM_FAIL);

    // FTR.XFAIL_AD
    _LIST_ADDFIELD_ASCII(listFields, "FTR.xfail_ad", QString::number(m_i4XFAIL_AD), nFieldSelection, eposXFAIL_AD);

    // FTR.YFAIL_AD
    _LIST_ADDFIELD_ASCII(listFields, "FTR.yfail_ad", QString::number(m_i4YFAIL_AD), nFieldSelection, eposYFAIL_AD);

    // FTR.VECT_OFF
    _LIST_ADDFIELD_ASCII(listFields, "FTR.vect_off", QString::number(m_i2VECT_OFF), nFieldSelection, eposVECT_OFF);

    // FTR.RTN_ICNT
    _LIST_ADDFIELD_ASCII(listFields, "FTR.rtn_icnt", QString::number(m_u2RTN_ICNT), nFieldSelection, eposRTN_ICNT);

    // FTR.PGM_ICNT
    _LIST_ADDFIELD_ASCII(listFields, "FTR.pgm_icnt", QString::number(m_u2PGM_ICNT), nFieldSelection, eposPGM_ICNT);

    // FTR.RTN_INDX
    _CREATEFIELD_FROM_KUi_ASCII(m_u2RTN_ICNT, m_ku2RTN_INDX, eposRTN_INDX);
    _LIST_ADDFIELD_ASCII(listFields, "FTR.rtn_indx", m_strFieldValue_macro, nFieldSelection, eposRTN_INDX);

    // FTR.RTN_STAT
    _CREATEFIELD_FROM_KN1_ASCII(m_u2RTN_ICNT, m_kn1RTN_STAT, eposRTN_STAT);
    _LIST_ADDFIELD_ASCII(listFields, "FTR.rtn_stat", m_strFieldValue_macro, nFieldSelection, eposRTN_STAT);

    // FTR.PGM_INDX
    _CREATEFIELD_FROM_KUi_ASCII(m_u2PGM_ICNT, m_ku2PGM_INDX, eposPGM_INDX);
    _LIST_ADDFIELD_ASCII(listFields, "FTR.pgm_indx", m_strFieldValue_macro, nFieldSelection, eposPGM_INDX);

    // FTR.PGM_STAT
    _CREATEFIELD_FROM_KN1_ASCII(m_u2PGM_ICNT, m_kn1PGM_STAT, eposPGM_STAT);
    _LIST_ADDFIELD_ASCII(listFields, "FTR.pgm_stat", m_strFieldValue_macro, nFieldSelection, eposPGM_STAT);

    // FTR.FAIL_PIN
    _CREATEFIELD_FROM_DN_ASCII(m_dnFAIL_PIN);
    _LIST_ADDFIELD_ASCII(listFields, "FTR.fail_pin", m_strFieldValue_macro, nFieldSelection, eposFAIL_PIN);

    // FTR.VECT_NAM
    _LIST_ADDFIELD_ASCII(listFields, "FTR.vect_nam", m_cnVECT_NAM, nFieldSelection, eposVECT_NAM);

    // FTR.TIME_SET
    _LIST_ADDFIELD_ASCII(listFields, "FTR.time_set", m_cnTIME_SET, nFieldSelection, eposTIME_SET);

    // FTR.OP_CODE
    _LIST_ADDFIELD_ASCII(listFields, "FTR.op_code", m_cnOP_CODE, nFieldSelection, eposOP_CODE);

    // FTR.TEST_TXT
    _LIST_ADDFIELD_ASCII(listFields, "FTR.test_txt", m_cnTEST_TXT, nFieldSelection, eposTEST_TXT);

    // FTR.ALARM_ID
    _LIST_ADDFIELD_ASCII(listFields, "FTR.alarm_id", m_cnALARM_ID, nFieldSelection, eposALARM_ID);

    // FTR.PROG_TXT
    _LIST_ADDFIELD_ASCII(listFields, "FTR.prog_txt", m_cnPROG_TXT, nFieldSelection, eposPROG_TXT);

    // FTR.RSLT_TXT
    _LIST_ADDFIELD_ASCII(listFields, "FTR.rslt_txt", m_cnRSLT_TXT, nFieldSelection, eposRSLT_TXT);

    // FTR.PATG_NUM
    _LIST_ADDFIELD_ASCII(listFields, "FTR.patg_num", QString::number(m_u1PATG_NUM), nFieldSelection, eposPATG_NUM);

    // FTR.SPIN_MAP
    _CREATEFIELD_FROM_DN_ASCII(m_dnSPIN_MAP);
    _LIST_ADDFIELD_ASCII(listFields, "FTR.spin_map", m_strFieldValue_macro, nFieldSelection, eposSPIN_MAP);
}

void Stdf_FTR_V4::GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection /*= 0*/)
{
    QString strTabs="";

    // Init XML string
    for(int nIndex=nIndentationLevel; nIndex>0; nIndex--)
        strTabs += "\t";
    strXmlString = strTabs;
    strXmlString += "<ftr>\n";

    // FTR.TEST_NUM
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "test_num", QString::number(m_u4TEST_NUM), nFieldSelection, eposTEST_NUM);

    // FTR.HEAD_NUM
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // FTR.SITE_NUM
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "site_num", QString::number(m_u1SITE_NUM), nFieldSelection, eposSITE_NUM);

    // FTR.TEST_FLG
    _CREATEFIELD_FROM_B1_XML(m_b1TEST_FLG)
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "test_flg", m_strFieldValue_macro, nFieldSelection, eposTEST_FLG);

    // FTR.OPT_FLAG
    _CREATEFIELD_FROM_B1_XML(m_b1OPT_FLAG)
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "opt_flag", m_strFieldValue_macro, nFieldSelection, eposOPT_FLAG);

    // FTR.CYCL_CNT
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "cycl_cnt", QString::number(m_u4CYCL_CNT), nFieldSelection, eposCYCL_CNT);

    // FTR.REL_VADR
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "rel_vadr", QString::number(m_u4REL_VADR), nFieldSelection, eposREL_VADR);

    // FTR.REPT_CNT
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "rept_cnt", QString::number(m_u4REPT_CNT), nFieldSelection, eposREPT_CNT);

    // FTR.NUM_FAIL
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "num_fail", QString::number(m_u4NUM_FAIL), nFieldSelection, eposNUM_FAIL);

    // FTR.XFAIL_AD
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "xfail_ad", QString::number(m_i4XFAIL_AD), nFieldSelection, eposXFAIL_AD);

    // FTR.YFAIL_AD
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "yfail_ad", QString::number(m_i4YFAIL_AD), nFieldSelection, eposYFAIL_AD);

    // FTR.VECT_OFF
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "vect_off", QString::number(m_i2VECT_OFF), nFieldSelection, eposVECT_OFF);

    // FTR.RTN_INDX
    _CREATEFIELD_FROM_KUi_XML(m_u2RTN_ICNT, m_ku2RTN_INDX, eposRTN_INDX);
    _STR_ADDLIST_XML(strXmlString, m_u2RTN_ICNT, nIndentationLevel+1, "rtn_indx", m_strFieldValue_macro, nFieldSelection, eposRTN_INDX);

    // FTR.RTN_STAT
    _CREATEFIELD_FROM_KN1_XML(m_u2RTN_ICNT, m_kn1RTN_STAT, eposRTN_STAT);
    _STR_ADDLIST_XML(strXmlString, m_u2RTN_ICNT, nIndentationLevel+1, "rtn_stat", m_strFieldValue_macro, nFieldSelection, eposRTN_STAT);

    // FTR.PGM_INDX
    _CREATEFIELD_FROM_KUi_XML(m_u2PGM_ICNT, m_ku2PGM_INDX, eposPGM_INDX);
    _STR_ADDLIST_XML(strXmlString, m_u2RTN_ICNT, nIndentationLevel+1, "pgm_indx", m_strFieldValue_macro, nFieldSelection, eposPGM_INDX);

    // FTR.PGM_STAT
    _CREATEFIELD_FROM_KN1_XML(m_u2PGM_ICNT, m_kn1PGM_STAT, eposPGM_STAT);
    _STR_ADDLIST_XML(strXmlString, m_u2RTN_ICNT, nIndentationLevel+1, "pgm_stat", m_strFieldValue_macro, nFieldSelection, eposPGM_STAT);

    // FTR.FAIL_PIN
    _CREATEFIELD_FROM_DN_XML(m_dnFAIL_PIN);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "fail_pin", m_strFieldValue_macro, nFieldSelection, eposFAIL_PIN);

    // FTR.VECT_NAM
    _CREATEFIELD_FROM_CN_XML(m_cnVECT_NAM);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "vect_nam", m_strFieldValue_macro, nFieldSelection, eposVECT_NAM);

    // FTR.TIME_SET
    _CREATEFIELD_FROM_CN_XML(m_cnTIME_SET);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "time_set", m_strFieldValue_macro, nFieldSelection, eposTIME_SET);

    // FTR.OP_CODE
    _CREATEFIELD_FROM_CN_XML(m_cnOP_CODE);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "op_code", m_strFieldValue_macro, nFieldSelection, eposOP_CODE);

    // FTR.TEST_TXT
    _CREATEFIELD_FROM_CN_XML(m_cnTEST_TXT);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "test_txt", m_strFieldValue_macro, nFieldSelection, eposTEST_TXT);

    // FTR.ALARM_ID
    _CREATEFIELD_FROM_CN_XML(m_cnALARM_ID);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "alarm_id", m_strFieldValue_macro, nFieldSelection, eposALARM_ID);

    // FTR.PROG_TXT
    _CREATEFIELD_FROM_CN_XML(m_cnPROG_TXT);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "prog_txt", m_strFieldValue_macro, nFieldSelection, eposPROG_TXT);

    // FTR.RSLT_TXT
    _CREATEFIELD_FROM_CN_XML(m_cnRSLT_TXT);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "rslt_txt", m_strFieldValue_macro, nFieldSelection, eposRSLT_TXT);

    // FTR.PATG_NUM
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "patg_num", QString::number(m_u1PATG_NUM), nFieldSelection, eposPATG_NUM);

    // FTR.SPIN_MAP
    _CREATEFIELD_FROM_DN_XML(m_dnSPIN_MAP);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "spin_map", m_strFieldValue_macro, nFieldSelection, eposSPIN_MAP);

    strXmlString += strTabs;
    strXmlString += "</ftr>\n";
}

void Stdf_FTR_V4::GetAtdfString(QString & strAtdfString)
{
    // Reset field index, and line Nb
    m_uiFieldIndex = 0;
    m_uiLineNb = 1;

    // Init string
    strAtdfString = "FTR:";

    // FTR.TEST_NUM
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u4TEST_NUM), eposTEST_NUM);

    // FTR.HEAD_NUM
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u1HEAD_NUM), eposHEAD_NUM);

    // FTR.SITE_NUM
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u1SITE_NUM), eposSITE_NUM);

    // FTR.Pass/Fail Flag
    m_strFieldValue_macro = "P";
    if((m_b1TEST_FLG & 0x40) == 0)
    {
        if(m_b1TEST_FLG & 0x80)
            m_strFieldValue_macro = "F";
        else
            m_strFieldValue_macro = "P";
    }
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposTEST_FLG);

    // FTR.Alarm Flags
    m_strFieldValue_macro = "";
    if(m_b1TEST_FLG & 0x01)
        m_strFieldValue_macro += "A";
    if(m_b1TEST_FLG & 0x04)
        m_strFieldValue_macro += "U";
    if(m_b1TEST_FLG & 0x08)
        m_strFieldValue_macro += "T";
    if(m_b1TEST_FLG & 0x10)
        m_strFieldValue_macro += "N";
    if(m_b1TEST_FLG & 0x20)
        m_strFieldValue_macro += "X";
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposTEST_FLG);

    // FTR.VECT_NAM
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnVECT_NAM, eposVECT_NAM);

    // FTR.TIME_SET
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnTIME_SET, eposTIME_SET);

    // FTR.CYCL_CNT
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u4CYCL_CNT), eposCYCL_CNT);

    // FTR.REL_VADR
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u4REL_VADR, 16), eposREL_VADR);

    // FTR.REPT_CNT
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u4REPT_CNT), eposREPT_CNT);

    // FTR.NUM_FAIL
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u4NUM_FAIL), eposNUM_FAIL);

    // FTR.XFAIL_AD
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_i4XFAIL_AD), eposXFAIL_AD);

    // FTR.YFAIL_AD
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_i4YFAIL_AD), eposYFAIL_AD);

    // FTR.VECT_OFF
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_i2VECT_OFF), eposVECT_OFF);

    // FTR.RTN_INDX
    _CREATEFIELD_FROM_KUi_ATDF(m_u2RTN_ICNT, m_ku2RTN_INDX, eposRTN_INDX);
    _STR_ADDFIELD_ATDF(strAtdfString,  m_strFieldValue_macro, eposRTN_INDX);

    // FTR.RTN_STAT
    _CREATEFIELD_FROM_KN1_ATDF(m_u2RTN_ICNT, m_kn1RTN_STAT, eposRTN_STAT);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposRTN_STAT);

    // FTR.PGM_INDX
    _CREATEFIELD_FROM_KUi_ATDF(m_u2PGM_ICNT, m_ku2PGM_INDX, eposPGM_INDX);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposPGM_INDX);

    // FTR.PGM_STAT
    _CREATEFIELD_FROM_KN1_ATDF(m_u2PGM_ICNT, m_kn1PGM_STAT, eposPGM_STAT);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposPGM_STAT);

    // FTR.FAIL_PIN
    _CREATEFIELD_FROM_DN_ATDF_FTR_PINS(m_dnFAIL_PIN);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposFAIL_PIN);

    // FTR.OP_CODE
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnOP_CODE, eposOP_CODE);

    // FTR.TEST_TXT
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnTEST_TXT, eposTEST_TXT);

    // FTR.ALARM_ID
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnALARM_ID, eposALARM_ID);

    // FTR.PROG_TXT
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnPROG_TXT, eposPROG_TXT);

    // FTR.RSLT_TXT
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnRSLT_TXT, eposRSLT_TXT);

    // FTR.PATG_NUM
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u1PATG_NUM), eposPATG_NUM);

    // FTR.SPIN_MAP
    _CREATEFIELD_FROM_DN_ATDF_FTR_PINS(m_dnSPIN_MAP);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposSPIN_MAP);

    _STR_FINISHFIELD_ATDF(strAtdfString);
}

///////////////////////////////////////////////////////////
// BPS RECORD
///////////////////////////////////////////////////////////
Stdf_BPS_V4::Stdf_BPS_V4() : Stdf_Record()
{
    Reset();
}

Stdf_BPS_V4::~Stdf_BPS_V4()
{
    Reset();
}

void Stdf_BPS_V4::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Select fields for reduced list
    m_pFieldFlags[eposSEQ_NAME]	|= FieldFlag_ReducedList;

    // Reset Data
    m_cnSEQ_NAME		= "";		// BPS.SEQ_NAME

    // Call Reset base method
    Stdf_Record::Reset();
}

bool Stdf_BPS_V4::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;

    return false;
}

QString Stdf_BPS_V4::GetRecordShortName(void)
{
    return "BPS";
}

QString Stdf_BPS_V4::GetRecordLongName(void)
{
    return "Begin Program Section Record";
}

int Stdf_BPS_V4::GetRecordType(void)
{
    return Rec_BPS;
}

bool Stdf_BPS_V4::Read(GS::StdLib::Stdf & clStdf)
{
    char	szString[STDF_MAX_U1+1];

    // First reset data
    Reset();

    // BPS.SEQ_NAME
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposSEQ_NAME);
    _FIELD_SET(m_cnSEQ_NAME = szString, !m_cnSEQ_NAME.isEmpty(), eposSEQ_NAME);

    return true;
}

bool Stdf_BPS_V4::Write(GS::StdLib::Stdf & clStdf)
{
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;

    RecordReadInfo.iRecordType = STDF_BPS_TYPE;
    RecordReadInfo.iRecordSubType = STDF_BPS_STYPE;
    clStdf.WriteHeader(&RecordReadInfo);

    // BPS.SEQ_NAME
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnSEQ_NAME.toLatin1().constData()), eposSEQ_NAME, clStdf.WriteRecord());

    clStdf.WriteRecord();

    return true;
}

void Stdf_BPS_V4::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    // Empty string first
    strAsciiString = "";

    // BPS.SEQ_NAME
    _STR_ADDFIELD_ASCII(strAsciiString, "BPS.seq_name", m_cnSEQ_NAME, nFieldSelection, eposSEQ_NAME);
}

void Stdf_BPS_V4::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
    // Empty string list first
    listFields.empty();

    // BPS.SEQ_NAME
    _LIST_ADDFIELD_ASCII(listFields, "BPS.seq_name", m_cnSEQ_NAME, nFieldSelection, eposSEQ_NAME);
}

void Stdf_BPS_V4::GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection /*= 0*/)
{
    QString strTabs="";

    // Init XML string
    for(int nIndex=nIndentationLevel; nIndex>0; nIndex--)
        strTabs += "\t";
    strXmlString = strTabs;
    strXmlString += "<bps>\n";

    // BPS.SEQ_NAME
    _CREATEFIELD_FROM_CN_XML(m_cnSEQ_NAME);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "seq_name", m_strFieldValue_macro, nFieldSelection, eposSEQ_NAME);

    strXmlString += strTabs;
    strXmlString += "</bps>\n";
}

void Stdf_BPS_V4::GetAtdfString(QString & strAtdfString)
{
    // Reset field index, and line Nb
    m_uiFieldIndex = 0;
    m_uiLineNb = 1;

    // Init string
    strAtdfString = "BPS:";

    // BPS.SEQ_NAME
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnSEQ_NAME, eposSEQ_NAME);

    _STR_FINISHFIELD_ATDF(strAtdfString);
}

///////////////////////////////////////////////////////////
// EPS RECORD
///////////////////////////////////////////////////////////
Stdf_EPS_V4::Stdf_EPS_V4() : Stdf_Record()
{
    Reset();
}

Stdf_EPS_V4::~Stdf_EPS_V4()
{
    Reset();
}

void Stdf_EPS_V4::Reset(void)
{
    // Reset field flags

    // Select fields for reduced list

    // Reset Data

    // Call Reset base method
    Stdf_Record::Reset();
}

QString Stdf_EPS_V4::GetRecordShortName(void)
{
    return "EPS";
}

QString Stdf_EPS_V4::GetRecordLongName(void)
{
    return "End Program Section Record";
}

int Stdf_EPS_V4::GetRecordType(void)
{
    return Rec_EPS;
}

bool Stdf_EPS_V4::Read(GS::StdLib::Stdf& /*clStdf*/)
{
    // First reset data
    Reset();

    // EPS (STDF V4)

    return true;
}

bool Stdf_EPS_V4::Write(GS::StdLib::Stdf & clStdf)
{
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;

    RecordReadInfo.iRecordType = STDF_EPS_TYPE;
    RecordReadInfo.iRecordSubType = STDF_EPS_STYPE;
    clStdf.WriteHeader(&RecordReadInfo);
    clStdf.WriteRecord();

    return true;
}

void Stdf_EPS_V4::GetAsciiString(QString& strAsciiString,
                                  int /*nFieldSelection = 0*/)
{
    // Empty string first
    strAsciiString = "";
}

void Stdf_EPS_V4::GetAsciiFieldList(QStringList& listFields,
                                     int /*nFieldSelection = 0*/)
{
    // Empty string list first
    listFields.empty();
}

void Stdf_EPS_V4::GetXMLString(QString& strXmlString,
                                const int nIndentationLevel,
                                int /*nFieldSelection = 0*/)
{
    QString strTabs="";

    // Init XML string
    for(int nIndex=nIndentationLevel; nIndex>0; nIndex--)
        strTabs += "\t";
    strXmlString = strTabs;
    strXmlString += "<eps />\n";
}

void Stdf_EPS_V4::GetAtdfString(QString & strAtdfString)
{
    // Reset field index, and line Nb
    m_uiFieldIndex = 0;
    m_uiLineNb = 1;

    // Init string
    strAtdfString = "EPS:";

    _STR_FINISHFIELD_ATDF(strAtdfString);
}

///////////////////////////////////////////////////////////
// DTR RECORD
///////////////////////////////////////////////////////////
Stdf_DTR_V4::Stdf_DTR_V4() : Stdf_Record()
{
    Reset();
}

Stdf_DTR_V4::~Stdf_DTR_V4()
{
    Reset();
}

void Stdf_DTR_V4::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Select fields for reduced list
    m_pFieldFlags[eposTEXT_DAT]	|= FieldFlag_ReducedList;

    // Reset Data
    m_cnTEXT_DAT		= "";		// DTR.TEXT_DAT

    // Call Reset base method
    Stdf_Record::Reset();
}

bool Stdf_DTR_V4::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;

    return false;
}

// Check if GEX command: Gross Die (if so, get value)
bool Stdf_DTR_V4::IsGexCommand_GrossDie(unsigned int *puiGrossDie, bool *pbOk)
{
    int		nGrossDie, nCommandIndex;
    QString strCommand = m_cnTEXT_DAT.simplified();
    QString	strLowerCommand = strCommand.toLower();
    QString strToken;

    *pbOk = true;

    // Check if GEX Gross die command: <cmd> gross_die=8290
    if(strLowerCommand.startsWith("<cmd> gross_die", Qt::CaseInsensitive))
    {
        // Parse command line
        nGrossDie = strCommand.section('=', 1, 1).simplified().toInt(pbOk);
        if(*pbOk == true)
            *puiGrossDie = nGrossDie;
        return true;
    }
    // Note: Vishay Eagle files have [0,15,"<cmd> GrossDieCount =8290",""] instead of [<cmd> gross_die=8290]
    nCommandIndex = strLowerCommand.indexOf("<cmd> gross_die", 0, Qt::CaseInsensitive);
    if(nCommandIndex >= 0)
    {
        // Parse command line
        strToken = strCommand.mid(nCommandIndex);
        nGrossDie = strToken.section(QRegExp("[=\"]"), 1, 1).simplified().toInt(pbOk);
        if(*pbOk == true)
            *puiGrossDie = nGrossDie;
        return true;
    }
    nCommandIndex = strLowerCommand.indexOf("<cmd> grossdiecount", 0, Qt::CaseInsensitive);
    if(nCommandIndex >= 0)
    {
        // Parse command line
        strToken = strCommand.mid(nCommandIndex);
        nGrossDie = strToken.section(QRegExp("[=\"]"), 1, 1).simplified().toInt(pbOk);
        if(*pbOk == true)
            *puiGrossDie = nGrossDie;
        return true;
    }

    return false;
}

// Check if GEX die-tracking command:
// <cmd> multi-die die=<die #>;wafer_product=<wafer ProductID>;wafer_lot=<wafer LotID>;wafer_sublot=<wafer SublotID>
// OR
// <cmd> die-trackinhg die=<die #>;wafer_product=<wafer ProductID>;wafer_lot=<wafer LotID>;wafer_sublot=<wafer SublotID>
bool Stdf_DTR_V4::IsGexCommand_DieTracking(QString & strDieID, QString & strCommand, bool *pbOk)
{
    QString		strTempDieID;
    QString		strTempCommand = m_cnTEXT_DAT.simplified();
    QString		strLowerCommand = strTempCommand.toLower();
    QString		strToken, strGexCmd;

    *pbOk = true;

    // Check if GEX Die-tracking command
    if(strLowerCommand.startsWith("<cmd> multi-die", Qt::CaseInsensitive))
        strGexCmd = "<cmd> multi-die";
    if(strLowerCommand.startsWith("<cmd> die-tracking", Qt::CaseInsensitive))
        strGexCmd = "<cmd> die-tracking";
    if(strGexCmd != "")
    {
        // Get Die#
        strToken = strTempCommand.mid(strGexCmd.length()).simplified().section(';',0,0).simplified();
        if(strToken.isEmpty())
        {
            *pbOk = false;
            return true;
        }
        strTempDieID = strToken.section('=',0,0).simplified();
        if(strTempDieID.toLower() != "die")
        {
            *pbOk = false;
            return true;
        }
        strTempDieID = strToken.section('=',1,1).simplified();
        if(strTempDieID.isEmpty())
        {
            *pbOk = false;
            return true;
        }
        // Get rest of the command string
        strToken = strTempCommand.section(';',1).simplified();
        if(strToken.isEmpty())
        {
            *pbOk = false;
            return true;
        }

        // Save die-tracking info
        strDieID = strTempDieID;
        strCommand = strTempCommand;
        return true;
    }

    return false;
}

// Check if GEX logPAT command:
// <cmd> logPAT <html string>
bool Stdf_DTR_V4::IsGexCommand_logPAT(QString & strHtmlString)
{
    // Check if GEX logPAT command
    if(m_cnTEXT_DAT.startsWith("<cmd> logpat ", Qt::CaseInsensitive))
    {
        // Get HTML string
        strHtmlString = m_cnTEXT_DAT;
        strHtmlString.remove(0, 13);
        return true;
    }

    return false;
}

// Check if GEX PATTestList command:
// <cmd> PATTestList <tests list>
bool Stdf_DTR_V4::IsGexCommand_PATTestList(QString & strHtmlString)
{
    // Check if GEX PATTestList command
    if(m_cnTEXT_DAT.startsWith("<cmd> PATTestList ", Qt::CaseInsensitive))
    {
        // Get HTML string
        strHtmlString = m_cnTEXT_DAT;
        strHtmlString.remove(0, 18);
        return true;
    }

    return false;
}

// Return the JSON object contained in DTR.TEXT_DATA if it contains a valid
// GS JSON (no missing mandatory keys...)
QJsonObject Stdf_DTR_V4::GetGsJson(const QString type) const
{
    QJsonParseError lJsonErr;
    QJsonDocument   lJsonDoc;
    QJsonObject     lJsonObject;
    QJsonValue      lJsonValue;

    // Try to create JSON doc from DTR text
    lJsonDoc = QJsonDocument::fromJson(m_cnTEXT_DAT.toUtf8(), &lJsonErr);

    // Check if JsonDoc is valid and contains a Json object
    if(lJsonDoc.isNull() || lJsonDoc.isEmpty() || !lJsonDoc.isObject())
        return QJsonObject();

    // Check if "TYPE" key found
    lJsonObject = lJsonDoc.object();
    lJsonValue = lJsonObject.value("TYPE");
    if((lJsonValue == QJsonValue::Undefined) || (!lJsonValue.isString()))
        return QJsonObject();

    // Check if supported value for "TYPE" key
    // If so, eventually do some additional checks
    QString lDtrType = lJsonValue.toString().toLower();
    if(lDtrType!= "ml"
       && lDtrType != "md"
       && lDtrType != "reticle")
    {
        return QJsonObject();
    }

    if(type.toLower() == lDtrType)
    {
        return lJsonObject;
    }

    return QJsonObject();
}

bool Stdf_DTR_V4::IsSyntricityTestcond(QMap<QString, QString> &syntricityTestCond, bool &isOk) const
{
    // Datalog Text Record (DTR) into the STDF file. To write condition information for a set of tests,
    // write the condition information with the following syntax in the appropriate tester language statement:
    // COND: <cond name 1>=<cond value 1>, <cond name 2>=<cond value 2>.
    // The first word is the keyword COND:, followed by <cond name> and <cond value> pairs separated by commas. Note that white space is ignored.
    // For example, the following is a condition statement that specifies the digital, analog, and reference supply voltage for a set of tests:
    // COND: VDD=4.75V, VAA=4.75, VREF=1.234V

    // Other syntax guidelines:
    // Only one “COND:” maybe appear in a DTR and it must start at the first character in the DTR.
    // A carriage return “\r” should NEVER be in a COND: statement
    // A newline “\n” can only occur as the last character in a COND: statement

    isOk = true;
    bool    lIsSyntricity = false;
    QString lSyntricityKey = "COND:";
    syntricityTestCond.clear();
    QString lDTRString;
    QString lTestCond = "(.*)=(.*)";
    QString lClearRx = "CLEAR";
    QRegExp lSyntricityRx("^" + lSyntricityKey + "((" + lTestCond + ",?)+|" + lClearRx + ")$");
    // Clear spaces because by specs it has to be ignored
    lDTRString = m_cnTEXT_DAT.simplified();
    lDTRString.replace(" ","");

    // Check if starts with "COND:"
    /// TODO to be updated with Customer feedback on syntricity specs
    if (lDTRString.startsWith(lSyntricityKey))
    {
        lIsSyntricity = true;
        // Check if String matches syntricity mask
        if (!lSyntricityRx.exactMatch(lDTRString))
        {
            isOk = false;
            syntricityTestCond.insert(lDTRString, "");
        }
        // if so extract conditions
        else
        {
            QStringList lConditions = lDTRString.remove(lSyntricityKey, Qt::CaseSensitive).split(",");
            // If "CLEAR" condition -->
            if (!lConditions.isEmpty() && (lConditions.first().toUpper() == lClearRx))
            {
                syntricityTestCond.insert(lClearRx, "");
            }
            // Extract conditions
            else
            {
                while(!lConditions.isEmpty())
                {
                    if (!lConditions.first().isEmpty())
                    {
                        QStringList lCondValue = lConditions.first().split("=");
                        // if more than one "=" --> error
                        if (lCondValue.size() != 2)
                            isOk = false;
                        else
                        {
                            QString lCond = "testCondition[" + lCondValue.at(0) + "]";
                            QString lValue = lCondValue.at(1);
                            syntricityTestCond.insert(lCond, lValue);
                        }
                    }
                    // clear condition
                    lConditions.takeFirst();
                }
            }
        }
    }

    return lIsSyntricity;
}

void Stdf_DTR_V4::SetTEXT_DAT(const QJsonObject & JsonObject)
{
    QJsonDocument lJsonDoc(JsonObject);
    _FIELD_SET(m_cnTEXT_DAT=QString(lJsonDoc.toJson(QJsonDocument::Compact)),
               true, eposTEXT_DAT)
}

QString Stdf_DTR_V4::GetRecordShortName(void)
{
    return "DTR";
}

QString Stdf_DTR_V4::GetRecordLongName(void)
{
    return "Datalog Text Record";
}

int Stdf_DTR_V4::GetRecordType(void)
{
    return Rec_DTR;
}

bool Stdf_DTR_V4::Read(GS::StdLib::Stdf & clStdf)
{
    char	szString[STDF_MAX_U1+1];

    // First reset data
    Reset();

    // DTR.TEXT_DAT
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposTEXT_DAT);
    _FIELD_SET(m_cnTEXT_DAT = QString::fromLatin1(szString), true, eposTEXT_DAT);

    return true;
}

bool Stdf_DTR_V4::Write(GS::StdLib::Stdf & clStdf)
{
    // Check field ranges
    if(m_cnTEXT_DAT.size() > STDF_MAX_U1)
        return false;

    GS::StdLib::StdfRecordReadInfo RecordReadInfo;

    RecordReadInfo.iRecordType = STDF_DTR_TYPE;
    RecordReadInfo.iRecordSubType = STDF_DTR_STYPE;
    clStdf.WriteHeader(&RecordReadInfo);

    // DTR.TEXT_DAT
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnTEXT_DAT.toLatin1().constData()), eposTEXT_DAT, clStdf.WriteRecord());

    clStdf.WriteRecord();

    return true;
}

void Stdf_DTR_V4::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    // Empty string first
    strAsciiString = "";

    // DTR.TEXT_DAT
    _STR_ADDFIELD_ASCII(strAsciiString, "DTR.text_dat", m_cnTEXT_DAT, nFieldSelection, eposTEXT_DAT);
}

void Stdf_DTR_V4::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
    // Empty string list first
    listFields.empty();

    // DTR.TEXT_DAT
    _LIST_ADDFIELD_ASCII(listFields, "DTR.text_dat", m_cnTEXT_DAT, nFieldSelection, eposTEXT_DAT);
}

void Stdf_DTR_V4::GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection /*= 0*/)
{
    QString strTabs="";

    // Init XML string
    for(int nIndex=nIndentationLevel; nIndex>0; nIndex--)
        strTabs += "\t";
    strXmlString = strTabs;
    strXmlString += "<dtr>\n";

    // DTR.TEXT_DAT
    _CREATEFIELD_FROM_CN_XML(m_cnTEXT_DAT);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "text_dat", m_strFieldValue_macro, nFieldSelection, eposTEXT_DAT);

    strXmlString += strTabs;
    strXmlString += "</dtr>\n";
}

void Stdf_DTR_V4::GetAtdfString(QString & strAtdfString)
{
    // Reset field index, and line Nb
    m_uiFieldIndex = 0;
    m_uiLineNb = 1;

    // Init string
    strAtdfString = "DTR:";

    // DTR.TEXT_DAT
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnTEXT_DAT, eposTEXT_DAT);

    _STR_FINISHFIELD_ATDF(strAtdfString);
}

///////////////////////////////////////////////////////////
// GDR RECORD
///////////////////////////////////////////////////////////
Stdf_GDR_V4::Stdf_GDR_V4() : Stdf_Record()
{
    m_vnGEN_DATA = NULL;
    Reset();
}

Stdf_GDR_V4::~Stdf_GDR_V4()
{
    Reset();
}

void Stdf_GDR_V4::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Select fields for reduced list

    // Reset Data
    m_u2FLD_CNT		= 0;			// GDR.FLD_CNT

    if(m_vnGEN_DATA != NULL)
    {
        delete [] m_vnGEN_DATA;		// GDR.GEN_DATA
        m_vnGEN_DATA = NULL;
    }

    // Call Reset base method
    Stdf_Record::Reset();
}

bool Stdf_GDR_V4::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;

    return false;
}

QString Stdf_GDR_V4::GetRecordShortName(void)
{
    return "GDR";
}

QString Stdf_GDR_V4::GetRecordLongName(void)
{
    return "Generic Data Record";
}

int Stdf_GDR_V4::GetRecordType(void)
{
    return Rec_GDR;
}

bool Stdf_GDR_V4::Read(GS::StdLib::Stdf & clStdf)
{
    char			szString[STDF_MAX_U1+1];
    float			fData=0;
    double			lfData=0;
    long			dwData=0;
    int				wData=0;
    BYTE			bData=0;
    unsigned int	i=0;

    // First reset data
    Reset();

    // GDR.FLD_CNT
    _FIELD_CHECKREAD(clStdf.ReadWord(&wData), eposFLD_CNT);
    _FIELD_SET(m_u2FLD_CNT = stdf_type_u2(wData), true, eposFLD_CNT);

    // GDR.GEN_DATA
    if(m_u2FLD_CNT > 0)
        m_vnGEN_DATA = new stdf_type_vn[m_u2FLD_CNT];
    for(i=0; i<(unsigned int)m_u2FLD_CNT; i++)
    {
        _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposGEN_DATA);
        _FIELD_SET(m_vnGEN_DATA[i].uiDataTypeCode = (unsigned int)bData, true, eposGEN_DATA);
        switch(bData)
        {
        case eTypePad:
            break;
        case eTypeU1:
            _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposGEN_DATA);
            m_vnGEN_DATA[i].m_u1Data = stdf_type_u1(bData);
            break;
        case eTypeU2:
            _FIELD_CHECKREAD(clStdf.ReadWord(&wData), eposGEN_DATA);
            m_vnGEN_DATA[i].m_u2Data = stdf_type_u2(wData);
            break;
        case eTypeU4:
            _FIELD_CHECKREAD(clStdf.ReadDword(&dwData), eposGEN_DATA);
            m_vnGEN_DATA[i].m_u4Data = stdf_type_u4(dwData);
            break;
        case eTypeI1:
            _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposGEN_DATA);
            m_vnGEN_DATA[i].m_i1Data = stdf_type_i1(bData);
            break;
        case eTypeI2:
            _FIELD_CHECKREAD(clStdf.ReadWord(&wData), eposGEN_DATA);
            m_vnGEN_DATA[i].m_i2Data = stdf_type_i2(wData);
            break;
        case eTypeI4:
            _FIELD_CHECKREAD(clStdf.ReadDword(&dwData), eposGEN_DATA);
            m_vnGEN_DATA[i].m_i4Data = stdf_type_i4(dwData);
            break;
        case eTypeR4:
            _FIELD_CHECKREAD(clStdf.ReadFloat(&fData), eposGEN_DATA);
            m_vnGEN_DATA[i].m_r4Data = stdf_type_r4(fData);
            break;
        case eTypeR8:
            _FIELD_CHECKREAD(clStdf.ReadDouble(&lfData), eposGEN_DATA);
            m_vnGEN_DATA[i].m_r8Data = stdf_type_r8(lfData);
            break;
        case eTypeCN:
            _FIELD_CHECKREAD(clStdf.ReadString(szString), eposGEN_DATA);
            m_vnGEN_DATA[i].m_cnData = szString;
            break;
        case eTypeBN:
            _FIELD_CHECKREAD(clStdf.ReadBitField(&(m_vnGEN_DATA[i].m_bnData.m_bLength), m_vnGEN_DATA[i].m_bnData.m_pBitField), eposGEN_DATA);
            break;
        case eTypeDN:
            _FIELD_CHECKREAD(clStdf.ReadDBitField(&(m_vnGEN_DATA[i].m_dnData.m_uiLength), m_vnGEN_DATA[i].m_dnData.m_pBitField), eposGEN_DATA);
            break;
        case eTypeN1:
            _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposGEN_DATA);
            m_vnGEN_DATA[i].m_n1Data = stdf_type_n1(bData);
            break;
        default:
            break;
        }
    }

    return true;
}

bool Stdf_GDR_V4::Write(GS::StdLib::Stdf & clStdf)
{
    GS::StdLib::StdfRecordReadInfo	RecordReadInfo;

    RecordReadInfo.iRecordType = STDF_GDR_TYPE;
    RecordReadInfo.iRecordSubType = STDF_GDR_STYPE;;
    clStdf.WriteHeader(&RecordReadInfo);

    _FIELD_CHECKWRITE(clStdf.WriteWord(m_u2FLD_CNT), eposFLD_CNT, clStdf.WriteRecord());

    for(stdf_type_u2 loop=0; loop<m_u2FLD_CNT; loop++)
    {
        stdf_type_vn pvData = m_vnGEN_DATA[loop];
        _FIELD_CHECKWRITE(clStdf.WriteByte(pvData.uiDataTypeCode), eposGEN_DATA, clStdf.WriteRecord());
        switch(pvData.uiDataTypeCode)
        {
        case eTypePad:
            break;
        case eTypeU1:
            _FIELD_CHECKWRITE(clStdf.WriteByte(pvData.m_u1Data), eposGEN_DATA, clStdf.WriteRecord());
            break;
        case eTypeU2:
            _FIELD_CHECKWRITE(clStdf.WriteWord(pvData.m_u2Data), eposGEN_DATA, clStdf.WriteRecord());
            break;
        case eTypeU4:
            _FIELD_CHECKWRITE(clStdf.WriteDword((DWORD)pvData.m_u4Data), eposGEN_DATA, clStdf.WriteRecord());
            break;
        case eTypeI1:
            _FIELD_CHECKWRITE(clStdf.WriteByte(pvData.m_i1Data), eposGEN_DATA, clStdf.WriteRecord());
            break;
        case eTypeI2:
            _FIELD_CHECKWRITE(clStdf.WriteWord(pvData.m_i2Data), eposGEN_DATA, clStdf.WriteRecord());
            break;
        case eTypeI4:
            _FIELD_CHECKWRITE(clStdf.WriteDword((DWORD)pvData.m_i4Data), eposGEN_DATA, clStdf.WriteRecord());
            break;
        case eTypeR4:
            _FIELD_CHECKWRITE(clStdf.WriteFloat(pvData.m_r4Data), eposGEN_DATA, clStdf.WriteRecord());
            break;
        case eTypeR8:
            _FIELD_CHECKWRITE(clStdf.WriteDouble(pvData.m_r8Data), eposGEN_DATA, clStdf.WriteRecord());
            break;
        case eTypeN1:
            _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE((pvData.m_n1Data & 0x0f))), eposGEN_DATA, clStdf.WriteRecord());
            break;
        case eTypeCN:
        {
            if(m_pFieldFlags[eposGEN_DATA] & FieldFlag_Present)
            {
                if(clStdf.WriteString(pvData.m_cnData.toLatin1().constData()) != GS::StdLib::Stdf::NoError)
                    return false;
            }
            else
            {
                if(eposGEN_DATA < eposFIRST_OPTIONAL)
                    return false;
                clStdf.WriteRecord();
                return true;
            }
            break;
        }
        case eTypeBN:
            _FIELD_CHECKWRITE(clStdf.WriteBitField(BYTE(pvData.m_bnData.m_bLength), pvData.m_bnData.m_pBitField), eposGEN_DATA, clStdf.WriteRecord());
            break;
        case eTypeDN:
        {
            _FIELD_CHECKWRITE(clStdf.WriteDBitField(BYTE(pvData.m_dnData.m_uiLength), pvData.m_dnData.m_pBitField), eposGEN_DATA, clStdf.WriteRecord());
            break;
        }
        default:
            return false;
        }
    }

    clStdf.WriteRecord();


    return true;
}

void Stdf_GDR_V4::SetGEN_DATA(const std::vector<stdf_type_vn> vnGEN_DATA)
{
    stdf_type_u2 lFieldCount = vnGEN_DATA.size();

    SetFLD_CNT(lFieldCount);
    m_vnGEN_DATA = new stdf_type_vn[lFieldCount];

    std::vector<stdf_type_vn>::const_iterator itBegin   = vnGEN_DATA.begin();
    std::vector<stdf_type_vn>::const_iterator itEnd     = vnGEN_DATA.end();
    stdf_type_u2 lFieldIndex = 0;

    while (itBegin != itEnd)
    {
        _FIELD_SET(m_vnGEN_DATA[lFieldIndex++] = (*itBegin), true, eposGEN_DATA)
        ++itBegin;
    }

}

void Stdf_GDR_V4::SetGEN_DATA(stdf_type_vn*	vnGEN_DATA)
{
    m_vnGEN_DATA = new stdf_type_vn[m_u2FLD_CNT];
    for (stdf_type_u2 i=0; i<m_u2FLD_CNT; ++i)
    {
        _FIELD_SET(m_vnGEN_DATA[i] = vnGEN_DATA[i], true, eposGEN_DATA)
    }
}

void Stdf_GDR_V4::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    unsigned int	i;

    // Empty string first
    strAsciiString = "";

    // GDR.FLD_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "GDR.fld_cnt", QString::number(m_u2FLD_CNT), nFieldSelection, eposFLD_CNT);

    // GDR.GEN_DATA
    for(i=0; i<(unsigned int)m_u2FLD_CNT; i++)
    {
        switch(m_vnGEN_DATA[i].uiDataTypeCode)
        {
            case eTypePad:
                break;
            case eTypeU1:
                _STR_ADDFIELD_ASCII(strAsciiString, "GDR.gen_data", QString::number(m_vnGEN_DATA[i].m_u1Data), nFieldSelection, eposGEN_DATA);
                break;
            case eTypeU2:
                _STR_ADDFIELD_ASCII(strAsciiString, "GDR.gen_data", QString::number(m_vnGEN_DATA[i].m_u2Data), nFieldSelection, eposGEN_DATA);
                break;
            case eTypeU4:
                _STR_ADDFIELD_ASCII(strAsciiString, "GDR.gen_data", QString::number(m_vnGEN_DATA[i].m_u4Data), nFieldSelection, eposGEN_DATA);
                break;
            case eTypeI1:
                _STR_ADDFIELD_ASCII(strAsciiString, "GDR.gen_data", QString::number(m_vnGEN_DATA[i].m_i1Data), nFieldSelection, eposGEN_DATA);
                break;
            case eTypeI2:
                _STR_ADDFIELD_ASCII(strAsciiString, "GDR.gen_data", QString::number(m_vnGEN_DATA[i].m_i2Data), nFieldSelection, eposGEN_DATA);
                break;
            case eTypeI4:
                _STR_ADDFIELD_ASCII(strAsciiString, "GDR.gen_data", QString::number(m_vnGEN_DATA[i].m_i4Data), nFieldSelection, eposGEN_DATA);
                break;
            case eTypeR4:
                _STR_ADDFIELD_ASCII(strAsciiString, "GDR.gen_data", QString::number(m_vnGEN_DATA[i].m_r4Data), nFieldSelection, eposGEN_DATA);
                break;
            case eTypeR8:
                if(m_stRecordInfo.iCpuType == 0)	// CPU Type = VAX-PDP
                    _STR_ADDFIELD_ASCII(strAsciiString, "GDR.gen_data", "R8 type not supported for STDF files generated on VAX/PDP CPU's", nFieldSelection, eposGEN_DATA)
                else
                    _STR_ADDFIELD_ASCII(strAsciiString, "GDR.gen_data", QString::number(m_vnGEN_DATA[i].m_r8Data), nFieldSelection, eposGEN_DATA)
                break;
            case eTypeCN:
                _STR_ADDFIELD_ASCII(strAsciiString, "GDR.gen_data", m_vnGEN_DATA[i].m_cnData, nFieldSelection, eposGEN_DATA);
                break;
            case eTypeBN:
                _CREATEFIELD_FROM_BN_ASCII(m_vnGEN_DATA[i].m_bnData);
                _STR_ADDFIELD_ASCII(strAsciiString, "GDR.gen_data", m_strFieldValue_macro, nFieldSelection, eposGEN_DATA);
                break;
            case eTypeDN:
                _CREATEFIELD_FROM_DN_ASCII(m_vnGEN_DATA[i].m_dnData);
                _STR_ADDFIELD_ASCII(strAsciiString, "GDR.gen_data", m_strFieldValue_macro, nFieldSelection, eposGEN_DATA);
                break;
            case eTypeN1:
                _CREATEFIELD_FROM_N1_ASCII(m_vnGEN_DATA[i].m_n1Data);
                _STR_ADDFIELD_ASCII(strAsciiString, "GDR.gen_data", m_strFieldValue_macro, nFieldSelection, eposGEN_DATA);
                break;
            default:
                break;
        }
    }
}

void Stdf_GDR_V4::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
    unsigned int	i;

    // Empty string list first
    listFields.empty();

    // GDR.FLD_CNT
    _LIST_ADDFIELD_ASCII(listFields, "GDR.fld_cnt", QString::number(m_u2FLD_CNT), nFieldSelection, eposFLD_CNT);

    // GDR.GEN_DATA
    for(i=0; i<(unsigned int)m_u2FLD_CNT; i++)
    {
        switch(m_vnGEN_DATA[i].uiDataTypeCode)
        {
            case eTypePad:
                break;
            case eTypeU1:
                _LIST_ADDFIELD_ASCII(listFields, "GDR.gen_data", QString::number(m_vnGEN_DATA[i].m_u1Data), nFieldSelection, eposGEN_DATA);
                break;
            case eTypeU2:
                _LIST_ADDFIELD_ASCII(listFields, "GDR.gen_data", QString::number(m_vnGEN_DATA[i].m_u2Data), nFieldSelection, eposGEN_DATA);
                break;
            case eTypeU4:
                _LIST_ADDFIELD_ASCII(listFields, "GDR.gen_data", QString::number(m_vnGEN_DATA[i].m_u4Data), nFieldSelection, eposGEN_DATA);
                break;
            case eTypeI1:
                _LIST_ADDFIELD_ASCII(listFields, "GDR.gen_data", QString::number(m_vnGEN_DATA[i].m_i1Data), nFieldSelection, eposGEN_DATA);
                break;
            case eTypeI2:
                _LIST_ADDFIELD_ASCII(listFields, "GDR.gen_data", QString::number(m_vnGEN_DATA[i].m_i2Data), nFieldSelection, eposGEN_DATA);
                break;
            case eTypeI4:
                _LIST_ADDFIELD_ASCII(listFields, "GDR.gen_data", QString::number(m_vnGEN_DATA[i].m_i4Data), nFieldSelection, eposGEN_DATA);
                break;
            case eTypeR4:
                _LIST_ADDFIELD_ASCII(listFields, "GDR.gen_data", QString::number(m_vnGEN_DATA[i].m_r4Data), nFieldSelection, eposGEN_DATA);
                break;
            case eTypeR8:
                if(m_stRecordInfo.iCpuType == 0)	// CPU Type = VAX-PDP
                    _LIST_ADDFIELD_ASCII(listFields, "GDR.gen_data", "R8 type not supported for STDF files generated on VAX/PDP CPU's", nFieldSelection, eposGEN_DATA)
                else
                    _LIST_ADDFIELD_ASCII(listFields, "GDR.gen_data", QString::number(m_vnGEN_DATA[i].m_r8Data), nFieldSelection, eposGEN_DATA)
                break;
            case eTypeCN:
                _LIST_ADDFIELD_ASCII(listFields, "GDR.gen_data", m_vnGEN_DATA[i].m_cnData, nFieldSelection, eposGEN_DATA);
                break;
            case eTypeBN:
                _CREATEFIELD_FROM_BN_ASCII(m_vnGEN_DATA[i].m_bnData);
                _LIST_ADDFIELD_ASCII(listFields, "GDR.gen_data", m_strFieldValue_macro, nFieldSelection, eposGEN_DATA);
                break;
            case eTypeDN:
                _CREATEFIELD_FROM_DN_ASCII(m_vnGEN_DATA[i].m_dnData);
                _LIST_ADDFIELD_ASCII(listFields, "GDR.gen_data", m_strFieldValue_macro, nFieldSelection, eposGEN_DATA);
                break;
            case eTypeN1:
                _CREATEFIELD_FROM_N1_ASCII(m_vnGEN_DATA[i].m_n1Data);
                _LIST_ADDFIELD_ASCII(listFields, "GDR.gen_data", m_strFieldValue_macro, nFieldSelection, eposGEN_DATA);
                break;
            default:
                break;
        }
    }
}

void Stdf_GDR_V4::GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection /*= 0*/)
{
    QString			strTabs="";
    unsigned int	i;

    // Init XML string
    for(int nIndex=nIndentationLevel; nIndex>0; nIndex--)
        strTabs += "\t";
    strXmlString = strTabs;
    strXmlString += "<gdr>\n";

    // GDR.GEN_DATA
    for(i=0; i<(unsigned int)m_u2FLD_CNT; i++)
    {
        switch(m_vnGEN_DATA[i].uiDataTypeCode)
        {
            case eTypePad:
                break;
            case eTypeU1:
                _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "gen_data", QString::number(m_vnGEN_DATA[i].m_u1Data), nFieldSelection, eposGEN_DATA);
                break;
            case eTypeU2:
                _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "gen_data", QString::number(m_vnGEN_DATA[i].m_u2Data), nFieldSelection, eposGEN_DATA);
                break;
            case eTypeU4:
                _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "gen_data", QString::number(m_vnGEN_DATA[i].m_u4Data), nFieldSelection, eposGEN_DATA);
                break;
            case eTypeI1:
                _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "gen_data", QString::number(m_vnGEN_DATA[i].m_i1Data), nFieldSelection, eposGEN_DATA);
                break;
            case eTypeI2:
                _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "gen_data", QString::number(m_vnGEN_DATA[i].m_i2Data), nFieldSelection, eposGEN_DATA);
                break;
            case eTypeI4:
                _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "gen_data", QString::number(m_vnGEN_DATA[i].m_i4Data), nFieldSelection, eposGEN_DATA);
                break;
            case eTypeR4:
                _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "gen_data", QString::number(m_vnGEN_DATA[i].m_r4Data), nFieldSelection, eposGEN_DATA);
                break;
            case eTypeR8:
                _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "gen_data", QString::number(m_vnGEN_DATA[i].m_r8Data), nFieldSelection, eposGEN_DATA);
                break;
            case eTypeCN:
                _CREATEFIELD_FROM_CN_XML(m_vnGEN_DATA[i].m_cnData);
                _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "gen_data", m_strFieldValue_macro, nFieldSelection, eposGEN_DATA);
                break;
            case eTypeBN:
                _CREATEFIELD_FROM_BN_XML(m_vnGEN_DATA[i].m_bnData);
                _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "gen_data", m_strFieldValue_macro, nFieldSelection, eposGEN_DATA);
                break;
            case eTypeDN:
                _CREATEFIELD_FROM_DN_XML(m_vnGEN_DATA[i].m_dnData);
                _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "gen_data", m_strFieldValue_macro, nFieldSelection, eposGEN_DATA);
                break;
            case eTypeN1:
                _CREATEFIELD_FROM_N1_XML(m_vnGEN_DATA[i].m_n1Data);
                _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "gen_data", m_strFieldValue_macro, nFieldSelection, eposGEN_DATA);
                break;
            default:
                break;
        }
    }

    strXmlString += strTabs;
    strXmlString += "</gdr>\n";
}

void Stdf_GDR_V4::GetAtdfString(QString & strAtdfString)
{
    unsigned int	i=0;

    // Reset field index, and line Nb
    m_uiFieldIndex = 0;
    m_uiLineNb = 1;

    // Init string
    strAtdfString = "GDR:";

    // GDR.GEN_DATA
    for(i=0; i<(unsigned int)m_u2FLD_CNT; i++)
    {
        switch(m_vnGEN_DATA[i].uiDataTypeCode)
        {
        case eTypePad:
            m_strFieldValue_macro.clear();
            break;
        case eTypeU1:
            m_strFieldValue_macro = "U" + QString::number(m_vnGEN_DATA[i].m_u1Data);
            break;
        case eTypeU2:
            m_strFieldValue_macro = "M" + QString::number(m_vnGEN_DATA[i].m_u2Data);
            break;
        case eTypeU4:
            m_strFieldValue_macro = "B" + QString::number(m_vnGEN_DATA[i].m_u4Data);
            break;
        case eTypeI1:
            m_strFieldValue_macro = "I" + QString::number(m_vnGEN_DATA[i].m_i1Data);
            break;
        case eTypeI2:
            m_strFieldValue_macro = "S" + QString::number(m_vnGEN_DATA[i].m_i2Data);
            break;
        case eTypeI4:
            m_strFieldValue_macro = "L" + QString::number(m_vnGEN_DATA[i].m_i4Data);
            break;
        case eTypeR4:
            m_strFieldValue_macro = "F" + QString::number(m_vnGEN_DATA[i].m_r4Data);
            break;
        case eTypeR8:
            if(m_stRecordInfo.iCpuType == 0)	// CPU Type = VAX-PDP
                m_strFieldValue_macro = QString("R8 type not supported for STDF files generated on VAX/PDP CPU's");
            else
                m_strFieldValue_macro = "D" + QString::number(m_vnGEN_DATA[i].m_r8Data);
            break;
        case eTypeCN:
            m_strFieldValue_macro = "T" + m_vnGEN_DATA[i].m_cnData;
            break;
        case eTypeBN:
            _CREATEFIELD_FROM_BN_ATDF(m_vnGEN_DATA[i].m_bnData);
            m_strFieldValue_macro = "X" + m_strFieldValue_macro;
            break;
        case eTypeDN:
            _CREATEFIELD_FROM_DN_ATDF(m_vnGEN_DATA[i].m_dnData);
            m_strFieldValue_macro = "Y" + m_strFieldValue_macro;
            break;
        case eTypeN1:
            _CREATEFIELD_FROM_N1_ATDF(m_vnGEN_DATA[i].m_n1Data);
            m_strFieldValue_macro = "N" + m_strFieldValue_macro;
            break;
        default:
            m_strFieldValue_macro.clear();
            break;
        }
        _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposGEN_DATA);
    }

    _STR_FINISHFIELD_ATDF(strAtdfString);
}

///////////////////////////////////////////////////////////
// VUR RECORD
///////////////////////////////////////////////////////////
Stdf_VUR_V4::Stdf_VUR_V4() : Stdf_Record()
{
    Reset();
}

Stdf_VUR_V4::~Stdf_VUR_V4()
{
    Reset();
}

void Stdf_VUR_V4::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    m_cnUPD_NAM		= "";		// VUR.UPD_NAM

    // Call Reset base method
    Stdf_Record::Reset();
}

bool Stdf_VUR_V4::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;

    return false;
}

QString Stdf_VUR_V4::GetRecordShortName(void)
{
    return "VUR";
}

QString Stdf_VUR_V4::GetRecordLongName(void)
{
    return "Version Update Record";
}

int Stdf_VUR_V4::GetRecordType(void)
{
    return Rec_VUR;
}

bool Stdf_VUR_V4::Read(GS::StdLib::Stdf & clStdf)
{
    char	szString[STDF_MAX_U1+1];
    // First reset data
    Reset();

    // VUR.UPD_NAM
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposUPD_NAM);
    _FIELD_SET(m_cnUPD_NAM= szString, !m_cnUPD_NAM.isEmpty(), eposUPD_NAM);

    return true;
}

bool Stdf_VUR_V4::Write(GS::StdLib::Stdf & clStdf)
{
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;

    RecordReadInfo.iRecordType = STDF_VUR_TYPE;
    RecordReadInfo.iRecordSubType = STDF_VUR_STYPE;
    clStdf.WriteHeader(&RecordReadInfo);

    // VUR.EXC_DESC
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnUPD_NAM.toLatin1().constData()), eposUPD_NAM, clStdf.WriteRecord());

    clStdf.WriteRecord();

    return true;
}

void Stdf_VUR_V4::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    // Empty string first
    strAsciiString = "";
    // VUR.UPD_NAM
    _STR_ADDFIELD_ASCII(strAsciiString, "VUR.upd_nam", m_cnUPD_NAM, nFieldSelection, eposUPD_NAM);
}

void Stdf_VUR_V4::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{

    // Empty string list first
    listFields.empty();

    // VUR.UPD_NAM
    _LIST_ADDFIELD_ASCII(listFields, "VUR.upd_nam", m_cnUPD_NAM, nFieldSelection, eposUPD_NAM);
}

void Stdf_VUR_V4::GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection /*= 0*/)
{
    QString strTabs="";

    // Init XML string
    for(int nIndex=nIndentationLevel; nIndex>0; nIndex--)
        strTabs += "\t";
    strXmlString = strTabs;
    strXmlString += "<VUR>\n";
    // VUR.UPD_NAM
    _CREATEFIELD_FROM_CN_XML(m_cnUPD_NAM);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "upd_nam", m_strFieldValue_macro, nFieldSelection, eposUPD_NAM);

    strXmlString += strTabs;
    strXmlString += "</VUR>\n";
}

void Stdf_VUR_V4::GetAtdfString(QString & strAtdfString)
{
    // Reset field index, and line Nb
    m_uiFieldIndex = 0;
    m_uiLineNb = 1;

    // Init string
    strAtdfString = "VUR:";

    // VUR.EXC_DESC
    _STR_ADDFIELD_ATDF(strAtdfString, m_cnUPD_NAM, eposUPD_NAM);

    _STR_FINISHFIELD_ATDF(strAtdfString);
}


///////////////////////////////////////////////////////////
// PSR RECORD
///////////////////////////////////////////////////////////
Stdf_PSR_V4::Stdf_PSR_V4() : Stdf_Record()
{
    Reset();
}

Stdf_PSR_V4::~Stdf_PSR_V4()
{
    Reset();
}

void Stdf_PSR_V4::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    m_b1CONT_FLG = 0;
    m_u2PSR_INDX = 0;
    m_cnPSR_NAM = "";
    m_b1OPT_FLG = 0;
    m_u2TOTP_CNT = 0;
    m_u2LOCP_CNT = 0;
    m_pu8PAT_BGN = 0;
    m_pu8PAT_END = 0;
    m_pcnPAT_FILE = 0;
    m_pcnPAT_LBL = 0;
    m_pcnFILE_UID = 0;
    m_pcnATPG_DSC = 0;
    m_pcnSRC_ID = 0;

    // Call Reset base method
    Stdf_Record::Reset();
}

bool Stdf_PSR_V4::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;

    return false;
}

QString Stdf_PSR_V4::GetRecordShortName(void)
{
    return "PSR";
}

QString Stdf_PSR_V4::GetRecordLongName(void)
{
    return "Pattern Sequence Record";
}

int Stdf_PSR_V4::GetRecordType(void)
{
    return Rec_PSR;
}

bool Stdf_PSR_V4::Read(GS::StdLib::Stdf & clStdf)
{
    char	szString[STDF_MAX_U1+1];
    stdf_type_u8 qwData;
    stdf_type_u1	bData;
    int wData;
    unsigned int i;
    // First reset data Stdf_PLR_V4
    Reset();

    _FIELD_CHECKREAD(	clStdf.ReadByte(&bData),	eposCONT_FLG	);
    _FIELD_SET(	m_b1CONT_FLG	=	stdf_type_b1	(  bData )	,true,	eposCONT_FLG	);
    _FIELD_CHECKREAD(	clStdf.ReadWord(&wData)	,	eposPSR_INDX	);
    _FIELD_SET(	m_u2PSR_INDX	=	stdf_type_u2	(  wData )	,true,	eposPSR_INDX	);
    _FIELD_CHECKREAD(	clStdf.ReadString(szString)	,	eposPSR_NAM	);
    _FIELD_SET(	m_cnPSR_NAM	=	stdf_type_cn	(  szString )	,true,	eposPSR_NAM	);
    _FIELD_CHECKREAD(	clStdf.ReadByte(&bData)	,	eposOPT_FLG	);
    _FIELD_SET(	m_b1OPT_FLG	=	stdf_type_b1	( bData )	,true,	eposOPT_FLG	);
    _FIELD_CHECKREAD(	clStdf.ReadWord(&wData)	,	eposTOTP_CNT	);
    _FIELD_SET(	m_u2TOTP_CNT	=	stdf_type_u2	( wData  )	,true,	eposTOTP_CNT	);
    _FIELD_CHECKREAD(	clStdf.ReadWord(&wData)	,	eposLOCP_CNT	);
    _FIELD_SET(	m_u2LOCP_CNT	=	stdf_type_u2	( wData  )	,true,	eposLOCP_CNT	);

    if(m_u2LOCP_CNT > 0)
        m_pu8PAT_BGN = new stdf_type_u8[m_u2LOCP_CNT];
    for(i=0; i<(unsigned int)m_u2LOCP_CNT; i++)
    {
        _FIELD_CHECKREAD(clStdf.ReadQword(&qwData), eposPAT_BGN);
        _FIELD_SET(m_pu8PAT_BGN[i] = qwData, true, eposPAT_BGN);
    }

    if(m_u2LOCP_CNT > 0)
        m_pu8PAT_END = new stdf_type_u8[m_u2LOCP_CNT];
    for(i=0; i<(unsigned int)m_u2LOCP_CNT; i++)
    {
        _FIELD_CHECKREAD(clStdf.ReadQword(&qwData), eposPAT_END);
        _FIELD_SET(m_pu8PAT_END[i] = qwData, true, eposPAT_END);
    }

    if(m_u2LOCP_CNT > 0)
        m_pcnPAT_FILE = new stdf_type_cn[m_u2LOCP_CNT];
    for(i=0; i<(unsigned int)m_u2LOCP_CNT; i++)
    {
        _FIELD_CHECKREAD(clStdf.ReadString(szString), eposPAT_FILE);
        _FIELD_SET(m_pcnPAT_FILE[i] = szString, true, eposPAT_FILE);
    }

    if((m_b1OPT_FLG & STDF_MASK_BIT0) == 0 && m_u2LOCP_CNT > 0)
    {
        m_pcnPAT_LBL = new stdf_type_cn[m_u2LOCP_CNT];
        for(i=0; i<(unsigned int)m_u2LOCP_CNT; i++)
        {
            _FIELD_CHECKREAD(clStdf.ReadString(szString), eposPAT_LBL);
            _FIELD_SET(m_pcnPAT_LBL[i] = szString, true, eposPAT_LBL);
        }
    }

    if((m_b1OPT_FLG & STDF_MASK_BIT1) == 0 && m_u2LOCP_CNT > 0)
    {
        m_pcnFILE_UID = new stdf_type_cn[m_u2LOCP_CNT];
        for(i=0; i<(unsigned int)m_u2LOCP_CNT; i++)
        {
            _FIELD_CHECKREAD(clStdf.ReadString(szString), eposFILE_UID);
            _FIELD_SET(m_pcnFILE_UID[i] = szString, true, eposFILE_UID);
        }
    }

    if((m_b1OPT_FLG & STDF_MASK_BIT2) == 0 && m_u2LOCP_CNT > 0)
    {
        m_pcnATPG_DSC = new stdf_type_cn[m_u2LOCP_CNT];
        for(i=0; i<(unsigned int)m_u2LOCP_CNT; i++)
        {
            _FIELD_CHECKREAD(clStdf.ReadString(szString), eposATPG_DSC);
            _FIELD_SET(m_pcnATPG_DSC[i] = szString, true, eposATPG_DSC);
        }
    }

    if((m_b1OPT_FLG & STDF_MASK_BIT3) == 0 && m_u2LOCP_CNT > 0)
    {
        m_pcnSRC_ID = new stdf_type_cn[m_u2LOCP_CNT];
        for(i=0; i<(unsigned int)m_u2LOCP_CNT; i++)
        {
            _FIELD_CHECKREAD(clStdf.ReadString(szString), eposSRC_ID);
            _FIELD_SET(m_pcnSRC_ID[i] = szString, true, eposSRC_ID);
        }
    }
    return true;
}

bool Stdf_PSR_V4::Write(GS::StdLib::Stdf & clStdf)
{
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;

    RecordReadInfo.iRecordType = STDF_PSR_TYPE;
    RecordReadInfo.iRecordSubType = STDF_PSR_STYPE;
    clStdf.WriteHeader(&RecordReadInfo);

    _FIELD_CHECKWRITE(clStdf.WriteByte(m_b1CONT_FLG),	eposCONT_FLG	,clStdf.WriteRecord());
    _FIELD_CHECKWRITE(clStdf.WriteWord(m_u2PSR_INDX),	eposPSR_INDX	,clStdf.WriteRecord());
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnPSR_NAM.toLatin1().constData()),	eposPSR_NAM	,clStdf.WriteRecord());
    _FIELD_CHECKWRITE(clStdf.WriteByte(m_b1OPT_FLG),    eposOPT_FLG	,clStdf.WriteRecord());
    _FIELD_CHECKWRITE(clStdf.WriteWord(m_u2TOTP_CNT),	eposTOTP_CNT	,clStdf.WriteRecord());
    _FIELD_CHECKWRITE(clStdf.WriteWord(m_u2LOCP_CNT),	eposLOCP_CNT	,clStdf.WriteRecord());

    _FIELD_CHECKWRITE_KUi(clStdf.WriteQWord, m_pu8PAT_BGN, m_u2LOCP_CNT, eposPAT_BGN, clStdf.WriteRecord());
    _FIELD_CHECKWRITE_KUi(clStdf.WriteQWord, m_pu8PAT_END, m_u2LOCP_CNT, eposPAT_END, clStdf.WriteRecord());

    // GCORE-9115: Do not return false if they are empty.
    WritePSR_KCN(clStdf, m_pcnPAT_FILE, m_u2LOCP_CNT, eposPAT_FILE);
    WritePSR_KCN(clStdf, m_pcnPAT_LBL, m_u2LOCP_CNT, eposPAT_LBL);
    WritePSR_KCN(clStdf, m_pcnFILE_UID, m_u2LOCP_CNT, eposFILE_UID);
    WritePSR_KCN(clStdf, m_pcnATPG_DSC, m_u2LOCP_CNT, eposATPG_DSC);
    WritePSR_KCN(clStdf, m_pcnSRC_ID, m_u2LOCP_CNT, eposSRC_ID);

    clStdf.WriteRecord();

    return true;
}

bool Stdf_PSR_V4::WritePSR_KCN(GS::StdLib::Stdf &stdfObject,
                               stdf_type_cn* array,
                               stdf_type_u2 size,
                               FieldPos pos)
{
    if(m_pFieldFlags[pos] & FieldFlag_Present || pos < mLastFieldAtdf)
    {
        int lRet;
        for (int lIdx = 0; lIdx < size; ++lIdx)
        {
            if (array == NULL || array[lIdx] == NULL)
            {
                lRet = stdfObject.WriteString("") != GS::StdLib::Stdf::NoError;
            }
            else
            {
                lRet = stdfObject.WriteString(array[lIdx].toLatin1().constData());
            }
            if (lRet != GS::StdLib::Stdf::NoError)
                return false;
        }
    }
    else
    {
        if(pos < eposFIRST_OPTIONAL)
            return false;
        stdfObject.WriteRecord();
    }
    return true;
}

void Stdf_PSR_V4::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    // Empty string first
    strAsciiString = "";
    // PSR.UPD_NAM
    _STR_ADDFIELD_ASCII(strAsciiString	,	"PSR.CONT_FLG",	QString::number(m_b1CONT_FLG)	,nFieldSelection, 	eposCONT_FLG	);
    _STR_ADDFIELD_ASCII(strAsciiString	,	"PSR.PSR_INDX",	QString::number(m_u2PSR_INDX)	,nFieldSelection, 	eposPSR_INDX	);
    _STR_ADDFIELD_ASCII(strAsciiString	,	"PSR.PSR_NAM",	m_cnPSR_NAM	,nFieldSelection, 	eposPSR_NAM	);
    _STR_ADDFIELD_ASCII(strAsciiString	,	"PSR.OPT_FLG",	QString::number(m_b1OPT_FLG)	,nFieldSelection, 	eposOPT_FLG	);
    _STR_ADDFIELD_ASCII(strAsciiString	,	"PSR.TOTP_CNT",	QString::number(m_u2TOTP_CNT)	,nFieldSelection, 	eposTOTP_CNT	);
    _STR_ADDFIELD_ASCII(strAsciiString	,	"LOCP_CNT",	QString::number(m_u2LOCP_CNT)	,nFieldSelection, 	eposLOCP_CNT	);

    _CREATEFIELD_FROM_KU8_ASCII(m_u2LOCP_CNT, m_pu8PAT_BGN, eposPAT_BGN);
    _STR_ADDFIELD_ASCII(strAsciiString	,	"PSR.PAT_BGN",	m_strFieldValue_macro	,nFieldSelection, 	eposPAT_BGN	);

    _CREATEFIELD_FROM_KU8_ASCII(m_u2LOCP_CNT, m_pu8PAT_END, eposPAT_END);
    _STR_ADDFIELD_ASCII(strAsciiString	,	"PSR.PAT_END",	m_strFieldValue_macro	,nFieldSelection, 	eposPAT_END	);

    _CREATEFIELD_FROM_KCN_ASCII(m_u2LOCP_CNT, m_pcnPAT_FILE, eposPAT_FILE);
    _STR_ADDFIELD_ASCII(strAsciiString	,	"PSR.PAT_FILE",	m_strFieldValue_macro	,nFieldSelection, 	eposPAT_FILE	);

    _CREATEFIELD_FROM_KCN_ASCII(m_u2LOCP_CNT, m_pcnPAT_LBL, eposPAT_LBL);
    _STR_ADDFIELD_ASCII(strAsciiString	,	"PSR.PAT_LBL",	m_strFieldValue_macro	,nFieldSelection, 	eposPAT_LBL	);

    _CREATEFIELD_FROM_KCN_ASCII(m_u2LOCP_CNT, m_pcnFILE_UID, eposFILE_UID);
    _STR_ADDFIELD_ASCII(strAsciiString	,	"PSR.FILE_UID",	m_strFieldValue_macro	,nFieldSelection, 	eposFILE_UID	);

    _CREATEFIELD_FROM_KCN_ASCII(m_u2LOCP_CNT, m_pcnATPG_DSC, eposATPG_DSC);
    _STR_ADDFIELD_ASCII(strAsciiString	,	"PSR.ATPG_DSC",	m_strFieldValue_macro	,nFieldSelection, 	eposATPG_DSC	);

    _CREATEFIELD_FROM_KCN_ASCII(m_u2LOCP_CNT, m_pcnSRC_ID, eposSRC_ID);
    _STR_ADDFIELD_ASCII(strAsciiString	,	"PSR.SRC_ID",	m_strFieldValue_macro	,nFieldSelection, 	eposSRC_ID	);

}

void Stdf_PSR_V4::GetAsciiFieldList(QStringList & listFields,int nFieldSelection  /* = 0*/)
{

    // Empty string list first
    listFields.empty();

    _LIST_ADDFIELD_ASCII(listFields	,	"PSR.CONT_FLG",	QString::number(m_b1CONT_FLG)	,nFieldSelection, 	eposCONT_FLG	);
    _LIST_ADDFIELD_ASCII(listFields	,	"PSR.PSR_INDX",	QString::number(m_u2PSR_INDX)	,nFieldSelection, 	eposPSR_INDX	);
    _LIST_ADDFIELD_ASCII(listFields	,	"PSR.PSR_NAM",	m_cnPSR_NAM	,nFieldSelection, 	eposPSR_NAM	);
    _LIST_ADDFIELD_ASCII(listFields	,	"PSR.OPT_FLG",	QString::number(m_b1OPT_FLG)	,nFieldSelection, 	eposOPT_FLG	);
    _LIST_ADDFIELD_ASCII(listFields	,	"PSR.TOTP_CNT",	QString::number(m_u2TOTP_CNT)	,nFieldSelection, 	eposTOTP_CNT	);
    _LIST_ADDFIELD_ASCII(listFields	,	"PSR.LOCP_CNT",	QString::number(m_u2LOCP_CNT)	,nFieldSelection, 	eposLOCP_CNT	);

    _CREATEFIELD_FROM_KU8_ASCII(m_u2LOCP_CNT, m_pu8PAT_BGN, eposPAT_BGN);
    _LIST_ADDFIELD_ASCII(listFields	,	"PSR.PAT_BGN",	m_strFieldValue_macro	,nFieldSelection, 	eposPAT_BGN	);

    _CREATEFIELD_FROM_KU8_ASCII(m_u2LOCP_CNT, m_pu8PAT_END, eposPAT_END);
    _LIST_ADDFIELD_ASCII(listFields	,	"PSR.PAT_END",	m_strFieldValue_macro	,nFieldSelection, 	eposPAT_END	);

    _CREATEFIELD_FROM_KCN_ASCII(m_u2LOCP_CNT, m_pcnPAT_FILE, eposPAT_FILE);
    _LIST_ADDFIELD_ASCII(listFields	,	"PSR.PAT_FILE",	m_strFieldValue_macro	,nFieldSelection, 	eposPAT_FILE	);

    _CREATEFIELD_FROM_KCN_ASCII(m_u2LOCP_CNT, m_pcnPAT_LBL, eposPAT_LBL);
    _LIST_ADDFIELD_ASCII(listFields	,	"PSR.PAT_LBL",	m_strFieldValue_macro	,nFieldSelection, 	eposPAT_LBL	);

    _CREATEFIELD_FROM_KCN_ASCII(m_u2LOCP_CNT, m_pcnFILE_UID, eposFILE_UID);
    _LIST_ADDFIELD_ASCII(listFields	,	"PSR.FILE_UID",	m_strFieldValue_macro	,nFieldSelection, 	eposFILE_UID	);

    _CREATEFIELD_FROM_KCN_ASCII(m_u2LOCP_CNT, m_pcnATPG_DSC, eposATPG_DSC);
    _LIST_ADDFIELD_ASCII(listFields	,	"PSR.ATPG_DSC",	m_strFieldValue_macro	,nFieldSelection, 	eposATPG_DSC	);

    _CREATEFIELD_FROM_KCN_ASCII(m_u2LOCP_CNT, m_pcnSRC_ID, eposSRC_ID);
    _LIST_ADDFIELD_ASCII(listFields	,	"PSR.SRC_ID",	m_strFieldValue_macro	,nFieldSelection, 	eposSRC_ID	);


}

void Stdf_PSR_V4::GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection /*= 0*/)
{

//    _CREATEFIELD_FROM_KUi_XML(m_u2GRP_CNT, m_ku1GRP_RADX, eposGRP_RADX);
//    _STR_ADDLIST_XML(strXmlString,m_u2GRP_CNT,nIndentationLevel+1,"grp_radx",m_strFieldValue_macro,nFieldSelection,eposGRP_RADX);

//    // PLR.PGM_CHAR
//    _CREATEFIELD_FROM_KCN_XML(m_u2GRP_CNT, m_kcnPGM_CHAR, eposPGM_CHAR);
//    _STR_ADDLIST_XML(strXmlString,m_u2GRP_CNT,nIndentationLevel+1,"pgm_char",m_strFieldValue_macro,nFieldSelection,eposPGM_CHAR);

    QString strTabs="";

    // Init XML string
    for(int nIndex=nIndentationLevel; nIndex>0; nIndex--)
        strTabs += "\t";
    strXmlString = strTabs;
    strXmlString += "<PSR>\n";

    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "rec_indx", QString::number(m_b1CONT_FLG), nFieldSelection, eposCONT_FLG);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "psr_indx", QString::number(m_u2PSR_INDX), nFieldSelection, eposPSR_INDX);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "psr_nam", m_cnPSR_NAM, nFieldSelection, eposPSR_NAM);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "opt_flg", QString::number(m_b1OPT_FLG), nFieldSelection, eposOPT_FLG);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "totp_cnt", QString::number(m_u2TOTP_CNT), nFieldSelection, eposTOTP_CNT);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "locp_cnt", QString::number(m_u2LOCP_CNT), nFieldSelection, eposLOCP_CNT);

    _CREATEFIELD_FROM_KU8_XML(m_u2LOCP_CNT, m_pu8PAT_BGN, eposPAT_BGN);
    _STR_ADDLIST_XML(strXmlString,m_u2LOCP_CNT, nIndentationLevel+1,"PAT_BGN",m_strFieldValue_macro,nFieldSelection,eposPAT_BGN	);

    _CREATEFIELD_FROM_KU8_XML(m_u2LOCP_CNT, m_pu8PAT_END, eposPAT_END);
    _STR_ADDLIST_XML(strXmlString,m_u2LOCP_CNT, nIndentationLevel+1	,"PAT_END",	m_strFieldValue_macro	,nFieldSelection,eposPAT_END );

    _CREATEFIELD_FROM_KCN_XML(m_u2LOCP_CNT, m_pcnPAT_FILE, eposPAT_FILE);
    _STR_ADDLIST_XML(strXmlString,m_u2LOCP_CNT, nIndentationLevel+1	,"PAT_FILE",	m_strFieldValue_macro	,nFieldSelection,eposPAT_FILE);

    _CREATEFIELD_FROM_KCN_XML(m_u2LOCP_CNT, m_pcnPAT_LBL, eposPAT_LBL);
    _STR_ADDLIST_XML(strXmlString,m_u2LOCP_CNT, nIndentationLevel+1	,"PAT_LBL",	m_strFieldValue_macro	,nFieldSelection,eposPAT_LBL);

    _CREATEFIELD_FROM_KCN_XML(m_u2LOCP_CNT, m_pcnFILE_UID, eposFILE_UID);
    _STR_ADDLIST_XML(strXmlString, m_u2LOCP_CNT,nIndentationLevel+1	,"FILE_UID",	m_strFieldValue_macro	,nFieldSelection,eposFILE_UID);

    _CREATEFIELD_FROM_KCN_XML(m_u2LOCP_CNT, m_pcnATPG_DSC, eposATPG_DSC);
    _STR_ADDLIST_XML(strXmlString, m_u2LOCP_CNT,nIndentationLevel+1	,"ATPG_DSC",	m_strFieldValue_macro	,nFieldSelection,eposATPG_DSC);

    _CREATEFIELD_FROM_KCN_XML(m_u2LOCP_CNT, m_pcnSRC_ID, eposSRC_ID);
    _STR_ADDLIST_XML(strXmlString, m_u2LOCP_CNT,nIndentationLevel+1	,"SRC_ID",	m_strFieldValue_macro	,nFieldSelection,eposSRC_ID);



    strXmlString += strTabs;

    strXmlString += "</PSR>\n";
}

void Stdf_PSR_V4::GetAtdfString(QString & strAtdfString)
{
    // Reset field index, and line Nb
    m_uiFieldIndex = 0;
    m_uiLineNb = 1;

    // Init string
    strAtdfString = "PSR:";
    _STR_ADDFIELD_ATDF(strAtdfString	,QString::number(m_b1CONT_FLG)	, 	eposCONT_FLG	);
    _STR_ADDFIELD_ATDF(strAtdfString	,QString::number(m_u2PSR_INDX)	, 	eposPSR_INDX	);
    _STR_ADDFIELD_ATDF(strAtdfString	,m_cnPSR_NAM	, 	eposPSR_NAM	);
    _STR_ADDFIELD_ATDF(strAtdfString	,QString::number(m_b1OPT_FLG)	, 	eposOPT_FLG	);
    _STR_ADDFIELD_ATDF(strAtdfString	,QString::number(m_u2TOTP_CNT)	, 	eposTOTP_CNT	);
    _STR_ADDFIELD_ATDF(strAtdfString	,QString::number(m_u2LOCP_CNT)	, 	eposLOCP_CNT	);

    m_strFieldValue_macro = "";
    _CREATEFIELD_FROM_KU8_ATDF(m_u2LOCP_CNT, m_pu8PAT_BGN, eposPAT_BGN);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposPAT_BGN);

    m_strFieldValue_macro = "";
    _CREATEFIELD_FROM_KU8_ATDF(m_u2LOCP_CNT, m_pu8PAT_END, eposPAT_END);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposPAT_END);

    m_strFieldValue_macro = "";
    _CREATEFIELD_FROM_KCN_ATDF(m_u2LOCP_CNT, m_pcnPAT_FILE, eposPAT_FILE);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposPAT_FILE);

    m_strFieldValue_macro = "";
    _CREATEFIELD_FROM_KCN_ATDF(m_u2LOCP_CNT, m_pcnPAT_LBL, eposPAT_LBL);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposPAT_LBL);

    m_strFieldValue_macro = "";
    _CREATEFIELD_FROM_KCN_ATDF(m_u2LOCP_CNT, m_pcnFILE_UID, eposFILE_UID);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposFILE_UID);

    m_strFieldValue_macro = "";
    _CREATEFIELD_FROM_KCN_ATDF(m_u2LOCP_CNT, m_pcnATPG_DSC, eposATPG_DSC);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposATPG_DSC);

    m_strFieldValue_macro = "";
    _CREATEFIELD_FROM_KCN_ATDF(m_u2LOCP_CNT, m_pcnSRC_ID, eposSRC_ID);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposSRC_ID);


    _STR_FINISHFIELD_ATDF(strAtdfString);
}

///////////////////////////////////////////////////////////
// NMR RECORD
///////////////////////////////////////////////////////////
Stdf_NMR_V4::Stdf_NMR_V4() : Stdf_Record()
{
    Reset();
}

Stdf_NMR_V4::~Stdf_NMR_V4()
{
    Reset();
}

void Stdf_NMR_V4::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    m_b1CONT_FLG = 0;
    m_u2NMR_INDX = 0;
    m_u2TOTM_CNT = 0;
    m_u2LOCM_CNT = 0;
    m_pu2PMR_INDX = 0;
    m_pcnATPG_NAM = 0;

    // Call Reset base method
    Stdf_Record::Reset();
}

bool Stdf_NMR_V4::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;

    return false;
}

QString Stdf_NMR_V4::GetRecordShortName(void)
{
    return "NMR";
}

QString Stdf_NMR_V4::GetRecordLongName(void)
{
    return "Name Map Record";
}

int Stdf_NMR_V4::GetRecordType(void)
{
    return Rec_NMR;
}

bool Stdf_NMR_V4::Read(GS::StdLib::Stdf & clStdf)
{
    char	szString[STDF_MAX_U1+1];
    stdf_type_u1	bData;
    int wData;
    unsigned int i;
    // First reset data Stdf_PLR_V4
    Reset();

    _FIELD_CHECKREAD(	clStdf.ReadByte(&bData),	eposCONT_FLG	);
    _FIELD_SET(	m_b1CONT_FLG	=	stdf_type_b1	(  bData )	,true,	eposCONT_FLG	);

    _FIELD_CHECKREAD(	clStdf.ReadWord(&wData)	,	eposNMR_INDX	);
    _FIELD_SET(	m_u2NMR_INDX	=	stdf_type_u2	(  wData )	,true,	eposNMR_INDX	);

    _FIELD_CHECKREAD(	clStdf.ReadWord(&wData)	,	eposTOTM_CNT	);
    _FIELD_SET(	m_u2TOTM_CNT	=	stdf_type_u2	(  wData )	,true,	eposTOTM_CNT	);

    _FIELD_CHECKREAD(	clStdf.ReadWord(&wData)	,	eposLOCM_CNT	);
    _FIELD_SET(	m_u2LOCM_CNT	=	stdf_type_u2	( wData )	,true,	eposLOCM_CNT	);

    if(m_u2LOCM_CNT > 0)
        m_pu2PMR_INDX = new stdf_type_u2[m_u2LOCM_CNT];
    for(i=0; i<(unsigned int)m_u2LOCM_CNT; i++)
    {
        _FIELD_CHECKREAD(clStdf.ReadWord(&wData), eposPMR_INDX);
        _FIELD_SET(m_pu2PMR_INDX[i] = wData, true, eposPMR_INDX);
    }

    if(m_u2LOCM_CNT > 0)
        m_pcnATPG_NAM = new stdf_type_cn[m_u2LOCM_CNT];
    for(i=0; i<(unsigned int)m_u2LOCM_CNT; i++)
    {
        _FIELD_CHECKREAD(clStdf.ReadString(szString), eposATPG_NAM);
        _FIELD_SET(m_pcnATPG_NAM[i] = szString, true, eposATPG_NAM);
    }

    return true;
}

bool Stdf_NMR_V4::Write(GS::StdLib::Stdf & clStdf)
{
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;

    RecordReadInfo.iRecordType = STDF_NMR_TYPE;
    RecordReadInfo.iRecordSubType = STDF_NMR_STYPE;
    clStdf.WriteHeader(&RecordReadInfo);

    _FIELD_CHECKWRITE(	clStdf.WriteByte(m_b1CONT_FLG),	eposCONT_FLG, clStdf.WriteRecord());
    _FIELD_CHECKWRITE(	clStdf.WriteWord(m_u2NMR_INDX),	eposNMR_INDX, clStdf.WriteRecord());
    _FIELD_CHECKWRITE(	clStdf.WriteWord(m_u2TOTM_CNT),	eposTOTM_CNT, clStdf.WriteRecord());
    _FIELD_CHECKWRITE(	clStdf.WriteWord(m_u2LOCM_CNT),	eposLOCM_CNT, clStdf.WriteRecord());

    _FIELD_CHECKWRITE_KUi(clStdf.WriteWord, m_pu2PMR_INDX, m_u2LOCM_CNT, eposPMR_INDX, clStdf.WriteRecord());
    _FIELD_CHECKWRITE_KCN(clStdf, m_pcnATPG_NAM, m_u2LOCM_CNT, eposATPG_NAM, clStdf.WriteRecord());

    clStdf.WriteRecord();

    return true;
}

void Stdf_NMR_V4::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    // Empty string first
    strAsciiString = "";
    _STR_ADDFIELD_ASCII(strAsciiString,"NMR.CONT_FLG",QString::number(m_b1CONT_FLG),nFieldSelection, eposCONT_FLG);
    _STR_ADDFIELD_ASCII(strAsciiString,"NMR.NMR_INDX",QString::number(m_u2NMR_INDX),nFieldSelection, eposNMR_INDX);
    _STR_ADDFIELD_ASCII(strAsciiString,"NMR.TOTM_CNT",QString::number(m_u2TOTM_CNT),nFieldSelection, eposTOTM_CNT);
    _STR_ADDFIELD_ASCII(strAsciiString,"NMR.LOCM_CNT",QString::number(m_u2LOCM_CNT),nFieldSelection, eposLOCM_CNT);

    _CREATEFIELD_FROM_KUi_ASCII(m_u2LOCM_CNT, m_pu2PMR_INDX, eposPMR_INDX);
    _STR_ADDFIELD_ASCII(strAsciiString	,	"NMR.PMR_INDX",	m_strFieldValue_macro	,nFieldSelection, 	eposPMR_INDX	);
    _CREATEFIELD_FROM_KCN_ASCII(m_u2LOCM_CNT, m_pcnATPG_NAM, eposATPG_NAM);
    _STR_ADDFIELD_ASCII(strAsciiString	,	"NMR.ATPG_NAM",	m_strFieldValue_macro	,nFieldSelection, 	eposATPG_NAM	);

}

void Stdf_NMR_V4::GetAsciiFieldList(QStringList & listFields,int nFieldSelection  /* = 0*/)
{

    // Empty string list first
    listFields.empty();

    _LIST_ADDFIELD_ASCII(listFields,"NMR.CONT_FLG",QString::number(m_b1CONT_FLG),nFieldSelection, eposCONT_FLG);
    _LIST_ADDFIELD_ASCII(listFields,"NMR.NMR_INDX",QString::number(m_u2NMR_INDX),nFieldSelection, eposNMR_INDX);
    _LIST_ADDFIELD_ASCII(listFields,"NMR.TOTM_CNT",QString::number(m_u2TOTM_CNT),nFieldSelection, eposTOTM_CNT);
    _LIST_ADDFIELD_ASCII(listFields,"NMR.LOCM_CNT",QString::number(m_u2LOCM_CNT),nFieldSelection, eposLOCM_CNT);

    _CREATEFIELD_FROM_KUi_ASCII(m_u2LOCM_CNT, m_pu2PMR_INDX, eposPMR_INDX);
    _LIST_ADDFIELD_ASCII(listFields	,	"NMR.PMR_INDX",	m_strFieldValue_macro	,nFieldSelection, 	eposPMR_INDX	);
    _CREATEFIELD_FROM_KCN_ASCII(m_u2LOCM_CNT, m_pcnATPG_NAM, eposATPG_NAM);
    _LIST_ADDFIELD_ASCII(listFields	,	"NMR.ATPG_NAM",	m_strFieldValue_macro	,nFieldSelection, 	eposATPG_NAM	);
}

void Stdf_NMR_V4::GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection /*= 0*/)
{



    QString strTabs="";

    // Init XML string
    for(int nIndex=nIndentationLevel; nIndex>0; nIndex--)
        strTabs += "\t";
    strXmlString = strTabs;
    strXmlString += "<NMR>\n";
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,"CONT_FLG",QString::number(m_b1CONT_FLG),nFieldSelection, eposCONT_FLG);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,"NMR_INDX",QString::number(m_u2NMR_INDX),nFieldSelection, eposNMR_INDX);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,"TOTM_CNT",QString::number(m_u2TOTM_CNT),nFieldSelection, eposTOTM_CNT);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,"LOCM_CNT",QString::number(m_u2LOCM_CNT),nFieldSelection, eposLOCM_CNT);

    _CREATEFIELD_FROM_KUi_XML(m_u2LOCM_CNT, m_pu2PMR_INDX, eposPMR_INDX);
    _STR_ADDLIST_XML(strXmlString,m_u2LOCM_CNT, nIndentationLevel+1,"PMR_INDX",	m_strFieldValue_macro	,nFieldSelection, 	eposPMR_INDX	);

    _CREATEFIELD_FROM_KCN_XML(m_u2LOCM_CNT, m_pcnATPG_NAM, eposATPG_NAM);
    _STR_ADDLIST_XML(strXmlString,m_u2LOCM_CNT, nIndentationLevel+1,"ATPG_NAM",	m_strFieldValue_macro	,nFieldSelection, 	eposATPG_NAM	);

    strXmlString += strTabs;

    strXmlString += "</NMR>\n";
}

void Stdf_NMR_V4::GetAtdfString(QString & strAtdfString)
{
    // Reset field index, and line Nb
    m_uiFieldIndex = 0;
    m_uiLineNb = 1;

    // Init string
    strAtdfString = "NMR:";

    _STR_ADDFIELD_ATDF(strAtdfString,QString::number(m_b1CONT_FLG), eposCONT_FLG);
    _STR_ADDFIELD_ATDF(strAtdfString,QString::number(m_u2NMR_INDX), eposNMR_INDX);
    _STR_ADDFIELD_ATDF(strAtdfString,QString::number(m_u2TOTM_CNT), eposTOTM_CNT);
    _STR_ADDFIELD_ATDF(strAtdfString,QString::number(m_u2LOCM_CNT), eposLOCM_CNT);

    m_strFieldValue_macro = "";
    _CREATEFIELD_FROM_KUi_ATDF(m_u2LOCM_CNT, m_pu2PMR_INDX, eposPMR_INDX);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposPMR_INDX);

    m_strFieldValue_macro = "";
    _CREATEFIELD_FROM_KCN_ATDF(m_u2LOCM_CNT, m_pcnATPG_NAM, eposATPG_NAM);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposATPG_NAM);

    _STR_FINISHFIELD_ATDF(strAtdfString);
}

///////////////////////////////////////////////////////////
// CNR RECORD
///////////////////////////////////////////////////////////
Stdf_CNR_V4::Stdf_CNR_V4() : Stdf_Record()
{
    Reset();
}

Stdf_CNR_V4::~Stdf_CNR_V4()
{
    Reset();
}

void Stdf_CNR_V4::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;
     m_u2CHN_NUM =0	;//U*2
     m_u4BIT_POS =0	;//U*4
     m_cnCELL_NAM ="";//S*n

     // Call Reset base method
     Stdf_Record::Reset();
}

bool Stdf_CNR_V4::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;

    return false;
}

QString Stdf_CNR_V4::GetRecordShortName(void)
{
    return "CNR";
}

QString Stdf_CNR_V4::GetRecordLongName(void)
{
    return "Scan Cell Name Record";
}

int Stdf_CNR_V4::GetRecordType(void)
{
    return Rec_CNR;
}

bool Stdf_CNR_V4::Read(GS::StdLib::Stdf & clStdf)
{
    char	szString[STDF_MAX_U2+1];
    int wData;
    long	dwData;
    // First reset data
    Reset();

    _FIELD_CHECKREAD(clStdf.ReadWord(&wData),	eposCHN_NUM	);
    _FIELD_SET(m_u2CHN_NUM	=	stdf_type_u2	(  wData )	,true,	eposCHN_NUM	);

    _FIELD_CHECKREAD(clStdf.ReadDword(&dwData)	,	eposBIT_POS	);
    _FIELD_SET(m_u4BIT_POS	=	stdf_type_u4	(  dwData )	,true,	eposBIT_POS	);

    _FIELD_CHECKREAD(clStdf.ReadSNString(szString), eposCELL_NAM);
    _FIELD_SET(m_cnCELL_NAM= szString, !m_cnCELL_NAM.isEmpty(), eposCELL_NAM);

    return true;
}

bool Stdf_CNR_V4::Write(GS::StdLib::Stdf & clStdf)
{
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;

    RecordReadInfo.iRecordType = STDF_CNR_TYPE;
    RecordReadInfo.iRecordSubType = STDF_CNR_STYPE;
    clStdf.WriteHeader(&RecordReadInfo);

    // CNR.EXC_DESC
    _FIELD_CHECKWRITE(clStdf.WriteWord(m_u2CHN_NUM), eposCHN_NUM, clStdf.WriteRecord());
    _FIELD_CHECKWRITE(clStdf.WriteDword(m_u4BIT_POS), eposBIT_POS, clStdf.WriteRecord());
    _FIELD_CHECKWRITE(clStdf.WriteSNString(m_cnCELL_NAM.toLatin1().constData()), eposCELL_NAM, clStdf.WriteRecord());

    clStdf.WriteRecord();

    return true;
}

void Stdf_CNR_V4::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    // Empty string first
    strAsciiString = "";

    _STR_ADDFIELD_ASCII(strAsciiString, "CNR.CHN_NUM", QString::number(m_u2CHN_NUM), nFieldSelection,   eposCHN_NUM);
    _STR_ADDFIELD_ASCII(strAsciiString, "CNR.BIT_POS", QString::number(m_u4BIT_POS), nFieldSelection,   eposBIT_POS);
    _STR_ADDFIELD_ASCII(strAsciiString, "CNR.CELL_NAM", m_cnCELL_NAM, nFieldSelection, eposCELL_NAM);
}

void Stdf_CNR_V4::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{

    // Empty string list first
    listFields.empty();

    _LIST_ADDFIELD_ASCII(listFields, "CNR.CHN_NUM", QString::number(m_u2CHN_NUM), nFieldSelection,   eposCHN_NUM);
    _LIST_ADDFIELD_ASCII(listFields, "CNR.BIT_POS", QString::number(m_u4BIT_POS), nFieldSelection,   eposBIT_POS);
    _LIST_ADDFIELD_ASCII(listFields, "CNR.CELL_NAM", m_cnCELL_NAM, nFieldSelection, eposCELL_NAM);
}

void Stdf_CNR_V4::GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection /*= 0*/)
{
    QString strTabs="";

    // Init XML string
    for(int nIndex=nIndentationLevel; nIndex>0; nIndex--)
        strTabs += "\t";
    strXmlString = strTabs;
    strXmlString += "<CNR>\n";
    // CNR.UPD_NAM

    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "CHN_NUM", QString::number(m_u2CHN_NUM), nFieldSelection, eposCHN_NUM);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "BIT_POS", QString::number(m_u4BIT_POS), nFieldSelection, eposBIT_POS);

    _CREATEFIELD_FROM_CN_XML(m_cnCELL_NAM);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "CELL_NAM", m_strFieldValue_macro, nFieldSelection, eposCELL_NAM);

    strXmlString += strTabs;
    strXmlString += "</CNR>\n";
}

void Stdf_CNR_V4::GetAtdfString(QString & strAtdfString)
{
    // Reset field index, and line Nb
    m_uiFieldIndex = 0;
    m_uiLineNb = 1;

    // Init string
    strAtdfString = "CNR:";

    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u2CHN_NUM), eposCHN_NUM);
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u4BIT_POS), eposBIT_POS);

    _STR_ADDFIELD_ATDF(strAtdfString, m_cnCELL_NAM, eposCELL_NAM);

    _STR_FINISHFIELD_ATDF(strAtdfString);
}


///////////////////////////////////////////////////////////
// SSR RECORD
///////////////////////////////////////////////////////////
Stdf_SSR_V4::Stdf_SSR_V4() : Stdf_Record()
{
    Reset();
}

Stdf_SSR_V4::~Stdf_SSR_V4()
{
    Reset();
}

void Stdf_SSR_V4::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;
    m_cnSSR_NAM   = "";
    m_u2CHN_CNT   = 0;
    m_pu2CHN_LIST = 0;

    // Call Reset base method
    Stdf_Record::Reset();
}

bool Stdf_SSR_V4::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;

    return false;
}

QString Stdf_SSR_V4::GetRecordShortName(void)
{
    return "SSR";
}

QString Stdf_SSR_V4::GetRecordLongName(void)
{
    return "Scan Structure Record";
}

int Stdf_SSR_V4::GetRecordType(void)
{
    return Rec_SSR;
}

bool Stdf_SSR_V4::Read(GS::StdLib::Stdf & clStdf)
{
    char	szString[STDF_MAX_U1+1];
    int     wData, i;
    // First reset data
    Reset();
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposSSR_NAM);
    _FIELD_SET(m_cnSSR_NAM = szString, !m_cnSSR_NAM.isEmpty(), eposSSR_NAM);

    _FIELD_CHECKREAD(clStdf.ReadWord(&wData), eposCHN_CNT);
    _FIELD_SET(m_u2CHN_CNT = stdf_type_u2(wData), true, eposCHN_CNT);

    if(m_u2CHN_CNT > 0)
        m_pu2CHN_LIST = new stdf_type_u2[m_u2CHN_CNT];
    for(i=0; i<m_u2CHN_CNT; i++)
    {
        _FIELD_CHECKREAD(clStdf.ReadWord(&wData), eposCHN_LIST);
        _FIELD_SET(m_pu2CHN_LIST[i] = wData, true, eposCHN_LIST);
    }

    return true;
}

bool Stdf_SSR_V4::Write(GS::StdLib::Stdf & clStdf)
{
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    RecordReadInfo.iRecordType = STDF_SSR_TYPE;
    RecordReadInfo.iRecordSubType = STDF_SSR_STYPE;
    clStdf.WriteHeader(&RecordReadInfo);

    // SSR.EXC_DESC
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnSSR_NAM.toLatin1().constData()), eposSSR_NAM, clStdf.WriteRecord());
    _FIELD_CHECKWRITE(clStdf.WriteWord(m_u2CHN_CNT), eposCHN_CNT, clStdf.WriteRecord());

    _FIELD_CHECKWRITE_KUi(clStdf.WriteWord, m_pu2CHN_LIST, m_u2CHN_CNT, eposCHN_LIST, clStdf.WriteRecord());

    clStdf.WriteRecord();

    return true;
}

void Stdf_SSR_V4::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    // Empty string first
    strAsciiString = "";


    _STR_ADDFIELD_ASCII(strAsciiString, "SSR.SSR_NAM", m_cnSSR_NAM, nFieldSelection, eposSSR_NAM);
    _STR_ADDFIELD_ASCII(strAsciiString, "SSR.CHN_CNT", QString::number(m_u2CHN_CNT), nFieldSelection,   eposCHN_CNT);

    _CREATEFIELD_FROM_KUi_ASCII(m_u2CHN_CNT, m_pu2CHN_LIST, eposCHN_LIST);
    _STR_ADDFIELD_ASCII(strAsciiString, "SSR.CHN_LIST", m_strFieldValue_macro, nFieldSelection, eposCHN_LIST);
}

void Stdf_SSR_V4::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{

    // Empty string list first
    listFields.empty();

    _LIST_ADDFIELD_ASCII(listFields, "SSR.SSR_NAM", m_cnSSR_NAM, nFieldSelection, eposSSR_NAM);
    _LIST_ADDFIELD_ASCII(listFields, "SSR.CHN_CNT", QString::number(m_u2CHN_CNT), nFieldSelection,   eposCHN_CNT);

    _CREATEFIELD_FROM_KUi_ASCII(m_u2CHN_CNT, m_pu2CHN_LIST, eposCHN_CNT);
    _LIST_ADDFIELD_ASCII(listFields, "PGR.CHN_CNT", m_strFieldValue_macro, nFieldSelection, eposCHN_LIST);

}

void Stdf_SSR_V4::GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection /*= 0*/)
{
    QString strTabs="";

    // Init XML string
    for(int nIndex=nIndentationLevel; nIndex>0; nIndex--)
        strTabs += "\t";
    strXmlString = strTabs;
    strXmlString += "<SSR>\n";
    // SSR.UPD_NAM

    _CREATEFIELD_FROM_CN_XML(m_cnSSR_NAM);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "SSR_NAM", m_strFieldValue_macro, nFieldSelection, eposSSR_NAM);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "CHN_CNT", QString::number(m_u2CHN_CNT), nFieldSelection, eposCHN_CNT);

    _CREATEFIELD_FROM_KUi_XML(m_u2CHN_CNT, m_pu2CHN_LIST, eposCHN_LIST);
    _STR_ADDLIST_XML(strXmlString, m_u2CHN_CNT, nIndentationLevel+1, "CHN_CNT", m_strFieldValue_macro, nFieldSelection, eposCHN_LIST);

    strXmlString += strTabs;
    strXmlString += "</SSR>\n";
}

void Stdf_SSR_V4::GetAtdfString(QString & strAtdfString)
{
    // Reset field index, and line Nb
    m_uiFieldIndex = 0;
    m_uiLineNb = 1;

    // Init string
    strAtdfString = "SSR:";

    _STR_ADDFIELD_ATDF(strAtdfString, m_cnSSR_NAM, eposSSR_NAM)
    _STR_ADDFIELD_ATDF(strAtdfString, QString::number(m_u2CHN_CNT), eposCHN_CNT);

    _CREATEFIELD_FROM_KUi_ATDF(m_u2CHN_CNT, m_pu2CHN_LIST, eposCHN_LIST);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposCHN_LIST);

    _STR_FINISHFIELD_ATDF(strAtdfString);
}



///////////////////////////////////////////////////////////
// CDR RECORD
///////////////////////////////////////////////////////////
Stdf_CDR_V4::Stdf_CDR_V4() : Stdf_Record()
{
    Reset();
}

Stdf_CDR_V4::~Stdf_CDR_V4()
{
    Reset();
}

void Stdf_CDR_V4::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    m_b1CONT_FLG  = 0;
    m_u2CDR_INDX  = 0;
    m_cnCHN_NAM   = "";
    m_u4CHN_LEN   = 0;
    m_u2SIN_PIN   = 0;
    m_u2SOUT_PIN  = 0;
    m_u1MSTR_CNT  = 0;
    m_pu2M_CLKS   = 0;
    m_u1SLAV_CNT  = 0;
    m_pu2S_CLKS   = 0;
    m_u1INV_VAL   = 0;
    m_u2LST_CNT   = 0;
    m_pcnCELL_LST = 0;

    // Call Reset base method
    Stdf_Record::Reset();
}

bool Stdf_CDR_V4::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;

    return false;
}

QString Stdf_CDR_V4::GetRecordShortName(void)
{
    return "CDR";
}

QString Stdf_CDR_V4::GetRecordLongName(void)
{
    return "Chain Description Record";
}

int Stdf_CDR_V4::GetRecordType(void)
{
    return Rec_CDR;
}

bool Stdf_CDR_V4::Read(GS::StdLib::Stdf & clStdf)
{
    char	szString[STDF_MAX_U2+1];
    long	dwData;
    stdf_type_u1	bData;
    int wData;
    unsigned int i;
    // First reset data Stdf_PLR_V4
    Reset();


    m_pu2M_CLKS   = 0;
    m_u1SLAV_CNT  = 0;
    m_pu2S_CLKS   = 0;
    m_u1INV_VAL   = 0;
    m_u2LST_CNT   = 0;
    m_pcnCELL_LST = 0;


    _FIELD_CHECKREAD(clStdf.ReadByte(&bData),	eposCONT_FLG	);
    _FIELD_SET(	m_b1CONT_FLG	=	stdf_type_b1	(  bData )	,true,	eposCONT_FLG	);

    _FIELD_CHECKREAD(clStdf.ReadWord(&wData)	,	eposCDR_INDX	);
    _FIELD_SET(	m_u2CDR_INDX	=	stdf_type_u2	(  wData )	,true,	eposCDR_INDX	);

    _FIELD_CHECKREAD(clStdf.ReadString(szString)	,	eposCHN_NAM	);
    _FIELD_SET(	m_cnCHN_NAM	=	stdf_type_cn	(  szString )	,true,	eposCHN_NAM	);

    _FIELD_CHECKREAD(clStdf.ReadDword(&dwData)	,	eposCHN_LEN	);
    _FIELD_SET(	m_u4CHN_LEN	=	stdf_type_u4	( dwData )	,true,	eposCHN_LEN	);

    _FIELD_CHECKREAD(clStdf.ReadWord(&wData)	,	eposSIN_PIN	);
    _FIELD_SET(	m_u2SIN_PIN	=	stdf_type_u2	( wData  )	,true,	eposSIN_PIN	);

    _FIELD_CHECKREAD(clStdf.ReadWord(&wData)	,	eposSOUT_PIN	);
    _FIELD_SET(	m_u2SOUT_PIN	=	stdf_type_u2	( wData  )	,true,	eposSOUT_PIN	);

    _FIELD_CHECKREAD(clStdf.ReadByte(&bData)	,	eposMSTR_CNT	);
    _FIELD_SET(	m_u1MSTR_CNT	=	stdf_type_u1	( bData  )	,true,	eposMSTR_CNT	);

    if(m_u1MSTR_CNT > 0)
        m_pu2M_CLKS = new stdf_type_u2[m_u1MSTR_CNT];
    for(i=0; i<m_u1MSTR_CNT; i++)
    {
        _FIELD_CHECKREAD(clStdf.ReadWord(&wData), eposM_CLKS);
        _FIELD_SET(m_pu2M_CLKS[i] = wData, true, eposM_CLKS);
    }

    _FIELD_CHECKREAD(clStdf.ReadByte(&bData)	,	eposSLAV_CNT	);
    _FIELD_SET(	m_u1SLAV_CNT	=	stdf_type_u1	( bData  )	,true,	eposSLAV_CNT	);

    if(m_u1SLAV_CNT > 0)
        m_pu2S_CLKS = new stdf_type_u2[m_u1SLAV_CNT];
    for(i=0; i<m_u1SLAV_CNT; i++)
    {
        _FIELD_CHECKREAD(clStdf.ReadWord(&wData), eposS_CLKS);
        _FIELD_SET(m_pu2S_CLKS[i] = wData, true, eposS_CLKS);
    }

    _FIELD_CHECKREAD(clStdf.ReadByte(&bData)	,	eposINV_VAL	);
    _FIELD_SET(	m_u1INV_VAL	=	stdf_type_u1	( bData  )	,true,	eposINV_VAL	);

    _FIELD_CHECKREAD(clStdf.ReadWord(&wData)	,	eposLST_CNT	);
    _FIELD_SET(	m_u2LST_CNT	=	stdf_type_u2	( wData  )	,true,	eposLST_CNT	);


    if(m_u2LST_CNT > 0)
        m_pcnCELL_LST = new stdf_type_cn[m_u2LST_CNT];
    for(i=0; i<m_u2LST_CNT; i++)
    {
        _FIELD_CHECKREAD(clStdf.ReadSNString(szString), eposCELL_LST);
        _FIELD_SET(m_pcnCELL_LST[i] = szString, true, eposCELL_LST);
    }

    return true;
}

bool Stdf_CDR_V4::Write(GS::StdLib::Stdf & clStdf)
{
    unsigned int i;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;

    RecordReadInfo.iRecordType = STDF_CDR_TYPE;
    RecordReadInfo.iRecordSubType = STDF_CDR_STYPE;
    clStdf.WriteHeader(&RecordReadInfo);

    _FIELD_CHECKWRITE(clStdf.WriteByte(m_b1CONT_FLG),	eposCONT_FLG	,clStdf.WriteRecord());
    _FIELD_CHECKWRITE(clStdf.WriteWord(m_u2CDR_INDX),	eposCDR_INDX	,clStdf.WriteRecord());
    _FIELD_CHECKWRITE(clStdf.WriteString(m_cnCHN_NAM.toLatin1().constData()),	eposCHN_NAM	,clStdf.WriteRecord());
    _FIELD_CHECKWRITE(clStdf.WriteDword(m_u4CHN_LEN),    eposCHN_LEN	,clStdf.WriteRecord());
    _FIELD_CHECKWRITE(clStdf.WriteWord(m_u2SIN_PIN),	eposSIN_PIN	,clStdf.WriteRecord());
    _FIELD_CHECKWRITE(clStdf.WriteWord(m_u2SOUT_PIN),	eposSOUT_PIN	,clStdf.WriteRecord());
    _FIELD_CHECKWRITE(clStdf.WriteByte(m_u1MSTR_CNT),	eposMSTR_CNT	,clStdf.WriteRecord());
    for(i=0; i<m_u1MSTR_CNT; i++)
        _FIELD_CHECKWRITE(clStdf.WriteWord(m_pu2M_CLKS[i]), eposM_CLKS, clStdf.WriteRecord());

    _FIELD_CHECKWRITE(clStdf.WriteByte(m_u1SLAV_CNT),	eposSLAV_CNT	,clStdf.WriteRecord());
    for(i=0; i<m_u1SLAV_CNT; i++)
        _FIELD_CHECKWRITE(clStdf.WriteWord(m_pu2S_CLKS [i]), eposS_CLKS, clStdf.WriteRecord());

    _FIELD_CHECKWRITE(clStdf.WriteByte(m_u1INV_VAL),	eposINV_VAL	,clStdf.WriteRecord());
    _FIELD_CHECKWRITE(clStdf.WriteWord(m_u2LST_CNT),	eposLST_CNT	,clStdf.WriteRecord());
    for(i=0; i<m_u2LST_CNT; i++)
        _FIELD_CHECKWRITE(clStdf.WriteSNString(m_pcnCELL_LST[i].toLatin1().data()), eposCELL_LST	, clStdf.WriteRecord());

    clStdf.WriteRecord();

    return true;
}

void Stdf_CDR_V4::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    // Empty string first
    strAsciiString = "";

    _STR_ADDFIELD_ASCII(strAsciiString	,	"CDR.CONT_FLG",	QString::number(m_b1CONT_FLG)	,nFieldSelection, 	eposCONT_FLG	);
    _STR_ADDFIELD_ASCII(strAsciiString	,	"CDR.CDR_INDX",	QString::number(m_u2CDR_INDX)	,nFieldSelection, 	eposCDR_INDX	);
    _STR_ADDFIELD_ASCII(strAsciiString	,	"CDR.CHN_NAM",	m_cnCHN_NAM	,nFieldSelection, 	eposCHN_NAM	);
    _STR_ADDFIELD_ASCII(strAsciiString	,	"CDR.CHN_LEN",	QString::number(m_u4CHN_LEN)	,nFieldSelection, 	eposCHN_LEN	);
    _STR_ADDFIELD_ASCII(strAsciiString	,	"CDR.SIN_PIN",	QString::number(m_u2SIN_PIN)	,nFieldSelection, 	eposSIN_PIN	);
    _STR_ADDFIELD_ASCII(strAsciiString	,	"CDR.SOUT_PIN",	QString::number(m_u2SOUT_PIN)	,nFieldSelection, 	eposSOUT_PIN	);

    _STR_ADDFIELD_ASCII(strAsciiString	,	"CDR.MSTR_CNT",	QString::number(m_u1MSTR_CNT)	,nFieldSelection, 	eposMSTR_CNT	);

    _CREATEFIELD_FROM_KUi_ASCII(m_u1MSTR_CNT, m_pu2M_CLKS, eposM_CLKS);
    _STR_ADDFIELD_ASCII(strAsciiString	,	"CDR.M_CLKS",	m_strFieldValue_macro	,nFieldSelection, 	eposM_CLKS	);

    _STR_ADDFIELD_ASCII(strAsciiString	,	"CDR.SLAV_CNT",	QString::number(m_u1SLAV_CNT)	,nFieldSelection, 	eposSLAV_CNT	);

    _CREATEFIELD_FROM_KUi_ASCII(m_u1SLAV_CNT, m_pu2S_CLKS, eposS_CLKS);
    _STR_ADDFIELD_ASCII(strAsciiString	,	"CDR.S_CLKS",	m_strFieldValue_macro	,nFieldSelection, 	eposS_CLKS	);

    _STR_ADDFIELD_ASCII(strAsciiString	,	"CDR.INV_VAL",	QString::number(m_u1INV_VAL)	,nFieldSelection, 	eposINV_VAL	);
    _STR_ADDFIELD_ASCII(strAsciiString	,	"CDR.LST_CNT",	QString::number(m_u2LST_CNT)	,nFieldSelection, 	eposLST_CNT	);

    _CREATEFIELD_FROM_KCN_ASCII(m_u2LST_CNT, m_pcnCELL_LST, eposCELL_LST);
    _STR_ADDFIELD_ASCII(strAsciiString	,	"CDR.CELL_LST",	m_strFieldValue_macro	,nFieldSelection, 	eposCELL_LST	);

}

void Stdf_CDR_V4::GetAsciiFieldList(QStringList & listFields,int nFieldSelection  /* = 0*/)
{

    // Empty string list first
    listFields.empty();

    _LIST_ADDFIELD_ASCII(listFields	,	"CDR.CONT_FLG",	QString::number(m_b1CONT_FLG)	,nFieldSelection, 	eposCONT_FLG	);
    _LIST_ADDFIELD_ASCII(listFields	,	"CDR.CDR_INDX",	QString::number(m_u2CDR_INDX)	,nFieldSelection, 	eposCDR_INDX	);
    _LIST_ADDFIELD_ASCII(listFields	,	"CDR.CHN_NAM",	m_cnCHN_NAM	,nFieldSelection, 	eposCHN_NAM	);
    _LIST_ADDFIELD_ASCII(listFields	,	"CDR.CHN_LEN",	QString::number(m_u4CHN_LEN)	,nFieldSelection, 	eposCHN_LEN	);
    _LIST_ADDFIELD_ASCII(listFields	,	"CDR.SIN_PIN",	QString::number(m_u2SIN_PIN)	,nFieldSelection, 	eposSIN_PIN	);
    _LIST_ADDFIELD_ASCII(listFields	,	"CDR.SOUT_PIN",	QString::number(m_u2SOUT_PIN)	,nFieldSelection, 	eposSOUT_PIN	);
    _LIST_ADDFIELD_ASCII(listFields	,	"CDR.MSTR_CNT",	QString::number(m_u1MSTR_CNT)	,nFieldSelection, 	eposMSTR_CNT	);

    _CREATEFIELD_FROM_KUi_ASCII(m_u1MSTR_CNT, m_pu2M_CLKS, eposM_CLKS);
    _LIST_ADDFIELD_ASCII(listFields	,	"CDR.M_CLKS",	m_strFieldValue_macro	,nFieldSelection, 	eposM_CLKS	);

    _LIST_ADDFIELD_ASCII(listFields	,	"CDR.SLAV_CNT",	QString::number(m_u1SLAV_CNT)	,nFieldSelection, 	eposSLAV_CNT	);
    _CREATEFIELD_FROM_KUi_ASCII(m_u1SLAV_CNT, m_pu2S_CLKS , eposS_CLKS );
    _LIST_ADDFIELD_ASCII(listFields	,	"CDR.S_CLKS ",	m_strFieldValue_macro	,nFieldSelection, 	eposS_CLKS 	);

    _LIST_ADDFIELD_ASCII(listFields	,	"CDR.INV_VAL",	QString::number(m_u1INV_VAL)	,nFieldSelection, 	eposINV_VAL	);
    _LIST_ADDFIELD_ASCII(listFields	,	"CDR.LST_CNT",	QString::number(m_u2LST_CNT)	,nFieldSelection, 	eposLST_CNT	);
    _CREATEFIELD_FROM_KCN_ASCII(m_u2LST_CNT, m_pcnCELL_LST , eposCELL_LST );
    _LIST_ADDFIELD_ASCII(listFields	,	"CDR.CELL_LST",	m_strFieldValue_macro	,nFieldSelection, 	eposCELL_LST 	);
}

void Stdf_CDR_V4::GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection /*= 0*/)
{

    QString strTabs="";

    // Init XML string
    for(int nIndex=nIndentationLevel; nIndex>0; nIndex--)
        strTabs += "\t";
    strXmlString = strTabs;
    strXmlString += "<CDR>\n";

    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1	,	"CONT_FLG",	QString::number(m_b1CONT_FLG)	,nFieldSelection, 	eposCONT_FLG	);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1	,	"CDR_INDX",	QString::number(m_u2CDR_INDX)	,nFieldSelection, 	eposCDR_INDX	);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1	,	"CHN_NAM",	m_cnCHN_NAM	,nFieldSelection, 	eposCHN_NAM	);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1	,	"CHN_LEN",	QString::number(m_u4CHN_LEN)	,nFieldSelection, 	eposCHN_LEN	);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1	,	"SIN_PIN",	QString::number(m_u2SIN_PIN)	,nFieldSelection, 	eposSIN_PIN	);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1	,	"SOUT_PIN",	QString::number(m_u2SOUT_PIN)	,nFieldSelection, 	eposSOUT_PIN	);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1	,	"MSTR_CNT",	QString::number(m_u1MSTR_CNT)	,nFieldSelection, 	eposMSTR_CNT	);

    _CREATEFIELD_FROM_KUi_XML(m_u1MSTR_CNT, m_pu2M_CLKS, eposM_CLKS);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1	,	"M_CLKS",	m_strFieldValue_macro	,nFieldSelection, 	eposM_CLKS	);

    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1	,	"SLAV_CNT",	QString::number(m_u1SLAV_CNT)	,nFieldSelection, 	eposSLAV_CNT	);

    _CREATEFIELD_FROM_KUi_XML(m_u1SLAV_CNT, m_pu2S_CLKS , eposS_CLKS );
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1	,	"S_CLKS ",	m_strFieldValue_macro	,nFieldSelection, 	eposS_CLKS 	);

    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1	,	"INV_VAL",	QString::number(m_u1INV_VAL)	,nFieldSelection, 	eposINV_VAL	);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1	,	"LST_CNT",	QString::number(m_u2LST_CNT)	,nFieldSelection, 	eposLST_CNT	);

    _CREATEFIELD_FROM_KCN_XML(m_u2LST_CNT, m_pcnCELL_LST , eposCELL_LST );
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1	,	"CELL_LST",	m_strFieldValue_macro	,nFieldSelection, 	eposCELL_LST 	);

    strXmlString += strTabs;

    strXmlString += "</CDR>\n";
}

void Stdf_CDR_V4::GetAtdfString(QString & strAtdfString)
{
    // Reset field index, and line Nb
    m_uiFieldIndex = 0;
    m_uiLineNb = 1;

    // Init string
    strAtdfString = "CDR:";


    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_b1CONT_FLG), 	eposCONT_FLG	);
    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_u2CDR_INDX), 	eposCDR_INDX	);
    _STR_ADDFIELD_ATDF(strAtdfString,	m_cnCHN_NAM, 	eposCHN_NAM	);
    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_u4CHN_LEN), 	eposCHN_LEN	);
    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_u2SIN_PIN), 	eposSIN_PIN	);
    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_u2SOUT_PIN), 	eposSOUT_PIN	);

    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_u1MSTR_CNT), 	eposMSTR_CNT);

    _CREATEFIELD_FROM_KUi_ATDF(m_u1MSTR_CNT, m_pu2M_CLKS, eposM_CLKS);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro	, 	eposM_CLKS	);

    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_u1SLAV_CNT)	, 	eposSLAV_CNT	);

    _CREATEFIELD_FROM_KUi_ATDF(m_u1SLAV_CNT, m_pu2S_CLKS, eposS_CLKS);
    _STR_ADDFIELD_ATDF(strAtdfString,m_strFieldValue_macro	, 	eposS_CLKS	);

    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_u1INV_VAL)	,eposINV_VAL	);
    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_u2LST_CNT)	,eposLST_CNT	);

    _CREATEFIELD_FROM_KCN_ATDF(m_u2LST_CNT, m_pcnCELL_LST, eposCELL_LST);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro	,eposCELL_LST	);

    _STR_FINISHFIELD_ATDF(strAtdfString);
}


///////////////////////////////////////////////////////////
// STR RECORD
///////////////////////////////////////////////////////////
Stdf_STR_V4::Stdf_STR_V4() : Stdf_Record()
{
    Reset();
}

Stdf_STR_V4::~Stdf_STR_V4()
{
    Reset();
}

void Stdf_STR_V4::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    m_b1CONT_FLG = 0 ;
    m_u4TEST_NUM = 0 ;
    m_u1HEAD_NUM = 1 ;
    m_u1SITE_NUM = 1 ;
    m_u2PSR_REF = 0 ;
    m_b1TEST_FLG = 0 ;
    m_cnLOG_TYP = "" ;
    m_cnTEST_TXT = "" ;
    m_cnALARM_ID = "" ;
    m_cnPROG_TXT = "";
    m_cnRSLT_TXT = "";
    m_u1Z_VAL = 0 ;
    m_b1FMU_FLG = 0 ;
    m_dnMASK_MAP.m_uiLength = 0;
    m_dnFAL_MAP.m_uiLength = 0;
    m_u8CYC_CNT = 0 ;
    m_u4TOTF_CNT = 0 ;
    m_u4TOTL_CNT = 0 ;
    m_u8CYC_BASE = 0 ;
    m_u4BIT_BASE = 0 ;
    m_u2COND_CNT = 0 ;
    m_u2LIM_CNT = 0 ;
    m_u1CYC_SIZE = 0 ;
    m_u1PMR_SIZE = 0 ;
    m_u1CHN_SIZE = 0 ;
    m_u1PAT_SIZE = 0 ;
    m_u1BIT_SIZE = 0 ;
    m_u1U1_SIZE = 0 ;
    m_u1U2_SIZE = 0 ;
    m_u1U3_SIZE = 0 ;
    m_u1UTX_SIZE = 0 ;
    m_u2CAP_BGN = 0 ;
    m_pu2LIM_INDX = 0 ;
    m_pu4LIM_SPEC = 0 ;
    m_pcnCOND_LST = 0;
    m_u2CYCO_CNT = 0 ;
    m_pu8CYC_OFST = 0 ;
    m_u2PMR_CNT = 0 ;
    m_pu8PMR_INDX = 0 ;
    m_u2CHN_CNT = 0 ;
    m_pu8CHN_NUM = 0 ;
    m_u2EXP_CNT = 0 ;
    m_pu1EXP_DATA = 0 ;
    m_u2CAP_CNT = 0 ;
    m_pu1CAP_DATA = 0 ;
    m_u2NEW_CNT = 0 ;
    m_pu1NEW_DATA = 0 ;
    m_u2PAT_CNT = 0 ;
    m_pu8PAT_NUM = 0 ;
    m_u2BPOS_CNT = 0 ;
    m_pu8BIT_POS = 0 ;
    m_u2USR1_CNT = 0 ;
    m_pu8USR1 = 0 ;
    m_u2USR2_CNT = 0 ;
    m_pu8USR2 = 0 ;
    m_u2USR3_CNT = 0 ;
    m_pu8USR3 = 0 ;
    m_u2TXT_CNT = 0 ;
    m_pcnUSER_TXT = 0 ;

    // Call Reset base method
    Stdf_Record::Reset();
}

bool Stdf_STR_V4::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;

    return false;
}

QString Stdf_STR_V4::GetRecordShortName(void)
{
    return "STR";
}

QString Stdf_STR_V4::GetRecordLongName(void)
{
    return "Scan Test Record";
}

int Stdf_STR_V4::GetRecordType(void)
{
    return Rec_STR;
}
stdf_type_u8 readType(const stdf_type_u1 &u1type, GS::StdLib::Stdf & clStdf, int &retVal)
{


    if(u1type == 1){
        stdf_type_u1 bData;
        retVal = clStdf.ReadByte(&bData);
        return bData;
    }
    else if(u1type == 2 ){
        int wData;
        retVal = clStdf.ReadWord(&wData);
        return wData;
    }
    else if(u1type == 4 ){
        long	dwData;
        retVal = clStdf.ReadDword(&dwData);
        return dwData;
    }
    else if(u1type == 8 ){
        unsigned long long	qwData;
        retVal = clStdf.ReadQword(&qwData);
        return qwData;
    }
    else{
        stdf_type_u1 bData;
        retVal = clStdf.ReadByte(&bData);
        return bData;
    }

}

int writeType(const stdf_type_u1 &u1type,stdf_type_u8 val ,GS::StdLib::Stdf & clStdf){


    if(u1type == 1){
    return clStdf.WriteByte((BYTE)val);
    }
    else if(u1type == 2 ){
    return clStdf.WriteWord((WORD)val);
    }
    else if(u1type == 4 ){
    return clStdf.WriteDword((DWORD)val);
    }
    else if(u1type == 8 ){
    return clStdf.WriteQWord(val);
    }
    else{
    return clStdf.WriteByte((BYTE)val);
    }

}
bool Stdf_STR_V4::Read(GS::StdLib::Stdf & clStdf)
{
    char	szString[STDF_MAX_U1+1];
    long	dwData;
    stdf_type_u1	bData;
    stdf_type_u8 qwData;
    int wData;
    unsigned int i;
    // First reset data
    Reset();

    _FIELD_CHECKREAD(clStdf.ReadByte	(	&bData	),	eposCONT_FLG	);
    _FIELD_SET(	m_b1CONT_FLG	=	stdf_type_b1	(	bData	),true,	eposCONT_FLG	);
    _FIELD_CHECKREAD(clStdf.ReadDword	(	&dwData	),	eposTEST_NUM	);
    _FIELD_SET(	m_u4TEST_NUM	=	stdf_type_u4	(	dwData	),true,	eposTEST_NUM	);
    _FIELD_CHECKREAD(clStdf.ReadByte	(	&bData	),	eposHEAD_NUM	);
    _FIELD_SET(	m_u1HEAD_NUM	=	stdf_type_u1	(	bData	),true,	eposHEAD_NUM	);
    _FIELD_CHECKREAD(clStdf.ReadByte	(	&bData	),	eposSITE_NUM	);
    _FIELD_SET(	m_u1SITE_NUM	=	stdf_type_u1	(	bData	),true,	eposSITE_NUM	);
    _FIELD_CHECKREAD(clStdf.ReadWord	(	&wData	),	eposPSR_REF	);
    _FIELD_SET(	m_u2PSR_REF	=	stdf_type_u2	(	wData	),true,	eposPSR_REF	);
    _FIELD_CHECKREAD(clStdf.ReadByte	(	&bData	),	eposTEST_FLG	);
    _FIELD_SET(	m_b1TEST_FLG	=	stdf_type_b1	(	bData	),true,	eposTEST_FLG	);
    _FIELD_CHECKREAD(clStdf.ReadString	(	szString	),	eposLOG_TYP	);
    _FIELD_SET(	m_cnLOG_TYP	=	stdf_type_cn	(	szString	),true,	eposLOG_TYP	);
    _FIELD_CHECKREAD(clStdf.ReadString	(	szString	),	eposTEST_TXT	);
    _FIELD_SET(	m_cnTEST_TXT	=	stdf_type_cn	(	szString	),true,	eposTEST_TXT	);
    _FIELD_CHECKREAD(clStdf.ReadString	(	szString	),	eposALARM_ID	);
    _FIELD_SET(	m_cnALARM_ID	=	stdf_type_cn	(	szString	),true,	eposALARM_ID	);
    _FIELD_CHECKREAD(clStdf.ReadString	(	szString	),	eposPROG_TXT	);
    _FIELD_SET(	m_cnPROG_TXT	=	stdf_type_cn	(	szString	),true,	eposPROG_TXT	);
    _FIELD_CHECKREAD(clStdf.ReadString	(	szString	),	eposRSLT_TXT	);
    _FIELD_SET(	m_cnRSLT_TXT	=	stdf_type_cn	(	szString	),true,	eposRSLT_TXT	);
    _FIELD_CHECKREAD(clStdf.ReadByte	(	&bData	),	eposZ_VAL	);
    _FIELD_SET(	m_u1Z_VAL	=	stdf_type_u1	(	bData	),true,	eposZ_VAL	);
    _FIELD_CHECKREAD(clStdf.ReadByte	(	&bData	),	eposFMU_FLG	);
    _FIELD_SET(	m_b1FMU_FLG	=	stdf_type_b1	(	bData	),true,	eposFMU_FLG	);


    if((m_b1FMU_FLG  & STDF_MASK_BIT0 ) && (m_b1FMU_FLG  & STDF_MASK_BIT1 )== 0){
        _FIELD_CHECKREAD(clStdf.ReadDBitField(&(m_dnMASK_MAP.m_uiLength), m_dnMASK_MAP.m_pBitField), eposMASK_MAP);
        _FIELD_SET(m_uiIndex_macro = 0, m_dnMASK_MAP.m_uiLength != 0, eposMASK_MAP);
    }

    if((m_b1FMU_FLG  & STDF_MASK_BIT2 ) && (m_b1FMU_FLG  & STDF_MASK_BIT3 )== 0){
        _FIELD_CHECKREAD(clStdf.ReadDBitField(&(m_dnFAL_MAP.m_uiLength), m_dnFAL_MAP.m_pBitField), eposFAL_MAP);
        _FIELD_SET(m_uiIndex_macro = 0, m_dnFAL_MAP.m_uiLength != 0, eposFAL_MAP);
    }

    _FIELD_CHECKREAD(clStdf.ReadQword(	&qwData	),	eposCYC_CNT	);
    _FIELD_SET(	m_u8CYC_CNT	=	stdf_type_u8	(	qwData	),true,	eposCYC_CNT	);
    _FIELD_CHECKREAD(clStdf.ReadDword	(	&dwData	),	eposTOTF_CNT	);
    _FIELD_SET(	m_u4TOTF_CNT	=	stdf_type_u4	(	dwData	),true,	eposTOTF_CNT	);
    _FIELD_CHECKREAD(clStdf.ReadDword	(	&dwData	),	eposTOTL_CNT	);
    _FIELD_SET(	m_u4TOTL_CNT	=	stdf_type_u4	(	dwData	),true,	eposTOTL_CNT	);
    _FIELD_CHECKREAD(clStdf.ReadQword	(	&qwData	),	eposCYC_BASE	);
    _FIELD_SET(	m_u8CYC_BASE	=	stdf_type_u8	(	qwData	),true,	eposCYC_BASE	);
    _FIELD_CHECKREAD(clStdf.ReadDword	(	&dwData	),	eposBIT_BASE	);
    _FIELD_SET(	m_u4BIT_BASE	=	stdf_type_u4	(	dwData	),true,	eposBIT_BASE	);
    _FIELD_CHECKREAD(clStdf.ReadWord	(	&wData	),	eposCOND_CNT	);
    _FIELD_SET(	m_u2COND_CNT	=	stdf_type_u2	(	wData	),true,	eposCOND_CNT	);
    _FIELD_CHECKREAD(clStdf.ReadWord	(	&wData	),	eposLIM_CNT	);
    _FIELD_SET(	m_u2LIM_CNT	=	stdf_type_u2	(	wData	),true,	eposLIM_CNT	);
    _FIELD_CHECKREAD(clStdf.ReadByte	(	&bData	),	eposCYC_SIZE	);
    _FIELD_SET(	m_u1CYC_SIZE	=	stdf_type_u1	(	bData	),true,	eposCYC_SIZE	);
    _FIELD_CHECKREAD(clStdf.ReadByte	(	&bData	),	eposPMR_SIZE	);
    _FIELD_SET(	m_u1PMR_SIZE	=	stdf_type_u1	(	bData	),true,	eposPMR_SIZE	);
    _FIELD_CHECKREAD(clStdf.ReadByte	(	&bData	),	eposCHN_SIZE	);
    _FIELD_SET(	m_u1CHN_SIZE	=	stdf_type_u1	(	bData	),true,	eposCHN_SIZE	);
    _FIELD_CHECKREAD(clStdf.ReadByte	(	&bData	),	eposPAT_SIZE	);
    _FIELD_SET(	m_u1PAT_SIZE	=	stdf_type_u1	(	bData	),true,	eposPAT_SIZE	);
    _FIELD_CHECKREAD(clStdf.ReadByte	(	&bData	),	eposBIT_SIZE	);
    _FIELD_SET(	m_u1BIT_SIZE	=	stdf_type_u1	(	bData	),true,	eposBIT_SIZE	);
    _FIELD_CHECKREAD(clStdf.ReadByte	(	&bData	),	eposU1_SIZE	);
    _FIELD_SET(	m_u1U1_SIZE	=	stdf_type_u1	(	bData	),true,	eposU1_SIZE	);
    _FIELD_CHECKREAD(clStdf.ReadByte	(	&bData	),	eposU2_SIZE	);
    _FIELD_SET(	m_u1U2_SIZE	=	stdf_type_u1	(	bData	),true,	eposU2_SIZE	);
    _FIELD_CHECKREAD(clStdf.ReadByte	(	&bData	),	eposU3_SIZE	);
    _FIELD_SET(	m_u1U3_SIZE	=	stdf_type_u1	(	bData	),true,	eposU3_SIZE	);
    _FIELD_CHECKREAD(clStdf.ReadByte	(	&bData	),	eposUTX_SIZE	);
    _FIELD_SET(	m_u1UTX_SIZE	=	stdf_type_u1	(	bData	),true,	eposUTX_SIZE	);
    _FIELD_CHECKREAD(clStdf.ReadWord	(	&wData	),	eposCAP_BGN	);
    _FIELD_SET(	m_u2CAP_BGN	=	stdf_type_u2	(	wData	),true,	eposCAP_BGN	);

    if(m_u2LIM_CNT > 0)
        m_pu2LIM_INDX = new stdf_type_u2[m_u2LIM_CNT];
    for(i=0; i<m_u2LIM_CNT; i++)
    {
        _FIELD_CHECKREAD(clStdf.ReadWord(&wData), eposLIM_INDX);
        _FIELD_SET(m_pu2LIM_INDX[i] = wData, true, eposLIM_INDX);
    }

    if(m_u2LIM_CNT > 0)
        m_pu4LIM_SPEC = new stdf_type_u4[m_u2LIM_CNT];
    for(i=0; i<m_u2LIM_CNT; i++)
    {
        _FIELD_CHECKREAD(clStdf.ReadDword(&dwData), eposLIM_SPEC);
        _FIELD_SET(m_pu4LIM_SPEC[i] = wData, true, eposLIM_SPEC);
    }

    if(m_u2COND_CNT > 0)
        m_pcnCOND_LST = new stdf_type_cn[m_u2COND_CNT];
    for(i=0; i<m_u2COND_CNT; i++)
    {
        _FIELD_CHECKREAD(clStdf.ReadString(szString), eposCOND_LST);
        _FIELD_SET(m_pcnCOND_LST[i] = szString, true, eposCOND_LST);
    }

    _FIELD_CHECKREAD(clStdf.ReadWord	(	&wData	),	eposCYCO_CNT	);
    _FIELD_SET(	m_u2CYCO_CNT	=	stdf_type_u2	(	wData	),true,	eposCYCO_CNT	);
    if(m_u2CYCO_CNT > 0)
        m_pu8CYC_OFST = new stdf_type_u8[m_u2CYCO_CNT];
    for(i=0; i<m_u2CYCO_CNT; i++)
    {
        int retVal = GS::StdLib::Stdf::NoError;
        dwData = readType(m_u1CYC_SIZE,clStdf, retVal);
        _FIELD_CHECKREAD(retVal, eposCYC_OFST);
        _FIELD_SET(m_pu8CYC_OFST[i] = dwData, true, eposCYC_OFST);
    }



    _FIELD_CHECKREAD(clStdf.ReadWord	(	&wData	),	eposPMR_CNT	);
    _FIELD_SET(	m_u2PMR_CNT	=	stdf_type_u2	(wData),true,	eposPMR_CNT	);
    if(m_u2PMR_CNT > 0)
        m_pu8PMR_INDX = new stdf_type_u8[m_u2PMR_CNT];
    for(i=0; i<m_u2PMR_CNT; i++)
    {
        int retVal = GS::StdLib::Stdf::NoError;
        dwData = readType(m_u1PMR_SIZE,clStdf,retVal);
        _FIELD_CHECKREAD(retVal, eposPMR_INDX);
        _FIELD_SET(m_pu8PMR_INDX[i] = dwData, true, eposPMR_INDX);
    }


    _FIELD_CHECKREAD(clStdf.ReadWord	(	&wData	),	eposCHN_CNT	);
    _FIELD_SET(	m_u2CHN_CNT	=	stdf_type_u2	(	wData	),true,	eposCHN_CNT	);
    if(m_u2CHN_CNT > 0)
        m_pu8CHN_NUM = new stdf_type_u8[m_u2CHN_CNT];
    for(i=0; i<m_u2CHN_CNT; i++)
    {
        int retVal =  GS::StdLib::Stdf::NoError;
        dwData = readType(m_u1CHN_SIZE,clStdf, retVal);
        _FIELD_CHECKREAD(retVal, eposCHN_NUM);
        _FIELD_SET(m_pu8CHN_NUM[i] = dwData, true, eposCHN_NUM);
    }


    _FIELD_CHECKREAD(clStdf.ReadWord	(	&wData	),	eposEXP_CNT	);
    _FIELD_SET(	m_u2EXP_CNT	=	stdf_type_u2	(	wData	),true,	eposEXP_CNT	);
    if(m_u2EXP_CNT > 0)
        m_pu1EXP_DATA = new stdf_type_u1[m_u2EXP_CNT];
    for(i=0; i<m_u2EXP_CNT; i++)
    {
        _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposEXP_DATA);
        _FIELD_SET(m_pu1EXP_DATA[i] = bData, true, eposEXP_DATA);
    }

    _FIELD_CHECKREAD(clStdf.ReadWord	(	&wData	),	eposCAP_CNT	);
    _FIELD_SET(	m_u2CAP_CNT	=	stdf_type_u2	(	wData	),true,	eposCAP_CNT	);
    if(m_u2CAP_CNT > 0)
        m_pu1CAP_DATA = new stdf_type_u1[m_u2CAP_CNT];
    for(i=0; i<m_u2CAP_CNT; i++)
    {
        _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposCAP_DATA);
        _FIELD_SET(m_pu1CAP_DATA[i] = bData, true, eposCAP_DATA);
    }

    _FIELD_CHECKREAD(clStdf.ReadWord	(	&wData	),	eposNEW_CNT	);
    _FIELD_SET(	m_u2NEW_CNT	=	stdf_type_u2	(	wData	),true,	eposNEW_CNT	);
    if(m_u2NEW_CNT > 0)
        m_pu1NEW_DATA = new stdf_type_u1[m_u2NEW_CNT];
    for(i=0; i<m_u2NEW_CNT; i++)
    {
        _FIELD_CHECKREAD(clStdf.ReadByte(&bData), eposNEW_DATA);
        _FIELD_SET(m_pu1NEW_DATA[i] = bData, true, eposNEW_DATA);
    }

    _FIELD_CHECKREAD(clStdf.ReadWord	(	&wData	),	eposPAT_CNT	);
    _FIELD_SET(	m_u2PAT_CNT	=	stdf_type_u2	(	wData	),true,	eposPAT_CNT	);
    if(m_u2PAT_CNT > 0)
        m_pu8PAT_NUM = new stdf_type_u8[m_u2PAT_CNT];
    for(i=0; i<m_u2PAT_CNT; i++)
    {
        int retVal = GS::StdLib::Stdf::NoError;
        dwData = readType(m_u1PAT_SIZE,clStdf, retVal);
        _FIELD_CHECKREAD(retVal, eposPAT_NUM);
        _FIELD_SET(m_pu8PAT_NUM[i] = dwData, true, eposPAT_NUM);
    }


    _FIELD_CHECKREAD(clStdf.ReadWord	(	&wData	),	eposBPOS_CNT	);
    _FIELD_SET(	m_u2BPOS_CNT	=	stdf_type_u2	(	wData	),true,	eposBPOS_CNT	);
    if(m_u2BPOS_CNT > 0)
        m_pu8BIT_POS = new stdf_type_u8[m_u2BPOS_CNT];
    for(i=0; i<m_u2BPOS_CNT; i++)
    {
        int retVal = GS::StdLib::Stdf::NoError;
        dwData = readType(m_u1BIT_SIZE,clStdf, retVal);
        _FIELD_CHECKREAD(retVal, eposBIT_POS);
        _FIELD_SET(m_pu8BIT_POS[i] = dwData, true, eposBIT_POS);
    }

    _FIELD_CHECKREAD(clStdf.ReadWord	(	&wData	),	eposUSR1_CNT	);
    _FIELD_SET(	m_u2USR1_CNT	=	stdf_type_u2	(	wData	),true,	eposUSR1_CNT	);
    if(m_u2USR1_CNT > 0)
        m_pu8USR1 = new stdf_type_u8[m_u2USR1_CNT];
    for(i=0; i<m_u2USR1_CNT; i++)
    {
        int retVal = GS::StdLib::Stdf::NoError;
        dwData = readType(m_u1U1_SIZE,clStdf, retVal);
        _FIELD_CHECKREAD(retVal, eposUSR1);
        _FIELD_SET(m_pu8USR1[i] = dwData, true, eposUSR1);
    }


    _FIELD_CHECKREAD(clStdf.ReadWord	(	&wData	),	eposUSR2_CNT	);
    _FIELD_SET(	m_u2USR2_CNT	=	stdf_type_u2	(	wData	),true,	eposUSR2_CNT	);
    if(m_u2USR2_CNT > 0)
        m_pu8USR2 = new stdf_type_u8[m_u2USR2_CNT];
    for(i=0; i<m_u2USR2_CNT; i++)
    {
        int retVal = GS::StdLib::Stdf::NoError;
        dwData = readType(m_u1U2_SIZE,clStdf,retVal);
        _FIELD_CHECKREAD(retVal, eposUSR2);
        _FIELD_SET(m_pu8USR2[i] = dwData, true, eposUSR2);
    }

    _FIELD_CHECKREAD(clStdf.ReadWord	(	&wData	),	eposUSR3_CNT	);
    _FIELD_SET(	m_u2USR3_CNT	=	stdf_type_u2	(	wData	),true,	eposUSR3_CNT	);
    if(m_u2USR3_CNT > 0)
        m_pu8USR3 = new stdf_type_u8[m_u2USR3_CNT];
    for(i=0; i<m_u2USR3_CNT; i++)
    {
        int retVal = GS::StdLib::Stdf::NoError;
        dwData = readType(m_u1U3_SIZE,clStdf,retVal);
        _FIELD_CHECKREAD(retVal, eposUSR3);
        _FIELD_SET(m_pu8USR3[i] = dwData, true, eposUSR3);
    }

    _FIELD_CHECKREAD(clStdf.ReadWord	(	&wData	),	eposTXT_CNT	);
    _FIELD_SET(	m_u2TXT_CNT	=	stdf_type_u2	(	wData	),true,	eposTXT_CNT	);
    if(m_u2TXT_CNT > 0)
        m_pcnUSER_TXT = new stdf_type_cn[m_u2TXT_CNT];
    for(i=0; i<m_u2TXT_CNT; i++)
    {
        _FIELD_CHECKREAD(clStdf.ReadCFString(m_u1UTX_SIZE, szString), eposUSER_TXT);//DONT FORGET m_u1UTX_SIZE
        _FIELD_SET(m_pcnUSER_TXT[i] = szString, true, eposUSER_TXT);
    }


    return true;
}

bool Stdf_STR_V4::Write(GS::StdLib::Stdf & clStdf)
{
    unsigned int i;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;

    RecordReadInfo.iRecordType = STDF_STR_TYPE;
    RecordReadInfo.iRecordSubType = STDF_STR_STYPE;
    clStdf.WriteHeader(&RecordReadInfo);

    _FIELD_CHECKWRITE(clStdf.WriteByte	(	m_b1CONT_FLG	),	eposCONT_FLG	,clStdf.WriteRecord());
    _FIELD_CHECKWRITE(clStdf.WriteDword	(	m_u4TEST_NUM	),	eposTEST_NUM	,clStdf.WriteRecord());
    _FIELD_CHECKWRITE(clStdf.WriteByte	(	m_u1HEAD_NUM	),	eposHEAD_NUM	,clStdf.WriteRecord());
    _FIELD_CHECKWRITE(clStdf.WriteByte	(	m_u1SITE_NUM	),	eposSITE_NUM	,clStdf.WriteRecord());
    _FIELD_CHECKWRITE(clStdf.WriteWord	(	m_u2PSR_REF	),	eposPSR_REF		,clStdf.WriteRecord());
    _FIELD_CHECKWRITE(clStdf.WriteByte	(	m_b1TEST_FLG	),	eposTEST_FLG	,clStdf.WriteRecord());
    _FIELD_CHECKWRITE(clStdf.WriteString	(	m_cnLOG_TYP.toLatin1().data()	),	eposLOG_TYP		,clStdf.WriteRecord());
    _FIELD_CHECKWRITE(clStdf.WriteString	(	m_cnTEST_TXT.toLatin1().data()	),	eposTEST_TXT	,clStdf.WriteRecord());
    _FIELD_CHECKWRITE(clStdf.WriteString	(	m_cnALARM_ID.toLatin1().data()	),	eposALARM_ID	,clStdf.WriteRecord());
    _FIELD_CHECKWRITE(clStdf.WriteString	(	m_cnPROG_TXT.toLatin1().data()	),	eposPROG_TXT	,clStdf.WriteRecord());
    _FIELD_CHECKWRITE(clStdf.WriteString	(	m_cnRSLT_TXT.toLatin1().data()	),	eposRSLT_TXT	,clStdf.WriteRecord());
    _FIELD_CHECKWRITE(clStdf.WriteByte	(	m_u1Z_VAL	),	eposZ_VAL		,clStdf.WriteRecord());
    _FIELD_CHECKWRITE(clStdf.WriteByte	(	m_b1FMU_FLG	),	eposFMU_FLG		,clStdf.WriteRecord());

    if((m_b1FMU_FLG  & STDF_MASK_BIT0 )  && (m_b1FMU_FLG  & STDF_MASK_BIT1 )== 0)
    {
        _FIELD_CHECKWRITE(clStdf.WriteDBitField(BYTE(m_dnMASK_MAP.m_uiLength), m_dnMASK_MAP.m_pBitField), eposMASK_MAP, clStdf.WriteRecord());
    }

    if((m_b1FMU_FLG  & STDF_MASK_BIT2 )  && (m_b1FMU_FLG  & STDF_MASK_BIT3 )== 0)
    {
        _FIELD_CHECKWRITE(clStdf.WriteDBitField(BYTE(m_dnFAL_MAP.m_uiLength), m_dnFAL_MAP.m_pBitField), eposFAL_MAP, clStdf.WriteRecord());
    }

    _FIELD_CHECKWRITE(clStdf.WriteQWord	(	m_u8CYC_CNT	),	eposCYC_CNT		,clStdf.WriteRecord());
    _FIELD_CHECKWRITE(clStdf.WriteDword	(	m_u4TOTF_CNT	),	eposTOTF_CNT	,clStdf.WriteRecord());
    _FIELD_CHECKWRITE(clStdf.WriteDword	(	m_u4TOTL_CNT	),	eposTOTL_CNT	,clStdf.WriteRecord());
    _FIELD_CHECKWRITE(clStdf.WriteQWord	(	m_u8CYC_BASE	),	eposCYC_BASE	,clStdf.WriteRecord());
    _FIELD_CHECKWRITE(clStdf.WriteDword	(	m_u4BIT_BASE	),	eposBIT_BASE	,clStdf.WriteRecord());
    _FIELD_CHECKWRITE(clStdf.WriteWord	(	m_u2COND_CNT	),	eposCOND_CNT	,clStdf.WriteRecord());
    _FIELD_CHECKWRITE(clStdf.WriteWord	(	m_u2LIM_CNT	),	eposLIM_CNT		,clStdf.WriteRecord());
    _FIELD_CHECKWRITE(clStdf.WriteByte	(	m_u1CYC_SIZE	),	eposCYC_SIZE	,clStdf.WriteRecord());
    _FIELD_CHECKWRITE(clStdf.WriteByte	(	m_u1PMR_SIZE	),	eposPMR_SIZE	,clStdf.WriteRecord());
    _FIELD_CHECKWRITE(clStdf.WriteByte	(	m_u1CHN_SIZE	),	eposCHN_SIZE	,clStdf.WriteRecord());
    _FIELD_CHECKWRITE(clStdf.WriteByte	(	m_u1PAT_SIZE	),	eposPAT_SIZE	,clStdf.WriteRecord());
    _FIELD_CHECKWRITE(clStdf.WriteByte	(	m_u1BIT_SIZE	),	eposBIT_SIZE	,clStdf.WriteRecord());
    _FIELD_CHECKWRITE(clStdf.WriteByte	(	m_u1U1_SIZE	),	eposU1_SIZE		,clStdf.WriteRecord());
    _FIELD_CHECKWRITE(clStdf.WriteByte	(	m_u1U2_SIZE	),	eposU2_SIZE		,clStdf.WriteRecord());
    _FIELD_CHECKWRITE(clStdf.WriteByte	(	m_u1U3_SIZE	),	eposU3_SIZE		,clStdf.WriteRecord());
    _FIELD_CHECKWRITE(clStdf.WriteByte	(	m_u1UTX_SIZE	),	eposUTX_SIZE	,clStdf.WriteRecord());
    _FIELD_CHECKWRITE(clStdf.WriteWord	(	m_u2CAP_BGN	),	eposCAP_BGN		,clStdf.WriteRecord());

    for(i=0; i<(unsigned int)m_u2LIM_CNT; i++)
        _FIELD_CHECKWRITE(clStdf.WriteWord(WORD(m_pu2LIM_INDX[i])), eposLIM_INDX, clStdf.WriteRecord());

    for(i=0; i<(unsigned int)m_u2LIM_CNT; i++)
        _FIELD_CHECKWRITE(clStdf.WriteWord(DWORD(m_pu4LIM_SPEC[i])), eposLIM_SPEC, clStdf.WriteRecord());

    for(i=0; i<(unsigned int)m_u2COND_CNT; i++)
        _FIELD_CHECKWRITE(clStdf.WriteString	(	m_pcnCOND_LST[i].toLatin1().data()	),	eposCOND_LST	,clStdf.WriteRecord());


    _FIELD_CHECKWRITE(clStdf.WriteWord	(	m_u2CYCO_CNT	),	eposCYCO_CNT	,clStdf.WriteRecord());
    for(i=0; i<(unsigned int)m_u2CYCO_CNT; i++)
        _FIELD_CHECKWRITE(writeType(m_u1CYC_SIZE,m_pu8CYC_OFST[i],clStdf),   eposCYC_OFST ,clStdf.WriteRecord());


    _FIELD_CHECKWRITE(clStdf.WriteWord	(	m_u2PMR_CNT	),	eposPMR_CNT	,clStdf.WriteRecord());
    for(i=0; i<(unsigned int)m_u2PMR_CNT; i++)
        _FIELD_CHECKWRITE(writeType(m_u1PMR_SIZE, m_pu8PMR_INDX[i],clStdf),   eposPMR_INDX ,clStdf.WriteRecord());

    _FIELD_CHECKWRITE(clStdf.WriteWord	(	m_u2CHN_CNT	),	eposCHN_CNT	,clStdf.WriteRecord());
    for(i=0; i<(unsigned int)m_u2CHN_CNT; i++)
        _FIELD_CHECKWRITE(writeType(m_u1CHN_SIZE, m_pu8CHN_NUM[i],clStdf),   eposCHN_NUM ,clStdf.WriteRecord());


    _FIELD_CHECKWRITE(clStdf.WriteWord	(	m_u2EXP_CNT	),	eposEXP_CNT	,clStdf.WriteRecord());
    for(i=0; i<(unsigned int)m_u2EXP_CNT; i++)
        _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_pu1EXP_DATA[i])), eposEXP_DATA, clStdf.WriteRecord());


    _FIELD_CHECKWRITE(clStdf.WriteWord	(	m_u2CAP_CNT	),	eposCAP_CNT	,clStdf.WriteRecord());
    for(i=0; i<(unsigned int)m_u2CAP_CNT; i++)
        _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_pu1CAP_DATA[i])), eposCAP_DATA, clStdf.WriteRecord());


    _FIELD_CHECKWRITE(clStdf.WriteWord	(	m_u2NEW_CNT	),	eposNEW_CNT	,clStdf.WriteRecord());
    for(i=0; i<(unsigned int)m_u2NEW_CNT; i++)
        _FIELD_CHECKWRITE(clStdf.WriteByte(BYTE(m_pu1NEW_DATA[i])), eposNEW_DATA, clStdf.WriteRecord());

    _FIELD_CHECKWRITE(clStdf.WriteWord	(	m_u2PAT_CNT	),	eposPAT_CNT	,clStdf.WriteRecord());
    for(i=0; i<(unsigned int)m_u2PAT_CNT; i++)
        _FIELD_CHECKWRITE(writeType(m_u1PAT_SIZE,m_pu8PAT_NUM[i],clStdf),   eposPAT_NUM ,clStdf.WriteRecord());


    _FIELD_CHECKWRITE(clStdf.WriteWord	(	m_u2BPOS_CNT	),	eposBPOS_CNT	,clStdf.WriteRecord());
    for(i=0; i<(unsigned int)m_u2BPOS_CNT; i++)
        _FIELD_CHECKWRITE(writeType(m_u1BIT_SIZE,m_pu8BIT_POS[i],clStdf),   eposBIT_POS ,clStdf.WriteRecord());

    _FIELD_CHECKWRITE(clStdf.WriteWord	(	m_u2USR1_CNT	),	eposUSR1_CNT	,clStdf.WriteRecord());
    for(i=0; i<(unsigned int)m_u2USR1_CNT; i++)
        _FIELD_CHECKWRITE(writeType(m_u1U1_SIZE,m_pu8USR1[i],clStdf),   eposUSR1 ,clStdf.WriteRecord());


    _FIELD_CHECKWRITE(clStdf.WriteWord	(	m_u2USR2_CNT	),	eposUSR2_CNT	,clStdf.WriteRecord());
    for(i=0; i<(unsigned int)m_u2USR2_CNT; i++)
        _FIELD_CHECKWRITE(writeType(m_u1U2_SIZE,m_pu8USR2[i],clStdf),   eposUSR2 ,clStdf.WriteRecord());

    _FIELD_CHECKWRITE(clStdf.WriteWord	(	m_u2USR3_CNT	),	eposUSR3_CNT	,clStdf.WriteRecord());
    for(i=0; i<(unsigned int)m_u2USR3_CNT; i++)
        _FIELD_CHECKWRITE(writeType(m_u1U3_SIZE,m_pu8USR3[i],clStdf),   eposUSR3 ,clStdf.WriteRecord());

    _FIELD_CHECKWRITE(clStdf.WriteWord	(	m_u2TXT_CNT	),	eposTXT_CNT	,clStdf.WriteRecord());
    for(i=0; i<(unsigned int)m_u2TXT_CNT; i++)
        _FIELD_CHECKWRITE(clStdf.WriteCFString(m_pcnUSER_TXT[i].toLatin1().data()),   eposUSER_TXT,clStdf.WriteRecord());

    clStdf.WriteRecord();

    return true;
}

void Stdf_STR_V4::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    // Empty string first
    strAsciiString = "";



    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.CONT_FLG"	,QString::number(m_b1CONT_FLG)	,nFieldSelection,	eposCONT_FLG	);
    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.TEST_NUM"	,QString::number(m_u4TEST_NUM)	,nFieldSelection,	eposTEST_NUM	);
    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.HEAD_NUM"	,QString::number(m_u1HEAD_NUM)	,nFieldSelection,	eposHEAD_NUM	);
    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.SITE_NUM"	,QString::number(m_u1SITE_NUM)	,nFieldSelection,	eposSITE_NUM	);
    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.PSR_REF"	,QString::number(m_u2PSR_REF),nFieldSelection,	eposPSR_REF	);

    _CREATEFIELD_FROM_B1_ASCII(m_b1TEST_FLG);
    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.TEST_FLG"	,m_strFieldValue_macro	,nFieldSelection,	eposTEST_FLG	);

    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.LOG_TYP"	,m_cnLOG_TYP	,nFieldSelection,	eposLOG_TYP	);
    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.TEST_TXT"	,m_cnTEST_TXT	,nFieldSelection,	eposTEST_TXT	);
    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.ALARM_ID"	,m_cnALARM_ID	,nFieldSelection,	eposALARM_ID	);
    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.PROG_TXT"	,m_cnPROG_TXT	,nFieldSelection,	eposPROG_TXT	);
    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.RSLT_TXT"	,m_cnRSLT_TXT	,nFieldSelection,	eposRSLT_TXT	);
    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.Z_VAL"	,QString::number(m_u1Z_VAL)	,nFieldSelection,	eposZ_VAL	);
    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.FMU_FLG"	,QString::number(m_b1FMU_FLG)	,nFieldSelection,	eposFMU_FLG	);

    if((m_b1FMU_FLG  & STDF_MASK_BIT0) && (m_b1FMU_FLG  & STDF_MASK_BIT1 ) == 0)
    {
        _CREATEFIELD_FROM_DN_ASCII(m_dnMASK_MAP);
        _STR_ADDFIELD_ASCII(strAsciiString, "STR.MASK_MAP", m_strFieldValue_macro, nFieldSelection, eposMASK_MAP);
    }

    if((m_b1FMU_FLG  & STDF_MASK_BIT2) && (m_b1FMU_FLG  & STDF_MASK_BIT3 )== 0)
    {
        _CREATEFIELD_FROM_DN_ASCII(m_dnFAL_MAP);
        _STR_ADDFIELD_ASCII(strAsciiString, "STR.FAL_MAP", m_strFieldValue_macro, nFieldSelection, eposFAL_MAP);
    }

    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.CYC_CNT"	,QString::number(m_u8CYC_CNT)	,nFieldSelection,	eposCYC_CNT	);
    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.TOTF_CNT"	,QString::number(m_u4TOTF_CNT)	,nFieldSelection,	eposTOTF_CNT	);
    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.TOTL_CNT"	,QString::number(m_u4TOTL_CNT)	,nFieldSelection,	eposTOTL_CNT	);
    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.CYC_BASE"	,QString::number(m_u8CYC_BASE)	,nFieldSelection,	eposCYC_BASE	);
    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.BIT_BASE"	,QString::number(m_u4BIT_BASE)	,nFieldSelection,	eposBIT_BASE	);
    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.COND_CNT"	,QString::number(m_u2COND_CNT)	,nFieldSelection,	eposCOND_CNT	);
    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.LIM_CNT"	,QString::number(m_u2LIM_CNT)	,nFieldSelection,	eposLIM_CNT	);
    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.CYC_SIZE"	,QString::number(m_u1CYC_SIZE)	,nFieldSelection,	eposCYC_SIZE	);
    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.PMR_SIZE"	,QString::number(m_u1PMR_SIZE)	,nFieldSelection,	eposPMR_SIZE	);
    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.CHN_SIZE"	,QString::number(m_u1CHN_SIZE)	,nFieldSelection,	eposCHN_SIZE	);
    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.PAT_SIZE"	,QString::number(m_u1PAT_SIZE)	,nFieldSelection,	eposPAT_SIZE	);
    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.BIT_SIZE"	,QString::number(m_u1BIT_SIZE)	,nFieldSelection,	eposBIT_SIZE	);
    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.U1_SIZE"	,QString::number(m_u1U1_SIZE)	,nFieldSelection,	eposU1_SIZE	);
    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.U2_SIZE"	,QString::number(m_u1U2_SIZE)	,nFieldSelection,	eposU2_SIZE	);
    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.U3_SIZE"	,QString::number(m_u1U3_SIZE)	,nFieldSelection,	eposU3_SIZE	);
    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.UTX_SIZE"	,QString::number(m_u1UTX_SIZE)	,nFieldSelection,	eposUTX_SIZE	);
    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.CAP_BGN"	,QString::number(m_u2CAP_BGN)	,nFieldSelection,	eposCAP_BGN	);


    //m_u2INDX_CNT, m_ku2PMR_INDX, eposPMR_INDX
    _CREATEFIELD_FROM_KUi_ASCII(m_u2LIM_CNT, m_pu2LIM_INDX,eposLIM_INDX);
    _STR_ADDFIELD_ASCII(strAsciiString, "STR.LIM_INDX", m_strFieldValue_macro, nFieldSelection, eposLIM_INDX);

    _CREATEFIELD_FROM_KU8_ASCII(m_u2LIM_CNT,m_pu4LIM_SPEC,eposLIM_SPEC);
    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.LIM_SPEC"	,	m_strFieldValue_macro	,nFieldSelection,	eposLIM_SPEC	);

    _CREATEFIELD_FROM_KCN_ASCII(m_u2COND_CNT,m_pcnCOND_LST,eposCOND_LST);
    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.COND_LST"	,	m_strFieldValue_macro	,nFieldSelection,	eposCOND_LST	);

    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.CYCO_CNT"	,QString::number(m_u2CYCO_CNT)	,nFieldSelection,	eposCYCO_CNT	);
    _CREATEFIELD_FROM_KU8_ASCII(m_u2CYCO_CNT,m_pu8CYC_OFST,eposCYC_OFST);
    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.CYC_OFST"	,	m_strFieldValue_macro,	nFieldSelection,eposCYC_OFST	);

    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.PMR_CNT"	,QString::number(m_u2PMR_CNT)	,nFieldSelection,	eposPMR_CNT	);
    _CREATEFIELD_FROM_KU8_ASCII(m_u2PMR_CNT,m_pu8PMR_INDX,eposPMR_INDX);
    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.PMR_INDX"	,	m_strFieldValue_macro, nFieldSelection, eposPMR_INDX	);

    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.CHN_CNT"	,QString::number(m_u2CHN_CNT)	,nFieldSelection,	eposCHN_CNT	);
    _CREATEFIELD_FROM_KU8_ASCII(m_u2CHN_CNT,m_pu8CHN_NUM,eposCHN_NUM);
    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.CHN_NUM"	,	m_strFieldValue_macro	,nFieldSelection,	eposCHN_NUM	);

    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.EXP_CNT"	,QString::number(m_u2EXP_CNT)	,nFieldSelection,	eposEXP_CNT	);
    _CREATEFIELD_FROM_KUi_ASCII(m_u2EXP_CNT,m_pu1EXP_DATA,eposEXP_DATA);
    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.EXP_DATA"	,	m_strFieldValue_macro	,nFieldSelection,	eposEXP_DATA	);

    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.CAP_CNT"	,QString::number(m_u2CAP_CNT)	,nFieldSelection,	eposCAP_CNT	);
    _CREATEFIELD_FROM_KUi_ASCII(m_u2CAP_CNT,m_pu1CAP_DATA,eposCAP_DATA);
    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.CAP_DATA"	,	m_strFieldValue_macro	,nFieldSelection,	eposCAP_DATA	);

    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.NEW_CNT"	,QString::number(m_u2NEW_CNT)	,nFieldSelection,	eposNEW_CNT	);
    _CREATEFIELD_FROM_KUi_ASCII(m_u2NEW_CNT,m_pu1NEW_DATA,eposNEW_DATA);
    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.NEW_DATA"	,	m_strFieldValue_macro	,nFieldSelection,	eposNEW_DATA	);

    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.PAT_CNT"	,QString::number(m_u2PAT_CNT)	,nFieldSelection,	eposPAT_CNT	);
    _CREATEFIELD_FROM_KU8_ASCII(m_u2PAT_CNT,m_pu8PAT_NUM,eposPAT_NUM);
    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.PAT_NUM"	,	m_strFieldValue_macro	,nFieldSelection,	eposPAT_NUM	);

    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.BPOS_CNT"	,QString::number(m_u2BPOS_CNT)	,nFieldSelection,	eposBPOS_CNT	);
    _CREATEFIELD_FROM_KU8_ASCII(m_u2BPOS_CNT,m_pu8BIT_POS,eposBIT_POS);
    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.BIT_POS"	,	m_strFieldValue_macro	,nFieldSelection,	eposBIT_POS	);

    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.USR1_CNT"	,QString::number(m_u2USR1_CNT)	,nFieldSelection,	eposUSR1_CNT	);
    _CREATEFIELD_FROM_KU8_ASCII(m_u2USR1_CNT,m_pu8USR1,eposUSR1);
    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.USR1"	,	m_strFieldValue_macro	,nFieldSelection,	eposUSR1	);

    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.USR2_CNT"	,QString::number(m_u2USR2_CNT)	,nFieldSelection,	eposUSR2_CNT	);
    _CREATEFIELD_FROM_KU8_ASCII(m_u2USR2_CNT,m_pu8USR2,eposUSR2);
    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.USR2"	,	m_strFieldValue_macro	,nFieldSelection,	eposUSR2	);

    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.USR3_CNT"	,QString::number(m_u2USR3_CNT)	,nFieldSelection,	eposUSR3_CNT	);
    _CREATEFIELD_FROM_KU8_ASCII(m_u2USR3_CNT,m_pu8USR3,eposUSR3);
    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.USR3"	,	m_strFieldValue_macro	,nFieldSelection,	eposUSR3	);


    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.TXT_CNT"	,QString::number(m_u2TXT_CNT)	,nFieldSelection,	eposTXT_CNT	);
    _CREATEFIELD_FROM_KCN_ASCII(m_u2TXT_CNT,m_pcnUSER_TXT,eposUSER_TXT);
    _STR_ADDFIELD_ASCII(strAsciiString,	"STR.USER_TXT"	,	m_strFieldValue_macro	,nFieldSelection,	eposUSER_TXT	);



}

void Stdf_STR_V4::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{

    // Empty string list first
    listFields.empty();

    _LIST_ADDFIELD_ASCII(listFields,	"STR.CONT_FLG"	,QString::number(m_b1CONT_FLG)	,nFieldSelection,	eposCONT_FLG	);
    _LIST_ADDFIELD_ASCII(listFields,	"STR.TEST_NUM"	,QString::number(m_u4TEST_NUM)	,nFieldSelection,	eposTEST_NUM	);
    _LIST_ADDFIELD_ASCII(listFields,	"STR.HEAD_NUM"	,QString::number(m_u1HEAD_NUM)	,nFieldSelection,	eposHEAD_NUM	);
    _LIST_ADDFIELD_ASCII(listFields,	"STR.SITE_NUM"	,QString::number(m_u1SITE_NUM)	,nFieldSelection,	eposSITE_NUM	);
    _LIST_ADDFIELD_ASCII(listFields,	"STR.PSR_REF"	,QString::number(m_u2PSR_REF),nFieldSelection,	eposPSR_REF	);
    _LIST_ADDFIELD_ASCII(listFields,	"STR.TEST_FLG"	,QString::number(m_b1TEST_FLG)	,nFieldSelection,	eposTEST_FLG	);
    _LIST_ADDFIELD_ASCII(listFields,	"STR.LOG_TYP"	,m_cnLOG_TYP	,nFieldSelection,	eposLOG_TYP	);
    _LIST_ADDFIELD_ASCII(listFields,	"STR.TEST_TXT"	,m_cnTEST_TXT	,nFieldSelection,	eposTEST_TXT	);
    _LIST_ADDFIELD_ASCII(listFields,	"STR.ALARM_ID"	,m_cnALARM_ID	,nFieldSelection,	eposALARM_ID	);
    _LIST_ADDFIELD_ASCII(listFields,	"STR.PROG_TXT"	,m_cnPROG_TXT	,nFieldSelection,	eposPROG_TXT	);
    _LIST_ADDFIELD_ASCII(listFields,	"STR.RSLT_TXT"	,m_cnRSLT_TXT	,nFieldSelection,	eposRSLT_TXT	);
    _LIST_ADDFIELD_ASCII(listFields,	"STR.Z_VAL"	,QString::number(m_u1Z_VAL)	,nFieldSelection,	eposZ_VAL	);
    _LIST_ADDFIELD_ASCII(listFields,	"STR.FMU_FLG"	,QString::number(m_b1FMU_FLG)	,nFieldSelection,	eposFMU_FLG	);

    if((m_b1FMU_FLG  & STDF_MASK_BIT0 ) && (m_b1FMU_FLG  & STDF_MASK_BIT1 )== 0)
    {
        _CREATEFIELD_FROM_DN_ASCII(m_dnMASK_MAP);
        _LIST_ADDFIELD_ASCII(listFields, "STR.MASK_MAP", m_strFieldValue_macro, nFieldSelection, eposMASK_MAP)
    }
    if((m_b1FMU_FLG  & STDF_MASK_BIT2 ) && (m_b1FMU_FLG  & STDF_MASK_BIT3 )== 0)
    {
        _CREATEFIELD_FROM_DN_ASCII(m_dnFAL_MAP);
        _LIST_ADDFIELD_ASCII(listFields, "STR.FAL_MAP", m_strFieldValue_macro, nFieldSelection, eposFAL_MAP);
    }
    _LIST_ADDFIELD_ASCII(listFields,	"STR.CYC_CNT"	,QString::number(m_u8CYC_CNT)	,nFieldSelection,	eposCYC_CNT	);
    _LIST_ADDFIELD_ASCII(listFields,	"STR.TOTF_CNT"	,QString::number(m_u4TOTF_CNT)	,nFieldSelection,	eposTOTF_CNT	);
    _LIST_ADDFIELD_ASCII(listFields,	"STR.TOTL_CNT"	,QString::number(m_u4TOTL_CNT)	,nFieldSelection,	eposTOTL_CNT	);
    _LIST_ADDFIELD_ASCII(listFields,	"STR.CYC_BASE"	,QString::number(m_u8CYC_BASE)	,nFieldSelection,	eposCYC_BASE	);
    _LIST_ADDFIELD_ASCII(listFields,	"STR.BIT_BASE"	,QString::number(m_u4BIT_BASE)	,nFieldSelection,	eposBIT_BASE	);
    _LIST_ADDFIELD_ASCII(listFields,	"STR.COND_CNT"	,QString::number(m_u2COND_CNT)	,nFieldSelection,	eposCOND_CNT	);
    _LIST_ADDFIELD_ASCII(listFields,	"STR.LIM_CNT"	,QString::number(m_u2LIM_CNT)	,nFieldSelection,	eposLIM_CNT	);
    _LIST_ADDFIELD_ASCII(listFields,	"STR.CYC_SIZE"	,QString::number(m_u1CYC_SIZE)	,nFieldSelection,	eposCYC_SIZE	);
    _LIST_ADDFIELD_ASCII(listFields,	"STR.PMR_SIZE"	,QString::number(m_u1PMR_SIZE)	,nFieldSelection,	eposPMR_SIZE	);
    _LIST_ADDFIELD_ASCII(listFields,	"STR.CHN_SIZE"	,QString::number(m_u1CHN_SIZE)	,nFieldSelection,	eposCHN_SIZE	);
    _LIST_ADDFIELD_ASCII(listFields,	"STR.PAT_SIZE"	,QString::number(m_u1PAT_SIZE)	,nFieldSelection,	eposPAT_SIZE	);
    _LIST_ADDFIELD_ASCII(listFields,	"STR.BIT_SIZE"	,QString::number(m_u1BIT_SIZE)	,nFieldSelection,	eposBIT_SIZE	);
    _LIST_ADDFIELD_ASCII(listFields,	"STR.U1_SIZE"	,QString::number(m_u1U1_SIZE)	,nFieldSelection,	eposU1_SIZE	);
    _LIST_ADDFIELD_ASCII(listFields,	"STR.U2_SIZE"	,QString::number(m_u1U2_SIZE)	,nFieldSelection,	eposU2_SIZE	);
    _LIST_ADDFIELD_ASCII(listFields,	"STR.U3_SIZE"	,QString::number(m_u1U3_SIZE)	,nFieldSelection,	eposU3_SIZE	);
    _LIST_ADDFIELD_ASCII(listFields,	"STR.UTX_SIZE"	,QString::number(m_u1UTX_SIZE)	,nFieldSelection,	eposUTX_SIZE	);
    _LIST_ADDFIELD_ASCII(listFields,	"STR.CAP_BGN"	,QString::number(m_u2CAP_BGN)	,nFieldSelection,	eposCAP_BGN	);


    //m_u2INDX_CNT, m_ku2PMR_INDX, eposPMR_INDX
    _CREATEFIELD_FROM_KUi_ASCII(m_u2LIM_CNT, m_pu2LIM_INDX,eposLIM_INDX);
    _LIST_ADDFIELD_ASCII(listFields, "STR.LIM_INDX", m_strFieldValue_macro, nFieldSelection, eposLIM_INDX);

    _CREATEFIELD_FROM_KU8_ASCII(m_u2LIM_CNT,m_pu4LIM_SPEC,eposLIM_SPEC);
    _LIST_ADDFIELD_ASCII(listFields,	"STR.LIM_SPEC"	,	m_strFieldValue_macro	,nFieldSelection,	eposLIM_SPEC	);

    _CREATEFIELD_FROM_KCN_ASCII(m_u2COND_CNT,m_pcnCOND_LST,eposCOND_LST);
    _LIST_ADDFIELD_ASCII(listFields,	"STR.COND_LST"	,	m_strFieldValue_macro	,nFieldSelection,	eposCOND_LST	);

    _LIST_ADDFIELD_ASCII(listFields,	"STR.CYCO_CNT"	,QString::number(m_u2CYCO_CNT)	,nFieldSelection,	eposCYCO_CNT	);
    _CREATEFIELD_FROM_KU8_ASCII(m_u2CYCO_CNT,m_pu8CYC_OFST,eposCYC_OFST);
    _LIST_ADDFIELD_ASCII(listFields,	"STR.CYC_OFST"	,	m_strFieldValue_macro,	nFieldSelection,eposCYC_OFST	);

    _LIST_ADDFIELD_ASCII(listFields,	"STR.PMR_CNT"	,QString::number(m_u2PMR_CNT)	,nFieldSelection,	eposPMR_CNT	);
    _CREATEFIELD_FROM_KU8_ASCII(m_u2PMR_CNT,m_pu8PMR_INDX,eposPMR_INDX);
    _LIST_ADDFIELD_ASCII(listFields,	"STR.PMR_INDX"	,	m_strFieldValue_macro, nFieldSelection, eposPMR_INDX	);

    _LIST_ADDFIELD_ASCII(listFields,	"STR.CHN_CNT"	,QString::number(m_u2CHN_CNT)	,nFieldSelection,	eposCHN_CNT	);
    _CREATEFIELD_FROM_KU8_ASCII(m_u2CHN_CNT,m_pu8CHN_NUM,eposCHN_NUM);
    _LIST_ADDFIELD_ASCII(listFields,	"STR.CHN_NUM"	,	m_strFieldValue_macro	,nFieldSelection,	eposCHN_NUM	);

    _LIST_ADDFIELD_ASCII(listFields,	"STR.EXP_CNT"	,QString::number(m_u2EXP_CNT)	,nFieldSelection,	eposEXP_CNT	);
    _CREATEFIELD_FROM_KUi_ASCII(m_u2CHN_CNT,m_pu1EXP_DATA,eposEXP_DATA);
    _LIST_ADDFIELD_ASCII(listFields,	"STR.EXP_DATA"	,	m_strFieldValue_macro	,nFieldSelection,	eposEXP_DATA	);

    _LIST_ADDFIELD_ASCII(listFields,	"STR.CAP_CNT"	,QString::number(m_u2CAP_CNT)	,nFieldSelection,	eposCAP_CNT	);
    _CREATEFIELD_FROM_KUi_ASCII(m_u2CAP_CNT,m_pu1CAP_DATA,eposCAP_DATA);
    _LIST_ADDFIELD_ASCII(listFields,	"STR.CAP_DATA"	,	m_strFieldValue_macro	,nFieldSelection,	eposCAP_DATA	);

    _LIST_ADDFIELD_ASCII(listFields,	"STR.NEW_CNT"	,QString::number(m_u2NEW_CNT)	,nFieldSelection,	eposNEW_CNT	);
    _CREATEFIELD_FROM_KUi_ASCII(m_u2NEW_CNT,m_pu1NEW_DATA,eposNEW_DATA);
    _LIST_ADDFIELD_ASCII(listFields,	"STR.NEW_DATA"	,	m_strFieldValue_macro	,nFieldSelection,	eposNEW_DATA	);

    _LIST_ADDFIELD_ASCII(listFields,	"STR.PAT_CNT"	,QString::number(m_u2PAT_CNT)	,nFieldSelection,	eposPAT_CNT	);
    _CREATEFIELD_FROM_KU8_ASCII(m_u2PAT_CNT,m_pu8PAT_NUM,eposPAT_NUM);
    _LIST_ADDFIELD_ASCII(listFields,	"STR.PAT_NUM"	,	m_strFieldValue_macro	,nFieldSelection,	eposPAT_NUM	);

    _LIST_ADDFIELD_ASCII(listFields,	"STR.BPOS_CNT"	,QString::number(m_u2BPOS_CNT)	,nFieldSelection,	eposBPOS_CNT	);
    _CREATEFIELD_FROM_KU8_ASCII(m_u2BPOS_CNT,m_pu8BIT_POS,eposBIT_POS);
    _LIST_ADDFIELD_ASCII(listFields,	"STR.BIT_POS"	,	m_strFieldValue_macro	,nFieldSelection,	eposBIT_POS	);

    _LIST_ADDFIELD_ASCII(listFields,	"STR.USR1_CNT"	,QString::number(m_u2USR1_CNT)	,nFieldSelection,	eposUSR1_CNT	);
    _CREATEFIELD_FROM_KU8_ASCII(m_u2USR1_CNT,m_pu8USR1,eposUSR1);
    _LIST_ADDFIELD_ASCII(listFields,	"STR.USR1"	,	m_strFieldValue_macro	,nFieldSelection,	eposUSR1	);

    _LIST_ADDFIELD_ASCII(listFields,	"STR.USR2_CNT"	,QString::number(m_u2USR2_CNT)	,nFieldSelection,	eposUSR2_CNT	);
    _CREATEFIELD_FROM_KU8_ASCII(m_u2USR2_CNT,m_pu8USR2,eposUSR2);
    _LIST_ADDFIELD_ASCII(listFields,	"STR.USR2"	,	m_strFieldValue_macro	,nFieldSelection,	eposUSR2	);

    _LIST_ADDFIELD_ASCII(listFields,	"STR.USR3_CNT"	,QString::number(m_u2USR3_CNT)	,nFieldSelection,	eposUSR3_CNT	);
    _CREATEFIELD_FROM_KU8_ASCII(m_u2USR3_CNT,m_pu8USR3,eposUSR3);
    _LIST_ADDFIELD_ASCII(listFields,	"STR.USR3"	,	m_strFieldValue_macro	,nFieldSelection,	eposUSR3	);


    _LIST_ADDFIELD_ASCII(listFields,	"STR.TXT_CNT"	,QString::number(m_u2TXT_CNT)	,nFieldSelection,	eposTXT_CNT	);
    _CREATEFIELD_FROM_KCN_ASCII(m_u2TXT_CNT,m_pcnUSER_TXT,eposUSER_TXT);
    _LIST_ADDFIELD_ASCII(listFields,	"STR.USER_TXT"	,	m_strFieldValue_macro	,nFieldSelection,	eposUSER_TXT	);
}

void Stdf_STR_V4::GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection /*= 0*/)
{
    QString strTabs="";

    // Init XML string
    for(int nIndex=nIndentationLevel; nIndex>0; nIndex--)
        strTabs += "\t";
    strXmlString = strTabs;
    strXmlString += "<STR>\n";
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"CONT_FLG"	,QString::number(m_b1CONT_FLG)	,nFieldSelection,	eposCONT_FLG	);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"TEST_NUM"	,QString::number(m_u4TEST_NUM)	,nFieldSelection,	eposTEST_NUM	);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"HEAD_NUM"	,QString::number(m_u1HEAD_NUM)	,nFieldSelection,	eposHEAD_NUM	);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"SITE_NUM"	,QString::number(m_u1SITE_NUM)	,nFieldSelection,	eposSITE_NUM	);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"PSR_REF"	,QString::number(m_u2PSR_REF),nFieldSelection,	eposPSR_REF	);

    _CREATEFIELD_FROM_B1_XML(m_b1TEST_FLG);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"TEST_FLG"	,m_strFieldValue_macro	,nFieldSelection,	eposTEST_FLG	);

    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"LOG_TYP"	,m_cnLOG_TYP	,nFieldSelection,	eposLOG_TYP	);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"TEST_TXT"	,m_cnTEST_TXT	,nFieldSelection,	eposTEST_TXT	);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"ALARM_ID"	,m_cnALARM_ID	,nFieldSelection,	eposALARM_ID	);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"PROG_TXT"	,m_cnPROG_TXT	,nFieldSelection,	eposPROG_TXT	);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"RSLT_TXT"	,m_cnRSLT_TXT	,nFieldSelection,	eposRSLT_TXT	);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"Z_VAL"	,QString::number(m_u1Z_VAL)	,nFieldSelection,	eposZ_VAL	);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"FMU_FLG"	,QString::number(m_b1FMU_FLG)	,nFieldSelection,	eposFMU_FLG	);

    if((m_b1FMU_FLG  & STDF_MASK_BIT0 )== 1 && (m_b1FMU_FLG  & STDF_MASK_BIT1 )== 0){
        _CREATEFIELD_FROM_DN_XML(m_dnMASK_MAP);
        _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "MASK_MAP", m_strFieldValue_macro, nFieldSelection, eposMASK_MAP);
    }
    if((m_b1FMU_FLG  & STDF_MASK_BIT2 )== 1 && (m_b1FMU_FLG  & STDF_MASK_BIT3 )== 0){
        _CREATEFIELD_FROM_DN_XML(m_dnFAL_MAP);
        _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "FAL_MAP", m_strFieldValue_macro, nFieldSelection, eposFAL_MAP);
    }
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"CYC_CNT"	,QString::number(m_u8CYC_CNT)	,nFieldSelection,	eposCYC_CNT	);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"TOTF_CNT"	,QString::number(m_u4TOTF_CNT)	,nFieldSelection,	eposTOTF_CNT	);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"TOTL_CNT"	,QString::number(m_u4TOTL_CNT)	,nFieldSelection,	eposTOTL_CNT	);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"CYC_BASE"	,QString::number(m_u8CYC_BASE)	,nFieldSelection,	eposCYC_BASE	);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"BIT_BASE"	,QString::number(m_u4BIT_BASE)	,nFieldSelection,	eposBIT_BASE	);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"COND_CNT"	,QString::number(m_u2COND_CNT)	,nFieldSelection,	eposCOND_CNT	);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"LIM_CNT"	,QString::number(m_u2LIM_CNT)	,nFieldSelection,	eposLIM_CNT	);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"CYC_SIZE"	,QString::number(m_u1CYC_SIZE)	,nFieldSelection,	eposCYC_SIZE	);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"PMR_SIZE"	,QString::number(m_u1PMR_SIZE)	,nFieldSelection,	eposPMR_SIZE	);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"CHN_SIZE"	,QString::number(m_u1CHN_SIZE)	,nFieldSelection,	eposCHN_SIZE	);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"PAT_SIZE"	,QString::number(m_u1PAT_SIZE)	,nFieldSelection,	eposPAT_SIZE	);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"BIT_SIZE"	,QString::number(m_u1BIT_SIZE)	,nFieldSelection,	eposBIT_SIZE	);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"U1_SIZE"	,QString::number(m_u1U1_SIZE)	,nFieldSelection,	eposU1_SIZE	);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"U2_SIZE"	,QString::number(m_u1U2_SIZE)	,nFieldSelection,	eposU2_SIZE	);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"U3_SIZE"	,QString::number(m_u1U3_SIZE)	,nFieldSelection,	eposU3_SIZE	);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"UTX_SIZE"	,QString::number(m_u1UTX_SIZE)	,nFieldSelection,	eposUTX_SIZE	);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"CAP_BGN"	,QString::number(m_u2CAP_BGN)	,nFieldSelection,	eposCAP_BGN	);


    //m_u2INDX_CNT, m_ku2PMR_INDX, eposPMR_INDX
    _CREATEFIELD_FROM_KUi_XML(m_u2LIM_CNT, m_pu2LIM_INDX,eposLIM_INDX);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1, "LIM_INDX", m_strFieldValue_macro, nFieldSelection, eposLIM_INDX);

    _CREATEFIELD_FROM_KU8_XML(m_u2LIM_CNT,m_pu4LIM_SPEC,eposLIM_SPEC);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"LIM_SPEC"	,	m_strFieldValue_macro	,nFieldSelection,	eposLIM_SPEC	);

    _CREATEFIELD_FROM_KCN_XML (m_u2COND_CNT,m_pcnCOND_LST,eposCOND_LST);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"COND_LST"	,	m_strFieldValue_macro	,nFieldSelection,	eposCOND_LST	);

    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"CYCO_CNT"	,QString::number(m_u2CYCO_CNT)	,nFieldSelection,	eposCYCO_CNT	);
    _CREATEFIELD_FROM_KU8_XML(m_u2CYCO_CNT,m_pu8CYC_OFST,eposCYC_OFST);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"CYC_OFST"	,	m_strFieldValue_macro,	nFieldSelection,eposCYC_OFST	);

    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"PMR_CNT"	,QString::number(m_u2PMR_CNT)	,nFieldSelection,	eposPMR_CNT	);
    _CREATEFIELD_FROM_KU8_XML(m_u2PMR_CNT,m_pu8PMR_INDX,eposPMR_INDX);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"PMR_INDX"	,	m_strFieldValue_macro, nFieldSelection, eposPMR_INDX	);

    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"CHN_CNT"	,QString::number(m_u2CHN_CNT)	,nFieldSelection,	eposCHN_CNT	);
    _CREATEFIELD_FROM_KU8_XML(m_u2CHN_CNT,m_pu8CHN_NUM,eposCHN_NUM);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"CHN_NUM"	,	m_strFieldValue_macro	,nFieldSelection,	eposCHN_NUM	);

    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"EXP_CNT"	,QString::number(m_u2EXP_CNT)	,nFieldSelection,	eposEXP_CNT	);
    _CREATEFIELD_FROM_KUi_XML(m_u2CHN_CNT,m_pu1EXP_DATA,eposEXP_DATA);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"EXP_DATA"	,	m_strFieldValue_macro	,nFieldSelection,	eposEXP_DATA	);

    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"CAP_CNT"	,QString::number(m_u2CAP_CNT)	,nFieldSelection,	eposCAP_CNT	);
    _CREATEFIELD_FROM_KUi_XML(m_u2CAP_CNT,m_pu1CAP_DATA,eposCAP_DATA);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"CAP_DATA"	,	m_strFieldValue_macro	,nFieldSelection,	eposCAP_DATA	);

    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"NEW_CNT"	,QString::number(m_u2NEW_CNT)	,nFieldSelection,	eposNEW_CNT	);
    _CREATEFIELD_FROM_KUi_XML(m_u2NEW_CNT,m_pu1NEW_DATA,eposNEW_DATA);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"NEW_DATA"	,	m_strFieldValue_macro	,nFieldSelection,	eposNEW_DATA	);

    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"PAT_CNT"	,QString::number(m_u2PAT_CNT)	,nFieldSelection,	eposPAT_CNT	);
    _CREATEFIELD_FROM_KU8_XML(m_u2PAT_CNT,m_pu8PAT_NUM,eposPAT_NUM);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"PAT_NUM"	,	m_strFieldValue_macro	,nFieldSelection,	eposPAT_NUM	);

    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"BPOS_CNT"	,QString::number(m_u2BPOS_CNT)	,nFieldSelection,	eposBPOS_CNT	);
    _CREATEFIELD_FROM_KU8_XML(m_u2BPOS_CNT,m_pu8BIT_POS,eposBIT_POS);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"BIT_POS"	,	m_strFieldValue_macro	,nFieldSelection,	eposBIT_POS	);

    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"USR1_CNT"	,QString::number(m_u2USR1_CNT)	,nFieldSelection,	eposUSR1_CNT	);
    _CREATEFIELD_FROM_KU8_XML(m_u2USR1_CNT,m_pu8USR1,eposUSR1);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"USR1"	,	m_strFieldValue_macro	,nFieldSelection,	eposUSR1	);

    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"USR2_CNT"	,QString::number(m_u2USR2_CNT)	,nFieldSelection,	eposUSR2_CNT	);
    _CREATEFIELD_FROM_KU8_XML(m_u2USR2_CNT,m_pu8USR2,eposUSR2);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"USR2"	,	m_strFieldValue_macro	,nFieldSelection,	eposUSR2	);

    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"USR3_CNT"	,QString::number(m_u2USR3_CNT)	,nFieldSelection,	eposUSR3_CNT	);
    _CREATEFIELD_FROM_KU8_XML(m_u2USR3_CNT,m_pu8USR3,eposUSR3);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"USR3"	,	m_strFieldValue_macro	,nFieldSelection,	eposUSR3	);


    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"TXT_CNT"	,QString::number(m_u2TXT_CNT)	,nFieldSelection,	eposTXT_CNT	);
    _CREATEFIELD_FROM_KCN_XML(m_u2TXT_CNT,m_pcnUSER_TXT,eposUSER_TXT);
    _STR_ADDFIELD_XML(strXmlString, nIndentationLevel+1,	"USER_TXT"	,	m_strFieldValue_macro	,nFieldSelection,	eposUSER_TXT	);

    strXmlString += strTabs;
    strXmlString += "</STR>\n";
}

void Stdf_STR_V4::GetAtdfString(QString & strAtdfString)
{
    // Reset field index, and line Nb
    m_uiFieldIndex = 0;
    m_uiLineNb = 1;

    // Init string
    strAtdfString = "STR:";


    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_b1CONT_FLG),	eposCONT_FLG	);
    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_u4TEST_NUM),	eposTEST_NUM	);
    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_u1HEAD_NUM),	eposHEAD_NUM	);
    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_u1SITE_NUM),	eposSITE_NUM	);
    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_u2PSR_REF),	eposPSR_REF	);
    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_b1TEST_FLG),	eposTEST_FLG	);
    _STR_ADDFIELD_ATDF(strAtdfString,	m_cnLOG_TYP,	eposLOG_TYP	);
    _STR_ADDFIELD_ATDF(strAtdfString,	m_cnTEST_TXT,	eposTEST_TXT	);
    _STR_ADDFIELD_ATDF(strAtdfString,	m_cnALARM_ID,	eposALARM_ID	);
    _STR_ADDFIELD_ATDF(strAtdfString,	m_cnPROG_TXT,	eposPROG_TXT	);
    _STR_ADDFIELD_ATDF(strAtdfString,	m_cnRSLT_TXT,	eposRSLT_TXT	);
    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_u1Z_VAL),	eposZ_VAL	);
    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_b1FMU_FLG),	eposFMU_FLG	);

    _CREATEFIELD_FROM_DN_ATDF_FTR_PINS(m_dnMASK_MAP);
    _STR_ADDFIELD_ATDF(strAtdfString,   m_strFieldValue_macro,          eposMASK_MAP);
    _CREATEFIELD_FROM_DN_ATDF_FTR_PINS(m_dnFAL_MAP);
    _STR_ADDFIELD_ATDF(strAtdfString,   m_strFieldValue_macro,          eposFAL_MAP);

    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_u8CYC_CNT),	eposCYC_CNT	);
    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_u4TOTF_CNT),	eposTOTF_CNT	);
    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_u4TOTL_CNT),	eposTOTL_CNT	);
    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_u8CYC_BASE),	eposCYC_BASE	);
    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_u4BIT_BASE),	eposBIT_BASE	);
    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_u2COND_CNT),	eposCOND_CNT	);
    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_u2LIM_CNT),	eposLIM_CNT	);
    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_u1CYC_SIZE),	eposCYC_SIZE	);
    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_u1PMR_SIZE),	eposPMR_SIZE	);
    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_u1CHN_SIZE),	eposCHN_SIZE	);
    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_u1PAT_SIZE),	eposPAT_SIZE	);
    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_u1BIT_SIZE),	eposBIT_SIZE	);
    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_u1U1_SIZE),	eposU1_SIZE	);
    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_u1U2_SIZE),	eposU2_SIZE	);
    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_u1U3_SIZE),	eposU3_SIZE	);
    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_u1UTX_SIZE),	eposUTX_SIZE	);
    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_u2CAP_BGN),	eposCAP_BGN	);


    //m_u2INDX_CNT, m_ku2PMR_INDX, eposPMR_INDX
    _CREATEFIELD_FROM_KUi_ATDF(m_u2LIM_CNT, m_pu2LIM_INDX,eposLIM_INDX);
    _STR_ADDFIELD_ATDF(strAtdfString, m_strFieldValue_macro, eposLIM_INDX);

    _CREATEFIELD_FROM_KU8_ATDF(m_u2LIM_CNT,m_pu4LIM_SPEC,eposLIM_SPEC);
    _STR_ADDFIELD_ATDF(strAtdfString,	m_strFieldValue_macro,	eposLIM_SPEC	);

    _CREATEFIELD_FROM_KCN_ATDF(m_u2COND_CNT,m_pcnCOND_LST,eposCOND_LST);
    _STR_ADDFIELD_ATDF(strAtdfString,	m_strFieldValue_macro,	eposCOND_LST	);

    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_u2CYCO_CNT),	eposCYCO_CNT	);
    _CREATEFIELD_FROM_KU8_ATDF(m_u2CYCO_CNT,m_pu8CYC_OFST,eposCYC_OFST);
    _STR_ADDFIELD_ATDF(strAtdfString,	m_strFieldValue_macro,	eposCYC_OFST	);

    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_u2PMR_CNT),	eposPMR_CNT	);
    _CREATEFIELD_FROM_KU8_ATDF(m_u2PMR_CNT,m_pu8PMR_INDX,eposPMR_INDX);
    _STR_ADDFIELD_ATDF(strAtdfString,	m_strFieldValue_macro, eposPMR_INDX	);

    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_u2CHN_CNT),	eposCHN_CNT	);
    _CREATEFIELD_FROM_KU8_ATDF(m_u2CHN_CNT,m_pu8CHN_NUM,eposCHN_NUM);
    _STR_ADDFIELD_ATDF(strAtdfString,	m_strFieldValue_macro,	eposCHN_NUM	);

    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_u2EXP_CNT),	eposEXP_CNT	);
    _CREATEFIELD_FROM_KUi_ATDF(m_u2EXP_CNT,m_pu1EXP_DATA,eposEXP_DATA);
    _STR_ADDFIELD_ATDF(strAtdfString,	m_strFieldValue_macro,	eposEXP_DATA	);

    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_u2CAP_CNT),	eposCAP_CNT	);
    _CREATEFIELD_FROM_KUi_ATDF(m_u2CAP_CNT,m_pu1CAP_DATA,eposCAP_DATA);
    _STR_ADDFIELD_ATDF(strAtdfString,	m_strFieldValue_macro,	eposCAP_DATA	);

    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_u2NEW_CNT),	eposNEW_CNT	);
    _CREATEFIELD_FROM_KUi_ATDF(m_u2NEW_CNT,m_pu1NEW_DATA,eposNEW_DATA);
    _STR_ADDFIELD_ATDF(strAtdfString,	m_strFieldValue_macro,	eposNEW_DATA	);

    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_u2PAT_CNT),	eposPAT_CNT	);
    _CREATEFIELD_FROM_KU8_ATDF(m_u2PAT_CNT,m_pu8PAT_NUM,eposPAT_NUM);
    _STR_ADDFIELD_ATDF(strAtdfString,	m_strFieldValue_macro,	eposPAT_NUM	);

    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_u2BPOS_CNT),	eposBPOS_CNT	);
    _CREATEFIELD_FROM_KU8_ATDF(m_u2BPOS_CNT,m_pu8BIT_POS,eposBIT_POS);
    _STR_ADDFIELD_ATDF(strAtdfString,	m_strFieldValue_macro,	eposBIT_POS	);

    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_u2USR1_CNT),	eposUSR1_CNT	);
    _CREATEFIELD_FROM_KU8_ATDF(m_u2USR1_CNT,m_pu8USR1,eposUSR1);
    _STR_ADDFIELD_ATDF(strAtdfString,	m_strFieldValue_macro,	eposUSR1	);

    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_u2USR2_CNT),	eposUSR2_CNT	);
    _CREATEFIELD_FROM_KU8_ATDF(m_u2USR2_CNT,m_pu8USR2,eposUSR2);
    _STR_ADDFIELD_ATDF(strAtdfString,	m_strFieldValue_macro,	eposUSR2	);

    _STR_ADDFIELD_ATDF(strAtdfString,	QString::number(m_u2USR3_CNT),	eposUSR3_CNT	);
    _CREATEFIELD_FROM_KU8_ATDF(m_u2USR3_CNT,m_pu8USR3,eposUSR3);
    _STR_ADDFIELD_ATDF(strAtdfString,	m_strFieldValue_macro,	eposUSR3	);


    _STR_ADDFIELD_ATDF(strAtdfString,QString::number(m_u2TXT_CNT),	eposTXT_CNT	);
    _CREATEFIELD_FROM_KCN_ATDF(m_u2TXT_CNT,m_pcnUSER_TXT,eposUSER_TXT);
    _STR_ADDFIELD_ATDF(strAtdfString,	m_strFieldValue_macro,	eposUSER_TXT	);

    _STR_FINISHFIELD_ATDF(strAtdfString);
}


}
