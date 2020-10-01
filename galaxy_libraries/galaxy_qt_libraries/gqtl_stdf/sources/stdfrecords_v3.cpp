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
// StdfRecords_V3 class IMPLEMENTATION :
// This class contains routines to read records from an
// stdf file version 3
///////////////////////////////////////////////////////////

#include <QDateTime>

#include "stdfrecords_v3.h"

namespace GQTL_STDF
{

///////////////////////////////////////////////////////////
// FAR RECORD
///////////////////////////////////////////////////////////
Stdf_FAR_V3::Stdf_FAR_V3() : Stdf_Record()
{
    Reset();
}

Stdf_FAR_V3::~Stdf_FAR_V3()
{
    Reset();
}

void Stdf_FAR_V3::Reset(void)
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
}

bool Stdf_FAR_V3::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;
    return false;
}

bool Stdf_FAR_V3::IsFieldPresent(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Present))
        return true;
    return false;
}



QString Stdf_FAR_V3::GetRecordShortName(void)
{
    return "FAR";
}

QString Stdf_FAR_V3::GetRecordLongName(void)
{
    return "File Attributes Record";
}

int Stdf_FAR_V3::GetRecordType(void)
{
    return Rec_FAR;
}

bool Stdf_FAR_V3::Read(GS::StdLib::Stdf & lStdf)
{
    BYTE	bData;

    // First reset data
    Reset();

    // FAR.CPU_TYPE
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposCPU_TYPE);
    _FIELD_SET(m_u1CPU_TYPE = stdf_type_u1(bData), true, eposCPU_TYPE);

    // FAR.STDF_VER
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposSTDF_VER);
    _FIELD_SET(m_u1STDF_VER = stdf_type_u1(bData), true, eposSTDF_VER);

    return true;
}

void Stdf_FAR_V3::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    // Empty string first
    strAsciiString = "";

    // FAR.CPU_TYPE
    _STR_ADDFIELD_ASCII(strAsciiString, "FAR.cpu_type", QString::number(m_u1CPU_TYPE), nFieldSelection, eposCPU_TYPE);

    // FAR.STDF_VER
    _STR_ADDFIELD_ASCII(strAsciiString, "FAR.stdf_ver", QString::number(m_u1STDF_VER), nFieldSelection, eposSTDF_VER);
}

void Stdf_FAR_V3::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
    // Empty string list first
    listFields.empty();

    // FAR.CPU_TYPE
    _LIST_ADDFIELD_ASCII(listFields, "FAR.cpu_type", QString::number(m_u1CPU_TYPE), nFieldSelection, eposCPU_TYPE);

    // FAR.STDF_VER
    _LIST_ADDFIELD_ASCII(listFields, "FAR.stdf_ver", QString::number(m_u1STDF_VER), nFieldSelection, eposSTDF_VER);
}



///////////////////////////////////////////////////////////
// MIR RECORD
///////////////////////////////////////////////////////////
Stdf_MIR_V3::Stdf_MIR_V3() : Stdf_Record()
{
    Reset();
}

Stdf_MIR_V3::~Stdf_MIR_V3()
{
    Reset();
}

void Stdf_MIR_V3::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Select fields for reduced list
    m_pFieldFlags[eposSETUP_T]	|= FieldFlag_ReducedList;
    m_pFieldFlags[eposSTART_T]	|= FieldFlag_ReducedList;
    m_pFieldFlags[eposSTAT_NUM]	|= FieldFlag_ReducedList;
    m_pFieldFlags[eposEXEC_TYP]	|= FieldFlag_ReducedList;


    // Reset Data
    m_u1CPU_TYPE        = 0;            // MIR.CPU_TYPE
    m_u1STDF_VER        = 0;            // MIR.STDF_VER
    m_u4SETUP_T			= 0;			// MIR.SETUP_T
    m_u4START_T			= 0;			// MIR.START_T
    m_u1STAT_NUM		= 0;			// MIR.STAT_NUM
    m_c1MODE_COD		= 0;			// MIR.MODE_COD
    m_c1RTST_COD		= ' ';			// MIR.RTST_COD
    m_c1PROT_COD		= ' ';			// MIR.PROT_COD
    m_c1CMOD_COD		= ' ';			// MIR.CMOD_COD
    m_cnLOT_ID			= "";			// MIR.LOT_ID
    m_cnPART_TYP		= "";			// MIR.PART_TYP
    m_cnNODE_NAM		= "";			// MIR.NODE_NAM
    m_cnTSTR_TYP		= "";			// MIR.TSTR_TYP
    m_cnJOB_NAM			= "";			// MIR.JOB_NAM
    m_cnJOB_REV			= "";			// MIR.JOB_REV
    m_cnHAND_ID         = "";           // MIR.HAND_ID
    m_cnSBLOT_ID		= "";			// MIR.SBLOT_ID
    m_cnOPER_NAM		= "";			// MIR.OPER_NAM
    m_cnEXEC_TYP		= "";			// MIR.EXEC_TYP
    m_cnTEST_COD		= "   ";		// MIR.TEST_COD
    m_cnPROC_ID			= "";			// MIR.PROC_ID
    m_cnSUPR_NAM		= "";			// MIR.SUPR_NAM
    m_cnPRB_CARD        = "";           // MIR.PRB_CARD
}

bool Stdf_MIR_V3::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;
    return false;
}

bool Stdf_MIR_V3::IsFieldPresent(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Present))
        return true;
    return false;
}

QString Stdf_MIR_V3::GetRecordShortName(void)
{
    return "MIR";
}

QString Stdf_MIR_V3::GetRecordLongName(void)
{
    return "Master Information Record";
}

int Stdf_MIR_V3::GetRecordType(void)
{
    return Rec_MIR;
}

bool Stdf_MIR_V3::Read(GS::StdLib::Stdf & lStdf)
{
    char	szString[STDF_MAX_U1+1];
    long	dwData;
    BYTE	bData;

    // First reset data
    Reset();

    // MIR.CPU_TYPE
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposCPU_TYPE);
    _FIELD_SET(m_u1CPU_TYPE = stdf_type_u1(bData), true, eposCPU_TYPE);

    // MIR.STDF_VER
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposSTDF_VER);
    _FIELD_SET(m_u1STDF_VER = stdf_type_u1(bData), true, eposSTDF_VER);

    // MIR.MODE_COD
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposMODE_COD);
    _FIELD_SET(m_c1MODE_COD = stdf_type_c1(bData), m_c1MODE_COD != ' ', eposMODE_COD);

    // MIR.STAT_NUM
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposSTAT_NUM);
    _FIELD_SET(m_u1STAT_NUM = stdf_type_u1(bData), true, eposSTAT_NUM);

    // MIR.TEST_COD
    // Read 3 char
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposTEST_COD);
    szString[0] = bData;
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposTEST_COD);
    szString[1] = bData;
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposTEST_COD);
    szString[2] = bData;
    szString[3] = '\0';
    _FIELD_SET(m_cnTEST_COD = QString(szString), m_cnTEST_COD != "   ", eposTEST_COD);

    // MIR.RTST_COD
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposRTST_COD);
    _FIELD_SET(m_c1RTST_COD = stdf_type_c1(bData), m_c1RTST_COD != ' ', eposRTST_COD);

    // MIR.PROT_COD
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposPROT_COD);
    _FIELD_SET(m_c1PROT_COD = stdf_type_c1(bData), m_c1PROT_COD != ' ', eposPROT_COD);

    // MIR.CMOD_COD
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposCMOD_COD);
    _FIELD_SET(m_c1CMOD_COD = stdf_type_c1(bData), m_c1CMOD_COD != ' ', eposCMOD_COD);

    // MIR.SETUP_T
    _FIELD_CHECKREAD(lStdf.ReadDword(&dwData), eposSETUP_T);
    _FIELD_SET(m_u4SETUP_T = stdf_type_u4(dwData), true, eposSETUP_T);

    // MIR.START_T
    _FIELD_CHECKREAD(lStdf.ReadDword(&dwData), eposSTART_T);
    _FIELD_SET(m_u4START_T = stdf_type_u4(dwData), true, eposSTART_T);

    // MIR.LOT_ID
    _FIELD_CHECKREAD(lStdf.ReadString(szString), eposLOT_ID);
    _FIELD_SET(m_cnLOT_ID = szString, true, eposLOT_ID);

    // MIR.PART_TYP
    _FIELD_CHECKREAD(lStdf.ReadString(szString), eposPART_TYP);
    _FIELD_SET(m_cnPART_TYP = szString, true, eposPART_TYP);

    // MIR.JOB_NAM
    _FIELD_CHECKREAD(lStdf.ReadString(szString), eposJOB_NAM);
    _FIELD_SET(m_cnJOB_NAM = szString, true, eposJOB_NAM);

    // MIR.OPER_NAM
    _FIELD_CHECKREAD(lStdf.ReadString(szString), eposOPER_NAM);
    _FIELD_SET(m_cnOPER_NAM = szString, !m_cnOPER_NAM.isEmpty(), eposOPER_NAM);

    // MIR.NODE_NAM
    _FIELD_CHECKREAD(lStdf.ReadString(szString), eposNODE_NAM);
    _FIELD_SET(m_cnNODE_NAM = szString, true, eposNODE_NAM);

    // MIR.TSTR_TYP
    _FIELD_CHECKREAD(lStdf.ReadString(szString), eposTSTR_TYP);
    _FIELD_SET(m_cnTSTR_TYP = szString, true, eposTSTR_TYP);

    // MIR.EXEC_TYP
    _FIELD_CHECKREAD(lStdf.ReadString(szString), eposEXEC_TYP);
    _FIELD_SET(m_cnEXEC_TYP = szString, true, eposEXEC_TYP);

    // MIR.SUPR_NAM
    _FIELD_CHECKREAD(lStdf.ReadString(szString), eposSUPR_NAM);
    _FIELD_SET(m_cnSUPR_NAM = szString, !m_cnSUPR_NAM.isEmpty(), eposSUPR_NAM);

    // MIR.HAND_ID
    _FIELD_CHECKREAD(lStdf.ReadString(szString), eposHAND_ID);
    _FIELD_SET(m_cnHAND_ID = szString, !m_cnHAND_ID.isEmpty(), eposHAND_ID);

    // MIR.SBLOT_ID
    _FIELD_CHECKREAD(lStdf.ReadString(szString), eposSBLOT_ID);
    _FIELD_SET(m_cnSBLOT_ID = szString, !m_cnSBLOT_ID.isEmpty(), eposSBLOT_ID);

    // MIR.JOB_REV
    _FIELD_CHECKREAD(lStdf.ReadString(szString), eposJOB_REV);
    _FIELD_SET(m_cnJOB_REV = szString, !m_cnJOB_REV.isEmpty(), eposJOB_REV);

    // MIR.PROC_ID
    _FIELD_CHECKREAD(lStdf.ReadString(szString), eposPROC_ID);
    _FIELD_SET(m_cnPROC_ID = szString, !m_cnPROC_ID.isEmpty(), eposPROC_ID);

    // MIR.PRB_CARD
    _FIELD_CHECKREAD(lStdf.ReadString(szString), eposPRB_CARD);
    _FIELD_SET(m_cnPRB_CARD = szString, !m_cnPRB_CARD.isEmpty(), eposPRB_CARD);

    return true;
}

void Stdf_MIR_V3::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    QDateTime	clDateTime;
    time_t		tDateTime;

    // Set QDateTime object to UTC mode
    clDateTime.setTimeSpec(Qt::UTC);

    // Empty string first
    strAsciiString = "";

    // MIR.CPU_TYPE
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.cpu_type", QString::number(m_u1CPU_TYPE), nFieldSelection, eposCPU_TYPE);

    // MIR.STDF_VER
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.stdf_ver", QString::number(m_u1STDF_VER), nFieldSelection, eposSTDF_VER);

    // MIR.MODE_COD
    _CREATEFIELD_FROM_C1_ASCII(m_c1MODE_COD);
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.mode_cod", m_strFieldValue_macro, nFieldSelection, eposMODE_COD);

    // MIR.STAT_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.stat_num", QString::number(m_u1STAT_NUM), nFieldSelection, eposSTAT_NUM);

    // MIR.TEST_COD
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.test_cod", m_cnTEST_COD, nFieldSelection, eposTEST_COD);

    // MIR.RTST_COD
    _CREATEFIELD_FROM_C1_ASCII(m_c1RTST_COD);
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.rtst_cod", m_strFieldValue_macro, nFieldSelection, eposRTST_COD);

    // MIR.PROT_COD
    _CREATEFIELD_FROM_C1_ASCII(m_c1PROT_COD);
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.prot_cod", m_strFieldValue_macro, nFieldSelection, eposPROT_COD);

    // MIR.CMOD_COD
    _CREATEFIELD_FROM_C1_ASCII(m_c1CMOD_COD);
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.cmod_cod", m_strFieldValue_macro, nFieldSelection, eposCMOD_COD);

    // MIR.SETUP_T
    tDateTime = (time_t)m_u4SETUP_T;
    clDateTime.setTime_t(tDateTime);

    // GCORE-178 : outputs days name in english
    //m_strFieldValue_macro.sprintf("%s (0x%lx)", clDateTime.toString("ddd dd MMM yyyy h:mm:ss").toLatin1().constData(),
      //                            (long unsigned int)m_u4SETUP_T);
    m_strFieldValue_macro.sprintf("%s (0x%lx)",
                                  ( GS::StdLib::Stdf::GetDaysOfWeekString(clDateTime.date().dayOfWeek()-1)
                                    + clDateTime.toString(" dd ")
                                    + GS::StdLib::Stdf::GetMonthName(clDateTime.date().month()-1)
                                    + clDateTime.toString(" yyyy h:mm:ss") )
                                  .toLatin1().constData(),
            (long unsigned int)m_u4SETUP_T);

    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.setup_t", m_strFieldValue_macro, nFieldSelection, eposSETUP_T);

    // MIR.START_T
    tDateTime = (time_t)m_u4START_T;
    clDateTime.setTime_t(tDateTime);
    // GCORE-178
    m_strFieldValue_macro.sprintf("%s (0x%lx)",
                                  ( GS::StdLib::Stdf::GetDaysOfWeekString(clDateTime.date().dayOfWeek()-1)
                                    + clDateTime.toString(" dd ")
                                    + GS::StdLib::Stdf::GetMonthName(clDateTime.date().month()-1)
                                    + clDateTime.toString(" yyyy h:mm:ss") )
                                  .toLatin1().constData(),
                                  (long unsigned int)m_u4START_T);
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.start_t", m_strFieldValue_macro, nFieldSelection, eposSTART_T);

    // MIR.LOT_ID
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.lot_id", m_cnLOT_ID, nFieldSelection, eposLOT_ID);

    // MIR.PART_TYP
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.part_typ", m_cnPART_TYP, nFieldSelection, eposPART_TYP);

    // MIR.JOB_NAM
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.job_nam", m_cnJOB_NAM, nFieldSelection, eposJOB_NAM);

    // MIR.OPER_NAM
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.oper_nam", m_cnOPER_NAM, nFieldSelection, eposOPER_NAM);

    // MIR.NODE_NAM
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.node_nam", m_cnNODE_NAM, nFieldSelection, eposNODE_NAM);

    // MIR.TSTR_TYP
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.tstr_typ", m_cnTSTR_TYP, nFieldSelection, eposTSTR_TYP);

    // MIR.EXEC_TYP
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.exec_typ", m_cnEXEC_TYP, nFieldSelection, eposEXEC_TYP);

    // MIR.SUPR_NAM
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.supr_nam", m_cnSUPR_NAM, nFieldSelection, eposSUPR_NAM);

    // MIR.HAND_ID
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.hand_id", m_cnHAND_ID, nFieldSelection, eposHAND_ID);

    // MIR.SBLOT_ID
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.sblot_id", m_cnSBLOT_ID, nFieldSelection, eposSBLOT_ID);

    // MIR.JOB_REV
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.job_rev", m_cnJOB_REV, nFieldSelection, eposJOB_REV);

    // MIR.PROC_ID
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.proc_id", m_cnPROC_ID, nFieldSelection, eposPROC_ID);

    // MIR.PRB_CARD
    _STR_ADDFIELD_ASCII(strAsciiString, "MIR.prb_card", m_cnPRB_CARD, nFieldSelection, eposPRB_CARD);
}

void Stdf_MIR_V3::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
    QDateTime	clDateTime;
    time_t		tDateTime;

    // Set QDateTime object to UTC mode
    clDateTime.setTimeSpec(Qt::UTC);

    // Empty string list first
    listFields.empty();

    // MIR.CPU_TYPE
    _LIST_ADDFIELD_ASCII(listFields, "MIR.cpu_type", QString::number(m_u1CPU_TYPE), nFieldSelection, eposCPU_TYPE);

    // MIR.STDF_VER
    _LIST_ADDFIELD_ASCII(listFields, "MIR.stdf_ver", QString::number(m_u1STDF_VER), nFieldSelection, eposSTDF_VER);

    // MIR.MODE_COD
    _CREATEFIELD_FROM_C1_ASCII(m_c1MODE_COD);
    _LIST_ADDFIELD_ASCII(listFields, "MIR.mode_cod", m_strFieldValue_macro, nFieldSelection, eposMODE_COD);

    // MIR.STAT_NUM
    _LIST_ADDFIELD_ASCII(listFields, "MIR.stat_num", QString::number(m_u1STAT_NUM), nFieldSelection, eposSTAT_NUM);

    // MIR.TEST_COD
    _LIST_ADDFIELD_ASCII(listFields, "MIR.test_cod", m_cnTEST_COD, nFieldSelection, eposTEST_COD);

    // MIR.RTST_COD
    _CREATEFIELD_FROM_C1_ASCII(m_c1RTST_COD);
    _LIST_ADDFIELD_ASCII(listFields, "MIR.rtst_cod", m_strFieldValue_macro, nFieldSelection, eposRTST_COD);

    // MIR.PROT_COD
    _CREATEFIELD_FROM_C1_ASCII(m_c1PROT_COD);
    _LIST_ADDFIELD_ASCII(listFields, "MIR.prot_cod", m_strFieldValue_macro, nFieldSelection, eposPROT_COD);

    // MIR.CMOD_COD
    _CREATEFIELD_FROM_C1_ASCII(m_c1CMOD_COD);
    _LIST_ADDFIELD_ASCII(listFields, "MIR.cmod_cod", m_strFieldValue_macro, nFieldSelection, eposCMOD_COD);

    // MIR.SETUP_T
    tDateTime = (time_t)m_u4SETUP_T;
    clDateTime.setTime_t(tDateTime);
    m_strFieldValue_macro.sprintf("%s (0x%lx)",
                                  ( GS::StdLib::Stdf::GetDaysOfWeekString(clDateTime.date().dayOfWeek()-1)
                                    + clDateTime.toString(" dd ")
                                    + GS::StdLib::Stdf::GetMonthName(clDateTime.date().month()-1)
                                    + clDateTime.toString(" yyyy h:mm:ss") )
                                  .toLatin1().constData(),
                                  (long unsigned int)m_u4SETUP_T);
    _LIST_ADDFIELD_ASCII(listFields, "MIR.setup_t", m_strFieldValue_macro, nFieldSelection, eposSETUP_T);

    // MIR.START_T
    tDateTime = (time_t)m_u4START_T;
    clDateTime.setTime_t(tDateTime);
    m_strFieldValue_macro.sprintf("%s (0x%lx)",
                                  ( GS::StdLib::Stdf::GetDaysOfWeekString(clDateTime.date().dayOfWeek()-1)
                                    + clDateTime.toString(" dd ")
                                    + GS::StdLib::Stdf::GetMonthName(clDateTime.date().month()-1)
                                    + clDateTime.toString(" yyyy h:mm:ss") )
                                  .toLatin1().constData(),
                                  (long unsigned int)m_u4START_T);
    _LIST_ADDFIELD_ASCII(listFields, "MIR.start_t", m_strFieldValue_macro, nFieldSelection, eposSTART_T);

    // MIR.LOT_ID
    _LIST_ADDFIELD_ASCII(listFields, "MIR.lot_id", m_cnLOT_ID, nFieldSelection, eposLOT_ID);

    // MIR.PART_TYP
    _LIST_ADDFIELD_ASCII(listFields, "MIR.part_typ", m_cnPART_TYP, nFieldSelection, eposPART_TYP);

    // MIR.JOB_NAM
    _LIST_ADDFIELD_ASCII(listFields, "MIR.job_nam", m_cnJOB_NAM, nFieldSelection, eposJOB_NAM);

    // MIR.OPER_NAM
    _LIST_ADDFIELD_ASCII(listFields, "MIR.oper_nam", m_cnOPER_NAM, nFieldSelection, eposOPER_NAM);

    // MIR.NODE_NAM
    _LIST_ADDFIELD_ASCII(listFields, "MIR.node_nam", m_cnNODE_NAM, nFieldSelection, eposNODE_NAM);

    // MIR.TSTR_TYP
    _LIST_ADDFIELD_ASCII(listFields, "MIR.tstr_typ", m_cnTSTR_TYP, nFieldSelection, eposTSTR_TYP);

    // MIR.EXEC_TYP
    _LIST_ADDFIELD_ASCII(listFields, "MIR.exec_typ", m_cnEXEC_TYP, nFieldSelection, eposEXEC_TYP);

    // MIR.SUPR_NAM
    _LIST_ADDFIELD_ASCII(listFields, "MIR.supr_nam", m_cnSUPR_NAM, nFieldSelection, eposSUPR_NAM);

    // MIR.HAND_ID
    _LIST_ADDFIELD_ASCII(listFields, "MIR.hand_id", m_cnHAND_ID, nFieldSelection, eposHAND_ID);

    // MIR.SBLOT_ID
    _LIST_ADDFIELD_ASCII(listFields, "MIR.sblot_id", m_cnSBLOT_ID, nFieldSelection, eposSBLOT_ID);

    // MIR.JOB_REV
    _LIST_ADDFIELD_ASCII(listFields, "MIR.job_rev", m_cnJOB_REV, nFieldSelection, eposJOB_REV);

    // MIR.PROC_ID
    _LIST_ADDFIELD_ASCII(listFields, "MIR.proc_id", m_cnPROC_ID, nFieldSelection, eposPROC_ID);

    // MIR.PRB_CARD
    _LIST_ADDFIELD_ASCII(listFields, "MIR.prb_card", m_cnPRB_CARD, nFieldSelection, eposPRB_CARD);
}



///////////////////////////////////////////////////////////
// MRR RECORD
///////////////////////////////////////////////////////////
Stdf_MRR_V3::Stdf_MRR_V3() : Stdf_Record()
{
    Reset();
}

Stdf_MRR_V3::~Stdf_MRR_V3()
{
    Reset();
}

void Stdf_MRR_V3::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Select fields for reduced list
    m_pFieldFlags[eposFINISH_T]	|= FieldFlag_ReducedList;

    // Reset Data
    m_u4FINISH_T		= 0;		// MRR.FINISH_T
    m_u4PART_CNT		= 0;		// MRR.PART_CNT
    m_i4RTST_CNT		= -1;		// MRR.RTST_CNT
    m_i4ABRT_CNT		= -1;		// MRR.ABRT_CNT
    m_i4GOOD_CNT		= -1;		// MRR.GOOD_CNT
    m_i4FUNC_CNT		= -1;		// MRR.FUNC_CNT
    m_c1DISP_COD		= ' ';		// MRR.DISP_COD
    m_cnUSR_DESC		= "";		// MRR.USR_DESC
    m_cnEXC_DESC		= "";		// MRR.EXC_DESC
}

