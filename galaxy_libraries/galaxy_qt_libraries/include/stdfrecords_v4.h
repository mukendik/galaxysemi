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
// StdfRecord class HEADER :
// This class contains classes to read and store STDF V4
// records
///////////////////////////////////////////////////////////

#ifndef _StdfRecords_V4_h_
#define _StdfRecords_V4_h_


// Galaxy modules includes
#include "stdf.h"
#include "stdfrecord.h"
#include <QMap>
#include <QString>
#include <QJsonObject>

namespace GQTL_STDF
{

///////////////////////////////////////////////////////////
// RESERVED_IMAGE RECORD
///////////////////////////////////////////////////////////
class Stdf_RESERVED_IMAGE_V4: public Stdf_Record
{
    Q_DISABLE_COPY(Stdf_RESERVED_IMAGE_V4)

// METHODS
public:
    // Constructor / destructor functions
    Stdf_RESERVED_IMAGE_V4();
    ~Stdf_RESERVED_IMAGE_V4();

    // Reset record data
    void		Reset(void);
    // Return short name of the record
    QString		GetRecordShortName(void);
    // Return long name of the record
    QString		GetRecordLongName(void);
    // Return record type
    int			GetRecordType(void);
    // Read record
    bool		Read(GS::StdLib::Stdf & clStdf);
    // Write record
    bool		Write(GS::StdLib::Stdf & clStdf);
    // Construct a stringlist with all fields to display
    // Each string in the list has following format: <Field name>;<Field value>
    void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
    // Construct a ASCII string of the record
    void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);
    // Construct a XML string of the record
    void		GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection = 0);
    // Construct a ATDF string of the record
    void		GetAtdfString(QString & strAtdfString);

    // Set members and flags

// DATA
public:

private:
};

///////////////////////////////////////////////////////////
// RESERVED_IG900 RECORD
///////////////////////////////////////////////////////////
class Stdf_RESERVED_IG900_V4: public Stdf_Record
{
    Q_DISABLE_COPY(Stdf_RESERVED_IG900_V4)

// METHODS
public:
    // Constructor / destructor functions
    Stdf_RESERVED_IG900_V4();
    ~Stdf_RESERVED_IG900_V4();

    // Reset record data
    void		Reset(void);
    // Return short name of the record
    QString		GetRecordShortName(void);
    // Return long name of the record
    QString		GetRecordLongName(void);
    // Return record type
    int			GetRecordType(void);
    // Read record
    bool		Read(GS::StdLib::Stdf & clStdf);
    // Write record
    bool		Write(GS::StdLib::Stdf & clStdf);
    // Construct a stringlist with all fields to display
    // Each string in the list has following format: <Field name>;<Field value>
    void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
    // Construct a ASCII string of the record
    void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);
    // Construct a XML string of the record
    void		GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection = 0);
    // Construct a ATDF string of the record
    void		GetAtdfString(QString & strAtdfString);

    // Set members and flags

// DATA
public:

private:
};

///////////////////////////////////////////////////////////
// UNKNOWN RECORD
///////////////////////////////////////////////////////////
class Stdf_UNKNOWN_V4: public Stdf_Record
{
    Q_DISABLE_COPY(Stdf_UNKNOWN_V4)

// METHODS
public:
    // Constructor / destructor functions
    Stdf_UNKNOWN_V4();
    ~Stdf_UNKNOWN_V4();

    // Reset record data
    void		Reset(void);
    // Return short name of the record
    QString		GetRecordShortName(void);
    // Return long name of the record
    QString		GetRecordLongName(void);
    // Return record type
    int			GetRecordType(void);
    // Read record
    bool		Read(GS::StdLib::Stdf & clStdf);
    // Write record
    bool		Write(GS::StdLib::Stdf & clStdf);
    // Construct a stringlist with all fields to display
    // Each string in the list has following format: <Field name>;<Field value>
    void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
    // Construct a ASCII string of the record
    void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);
    // Construct a XML string of the record
    void		GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection = 0);
    // Construct a ATDF string of the record
    void		GetAtdfString(QString & strAtdfString);

    // Set members and flags

// DATA
public:

private:
};

///////////////////////////////////////////////////////////
// FAR RECORD
///////////////////////////////////////////////////////////
class Stdf_FAR_V4: public Stdf_Record
{
    Q_DISABLE_COPY(Stdf_FAR_V4)

// METHODS
public:
    // Constructor / destructor functions
    Stdf_FAR_V4();
    ~Stdf_FAR_V4();

    // Reset record data
    void		Reset(void);
    // Return short name of the record
    QString		GetRecordShortName(void);
    // Return long name of the record
    QString		GetRecordLongName(void);
    // Return record type
    int			GetRecordType(void);
    // Read record
    bool		Read(GS::StdLib::Stdf & clStdf);
    // Write record
    bool		Write(GS::StdLib::Stdf & clStdf);
    void SetFieldFlags (const int pos, const int val){
        m_pFieldFlags[pos] = val;
    }

    // Construct a stringlist with all fields to display
    // Each string in the list has following format: <Field name>;<Field value>
    void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
    // Construct a ASCII string of the record
    void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);
    // Construct a XML string of the record
    void		GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection = 0);
    // Construct a ATDF string of the record
    void		GetAtdfString(QString & strAtdfString);

    // Set members and flags
    void		SetCPU_TYPE(const stdf_type_u1 u1CPU_TYPE)	_FIELD_SET(m_u1CPU_TYPE = u1CPU_TYPE, true, eposCPU_TYPE)
    void		SetSTDF_VER(const stdf_type_u1 u1STDF_VER)	_FIELD_SET(m_u1STDF_VER = u1STDF_VER, true, eposSTDF_VER)

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposCPU_TYPE			= 0,
            eposSTDF_VER			= 1,
            eposEND					= 2,
            eposFIRST_OPTIONAL		= 2
    };

    stdf_type_u1	m_u1CPU_TYPE;		// FAR.CPU_TYPE
    stdf_type_u1	m_u1STDF_VER;		// FAR.STDF_VER

private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};

///////////////////////////////////////////////////////////
// ATR RECORD
///////////////////////////////////////////////////////////
class Stdf_ATR_V4: public Stdf_Record
{
    Q_DISABLE_COPY(Stdf_ATR_V4)

// METHODS
public:
    // Constructor / destructor functions
    Stdf_ATR_V4();
    ~Stdf_ATR_V4();

    // Reset record data
    void		Reset(void);
    // Return short name of the record
    QString		GetRecordShortName(void);
    // Return long name of the record
    QString		GetRecordLongName(void);
    // Return record type
    int			GetRecordType(void);
    // Read record
    bool		Read(GS::StdLib::Stdf & clStdf);
    // Write record
    bool		Write(GS::StdLib::Stdf & clStdf);
    // Construct a stringlist with all fields to display
    // Each string in the list has following format: <Field name>;<Field value>
    void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
    // Construct a ASCII string of the record
    void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);
    // Construct a XML string of the record
    void		GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection = 0);
    // Construct a ATDF string of the record
    void		GetAtdfString(QString & strAtdfString);

    // Set members and flags
    void		SetMOD_TIM(const stdf_type_u4 u4MOD_TIM)	_FIELD_SET(m_u4MOD_TIM = u4MOD_TIM, true, eposMOD_TIM)
    void		SetCMD_LINE(const stdf_type_cn cnCMD_LINE)	_FIELD_SET(m_cnCMD_LINE = cnCMD_LINE, true, eposCMD_LINE)
    void		SetCMD_LINE()                               _FIELD_SET_FLAGS(true, eposCMD_LINE)

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposMOD_TIM				= 0,
            eposCMD_LINE			= 1,
            eposEND					= 2,
            eposFIRST_OPTIONAL		= 2
    };

    time_t			m_u4MOD_TIM;		// ATR.MOD_TIM
    stdf_type_cn	m_cnCMD_LINE;		// ATR.CMD_LINE

private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};

///////////////////////////////////////////////////////////
// MIR RECORD
///////////////////////////////////////////////////////////
class Stdf_MIR_V4: public Stdf_Record
{
    Q_DISABLE_COPY(Stdf_MIR_V4)

// METHODS
public:
    // Constructor / destructor functions
    Stdf_MIR_V4();
    ~Stdf_MIR_V4();

    // Reset record data
    void		Reset(void);
    // Return short name of the record
    QString		GetRecordShortName(void);
    // Return long name of the record
    QString		GetRecordLongName(void);
    // Return record type
    int			GetRecordType(void);
    // Read record
    bool		Read(GS::StdLib::Stdf & clStdf);
    // Write record
    bool		Write(GS::StdLib::Stdf & clStdf);
    // Construct a stringlist with all fields to display
    // Each string in the list has following format: <Field name>;<Field value>
    void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
    // Construct a ASCII string of the record
    void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);
    // Construct a XML string of the record
    void		GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection = 0);
    // Construct a ATDF string of the record
    void		GetAtdfString(QString & strAtdfString);

    // Set members and flags
    void		SetSETUP_T(const stdf_type_u4 u4SETUP_T)	_FIELD_SET(m_u4SETUP_T = u4SETUP_T, true, eposSETUP_T)
    void		SetSTART_T(const stdf_type_u4 u4START_T)	_FIELD_SET(m_u4START_T = u4START_T, true, eposSTART_T)
    void		SetSTAT_NUM(const stdf_type_u1 u1STAT_NUM)	_FIELD_SET(m_u1STAT_NUM = u1STAT_NUM, true, eposSTAT_NUM)
    void		SetMODE_COD(const stdf_type_c1 c1MODE_COD)	_FIELD_SET(m_c1MODE_COD = c1MODE_COD, true, eposMODE_COD)
    void		SetRTST_COD(const stdf_type_c1 c1RTST_COD)	_FIELD_SET(m_c1RTST_COD = c1RTST_COD, true, eposRTST_COD)
    void		SetPROT_COD(const stdf_type_c1 c1PROT_COD)	_FIELD_SET(m_c1PROT_COD = c1PROT_COD, true, eposPROT_COD)
    void		SetBURN_TIM(const stdf_type_u2 u2BURN_TIM)	_FIELD_SET(m_u2BURN_TIM = u2BURN_TIM, true, eposBURN_TIM)
    void		SetCMOD_COD(const stdf_type_c1 c1CMOD_COD)	_FIELD_SET(m_c1CMOD_COD = c1CMOD_COD, true, eposCMOD_COD)
    void		SetLOT_ID(const stdf_type_cn cnLOT_ID)		_FIELD_SET(m_cnLOT_ID = cnLOT_ID, true, eposLOT_ID)
    void		SetPART_TYP(const stdf_type_cn cnPART_TYP)	_FIELD_SET(m_cnPART_TYP = cnPART_TYP, true, eposPART_TYP)
    void		SetNODE_NAM(const stdf_type_cn cnNODE_NAM)	_FIELD_SET(m_cnNODE_NAM = cnNODE_NAM, true, eposNODE_NAM)
    void		SetTSTR_TYP(const stdf_type_cn cnTSTR_TYP)	_FIELD_SET(m_cnTSTR_TYP = cnTSTR_TYP, true, eposTSTR_TYP)
    void		SetJOB_NAM(const stdf_type_cn cnJOB_NAM)	_FIELD_SET(m_cnJOB_NAM = cnJOB_NAM, true, eposJOB_NAM)
    void		SetJOB_REV(const stdf_type_cn cnJOB_REV)	_FIELD_SET(m_cnJOB_REV = cnJOB_REV, true, eposJOB_REV)
    void		SetSBLOT_ID(const stdf_type_cn cnSBLOT_ID)	_FIELD_SET(m_cnSBLOT_ID = cnSBLOT_ID, true, eposSBLOT_ID)
    void		SetOPER_NAM(const stdf_type_cn cnOPER_NAM)	_FIELD_SET(m_cnOPER_NAM = cnOPER_NAM, true, eposOPER_NAM)
    void		SetEXEC_TYP(const stdf_type_cn cnEXEC_TYP)	_FIELD_SET(m_cnEXEC_TYP = cnEXEC_TYP, true, eposEXEC_TYP)
    void		SetEXEC_VER(const stdf_type_cn cnEXEC_VER)	_FIELD_SET(m_cnEXEC_VER = cnEXEC_VER, true, eposEXEC_VER)
    void		SetTEST_COD(const stdf_type_cn cnTEST_COD)	_FIELD_SET(m_cnTEST_COD = cnTEST_COD, true, eposTEST_COD)
    void		SetTST_TEMP(const stdf_type_cn cnTST_TEMP)	_FIELD_SET(m_cnTST_TEMP = cnTST_TEMP, true, eposTST_TEMP)
    void		SetUSER_TXT(const stdf_type_cn cnUSER_TXT)	_FIELD_SET(m_cnUSER_TXT = cnUSER_TXT, true, eposUSER_TXT)
    void		SetAUX_FILE(const stdf_type_cn cnAUX_FILE)	_FIELD_SET(m_cnAUX_FILE = cnAUX_FILE, true, eposAUX_FILE)
    void		SetPKG_TYP(const stdf_type_cn cnPKG_TYP)	_FIELD_SET(m_cnPKG_TYP = cnPKG_TYP, true, eposPKG_TYP)
    void		SetFAMLY_ID(const stdf_type_cn cnFAMLY_ID)	_FIELD_SET(m_cnFAMLY_ID = cnFAMLY_ID, true, eposFAMLY_ID)
    void		SetDATE_COD(const stdf_type_cn cnDATE_COD)	_FIELD_SET(m_cnDATE_COD = cnDATE_COD, true, eposDATE_COD)
    void		SetFACIL_ID(const stdf_type_cn cnFACIL_ID)	_FIELD_SET(m_cnFACIL_ID = cnFACIL_ID, true, eposFACIL_ID)
    void		SetFLOOR_ID(const stdf_type_cn cnFLOOR_ID)	_FIELD_SET(m_cnFLOOR_ID = cnFLOOR_ID, true, eposFLOOR_ID)
    void		SetPROC_ID(const stdf_type_cn cnPROC_ID)	_FIELD_SET(m_cnPROC_ID = cnPROC_ID, true, eposPROC_ID)
    void		SetOPER_FRQ(const stdf_type_cn cnOPER_FRQ)	_FIELD_SET(m_cnOPER_FRQ = cnOPER_FRQ, true, eposOPER_FRQ)
    void		SetSPEC_NAM(const stdf_type_cn cnSPEC_NAM)	_FIELD_SET(m_cnSPEC_NAM = cnSPEC_NAM, true, eposSPEC_NAM)
    void		SetSPEC_VER(const stdf_type_cn cnSPEC_VER)	_FIELD_SET(m_cnSPEC_VER = cnSPEC_VER, true, eposSPEC_VER)
    void		SetFLOW_ID(const stdf_type_cn cnFLOW_ID)	_FIELD_SET(m_cnFLOW_ID = cnFLOW_ID, true, eposFLOW_ID)
    void		SetSETUP_ID(const stdf_type_cn cnSETUP_ID)	_FIELD_SET(m_cnSETUP_ID = cnSETUP_ID, true, eposSETUP_ID)
    void		SetDSGN_REV(const stdf_type_cn cnDSGN_REV)	_FIELD_SET(m_cnDSGN_REV = cnDSGN_REV, true, eposDSGN_REV)
    void		SetENG_ID(const stdf_type_cn cnENG_ID)		_FIELD_SET(m_cnENG_ID = cnENG_ID, true, eposENG_ID)
    void		SetROM_COD(const stdf_type_cn cnROM_COD)	_FIELD_SET(m_cnROM_COD = cnROM_COD, true, eposROM_COD)
    void		SetSERL_NUM(const stdf_type_cn cnSERL_NUM)	_FIELD_SET(m_cnSERL_NUM = cnSERL_NUM, true, eposSERL_NUM)
    void		SetSUPR_NAM(const stdf_type_cn cnSUPR_NAM)	_FIELD_SET(m_cnSUPR_NAM = cnSUPR_NAM, true, eposSUPR_NAM)

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposSETUP_T				= 0,
            eposSTART_T				= 1,
            eposSTAT_NUM			= 2,
            eposMODE_COD			= 3,
            eposRTST_COD			= 4,
            eposPROT_COD			= 5,
            eposBURN_TIM			= 6,
            eposCMOD_COD			= 7,
            eposLOT_ID				= 8,
            eposPART_TYP			= 9,
            eposNODE_NAM			= 10,
            eposTSTR_TYP			= 11,
            eposJOB_NAM				= 12,
            eposJOB_REV				= 13,
            eposFIRST_OPTIONAL		= 13,
            eposSBLOT_ID			= 14,
            eposOPER_NAM			= 15,
            eposEXEC_TYP			= 16,
            eposEXEC_VER			= 17,
            eposTEST_COD			= 18,
            eposTST_TEMP			= 19,
            eposUSER_TXT			= 20,
            eposAUX_FILE			= 21,
            eposPKG_TYP				= 22,
            eposFAMLY_ID			= 23,
            eposDATE_COD			= 24,
            eposFACIL_ID			= 25,
            eposFLOOR_ID			= 26,
            eposPROC_ID				= 27,
            eposOPER_FRQ			= 28,
            eposSPEC_NAM			= 29,
            eposSPEC_VER			= 30,
            eposFLOW_ID				= 31,
            eposSETUP_ID			= 32,
            eposDSGN_REV			= 33,
            eposENG_ID				= 34,
            eposROM_COD				= 35,
            eposSERL_NUM			= 36,
            eposSUPR_NAM			= 37,
            eposEND					= 38
    };

    time_t			m_u4SETUP_T;		// MIR.SETUP_T
    time_t			m_u4START_T;		// MIR.START_T
    stdf_type_u1	m_u1STAT_NUM;		// MIR.STAT_NUM
    stdf_type_c1	m_c1MODE_COD;		// MIR.MODE_COD
    stdf_type_c1	m_c1RTST_COD;		// MIR.RTST_COD
    stdf_type_c1	m_c1PROT_COD;		// MIR.PROT_COD
    stdf_type_u2	m_u2BURN_TIM;		// MIR.BURN_TIM
    stdf_type_c1	m_c1CMOD_COD;		// MIR.CMOD_COD
    stdf_type_cn	m_cnLOT_ID;			// MIR.LOT_ID
    stdf_type_cn	m_cnPART_TYP;		// MIR.PART_TYP
    stdf_type_cn	m_cnNODE_NAM;		// MIR.NODE_NAM
    stdf_type_cn	m_cnTSTR_TYP;		// MIR.TSTR_TYP
    stdf_type_cn	m_cnJOB_NAM;		// MIR.JOB_NAM
    stdf_type_cn	m_cnJOB_REV;		// MIR.JOB_REV
    stdf_type_cn	m_cnSBLOT_ID;		// MIR.SBLOT_ID
    stdf_type_cn	m_cnOPER_NAM;		// MIR.OPER_NAM
    stdf_type_cn	m_cnEXEC_TYP;		// MIR.EXEC_TYP
    stdf_type_cn	m_cnEXEC_VER;		// MIR.EXEC_VER
    stdf_type_cn	m_cnTEST_COD;		// MIR.TEST_COD
    stdf_type_cn	m_cnTST_TEMP;		// MIR.TST_TEMP
    stdf_type_cn	m_cnUSER_TXT;		// MIR.USER_TXT
    stdf_type_cn	m_cnAUX_FILE;		// MIR.AUX_FILE
    stdf_type_cn	m_cnPKG_TYP;		// MIR.PKG_TYP
    stdf_type_cn	m_cnFAMLY_ID;		// MIR.FAMLY_ID
    stdf_type_cn	m_cnDATE_COD;		// MIR.DATE_COD
    stdf_type_cn	m_cnFACIL_ID;		// MIR.FACIL_ID
    stdf_type_cn	m_cnFLOOR_ID;		// MIR.FLOOR_ID
    stdf_type_cn	m_cnPROC_ID;		// MIR.PROC_ID
    stdf_type_cn	m_cnOPER_FRQ;		// MIR.OPER_FRQ
    stdf_type_cn	m_cnSPEC_NAM;		// MIR.SPEC_NAM
    stdf_type_cn	m_cnSPEC_VER;		// MIR.SPEC_VER
    stdf_type_cn	m_cnFLOW_ID;		// MIR.FLOW_ID
    stdf_type_cn	m_cnSETUP_ID;		// MIR.SETUP_ID
    stdf_type_cn	m_cnDSGN_REV;		// MIR.DSGN_REV
    stdf_type_cn	m_cnENG_ID;			// MIR.ENG_ID
    stdf_type_cn	m_cnROM_COD;		// MIR.ROM_COD
    stdf_type_cn	m_cnSERL_NUM;		// MIR.SERL_NUM
    stdf_type_cn	m_cnSUPR_NAM;		// MIR.SUPR_NAM

private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};

///////////////////////////////////////////////////////////
// MRR RECORD
///////////////////////////////////////////////////////////
class Stdf_MRR_V4: public Stdf_Record
{
    Q_DISABLE_COPY(Stdf_MRR_V4)

// METHODS
public:
    // Constructor / destructor functions
    Stdf_MRR_V4();
    ~Stdf_MRR_V4();

    // Reset record data
    void		Reset(void);
    // Return short name of the record
    QString		GetRecordShortName(void);
    // Return long name of the record
    QString		GetRecordLongName(void);
    // Return record type
    int			GetRecordType(void);
    // Read record
    bool		Read(GS::StdLib::Stdf & clStdf);
    // Write record
    bool		Write(GS::StdLib::Stdf & clStdf);
    // Construct a stringlist with all fields to display
    // Each string in the list has following format: <Field name>;<Field value>
    void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
    // Construct a ASCII string of the record
    void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);
    // Construct a XML string of the record
    void		GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection = 0);
    // Construct a ATDF string of the record
    void		GetAtdfString(QString & strAtdfString);

    // Set members and flags
    void		SetFINISH_T(const stdf_type_u4 u4FINISH_T)	_FIELD_SET(m_u4FINISH_T = u4FINISH_T, true, eposFINISH_T)
    void		SetDISP_COD(const stdf_type_c1 c1DISP_COD)	_FIELD_SET(m_c1DISP_COD = c1DISP_COD, true, eposDISP_COD)
    void		SetUSR_DESC(const stdf_type_cn cnUSR_DESC)	_FIELD_SET(m_cnUSR_DESC = cnUSR_DESC, true, eposUSR_DESC)
    void		SetEXC_DESC(const stdf_type_cn cnEXC_DESC)	_FIELD_SET(m_cnEXC_DESC = cnEXC_DESC, true, eposEXC_DESC)

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposFINISH_T			= 0,
            eposDISP_COD			= 1,
            eposFIRST_OPTIONAL		= 1,
            eposUSR_DESC			= 2,
            eposEXC_DESC			= 3,
            eposEND					= 4
    };

    time_t			m_u4FINISH_T;		// MRR.FINISH_T
    stdf_type_c1	m_c1DISP_COD;		// MRR.DISP_COD
    stdf_type_cn	m_cnUSR_DESC;		// MRR.USR_DESC
    stdf_type_cn	m_cnEXC_DESC;		// MRR.EXC_DESC

