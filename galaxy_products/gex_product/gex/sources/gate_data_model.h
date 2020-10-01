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

/////////////////////////////////////////////////////////////////////////////////////////
// This file defines classes to store and handle Yield-123 data.
/////////////////////////////////////////////////////////////////////////////////////////

#ifndef GEX_PLUGIN_YIELD123_DATA_H
#define GEX_PLUGIN_YIELD123_DATA_H

class C_Gate_DataModel_Object;
class C_Gate_DataModel;
class C_Gate_DataModel_TestingStage;
class C_Gate_DataModel_Lot;
class C_Gate_DataModel_Sublot;
class C_Gate_DataModel_Batch;
class C_Gate_DataModel_Site;

class C_Y123_ResultSet;

/////////////////////////////////////////////////////////////////////////////////////////
// INCLUDES
/////////////////////////////////////////////////////////////////////////////////////////
// Standard includes
#include <QProgressDialog>
#include <QApplication>

#include <QString>
#include <QDateTime>
#include <QList>
#include <QtAlgorithms>
#include <QVector>
#include <QFile>

// Local includes
#include "gate_event_manager.h"
#include "gate_data_constants.h"

// Galaxy libraries includes
#include <gstdl_errormgr.h>

/////////////////////////////////////////////////////////////////////////////////////////
// CONSTANTS
/////////////////////////////////////////////////////////////////////////////////////////
// Flags for parameter defs
#define YIELD123_PARAMETERDEF_HASLL							0x0001		// Parameter has a LL
#define YIELD123_PARAMETERDEF_HASHL							0x0002		// Parameter has a HL
#define YIELD123_PARAMETERDEF_LLNOTSTRICT					0x0004		// Parameter's LL is not strict (if result == LL, test is PASS)
#define YIELD123_PARAMETERDEF_HLNOTSTRICT					0x0008		// Parameter's HL is not strict (if result == HL, test is PASS)
#define YIELD123_PARAMETERDEF_PARAMETRICUNITS				0x0010		// Parameter has parametric units (A, V, Hz...)

// Flags for test results
#define YIELD123_TESTRESULT_EXECUTED						0x0001		// Test has been executed
#define YIELD123_TESTRESULT_PASSFAILSTATUS_VALID			0x0002		// Test pass/fail status valid
#define YIELD123_TESTRESULT_PASSFAILSTATUS_FAILED			0x0004		// Test failed (valid only if YIELD123_TESTRESULT_PASSFAILSTATUS_VALID set)
#define YIELD123_TESTRESULT_VALUE_FAILED_H					0x0008		// Value outside limits (> HL)
#define YIELD123_TESTRESULT_VALUE_FAILED_L					0x0010		// Value outside limits (< LL)
#define YIELD123_TESTRESULT_FINALSTATUS_FAILED				0x0020		// Final status: set if the test should be considered FAIL
#define YIELD123_TESTRESULT_OUTLIER_H						0x0040		// Test result is an outlier (towards high values)
#define YIELD123_TESTRESULT_OUTLIER_L						0x0080		// Test result is an outlier (towards low values)

// Flags for part results
#define YIELD123_PARTRESULT_EXECUTED						0x0001		// This run has been executed (corrsponds to a part on this site)
#define YIELD123_PARTRESULT_FAIL							0x0002		// Part is FAIL
#define YIELD123_PARTRESULT_HASOUTLIERS						0x0004		// Part has outliers
#define YIELD123_PARTRESULT_RETEST							0x0008		// Part is a retest
#define YIELD123_PARTRESULT_HASBEENRETESTED					0x0010		// Part has been retested

// Flags for parameter stats
#define YIELD123_PARAMETERSTATS_HASVALIDRESULTS				0x0001		// Parameter has at least 1 floating (non integer) test result
#define YIELD123_PARAMETERSTATS_HASFRESULTS					0x0002		// Parameter has at least 1 floating (non integer) test result
#define YIELD123_PARAMETERSTATS_ARTIFICIAL_LL				0x0004		// LL is artificial (no pb if values close to LL)
#define YIELD123_PARAMETERSTATS_ARTIFICIAL_HL				0x0008		// HL is artificial (no pb if values close to HL)


// Flags for information display
// Parts filter
#define YIELD123_FLAGS_FILTER_ON_Parts						1
#define YIELD123_FLAGS_FILTER_ON_OddParts					2
#define YIELD123_FLAGS_FILTER_ON_RetestParts				3
#define YIELD123_FLAGS_FILTER_ON_GoodBins					4
#define YIELD123_FLAGS_FILTER_ON_HardBins					5
#define YIELD123_FLAGS_FILTER_ON_SoftBins					6

// Statistic information
#define YIELD123_FLAGS_STATVAR_Flags						0x000001
#define YIELD123_FLAGS_STATVAR_Samples						0x000002
#define YIELD123_FLAGS_STATVAR_Fail							0x000004
#define YIELD123_FLAGS_STATVAR_Fail_L						0x000008
#define YIELD123_FLAGS_STATVAR_Fail_H						0x000010
#define YIELD123_FLAGS_STATVAR_Pass							0x000020
#define YIELD123_FLAGS_STATVAR_Outliers						0x000040
#define YIELD123_FLAGS_STATVAR_Outliers_L					0x000080
#define YIELD123_FLAGS_STATVAR_Outliers_H					0x000100
#define YIELD123_FLAGS_STATVAR_Mean							0x000200
#define YIELD123_FLAGS_STATVAR_Min							0x000400
#define YIELD123_FLAGS_STATVAR_Max							0x000800
#define YIELD123_FLAGS_STATVAR_Range						0x001000
#define YIELD123_FLAGS_STATVAR_Sigma						0x002000
#define YIELD123_FLAGS_STATVAR_Cpk							0x004000
#define YIELD123_FLAGS_STATVAR_Cp							0x008000
#define YIELD123_FLAGS_STATVAR_Q1							0x010000
#define YIELD123_FLAGS_STATVAR_Q2							0x020000
#define YIELD123_FLAGS_STATVAR_Q3							0x040000
#define YIELD123_FLAGS_STATVAR_Number						0x080000
#define YIELD123_FLAGS_STATVAR_Name							0x100000
#define YIELD123_FLAGS_STATVAR_Shape						0x200000
#define YIELD123_FLAGS_STATVAR_Limits						0x400000
#define YIELD123_FLAGS_STATVAR_Sequence						0x800000

// Global information
#define YIELD123_FLAGS_GLOBALINFO_ProductID					0x00000001
#define YIELD123_FLAGS_GLOBALINFO_FromDate					0x00000002
#define YIELD123_FLAGS_GLOBALINFO_ToDate					0x00000004
#define YIELD123_FLAGS_GLOBALINFO_LotID						0x00000008
#define YIELD123_FLAGS_GLOBALINFO_SublotID					0x00000010
#define YIELD123_FLAGS_GLOBALINFO_WaferID					0x00000020
#define YIELD123_FLAGS_GLOBALINFO_NbParameters				0x00000040
#define YIELD123_FLAGS_GLOBALINFO_NbParts					0x00000080
#define YIELD123_FLAGS_GLOBALINFO_NbPassParts				0x00000100
#define YIELD123_FLAGS_GLOBALINFO_NbFailParts				0x00000200
#define YIELD123_FLAGS_GLOBALINFO_NbRetestParts				0x00000400
#define YIELD123_FLAGS_GLOBALINFO_NbFailPartsAfterRetest	0x00000800
#define YIELD123_FLAGS_GLOBALINFO_NbSites					0x00001000
#define YIELD123_FLAGS_GLOBALINFO_OperatorName				0x00002000
#define YIELD123_FLAGS_GLOBALINFO_JobName					0x00004000
#define YIELD123_FLAGS_GLOBALINFO_JobRevision				0x00008000
#define YIELD123_FLAGS_GLOBALINFO_ExecType					0x00010000
#define YIELD123_FLAGS_GLOBALINFO_ExecVersion				0x00020000
#define YIELD123_FLAGS_GLOBALINFO_TestCode					0x00040000
#define YIELD123_FLAGS_GLOBALINFO_TestTemperature			0x00080000
#define YIELD123_FLAGS_GLOBALINFO_TesterType				0x00100000
#define YIELD123_FLAGS_GLOBALINFO_TesterName				0x00200000
#define YIELD123_FLAGS_GLOBALINFO_HandlerProberID			0x00400000
#define YIELD123_FLAGS_GLOBALINFO_ProbeCardID				0x00800000
#define YIELD123_FLAGS_GLOBALINFO_LoadBoardID				0x01000000
#define YIELD123_FLAGS_GLOBALINFO_DibBoardID				0x02000000
#define YIELD123_FLAGS_GLOBALINFO_InterfaceCableID			0x04000000
#define YIELD123_FLAGS_GLOBALINFO_HandlerContactorID		0x08000000
#define YIELD123_FLAGS_GLOBALINFO_LaserID					0x10000000
#define YIELD123_FLAGS_GLOBALINFO_ExtraEquipmentID			0x20000000
#define YIELD123_FLAGS_GLOBALINFO_Yield						0x40000000
#define YIELD123_FLAGS_GLOBALINFO_FileName					0x80000000


// Audit Test information
#define YIELD123_FLAGS_AUDITTEST_Zero						0x00000001
#define YIELD123_FLAGS_AUDITTEST_Categories					0x00000002
#define YIELD123_FLAGS_AUDITTEST_MultiModal					0x00000004
#define YIELD123_FLAGS_AUDITTEST_Resolution					0x00000008
#define YIELD123_FLAGS_AUDITTEST_SiteToSite					0x00000010
#define YIELD123_FLAGS_AUDITTEST_Histogram					0x00000020
#define YIELD123_FLAGS_AUDITTEST_IncorrectRange				0x00000040

// Audit File information
#define YIELD123_FLAGS_AUDITFILE_InvalidRecords				0x00000001
#define YIELD123_FLAGS_AUDITFILE_MissingRecords				0x00000002
#define YIELD123_FLAGS_AUDITFILE_EndRecords					0x00000004
#define YIELD123_FLAGS_AUDITFILE_DuplicatedTests			0x00000008
#define YIELD123_FLAGS_AUDITFILE_MissingTestLimits			0x00000010
#define YIELD123_FLAGS_AUDITFILE_DiscrepanciesTestLimits	0x00000020
#define YIELD123_FLAGS_AUDITFILE_MprIndex					0x00000040
#define YIELD123_FLAGS_AUDITFILE_TestsResultOutside			0x00000080

#define YIELD123_FLAGS_AUDITFILE_TestProblems				0x000000F8
#define YIELD123_FLAGS_AUDITFILE_TestDiscrepancies			0x000000C0