bool Stdf_MRR_V3::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;
    return false;
}

bool Stdf_MRR_V3::IsFieldPresent(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Present))
        return true;
    return false;
}

QString Stdf_MRR_V3::GetRecordShortName(void)
{
    return "MRR";
}

QString Stdf_MRR_V3::GetRecordLongName(void)
{
    return "Master Results Record";
}

int Stdf_MRR_V3::GetRecordType(void)
{
    return Rec_MRR;
}

bool Stdf_MRR_V3::Read(GS::StdLib::Stdf & lStdf)
{
    char	szString[STDF_MAX_U1+1];
    long	dwData;
    BYTE	bData;

    // First reset data
    Reset();

    // MRR.FINISH_T
    _FIELD_CHECKREAD(lStdf.ReadDword(&dwData), eposFINISH_T);
    _FIELD_SET(m_u4FINISH_T = stdf_type_u4(dwData), true, eposFINISH_T);

    // MRR.PART_CNT
    _FIELD_CHECKREAD(lStdf.ReadDword(&dwData), eposPART_CNT);
    _FIELD_SET(m_u4PART_CNT = stdf_type_u4(dwData), true, eposPART_CNT);

    // MRR.RTST_CNT
    _FIELD_CHECKREAD(lStdf.ReadDword(&dwData), eposRTST_CNT);
    _FIELD_SET(m_i4RTST_CNT = stdf_type_i4(dwData), m_i4RTST_CNT != -1, eposRTST_CNT);

    // MRR.ABRT_CNT
    _FIELD_CHECKREAD(lStdf.ReadDword(&dwData), eposABRT_CNT);
    _FIELD_SET(m_i4ABRT_CNT = stdf_type_i4(dwData), m_i4ABRT_CNT != -1, eposABRT_CNT);

    // MRR.GOOD_CNT
    _FIELD_CHECKREAD(lStdf.ReadDword(&dwData), eposGOOD_CNT);
    _FIELD_SET(m_i4GOOD_CNT = stdf_type_i4(dwData), m_i4GOOD_CNT != -1, eposGOOD_CNT);

    // MRR.FUNC_CNT
    _FIELD_CHECKREAD(lStdf.ReadDword(&dwData), eposFUNC_CNT);
    _FIELD_SET(m_i4FUNC_CNT = stdf_type_i4(dwData), m_i4FUNC_CNT != -1, eposFUNC_CNT);

    // MRR.DISP_COD
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposDISP_COD);
    _FIELD_SET(m_c1DISP_COD = stdf_type_c1(bData), m_c1DISP_COD != ' ', eposDISP_COD);

    // MRR.USR_DESC
    _FIELD_CHECKREAD(lStdf.ReadString(szString), eposUSR_DESC);
    _FIELD_SET(m_cnUSR_DESC = szString, !m_cnUSR_DESC.isEmpty(), eposUSR_DESC);

    // MRR.EXC_DESC
    _FIELD_CHECKREAD(lStdf.ReadString(szString), eposEXC_DESC);
    _FIELD_SET(m_cnEXC_DESC = szString, !m_cnEXC_DESC.isEmpty(), eposEXC_DESC);

    return true;
}

void Stdf_MRR_V3::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
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
    // GCORE-178
    m_strFieldValue_macro.sprintf("%s (0x%lx)",
                                  ( GS::StdLib::Stdf::GetDaysOfWeekString(clDateTime.date().dayOfWeek()-1)
                                    + clDateTime.toString(" dd ")
                                    + GS::StdLib::Stdf::GetMonthName(clDateTime.date().month()-1)
                                    + clDateTime.toString(" yyyy h:mm:ss") )
                                  .toLatin1().constData(), (long unsigned int)m_u4FINISH_T);
    _STR_ADDFIELD_ASCII(strAsciiString, "MRR.finish_t", m_strFieldValue_macro, nFieldSelection, eposFINISH_T);

    // MRR.PART_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "MRR.part_cnt", QString::number(m_u4PART_CNT), nFieldSelection, eposPART_CNT);

    // MRR.RTST_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "MRR.rtst_cnt", QString::number(m_i4RTST_CNT), nFieldSelection, eposRTST_CNT);

    // MRR.ABRT_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "MRR.abrt_cnt", QString::number(m_i4ABRT_CNT), nFieldSelection, eposABRT_CNT);

    // MRR.GOOD_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "MRR.good_cnt", QString::number(m_i4GOOD_CNT), nFieldSelection, eposGOOD_CNT);

    // MRR.FUNC_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "MRR.func_cnt", QString::number(m_i4FUNC_CNT), nFieldSelection, eposFUNC_CNT);

    // MRR.DISP_COD
    _CREATEFIELD_FROM_C1_ASCII(m_c1DISP_COD);
    _STR_ADDFIELD_ASCII(strAsciiString, "MRR.disp_cod", m_strFieldValue_macro, nFieldSelection, eposDISP_COD);

    // MRR.USR_DESC
    _STR_ADDFIELD_ASCII(strAsciiString, "MRR.usr_desc", m_cnUSR_DESC, nFieldSelection, eposUSR_DESC);

    // MRR.EXC_DESC
    _STR_ADDFIELD_ASCII(strAsciiString, "MRR.exc_desc", m_cnEXC_DESC, nFieldSelection, eposEXC_DESC);
}

void Stdf_MRR_V3::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
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
    // GCORE-178
    m_strFieldValue_macro.sprintf("%s (0x%lx)",
                                  ( GS::StdLib::Stdf::GetDaysOfWeekString(clDateTime.date().dayOfWeek()-1)
                                    + clDateTime.toString(" dd ")
                                    + GS::StdLib::Stdf::GetMonthName(clDateTime.date().month()-1)
                                    + clDateTime.toString(" yyyy h:mm:ss") )
                                  .toLatin1().constData(), (long unsigned int)m_u4FINISH_T);
    _LIST_ADDFIELD_ASCII(listFields, "MRR.finish_t", m_strFieldValue_macro, nFieldSelection, eposFINISH_T);

    // MRR.PART_CNT
    _LIST_ADDFIELD_ASCII(listFields, "MRR.part_cnt", QString::number(m_u4PART_CNT), nFieldSelection, eposPART_CNT);

    // MRR.RTST_CNT
    _LIST_ADDFIELD_ASCII(listFields, "MRR.rtst_cnt", QString::number(m_i4RTST_CNT), nFieldSelection, eposRTST_CNT);

    // MRR.ABRT_CNT
    _LIST_ADDFIELD_ASCII(listFields, "MRR.abrt_cnt", QString::number(m_i4ABRT_CNT), nFieldSelection, eposABRT_CNT);

    // MRR.GOOD_CNT
    _LIST_ADDFIELD_ASCII(listFields, "MRR.good_cnt", QString::number(m_i4GOOD_CNT), nFieldSelection, eposGOOD_CNT);

    // MRR.FUNC_CNT
    _LIST_ADDFIELD_ASCII(listFields, "MRR.func_cnt", QString::number(m_i4FUNC_CNT), nFieldSelection, eposFUNC_CNT);

    // MRR.DISP_COD
    _CREATEFIELD_FROM_C1_ASCII(m_c1DISP_COD);
    _LIST_ADDFIELD_ASCII(listFields, "MRR.disp_cod", m_strFieldValue_macro, nFieldSelection, eposDISP_COD);

    // MRR.USR_DESC
    _LIST_ADDFIELD_ASCII(listFields, "MRR.usr_desc", m_cnUSR_DESC, nFieldSelection, eposUSR_DESC);

    // MRR.EXC_DESC
    _LIST_ADDFIELD_ASCII(listFields, "MRR.exc_desc", m_cnEXC_DESC, nFieldSelection, eposEXC_DESC);
}


///////////////////////////////////////////////////////////
// WIR RECORD
///////////////////////////////////////////////////////////
Stdf_WIR_V3::Stdf_WIR_V3() : Stdf_Record()
{
    Reset();
}

Stdf_WIR_V3::~Stdf_WIR_V3()
{
    Reset();
}

void Stdf_WIR_V3::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Select fields for reduced list

    // Reset Data
    m_u1HEAD_NUM	= 0;		// WIR.HEAD_NUM
    m_b1PAD_BYTE	= 0;		// WIR.PAD_BYTE
    m_u4START_T		= 0;		// WIR.START_T
    m_cnWAFER_ID	= "";		// WIR.WAFER_ID
}

bool Stdf_WIR_V3::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;
    return false;
}

bool Stdf_WIR_V3::IsFieldPresent(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Present))
        return true;
    return false;
}

QString Stdf_WIR_V3::GetRecordShortName(void)
{
    return "WIR";
}

QString Stdf_WIR_V3::GetRecordLongName(void)
{
    return "Wafer Information Record";
}

int Stdf_WIR_V3::GetRecordType(void)
{
    return Rec_WIR;
}

bool Stdf_WIR_V3::Read(GS::StdLib::Stdf & lStdf)
{
    char	szString[STDF_MAX_U1+1];
    long	dwData;
    BYTE	bData;

    // First reset data
    Reset();

    // WIR.HEAD_NUM
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposHEAD_NUM);
    _FIELD_SET(m_u1HEAD_NUM = stdf_type_u1(bData), true, eposHEAD_NUM);

    // WIR.PAD_BYTE
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposPAD_BYTE);
    _FIELD_SET(m_b1PAD_BYTE = stdf_type_b1(bData), true, eposPAD_BYTE);

    // WIR.START_T
    _FIELD_CHECKREAD(lStdf.ReadDword(&dwData), eposSTART_T);
    _FIELD_SET(m_u4START_T = stdf_type_u4(dwData), true, eposSTART_T);

    // WIR.WAFER_ID
    _FIELD_CHECKREAD(lStdf.ReadString(szString), eposWAFER_ID);
    _FIELD_SET(m_cnWAFER_ID = szString, !m_cnWAFER_ID.isEmpty(), eposWAFER_ID);

    return true;
}

void Stdf_WIR_V3::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    QDateTime	clDateTime;
    time_t		tDateTime;

    // Set QDateTime object to UTC mode
    clDateTime.setTimeSpec(Qt::UTC);

    // Empty string first
    strAsciiString = "";

    // WIR.HEAD_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "WIR.head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // WIR.PAD_BYTE
    _CREATEFIELD_FROM_B1_ASCII(m_b1PAD_BYTE)
    _STR_ADDFIELD_ASCII(strAsciiString, "WIR.pad_byte", m_strFieldValue_macro, nFieldSelection, eposPAD_BYTE);

    // WIR.START_T
    tDateTime = (time_t)m_u4START_T;
    clDateTime.setTime_t(tDateTime);
    // GCORE-178
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

void Stdf_WIR_V3::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
    QDateTime	clDateTime;
    time_t		tDateTime;

    // Set QDateTime object to UTC mode
    clDateTime.setTimeSpec(Qt::UTC);

    // Empty string list first
    listFields.empty();

    // WIR.HEAD_NUM
    _LIST_ADDFIELD_ASCII(listFields, "WIR.head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // WIR.PAD_BYTE
    _CREATEFIELD_FROM_B1_ASCII(m_b1PAD_BYTE)
    _LIST_ADDFIELD_ASCII(listFields, "WIR.pad_byte", m_strFieldValue_macro, nFieldSelection, eposPAD_BYTE);

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


///////////////////////////////////////////////////////////
// WRR RECORD
///////////////////////////////////////////////////////////
Stdf_WRR_V3::Stdf_WRR_V3() : Stdf_Record()
{
    Reset();
}

Stdf_WRR_V3::~Stdf_WRR_V3()
{
    Reset();
}

void Stdf_WRR_V3::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Select fields for reduced list

    // Reset Data
    m_u4FINISH_T	= 0;		// WRR.FINISH_T
    m_u1HEAD_NUM	= 0;		// WRR.HEAD_NUM
    m_b1PAD_BYTE	= 0;		// WRR.PAD_BYTE
    m_u4PART_CNT	= 0;		// WRR.PART_CNT
    m_i4RTST_CNT	= -1;		// WRR.RTST_CNT
    m_i4ABRT_CNT	= -1;		// WRR.ABRT_CNT
    m_i4GOOD_CNT	= -1;		// WRR.GOOD_CNT
    m_i4FUNC_CNT	= -1;		// WRR.FUNC_CNT
    m_cnWAFER_ID	= "";		// WRR.WAFER_ID
    m_cnHAND_ID     = "";       // WRR.HAND_ID
    m_cnPRB_CARD    = "";       // WRR.PRB_CARD
    m_cnUSR_DESC	= "";		// WRR.USR_DESC
    m_cnEXC_DESC	= "";		// WRR.EXC_DESC
}

bool Stdf_WRR_V3::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;
    return false;
}

bool Stdf_WRR_V3::IsFieldPresent(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Present))
        return true;
    return false;
}

void Stdf_WRR_V3::InvalidateField(int nFieldPos){
    if((nFieldPos >= 0) && (nFieldPos < eposEND)){
        m_pFieldFlags[nFieldPos] &= !(FieldFlag_Valid);
        m_pFieldFlags[nFieldPos] |= FieldFlag_Present;
    }
}

QString Stdf_WRR_V3::GetRecordShortName(void)
{
    return "WRR";
}

QString Stdf_WRR_V3::GetRecordLongName(void)
{
    return "Wafer Results Record";
}

int Stdf_WRR_V3::GetRecordType(void)
{
    return Rec_WRR;
}

bool Stdf_WRR_V3::Read(GS::StdLib::Stdf & lStdf)
{
    char	szString[STDF_MAX_U1+1];
    long	dwData;
    BYTE	bData;

    // First reset data
    Reset();

    // WRR.FINISH_T
    _FIELD_CHECKREAD(lStdf.ReadDword(&dwData), eposFINISH_T);
    _FIELD_SET(m_u4FINISH_T = stdf_type_u4(dwData), true, eposFINISH_T);

    // WRR.HEAD_NUM
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposHEAD_NUM);
    _FIELD_SET(m_u1HEAD_NUM = stdf_type_u1(bData), true, eposHEAD_NUM);

    // WRR.PAD_BYTE
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposPAD_BYTE);
    _FIELD_SET(m_b1PAD_BYTE = stdf_type_b1(bData), true, eposPAD_BYTE);

    // WRR.PART_CNT
    _FIELD_CHECKREAD(lStdf.ReadDword(&dwData), eposPART_CNT);
    _FIELD_SET(m_u4PART_CNT = stdf_type_u4(dwData), true, eposPART_CNT);

    // WRR.RTST_CNT
    _FIELD_CHECKREAD(lStdf.ReadDword(&dwData), eposRTST_CNT);
    _FIELD_SET(m_i4RTST_CNT = stdf_type_u4(dwData), m_i4RTST_CNT != -1, eposRTST_CNT);

    // WRR.ABRT_CNT
    _FIELD_CHECKREAD(lStdf.ReadDword(&dwData), eposABRT_CNT);
    _FIELD_SET(m_i4ABRT_CNT = stdf_type_u4(dwData), m_i4ABRT_CNT != -1, eposABRT_CNT);

    // WRR.GOOD_CNT
    _FIELD_CHECKREAD(lStdf.ReadDword(&dwData), eposGOOD_CNT);
    _FIELD_SET(m_i4GOOD_CNT = stdf_type_u4(dwData), m_i4GOOD_CNT != -1, eposGOOD_CNT);

    // WRR.FUNC_CNT
    _FIELD_CHECKREAD(lStdf.ReadDword(&dwData), eposFUNC_CNT);
    _FIELD_SET(m_i4FUNC_CNT = stdf_type_u4(dwData), m_i4FUNC_CNT != -1, eposFUNC_CNT);

    // WRR.WAFER_ID
    _FIELD_CHECKREAD(lStdf.ReadString(szString), eposWAFER_ID);
    _FIELD_SET(m_cnWAFER_ID = szString, !m_cnWAFER_ID.isEmpty(), eposWAFER_ID);

    // WRR.HAND_ID
    _FIELD_CHECKREAD(lStdf.ReadString(szString), eposHAND_ID);
    _FIELD_SET(m_cnHAND_ID = szString, !m_cnHAND_ID.isEmpty(), eposHAND_ID);

    // WRR.PRB_CARD
    _FIELD_CHECKREAD(lStdf.ReadString(szString), eposPRB_CARD);
    _FIELD_SET(m_cnPRB_CARD = szString, !m_cnPRB_CARD.isEmpty(), eposPRB_CARD);

    // WRR.USR_DESC
    _FIELD_CHECKREAD(lStdf.ReadString(szString), eposUSR_DESC);
    _FIELD_SET(m_cnUSR_DESC = szString, !m_cnUSR_DESC.isEmpty(), eposUSR_DESC);

    // WRR.EXC_DESC
    _FIELD_CHECKREAD(lStdf.ReadString(szString), eposEXC_DESC);
    _FIELD_SET(m_cnEXC_DESC = szString, !m_cnEXC_DESC.isEmpty(), eposEXC_DESC);

    return true;
}

void Stdf_WRR_V3::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    QDateTime	clDateTime;
    time_t		tDateTime;

    // Set QDateTime object to UTC mode
    clDateTime.setTimeSpec(Qt::UTC);

    // Empty string first
    strAsciiString = "";

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

    // WRR.HEAD_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "WRR.head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // WRR.PAD_BYTE
    _CREATEFIELD_FROM_B1_ASCII(m_b1PAD_BYTE)
    _STR_ADDFIELD_ASCII(strAsciiString, "WRR.pad_byte", m_strFieldValue_macro, nFieldSelection, eposPAD_BYTE);

    // WRR.PART_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "WRR.part_cnt", QString::number(m_u4PART_CNT), nFieldSelection, eposPART_CNT);

    // WRR.RTST_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "WRR.rtst_cnt", QString::number(m_i4RTST_CNT), nFieldSelection, eposRTST_CNT);

    // WRR.ABRT_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "WRR.abrt_cnt", QString::number(m_i4ABRT_CNT), nFieldSelection, eposABRT_CNT);

    // WRR.GOOD_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "WRR.good_cnt", QString::number(m_i4GOOD_CNT), nFieldSelection, eposGOOD_CNT);

    // WRR.FUNC_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "WRR.func_cnt", QString::number(m_i4FUNC_CNT), nFieldSelection, eposFUNC_CNT);

    // WRR.WAFER_ID
    _STR_ADDFIELD_ASCII(strAsciiString, "WRR.wafer_id", m_cnWAFER_ID, nFieldSelection, eposWAFER_ID);

    // WRR.HAND_ID
    _STR_ADDFIELD_ASCII(strAsciiString, "WRR.hand_id", m_cnHAND_ID, nFieldSelection, eposHAND_ID);

    // WRR.PRB_CARD
    _STR_ADDFIELD_ASCII(strAsciiString, "WRR.prb_card", m_cnPRB_CARD, nFieldSelection, eposPRB_CARD);

    // WRR.USR_DESC
    _STR_ADDFIELD_ASCII(strAsciiString, "WRR.usr_desc", m_cnUSR_DESC, nFieldSelection, eposUSR_DESC);

    // WRR.EXC_DESC
    _STR_ADDFIELD_ASCII(strAsciiString, "WRR.exc_desc", m_cnEXC_DESC, nFieldSelection, eposEXC_DESC);
}

void Stdf_WRR_V3::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
    QDateTime	clDateTime;
    time_t		tDateTime;

    // Set QDateTime object to UTC mode
    clDateTime.setTimeSpec(Qt::UTC);

    // Empty string list first
    listFields.empty();

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

    // WRR.HEAD_NUM
    _LIST_ADDFIELD_ASCII(listFields, "WRR.head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // WIR.PAD_BYTE
    _CREATEFIELD_FROM_B1_ASCII(m_b1PAD_BYTE)
    _LIST_ADDFIELD_ASCII(listFields, "WRR.pad_byte", m_strFieldValue_macro, nFieldSelection, eposPAD_BYTE);

    // WRR.PART_CNT
    _LIST_ADDFIELD_ASCII(listFields, "WRR.part_cnt", QString::number(m_u4PART_CNT), nFieldSelection, eposPART_CNT);

    // WRR.RTST_CNT
    _LIST_ADDFIELD_ASCII(listFields, "WRR.rtst_cnt", QString::number(m_i4RTST_CNT), nFieldSelection, eposRTST_CNT);

    // WRR.ABRT_CNT
    _LIST_ADDFIELD_ASCII(listFields, "WRR.abrt_cnt", QString::number(m_i4ABRT_CNT), nFieldSelection, eposABRT_CNT);

    // WRR.GOOD_CNT
    _LIST_ADDFIELD_ASCII(listFields, "WRR.good_cnt", QString::number(m_i4GOOD_CNT), nFieldSelection, eposGOOD_CNT);

    // WRR.FUNC_CNT
    _LIST_ADDFIELD_ASCII(listFields, "WRR.func_cnt", QString::number(m_i4FUNC_CNT), nFieldSelection, eposFUNC_CNT);

    // WRR.WAFER_ID
    _LIST_ADDFIELD_ASCII(listFields, "WRR.wafer_id", m_cnWAFER_ID, nFieldSelection, eposWAFER_ID);

    // WRR.HAND_ID
    _LIST_ADDFIELD_ASCII(listFields, "WRR.hand_id", m_cnHAND_ID, nFieldSelection, eposHAND_ID);

    // WRR.PRB_CARD
    _LIST_ADDFIELD_ASCII(listFields, "WRR.prb_card", m_cnPRB_CARD, nFieldSelection, eposPRB_CARD);

    // WRR.USR_DESC
    _LIST_ADDFIELD_ASCII(listFields, "WRR.usr_desc", m_cnUSR_DESC, nFieldSelection, eposUSR_DESC);

    // WRR.EXC_DESC
    _LIST_ADDFIELD_ASCII(listFields, "WRR.exc_desc", m_cnEXC_DESC, nFieldSelection, eposEXC_DESC);
}


///////////////////////////////////////////////////////////
// WCR RECORD
///////////////////////////////////////////////////////////
Stdf_WCR_V3::Stdf_WCR_V3() : Stdf_Record()
{
    Reset();
}

Stdf_WCR_V3::~Stdf_WCR_V3()
{
    Reset();
}

void Stdf_WCR_V3::Reset(void)
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
}

bool Stdf_WCR_V3::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;
    return false;
}

bool Stdf_WCR_V3::IsFieldPresent(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Present))
        return true;
    return false;
}

QString Stdf_WCR_V3::GetRecordShortName(void)
{
    return "WCR";
}

QString Stdf_WCR_V3::GetRecordLongName(void)
{
    return "Wafer Configuration Record";
}

int Stdf_WCR_V3::GetRecordType(void)
{
    return Rec_WCR;
}