private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};

///////////////////////////////////////////////////////////
// PCR RECORD
///////////////////////////////////////////////////////////
class Stdf_PCR_V4: public Stdf_Record
{
    Q_DISABLE_COPY(Stdf_PCR_V4)

// METHODS
public:
    // Constructor / destructor functions
    Stdf_PCR_V4();
    ~Stdf_PCR_V4();

    // Reset record data
    void		Reset(void);
    // Return short name of the record
    QString		GetRecordShortName(void);
    // Return long name of the record
    QString		GetRecordLongName(void);
    // Return record type
    int			GetRecordType(void);
    // Read record
    bool		Read(GS::StdLib::Stdf & clStdf);
    // Write record
    bool		Write(GS::StdLib::Stdf & clStdf);
    // Construct a stringlist with all fields to display
    // Each string in the list has following format: <Field name>;<Field value>
    void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
    // Construct a ASCII string of the record
    void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);
    // Construct a XML string of the record
    void		GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection = 0);
    // Construct a ATDF string of the record
    void		GetAtdfString(QString & strAtdfString);

    // Set members and flags
    void		SetHEAD_NUM(const stdf_type_u1 u1HEAD_NUM)	_FIELD_SET(m_u1HEAD_NUM = u1HEAD_NUM, true, eposHEAD_NUM)
    void		SetSITE_NUM(const stdf_type_u1 u1SITE_NUM)	_FIELD_SET(m_u1SITE_NUM = u1SITE_NUM, true, eposSITE_NUM)
    void		SetPART_CNT(const stdf_type_u4 u4PART_CNT)	_FIELD_SET(m_u4PART_CNT = u4PART_CNT, true, eposPART_CNT)
    void		SetRTST_CNT(const stdf_type_u4 u4RTST_CNT)	_FIELD_SET(m_u4RTST_CNT = u4RTST_CNT, true, eposRTST_CNT)
    void		SetABRT_CNT(const stdf_type_u4 u4ABRT_CNT)	_FIELD_SET(m_u4ABRT_CNT = u4ABRT_CNT, true, eposABRT_CNT)
    void		SetGOOD_CNT(const stdf_type_u4 u4GOOD_CNT)	_FIELD_SET(m_u4GOOD_CNT = u4GOOD_CNT, true, eposGOOD_CNT)
    void		SetFUNC_CNT(const stdf_type_u4 u4FUNC_CNT)	_FIELD_SET(m_u4FUNC_CNT = u4FUNC_CNT, true, eposFUNC_CNT)

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposHEAD_NUM			= 0,
            eposSITE_NUM			= 1,
            eposPART_CNT			= 2,
            eposRTST_CNT			= 3,
            eposFIRST_OPTIONAL		= 3,
            eposABRT_CNT			= 4,
            eposGOOD_CNT			= 5,
            eposFUNC_CNT			= 6,
            eposEND					= 7
    };

    stdf_type_u1	m_u1HEAD_NUM;		// PCR.HEAD_NUM
    stdf_type_u1	m_u1SITE_NUM;		// PCR.SITE_NUM
    stdf_type_u4	m_u4PART_CNT;		// PCR.PART_CNT
    stdf_type_u4	m_u4RTST_CNT;		// PCR.RTST_CNT
    stdf_type_u4	m_u4ABRT_CNT;		// PCR.ABRT_CNT
    stdf_type_u4	m_u4GOOD_CNT;		// PCR.GOOD_CNT
    stdf_type_u4	m_u4FUNC_CNT;		// PCR.FUNC_CNT

private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};

///////////////////////////////////////////////////////////
// HBR RECORD
///////////////////////////////////////////////////////////
class Stdf_HBR_V4: public Stdf_Record
{
    Q_DISABLE_COPY(Stdf_HBR_V4)

// METHODS
public:
    // Constructor / destructor functions
    Stdf_HBR_V4();
    ~Stdf_HBR_V4();

    // Reset record data
    void		Reset(void);
    // Return short name of the record
    QString		GetRecordShortName(void);
    // Return long name of the record
    QString		GetRecordLongName(void);
    // Return record type
    int			GetRecordType(void);
    // Read record
    bool		Read(GS::StdLib::Stdf & clStdf);
    // Write record
    bool		Write(GS::StdLib::Stdf & clStdf);
    // Construct a stringlist with all fields to display
    // Each string in the list has following format: <Field name>;<Field value>
    void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
    // Construct a ASCII string of the record
    void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);
    // Construct a XML string of the record
    void		GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection = 0);
    // Construct a ATDF string of the record
    void		GetAtdfString(QString & strAtdfString);

    // Set members and flags
    void		SetHEAD_NUM(const stdf_type_u1 u1HEAD_NUM)	_FIELD_SET(m_u1HEAD_NUM = u1HEAD_NUM, true, eposHEAD_NUM);
    void		SetSITE_NUM(const stdf_type_u1 u1SITE_NUM)	_FIELD_SET(m_u1SITE_NUM = u1SITE_NUM, true, eposSITE_NUM);
    void		SetHBIN_NUM(const stdf_type_u2 u2HBIN_NUM)	_FIELD_SET(m_u2HBIN_NUM = u2HBIN_NUM, true, eposHBIN_NUM);
    void		SetHBIN_CNT(const stdf_type_u4 u4HBIN_CNT)	_FIELD_SET(m_u4HBIN_CNT = u4HBIN_CNT, true, eposHBIN_CNT);
    void		SetHBIN_PF(const stdf_type_c1 c1HBIN_PF)	_FIELD_SET(m_c1HBIN_PF = c1HBIN_PF, true, eposHBIN_PF);
    void		SetHBIN_NAM(const stdf_type_cn cnHBIN_NAM)	_FIELD_SET(m_cnHBIN_NAM = cnHBIN_NAM, true, eposHBIN_NAM);

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposHEAD_NUM			= 0,
            eposSITE_NUM			= 1,
            eposHBIN_NUM			= 2,
            eposHBIN_CNT			= 3,
            eposHBIN_PF				= 4,
            eposFIRST_OPTIONAL		= 4,
            eposHBIN_NAM			= 5,
            eposEND					= 6
    };

    stdf_type_u1	m_u1HEAD_NUM;		// HBR.HEAD_NUM
    stdf_type_u1	m_u1SITE_NUM;		// HBR.SITE_NUM
    stdf_type_u2	m_u2HBIN_NUM;		// HBR.HBIN_NUM
    stdf_type_u4	m_u4HBIN_CNT;		// HBR.HBIN_CNT
    stdf_type_c1	m_c1HBIN_PF;		// HBR.HBIN_PF
    stdf_type_cn	m_cnHBIN_NAM;		// HBR.HBIN_NAM

private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};

///////////////////////////////////////////////////////////
// SBR RECORD
///////////////////////////////////////////////////////////
class Stdf_SBR_V4: public Stdf_Record
{
    Q_DISABLE_COPY(Stdf_SBR_V4)

// METHODS
public:
    // Constructor / destructor functions
    Stdf_SBR_V4();
    ~Stdf_SBR_V4();

    // Reset record data
    void		Reset(void);
    // Return short name of the record
    QString		GetRecordShortName(void);
    // Return long name of the record
    QString		GetRecordLongName(void);
    // Return record type
    int			GetRecordType(void);
    // Read record
    bool		Read(GS::StdLib::Stdf & clStdf);
    // Write record
    bool		Write(GS::StdLib::Stdf & clStdf);
    // Construct a stringlist with all fields to display
    // Each string in the list has following format: <Field name>;<Field value>
    void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
    // Construct a ASCII string of the record
    void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);
    // Construct a XML string of the record
    void		GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection = 0);
    // Construct a ATDF string of the record
    void		GetAtdfString(QString & strAtdfString);

    // Set members and flags
    void		SetHEAD_NUM(const stdf_type_u1 u1HEAD_NUM)	_FIELD_SET(m_u1HEAD_NUM = u1HEAD_NUM, true, eposHEAD_NUM)
    void		SetSITE_NUM(const stdf_type_u1 u1SITE_NUM)	_FIELD_SET(m_u1SITE_NUM = u1SITE_NUM, true, eposSITE_NUM)
    void		SetSBIN_NUM(const stdf_type_u2 u2SBIN_NUM)	_FIELD_SET(m_u2SBIN_NUM = u2SBIN_NUM, true, eposSBIN_NUM)
    void		SetSBIN_CNT(const stdf_type_u4 u4SBIN_CNT)	_FIELD_SET(m_u4SBIN_CNT = u4SBIN_CNT, true, eposSBIN_CNT)
    void		SetSBIN_PF(const stdf_type_c1 c1SBIN_PF)	_FIELD_SET(m_c1SBIN_PF = c1SBIN_PF, true, eposSBIN_PF)
    void		SetSBIN_NAM(const stdf_type_cn cnSBIN_NAM)	_FIELD_SET(m_cnSBIN_NAM = cnSBIN_NAM, true, eposSBIN_NAM)

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposHEAD_NUM			= 0,
            eposSITE_NUM			= 1,
            eposSBIN_NUM			= 2,
            eposSBIN_CNT			= 3,
            eposSBIN_PF				= 4,
            eposFIRST_OPTIONAL		= 4,
            eposSBIN_NAM			= 5,
            eposEND					= 6
    };

    stdf_type_u1	m_u1HEAD_NUM;		// SBR.HEAD_NUM
    stdf_type_u1	m_u1SITE_NUM;		// SBR.SITE_NUM
    stdf_type_u2	m_u2SBIN_NUM;		// SBR.SBIN_NUM
    stdf_type_u4	m_u4SBIN_CNT;		// SBR.SBIN_CNT
    stdf_type_c1	m_c1SBIN_PF;		// SBR.SBIN_PF
    stdf_type_cn	m_cnSBIN_NAM;		// SBR.SBIN_NAM

private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};

///////////////////////////////////////////////////////////
// PMR RECORD
///////////////////////////////////////////////////////////
class Stdf_PMR_V4: public Stdf_Record
{
    Q_DISABLE_COPY(Stdf_PMR_V4)

// METHODS
public:
    // Constructor / destructor functions
    Stdf_PMR_V4();
    ~Stdf_PMR_V4();

    // Reset record data
    void		Reset(void);
    // Return short name of the record
    QString		GetRecordShortName(void);
    // Return long name of the record
    QString		GetRecordLongName(void);
    // Return record type
    int			GetRecordType(void);
    // Read record
    bool		Read(GS::StdLib::Stdf & clStdf);
    // Write record
    bool		Write(GS::StdLib::Stdf & clStdf);
    // Construct a stringlist with all fields to display
    // Each string in the list has following format: <Field name>;<Field value>
    void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
    // Construct a ASCII string of the record
    void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);
    // Construct a XML string of the record
    void		GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection = 0);
    // Construct a ATDF string of the record
    void		GetAtdfString(QString & strAtdfString);

    // Set members and flags
    void		SetPMR_INDX(const stdf_type_u2 u2PMR_INDX)	_FIELD_SET(m_u2PMR_INDX = u2PMR_INDX, true, eposPMR_INDX)
    void		SetCHAN_TYP(const stdf_type_u2 u2CHAN_TYP)	_FIELD_SET(m_u2CHAN_TYP = u2CHAN_TYP, true, eposCHAN_TYP)
    void		SetCHAN_NAM(const stdf_type_cn cnCHAN_NAM)	_FIELD_SET(m_cnCHAN_NAM = cnCHAN_NAM, true, eposCHAN_NAM)
    void		SetPHY_NAM(const stdf_type_cn cnPHY_NAM)	_FIELD_SET(m_cnPHY_NAM = cnPHY_NAM, true, eposPHY_NAM)
    void		SetLOG_NAM(const stdf_type_cn cnLOG_NAM)	_FIELD_SET(m_cnLOG_NAM = cnLOG_NAM, true, eposLOG_NAM)
    void		SetHEAD_NUM(const stdf_type_u1 u1HEAD_NUM)	_FIELD_SET(m_u1HEAD_NUM = u1HEAD_NUM, true, eposHEAD_NUM)
    void		SetSITE_NUM(const stdf_type_u1 u1SITE_NUM)	_FIELD_SET(m_u1SITE_NUM = u1SITE_NUM, true, eposSITE_NUM)

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposPMR_INDX			= 0,
            eposCHAN_TYP			= 1,
            eposFIRST_OPTIONAL		= 1,
            eposCHAN_NAM			= 2,
            eposPHY_NAM				= 3,
            eposLOG_NAM				= 4,
            eposHEAD_NUM			= 5,
            eposSITE_NUM			= 6,
            eposEND					= 7
    };

    stdf_type_u2	m_u2PMR_INDX;		// PMR.PMR_INDX
    stdf_type_u2	m_u2CHAN_TYP;		// PMR.CHAN_TYP
    stdf_type_cn	m_cnCHAN_NAM;		// PMR.CHAN_NAM
    stdf_type_cn	m_cnPHY_NAM;		// PMR.PHY_NAM
    stdf_type_cn	m_cnLOG_NAM;		// PMR.LOG_NAM
    stdf_type_u1	m_u1HEAD_NUM;		// PMR.HEAD_NUM
    stdf_type_u1	m_u1SITE_NUM;		// PMR.SITE_NUM

private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};

///////////////////////////////////////////////////////////
// PGR RECORD
///////////////////////////////////////////////////////////
class Stdf_PGR_V4: public Stdf_Record
{
    Q_DISABLE_COPY(Stdf_PGR_V4)

// METHODS
public:
    // Constructor / destructor functions
    Stdf_PGR_V4();
    ~Stdf_PGR_V4();

    // Reset record data
    void		Reset(void);
    // Return short name of the record
    QString		GetRecordShortName(void);
    // Return long name of the record
    QString		GetRecordLongName(void);
    // Return record type
    int			GetRecordType(void);
    // Read record
    bool		Read(GS::StdLib::Stdf & clStdf);
    // Write record
    bool		Write(GS::StdLib::Stdf & clStdf);
    // Construct a stringlist with all fields to display
    // Each string in the list has following format: <Field name>;<Field value>
    void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
    // Construct a ASCII string of the record
    void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);
    // Construct a XML string of the record
    void		GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection = 0);
    // Construct a ATDF string of the record
    void		GetAtdfString(QString & strAtdfString);

    // Set members and flags
    void		SetGRP_INDX(const stdf_type_u2 u2GRP_INDX)		_FIELD_SET(m_u2GRP_INDX = u2GRP_INDX, true, eposGRP_INDX)
    void		SetGRP_NAM(const stdf_type_cn cnGRP_NAM)		_FIELD_SET(m_cnGRP_NAM = cnGRP_NAM, true, eposGRP_NAM)
    void		SetINDX_CNT(const stdf_type_u2 u2INDX_CNT)		_FIELD_SET(m_u2INDX_CNT = u2INDX_CNT, true, eposINDX_CNT)
    void		SetPMR_INDX()                                   _FIELD_SET_FLAGS(m_u2INDX_CNT > 0, eposPMR_INDX)
    void		SetPMR_INDX(const stdf_type_u2 u2Index, const stdf_type_u2 u2PMR_INDX)
    {
        if((m_pFieldFlags[eposINDX_CNT] & FieldFlag_Present) && (m_u2INDX_CNT > 0))
        {
            if(m_ku2PMR_INDX == NULL)
                m_ku2PMR_INDX = new stdf_type_u2[m_u2INDX_CNT];
            if(u2Index < m_u2INDX_CNT)	_FIELD_SET(m_ku2PMR_INDX[u2Index] = u2PMR_INDX, true, eposPMR_INDX)
        }
    }

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposGRP_INDX			= 0,
            eposGRP_NAM				= 1,
            eposINDX_CNT			= 2,
            eposPMR_INDX			= 3,
            eposEND					= 4,
            eposFIRST_OPTIONAL		= 4
    };

    stdf_type_u2	m_u2GRP_INDX;		// PGR.GRP_INDX
    stdf_type_cn	m_cnGRP_NAM;		// PGR.GRP_NAM
    stdf_type_u2	m_u2INDX_CNT;		// PGR.INDX_CNT
    stdf_type_u2*	m_ku2PMR_INDX;		// PGR.PMR_INDX

private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};

///////////////////////////////////////////////////////////
// PLR RECORD
///////////////////////////////////////////////////////////
class Stdf_PLR_V4: public Stdf_Record
{
    Q_DISABLE_COPY(Stdf_PLR_V4)

// METHODS
public:
    // Constructor / destructor functions
    Stdf_PLR_V4();
    ~Stdf_PLR_V4();

    // Reset record data
    void		Reset(void);
    // Return short name of the record
    QString		GetRecordShortName(void);
    // Return long name of the record
    QString		GetRecordLongName(void);
    // Return record type
    int			GetRecordType(void);
    // Read record
    bool		Read(GS::StdLib::Stdf & clStdf);
    // Write record
    bool		Write(GS::StdLib::Stdf & clStdf);
    // Construct a stringlist with all fields to display
    // Each string in the list has following format: <Field name>;<Field value>
    void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
    // Construct a ASCII string of the record
    void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);
    // Construct a XML string of the record
    void		GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection = 0);
    // Construct a ATDF string of the record
    void		GetAtdfString(QString & strAtdfString);

    // Set members and flags
    void		SetGRP_CNT(const stdf_type_u2 u2GRP_CNT)	_FIELD_SET(m_u2GRP_CNT = u2GRP_CNT, true, eposGRP_CNT)
    void		SetGRP_INDX()                               _FIELD_SET_FLAGS(m_u2GRP_CNT>0, eposGRP_INDX)
    void		SetGRP_INDX(const stdf_type_u2 u2Index, const stdf_type_u2 u2GRP_INDX)
    {
        if((m_pFieldFlags[eposGRP_CNT] & FieldFlag_Present) && (m_u2GRP_CNT > 0))
        {
            if(m_ku2GRP_INDX == NULL)
                m_ku2GRP_INDX = new stdf_type_u2[m_u2GRP_CNT];
            if(u2Index < m_u2GRP_CNT)	_FIELD_SET(m_ku2GRP_INDX[u2Index] = u2GRP_INDX, true, eposGRP_INDX)
        }
    }
    void		SetGRP_MODE()                               _FIELD_SET_FLAGS(m_u2GRP_CNT>0, eposGRP_MODE)
    void		SetGRP_MODE(const stdf_type_u2 u2Index, const stdf_type_u2 u2GRP_MODE)
    {
        if((m_pFieldFlags[eposGRP_CNT] & FieldFlag_Present) && (m_u2GRP_CNT > 0))
        {
            if(m_ku2GRP_MODE == NULL)
                m_ku2GRP_MODE = new stdf_type_u2[m_u2GRP_CNT];
            if(u2Index < m_u2GRP_CNT)	_FIELD_SET(m_ku2GRP_MODE[u2Index] = u2GRP_MODE, true, eposGRP_MODE)
        }
    }
    void		SetGRP_RADX(const stdf_type_u2 u2Index, const stdf_type_u1 u1GRP_RADX)
    {
        if((m_pFieldFlags[eposGRP_CNT] & FieldFlag_Present) && (m_u2GRP_CNT > 0))
        {
            if(m_ku1GRP_RADX == NULL)
                m_ku1GRP_RADX = new stdf_type_u1[m_u2GRP_CNT];
            if(u2Index < m_u2GRP_CNT)	_FIELD_SET(m_ku1GRP_RADX[u2Index] = u1GRP_RADX, true, eposGRP_RADX)
        }
    }
    void		SetPGM_CHAR()           _FIELD_SET_FLAGS(m_u2GRP_CNT>0, eposPGM_CHAR)
    void		SetPGM_CHAR(const stdf_type_u2 u2Index, const stdf_type_cn cnPGM_CHAR)
    {
        if((m_pFieldFlags[eposGRP_CNT] & FieldFlag_Present) && (m_u2GRP_CNT > 0))
        {
            if(m_kcnPGM_CHAR == NULL)
                m_kcnPGM_CHAR = new stdf_type_cn[m_u2GRP_CNT];
            if(u2Index < m_u2GRP_CNT)	_FIELD_SET(m_kcnPGM_CHAR[u2Index] = cnPGM_CHAR, true, eposPGM_CHAR)
        }
    }
    void		SetRTN_CHAR()           _FIELD_SET_FLAGS(m_u2GRP_CNT>0, eposRTN_CHAR)
    void		SetRTN_CHAR(const stdf_type_u2 u2Index, const stdf_type_cn cnRTN_CHAR)
    {
        if((m_pFieldFlags[eposGRP_CNT] & FieldFlag_Present) && (m_u2GRP_CNT > 0))
        {
            if(m_kcnRTN_CHAR == NULL)
                m_kcnRTN_CHAR = new stdf_type_cn[m_u2GRP_CNT];
            if(u2Index < m_u2GRP_CNT)	_FIELD_SET(m_kcnRTN_CHAR[u2Index] = cnRTN_CHAR, true, eposRTN_CHAR)
        }
    }
    void		SetPGM_CHAL()           _FIELD_SET_FLAGS(m_u2GRP_CNT>0, eposPGM_CHAL)
    void		SetPGM_CHAL(const stdf_type_u2 u2Index, const stdf_type_cn cnPGM_CHAL)
    {
        if((m_pFieldFlags[eposGRP_CNT] & FieldFlag_Present) && (m_u2GRP_CNT > 0))
        {
            if(m_kcnPGM_CHAL == NULL)
                m_kcnPGM_CHAL = new stdf_type_cn[m_u2GRP_CNT];
            if(u2Index < m_u2GRP_CNT)	_FIELD_SET(m_kcnPGM_CHAL[u2Index] = cnPGM_CHAL, true, eposPGM_CHAL)
        }
    }
    void		SetRTN_CHAL()           _FIELD_SET_FLAGS(m_u2GRP_CNT>0, eposRTN_CHAL)
    void		SetRTN_CHAL(const stdf_type_u2 u2Index, const stdf_type_cn cnRTN_CHAL)
    {
        if((m_pFieldFlags[eposGRP_CNT] & FieldFlag_Present) && (m_u2GRP_CNT > 0))
        {
            if(m_kcnRTN_CHAL == NULL)
                m_kcnRTN_CHAL = new stdf_type_cn[m_u2GRP_CNT];
            if(u2Index < m_u2GRP_CNT)	_FIELD_SET(m_kcnRTN_CHAL[u2Index] = cnRTN_CHAL, true, eposRTN_CHAL)
        }
    }


    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposGRP_CNT				= 0,
            eposGRP_INDX			= 1,
            eposGRP_MODE			= 2,
            eposFIRST_OPTIONAL		= 2,
            eposGRP_RADX			= 3,
            eposPGM_CHAR			= 4,
            eposRTN_CHAR			= 5,
            eposPGM_CHAL			= 6,
            eposRTN_CHAL			= 7,
            eposEND					= 8
    };

    stdf_type_u2	m_u2GRP_CNT;		// PLR.GRP_CNT
    stdf_type_u2*	m_ku2GRP_INDX;		// PLR.GRP_INDX
    stdf_type_u2*	m_ku2GRP_MODE;		// PLR.GRP_MODE
    stdf_type_u1*	m_ku1GRP_RADX;		// PLR.GRP_RADX
    stdf_type_cn*	m_kcnPGM_CHAR;		// PLR.PGM_CHAR
    stdf_type_cn*	m_kcnRTN_CHAR;		// PLR.RTN_CHAR
    stdf_type_cn*	m_kcnPGM_CHAL;		// PLR.PGM_CHAL
    stdf_type_cn*	m_kcnRTN_CHAL;		// PLR.RTN_CHAL

