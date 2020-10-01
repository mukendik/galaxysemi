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
// Stdf_XXX_V3 class HEADER :
// This file contains classes to read and store STDF V3
// records
///////////////////////////////////////////////////////////
#ifndef STDFRECORDS_V3_H
#define STDFRECORDS_V3_H


// Galaxy modules includes
#include "stdf.h"
#include "stdfrecord.h"
#include <QStringList>
#include <QString>


namespace GQTL_STDF
{
///////////////////////////////////////////////////////////
// FAR RECORD
///////////////////////////////////////////////////////////
class Stdf_FAR_V3: public Stdf_Record
{
// METHODS
public:
    // Constructor / destructor functions
    Stdf_FAR_V3();
    ~Stdf_FAR_V3();

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

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

    /// \brief Check if the field is present
    /// \param positon of the field
    /// \return true if field is present
    bool		IsFieldPresent(int nFieldPos);

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
// MIR RECORD
///////////////////////////////////////////////////////////
class Stdf_MIR_V3: public Stdf_Record
{
// METHODS
public:
    // Constructor / destructor functions
    Stdf_MIR_V3();
    ~Stdf_MIR_V3();

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

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

    /// \brief Check if the field is present
    /// \param positon of the field
    /// \return true if field is present
    bool		IsFieldPresent(int nFieldPos);

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposCPU_TYPE            = 0,
            eposSTDF_VER			= 1,
            eposMODE_COD			= 2,
            eposSTAT_NUM			= 3,
            eposTEST_COD			= 4,
            eposFIRST_OPTIONAL		= 4,
            eposRTST_COD			= 5,
            eposPROT_COD			= 6,
            eposCMOD_COD			= 7,
            eposSETUP_T				= 8,
            eposSTART_T				= 9,
            eposLOT_ID				= 10,
            eposPART_TYP			= 11,
            eposJOB_NAM				= 12,
            eposOPER_NAM			= 13,
            eposNODE_NAM			= 14,
            eposTSTR_TYP			= 15,
            eposEXEC_TYP			= 16,
            eposSUPR_NAM			= 17,
            eposHAND_ID             = 18,
            eposSBLOT_ID			= 19,
            eposJOB_REV				= 20,
            eposPROC_ID				= 21,
            eposPRB_CARD			= 22,
            eposEND					= 23
    };

    stdf_type_u1	m_u1CPU_TYPE;		// MIR.CPU_TYPE
    stdf_type_u1	m_u1STDF_VER;		// MIR.STDF_VER
    time_t			m_u4SETUP_T;		// MIR.SETUP_T
    time_t			m_u4START_T;		// MIR.START_T
    stdf_type_u1	m_u1STAT_NUM;		// MIR.STAT_NUM
    stdf_type_c1	m_c1MODE_COD;		// MIR.MODE_COD
    stdf_type_c1	m_c1RTST_COD;		// MIR.RTST_COD
    stdf_type_c1	m_c1PROT_COD;		// MIR.PROT_COD
    stdf_type_c1	m_c1CMOD_COD;		// MIR.CMOD_COD
    stdf_type_cn	m_cnLOT_ID;			// MIR.LOT_ID
    stdf_type_cn	m_cnPART_TYP;		// MIR.PART_TYP
    stdf_type_cn	m_cnNODE_NAM;		// MIR.NODE_NAM
    stdf_type_cn	m_cnTSTR_TYP;		// MIR.TSTR_TYP
    stdf_type_cn	m_cnJOB_NAM;		// MIR.JOB_NAM
    stdf_type_cn	m_cnJOB_REV;		// MIR.JOB_REV
    stdf_type_cn	m_cnHAND_ID;		// MIR.HAND_ID
    stdf_type_cn	m_cnSBLOT_ID;		// MIR.SBLOT_ID
    stdf_type_cn	m_cnOPER_NAM;		// MIR.OPER_NAM
    stdf_type_cn	m_cnEXEC_TYP;		// MIR.EXEC_TYP
    stdf_type_cn	m_cnTEST_COD;		// MIR.TEST_COD
    stdf_type_cn	m_cnPROC_ID;		// MIR.PROC_ID
    stdf_type_cn	m_cnSUPR_NAM;		// MIR.SUPR_NAM
    stdf_type_cn	m_cnPRB_CARD;		// MIR.PRB_CARD

private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};

///////////////////////////////////////////////////////////
// MRR RECORD
///////////////////////////////////////////////////////////
class Stdf_MRR_V3: public Stdf_Record
{
// METHODS
public:
    // Constructor / destructor functions
    Stdf_MRR_V3();
    ~Stdf_MRR_V3();

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

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

    /// \brief Check if the field is present
    /// \param positon of the field
    /// \return true if field is present
    bool		IsFieldPresent(int nFieldPos);

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposFINISH_T			= 0,
            eposPART_CNT			= 1,
            eposRTST_CNT			= 2,
            eposFIRST_OPTIONAL		= 2,
            eposABRT_CNT			= 3,
            eposGOOD_CNT			= 4,
            eposFUNC_CNT			= 5,
            eposDISP_COD			= 6,
            eposUSR_DESC			= 7,
            eposEXC_DESC			= 8,
            eposEND					= 9
    };

    time_t			m_u4FINISH_T;		// MRR.FINISH_T
    stdf_type_u4	m_u4PART_CNT;		// MRR.PART_CNT
    stdf_type_i4	m_i4RTST_CNT;		// MRR.RTST_CNT
    stdf_type_i4	m_i4ABRT_CNT;		// MRR.ABRT_CNT
    stdf_type_i4	m_i4GOOD_CNT;		// MRR.GOOD_CNT
    stdf_type_i4	m_i4FUNC_CNT;		// MRR.FUNC_CNT
    stdf_type_c1	m_c1DISP_COD;		// MRR.DISP_COD
    stdf_type_cn	m_cnUSR_DESC;		// MRR.USR_DESC
    stdf_type_cn	m_cnEXC_DESC;		// MRR.EXC_DESC

private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};