bool Stdf_WCR_V3::Read(GS::StdLib::Stdf & lStdf)
{
    float	fData;
    int		wData;
    BYTE	bData;

    // First reset data
    Reset();

    // WCR.WAFR_SIZ
    _FIELD_CHECKREAD(lStdf.ReadFloat(&fData), eposWAFR_SIZ);
    _FIELD_SET(m_r4WAFR_SIZ = stdf_type_r4(fData), m_r4WAFR_SIZ != 0.0, eposWAFR_SIZ);

    // WCR.DIE_HT
    _FIELD_CHECKREAD(lStdf.ReadFloat(&fData), eposDIE_HT);
    _FIELD_SET(m_r4DIE_HT = stdf_type_r4(fData), m_r4DIE_HT != 0.0, eposDIE_HT);

    // WCR.DIE_WID
    _FIELD_CHECKREAD(lStdf.ReadFloat(&fData), eposDIE_WID);
    _FIELD_SET(m_r4DIE_WID = stdf_type_r4(fData), m_r4DIE_WID != 0.0, eposDIE_WID);

    // WCR.WF_UNITS
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposWF_UNITS);
    _FIELD_SET(m_u1WF_UNITS = stdf_type_u1(bData), m_u1WF_UNITS != 0, eposWF_UNITS);

    // WCR.WF_FLAT
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposWF_FLAT);
    _FIELD_SET(m_c1WF_FLAT = stdf_type_c1(bData), m_c1WF_FLAT != ' ', eposWF_FLAT);

    // WCR.CENTER_X
    _FIELD_CHECKREAD(lStdf.ReadWord(&wData), eposCENTER_X);
    _FIELD_SET(m_i2CENTER_X = stdf_type_i2(wData), m_i2CENTER_X != -32768, eposCENTER_X);

    // WCR.CENTER_Y
    _FIELD_CHECKREAD(lStdf.ReadWord(&wData), eposCENTER_Y);
    _FIELD_SET(m_i2CENTER_Y = stdf_type_i2(wData), m_i2CENTER_Y != -32768, eposCENTER_Y);

    // WCR.POS_X
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposPOS_X);
    _FIELD_SET(m_c1POS_X = stdf_type_c1(bData), m_c1POS_X != ' ', eposPOS_X);

    // WCR.POS_Y
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposPOS_Y);
    _FIELD_SET(m_c1POS_Y = stdf_type_c1(bData), m_c1POS_Y != ' ', eposPOS_Y);

    return true;
}

void Stdf_WCR_V3::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
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

void Stdf_WCR_V3::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
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



///////////////////////////////////////////////////////////
// HBR RECORD
///////////////////////////////////////////////////////////
Stdf_HBR_V3::Stdf_HBR_V3() : Stdf_Record()
{
    Reset();
}

Stdf_HBR_V3::~Stdf_HBR_V3()
{
    Reset();
}

void Stdf_HBR_V3::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Select fields for reduced list

    // Reset Data
    m_u2HBIN_NUM	= 0;		// HBR.HBIN_NUM
    m_u4HBIN_CNT	= 0;		// HBR.HBIN_CNT
    m_cnHBIN_NAM	= "";		// HBR.HBIN_NAM
}

bool Stdf_HBR_V3::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;
    return false;
}

bool Stdf_HBR_V3::IsFieldPresent(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Present))
        return true;
    return false;
}


QString Stdf_HBR_V3::GetRecordShortName(void)
{
    return "HBR";
}

QString Stdf_HBR_V3::GetRecordLongName(void)
{
    return "Hardware Bin Record";
}

int Stdf_HBR_V3::GetRecordType(void)
{
    return Rec_HBR;
}

bool Stdf_HBR_V3::Read(GS::StdLib::Stdf & lStdf)
{
    char	szString[STDF_MAX_U1+1];
    long	dwData;
    int		wData;

    // First reset data
    Reset();

    // HBR.HBIN_NUM
    _FIELD_CHECKREAD(lStdf.ReadWord(&wData), eposHBIN_NUM);
    _FIELD_SET(m_u2HBIN_NUM = stdf_type_u2(wData), true, eposHBIN_NUM);

    // HBR.HBIN_CNT
    _FIELD_CHECKREAD(lStdf.ReadDword(&dwData), eposHBIN_CNT);
    _FIELD_SET(m_u4HBIN_CNT = stdf_type_u4(dwData), true, eposHBIN_CNT);

    // HBR.HBIN_NAM
    _FIELD_CHECKREAD(lStdf.ReadString(szString), eposHBIN_NAM);
    _FIELD_SET(m_cnHBIN_NAM = szString, !m_cnHBIN_NAM.isEmpty(), eposHBIN_NAM);

    return true;
}


void Stdf_HBR_V3::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    // Empty string first
    strAsciiString = "";

    // HBR.HBIN_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "HBR.hbin_num", QString::number(m_u2HBIN_NUM), nFieldSelection, eposHBIN_NUM);

    // HBR.HBIN_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "HBR.hbin_cnt", QString::number(m_u4HBIN_CNT), nFieldSelection, eposHBIN_CNT);

    // HBR.HBIN_NAM
    _STR_ADDFIELD_ASCII(strAsciiString, "HBR.hbin_nam", m_cnHBIN_NAM, nFieldSelection, eposHBIN_NAM);
}

void Stdf_HBR_V3::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
    // Empty string list first
    listFields.empty();

    // HBR.HBIN_NUM
    _LIST_ADDFIELD_ASCII(listFields, "HBR.hbin_num", QString::number(m_u2HBIN_NUM), nFieldSelection, eposHBIN_NUM);

    // HBR.HBIN_CNT
    _LIST_ADDFIELD_ASCII(listFields, "HBR.hbin_cnt", QString::number(m_u4HBIN_CNT), nFieldSelection, eposHBIN_CNT);

    // HBR.HBIN_NAM
    _LIST_ADDFIELD_ASCII(listFields, "HBR.hbin_nam", m_cnHBIN_NAM, nFieldSelection, eposHBIN_NAM);
}


///////////////////////////////////////////////////////////
// SBR RECORD
///////////////////////////////////////////////////////////
Stdf_SBR_V3::Stdf_SBR_V3() : Stdf_Record()
{
    Reset();
}

Stdf_SBR_V3::~Stdf_SBR_V3()
{
    Reset();
}

void Stdf_SBR_V3::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Select fields for reduced list

    // Reset Data
    m_u2SBIN_NUM	= 0;		// SBR.SBIN_NUM
    m_u4SBIN_CNT	= 0;		// SBR.SBIN_CNT
    m_cnSBIN_NAM	= "";		// SBR.SBIN_NAM
}

bool Stdf_SBR_V3::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;
    return false;
}

bool Stdf_SBR_V3::IsFieldPresent(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Present))
        return true;
    return false;
}

QString Stdf_SBR_V3::GetRecordShortName(void)
{
    return "SBR";
}

QString Stdf_SBR_V3::GetRecordLongName(void)
{
    return "Software Bin Record";
}

int Stdf_SBR_V3::GetRecordType(void)
{
    return Rec_SBR;
}

bool Stdf_SBR_V3::Read(GS::StdLib::Stdf & lStdf)
{
    char	szString[STDF_MAX_U1+1];
    long	dwData;
    int		wData;

    // First reset data
    Reset();

    // SBR.SBIN_NUM
    _FIELD_CHECKREAD(lStdf.ReadWord(&wData), eposSBIN_NUM);
    _FIELD_SET(m_u2SBIN_NUM = stdf_type_u2(wData), true, eposSBIN_NUM);

    // SBR.SBIN_CNT
    _FIELD_CHECKREAD(lStdf.ReadDword(&dwData), eposSBIN_CNT);
    _FIELD_SET(m_u4SBIN_CNT = stdf_type_u4(dwData), true, eposSBIN_CNT);

    // SBR.SBIN_NAM
    _FIELD_CHECKREAD(lStdf.ReadString(szString), eposSBIN_NAM);
    _FIELD_SET(m_cnSBIN_NAM = szString, !m_cnSBIN_NAM.isEmpty(), eposSBIN_NAM);

    return true;
}

void Stdf_SBR_V3::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    // Empty string first
    strAsciiString = "";

    // SBR.SBIN_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "SBR.sbin_num", QString::number(m_u2SBIN_NUM), nFieldSelection, eposSBIN_NUM);

    // SBR.SBIN_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "SBR.sbin_cnt", QString::number(m_u4SBIN_CNT), nFieldSelection, eposSBIN_CNT);

    // SBR.SBIN_NAM
    _STR_ADDFIELD_ASCII(strAsciiString, "SBR.sbin_nam", m_cnSBIN_NAM, nFieldSelection, eposSBIN_NAM);
}

void Stdf_SBR_V3::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
    // Empty string list first
    listFields.empty();

    // SBR.SBIN_NUM
    _LIST_ADDFIELD_ASCII(listFields, "SBR.sbin_num", QString::number(m_u2SBIN_NUM), nFieldSelection, eposSBIN_NUM);

    // SBR.SBIN_CNT
    _LIST_ADDFIELD_ASCII(listFields, "SBR.sbin_cnt", QString::number(m_u4SBIN_CNT), nFieldSelection, eposSBIN_CNT);

    // SBR.SBIN_NAM
    _LIST_ADDFIELD_ASCII(listFields, "SBR.sbin_nam", m_cnSBIN_NAM, nFieldSelection, eposSBIN_NAM);
}


///////////////////////////////////////////////////////////
// TSR RECORD
///////////////////////////////////////////////////////////
Stdf_TSR_V3::Stdf_TSR_V3() : Stdf_Record()
{
    Reset();
}

Stdf_TSR_V3::~Stdf_TSR_V3()
{
    Reset();
}

void Stdf_TSR_V3::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Select fields for reduced list

    // Reset Data
    m_u4TEST_NUM	= 0;				// TSR.TEST_NUM
    m_i4EXEC_CNT	= -1;               // TSR.EXEC_CNT
    m_i4FAIL_CNT	= -1;               // TSR.FAIL_CNT
    m_i4ALRM_CNT	= -1;               // TSR.ALRM_CNT
    m_cnTEST_NAM	= "";				// TSR.TEST_NAM
    m_cnSEQ_NAME	= "";				// TSR.SEQ_NAME
    m_b1OPT_FLAG	= 0;				// TSR.OPT_FLAG
    m_b1PAD_BYTE	= 0;				// TSR.PAD_BYTE
    m_r4TEST_MIN	= 0.0;				// TSR.TEST_MIN
    m_r4TEST_MAX	= 0.0;				// TSR.TEST_MAX
    m_r4TST_MEAN    = 0.0;              // TSR.TST_MEAN
    m_r4TST_SQRS    = 0.0;              // TSR.TST_SQRS
    m_r4TST_SUMS	= 0.0;				// TSR.TST_SUMS
    m_r4TST_SQRS	= 0.0;				// TSR.TST_SQRS
}

bool Stdf_TSR_V3::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;
    return false;
}

bool Stdf_TSR_V3::IsFieldPresent(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Present))
        return true;
    return false;
}

QString Stdf_TSR_V3::GetRecordShortName(void)
{
    return "TSR";
}

QString Stdf_TSR_V3::GetRecordLongName(void)
{
    return "Test Synopsis Record";
}

int Stdf_TSR_V3::GetRecordType(void)
{
    return Rec_TSR;
}

bool Stdf_TSR_V3::Read(GS::StdLib::Stdf & lStdf)
{
    char	szString[STDF_MAX_U1+1];
    long	dwData;
    float	fData;
    BYTE	bData;

    // First reset data
    Reset();

    // TSR.TEST_NUM
    _FIELD_CHECKREAD(lStdf.ReadDword(&dwData), eposTEST_NUM);
    _FIELD_SET(m_u4TEST_NUM = stdf_type_u4(dwData), true, eposTEST_NUM);

    // TSR.EXEC_CNT
    _FIELD_CHECKREAD(lStdf.ReadDword(&dwData), eposEXEC_CNT);
    _FIELD_SET(m_i4EXEC_CNT = stdf_type_i4(dwData), m_i4EXEC_CNT != -1, eposEXEC_CNT);

    // TSR.FAIL_CNT
    _FIELD_CHECKREAD(lStdf.ReadDword(&dwData), eposFAIL_CNT);
    _FIELD_SET(m_i4FAIL_CNT = stdf_type_i4(dwData), m_i4FAIL_CNT != -1, eposFAIL_CNT);

    // TSR.ALRM_CNT
    _FIELD_CHECKREAD(lStdf.ReadDword(&dwData), eposALRM_CNT);
    _FIELD_SET(m_i4ALRM_CNT = stdf_type_i4(dwData), m_i4ALRM_CNT != -1, eposALRM_CNT);

    // TSR.OPT_FLAG
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposOPT_FLAG);
    _FIELD_SET(m_b1OPT_FLAG = stdf_type_b1(bData), true, eposOPT_FLAG);

    // TSR.PAD_BYTE
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposPAD_BYTE);
    _FIELD_SET(m_b1PAD_BYTE = stdf_type_b1(bData), true, eposPAD_BYTE);

    // TSR.TEST_MIN
    _FIELD_CHECKREAD(lStdf.ReadFloat(&fData), eposTEST_MIN);
    _FIELD_SET(m_r4TEST_MIN = stdf_type_r4(fData), (m_b1OPT_FLAG & STDF_MASK_BIT0) == 0, eposTEST_MIN);

    // TSR.TEST_MAX
    _FIELD_CHECKREAD(lStdf.ReadFloat(&fData), eposTEST_MAX);
    _FIELD_SET(m_r4TEST_MAX = stdf_type_r4(fData), (m_b1OPT_FLAG & STDF_MASK_BIT1) == 0, eposTEST_MAX);

    // TSR.TST_MEAN
    _FIELD_CHECKREAD(lStdf.ReadFloat(&fData), eposTST_MEAN);
    _FIELD_SET(m_r4TST_MEAN = stdf_type_r4(fData), (m_b1OPT_FLAG & STDF_MASK_BIT2) == 0, eposTST_MEAN);

    // TSR.TST_SDEV
    _FIELD_CHECKREAD(lStdf.ReadFloat(&fData), eposTST_SDEV);
    _FIELD_SET(m_r4TST_SDEV = stdf_type_r4(fData), (m_b1OPT_FLAG & STDF_MASK_BIT3) == 0, eposTST_SDEV);

    // TSR.TST_SUMS
    _FIELD_CHECKREAD(lStdf.ReadFloat(&fData), eposTST_SUMS);
    _FIELD_SET(m_r4TST_SUMS = stdf_type_r4(fData), (m_b1OPT_FLAG & STDF_MASK_BIT4) == 0, eposTST_SUMS);

    // TSR.TST_SQRS
    _FIELD_CHECKREAD(lStdf.ReadFloat(&fData), eposTST_SQRS);
    _FIELD_SET(m_r4TST_SQRS = stdf_type_r4(fData), (m_b1OPT_FLAG & STDF_MASK_BIT5) == 0, eposTST_SQRS);

    // TSR.TEST_NAM
    _FIELD_CHECKREAD(lStdf.ReadString(szString), eposTEST_NAM);
    _FIELD_SET(m_cnTEST_NAM = szString, !m_cnTEST_NAM.isEmpty(), eposTEST_NAM);

    // TSR.SEQ_NAME
    _FIELD_CHECKREAD(lStdf.ReadString(szString), eposSEQ_NAME);
    _FIELD_SET(m_cnSEQ_NAME = szString, !m_cnSEQ_NAME.isEmpty(), eposSEQ_NAME);

    return true;
}

void Stdf_TSR_V3::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    // Empty string first
    strAsciiString = "";

    // TSR.TEST_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "TSR.test_num", QString::number(m_u4TEST_NUM), nFieldSelection, eposTEST_NUM);

    // TSR.EXEC_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "TSR.exec_cnt", QString::number(m_i4EXEC_CNT), nFieldSelection, eposEXEC_CNT);

    // TSR.FAIL_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "TSR.fail_cnt", QString::number(m_i4FAIL_CNT), nFieldSelection, eposFAIL_CNT);

    // TSR.ALRM_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "TSR.alrm_cnt", QString::number(m_i4ALRM_CNT), nFieldSelection, eposALRM_CNT);

    // TSR.OPT_FLAG
    _CREATEFIELD_FROM_B1_ASCII(m_b1OPT_FLAG)
    _STR_ADDFIELD_ASCII(strAsciiString, "TSR.opt_flag", m_strFieldValue_macro, nFieldSelection, eposOPT_FLAG);

    // TSR.PAD_BYTE
    _CREATEFIELD_FROM_B1_ASCII(m_b1PAD_BYTE)
    _STR_ADDFIELD_ASCII(strAsciiString, "TSR.pad_byte", m_strFieldValue_macro, nFieldSelection, eposPAD_BYTE);

    // TSR.TEST_MIN
    _STR_ADDFIELD_ASCII(strAsciiString, "TSR.test_min", QString::number(m_r4TEST_MIN), nFieldSelection, eposTEST_MIN);

    // TSR.TEST_MAX
    _STR_ADDFIELD_ASCII(strAsciiString, "TSR.test_max", QString::number(m_r4TEST_MAX), nFieldSelection, eposTEST_MAX);

    // TSR.TST_MEAN
    _STR_ADDFIELD_ASCII(strAsciiString, "TSR.tst_mean", QString::number(m_r4TST_MEAN), nFieldSelection, eposTST_MEAN);

    // TSR.TST_SDEV
    _STR_ADDFIELD_ASCII(strAsciiString, "TSR.tst_sdev", QString::number(m_r4TST_SDEV), nFieldSelection, eposTST_SDEV);

    // TSR.TST_SUMS
    _STR_ADDFIELD_ASCII(strAsciiString, "TSR.tst_sums", QString::number(m_r4TST_SUMS), nFieldSelection, eposTST_SUMS);

    // TSR.TST_SQRS
    _STR_ADDFIELD_ASCII(strAsciiString, "TSR.tst_sqrs", QString::number(m_r4TST_SQRS), nFieldSelection, eposTST_SQRS);

    // TSR.TEST_NAM
    _STR_ADDFIELD_ASCII(strAsciiString, "TSR.test_nam", m_cnTEST_NAM, nFieldSelection, eposTEST_NAM);

    // TSR.SEQ_NAME
    _STR_ADDFIELD_ASCII(strAsciiString, "TSR.seq_name", m_cnSEQ_NAME, nFieldSelection, eposSEQ_NAME);

}

void Stdf_TSR_V3::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{

    // Empty string list first
    listFields.empty();

    // TSR.TEST_NUM
    _LIST_ADDFIELD_ASCII(listFields, "TSR.test_num", QString::number(m_u4TEST_NUM), nFieldSelection, eposTEST_NUM);

    // TSR.EXEC_CNT
    _LIST_ADDFIELD_ASCII(listFields, "TSR.exec_cnt", QString::number(m_i4EXEC_CNT), nFieldSelection, eposEXEC_CNT);

    // TSR.FAIL_CNT
    _LIST_ADDFIELD_ASCII(listFields, "TSR.fail_cnt", QString::number(m_i4FAIL_CNT), nFieldSelection, eposFAIL_CNT);

    // TSR.ALRM_CNT
    _LIST_ADDFIELD_ASCII(listFields, "TSR.alrm_cnt", QString::number(m_i4ALRM_CNT), nFieldSelection, eposALRM_CNT);

    // TSR.OPT_FLAG
    _CREATEFIELD_FROM_B1_ASCII(m_b1OPT_FLAG)
    _LIST_ADDFIELD_ASCII(listFields, "TSR.opt_flag", m_strFieldValue_macro, nFieldSelection, eposOPT_FLAG);

    // TSR.PAD_BYTE
    _CREATEFIELD_FROM_B1_ASCII(m_b1PAD_BYTE)
    _LIST_ADDFIELD_ASCII(listFields, "TSR.pad_byte", m_strFieldValue_macro, nFieldSelection, eposPAD_BYTE);

    // TSR.TEST_MIN
    _LIST_ADDFIELD_ASCII(listFields, "TSR.test_min", QString::number(m_r4TEST_MIN), nFieldSelection, eposTEST_MIN);

    // TSR.TEST_MAX
    _LIST_ADDFIELD_ASCII(listFields, "TSR.test_max", QString::number(m_r4TEST_MAX), nFieldSelection, eposTEST_MAX);

    // TSR.TST_MEAN
    _LIST_ADDFIELD_ASCII(listFields, "TSR.tst_mean", QString::number(m_r4TST_MEAN), nFieldSelection, eposTST_MEAN);

    // TSR.TST_SDEV
    _LIST_ADDFIELD_ASCII(listFields, "TSR.tst_sdev", QString::number(m_r4TST_SDEV), nFieldSelection, eposTST_SDEV);

    // TSR.TST_SUMS
    _LIST_ADDFIELD_ASCII(listFields, "TSR.tst_sums", QString::number(m_r4TST_SUMS), nFieldSelection, eposTST_SUMS);

    // TSR.TST_SQRS
    _LIST_ADDFIELD_ASCII(listFields, "TSR.tst_sqrs", QString::number(m_r4TST_SQRS), nFieldSelection, eposTST_SQRS);

    // TSR.TEST_NAM
    _LIST_ADDFIELD_ASCII(listFields, "TSR.test_nam", m_cnTEST_NAM, nFieldSelection, eposTEST_NAM);

    // TSR.SEQ_NAME
    _LIST_ADDFIELD_ASCII(listFields, "TSR.seq_name", m_cnSEQ_NAME, nFieldSelection, eposSEQ_NAME);
}




///////////////////////////////////////////////////////////
// SHB RECORD
///////////////////////////////////////////////////////////
Stdf_SHB_V3::Stdf_SHB_V3() : Stdf_Record()
{
    Reset();
}

Stdf_SHB_V3::~Stdf_SHB_V3()
{
    Reset();
}

void Stdf_SHB_V3::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Select fields for reduced list

    // Reset Data
    m_u1HEAD_NUM	= 0;		// SHB.HEAD_NUM
    m_u1SITE_NUM	= 0;		// SHB.SITE_NUM
    m_u2HBIN_NUM	= 0;		// SHB.HBIN_NUM
    m_u4HBIN_CNT	= 0;		// SHB.HBIN_CNT
    m_cnHBIN_NAM	= "";		// SHB.HBIN_NAM
}

bool Stdf_SHB_V3::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;
    return false;
}

bool Stdf_SHB_V3::IsFieldPresent(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Present))
        return true;
    return false;
}

QString Stdf_SHB_V3::GetRecordShortName(void)
{
    return "SHB";
}

QString Stdf_SHB_V3::GetRecordLongName(void)
{
    return "Site-Specific Hardware Bin Record";
}

int Stdf_SHB_V3::GetRecordType(void)
{
    return Rec_SHB;
}

