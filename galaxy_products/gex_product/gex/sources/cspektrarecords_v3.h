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
// CSpektraRecords_V3 class HEADER :
// This class contains classes to read and store TESEC Spektra V3 files
// records
///////////////////////////////////////////////////////////

#ifndef _CSpektraRecords_V3_h_
#define _CSpektraRecords_V3_h_

 // Galaxy modules includes
#include "cspektra.h"

#include <QDateTime>

// Some constant definition
// Some default values
#define SPEKTRA_MAX_U1				255
#define SPEKTRA_MAX_U2				65535

class QString;
class QStringList;
class QDateTime;

// SPEKTRA type mapping
typedef QDateTime		spektra_type_dt;
typedef char			spektra_type_c1;
typedef QString			spektra_type_cn;
typedef char			spektra_type_b1;
typedef unsigned char	spektra_type_u1;
typedef unsigned short	spektra_type_u2;
typedef unsigned long	spektra_type_u4;
typedef char			spektra_type_i1;
typedef short			spektra_type_i2;
typedef long			spektra_type_i4;
typedef float			spektra_type_r4;

///////////////////////////////////////////////////////////
// GENERIC RECORD
///////////////////////////////////////////////////////////
class CSpektra_Record_V3
{
// METHODS
public:
	enum FieldFlags {
			FieldFlag_Empty			= 0x00,
			FieldFlag_None			= 0x01,
			FieldFlag_Present		= 0x02,
			FieldFlag_Valid			= 0x04,
			FieldFlag_ReducedList	= 0x08
	};

	// Constructor / destructor functions
	CSpektra_Record_V3();
	// Use a virtual destructor to avoid warning under gcc (Class has virtual functions, but destructor is not virtual).
	virtual ~CSpektra_Record_V3();

	// Reset record data
	virtual void		Reset(void);
	// Return short name of the record
	virtual QString		GetRecordShortName(void);
	// Return long name of the record
	virtual QString		GetRecordLongName(void);
	// Return record type
	virtual int			GetRecordType(void);
	// Read record
	virtual bool		Read(CSpektra & clSpektra);
	// Construct a stringlist with all fields to display
	// Each string in the list has following format: <Field name>;<Field value>
	virtual void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
	// Construct a ASCII string of the record
	virtual void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);
	// Construct a XML string of the record
	virtual void		GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection = 0);
	
	// Construct a ASCII string of the record
	void				GetAsciiRecord(QString & strRecord, int nFieldSelection = 0);

// DATA
public:
	// Spektra V3 record types
	enum RecordTypes {
			Rec_Label				= 0,		// Label record
			Rec_TestItem			= 1,		// TestItem record
			Rec_WaferID				= 2,		// WaferID record
            Rec_DeviceID			= 3,		// DeviceID record
            Rec_TestData			= 4,		// TestData record
            Rec_UNKNOWN				= 5,		// Unknown record type.
			Rec_COUNT				= 6			// !!!! Marker used for array sizes. !!!! Must be (last+1). !!!! Always leave as last in the list !!!!!
	};

protected:
	// Some protected variables used by the macros
	char				m_szTmp_macro[1024];
	unsigned int		m_uiIndex_macro;
	int					m_nTabs_macro;
	const char*			m_ptChar_macro;
	QString				m_strFieldValue_macro;
	QString				m_strTmp_macro;
	QString				m_strTmp2_macro;
	char				m_c1Value_macro;
};

///////////////////////////////////////////////////////////
// Label RECORD
///////////////////////////////////////////////////////////
class CSpektra_Label_V3: public CSpektra_Record_V3
{
// METHODS
public:
	// Constructor / destructor functions
	CSpektra_Label_V3();
	~CSpektra_Label_V3();

	// Reset record data
	void		Reset(void);
	// Return short name of the record
	QString		GetRecordShortName(void);
	// Return long name of the record
	QString		GetRecordLongName(void);
	// Return record type
	int			GetRecordType(void);
	// Read record
	bool		Read(CSpektra & clSpektra);
	// Construct a stringlist with all fields to display
	// Each string in the list has following format: <Field name>;<Field value>
	void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
	// Construct a ASCII string of the record
	void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);
	// Construct a XML string of the record
	void		GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection = 0);