private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};

///////////////////////////////////////////////////////////
// RDR RECORD
///////////////////////////////////////////////////////////
class Stdf_RDR_V4: public Stdf_Record
{
    Q_DISABLE_COPY(Stdf_RDR_V4)

// METHODS
public:
    // Constructor / destructor functions
    Stdf_RDR_V4();
    ~Stdf_RDR_V4();

    // Reset record data
    void		Reset(void);
    // Return short name of the record
    QString		GetRecordShortName(void);
    // Return long name of the record
    QString		GetRecordLongName(void);
    // Return record type
    int			GetRecordType(void);
    // Read record
    bool		Read(GS::StdLib::Stdf & clStdf);
    // Write record
    bool		Write(GS::StdLib::Stdf & clStdf);
    // Construct a stringlist with all fields to display
    // Each string in the list has following format: <Field name>;<Field value>
    void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
    // Construct a ASCII string of the record
    void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);
    // Construct a XML string of the record
    void		GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection = 0);
    // Construct a ATDF string of the record
    void		GetAtdfString(QString & strAtdfString);

    // Set members and flags
    void		SetNUM_BINS(const stdf_type_u2 u2NUM_BINS)	_FIELD_SET(m_u2NUM_BINS = u2NUM_BINS, true, eposNUM_BINS)
    void		SetRTST_BIN()                               _FIELD_SET_FLAGS(m_u2NUM_BINS>0, eposRTST_BIN)
    void		SetRTST_BIN(const stdf_type_u2 u2Index, const stdf_type_u2 u2RTST_BIN)
    {
        if((m_pFieldFlags[eposNUM_BINS] & FieldFlag_Present) && (m_u2NUM_BINS > 0))
        {
            if(m_ku2RTST_BIN == NULL)
                m_ku2RTST_BIN = new stdf_type_u2[m_u2NUM_BINS];
            if(u2Index < m_u2NUM_BINS)	_FIELD_SET(m_ku2RTST_BIN[u2Index] = u2RTST_BIN, true, eposRTST_BIN)
        }
    }

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposNUM_BINS			= 0,
            eposRTST_BIN			= 1,
            eposEND					= 2,
            eposFIRST_OPTIONAL		= 2
    };

    stdf_type_u2	m_u2NUM_BINS;		// RDR.NUM_BINS
    stdf_type_u2*	m_ku2RTST_BIN;		// RDR.RTST_BIN

private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};

///////////////////////////////////////////////////////////
// SDR RECORD
///////////////////////////////////////////////////////////
class Stdf_SDR_V4: public Stdf_Record
{

// METHODS
public:
    // Constructor / destructor functions
    Stdf_SDR_V4();
    Stdf_SDR_V4(const Stdf_SDR_V4& cssv4Other);

    Stdf_SDR_V4& operator=(const Stdf_SDR_V4& cssv4Other);


    ~Stdf_SDR_V4();

    // Reset record data
    void		Reset(void);
    // Return short name of the record
    QString		GetRecordShortName(void);
    // Return long name of the record
    QString		GetRecordLongName(void);
    // Return record type
    int			GetRecordType(void);
    // Read record
    bool		Read(GS::StdLib::Stdf & clStdf);
    // Write record
    bool		Write(GS::StdLib::Stdf & clStdf);
    // Construct a stringlist with all fields to display
    // Each string in the list has following format: <Field name>;<Field value>
    void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
    // Construct a ASCII string of the record
    void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);
    // Construct a XML string of the record
    void		GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection = 0);
    // Construct a ATDF string of the record
    void		GetAtdfString(QString & strAtdfString);

    // Set members and flags
    void		SetHEAD_NUM(const stdf_type_u1 u1HEAD_NUM)  _FIELD_SET(m_u1HEAD_NUM = u1HEAD_NUM, true, eposHEAD_NUM)
    void		SetSITE_GRP(const stdf_type_u1 u1SITE_GRP)  _FIELD_SET(m_u1SITE_GRP = u1SITE_GRP, true, eposSITE_GRP)
    void		SetSITE_CNT(const stdf_type_u1 u1SITE_CNT)
    {
        if(m_u1SITE_CNT != u1SITE_CNT)      // Comment : changing site_cnt reset site_num !
        {
            if(m_ku1SITE_NUM!=NULL)
            {
                delete [] m_ku1SITE_NUM;
                m_ku1SITE_NUM = NULL;
            }
        }
        _FIELD_SET(m_u1SITE_CNT = u1SITE_CNT, true, eposSITE_CNT)

        if( (m_ku1SITE_NUM==NULL) && (m_u1SITE_CNT!=0) )
        {
            m_ku1SITE_NUM = new stdf_type_u1[m_u1SITE_CNT];
            for(int ii=0; ii<m_u1SITE_CNT; ii++)
                _FIELD_SET(m_ku1SITE_NUM[ii]=0, true, eposSITE_NUM);
        }
    }
    void		SetSITE_NUM()                           _FIELD_SET_FLAGS(m_u1SITE_CNT>0, eposSITE_NUM)
    void		SetSITE_NUM(const stdf_type_u1 u1Index, const stdf_type_u1 u1SITE_NUM)
    {
        if((m_pFieldFlags[eposSITE_CNT] & FieldFlag_Present))// && (u1Index >= 0))
        {
            if(m_ku1SITE_NUM == NULL)
                m_ku1SITE_NUM = new stdf_type_u1[m_u1SITE_CNT];
            if(u1Index < m_u1SITE_CNT)
                _FIELD_SET(m_ku1SITE_NUM[u1Index] = u1SITE_NUM, true, eposSITE_NUM)
        }
    }
    void		SetHAND_TYP(const stdf_type_cn cnHAND_TYP)	{_FIELD_SET(m_cnHAND_TYP = cnHAND_TYP, true, eposHAND_TYP);}
    void		SetHAND_ID(const stdf_type_cn cnHAND_ID)	{_FIELD_SET(m_cnHAND_ID = cnHAND_ID, true, eposHAND_ID);}
    void		SetCARD_TYP(const stdf_type_cn cnCARD_TYP)	{_FIELD_SET(m_cnCARD_TYP = cnCARD_TYP, true, eposCARD_TYP);}
    void		SetCARD_ID(const stdf_type_cn cnCARD_ID)	{_FIELD_SET(m_cnCARD_ID = cnCARD_ID, true, eposCARD_ID);}
    void		SetLOAD_TYP(const stdf_type_cn cnLOAD_TYP)	{_FIELD_SET(m_cnLOAD_TYP = cnLOAD_TYP, true, eposLOAD_TYP);}
    void		SetLOAD_ID(const stdf_type_cn cnLOAD_ID)	{_FIELD_SET(m_cnLOAD_ID = cnLOAD_ID, true, eposLOAD_ID);}
    void		SetDIB_TYP(const stdf_type_cn cnDIB_TYP)	{_FIELD_SET(m_cnDIB_TYP = cnDIB_TYP, true, eposDIB_TYP);}
    void		SetDIB_ID(const stdf_type_cn cnDIB_ID)		{_FIELD_SET(m_cnDIB_ID = cnDIB_ID, true, eposDIB_ID);}
    void		SetCABL_TYP(const stdf_type_cn cnCABL_TYP)	{_FIELD_SET(m_cnCABL_TYP = cnCABL_TYP, true, eposCABL_TYP);}
    void		SetCABL_ID(const stdf_type_cn cnCABL_ID)	{_FIELD_SET(m_cnCABL_ID = cnCABL_ID, true, eposCABL_ID);}
    void		SetCONT_TYP(const stdf_type_cn cnCONT_TYP)	{_FIELD_SET(m_cnCONT_TYP = cnCONT_TYP, true, eposCONT_TYP);}
    void		SetCONT_ID(const stdf_type_cn cnCONT_ID)	{_FIELD_SET(m_cnCONT_ID = cnCONT_ID, true, eposCONT_ID);}
    void		SetLASR_TYP(const stdf_type_cn cnLASR_TYP)	{_FIELD_SET(m_cnLASR_TYP = cnLASR_TYP, true, eposLASR_TYP);}
    void		SetLASR_ID(const stdf_type_cn cnLASR_ID)	{_FIELD_SET(m_cnLASR_ID = cnLASR_ID, true, eposLASR_ID);}
    void		SetEXTR_TYP(const stdf_type_cn cnEXTR_TYP)	{_FIELD_SET(m_cnEXTR_TYP = cnEXTR_TYP, true, eposEXTR_TYP);}
    void		SetEXTR_ID(const stdf_type_cn cnEXTR_ID)	{_FIELD_SET(m_cnEXTR_ID = cnEXTR_ID, true, eposEXTR_ID);}

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposHEAD_NUM		= 0,
            eposSITE_GRP		= 1,
            eposSITE_CNT		= 2,
            eposSITE_NUM		= 3,
            eposHAND_TYP		= 4,
            eposFIRST_OPTIONAL	= 4,
            eposHAND_ID			= 5,
            eposCARD_TYP		= 6,
            eposCARD_ID			= 7,
            eposLOAD_TYP		= 8,
            eposLOAD_ID			= 9,
            eposDIB_TYP			= 10,
            eposDIB_ID			= 11,
            eposCABL_TYP		= 12,
            eposCABL_ID			= 13,
            eposCONT_TYP		= 14,
            eposCONT_ID			= 15,
            eposLASR_TYP		= 16,
            eposLASR_ID			= 17,
            eposEXTR_TYP		= 18,
            eposEXTR_ID			= 19,
            eposEND				= 20
    };

    stdf_type_u1	m_u1HEAD_NUM;		// SDR.HEAD_NUM
    stdf_type_u1	m_u1SITE_GRP;		// SDR.SITE_GRP
    stdf_type_u1	m_u1SITE_CNT;		// SDR.SITE_CNT
    stdf_type_u1*	m_ku1SITE_NUM;		// SDR.SITE_NUM
    stdf_type_cn	m_cnHAND_TYP;		// SDR.HAND_TYP
    stdf_type_cn	m_cnHAND_ID;		// SDR.HAND_ID
    stdf_type_cn	m_cnCARD_TYP;		// SDR.CARD_TYP
    stdf_type_cn	m_cnCARD_ID;		// SDR.CARD_ID
    stdf_type_cn	m_cnLOAD_TYP;		// SDR.LOAD_TYP
    stdf_type_cn	m_cnLOAD_ID;		// SDR.LOAD_ID
    stdf_type_cn	m_cnDIB_TYP;		// SDR.DIB_TYP
    stdf_type_cn	m_cnDIB_ID;			// SDR.DIB_ID
    stdf_type_cn	m_cnCABL_TYP;		// SDR.CABL_TYP
    stdf_type_cn	m_cnCABL_ID;		// SDR.CABL_ID
    stdf_type_cn	m_cnCONT_TYP;		// SDR.CONT_TYP
    stdf_type_cn	m_cnCONT_ID;		// SDR.CONT_ID
    stdf_type_cn	m_cnLASR_TYP;		// SDR.LASR_TYP
    stdf_type_cn	m_cnLASR_ID;		// SDR.LASR_ID
    stdf_type_cn	m_cnEXTR_TYP;		// SDR.EXTR_TYP
    stdf_type_cn	m_cnEXTR_ID;		// SDR.EXTR_ID

private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};

///////////////////////////////////////////////////////////
// WIR RECORD
///////////////////////////////////////////////////////////
class Stdf_WIR_V4: public Stdf_Record
{
    Q_DISABLE_COPY(Stdf_WIR_V4)

// METHODS
public:
    // Constructor / destructor functions
    Stdf_WIR_V4();
    ~Stdf_WIR_V4();

    // Reset record data
    void		Reset(void);
    // Return short name of the record
    QString		GetRecordShortName(void);
    // Return long name of the record
    QString		GetRecordLongName(void);
    // Return record type
    int			GetRecordType(void);
    // Read record
    bool		Read(GS::StdLib::Stdf & clStdf);
    // Write record
    bool		Write(GS::StdLib::Stdf & clStdf);
    // Construct a stringlist with all fields to display
    // Each string in the list has following format: <Field name>;<Field value>
    void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
    // Construct a ASCII string of the record
    void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);
    // Construct a XML string of the record
    void		GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection = 0);
    // Construct a ATDF string of the record
    void		GetAtdfString(QString & strAtdfString);

    // Set members and flags
    void		SetHEAD_NUM(const stdf_type_u1 u1HEAD_NUM)	_FIELD_SET(m_u1HEAD_NUM = u1HEAD_NUM, true, eposHEAD_NUM)
    void		SetSITE_GRP(const stdf_type_u1 u1SITE_GRP)	_FIELD_SET(m_u1SITE_GRP = u1SITE_GRP, true, eposSITE_GRP)
    void		SetSTART_T(const stdf_type_u4 u4START_T)	_FIELD_SET(m_u4START_T = u4START_T, true, eposSTART_T)
    void		SetWAFER_ID(const stdf_type_cn cnWAFER_ID)	_FIELD_SET(m_cnWAFER_ID = cnWAFER_ID, true, eposWAFER_ID)

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposHEAD_NUM			= 0,		// WIR.HEAD_NUM
            eposSITE_GRP			= 1,		// WIR.SITE_GRP
            eposSTART_T				= 2,		// WIR.START_T
            eposWAFER_ID			= 3,		// WIR.WAFER_ID
            eposFIRST_OPTIONAL		= 3,
            eposEND					= 4
    };

    stdf_type_u1	m_u1HEAD_NUM;		// WIR.HEAD_NUM
    stdf_type_u1	m_u1SITE_GRP;		// WIR.SITE_GRP
    stdf_type_u4	m_u4START_T;		// WIR.START_T
    stdf_type_cn	m_cnWAFER_ID;		// WIR.WAFER_ID

private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};

///////////////////////////////////////////////////////////
// WRR RECORD
///////////////////////////////////////////////////////////
class Stdf_WRR_V4: public Stdf_Record
{
// METHODS
public:
    // Constructor / destructor functions
    Stdf_WRR_V4();
    Stdf_WRR_V4(const Stdf_WRR_V4 & other);
    ~Stdf_WRR_V4();

    Stdf_WRR_V4& operator=(const Stdf_WRR_V4& other);

    // Reset record data
    void		Reset(void);
    // Return short name of the record
    QString		GetRecordShortName(void);
    // Return long name of the record
    QString		GetRecordLongName(void);
    // Return record type
    int			GetRecordType(void);
    // Read record
    bool		Read(GS::StdLib::Stdf & clStdf);
    // Write record
    bool		Write(GS::StdLib::Stdf & clStdf);
    // Construct a stringlist with all fields to display
    // Each string in the list has following format: <Field name>;<Field value>
    void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
    // Construct a ASCII string of the record
    void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);
    // Construct a XML string of the record
    void		GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection = 0);
    // Construct a ATDF string of the record
    void		GetAtdfString(QString & strAtdfString);

    // Set members and flags
    void		SetHEAD_NUM(const stdf_type_u1 u1HEAD_NUM)	_FIELD_SET(m_u1HEAD_NUM = u1HEAD_NUM, true, eposHEAD_NUM)
    void		SetSITE_GRP(const stdf_type_u1 u1SITE_GRP)	_FIELD_SET(m_u1SITE_GRP = u1SITE_GRP, true, eposSITE_GRP)
    void		SetFINISH_T(const stdf_type_u4 u4FINISH_T)	_FIELD_SET(m_u4FINISH_T = u4FINISH_T, true, eposFINISH_T)
    void		SetPART_CNT(const stdf_type_u4 u4PART_CNT)	_FIELD_SET(m_u4PART_CNT = u4PART_CNT, true, eposPART_CNT)
    void		SetRTST_CNT(const stdf_type_u4 u4RTST_CNT)	_FIELD_SET(m_u4RTST_CNT = u4RTST_CNT, true, eposRTST_CNT)
    void		SetABRT_CNT(const stdf_type_u4 u4ABRT_CNT)	_FIELD_SET(m_u4ABRT_CNT = u4ABRT_CNT, true, eposABRT_CNT)
    void		SetGOOD_CNT(const stdf_type_u4 u4GOOD_CNT)	_FIELD_SET(m_u4GOOD_CNT = u4GOOD_CNT, true, eposGOOD_CNT)
    void		SetFUNC_CNT(const stdf_type_u4 u4FUNC_CNT)	_FIELD_SET(m_u4FUNC_CNT = u4FUNC_CNT, true, eposFUNC_CNT)
    void		SetWAFER_ID(const stdf_type_cn cnWAFER_ID)	_FIELD_SET(m_cnWAFER_ID = cnWAFER_ID, true, eposWAFER_ID)
    void		SetFABWF_ID(const stdf_type_cn cnFABWF_ID)	_FIELD_SET(m_cnFABWF_ID = cnFABWF_ID, true, eposFABWF_ID)
    void		SetFRAME_ID(const stdf_type_cn cnFRAME_ID)	_FIELD_SET(m_cnFRAME_ID = cnFRAME_ID, true, eposFRAME_ID)
    void		SetMASK_ID(const stdf_type_cn cnMASK_ID)	_FIELD_SET(m_cnMASK_ID = cnMASK_ID, true, eposMASK_ID)
    void		SetUSR_DESC(const stdf_type_cn cnUSR_DESC)	_FIELD_SET(m_cnUSR_DESC = cnUSR_DESC, true, eposUSR_DESC)
    void		SetEXC_DESC(const stdf_type_cn cnEXC_DESC)	_FIELD_SET(m_cnEXC_DESC = cnEXC_DESC, true, eposEXC_DESC)

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid
    void        invalidateField(int nFieldPos);

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposHEAD_NUM			= 0,		// WRR.HEAD_NUM
            eposSITE_GRP			= 1,		// WRR.SITE_GRP
            eposFINISH_T			= 2,		// WRR.FINISH_T
            eposPART_CNT			= 3,		// WRR.PART_CNT
            eposRTST_CNT			= 4,		// WRR.RTST_CNT
            eposFIRST_OPTIONAL		= 4,
            eposABRT_CNT			= 5,		// WRR.ABRT_CNT
            eposGOOD_CNT			= 6,		// WRR.GOOD_CNT
            eposFUNC_CNT			= 7,		// WRR.FUNC_CNT
            eposWAFER_ID			= 8,		// WRR.WAFER_ID
            eposFABWF_ID			= 9,		// WRR.FABWF_ID
            eposFRAME_ID			= 10,		// WRR.FRAME_ID
            eposMASK_ID				= 11,		// WRR.MASK_ID
            eposUSR_DESC			= 12,		// WRR.USR_DESC
            eposEXC_DESC			= 13,		// WRR.EXC_DESC
            eposEND					= 14
    };

    stdf_type_u1	m_u1HEAD_NUM;		// WRR.HEAD_NUM
    stdf_type_u1	m_u1SITE_GRP;		// WRR.SITE_GRP
    stdf_type_u4	m_u4FINISH_T;		// WRR.FINISH_T
    stdf_type_u4	m_u4PART_CNT;		// WRR.PART_CNT
    stdf_type_u4	m_u4RTST_CNT;		// WRR.RTST_CNT
    stdf_type_u4	m_u4ABRT_CNT;		// WRR.ABRT_CNT
    stdf_type_u4	m_u4GOOD_CNT;		// WRR.GOOD_CNT
    stdf_type_u4	m_u4FUNC_CNT;		// WRR.FUNC_CNT
    stdf_type_cn	m_cnWAFER_ID;		// WRR.WAFER_ID
    stdf_type_cn	m_cnFABWF_ID;		// WRR.FABWF_ID
    stdf_type_cn	m_cnFRAME_ID;		// WRR.FRAME_ID
    stdf_type_cn	m_cnMASK_ID;		// WRR.MASK_ID
    stdf_type_cn	m_cnUSR_DESC;		// WRR.USR_DESC
    stdf_type_cn	m_cnEXC_DESC;		// WRR.EXC_DESC

private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};

///////////////////////////////////////////////////////////
// WCR RECORD
///////////////////////////////////////////////////////////
class Stdf_WCR_V4: public Stdf_Record
{
    Q_DISABLE_COPY(Stdf_WCR_V4)

// METHODS
public:
    // Constructor / destructor functions
    Stdf_WCR_V4();
    ~Stdf_WCR_V4();

    // Reset record data
    void		Reset(void);
    // Return short name of the record
    QString		GetRecordShortName(void);
    // Return long name of the record
    QString		GetRecordLongName(void);
    // Return record type
    int			GetRecordType(void);
    // Read record
    bool		Read(GS::StdLib::Stdf & clStdf);
    // Write record
    bool		Write(GS::StdLib::Stdf & clStdf);
    // Construct a stringlist with all fields to display
    // Each string in the list has following format: <Field name>;<Field value>
    void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
    // Construct a ASCII string of the record
    void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);
    // Construct a XML string of the record
    void		GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection = 0);
    // Construct a ATDF string of the record
    void		GetAtdfString(QString & strAtdfString);

    // Set members and flags
    void		SetWAFR_SIZ(const stdf_type_r4 r4WAFR_SIZ)	_FIELD_SET(m_r4WAFR_SIZ = r4WAFR_SIZ, true, eposWAFR_SIZ)
    void		SetDIE_HT(const stdf_type_r4 r4DIE_HT)		_FIELD_SET(m_r4DIE_HT = r4DIE_HT, true, eposDIE_HT)
    void		SetDIE_WID(const stdf_type_r4 r4DIE_WID)	_FIELD_SET(m_r4DIE_WID = r4DIE_WID, true, eposDIE_WID)
    void		SetWF_UNITS(const stdf_type_u1 u1WF_UNITS)	_FIELD_SET(m_u1WF_UNITS = u1WF_UNITS, true, eposWF_UNITS)
    void		SetWF_FLAT(const stdf_type_c1 c1WF_FLAT)	_FIELD_SET(m_c1WF_FLAT = c1WF_FLAT, true, eposWF_FLAT)
    void		SetCENTER_X(const stdf_type_i2 i2CENTER_X)	_FIELD_SET(m_i2CENTER_X = i2CENTER_X, true, eposCENTER_X)
    void		SetCENTER_Y(const stdf_type_i2 i2CENTER_Y)	_FIELD_SET(m_i2CENTER_Y = i2CENTER_Y, true, eposCENTER_Y)
    void		SetPOS_X(const stdf_type_c1 c1POS_X)		_FIELD_SET(m_c1POS_X = c1POS_X, true, eposPOS_X)
    void		SetPOS_Y(const stdf_type_c1 c1POS_Y)		_FIELD_SET(m_c1POS_Y = c1POS_Y, true, eposPOS_Y)

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposWAFR_SIZ			= 0,		// WCR.WAFR_SIZ
            eposFIRST_OPTIONAL		= 0,
            eposDIE_HT				= 1,		// WCR.DIE_HT
            eposDIE_WID				= 2,		// WCR.DIE_WID
            eposWF_UNITS			= 3,		// WCR.WF_UNITS
            eposWF_FLAT				= 4,		// WCR.WF_FLAT
            eposCENTER_X			= 5,		// WCR.CENTER_X
            eposCENTER_Y			= 6,		// WCR.CENTER_Y
            eposPOS_X				= 7,		// WCR.POS_X
            eposPOS_Y				= 8,		// WCR.POS_Y
            eposEND					= 9
    };

    stdf_type_r4	m_r4WAFR_SIZ;		// WCR.WAFR_SIZ
    stdf_type_r4	m_r4DIE_HT;			// WCR.DIE_HT
    stdf_type_r4	m_r4DIE_WID;		// WCR.DIE_WID
    stdf_type_u1	m_u1WF_UNITS;		// WCR.WF_UNITS
    stdf_type_c1	m_c1WF_FLAT;		// WCR.WF_FLAT
    stdf_type_i2	m_i2CENTER_X;		// WCR.CENTER_X
    stdf_type_i2	m_i2CENTER_Y;		// WCR.CENTER_Y
    stdf_type_c1	m_c1POS_X;			// WCR.POS_X
    stdf_type_c1	m_c1POS_Y;			// WCR.POS_Y