bool Stdf_SHB_V3::Read(GS::StdLib::Stdf & lStdf)
{
    char	szString[STDF_MAX_U1+1];
    long	dwData;
    int		wData;
    BYTE	bData;

    // First reset data
    Reset();

    // SHB.HEAD_NUM
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposHEAD_NUM);
    _FIELD_SET(m_u1HEAD_NUM = stdf_type_u1(bData), true, eposHEAD_NUM);

    // SHB.SITE_NUM
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposSITE_NUM);
    _FIELD_SET(m_u1SITE_NUM = stdf_type_u1(bData), true, eposSITE_NUM);

    // SHB.HBIN_NUM
    _FIELD_CHECKREAD(lStdf.ReadWord(&wData), eposHBIN_NUM);
    _FIELD_SET(m_u2HBIN_NUM = stdf_type_u2(wData), true, eposHBIN_NUM);

    // SHB.HBIN_CNT
    _FIELD_CHECKREAD(lStdf.ReadDword(&dwData), eposHBIN_CNT);
    _FIELD_SET(m_u4HBIN_CNT = stdf_type_u4(dwData), true, eposHBIN_CNT);

    // SHB.HBIN_NAM
    _FIELD_CHECKREAD(lStdf.ReadString(szString), eposHBIN_NAM);
    _FIELD_SET(m_cnHBIN_NAM = szString, !m_cnHBIN_NAM.isEmpty(), eposHBIN_NAM);

    return true;
}


void Stdf_SHB_V3::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    // Empty string first
    strAsciiString = "";

    // SHB.HEAD_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "SHB.head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // SHB.SITE_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "SHB.site_num", QString::number(m_u1SITE_NUM), nFieldSelection, eposSITE_NUM);

    // SHB.HBIN_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "SHB.hbin_num", QString::number(m_u2HBIN_NUM), nFieldSelection, eposHBIN_NUM);

    // SHB.HBIN_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "SHB.hbin_cnt", QString::number(m_u4HBIN_CNT), nFieldSelection, eposHBIN_CNT);

    // SHB.HBIN_NAM
    _STR_ADDFIELD_ASCII(strAsciiString, "SHB.hbin_nam", m_cnHBIN_NAM, nFieldSelection, eposHBIN_NAM);
}

void Stdf_SHB_V3::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
    // Empty string list first
    listFields.empty();

    // SHB.HEAD_NUM
    _LIST_ADDFIELD_ASCII(listFields, "SHB.head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // SHB.SITE_NUM
    _LIST_ADDFIELD_ASCII(listFields, "SHB.site_num", QString::number(m_u1SITE_NUM), nFieldSelection, eposSITE_NUM);

    // SHB.HBIN_NUM
    _LIST_ADDFIELD_ASCII(listFields, "SHB.hbin_num", QString::number(m_u2HBIN_NUM), nFieldSelection, eposHBIN_NUM);

    // SHB.HBIN_CNT
    _LIST_ADDFIELD_ASCII(listFields, "SHB.hbin_cnt", QString::number(m_u4HBIN_CNT), nFieldSelection, eposHBIN_CNT);

    // SHB.HBIN_NAM
    _LIST_ADDFIELD_ASCII(listFields, "SHB.hbin_nam", m_cnHBIN_NAM, nFieldSelection, eposHBIN_NAM);
}



///////////////////////////////////////////////////////////
// SSB RECORD
///////////////////////////////////////////////////////////
Stdf_SSB_V3::Stdf_SSB_V3() : Stdf_Record()
{
    Reset();
}

Stdf_SSB_V3::~Stdf_SSB_V3()
{
    Reset();
}

void Stdf_SSB_V3::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Select fields for reduced list

    // Reset Data
    m_u1HEAD_NUM	= 0;		// SSB.HEAD_NUM
    m_u1SITE_NUM	= 0;		// SSB.SITE_NUM
    m_u2SBIN_NUM	= 0;		// SSB.SBIN_NUM
    m_u4SBIN_CNT	= 0;		// SSB.SBIN_CNT
    m_cnSBIN_NAM	= "";		// SSB.SBIN_NAM
}

bool Stdf_SSB_V3::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;
    return false;
}

bool Stdf_SSB_V3::IsFieldPresent(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Present))
        return true;
    return false;
}

QString Stdf_SSB_V3::GetRecordShortName(void)
{
    return "SSB";
}

QString Stdf_SSB_V3::GetRecordLongName(void)
{
    return "Site-Specific Hardware Bin Record";
}

int Stdf_SSB_V3::GetRecordType(void)
{
    return Rec_SSB;
}

bool Stdf_SSB_V3::Read(GS::StdLib::Stdf & lStdf)
{
    char	szString[STDF_MAX_U1+1];
    long	dwData;
    int		wData;
    BYTE	bData;

    // First reset data
    Reset();

    // SSB.HEAD_NUM
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposHEAD_NUM);
    _FIELD_SET(m_u1HEAD_NUM = stdf_type_u1(bData), true, eposHEAD_NUM);

    // SSB.SITE_NUM
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposSITE_NUM);
    _FIELD_SET(m_u1SITE_NUM = stdf_type_u1(bData), true, eposSITE_NUM);

    // SSB.SBIN_NUM
    _FIELD_CHECKREAD(lStdf.ReadWord(&wData), eposSBIN_NUM);
    _FIELD_SET(m_u2SBIN_NUM = stdf_type_u2(wData), true, eposSBIN_NUM);

    // SSB.SBIN_CNT
    _FIELD_CHECKREAD(lStdf.ReadDword(&dwData), eposSBIN_CNT);
    _FIELD_SET(m_u4SBIN_CNT = stdf_type_u4(dwData), true, eposSBIN_CNT);

    // SSB.SBIN_NAM
    _FIELD_CHECKREAD(lStdf.ReadString(szString), eposSBIN_NAM);
    _FIELD_SET(m_cnSBIN_NAM = szString, !m_cnSBIN_NAM.isEmpty(), eposSBIN_NAM);

    return true;
}


void Stdf_SSB_V3::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    // Empty string first
    strAsciiString = "";

    // SSB.HEAD_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "SSB.head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // SSB.SITE_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "SSB.site_num", QString::number(m_u1SITE_NUM), nFieldSelection, eposSITE_NUM);

    // SSB.SBIN_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "SSB.SBIN_num", QString::number(m_u2SBIN_NUM), nFieldSelection, eposSBIN_NUM);

    // SSB.SBIN_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "SSB.SBIN_cnt", QString::number(m_u4SBIN_CNT), nFieldSelection, eposSBIN_CNT);

    // SSB.SBIN_NAM
    _STR_ADDFIELD_ASCII(strAsciiString, "SSB.SBIN_nam", m_cnSBIN_NAM, nFieldSelection, eposSBIN_NAM);
}

void Stdf_SSB_V3::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
    // Empty string list first
    listFields.empty();

    // SSB.HEAD_NUM
    _LIST_ADDFIELD_ASCII(listFields, "SSB.head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // SSB.SITE_NUM
    _LIST_ADDFIELD_ASCII(listFields, "SSB.site_num", QString::number(m_u1SITE_NUM), nFieldSelection, eposSITE_NUM);

    // SSB.SBIN_NUM
    _LIST_ADDFIELD_ASCII(listFields, "SSB.SBIN_num", QString::number(m_u2SBIN_NUM), nFieldSelection, eposSBIN_NUM);

    // SSB.SBIN_CNT
    _LIST_ADDFIELD_ASCII(listFields, "SSB.SBIN_cnt", QString::number(m_u4SBIN_CNT), nFieldSelection, eposSBIN_CNT);

    // SSB.SBIN_NAM
    _LIST_ADDFIELD_ASCII(listFields, "SSB.SBIN_nam", m_cnSBIN_NAM, nFieldSelection, eposSBIN_NAM);
}


///////////////////////////////////////////////////////////
// STS RECORD
///////////////////////////////////////////////////////////
Stdf_STS_V3::Stdf_STS_V3() : Stdf_Record()
{
    Reset();
}

Stdf_STS_V3::~Stdf_STS_V3()
{
    Reset();
}

void Stdf_STS_V3::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Select fields for reduced list

    // Reset Data
    m_u1HEAD_NUM	= 0;                // STS.HEAD_NUM
    m_u1SITE_NUM	= 0;                // STS.SITE_NUM
    m_u4TEST_NUM	= 0;				// STS.TEST_NUM
    m_i4FAIL_CNT	= -1;               // STS.FAIL_CNT
    m_i4ALRM_CNT	= -1;               // STS.ALRM_CNT
    m_cnTEST_NAM	= "";				// STS.TEST_NAM
    m_cnSEQ_NAME	= "";				// STS.SEQ_NAME
    m_b1OPT_FLAG	= 0;				// STS.OPT_FLAG
    m_b1PAD_BYTE	= 0;				// STS.PAD_BYTE
    m_r4TEST_MIN	= 0.0;				// STS.TEST_MIN
    m_r4TEST_MAX	= 0.0;				// STS.TEST_MAX
    m_r4TST_MEAN    = 0.0;              // STS.TST_MEAN
    m_r4TST_SQRS    = 0.0;              // STS.TST_SQRS
    m_r4TST_SUMS	= 0.0;				// STS.TST_SUMS
    m_r4TST_SQRS	= 0.0;				// STS.TST_SQRS
    m_cnTEST_LBL    = "";               // STS.m_cnTEST_LBL
}

bool Stdf_STS_V3::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;
    return false;
}

bool Stdf_STS_V3::IsFieldPresent(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Present))
        return true;
    return false;
}

QString Stdf_STS_V3::GetRecordShortName(void)
{
    return "STS";
}

QString Stdf_STS_V3::GetRecordLongName(void)
{
    return "Site-Specific Test Synopsis Record";
}

int Stdf_STS_V3::GetRecordType(void)
{
    return Rec_STS;
}

bool Stdf_STS_V3::Read(GS::StdLib::Stdf & lStdf)
{
    char	szString[STDF_MAX_U1+1];
    long	dwData;
    float	fData;
    BYTE	bData;

    // First reset data
    Reset();

    // STS.HEAD_NUM
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposHEAD_NUM);
    _FIELD_SET(m_u1HEAD_NUM = stdf_type_u1(bData), true, eposHEAD_NUM);

    // STS.SITE_NUM
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposSITE_NUM);
    _FIELD_SET(m_u1SITE_NUM = stdf_type_u1(bData), true, eposSITE_NUM);

    // STS.TEST_NUM
    _FIELD_CHECKREAD(lStdf.ReadDword(&dwData), eposTEST_NUM);
    _FIELD_SET(m_u4TEST_NUM = stdf_type_u4(dwData), true, eposTEST_NUM);

    // STS.EXEC_CNT
    _FIELD_CHECKREAD(lStdf.ReadDword(&dwData), eposEXEC_CNT);
    _FIELD_SET(m_i4EXEC_CNT = stdf_type_i4(dwData), m_i4EXEC_CNT != -1, eposEXEC_CNT);

    // STS.FAIL_CNT
    _FIELD_CHECKREAD(lStdf.ReadDword(&dwData), eposFAIL_CNT);
    _FIELD_SET(m_i4FAIL_CNT = stdf_type_i4(dwData), m_i4FAIL_CNT != -1, eposFAIL_CNT);

    // STS.ALRM_CNT
    _FIELD_CHECKREAD(lStdf.ReadDword(&dwData), eposALRM_CNT);
    _FIELD_SET(m_i4ALRM_CNT = stdf_type_i4(dwData), m_i4ALRM_CNT != -1, eposALRM_CNT);

    // STS.OPT_FLAG
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposOPT_FLAG);
    _FIELD_SET(m_b1OPT_FLAG = stdf_type_b1(bData), true, eposOPT_FLAG);

    // STS.PAD_BYTE
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposPAD_BYTE);
    _FIELD_SET(m_b1PAD_BYTE = stdf_type_b1(bData), true, eposPAD_BYTE);

    // STS.TEST_MIN
    _FIELD_CHECKREAD(lStdf.ReadFloat(&fData), eposTEST_MIN);
    _FIELD_SET(m_r4TEST_MIN = stdf_type_r4(fData), (m_b1OPT_FLAG & STDF_MASK_BIT0) == 0, eposTEST_MIN);

    // STS.TEST_MAX
    _FIELD_CHECKREAD(lStdf.ReadFloat(&fData), eposTEST_MAX);
    _FIELD_SET(m_r4TEST_MAX = stdf_type_r4(fData), (m_b1OPT_FLAG & STDF_MASK_BIT1) == 0, eposTEST_MAX);

    // STS.TST_MEAN
    _FIELD_CHECKREAD(lStdf.ReadFloat(&fData), eposTST_MEAN);
    _FIELD_SET(m_r4TST_MEAN = stdf_type_r4(fData), (m_b1OPT_FLAG & STDF_MASK_BIT2) == 0, eposTST_MEAN);

    // STS.TST_SDEV
    _FIELD_CHECKREAD(lStdf.ReadFloat(&fData), eposTST_SDEV);
    _FIELD_SET(m_r4TST_SDEV = stdf_type_r4(fData), (m_b1OPT_FLAG & STDF_MASK_BIT3) == 0, eposTST_SDEV);

    // STS.TST_SUMS
    _FIELD_CHECKREAD(lStdf.ReadFloat(&fData), eposTST_SUMS);
    _FIELD_SET(m_r4TST_SUMS = stdf_type_r4(fData), (m_b1OPT_FLAG & STDF_MASK_BIT4) == 0, eposTST_SUMS);

    // STS.TST_SQRS
    _FIELD_CHECKREAD(lStdf.ReadFloat(&fData), eposTST_SQRS);
    _FIELD_SET(m_r4TST_SQRS = stdf_type_r4(fData), (m_b1OPT_FLAG & STDF_MASK_BIT5) == 0, eposTST_SQRS);

    // STS.TEST_NAM
    _FIELD_CHECKREAD(lStdf.ReadString(szString), eposTEST_NAM);
    _FIELD_SET(m_cnTEST_NAM = szString, !m_cnTEST_NAM.isEmpty(), eposTEST_NAM);

    // STS.SEQ_NAME
    _FIELD_CHECKREAD(lStdf.ReadString(szString), eposSEQ_NAME);
    _FIELD_SET(m_cnSEQ_NAME = szString, !m_cnSEQ_NAME.isEmpty(), eposSEQ_NAME);

    // STS.TEST_LBL
    _FIELD_CHECKREAD(lStdf.ReadString(szString), eposTEST_LBL);
    _FIELD_SET(m_cnTEST_LBL = szString, !m_cnTEST_LBL.isEmpty(), eposTEST_LBL);

    return true;
}

void Stdf_STS_V3::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    // Empty string first
    strAsciiString = "";

    // STS.HEAD_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "STS.head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // STS.SITE_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "STS.site_num", QString::number(m_u1SITE_NUM), nFieldSelection, eposSITE_NUM);

    // STS.TEST_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "STS.test_num", QString::number(m_u4TEST_NUM), nFieldSelection, eposTEST_NUM);

    // STS.EXEC_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "STS.exec_cnt", QString::number(m_i4EXEC_CNT), nFieldSelection, eposEXEC_CNT);

    // STS.FAIL_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "STS.fail_cnt", QString::number(m_i4FAIL_CNT), nFieldSelection, eposFAIL_CNT);

    // STS.ALRM_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "STS.alrm_cnt", QString::number(m_i4ALRM_CNT), nFieldSelection, eposALRM_CNT);

    // STS.OPT_FLAG
    _CREATEFIELD_FROM_B1_ASCII(m_b1OPT_FLAG)
    _STR_ADDFIELD_ASCII(strAsciiString, "STS.opt_flag", m_strFieldValue_macro, nFieldSelection, eposOPT_FLAG);

    // STS.PAD_BYTE
    _CREATEFIELD_FROM_B1_ASCII(m_b1PAD_BYTE)
    _STR_ADDFIELD_ASCII(strAsciiString, "STS.pad_byte", m_strFieldValue_macro, nFieldSelection, eposPAD_BYTE);

    // STS.TEST_MIN
    _STR_ADDFIELD_ASCII(strAsciiString, "STS.test_min", QString::number(m_r4TEST_MIN), nFieldSelection, eposTEST_MIN);

    // STS.TEST_MAX
    _STR_ADDFIELD_ASCII(strAsciiString, "STS.test_max", QString::number(m_r4TEST_MAX), nFieldSelection, eposTEST_MAX);

    // STS.TST_MEAN
    _STR_ADDFIELD_ASCII(strAsciiString, "STS.tst_mean", QString::number(m_r4TST_MEAN), nFieldSelection, eposTST_MEAN);

    // STS.TST_SDEV
    _STR_ADDFIELD_ASCII(strAsciiString, "STS.tst_sdev", QString::number(m_r4TST_SDEV), nFieldSelection, eposTST_SDEV);

    // STS.TST_SUMS
    _STR_ADDFIELD_ASCII(strAsciiString, "STS.tst_sums", QString::number(m_r4TST_SUMS), nFieldSelection, eposTST_SUMS);

    // STS.TST_SQRS
    _STR_ADDFIELD_ASCII(strAsciiString, "STS.tst_sqrs", QString::number(m_r4TST_SQRS), nFieldSelection, eposTST_SQRS);

    // STS.TEST_NAM
    _STR_ADDFIELD_ASCII(strAsciiString, "STS.test_nam", m_cnTEST_NAM, nFieldSelection, eposTEST_NAM);

    // STS.SEQ_NAME
    _STR_ADDFIELD_ASCII(strAsciiString, "STS.seq_name", m_cnSEQ_NAME, nFieldSelection, eposSEQ_NAME);

    // STS.TEST_LBL
    _STR_ADDFIELD_ASCII(strAsciiString, "STS.test_lbl", m_cnTEST_LBL, nFieldSelection, eposTEST_LBL);
}

void Stdf_STS_V3::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{

    // Empty string list first
    listFields.empty();

    // SHB.HEAD_NUM
    _LIST_ADDFIELD_ASCII(listFields, "STS.head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // SHB.SITE_NUM
    _LIST_ADDFIELD_ASCII(listFields, "STS.site_num", QString::number(m_u1SITE_NUM), nFieldSelection, eposSITE_NUM);

    // STS.TEST_NUM
    _LIST_ADDFIELD_ASCII(listFields, "STS.test_num", QString::number(m_u4TEST_NUM), nFieldSelection, eposTEST_NUM);

    // STS.EXEC_CNT
    _LIST_ADDFIELD_ASCII(listFields, "STS.exec_cnt", QString::number(m_i4EXEC_CNT), nFieldSelection, eposEXEC_CNT);

    // STS.FAIL_CNT
    _LIST_ADDFIELD_ASCII(listFields, "STS.fail_cnt", QString::number(m_i4FAIL_CNT), nFieldSelection, eposFAIL_CNT);

    // STS.ALRM_CNT
    _LIST_ADDFIELD_ASCII(listFields, "STS.alrm_cnt", QString::number(m_i4ALRM_CNT), nFieldSelection, eposALRM_CNT);

    // STS.OPT_FLAG
    _CREATEFIELD_FROM_B1_ASCII(m_b1OPT_FLAG)
    _LIST_ADDFIELD_ASCII(listFields, "STS.opt_flag", m_strFieldValue_macro, nFieldSelection, eposOPT_FLAG);

    // STS.PAD_BYTE
    _CREATEFIELD_FROM_B1_ASCII(m_b1PAD_BYTE)
    _LIST_ADDFIELD_ASCII(listFields, "STS.pad_byte", m_strFieldValue_macro, nFieldSelection, eposPAD_BYTE);

    // STS.TEST_MIN
    _LIST_ADDFIELD_ASCII(listFields, "STS.test_min", QString::number(m_r4TEST_MIN), nFieldSelection, eposTEST_MIN);

    // STS.TEST_MAX
    _LIST_ADDFIELD_ASCII(listFields, "STS.test_max", QString::number(m_r4TEST_MAX), nFieldSelection, eposTEST_MAX);

    // STS.TST_MEAN
    _LIST_ADDFIELD_ASCII(listFields, "STS.tst_mean", QString::number(m_r4TST_MEAN), nFieldSelection, eposTST_MEAN);

    // STS.TST_SDEV
    _LIST_ADDFIELD_ASCII(listFields, "STS.tst_sdev", QString::number(m_r4TST_SDEV), nFieldSelection, eposTST_SDEV);

    // STS.TST_SUMS
    _LIST_ADDFIELD_ASCII(listFields, "STS.tst_sums", QString::number(m_r4TST_SUMS), nFieldSelection, eposTST_SUMS);

    // STS.TST_SQRS
    _LIST_ADDFIELD_ASCII(listFields, "STS.tst_sqrs", QString::number(m_r4TST_SQRS), nFieldSelection, eposTST_SQRS);

    // STS.TEST_NAM
    _LIST_ADDFIELD_ASCII(listFields, "STS.test_nam", m_cnTEST_NAM, nFieldSelection, eposTEST_NAM);

    // STS.SEQ_NAME
    _LIST_ADDFIELD_ASCII(listFields, "STS.seq_name", m_cnSEQ_NAME, nFieldSelection, eposSEQ_NAME);

    // STS.TEST_LBL
    _LIST_ADDFIELD_ASCII(listFields, "STS.test_lbl", m_cnTEST_LBL, nFieldSelection, eposTEST_LBL);
}


///////////////////////////////////////////////////////////
// SCR RECORD
///////////////////////////////////////////////////////////
Stdf_SCR_V3::Stdf_SCR_V3() : Stdf_Record()
{
    Reset();
}

Stdf_SCR_V3::~Stdf_SCR_V3()
{
    Reset();
}

void Stdf_SCR_V3::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Select fields for reduced list

    // Reset Data
    m_u1HEAD_NUM	= 0;		// SCR.HEAD_NUM
    m_u1SITE_NUM	= 0;		// SCR.SITE_NUM
    m_u4FINISH_T	= 0;		// SCR.FINISH_T
    m_u1HEAD_NUM	= 0;		// SCR.HEAD_NUM
    m_u4PART_CNT	= 0;		// SCR.PART_CNT
    m_i4RTST_CNT	= -1;		// SCR.RTST_CNT
    m_i4ABRT_CNT	= -1;		// SCR.ABRT_CNT
    m_i4GOOD_CNT	= -1;		// SCR.GOOD_CNT
    m_i4FUNC_CNT	= -1;		// SCR.FUNC_CNT
}

bool Stdf_SCR_V3::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;
    return false;
}

bool Stdf_SCR_V3::IsFieldPresent(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Present))
        return true;
    return false;
}

QString Stdf_SCR_V3::GetRecordShortName(void)
{
    return "SCR";
}

QString Stdf_SCR_V3::GetRecordLongName(void)
{
    return "Wafer Results Record";
}

int Stdf_SCR_V3::GetRecordType(void)
{
    return Rec_SCR;
}