///////////////////////////////////////////////////////////
// WIR RECORD
///////////////////////////////////////////////////////////
class Stdf_WIR_V3: public Stdf_Record
{
// METHODS
public:
    // Constructor / destructor functions
    Stdf_WIR_V3();
    ~Stdf_WIR_V3();

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

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

    /// \brief Check if the field is present
    /// \param positon of the field
    /// \return true if field is present
    bool		IsFieldPresent(int nFieldPos);

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposHEAD_NUM			= 0,		// WIR.HEAD_NUM
            eposPAD_BYTE			= 1,		// WIR.PAD_BYTE
            eposSTART_T				= 2,		// WIR.START_T
            eposWAFER_ID			= 3,		// WIR.WAFER_ID
            eposFIRST_OPTIONAL		= 3,
            eposEND					= 4
    };

    stdf_type_u1	m_u1HEAD_NUM;		// WIR.HEAD_NUM
    stdf_type_b1	m_b1PAD_BYTE;		// WIR.PAD_BYTE
    stdf_type_u4	m_u4START_T;		// WIR.START_T
    stdf_type_cn	m_cnWAFER_ID;		// WIR.WAFER_ID

private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};

///////////////////////////////////////////////////////////
// WRR RECORD
///////////////////////////////////////////////////////////
class Stdf_WRR_V3: public Stdf_Record
{
// METHODS
public:
    // Constructor / destructor functions
    Stdf_WRR_V3();
    ~Stdf_WRR_V3();

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

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid
    void        InvalidateField(int nFieldPos);

    /// \brief Check if the field is present
    /// \param positon of the field
    /// \return true if field is present
    bool		IsFieldPresent(int nFieldPos);

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposFINISH_T			= 0,		// WRR.FINISH_T
            eposHEAD_NUM			= 1,		// WRR.HEAD_NUM
            eposPAD_BYTE			= 2,		// WIR.PAD_BYTE
            eposPART_CNT			= 3,		// WRR.PART_CNT
            eposRTST_CNT			= 4,		// WRR.RTST_CNT
            eposFIRST_OPTIONAL		= 4,
            eposABRT_CNT			= 5,		// WRR.ABRT_CNT
            eposGOOD_CNT			= 6,		// WRR.GOOD_CNT
            eposFUNC_CNT			= 7,		// WRR.FUNC_CNT
            eposWAFER_ID			= 8,		// WRR.WAFER_ID
            eposHAND_ID             = 9,		// WRR.HAND_ID
            eposPRB_CARD			= 10,		// WRR.PRB_CARD
            eposUSR_DESC			= 11,		// WRR.USR_DESC
            eposEXC_DESC			= 12,		// WRR.EXC_DESC
            eposEND					= 13
    };

    stdf_type_u1	m_u1HEAD_NUM;		// WRR.HEAD_NUM
    stdf_type_b1	m_b1PAD_BYTE;		// WRR.PAD_BYTE
    stdf_type_u4	m_u4FINISH_T;		// WRR.FINISH_T
    stdf_type_u4	m_u4PART_CNT;		// WRR.PART_CNT
    stdf_type_i4	m_i4RTST_CNT;		// WRR.RTST_CNT
    stdf_type_i4	m_i4ABRT_CNT;		// WRR.ABRT_CNT
    stdf_type_i4	m_i4GOOD_CNT;		// WRR.GOOD_CNT
    stdf_type_i4	m_i4FUNC_CNT;		// WRR.FUNC_CNT
    stdf_type_cn	m_cnWAFER_ID;		// WRR.WAFER_ID
    stdf_type_cn	m_cnHAND_ID;		// WRR.HAND_ID
    stdf_type_cn	m_cnPRB_CARD;		// WRR.PRB_CARD
    stdf_type_cn	m_cnUSR_DESC;		// WRR.USR_DESC
    stdf_type_cn	m_cnEXC_DESC;		// WRR.EXC_DESC

private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};

///////////////////////////////////////////////////////////
// WCR RECORD
///////////////////////////////////////////////////////////
class Stdf_WCR_V3: public Stdf_Record
{
// METHODS
public:
    // Constructor / destructor functions
    Stdf_WCR_V3();
    ~Stdf_WCR_V3();

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

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

    /// \brief Check if the field is present
    /// \param positon of the field
    /// \return true if field is present
    bool		IsFieldPresent(int nFieldPos);

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
class Stdf_PIR_V3: public Stdf_Record
{
// METHODS
public:
    // Constructor / destructor functions
    Stdf_PIR_V3();
    ~Stdf_PIR_V3();

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

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

    /// \brief Check if the field is present
    /// \param positon of the field
    /// \return true if field is present
    bool		IsFieldPresent(int nFieldPos);

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposHEAD_NUM			= 0,	// PIR.HEAD_NUM
            eposSITE_NUM			= 1,	// PIR.SITE_NUM
            eposX_COORD 			= 2,	// PIR.X_COORD
            eposFIRST_OPTIONAL		= 2,
            eposY_COORD 			= 3,	// PIR.Y_COORD
            eposPART_ID 			= 4,	// PIR.PART_ID
            eposEND					= 5
    };

    stdf_type_u1	m_u1HEAD_NUM;		// PIR.HEAD_NUM
    stdf_type_u1	m_u1SITE_NUM;		// PIR.SITE_NUM
    stdf_type_i2	m_i2X_COORD;		// PIR.X_COORD
    stdf_type_i2	m_i2Y_COORD;		// PIR.Y_COORD
    stdf_type_cn	m_cnPART_ID;		// PIR.PART_ID

private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};