// Audit Miscellaneous information
#define YIELD123_FLAGS_AUDITMISCELLANEOUS_MissingProductID			0x00001000
#define YIELD123_FLAGS_AUDITMISCELLANEOUS_MissingLotID				0x00002000
#define YIELD123_FLAGS_AUDITMISCELLANEOUS_MissingWaferID			0x00004000
#define YIELD123_FLAGS_AUDITMISCELLANEOUS_MissingWaferCoordinates	0x00008000
#define YIELD123_FLAGS_AUDITMISCELLANEOUS_RecordsWirWrrCount		0x00010000
#define YIELD123_FLAGS_AUDITMISCELLANEOUS_RecordsWirOverloaded		0x00020000
#define YIELD123_FLAGS_AUDITMISCELLANEOUS_RecordsWirCount			0x00040000
#define YIELD123_FLAGS_AUDITMISCELLANEOUS_RecordsWirMultihead		0x00080000
#define YIELD123_FLAGS_AUDITMISCELLANEOUS_RecordsMirCount			0x00100000
#define YIELD123_FLAGS_AUDITMISCELLANEOUS_RecordsSdrCount			0x00200000
#define YIELD123_FLAGS_AUDITMISCELLANEOUS_RecordsPcrPartsCount		0x00400000

#define YIELD123_FLAGS_AUDITMISCELLANEOUS_MissingFields				0x0000F000
#define YIELD123_FLAGS_AUDITMISCELLANEOUS_DiscrepanciesRecords		0x0FFF0000

#define YIELD123_FLAGS_EMPTY_ANALYSIS								0x80000000

#define YIELD123_FLAGS_WAFERMAP_BINNING								0x00000001
#define YIELD123_FLAGS_WAFERMAP_PARAMETRICS							0x00000002


#define	YIELD123_TESTNBR_OFFSET_EXT	786000							// Extended tests created by Examinator. eg: Test time, Binning, etc...
#define YIELD123_TESTNBR_OFFSET_EXT_SBIN							(YIELD123_TESTNBR_OFFSET_EXT)
#define YIELD123_TESTNBR_OFFSET_EXT_HBIN							(YIELD123_TESTNBR_OFFSET_EXT+1)
#define YIELD123_TESTNBR_OFFSET_EXT_DIEX							(YIELD123_TESTNBR_OFFSET_EXT+2)
#define YIELD123_TESTNBR_OFFSET_EXT_DIEY							(YIELD123_TESTNBR_OFFSET_EXT+3)
#define YIELD123_TESTNBR_OFFSET_EXT_TTIME							(YIELD123_TESTNBR_OFFSET_EXT+4)
#define YIELD123_TESTNBR_OFFSET_EXT_TEMPERATURE						(YIELD123_TESTNBR_OFFSET_EXT+5)

/////////////////////////////////////////////////////////////////////////////////////////
// DATA TYPES
/////////////////////////////////////////////////////////////////////////////////////////
class QTextStream;

/////////////////////////////////////////////////////////////////////////////////////////
// GLOBAL FUNCTIONS
/////////////////////////////////////////////////////////////////////////////////////////
QString GetCsvString(const QString & strString);

// CLASS: C_Gate_DataModel_Progress
// Progress dialog
///////////////////////////////////////////////////////////
class C_Gate_DataModel_Progress : public QProgressDialog
{
public:
	C_Gate_DataModel_Progress(QString strText, int nTotalSteps, QApplication* qApplication);

	void Init(QString strText, int nTotalSteps);
	void Increment(void);
	void ProcessEvents(void);

private:
	QApplication*	m_qApplication;
	int				m_nCurrentStep;
};


/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: C_Gate_DataModel_ObjectList
// Holds a list of objects (C_Gate_DataModel_Object)
/////////////////////////////////////////////////////////////////////////////////////////
class C_Gate_DataModel_ObjectList: public QList<C_Gate_DataModel_Object *>
{
// CONSTRUCTOR/DESTRUCTOR
public:
	C_Gate_DataModel_ObjectList() {	m_iCurrentPos = -1;};

// PUBLIC METHODS
public:
	C_Gate_DataModel_Object *GetFirst()		{ if(isEmpty()) return NULL;	m_iCurrentPos = 0;	return at(m_iCurrentPos);};
	C_Gate_DataModel_Object *GetCurrent()	{ if(isEmpty()) return NULL;	if(m_iCurrentPos == -1) m_iCurrentPos = 0; return at(m_iCurrentPos);};
	C_Gate_DataModel_Object *GetNext()		{ if(isEmpty()) return NULL;	if(m_iCurrentPos >= size()-1) return NULL; m_iCurrentPos++;	return at(m_iCurrentPos);};
	void					RemoveAll()		{ while (!isEmpty()) /*delete*/ takeFirst(); m_iCurrentPos = -1;};
	void					RemoveCurrent()	{ if(isEmpty()) return; if(m_iCurrentPos >= size()) return; /*delete*/ takeAt(m_iCurrentPos--);};

// PUBLIC DATA
public:

// PRIVATE METHODS
private:

// PRIVATE DATA
private:
	int				m_iCurrentPos;
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: C_Gate_DataModel_Object
// Base class for objects part of the object hierarchy representing a dataset
// (data, testingstage, lot, sublot, batch, site)
/////////////////////////////////////////////////////////////////////////////////////////
class C_Gate_DataModel_Object
{
// CONSTRUCTOR/DESTRUCTOR
public:
	// !!!! IMPORTANT: the hierarchical order must be respected parent first
	enum ObjectType
	{
		eData,					// For C_Gate_DataModel object
		eData_TestingStage,		// For C_Y123_TestingStage object
		eData_Lot,				// For C_Y123_Lot object
		eData_Sublot,			// For C_Y123_Sublot object
		eData_Batch,			// For C_Y123_Batch object
		eData_Site				// For C_Y123_Site object
	};

	C_Gate_DataModel_Object(ObjectType eObjectType, C_Gate_DataModel_Object *pParent = NULL, C_Gate_DataModel_Progress *pProgress = NULL)
	{ m_eObjectType = eObjectType; m_pParent = pParent; m_pProgress = pProgress;}

// PUBLIC METHODS
public:
	ObjectType						GetObjectType() { return m_eObjectType; }
	bool							IsObjectType(ObjectType eObjectType) { return (eObjectType == m_eObjectType); }

	// Object navigation
	C_Gate_DataModel_Object*		GetFirstObject(ObjectType eObjectType);
	C_Gate_DataModel_Object*		GetNextObject();

// PUBLIC DATA
public:
	C_Gate_DataModel_Progress		*m_pProgress;						// Progress dialog when analyzes in progress ...

// PRIVATE METHODS
private:
	C_Gate_DataModel_Object*		GetFirstObject_Private(ObjectType eObjectType);
	C_Gate_DataModel_Object*		GetNextObject_Private(ObjectType eObjectType);

// PRIVATE DATA
private:
	ObjectType						m_eObjectType;

// PROTECTED METHODS
protected:

// PROTECTED DATA
protected:
	C_Gate_DataModel_Object			*m_pParent;							// Parent object
	C_Gate_DataModel_ObjectList		m_clChildren;						// List of children objects
	static C_Gate_DataModel_Object	*m_pRootForFirstNextSearch;			// Ptr on root node for first/next object search
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: C_Gate_DataModel_Binning
// Holds binning result for 1 bin
/////////////////////////////////////////////////////////////////////////////////////////
class C_Gate_DataModel_Binning
{
// CONSTRUCTOR/DESTRUCTOR
public:
	C_Gate_DataModel_Binning(unsigned int uiBinNb);

// OPERATORS
public:
	C_Gate_DataModel_Binning& operator=(const C_Gate_DataModel_Binning& source);		// assignment operator

// PUBLIC METHODS
public:

// PUBLIC DATA
public:
	unsigned int	m_uiBinNb;				// Bin nb
	QString			m_strBinName;
	BYTE			m_bBinCat;
	unsigned int	m_uiBinCount;			// Count for this binning (including retest)
	unsigned int	m_uiBinCount_Retest;	// Retest count for this binning
	unsigned int	m_uiBinColor;

// PRIVATE METHODS
private:

// PRIVATE DATA
private:
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: C_Gate_DataModel_BinningList
// Holds binning result for 1 bin
/////////////////////////////////////////////////////////////////////////////////////////
class C_Gate_DataModel_BinningList : public QList<C_Gate_DataModel_Binning *>
{
// CONSTRUCTOR/DESTRUCTOR
public:
	C_Gate_DataModel_BinningList();
	~C_Gate_DataModel_BinningList() {clearItems();};

	enum SortOn
	{
		eNotSorted,
		eSortOnBinNb,			// Sorting should be done on Bin nb
		eSortOnBinCount			// Sorting should be done on Bin count
	};

// PUBLIC METHODS
public:
	void	InitBinning(unsigned int uiBinNb, QString strBinName, BYTE bBinCat);
	void	AddBinning(unsigned int uiBinNb, bool bBinPass, bool bRetest = FALSE);
	void	Sort(SortOn eSortSelector, bool bAscending);
	int		Find(unsigned int uiBinNb);

	void					clearItems();			// Clears all items of ptr list
	C_Gate_DataModel_Binning *GetFirst()		{ if(isEmpty()) return NULL;	m_iCurrentPos = 0;	return at(m_iCurrentPos);};
	C_Gate_DataModel_Binning *GetCurrent()	{ if(isEmpty()) return NULL;	if(m_iCurrentPos == -1) m_iCurrentPos = 0; return at(m_iCurrentPos);};
	C_Gate_DataModel_Binning *GetNext()		{ if(isEmpty()) return NULL;	if(m_iCurrentPos >= size()-1) return NULL; m_iCurrentPos++;	return at(m_iCurrentPos);};
	void					RemoveAll()		{ while (!isEmpty()) if(m_bAutoDelete) delete takeFirst(); else takeFirst(); m_iCurrentPos = -1;};
	void					RemoveCurrent()	{ if(isEmpty()) return; if(m_iCurrentPos >= size()) return; if(m_bAutoDelete) delete takeAt(m_iCurrentPos--); else takeAt(m_iCurrentPos--); };

// PUBLIC DATA
public:

// PRIVATE METHODS
private:

// PRIVATE DATA
private:
	int				m_iCurrentPos;
// PROTECTED METHODS
protected:
	static bool		lessThanSortOnBinNb(const C_Gate_DataModel_Binning* item1,const C_Gate_DataModel_Binning* item2);
	static bool		moreThanSortOnBinNb(const C_Gate_DataModel_Binning* item1,const C_Gate_DataModel_Binning* item2);
	static bool		lessThanSortOnBinCount(const C_Gate_DataModel_Binning* item1,const C_Gate_DataModel_Binning* item2);
	static bool		moreThanSortOnBinCount(const C_Gate_DataModel_Binning* item1,const C_Gate_DataModel_Binning* item2);

// PROTECTED DATA
protected:
	bool			m_bAutoDelete;
	// For sorting
	bool			m_bSortAscending;	// TRUE if sorting should be ascending
	int				m_eSortSelector;	// Tells on which var to sort
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: C_Gate_DataModel_TestResult
// Holds 1 test result
/////////////////////////////////////////////////////////////////////////////////////////
class C_Gate_DataModel_TestResult
{
// CONSTRUCTOR/DESTRUCTOR
public:
	C_Gate_DataModel_TestResult();

// OPERATORS
public:
	C_Gate_DataModel_TestResult& operator=(const C_Gate_DataModel_TestResult& source);		// assignment operator

// PUBLIC METHODS
public:

// PUBLIC DATA
public:
	float			m_fResult;
	unsigned int	m_uiFlags;

// PRIVATE METHODS
private:

// PRIVATE DATA
private:
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: C_Gate_DataModel_TestResult_Item
// Holds 1 test result item for a test result vector
/////////////////////////////////////////////////////////////////////////////////////////
class C_Gate_DataModel_TestResult_Item
{
// CONSTRUCTOR/DESTRUCTOR
public:
	C_Gate_DataModel_TestResult_Item(C_Gate_DataModel_TestResult *pTestResult, unsigned int uiRunIndex, unsigned int uiParameterIndex);

// PUBLIC METHODS
public:

// PUBLIC DATA
public:
	C_Gate_DataModel_TestResult	*m_pTestResult;
	unsigned int				m_uiRunIndex;
	unsigned int				m_uiParameterIndex;

// PRIVATE METHODS
private:

// PRIVATE DATA
private:
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: C_Gate_DataModel_TestResultVector
// Vector of C_Gate_DataModel_TestResult
/////////////////////////////////////////////////////////////////////////////////////////
class C_Gate_DataModel_TestResultVector : public QVector<C_Gate_DataModel_TestResult_Item *>
{
// CONSTRUCTOR/DESTRUCTOR
public:
	C_Gate_DataModel_TestResultVector();
	~C_Gate_DataModel_TestResultVector() {ClearValues();};