bool Stdf_SCR_V3::Read(GS::StdLib::Stdf & lStdf)
{
    long	dwData;
    BYTE	bData;

    // First reset data
    Reset();

    // SCR.HEAD_NUM
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposHEAD_NUM);
    _FIELD_SET(m_u1HEAD_NUM = stdf_type_u1(bData), true, eposHEAD_NUM);

    // SHB.SITE_NUM
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposSITE_NUM);
    _FIELD_SET(m_u1SITE_NUM = stdf_type_u1(bData), true, eposSITE_NUM);

    // SCR.FINISH_T
    _FIELD_CHECKREAD(lStdf.ReadDword(&dwData), eposFINISH_T);
    _FIELD_SET(m_u4FINISH_T = stdf_type_u4(dwData), true, eposFINISH_T);

    // SCR.PART_CNT
    _FIELD_CHECKREAD(lStdf.ReadDword(&dwData), eposPART_CNT);
    _FIELD_SET(m_u4PART_CNT = stdf_type_u4(dwData), true, eposPART_CNT);

    // SCR.RTST_CNT
    _FIELD_CHECKREAD(lStdf.ReadDword(&dwData), eposRTST_CNT);
    _FIELD_SET(m_i4RTST_CNT = stdf_type_i4(dwData), m_i4RTST_CNT != -1, eposRTST_CNT);

    // SCR.ABRT_CNT
    _FIELD_CHECKREAD(lStdf.ReadDword(&dwData), eposABRT_CNT);
    _FIELD_SET(m_i4ABRT_CNT = stdf_type_i4(dwData), m_i4ABRT_CNT != -1, eposABRT_CNT);

    // SCR.GOOD_CNT
    _FIELD_CHECKREAD(lStdf.ReadDword(&dwData), eposGOOD_CNT);
    _FIELD_SET(m_i4GOOD_CNT = stdf_type_i4(dwData), m_i4GOOD_CNT != -1, eposGOOD_CNT);

    // SCR.FUNC_CNT
    _FIELD_CHECKREAD(lStdf.ReadDword(&dwData), eposFUNC_CNT);
    _FIELD_SET(m_i4FUNC_CNT = stdf_type_i4(dwData), m_i4FUNC_CNT != -1, eposFUNC_CNT);

    return true;
}

void Stdf_SCR_V3::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    QDateTime	clDateTime;
    time_t		tDateTime;

    // Set QDateTime object to UTC mode
    clDateTime.setTimeSpec(Qt::UTC);

    // Empty string first
    strAsciiString = "";

    // SCR.HEAD_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "SCR.head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // SCR.SITE_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "SCR.site_num", QString::number(m_u1SITE_NUM), nFieldSelection, eposSITE_NUM);

    // SCR.FINISH_T
    tDateTime = (time_t)m_u4FINISH_T;
    clDateTime.setTime_t(tDateTime);
    m_strFieldValue_macro.sprintf("%s (0x%lx)",
                                  ( GS::StdLib::Stdf::GetDaysOfWeekString(clDateTime.date().dayOfWeek()-1)
                                    + clDateTime.toString(" dd ")
                                    + GS::StdLib::Stdf::GetMonthName(clDateTime.date().month()-1)
                                    + clDateTime.toString(" yyyy h:mm:ss") )
                                  .toLatin1().constData(), m_u4FINISH_T);
    _STR_ADDFIELD_ASCII(strAsciiString, "SCR.finish_t", m_strFieldValue_macro, nFieldSelection, eposFINISH_T);

    // SCR.PART_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "SCR.part_cnt", QString::number(m_u4PART_CNT), nFieldSelection, eposPART_CNT);

    // SCR.RTST_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "SCR.rtst_cnt", QString::number(m_i4RTST_CNT), nFieldSelection, eposRTST_CNT);

    // SCR.ABRT_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "SCR.abrt_cnt", QString::number(m_i4ABRT_CNT), nFieldSelection, eposABRT_CNT);

    // SCR.GOOD_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "SCR.good_cnt", QString::number(m_i4GOOD_CNT), nFieldSelection, eposGOOD_CNT);

    // SCR.FUNC_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "SCR.func_cnt", QString::number(m_i4FUNC_CNT), nFieldSelection, eposFUNC_CNT);
}

void Stdf_SCR_V3::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
    QDateTime	clDateTime;
    time_t		tDateTime;

    // Set QDateTime object to UTC mode
    clDateTime.setTimeSpec(Qt::UTC);

    // Empty string list first
    listFields.empty();

    // SCR.HEAD_NUM
    _LIST_ADDFIELD_ASCII(listFields, "SCR.head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // SCR.SITE_NUM
    _LIST_ADDFIELD_ASCII(listFields, "SCR.site_num", QString::number(m_u1SITE_NUM), nFieldSelection, eposSITE_NUM);

    // SCR.FINISH_T
    tDateTime = (time_t)m_u4FINISH_T;
    clDateTime.setTime_t(tDateTime);
    m_strFieldValue_macro.sprintf("%s (0x%lx)",
                                  ( GS::StdLib::Stdf::GetDaysOfWeekString(clDateTime.date().dayOfWeek()-1)
                                    + clDateTime.toString(" dd ")
                                    + GS::StdLib::Stdf::GetMonthName(clDateTime.date().month()-1)
                                    + clDateTime.toString(" yyyy h:mm:ss") )
                                  .toLatin1().constData(), m_u4FINISH_T);
    _LIST_ADDFIELD_ASCII(listFields, "SCR.finish_t", m_strFieldValue_macro, nFieldSelection, eposFINISH_T);

    // SCR.PART_CNT
    _LIST_ADDFIELD_ASCII(listFields, "SCR.part_cnt", QString::number(m_u4PART_CNT), nFieldSelection, eposPART_CNT);

    // SCR.RTST_CNT
    _LIST_ADDFIELD_ASCII(listFields, "SCR.rtst_cnt", QString::number(m_i4RTST_CNT), nFieldSelection, eposRTST_CNT);

    // SCR.ABRT_CNT
    _LIST_ADDFIELD_ASCII(listFields, "SCR.abrt_cnt", QString::number(m_i4ABRT_CNT), nFieldSelection, eposABRT_CNT);

    // SCR.GOOD_CNT
    _LIST_ADDFIELD_ASCII(listFields, "SCR.good_cnt", QString::number(m_i4GOOD_CNT), nFieldSelection, eposGOOD_CNT);

    // SCR.FUNC_CNT
    _LIST_ADDFIELD_ASCII(listFields, "SCR.func_cnt", QString::number(m_i4FUNC_CNT), nFieldSelection, eposFUNC_CNT);

}

///////////////////////////////////////////////////////////
// PIR RECORD
///////////////////////////////////////////////////////////
Stdf_PIR_V3::Stdf_PIR_V3() : Stdf_Record()
{
    Reset();
}

Stdf_PIR_V3::~Stdf_PIR_V3()
{
    Reset();
}

void Stdf_PIR_V3::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Select fields for reduced list

    // Reset Data
    m_u1HEAD_NUM	= 0;		// PIR.HEAD_NUM
    m_u1SITE_NUM	= 0;		// PIR.SITE_NUM
    m_i2X_COORD     = -32768;   // PIR.m_i2X_COORD
    m_i2Y_COORD     = -32768;   // PIR.m_i2Y_COORD
    m_cnPART_ID     = "";       // PIR.m_cnPART_ID

}

bool Stdf_PIR_V3::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;
    return false;
}

bool Stdf_PIR_V3::IsFieldPresent(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Present))
        return true;
    return false;
}

QString Stdf_PIR_V3::GetRecordShortName(void)
{
    return "PIR";
}

QString Stdf_PIR_V3::GetRecordLongName(void)
{
    return "Part Information Record";
}

int Stdf_PIR_V3::GetRecordType(void)
{
    return Rec_PIR;
}

bool Stdf_PIR_V3::Read(GS::StdLib::Stdf & lStdf)
{
    BYTE	bData;
    int		wData;
    char	szString[STDF_MAX_U1+1];

    // First reset data
    Reset();

    // PIR.HEAD_NUM
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposHEAD_NUM);
    _FIELD_SET(m_u1HEAD_NUM = stdf_type_u1(bData), true, eposHEAD_NUM);

    // PIR.SITE_NUM
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposSITE_NUM);
    _FIELD_SET(m_u1SITE_NUM = stdf_type_u1(bData), true, eposSITE_NUM);

    // PIR.X_COORD
    _FIELD_CHECKREAD(lStdf.ReadWord(&wData), eposX_COORD);
    _FIELD_SET(m_i2X_COORD = stdf_type_i2(wData), m_i2X_COORD != -32768, eposX_COORD);

    // PIR.Y_COORD
    _FIELD_CHECKREAD(lStdf.ReadWord(&wData), eposY_COORD);
    _FIELD_SET(m_i2Y_COORD = stdf_type_i2(wData), m_i2Y_COORD != -32768, eposY_COORD);

    // PIR.PART_ID
    _FIELD_CHECKREAD(lStdf.ReadString(szString), eposPART_ID);
    _FIELD_SET(m_cnPART_ID = szString, !m_cnPART_ID.isEmpty(), eposPART_ID);

    return true;
}


void Stdf_PIR_V3::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    // Empty string first
    strAsciiString = "";

    // PIR.HEAD_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "PIR.head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // PIR.SITE_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "PIR.site_num", QString::number(m_u1SITE_NUM), nFieldSelection, eposSITE_NUM);

    // PRR.X_COORD
    _STR_ADDFIELD_ASCII(strAsciiString, "PIR.x_coord", QString::number(m_i2X_COORD), nFieldSelection, eposX_COORD);

    // PRR.Y_COORD
    _STR_ADDFIELD_ASCII(strAsciiString, "PIR.y_coord", QString::number(m_i2Y_COORD), nFieldSelection, eposY_COORD);

    // PIR.PART_ID
    _STR_ADDFIELD_ASCII(strAsciiString, "PIR.part_id", m_cnPART_ID, nFieldSelection, eposPART_ID);
}

void Stdf_PIR_V3::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
    QString		strString;

    // Empty string list first
    listFields.empty();

    // PIR.HEAD_NUM
    _LIST_ADDFIELD_ASCII(listFields, "PIR.head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // PIR.SITE_NUM
    _LIST_ADDFIELD_ASCII(listFields, "PIR.site_num", QString::number(m_u1SITE_NUM), nFieldSelection, eposSITE_NUM);

    // PIR.X_COORD
    _LIST_ADDFIELD_ASCII(listFields, "PIR.x_coord", QString::number(m_i2X_COORD), nFieldSelection, eposX_COORD);

    // PIR.Y_COORD
    _LIST_ADDFIELD_ASCII(listFields, "PIR.y_coord", QString::number(m_i2Y_COORD), nFieldSelection, eposY_COORD);

    // PIR.PART_ID
    _LIST_ADDFIELD_ASCII(listFields, "PIR.part_id", m_cnPART_ID, nFieldSelection, eposPART_ID);
}


///////////////////////////////////////////////////////////
// PRR RECORD
///////////////////////////////////////////////////////////
Stdf_PRR_V3::Stdf_PRR_V3() : Stdf_Record()
{
    Reset();
}

Stdf_PRR_V3::~Stdf_PRR_V3()
{
    Reset();
}

void Stdf_PRR_V3::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Select fields for reduced list

    // Reset Data
    m_u1HEAD_NUM	= 0;					// PRR.HEAD_NUM
    m_u1SITE_NUM	= 0;					// PRR.SITE_NUM
    m_b1PART_FLG	= 0;					// PRR.PART_FLG
    m_u2NUM_TEST	= 0;					// PRR.NUM_TEST
    m_u2HARD_BIN	= 0;					// PRR.HARD_BIN
    m_u2SOFT_BIN	= 0;					// PRR.SOFT_BIN
    m_i2X_COORD		= INVALID_SMALLINT;		// PRR.X_COORD
    m_i2Y_COORD		= INVALID_SMALLINT;		// PRR.Y_COORD
    m_cnPART_ID		= "";					// PRR.PART_ID
    m_cnPART_TXT	= "";					// PRR.PART_TXT
    m_bnPART_FIX.m_bLength = 0;				// PRR.PART_FIX
}

bool Stdf_PRR_V3::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;
    return false;
}

bool Stdf_PRR_V3::IsFieldPresent(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Present))
        return true;
    return false;
}

QString Stdf_PRR_V3::GetRecordShortName(void)
{
    return "PRR";
}

QString Stdf_PRR_V3::GetRecordLongName(void)
{
    return "Part Results Record";
}

int Stdf_PRR_V3::GetRecordType(void)
{
    return Rec_PRR;
}

bool Stdf_PRR_V3::Read(GS::StdLib::Stdf & lStdf)
{
    char	szString[STDF_MAX_U1+1];
    int		wData;
    BYTE	bData;

    // First reset data
    Reset();

    // PRR.HEAD_NUM
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposHEAD_NUM);
    _FIELD_SET(m_u1HEAD_NUM = stdf_type_u1(bData), true, eposHEAD_NUM);

    // PRR.SITE_NUM
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposSITE_NUM);
    _FIELD_SET(m_u1SITE_NUM = stdf_type_u1(bData), true, eposSITE_NUM);

    // PRR.NUM_TEST
    _FIELD_CHECKREAD(lStdf.ReadWord(&wData), eposNUM_TEST);
    _FIELD_SET(m_u2NUM_TEST = stdf_type_u2(wData), true, eposNUM_TEST);

    // PRR.HARD_BIN
    _FIELD_CHECKREAD(lStdf.ReadWord(&wData), eposHARD_BIN);
    _FIELD_SET(m_u2HARD_BIN = stdf_type_u2(wData), true, eposHARD_BIN);

    // PRR.SOFT_BIN
    _FIELD_CHECKREAD(lStdf.ReadWord(&wData), eposSOFT_BIN);
    _FIELD_SET(m_u2SOFT_BIN = stdf_type_u2(wData), m_u2SOFT_BIN != 65535, eposSOFT_BIN);

    // PRR.PART_FLG
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposPART_FLG);
    _FIELD_SET(m_b1PART_FLG = stdf_type_b1(bData), true, eposPART_FLG);

    // PRR.PAD_BYTE
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposPAD_BYTE);
    _FIELD_SET(m_b1PAD_BYTE = stdf_type_b1(bData), true, eposPAD_BYTE);

    // PRR.X_COORD
    _FIELD_CHECKREAD(lStdf.ReadWord(&wData), eposX_COORD);
    _FIELD_SET(m_i2X_COORD = stdf_type_i2(wData), m_i2X_COORD != INVALID_SMALLINT, eposX_COORD);

    // PRR.Y_COORD
    _FIELD_CHECKREAD(lStdf.ReadWord(&wData), eposY_COORD);
    _FIELD_SET(m_i2Y_COORD = stdf_type_i2(wData), m_i2Y_COORD != INVALID_SMALLINT, eposY_COORD);

    // PRR.PART_ID
    _FIELD_CHECKREAD(lStdf.ReadString(szString), eposPART_ID);
    _FIELD_SET(m_cnPART_ID = szString, !m_cnPART_ID.isEmpty(), eposPART_ID);

    // PRR.PART_TXT
    _FIELD_CHECKREAD(lStdf.ReadString(szString), eposPART_TXT);
    _FIELD_SET(m_cnPART_TXT = szString, !m_cnPART_TXT.isEmpty(), eposPART_TXT);

    // PRR.PART_FIX
    _FIELD_CHECKREAD(lStdf.ReadBitField(&(m_bnPART_FIX.m_bLength), m_bnPART_FIX.m_pBitField), eposPART_FIX);
    // The field has already been set by the ReadBitField function. We use a dummy instruction as parameter 1 to avoid warning under gcc.
    _FIELD_SET(m_uiIndex_macro = 0, m_bnPART_FIX.m_bLength != 0, eposPART_FIX);

    return true;
}

void Stdf_PRR_V3::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    // Empty string first
    strAsciiString = "";

    // PRR.HEAD_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "PRR.head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // PRR.SITE_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "PRR.site_num", QString::number(m_u1SITE_NUM), nFieldSelection, eposSITE_NUM);

    // PRR.NUM_TEST
    _STR_ADDFIELD_ASCII(strAsciiString, "PRR.num_test", QString::number(m_u2NUM_TEST), nFieldSelection, eposNUM_TEST);

    // PRR.HARD_BIN
    _STR_ADDFIELD_ASCII(strAsciiString, "PRR.hard_bin", QString::number(m_u2HARD_BIN), nFieldSelection, eposHARD_BIN);

    // PRR.SOFT_BIN
    _STR_ADDFIELD_ASCII(strAsciiString, "PRR.soft_bin", QString::number(m_u2SOFT_BIN), nFieldSelection, eposSOFT_BIN);

    // PRR.PART_FLG
    _CREATEFIELD_FROM_B1_ASCII(m_b1PART_FLG)
    _STR_ADDFIELD_ASCII(strAsciiString, "PRR.part_flg", m_strFieldValue_macro, nFieldSelection, eposPART_FLG);

    // PRR.PAD_BYTE
    _CREATEFIELD_FROM_B1_ASCII(m_b1PAD_BYTE)
    _STR_ADDFIELD_ASCII(strAsciiString, "PRR.pad_byte", m_strFieldValue_macro, nFieldSelection, eposPAD_BYTE);

    // PRR.X_COORD
    _STR_ADDFIELD_ASCII(strAsciiString, "PRR.x_coord", QString::number(m_i2X_COORD), nFieldSelection, eposX_COORD);

    // PRR.Y_COORD
    _STR_ADDFIELD_ASCII(strAsciiString, "PRR.y_coord", QString::number(m_i2Y_COORD), nFieldSelection, eposY_COORD);

    // PRR.PART_ID
    _STR_ADDFIELD_ASCII(strAsciiString, "PRR.part_id", m_cnPART_ID, nFieldSelection, eposPART_ID);

    // PRR.PART_TXT
    _STR_ADDFIELD_ASCII(strAsciiString, "PRR.part_txt", m_cnPART_TXT, nFieldSelection, eposPART_TXT);

    // PRR.PART_FIX
    _CREATEFIELD_FROM_BN_ASCII(m_bnPART_FIX);
    QString	strPartFix;
    if(m_bnPART_FIX.m_bLength > 0)
    {
        m_ptChar_macro = m_bnPART_FIX.m_pBitField;
        for(m_uiIndex_macro=0; m_uiIndex_macro<m_bnPART_FIX.m_bLength; m_uiIndex_macro++)
            strPartFix += m_ptChar_macro[m_uiIndex_macro];
    }
    m_strFieldValue_macro += " (\"" + strPartFix;
    m_strFieldValue_macro += "\")";
    _STR_ADDFIELD_ASCII(strAsciiString, "PRR.part_fix", m_strFieldValue_macro, nFieldSelection, eposPART_FIX);
}


void Stdf_PRR_V3::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
    // Empty string list first
    listFields.empty();

    // PRR.HEAD_NUM
    _LIST_ADDFIELD_ASCII(listFields, "PRR.head_num", QString::number(m_u1HEAD_NUM), nFieldSelection, eposHEAD_NUM);

    // PRR.SITE_NUM
    _LIST_ADDFIELD_ASCII(listFields, "PRR.site_num", QString::number(m_u1SITE_NUM), nFieldSelection, eposSITE_NUM);

    // PRR.NUM_TEST
    _LIST_ADDFIELD_ASCII(listFields, "PRR.num_test", QString::number(m_u2NUM_TEST), nFieldSelection, eposNUM_TEST);

    // PRR.HARD_BIN
    _LIST_ADDFIELD_ASCII(listFields, "PRR.hard_bin", QString::number(m_u2HARD_BIN), nFieldSelection, eposHARD_BIN);

    // PRR.SOFT_BIN
    _LIST_ADDFIELD_ASCII(listFields, "PRR.soft_bin", QString::number(m_u2SOFT_BIN), nFieldSelection, eposSOFT_BIN);

    // PRR.PART_FLG
    _CREATEFIELD_FROM_B1_ASCII(m_b1PART_FLG)
    _LIST_ADDFIELD_ASCII(listFields, "PRR.part_flg", m_strFieldValue_macro, nFieldSelection, eposPART_FLG);

    // PRR.X_COORD
    _LIST_ADDFIELD_ASCII(listFields, "PRR.x_coord", QString::number(m_i2X_COORD), nFieldSelection, eposX_COORD);

    // PRR.Y_COORD
    _LIST_ADDFIELD_ASCII(listFields, "PRR.y_coord", QString::number(m_i2Y_COORD), nFieldSelection, eposY_COORD);

    // PRR.PAD_BYTE
    _CREATEFIELD_FROM_B1_ASCII(m_b1PAD_BYTE)
    _LIST_ADDFIELD_ASCII(listFields, "PRR.pad_byte", m_strFieldValue_macro, nFieldSelection, eposPAD_BYTE);

    // PRR.PART_ID
    _LIST_ADDFIELD_ASCII(listFields, "PRR.part_id", m_cnPART_ID, nFieldSelection, eposPART_ID);

    // PRR.PART_TXT
    _LIST_ADDFIELD_ASCII(listFields, "PRR.part_txt", m_cnPART_TXT, nFieldSelection, eposPART_TXT);

    // PRR.PART_FIX
    _CREATEFIELD_FROM_BN_ASCII(m_bnPART_FIX);
    QString	strPartFix;
    if(m_bnPART_FIX.m_bLength > 0)
    {
        m_ptChar_macro = m_bnPART_FIX.m_pBitField;
        for(m_uiIndex_macro=0; m_uiIndex_macro<m_bnPART_FIX.m_bLength; m_uiIndex_macro++)
            strPartFix += m_ptChar_macro[m_uiIndex_macro];
    }
    m_strFieldValue_macro += " (\"" + strPartFix;
    m_strFieldValue_macro += "\")";
    _LIST_ADDFIELD_ASCII(listFields, "PRR.part_fix", m_strFieldValue_macro, nFieldSelection, eposPART_FIX);
}


///////////////////////////////////////////////////////////
// PDR RECORD
///////////////////////////////////////////////////////////
Stdf_PDR_V3::Stdf_PDR_V3() : Stdf_Record()
{
    Reset();
}

Stdf_PDR_V3::~Stdf_PDR_V3()
{
    Reset();
}

void Stdf_PDR_V3::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Select fields for reduced list

    // Reset Data
    m_u4TEST_NUM		= 0;		// PDR.TEST_NUM
    m_b1DESC_FLG		= 0;		// PDR.DESC_FLG
    m_b1OPT_FLAG		= eOPT_FLAG_ALL;		// PDR.OPT_FLAG
    m_i1RES_SCAL		= 0;		// PDR.RES_SCAL
    m_cnUNITS           = "";       // PDR.UNITS
    m_u1RES_LDIG        = 0;        // PDR.RES_LDIG
    m_u1RES_RDIG        = 0;        // PDR.RES_RDIG
    m_i1LLM_SCAL		= 0;		// PDR.LLM_SCAL
    m_i1HLM_SCAL		= 0;		// PDR.HLM_SCAL
    m_u1LLM_LDIG		= 0;		// PDR.LLM_LDIG
    m_u1LLM_RDIG		= 0;		// PDR.LLM_RDIG
    m_u1HLM_LDIG		= 0;		// PDR.HLM_LDIG
    m_u1HLM_RDIG		= 0;		// PDR.HLM_RDIG
    m_r4LO_LIMIT		= 0.0;		// PDR.LO_LIMIT
    m_r4HI_LIMIT		= 0.0;		// PDR.HI_LIMIT
    m_cnTEST_NAM		= "";       // PDR.TEST_NAM
    m_cnSEQ_NAME		= "";       // PDR.SEQ_NAME
}

bool Stdf_PDR_V3::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;
    return false;
}

bool Stdf_PDR_V3::IsFieldPresent(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Present))
        return true;
    return false;
}



QString Stdf_PDR_V3::GetRecordShortName(void)
{
    return "PDR";
}

QString Stdf_PDR_V3::GetRecordLongName(void)
{
    return "Parametric Test Description Record";
}

int Stdf_PDR_V3::GetRecordType(void)
{
    return Rec_PDR;
}