private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};

///////////////////////////////////////////////////////////
// PIR RECORD
///////////////////////////////////////////////////////////
class Stdf_PIR_V4: public Stdf_Record
{
// METHODS
public:
    // Constructor / destructor functions
    Stdf_PIR_V4();
    Stdf_PIR_V4(const Stdf_PIR_V4& other);
    ~Stdf_PIR_V4();

    Stdf_PIR_V4& operator=(const Stdf_PIR_V4& other);

    // Reset record data
    void		Reset(void);
    // Return short name of the record
    QString		GetRecordShortName(void);
    // Return long name of the record
    QString		GetRecordLongName(void);
    // Return record type
    int			GetRecordType(void);
    // Read record
    bool		Read(GS::StdLib::Stdf & clStdf);
    // Write record
    bool		Write(GS::StdLib::Stdf & clStdf);
    // Construct a stringlist with all fields to display
    // Each string in the list has following format: <Field name>;<Field value>
    void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
    // Construct a ASCII string of the record
    void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);
    // Construct a XML string of the record
    void		GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection = 0);
    // Construct a ATDF string of the record
    void		GetAtdfString(QString & strAtdfString);

    // Set members and flags
    void		SetHEAD_NUM(const stdf_type_u1 u1HEAD_NUM)	_FIELD_SET(m_u1HEAD_NUM = u1HEAD_NUM, true, eposHEAD_NUM)
    void		SetSITE_NUM(const stdf_type_u1 u1SITE_NUM)	_FIELD_SET(m_u1SITE_NUM = u1SITE_NUM, true, eposSITE_NUM)

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposHEAD_NUM			= 0,	// PIR.HEAD_NUM
            eposSITE_NUM			= 1,	// PIR.SITE_NUM
            eposEND					= 2,
            eposFIRST_OPTIONAL		= 2
    };

    stdf_type_u1	m_u1HEAD_NUM;		// PIR.HEAD_NUM
    stdf_type_u1	m_u1SITE_NUM;		// PIR.SITE_NUM

private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};

///////////////////////////////////////////////////////////
// PRR RECORD
///////////////////////////////////////////////////////////
class Stdf_PRR_V4: public Stdf_Record
{
// METHODS
public:
    // Constructor / destructor functions
    Stdf_PRR_V4();
    Stdf_PRR_V4(const Stdf_PRR_V4& other);
    ~Stdf_PRR_V4();

    Stdf_PRR_V4& operator=(const Stdf_PRR_V4& other);

    enum EPART_FLG {	eREPEAT_PARTID = 0x1,	// See complete flags definition in STDF specification
                        eREPEAT_COORD  = 0x2,	// version 4, page 35
                        eABNORMAL_END  = 0x4,
                        ePART_FAILED   = 0x8,
                        eNO_PASSFAIL   = 0x10
    };

    // Reset record data
    void		Reset(void);
    // Return short name of the record
    QString		GetRecordShortName(void);
    // Return long name of the record
    QString		GetRecordLongName(void);
    // Return record type
    int			GetRecordType(void);
    // Read record
    bool		Read(GS::StdLib::Stdf & clStdf);
    // Write record
    bool		Write(GS::StdLib::Stdf & clStdf);
    // Construct a stringlist with all fields to display
    // Each string in the list has following format: <Field name>;<Field value>
    void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
    // Construct a ASCII string of the record
    void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);
    // Construct a XML string of the record
    void		GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection = 0);
    // Construct a ATDF string of the record
    void		GetAtdfString(QString & strAtdfString);

    // Set members and flags
    void		SetHEAD_NUM(const stdf_type_u1 u1HEAD_NUM)	_FIELD_SET(m_u1HEAD_NUM = u1HEAD_NUM, true, eposHEAD_NUM)
    void		SetSITE_NUM(const stdf_type_u1 u1SITE_NUM)	_FIELD_SET(m_u1SITE_NUM = u1SITE_NUM, true, eposSITE_NUM)
    void		SetPART_FLG(const stdf_type_b1 b1PART_FLG)	_FIELD_SET(m_b1PART_FLG = b1PART_FLG, true, eposPART_FLG)
    void		SetNUM_TEST(const stdf_type_u2 u2NUM_TEST)	_FIELD_SET(m_u2NUM_TEST = u2NUM_TEST, true, eposNUM_TEST)
    void		SetHARD_BIN(const stdf_type_u2 u2HARD_BIN)	_FIELD_SET(m_u2HARD_BIN = u2HARD_BIN, true, eposHARD_BIN)
    void		SetSOFT_BIN(const stdf_type_u2 u2SOFT_BIN)	_FIELD_SET(m_u2SOFT_BIN = u2SOFT_BIN, true, eposSOFT_BIN)
    void		SetX_COORD(const stdf_type_i2 i2X_COORD)	_FIELD_SET(m_i2X_COORD = i2X_COORD, true, eposX_COORD)
    void		SetY_COORD(const stdf_type_i2 i2Y_COORD)	_FIELD_SET(m_i2Y_COORD = i2Y_COORD, true, eposY_COORD)
    void		SetTEST_T(const stdf_type_u4 u4TEST_T)		_FIELD_SET(m_u4TEST_T = u4TEST_T, true, eposTEST_T)
    void		SetPART_ID(const stdf_type_cn cnPART_ID)	_FIELD_SET(m_cnPART_ID = cnPART_ID, true, eposPART_ID)
    void		SetPART_TXT(const stdf_type_cn cnPART_TXT)	_FIELD_SET(m_cnPART_TXT = cnPART_TXT, true, eposPART_TXT)
    void		SetPART_FIX()                               _FIELD_SET_FLAGS(m_bnPART_FIX.m_bLength > 0, eposPART_FIX)

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposHEAD_NUM			= 0,	// PRR.HEAD_NUM
            eposSITE_NUM			= 1,	// PRR.SITE_NUM
            eposPART_FLG			= 2,	// PRR.PART_FLG
            eposNUM_TEST			= 3,	// PRR.NUM_TEST
            eposHARD_BIN			= 4,	// PRR.HARD_BIN
            eposSOFT_BIN			= 5,	// PRR.SOFT_BIN
            eposFIRST_OPTIONAL		= 5,
            eposX_COORD				= 6,	// PRR.X_COORD
            eposY_COORD				= 7,	// PRR.Y_COORD
            eposTEST_T				= 8,	// PRR.TEST_T
            eposPART_ID				= 9,	// PRR.PART_ID
            eposPART_TXT			= 10,	// PRR.PART_TXT
            eposPART_FIX			= 11,	// PRR.PART_FIX
            eposEND					= 12
    };

    stdf_type_u1	m_u1HEAD_NUM;		// PRR.HEAD_NUM
    stdf_type_u1	m_u1SITE_NUM;		// PRR.SITE_NUM
    stdf_type_b1	m_b1PART_FLG;		// PRR.PART_FLG
    stdf_type_u2	m_u2NUM_TEST;		// PRR.NUM_TEST
    stdf_type_u2	m_u2HARD_BIN;		// PRR.HARD_BIN
    stdf_type_u2	m_u2SOFT_BIN;		// PRR.SOFT_BIN
    stdf_type_i2	m_i2X_COORD;		// PRR.X_COORD
    stdf_type_i2	m_i2Y_COORD;		// PRR.Y_COORD
    stdf_type_u4	m_u4TEST_T;			// PRR.TEST_T
    stdf_type_cn	m_cnPART_ID;		// PRR.PART_ID
    stdf_type_cn	m_cnPART_TXT;		// PRR.PART_TXT
    stdf_type_bn	m_bnPART_FIX;		// PRR.PART_FIX

private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};

///////////////////////////////////////////////////////////
// TSR RECORD
///////////////////////////////////////////////////////////
class Stdf_TSR_V4: public Stdf_Record
{
    Q_DISABLE_COPY(Stdf_TSR_V4)

// METHODS
public:
    // Constructor / destructor functions
    Stdf_TSR_V4();
    ~Stdf_TSR_V4();

    // Reset record data
    void		Reset(void);
    // Return short name of the record
    QString		GetRecordShortName(void);
    // Return long name of the record
    QString		GetRecordLongName(void);
    // Return record type
    int			GetRecordType(void);
    // Read record
    bool		Read(GS::StdLib::Stdf & clStdf);
    // Write record
    bool		Write(GS::StdLib::Stdf & clStdf);
    // Construct a stringlist with all fields to display
    // Each string in the list has following format: <Field name>;<Field value>
    void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
    // Construct a ASCII string of the record
    void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);
    // Construct a XML string of the record
    void		GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection = 0);
    // Construct a ATDF string of the record
    void		GetAtdfString(QString & strAtdfString);

    // Set members and flags
    void		SetHEAD_NUM(const stdf_type_u1 u1HEAD_NUM)	_FIELD_SET(m_u1HEAD_NUM = u1HEAD_NUM, true, eposHEAD_NUM)
    void		SetSITE_NUM(const stdf_type_u1 u1SITE_NUM)	_FIELD_SET(m_u1SITE_NUM = u1SITE_NUM, true, eposSITE_NUM)
    void		SetTEST_TYP(const stdf_type_c1 c1TEST_TYP)	_FIELD_SET(m_c1TEST_TYP = c1TEST_TYP, true, eposTEST_TYP)
    void		SetTEST_NUM(const stdf_type_u4 u4TEST_NUM)	_FIELD_SET(m_u4TEST_NUM = u4TEST_NUM, true, eposTEST_NUM)
    void		SetEXEC_CNT(const stdf_type_u4 u4EXEC_CNT)	_FIELD_SET(m_u4EXEC_CNT = u4EXEC_CNT, true, eposEXEC_CNT)
    void		SetFAIL_CNT(const stdf_type_u4 u4FAIL_CNT)	_FIELD_SET(m_u4FAIL_CNT = u4FAIL_CNT, true, eposFAIL_CNT)
    void		SetALRM_CNT(const stdf_type_u4 u4ALRM_CNT)	_FIELD_SET(m_u4ALRM_CNT = u4ALRM_CNT, true, eposALRM_CNT)
    void		SetTEST_NAM(const stdf_type_cn cnTEST_NAM)	_FIELD_SET(m_cnTEST_NAM = cnTEST_NAM, true, eposTEST_NAM)
    void		SetSEQ_NAME(const stdf_type_cn cnSEQ_NAME)	_FIELD_SET(m_cnSEQ_NAME = cnSEQ_NAME, true, eposSEQ_NAME)
    void		SetTEST_LBL(const stdf_type_cn cnTEST_LBL)	_FIELD_SET(m_cnTEST_LBL = cnTEST_LBL, true, eposTEST_LBL)
    void		SetOPT_FLAG(const stdf_type_b1 b1OPT_FLAG)	_FIELD_SET(m_b1OPT_FLAG = b1OPT_FLAG, true, eposOPT_FLAG)
    void		SetTEST_TIM(const stdf_type_r4 r4TEST_TIM)	_FIELD_SET(m_r4TEST_TIM = r4TEST_TIM, true, eposTEST_TIM)
    void		SetTEST_MIN(const stdf_type_r4 r4TEST_MIN)	_FIELD_SET(m_r4TEST_MIN = r4TEST_MIN, true, eposTEST_MIN)
    void		SetTEST_MAX(const stdf_type_r4 r4TEST_MAX)	_FIELD_SET(m_r4TEST_MAX = r4TEST_MAX, true, eposTEST_MAX)
    void		SetTST_SUMS(const stdf_type_r4 r4TST_SUMS)	_FIELD_SET(m_r4TST_SUMS = r4TST_SUMS, true, eposTST_SUMS)
    void		SetTST_SQRS(const stdf_type_r4 r4TST_SQRS)	_FIELD_SET(m_r4TST_SQRS = r4TST_SQRS, true, eposTST_SQRS)

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposHEAD_NUM			= 0,	// TSR.HEAD_NUM
            eposSITE_NUM			= 1,	// TSR.SITE_NUM
            eposTEST_TYP			= 2,	// TSR.TEST_TYP
            eposTEST_NUM			= 3,	// TSR.TEST_NUM
            eposEXEC_CNT			= 4,	// TSR.EXEC_CNT
            eposFIRST_OPTIONAL		= 4,
            eposFAIL_CNT			= 5,	// TSR.FAIL_CNT
            eposALRM_CNT			= 6,	// TSR.ALRM_CNT
            eposTEST_NAM			= 7,	// TSR.TEST_NAM
            eposSEQ_NAME			= 8,	// TSR.SEQ_NAME
            eposTEST_LBL			= 9,	// TSR.TEST_LBL
            eposOPT_FLAG			= 10,	// TSR.OPT_FLAG
            eposTEST_TIM			= 11,	// TSR.TEST_TIM
            eposTEST_MIN			= 12,	// TSR.TEST_MIN
            eposTEST_MAX			= 13,	// TSR.TEST_MAX
            eposTST_SUMS			= 14,	// TSR.TST_SUMS
            eposTST_SQRS			= 15,	// TSR.TST_SQRS
            eposEND					= 16
    };

    stdf_type_u1	m_u1HEAD_NUM;		// TSR.HEAD_NUM
    stdf_type_u1	m_u1SITE_NUM;		// TSR.SITE_NUM
    stdf_type_c1	m_c1TEST_TYP;		// TSR.TEST_TYP
    stdf_type_u4	m_u4TEST_NUM;		// TSR.TEST_NUM
    stdf_type_u4	m_u4EXEC_CNT;		// TSR.EXEC_CNT
    stdf_type_u4	m_u4FAIL_CNT;		// TSR.FAIL_CNT
    stdf_type_u4	m_u4ALRM_CNT;		// TSR.ALRM_CNT
    stdf_type_cn	m_cnTEST_NAM;		// TSR.TEST_NAM
    stdf_type_cn	m_cnSEQ_NAME;		// TSR.SEQ_NAME
    stdf_type_cn	m_cnTEST_LBL;		// TSR.TEST_LBL
    stdf_type_b1	m_b1OPT_FLAG;		// TSR.OPT_FLAG
    stdf_type_r4	m_r4TEST_TIM;		// TSR.TEST_TIM
    stdf_type_r4	m_r4TEST_MIN;		// TSR.TEST_MIN
    stdf_type_r4	m_r4TEST_MAX;		// TSR.TEST_MAX
    stdf_type_r4	m_r4TST_SUMS;		// TSR.TST_SUMS
    stdf_type_r4	m_r4TST_SQRS;		// TSR.TST_SQRS

private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};

///////////////////////////////////////////////////////////
// PTR RECORD
///////////////////////////////////////////////////////////
class Stdf_PTR_V4: public Stdf_Record
{
// METHODS
public:

    // Constructor / destructor functions
    Stdf_PTR_V4();
    Stdf_PTR_V4(const Stdf_PTR_V4 &other);
    ~Stdf_PTR_V4();

    Stdf_PTR_V4& operator=(const Stdf_PTR_V4& other);

    enum ETEST_FLG	{	eALARM				= 0x1,	// See complete flags definition in STDF specification
                        eNOTVALID_RESULT	= 0x2,	// version 4, page 39
                        eNOTRELIABLE_RESULT	= 0x4,
                        eTIMEOUT			= 0x8,
                        eTEST_NOTEXECUTED	= 0x10,
                        eTEST_ABORTED		= 0x20,
                        eNO_PASSFAIL		= 0x40,
                        eTEST_FAILED		= 0x80
    };

    enum EPARM_FLG	{	eSCALE_ERROR		= 0x1,	// See complete flags definition in STDF specification
                        eDRIFT_ERROR		= 0x2,	// version 4, page 40
                        eOSCILLAT_DETECTED	= 0x4,
                        eOVER_HIGHLIMIT		= 0x8,
                        eUNDER_LOWLIMIT		= 0x10,
                        ePASS_ALTERNATE_LIM	= 0x20,
                        eLOWLIM_NOTSTRICT	= 0x40,
                        eHIGHLIM_NOTSTRICT	= 0x80
    };

    enum EOPT_FLAG	{	eNOTVALID_RES_SCAL	= 0x1,	// See complete flags definition in STDF specification
                        eRESERVED_BIT1		= 0x2,	// version 4, page 40
                        eNO_LOWSPEC_LIMIT	= 0x4,
                        eNO_HIGHSPEC_LIMIT	= 0x8,
                        eNO_LL_USEFIRSTPTR	= 0x10,
                        eNO_HL_USEFIRSTPTR	= 0x20,
                        eNO_LL				= 0x40,
                        eNO_HL				= 0x80,
                        eOPT_FLAG_ALL		= 0xFF
    };

    // Reset record data
    void		Reset(void);
    // Return short name of the record
    QString		GetRecordShortName(void);
    // Return long name of the record
    QString		GetRecordLongName(void);
    // Return record type
    int			GetRecordType(void);
    // Read record
    bool		Read(GS::StdLib::Stdf & clStdf);
    // Write record
    bool		Write(GS::StdLib::Stdf & clStdf);
    // Construct a stringlist with all fields to display
    // Each string in the list has following format: <Field name>;<Field value>
    void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
    // Construct a ASCII string of the record
    void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);
    // Construct a XML string of the record
    void		GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection = 0);
    // Construct a ATDF string of the record
    void		GetAtdfString(QString & strAtdfString);

    // Set members and flags
    void		SetTEST_NUM(const stdf_type_u4 u4TEST_NUM)	_FIELD_SET(m_u4TEST_NUM = u4TEST_NUM, true, eposTEST_NUM)
    void		SetHEAD_NUM(const stdf_type_u1 u1HEAD_NUM)	_FIELD_SET(m_u1HEAD_NUM = u1HEAD_NUM, true, eposHEAD_NUM)
    void		SetSITE_NUM(const stdf_type_u1 u1SITE_NUM)	_FIELD_SET(m_u1SITE_NUM = u1SITE_NUM, true, eposSITE_NUM)
    void		SetTEST_FLG(const stdf_type_b1 b1TEST_FLG)	_FIELD_SET(m_b1TEST_FLG = b1TEST_FLG, true, eposTEST_FLG)
    void		SetPARM_FLG(const stdf_type_b1 b1PARM_FLG)	_FIELD_SET(m_b1PARM_FLG = b1PARM_FLG, true, eposPARM_FLG)
    void		SetRESULT(const stdf_type_r4 r4RESULT)		_FIELD_SET(m_r4RESULT = r4RESULT, true, eposRESULT)
    void		SetTEST_TXT(const stdf_type_cn cnTEST_TXT)	_FIELD_SET(m_cnTEST_TXT = cnTEST_TXT, true, eposTEST_TXT)
    void		SetALARM_ID(const stdf_type_cn cnALARM_ID)	_FIELD_SET(m_cnALARM_ID = cnALARM_ID, true, eposALARM_ID)
    void		SetOPT_FLAG(const stdf_type_b1 b1OPT_FLAG)	_FIELD_SET(m_b1OPT_FLAG = b1OPT_FLAG, true, eposOPT_FLAG)
    void		SetRES_SCAL(const stdf_type_i1 i1RES_SCAL)	_FIELD_SET(m_i1RES_SCAL = i1RES_SCAL, true, eposRES_SCAL)
    void		SetLLM_SCAL(const stdf_type_i1 i1LLM_SCAL)	_FIELD_SET(m_i1LLM_SCAL = i1LLM_SCAL, true, eposLLM_SCAL)
    void		SetHLM_SCAL(const stdf_type_i1 i1HLM_SCAL)	_FIELD_SET(m_i1HLM_SCAL = i1HLM_SCAL, true, eposHLM_SCAL)
    void		SetLO_LIMIT(const stdf_type_r4 r4LO_LIMIT)	_FIELD_SET(m_r4LO_LIMIT = r4LO_LIMIT, true, eposLO_LIMIT)
    void		SetHI_LIMIT(const stdf_type_r4 r4HI_LIMIT)	_FIELD_SET(m_r4HI_LIMIT = r4HI_LIMIT, true, eposHI_LIMIT)
    void		SetUNITS(const stdf_type_cn cnUNITS)		_FIELD_SET(m_cnUNITS = cnUNITS, true, eposUNITS)
    void		SetC_RESFMT(const stdf_type_cn cnC_RESFMT)	_FIELD_SET(m_cnC_RESFMT = cnC_RESFMT, true, eposC_RESFMT)
    void		SetC_LLMFMT(const stdf_type_cn cnC_LLMFMT)	_FIELD_SET(m_cnC_LLMFMT = cnC_LLMFMT, true, eposC_LLMFMT)
    void		SetC_HLMFMT(const stdf_type_cn cnC_HLMFMT)	_FIELD_SET(m_cnC_HLMFMT = cnC_HLMFMT, true, eposC_HLMFMT)
    void		SetLO_SPEC(const stdf_type_r4 r4LO_SPEC)	_FIELD_SET(m_r4LO_SPEC = r4LO_SPEC, true, eposLO_SPEC)
    void		SetHI_SPEC(const stdf_type_r4 r4HI_SPEC)	_FIELD_SET(m_r4HI_SPEC = r4HI_SPEC, true, eposHI_SPEC)

    // Some test related functions
    bool		IsTestExecuted();								// Return true if test has been executed
    bool		IsTestFail();                                   // Return true if test is FAIL
    bool		IsTestFail(Stdf_PTR_V4 & clRefPTR);			// Return true if test is FAIL
    void		UpdatePassFailInfo(Stdf_PTR_V4 & clRefPTR);	// Update Pass/Fail flags in PTR according to limits from the reference PTR

    // Check field validity
    bool		IsFieldValid(int nFieldPos) const;				// Return true if field is valid

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposTEST_NUM			= 0,	// PTR.TEST_NUM
            eposHEAD_NUM			= 1,	// PTR.HEAD_NUM
            eposSITE_NUM			= 2,	// PTR.SITE_NUM
            eposTEST_FLG			= 3,	// PTR.TEST_FLG
            eposPARM_FLG			= 4,	// PTR.PARM_FLG
            eposRESULT				= 5,	// PTR.RESULT
            eposFIRST_OPTIONAL		= 5,
            eposTEST_TXT			= 6,	// PTR.TEST_TXT
            eposALARM_ID			= 7,	// PTR.ALARM_ID
            eposOPT_FLAG			= 8,	// PTR.OPT_FLAG
            eposRES_SCAL			= 9,	// PTR.RES_SCAL
            eposLLM_SCAL			= 10,	// PTR.LLM_SCAL
            eposHLM_SCAL			= 11,	// PTR.HLM_SCAL
            eposLO_LIMIT			= 12,	// PTR.LO_LIMIT
            eposHI_LIMIT			= 13,	// PTR.HI_LIMIT
            eposUNITS				= 14,	// PTR.UNITS
            eposC_RESFMT			= 15,	// PTR.C_RESFMT
            eposC_LLMFMT			= 16,	// PTR.C_LLMFMT
            eposC_HLMFMT			= 17,	// PTR.C_HLMFMT
            eposLO_SPEC				= 18,	// PTR.LO_SPEC
            eposHI_SPEC				= 19,	// PTR.HI_SPEC
            eposEND					= 20
    };

    stdf_type_u4	m_u4TEST_NUM;		// PTR.TEST_NUM
    stdf_type_u1	m_u1HEAD_NUM;		// PTR.HEAD_NUM
    stdf_type_u1	m_u1SITE_NUM;		// PTR.SITE_NUM
    stdf_type_b1	m_b1TEST_FLG;		// PTR.TEST_FLG
    stdf_type_b1	m_b1PARM_FLG;		// PTR.PARM_FLG
    stdf_type_r4	m_r4RESULT;			// PTR.RESULT
    bool			m_bRESULT_IsNAN;	// Set to true if PTR.RESULT is a NAN value
    stdf_type_cn	m_cnTEST_TXT;		// PTR.TEST_TXT
    stdf_type_cn	m_cnALARM_ID;		// PTR.ALARM_ID
    stdf_type_b1	m_b1OPT_FLAG;		// PTR.OPT_FLAG
    stdf_type_i1	m_i1RES_SCAL;		// PTR.RES_SCAL
    stdf_type_i1	m_i1LLM_SCAL;		// PTR.LLM_SCAL
    stdf_type_i1	m_i1HLM_SCAL;		// PTR.HLM_SCAL
    stdf_type_r4	m_r4LO_LIMIT;		// PTR.LO_LIMIT
    stdf_type_r4	m_r4HI_LIMIT;		// PTR.HI_LIMIT
    stdf_type_cn	m_cnUNITS;			// PTR.UNITS
    stdf_type_cn	m_cnC_RESFMT;		// PTR.C_RESFMT
    stdf_type_cn	m_cnC_LLMFMT;		// PTR.C_LLMFMT
    stdf_type_cn	m_cnC_HLMFMT;		// PTR.C_HLMFMT
    stdf_type_r4	m_r4LO_SPEC;		// PTR.LO_SPEC
    stdf_type_r4	m_r4HI_SPEC;		// PTR.HI_SPEC