///////////////////////////////////////////////////////////
// PRR RECORD
///////////////////////////////////////////////////////////
class Stdf_PRR_V3: public Stdf_Record
{
// METHODS
public:
    // Constructor / destructor functions
    Stdf_PRR_V3();
    ~Stdf_PRR_V3();

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

    // Construct a stringlist with all fields to display
    // Each string in the list has following format: <Field name>;<Field value>
    void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
    // Construct a ASCII string of the record
    void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

    /// \brief Check if the field is present
    /// \param positon of the field
    /// \return true if field is present
    bool		IsFieldPresent(int nFieldPos);

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposHEAD_NUM			= 0,	// PRR.HEAD_NUM
            eposSITE_NUM			= 1,	// PRR.SITE_NUM
            eposNUM_TEST			= 3,	// PRR.NUM_TEST
            eposHARD_BIN			= 4,	// PRR.HARD_BIN
            eposSOFT_BIN			= 5,	// PRR.SOFT_BIN
            eposFIRST_OPTIONAL		= 5,
            eposPART_FLG			= 2,	// PRR.PART_FLG
            eposPAD_BYTE			= 8,	// PRR.PAD_BYTE
            eposX_COORD				= 6,	// PRR.X_COORD
            eposY_COORD				= 7,	// PRR.Y_COORD
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
    stdf_type_b1	m_b1PAD_BYTE;		// PRR.PAD_BYTE
    stdf_type_cn	m_cnPART_ID;		// PRR.PART_ID
    stdf_type_cn	m_cnPART_TXT;		// PRR.PART_TXT
    stdf_type_bn	m_bnPART_FIX;		// PRR.PART_FIX

private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};

///////////////////////////////////////////////////////////
// PDR RECORD
///////////////////////////////////////////////////////////
class Stdf_PDR_V3: public Stdf_Record
{
// METHODS
public:

    // Constructor / destructor functions
    Stdf_PDR_V3();
    ~Stdf_PDR_V3();

    enum DESC_FLG	{	RESULTDECIMAL			= 0x0,	// See complete flags definition in STDF specification
                        RESULTOCTAL           	= 0x1,	// version 3, page 28
                        RESULTHEXADECIMAL     	= 0x2,
                        RESULTBINARY  			= 0x3,
                        LIMITDECIMAL			= 0x3,
                        LIMITSOCTAL           	= 0x4,
                        LIMITHEXADECIMAL     	= 0x8,
                        LIMITBINARY  			= 0xC
    };


    enum EOPT_FLAG	{	eNOTVALID_RES_SCAL	= 0x1,	// See complete flags definition in STDF specification
                        eRESERVED_BIT1		= 0x2,	// version 3, page 29
                        eNO_LOWSPEC_LIMIT	= 0x4,
                        eNO_HIGHSPEC_LIMIT	= 0x8,
                        eNO_LL_USEFIRSTPDR	= 0x10,
                        eNO_HL_USEFIRSTPDR	= 0x20,
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

    // Some test related functions
    bool		IsTestExecuted();								// Return true if test has been executed
    bool		IsTestFail();                                   // Return true if test is FAIL
    bool		IsTestFail(Stdf_PDR_V3 & clRefPDR);			// Return true if test is FAIL
    void		UpdatePassFailInfo(Stdf_PDR_V3 & clRefPDR);	// Update Pass/Fail flags in PDR according to limits from the reference PDR

    // Construct a stringlist with all fields to display
    // Each string in the list has following format: <Field name>;<Field value>
    void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
    // Construct a ASCII string of the record
    void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);

    // Check field validity
    bool		IsFieldValid(int nFieldPos);				// Return true if field is valid

    /// \brief Check if the field is present
    /// \param positon of the field
    /// \return true if field is present
    bool		IsFieldPresent(int nFieldPos);

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposTEST_NUM			= 0,	// PDR.TEST_NUM
            eposDESC_FLG			= 1,	// PDR.DESC_FLG
            eposOPT_FLAG			= 2,	// PDR.OPT_FLAG
            eposRES_SCAL			= 3,	// PDR.RES_SCAL
            eposUNITS				= 4,	// PDR.UNITS
            eposRES_LDIG			= 5,	// PDR.RES_LDIG
            eposFIRST_OPTIONAL		= 5,
            eposRES_RDIG			= 6,	// PDR.RES_RDIG
            eposLLM_SCAL			= 7,	// PDR.LLM_SCAL
            eposHLM_SCAL			= 8,	// PDR.HLM_SCAL
            eposLLM_LDIG			= 9,	// PDR.LLM_LDIG
            eposLLM_RDIG			= 10,	// PDR.LLM_SCAL
            eposHLM_LDIG			= 11,	// PDR.HLM_LDIG
            eposHLM_RDIG			= 12,	// PDR.HLM_RDIG
            eposLO_LIMIT			= 13,	// PDR.LO_LIMIT
            eposHI_LIMIT			= 14,	// PDR.HI_LIMIT
            eposTEST_NAM			= 15,	// PDR.TEST_NAM
            eposSEQ_NAME			= 16,	// PDR.SEQ_NAME
            eposEND					= 17
    };

    stdf_type_u4	m_u4TEST_NUM;		// PDR.TEST_NUM
    stdf_type_b1	m_b1OPT_FLAG;		// PDR.OPT_FLAG
    stdf_type_i1	m_i1RES_SCAL;		// PDR.RES_SCAL
    stdf_type_u1	m_u1RES_LDIG;		// PDR.RES_LDIG
    stdf_type_u1	m_u1RES_RDIG;		// PDR.RES_RDIG
    stdf_type_b1	m_b1DESC_FLG;		// PDR.DESC_FLG
    stdf_type_i1	m_i1LLM_SCAL;		// PDR.LLM_SCAL
    stdf_type_i1	m_i1HLM_SCAL;		// PDR.HLM_SCAL
    stdf_type_u1	m_u1LLM_LDIG;		// PDR.LLM_LDIG
    stdf_type_u1	m_u1LLM_RDIG;		// PDR.LLM_RDIG
    stdf_type_u1	m_u1HLM_LDIG;		// PDR.HLM_LDIG
    stdf_type_u1	m_u1HLM_RDIG;		// PDR.HLM_RDIG
    stdf_type_r4	m_r4LO_LIMIT;		// PDR.LO_LIMIT
    stdf_type_r4	m_r4HI_LIMIT;		// PDR.HI_LIMIT
    stdf_type_cn    m_cnUNITS;			// PDR.UNITS
    stdf_type_cn	m_cnTEST_NAM;		// PDR.TEST_NAM
    stdf_type_cn	m_cnSEQ_NAME;		// PDR.SEQ_NAME