bool Stdf_PDR_V3::Read(GS::StdLib::Stdf & lStdf)
{
    char	szString[STDF_MAX_U1+1];
    long	dwData;
    float	fData;
    BYTE	bData;

    // First reset data
    Reset();

    // PDR.TEST_NUM
    _FIELD_CHECKREAD(lStdf.ReadDword(&dwData), eposTEST_NUM);
    _FIELD_SET(m_u4TEST_NUM = stdf_type_u4(dwData), true, eposTEST_NUM);

    // PDR.DESC_FLG
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposDESC_FLG);
    _FIELD_SET(m_b1DESC_FLG = stdf_type_b1(bData), true, eposDESC_FLG);

    // PDR.OPT_FLAG
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposOPT_FLAG);
    _FIELD_SET(m_b1OPT_FLAG = stdf_type_b1(bData), true, eposOPT_FLAG);

    // PDR.RES_SCAL
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposRES_SCAL);
    _FIELD_SET(m_i1RES_SCAL = stdf_type_i1(bData), (m_b1OPT_FLAG & STDF_MASK_BIT0) == 0, eposRES_SCAL);

    // PDR.UNITS
    int i=0;
    for (i=0; i<7; ++i)
    {
        _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposUNITS);
        szString[i] = bData;
    }
    szString[i] = '\0';
    _FIELD_SET(m_cnUNITS = QString(szString).trimmed(), true, eposUNITS);

    // PDR.RES_LDIG
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposRES_LDIG);
    _FIELD_SET(m_u1RES_LDIG = stdf_type_u1(bData), true, eposRES_LDIG);

    // PDR.RES_RDIG
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposRES_RDIG);
    _FIELD_SET(m_u1RES_RDIG = stdf_type_u1(bData), true, eposRES_RDIG);

    // PDR.LLM_SCAL
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposLLM_SCAL);
    _FIELD_SET(m_i1LLM_SCAL = stdf_type_i1(bData), (m_b1OPT_FLAG & (STDF_MASK_BIT4|STDF_MASK_BIT6)) == 0, eposLLM_SCAL);

    // PDR.HLM_SCAL
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposHLM_SCAL);
    _FIELD_SET(m_i1HLM_SCAL = stdf_type_i1(bData), (m_b1OPT_FLAG & (STDF_MASK_BIT5|STDF_MASK_BIT7)) == 0, eposHLM_SCAL);

    // PDR.LLM_LDIG
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposLLM_LDIG);
    _FIELD_SET(m_u1LLM_LDIG = stdf_type_u1(bData), true, eposLLM_LDIG);

    // PDR.LLM_RDIG
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposLLM_RDIG);
    _FIELD_SET(m_u1LLM_RDIG = stdf_type_u1(bData), true, eposLLM_RDIG);

    // PDR.HLM_LDIG
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposHLM_LDIG);
    _FIELD_SET(m_u1HLM_LDIG = stdf_type_u1(bData), true, eposHLM_LDIG);

    // PDR.HLM_RDIG
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposHLM_RDIG);
    _FIELD_SET(m_u1HLM_RDIG = stdf_type_u1(bData), true, eposHLM_RDIG);

    // PDR.LO_LIMIT
    _FIELD_CHECKREAD(lStdf.ReadFloat(&fData), eposLO_LIMIT);
    _FIELD_SET(m_r4LO_LIMIT = stdf_type_r4(fData), (m_b1OPT_FLAG & (STDF_MASK_BIT4 | STDF_MASK_BIT6)) == 0, eposLO_LIMIT);

    // PDR.HI_LIMIT
    _FIELD_CHECKREAD(lStdf.ReadFloat(&fData), eposHI_LIMIT);
    _FIELD_SET(m_r4HI_LIMIT = stdf_type_r4(fData), (m_b1OPT_FLAG & (STDF_MASK_BIT5 | STDF_MASK_BIT7)) == 0, eposHI_LIMIT);

    // PDR.TEST_NAM
    _FIELD_CHECKREAD(lStdf.ReadString(szString), eposTEST_NAM);
    _FIELD_SET(m_cnTEST_NAM = szString, !m_cnTEST_NAM.isEmpty(), eposTEST_NAM);

    // PDR.SEQ_NAME
    _FIELD_CHECKREAD(lStdf.ReadString(szString), eposSEQ_NAME);
    _FIELD_SET(m_cnSEQ_NAME = szString, !m_cnSEQ_NAME.isEmpty(), eposSEQ_NAME);

    return true;
}

void Stdf_PDR_V3::GetAsciiString(QString & strAsciiString, int nFieldSelection )
{
    // Empty string first
    strAsciiString = "";

    // PDR.TEST_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "PDR.test_num", QString::number(m_u4TEST_NUM), nFieldSelection, eposTEST_NUM);

    // PDR.DESC_FLG
    _CREATEFIELD_FROM_B1_ASCII(m_b1DESC_FLG)
    _STR_ADDFIELD_ASCII(strAsciiString, "PDR.desc_flg", m_strFieldValue_macro, nFieldSelection, eposDESC_FLG);

    // PDR.OPT_FLAG
    _CREATEFIELD_FROM_B1_ASCII(m_b1OPT_FLAG)
    _STR_ADDFIELD_ASCII(strAsciiString, "PDR.opt_flag", m_strFieldValue_macro, nFieldSelection, eposOPT_FLAG);

    // PDR.RES_SCAL
    _STR_ADDFIELD_ASCII(strAsciiString, "PDR.res_scal", QString::number(m_i1RES_SCAL), nFieldSelection, eposRES_SCAL);

    // PDR.UNITS
    _STR_ADDFIELD_ASCII(strAsciiString, "PDR.units", m_cnUNITS, nFieldSelection, eposUNITS);

     // PDR.RES_LDIG
     _STR_ADDFIELD_ASCII(strAsciiString, "PDR.res_ldig", QString::number(m_u1RES_LDIG), nFieldSelection, eposRES_LDIG);

     // PDR.RES_RDIG
     _STR_ADDFIELD_ASCII(strAsciiString, "PDR.res_rdig", QString::number(m_u1RES_RDIG), nFieldSelection, eposRES_RDIG);

     // PDR.LLM_SCAL
     _STR_ADDFIELD_ASCII(strAsciiString, "PDR.llm_scal", QString::number(m_i1LLM_SCAL), nFieldSelection, eposLLM_SCAL);

     // PDR.HLM_SCAL
     _STR_ADDFIELD_ASCII(strAsciiString, "PDR.hlm_scal", QString::number(m_i1HLM_SCAL), nFieldSelection, eposHLM_SCAL);

     // PDR.LLM_LDIG
     _STR_ADDFIELD_ASCII(strAsciiString, "PDR.llm_ldig", QString::number(m_u1LLM_LDIG), nFieldSelection, eposLLM_LDIG);

     // PDR.LLM_RDIG
     _STR_ADDFIELD_ASCII(strAsciiString, "PDR.llm_rdig", QString::number(m_u1LLM_RDIG), nFieldSelection, eposLLM_RDIG);

     // PDR.HLM_LDIG
     _STR_ADDFIELD_ASCII(strAsciiString, "PDR.hlm_ldig", QString::number(m_u1HLM_LDIG), nFieldSelection, eposHLM_LDIG);

     // PDR.HLM_RDIG
     _STR_ADDFIELD_ASCII(strAsciiString, "PDR.hlm_rdig", QString::number(m_u1HLM_RDIG), nFieldSelection, eposHLM_RDIG);

     // PDR.LO_LIMIT
     _STR_ADDFIELD_ASCII(strAsciiString, "PDR.lo_limit", QString::number(m_r4LO_LIMIT), nFieldSelection, eposLO_LIMIT);

     // PDR.HI_LIMIT
     _STR_ADDFIELD_ASCII(strAsciiString, "PDR.hi_limit", QString::number(m_r4HI_LIMIT), nFieldSelection, eposHI_LIMIT);

     // PDR.TEST_NAM
     _STR_ADDFIELD_ASCII(strAsciiString, "PDR.test_nam", m_cnTEST_NAM, nFieldSelection, eposTEST_NAM);

     // PDR.SEQ_NAME
     _STR_ADDFIELD_ASCII(strAsciiString, "PDR.seq_name", m_cnSEQ_NAME, nFieldSelection, eposSEQ_NAME);
}

void Stdf_PDR_V3::GetAsciiFieldList(QStringList & listFields, int nFieldSelection )
{
    // Empty string list first
    listFields.empty();

    // PDR.TEST_NUM
    _LIST_ADDFIELD_ASCII(listFields, "PDR.test_num", QString::number(m_u4TEST_NUM), nFieldSelection, eposTEST_NUM);

    // PDR.DESC_FLG
    _CREATEFIELD_FROM_B1_ASCII(m_b1DESC_FLG)
    _LIST_ADDFIELD_ASCII(listFields, "PDR.desc_flg", m_strFieldValue_macro, nFieldSelection, eposDESC_FLG);

    // PDR.OPT_FLAG
    _CREATEFIELD_FROM_B1_ASCII(m_b1OPT_FLAG)
    _LIST_ADDFIELD_ASCII(listFields, "PDR.opt_flag", m_strFieldValue_macro, nFieldSelection, eposOPT_FLAG);

    // PDR.RES_SCAL
    _LIST_ADDFIELD_ASCII(listFields, "PDR.res_scal", QString::number(m_i1RES_SCAL), nFieldSelection, eposRES_SCAL);

    // PDR.UNITS
    _LIST_ADDFIELD_ASCII(listFields, "PDR.units", m_cnUNITS, nFieldSelection, eposUNITS);

     // PDR.RES_LDIG
     _LIST_ADDFIELD_ASCII(listFields, "PDR.res_ldig", QString::number(m_u1RES_LDIG), nFieldSelection, eposRES_LDIG);

     // PDR.RES_RDIG
     _LIST_ADDFIELD_ASCII(listFields, "PDR.res_rdig", QString::number(m_u1RES_RDIG), nFieldSelection, eposRES_RDIG);

     // PDR.LLM_SCAL
     _LIST_ADDFIELD_ASCII(listFields, "PDR.llm_scal", QString::number(m_i1LLM_SCAL), nFieldSelection, eposLLM_SCAL);

     // PDR.HLM_SCAL
     _LIST_ADDFIELD_ASCII(listFields, "PDR.hlm_scal", QString::number(m_i1HLM_SCAL), nFieldSelection, eposHLM_SCAL);

     // PDR.LLM_LDIG
     _LIST_ADDFIELD_ASCII(listFields, "PDR.llm_ldig", QString::number(m_u1LLM_LDIG), nFieldSelection, eposLLM_LDIG);

     // PDR.LLM_RDIG
     _LIST_ADDFIELD_ASCII(listFields, "PDR.llm_rdig", QString::number(m_u1LLM_RDIG), nFieldSelection, eposLLM_RDIG);

     // PDR.HLM_LDIG
     _LIST_ADDFIELD_ASCII(listFields, "PDR.hlm_ldig", QString::number(m_u1HLM_LDIG), nFieldSelection, eposHLM_LDIG);

     // PDR.HLM_RDIG
     _LIST_ADDFIELD_ASCII(listFields, "PDR.hlm_rdig", QString::number(m_u1HLM_RDIG), nFieldSelection, eposHLM_RDIG);

     // PDR.LO_LIMIT
     _LIST_ADDFIELD_ASCII(listFields, "PDR.lo_limit", QString::number(m_r4LO_LIMIT), nFieldSelection, eposLO_LIMIT);

     // PDR.HI_LIMIT
     _LIST_ADDFIELD_ASCII(listFields, "PDR.hi_limit", QString::number(m_r4HI_LIMIT), nFieldSelection, eposHI_LIMIT);

     // PDR.TEST_NAM
     _LIST_ADDFIELD_ASCII(listFields, "PDR.test_nam", m_cnTEST_NAM, nFieldSelection, eposTEST_NAM);

     // PDR.SEQ_NAME
     _LIST_ADDFIELD_ASCII(listFields, "PDR.seq_name", m_cnSEQ_NAME, nFieldSelection, eposSEQ_NAME);
}





///////////////////////////////////////////////////////////
// FDR RECORD
///////////////////////////////////////////////////////////
Stdf_FDR_V3::Stdf_FDR_V3() : Stdf_Record()
{
    Reset();
}

Stdf_FDR_V3::~Stdf_FDR_V3()
{
    Reset();
}

void Stdf_FDR_V3::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Select fields for reduced list

    // Reset Data
    m_u4TEST_NUM	= 0;            // FDR.TEST_NUM
    m_b1DESC_FLG    = HEXADECIMAL;  // FDR.DESC_FLG
    m_cnTEST_NAM	= "";           // FDR.TEST_NAM
    m_cnSEQ_NAME    = "";           // FDR.SEQ_NAME

}

bool Stdf_FDR_V3::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;
    return false;
}

bool Stdf_FDR_V3::IsFieldPresent(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Present))
        return true;
    return false;
}

QString Stdf_FDR_V3::GetRecordShortName(void)
{
    return "FDR";
}

QString Stdf_FDR_V3::GetRecordLongName(void)
{
    return "Functional Test Description Record";
}

int Stdf_FDR_V3::GetRecordType(void)
{
    return Rec_FDR;
}

bool Stdf_FDR_V3::Read(GS::StdLib::Stdf & lStdf)
{
    BYTE	bData;
    long    dwData;
    char	szString[STDF_MAX_U1+1];

    // First reset data
    Reset();

    // FDR.TEST_NUM
    _FIELD_CHECKREAD(lStdf.ReadDword(&dwData), eposTEST_NUM);
    _FIELD_SET(m_u4TEST_NUM = stdf_type_u4(dwData), true, eposTEST_NUM);

    // FDR.DESC_FLG
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposDESC_FLG);
    _FIELD_SET(m_b1DESC_FLG = stdf_type_b1(bData), true, eposDESC_FLG);

    // FDR.TEST_NAM
    _FIELD_CHECKREAD(lStdf.ReadString(szString), eposTEST_NAM);
    _FIELD_SET(m_cnTEST_NAM = szString, !m_cnTEST_NAM.isEmpty(), eposTEST_NAM);

    // FDR.SEQ_NAME
    _FIELD_CHECKREAD(lStdf.ReadString(szString), eposSEQ_NAME);
    _FIELD_SET(m_cnSEQ_NAME = szString, !m_cnSEQ_NAME.isEmpty(), eposSEQ_NAME);

    return true;
}


void Stdf_FDR_V3::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    // Empty string first
    strAsciiString = "";

    // FDR.TEST_NUM
    _STR_ADDFIELD_ASCII(strAsciiString, "FDR.test_num", QString::number(m_u4TEST_NUM), nFieldSelection, eposTEST_NUM);

    // FDR.DESC_FLG
    _CREATEFIELD_FROM_B1_ASCII(m_b1DESC_FLG)
    _STR_ADDFIELD_ASCII(strAsciiString, "FDR.desc_flg", QString::number(m_b1DESC_FLG), nFieldSelection, eposDESC_FLG);

    // PRR.TEST_NAM
    _STR_ADDFIELD_ASCII(strAsciiString, "FDR.test_nam", m_cnTEST_NAM, nFieldSelection, eposTEST_NAM);

    // FDR.SEQ_NAME
    _STR_ADDFIELD_ASCII(strAsciiString, "FDR.seq_name", m_cnSEQ_NAME, nFieldSelection, eposSEQ_NAME);
}

void Stdf_FDR_V3::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
    // Empty string list first
    listFields.empty();

    // FDR.TEST_NUM
    _LIST_ADDFIELD_ASCII(listFields, "FDR.test_num", QString::number(m_u4TEST_NUM), nFieldSelection, eposTEST_NUM);

    // FDR.DESC_FLG
    _CREATEFIELD_FROM_B1_ASCII(m_b1DESC_FLG)
    _LIST_ADDFIELD_ASCII(listFields, "FDR.desc_flg", QString::number(m_b1DESC_FLG), nFieldSelection, eposDESC_FLG);


    // FDR.TEST_NAM
    _LIST_ADDFIELD_ASCII(listFields, "FDR.test_nam", m_cnTEST_NAM, nFieldSelection, eposTEST_NAM);

    // FDR.SEQ_NAME
    _LIST_ADDFIELD_ASCII(listFields, "FDR.seq_name", m_cnSEQ_NAME, nFieldSelection, eposSEQ_NAME);
}



///////////////////////////////////////////////////////////
// PTR RECORD
///////////////////////////////////////////////////////////
Stdf_PTR_V3::Stdf_PTR_V3() : Stdf_Record()
{
    Reset();
}

Stdf_PTR_V3::~Stdf_PTR_V3()
{
    Reset();
}

void Stdf_PTR_V3::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Select fields for reduced list

    // Reset Data
    m_u4TEST_NUM		= 0;            // PTR.TEST_NUM
    m_u1HEAD_NUM		= 0;            // PTR.HEAD_NUM
    m_u1SITE_NUM		= 0;            // PTR.SITE_NUM
    m_b1TEST_FLG		= 0;            // PTR.TEST_FLG
    m_b1PARM_FLG		= 0;            // PTR.PARM_FLG
    m_r4RESULT			= 0.0;          // PTR.RESULT
    m_b1OPT_FLAG		= eOPT_FLAG_ALL;// PTR.OPT_FLAG
    m_bRESULT_IsNAN		= false;
    m_i1RES_SCAL		= 0;            // PTR.RES_SCAL
    m_u1RES_LDIG        = 0;            // PTR.RES_LDIG
    m_u1RES_RDIG        = 0;            // PTR.RES_RDIG
    m_b1DESC_FLG        = 0;            // PTR.DESC_FLG
    m_cnUNITS           = "";           // PTR.UNITS
    m_i1LLM_SCAL		= 0;            // PTR.LLM_SCAL
    m_i1HLM_SCAL		= 0;            // PTR.HLM_SCAL
    m_u1LLM_LDIG		= 0;            // PTR.LLM_LDIG
    m_u1LLM_RDIG		= 0;            // PTR.LLM_RDIG
    m_u1HLM_LDIG		= 0;            // PTR.HLM_LDIG
    m_u1HLM_RDIG		= 0;            // PTR.HLM_RDIG
    m_r4LO_LIMIT		= 0.0;          // PTR.LO_LIMIT
    m_r4HI_LIMIT		= 0.0;          // PTR.HI_LIMIT
    m_cnTEST_NAM		= "";           // PTR.TEST_NAM
    m_cnSEQ_NAME		= "";           // PTR.SEQ_NAME
    m_cnTEST_TXT		= "";           // PTR.TEST_TXT
}

bool Stdf_PTR_V3::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;
    return false;
}

bool Stdf_PTR_V3::IsFieldPresent(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Present))
        return true;
    return false;
}

bool Stdf_PTR_V3::IsTestExecuted()
{
    return ((m_b1TEST_FLG & STDF_MASK_BIT4) == 0);
}

bool Stdf_PTR_V3::IsTestFail()
{
    return IsTestFail(*this);
}

bool Stdf_PTR_V3::IsTestFail(Stdf_PTR_V3 & clRefPTR)
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

void Stdf_PTR_V3::UpdatePassFailInfo(Stdf_PTR_V3 & clRefPTR)
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

QString Stdf_PTR_V3::GetRecordShortName(void)
{
    return "PTR";
}

QString Stdf_PTR_V3::GetRecordLongName(void)
{
    return "Parametric Test Record";
}

int Stdf_PTR_V3::GetRecordType(void)
{
    return Rec_PTR;
}

bool Stdf_PTR_V3::Read(GS::StdLib::Stdf & lStdf)
{
    char	szString[STDF_MAX_U1+1];
    long	dwData;
    float	fData;
    BYTE	bData;

    // First reset data
    Reset();

    // PTR.TEST_NUM
    _FIELD_CHECKREAD(lStdf.ReadDword(&dwData), eposTEST_NUM);
    _FIELD_SET(m_u4TEST_NUM = stdf_type_u4(dwData), true, eposTEST_NUM);

    // PTR.HEAD_NUM
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposHEAD_NUM);
    _FIELD_SET(m_u1HEAD_NUM = stdf_type_u1(bData), true, eposHEAD_NUM);

    // PTR.SITE_NUM
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposSITE_NUM);
    _FIELD_SET(m_u1SITE_NUM = stdf_type_u1(bData), true, eposSITE_NUM);

    // PTR.TEST_FLG
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposTEST_FLG);
    _FIELD_SET(m_b1TEST_FLG = stdf_type_b1(bData), true, eposTEST_FLG);

    // PTR.PARM_FLG
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposPARM_FLG);
    _FIELD_SET(m_b1PARM_FLG = stdf_type_b1(bData), true, eposPARM_FLG);

    // PTR.RESULT
    _FIELD_CHECKREAD(lStdf.ReadFloat(&fData, &m_bRESULT_IsNAN), eposRESULT);
    _FIELD_SET(m_r4RESULT = stdf_type_r4(fData), (m_b1TEST_FLG & STDF_MASK_BIT1) == 0, eposRESULT);

    // PTR.OPT_FLAG
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposOPT_FLAG);
    _FIELD_SET(m_b1OPT_FLAG = stdf_type_b1(bData), true, eposOPT_FLAG);

    // PTR.RES_SCAL
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposRES_SCAL);
    _FIELD_SET(m_i1RES_SCAL = stdf_type_i1(bData), (m_b1OPT_FLAG & STDF_MASK_BIT0) == 0, eposRES_SCAL);

    // PTR.RES_LDIG
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposRES_LDIG);
    _FIELD_SET(m_u1RES_LDIG = stdf_type_u1(bData), (m_b1OPT_FLAG & STDF_MASK_BIT1) == 0, eposRES_LDIG);

    // PTR.RES_RDIG
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposRES_RDIG);
    _FIELD_SET(m_u1RES_RDIG = stdf_type_u1(bData), (m_b1OPT_FLAG & STDF_MASK_BIT1) == 0, eposRES_RDIG);

    // PDR.DESC_FLG
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposDESC_FLG);
    _FIELD_SET(m_b1DESC_FLG = stdf_type_b1(bData), (m_b1OPT_FLAG & STDF_MASK_BIT2) == 0, eposDESC_FLG);

    // PTR.UNITS
    int lIndex=0;
    for (lIndex=0; lIndex<7; ++lIndex)
    {
        _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposUNITS);
        szString[lIndex] = bData;
    }
    szString[lIndex] = '\0';
    _FIELD_SET(m_cnUNITS = QString(szString).trimmed(), (m_b1OPT_FLAG & STDF_MASK_BIT3) == 0, eposUNITS);

    // PTR.LLM_SCAL
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposLLM_SCAL);
    _FIELD_SET(m_i1LLM_SCAL = stdf_type_i1(bData), (m_b1OPT_FLAG & (STDF_MASK_BIT4 | STDF_MASK_BIT6))==0, eposLLM_SCAL);

    // PTR.HLM_SCAL
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposHLM_SCAL);
    _FIELD_SET(m_i1HLM_SCAL = stdf_type_i1(bData), (m_b1OPT_FLAG & (STDF_MASK_BIT5 | STDF_MASK_BIT7))==0, eposHLM_SCAL);

    // PTR.LLM_LDIG
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposLLM_LDIG);
    _FIELD_SET(m_u1LLM_LDIG = stdf_type_u1(bData), (m_b1OPT_FLAG & (STDF_MASK_BIT4 | STDF_MASK_BIT6))==0, eposLLM_LDIG);

    // PTR.LLM_RDIG
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposLLM_RDIG);
    _FIELD_SET(m_u1LLM_RDIG = stdf_type_u1(bData), (m_b1OPT_FLAG & (STDF_MASK_BIT4 | STDF_MASK_BIT6))==0, eposLLM_RDIG);

    // PTR.HLM_LDIG
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposHLM_LDIG);
    _FIELD_SET(m_u1HLM_LDIG = stdf_type_u1(bData), (m_b1OPT_FLAG & (STDF_MASK_BIT5 | STDF_MASK_BIT7))==0, eposHLM_LDIG);

    // PTR.HLM_RDIG
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposHLM_RDIG);
    _FIELD_SET(m_u1HLM_RDIG = stdf_type_u1(bData), (m_b1OPT_FLAG & (STDF_MASK_BIT5 | STDF_MASK_BIT7))==0, eposHLM_RDIG);

    // PTR.LO_LIMIT
    _FIELD_CHECKREAD(lStdf.ReadFloat(&fData), eposLO_LIMIT);
    _FIELD_SET(m_r4LO_LIMIT = stdf_type_r4(fData), (m_b1OPT_FLAG & STDF_MASK_BIT6 )==0, eposLO_LIMIT);

    // PTR.HI_LIMIT
    _FIELD_CHECKREAD(lStdf.ReadFloat(&fData), eposHI_LIMIT);
    _FIELD_SET(m_r4HI_LIMIT = stdf_type_r4(fData), (m_b1OPT_FLAG & STDF_MASK_BIT7)==0, eposHI_LIMIT);

    // PTR.TEST_NAM
    _FIELD_CHECKREAD(lStdf.ReadString(szString), eposTEST_NAM);
    _FIELD_SET(m_cnTEST_NAM = szString, !m_cnTEST_NAM.isEmpty(), eposTEST_NAM);

    // PTR.SEQ_NAME
    _FIELD_CHECKREAD(lStdf.ReadString(szString), eposSEQ_NAME);
    _FIELD_SET(m_cnSEQ_NAME = szString, !m_cnSEQ_NAME.isEmpty(), eposSEQ_NAME);

    // PTR.TEST_TXT
    _FIELD_CHECKREAD(lStdf.ReadString(szString), eposTEST_TXT);
    _FIELD_SET(m_cnTEST_TXT = szString, !m_cnTEST_TXT.isEmpty(), eposTEST_TXT);


    return true;
}