private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};

///////////////////////////////////////////////////////////
// MPR RECORD
///////////////////////////////////////////////////////////
class Stdf_MPR_V4: public Stdf_Record
{

// METHODS
public:
    // Constructor / destructor functions
    Stdf_MPR_V4();
    Stdf_MPR_V4(const Stdf_MPR_V4& other);
    ~Stdf_MPR_V4();

    Stdf_MPR_V4&    operator=(const Stdf_MPR_V4& other);

    enum ETEST_FLG	{	eALARM				= 0x1,	// See complete flags definition in STDF specification
                        eRESERVED_BIT1		= 0x2,	// version 4, page 44
                        eNOTRELIABLE_RESULT	= 0x4,
                        eTIMEOUT			= 0x8,
                        eTEST_NOTEXECUTED	= 0x10,
                        eTEST_ABORTED		= 0x20,
                        eNO_PASSFAIL		= 0x40,
                        eTEST_FAILED		= 0x80
    };

    enum EPARM_FLG	{	eSCALE_ERROR		= 0x1,	// See complete flags definition in STDF specification
                        eDRIFT_ERROR		= 0x2,	// version 4, page 44
                        eOSCILLAT_DETECTED	= 0x4,
                        eOVER_HIGHLIMIT		= 0x8,
                        eUNDER_LOWLIMIT		= 0x10,
                        ePASS_ALTERNATE_LIM	= 0x20,
                        eLOWLIM_NOTSTRICT	= 0x40,
                        eHIGHLIM_NOTSTRICT	= 0x80
    };

    enum EOPT_FLAG	{	eNOTVALID_RES_SCAL	= 0x1,	// See complete flags definition in STDF specification
                        eNOTVALID_START_INCR= 0x2,	// version 4, page 45
                        eNO_LOWSPEC_LIMIT	= 0x4,
                        eNO_HIGHSPEC_LIMIT	= 0x8,
                        eNO_LL_USEFIRSTMPR	= 0x10,
                        eNO_HL_USEFIRSTMPR	= 0x20,
                        eNO_LL				= 0x40,
                        eNO_HL				= 0x80,
                        eOPT_FLAG_ALL		= 0xFF
    };
    // Reset record data
    void		Reset(void);
    // Return short name of the record
    QString		GetRecordShortName(void);
    // Return long name of the record
    QString		GetRecordLongName(void);
    // Return record type
    int			GetRecordType(void);
    // Read record
    bool		Read(GS::StdLib::Stdf & clStdf);
    // Write record
    bool		Write(GS::StdLib::Stdf & clStdf);
    // Construct a stringlist with all fields to display
    // Each string in the list has following format: <Field name>;<Field value>
    void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
    // Construct a ASCII string of the record
    void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);
    // Construct a XML string of the record
    void		GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection = 0);
    // Construct a ATDF string of the record
    void		GetAtdfString(QString & strAtdfString);

    // Set members and flags
    void		SetTEST_NUM(const stdf_type_u4 u4TEST_NUM)	_FIELD_SET(m_u4TEST_NUM = u4TEST_NUM, true, eposTEST_NUM)
    void		SetHEAD_NUM(const stdf_type_u1 u1HEAD_NUM)	_FIELD_SET(m_u1HEAD_NUM = u1HEAD_NUM, true, eposHEAD_NUM)
    void		SetSITE_NUM(const stdf_type_u1 u1SITE_NUM)	_FIELD_SET(m_u1SITE_NUM = u1SITE_NUM, true, eposSITE_NUM)
    void		SetTEST_FLG(const stdf_type_b1 b1TEST_FLG)	_FIELD_SET(m_b1TEST_FLG = b1TEST_FLG, true, eposTEST_FLG)
    void		SetPARM_FLG(const stdf_type_b1 b1PARM_FLG)	_FIELD_SET(m_b1PARM_FLG = b1PARM_FLG, true, eposPARM_FLG)
    void		SetRTN_ICNT(const stdf_type_u2 u2RTN_ICNT)	_FIELD_SET(m_u2RTN_ICNT = u2RTN_ICNT, true, eposRTN_ICNT)
    void		SetRSLT_CNT(const stdf_type_u2 u2RSLT_CNT)	_FIELD_SET(m_u2RSLT_CNT = u2RSLT_CNT, true, eposRSLT_CNT)
    void		SetRTN_STAT(const stdf_type_u2 u2Index, const stdf_type_n1 n1RTN_STAT)
    {
        if((m_pFieldFlags[eposRTN_ICNT] & FieldFlag_Present) && (m_u2RTN_ICNT > 0) && (u2Index < m_u2RTN_ICNT))
        {
            stdf_type_u2 u2Temp_Index;
            if(m_kn1RTN_STAT == NULL)
            {
                m_kn1RTN_STAT = new stdf_type_n1[m_u2RTN_ICNT];
                for(u2Temp_Index=0; u2Temp_Index<m_u2RTN_ICNT; u2Temp_Index++)
                    m_kn1RTN_STAT[u2Temp_Index] = 0;
            }
            _FIELD_SET(m_kn1RTN_STAT[u2Index] = n1RTN_STAT, true, eposRTN_STAT)
        }
    }
    void		SetRTN_RSLT(const Stdf_MPR_V4 & RefMpr)
    {
        //Any results?
        if(!RefMpr.m_kr4RTN_RSLT)
        {
            m_u2RSLT_CNT = 0;
            if(m_kr4RTN_RSLT)
            {
                delete m_kr4RTN_RSLT;
                m_kr4RTN_RSLT = NULL;
            }
            if(m_kbRTN_RSLT_IsNan)
            {
                delete m_kbRTN_RSLT_IsNan;
                m_kbRTN_RSLT_IsNan = NULL;
            }
        }
        else
        {
            // Make sure we have same count of results
            if(m_u2RSLT_CNT != RefMpr.m_u2RSLT_CNT)
            {
                m_u2RSLT_CNT = RefMpr.m_u2RSLT_CNT;
                if(m_kr4RTN_RSLT)
                    delete m_kr4RTN_RSLT;
                m_kr4RTN_RSLT = new stdf_type_r4[m_u2RSLT_CNT];
                if(m_kbRTN_RSLT_IsNan)
                    delete m_kbRTN_RSLT_IsNan;
                m_kbRTN_RSLT_IsNan = new bool[m_u2RSLT_CNT];
            }
            // Copy results
            for(int lIndex=0; lIndex<m_u2RSLT_CNT; ++lIndex)
            {
                m_kbRTN_RSLT_IsNan[lIndex] = RefMpr.m_kbRTN_RSLT_IsNan[lIndex];
                _FIELD_SET(m_kr4RTN_RSLT[lIndex] = RefMpr.m_kr4RTN_RSLT[lIndex], true, eposRTN_RSLT)
            }
        }
    }
    void		SetRTN_RSLT(const stdf_type_u2 u2Index, const stdf_type_r4 r4RTN_RSLT)
    {
        if((m_pFieldFlags[eposRSLT_CNT] & FieldFlag_Present) && (m_u2RSLT_CNT > 0) && (u2Index < m_u2RSLT_CNT))
        {
            stdf_type_u2 u2Temp_Index;
            if(m_kr4RTN_RSLT == NULL)
            {
                m_kr4RTN_RSLT = new stdf_type_r4[m_u2RSLT_CNT];
                for(u2Temp_Index=0; u2Temp_Index<m_u2RSLT_CNT; u2Temp_Index++)
                    m_kr4RTN_RSLT[u2Temp_Index] = 0.0F;
            }
            _FIELD_SET(m_kr4RTN_RSLT[u2Index] = r4RTN_RSLT, true, eposRTN_RSLT)
        }
    }
    void		SetTEST_TXT(const stdf_type_cn cnTEST_TXT)	_FIELD_SET(m_cnTEST_TXT = cnTEST_TXT, true, eposTEST_TXT)
    void		SetALARM_ID(const stdf_type_cn cnALARM_ID)	_FIELD_SET(m_cnALARM_ID = cnALARM_ID, true, eposALARM_ID)
    void		SetOPT_FLAG(const stdf_type_b1 b1OPT_FLAG)	_FIELD_SET(m_b1OPT_FLAG = b1OPT_FLAG, true, eposOPT_FLAG)
    void		SetRES_SCAL(const stdf_type_i1 i1RES_SCAL)	_FIELD_SET(m_i1RES_SCAL = i1RES_SCAL, true, eposRES_SCAL)
    void		SetLLM_SCAL(const stdf_type_i1 i1LLM_SCAL)	_FIELD_SET(m_i1LLM_SCAL = i1LLM_SCAL, true, eposLLM_SCAL)
    void		SetHLM_SCAL(const stdf_type_i1 i1HLM_SCAL)	_FIELD_SET(m_i1HLM_SCAL = i1HLM_SCAL, true, eposHLM_SCAL)
    void		SetLO_LIMIT(const stdf_type_r4 r4LO_LIMIT)	_FIELD_SET(m_r4LO_LIMIT = r4LO_LIMIT, true, eposLO_LIMIT)
    void		SetHI_LIMIT(const stdf_type_r4 r4HI_LIMIT)	_FIELD_SET(m_r4HI_LIMIT = r4HI_LIMIT, true, eposHI_LIMIT)
    void		SetSTART_IN(const stdf_type_r4 r4START_IN)	_FIELD_SET(m_r4START_IN = r4START_IN, true, eposSTART_IN)
    void		SetINCR_IN(const stdf_type_r4 r4INCR_IN)	_FIELD_SET(m_r4INCR_IN = r4INCR_IN, true, eposINCR_IN)
    void		SetRTN_INDX(const stdf_type_u2 u2Index, const stdf_type_u2 u2RTN_INDX)
    {
        if((m_pFieldFlags[eposRTN_ICNT] & FieldFlag_Present) && (m_u2RTN_ICNT > 0))
        {
            if(m_ku2RTN_INDX == NULL)
                m_ku2RTN_INDX = new stdf_type_u2[m_u2RTN_ICNT];
            if(u2Index < m_u2RTN_ICNT)	_FIELD_SET(m_ku2RTN_INDX[u2Index] = u2RTN_INDX, true, eposRTN_INDX)
        }
    }
    void		SetUNITS(const stdf_type_cn cnUNITS)		_FIELD_SET(m_cnUNITS = cnUNITS, true, eposUNITS)
    void		SetUNITS_IN(const stdf_type_cn cnUNITS_IN)	_FIELD_SET(m_cnUNITS_IN = cnUNITS_IN, true, eposUNITS_IN)
    void		SetC_RESFMT(const stdf_type_cn cnC_RESFMT)	_FIELD_SET(m_cnC_RESFMT = cnC_RESFMT, true, eposC_RESFMT)
    void		SetC_LLMFMT(const stdf_type_cn cnC_LLMFMT)	_FIELD_SET(m_cnC_LLMFMT = cnC_LLMFMT, true, eposC_LLMFMT)
    void		SetC_HLMFMT(const stdf_type_cn cnC_HLMFMT)	_FIELD_SET(m_cnC_HLMFMT = cnC_HLMFMT, true, eposC_HLMFMT)
    void		SetLO_SPEC(const stdf_type_r4 r4LO_SPEC)	_FIELD_SET(m_r4LO_SPEC = r4LO_SPEC, true, eposLO_SPEC)
    void		SetHI_SPEC(const stdf_type_r4 r4HI_SPEC)	_FIELD_SET(m_r4HI_SPEC = r4HI_SPEC, true, eposHI_SPEC)

    // Some test related functions
    bool		IsTestExecuted();                       // Return true if test has been executed
    bool		IsTestFail();                           // Return true if test is FAIL
    bool		IsTestFail(Stdf_MPR_V4 & clRefMPR);	// Return true if test is FAIL
    void        UpdatePassFailInfo(Stdf_MPR_V4 & clRefMPR);

    // Check field validity
    bool		IsFieldValid(int nFieldPos) const;		// Return true if field is valid

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposTEST_NUM			= 0,	// MPR.TEST_NUM
            eposHEAD_NUM			= 1,	// MPR.HEAD_NUM
            eposSITE_NUM			= 2,	// MPR.SITE_NUM
            eposTEST_FLG			= 3,	// MPR.TEST_FLG
            eposPARM_FLG			= 4,	// MPR.PARM_FLG
            eposRTN_ICNT			= 5,	// MPR.RTN_ICNT
            eposFIRST_OPTIONAL		= 5,
            eposRSLT_CNT			= 6,	// MPR.RSLT_CNT
            eposRTN_STAT			= 7,	// MPR.RTN_STAT
            eposRTN_RSLT			= 8,	// MPR.RTN_RSLT
            eposTEST_TXT			= 9,	// MPR.TEST_TXT
            eposALARM_ID			= 10,	// MPR.ALARM_ID
            eposOPT_FLAG			= 11,	// MPR.OPT_FLAG
            eposRES_SCAL			= 12,	// MPR.RES_SCAL
            eposLLM_SCAL			= 13,	// MPR.LLM_SCAL
            eposHLM_SCAL			= 14,	// MPR.HLM_SCAL
            eposLO_LIMIT			= 15,	// MPR.LO_LIMIT
            eposHI_LIMIT			= 16,	// MPR.HI_LIMIT
            eposSTART_IN			= 17,	// MPR.START_IN
            eposINCR_IN				= 18,	// MPR.INCR_IN
            eposRTN_INDX			= 19,	// MPR.RTN_INDX
            eposUNITS				= 20,	// MPR.UNITS
            eposUNITS_IN			= 21,	// MPR.UNITS_IN
            eposC_RESFMT			= 22,	// MPR.C_RESFMT
            eposC_LLMFMT			= 23,	// MPR.C_LLMFMT
            eposC_HLMFMT			= 24,	// MPR.C_HLMFMT
            eposLO_SPEC				= 25,	// MPR.LO_SPEC
            eposHI_SPEC				= 26,	// MPR.HI_SPEC
            eposEND					= 27
    };

    stdf_type_u4	m_u4TEST_NUM;		// MPR.TEST_NUM
    stdf_type_u1	m_u1HEAD_NUM;		// MPR.HEAD_NUM
    stdf_type_u1	m_u1SITE_NUM;		// MPR.SITE_NUM
    stdf_type_b1	m_b1TEST_FLG;		// MPR.TEST_FLG
    stdf_type_b1	m_b1PARM_FLG;		// MPR.PARM_FLG
    stdf_type_u2	m_u2RTN_ICNT;		// MPR.RTN_ICNT
    stdf_type_u2	m_u2RSLT_CNT;		// MPR.RSLT_CNT
    stdf_type_n1*	m_kn1RTN_STAT;		// MPR.RTN_STAT
    stdf_type_r4*	m_kr4RTN_RSLT;		// MPR.RTN_RSLT
    bool*			m_kbRTN_RSLT_IsNan;	// Set to true if MPR.RTN_RSLT is a NAN value
    stdf_type_cn	m_cnTEST_TXT;		// MPR.TEST_TXT
    stdf_type_cn	m_cnALARM_ID;		// MPR.ALARM_ID
    stdf_type_b1	m_b1OPT_FLAG;		// MPR.OPT_FLAG
    stdf_type_i1	m_i1RES_SCAL;		// MPR.RES_SCAL
    stdf_type_i1	m_i1LLM_SCAL;		// MPR.LLM_SCAL
    stdf_type_i1	m_i1HLM_SCAL;		// MPR.HLM_SCAL
    stdf_type_r4	m_r4LO_LIMIT;		// MPR.LO_LIMIT
    stdf_type_r4	m_r4HI_LIMIT;		// MPR.HI_LIMIT
    stdf_type_r4	m_r4START_IN;		// MPR.START_IN
    stdf_type_r4	m_r4INCR_IN;		// MPR.INCR_IN
    stdf_type_u2*	m_ku2RTN_INDX;		// MPR.RTN_INDX
    stdf_type_cn	m_cnUNITS;			// MPR.UNITS
    stdf_type_cn	m_cnUNITS_IN;		// MPR.UNITS_IN
    stdf_type_cn	m_cnC_RESFMT;		// MPR.C_RESFMT
    stdf_type_cn	m_cnC_LLMFMT;		// MPR.C_LLMFMT
    stdf_type_cn	m_cnC_HLMFMT;		// MPR.C_HLMFMT
    stdf_type_r4	m_r4LO_SPEC;		// MPR.LO_SPEC
    stdf_type_r4	m_r4HI_SPEC;		// MPR.HI_SPEC

private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};

///////////////////////////////////////////////////////////
// FTR RECORD
///////////////////////////////////////////////////////////
class Stdf_FTR_V4: public Stdf_Record
{
    Q_DISABLE_COPY(Stdf_FTR_V4)

// METHODS
public:
    // Constructor / destructor functions
    Stdf_FTR_V4();
    ~Stdf_FTR_V4();

    enum ETEST_FLG	{	eALARM				= 0x1,	// See complete flags definition in STDF specification
                        eRESERVED_BIT1		= 0x2,	// version 4, page 48
                        eNOTRELIABLE_RESULT	= 0x4,
                        eTIMEOUT			= 0x8,
                        eTEST_NOTEXECUTED	= 0x10,
                        eTEST_ABORTED		= 0x20,
                        eNO_PASSFAIL		= 0x40,
                        eTEST_FAILED		= 0x80
    };

    enum EOPT_FLAG	{	eNOTVALID_CYCL_CNT	= 0x1,	// See complete flags definition in STDF specification
                        eNOTVALID_REL_VADR	= 0x2,	// version 4, page 48
                        eNOTVALID_REPT_CNT	= 0x4,
                        eNOTVALID_NUM_FAIL	= 0x8,
                        eNOTVALID_xFAIL_AD	= 0x10,
                        eNOTVALID_VECT_OFF	= 0x20,
                        eRESERVED_BIT6		= 0x40,
                        eRESERVED_BIT7		= 0x80,
                        eOPT_FLAG_ALL		= 0xFF
    };