// DATA
public:
	spektra_type_u1		m_u1DATETIME_Year;		// Label.DATETIME.YEAR
	spektra_type_u1		m_u1DATETIME_Month;		// Label.DATETIME.MONTH
	spektra_type_u1		m_u1DATETIME_Day;		// Label.DATETIME.DAY
	spektra_type_u1		m_u1DATETIME_Hour;		// Label.DATETIME.HOUR
	spektra_type_u1		m_u1DATETIME_Minute;	// Label.DATETIME.MINUTE
	spektra_type_u1		m_u1DATETIME_Second;	// Label.DATETIME.SECOND
	spektra_type_dt		m_dtDATETIME;			// Label.DATETIME
	spektra_type_cn		m_cnFILENAME;			// Label.FILENAME
	spektra_type_cn		m_cnDEVICENAME;			// Label.DEVICENAME
	spektra_type_cn		m_cnOPERATOR;			// Label.OPERATOR
	spektra_type_b1		m_b1TESTINGMODE;		// Label.TESTINGMODE
	spektra_type_c1		m_c1STATIONNAME;		// Label.STATIONNAME
	spektra_type_cn		m_cnLOTNAME;			// Label.LOTNAME
	spektra_type_cn		m_cnCOMMENT;			// Label.COMMENT
	spektra_type_u2		m_u2TIMEPOINT;			// Label.TIMEPOINT
	spektra_type_u2		m_u2SETQUANTITY;		// Label.SETQUANTITY
	spektra_type_u2		m_u2LOGGINGRATE;		// Label.LOGGINGRATE
	spektra_type_u2		m_u2TESTMAX;			// Label.TESTMAX
	spektra_type_u2		m_u2DATABLOCKNUMBER;	// Label.DATABLOCKNUMBER
	spektra_type_u2		m_u2LOGGEDQUANTITY;		// Label.LOGGEDQUANTITY
	spektra_type_u2		m_u2INDEXMAX;			// Label.INDEXMAX
	spektra_type_cn		m_cnRESERVED;			// Label.RESERVED

private:
	// Define position of each field
	enum FieldPos {	
			eposDATETIME			= 0,
			eposFILENAME			= 1,
			eposDEVICENAME			= 2,
			eposOPERATOR			= 3,
			eposTESTINGMODE			= 4,
			eposSTATIONNAME			= 5,
			eposLOTNAME				= 6,
			eposCOMMENT				= 7,
			eposTIMEPOINT			= 8,
			eposSETQUANTITY			= 9,
			eposLOGGINGRATE			= 10,
			eposTESTMAX				= 11,
			eposDATABLOCKNUMBER		= 12,
			eposLOGGEDQUANTITY		= 13,
			eposINDEXMAX			= 14,
			eposRESERVED			= 15,
			eposEND					= 16,
			eposFIRST_OPTIONAL		= 16
	};

	// Hold flags for each field
	int		m_pFieldFlags[eposEND];
};

///////////////////////////////////////////////////////////
// TestItem RECORD
///////////////////////////////////////////////////////////
class CSpektra_TestItem_V3: public CSpektra_Record_V3
{
// METHODS
public:
	// Constructor / destructor functions
	CSpektra_TestItem_V3();
	~CSpektra_TestItem_V3();

	// Reset record data
	void		Reset(void);
	// Return short name of the record
	QString		GetRecordShortName(void);
	// Return long name of the record
	QString		GetRecordLongName(void);
	// Return record type
	int			GetRecordType(void);
	// Read record
	bool		Read(CSpektra & clSpektra);
	// Construct a stringlist with all fields to display
	// Each string in the list has following format: <Field name>;<Field value>
	void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
	// Construct a ASCII string of the record
	void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);
	// Construct a XML string of the record
	void		GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection = 0);