private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};


///////////////////////////////////////////////////////////
// FDR RECORD
///////////////////////////////////////////////////////////
class Stdf_FDR_V3: public Stdf_Record
{
// METHODS
public:

    // Constructor / destructor functions
    Stdf_FDR_V3();
    ~Stdf_FDR_V3();

    enum DESC_FLG	{	DECIMAL				= 0x0,	// See complete flags definition in STDF specification
                        OCTAL           	= 0x1,	// version 3, page 30
                        HEXADECIMAL     	= 0x2,
                        BINARY  			= 0x3
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

    // Construct a stringlist with all fields to display
    // Each string in the list has following format: <Field name>;<Field value>
    void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
    // Construct a ASCII string of the record
    void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);

    // Check field validity
    bool		IsFieldValid(int nFieldPos);				// Return true if field is valid

    /// \brief Check if the field is present
    /// \param positon of the field
    /// \return true if field is present
    bool		IsFieldPresent(int nFieldPos);

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposTEST_NUM			= 0,	// FDR.TEST_NUM
            eposDESC_FLG			= 1,	// FDR.HEAD_NUM
            eposTEST_NAM			= 2,	// FDR.TEST_NAM
            eposFIRST_OPTIONAL		= 2,
            eposSEQ_NAME			= 3,	// FDR.SEQ_NAME
            eposEND					= 4
    };

    stdf_type_u4	m_u4TEST_NUM;		// FDR.TEST_NUM
    stdf_type_b1	m_b1DESC_FLG;		// FDR.DESC_FLG
    stdf_type_cn	m_cnTEST_NAM;		// FDR.TEST_NAM
    stdf_type_cn	m_cnSEQ_NAME;		// FDR.SEQ_NAME

private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};



///////////////////////////////////////////////////////////
// PTR RECORD
///////////////////////////////////////////////////////////
class Stdf_PTR_V3: public Stdf_Record
{
// METHODS
public:

    // Constructor / destructor functions
    Stdf_PTR_V3();
    ~Stdf_PTR_V3();

    enum TEST_FLG	{	eALARM				= 0x1,	// See complete flags definition in STDF specification
                        eNOTVALID_RESULT	= 0x2,	// version 3, page 33
                        eNOTRELIABLE_RESULT	= 0x4,
                        eTIMEOUT			= 0x8,
                        eTEST_NOTEXECUTED	= 0x10,
                        eTEST_ABORTED		= 0x20,
                        eNO_PASSFAIL		= 0x40,
                        eTEST_FAILED		= 0x80
    };

    enum PARM_FLG	{	eSCALE_ERROR		= 0x1,	// See complete flags definition in STDF specification
                        eDRIFT_ERROR		= 0x2,	// version 3, page 33
                        eOSCILLAT_DETECTED	= 0x4,
                        eOVER_HIGHLIMIT		= 0x8,
                        eUNDER_LOWLIMIT		= 0x10,
                        ePASS_ALTERNATE_LIM	= 0x20
    };

    enum OPT_FLAG	{	eNOTVALID_RES_SCAL	= 0x1,	// See complete flags definition in STDF specification
                        eRESERVED_BIT1		= 0x2,	// version 3, page 34
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

    // Some test related functions
    bool		IsTestExecuted();								// Return true if test has been executed
    bool		IsTestFail();                                   // Return true if test is FAIL
    bool		IsTestFail(Stdf_PTR_V3 & clRefPTR);			// Return true if test is FAIL
    void		UpdatePassFailInfo(Stdf_PTR_V3 & clRefPTR);	// Update Pass/Fail flags in PTR according to limits from the reference PTR


    // Construct a stringlist with all fields to display
    // Each string in the list has following format: <Field name>;<Field value>
    void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
    // Construct a ASCII string of the record
    void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);

    // Check field validity
    bool		IsFieldValid(int nFieldPos);				// Return true if field is valid