    // Reset record data
    void		Reset(void);
    // Return short name of the record
    QString		GetRecordShortName(void);
    // Return long name of the record
    QString		GetRecordLongName(void);
    // Return record type
    int			GetRecordType(void);
    // Read record
    bool		Read(GS::StdLib::Stdf & clStdf);
    // Write record
    bool		Write(GS::StdLib::Stdf & clStdf);
    // Construct a stringlist with all fields to display
    // Each string in the list has following format: <Field name>;<Field value>
    void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
    // Construct a ASCII string of the record
    void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);
    // Construct a XML string of the record
    void		GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection = 0);
    // Construct a ATDF string of the record
    void		GetAtdfString(QString & strAtdfString);
    // Define a filter when reading data
    void		SetFilter(Stdf_FTR_V4 * pFilter);

    // Set members and flags
    void		SetTEST_NUM(const stdf_type_u4 u4TEST_NUM)	_FIELD_SET(m_u4TEST_NUM = u4TEST_NUM, true, eposTEST_NUM)
    void		SetHEAD_NUM(const stdf_type_u1 u1HEAD_NUM)	_FIELD_SET(m_u1HEAD_NUM = u1HEAD_NUM, true, eposHEAD_NUM)
    void		SetSITE_NUM(const stdf_type_u1 u1SITE_NUM)	_FIELD_SET(m_u1SITE_NUM = u1SITE_NUM, true, eposSITE_NUM)
    void		SetTEST_FLG(const stdf_type_b1 b1TEST_FLG)	_FIELD_SET(m_b1TEST_FLG = b1TEST_FLG, true, eposTEST_FLG)
    void		SetOPT_FLAG(const stdf_type_b1 b1OPT_FLAG)	_FIELD_SET(m_b1OPT_FLAG = b1OPT_FLAG, true, eposOPT_FLAG)
    void		SetCYCL_CNT(const stdf_type_u4 u4CYCL_CNT)	_FIELD_SET(m_u4CYCL_CNT = u4CYCL_CNT, true, eposCYCL_CNT)
    void		SetREL_VADR(const stdf_type_u4 u4REL_VADR)	_FIELD_SET(m_u4REL_VADR = u4REL_VADR, true, eposREL_VADR)
    void		SetREPT_CNT(const stdf_type_u4 u4REPT_CNT)	_FIELD_SET(m_u4REPT_CNT = u4REPT_CNT, true, eposREPT_CNT)
    void		SetNUM_FAIL(const stdf_type_u4 u4NUM_FAIL)	_FIELD_SET(m_u4NUM_FAIL = u4NUM_FAIL, true, eposNUM_FAIL)
    void		SetXFAIL_AD(const stdf_type_i4 i4XFAIL_AD)	_FIELD_SET(m_i4XFAIL_AD = i4XFAIL_AD, true, eposXFAIL_AD)
    void		SetYFAIL_AD(const stdf_type_i4 i4YFAIL_AD)	_FIELD_SET(m_i4YFAIL_AD = i4YFAIL_AD, true, eposYFAIL_AD)
    void		SetVECT_OFF(const stdf_type_i2 i2VECT_OFF)	_FIELD_SET(m_i2VECT_OFF = i2VECT_OFF, true, eposVECT_OFF)
    void		SetRTN_ICNT(const stdf_type_u2 u2RTN_ICNT)	_FIELD_SET(m_u2RTN_ICNT = u2RTN_ICNT, true, eposRTN_ICNT)
    void		SetPGM_ICNT(const stdf_type_u2 u2PGM_ICNT)	_FIELD_SET(m_u2PGM_ICNT = u2PGM_ICNT, true, eposPGM_ICNT)
    void		SetRTN_INDX(const stdf_type_u2 u2Index, const stdf_type_u2 u2RTN_INDX)
    {
        if((m_pFieldFlags[eposRTN_ICNT] & FieldFlag_Present) && (m_u2RTN_ICNT > 0))
        {
            if(m_ku2RTN_INDX == NULL)
                m_ku2RTN_INDX = new stdf_type_u2[m_u2RTN_ICNT];
            if(u2Index < m_u2RTN_ICNT)	_FIELD_SET(m_ku2RTN_INDX[u2Index] = u2RTN_INDX, true, eposRTN_INDX)
        }
    }
    void		SetRTN_STAT(const stdf_type_u2 u2Index, const stdf_type_n1 n1RTN_STAT)
    {
        if((m_pFieldFlags[eposRTN_ICNT] & FieldFlag_Present) && (m_u2RTN_ICNT > 0) && (u2Index < m_u2RTN_ICNT))
        {
            stdf_type_u2 u2Temp_Index;
            if(m_kn1RTN_STAT == NULL)
            {
                m_kn1RTN_STAT = new stdf_type_n1[m_u2RTN_ICNT];
                for(u2Temp_Index=0; u2Temp_Index<m_u2RTN_ICNT; u2Temp_Index++)
                    m_kn1RTN_STAT[u2Temp_Index] = 0;
            }
            _FIELD_SET(m_kn1RTN_STAT[u2Index] = n1RTN_STAT, true, eposRTN_STAT)
        }
    }
    void		SetPGM_INDX(const stdf_type_u2 u2Index, const stdf_type_u2 u2PGM_INDX)
    {
        if((m_pFieldFlags[eposPGM_ICNT] & FieldFlag_Present) && (m_u2PGM_ICNT > 0))
        {
            if(m_ku2PGM_INDX == NULL)
                m_ku2PGM_INDX = new stdf_type_u2[m_u2PGM_ICNT];
            if(u2Index < m_u2PGM_ICNT)	_FIELD_SET(m_ku2PGM_INDX[u2Index] = u2PGM_INDX, true, eposPGM_INDX)
        }
    }
    void		SetPGM_STAT(const stdf_type_u2 u2Index, const stdf_type_n1 n1PGM_STAT)
    {
        if((m_pFieldFlags[eposPGM_ICNT] & FieldFlag_Present) && (m_u2PGM_ICNT > 0) && (u2Index < m_u2PGM_ICNT))
        {
            stdf_type_u2 u2Temp_Index;
            if(m_kn1PGM_STAT == NULL)
            {
                m_kn1PGM_STAT = new stdf_type_n1[m_u2PGM_ICNT];
                for(u2Temp_Index=0; u2Temp_Index<m_u2PGM_ICNT; u2Temp_Index++)
                    m_kn1PGM_STAT[u2Temp_Index] = 0;
            }
            _FIELD_SET(m_kn1PGM_STAT[u2Index] = n1PGM_STAT, true, eposPGM_STAT)
        }
    }
    void		SetFAIL_PIN()                               _FIELD_SET_FLAGS(m_dnFAIL_PIN.m_uiLength > 0, eposFAIL_PIN)
    void		SetVECT_NAM(const stdf_type_cn cnVECT_NAM)	_FIELD_SET(m_cnVECT_NAM = cnVECT_NAM, true, eposVECT_NAM)
    void		SetTIME_SET(const stdf_type_cn cnTIME_SET)	_FIELD_SET(m_cnTIME_SET = cnTIME_SET, true, eposTIME_SET)
    void		SetOP_CODE(const stdf_type_cn cnOP_CODE)	_FIELD_SET(m_cnOP_CODE = cnOP_CODE, true, eposOP_CODE)
    void		SetTEST_TXT(const stdf_type_cn cnTEST_TXT)	_FIELD_SET(m_cnTEST_TXT = cnTEST_TXT, true, eposTEST_TXT)
    void		SetALARM_ID(const stdf_type_cn cnALARM_ID)	_FIELD_SET(m_cnALARM_ID = cnALARM_ID, true, eposALARM_ID)
    void		SetPROG_TXT(const stdf_type_cn cnPROG_TXT)	_FIELD_SET(m_cnPROG_TXT = cnPROG_TXT, true, eposPROG_TXT)
    void		SetRSLT_TXT(const stdf_type_cn cnRSLT_TXT)	_FIELD_SET(m_cnRSLT_TXT = cnRSLT_TXT, true, eposRSLT_TXT)
    void		SetPATG_NUM(const stdf_type_u1 u1PATG_NUM)	_FIELD_SET(m_u1PATG_NUM = u1PATG_NUM, true, eposPATG_NUM)
    void		SetSPIN_MAP()
    {
        _FIELD_SET(m_dnSPIN_MAP.m_uiLength=m_dnSPIN_MAP.m_uiLength, m_dnSPIN_MAP.m_uiLength > 0, eposSPIN_MAP);
    }

    // Some test related functions
    bool		IsTestExecuted();					// Return true if test has been executed
    bool		IsTestFail();						// Return true if test is FAIL

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposTEST_NUM			= 0,	// FTR.TEST_NUM
            eposHEAD_NUM			= 1,	// FTR.HEAD_NUM
            eposSITE_NUM			= 2,	// FTR.SITE_NUM
            eposTEST_FLG			= 3,	// FTR.TEST_FLG
            eposOPT_FLAG			= 4,	// FTR.OPT_FLAG
            eposFIRST_OPTIONAL		= 4,
            eposCYCL_CNT			= 5,	// FTR.CYCL_CNT
            eposREL_VADR			= 6,	// FTR.REL_VADR
            eposREPT_CNT			= 7,	// FTR.REPT_CNT
            eposNUM_FAIL			= 8,	// FTR.NUM_FAIL
            eposXFAIL_AD			= 9,	// FTR.XFAIL_AD
            eposYFAIL_AD			= 10,	// FTR.YFAIL_AD
            eposVECT_OFF			= 12,	// FTR.VECT_OFF
            eposRTN_ICNT			= 13,	// FTR.RTN_ICNT
            eposPGM_ICNT			= 14,	// FTR.PGM_ICNT
            eposRTN_INDX			= 15,	// FTR.RTN_INDX
            eposRTN_STAT			= 16,	// FTR.RTN_STAT
            eposPGM_INDX			= 17,	// FTR.PGM_INDX
            eposPGM_STAT			= 18,	// FTR.PGM_STAT
            eposFAIL_PIN			= 19,	// FTR.FAIL_PIN
            eposVECT_NAM			= 20,	// FTR.VECT_NAM
            eposTIME_SET			= 21,	// FTR.TIME_SET
            eposOP_CODE				= 22,	// FTR.OP_CODE
            eposTEST_TXT			= 23,	// FTR.TEST_TXT
            eposALARM_ID			= 24,	// FTR.ALARM_ID
            eposPROG_TXT			= 25,	// FTR.PROG_TXT
            eposRSLT_TXT			= 26,	// FTR.RSLT_TXT
            eposPATG_NUM			= 27,	// FTR.PATG_NUM
            eposSPIN_MAP			= 28,	// FTR.SPIN_MAP
            eposEND					= 29
    };

    stdf_type_u4	m_u4TEST_NUM;		// FTR.TEST_NUM
    stdf_type_u1	m_u1HEAD_NUM;		// FTR.HEAD_NUM
    stdf_type_u1	m_u1SITE_NUM;		// FTR.SITE_NUM
    stdf_type_b1	m_b1TEST_FLG;		// FTR.TEST_FLG
    stdf_type_b1	m_b1OPT_FLAG;		// FTR.OPT_FLAG
    stdf_type_u4	m_u4CYCL_CNT;		// FTR.CYCL_CNT
    stdf_type_u4	m_u4REL_VADR;		// FTR.REL_VADR
    stdf_type_u4	m_u4REPT_CNT;		// FTR.REPT_CNT
    stdf_type_u4	m_u4NUM_FAIL;		// FTR.NUM_FAIL
    stdf_type_i4	m_i4XFAIL_AD;		// FTR.XFAIL_AD
    stdf_type_i4	m_i4YFAIL_AD;		// FTR.YFAIL_AD
    stdf_type_i2	m_i2VECT_OFF;		// FTR.VECT_OFF
    stdf_type_u2	m_u2RTN_ICNT;		// FTR.RTN_ICNT
    stdf_type_u2	m_u2PGM_ICNT;		// FTR.PGM_ICNT
    stdf_type_u2*	m_ku2RTN_INDX;		// FTR.RTN_INDX
    stdf_type_n1*	m_kn1RTN_STAT;		// FTR.RTN_STAT
    stdf_type_u2*	m_ku2PGM_INDX;		// FTR.PGM_INDX
    stdf_type_n1*	m_kn1PGM_STAT;		// FTR.PGM_STAT
    stdf_type_dn	m_dnFAIL_PIN;		// FTR.FAIL_PIN
    stdf_type_cn	m_cnVECT_NAM;		// FTR.VECT_NAM
    stdf_type_cn	m_cnTIME_SET;		// FTR.TIME_SET
    stdf_type_cn	m_cnOP_CODE;		// FTR.OP_CODE
    stdf_type_cn	m_cnTEST_TXT;		// FTR.TEST_TXT
    stdf_type_cn	m_cnALARM_ID;		// FTR.ALARM_ID
    stdf_type_cn	m_cnPROG_TXT;		// FTR.PROG_TXT
    stdf_type_cn	m_cnRSLT_TXT;		// FTR.RSLT_TXT
    stdf_type_u1	m_u1PATG_NUM;		// FTR.PATG_NUM
    stdf_type_dn	m_dnSPIN_MAP;		// FTR.SPIN_MAP

private:
    // Hold flags for each field
    int				m_pFieldFlags[eposEND];
    Stdf_FTR_V4 *	m_pFilter;
};

///////////////////////////////////////////////////////////
// BPS RECORD
///////////////////////////////////////////////////////////
class Stdf_BPS_V4: public Stdf_Record
{
    Q_DISABLE_COPY(Stdf_BPS_V4)

// METHODS
public:
    // Constructor / destructor functions
    Stdf_BPS_V4();
    ~Stdf_BPS_V4();

    // Reset record data
    void		Reset(void);
    // Return short name of the record
    QString		GetRecordShortName(void);
    // Return long name of the record
    QString		GetRecordLongName(void);
    // Return record type
    int			GetRecordType(void);
    // Read record
    bool		Read(GS::StdLib::Stdf & clStdf);
    // Write record
    bool		Write(GS::StdLib::Stdf & clStdf);
    // Construct a stringlist with all fields to display
    // Each string in the list has following format: <Field name>;<Field value>
    void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
    // Construct a ASCII string of the record
    void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);
    // Construct a XML string of the record
    void		GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection = 0);
    // Construct a ATDF string of the record
    void		GetAtdfString(QString & strAtdfString);

    // Set members and flags
    void		SetSEQ_NAME(const stdf_type_cn cnSEQ_NAME)	_FIELD_SET(m_cnSEQ_NAME = cnSEQ_NAME, true, eposSEQ_NAME)

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposSEQ_NAME			= 0,
            eposFIRST_OPTIONAL		= 0,
            eposEND					= 1
    };

    stdf_type_cn	m_cnSEQ_NAME;		// BPS.SEQ_NAME

private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};

///////////////////////////////////////////////////////////
// EPS RECORD
///////////////////////////////////////////////////////////
class Stdf_EPS_V4: public Stdf_Record
{
    Q_DISABLE_COPY(Stdf_EPS_V4)

// METHODS
public:
    // Constructor / destructor functions
    Stdf_EPS_V4();
    ~Stdf_EPS_V4();

    // Reset record data
    void		Reset(void);
    // Return short name of the record
    QString		GetRecordShortName(void);
    // Return long name of the record
    QString		GetRecordLongName(void);
    // Return record type
    int			GetRecordType(void);
    // Read record
    bool		Read(GS::StdLib::Stdf & clStdf);
    // Write record
    bool		Write(GS::StdLib::Stdf & clStdf);
    // Construct a stringlist with all fields to display
    // Each string in the list has following format: <Field name>;<Field value>
    void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
    // Construct a ASCII string of the record
    void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);
    // Construct a XML string of the record
    void		GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection = 0);
    // Construct a ATDF string of the record
    void		GetAtdfString(QString & strAtdfString);

// DATA
public:

private:
    // Define position of each field

    // Hold flags for each field
};

///////////////////////////////////////////////////////////
// DTR RECORD
///////////////////////////////////////////////////////////
//! \class Manage STDF DTR records
//! \brief This class is useful to manage STDF DTR records: read, write, get data...
class Stdf_DTR_V4: public Stdf_Record
{
    Q_DISABLE_COPY(Stdf_DTR_V4)

// METHODS
public:
    // Constructor / destructor
    //! \brief Constructor: calls Reset()
    Stdf_DTR_V4();
    //! \brief Destructor
    ~Stdf_DTR_V4();

    //! \brief Reset record data
    void		Reset(void);
    //! \brief Return short name of the record
    QString		GetRecordShortName(void);
    //! \brief Return long name of the record
    QString		GetRecordLongName(void);
    //! \brief Return record type
    int			GetRecordType(void);
    //! \brief Read record
    bool		Read(GS::StdLib::Stdf & clStdf);
    //! \brief Write record
    bool		Write(GS::StdLib::Stdf & clStdf);
    //! \brief Construct a stringlist with all fields to display
    //! Each string in the list has following format: <Field name>;<Field value>
    void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
    //! \brief Construct a ASCII string of the record
    void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);
    //! \brief Construct a XML string of the record
    void		GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection = 0);
    //! \brief Construct a ATDF string of the record
    void		GetAtdfString(QString & strAtdfString);

    //! \brief Set DTR.TEXT_DATA field from a string
    void		SetTEXT_DAT(const stdf_type_cn cnTEXT_DAT)	_FIELD_SET(m_cnTEXT_DAT = cnTEXT_DAT, true, eposTEXT_DAT)
    //! \brief Set DTR.TEXT_DATA field from a JSON object
    void        SetTEXT_DAT(const QJsonObject & JsonObject);

    //! \brief Check field validity: returns true if the field at position nFieldPos is valid
    bool		IsFieldValid(int nFieldPos);

    //! \brief Check if DTR.TEXT_DATA contains a GS Gross-Die command. If so, get gross die value.
    bool		IsGexCommand_GrossDie(unsigned int *puiGrossDie, bool *pbOk);
    //! \brief Check if DTR.TEXT_DATA contains a GS Die-Tracking command. If so, get die tracking values.
    bool		IsGexCommand_DieTracking(QString & strDieID, QString & strCommand, bool *pbOk);
    //! \brief Check if DTR.TEXT_DATA contains a GS logPAT command. If so, get PAT log string.
    bool		IsGexCommand_logPAT(QString & strCommand);
    //! \brief Check if DTR.TEXT_DATA contains a GS PATTestList command. If so, get the PAT test list.
    bool		IsGexCommand_PATTestList(QString & strCommand);
    //! \brief Return the JSON object contained in DTR.TEXT_DATA if it contains a valid
    //! GS JSON (no missing mandatory keys...)
    //! ex: ”{"TYPE":"ML","TNUM":2,"LL":-3e-6,"HL":20e-6,"HBIN":1}”
    //! The returned Json will be empty if:
    //! 1) DTR.TEXT_DAT is not a valid JSON format
    //! 2) DTR.TEXT_DAT is not a JSON object
    //! 3) DTR.TEXT_DAT JSON does not have a "TYPE" key
    //! 4) DTR.TEXT_DAT JSON with specified "TYPE" is not valid for that type (missing other keys...)
    //! The error string is populated only in above case 4
    QJsonObject GetGsJson(const QString type) const;

    //! \brief Check if DTR.TEXT_DATA is a Syntricity Test Condition.
    //! Starts with: COND
    //! Follwed by "CLEAR" or something that matchs the RegExp:  ([a-zA-Z|_]+=[0-9]+\.[0-9]+[a-zA-Z]+\,?)+
    //! Example: DTR.text_dat = COND: VDDBAT=2.3V,VDDC_WL=1.2V,VDDO=2.3V,VDDA_WL=1.2V,VDDPA=2.3V,
    bool IsSyntricityTestcond(QMap<QString, QString> &syntricityTestCond, bool &isOk) const;

// DATA
public:
    //! \brief Define position of each field
    enum FieldPos {
            eposTEXT_DAT			= 0,
            eposEND					= 1,
            eposFIRST_OPTIONAL		= 1
    };

    //! \brief DTR.TEXT_DAT
    stdf_type_cn	m_cnTEXT_DAT;

private:
    //! \brief Holds flags for each field
    int		m_pFieldFlags[eposEND];
};

///////////////////////////////////////////////////////////
// GDR RECORD
///////////////////////////////////////////////////////////
class Stdf_GDR_V4: public Stdf_Record
{
    Q_DISABLE_COPY(Stdf_GDR_V4)

// METHODS
public:
    // Constructor / destructor functions
    Stdf_GDR_V4();
    ~Stdf_GDR_V4();

    // Reset record data
    void		Reset(void);
    // Return short name of the record
    QString		GetRecordShortName(void);
    // Return long name of the record
    QString		GetRecordLongName(void);
    // Return record type
    int			GetRecordType(void);
    // Read record
    bool		Read(GS::StdLib::Stdf & clStdf);
    // Construct a stringlist with all fields to display
    // Each string in the list has following format: <Field name>;<Field value>
    void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
    // Construct a ASCII string of the record
    void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);
    // Construct a XML string of the record
    void		GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection = 0);
    // Construct a ATDF string of the record
    void		GetAtdfString(QString & strAtdfString);

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid


    // Write record
    bool		Write(GS::StdLib::Stdf & clStdf);
    void		SetFLD_CNT(const stdf_type_u2 u2FLD_CNT)	_FIELD_SET(m_u2FLD_CNT = u2FLD_CNT, true, eposFLD_CNT)
    void		SetGEN_DATA(const std::vector<stdf_type_vn>	vnGEN_DATA);
    void		SetGEN_DATA(stdf_type_vn*	vnGEN_DATA);

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposFLD_CNT				= 0,
            eposGEN_DATA			= 1,
            eposEND					= 2,
            eposFIRST_OPTIONAL		= 2
    };

    enum DataType {
            eTypePad				= 0,
            eTypeU1					= 1,
            eTypeU2					= 2,
            eTypeU4					= 3,
            eTypeI1					= 4,
            eTypeI2					= 5,
            eTypeI4					= 6,
            eTypeR4					= 7,
            eTypeR8					= 8,
            eTypeCN					= 10,
            eTypeBN					= 11,
            eTypeDN					= 12,
            eTypeN1					= 13
    };
    stdf_type_u2	m_u2FLD_CNT;		// GDR.FLD_CNT
    stdf_type_vn*	m_vnGEN_DATA;		// GDR.GEN_DATA


private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};

class Stdf_VUR_V4: public Stdf_Record
{
    Q_DISABLE_COPY(Stdf_VUR_V4)

// METHODS
public:
    // Constructor / destructor functions
    Stdf_VUR_V4();
    ~Stdf_VUR_V4();

    // Reset record data
    void		Reset(void);
    // Return short name of the record
    QString		GetRecordShortName(void);
    // Return long name of the record
    QString		GetRecordLongName(void);
    // Return record type
    int			GetRecordType(void);
    // Read record
    bool		Read(GS::StdLib::Stdf & clStdf);
    // Write record
    bool		Write(GS::StdLib::Stdf & clStdf);
    // Construct a stringlist with all fields to display
    // Each string in the list has following format: <Field name>;<Field value>
    void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
    // Construct a ASCII string of the record
    void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);
    // Construct a XML string of the record
    void		GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection = 0);
    // Construct a ATDF string of the record
    void		GetAtdfString(QString & strAtdfString);

    // Set members and flags
    void		SetUPD_NAM(const stdf_type_cn cnUPD_NAM)	_FIELD_SET(m_cnUPD_NAM = cnUPD_NAM, true, eposUPD_NAM)

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposUPD_NAM			= 0,
            eposEND					= 1,
            eposFIRST_OPTIONAL =1
    };
    stdf_type_cn	m_cnUPD_NAM;		// VUR.UPD_NAM

private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};


class Stdf_PSR_V4: public Stdf_Record
{
    Q_DISABLE_COPY(Stdf_PSR_V4)

// METHODS
public:
    // Constructor / destructor functions
    Stdf_PSR_V4();
    ~Stdf_PSR_V4();