// DATA
public:
	// For internal use
	bool				m_bIsHighLimit;				// true if limit is a HL, false if LL
	bool				m_bIsStrictLimit;			// true if limit is strict, false else
	bool				m_bHaslimit;				// true if test has a valid limit, false else
	
	// Record fields
	spektra_type_u1		m_u1TESTNUMBER;				// TestItem.TESTNUMBER
	spektra_type_u1		m_u1ITEMGROUPCODE;			// TestItem.ITEMGROUPCODE
	spektra_type_u2		m_u2ITEMCODE;				// TestItem.ITEMCODE
	spektra_type_b1		m_b1BECONDITIONANDFLAGS;	// TestItem.BECONDITIONANDFLAGS
	spektra_type_b1		m_b1RESULTASBIASFLAGS;		// TestItem.RESULTASBIASFLAGS
	spektra_type_cn		m_cnLIMITITEMNAME;			// TestItem.LIMITITEMNAME
	spektra_type_cn		m_cnLIMITUNIT;				// TestItem.LIMITUNIT
	spektra_type_r4		m_r4LIMITVALUE;				// TestItem.LIMITVALUE
	spektra_type_r4		m_r4LIMITMIN;				// TestItem.LIMITMIN
	spektra_type_r4		m_r4LIMITMAX;				// TestItem.LIMITMAX
	spektra_type_cn		m_cnBIAS1NAME;				// TestItem.BIAS1NAME
	spektra_type_cn		m_cnBIAS1UNIT;				// TestItem.BIAS1UNIT
	spektra_type_r4		m_r4BIAS1VALUE;				// TestItem.BIAS1VALUE
	spektra_type_cn		m_cnBIAS2NAME;				// TestItem.BIAS2NAME
	spektra_type_cn		m_cnBIAS2UNIT;				// TestItem.BIAS2UNIT
	spektra_type_r4		m_r4BIAS2VALUE;				// TestItem.BIAS2VALUE
	spektra_type_cn		m_cnTIMECONDITIONNAME;		// TestItem.TIMECONDITIONNAME
	spektra_type_cn		m_cnTIMEUNIT;				// TestItem.TIMEUNIT
	spektra_type_r4		m_r4TIMEVALUE;				// TestItem.TIMEVALUE
	spektra_type_b1		m_b1DLINHIBITFLAGS;			// TestItem.DLINHIBITFLAGS
	spektra_type_cn		m_cnRESERVED;				// TestItem.RESERVED

private:
	// Define position of each field
	enum FieldPos {	
			eposTESTNUMBER				= 0,
			eposITEMGROUPCODE			= 1,
			eposITEMCODE				= 2,
			eposBECONDITIONANDFLAGS		= 3,
			eposRESULTASBIASFLAGS		= 4,
			eposLIMITITEMNAME			= 5,
			eposLIMITUNIT				= 6,
			eposLIMITVALUE				= 7,
			eposLIMITMIN				= 8,
			eposLIMITMAX				= 9,
			eposBIAS1NAME				= 10,
			eposBIAS1UNIT				= 11,
			eposBIAS1VALUE				= 12,
			eposBIAS2NAME				= 13,
			eposBIAS2UNIT				= 14,
			eposBIAS2VALUE				= 15,
			eposTIMECONDITIONNAME		= 16,
			eposTIMEUNIT				= 17,
			eposTIMEVALUE				= 18,
			eposDLINHIBITFLAGS			= 19,
			eposRESERVED				= 20,
			eposEND						= 21,
			eposFIRST_OPTIONAL			= 21
	};

	// Hold flags for each field
	int		m_pFieldFlags[eposEND];
};

///////////////////////////////////////////////////////////
// WaferID RECORD
///////////////////////////////////////////////////////////
class CSpektra_WaferID_V3: public CSpektra_Record_V3
{
// METHODS
public:
	// Constructor / destructor functions
	CSpektra_WaferID_V3();
	~CSpektra_WaferID_V3();