    /// \brief Check if the field is present
    /// \param positon of the field
    /// \return true if field is present
    bool		IsFieldPresent(int nFieldPos);

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
            eposOPT_FLAG			= 6,	// PTR.OPT_FLAG
            eposRES_SCAL			= 7,	// PTR.RES_SCAL
            eposRES_LDIG			= 8,	// PTR.RES_LDIG
            eposRES_RDIG			= 9,	// PTR.RES_RDIG
            eposDESC_FLG			= 10,	// PTR.DESC_FLG
            eposUNITS				= 11,	// PTR.UNITS
            eposLLM_SCAL			= 12,	// PTR.LLM_SCAL
            eposHLM_SCAL			= 13,	// PTR.HLM_SCAL
            eposLLM_LDIG			= 14,	// PTR.LLM_LDIG
            eposLLM_RDIG			= 15,	// PTR.LLM_SCAL
            eposHLM_LDIG			= 16,	// PTR.HLM_LDIG
            eposHLM_RDIG			= 17,	// PTR.HLM_RDIG
            eposLO_LIMIT			= 18,	// PTR.LO_LIMIT
            eposHI_LIMIT			= 19,	// PTR.HI_LIMIT
            eposTEST_NAM			= 20,	// PTR.TEST_NAM
            eposSEQ_NAME			= 21,	// PTR.SEQ_NAME
            eposTEST_TXT			= 22,	// PTR.TEST_TXT
            eposEND					= 23
    };

    stdf_type_u4	m_u4TEST_NUM;		// PTR.TEST_NUM
    stdf_type_u1	m_u1HEAD_NUM;		// PTR.HEAD_NUM
    stdf_type_u1	m_u1SITE_NUM;		// PTR.SITE_NUM
    stdf_type_b1	m_b1TEST_FLG;		// PTR.TEST_FLG
    stdf_type_b1	m_b1PARM_FLG;		// PTR.PARM_FLG
    stdf_type_r4	m_r4RESULT;			// PTR.RESULT
    bool			m_bRESULT_IsNAN;	// Set to true if PTR.RESULT is a NAN value
    stdf_type_b1	m_b1OPT_FLAG;		// PTR.OPT_FLAG
    stdf_type_i1	m_i1RES_SCAL;		// PTR.RES_SCAL
    stdf_type_u1	m_u1RES_LDIG;		// PTR.RES_LDIG
    stdf_type_u1	m_u1RES_RDIG;		// PTR.RES_RDIG
    stdf_type_b1	m_b1DESC_FLG;		// PTR.DESC_FLG
    stdf_type_i1	m_i1LLM_SCAL;		// PTR.LLM_SCAL
    stdf_type_i1	m_i1HLM_SCAL;		// PTR.HLM_SCAL
    stdf_type_u1	m_u1LLM_LDIG;		// PTR.LLM_LDIG
    stdf_type_u1	m_u1LLM_RDIG;		// PTR.LLM_RDIG
    stdf_type_u1	m_u1HLM_LDIG;		// PTR.HLM_LDIG
    stdf_type_u1	m_u1HLM_RDIG;		// PTR.HLM_RDIG
    stdf_type_r4	m_r4LO_LIMIT;		// PTR.LO_LIMIT
    stdf_type_r4	m_r4HI_LIMIT;		// PTR.HI_LIMIT
    stdf_type_cn	m_cnUNITS;			// PTR.UNITS
    stdf_type_cn	m_cnTEST_NAM;		// PTR.TEST_NAM
    stdf_type_cn	m_cnSEQ_NAME;		// PTR.SEQ_NAME
    stdf_type_cn	m_cnTEST_TXT;		// PTR.TEST_TXT

private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};

///////////////////////////////////////////////////////////
// FTR RECORD
///////////////////////////////////////////////////////////
class Stdf_FTR_V3: public Stdf_Record
{
// METHODS
public:
    // Constructor / destructor functions
    Stdf_FTR_V3();
    ~Stdf_FTR_V3();

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

    // Some test related functions
    bool		IsTestExecuted();					// Return true if test has been executed
    bool		IsTestFail();						// Return true if test is FAIL
    void        SetFilter(Stdf_FTR_V3 * pFilter);

    // Construct a stringlist with all fields to display
    // Each string in the list has following format: <Field name>;<Field value>
    void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
    // Construct a ASCII string of the record
    void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

    /// \brief Check if the field is present
    /// \param positon of the field
    /// \return true if field is present
    bool		IsFieldPresent(int nFieldPos);

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposTEST_NUM			= 0,	// FTR.TEST_NUM
            eposHEAD_NUM			= 1,	// FTR.HEAD_NUM
            eposSITE_NUM			= 2,	// FTR.SITE_NUM
            eposTEST_FLG			= 3,	// FTR.TEST_FLG
            eposDESC_FLG			= 4,	// FTR.DESC_FLG
            eposOPT_FLAG			= 5,	// FTR.OPT_FLAG
            eposFIRST_OPTIONAL		= 4,
            eposTIME_SET			= 6,	// FTR.TIME_SET
            eposVECT_ADR			= 7,	// FTR.VECT_ADR
            eposCYCL_CNT			= 8,	// FTR.CYCL_CNT
            eposREPT_CNT            = 9,	// FTR.DEPT_CNT
            eposPCP_ADDR			= 10,	// FTR.PCP_ADDR
            eposNUM_FAIL			= 11,	// FTR.NUM_FAIL
            eposFAIL_PIN			= 12,	// FTR.FAIL_PIN
            eposVECT_DAT			= 13,	// FTR.VECT_DAT
            eposDEV_DAT             = 14,	// FTR.DEV_DAT
            eposRPIN_MAP			= 15,	// FTR.RPIN_MAP
            eposTEST_NAM			= 16,	// FTR.TEST_NAM
            eposSEQ_NAME			= 17,	// FTR.SEQ_NAME
            eposTEST_TXT			= 18,	// FTR.TEST_TXT
            eposEND					= 19
    };

    stdf_type_u4	m_u4TEST_NUM;		// FTR.TEST_NUM
    stdf_type_u1	m_u1HEAD_NUM;		// FTR.HEAD_NUM
    stdf_type_u1	m_u1SITE_NUM;		// FTR.SITE_NUM
    stdf_type_b1	m_b1TEST_FLG;		// FTR.TEST_FLG
    stdf_type_b1	m_b1DESC_FLG;		// PTR.DESC_FLG
    stdf_type_b1	m_b1OPT_FLAG;		// FTR.OPT_FLAG
    stdf_type_u4	m_u4CYCL_CNT;		// FTR.CYCL_CNT
    stdf_type_u2	m_u2REPT_CNT;		// FTR.REPT_CNT
    stdf_type_u1	m_u1TIME_SET;		// FTR.TIME_SET
    stdf_type_u2	m_u2PCP_ADDR;		// FTR.PCP_ADDR
    stdf_type_u4	m_u4NUM_FAIL;		// FTR.NUM_FAIL
    stdf_type_u4	m_u4VECT_ADR;		// FTR.VECT_ADR
    stdf_type_bn	m_bnFAIL_PIN;		// FTR.FAIL_PIN
    stdf_type_bn	m_bnVECT_DAT;		// FTR.VECT_DAT
    stdf_type_bn	m_bnDEV_DAT;		// FTR.DEV_DAT
    stdf_type_bn	m_bnRPIN_MAP;		// FTR.RPIN_MAP
    stdf_type_cn	m_cnTEST_NAM;		// FTR.TEST_NAM
    stdf_type_cn	m_cnSEQ_NAME;		// FTR.SEQ_NAME
    stdf_type_cn	m_cnTEST_TXT;		// FTR.TEST_TXT