    // Reset record data
    void		Reset(void);
    // Return short name of the record
    QString		GetRecordShortName(void);
    // Return long name of the record
    QString		GetRecordLongName(void);
    // Return record type
    int			GetRecordType(void);
    // Read record
    bool		Read(GS::StdLib::Stdf & clStdf);
    // Write record
    bool		Write(GS::StdLib::Stdf & clStdf);
    // Construct a stringlist with all fields to display
    // Each string in the list has following format: <Field name>;<Field value>
    void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
    // Construct a ASCII string of the record
    void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);
    // Construct a XML string of the record
    void		GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection = 0);
    // Construct a ATDF string of the record
    void		GetAtdfString(QString & strAtdfString);

    // Set members and flags
    void	SetCONT_FLG	(const	stdf_type_b1	b1CONT_FLG	)       _FIELD_SET(	m_b1CONT_FLG	=	b1CONT_FLG	,true,	eposCONT_FLG	);
    void	SetPSR_INDX	(const	stdf_type_u2	u2PSR_INDX	)       _FIELD_SET(	m_u2PSR_INDX	=	u2PSR_INDX	,true,	eposPSR_INDX	);
    void	SetPSR_NAM	(const	stdf_type_cn	cnPSR_NAM	)       _FIELD_SET(	m_cnPSR_NAM	=	cnPSR_NAM	,true,	eposPSR_NAM	);
    void	SetOPT_FLG	(const	stdf_type_b1	b1OPT_FLG	)       _FIELD_SET(	m_b1OPT_FLG	=	b1OPT_FLG	,true,	eposOPT_FLG	);
    void	SetTOTP_CNT	(const	stdf_type_u2	u2TOTP_CNT	)       _FIELD_SET(	m_u2TOTP_CNT	=	u2TOTP_CNT	,true,	eposTOTP_CNT	);
    void	SetLOCP_CNT	(const	stdf_type_u2	u2LOCP_CNT	)       _FIELD_SET(	m_u2LOCP_CNT	=	u2LOCP_CNT	,true,	eposLOCP_CNT	);

    void		SetPAT_BGN(const stdf_type_u2 u2Index, const stdf_type_u8  u8PAT_BGN)
    {
        if(m_u2LOCP_CNT > 0)
        {
            if(m_pu8PAT_BGN == NULL)
                m_pu8PAT_BGN = new stdf_type_u8[m_u2LOCP_CNT];
            if(u2Index < m_u2LOCP_CNT)	_FIELD_SET(m_pu8PAT_BGN[u2Index] = u8PAT_BGN, true, eposPAT_BGN)
        }
    }
    void		SetPAT_END(const stdf_type_u2 u2Index, const stdf_type_u8  u8PAT_END)
    {
        if(m_u2LOCP_CNT > 0)
        {
            if(m_pu8PAT_END == NULL)
                m_pu8PAT_END = new stdf_type_u8[m_u2LOCP_CNT];
            if(u2Index < m_u2LOCP_CNT)	_FIELD_SET(m_pu8PAT_END[u2Index] = u8PAT_END, true, eposPAT_END)
        }
    }
    void		SetPAT_FILE(const stdf_type_u2 u2Index, const stdf_type_cn cnPAT_FILE)
    {
        if(m_u2LOCP_CNT > 0)
        {
            if(m_pcnPAT_FILE == NULL)
                m_pcnPAT_FILE = new stdf_type_cn[m_u2LOCP_CNT];
            if(u2Index < m_u2LOCP_CNT)	_FIELD_SET(m_pcnPAT_FILE[u2Index] = cnPAT_FILE, true, eposPAT_FILE)
        }
    }
    void		SetPAT_LBL(const stdf_type_u2 u2Index, const stdf_type_cn  cnPAT_LBL)
    {
        if((m_b1OPT_FLG & STDF_MASK_BIT0) == 0 && m_u2LOCP_CNT > 0)
        {
            if(m_pcnPAT_LBL == NULL)
                m_pcnPAT_LBL = new stdf_type_cn[m_u2LOCP_CNT];
            if(u2Index < m_u2LOCP_CNT)	_FIELD_SET(m_pcnPAT_LBL[u2Index] = cnPAT_LBL, true, eposPAT_LBL)
        }
    }
    void		SetFILE_UID(const stdf_type_u2 u2Index, const stdf_type_cn  cnFILE_UID)
    {
        if((m_b1OPT_FLG & STDF_MASK_BIT1) == 0 && m_u2LOCP_CNT > 0)
        {
            if(m_pcnFILE_UID == NULL)
                m_pcnFILE_UID = new stdf_type_cn[m_u2LOCP_CNT];
            if(u2Index < m_u2LOCP_CNT)	_FIELD_SET(m_pcnFILE_UID[u2Index] = cnFILE_UID, true, eposFILE_UID)
        }
    }
    void		SetATPG_DSC(const stdf_type_u2 u2Index, const stdf_type_cn  cnATPG_DSC)
    {
        if((m_b1OPT_FLG & STDF_MASK_BIT2) == 0 && m_u2LOCP_CNT > 0)
        {
            if(m_pcnATPG_DSC == NULL)
                m_pcnATPG_DSC = new stdf_type_cn[m_u2LOCP_CNT];
            if(u2Index < m_u2LOCP_CNT)	_FIELD_SET(m_pcnATPG_DSC[u2Index] = cnATPG_DSC, true, eposATPG_DSC)
        }
    }
    void		SetSRC_ID(const stdf_type_u2 u2Index, const stdf_type_cn  cnSRC_ID)
    {
        if( (m_b1OPT_FLG & STDF_MASK_BIT3) == 0 && m_u2LOCP_CNT > 0 )
        {
            if(m_pcnSRC_ID == NULL)
                m_pcnSRC_ID = new stdf_type_cn[m_u2LOCP_CNT];
            if(u2Index < m_u2LOCP_CNT)	_FIELD_SET(m_pcnSRC_ID[u2Index] = cnSRC_ID, true, eposSRC_ID)
        }
    }


    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposCONT_FLG	=	0,
            eposPSR_INDX	=	1,
            eposPSR_NAM	=	2,
            eposOPT_FLG	=	3,
            eposTOTP_CNT	=	4,
            eposLOCP_CNT	=	5,
            eposPAT_BGN	=	6,
            eposPAT_END	=	7,
            eposPAT_FILE	=	8,
            eposPAT_LBL	=	9,
            eposFIRST_OPTIONAL =	9,
            eposFILE_UID	=	10,
            eposATPG_DSC	=	11,
            eposSRC_ID	=	12,
            eposEND		= 13
    };
    stdf_type_b1	m_b1CONT_FLG	;//	U*1
    stdf_type_u2	m_u2PSR_INDX	;//	U*2
    stdf_type_cn	m_cnPSR_NAM	;//	C*n
    stdf_type_b1	m_b1OPT_FLG	;//	B*1
    stdf_type_u2	m_u2TOTP_CNT	;//	U*2
    stdf_type_u2	m_u2LOCP_CNT	;//	U*2
    stdf_type_u8*	m_pu8PAT_BGN	;//	kxU*8
    stdf_type_u8*	m_pu8PAT_END	;//	kxU*8
    stdf_type_cn*	m_pcnPAT_FILE	;//	kxC*n
    stdf_type_cn*	m_pcnPAT_LBL	;//	K*C*n
    stdf_type_cn*	m_pcnFILE_UID	;//	kxC*n
    stdf_type_cn*	m_pcnATPG_DSC	;//	kxC*n
    stdf_type_cn*	m_pcnSRC_ID	;//	kxC*n


private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
    bool    WritePSR_KCN(GS::StdLib::Stdf &stdfObject, stdf_type_cn *array, stdf_type_u2 size, FieldPos pos);
};

class Stdf_NMR_V4: public Stdf_Record
{
    Q_DISABLE_COPY(Stdf_NMR_V4)

// METHODS
public:
    // Constructor / destructor functions
    Stdf_NMR_V4();
    ~Stdf_NMR_V4();

    // Reset record data
    void		Reset(void);
    // Return short name of the record
    QString		GetRecordShortName(void);
    // Return long name of the record
    QString		GetRecordLongName(void);
    // Return record type
    int			GetRecordType(void);
    // Read record
    bool		Read(GS::StdLib::Stdf & clStdf);
    // Write record
    bool		Write(GS::StdLib::Stdf & clStdf);
    // Construct a stringlist with all fields to display
    // Each string in the list has following format: <Field name>;<Field value>
    void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
    // Construct a ASCII string of the record
    void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);
    // Construct a XML string of the record
    void		GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection = 0);
    // Construct a ATDF string of the record
    void		GetAtdfString(QString & strAtdfString);

    // Set members and flags
    void	SetCONT_FLG	(const	stdf_type_b1	b1CONT_FLG	)       _FIELD_SET(	m_b1CONT_FLG	=	b1CONT_FLG	,true,	eposCONT_FLG	);
    void	SetNMR_INDX	(const	stdf_type_u2	u2NMR_INDX	)       _FIELD_SET(	m_u2NMR_INDX	=	u2NMR_INDX	,true,	eposNMR_INDX	);
    void	SetTOTM_CNT	(const	stdf_type_u2	u2TOTCNT	)       _FIELD_SET(	m_u2TOTM_CNT	=	u2TOTCNT	,true,	eposTOTM_CNT	);
    void	SetLOCM_CNT	(const	stdf_type_u2	u2LOCCNT	)       _FIELD_SET(	m_u2LOCM_CNT	=	u2LOCCNT	,true,	eposLOCM_CNT	);
    void	SetPMR_INDX(const stdf_type_u2 u2Index, const stdf_type_u2 u2PMR_INDX)
    {
        if(m_u2LOCM_CNT > 0)
        {
            if(m_pu2PMR_INDX == NULL)
                m_pu2PMR_INDX = new stdf_type_u2[m_u2LOCM_CNT];
            if(u2Index < m_u2LOCM_CNT)	_FIELD_SET(m_pu2PMR_INDX[u2Index] = u2PMR_INDX, true, eposPMR_INDX)
        }
    }
    void	SetATPG_NAM(const stdf_type_u2 u2Index, const stdf_type_cn cnATPG_NAM)
    {
        if(m_u2LOCM_CNT > 0)
        {
            if(m_pcnATPG_NAM == NULL)
                m_pcnATPG_NAM = new stdf_type_cn[m_u2LOCM_CNT];
            if(u2Index < m_u2LOCM_CNT)	_FIELD_SET(m_pcnATPG_NAM[u2Index] = cnATPG_NAM, true, eposATPG_NAM)
        }
    }

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposCONT_FLG	=	0,
            eposNMR_INDX	=	1,
            eposTOTM_CNT	=	2,
            eposFIRST_OPTIONAL =	2,
            eposLOCM_CNT	=	3,
            eposPMR_INDX	=	4,
            eposATPG_NAM	=	5,
            eposEND		=	6
    };
    stdf_type_b1	m_b1CONT_FLG	;//	U*1
    stdf_type_u2	m_u2NMR_INDX	;//	U*1
    stdf_type_u2	m_u2TOTM_CNT	;//	U*2
    stdf_type_u2	m_u2LOCM_CNT	;//	U*2
    stdf_type_u2*	m_pu2PMR_INDX	;//	U*2
    stdf_type_cn*	m_pcnATPG_NAM	;//	kxC*n

private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};

class Stdf_CNR_V4: public Stdf_Record
{
    Q_DISABLE_COPY(Stdf_CNR_V4)

// METHODS
public:
    // Constructor / destructor functions
    Stdf_CNR_V4();
    ~Stdf_CNR_V4();

    // Reset record data
    void		Reset(void);
    // Return short name of the record
    QString		GetRecordShortName(void);
    // Return long name of the record
    QString		GetRecordLongName(void);
    // Return record type
    int			GetRecordType(void);
    // Read record
    bool		Read(GS::StdLib::Stdf & clStdf);
    // Write record
    bool		Write(GS::StdLib::Stdf & clStdf);
    // Construct a stringlist with all fields to display
    // Each string in the list has following format: <Field name>;<Field value>
    void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
    // Construct a ASCII string of the record
    void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);
    // Construct a XML string of the record
    void		GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection = 0);
    // Construct a ATDF string of the record
    void		GetAtdfString(QString & strAtdfString);

    // Set members and flags
    void		SetCHN_NUM	(const stdf_type_u2  u2CHN_NUM  )	_FIELD_SET(m_u2CHN_NUM 	 	= u2CHN_NUM  , true, eposCHN_NUM  );
    void		SetBIT_POS (const stdf_type_u4   u4BIT_POS  )	_FIELD_SET(m_u4BIT_POS 	= u4BIT_POS  , true, eposBIT_POS  );
    void		SetCELL_NAM(const stdf_type_cn   cnCELL_NAM)	_FIELD_SET(m_cnCELL_NAM  	= cnCELL_NAM, true, eposCELL_NAM);

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposCHN_NUM	 = 0,
            eposBIT_POS  = 1,
            eposCELL_NAM = 2,
            eposEND		 = 3,
            eposFIRST_OPTIONAL = 3
    };
    stdf_type_u2 m_u2CHN_NUM 	;//U*2
    stdf_type_u4 m_u4BIT_POS 	;//U*4
    stdf_type_cn m_cnCELL_NAM ;//S*n

private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};

class Stdf_SSR_V4: public Stdf_Record
{
    Q_DISABLE_COPY(Stdf_SSR_V4)

// METHODS
public:
    // Constructor / destructor functions
    Stdf_SSR_V4();
    ~Stdf_SSR_V4();

    // Reset record data
    void		Reset(void);
    // Return short name of the record
    QString		GetRecordShortName(void);
    // Return long name of the record
    QString		GetRecordLongName(void);
    // Return record type
    int			GetRecordType(void);
    // Read record
    bool		Read(GS::StdLib::Stdf & clStdf);
    // Write record
    bool		Write(GS::StdLib::Stdf & clStdf);
    // Construct a stringlist with all fields to display
    // Each string in the list has following format: <Field name>;<Field value>
    void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
    // Construct a ASCII string of the record
    void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);
    // Construct a XML string of the record
    void		GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection = 0);
    // Construct a ATDF string of the record
    void		GetAtdfString(QString & strAtdfString);

    // Set members and flags
    void		SetSSR_NAM(const stdf_type_cn   cnSSR_NAM)	_FIELD_SET(m_cnSSR_NAM = cnSSR_NAM, true, eposSSR_NAM);
    void		SetCHN_CNT(const stdf_type_u2  u2CHN_CNT)	_FIELD_SET(m_u2CHN_CNT = u2CHN_CNT, true, eposCHN_CNT);

    void	SetCHN_LIST(const stdf_type_u2 u2Index, const stdf_type_u2 u2CHN_LIST)
    {
        if(m_u2CHN_CNT > 0)
        {
            if(m_pu2CHN_LIST == NULL)
                m_pu2CHN_LIST = new stdf_type_u2[m_u2CHN_CNT];
            if(u2Index < m_u2CHN_CNT)	_FIELD_SET(m_pu2CHN_LIST[u2Index] = u2CHN_LIST, true, eposCHN_LIST)
        }
    }

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposSSR_NAM        = 0,
            eposCHN_CNT    = 1,
            eposCHN_LIST  = 2,
            eposEND		 = 3,
            eposFIRST_OPTIONAL = 3
    };
    stdf_type_cn  m_cnSSR_NAM    ;//C*n
    stdf_type_u2  m_u2CHN_CNT    ;//U*2
    stdf_type_u2* m_pu2CHN_LIST ;//kxU*2

private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};


class Stdf_CDR_V4: public Stdf_Record
{
    Q_DISABLE_COPY(Stdf_CDR_V4)

// METHODS
public:
    // Constructor / destructor functions
    Stdf_CDR_V4();
    ~Stdf_CDR_V4();

    // Reset record data
    void		Reset(void);
    // Return short name of the record
    QString		GetRecordShortName(void);
    // Return long name of the record
    QString		GetRecordLongName(void);
    // Return record type
    int			GetRecordType(void);
    // Read record
    bool		Read(GS::StdLib::Stdf & clStdf);
    // Write record
    bool		Write(GS::StdLib::Stdf & clStdf);
    // Construct a stringlist with all fields to display
    // Each string in the list has following format: <Field name>;<Field value>
    void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
    // Construct a ASCII string of the record
    void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);
    // Construct a XML string of the record
    void		GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection = 0);
    // Construct a ATDF string of the record
    void		GetAtdfString(QString & strAtdfString);

    // Set members and flags
    void		SetCONT_FLG	(const stdf_type_b1 b1CONT_FLG	)	_FIELD_SET(m_b1CONT_FLG	=  b1CONT_FLG	 , true,    eposCONT_FLG );
    void		SetCDR_INDX	(const stdf_type_u2 u2CDR_INDX	)	_FIELD_SET(m_u2CDR_INDX	=  u2CDR_INDX	 , true,    eposCDR_INDX );
    void		SetCHN_NAM	(const stdf_type_cn cnCHN_NAM 	)	_FIELD_SET(m_cnCHN_NAM 	=  cnCHN_NAM 	 , true,    eposCHN_NAM  );
    void		SetCHN_LEN	(const stdf_type_u4 u4CHN_LEN  )	_FIELD_SET(m_u4CHN_LEN  =  u4CHN_LEN   , true,  eposCHN_LEN);
    void		SetSIN_PIN	(const stdf_type_u2 u2SIN_PIN  )	_FIELD_SET(m_u2SIN_PIN  =  u2SIN_PIN   , true,  eposSIN_PIN);
    void		SetSOUT_PIN	(const stdf_type_u2 u2SOUT_PIN )	_FIELD_SET(m_u2SOUT_PIN =  u2SOUT_PIN  , true,  eposSOUT_PIN);
    void		SetMSTR_CNT	(const stdf_type_u1 u1MSTR_CNT )	_FIELD_SET(m_u1MSTR_CNT =  u1MSTR_CNT  , true,  eposMSTR_CNT);

    void	SetM_CLKS(const stdf_type_u2 u2Index, const stdf_type_u2 u2M_CLKS)
    {
        if(m_u1MSTR_CNT > 0)
        {
            if(m_pu2M_CLKS == NULL)
                m_pu2M_CLKS = new stdf_type_u2[m_u1MSTR_CNT];
            if(u2Index < m_u1MSTR_CNT)	_FIELD_SET(m_pu2M_CLKS[u2Index] = u2M_CLKS, true, eposM_CLKS)
        }
    }

    void		SetSLAV_CNT	(const stdf_type_u1 u1SLAV_CNT )	_FIELD_SET(m_u1SLAV_CNT =  u1SLAV_CNT  , true,  eposSLAV_CNT);
    void	SetS_CLKS(const stdf_type_u2 u2Index, const stdf_type_u2 u2S_CLKS)
    {
        if(m_u1SLAV_CNT > 0)
        {
            if(m_pu2S_CLKS == NULL)
                m_pu2S_CLKS = new stdf_type_u2[m_u1SLAV_CNT];
            if(u2Index < m_u1SLAV_CNT)	_FIELD_SET(m_pu2S_CLKS[u2Index] = u2S_CLKS, true, eposS_CLKS)
        }
    }

    void		SetINV_VAL	(const stdf_type_u1 u1INV_VAL  )	_FIELD_SET(m_u1INV_VAL  =  u1INV_VAL   , true,  eposINV_VAL);

    void		SetLST_CNT	(const stdf_type_u2 u2LST_CNT  )	_FIELD_SET(m_u2LST_CNT  =  u2LST_CNT   , true,  eposLST_CNT);
    void	SetCELL_LST(const stdf_type_u2 u2Index, const stdf_type_cn cnCELL_LST)
    {
        if(m_u2LST_CNT > 0)
        {
            if(m_pcnCELL_LST == NULL)
                m_pcnCELL_LST = new stdf_type_cn[m_u2LST_CNT];
            if(u2Index < m_u2LST_CNT)	_FIELD_SET(m_pcnCELL_LST[u2Index] = cnCELL_LST, true, eposCELL_LST)
        }
    }

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposCONT_FLG    =0,
            eposCDR_INDX    =1,
            eposCHN_NAM	    =2,
            eposCHN_LEN	    =3,
            eposSIN_PIN	    =4,
            eposSOUT_PIN    =5,
            eposMSTR_CNT    =6,
            eposM_CLKS	    =7,
            eposSLAV_CNT    =8,
            eposS_CLKS	    =9,
            eposINV_VAL	    =10,
            eposLST_CNT	    =11,
            eposCELL_LST    =12,
            eposEND = 13,
            eposFIRST_OPTIONAL = 13
    };
    stdf_type_b1  m_b1CONT_FLG  ;//B*1
    stdf_type_u2  m_u2CDR_INDX  ;//U*2
    stdf_type_cn  m_cnCHN_NAM   ;//C*n
    stdf_type_u4  m_u4CHN_LEN   ;//U*4
    stdf_type_u2  m_u2SIN_PIN   ;//U*2
    stdf_type_u2  m_u2SOUT_PIN  ;//U*2
    stdf_type_u1  m_u1MSTR_CNT  ;//U*1
    stdf_type_u2* m_pu2M_CLKS   ;//mxU*2
    stdf_type_u1  m_u1SLAV_CNT  ;//U*1
    stdf_type_u2* m_pu2S_CLKS   ;//nxU*2
    stdf_type_u1  m_u1INV_VAL   ;//U*1
    stdf_type_u2  m_u2LST_CNT   ;//U*2
    stdf_type_cn* m_pcnCELL_LST ;//kxS*n

private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};

class Stdf_STR_V4: public Stdf_Record
{
    Q_DISABLE_COPY(Stdf_STR_V4)

// METHODS
public:
    // Constructor / destructor functions
    Stdf_STR_V4();
    ~Stdf_STR_V4();