void Stdf_PTR_V3::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
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
        strString = QString::number(m_r4RESULT,'f');
        //strString.sprintf("%x", m_r4RESULT); // GCORE-859
    _STR_ADDFIELD_ASCII(strAsciiString, "PTR.result", strString, nFieldSelection, eposRESULT);

    // PTR.OPT_FLAG
    _CREATEFIELD_FROM_B1_ASCII(m_b1OPT_FLAG)
    _STR_ADDFIELD_ASCII(strAsciiString, "PTR.opt_flag", m_strFieldValue_macro, nFieldSelection, eposOPT_FLAG);

    // PTR.RES_SCAL
    _STR_ADDFIELD_ASCII(strAsciiString, "PTR.res_scal", QString::number(m_i1RES_SCAL), nFieldSelection, eposRES_SCAL);

    // PTR.RES_LDIG
    _STR_ADDFIELD_ASCII(strAsciiString, "PTR.res_ldig", QString::number(m_u1RES_LDIG), nFieldSelection, eposRES_LDIG);

    // PTR.RES_RDIG
    _STR_ADDFIELD_ASCII(strAsciiString, "PTR.res_rdig", QString::number(m_u1RES_RDIG), nFieldSelection, eposRES_RDIG);

    // PTR.DESC_FLG
    _CREATEFIELD_FROM_B1_ASCII(m_b1DESC_FLG)
    _STR_ADDFIELD_ASCII(strAsciiString, "PTR.desc_flg", m_strFieldValue_macro, nFieldSelection, eposDESC_FLG);

    // PTR.UNITS
    _STR_ADDFIELD_ASCII(strAsciiString, "PTR.units", m_cnUNITS, nFieldSelection, eposUNITS);

    // PTR.LLM_SCAL
    _STR_ADDFIELD_ASCII(strAsciiString, "PTR.llm_scal", QString::number(m_i1LLM_SCAL), nFieldSelection, eposLLM_SCAL);

    // PTR.HLM_SCAL
    _STR_ADDFIELD_ASCII(strAsciiString, "PTR.hlm_scal", QString::number(m_i1HLM_SCAL), nFieldSelection, eposHLM_SCAL);

    // PTR.LLM_LDIG
    _STR_ADDFIELD_ASCII(strAsciiString, "PTR.llm_ldig", QString::number(m_u1LLM_LDIG), nFieldSelection, eposLLM_LDIG);

    // PTR.LLM_RDIG
    _STR_ADDFIELD_ASCII(strAsciiString, "PTR.llm_rdig", QString::number(m_u1LLM_RDIG), nFieldSelection, eposLLM_RDIG);

    // PTR.HLM_LDIG
    _STR_ADDFIELD_ASCII(strAsciiString, "PTR.hlm_ldig", QString::number(m_u1HLM_LDIG), nFieldSelection, eposHLM_LDIG);

    // PTR.HLM_RDIG
    _STR_ADDFIELD_ASCII(strAsciiString, "PTR.hlm_rdig", QString::number(m_u1HLM_RDIG), nFieldSelection, eposHLM_RDIG);

    // PTR.LO_LIMIT
    _STR_ADDFIELD_ASCII(strAsciiString, "PTR.lo_limit", QString::number(m_r4LO_LIMIT), nFieldSelection, eposLO_LIMIT);

    // PTR.HI_LIMIT
    _STR_ADDFIELD_ASCII(strAsciiString, "PTR.hi_limit", QString::number(m_r4HI_LIMIT), nFieldSelection, eposHI_LIMIT);

    // PTR.TEST_NAM
    _STR_ADDFIELD_ASCII(strAsciiString, "PTR.test_nam", m_cnTEST_NAM, nFieldSelection, eposTEST_NAM);

    // PTR.SEQ_NAME
    _STR_ADDFIELD_ASCII(strAsciiString, "PTR.seq_name", m_cnSEQ_NAME, nFieldSelection, eposSEQ_NAME);

    // PTR.TEST_TXT
    _STR_ADDFIELD_ASCII(strAsciiString, "PTR.test_txt", m_cnTEST_TXT, nFieldSelection, eposTEST_TXT);
}

void Stdf_PTR_V3::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
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
        strString = QString::number(m_r4RESULT,'f');
    _LIST_ADDFIELD_ASCII(listFields, "PTR.result", strString, nFieldSelection, eposRESULT);

    // PTR.OPT_FLAG
    _CREATEFIELD_FROM_B1_ASCII(m_b1OPT_FLAG)
    _LIST_ADDFIELD_ASCII(listFields, "PTR.opt_flag", m_strFieldValue_macro, nFieldSelection, eposOPT_FLAG);

    // PTR.RES_SCAL
    _LIST_ADDFIELD_ASCII(listFields, "PTR.res_scal", QString::number(m_i1RES_SCAL), nFieldSelection, eposRES_SCAL);

    // PTR.RES_LDIG
    _LIST_ADDFIELD_ASCII(listFields, "PTR.res_ldig", QString::number(m_u1RES_LDIG), nFieldSelection, eposRES_LDIG);

    // PTR.RES_RDIG
    _LIST_ADDFIELD_ASCII(listFields, "PTR.res_rdig", QString::number(m_u1RES_RDIG), nFieldSelection, eposRES_RDIG);

    // PTR.DESC_FLG
    _CREATEFIELD_FROM_B1_ASCII(m_b1DESC_FLG)
    _LIST_ADDFIELD_ASCII(listFields, "PTR.desc_flg", m_strFieldValue_macro, nFieldSelection, eposDESC_FLG);

    // PTR.UNITS
    _LIST_ADDFIELD_ASCII(listFields, "PTR.units", m_cnUNITS, nFieldSelection, eposUNITS);

    // PTR.LLM_SCAL
    _LIST_ADDFIELD_ASCII(listFields, "PTR.llm_scal", QString::number(m_i1LLM_SCAL), nFieldSelection, eposLLM_SCAL);

    // PTR.HLM_SCAL
    _LIST_ADDFIELD_ASCII(listFields, "PTR.hlm_scal", QString::number(m_i1HLM_SCAL), nFieldSelection, eposHLM_SCAL);

    // PTR.LLM_LDIG
    _LIST_ADDFIELD_ASCII(listFields, "PTR.llm_ldig", QString::number(m_u1LLM_LDIG), nFieldSelection, eposLLM_LDIG);

    // PTR.LLM_RDIG
    _LIST_ADDFIELD_ASCII(listFields, "PTR.llm_rdig", QString::number(m_u1LLM_RDIG), nFieldSelection, eposLLM_RDIG);

    // PTR.HLM_LDIG
    _LIST_ADDFIELD_ASCII(listFields, "PTR.hlm_ldig", QString::number(m_u1HLM_LDIG), nFieldSelection, eposHLM_LDIG);

    // PTR.HLM_RDIG
    _LIST_ADDFIELD_ASCII(listFields, "PTR.llm_rdig", QString::number(m_u1HLM_RDIG), nFieldSelection, eposHLM_RDIG);

    // PTR.LO_LIMIT
    _LIST_ADDFIELD_ASCII(listFields, "PTR.lo_limit", QString::number(m_r4LO_LIMIT), nFieldSelection, eposLO_LIMIT);

    // PTR.HI_LIMIT
    _LIST_ADDFIELD_ASCII(listFields, "PTR.hi_limit", QString::number(m_r4HI_LIMIT), nFieldSelection, eposHI_LIMIT);

    // PTR.TEST_NAM
    _LIST_ADDFIELD_ASCII(listFields, "PTR.test_nam", m_cnTEST_NAM, nFieldSelection, eposTEST_NAM);

    // PTR.SEQ_NAME
    _LIST_ADDFIELD_ASCII(listFields, "PTR.seq_name", m_cnSEQ_NAME, nFieldSelection, eposSEQ_NAME);

    // PTR.TEST_TXT
    _STR_ADDFIELD_ASCII(listFields, "PTR.test_txt", m_cnTEST_TXT, nFieldSelection, eposTEST_TXT);
}





///////////////////////////////////////////////////////////
// FTR RECORD
///////////////////////////////////////////////////////////
Stdf_FTR_V3::Stdf_FTR_V3() : Stdf_Record(), m_pFilter(NULL)
{
    Reset();
}

Stdf_FTR_V3::~Stdf_FTR_V3()
{
    Reset();
}

void Stdf_FTR_V3::SetFilter(Stdf_FTR_V3 * pFilter)
{
    m_pFilter = pFilter;
}

void Stdf_FTR_V3::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Select fields for reduced list

    // Reset Data
    m_u4TEST_NUM	= 0;		// FTR.TEST_NUM
    m_u1HEAD_NUM	= 0;		// FTR.HEAD_NUM
    m_u1SITE_NUM	= 0;		// FTR.SITE_NUM
    m_b1TEST_FLG	= 0;		// FTR.TEST_FLG
    m_b1DESC_FLG	= 0x02;		// PTR.DESC_FLG
    m_b1OPT_FLAG	= 0;		// FTR.OPT_FLAG
    m_u1TIME_SET	= 0;		// FTR.TIME_SET
    m_u4CYCL_CNT	= 0;		// FTR.CYCL_CNT
    m_u2REPT_CNT	= 0;		// FTR.REPT_CNT
    m_u2PCP_ADDR	= 0;		// FTR.PCP_ADDR
    m_u4NUM_FAIL	= 0;		// FTR.NUM_FAIL
    m_u4VECT_ADR	= 0;		// FTR.VECT_ADR
    m_bnFAIL_PIN.m_bLength = 0;	// FTR.FAIL_PIN
    m_bnVECT_DAT.m_bLength = 0;	// FTR.VECT_DAT
    m_bnDEV_DAT.m_bLength = 0;	// FTR.DEV_DAT
    m_bnRPIN_MAP.m_bLength = 0;	// FTR.RPIN_MAP
    m_cnTEST_NAM	= "";		// FTR.TEST_NAM
    m_cnSEQ_NAME	= "";		// FTR.SEQ_NAME
    m_cnTEST_TXT	= "";		// FTR.TEST_TXT
}

bool Stdf_FTR_V3::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;
    return false;
}

bool Stdf_FTR_V3::IsFieldPresent(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Present))
        return true;
    return false;
}


bool Stdf_FTR_V3::IsTestExecuted()
{
    return ((m_b1TEST_FLG & STDF_MASK_BIT4) == 0);
}

bool Stdf_FTR_V3::IsTestFail()
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

QString Stdf_FTR_V3::GetRecordShortName(void)
{
    return "FTR";
}

QString Stdf_FTR_V3::GetRecordLongName(void)
{
    return "Functional Test Result Record";
}

int Stdf_FTR_V3::GetRecordType(void)
{
    return Rec_FTR;
}

bool Stdf_FTR_V3::Read(GS::StdLib::Stdf & lStdf)
{
    char			szString[STDF_MAX_U1+1];
    long			dwData;
    int             lIntData;
    BYTE			bData;

    // First reset data
    Reset();

    // FTR.TEST_NUM
    _FIELD_CHECKREAD(lStdf.ReadDword(&dwData), eposTEST_NUM);
    _FIELD_SET(m_u4TEST_NUM = stdf_type_u4(dwData), true, eposTEST_NUM);

    // FTR.HEAD_NUM
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposHEAD_NUM);
    _FIELD_SET(m_u1HEAD_NUM = stdf_type_u1(bData), true, eposHEAD_NUM);

    // FTR.SITE_NUM
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposSITE_NUM);
    _FIELD_CHECKFILTER(stdf_type_u1(bData), m_u1SITE_NUM, eposSITE_NUM);
    _FIELD_SET(m_u1SITE_NUM = stdf_type_u1(bData), true, eposSITE_NUM);

    // FTR.TEST_FLG
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposTEST_FLG);
    _FIELD_SET(m_b1TEST_FLG = stdf_type_b1(bData), true, eposTEST_FLG);

    // FTR.DESC_FLG
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposDESC_FLG);
    _FIELD_SET(m_b1DESC_FLG = stdf_type_b1(bData), (m_b1OPT_FLAG & STDF_MASK_BIT2) == 0, eposDESC_FLG);

    // FTR.OPT_FLAG
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposOPT_FLAG);
    _FIELD_SET(m_b1OPT_FLAG = stdf_type_b1(bData), true, eposOPT_FLAG);

    // FTR.TIME_SET
    _FIELD_CHECKREAD(lStdf.ReadByte(&bData), eposTIME_SET);
    _FIELD_SET(m_u1TIME_SET = stdf_type_u1(bData), (m_b1OPT_FLAG & STDF_MASK_BIT3) == 0, eposTIME_SET);

    // FTR.VECT_ADR
    _FIELD_CHECKREAD(lStdf.ReadDword(&dwData), eposVECT_ADR);
    _FIELD_SET(m_u4VECT_ADR = stdf_type_u4(dwData), (m_b1OPT_FLAG & STDF_MASK_BIT0) == 0, eposVECT_ADR);

    // FTR.CYCL_CNT
    _FIELD_CHECKREAD(lStdf.ReadDword(&dwData), eposCYCL_CNT);
    _FIELD_SET(m_u4CYCL_CNT = stdf_type_u4(dwData), (m_b1OPT_FLAG & STDF_MASK_BIT1) == 0, eposCYCL_CNT);

    // FTR.DEPT_CNT
    _FIELD_CHECKREAD(lStdf.ReadWord(&lIntData), eposREPT_CNT);
    _FIELD_SET(m_u2REPT_CNT = stdf_type_u2(lIntData), (m_b1OPT_FLAG & STDF_MASK_BIT5) == 0, eposREPT_CNT);

    // FTR.PCP_ADDR
    _FIELD_CHECKREAD(lStdf.ReadWord(&lIntData), eposPCP_ADDR);
    _FIELD_SET(m_u2PCP_ADDR = stdf_type_u2(lIntData), (m_b1OPT_FLAG & STDF_MASK_BIT2) == 0, eposPCP_ADDR);

    // FTR.NUM_FAIL
    _FIELD_CHECKREAD(lStdf.ReadDword(&dwData), eposNUM_FAIL);
    _FIELD_SET(m_u4NUM_FAIL = stdf_type_u4(dwData), (m_b1OPT_FLAG & STDF_MASK_BIT4) == 0, eposNUM_FAIL);

    // FTR.FAIL_PIN
    _FIELD_CHECKREAD(lStdf.ReadBitField(&(m_bnFAIL_PIN.m_bLength), m_bnFAIL_PIN.m_pBitField), eposFAIL_PIN);
    // The field has already been set by the ReadBitField function. We use a dummy instruction as parameter 1 to avoid warning under gcc.
    _FIELD_SET(m_uiIndex_macro = 0, m_bnFAIL_PIN.m_bLength != 0, eposFAIL_PIN);

    // FTR.VECT_DAT
    _FIELD_CHECKREAD(lStdf.ReadBitField(&(m_bnVECT_DAT.m_bLength), m_bnVECT_DAT.m_pBitField), eposVECT_DAT);
    // The field has already been set by the ReadBitField function. We use a dummy instruction as parameter 1 to avoid warning under gcc.
    _FIELD_SET(m_uiIndex_macro = 0, m_bnVECT_DAT.m_bLength != 0, eposVECT_DAT);

    // FTR.DEV_DAT
    _FIELD_CHECKREAD(lStdf.ReadBitField(&(m_bnDEV_DAT.m_bLength), m_bnDEV_DAT.m_pBitField), eposDEV_DAT);
    // The field has already been set by the ReadBitField function. We use a dummy instruction as parameter 1 to avoid warning under gcc.
    _FIELD_SET(m_uiIndex_macro = 0, m_bnDEV_DAT.m_bLength != 0, eposDEV_DAT);

    // FTR.RPIN_MAP
    _FIELD_CHECKREAD(lStdf.ReadBitField(&(m_bnRPIN_MAP.m_bLength), m_bnRPIN_MAP.m_pBitField), eposRPIN_MAP);
    // The field has already been set by the ReadBitField function. We use a dummy instruction as parameter 1 to avoid warning under gcc.
    _FIELD_SET(m_uiIndex_macro = 0, m_bnRPIN_MAP.m_bLength != 0, eposRPIN_MAP);

    // PTR.TEST_NAM
    _FIELD_CHECKREAD(lStdf.ReadString(szString), eposTEST_NAM);
    _FIELD_SET(m_cnTEST_NAM = szString, !m_cnTEST_NAM.isEmpty(), eposTEST_NAM);

    // PTR.SEQ_NAME
    _FIELD_CHECKREAD(lStdf.ReadString(szString), eposSEQ_NAME);
    _FIELD_SET(m_cnSEQ_NAME = szString, !m_cnSEQ_NAME.isEmpty(), eposSEQ_NAME);

    // PTR.TEST_TXT
    _FIELD_CHECKREAD(lStdf.ReadString(szString), eposTEST_TXT);
    _FIELD_SET(m_cnTEST_TXT = szString, !m_cnTEST_TXT.isEmpty(), eposTEST_TXT);

    return true;
}

void Stdf_FTR_V3::GetAsciiString(QString & strAsciiString, int nFieldSelection )
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

    // FTR.DESC_FLG
    _CREATEFIELD_FROM_B1_ASCII(m_b1DESC_FLG)
    _STR_ADDFIELD_ASCII(strAsciiString, "FTR.desc_flg", m_strFieldValue_macro, nFieldSelection, eposDESC_FLG);

    // FTR.OPT_FLAG
    _CREATEFIELD_FROM_B1_ASCII(m_b1OPT_FLAG)
    _STR_ADDFIELD_ASCII(strAsciiString, "FTR.opt_flag", m_strFieldValue_macro, nFieldSelection, eposOPT_FLAG);

    // FTR.TIME_SET
    _STR_ADDFIELD_ASCII(strAsciiString, "FTR.time_set", QString::number(m_u1TIME_SET), nFieldSelection, eposTIME_SET);

    // FTR.VECT_ADR
    _STR_ADDFIELD_ASCII(strAsciiString, "FTR.vect_adr", QString::number(m_u4VECT_ADR), nFieldSelection, eposVECT_ADR);

    // FTR.CYCL_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "FTR.cycl_cnt", QString::number(m_u4CYCL_CNT), nFieldSelection, eposCYCL_CNT);

    // FTR.REPT_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "FTR.rept_cnt", QString::number(m_u2REPT_CNT), nFieldSelection, eposREPT_CNT);

    // FTR.PCP_ADDR
    _STR_ADDFIELD_ASCII(strAsciiString, "FTR.pcp_addr", QString::number(m_u2PCP_ADDR), nFieldSelection, eposPCP_ADDR);

    // FTR.NUM_FAIL
    _STR_ADDFIELD_ASCII(strAsciiString, "FTR.num_fail", QString::number(m_u4NUM_FAIL), nFieldSelection, eposNUM_FAIL);

    // FTR.FAIL_PIN
    _CREATEFIELD_FROM_BN_ASCII(m_bnFAIL_PIN);
    QString	lFailPinStr;
    if(m_bnFAIL_PIN.m_bLength > 0)
    {
        m_ptChar_macro = m_bnFAIL_PIN.m_pBitField;
        for(m_uiIndex_macro=0; m_uiIndex_macro<m_bnFAIL_PIN.m_bLength; m_uiIndex_macro++)
            lFailPinStr += m_ptChar_macro[m_uiIndex_macro];
    }
    m_strFieldValue_macro += " (\"" + lFailPinStr;
    m_strFieldValue_macro += "\")";
    _STR_ADDFIELD_ASCII(strAsciiString, "FTR.fail_pin", m_strFieldValue_macro, nFieldSelection, eposFAIL_PIN);

    // FTR.VECT_DAT
    _CREATEFIELD_FROM_BN_ASCII(m_bnVECT_DAT);
    QString	lVECT_DATStr;
    if(m_bnVECT_DAT.m_bLength > 0)
    {
        m_ptChar_macro = m_bnVECT_DAT.m_pBitField;
        for(m_uiIndex_macro=0; m_uiIndex_macro<m_bnVECT_DAT.m_bLength; m_uiIndex_macro++)
            lFailPinStr += m_ptChar_macro[m_uiIndex_macro];
    }
    m_strFieldValue_macro += " (\"" + lVECT_DATStr;
    m_strFieldValue_macro += "\")";
    _STR_ADDFIELD_ASCII(strAsciiString, "FTR.vect_dat", m_strFieldValue_macro, nFieldSelection, eposVECT_DAT);

    // FTR.DEV_DAT
    _CREATEFIELD_FROM_BN_ASCII(m_bnDEV_DAT);
    QString	lDEV_DATStr;
    if(m_bnDEV_DAT.m_bLength > 0)
    {
        m_ptChar_macro = m_bnDEV_DAT.m_pBitField;
        for(m_uiIndex_macro=0; m_uiIndex_macro<m_bnDEV_DAT.m_bLength; m_uiIndex_macro++)
            lFailPinStr += m_ptChar_macro[m_uiIndex_macro];
    }
    m_strFieldValue_macro += " (\"" + lDEV_DATStr;
    m_strFieldValue_macro += "\")";
    _STR_ADDFIELD_ASCII(strAsciiString, "FTR.dev_dat", m_strFieldValue_macro, nFieldSelection, eposDEV_DAT);

    // FTR.RPIN_MAP
    _CREATEFIELD_FROM_BN_ASCII(m_bnRPIN_MAP);
    QString	lRPIN_MAPStr;
    if(m_bnRPIN_MAP.m_bLength > 0)
    {
        m_ptChar_macro = m_bnRPIN_MAP.m_pBitField;
        for(m_uiIndex_macro=0; m_uiIndex_macro<m_bnRPIN_MAP.m_bLength; m_uiIndex_macro++)
            lFailPinStr += m_ptChar_macro[m_uiIndex_macro];
    }
    m_strFieldValue_macro += " (\"" + lRPIN_MAPStr;
    m_strFieldValue_macro += "\")";
    _STR_ADDFIELD_ASCII(strAsciiString, "FTR.rpin_map", m_strFieldValue_macro, nFieldSelection, eposRPIN_MAP);

    // FTR.TEST_NAM
    _STR_ADDFIELD_ASCII(strAsciiString, "FTR.test_nam", m_cnTEST_NAM, nFieldSelection, eposTEST_NAM);

    // FTR.SEQ_NAME
    _STR_ADDFIELD_ASCII(strAsciiString, "FTR.seq_name", m_cnSEQ_NAME, nFieldSelection, eposSEQ_NAME);

    // FTR.TEST_TXT
    _STR_ADDFIELD_ASCII(strAsciiString, "FTR.test_txt", m_cnTEST_TXT, nFieldSelection, eposTEST_TXT);

}