	enum SortOn
	{
		eNotSorted,
		eSortOnRunIndex,		// Sorting should be done on RunIndex
		eSortOnParameterIndex,	// Sorting should be done on ParameterIndex
		eSortOnResult			// Sorting should be done on m_pTestResult->m_fResult
	};

// PUBLIC METHODS
public:
	unsigned int CountValues() {return (unsigned int) size()-count(0);};
	void	InsertValue(C_Gate_DataModel_TestResult_Item *pItem) {insert(CountValues(), pItem);};
	void	RemoveIndex(unsigned int iIndex) {if(m_bAutoDelete && at(iIndex)) delete value(iIndex); remove(iIndex);};
	void	ClearValues() {if(m_bAutoDelete) {while(!empty()) RemoveIndex(0);} else remove(0,size()); Resize(0);};
	bool	Resize(unsigned int uiSize);
	void	Sort(SortOn eSortSelector, bool bAscending);
	void	FillFromParameter(C_Gate_DataModel_Site *pSite, unsigned int uiParameterIndex, ResultFilter eResultFilter = eResult_Any);
	void	FillFromRun(C_Gate_DataModel_Site *pSite, unsigned int uiRunIndex);

// PUBLIC DATA
public:

// PRIVATE METHODS
private:

// PRIVATE DATA
private:
	bool	m_bAutoDelete;

// PROTECTED METHODS
protected:
	static bool		lessThanSortOnRunIndex(const C_Gate_DataModel_TestResult_Item * item1, const C_Gate_DataModel_TestResult_Item * item2);
	static bool		moreThanSortOnRunIndex(const C_Gate_DataModel_TestResult_Item * item1, const C_Gate_DataModel_TestResult_Item * item2);
	static bool		lessThanSortOnParameterIndex(const C_Gate_DataModel_TestResult_Item * item1, const C_Gate_DataModel_TestResult_Item * item2);
	static bool		moreThanSortOnParameterIndex(const C_Gate_DataModel_TestResult_Item * item1, const C_Gate_DataModel_TestResult_Item * item2);
	static bool		lessThanSortOnResult(const C_Gate_DataModel_TestResult_Item * item1, const C_Gate_DataModel_TestResult_Item * item2);
	static bool		moreThanSortOnResult(const C_Gate_DataModel_TestResult_Item * item1, const C_Gate_DataModel_TestResult_Item * item2);

// PROTECTED DATA
protected:
	// For sorting
	bool			m_bSortAscending;	// TRUE if sorting should be ascending
	int				m_eSortSelector;	// Tells on which var to sort
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: C_Gate_DataModel_HistoClass
// Holds histogram class data
/////////////////////////////////////////////////////////////////////////////////////////
class C_Gate_DataModel_HistoClass
{
// CONSTRUCTOR/DESTRUCTOR
public:
	C_Gate_DataModel_HistoClass();

// OPERATORS
public:

// PUBLIC METHODS
public:
	void ClearData();

// PUBLIC DATA
public:
	float			m_fLowLimit;
	float			m_fHighLimit;
	unsigned int	m_uiSamples;

// PRIVATE METHODS
private:

// PRIVATE DATA
private:
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: C_Gate_DataModel_Histogram
// Holds histogram data for 1 parameter
/////////////////////////////////////////////////////////////////////////////////////////
class C_Gate_DataModel_Histogram
{
// CONSTRUCTOR/DESTRUCTOR
public:
	C_Gate_DataModel_Histogram();
	~C_Gate_DataModel_Histogram();

// OPERATORS
public:

// PUBLIC METHODS
public:
	void	ClearData();
	bool	Init(float fMinValue, float fMaxValue, unsigned int uiNbClasses, float fLowLimit=-3.4e+38F, float fHighLimit=3.4e+38F);
	void	UpdateClasses(float fValue, float fMin, float fMax);

// PUBLIC DATA
public:
	unsigned int				m_uiNbClasses;
	unsigned int				m_uiNbNonEmptyClasses;
	C_Gate_DataModel_HistoClass	*m_pHistoClasses;

// PRIVATE METHODS
private:

// PRIVATE DATA
private:
};

class C_Gate_DataModel_ParameterDef;

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: C_Gate_DataModel_ParameterStat
// Holds statistics for 1 parameter
/////////////////////////////////////////////////////////////////////////////////////////
class C_Gate_DataModel_ParameterStat
{
// CONSTRUCTOR/DESTRUCTOR
public:
	C_Gate_DataModel_ParameterStat();
	~C_Gate_DataModel_ParameterStat();

// OPERATORS
public:


	// List of possible Distribution shaped detected.
	enum ShapeDistribution
	{
		eShapeUnknow,							// Unknown
		eShapeCategory,							// Category
		eShapeMultiModal,						// Multi-Modal (bi-modal partially merged, or more modes)
		eShapeClampedDouble,					// Double Clamped
		eShapeClampedLeft,						// Clamped (left clamped)
		eShapeClampedRight,						// Clamped (right clamped)
		eShapeLogNormalLeft,					// LogNormal: Left tailed
		eShapeLogNormalRight,					// LogNormal: Righttailed
		eShapeGaussianLeft,						// Gaussian: Left tailed
		eShapeGaussianRight,					// Gaussian: Right tailed
		eShapeGaussian,							// Gaussian
		eShapeBiModal							// Bi-Modal with each mode clearly appart from the other
	};


// PUBLIC METHODS
public:
	void				ClearData();
	bool				InitHistogram(unsigned int uiNbClasses);
	void				UpdateHistogram(float fValue);
	bool				InitXBarTrend(float fMin, float fMax, unsigned int uiNbClasses);
	void				UpdateXBarTrend(float fMin, float fMax, float fValue);
	bool				InitRangeTrend(float fMin, float fMax, unsigned int uiNbClasses);
	void				UpdateRangeTrend(float fMin, float fMax, float fValue);
	void				UpdateShapeDistribution(C_Gate_DataModel_ParameterStat::ShapeDistribution  eShapeDistribution);
	void				ClearShapeDistribution();
	QString				GetShapeDistributionName(bool bShortName=true);

// PUBLIC DATA
public:
	// Static information for sort by TestNumber and TestName
	C_Gate_DataModel_ParameterDef	*m_pclParameter;		// Parameter definitions
	unsigned int			m_uiParameterIndex;			// This is equal to the index in the stats array, but we need the parameter index here also, when using the vectors for sorting


	int						m_nResScaleFactor;			// Scale computed to display results from this stat

	unsigned int			m_uiFlags;					// Some flags for parameter results
	unsigned int			m_uiOutliers_H;				// Nb of outliers (towards high values)
	unsigned int			m_uiOutliers_L;				// Nb of outliers (towards low values)
	unsigned int			m_uiOutliers_H_GoodPart;	// Nb of outliers (towards high values) on good parts
	unsigned int			m_uiOutliers_L_GoodPart;	// Nb of outliers (towards low values) on good parts
	unsigned int			m_uiOutliers_H_LastRetest;	// Nb of outliers (towards high values) on good parts
	unsigned int			m_uiOutliers_L_LastRetest;	// Nb of outliers (towards low values) on good parts
	unsigned int			m_uiExecs;					// Nb of samples
	unsigned int			m_uiPass;					// Nb of Pass
	unsigned int			m_uiFail;					// Nb of Fails
	unsigned int			m_uiFail_L;					// Nb of Fails under LL
	unsigned int			m_uiFail_H;					// Nb of Fails over HL
	double					m_lfSumX;					// Sum of samples
	double					m_lfSumX2;					// Sum of square of samples
	float					m_fMin;						// Minimum sample value
	float					m_fMax;						// Maximum sample value
	float					m_fRange;					// Range of values (Max - Min)
	float					m_fQ1;						// 1st quartile
	float					m_fQ2;						// 2nd quartile
	float					m_fQ3;						// 3d quartile
	float					m_fIQR;						// Inter Quartile Range (Q3-Q1)
	float					m_fIQS;						// Inter Quartile Sigma (Sigma of samples in [Q1-Q3] space)
	float					m_fMean;					// Mean of samples
	float					m_fSigma;					// Standard deviation of samples
	float					m_fRobustMean;				// Robust mean (Q2)
	float					m_fRobustSigma;				// Robust sigma (Q3-Q1)/1.35
	float					m_fCpkL;					// Low Cpk
	float					m_fCpkH;					// High Cpk
	float					m_fCpk;						// Cpk of samples (min(CpkL, CpkH))
	float					m_fCp;						// Cp of samples

	C_Gate_DataModel_Histogram	*m_pHistogram;				// Histogram classes for parameter
	C_Gate_DataModel_Histogram	*m_pHistogramSmooth;		// Histogram classes for parameter adapt to a limit space to smooth the data