private:
    // Hold flags for each field
    int				m_pFieldFlags[eposEND];
    Stdf_FTR_V3 *	m_pFilter;
};

///////////////////////////////////////////////////////////
// HBR RECORD
///////////////////////////////////////////////////////////
class Stdf_HBR_V3: public Stdf_Record
{
// METHODS
public:
    // Constructor / destructor functions
    Stdf_HBR_V3();
    ~Stdf_HBR_V3();

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

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

    /// \brief Check if the field is present
    /// \param positon of the field
    /// \return true if field is present
    bool		IsFieldPresent(int nFieldPos);

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposHBIN_NUM			= 0,
            eposHBIN_CNT			= 1,
            eposHBIN_NAM			= 2,
            eposFIRST_OPTIONAL		= 2,
            eposEND					= 3
    };

    stdf_type_u2	m_u2HBIN_NUM;		// HBR.HBIN_NUM
    stdf_type_u4	m_u4HBIN_CNT;		// HBR.HBIN_CNT
    stdf_type_cn	m_cnHBIN_NAM;		// HBR.HBIN_NAM

private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};

///////////////////////////////////////////////////////////
// SBR RECORD
///////////////////////////////////////////////////////////
class Stdf_SBR_V3: public Stdf_Record
{
// METHODS
public:
    // Constructor / destructor functions
    Stdf_SBR_V3();
    ~Stdf_SBR_V3();

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

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

    /// \brief Check if the field is present
    /// \param positon of the field
    /// \return true if field is present
    bool		IsFieldPresent(int nFieldPos);

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposSBIN_NUM			= 0,
            eposSBIN_CNT			= 1,
            eposSBIN_NAM			= 2,
            eposFIRST_OPTIONAL		= 2,
            eposEND					= 3
    };

    stdf_type_u2	m_u2SBIN_NUM;		// SBR.SBIN_NUM
    stdf_type_u4	m_u4SBIN_CNT;		// SBR.SBIN_CNT
    stdf_type_cn	m_cnSBIN_NAM;		// SBR.SBIN_NAM

private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};

///////////////////////////////////////////////////////////
// TSR RECORD
///////////////////////////////////////////////////////////
class Stdf_TSR_V3: public Stdf_Record
{
// METHODS
public:
    // Constructor / destructor functions
    Stdf_TSR_V3();
    ~Stdf_TSR_V3();

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

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

    /// \brief Check if the field is present
    /// \param positon of the field
    /// \return true if field is present
    bool		IsFieldPresent(int nFieldPos);

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposTEST_NUM			= 0,	// TSR.TEST_NUM
            eposEXEC_CNT			= 1,	// TSR.EXEC_CNT
            eposFIRST_OPTIONAL		= 1,
            eposFAIL_CNT			= 2,	// TSR.FAIL_CNT
            eposALRM_CNT			= 3,	// TSR.ALRM_CNT
            eposOPT_FLAG			= 4,	// TSR.OPT_FLAG
            eposPAD_BYTE			= 5,	// TSR.PAD_BYTE
            eposTEST_MIN			= 6,	// TSR.TEST_MIN
            eposTEST_MAX			= 7,	// TSR.TEST_MAX
            eposTST_MEAN			= 8,	// TSR.TST_MEAN
            eposTST_SDEV			= 9,	// TSR.TST_SDEV
            eposTST_SUMS			= 10,	// TSR.TST_SUMS
            eposTST_SQRS			= 11,	// TSR.TST_SQRS
            eposTEST_NAM			= 12,	// TSR.TEST_NAM
            eposSEQ_NAME			= 13,	// TSR.SEQ_NAME
            eposEND					= 14
    };

    stdf_type_u4	m_u4TEST_NUM;		// TSR.TEST_NUM
    stdf_type_i4	m_i4EXEC_CNT;		// TSR.EXEC_CNT
    stdf_type_i4	m_i4FAIL_CNT;		// TSR.FAIL_CNT
    stdf_type_i4	m_i4ALRM_CNT;		// TSR.ALRM_CNT
    stdf_type_b1	m_b1OPT_FLAG;		// TSR.OPT_FLAG
    stdf_type_b1	m_b1PAD_BYTE;		// TSR.PAD_BYTE
    stdf_type_r4	m_r4TEST_MIN;		// TSR.TEST_MIN
    stdf_type_r4	m_r4TEST_MAX;		// TSR.TEST_MAX
    stdf_type_r4	m_r4TST_MEAN;		// TSR.TST_MEAN
    stdf_type_r4	m_r4TST_SDEV;		// TSR.TST_SDEV
    stdf_type_r4	m_r4TST_SUMS;		// TSR.TST_SUMS
    stdf_type_r4	m_r4TST_SQRS;		// TSR.TST_SQRS
    stdf_type_cn	m_cnTEST_NAM;		// TSR.TEST_NAM
    stdf_type_cn	m_cnSEQ_NAME;		// TSR.SEQ_NAME



private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};