    // Reset record data
    void		Reset(void);
    // Return short name of the record
    QString		GetRecordShortName(void);
    // Return long name of the record
    QString		GetRecordLongName(void);
    // Return record type
    int			GetRecordType(void);
    // Read record
    bool		Read(GS::StdLib::Stdf & clStdf);
    // Write record
    bool		Write(GS::StdLib::Stdf & clStdf);
    // Construct a stringlist with all fields to display
    // Each string in the list has following format: <Field name>;<Field value>
    void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
    // Construct a ASCII string of the record
    void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);
    // Construct a XML string of the record
    void		GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection = 0);
    // Construct a ATDF string of the record
    void		GetAtdfString(QString & strAtdfString);

    // Set members and flags
    void	SetCONT_FLG	(const	stdf_type_b1	b1CONT_FLG	)	 _FIELD_SET(	m_b1CONT_FLG	=	b1CONT_FLG	,true,	eposCONT_FLG	);
    void	SetTEST_NUM	(const	stdf_type_u4	u4TEST_NUM	)	 _FIELD_SET(	m_u4TEST_NUM	=	u4TEST_NUM	,true,	eposTEST_NUM	);
    void	SetHEAD_NUM	(const	stdf_type_u1	u1HEAD_NUM	)	 _FIELD_SET(	m_u1HEAD_NUM	=	u1HEAD_NUM	,true,	eposHEAD_NUM	);
    void	SetSITE_NUM	(const	stdf_type_u1	u1SITE_NUM	)	 _FIELD_SET(	m_u1SITE_NUM	=	u1SITE_NUM	,true,	eposSITE_NUM	);
    void	SetPSR_REF	(const	stdf_type_u2	u2PSR_REF	)	 _FIELD_SET(	m_u2PSR_REF	=	u2PSR_REF	,true,	eposPSR_REF	);
    void	SetTEST_FLG	(const	stdf_type_b1	b1TEST_FLG	)	 _FIELD_SET(	m_b1TEST_FLG	=	b1TEST_FLG	,true,	eposTEST_FLG	);
    void	SetLOG_TYP	(const	stdf_type_cn	cnLOG_TYP	)	 _FIELD_SET(	m_cnLOG_TYP	=	cnLOG_TYP	,true,	eposLOG_TYP	);
    void	SetTEST_TXT	(const	stdf_type_cn	cnTEST_TXT	)	 _FIELD_SET(	m_cnTEST_TXT	=	cnTEST_TXT	,true,	eposTEST_TXT	);
    void	SetALARM_ID	(const	stdf_type_cn	cnALARM_ID	)	 _FIELD_SET(	m_cnALARM_ID	=	cnALARM_ID	,true,	eposALARM_ID	);
    void	SetPROG_TXT	(const	stdf_type_cn	cnPROG_TXT	)	 _FIELD_SET(	m_cnPROG_TXT	=	cnPROG_TXT	,true,	eposPROG_TXT	);
    void	SetRSLT_TXT	(const	stdf_type_cn	cnRSLT_TXT	)	 _FIELD_SET(	m_cnRSLT_TXT	=	cnRSLT_TXT	,true,	eposRSLT_TXT	);
    void	SetZ_VAL	(const	stdf_type_u1	u1Z_VAL	)        _FIELD_SET(	m_u1Z_VAL	=	u1Z_VAL	,true,	eposZ_VAL	);
    void	SetFMU_FLG	(const	stdf_type_b1	b1FMU_FLG	)	 _FIELD_SET(	m_b1FMU_FLG	=	b1FMU_FLG	,true,	eposFMU_FLG	);
    void	SetMASK_MAP	(const	stdf_type_dn	dnMASK_MAP	)	 _FIELD_SET(	m_dnMASK_MAP	=	dnMASK_MAP, dnMASK_MAP.m_uiLength != 0,	eposMASK_MAP);
    void	SetFAL_MAP	(const	stdf_type_dn	dnFAL_MAP	)	 _FIELD_SET(	m_dnFAL_MAP	=	dnFAL_MAP, dnFAL_MAP.m_uiLength != 0,	eposFAL_MAP	);
    void	SetCYC_CNT	(const	stdf_type_u8	u8CYC_CNT	)	 _FIELD_SET(	m_u8CYC_CNT	=	u8CYC_CNT	,true,	eposCYC_CNT	);
    void	SetTOTF_CNT	(const	stdf_type_u4	u4TOTF_CNT	)	 _FIELD_SET(	m_u4TOTF_CNT	=	u4TOTF_CNT	,true,	eposTOTF_CNT	);
    void	SetTOTL_CNT	(const	stdf_type_u4	u4TOTL_CNT	)	 _FIELD_SET(	m_u4TOTL_CNT	=	u4TOTL_CNT	,true,	eposTOTL_CNT	);
    void	SetCYC_BASE	(const	stdf_type_u8	u8CYC_BASE	)	 _FIELD_SET(	m_u8CYC_BASE	=	u8CYC_BASE	,true,	eposCYC_BASE	);
    void	SetBIT_BASE	(const	stdf_type_u4	u4BIT_BASE	)	 _FIELD_SET(	m_u4BIT_BASE	=	u4BIT_BASE	,true,	eposBIT_BASE	);
    void	SetCOND_CNT	(const	stdf_type_u2	u2COND_CNT	)	 _FIELD_SET(	m_u2COND_CNT	=	u2COND_CNT	,true,	eposCOND_CNT	);
    void	SetLIM_CNT	(const	stdf_type_u2	u2LIM_CNT	)	 _FIELD_SET(	m_u2LIM_CNT	=	u2LIM_CNT	,true,	eposLIM_CNT	);
    void	SetCYC_SIZE	(const	stdf_type_u1	u1CYC_SIZE	)	 _FIELD_SET(	m_u1CYC_SIZE	=	u1CYC_SIZE	,true,	eposCYC_SIZE	);
    void	SetPMR_SIZE	(const	stdf_type_u1	u1PMR_SIZE	)	 _FIELD_SET(	m_u1PMR_SIZE	=	u1PMR_SIZE	,true,	eposPMR_SIZE	);
    void	SetCHN_SIZE	(const	stdf_type_u1	u1CHN_SIZE	)	 _FIELD_SET(	m_u1CHN_SIZE	=	u1CHN_SIZE	,true,	eposCHN_SIZE	);
    void	SetPAT_SIZE	(const	stdf_type_u1	u1PAT_SIZE	)	 _FIELD_SET(	m_u1PAT_SIZE	=	u1PAT_SIZE	,true,	eposPAT_SIZE	);
    void	SetBIT_SIZE	(const	stdf_type_u1	u1BIT_SIZE	)	 _FIELD_SET(	m_u1BIT_SIZE	=	u1BIT_SIZE	,true,	eposBIT_SIZE	);
    void	SetU1_SIZE	(const	stdf_type_u1	u1U1_SIZE	)	 _FIELD_SET(	m_u1U1_SIZE	=	u1U1_SIZE	,true,	eposU1_SIZE	);
    void	SetU2_SIZE	(const	stdf_type_u1	u1U2_SIZE	)	 _FIELD_SET(	m_u1U2_SIZE	=	u1U2_SIZE	,true,	eposU2_SIZE	);
    void	SetU3_SIZE	(const	stdf_type_u1	u1U3_SIZE	)	 _FIELD_SET(	m_u1U3_SIZE	=	u1U3_SIZE	,true,	eposU3_SIZE	);
    void	SetUTX_SIZE	(const	stdf_type_u1	u1UTX_SIZE	)	 _FIELD_SET(	m_u1UTX_SIZE	=	u1UTX_SIZE	,true,	eposUTX_SIZE	);
    void	SetCAP_BGN	(const	stdf_type_u2	u2CAP_BGN	)	 _FIELD_SET(	m_u2CAP_BGN	=	u2CAP_BGN	,true,	eposCAP_BGN	);


    void	SetLIM_INDX(const stdf_type_u2 u2Index, const stdf_type_u2 u2CELL_LST)
    {
        if(m_u2LIM_CNT > 0)
        {
            if(m_pu2LIM_INDX == NULL)
                m_pu2LIM_INDX = new stdf_type_u2[m_u2LIM_CNT];
            if(u2Index < m_u2LIM_CNT)	_FIELD_SET(m_pu2LIM_INDX[u2Index] = u2CELL_LST, true, eposLIM_INDX)
        }
    }
    void	SetLIM_SPEC(const stdf_type_u2 u2Index, const stdf_type_u4 u4LIM_SPEC)
    {
        if(m_u2LIM_CNT > 0)
        {
            if(m_pu4LIM_SPEC == NULL)
                m_pu4LIM_SPEC = new stdf_type_u4[m_u2LIM_CNT];
            if(u2Index < m_u2LIM_CNT)	_FIELD_SET(m_pu4LIM_SPEC[u2Index] = u4LIM_SPEC, true, eposLIM_SPEC)
        }
    }
    void	SetCOND_LST(const stdf_type_u2 u2Index, const stdf_type_cn cnCOND_LST)
    {
        if(m_u2COND_CNT > 0)
        {
            if(m_pcnCOND_LST == NULL)
                m_pcnCOND_LST = new stdf_type_cn[m_u2COND_CNT];
            if(u2Index < m_u2COND_CNT)	_FIELD_SET(m_pcnCOND_LST[u2Index] = cnCOND_LST, true, eposCOND_LST)
        }
    }

    void	SetCYCO_CNT	(const	stdf_type_u2	u2CYCO_CNT	)	 _FIELD_SET(	m_u2CYCO_CNT	=	u2CYCO_CNT	,true,	eposCYCO_CNT	);
    void	SetCYC_OFST(const stdf_type_u2 u2Index, const stdf_type_u8 u8CYC_OFST)
    {
        if(m_u2CYCO_CNT > 0)
        {
            if(m_pu8CYC_OFST == NULL)
                m_pu8CYC_OFST = new stdf_type_u8[m_u2CYCO_CNT];
            if(u2Index < m_u2CYCO_CNT)	_FIELD_SET(m_pu8CYC_OFST[u2Index] = u8CYC_OFST, true, eposCYC_OFST)
        }
    }


    void	SetPMR_CNT	(const	stdf_type_u2	u2PMR_CNT	)	 _FIELD_SET(	m_u2PMR_CNT	=	u2PMR_CNT	,true,	eposPMR_CNT	);
    void	SetPMR_INDX(const stdf_type_u2 u2Index, const stdf_type_u8 u8PMR_INDX)
    {
        if(m_u2PMR_CNT > 0)
        {
            if(m_pu8PMR_INDX == NULL)
                m_pu8PMR_INDX = new stdf_type_u8[m_u2PMR_CNT];
            if(u2Index < m_u2PMR_CNT)	_FIELD_SET(m_pu8PMR_INDX[u2Index] = u8PMR_INDX, true, eposPMR_INDX)
        }
    }

    void	SetCHN_CNT	(const	stdf_type_u2	u2CHN_CNT	)	 _FIELD_SET(	m_u2CHN_CNT	=	u2CHN_CNT	,true,	eposCHN_CNT	);
    void	SetCHN_NUM(const stdf_type_u2 u2Index, const stdf_type_u8 u8CHN_NUM)
    {
        if(m_u2CHN_CNT > 0)
        {
            if(m_pu8CHN_NUM == NULL)
                m_pu8CHN_NUM = new stdf_type_u8[m_u2CHN_CNT];
            if(u2Index < m_u2CHN_CNT)	_FIELD_SET(m_pu8CHN_NUM[u2Index] = u8CHN_NUM, true, eposCHN_NUM)
        }
    }

    void	SetEXP_CNT	(const	stdf_type_u2	u2EXP_CNT	)	 _FIELD_SET(	m_u2EXP_CNT	=	u2EXP_CNT	,true,	eposEXP_CNT	);
    void	SetEXP_DATA(const stdf_type_u2 u2Index, const stdf_type_u1 u1EXP_DATA)
    {
        if(m_u2CHN_CNT > 0)
        {
            if(m_pu1EXP_DATA == NULL)
                m_pu1EXP_DATA = new stdf_type_u1[m_u2CHN_CNT];
            if(u2Index < m_u2CHN_CNT)	_FIELD_SET(m_pu1EXP_DATA[u2Index] = u1EXP_DATA, true, eposEXP_DATA)
        }
    }

    void	SetCAP_CNT	(const	stdf_type_u2	u2CAP_CNT	)	 _FIELD_SET(	m_u2CAP_CNT	=	u2CAP_CNT	,true,	eposCAP_CNT	);
    void	SetCAP_DATA(const stdf_type_u2 u2Index, const stdf_type_u1 u1CAP_DATA)
    {
        if(m_u2CAP_CNT > 0)
        {
            if(m_pu1CAP_DATA == NULL)
                m_pu1CAP_DATA = new stdf_type_u1[m_u2CAP_CNT];
            if(u2Index < m_u2CAP_CNT)	_FIELD_SET(m_pu1CAP_DATA[u2Index] = u1CAP_DATA, true, eposCAP_DATA)
        }
    }

    void	SetNEW_CNT	(const	stdf_type_u2	u2NEW_CNT	)	 _FIELD_SET(	m_u2NEW_CNT	=	u2NEW_CNT	,true,	eposNEW_CNT	);
    void	SetNEW_DATA(const stdf_type_u2 u2Index, const stdf_type_u1 u1NEW_DATA)
    {
        if(m_u2NEW_CNT > 0)
        {
            if(m_pu1NEW_DATA == NULL)
                m_pu1NEW_DATA = new stdf_type_u1[m_u2NEW_CNT];
            if(u2Index < m_u2NEW_CNT)	_FIELD_SET(m_pu1NEW_DATA[u2Index] = u1NEW_DATA, true, eposNEW_DATA)
        }
    }

    void	SetPAT_CNT	(const	stdf_type_u2	u2PAT_CNT	)	 _FIELD_SET(	m_u2PAT_CNT	=	u2PAT_CNT	,true,	eposPAT_CNT	);
    void	SetPAT_NUM(const stdf_type_u2 u2Index, const stdf_type_u8 u8PAT_NUM)
    {
        if(m_u2PAT_CNT > 0)
        {
            if(m_pu8PAT_NUM == NULL)
                m_pu8PAT_NUM = new stdf_type_u8[m_u2PAT_CNT];
            if(u2Index < m_u2PAT_CNT)	_FIELD_SET(m_pu8PAT_NUM[u2Index] = u8PAT_NUM, true, eposPAT_NUM)
        }
    }

    void	SetBPOS_CNT	(const	stdf_type_u2	u2BPOS_CNT	)	 _FIELD_SET(	m_u2BPOS_CNT	=	u2BPOS_CNT	,true,	eposBPOS_CNT	);
    void	SetBIT_POS(const stdf_type_u2 u2Index, const stdf_type_u8 u8BIT_POS)
    {
        if(m_u2BPOS_CNT > 0)
        {
            if(m_pu8BIT_POS == NULL)
                m_pu8BIT_POS = new stdf_type_u8[m_u2BPOS_CNT];
            if(u2Index < m_u2BPOS_CNT)	_FIELD_SET(m_pu8BIT_POS[u2Index] = u8BIT_POS, true, eposBIT_POS)
        }
    }

    void	SetUSR1_CNT	(const	stdf_type_u2	u2USR1_CNT	)	 _FIELD_SET(	m_u2USR1_CNT	=	u2USR1_CNT	,true,	eposUSR1_CNT	);
    void	SetUSR1(const stdf_type_u2 u2Index, const stdf_type_u8 u8USR1)
    {
        if(m_u2USR1_CNT > 0)
        {
            if(m_pu8USR1 == NULL)
                m_pu8USR1 = new stdf_type_u8[m_u2USR1_CNT];
            if(u2Index < m_u2USR1_CNT)	_FIELD_SET(m_pu8USR1[u2Index] = u8USR1, true, eposUSR1)
        }
    }
    void	SetUSR2_CNT	(const	stdf_type_u2	u2USR2_CNT	)	 _FIELD_SET(	m_u2USR2_CNT	=	u2USR2_CNT	,true,	eposUSR2_CNT	);
    void	SetUSR2(const stdf_type_u2 u2Index, const stdf_type_u8 u8USR2)
    {
        if(m_u2USR2_CNT > 0)
        {
            if(m_pu8USR2 == NULL)
                m_pu8USR2 = new stdf_type_u8[m_u2USR2_CNT];
            if(u2Index < m_u2USR2_CNT)	_FIELD_SET(m_pu8USR2[u2Index] = u8USR2, true, eposUSR2)
        }
    }

    void	SetUSR3_CNT	(const	stdf_type_u2	u2USR3_CNT	)	 _FIELD_SET(	m_u2USR3_CNT	=	u2USR3_CNT	,true,	eposUSR3_CNT	);
    void	SetUSR3(const stdf_type_u2 u2Index, const stdf_type_u8 u8USR3)
    {
        if(m_u2USR3_CNT > 0)
        {
            if(m_pu8USR3 == NULL)
                m_pu8USR3 = new stdf_type_u8[m_u2USR3_CNT];
            if(u2Index < m_u2USR3_CNT)	_FIELD_SET(m_pu8USR3[u2Index] = u8USR3, true, eposUSR3)
        }
    }

    void	SetTXT_CNT	(const	stdf_type_u2	u2TXT_CNT	)	 _FIELD_SET(	m_u2TXT_CNT	=	u2TXT_CNT	,true,	eposTXT_CNT	);
    void	SetUSR_TXT(const stdf_type_u2 u2Index, const stdf_type_cn cnUSR_TXT)
    {
        if(m_u2TXT_CNT > 0)
        {
            if(m_pcnUSER_TXT == NULL)
                m_pcnUSER_TXT = new stdf_type_cn[m_u2TXT_CNT];
            if(u2Index < m_u2TXT_CNT)	_FIELD_SET(m_pcnUSER_TXT[u2Index] = cnUSR_TXT, true, eposUSER_TXT)
        }
    }

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposCONT_FLG	=	0	,
            eposTEST_NUM	=	1	,
            eposHEAD_NUM	=	2	,
            eposSITE_NUM	=	3	,
            eposPSR_REF	=	4	,
            eposTEST_FLG	=	5	,
            eposLOG_TYP	=	6	,
            eposTEST_TXT	=	7	,
            eposALARM_ID	=	8	,
            eposPROG_TXT	=	9	,
            eposRSLT_TXT	=	10	,
            eposZ_VAL	=	11	,
            eposFMU_FLG	=	12	,
            eposMASK_MAP	=	13	,
            eposFAL_MAP	=	14	,
            eposCYC_CNT	=	15	,
            eposTOTF_CNT	=	16	,
            eposTOTL_CNT	=	17	,
            eposCYC_BASE	=	18	,
            eposBIT_BASE	=	19	,
            eposCOND_CNT	=	20	,
            eposLIM_CNT	=	21	,
            eposCYC_SIZE	=	22	,
            eposFIRST_OPTIONAL	=	22,
            eposPMR_SIZE	=	23	,
            eposCHN_SIZE	=	24	,
            eposPAT_SIZE	=	25	,
            eposBIT_SIZE	=	26	,
            eposU1_SIZE	=	27	,
            eposU2_SIZE	=	28	,
            eposU3_SIZE	=	29	,
            eposUTX_SIZE	=	30	,
            eposCAP_BGN	=	31	,
            eposLIM_INDX	=	32	,
            eposLIM_SPEC	=	33	,
            eposCOND_LST	=	34	,
            eposCYCO_CNT	=	35	,
            eposCYC_OFST	=	36	,
            eposPMR_CNT	=	37	,
            eposPMR_INDX	=	38	,
            eposCHN_CNT	=	39	,
            eposCHN_NUM	=	40	,
            eposEXP_CNT	=	41	,
            eposEXP_DATA	=	42	,
            eposCAP_CNT	=	43	,
            eposCAP_DATA	=	44	,
            eposNEW_CNT	=	45	,
            eposNEW_DATA	=	46	,
            eposPAT_CNT	=	47	,
            eposPAT_NUM	=	48	,
            eposBPOS_CNT	=	49	,
            eposBIT_POS	=	50	,
            eposUSR1_CNT	=	51	,
            eposUSR1	=	52	,
            eposUSR2_CNT	=	53	,
            eposUSR2	=	54	,
            eposUSR3_CNT	=	55	,
            eposUSR3	=	56	,
            eposTXT_CNT	=	57	,
            eposUSER_TXT	=	58	,
            eposEND	=	59
    };

    stdf_type_b1	m_b1CONT_FLG	;	//B*1
    stdf_type_u4	m_u4TEST_NUM	;	//U*4
    stdf_type_u1	m_u1HEAD_NUM	;	//U*1
    stdf_type_u1	m_u1SITE_NUM	;	//U*1
    stdf_type_u2	m_u2PSR_REF	;	//U*2
    stdf_type_b1	m_b1TEST_FLG	;	//B*1
    stdf_type_cn	m_cnLOG_TYP	;	//C*n
    stdf_type_cn	m_cnTEST_TXT	;	//C*n
    stdf_type_cn	m_cnALARM_ID	;	//C*n
    stdf_type_cn	m_cnPROG_TXT	;	//C*n
    stdf_type_cn	m_cnRSLT_TXT	;	//C*n
    stdf_type_u1	m_u1Z_VAL	;	//U*1
    stdf_type_b1	m_b1FMU_FLG	;	//B*1
    stdf_type_dn	m_dnMASK_MAP	;	//D*n
    stdf_type_dn	m_dnFAL_MAP	;	//D*n
    stdf_type_u8	m_u8CYC_CNT	;	//U*8
    stdf_type_u4	m_u4TOTF_CNT	;	//U*4
    stdf_type_u4	m_u4TOTL_CNT	;	//U*4
    stdf_type_u8	m_u8CYC_BASE	;	//U*8
    stdf_type_u4	m_u4BIT_BASE	;	//U*4
    stdf_type_u2	m_u2COND_CNT	;	//U*2
    stdf_type_u2	m_u2LIM_CNT	;	//U*2
    stdf_type_u1	m_u1CYC_SIZE	;	//U*1
    stdf_type_u1	m_u1PMR_SIZE	;	//U*1
    stdf_type_u1	m_u1CHN_SIZE	;	//U*1
    stdf_type_u1	m_u1PAT_SIZE	;	//U*1
    stdf_type_u1	m_u1BIT_SIZE	;	//U*1
    stdf_type_u1	m_u1U1_SIZE	;	//U*1
    stdf_type_u1	m_u1U2_SIZE	;	//U*1
    stdf_type_u1	m_u1U3_SIZE	;	//U*1
    stdf_type_u1	m_u1UTX_SIZE	;	//U*1
    stdf_type_u2	m_u2CAP_BGN	;	//U*2
    stdf_type_u2*	m_pu2LIM_INDX	;	//jxU*2
    stdf_type_u4*	m_pu4LIM_SPEC	;	//jxU*4
    stdf_type_cn*	m_pcnCOND_LST	;	//gxC*n
    stdf_type_u2	m_u2CYCO_CNT	;	//U*2
    stdf_type_u8*	m_pu8CYC_OFST	;	//kxU*f
    stdf_type_u2	m_u2PMR_CNT	;	//U*2
    stdf_type_u8*	m_pu8PMR_INDX	;	//kxU*f
    stdf_type_u2	m_u2CHN_CNT	;	//U*2
    stdf_type_u8*	m_pu8CHN_NUM	;	//kxU*f
    stdf_type_u2	m_u2EXP_CNT	;	//U*2
    stdf_type_u1*	m_pu1EXP_DATA	;	//mxU*1
    stdf_type_u2	m_u2CAP_CNT	;	//U*2
    stdf_type_u1*	m_pu1CAP_DATA	;	//kxU*1
    stdf_type_u2	m_u2NEW_CNT	;	//U*2
    stdf_type_u1*	m_pu1NEW_DATA	;	//kxU*1
    stdf_type_u2	m_u2PAT_CNT	;	//U*2
    stdf_type_u8*	m_pu8PAT_NUM	;	//kxU*f
    stdf_type_u2	m_u2BPOS_CNT	;	//U*2
    stdf_type_u8*	m_pu8BIT_POS	;	//kxU*f
    stdf_type_u2	m_u2USR1_CNT	;	//U*2
    stdf_type_u8*	m_pu8USR1	;	//kxU*f
    stdf_type_u2	m_u2USR2_CNT	;	//U*2
    stdf_type_u8*	m_pu8USR2	;	//kxU*f
    stdf_type_u2	m_u2USR3_CNT	;	//U*2
    stdf_type_u8*	m_pu8USR3	;	//kxU*f
    stdf_type_u2	m_u2TXT_CNT	;	//U*2
    stdf_type_cn*	m_pcnUSER_TXT	;	//kxC*f


private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};

}

#endif // #ifndef _StdfRecords_V4_h_