	enum ShapeDistribution  m_eShapeDistribution;
	long					m_lDifferentValues;
	double					m_lfSamplesSkew;			// Population degree of asymmetry of a distribution around its mean
	double					m_lfSamplesKurtosis;		// Population Kurtosis or relative peakedness or flatness of a distribution compared with the normal distribution

// X Bar R METHOD
	unsigned int			m_uiNbPointsPerSubGroup;	// Nb values per SubGroup
	float					m_fXBarLCL;					// XBar Limit Control
	float					m_fXBarBar;					// XBar Limit Control
	float					m_fXBarUCL;					// XBar Limit Control
	float					m_fRLCL;					// R Limit Control
	float					m_fRBar;					// R Limit Control
	float					m_fRUCL;					// R Limit Control

// PRIVATE METHODS
private:

// PRIVATE DATA
private:
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: C_Gate_DataModel_ParameterStatVector
// Vector of C_Gate_DataModel_ParameterStat
/////////////////////////////////////////////////////////////////////////////////////////
class C_Gate_DataModel_ParameterStatVector : public QVector<C_Gate_DataModel_ParameterStat *>
{
// CONSTRUCTOR/DESTRUCTOR
public:
	C_Gate_DataModel_ParameterStatVector();
	~C_Gate_DataModel_ParameterStatVector() {ClearValues();};