///////////////////////////////////////////////////////////
// SHB RECORD
///////////////////////////////////////////////////////////
class Stdf_SHB_V3: public Stdf_Record
{
// METHODS
public:
    // Constructor / destructor functions
    Stdf_SHB_V3();
    ~Stdf_SHB_V3();

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

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

    /// \brief Check if the field is present
    /// \param positon of the field
    /// \return true if field is present
    bool		IsFieldPresent(int nFieldPos);

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposHEAD_NUM			= 0,	// SHB.HEAD_NUM
            eposSITE_NUM			= 1,	// SHB.SITE_NUM
            eposHBIN_NUM			= 2,	// SHB.HBIN_NUM
            eposHBIN_CNT			= 3,	// SHB.HBIN_CNT
            eposHBIN_NAM			= 4,	// SHB.HBIN_NAM
            eposFIRST_OPTIONAL		= 4,
            eposEND					= 5
    };

    stdf_type_u1	m_u1HEAD_NUM;		// SHB.HEAD_NUM
    stdf_type_u1	m_u1SITE_NUM;		// SHB.SITE_NUM
    stdf_type_u2	m_u2HBIN_NUM;		// SHB.HBIN_NUM
    stdf_type_u4	m_u4HBIN_CNT;		// SHB.HBIN_CNT
    stdf_type_cn	m_cnHBIN_NAM;		// SHB.HBIN_NAM
private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};


///////////////////////////////////////////////////////////
// SSB RECORD
///////////////////////////////////////////////////////////
class Stdf_SSB_V3: public Stdf_Record
{
// METHODS
public:
    // Constructor / destructor functions
    Stdf_SSB_V3();
    ~Stdf_SSB_V3();

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

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

    /// \brief Check if the field is present
    /// \param positon of the field
    /// \return true if field is present
    bool		IsFieldPresent(int nFieldPos);

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposHEAD_NUM			= 0,	// SSB.HEAD_NUM
            eposSITE_NUM			= 1,	// SSB.SITE_NUM
            eposSBIN_NUM			= 2,	// SSB.SBIN_NUM
            eposSBIN_CNT			= 3,	// SSB.SBIN_CNT
            eposSBIN_NAM			= 4,	// SSB.SBIN_NAM
            eposFIRST_OPTIONAL		= 4,
            eposEND					= 5
    };

    stdf_type_u1	m_u1HEAD_NUM;		// SSB.HEAD_NUM
    stdf_type_u1	m_u1SITE_NUM;		// SSB.SITE_NUM
    stdf_type_u2	m_u2SBIN_NUM;		// SSB.SBIN_NUM
    stdf_type_u4	m_u4SBIN_CNT;		// SSB.SBIN_CNT
    stdf_type_cn	m_cnSBIN_NAM;		// SSB.SBIN_NAM
private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};



///////////////////////////////////////////////////////////
// STS RECORD
///////////////////////////////////////////////////////////
class Stdf_STS_V3: public Stdf_Record
{
// METHODS
public:
    // Constructor / destructor functions
    Stdf_STS_V3();
    ~Stdf_STS_V3();

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

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

    /// \brief Check if the field is present
    /// \param positon of the field
    /// \return true if field is present
    bool		IsFieldPresent(int nFieldPos);

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposHEAD_NUM			= 0,	// STS.HEAD_NUM
            eposSITE_NUM			= 1,	// STS.SITE_NUM
            eposTEST_NUM			= 2,	// STS.TEST_NUM
            eposEXEC_CNT			= 3,	// STS.EXEC_CNT
            eposFIRST_OPTIONAL		= 3,
            eposFAIL_CNT			= 4,	// STS.FAIL_CNT
            eposALRM_CNT			= 5,	// STS.ALRM_CNT
            eposOPT_FLAG			= 6,	// STS.OPT_FLAG
            eposPAD_BYTE			= 7,	// STS.PAD_BYTE
            eposTEST_MIN			= 8,	// STS.TEST_MIN
            eposTEST_MAX			= 9,	// STS.TEST_MAX
            eposTST_MEAN			= 10,	// STS.TST_MEAN
            eposTST_SDEV			= 11,	// STS.TST_SDEV
            eposTST_SUMS			= 12,	// STS.TST_SUMS
            eposTST_SQRS			= 13,	// STS.TST_SQRS
            eposTEST_NAM			= 14,	// STS.TEST_NAM
            eposSEQ_NAME			= 15,	// STS.SEQ_NAME
            eposTEST_LBL			= 16,	// STS.TEST_LBL
            eposEND					= 17
    };

    stdf_type_u1	m_u1HEAD_NUM;		// STS.HEAD_NUM
    stdf_type_u1	m_u1SITE_NUM;		// STS.SITE_NUM
    stdf_type_u4	m_u4TEST_NUM;		// STS.TEST_NUM
    stdf_type_i4	m_i4EXEC_CNT;		// STS.EXEC_CNT
    stdf_type_i4	m_i4FAIL_CNT;		// STS.FAIL_CNT
    stdf_type_i4	m_i4ALRM_CNT;		// STS.ALRM_CNT
    stdf_type_b1	m_b1OPT_FLAG;		// STS.OPT_FLAG
    stdf_type_b1	m_b1PAD_BYTE;		// STS.PAD_BYTE
    stdf_type_r4	m_r4TEST_MIN;		// STS.TEST_MIN
    stdf_type_r4	m_r4TEST_MAX;		// STS.TEST_MAX
    stdf_type_r4	m_r4TST_MEAN;		// STS.TST_MEAN
    stdf_type_r4	m_r4TST_SDEV;		// STS.TST_SDEV
    stdf_type_r4	m_r4TST_SUMS;		// STS.TST_SUMS
    stdf_type_r4	m_r4TST_SQRS;		// STS.TST_SQRS
    stdf_type_cn	m_cnTEST_NAM;		// STS.TEST_NAM
    stdf_type_cn	m_cnSEQ_NAME;		// STS.SEQ_NAME
    stdf_type_cn	m_cnTEST_LBL;		// STS.TEST_LBL