	// Reset record data
	void		Reset(void);
	// Return short name of the record
	QString		GetRecordShortName(void);
	// Return long name of the record
	QString		GetRecordLongName(void);
	// Return record type
	int			GetRecordType(void);
	// Read record
	bool		Read(CSpektra & clSpektra);
	// Construct a stringlist with all fields to display
	// Each string in the list has following format: <Field name>;<Field value>
	void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
	// Construct a ASCII string of the record
	void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);
	// Construct a XML string of the record
	void		GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection = 0);

// DATA
public:
	spektra_type_u1		m_u1WAFERNUMBER;				// WaferID.WAFERNUMBER
	spektra_type_u2		m_u2NUMBEROFDEVICES;			// WaferID.NUMBEROFDEVICES
	spektra_type_cn		m_cnRESERVED;					// WaferID.RESERVED

private:
	// Define position of each field
	enum FieldPos {	
			eposWAFERNUMBER			= 0,
			eposNUMBEROFDEVICES		= 1,
			eposRESERVED			= 2,
			eposEND					= 3,
			eposFIRST_OPTIONAL		= 3
	};

	// Hold flags for each field
	int		m_pFieldFlags[eposEND];
};

///////////////////////////////////////////////////////////
// DeviceID RECORD
///////////////////////////////////////////////////////////
class CSpektra_DeviceID_V3: public CSpektra_Record_V3
{
// METHODS
public:
	// Constructor / destructor functions
	CSpektra_DeviceID_V3();
	~CSpektra_DeviceID_V3();

	// Reset record data
	void		Reset(void);
	// Return short name of the record
	QString		GetRecordShortName(void);
	// Return long name of the record
	QString		GetRecordLongName(void);
	// Return record type
	int			GetRecordType(void);
	// Read record
	bool		Read(CSpektra & clSpektra);
	// Construct a stringlist with all fields to display
	// Each string in the list has following format: <Field name>;<Field value>
	void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
	// Construct a ASCII string of the record
	void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);
	// Construct a XML string of the record
	void		GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection = 0);

// DATA
public:
	spektra_type_u2		m_u2SERIALNUMBER;				// DeviceID.SERIALNUMBER
	spektra_type_u2		m_u2BINNUMBER;					// DeviceID.BINNUMBER

private:
	// Define position of each field
	enum FieldPos {	
			eposSERIALNUMBER		= 0,
			eposBINNUMBER			= 1,
			eposEND					= 2,
			eposFIRST_OPTIONAL		= 2
	};

	// Hold flags for each field
	int		m_pFieldFlags[eposEND];
};

///////////////////////////////////////////////////////////
// TestData RECORD
///////////////////////////////////////////////////////////
class CSpektra_TestData_V3: public CSpektra_Record_V3
{
// METHODS
public:
	// Constructor / destructor functions
	CSpektra_TestData_V3();
	~CSpektra_TestData_V3();

	// Reset record data
	void		Reset(void);
	// Return short name of the record
	QString		GetRecordShortName(void);
	// Return long name of the record
	QString		GetRecordLongName(void);
	// Return record type
	int			GetRecordType(void);
	// Read record
	bool		Read(CSpektra & clSpektra);
	// Construct a stringlist with all fields to display
	// Each string in the list has following format: <Field name>;<Field value>
	void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
	// Construct a ASCII string of the record
	void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0);
	// Construct a XML string of the record
	void		GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection = 0);

// DATA
public:
	spektra_type_u1		m_u1TESTNUMBER;					// TestData.TESTNUMBER
	spektra_type_b1		m_b1FLAGS;						// TestData.FLAGS
	spektra_type_r4		m_r4TESTRESULTVALUE;			// TestData.TESTRESULTVALUE

private:
	// Define position of each field
	enum FieldPos {	
			eposTESTNUMBER			= 0,
			eposFLAGS				= 1,
			eposTESTRESULTVALUE		= 2,
			eposEND					= 3,
			eposFIRST_OPTIONAL		= 3
	};

	// Hold flags for each field
	int		m_pFieldFlags[eposEND];
};

#endif // #ifndef _CSpektraRecords_V3_h_