	enum SortOn
	{
		eNotSorted,
		eSortOnParameterIndex,			// Sorting should be done on ParameterIndex
		eSortOnOutliers,				// Sorting should be done on nb of Outliers
		eSortOnOutliers_L,				// Sorting should be done on nb of Outliers
		eSortOnOutliers_H,				// Sorting should be done on nb of Outliers
		eSortOnOutliers_GoodRun,		// Sorting should be done on nb of Outliers on good runs
		eSortOnOutliers_LatestRetest,	// Sorting should be done on nb of Outliers on latest retests
		eSortOnCpk,						// Sorting should be done on Cpk
		eSortOnCp,						// Sorting should be done on Cp
		eSortOnMean,					// Sorting should be done on Mean
		eSortOnMin,						// Sorting should be done on Min
		eSortOnMax,						// Sorting should be done on Max
		eSortOnRange,					// Sorting should be done on Range
		eSortOnSigma,					// Sorting should be done on Sigma
		eSortOnQ1,						// Sorting should be done on Q1
		eSortOnQ2,						// Sorting should be done on Q2
		eSortOnQ3,						// Sorting should be done on Q3
		eSortOnExecs,					// Sorting should be done on nb of Execs
		eSortOnPass,					// Sorting should be done on nb of Pass
		eSortOnFail,					// Sorting should be done on nb of Fails
		eSortOnFail_L,					// Sorting should be done on nb of Fails
		eSortOnFail_H,					// Sorting should be done on nb of Fails
		eSortOnTestNumber,				// Sorting should be done on Test Number
		eSortOnTestName					// Sorting should be done on Test Name
	};

// PUBLIC METHODS
public:
	int		CountValues() {return size()-count(0);};
	void	InsertValue(C_Gate_DataModel_ParameterStat *pItem) {insert(CountValues(), pItem);};
	void	RemoveIndex(unsigned int iIndex) {if(m_bAutoDelete && at(iIndex)) delete value(iIndex); remove(iIndex);};
	void	ClearValues() {if(m_bAutoDelete) {while(!empty()) RemoveIndex(0);} else remove(0,size()); Resize(0);};
	bool	Resize(unsigned int uiSize);
	void	Sort(SortOn eSortSelector, bool bAscending);
	void	Fill(C_Gate_DataModel_Site *pSite, ParameterFilter eParameterFilter = eParameter_Any);
	QString SortOnToString(int eSortSelector = -1, bool bAscending = true);

// PUBLIC DATA
public:

// PRIVATE METHODS
private:

// PRIVATE DATA
private:
	bool	m_bAutoDelete;

// PROTECTED METHODS
protected:
	static bool		lessThanSortOnParameterIndex(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		moreThanSortOnParameterIndex(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		lessThanSortOnOutliers(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		moreThanSortOnOutliers(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		lessThanSortOnOutliers_L(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		moreThanSortOnOutliers_L(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		lessThanSortOnOutliers_H(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		moreThanSortOnOutliers_H(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		lessThanSortOnOutliers_GoodRun(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		moreThanSortOnOutliers_GoodRun(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		lessThanSortOnOutliers_LatestRetest(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		moreThanSortOnOutliers_LatestRetest(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		lessThanSortOnCpk(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		moreThanSortOnCpk(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		lessThanSortOnCp(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		moreThanSortOnCp(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		lessThanSortOnMean(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		moreThanSortOnMean(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		lessThanSortOnMin(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		moreThanSortOnMin(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		lessThanSortOnMax(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		moreThanSortOnMax(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		lessThanSortOnRange(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		moreThanSortOnRange(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		lessThanSortOnSigma(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		moreThanSortOnSigma(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		lessThanSortOnQ1(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		moreThanSortOnQ1(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		lessThanSortOnQ2(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		moreThanSortOnQ2(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		lessThanSortOnQ3(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		moreThanSortOnQ3(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		lessThanSortOnExecs(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		moreThanSortOnExecs(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		lessThanSortOnPass(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		moreThanSortOnPass(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		lessThanSortOnFail(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		moreThanSortOnFail(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		lessThanSortOnFail_L(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		moreThanSortOnFail_L(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		lessThanSortOnFail_H(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		moreThanSortOnFail_H(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		lessThanSortOnTestNumber(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		moreThanSortOnTestNumber(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		lessThanSortOnTestName(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);
	static bool		moreThanSortOnTestName(const C_Gate_DataModel_ParameterStat * item1, const C_Gate_DataModel_ParameterStat * item2);

// PROTECTED DATA
protected:
	// For sorting
	bool			m_bSortAscending;	// TRUE if sorting should be ascending
	int				m_eSortSelector;	// Tells on which var to sort
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: C_Gate_DataModel_BinningVector
// Vector of C_Gate_DataModel_Binning
/////////////////////////////////////////////////////////////////////////////////////////
class C_Gate_DataModel_BinningVector : public QVector<C_Gate_DataModel_Binning *>
{
// CONSTRUCTOR/DESTRUCTOR
public:
	C_Gate_DataModel_BinningVector();
	~C_Gate_DataModel_BinningVector() {ClearValues();};

	enum SortOn
	{
		eNotSorted,
		eSortOnExecs,					// Sorting should be done on nb of Execs
		eSortOnTestNumber,				// Sorting should be done on Test Number
		eSortOnTestName,				// Sorting should be done on Test Name
		eSortOnPass						// Sorting should be done on bin cat
	};

// PUBLIC METHODS
public:
	int		CountValues() {return size()-count(0);};
	void	InsertValue(C_Gate_DataModel_Binning *pItem) {insert(CountValues(), pItem);};
	void	RemoveIndex(unsigned int iIndex) {if(m_bAutoDelete && at(iIndex)) delete value(iIndex); remove(iIndex);};
	void	ClearValues() {if(m_bAutoDelete) {while(!empty()) RemoveIndex(0);} else remove(0,size()); Resize(0);};
	bool	Resize(unsigned int uiSize);
	void	Sort(SortOn eSortSelector, bool bAscending);
	void	Fill(C_Gate_DataModel_Site *pSite, ParameterFilter eParameterFilter ,bool bHardBinning);
	void	Fill(C_Gate_DataModel_Batch *pBatch, ParameterFilter eParameterFilter ,bool bHardBinning);

// PUBLIC DATA
public:

	bool			m_bHardBinning;		// TRUE if this is HArdBin

// PRIVATE METHODS
private:

// PRIVATE DATA
private:
	bool	m_bAutoDelete;

// PROTECTED METHODS
protected:
	static bool		lessThanSortOnExecs(const C_Gate_DataModel_Binning * item1, const C_Gate_DataModel_Binning * item2);
	static bool		moreThanSortOnExecs(const C_Gate_DataModel_Binning * item1, const C_Gate_DataModel_Binning * item2);
	static bool		lessThanSortOnTestNumber(const C_Gate_DataModel_Binning * item1, const C_Gate_DataModel_Binning * item2);
	static bool		moreThanSortOnTestNumber(const C_Gate_DataModel_Binning * item1, const C_Gate_DataModel_Binning * item2);
	static bool		lessThanSortOnTestName(const C_Gate_DataModel_Binning * item1, const C_Gate_DataModel_Binning * item2);
	static bool		moreThanSortOnTestName(const C_Gate_DataModel_Binning * item1, const C_Gate_DataModel_Binning * item2);
	static bool		lessThanSortOnPass(const C_Gate_DataModel_Binning * item1, const C_Gate_DataModel_Binning * item2);
	static bool		moreThanSortOnPass(const C_Gate_DataModel_Binning * item1, const C_Gate_DataModel_Binning * item2);

// PROTECTED DATA
protected:
	// For sorting
	bool			m_bSortAscending;	// TRUE if sorting should be ascending
	int				m_eSortSelector;	// Tells on which var to sort
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: C_Gate_DataModel_PartResult
// Holds 1 part result
/////////////////////////////////////////////////////////////////////////////////////////
class C_Gate_DataModel_PartResult
{
// CONSTRUCTOR/DESTRUCTOR
public:
	C_Gate_DataModel_PartResult();

// OPERATORS
public:

// PUBLIC METHODS
public:

// PUBLIC DATA
public:
	unsigned int	m_uiRunIndex;	// This is equal to the index in the part result array, but we need the run index here also, when using the vectors for sorting
	QString			m_strPartID;
	int				m_nXLoc;
	int				m_nYLoc;
	int				m_nSoftBin;
	int				m_nHardBin;
	unsigned int	m_uiFlags;
	unsigned int	m_uiTestsExecuted;

// PRIVATE METHODS
private:

// PRIVATE DATA
private:
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: C_Gate_DataModel_PartResultVector
// Vector of C_Gate_DataModel_PartResult
/////////////////////////////////////////////////////////////////////////////////////////
class C_Gate_DataModel_PartResultVector : public QVector<C_Gate_DataModel_PartResult *>
{
// CONSTRUCTOR/DESTRUCTOR
public:
	C_Gate_DataModel_PartResultVector();
	~C_Gate_DataModel_PartResultVector() {ClearValues();};

	enum SortOn
	{
		eNotSorted,
		eSortOnPartID,			// Sorting should be done on PartID
		eSortOnXLoc,			// Sorting should be done on XLoc
		eSortOnYLoc,			// Sorting should be done on YLoc
		eSortOnSoftBin,			// Sorting should be done on SoftBin
		eSortOnHardBin,			// Sorting should be done on HardBin
		eSortOnTestsExecuted	// Sorting should be done on TestsExecuted
	};

// PUBLIC METHODS
public:
	unsigned int CountValues() {return (unsigned int) size()-count(0);};
	void	InsertValue(C_Gate_DataModel_PartResult *pItem) {insert(CountValues(), pItem);};
	void	RemoveIndex(unsigned int iIndex) {if(m_bAutoDelete && at(iIndex)) delete value(iIndex); remove(iIndex);};
	void	ClearValues() {if(m_bAutoDelete) {while(!empty()) RemoveIndex(0);} else remove(0,size()); Resize(0);};
	bool	Resize(unsigned int uiSize);
	void	Sort(SortOn eSortSelector, bool bAscending);
	void	Fill(C_Gate_DataModel_Site *pSite, RunFilter eRunFilter = eRun_Any, int nXpos=-1, int nYPos=-1);

// PUBLIC DATA
public:

// PRIVATE METHODS
private:

// PRIVATE DATA
private:
	bool	m_bAutoDelete;

// PROTECTED METHODS
protected:
	static bool		lessThanSortOnPartID(const C_Gate_DataModel_PartResult * item1, const C_Gate_DataModel_PartResult * item2);
	static bool		moreThanSortOnPartID(const C_Gate_DataModel_PartResult * item1, const C_Gate_DataModel_PartResult * item2);
	static bool		lessThanSortOnXLoc(const C_Gate_DataModel_PartResult * item1, const C_Gate_DataModel_PartResult * item2);
	static bool		moreThanSortOnXLoc(const C_Gate_DataModel_PartResult * item1, const C_Gate_DataModel_PartResult * item2);
	static bool		lessThanSortOnYLoc(const C_Gate_DataModel_PartResult * item1, const C_Gate_DataModel_PartResult * item2);
	static bool		moreThanSortOnYLoc(const C_Gate_DataModel_PartResult * item1, const C_Gate_DataModel_PartResult * item2);
	static bool		lessThanSortOnSoftBin(const C_Gate_DataModel_PartResult * item1, const C_Gate_DataModel_PartResult * item2);
	static bool		moreThanSortOnSoftBin(const C_Gate_DataModel_PartResult * item1, const C_Gate_DataModel_PartResult * item2);
	static bool		lessThanSortOnHardBin(const C_Gate_DataModel_PartResult * item1, const C_Gate_DataModel_PartResult * item2);
	static bool		moreThanSortOnHardBin(const C_Gate_DataModel_PartResult * item1, const C_Gate_DataModel_PartResult * item2);
	static bool		lessThanSortOnTestsExecuted(const C_Gate_DataModel_PartResult * item1, const C_Gate_DataModel_PartResult * item2);
	static bool		moreThanSortOnTestsExecuted(const C_Gate_DataModel_PartResult * item1, const C_Gate_DataModel_PartResult * item2);

// PROTECTED DATA
protected:
	// For sorting
	bool			m_bSortAscending;	// TRUE if sorting should be ascending
	int				m_eSortSelector;	// Tells on which var to sort
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: C_Gate_DataModel_EquipmentID
// Holds batch identification parameters
/////////////////////////////////////////////////////////////////////////////////////////
class C_Gate_DataModel_EquipmentID
{
// CONSTRUCTOR/DESTRUCTOR
public:
	C_Gate_DataModel_EquipmentID();
	~C_Gate_DataModel_EquipmentID();

// OPERATORS
public:
	bool operator==(const C_Gate_DataModel_EquipmentID &s);							// comparison operator
	bool operator!=(const C_Gate_DataModel_EquipmentID &s) { return !(*this == s); }	// comparison operator

// PUBLIC METHODS
public:
	void			ClearData();					// Clear object data (delete allocated ressources)

	// For debug
	void			Dump(QTextStream & hDumpFile);	// Dump all data structure

// PUBLIC DATA
public:
	QString			m_strTesterType;				// Tester type
	QString			m_strTesterName;				// Tester name
	QString			m_strHandlerProberID;			// Handler/Prober ID
	QString			m_strProbeCardID;				// Prober card ID
	QString			m_strLoadBoardID;				// Loadboard ID
	QString			m_strDibBoardID;				// DibBoard ID
	QString			m_strInterfaceCableID;			// Interface cable ID
	QString			m_strHandlerContactorID;		// Handler Contactor ID
	QString			m_strLaserID;					// Laser ID
	QString			m_strExtraEquipmentID;			// Extra equipment ID

// PRIVATE METHODS
private:

// PRIVATE DATA
private:
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: C_Gate_DataModel_ParameterDef
// Definition of a parameter (name, number, index, limits...)
/////////////////////////////////////////////////////////////////////////////////////////
class C_Gate_DataModel_ParameterDef
{
// CONSTRUCTOR/DESTRUCTOR
public:
	C_Gate_DataModel_ParameterDef();
	~C_Gate_DataModel_ParameterDef();

// OPERATORS
public:
	C_Gate_DataModel_ParameterDef& operator=(const C_Gate_DataModel_ParameterDef& source);		// assignment operator
	bool operator==(const C_Gate_DataModel_ParameterDef &s);								// comparison operator
	bool operator!=(const C_Gate_DataModel_ParameterDef &s) { return !(*this == s); }	// comparison operator

// PUBLIC METHODS
public:
	void					ClearData();		// Clear object data (delete allocated ressources)
	QString					GetParameterType(bool bShortDescription=true);
	void					ComputeScaleFromStat(C_Gate_DataModel_ParameterStat *pParameterStat, bool bUseLimitRange=false);
	QString					GetScaleUnitFromStat();
	QString					GetScaleUnitFromStat(int nScale);
	QString					FormatValue(float fValue);
	QString					FormatValue(float fValue, int nScale, bool bWithUnit=false, int nNbDigit = 6);
	QString					PrintableValue(float fValue, int nNbDigit = 6, bool bUseStatScale=false);
	QString					NormalizedUnit();

// PUBLIC DATA
public:

	unsigned int			m_uiNumber;			// Parameter number (100, 110, 200, ...)
	BYTE					m_bType;			// Test type ('P'=parametric, 'F'=functional, '-'=gex custom...)
	QString					m_strName;			// Parameter name
	QString					m_strUnits;			// Parameter units
	unsigned int			m_uiPinmapIndex;	// Index in pinmap
	float					m_fLL;				// Parameter low limit
	int						m_nLLScaleFactor;	// Scale factor for LL
	float					m_fHL;				// Parameter high limit
	int						m_nHLScaleFactor;	// Scale factor for HL
	unsigned int			m_uiFlags;			// Parameter flags
	unsigned int			m_uiLimitFlags;		// Type of limits
	int						m_nResScaleFactor;	// Scale factor for results

// PRIVATE METHODS
private:

// PRIVATE DATA
private:
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: C_Gate_DataModel_ParameterSet
// Set of parameters
/////////////////////////////////////////////////////////////////////////////////////////
class C_Gate_DataModel_ParameterSet
{
// ERROR MAP
public:
	GDECLARE_ERROR_MAP(C_Gate_DataModel_ParameterSet)
	{
		eMalloc,					// Failed Allocating memory
		eParameterIndexOverflow,	// Parameter index too high (out of allocated memory)
		eNotAllParametersDefined	// Not all declared parameters are defined
	}
	GDECLARE_END_ERROR_MAP(C_Gate_DataModel_ParameterSet)

// CONSTRUCTOR/DESTRUCTOR
public:
	C_Gate_DataModel_ParameterSet();
	~C_Gate_DataModel_ParameterSet();

// OPERATORS
public:
	bool operator==(const C_Gate_DataModel_ParameterSet &s);								// comparison operator
	bool operator!=(const C_Gate_DataModel_ParameterSet &s) { return !(*this == s); }	// comparison operator

// PUBLIC METHODS
public:
	void						ClearData();		// Clear object data (delete allocated ressources)
	bool						Init(unsigned int uiNbParameters);							// Initialize testing stage
	bool						SetParameter(const Gate_ParameterDef & ParameterDefinition);	// Set parameter definition
	bool						Check();			// Check if parameter set is OK

	// For debug
	void						Dump(QTextStream & hDumpFile);						// Dump all data structure
	bool						SetStdfRecordInformation(const QString strWarning, uint &uiNumber, QString &strName, QString &strType, uint &uiFlags);
	QStringList					GetStdfRecordInformation(const uint uiFlags, int uiParameterIndex=-1);

// PUBLIC DATA
public:
	unsigned int				m_uiParameterSetNb;		// Nb identifying the parameter set
	unsigned int				m_uiNbParameters;		// Nb of parameters
	unsigned int				m_uiNbValidParameters;	// Nb of valid parameters (for which a definition has been received)
	unsigned int				m_uiNbCustomParameters;	// Nb of custom parameters (binning, StdfRecord, ...)
	C_Gate_DataModel_ParameterDef	*m_pclParameters;	// Array of parameter definitions

// PRIVATE METHODS
private:
	bool						HasParametricUnits(const QString & strUnit);

// PRIVATE DATA
private:
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: C_Gate_DataModel_ParameterSetList
// Holds a list of Parameter sets (C_Gate_DataModel_ParameterSet)
/////////////////////////////////////////////////////////////////////////////////////////
class C_Gate_DataModel_ParameterSetList: public QList<C_Gate_DataModel_ParameterSet *>
{
// CONSTRUCTOR/DESTRUCTOR
public:
	C_Gate_DataModel_ParameterSetList();
	~C_Gate_DataModel_ParameterSetList() {clearItems();};

// PUBLIC METHODS
public:
	void appendItem(C_Gate_DataModel_ParameterSet* pParameterSet);	// Sets ParameterSet nb and appends item into ptr list
	void clearItems();											// Resets ParameterSet nb, and clears all items of ptr list

	C_Gate_DataModel_ParameterSet *GetFirst()		{ if(isEmpty()) return NULL;	m_iCurrentPos = 0;	return at(m_iCurrentPos);};
	C_Gate_DataModel_ParameterSet *GetCurrent()	{ if(isEmpty()) return NULL;	if(m_iCurrentPos == -1) m_iCurrentPos = 0; return at(m_iCurrentPos);};
	C_Gate_DataModel_ParameterSet *GetNext()		{ if(isEmpty()) return NULL;	if(m_iCurrentPos >= size()-1) return NULL; m_iCurrentPos++;	return at(m_iCurrentPos);};
	void						RemoveAll()		{ while (!isEmpty()) if(m_bAutoDelete) delete takeFirst(); else takeFirst(); m_iCurrentPos = -1;};
	void						RemoveCurrent()	{ if(isEmpty()) return; if(m_iCurrentPos >= size()) return; if(m_bAutoDelete) delete takeAt(m_iCurrentPos--); else takeAt(m_iCurrentPos--); };

// PUBLIC DATA
public:

// PRIVATE METHODS
private:

// PRIVATE DATA
private:
	int				m_iCurrentPos;

	bool			m_bAutoDelete;
	unsigned int	m_uiParameterSetNb;
};

class C_Gate_DataModel_WaferPoint;

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: C_Gate_DataModel_WaferZone
// Holds statistics for 1 Wafer Zone (12 total)
/////////////////////////////////////////////////////////////////////////////////////////
class C_Gate_DataModel_WaferZone
{
// ERROR MAP
public:
	GDECLARE_ERROR_MAP(C_Gate_DataModel_WaferZone)
	{
		eMalloc,					// Failed Allocating memory
		eWaferZoneIndexOverflow,	// Parameter index too high (out of allocated memory)
	}
	GDECLARE_END_ERROR_MAP(C_Gate_DataModel_WaferZone)

// CONSTRUCTOR/DESTRUCTOR
public:
	C_Gate_DataModel_WaferZone();
	~C_Gate_DataModel_WaferZone();

// OPERATORS
public:

// PUBLIC METHODS
public:
	bool			InitWaferZone(int nZoneIndex, int nSiteIndex, int nMaxSoftBin, int nMaxHardBin, int nNbZones, int nNbSites, int nNbParameters);
	void			ClearData();
	bool			InitData();
	bool			UpdateData(C_Gate_DataModel_Batch* pBatch, C_Gate_DataModel_PartResult	clPartResult);
	bool			UpdateStats(C_Gate_DataModel_Batch* pBatch, C_Gate_DataModel_TestResultVector *pTestResultVector);

// PUBLIC DATA
public:
	unsigned int				m_uiFlags;					// Some flags
	unsigned int				m_uiExecs;					// Nb of samples
	unsigned int				m_uiPass;					// Nb of parts pass
	unsigned int				m_uiFail;					// Nb of parts fail

	int							m_nZoneIndex;				// Zone number
	int							m_nSiteIndex;				// Site number
	int							m_nMaxSoftBin;				// Nb of SoftBin
	int							m_nMaxHardBin;				// Nb of HardBin
	int*						m_pnSoftBin;				// [nSoftBin] = total exec with bin = nSoftBin
	int*						m_pnHardBin;				// [nHardBin] = total exec with bin = nHardBin
	int							m_nNbZones;					// Number max of Zones
	int							m_nNbSites;					// Number max of sites
	int							m_nNbParameters;			// Number max of parameters
	C_Gate_DataModel_ParameterStat*	m_pZoneParameterStats;		// [nParameterIndex] Information collected for this zone and this site


// PRIVATE METHODS
private:

// PRIVATE DATA
private:
};



/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: C_Gate_DataModel_WaferPoint
// Holds 1 part and tests result item for a point in a WaferMap
/////////////////////////////////////////////////////////////////////////////////////////
class C_Gate_DataModel_WaferPoint
{
// CONSTRUCTOR/DESTRUCTOR
public:
	C_Gate_DataModel_WaferPoint();
	~C_Gate_DataModel_WaferPoint();

// PUBLIC METHODS
public:

// PUBLIC DATA
public:
	int						m_nZoneIndex;		// the WaferMap was divised between X zones for stat computations
												// if the zone index is -1, the point doesn't contain information (out of wafer or not executed)
	int						m_nRunIndex;		// Depending of the position in the WaferMap
	int						m_nSiteIndex;		// Depending of the position in the WaferMap


	int						m_nSoftBin;			// result for Soft Bin
	int						m_nHardBin;			// result for Hard Bin
	int						m_nPass;			// pass result, 0 if not executed

	int						m_nNbParameters;	// Number max of parameters
	int*					m_pnParameterPass;	// [nParameterIndex] = pass result, 0 if not executed
	float*					m_pfParameterValues;// [nParameterIndex] = value result
	int*					m_pnParameterPlots;	// [nParameterIndex] = normalized result (under 0 and 100)
// PRIVATE METHODS
private:

// PRIVATE DATA
private:
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: C_Gate_DataModel_WaferMap
// Holds WaferPoint for WaferMap
/////////////////////////////////////////////////////////////////////////////////////////
class C_Gate_DataModel_WaferMap
{
// ERROR MAP
public:
	GDECLARE_ERROR_MAP(C_Gate_DataModel_WaferMap)
	{
		eMalloc,					// Failed Allocating memory
		eWaferMapIndexOverflow,		// Parameter index too high (out of allocated memory)
	}
	GDECLARE_END_ERROR_MAP(C_Gate_DataModel_WaferMap)

// CONSTRUCTOR/DESTRUCTOR
public:
	C_Gate_DataModel_WaferMap();
	~C_Gate_DataModel_WaferMap();

// PUBLIC METHODS
public:
	bool	InitializeWaferMap(C_Gate_DataModel_Batch* pBatch);	// allocate memory
	bool	UpdateWaferPoints(C_Gate_DataModel_Batch* pBatch);		// initialize and compute stats
	bool	UpdateWaferZone(C_Gate_DataModel_Batch* pBatch, int nZoneIndex, int nSiteIndex);		// initialize and compute stats
	void	ClearWaferMap();								// free memory

	int		GiveZoneIndex(int nXLoc, int nYLoc, bool bCutPeriph = FALSE);// Give the zone index for X,Y in the current WaferMap
	bool	UpdateParameterStat(C_Gate_DataModel_ParameterStat* pParameterStat,const C_Gate_DataModel_ParameterStat* pInitParameterStat); // for first initialization

	float	WaferZoneHasCircleDeviation(float fLimitDeviation, bool bUsePass=true, bool bSoftBin=true, int nBinIndex=0, int iParameterIndex = -1);
	float	WaferZoneHasDiagonalLeftTopDeviation(float fLimitDeviation, bool bUsePass=true, bool bSoftBin=true, int nBinIndex=0, int iParameterIndex = -1);
	float	WaferZoneHasDiagonalLeftBottomDeviation(float fLimitDeviation, bool bUsePass=true, bool bSoftBin=true, int nBinIndex=0, int iParameterIndex = -1);
	float	WaferZoneHasHorizontalDeviation(float fLimitDeviation, bool bUsePass=true, bool bSoftBin=true, int nBinIndex=0, int iParameterIndex = -1);
	float	WaferZoneHasVerticalDeviation(float fLimitDeviation, bool bUsePass=true, bool bSoftBin=true, int nBinIndex=0, int iParameterIndex = -1);

// PUBLIC DATA
public:

	unsigned int						m_uiFlags;		// Define witch stats you want (only binning wafer map or also parametric wafermap

	// Definition of the position in the wafermap
	int									m_nXOffset;		// Delta between the tab and the original position
	int									m_nYOffset;
	int									m_nXMax;		// Dim of the tab
	int									m_nYMax;

	// Static Information

	int									m_nNbZones;		// Number max of zones
	int									m_nNbSites;		// Number max of sites
	int									m_nNbParameters;// Number max of parameters

	// Information collected
	C_Gate_DataModel_WaferPoint**		m_pWaferPoints;			// [nXMax][nYMax]
	C_Gate_DataModel_WaferZone**		m_pWaferZones;			// [nZoneIndex+1][nSiteIndex+1], [0][0] regroups all zones and all sites

// PRIVATE METHODS
private:
// PRIVATE DATA
private:
};



/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: C_Gate_DataModel_Site
// Holds test results for a specific site
/////////////////////////////////////////////////////////////////////////////////////////
class C_Gate_DataModel_Site : public C_Gate_DataModel_Object
{
// ERROR MAP
public:
	GDECLARE_ERROR_MAP(C_Gate_DataModel_Site)
	{
		eMalloc,					// Failed Allocating memory
		eParameterIndexOverflow,	// Parameter index too high (out of allocated memory)
		eRunIndexOverflow,			// Run index too high (out of allocated memory)
		eNoMemForResults			// No memory allocated to store test results
	}
	GDECLARE_END_ERROR_MAP(C_Gate_DataModel_Site)

// CONSTRUCTOR/DESTRUCTOR
public:
	C_Gate_DataModel_Site(C_Gate_DataModel_Batch *pParent, C_Gate_DataModel_Progress *pProgress = NULL);
	~C_Gate_DataModel_Site();

// PUBLIC METHODS
public:
	C_Gate_DataModel_Batch*	GetParent(){ return (C_Gate_DataModel_Batch*)m_pParent; }// Return ptr on parent object
	void				ClearData();										// Clear object data (delete allocated ressources)
	void				Cleanup();											// Cleanup site object (remove empty structures...)
	bool				Init(unsigned int uiSiteIndex, int nSiteNb, const Gate_SiteDescription* pSiteDescription, const Gate_LotDef & LotDefinition, const C_Gate_DataModel_ParameterSet* pParameterSet);	// Initialize site object (allocate memory for results...)
	bool				SetTestResult(const Gate_DataResult & DataResult, C_Gate_DataModel_ParameterSet* pParameterSet);	// Sets a result in the test result array
	bool				SetPartResult(const Gate_PartResult & PartResult, bool bPartIsFail, bool bPartIsRetest);	// Sets a result in the part result array
	bool				SetBinning(const Gate_BinningDef & BinDef);			// Set binning definition

	C_Gate_DataModel_ParameterStatVector* GetParameterStatVector(C_Gate_DataModel_ParameterStatVector::SortOn eSortSelector, bool bAscending);
	C_Gate_DataModel_BinningVector* GetHardBinningVector(C_Gate_DataModel_BinningVector::SortOn eSortSelector, bool bAscending);
	C_Gate_DataModel_BinningVector* GetSoftBinningVector(C_Gate_DataModel_BinningVector::SortOn eSortSelector, bool bAscending);

	// For debug
	void				Dump(QTextStream & hDumpFile, C_Gate_DataModel_ParameterSet* pParameterSet);	// Dump all data structure

// PUBLIC DATA
public:
	unsigned int							m_uiSiteIndex;								// Site index (0, 1, 2...)
	int										m_nHeadNb;									// Head Nb
	int										m_nSiteNb;									// Site Nb
	C_Gate_DataModel_EquipmentID			m_clEquipmentID;							// Site-specific equipment ID
	unsigned int							m_uiNbParameters;							// Nb of parameters
	unsigned int							m_uiNbRuns;									// Nb of runs (program runs)
	unsigned int							m_uiNbParts;								// Nb of parts for this site
	unsigned int							m_uiNbPassParts;							// Nb of PASS parts for this site
	unsigned int							m_uiNbFailParts;							// Nb of FAIL parts for this site
	unsigned int							m_uiNbRetestParts;							// Nb of parts that are retests for this site
	unsigned int							m_uiNbFailPartsAfterRetest;					// Nb of FAIL parts after retests for this site
	C_Gate_DataModel_TestResult*			m_pTestResults;								// Array of test result structures
	C_Gate_DataModel_PartResult*			m_pPartResults;								// Array of part result structures
	C_Gate_DataModel_ParameterStat*			m_pParameterStats_AllData;					// Array of parameter stats (stats on all available data for each parameter)
	C_Gate_DataModel_ParameterStat*			m_pParameterStats_Outliers;					// Array of parameter stats (stats on outlier data for each parameter)
	C_Gate_DataModel_ParameterStat*			m_pParameterStats_Distribution;				// Array of parameter stats (stats on non-outlier data for each parameter)
	C_Gate_DataModel_BinningList			m_pSoftBins;								// Software Binnings for this site
	C_Gate_DataModel_BinningList			m_pHardBins;								// Hardware Binnings for this site


	C_Gate_DataModel_PartResultVector		m_clPartVector;
	C_Gate_DataModel_ParameterStatVector	m_clParameterStatVector;
	C_Gate_DataModel_BinningVector			m_clHardBinningVector;
	C_Gate_DataModel_BinningVector			m_clSoftBinningVector;
//	C_Y123_ResultSet						m_clResultSet_Failures;						// Ruleset result object for failures analysis
//	C_Y123_ResultSet						m_clResultSet_Outliers;						// Ruleset result object for outliers analysis
//	C_Y123_ResultSet						m_clResultSet_Distribution;					// Ruleset result object for distribution analysis
//	C_Y123_ResultSet						m_clResultSet_Compliance;					// Ruleset result object for compliance analysis

// PRIVATE METHODS
private:

// PRIVATE DATA
private:
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: C_Gate_DataModel_BatchID
// Holds batch identification parameters
/////////////////////////////////////////////////////////////////////////////////////////
class C_Gate_DataModel_BatchID
{
// CONSTRUCTOR/DESTRUCTOR
public:
	C_Gate_DataModel_BatchID();
	~C_Gate_DataModel_BatchID();

// OPERATORS
public:
	bool operator==(const C_Gate_DataModel_BatchID &s);							// comparison operator
	bool operator!=(const C_Gate_DataModel_BatchID &s) { return !(*this == s); }	// comparison operator

// PUBLIC METHODS
public:
	void					ClearData();				// Clear object data (delete allocated ressources)

	// For debug
	void					Dump(QTextStream & hDumpFile);					// Dump all data structure

// PUBLIC DATA
public:
	// Lot information
	QString					m_strLotID;					// Lot ID
	QString					m_strSublotID;				// Sublot ID
	QString					m_strWaferID;				// Wafer ID

	// Test conditions
	C_Gate_DataModel_ParameterSet*	m_pParameterSet;		// Ptr to Parameter set
	unsigned int			m_uiNbSites;				// Nb of sites
	QString					m_strOperatorName;			// Operator name
	QString					m_strJobName;				// Job name
	QString					m_strJobRevision;			// Job revision
	QString					m_strExecType;				// Tester Exec type
	QString					m_strExecVersion;			// Tester Exec version
	QString					m_strTestCode;				// Test code
	QString					m_strTestTemperature;		// Test temperature

	// Equipment identification
	C_Gate_DataModel_EquipmentID	m_clEquipmentID;			// Equipment ID object

// PRIVATE METHODS
private:

// PRIVATE DATA
private:
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: C_Gate_DataModel_Batch
// Holds test results for a specific batch of results
/////////////////////////////////////////////////////////////////////////////////////////
class C_Gate_DataModel_Batch : public C_Gate_DataModel_Object
{
// ERROR MAP
public:
	GDECLARE_ERROR_MAP(C_Gate_DataModel_Batch)
	{
		eMultipleHeads,						// Data from several test heads
		eSiteNotInList,						// Attempt to set a result for a site that is not in the site list
		eNoParameterSet,					// Attempt to set a test result, but no Parameter set is available
		eSetTestResult,						// Error setting test result
		eSetPartResult						// Error setting part result
	}
	GDECLARE_END_ERROR_MAP(C_Gate_DataModel_Batch)

// CONSTRUCTOR/DESTRUCTOR
public:
	C_Gate_DataModel_Batch(C_Gate_DataModel_Sublot *pParent, C_Gate_DataModel_Progress *pProgress = NULL);
	~C_Gate_DataModel_Batch();

// OPERATORS
public:
	bool operator==(const C_Gate_DataModel_Batch &s);							// comparison operator
	bool operator!=(const C_Gate_DataModel_Batch &s) { return !(*this == s); }	// comparison operator

// PUBLIC METHODS
public:
	C_Gate_DataModel_Sublot*	GetParent(){ return (C_Gate_DataModel_Sublot*)m_pParent; }				// Return ptr on parent object
	void						ClearData();														// Clear object data (delete allocated ressources)
	void						Cleanup();															// Cleanup batch (remove empty sites..)
	int							NbSites() { return m_clChildren.count(); }
	bool						CheckHead(const Gate_SiteDescriptionMap* pSiteEquipmentIDMap);		// Make sure data comes from a single test head (we don't support multi-head data)
	bool						CheckHead(int nHeadNum);											// Make sure data comes from a single test head (we don't support multi-head data)
	bool						SetTestResult(const Gate_DataResult & DataResult);					// Set test result for specific parameter
	bool						SetPartResult(const Gate_PartResult & PartResult);					// Set part result for specific part
	bool						SetBinning(const Gate_BinningDef & BinDef);							// Set binning definition
	void						UpdateFromToDates(long lStartTime, long lFinishTime);				// Update Batch's From and To dates
	C_Gate_DataModel_Site*		GetSiteFromSiteIndex(unsigned int uiSiteIndex);						// Return ptr on site with specified site index
	C_Gate_DataModel_Site*		GetSiteFromSiteNumber(int nSiteNb);									// Return ptr on site with specified site nb
	void						AddSite(C_Gate_DataModel_Site* pSite) { m_clChildren.append(pSite); }		// Add a site to the sitelist of this batch
	C_Gate_DataModel_Site*		GetFirstSite() { return (C_Gate_DataModel_Site*)(m_clChildren.GetFirst()); }	// Get ptr on first site of this batch
	C_Gate_DataModel_Site*		GetNextSite() { return (C_Gate_DataModel_Site*)(m_clChildren.GetNext()); }	// Get ptr on next site of this batch
	C_Gate_DataModel_Site*		GetSiteAt(unsigned int uiIndex) { return (C_Gate_DataModel_Site*)m_clChildren.at(uiIndex); }				// Get ptr on site at specified position
	C_Gate_DataModel_Site*		RemoveCurrentSite() { m_clChildren.RemoveCurrent(); return (C_Gate_DataModel_Site*)(m_clChildren.GetCurrent()); }	// Remove current site, and return ptr on new current site
	void						ClearSites() { m_clChildren.RemoveAll(); }								// Clear list of sites
	bool						IsPartFail(const Gate_PartResult & PartResult);						// Check if Part is FAIL
	bool						IsPartRetest(const Gate_PartResult & PartResult);						// Check if part is a retest

	// FOR WAFERMAP STATS
	bool						InitWaferMap();								// Allocate and initialize all stats in m_pWaferMap

	// For debug
	void						Dump(QTextStream & hDumpFile);										// Dump all data structure

	C_Gate_DataModel_ParameterStatVector* GetParameterStatVector(C_Gate_DataModel_ParameterStatVector::SortOn eSortSelector, bool bAscending);
	C_Gate_DataModel_BinningVector* GetHardBinningVector(C_Gate_DataModel_BinningVector::SortOn eSortSelector, bool bAscending);
	C_Gate_DataModel_BinningVector* GetSoftBinningVector(C_Gate_DataModel_BinningVector::SortOn eSortSelector, bool bAscending);

// PUBLIC DATA
public:
	int								m_nHeadNum;						// Used to make sure batch has a single Head nb (we don't support multiple heads in same batch)
	C_Gate_DataModel_BatchID		m_clBatchID;					// Batch identification (unique key)
	QDateTime						m_clFromDate;					// Smallest Date/time found in this batch
	QDateTime						m_clToDate;						// Biggest Date/time found in this batch
//	C_Y123_ResultSet				m_clResultSet_Multisite;		// Ruleset result object for multisite analysis
//	C_Y123_ResultSet				m_clResultSet_Compliance;		// Ruleset result object for multisite analysis
	unsigned int					m_uiNbParameters;				// Nb of parameters
	unsigned int					m_uiNbRuns;						// Nb of runs (program runs)
	unsigned int					m_uiNbParts;					// Nb of parts (all sites)
	unsigned int					m_uiNbPassParts;				// Nb of PASS parts (all sites)
	unsigned int					m_uiNbFailParts;				// Nb of FAIL parts (all sites)
	unsigned int					m_uiNbRetestParts;				// Nb of parts that are retests (all sites)
	unsigned int					m_uiNbFailPartsAfterRetest;		// Nb of FAIL parts after retests (all sites)
	C_Gate_DataModel_BinningList	m_pSoftBins;					// Software Binnings (all sites)
	C_Gate_DataModel_BinningList	m_pHardBins;					// Hardware Binnings (all sites)
	C_Gate_DataModel_BinningList	m_pGoodBins_SoftBins;			// List of Bins considered good for Software Binning
	C_Gate_DataModel_BinningList	m_pGoodBins_HardBins;			// List of Bins considered good for Hardware Binning

	C_Gate_DataModel_BinningVector	m_clHardBinningVector;
	C_Gate_DataModel_BinningVector	m_clSoftBinningVector;

	bool							m_bWithWaferInfo;				// true if have valid X, Y
	C_Gate_DataModel_WaferMap*		m_pWaferMap;					// WaferMap statistic
//	C_Y123_ResultSet				m_clResultSet_WaferMap;			// Ruleset result object for wafermmap analysis

// PRIVATE METHODS
private:

// PRIVATE DATA
private:
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: C_Gate_DataModel_Sublot
// Holds test results for a specific sublot (wafer in wafer test)
/////////////////////////////////////////////////////////////////////////////////////////
class C_Gate_DataModel_Sublot : public C_Gate_DataModel_Object
{
// ERROR MAP
public:
	GDECLARE_ERROR_MAP(C_Gate_DataModel_Sublot)
	{
		eInitSite,					// Error initializing site
		eNoSite,					// No site was found in dataset (runs per site array has 0 for all sites)
		eNoParameterSet,			// Attempt to add a batch, but no Parameter set is available
		eCheckHead					// Error checking test heads (multiple test heads not supported)
	}
	GDECLARE_END_ERROR_MAP(C_Gate_DataModel_Sublot)

// CONSTRUCTOR/DESTRUCTOR
public:
	C_Gate_DataModel_Sublot(C_Gate_DataModel_Lot *pParent, C_Gate_DataModel_Progress *pProgress = NULL);
	C_Gate_DataModel_Sublot(C_Gate_DataModel_Lot *pParent, const QString &strLotID, const QString &strSublotID, const QString &strWaferID, C_Gate_DataModel_Progress *pProgress = NULL);
	~C_Gate_DataModel_Sublot();

// OPERATORS
public:
	bool operator==(const C_Gate_DataModel_Sublot &s);							// comparison operator
	bool operator!=(const C_Gate_DataModel_Sublot &s) { return !(*this == s); }	// comparison operator

// PUBLIC METHODS
public:
	C_Gate_DataModel_Lot*	GetParent(){ return (C_Gate_DataModel_Lot*)m_pParent; }						// Return ptr on parent object
	void					ClearData();															// Clear object data (delete allocated ressources)
	void					Cleanup();																// Cleanup sublot (remove empty batches..)
	int						NbBatches() { return m_clChildren.count(); }
	C_Gate_DataModel_Batch*	AddBatch(const Gate_LotDef & LotDefinition, C_Gate_DataModel_ParameterSet* pParameterSet, const Gate_SiteDescription& clGlobalEquipmentID, const Gate_SiteDescriptionMap* pSiteEquipmentIDMap);	// Get batch to be used to add data to be received (create it if no corresponding batch already in the list)
	void					UpdateFromToDates(long lStartTime, long lFinishTime);					// Update Sublot's From and To dates
	void					AddBatch(C_Gate_DataModel_Batch* pBatch) { m_clChildren.append(pBatch); }		// Add a batch to the batchlist of this sublot
	C_Gate_DataModel_Batch*	GetFirstBatch() { return (C_Gate_DataModel_Batch*)(m_clChildren.GetFirst()); }	// Get ptr on first batch of this sublot
	C_Gate_DataModel_Batch*	GetNextBatch() { return (C_Gate_DataModel_Batch*)(m_clChildren.GetNext()); }		// Get ptr on next batch of this sublot
	C_Gate_DataModel_Batch*	RemoveCurrentBatch() { m_clChildren.RemoveCurrent(); return (C_Gate_DataModel_Batch*)(m_clChildren.GetCurrent()); }	// Remove current batch, and return ptr on new current batch
	void					ClearBatches() { m_clChildren.RemoveAll(); }									// Clear list of batches

	// For debug
	void					Dump(QTextStream & hDumpFile);							// Dump all data structure

// PUBLIC DATA
public:
	QString					m_strLotID;			// Lot ID
	QString					m_strSublotID;		// Sublot ID
	QString					m_strWaferID;		// Wafer ID
	QDateTime				m_clFromDate;		// Smallest Date/time found in this sublot
	QDateTime				m_clToDate;			// Biggest Date/time found in this sublot

// PRIVATE METHODS
private:

// PRIVATE DATA
private:
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: C_Gate_DataModel_Lot
// Holds test results for a specific lot
/////////////////////////////////////////////////////////////////////////////////////////
class C_Gate_DataModel_Lot : public C_Gate_DataModel_Object
{
// CONSTRUCTOR/DESTRUCTOR
public:
	C_Gate_DataModel_Lot(C_Gate_DataModel_TestingStage *pParent, C_Gate_DataModel_Progress *pProgress = NULL);
	C_Gate_DataModel_Lot(C_Gate_DataModel_TestingStage *pParent, const QString &strLotID, C_Gate_DataModel_Progress *pProgress = NULL);
	~C_Gate_DataModel_Lot();

// OPERATORS
public:
	bool operator==(const C_Gate_DataModel_Lot &s);							// comparison operator
	bool operator!=(const C_Gate_DataModel_Lot &s) { return !(*this == s); }	// comparison operator

// PUBLIC METHODS
public:
	C_Gate_DataModel_TestingStage*	GetParent(){ return (C_Gate_DataModel_TestingStage*)m_pParent; }				// Return ptr on parent object
	void							ClearData();															// Clear object data (delete allocated ressources)
	void							Cleanup();																// Cleanup lot (remove empty sublots..)
	int								NbSublots() { return m_clChildren.count(); }
	C_Gate_DataModel_Sublot*		GetSublot(const Gate_LotDef & LotDefinition);								// Get sublot to be used to add data to be received (create it if no corresponding sublot already in the list)
	void							UpdateFromToDates(long lStartTime, long lFinishTime);					// Update Lot's From and To dates
	void							AddSublot(C_Gate_DataModel_Sublot* pSublot) { m_clChildren.append(pSublot); }	// Add a sublot to the sublot list of this lot
	C_Gate_DataModel_Sublot*		GetFirstSublot() { return (C_Gate_DataModel_Sublot*)(m_clChildren.GetFirst()); }	// Get ptr on first sublot of this lot
	C_Gate_DataModel_Sublot*		GetNextSublot() { return (C_Gate_DataModel_Sublot*)(m_clChildren.GetNext()); }	// Get ptr on next sublot of this lot
	C_Gate_DataModel_Sublot*		RemoveCurrentSublot() { m_clChildren.RemoveCurrent(); return (C_Gate_DataModel_Sublot*)(m_clChildren.GetCurrent()); }	// Remove current sublot, and return ptr on new current sublot
	void							ClearSublots() { m_clChildren.RemoveAll(); }									// Clear list of sublots

	// For debug
	void							Dump(QTextStream & hDumpFile);							// Dump all data structure

// PUBLIC DATA
public:
	QString							m_strLotID;			// Lot ID
	unsigned int					m_uiFlags;			// Lot flags
	QDateTime						m_clFromDate;		// Smallest Date/time found in this lot
	QDateTime						m_clToDate;			// Biggest Date/time found in this lot

// PRIVATE METHODS
private:

// PRIVATE DATA
private:
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: C_Gate_DataModel_TestingStage
// Holds test results for a specific testing stage
/////////////////////////////////////////////////////////////////////////////////////////
class C_Gate_DataModel_TestingStage : public C_Gate_DataModel_Object
{
// ERROR MAP
public:
	GDECLARE_ERROR_MAP(C_Gate_DataModel_TestingStage)
	{
		eUnknownDataOrigin,			// Dataset from an undefined origin
		eInvalidParameterSet		// Invalid Parameter set (not al Paremeters defined)
	}
	GDECLARE_END_ERROR_MAP(C_Gate_DataModel_TestingStage)

// CONSTRUCTOR/DESTRUCTOR
public:
	C_Gate_DataModel_TestingStage(C_Gate_DataModel *pParent, C_Gate_DataModel_Progress *pProgress = NULL);
	~C_Gate_DataModel_TestingStage();

// OPERATORS
public:
	bool operator==(const C_Gate_DataModel_TestingStage &s);								// comparison operator
	bool operator!=(const C_Gate_DataModel_TestingStage &s) { return !(*this == s); }	// comparison operator

// PUBLIC METHODS
public:
	C_Gate_DataModel*			GetParent(){ return (C_Gate_DataModel*)m_pParent; }							// Return ptr on parent object
	void						ClearData();															// Clear object data (delete allocated ressources)
	void						Cleanup();																// Cleanup testing stage (remove empty lots..)
	int							NbLots() { return m_clChildren.count(); }
	bool						SetStageID(const QString & strDataOrigin);								// Set testing stage ID
	void						UpdateFromToDates(long lStartTime, long lFinishTime);					// Update Testing stage's From and To dates
	C_Gate_DataModel_Lot*		GetLot(const Gate_LotDef & LotDefinition);								// Get lot to be used to add data to be received (create it if no corresponding lot already in the list)
	C_Gate_DataModel_ParameterSet*	GetParameterSet(C_Gate_DataModel_ParameterSet* pNewParameterSet, bool* pbAlreadyInList);	// Get Parameter set to be used to add data to be received (create it if no corresponding Parameter set already in the list)
	void						AddLot(C_Gate_DataModel_Lot* pLot) { m_clChildren.append(pLot); }				// Add a lot to the lot list of this testing stage
	C_Gate_DataModel_Lot*		GetFirstLot() { return (C_Gate_DataModel_Lot*)(m_clChildren.GetFirst()); }		// Get ptr on first lot of this testing stage
	C_Gate_DataModel_Lot*		GetNextLot() { return (C_Gate_DataModel_Lot*)(m_clChildren.GetNext()); }			// Get ptr on next lot of this testing stage
	C_Gate_DataModel_Lot*		RemoveCurrentLot() { m_clChildren.RemoveCurrent(); return (C_Gate_DataModel_Lot*)(m_clChildren.GetCurrent()); }	// Remove current lot, and return ptr on new current lot
	void						ClearLots() { m_clChildren.RemoveAll(); }									// Clear list of lots
	bool						IsETest() { return (m_uiStageID == YIELD123_TESTINGSTAGE_ID_ETEST); }
	bool						IsWTest() { return (m_uiStageID == YIELD123_TESTINGSTAGE_ID_WTEST); }
	bool						IsFTest() { return (m_uiStageID == YIELD123_TESTINGSTAGE_ID_FTEST); }

	// For debug
	void						Dump(QTextStream & hDumpFile);						// Dump all data structure

// PUBLIC DATA
public:
	unsigned int						m_uiStageID;		// Testing stage ID
	QString								m_strStageName;		// Testing stage name
	QDateTime							m_clFromDate;		// Smallest Date/time found in this testing stage
	QDateTime							m_clToDate;			// Biggest Date/time found in this testing stage
	C_Gate_DataModel_ParameterSetList	m_pParameterSets;	// List of Parameter sets used in this testing stage

// PRIVATE METHODS
private:

// PRIVATE DATA
private:
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: C_Gate_DataModel
// Holds data to be analyzed
/////////////////////////////////////////////////////////////////////////////////////////
class C_Gate_DataModel : public C_Gate_DataModel_Object
{
// ERROR MAP
public:
	GDECLARE_ERROR_MAP(C_Gate_DataModel)
	{
		eMultipleProducts,					// Data from several Products
		eTestingStage						// Error occured in testing stage layer (just use error message from testing stage object)
	}
	GDECLARE_END_ERROR_MAP(C_Gate_DataModel)

// CONSTRUCTOR/DESTRUCTOR
public:
	C_Gate_DataModel(C_Gate_DataModel_Progress *pProgress = NULL);
	~C_Gate_DataModel();

// PUBLIC METHODS
public:
	void							ClearData();											// Clear object data (delete allocated ressources)
	unsigned int					NbTestingStages() { return m_clChildren.count(); }		// Return nb of testing stages
	C_Gate_DataModel_TestingStage*	GetTestingStage(const Gate_LotDef & LotDefinition);		// Return testing stage corresponding to a product ID and a specified data origin (create it if none found in the list)
	void							UpdateFromToDates(long lStartTime, long lFinishTime);	// Update Dataset's From and To dates
	void							Cleanup();												// Cleanup dataset (remove empty testing stages...)
	void							AddTestingStage(C_Gate_DataModel_TestingStage* pTestingStage) { m_clChildren.append(pTestingStage); }	// Add a testing stage to the testing stages list of this dataset
	C_Gate_DataModel_TestingStage*	GetFirstTestingStage() { return (C_Gate_DataModel_TestingStage*)(m_clChildren.GetFirst()); }				// Get ptr on first testing stage of this dataset
	C_Gate_DataModel_TestingStage*	GetNextTestingStage() { return (C_Gate_DataModel_TestingStage*)(m_clChildren.GetNext()); }				// Get ptr on next testing stage of this dataset
	C_Gate_DataModel_TestingStage*	RemoveCurrentTestingStage() { m_clChildren.RemoveCurrent(); return (C_Gate_DataModel_TestingStage*)(m_clChildren.GetCurrent()); }	// Remove current testing stage, and return ptr on new current testing stage
	void							ClearTestingStages() { m_clChildren.RemoveAll(); }													// Clear list of testing stages

	// For debug
	void							Dump(QTextStream & hDumpFile);									// Dump all data structure

// PUBLIC DATA
public:
	QString							m_strProductID;			// Product ID
	QDateTime						m_clFromDate;			// Smallest date/time found in this set of data
	QDateTime						m_clToDate;				// Highest date/time found in this set of data

// PRIVATE METHODS
private:

// PRIVATE DATA
private:
};

#endif // #ifndef GEX_PLUGIN_YIELD123_DATA_H