private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};


///////////////////////////////////////////////////////////
// SCR RECORD
///////////////////////////////////////////////////////////
class Stdf_SCR_V3: public Stdf_Record
{
// METHODS
public:
    // Constructor / destructor functions
    Stdf_SCR_V3();
    ~Stdf_SCR_V3();

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

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

    /// \brief Check if the field is present
    /// \param positon of the field
    /// \return true if field is present
    bool		IsFieldPresent(int nFieldPos);

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposHEAD_NUM			= 0,
            eposSITE_NUM			= 1,
            eposFINISH_T			= 2,
            eposFIRST_OPTIONAL		= 2,
            eposPART_CNT			= 3,
            eposRTST_CNT			= 4,
            eposABRT_CNT			= 5,
            eposGOOD_CNT			= 6,
            eposFUNC_CNT			= 7,
            eposEND					= 8
    };

    stdf_type_u1	m_u1HEAD_NUM;		// SCR.HEAD_NUM
    stdf_type_u1	m_u1SITE_NUM;		// SCR.SITE_NUM
    stdf_type_u4	m_u4FINISH_T;		// SCR.FINISH_T
    stdf_type_u4	m_u4PART_CNT;		// SCR.PART_CNT
    stdf_type_i4	m_i4RTST_CNT;		// SCR.RTST_CNT
    stdf_type_i4	m_i4ABRT_CNT;		// SCR.ABRT_CNT
    stdf_type_i4	m_i4GOOD_CNT;		// SCR.GOOD_CNT
    stdf_type_i4	m_i4FUNC_CNT;		// SCR.FUNC_CNT

private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};

///////////////////////////////////////////////////////////
// PMR RECORD
///////////////////////////////////////////////////////////
class Stdf_PMR_V3: public Stdf_Record
{
// METHODS
public:
    // Constructor / destructor functions
    Stdf_PMR_V3();
    ~Stdf_PMR_V3();

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

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

    /// \brief Check if the field is present
    /// \param positon of the field
    /// \return true if field is present
    bool		IsFieldPresent(int nFieldPos);

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposCHAN_CNT			= 0,
            eposNAME_CNT			= 1,
            eposFIRST_OPTIONAL		= 1,
            eposCHAN_NUM			= 2,
            eposPIN_NAME			= 3,
            eposEND					= 4
    };

    stdf_type_u2	m_u2CHAN_CNT;		// PMR.CHAN_CNT
    stdf_type_u2	m_u2NAME_CNT;		// PMR.NAME_CNT
    stdf_type_u2*	m_u2CHAN_NUM;		// PMR.CHAN_NUM
    stdf_type_cn*	m_cnPIN_NAME;		// PMR.PIN_NAME

private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};

///////////////////////////////////////////////////////////
// BPS RECORD
///////////////////////////////////////////////////////////
class Stdf_BPS_V3: public Stdf_Record
{
// METHODS
public:
    // Constructor / destructor functions
    Stdf_BPS_V3();
    ~Stdf_BPS_V3();

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

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

    /// \brief Check if the field is present
    /// \param positon of the field
    /// \return true if field is present
    bool		IsFieldPresent(int nFieldPos);

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
class Stdf_EPS_V3: public Stdf_Record
{
// METHODS
public:
    // Constructor / destructor functions
    Stdf_EPS_V3();
    ~Stdf_EPS_V3();

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

};

///////////////////////////////////////////////////////////
// GDR RECORD
///////////////////////////////////////////////////////////
class Stdf_GDR_V3: public Stdf_Record
{
// METHODS
public:
    // Constructor / destructor functions
    Stdf_GDR_V3();
    ~Stdf_GDR_V3();

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

    bool IsFieldValid(int nFieldPos);

    /// \brief Check if the field is present
    /// \param positon of the field
    /// \return true if field is present
    bool		IsFieldPresent(int nFieldPos);

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

///////////////////////////////////////////////////////////
// DTR RECORD
///////////////////////////////////////////////////////////
class Stdf_DTR_V3: public Stdf_Record
{
// METHODS
public:
    // Constructor / destructor functions
    Stdf_DTR_V3();
    ~Stdf_DTR_V3();

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

    // Set members and flags
    void		SetTEXT_DAT(const stdf_type_cn cnTEXT_DAT)	_FIELD_SET(m_cnTEXT_DAT = cnTEXT_DAT, true, eposTEXT_DAT)

    // Check field validity
    bool		IsFieldValid(int nFieldPos);		// Return true if field is valid

    /// \brief Check if the field is present
    /// \param positon of the field
    /// \return true if field is present
    bool		IsFieldPresent(int nFieldPos);

    // Check if GEX command: Gross-Die (if so, get value)
    bool		IsGexCommand_GrossDie(unsigned int *puiGrossDie, bool *pbOk);
    // Check if GEX command: Die-Tracking (if so, get value)
    bool		IsGexCommand_DieTracking(QString & strDieID, QString & strCommand, bool *pbOk);
    // Check if GEX command: logPAT (if so, get value)
    bool		IsGexCommand_logPAT(QString & strCommand);
    // Check if GEX command: PATTestList (if so, get value)
    bool		IsGexCommand_PATTestList(QString & strCommand);

// DATA
public:
    // Define position of each field
    enum FieldPos {
            eposTEXT_DAT			= 0,
            eposEND					= 1,
            eposFIRST_OPTIONAL		= 1
    };

    stdf_type_cn	m_cnTEXT_DAT;		// DTR.TEXT_DAT

private:
    // Hold flags for each field
    int		m_pFieldFlags[eposEND];
};
}

#endif // STDFRECORDS_V3_H