void Stdf_FTR_V3::GetAsciiFieldList(QStringList & listFields, int nFieldSelection )
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

    // FTR.DESC_FLG
    _CREATEFIELD_FROM_B1_ASCII(m_b1DESC_FLG)
    _LIST_ADDFIELD_ASCII(listFields, "FTR.desc_flg", m_strFieldValue_macro, nFieldSelection, eposDESC_FLG);

    // FTR.OPT_FLAG
    _CREATEFIELD_FROM_B1_ASCII(m_b1OPT_FLAG)
    _LIST_ADDFIELD_ASCII(listFields, "FTR.opt_flag", m_strFieldValue_macro, nFieldSelection, eposOPT_FLAG);

    // FTR.TIME_SET
    _LIST_ADDFIELD_ASCII(listFields, "FTR.time_set", QString::number(m_u1TIME_SET), nFieldSelection, eposTIME_SET);

    // FTR.VECT_ADR
    _LIST_ADDFIELD_ASCII(listFields, "FTR.vect_adr", QString::number(m_u4VECT_ADR), nFieldSelection, eposVECT_ADR);

    // FTR.CYCL_CNT
    _LIST_ADDFIELD_ASCII(listFields, "FTR.cycl_cnt", QString::number(m_u4CYCL_CNT), nFieldSelection, eposCYCL_CNT);

    // FTR.REPT_CNT
    _LIST_ADDFIELD_ASCII(listFields, "FTR.rept_cnt", QString::number(m_u2REPT_CNT), nFieldSelection, eposREPT_CNT);

    // FTR.PCP_ADDR
    _LIST_ADDFIELD_ASCII(listFields, "FTR.pcp_addr", QString::number(m_u2PCP_ADDR), nFieldSelection, eposPCP_ADDR);

    // FTR.NUM_FAIL
    _LIST_ADDFIELD_ASCII(listFields, "FTR.num_fail", QString::number(m_u4NUM_FAIL), nFieldSelection, eposNUM_FAIL);

    // FTR.FAIL_PIN
    _CREATEFIELD_FROM_BN_ASCII(m_bnFAIL_PIN);
    QString	lFailPinStr;
    if(m_bnFAIL_PIN.m_bLength > 0)
    {
        m_ptChar_macro = m_bnFAIL_PIN.m_pBitField;
        for(m_uiIndex_macro=0; m_uiIndex_macro<m_bnFAIL_PIN.m_bLength; m_uiIndex_macro++)
            lFailPinStr += m_ptChar_macro[m_uiIndex_macro];
    }
    m_strFieldValue_macro += " (\"" + lFailPinStr;
    m_strFieldValue_macro += "\")";
    _LIST_ADDFIELD_ASCII(listFields, "FTR.fail_pin", m_strFieldValue_macro, nFieldSelection, eposFAIL_PIN);

    // FTR.VECT_DAT
    _CREATEFIELD_FROM_BN_ASCII(m_bnVECT_DAT);
    QString	lVECT_DATStr;
    if(m_bnVECT_DAT.m_bLength > 0)
    {
        m_ptChar_macro = m_bnVECT_DAT.m_pBitField;
        for(m_uiIndex_macro=0; m_uiIndex_macro<m_bnVECT_DAT.m_bLength; m_uiIndex_macro++)
            lFailPinStr += m_ptChar_macro[m_uiIndex_macro];
    }
    m_strFieldValue_macro += " (\"" + lVECT_DATStr;
    m_strFieldValue_macro += "\")";
    _LIST_ADDFIELD_ASCII(listFields, "FTR.vect_dat", m_strFieldValue_macro, nFieldSelection, eposVECT_DAT);

    // FTR.DEV_DAT
    _CREATEFIELD_FROM_BN_ASCII(m_bnDEV_DAT);
    QString	lDEV_DATStr;
    if(m_bnDEV_DAT.m_bLength > 0)
    {
        m_ptChar_macro = m_bnDEV_DAT.m_pBitField;
        for(m_uiIndex_macro=0; m_uiIndex_macro<m_bnDEV_DAT.m_bLength; m_uiIndex_macro++)
            lFailPinStr += m_ptChar_macro[m_uiIndex_macro];
    }
    m_strFieldValue_macro += " (\"" + lDEV_DATStr;
    m_strFieldValue_macro += "\")";
    _LIST_ADDFIELD_ASCII(listFields, "FTR.dev_dat", m_strFieldValue_macro, nFieldSelection, eposDEV_DAT);

    // FTR.RPIN_MAP
    _CREATEFIELD_FROM_BN_ASCII(m_bnRPIN_MAP);
    QString	lRPIN_MAPStr;
    if(m_bnRPIN_MAP.m_bLength > 0)
    {
        m_ptChar_macro = m_bnRPIN_MAP.m_pBitField;
        for(m_uiIndex_macro=0; m_uiIndex_macro<m_bnRPIN_MAP.m_bLength; m_uiIndex_macro++)
            lFailPinStr += m_ptChar_macro[m_uiIndex_macro];
    }
    m_strFieldValue_macro += " (\"" + lRPIN_MAPStr;
    m_strFieldValue_macro += "\")";
    _LIST_ADDFIELD_ASCII(listFields, "FTR.rpin_map", m_strFieldValue_macro, nFieldSelection, eposRPIN_MAP);

    // FTR.TEST_NAM
    _LIST_ADDFIELD_ASCII(listFields, "FTR.test_nam", m_cnTEST_NAM, nFieldSelection, eposTEST_NAM);

    // FTR.SEQ_NAME
    _LIST_ADDFIELD_ASCII(listFields, "FTR.seq_name", m_cnSEQ_NAME, nFieldSelection, eposSEQ_NAME);

    // FTR.TEST_TXT
    _LIST_ADDFIELD_ASCII(listFields, "FTR.test_txt", m_cnTEST_TXT, nFieldSelection, eposTEST_TXT);}


///////////////////////////////////////////////////////////
// BPS RECORD
///////////////////////////////////////////////////////////
Stdf_BPS_V3::Stdf_BPS_V3() : Stdf_Record()
{
    Reset();
}

Stdf_BPS_V3::~Stdf_BPS_V3()
{
    Reset();
}

void Stdf_BPS_V3::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Select fields for reduced list
    m_pFieldFlags[eposSEQ_NAME]	|= FieldFlag_ReducedList;

    // Reset Data
    m_cnSEQ_NAME		= "";		// BPS.SEQ_NAME
}

bool Stdf_BPS_V3::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;

    return false;
}

bool Stdf_BPS_V3::IsFieldPresent(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Present))
        return true;
    return false;
}

QString Stdf_BPS_V3::GetRecordShortName(void)
{
    return "BPS";
}

QString Stdf_BPS_V3::GetRecordLongName(void)
{
    return "Begin Program Section Record";
}

int Stdf_BPS_V3::GetRecordType(void)
{
    return Rec_BPS;
}

bool Stdf_BPS_V3::Read(GS::StdLib::Stdf & clStdf)
{
    char	szString[STDF_MAX_U1+1];

    // First reset data
    Reset();

    // BPS.SEQ_NAME
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposSEQ_NAME);
    _FIELD_SET(m_cnSEQ_NAME = szString, !m_cnSEQ_NAME.isEmpty(), eposSEQ_NAME);

    return true;
}


void Stdf_BPS_V3::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    // Empty string first
    strAsciiString = "";

    // BPS.SEQ_NAME
    _STR_ADDFIELD_ASCII(strAsciiString, "BPS.seq_name", m_cnSEQ_NAME, nFieldSelection, eposSEQ_NAME);
}

void Stdf_BPS_V3::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
    // Empty string list first
    listFields.empty();

    // BPS.SEQ_NAME
    _LIST_ADDFIELD_ASCII(listFields, "BPS.seq_name", m_cnSEQ_NAME, nFieldSelection, eposSEQ_NAME);
}




///////////////////////////////////////////////////////////
// EPS RECORD
///////////////////////////////////////////////////////////
Stdf_EPS_V3::Stdf_EPS_V3() : Stdf_Record()
{
    Reset();
}

Stdf_EPS_V3::~Stdf_EPS_V3()
{
    Reset();
}

void Stdf_EPS_V3::Reset(void)
{
    // Reset field flags

    // Select fields for reduced list

    // Reset Data
}

QString Stdf_EPS_V3::GetRecordShortName(void)
{
    return "EPS";
}

QString Stdf_EPS_V3::GetRecordLongName(void)
{
    return "End Program Section Record";
}

int Stdf_EPS_V3::GetRecordType(void)
{
    return Rec_EPS;
}

bool Stdf_EPS_V3::Read(GS::StdLib::Stdf& /*clStdf*/)
{
    // First reset data
    Reset();

    // EPS (STDF V3)

    return true;
}

void Stdf_EPS_V3::GetAsciiString(QString& strAsciiString,
                                  int /*nFieldSelection = 0*/)
{
    // Empty string first
    strAsciiString = "";
}

void Stdf_EPS_V3::GetAsciiFieldList(QStringList& listFields,
                                     int /*nFieldSelection = 0*/)
{
    // Empty string list first
    listFields.empty();
}


///////////////////////////////////////////////////////////
// GDR RECORD
///////////////////////////////////////////////////////////
Stdf_GDR_V3::Stdf_GDR_V3() : Stdf_Record()
{
    m_vnGEN_DATA = NULL;
    Reset();
}

Stdf_GDR_V3::~Stdf_GDR_V3()
{
    Reset();
}

void Stdf_GDR_V3::Reset(void)
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
}

bool Stdf_GDR_V3::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;
    return false;
}

bool Stdf_GDR_V3::IsFieldPresent(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Present))
        return true;
    return false;
}

QString Stdf_GDR_V3::GetRecordShortName(void)
{
    return "GDR";
}

QString Stdf_GDR_V3::GetRecordLongName(void)
{
    return "Generic Data Record";
}

int Stdf_GDR_V3::GetRecordType(void)
{
    return Rec_GDR;
}

bool Stdf_GDR_V3::Read(GS::StdLib::Stdf & clStdf)
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
            _FIELD_CHECKREAD(clStdf.ReadBitField(&(m_vnGEN_DATA[i].m_bnData.m_bLength),
                                                 m_vnGEN_DATA[i].m_bnData.m_pBitField), eposGEN_DATA);
            break;
        default:
            break;
        }
    }

    return true;
}


void Stdf_GDR_V3::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
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
            default:
                break;
        }
    }
}

void Stdf_GDR_V3::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
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
                _LIST_ADDFIELD_ASCII(listFields, "GDR.gen_data", QString::number(m_vnGEN_DATA[i].m_u1Data),
                                     nFieldSelection, eposGEN_DATA);
                break;
            case eTypeU2:
                _LIST_ADDFIELD_ASCII(listFields, "GDR.gen_data", QString::number(m_vnGEN_DATA[i].m_u2Data),
                                     nFieldSelection, eposGEN_DATA);
                break;
            case eTypeU4:
                _LIST_ADDFIELD_ASCII(listFields, "GDR.gen_data", QString::number(m_vnGEN_DATA[i].m_u4Data),
                                     nFieldSelection, eposGEN_DATA);
                break;
            case eTypeI1:
                _LIST_ADDFIELD_ASCII(listFields, "GDR.gen_data", QString::number(m_vnGEN_DATA[i].m_i1Data),
                                     nFieldSelection, eposGEN_DATA);
                break;
            case eTypeI2:
                _LIST_ADDFIELD_ASCII(listFields, "GDR.gen_data", QString::number(m_vnGEN_DATA[i].m_i2Data),
                                     nFieldSelection, eposGEN_DATA);
                break;
            case eTypeI4:
                _LIST_ADDFIELD_ASCII(listFields, "GDR.gen_data", QString::number(m_vnGEN_DATA[i].m_i4Data),
                                     nFieldSelection, eposGEN_DATA);
                break;
            case eTypeR4:
                _LIST_ADDFIELD_ASCII(listFields, "GDR.gen_data", QString::number(m_vnGEN_DATA[i].m_r4Data),
                                     nFieldSelection, eposGEN_DATA);
                break;
            case eTypeR8:
                if(m_stRecordInfo.iCpuType == 0)	// CPU Type = VAX-PDP
                    _LIST_ADDFIELD_ASCII(listFields, "GDR.gen_data", "R8 type not supported for STDF files generated on VAX/PDP CPU's", nFieldSelection, eposGEN_DATA)
                else
                    _LIST_ADDFIELD_ASCII(listFields, "GDR.gen_data", QString::number(m_vnGEN_DATA[i].m_r8Data),
                                         nFieldSelection, eposGEN_DATA)
                break;
            case eTypeCN:
                _LIST_ADDFIELD_ASCII(listFields, "GDR.gen_data", m_vnGEN_DATA[i].m_cnData,nFieldSelection,eposGEN_DATA);
                break;
            case eTypeBN:
                _CREATEFIELD_FROM_BN_ASCII(m_vnGEN_DATA[i].m_bnData);
                _LIST_ADDFIELD_ASCII(listFields, "GDR.gen_data", m_strFieldValue_macro, nFieldSelection, eposGEN_DATA);
                break;
            default:
                break;
        }
    }
}










///////////////////////////////////////////////////////////
// DTR RECORD
///////////////////////////////////////////////////////////
Stdf_DTR_V3::Stdf_DTR_V3() : Stdf_Record()
{
    Reset();
}

Stdf_DTR_V3::~Stdf_DTR_V3()
{
    Reset();
}

void Stdf_DTR_V3::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Select fields for reduced list
    m_pFieldFlags[eposTEXT_DAT]	|= FieldFlag_ReducedList;

    // Reset Data
    m_cnTEXT_DAT		= "";		// DTR.TEXT_DAT
}

bool Stdf_DTR_V3::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;
    return false;
}

bool Stdf_DTR_V3::IsFieldPresent(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Present))
        return true;
    return false;
}

// Check if GEX command: Gross Die (if so, get value)
bool Stdf_DTR_V3::IsGexCommand_GrossDie(unsigned int *puiGrossDie, bool *pbOk)
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
bool Stdf_DTR_V3::IsGexCommand_DieTracking(QString & strDieID, QString & strCommand, bool *pbOk)
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
bool Stdf_DTR_V3::IsGexCommand_logPAT(QString & strHtmlString)
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
bool Stdf_DTR_V3::IsGexCommand_PATTestList(QString & strHtmlString)
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

QString Stdf_DTR_V3::GetRecordShortName(void)
{
    return "DTR";
}

QString Stdf_DTR_V3::GetRecordLongName(void)
{
    return "Datalog Text Record";
}

int Stdf_DTR_V3::GetRecordType(void)
{
    return Rec_DTR;
}

bool Stdf_DTR_V3::Read(GS::StdLib::Stdf & clStdf)
{
    char	szString[STDF_MAX_U1+1];

    // First reset data
    Reset();

    // DTR.TEXT_DAT
    _FIELD_CHECKREAD(clStdf.ReadString(szString), eposTEXT_DAT);
    _FIELD_SET(m_cnTEXT_DAT = szString, true, eposTEXT_DAT);

    return true;
}

void Stdf_DTR_V3::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    // Empty string first
    strAsciiString = "";

    // DTR.TEXT_DAT
    _STR_ADDFIELD_ASCII(strAsciiString, "DTR.text_dat", m_cnTEXT_DAT, nFieldSelection, eposTEXT_DAT);
}

void Stdf_DTR_V3::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
    // Empty string list first
    listFields.empty();

    // DTR.TEXT_DAT
    _LIST_ADDFIELD_ASCII(listFields, "DTR.text_dat", m_cnTEXT_DAT, nFieldSelection, eposTEXT_DAT);
}

///////////////////////////////////////////////////////////
// PMR RECORD
///////////////////////////////////////////////////////////
Stdf_PMR_V3::Stdf_PMR_V3() : Stdf_Record(),m_u2CHAN_NUM(NULL),m_cnPIN_NAME(NULL)
{
    Reset();
}

Stdf_PMR_V3::~Stdf_PMR_V3()
{
    Reset();
}

void Stdf_PMR_V3::Reset(void)
{
    // Reset field flags
    for(int i=0; i<eposEND; i++)
        m_pFieldFlags[i] = FieldFlag_Empty;

    // Reset Data
    m_u2CHAN_CNT	= 0;		// PMR.CHAN_CNT
    m_u2NAME_CNT	= 0;		// PMR.NAME_CNT
    if (m_u2CHAN_NUM != NULL)   // PMR.CHAN_NUM
    {
        delete[] m_u2CHAN_NUM;
        m_u2CHAN_NUM	= NULL;
    }
    if (m_cnPIN_NAME != NULL)   // PMR.PIN_NAME
    {
        delete[] m_cnPIN_NAME;
        m_cnPIN_NAME	= NULL;
    }

}

bool Stdf_PMR_V3::IsFieldValid(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Valid))
        return true;
    return false;
}

bool Stdf_PMR_V3::IsFieldPresent(int nFieldPos)
{
    if((nFieldPos >= 0) && (nFieldPos < eposEND) && (m_pFieldFlags[nFieldPos] & FieldFlag_Present))
        return true;
    return false;
}

QString Stdf_PMR_V3::GetRecordShortName(void)
{
    return "PMR";
}

QString Stdf_PMR_V3::GetRecordLongName(void)
{
    return "Pin Map Record";
}

int Stdf_PMR_V3::GetRecordType(void)
{
    return Rec_PMR;
}

bool Stdf_PMR_V3::Read(GS::StdLib::Stdf & clStdf)
{
    char	szString[STDF_MAX_U1+1];
    int		wData;

    // First reset data
    Reset();

    // PMR.CHAN_CNT
    _FIELD_CHECKREAD(clStdf.ReadWord(&wData), eposCHAN_CNT);
    _FIELD_SET(m_u2CHAN_CNT = stdf_type_u2(wData), true, eposCHAN_CNT);

    // PMR.NAME_CNT
    _FIELD_CHECKREAD(clStdf.ReadWord(&wData), eposNAME_CNT);
    _FIELD_SET(m_u2NAME_CNT = stdf_type_u2(wData), true, eposNAME_CNT);

    // PMR.CHAN_NUM
    if (m_u2CHAN_CNT > 0)
        m_u2CHAN_NUM = new stdf_type_u2[m_u2CHAN_CNT];
    for (int i=0; i<m_u2CHAN_CNT; ++i)
    {
        _FIELD_CHECKREAD(clStdf.ReadWord(&wData), eposCHAN_NUM);
        _FIELD_SET(m_u2CHAN_NUM[i] = wData, true, eposCHAN_NUM);
    }

    // PMR.PIN_NAME
    if (m_u2NAME_CNT > 0)
        m_cnPIN_NAME = new stdf_type_cn[m_u2NAME_CNT];
    for (int i=0; i<m_u2NAME_CNT; ++i)
    {
        _FIELD_CHECKREAD(clStdf.ReadString(szString), eposPIN_NAME);
        _FIELD_SET(m_cnPIN_NAME[i] = szString, true, eposPIN_NAME);
    }


    return true;
}

void Stdf_PMR_V3::GetAsciiString(QString & strAsciiString, int nFieldSelection /*= 0*/)
{
    // Empty string first
    strAsciiString = "";

    // PMR.CHAN_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "PMR.chan_cnt", QString::number(m_u2CHAN_CNT), nFieldSelection, eposCHAN_CNT);

    // PMR.NAME_CNT
    _STR_ADDFIELD_ASCII(strAsciiString, "PMR.name_cnt", QString::number(m_u2NAME_CNT), nFieldSelection, eposNAME_CNT);

    // PMR.CHAN_NUM
    for (int i=0; i<m_u2CHAN_CNT; ++i)
    {
        _STR_ADDFIELD_ASCII(strAsciiString, "PMR.chan_num", QString::number(m_u2CHAN_NUM[i]),
                            nFieldSelection, eposCHAN_NUM);
    }

    // PMR.PIN_NAME
    for (int i=0; i<m_u2NAME_CNT; ++i)
    {
        _STR_ADDFIELD_ASCII(strAsciiString, "PMR.chan_num", m_cnPIN_NAME[i], nFieldSelection, eposPIN_NAME);
    }

}

void Stdf_PMR_V3::GetAsciiFieldList(QStringList & listFields, int nFieldSelection /*= 0*/)
{
    // Empty string list first
    listFields.empty();

    // PMR.CHAN_CNT
    _LIST_ADDFIELD_ASCII(listFields, "PMR.chan_cnt", QString::number(m_u2CHAN_CNT), nFieldSelection, eposCHAN_CNT);

    // PMR.NAME_CNT
    _LIST_ADDFIELD_ASCII(listFields, "PMR.name_cnt", QString::number(m_u2NAME_CNT), nFieldSelection, eposNAME_CNT);

    // PMR.CHAN_NUM
    for (int i=0; i<m_u2CHAN_CNT; ++i)
    {
        _LIST_ADDFIELD_ASCII(listFields, "PMR.chan_num", QString::number(m_u2CHAN_NUM[i]),
                            nFieldSelection, eposCHAN_NUM);
    }

    // PMR.PIN_NAME
    for (int i=0; i<m_u2NAME_CNT; ++i)
    {
        _LIST_ADDFIELD_ASCII(listFields, "PMR.chan_num", m_cnPIN_NAME[i], nFieldSelection, eposPIN_NAME);
    }
}


}
