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
// Implementation of classes to store and handle Yield-123 data.
/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
// INCLUDES
/////////////////////////////////////////////////////////////////////////////////////////
// Standard includes
#include <stdlib.h>
#include <time.h>
#include <math.h>


// QT includes
#include <QRegExp>
#include <QFile>
#include <QTextStream>

// Galaxy libraries includes

// Local includes
#include "gate_data_stats.h"
#include "gate_data_model.h"
#include "gate_data_constants.h"
#include "gate_event_constants.h"


// Static member initialization
C_Gate_DataModel_Object* C_Gate_DataModel_Object::m_pRootForFirstNextSearch = NULL;

int	g_DataModel_colorsPass[] = {0x00CC00, 0x00FF00, 0x009900, 0x006600, 0x336600, 0x33FF00, 0x339900, 0x33CC00, 0x33FF66 , 0x33CC66};
int	g_DataModel_colorsFails[] = {0xFF0000, 0xFFFF00, 0xFF00FF, 0x8484FF, 0x008484, 0xFFC6C6, 0xC6FFC6, 0x848400, 0xFF9900, 0xFF99FF};


/////////////////////////////////////////////////////////////////////////////////////////
// GLOBAL FUNCTIONS
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// Description : Returns a formatted string that can be used as a field in a csv file
//               (mainly replacing the csv separator with another character)
//
// Argument(s) :
//
//	QString & strString
//		String to be formatted
//
// Return type : The formatted string
/////////////////////////////////////////////////////////////////////////////////////////
QString GetCsvString(const QString & strString)
{
	QString strFormattedString = strString;

	if(CSV_SEPARATOR == ',')
		strFormattedString.replace(CSV_SEPARATOR, ';');
	else if(CSV_SEPARATOR == ';')
		strFormattedString.replace(CSV_SEPARATOR, ',');
	if(strFormattedString.isEmpty())
		strFormattedString = "-";

	return strFormattedString;
}

///////////////////////////////////////////////////////////
// C_Gate_DataModel_Progress: constructor
///////////////////////////////////////////////////////////
C_Gate_DataModel_Progress::C_Gate_DataModel_Progress(QString strText, int nTotalSteps, QApplication* qApplication) :
					   QProgressDialog(strText,0, 0, nTotalSteps)
{
	setWindowTitle("Yield-123");
	m_qApplication = qApplication;
	m_nCurrentStep = 0;
	setValue(0);
	show();
	if(m_qApplication != NULL)
		m_qApplication->processEvents();
}

///////////////////////////////////////////////////////////
// C_Gate_DataModel_Progress: Init
///////////////////////////////////////////////////////////
void C_Gate_DataModel_Progress::Init(QString strText, int nTotalSteps)
{
	setLabelText(strText);
	setMaximum(nTotalSteps);

	m_nCurrentStep = 0;
	setValue(0);
	show();
	if(m_qApplication != NULL)
		m_qApplication->processEvents();
}

///////////////////////////////////////////////////////////
// C_Gate_DataModel_Progress: Increment
///////////////////////////////////////////////////////////
void C_Gate_DataModel_Progress::Increment(void)
{
	m_nCurrentStep++;
	if(m_nCurrentStep > maximum())
		m_nCurrentStep--;
	setValue(m_nCurrentStep);
	show();
	if(m_qApplication != NULL)
		m_qApplication->processEvents();
}


///////////////////////////////////////////////////////////
// C_Gate_DataModel_Progress: Increment
///////////////////////////////////////////////////////////
void C_Gate_DataModel_Progress::ProcessEvents(void)
{
	if(m_qApplication != NULL)
		m_qApplication->processEvents();
}


/////////////////////////////////////////////////////////////////////////////////////////
// Description : Get ptr on first available object of specified type
//
// Argument(s) :
//
// Return type : Ptr on first object found, NULL if no object found
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_Object* C_Gate_DataModel_Object::GetFirstObject(ObjectType eObjectType)
{
	// Set root object for first/next object searches
	m_pRootForFirstNextSearch = this;

	// Call private function
	return GetFirstObject_Private(eObjectType);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Get ptr on first available object of specified type
//
// Argument(s) :
//
// Return type : Ptr on first object found, NULL if no object found
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_Object* C_Gate_DataModel_Object::GetFirstObject_Private(ObjectType eObjectType)
{
	C_Gate_DataModel_Object	*pChild = m_clChildren.GetFirst();
	C_Gate_DataModel_Object	*pObject;

	// While child available, go through tree
	while(pChild)
	{
		// If the child is of wanted type, we found our object, return ptr
		if(pChild->IsObjectType(eObjectType))
			return pChild;

		// If child not of wanted type, get first object of wanted type from this child
		pObject = pChild->GetFirstObject_Private(eObjectType);
		if(pObject != NULL)
			return pObject;

		// Move to next child
		pChild = m_clChildren.GetNext();
	}

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Get ptr on next available object of same type
//
// Argument(s) :
//
// Return type : Ptr on next object found, NULL if no object found
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_Object* C_Gate_DataModel_Object::GetNextObject()
{
	// If valid parent, get next object of same type as current object from parent
	if(m_pParent == NULL)
		return NULL;

	return m_pParent->GetNextObject_Private(m_eObjectType);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Get ptr on next available object of specified type
//
// Argument(s) :
//
// Return type : Ptr on next object found, NULL if no object found
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_Object* C_Gate_DataModel_Object::GetNextObject_Private(ObjectType eObjectType)
{
	C_Gate_DataModel_Object	*pChild = m_clChildren.GetNext();
	C_Gate_DataModel_Object	*pObject;

	// While child available, go through tree
	while(pChild)
	{
		// If the child is of wanted type, we found our object, return ptr
		if(pChild->IsObjectType(eObjectType))
			return pChild;

		// If child not of wanted type, get first object of wanted type from this child
		pObject = pChild->GetFirstObject_Private(eObjectType);
		if(pObject != NULL)
			return pObject;

		// Move to next child
		pChild = m_clChildren.GetNext();
	}

	// No more child under this node.

	// If we reached the root object for first/next object searches, search is over.
	if(this == m_pRootForFirstNextSearch)
		return NULL;

	// If valid parent, get next object of wanted type from parent
	if(m_pParent == NULL)
		return NULL;

	return m_pParent->GetNextObject_Private(eObjectType);
}

/////////////////////////////////////////////////////////////////////////////////////////
// BINNING OBJECT
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// Description : Constructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_Binning::C_Gate_DataModel_Binning(unsigned int uiBinNb)
{
	m_uiBinNb = uiBinNb;
	m_bBinCat = 'P';
	m_uiBinCount = 0;
	m_uiBinCount_Retest = 0;
	m_uiBinColor = g_DataModel_colorsPass[0];
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Assignment operator
//
// Argument(s) :
//
//	const C_Gate_DataModel_Binning& source
//		reference to use for the assignement
//
// Return type : ptr to this object
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_Binning& C_Gate_DataModel_Binning::operator=(const C_Gate_DataModel_Binning& source)
{
	m_uiBinNb = source.m_uiBinNb;
	m_strBinName = source.m_strBinName;
	m_bBinCat = source.m_bBinCat;
	m_uiBinCount = source.m_uiBinCount;
	m_uiBinCount_Retest = source.m_uiBinCount_Retest;
	m_uiBinColor = source.m_uiBinColor;

	return *this;
}

/////////////////////////////////////////////////////////////////////////////////////////
// BINNINGLIST OBJECT
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// Description : Constructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_BinningList::C_Gate_DataModel_BinningList(): QList<C_Gate_DataModel_Binning *>()
{
	m_bAutoDelete = TRUE;

	m_iCurrentPos = -1;
	// Set sorting parameters
	m_bSortAscending = TRUE;
	m_eSortSelector = eSortOnBinNb;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Clears all items of ptr list
//
// Argument(s) :
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_BinningList::clearItems()
{
	if(m_bAutoDelete)
	{
		while(!isEmpty())
			delete takeFirst();
	}

	clear();
	m_iCurrentPos = -1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Compares 2 items of the binning list
//
// Argument(s) :
//
//	C_Gate_DataModel_Binning * Item1
//		First item
//
//	C_Gate_DataModel_Binning * Item2
//		Second item
//
// Return type : 0 if item1==item2, >0 if item1>item2, <0 if item1<item2
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_BinningList::lessThanSortOnBinNb(const C_Gate_DataModel_Binning * pBinning1,const C_Gate_DataModel_Binning * pBinning2)
{
	return pBinning1->m_uiBinNb < pBinning2->m_uiBinNb;
}
bool C_Gate_DataModel_BinningList::moreThanSortOnBinNb(const C_Gate_DataModel_Binning * pBinning1,const C_Gate_DataModel_Binning * pBinning2)
{
	return !lessThanSortOnBinNb(pBinning1,pBinning2);
}

bool C_Gate_DataModel_BinningList::lessThanSortOnBinCount(const C_Gate_DataModel_Binning * pBinning1,const C_Gate_DataModel_Binning * pBinning2)
{
		if(pBinning1->m_uiBinCount < pBinning2->m_uiBinCount)
			return true;

		// If same Bin count, sort by Bin nb
		return pBinning1->m_uiBinNb < pBinning2->m_uiBinNb;
}
bool C_Gate_DataModel_BinningList::moreThanSortOnBinCount(const C_Gate_DataModel_Binning * pBinning1,const C_Gate_DataModel_Binning * pBinning2)
{
	return !lessThanSortOnBinCount(pBinning1,pBinning2);
}
/////////////////////////////////////////////////////////////////////////////////////////
// Description : Sort list
//
// Argument(s) :
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_BinningList::Sort(SortOn eSortSelector, bool bAscending)
{
	// Check if already Sorted
	if((m_eSortSelector == eSortSelector) && (m_bSortAscending == bAscending))
		return;

	// Set sort parameters
	m_eSortSelector = eSortSelector;
	m_bSortAscending = bAscending;

	// Now sort
	switch (m_eSortSelector)
	{
	case eSortOnBinNb:
		if(m_bSortAscending)
			qSort(begin() ,end(), lessThanSortOnBinNb );
		else
			qSort(begin() ,end(), moreThanSortOnBinNb );
		break;
	case eSortOnBinCount:
		if(m_bSortAscending)
			qSort(begin() ,end(), lessThanSortOnBinCount );
		else
			qSort(begin() ,end(), moreThanSortOnBinCount );
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Find an element in the list
//
// Argument(s) :
//
// Return type : index of item in the list
/////////////////////////////////////////////////////////////////////////////////////////
int C_Gate_DataModel_BinningList::Find(unsigned int uiBinNb)
{
	// Find the item
	C_Gate_DataModel_Binning *pBin = NULL;
	Iterator it;
	for(it=begin(); it!=end(); it++)
	{
		if((*it)->m_uiBinNb == uiBinNb)
		{
			pBin = *it;
			break;
		}
	}
	if(pBin == NULL)
		return -1;

	return indexOf((C_Gate_DataModel_Binning *)pBin);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Adds the binning to the list if it doesn't contain this bin nb
//
// Argument(s) :
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_BinningList::InitBinning(unsigned int uiBinNb, QString strBinName, BYTE bBinCat)
{
	// Check if bin nb found in the list
	C_Gate_DataModel_Binning *pBinning;

	// for bin color
	int nPassPos = 0;
	int nFailPos = 0;

	for(pBinning = GetFirst(); pBinning; pBinning = GetNext())
	{
		if(pBinning->m_uiBinNb == uiBinNb)
			break;
		if(pBinning->m_bBinCat == 'P')
			nPassPos++;
		else
			nFailPos++;
	}

	if(nPassPos > 9)
		nPassPos = 9;

	if(nFailPos > 9)
		nFailPos = 9;

	// Check if binning found, otherwise, create a new one
	if(pBinning == NULL)
	{
		pBinning = new C_Gate_DataModel_Binning(uiBinNb);
		pBinning->m_strBinName = strBinName;
		pBinning->m_bBinCat = bBinCat;
		if(bBinCat == 'P')
			pBinning->m_uiBinColor = g_DataModel_colorsPass[nPassPos];
		else
			pBinning->m_uiBinColor = g_DataModel_colorsFails[nFailPos];
		append(pBinning);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Adds the binning to the list if it doesn't contain this bin nb, increments
//				 the corresponding element else.
//
// Argument(s) :
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_BinningList::AddBinning(unsigned int uiBinNb, bool bBinPass, bool bRetest /*= FALSE*/)
{
	// Check if bin nb found in the list
	C_Gate_DataModel_Binning *pBinning;

	// for bin color
	int nPassPos = 0;
	int nFailPos = 0;

	for(pBinning = GetFirst(); pBinning; pBinning = GetNext())
	{
		if(pBinning->m_uiBinNb == uiBinNb)
			break;
		if(pBinning->m_bBinCat == 'P')
			nPassPos++;
		else
			nFailPos++;
	}

	if(nPassPos > 9)
		nPassPos = 9;

	if(nFailPos > 9)
		nFailPos = 9;

	// Check if binning found, otherwise, create a new one
	if(pBinning == NULL)
	{
		pBinning = new C_Gate_DataModel_Binning(uiBinNb);

		if(bBinPass)
		{
			pBinning->m_bBinCat = 'P';
			pBinning->m_uiBinColor = g_DataModel_colorsPass[nPassPos];
		}
		else
		{
			pBinning->m_bBinCat = 'F';
			pBinning->m_uiBinColor = g_DataModel_colorsFails[nFailPos];
		}
		append(pBinning);
	}

	// Increment count
	(pBinning->m_uiBinCount)++;
	if(bRetest == TRUE)
		(pBinning->m_uiBinCount_Retest)++;
}

/////////////////////////////////////////////////////////////////////////////////////////
// TESTRESULT OBJECT
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// Description : Constructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_TestResult::C_Gate_DataModel_TestResult()
{
	m_fResult			= GEX_PLUGIN_INVALID_VALUE_FLOAT;
	m_uiFlags			= 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Assignment operator
//
// Argument(s) :
//
//	const C_Gate_DataModel_TestResult& source
//		reference to use for the assignement
//
// Return type : ptr to this object
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_TestResult& C_Gate_DataModel_TestResult::operator=(const C_Gate_DataModel_TestResult& source)
{
	m_fResult			= source.m_fResult;
	m_uiFlags			= source.m_uiFlags;

	return *this;
}

/////////////////////////////////////////////////////////////////////////////////////////
// TESTRESULTITEM OBJECT
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// Description : Constructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_TestResult_Item::C_Gate_DataModel_TestResult_Item(C_Gate_DataModel_TestResult *pTestResult, unsigned int uiRunIndex, unsigned int uiParameterIndex)
{
	m_pTestResult = pTestResult;
	m_uiRunIndex = uiRunIndex;
	m_uiParameterIndex = uiParameterIndex;
}

/////////////////////////////////////////////////////////////////////////////////////////
// TESTRESULTVECTOR OBJECT
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// Description : Constructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_TestResultVector::C_Gate_DataModel_TestResultVector(): QVector<C_Gate_DataModel_TestResult_Item *>()
{
	// Set autodelete flag to TRUE: items used for sorting should be auto-deleted, test results themselves
	// won't be deleted, because the item used for sorting just has a ptr on the test result structure.
	// Test result items are deleted with delete [] when site object is destroyed.
	m_bAutoDelete = TRUE;

	// Set sorting parameters
	m_bSortAscending = TRUE;
	m_eSortSelector = eNotSorted;//eSortOnResult;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Compares 2 items of the stat vector
//
// Argument(s) :
//
//	C_Gate_DataModel_TestResult_Item * Item1
//		First item
//
//	C_Gate_DataModel_TestResult_Item * Item2
//		Second item
//
// Return type : 0 if item1==item2, >0 if item1>item2, <0 if item1<item2
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_TestResultVector::lessThanSortOnRunIndex(const C_Gate_DataModel_TestResult_Item * pTestResultItem1, const C_Gate_DataModel_TestResult_Item * pTestResultItem2)
{
	return pTestResultItem1->m_uiRunIndex < pTestResultItem2->m_uiRunIndex;
}
bool C_Gate_DataModel_TestResultVector::moreThanSortOnRunIndex(const C_Gate_DataModel_TestResult_Item * pTestResultItem1, const C_Gate_DataModel_TestResult_Item * pTestResultItem2)
{
	return !lessThanSortOnRunIndex(pTestResultItem1,pTestResultItem2);
}
bool C_Gate_DataModel_TestResultVector::lessThanSortOnParameterIndex(const C_Gate_DataModel_TestResult_Item * pTestResultItem1, const C_Gate_DataModel_TestResult_Item * pTestResultItem2)
{
	return pTestResultItem1->m_uiParameterIndex < pTestResultItem2->m_uiParameterIndex;
}
bool C_Gate_DataModel_TestResultVector::moreThanSortOnParameterIndex(const C_Gate_DataModel_TestResult_Item * pTestResultItem1, const C_Gate_DataModel_TestResult_Item * pTestResultItem2)
{
	return !lessThanSortOnParameterIndex(pTestResultItem1,pTestResultItem2);
}
bool C_Gate_DataModel_TestResultVector::lessThanSortOnResult(const C_Gate_DataModel_TestResult_Item * pTestResultItem1, const C_Gate_DataModel_TestResult_Item * pTestResultItem2)
{
	return pTestResultItem1->m_pTestResult->m_fResult < pTestResultItem2->m_pTestResult->m_fResult;
}
bool C_Gate_DataModel_TestResultVector::moreThanSortOnResult(const C_Gate_DataModel_TestResult_Item * pTestResultItem1, const C_Gate_DataModel_TestResult_Item * pTestResultItem2)
{
	return !lessThanSortOnResult(pTestResultItem1,pTestResultItem2);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Resize vector
//
// Argument(s) :
//
// Return type : TRUE if resize successful, FALSE else
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_TestResultVector::Resize(unsigned int uiSize)
{
	C_Gate_DataModel_TestResult_Item *pItem;
	// Resize vector
	while (uiSize > (unsigned int) size())
		append(0);

	while (uiSize < (unsigned int) size())
	{
		pItem = last();
		remove(size() - 1);
		if(m_bAutoDelete)
			delete pItem;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Fill a test result vector with test results from 1 parameter
//
// Argument(s) :
//
//	C_Gate_DataModel_Site *pSite
//		<description>
//
//	unsigned int uiParameterIndex
//		<description>
//
//	ResultFilter eResultFilter /*= eResult_Any*/
//		<description>
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_TestResultVector::FillFromParameter(C_Gate_DataModel_Site *pSite, unsigned int uiParameterIndex, ResultFilter eResultFilter /*= eResult_Any*/)
{
	unsigned int			uiRunIndex;
	long					lIndex;
	C_Gate_DataModel_TestResult	*pTestResult;

	// First clear the vector
	ClearValues();

	// Size of vector is nb of samples, including outliers, even if vector will be filled only with non-outlier data
	//Resize(pSite->m_pParameterStats_AllData[uiParameterIndex].m_uiExecs);

	// Insert test results
	for(uiRunIndex = 0; uiRunIndex < pSite->m_uiNbRuns; uiRunIndex++)
	{
		lIndex = uiParameterIndex*pSite->m_uiNbRuns+uiRunIndex;
		pTestResult = pSite->m_pTestResults + lIndex;
		if(pTestResult->m_uiFlags & YIELD123_TESTRESULT_EXECUTED)
		{
			switch(eResultFilter)
			{
				case eResult_NotOutlier:
					if(!(pTestResult->m_uiFlags & (YIELD123_TESTRESULT_OUTLIER_H | YIELD123_TESTRESULT_OUTLIER_L)))
						InsertValue(new C_Gate_DataModel_TestResult_Item(pTestResult, uiRunIndex, uiParameterIndex));
					break;

				case eResult_Outlier:
					if(pTestResult->m_uiFlags & (YIELD123_TESTRESULT_OUTLIER_H | YIELD123_TESTRESULT_OUTLIER_L))
						InsertValue(new C_Gate_DataModel_TestResult_Item(pTestResult, uiRunIndex, uiParameterIndex));
					break;

				case eResult_LowOutlier:
					if(pTestResult->m_uiFlags & YIELD123_TESTRESULT_OUTLIER_L)
						InsertValue(new C_Gate_DataModel_TestResult_Item(pTestResult, uiRunIndex, uiParameterIndex));
					break;

				case eResult_HighOutlier:
					if(pTestResult->m_uiFlags & YIELD123_TESTRESULT_OUTLIER_H)
						InsertValue(new C_Gate_DataModel_TestResult_Item(pTestResult, uiRunIndex, uiParameterIndex));
					break;

				case eResult_Pass:
					if(!(pTestResult->m_uiFlags & YIELD123_TESTRESULT_FINALSTATUS_FAILED))
						InsertValue(new C_Gate_DataModel_TestResult_Item(pTestResult, uiRunIndex, uiParameterIndex));
					break;

				case eResult_Fail:
					if(pTestResult->m_uiFlags & YIELD123_TESTRESULT_FINALSTATUS_FAILED)
						InsertValue(new C_Gate_DataModel_TestResult_Item(pTestResult, uiRunIndex, uiParameterIndex));
					break;

				case eResult_LowFail:
					if((pTestResult->m_uiFlags & YIELD123_TESTRESULT_FINALSTATUS_FAILED) && (pTestResult->m_uiFlags & YIELD123_TESTRESULT_VALUE_FAILED_L))
						InsertValue(new C_Gate_DataModel_TestResult_Item(pTestResult, uiRunIndex, uiParameterIndex));
					break;

				case eResult_HighFail:
					if((pTestResult->m_uiFlags & YIELD123_TESTRESULT_FINALSTATUS_FAILED) && (pTestResult->m_uiFlags & YIELD123_TESTRESULT_VALUE_FAILED_H))
						InsertValue(new C_Gate_DataModel_TestResult_Item(pTestResult, uiRunIndex, uiParameterIndex));
					break;

				case eResult_Any:
				default:
					InsertValue(new C_Gate_DataModel_TestResult_Item(pTestResult, uiRunIndex, uiParameterIndex));
					break;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Fill a test result vector with test results from 1 run
//
// Argument(s) :
//
//	C_Gate_DataModel_Site *pSite
//		<description>
//
//	unsigned int uiRunIndex
//		<description>
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_TestResultVector::FillFromRun(C_Gate_DataModel_Site *pSite, unsigned int uiRunIndex)
{
	unsigned int			uiParameterIndex;
	long					lIndex;
	C_Gate_DataModel_TestResult	*pTestResult;

	// First clear the vector
	ClearValues();

	// Size of vector is nb of samples, including outliers, even if vector will be filled only with non-outlier data
	// Insert test results
	for(uiParameterIndex = 0; uiParameterIndex < pSite->m_uiNbParameters; uiParameterIndex++)
	{
		lIndex = uiParameterIndex*pSite->m_uiNbRuns+uiRunIndex;
		pTestResult = pSite->m_pTestResults + lIndex;
		if(pTestResult->m_uiFlags & YIELD123_TESTRESULT_EXECUTED)
		{
			InsertValue(new C_Gate_DataModel_TestResult_Item(pTestResult, uiRunIndex, uiParameterIndex));
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Sort vector
//
// Argument(s) :
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_TestResultVector::Sort(SortOn eSortSelector, bool bAscending)
{
	// Check if already Sorted
	if((m_eSortSelector == eSortSelector) && (m_bSortAscending == bAscending))
		return;

	// Set sort parameters
	m_eSortSelector = eSortSelector;
	m_bSortAscending = bAscending;

	// Now sort
	switch(m_eSortSelector)
	{
	case eSortOnRunIndex:
		if(m_bSortAscending)
			qSort(begin(),end(),lessThanSortOnRunIndex);
		else
			qSort(begin(),end(),moreThanSortOnRunIndex);
		break;
	case eSortOnParameterIndex:
		if(m_bSortAscending)
			qSort(begin(),end(),lessThanSortOnParameterIndex);
		else
			qSort(begin(),end(),moreThanSortOnParameterIndex);
		break;
	case eSortOnResult:
		if(m_bSortAscending)
			qSort(begin(),end(),lessThanSortOnResult);
		else
			qSort(begin(),end(),moreThanSortOnResult);
		break;

	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// HISTOCLASS OBJECT
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// Description : Constructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_HistoClass::C_Gate_DataModel_HistoClass()
{
	ClearData();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Reset data
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_HistoClass::ClearData()
{
	// Free allocated ressources

	// Reset variables
	m_fLowLimit		= GEX_PLUGIN_INVALID_VALUE_FLOAT;
	m_fHighLimit	= GEX_PLUGIN_INVALID_VALUE_FLOAT;
	m_uiSamples		= 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// PARAMETERHISTO OBJECT
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// Description : Constructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_Histogram::C_Gate_DataModel_Histogram()
{
	m_pHistoClasses = NULL;

	ClearData();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Destructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_Histogram::~C_Gate_DataModel_Histogram()
{
	ClearData();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Update histogram classes with specified value
//
// Argument(s) :
//
//	float fValue
//		Value to update in histogram classes
//
//	float fMin
//		Min of samples
//
//	float fMax
//		Max of samples
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_Histogram::UpdateClasses(float fValue, float fMin, float fMax)
{
	// Is value within range of the histogram classes?
	if((fValue < fMin) || (fValue > fMax))
		return;

#if 0
	// Is value the min of samples?
	if(fValue == fMin)
	{
		if(m_pHistoClasses[0].m_uiSamples == 0)
			m_uiNbNonEmptyClasses++;
		(m_pHistoClasses[0].m_uiSamples)++;
		return;
	}

	// Is value the max of samples?
	if(fValue == fMax)
	{
		if(m_pHistoClasses[m_uiNbClasses-1].m_uiSamples == 0)
			m_uiNbNonEmptyClasses++;
		(m_pHistoClasses[m_uiNbClasses-1].m_uiSamples)++;
		return;
	}
#endif

	// Increment class to which the specified value belongs too
	for(unsigned int uiIndex = 0; uiIndex < m_uiNbClasses; uiIndex++)
	{
		if(fValue <= m_pHistoClasses[uiIndex].m_fHighLimit)
		{
			if(m_pHistoClasses[uiIndex].m_uiSamples == 0)
				m_uiNbNonEmptyClasses++;
			(m_pHistoClasses[uiIndex].m_uiSamples)++;
			return;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Initialize histogram classes
//
// Argument(s) :
//
//	float fMinValue
//		Min value of samples
//
//	float fMaxValue
//		Max value of samples
//
//	unsigned int uiNbClasses
//		Nb of histogram classes
//
// Return type : bool
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_Histogram::Init(float fMinValue, float fMaxValue, unsigned int uiNbClasses,
									  float fLowLimit, float fHighLimit)
{
	// First clear data in case
	ClearData();

	// Make sure at least 2 classes are defined
	if(uiNbClasses < 2)
		return FALSE;


	// Compute class interval, and create classes

	// If have LowLimit or HighLimit
	// interval must correspond exactly to (High - Low)*xx
	float fMinInterval = fMinValue;
	float fMaxInterval = fMaxValue;

	if(fLowLimit != fHighLimit)
	{
		if((fLowLimit != -GEX_PLUGIN_INVALID_VALUE_FLOAT)
		&& (fLowLimit >= fMinValue))
			fMinInterval = fLowLimit;


		if((fHighLimit != GEX_PLUGIN_INVALID_VALUE_FLOAT)
		&& (fHighLimit <= fMaxValue))
			fMaxInterval = fHighLimit;
	}


	float fClassInterval = (fMaxValue-fMinValue)/(float)uiNbClasses;
	int nValue = (int)((	(fMaxInterval - fMinInterval) * (float)uiNbClasses)
								/  (fMaxValue-fMinValue));

	if(nValue != 0)
	{
		fClassInterval = (fMaxInterval - fMinInterval)
					/ (float)nValue;
	}
	else
	{
		fMinInterval = fMinValue;
		fMaxInterval = fMaxValue;
	}

	// Then we have to redifined the Min and the Max
	if(fMinInterval != fMinValue)
		fMinInterval = fMinInterval - ((float)((int)((fMinInterval - fMinValue) / fClassInterval) + 1.0) * fClassInterval);

	if(fMaxInterval != fMaxValue)
		fMaxInterval = fMaxInterval + ((float)((int)((fMaxValue - fMaxInterval) / fClassInterval) + 1.0) * fClassInterval);


	// For all HistoClass
	// uiNbClassees = 4
	// min=1 and max=5, must start to ]0-1],]1-2],]2-3],]3-4],]4-5]
	// Have to add 1 to uiNbClasses and the minIntervalle = fMin-ClassInterval

	m_uiNbClasses = uiNbClasses+1;

	m_pHistoClasses = new C_Gate_DataModel_HistoClass[m_uiNbClasses];
	if(m_pHistoClasses == NULL)
		return FALSE;



	// Init min, max for each class
	m_pHistoClasses[0].m_fLowLimit = fMinInterval - fClassInterval;
	m_pHistoClasses[0].m_fHighLimit = fMinInterval;
	for(unsigned int uiIndex = 1; uiIndex < m_uiNbClasses; uiIndex++)
	{
		m_pHistoClasses[uiIndex].m_fLowLimit = m_pHistoClasses[uiIndex-1].m_fHighLimit;
		m_pHistoClasses[uiIndex].m_fHighLimit = m_pHistoClasses[uiIndex].m_fLowLimit + fClassInterval;
	}
	m_pHistoClasses[m_uiNbClasses-1].m_fHighLimit = fMaxInterval;


	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Clear data
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_Histogram::ClearData()
{
	// Free allocated ressources
	if(m_pHistoClasses == NULL)
	{
		delete [] m_pHistoClasses;
		m_pHistoClasses = NULL;
	}

	// Reset variables
	m_uiNbClasses	= 0;
	m_uiNbNonEmptyClasses = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// PARAMETERSTAT OBJECT
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// Description : Constructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_ParameterStat::C_Gate_DataModel_ParameterStat()
{
	m_pHistogram = NULL;
	m_pHistogramSmooth = NULL;
	// Static information for sort by TestNumber and TestName
	m_pclParameter = NULL;		// Parameter definitions
	m_uiParameterIndex	= 0;

	m_nResScaleFactor = 0;

	ClearData();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Destructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_ParameterStat::~C_Gate_DataModel_ParameterStat()
{
	ClearData();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Initializes the histogram object
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_ParameterStat::InitHistogram(unsigned int uiNbClasses)
{
	if((m_fMin == GEX_PLUGIN_INVALID_VALUE_FLOAT) || (m_fMax == GEX_PLUGIN_INVALID_VALUE_FLOAT))
		return FALSE;

	unsigned int uiNbBarHistogram = YIELD123_STATS_MAXHISTOCLASSES;
	unsigned int uiNbBarHistogramSmooth = uiNbClasses;
	float fMin = m_fMin;
	float fMax = m_fMax;
	float fLL = m_pclParameter->m_fLL;
	float fHL = m_pclParameter->m_fHL;


	m_pHistogram = new C_Gate_DataModel_Histogram;
	if(m_pHistogram == NULL)
		return FALSE;

	m_pHistogramSmooth = new C_Gate_DataModel_Histogram;
	if(m_pHistogramSmooth == NULL)
		return FALSE;


	if(m_fRange == 0.0F)
	{
		// No range = only one value
		// Have to force the init to 10
		uiNbBarHistogramSmooth = 10;
		fMin -= (float)0.00005;
		fMax += (float)0.00005;

		fLL = -GEX_PLUGIN_INVALID_VALUE_FLOAT;
		fHL = GEX_PLUGIN_INVALID_VALUE_FLOAT;
		// return FALSE;
	}
	// HistogramSmooth under Min and Max
	m_pHistogramSmooth->Init(fMin, fMax, uiNbBarHistogramSmooth, fLL, fHL);

	if(m_fRange != 0.0F)
	{
		if((m_pclParameter->m_uiLimitFlags&YIELD123_PARAMETERDEF_HASLL)
		&& (m_pclParameter->m_fLL < m_fMin))
			fMin = m_pclParameter->m_fLL;

		if((m_pclParameter->m_uiLimitFlags&YIELD123_PARAMETERDEF_HASHL)
		&& (m_pclParameter->m_fHL > m_fMax))
			fMax = m_pclParameter->m_fHL;
	}
	else
	{
		// No range = only one value
		// Check if have some limits
		if(m_pclParameter->m_uiLimitFlags&YIELD123_PARAMETERDEF_HASLL)
			fLL = m_pclParameter->m_fLL;
		if(m_pclParameter->m_uiLimitFlags&YIELD123_PARAMETERDEF_HASHL)
			fHL = m_pclParameter->m_fHL;

		if((m_pclParameter->m_uiLimitFlags&YIELD123_PARAMETERDEF_HASLL)
		&& (m_pclParameter->m_fLL < m_fMin))
			fMin = m_pclParameter->m_fLL;

		if((m_pclParameter->m_uiLimitFlags&YIELD123_PARAMETERDEF_HASHL)
		&& (m_pclParameter->m_fHL > m_fMax))
			fMax = m_pclParameter->m_fHL;

		if((fLL == -GEX_PLUGIN_INVALID_VALUE_FLOAT) && (fHL = GEX_PLUGIN_INVALID_VALUE_FLOAT))
		{
			// Have to force the init to 10
			uiNbBarHistogram = 10;
			fMin -= (float)0.00005;
			fMax += (float)0.00005;
		}

		// return FALSE;
	}

	// Histogram under Min/LowLimit and Max/HighLimit
	return m_pHistogram->Init(fMin, fMax, uiNbBarHistogram, fLL, fHL);
}


/////////////////////////////////////////////////////////////////////////////////////////
// Description : Update histogram classes with specified value
//
// Argument(s) :
//
//	float fValue
//		Value to update in histogram classes
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_ParameterStat::UpdateHistogram(float fValue)
{
	if((m_pHistogram == NULL ) || (fValue == GEX_PLUGIN_INVALID_VALUE_FLOAT))
		return;

	m_pHistogram->UpdateClasses(fValue, m_fMin, m_fMax);
	m_pHistogramSmooth->UpdateClasses(fValue, m_fMin, m_fMax);
}


/////////////////////////////////////////////////////////////////////////////////////////
// Description : Update shape distribution classes with specified value
//				 if already initialized, take the best of 2 values
//
// Argument(s) :
//
//	float fValue
//		Value to update shape distribution
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_ParameterStat::UpdateShapeDistribution(C_Gate_DataModel_ParameterStat::ShapeDistribution eShapeDistribution)
{
	if(m_eShapeDistribution < eShapeDistribution)
		m_eShapeDistribution = eShapeDistribution;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Clear shape distribution classes //
// Argument(s) :
//
//	float fValue
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_ParameterStat::ClearShapeDistribution()
{
		m_eShapeDistribution = C_Gate_DataModel_ParameterStat::eShapeUnknow;
}

/////////////////////////////////////////////////////////////////////////////
// Tells type of distribution
/////////////////////////////////////////////////////////////////////////////
QString C_Gate_DataModel_ParameterStat::GetShapeDistributionName(bool bShortName)
{
	QString strShape = "Unknown";

	switch(m_eShapeDistribution)
	{
		case C_Gate_DataModel_ParameterStat::eShapeGaussian:
			if(bShortName)
				strShape = "Gaussian";
			else
				strShape = "Gaussian";
			break;

		case C_Gate_DataModel_ParameterStat::eShapeGaussianLeft:
			if(bShortName)
				strShape = "Gaussian";
			else
				strShape = "Gaussian (Left tailed)";
			break;

		case C_Gate_DataModel_ParameterStat::eShapeGaussianRight:
			if(bShortName)
				strShape = "Gaussian";
			else
				strShape = "Gaussian (Right tailed)";
			break;

		case C_Gate_DataModel_ParameterStat::eShapeLogNormalLeft:
			if(bShortName)
				strShape = "LogNormal";
			else
				strShape = "LogNormal (Left tailed)";
			break;

		case C_Gate_DataModel_ParameterStat::eShapeLogNormalRight:
			if(bShortName)
				strShape = "LogNormal";
			else
				strShape = "LogNormal (Right tailed)";
			break;

		case C_Gate_DataModel_ParameterStat::eShapeBiModal:
			if(bShortName)
				strShape = "BiModal";
			else
				strShape = "Bi-Modal (clear modes)";
			break;

		case C_Gate_DataModel_ParameterStat::eShapeMultiModal:
			if(bShortName)
				strShape = "MultiModal";
			else
				strShape = "Multi-Modal";
			break;

		case C_Gate_DataModel_ParameterStat::eShapeClampedLeft:
			if(bShortName)
				strShape = "Clamped";
			else
				strShape = "Clamped (Left)";
			break;

		case C_Gate_DataModel_ParameterStat::eShapeClampedRight:
			if(bShortName)
				strShape = "Clamped";
			else
				strShape = "Clamped (Right)";
			break;

		case C_Gate_DataModel_ParameterStat::eShapeClampedDouble:
			if(bShortName)
				strShape = "Clamped";
			else
				strShape = "Clamped (Both sides)";
			break;

		case C_Gate_DataModel_ParameterStat::eShapeCategory:
			if(bShortName)
				strShape = "Categories";
			else
				strShape = "Categories";
			break;

		case C_Gate_DataModel_ParameterStat::eShapeUnknow:
		default:
			if(bShortName)
				strShape = "Unknown";
			else
				strShape = "Unknown";
			break;
	}

	return strShape;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Clear object data (free allocated ressources)
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_ParameterStat::ClearData()
{
	// Free allocated ressources
	if(m_pHistogram != NULL)
	{
		delete m_pHistogram; m_pHistogram = NULL;
	}

	if(m_pHistogramSmooth != NULL)
	{
		delete m_pHistogramSmooth; m_pHistogramSmooth = NULL;
	}

	// Reset variables
	m_uiFlags					= 0;
	m_uiOutliers_H				= 0;
	m_uiOutliers_L				= 0;
	m_uiOutliers_H_GoodPart		= 0;
	m_uiOutliers_L_GoodPart		= 0;
	m_uiOutliers_H_LastRetest	= 0;
	m_uiOutliers_L_LastRetest	= 0;
	m_uiExecs					= 0;
	m_uiPass					= 0	;
	m_uiFail					= 0;
	m_uiFail_L					= 0;
	m_uiFail_H					= 0;
	m_lfSumX					= 0.0;
	m_lfSumX2					= 0.0;
	m_fMin						= GEX_PLUGIN_INVALID_VALUE_FLOAT;
	m_fMax						= GEX_PLUGIN_INVALID_VALUE_FLOAT;
	m_fRange					= GEX_PLUGIN_INVALID_VALUE_FLOAT;
	m_fQ1						= GEX_PLUGIN_INVALID_VALUE_FLOAT;
	m_fQ2						= GEX_PLUGIN_INVALID_VALUE_FLOAT;
	m_fQ3						= GEX_PLUGIN_INVALID_VALUE_FLOAT;
	m_fIQR						= GEX_PLUGIN_INVALID_VALUE_FLOAT;
	m_fIQS						= GEX_PLUGIN_INVALID_VALUE_FLOAT;
	m_fMean						= GEX_PLUGIN_INVALID_VALUE_FLOAT;
	m_fSigma					= GEX_PLUGIN_INVALID_VALUE_FLOAT;
	m_fRobustMean				= GEX_PLUGIN_INVALID_VALUE_FLOAT;
	m_fRobustSigma				= GEX_PLUGIN_INVALID_VALUE_FLOAT;
	m_fCpkL						= GEX_PLUGIN_INVALID_VALUE_FLOAT;
	m_fCpkH						= GEX_PLUGIN_INVALID_VALUE_FLOAT;
	m_fCpk						= GEX_PLUGIN_INVALID_VALUE_FLOAT;
	m_fCp						= GEX_PLUGIN_INVALID_VALUE_FLOAT;


	m_eShapeDistribution		= eShapeUnknow;

	m_lDifferentValues			= 0;
	m_lfSamplesSkew				= 0.0;
	m_lfSamplesKurtosis			= 0.0;

	m_uiNbPointsPerSubGroup		= 10;
	m_fXBarLCL					= GEX_PLUGIN_INVALID_VALUE_FLOAT;
	m_fXBarBar					= GEX_PLUGIN_INVALID_VALUE_FLOAT;
	m_fXBarUCL					= GEX_PLUGIN_INVALID_VALUE_FLOAT;
	m_fRLCL						= GEX_PLUGIN_INVALID_VALUE_FLOAT;
	m_fRBar						= GEX_PLUGIN_INVALID_VALUE_FLOAT;
	m_fRUCL						= GEX_PLUGIN_INVALID_VALUE_FLOAT;
}

/////////////////////////////////////////////////////////////////////////////////////////
// PARAMETERSTATVECTOR OBJECT
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// Description : Constructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_ParameterStatVector::C_Gate_DataModel_ParameterStatVector(): QVector<C_Gate_DataModel_ParameterStat*>()
{
	// Set autodelete flag to false (items are deleted with delete [] when site object is destroyed)
	m_bAutoDelete = FALSE;

	// Set sorting parameters
	m_bSortAscending = FALSE;
	m_eSortSelector = eNotSorted;//eSortOnFail;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Resize vector
//
// Argument(s) :
//
// Return type : TRUE if resize successful, FALSE else
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_ParameterStatVector::Resize(unsigned int uiSize)
{
	C_Gate_DataModel_ParameterStat *pItem;
	// Resize vector
	while (uiSize > (unsigned int) size())
		append(0);

	while (uiSize < (unsigned int) size())
	{
		pItem = last();
		remove(size() - 1);
		if(m_bAutoDelete)
			delete pItem;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Fill a parameter stat vector with parameter stats
//
// Argument(s) :
//
//	C_Gate_DataModel_Site *pSite
//		<description>
//
//	ParameterFilter eParameterFilter /*= eParameter_Any*/
//		<description>
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_ParameterStatVector::Fill(C_Gate_DataModel_Site *pSite, ParameterFilter eParameterFilter /*= eParameter_Any*/)
{
	unsigned int				uiParameterIndex;
	C_Gate_DataModel_ParameterStat	*pParameterStat;

	// First clear the vector
	ClearValues();

	// Size of vector is nb of parameters, even if vector will be filled only with subset of parameters
	//Resize(pSite->m_uiNbParameters);

	// Insert parameter stats
	for(uiParameterIndex = 0; uiParameterIndex < pSite->m_uiNbParameters; uiParameterIndex++)
	{
		pParameterStat = pSite->m_pParameterStats_AllData + uiParameterIndex;
		//if(pParameterStat->m_uiExecs > 0)
		{
			switch(eParameterFilter)
			{
				case eParameter_WithResults:
					if(pParameterStat->m_uiExecs > 0)
						InsertValue(pParameterStat);
					break;

				case eParameter_WithFails:
					if(pParameterStat->m_uiFail > 0)
						InsertValue(pParameterStat);
					break;

				case eParameter_WithOutliers:
					if((pParameterStat->m_uiOutliers_H + pParameterStat->m_uiOutliers_L) > 0)
						InsertValue(pParameterStat);
					break;

				case eParameter_WithOutliersOnGoodRuns:
					if((pParameterStat->m_uiOutliers_H_GoodPart + pParameterStat->m_uiOutliers_L_GoodPart) > 0)
						InsertValue(pParameterStat);
					break;

				case eParameter_WithOutliersOnLatestRetest:
					if((pParameterStat->m_uiOutliers_H_LastRetest + pParameterStat->m_uiOutliers_L_LastRetest) > 0)
						InsertValue(pParameterStat);
					break;

				case eParameter_Any:
				default:
					InsertValue(pParameterStat);
					break;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : SortOn field format in QString
/////////////////////////////////////////////////////////////////////////////////////////
QString C_Gate_DataModel_ParameterStatVector::SortOnToString(int eSortSelector, bool bAscending)
{
	QString strSortingMode;

	int		eSort = m_eSortSelector;
	bool	bAsc = m_bSortAscending;

	if(eSortSelector != -1)
	{
		eSort = eSortSelector;
		bAsc = bAscending;
	}

	switch(eSort)
	{
	case eSortOnParameterIndex:
		strSortingMode = "Parameter Index";
		break;
	case eSortOnOutliers:
		strSortingMode = "Outliers";
		break;
	case eSortOnOutliers_L:
		strSortingMode = "Outliers Low";
		break;
	case eSortOnOutliers_H:
		strSortingMode = "Outliers High";
		break;
	case eSortOnOutliers_GoodRun:
		strSortingMode = "Good Run";
		break;
	case eSortOnOutliers_LatestRetest:
		strSortingMode = "Latest Retest";
		break;
	case eSortOnCpk:
		strSortingMode = "Cpk";
		break;
	case eSortOnCp:
		strSortingMode = "Cp";
		break;
	case eSortOnMean:
		strSortingMode = "Mean";
		break;
	case eSortOnMin:
		strSortingMode = "Min";
		break;
	case eSortOnMax:
		strSortingMode = "Max";
		break;
	case eSortOnRange:
		strSortingMode = "Range";
		break;
	case eSortOnSigma:
		strSortingMode = "Sigma";
		break;
	case eSortOnQ1:
		strSortingMode = "Q1";
		break;
	case eSortOnQ2:
		strSortingMode = "Q2";
		break;
	case eSortOnQ3:
		strSortingMode = "Q3";
		break;
	case eSortOnExecs:
		strSortingMode = "Execs";
		break;
	case eSortOnPass:
		strSortingMode = "Pass";
		break;
	case eSortOnFail:
		strSortingMode = "Fail";
		break;
	case eSortOnFail_L:
		strSortingMode = "Fail Low";
		break;
	case eSortOnFail_H:
		strSortingMode = "Fail High";
		break;
	case eSortOnTestNumber:
		strSortingMode = "Test Number";
		break;
	case eSortOnTestName:
		strSortingMode = "Test Name";
		break;

	}

	if(!bAsc)
		strSortingMode += " Descending";

	return strSortingMode;

}


/////////////////////////////////////////////////////////////////////////////////////////
// Description : Compares 2 items of the stat vector
//
// Argument(s) :
//
//	C_Gate_DataModel_ParameterStat * Item1
//		First item
//
//	C_Gate_DataModel_ParameterStat * Item2
//		Second item
//
// Return type : 0 if item1==item2, >0 if item1>item2, <0 if item1<item2
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_ParameterStatVector::lessThanSortOnParameterIndex(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return pParameterStat1->m_uiParameterIndex < pParameterStat2->m_uiParameterIndex;
}
bool C_Gate_DataModel_ParameterStatVector::moreThanSortOnParameterIndex(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return !lessThanSortOnParameterIndex(pParameterStat1,pParameterStat2);
}
bool C_Gate_DataModel_ParameterStatVector::lessThanSortOnOutliers(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return (pParameterStat1->m_uiOutliers_H+pParameterStat1->m_uiOutliers_L) < (pParameterStat2->m_uiOutliers_H+pParameterStat2->m_uiOutliers_L);
}
bool C_Gate_DataModel_ParameterStatVector::moreThanSortOnOutliers(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return !lessThanSortOnOutliers(pParameterStat1,pParameterStat2);
}
bool C_Gate_DataModel_ParameterStatVector::lessThanSortOnOutliers_L(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return pParameterStat1->m_uiOutliers_L < pParameterStat2->m_uiOutliers_L;
}
bool C_Gate_DataModel_ParameterStatVector::moreThanSortOnOutliers_L(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return !lessThanSortOnOutliers_L(pParameterStat1,pParameterStat2);
}
bool C_Gate_DataModel_ParameterStatVector::lessThanSortOnOutliers_H(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return pParameterStat1->m_uiOutliers_H < pParameterStat2->m_uiOutliers_H;
}
bool C_Gate_DataModel_ParameterStatVector::moreThanSortOnOutliers_H(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return !lessThanSortOnOutliers_H(pParameterStat1,pParameterStat2);
}
bool C_Gate_DataModel_ParameterStatVector::lessThanSortOnOutliers_GoodRun(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return (pParameterStat1->m_uiOutliers_H_GoodPart+pParameterStat1->m_uiOutliers_L_GoodPart) < (pParameterStat2->m_uiOutliers_H_GoodPart+pParameterStat2->m_uiOutliers_L_GoodPart);
}
bool C_Gate_DataModel_ParameterStatVector::moreThanSortOnOutliers_GoodRun(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return !lessThanSortOnOutliers_GoodRun(pParameterStat1,pParameterStat2);
}
bool C_Gate_DataModel_ParameterStatVector::lessThanSortOnOutliers_LatestRetest(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return (pParameterStat1->m_uiOutliers_H_LastRetest+pParameterStat1->m_uiOutliers_L_LastRetest) < (pParameterStat2->m_uiOutliers_H_LastRetest+pParameterStat2->m_uiOutliers_L_LastRetest);
}
bool C_Gate_DataModel_ParameterStatVector::moreThanSortOnOutliers_LatestRetest(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return !lessThanSortOnOutliers_LatestRetest(pParameterStat1,pParameterStat2);
}
bool C_Gate_DataModel_ParameterStatVector::lessThanSortOnCpk(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return (pParameterStat1->m_fCpk < pParameterStat2->m_fCpk);
}
bool C_Gate_DataModel_ParameterStatVector::moreThanSortOnCpk(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return !lessThanSortOnCpk(pParameterStat1,pParameterStat2);
}
bool C_Gate_DataModel_ParameterStatVector::lessThanSortOnCp(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return (pParameterStat1->m_fCp < pParameterStat2->m_fCp);
}
bool C_Gate_DataModel_ParameterStatVector::moreThanSortOnCp(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return !lessThanSortOnCp(pParameterStat1,pParameterStat2);
}
bool C_Gate_DataModel_ParameterStatVector::lessThanSortOnMean(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return (pParameterStat1->m_fMean < pParameterStat2->m_fMean);
}
bool C_Gate_DataModel_ParameterStatVector::moreThanSortOnMean(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return !lessThanSortOnMean(pParameterStat1,pParameterStat2);
}
bool C_Gate_DataModel_ParameterStatVector::lessThanSortOnMin(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return (pParameterStat1->m_fMin < pParameterStat2->m_fMin);
}
bool C_Gate_DataModel_ParameterStatVector::moreThanSortOnMin(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return !lessThanSortOnMin(pParameterStat1,pParameterStat2);
}
bool C_Gate_DataModel_ParameterStatVector::lessThanSortOnMax(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return (pParameterStat1->m_fMax < pParameterStat2->m_fMax);
}
bool C_Gate_DataModel_ParameterStatVector::moreThanSortOnMax(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return !lessThanSortOnMax(pParameterStat1,pParameterStat2);
}
bool C_Gate_DataModel_ParameterStatVector::lessThanSortOnRange(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return (pParameterStat1->m_fRange < pParameterStat2->m_fRange);
}
bool C_Gate_DataModel_ParameterStatVector::moreThanSortOnRange(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return !lessThanSortOnRange(pParameterStat1,pParameterStat2);
}
bool C_Gate_DataModel_ParameterStatVector::lessThanSortOnSigma(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return (pParameterStat1->m_fSigma < pParameterStat2->m_fSigma);
}
bool C_Gate_DataModel_ParameterStatVector::moreThanSortOnSigma(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return !lessThanSortOnSigma(pParameterStat1,pParameterStat2);
}
bool C_Gate_DataModel_ParameterStatVector::lessThanSortOnQ1(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return (pParameterStat1->m_fQ1 < pParameterStat2->m_fQ1);
}
bool C_Gate_DataModel_ParameterStatVector::moreThanSortOnQ1(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return !lessThanSortOnQ1(pParameterStat1,pParameterStat2);
}
bool C_Gate_DataModel_ParameterStatVector::lessThanSortOnQ2(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return (pParameterStat1->m_fQ2 < pParameterStat2->m_fQ2);
}
bool C_Gate_DataModel_ParameterStatVector::moreThanSortOnQ2(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return !lessThanSortOnQ2(pParameterStat1,pParameterStat2);
}
bool C_Gate_DataModel_ParameterStatVector::lessThanSortOnQ3(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return (pParameterStat1->m_fQ3 < pParameterStat2->m_fQ3);
}
bool C_Gate_DataModel_ParameterStatVector::moreThanSortOnQ3(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return !lessThanSortOnQ3(pParameterStat1,pParameterStat2);
}
bool C_Gate_DataModel_ParameterStatVector::lessThanSortOnExecs(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return (pParameterStat1->m_uiExecs < pParameterStat2->m_uiExecs);
}
bool C_Gate_DataModel_ParameterStatVector::moreThanSortOnExecs(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return !lessThanSortOnExecs(pParameterStat1,pParameterStat2);
}
bool C_Gate_DataModel_ParameterStatVector::lessThanSortOnPass(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return (pParameterStat1->m_uiPass < pParameterStat2->m_uiPass);
}
bool C_Gate_DataModel_ParameterStatVector::moreThanSortOnPass(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return !lessThanSortOnPass(pParameterStat1,pParameterStat2);
}
bool C_Gate_DataModel_ParameterStatVector::lessThanSortOnFail(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return pParameterStat1->m_uiFail < pParameterStat2->m_uiFail;
}
bool C_Gate_DataModel_ParameterStatVector::moreThanSortOnFail(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return !lessThanSortOnFail(pParameterStat1,pParameterStat2);
}
bool C_Gate_DataModel_ParameterStatVector::lessThanSortOnFail_L(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return (pParameterStat1->m_uiFail_L < pParameterStat2->m_uiFail_L);
}
bool C_Gate_DataModel_ParameterStatVector::moreThanSortOnFail_L(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return !lessThanSortOnFail_L(pParameterStat1,pParameterStat2);
}
bool C_Gate_DataModel_ParameterStatVector::lessThanSortOnFail_H(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return (pParameterStat1->m_uiFail_H < pParameterStat2->m_uiFail_H);
}
bool C_Gate_DataModel_ParameterStatVector::moreThanSortOnFail_H(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return !lessThanSortOnFail_H(pParameterStat1,pParameterStat2);
}
bool C_Gate_DataModel_ParameterStatVector::lessThanSortOnTestNumber(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	if(pParameterStat1->m_pclParameter == NULL)
		return lessThanSortOnParameterIndex(pParameterStat1,pParameterStat2);
	return (pParameterStat1->m_pclParameter->m_uiNumber < pParameterStat2->m_pclParameter->m_uiNumber);
}
bool C_Gate_DataModel_ParameterStatVector::moreThanSortOnTestNumber(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return !lessThanSortOnTestNumber(pParameterStat1,pParameterStat2);
}
bool C_Gate_DataModel_ParameterStatVector::lessThanSortOnTestName(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	if(pParameterStat1->m_pclParameter == NULL)
		return lessThanSortOnParameterIndex(pParameterStat1,pParameterStat2);
	return (pParameterStat1->m_pclParameter->m_strName < pParameterStat2->m_pclParameter->m_strName);
}
bool C_Gate_DataModel_ParameterStatVector::moreThanSortOnTestName(const C_Gate_DataModel_ParameterStat * pParameterStat1, const C_Gate_DataModel_ParameterStat * pParameterStat2)
{
	return !lessThanSortOnTestName(pParameterStat1,pParameterStat2);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Sort vector
//
// Argument(s) :
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_ParameterStatVector::Sort(SortOn eSortSelector, bool bAscending)
{
	// Check if already Sorted
	if((m_eSortSelector == eSortSelector) && (m_bSortAscending == bAscending))
		return;

	// Set sort parameters
	m_eSortSelector = eSortSelector;
	m_bSortAscending = bAscending;

	// Now sort
	switch(m_eSortSelector)
	{
	case eSortOnParameterIndex:
		if(m_bSortAscending)
			qSort(begin(),end(),lessThanSortOnParameterIndex);
		else
			qSort(begin(),end(),moreThanSortOnParameterIndex);
		break;
	case eSortOnOutliers:
		if(m_bSortAscending)
			qSort(begin(),end(),lessThanSortOnOutliers);
		else
			qSort(begin(),end(),moreThanSortOnOutliers);
		break;
	case eSortOnOutliers_L:
		if(m_bSortAscending)
			qSort(begin(),end(),lessThanSortOnOutliers_L);
		else
			qSort(begin(),end(),moreThanSortOnOutliers_L);
		break;
	case eSortOnOutliers_H:
		if(m_bSortAscending)
			qSort(begin(),end(),lessThanSortOnOutliers_H);
		else
			qSort(begin(),end(),moreThanSortOnOutliers_H);
		break;
	case eSortOnOutliers_GoodRun:
		if(m_bSortAscending)
			qSort(begin(),end(),lessThanSortOnOutliers_GoodRun);
		else
			qSort(begin(),end(),moreThanSortOnOutliers_GoodRun);
		break;
	case eSortOnOutliers_LatestRetest:
		if(m_bSortAscending)
			qSort(begin(),end(),lessThanSortOnOutliers_LatestRetest);
		else
			qSort(begin(),end(),moreThanSortOnOutliers_LatestRetest);
		break;
	case eSortOnCpk:
		if(m_bSortAscending)
			qSort(begin(),end(),lessThanSortOnCpk);
		else
			qSort(begin(),end(),moreThanSortOnCpk);
		break;
	case eSortOnCp:
		if(m_bSortAscending)
			qSort(begin(),end(),lessThanSortOnCp);
		else
			qSort(begin(),end(),moreThanSortOnCp);
		break;
	case eSortOnMean:
		if(m_bSortAscending)
			qSort(begin(),end(),lessThanSortOnMean);
		else
			qSort(begin(),end(),moreThanSortOnMean);
		break;
	case eSortOnMin:
		if(m_bSortAscending)
			qSort(begin(),end(),lessThanSortOnMin);
		else
			qSort(begin(),end(),moreThanSortOnMin);
		break;
	case eSortOnMax:
		if(m_bSortAscending)
			qSort(begin(),end(),lessThanSortOnMax);
		else
			qSort(begin(),end(),moreThanSortOnMax);
		break;
	case eSortOnRange:
		if(m_bSortAscending)
			qSort(begin(),end(),lessThanSortOnRange);
		else
			qSort(begin(),end(),moreThanSortOnRange);
		break;
	case eSortOnSigma:
		if(m_bSortAscending)
			qSort(begin(),end(),lessThanSortOnSigma);
		else
			qSort(begin(),end(),moreThanSortOnSigma);
		break;
	case eSortOnQ1:
		if(m_bSortAscending)
			qSort(begin(),end(),lessThanSortOnQ1);
		else
			qSort(begin(),end(),moreThanSortOnQ1);
		break;
	case eSortOnQ2:
		if(m_bSortAscending)
			qSort(begin(),end(),lessThanSortOnQ2);
		else
			qSort(begin(),end(),moreThanSortOnQ2);
		break;
	case eSortOnQ3:
		if(m_bSortAscending)
			qSort(begin(),end(),lessThanSortOnQ3);
		else
			qSort(begin(),end(),moreThanSortOnQ3);
		break;
	case eSortOnExecs:
		if(m_bSortAscending)
			qSort(begin(),end(),lessThanSortOnExecs);
		else
			qSort(begin(),end(),moreThanSortOnExecs);
		break;
	case eSortOnPass:
		if(m_bSortAscending)
			qSort(begin(),end(),lessThanSortOnPass);
		else
			qSort(begin(),end(),moreThanSortOnPass);
		break;
	case eSortOnFail:
		if(m_bSortAscending)
			qSort(begin(),end(),lessThanSortOnFail);
		else
			qSort(begin(),end(),moreThanSortOnFail);
		break;
	case eSortOnFail_L:
		if(m_bSortAscending)
			qSort(begin(),end(),lessThanSortOnFail_L);
		else
			qSort(begin(),end(),moreThanSortOnFail_L);
		break;
	case eSortOnFail_H:
		if(m_bSortAscending)
			qSort(begin(),end(),lessThanSortOnFail_H);
		else
			qSort(begin(),end(),moreThanSortOnFail_H);
		break;
	case eSortOnTestNumber:
		if(m_bSortAscending)
			qSort(begin(),end(),lessThanSortOnTestNumber);
		else
			qSort(begin(),end(),moreThanSortOnTestNumber);
		break;
	case eSortOnTestName:
		if(m_bSortAscending)
			qSort(begin(),end(),lessThanSortOnTestName);
		else
			qSort(begin(),end(),moreThanSortOnTestName);
		break;

	}

}

/////////////////////////////////////////////////////////////////////////////////////////
// BINNINGVECTOR OBJECT
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// Description : Constructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_BinningVector::C_Gate_DataModel_BinningVector(): QVector<C_Gate_DataModel_Binning*>()
{
	// Set autodelete flag to false (items are deleted with delete [] when site object is destroyed)
	m_bAutoDelete = FALSE;

	// Set sorting parameters
	m_bHardBinning = FALSE;
	m_bSortAscending = FALSE;
	m_eSortSelector = eNotSorted;//eSortOnExecs;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Resize vector
//
// Argument(s) :
//
// Return type : TRUE if resize successful, FALSE else
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_BinningVector::Resize(unsigned int uiSize)
{
	C_Gate_DataModel_Binning *pItem;
	// Resize vector
	while (uiSize > (unsigned int) size())
		append(0);

	while (uiSize < (unsigned int) size())
	{
		pItem = last();
		remove(size() - 1);
		if(m_bAutoDelete)
			delete pItem;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Fill a binning vector with binning
//
// Argument(s) :
//
//	C_Gate_DataModel_Site *pSite
//		<description>
//
//	ParameterFilter eParameterFilter /*= eParameter_Any*/
//		<description>
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_BinningVector::Fill(C_Gate_DataModel_Batch *pBatch, ParameterFilter eParameterFilter,bool bHardBinning)
{
	//unsigned int				uiParameterIndex;
	C_Gate_DataModel_Binning	*pBinning;

	// First clear the vector
	ClearValues();

	// Size of vector is nb of parameters, even if vector will be filled only with subset of parameters
	//Resize(pSite->m_uiNbParameters);

	// Insert parameter stats
	C_Gate_DataModel_BinningList *pBinList;

	if(bHardBinning)
	{
		m_bHardBinning = TRUE;
		pBinList = &pBatch->m_pHardBins;
	}
	else
	{
		m_bHardBinning = FALSE;
		pBinList = &pBatch->m_pSoftBins;
	}

	for (C_Gate_DataModel_BinningList::const_iterator
		 iter  = pBinList->begin();
		 iter != pBinList->end(); ++iter) {
		pBinning = *iter;
		if(eParameterFilter == eParameter_WithResults)
		{
			if(pBinning->m_uiBinCount > 0)
				InsertValue(pBinning);
		}
		if(eParameterFilter == eParameter_Any)
		{
			InsertValue(pBinning);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Fill a binning vector with binning
//
// Argument(s) :
//
//	C_Gate_DataModel_Site *pSite
//		<description>
//
//	ParameterFilter eParameterFilter /*= eParameter_Any*/
//		<description>
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_BinningVector::Fill(C_Gate_DataModel_Site *pSite, ParameterFilter eParameterFilter, bool bHardBinning)
{
	//unsigned int				uiParameterIndex;
	C_Gate_DataModel_Binning	*pBinning;

	// First clear the vector
	ClearValues();

	// Size of vector is nb of parameters, even if vector will be filled only with subset of parameters
	//Resize(pSite->m_uiNbParameters);

	// Insert parameter stats
	C_Gate_DataModel_BinningList *pBinList;

	if(bHardBinning)
	{
		m_bHardBinning = TRUE;
		pBinList = &pSite->m_pHardBins;
	}
	else
	{
		m_bHardBinning = FALSE;
		pBinList = &pSite->m_pSoftBins;
	}

	for (C_Gate_DataModel_BinningList::const_iterator
		iter  = pBinList->begin();
		iter != pBinList->end(); ++iter) {
		pBinning = *iter;
		if(eParameterFilter == eParameter_WithResults)
		{
			if(pBinning->m_uiBinCount > 0)
				InsertValue(pBinning);
		}
		if(eParameterFilter == eParameter_Any)
		{
			InsertValue(pBinning);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Compares 2 items of the Binning vector
//
// Argument(s) :
//
//	C_Gate_DataModel_Binning * Item1
//		First item
//
//	C_Gate_DataModel_Binning * Item2
//		Second item
//
// Return type : 0 if item1==item2, >0 if item1>item2, <0 if item1<item2
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_BinningVector::lessThanSortOnExecs(const C_Gate_DataModel_Binning * pBinning1, const C_Gate_DataModel_Binning * pBinning2)
{
	return (pBinning1->m_uiBinCount < pBinning2->m_uiBinCount);
}
bool C_Gate_DataModel_BinningVector::moreThanSortOnExecs(const C_Gate_DataModel_Binning * pBinning1, const C_Gate_DataModel_Binning * pBinning2)
{
	return !lessThanSortOnExecs(pBinning1,pBinning2);
}
bool C_Gate_DataModel_BinningVector::lessThanSortOnTestNumber(const C_Gate_DataModel_Binning * pBinning1, const C_Gate_DataModel_Binning * pBinning2)
{
	return (pBinning1->m_uiBinNb < pBinning2->m_uiBinNb);
}
bool C_Gate_DataModel_BinningVector::moreThanSortOnTestNumber(const C_Gate_DataModel_Binning * pBinning1, const C_Gate_DataModel_Binning * pBinning2)
{
	return !lessThanSortOnTestNumber(pBinning1,pBinning2);
}
bool C_Gate_DataModel_BinningVector::lessThanSortOnTestName(const C_Gate_DataModel_Binning * pBinning1, const C_Gate_DataModel_Binning * pBinning2)
{
	return (pBinning1->m_strBinName < pBinning2->m_strBinName);
}
bool C_Gate_DataModel_BinningVector::moreThanSortOnTestName(const C_Gate_DataModel_Binning * pBinning1, const C_Gate_DataModel_Binning * pBinning2)
{
	return !lessThanSortOnTestName(pBinning1,pBinning2);
}
bool C_Gate_DataModel_BinningVector::lessThanSortOnPass(const C_Gate_DataModel_Binning * pBinning1, const C_Gate_DataModel_Binning * pBinning2)
{
	return !(pBinning1->m_bBinCat < pBinning2->m_bBinCat);
}
bool C_Gate_DataModel_BinningVector::moreThanSortOnPass(const C_Gate_DataModel_Binning * pBinning1, const C_Gate_DataModel_Binning * pBinning2)
{
	return !lessThanSortOnPass(pBinning1,pBinning2);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Sort vector
//
// Argument(s) :
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_BinningVector::Sort(SortOn eSortSelector, bool bAscending)
{
	// Check if already Sorted
	if((m_eSortSelector == eSortSelector) && (m_bSortAscending == bAscending))
		return;

	// Set sort parameters
	m_eSortSelector = eSortSelector;
	m_bSortAscending = bAscending;

	// Now sort
	switch(m_eSortSelector)
	{
	case eSortOnExecs:
		if(m_bSortAscending)
			qSort(begin(),end(),lessThanSortOnExecs);
		else
			qSort(begin(),end(),moreThanSortOnExecs);
		break;
	case eSortOnTestNumber:
		if(m_bSortAscending)
			qSort(begin(),end(),lessThanSortOnTestNumber);
		else
			qSort(begin(),end(),moreThanSortOnTestNumber);
		break;
	case eSortOnTestName:
		if(m_bSortAscending)
			qSort(begin(),end(),lessThanSortOnTestName);
		else
			qSort(begin(),end(),moreThanSortOnTestName);
		break;

	}

}

/////////////////////////////////////////////////////////////////////////////////////////
// PARTRESULT OBJECT
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// Description : Constructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_PartResult::C_Gate_DataModel_PartResult()
{
	m_uiRunIndex		= 0;
	m_strPartID			= "";
	m_nXLoc				= 0;
	m_nYLoc				= 0;
	m_nSoftBin			= -1;
	m_nHardBin			= -1;
	m_uiFlags			= 0;
	m_uiTestsExecuted	= 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// PARTRESULTVECTOR OBJECT
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// Description : Constructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_PartResultVector::C_Gate_DataModel_PartResultVector(): QVector<C_Gate_DataModel_PartResult*>()
{
	// Set autodelete flag to false (items are deleted with delete [] when site object is destroyed)
	m_bAutoDelete = FALSE;

	// Set sorting parameters
	m_bSortAscending = FALSE;
	m_eSortSelector = eNotSorted;//eSortOnPartID;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Resize vector
//
// Argument(s) :
//
// Return type : TRUE if resize successful, FALSE else
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_PartResultVector::Resize(unsigned int uiSize)
{
	C_Gate_DataModel_PartResult *pItem;
	// Resize vector
	while (uiSize > (unsigned int) size())
		append(0);

	while (uiSize < (unsigned int) size())
	{
		pItem = last();
		remove(size() - 1);
		if(m_bAutoDelete)
			delete pItem;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Fill a part result vector with part results
//
// Argument(s) :
//
//	C_Gate_DataModel_Site *pSite
//		<description>
//
//	RunFilter eRunFilter /*= eRun_Any*/
//		<description>
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_PartResultVector::Fill(C_Gate_DataModel_Site *pSite, RunFilter eRunFilter /*= eRun_Any*/,
											 int nXPos, int nYPos)
{
	unsigned int			uiRunIndex;
	C_Gate_DataModel_PartResult	*pPartResult;

	// First clear the vector
	ClearValues();

	// Size of vector is nb of runs, even if vector will be filled only with pass or fail data
	//Resize(pSite->m_uiNbParts);

	// Insert part results
	for(uiRunIndex = 0; uiRunIndex < pSite->m_uiNbRuns; uiRunIndex++)
	{
		pPartResult = pSite->m_pPartResults + uiRunIndex;
		// Fill only parts with the same Location
		if((nXPos != -1) && (pPartResult->m_nXLoc != nXPos))
			continue;
		if((nYPos != -1) && (pPartResult->m_nYLoc != nYPos))
			continue;

		if(pPartResult->m_uiFlags & YIELD123_PARTRESULT_EXECUTED)
		{
			switch(eRunFilter)
			{
				case eRun_Pass:
					if(!(pPartResult->m_uiFlags & YIELD123_PARTRESULT_FAIL))
						InsertValue(pPartResult);
					break;

				case eRun_Fail:
					if(pPartResult->m_uiFlags & YIELD123_PARTRESULT_FAIL)
						InsertValue(pPartResult);
					break;

				case eRun_PassWithOutliers:
					if(!(pPartResult->m_uiFlags & YIELD123_PARTRESULT_FAIL) && (pPartResult->m_uiFlags & YIELD123_PARTRESULT_HASOUTLIERS))
						InsertValue(pPartResult);
					break;

				case eRun_FailWithOutliers:
					if((pPartResult->m_uiFlags & YIELD123_PARTRESULT_FAIL) && (pPartResult->m_uiFlags & YIELD123_PARTRESULT_HASOUTLIERS))
						InsertValue(pPartResult);
					break;

				case eRun_WithOutliers:
					if(pPartResult->m_uiFlags & YIELD123_PARTRESULT_HASOUTLIERS)
						InsertValue(pPartResult);
					break;

				case eRun_LastRetestWithOutliers:
					if(!(pPartResult->m_uiFlags & YIELD123_PARTRESULT_HASBEENRETESTED) && (pPartResult->m_uiFlags & YIELD123_PARTRESULT_HASOUTLIERS))
						InsertValue(pPartResult);
					break;

				case eRun_Any:
				default:
					InsertValue(pPartResult);
					break;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Compares 2 items of the stat vector
//
// Argument(s) :
//
//	C_Gate_DataModel_PartResult * Item1
//		First item
//
//	C_Gate_DataModel_PartResult * Item2
//		Second item
//
// Return type : 0 if item1==item2, >0 if item1>item2, <0 if item1<item2
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_PartResultVector::lessThanSortOnPartID(const C_Gate_DataModel_PartResult * pPartResult1, const C_Gate_DataModel_PartResult * pPartResult2)
{
	bool bOK1;
	int iPartID1 = pPartResult1->m_strPartID.toInt(&bOK1);
	bool bOK2;
	int iPartID2 = pPartResult2->m_strPartID.toInt(&bOK2);
	if(!bOK1 || !bOK2)
		return pPartResult1->m_strPartID.compare(pPartResult2->m_strPartID, Qt::CaseInsensitive);

	return iPartID1 < iPartID2;
}
bool C_Gate_DataModel_PartResultVector::moreThanSortOnPartID(const C_Gate_DataModel_PartResult * pPartResult1, const C_Gate_DataModel_PartResult * pPartResult2)
{
	return !lessThanSortOnPartID(pPartResult1,pPartResult2);
}
bool C_Gate_DataModel_PartResultVector::lessThanSortOnXLoc(const C_Gate_DataModel_PartResult * pPartResult1, const C_Gate_DataModel_PartResult * pPartResult2)
{
	return pPartResult1->m_nXLoc < pPartResult2->m_nXLoc;
}
bool C_Gate_DataModel_PartResultVector::moreThanSortOnXLoc(const C_Gate_DataModel_PartResult * pPartResult1, const C_Gate_DataModel_PartResult * pPartResult2)
{
	return !lessThanSortOnXLoc(pPartResult1,pPartResult2);
}
bool C_Gate_DataModel_PartResultVector::lessThanSortOnYLoc(const C_Gate_DataModel_PartResult * pPartResult1, const C_Gate_DataModel_PartResult * pPartResult2)
{
	return pPartResult1->m_nYLoc < pPartResult2->m_nYLoc;
}
bool C_Gate_DataModel_PartResultVector::moreThanSortOnYLoc(const C_Gate_DataModel_PartResult * pPartResult1, const C_Gate_DataModel_PartResult * pPartResult2)
{
	return !lessThanSortOnYLoc(pPartResult1,pPartResult2);
}
bool C_Gate_DataModel_PartResultVector::lessThanSortOnSoftBin(const C_Gate_DataModel_PartResult * pPartResult1, const C_Gate_DataModel_PartResult * pPartResult2)
{
	return pPartResult1->m_nSoftBin < pPartResult2->m_nSoftBin;
}
bool C_Gate_DataModel_PartResultVector::moreThanSortOnSoftBin(const C_Gate_DataModel_PartResult * pPartResult1, const C_Gate_DataModel_PartResult * pPartResult2)
{
	return !lessThanSortOnSoftBin(pPartResult1,pPartResult2);
}
bool C_Gate_DataModel_PartResultVector::lessThanSortOnHardBin(const C_Gate_DataModel_PartResult * pPartResult1, const C_Gate_DataModel_PartResult * pPartResult2)
{
	return pPartResult1->m_nHardBin < pPartResult2->m_nHardBin;
}
bool C_Gate_DataModel_PartResultVector::moreThanSortOnHardBin(const C_Gate_DataModel_PartResult * pPartResult1, const C_Gate_DataModel_PartResult * pPartResult2)
{
	return !lessThanSortOnHardBin(pPartResult1,pPartResult2);
}
bool C_Gate_DataModel_PartResultVector::lessThanSortOnTestsExecuted(const C_Gate_DataModel_PartResult * pPartResult1, const C_Gate_DataModel_PartResult * pPartResult2)
{
	return pPartResult1->m_uiTestsExecuted < pPartResult2->m_uiTestsExecuted;
}
bool C_Gate_DataModel_PartResultVector::moreThanSortOnTestsExecuted(const C_Gate_DataModel_PartResult * pPartResult1, const C_Gate_DataModel_PartResult * pPartResult2)
{
	return !lessThanSortOnTestsExecuted(pPartResult1,pPartResult2);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Sort vector
//
// Argument(s) :
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_PartResultVector::Sort(SortOn eSortSelector, bool bAscending)
{
	// Check if already Sorted
	if((m_eSortSelector == eSortSelector) && (m_bSortAscending == bAscending))
		return;

	// Set sort parameters
	m_eSortSelector = eSortSelector;
	m_bSortAscending = bAscending;

	// Now sort
	switch(m_eSortSelector)
	{
	case eSortOnPartID:
		if(m_bSortAscending)
			qSort(begin(),end(),lessThanSortOnPartID);
		else
			qSort(begin(),end(),moreThanSortOnPartID);
		break;
	case eSortOnXLoc:
		if(m_bSortAscending)
			qSort(begin(),end(),lessThanSortOnXLoc);
		else
			qSort(begin(),end(),moreThanSortOnXLoc);
		break;
	case eSortOnYLoc:
		if(m_bSortAscending)
			qSort(begin(),end(),lessThanSortOnYLoc);
		else
			qSort(begin(),end(),moreThanSortOnYLoc);
		break;
	case eSortOnSoftBin:
		if(m_bSortAscending)
			qSort(begin(),end(),lessThanSortOnSoftBin);
		else
			qSort(begin(),end(),moreThanSortOnSoftBin);
		break;
	case eSortOnHardBin:
		if(m_bSortAscending)
			qSort(begin(),end(),lessThanSortOnHardBin);
		else
			qSort(begin(),end(),moreThanSortOnHardBin);
		break;
	case eSortOnTestsExecuted:
		if(m_bSortAscending)
			qSort(begin(),end(),lessThanSortOnTestsExecuted);
		else
			qSort(begin(),end(),moreThanSortOnTestsExecuted);
		break;
	}
}
/////////////////////////////////////////////////////////////////////////////////////////
// WAFERZONE OBJECT
/////////////////////////////////////////////////////////////////////////////////////////
// ERROR MAP
GBEGIN_ERROR_MAP(C_Gate_DataModel_WaferZone)
	GMAP_ERROR(eMalloc,"Failed to allocate memory")
	GMAP_ERROR(eWaferZoneIndexOverflow,"WaferZone index overflow Zone=%d")
GEND_ERROR_MAP(C_Gate_DataModel_WaferZone)

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Constructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_WaferZone::C_Gate_DataModel_WaferZone()
{
	m_uiPass				= 0;
	m_uiFail				= 0;

	m_pnSoftBin				= NULL;
	m_pnHardBin				= NULL;
	m_pZoneParameterStats	= NULL;
	m_nZoneIndex			= -1;
	m_nSiteIndex			= -1;
	m_nMaxSoftBin			= -1;
	m_nMaxHardBin			= -1;
	m_nNbZones				= -1;
	m_nNbSites				= -1;
	ClearData();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Destructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_WaferZone::~C_Gate_DataModel_WaferZone()
{
	ClearData();
}


/////////////////////////////////////////////////////////////////////////////////////////
// Description : Init Constructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_WaferZone::InitWaferZone(	int nZoneIndex, int nSiteIndex,
											int nMaxSoftBin, int nMaxHardBin,
											int nNbZones, int nNbSites,
											int nNbParameters)
{
	int	nIndex;

	m_uiPass				= 0;
	m_uiFail				= 0;

	m_nZoneIndex			= nZoneIndex;
	m_nSiteIndex			= nSiteIndex;
	m_nMaxSoftBin			= nMaxSoftBin;
	m_nMaxHardBin			= nMaxHardBin;
	m_nNbZones				= nNbZones;
	m_nNbSites				= nNbSites;
	m_nNbParameters			= nNbParameters;
	m_pnSoftBin				= new int[m_nMaxSoftBin+1];
	if(m_pnSoftBin == NULL)
	{
		GSET_ERROR0(C_Gate_DataModel_WaferZone, eMalloc, NULL);
		return FALSE;
	}
	for(nIndex=0; nIndex<m_nMaxSoftBin+1; nIndex++)
		m_pnSoftBin[nIndex] = 0;

	m_pnHardBin				= new int[m_nMaxHardBin+1];
	if(m_pnHardBin == NULL)
	{
		GSET_ERROR0(C_Gate_DataModel_WaferZone, eMalloc, NULL);
		return FALSE;
	}
	for(nIndex=0; nIndex<m_nMaxHardBin+1; nIndex++)
		m_pnHardBin[nIndex] = 0;

	m_pZoneParameterStats = NULL;
	if(m_nNbParameters > 0)
	{
		m_pZoneParameterStats	= new C_Gate_DataModel_ParameterStat[m_nNbParameters];
		if(m_pZoneParameterStats == NULL)
		{
			GSET_ERROR0(C_Gate_DataModel_WaferZone, eMalloc, NULL);
			return FALSE;
		}
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Clear object data (free allocated ressources)
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_WaferZone::ClearData()
{
	// Reset variables
	m_uiFlags					= 0;
	m_uiExecs					= 0;
	m_uiPass					= 0;
	m_uiFail					= 0;

	if(m_pnSoftBin)
		delete [] m_pnSoftBin;
	m_pnSoftBin = NULL;
	if(m_pnHardBin)
		delete [] m_pnHardBin;
	m_pnHardBin = NULL;
	if(m_pZoneParameterStats)
		delete [] m_pZoneParameterStats;
	m_pZoneParameterStats = NULL;
}



/////////////////////////////////////////////////////////////////////////////////////////
// Description : Init object data (allocated ressources)
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_WaferZone::InitData()
{
	int	nIndex;
	ClearData();
	m_pnSoftBin = new int[m_nMaxSoftBin+1];
	if(m_pnSoftBin == NULL)
	{
		GSET_ERROR0(C_Gate_DataModel_WaferZone, eMalloc, NULL);
		return FALSE;
	}
	for(nIndex=0; nIndex<m_nMaxSoftBin+1; nIndex++)
		m_pnSoftBin[nIndex] = 0;

	m_pnHardBin = new int[m_nMaxHardBin+1];
	if(m_pnHardBin == NULL)
	{
		GSET_ERROR0(C_Gate_DataModel_WaferZone, eMalloc, NULL);
		return FALSE;
	}
	for(nIndex=0; nIndex<m_nMaxHardBin+1; nIndex++)
		m_pnHardBin[nIndex] = 0;

	m_pZoneParameterStats = NULL;
	if(m_nNbParameters > 0)
	{
		m_pZoneParameterStats	= new C_Gate_DataModel_ParameterStat[m_nNbParameters];
		if(m_pZoneParameterStats == NULL)
		{
			GSET_ERROR0(C_Gate_DataModel_WaferZone, eMalloc, NULL);
			return FALSE;
		}
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Update object data
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_WaferZone::UpdateData(C_Gate_DataModel_Batch* pBatch, C_Gate_DataModel_PartResult	clPartResult)
{
	if((m_pnSoftBin == NULL) || (m_pnHardBin == NULL))
	{
		if(!InitData())
			return FALSE;
	}

	m_uiExecs++;

	Gate_PartResult clPartBin;
	clPartBin.m_nSoftBin = clPartResult.m_nSoftBin;
	clPartBin.m_nHardBin = clPartResult.m_nHardBin;
	clPartBin.m_bPassFailStatusValid = FALSE;
	if(pBatch->IsPartFail(clPartBin))
		m_uiFail++;
	else
		m_uiPass++;

	if(clPartResult.m_nSoftBin <= m_nMaxSoftBin)
		m_pnSoftBin[clPartResult.m_nSoftBin]++;
	if(clPartResult.m_nHardBin <= m_nMaxHardBin)
		m_pnHardBin[clPartResult.m_nHardBin]++;

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////////////////
// Description : Update object data
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_WaferZone::UpdateStats(C_Gate_DataModel_Batch* pBatch, C_Gate_DataModel_TestResultVector *pTestResultVector)
{
	if(m_pZoneParameterStats == NULL)
		return TRUE;

	if(pTestResultVector == NULL)
		return TRUE;

	int nParameterIndex;
	C_Gate_DataStats clStats(NULL);

	for(nParameterIndex=0; nParameterIndex<m_nNbParameters; nParameterIndex++)
	{
		(pTestResultVector[nParameterIndex]).Sort(C_Gate_DataModel_TestResultVector::eSortOnResult, TRUE);
		clStats.ComputeStatCumuls(m_pZoneParameterStats[nParameterIndex],
								  (pTestResultVector[nParameterIndex]));
		clStats.ComputeBasicStats(m_pZoneParameterStats[nParameterIndex],
								  pBatch->m_clBatchID.m_pParameterSet->m_pclParameters);
		clStats.ComputeQuartiles(m_pZoneParameterStats[nParameterIndex],
								 (pTestResultVector[nParameterIndex]));
		clStats.ComputeShapeDistribution(m_pZoneParameterStats[nParameterIndex],
								 (pTestResultVector[nParameterIndex]));
	}

	return TRUE;
}




/////////////////////////////////////////////////////////////////////////////////////////
// WAFERPOINT OBJECT
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// Description : Constructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_WaferPoint::C_Gate_DataModel_WaferPoint()
{
	m_nZoneIndex	= -1;
	m_nRunIndex		= -1;
	m_nSiteIndex	= -1;
	m_nSoftBin		= -1;
	m_nHardBin		= -1;
	m_nPass			= 0;
	m_pnParameterPass	= NULL;
	m_pfParameterValues = NULL;
	m_pnParameterPlots	= NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Destructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_WaferPoint::~C_Gate_DataModel_WaferPoint()
{
	if(m_pnParameterPass)
		delete []m_pnParameterPass;
	if(m_pfParameterValues)
		delete []m_pfParameterValues;
	if(m_pnParameterPlots)
		delete []m_pnParameterPlots;
}


/////////////////////////////////////////////////////////////////////////////////////////
// WAFERMAP OBJECT
/////////////////////////////////////////////////////////////////////////////////////////
// ERROR MAP
GBEGIN_ERROR_MAP(C_Gate_DataModel_WaferMap)
	GMAP_ERROR(eMalloc,"Failed to allocate memory")
	GMAP_ERROR(eWaferMapIndexOverflow,"WaferMap index overflow (X=%d, Y=%d)")
GEND_ERROR_MAP(C_Gate_DataModel_WaferMap)

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Constructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_WaferMap::C_Gate_DataModel_WaferMap()
{
	m_uiFlags	=	YIELD123_FLAGS_WAFERMAP_BINNING;
	m_nXOffset	=	GEX_PLUGIN_INVALID_VALUE_INT;
	m_nYOffset	=	GEX_PLUGIN_INVALID_VALUE_INT;
	m_nXMax		=	GEX_PLUGIN_INVALID_VALUE_INT;
	m_nYMax		=	GEX_PLUGIN_INVALID_VALUE_INT;
	m_nNbZones = 12;
	m_nNbSites = -1;
	m_nNbParameters = -1;
	m_pWaferPoints = NULL;
	m_pWaferZones = NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Destructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_WaferMap::~C_Gate_DataModel_WaferMap()
{
	ClearWaferMap();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Clear WaferMap object (free allocated ressources)
//
// Argument(s) :
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_WaferMap::ClearWaferMap()
{
	int nXIndex, nYIndex;
	// Free allocated ressources
	if(m_pWaferPoints != NULL)
	{
		for(nXIndex=0; nXIndex<m_nXMax; nXIndex++)
		{
			for(nYIndex=0; nYIndex<m_nYMax; nYIndex++)
			{
				if(m_pWaferPoints[nXIndex][nYIndex].m_pnParameterPass)
					delete [] m_pWaferPoints[nXIndex][nYIndex].m_pnParameterPass;
				if(m_pWaferPoints[nXIndex][nYIndex].m_pfParameterValues)
					delete [] m_pWaferPoints[nXIndex][nYIndex].m_pfParameterValues;
				if(m_pWaferPoints[nXIndex][nYIndex].m_pnParameterPlots)
					delete [] m_pWaferPoints[nXIndex][nYIndex].m_pnParameterPlots;
			}
			delete [] m_pWaferPoints[nXIndex];
		}
		delete [] m_pWaferPoints;
		m_pWaferPoints = NULL;
	}
	if(m_pWaferZones != NULL)
	{
		for(nXIndex=0; nXIndex<(m_nNbZones+1); nXIndex++)
			delete [] m_pWaferZones[nXIndex];
		delete [] m_pWaferZones;
		m_pWaferZones = NULL;
	}
	m_nXOffset	=	GEX_PLUGIN_INVALID_VALUE_INT;
	m_nYOffset	=	GEX_PLUGIN_INVALID_VALUE_INT;
	m_nXMax		=	GEX_PLUGIN_INVALID_VALUE_INT;
	m_nYMax		=	GEX_PLUGIN_INVALID_VALUE_INT;
	m_nNbZones	=	12;
	m_nNbSites	=	-1;
	m_nNbParameters = -1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Give the Zone Nb in the Wafer
//
// Argument(s) :
//	int nXLoc
//	int nYLoc
//		Location of the current point
//  bool bCutPeriph to elimine the periphery border of the circle
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
int C_Gate_DataModel_WaferMap::GiveZoneIndex(int nXLoc, int nYLoc, bool bCutPeriph)
{
	int				nA, nB;
	float			fC, fR;
	int				nRA, nRB;
	int				nZoneIndex = -1;

	nRA = (m_nXMax/2);
	nRB = (m_nYMax/2);
	// Computation of the zone
	if(nXLoc < nRA)
	{
		nA = nRA - nXLoc;
		if(nYLoc > nRB)
		{
			nZoneIndex = 4;
			nB = nYLoc - nRB ;
		}
		else
		{
			nZoneIndex = 1;
			nB = nRB - nYLoc;
		}
	}
	else
	{
		nA = nXLoc - nRA ;
		if(nYLoc > nRB)
		{
			nZoneIndex = 7;
			nB = nYLoc - nRB ;
		}
		else
		{
			nZoneIndex = 10;
			nB = nRB - nYLoc;
		}
	}
	// Add section in each zone
	// find the position in 3 circles with Pythagore

	// adaptive ray, depends of X and Y position
	// ellipse configurartion
	float	fAPercent = 0.0;
	float	fBPercent = 0.0;

	if((nA == 0) && (nB == 0))
	{
		fAPercent = 1.0;
		fBPercent = 1.0;
	}
	else
	{
		fAPercent = ((float) nA/(float)nRA);
		fBPercent = ((float) nB/(float)nRB);
	}

	fR = (fAPercent*(float)nRA) + (fBPercent*(float)nRB);
	fR /= (fAPercent + fBPercent);

	// standard circle
	//fR = ((float)(m_nXMax+m_nYMax))/4.0;

	fC = sqrt(pow((float)nA,2) + pow((float)nB,2));
	if(fC > ((2.0/3.0)*fR))
	{
		//last circle
		nZoneIndex += 2;
	}
	else if(fC > ((1.0/3.0)*fR))
	{
		// second circle
		nZoneIndex += 1;
	}
	else
	{
		// first circle
		nZoneIndex += 0;
	}

	if(bCutPeriph)
	{
		if(fC >= (fR-1.0))
			nZoneIndex = -1;
	}

	return nZoneIndex;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Give the MaxDeviation if have a CircleDeviation
//
// Argument(s) :
//	pBatch
//	fLimitDeviation = x%
//	bUsePass = use Pass information
//	bSoftBin = if !bUsePass, use the SoftBin or the HardBin
//	nBinIndex = the Bin to use
//	iParameterIndex = if > -1 then use the parameter mean
//
// Return type : 0 if no deviation
//				 - if negative deviation
//				 + if positive deviation
/////////////////////////////////////////////////////////////////////////////////////////
float C_Gate_DataModel_WaferMap::WaferZoneHasCircleDeviation(float fLimitDeviation, bool bUsePass, bool bSoftBin, int nBinIndex, int iParameterIndex)
{
	// have some stats
	float	fExecs[6];
	float	fPass[6];
	float	fYield[6];
	int		nNbZones;
	int		nZoneIndex, nIndex;
	float	fDeviation, fMaxDeviation;
	bool	bDeviation = false;	// if have a deviation
	int		nDeviation;			// indecate the way of the deviation

	if(m_pWaferZones == NULL)
		return 0.0;

	if((m_nNbParameters == -1) && (iParameterIndex > -1))
		return 0.0;

	// Initialization
	fMaxDeviation = 0.0;
	for(nIndex=0; nIndex<6; nIndex++)
	{
		fExecs[nIndex] = 0;
		fPass[nIndex] = 0;
		fYield[nIndex] = 0;
	}
	// parse all zones and find some variation

	// CIRCLES DEVIATION
	// information about each circle
	nNbZones = 3;
	// considere have a deviation if the yield move more then 10%
	nDeviation = 0;


	for(nZoneIndex=1; nZoneIndex<=m_nNbZones; nZoneIndex+=3)
	{
		for(nIndex=0; nIndex<nNbZones; nIndex++)
		{
			if(iParameterIndex < 0)
			{
				fExecs[nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_uiExecs)*100.0;
				if(bUsePass)
					fPass[nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_uiPass)*100.0;
				else if(bSoftBin)
					fPass[nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pnSoftBin[nBinIndex])*100.0;
				else
					fPass[nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pnHardBin[nBinIndex]);
			}
			else
			{
				fExecs[nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pZoneParameterStats[iParameterIndex].m_uiExecs);
				if(bUsePass)
					fPass[nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pZoneParameterStats[iParameterIndex].m_uiPass)*100.0;
				else
					fPass[nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pZoneParameterStats[iParameterIndex].m_lfSumX);
			}

		}
	}
	for(nIndex=0; nIndex<nNbZones; nIndex++)
	{
		fYield[nIndex] = fPass[nIndex]/fExecs[nIndex];
		if(nIndex>0)
		{
			fDeviation = fYield[nIndex-1]-fYield[nIndex];
			if((fDeviation>=0.0) && (fDeviation > fLimitDeviation))
			{
				bDeviation = true;
				nDeviation += 1;
				fMaxDeviation = gex_max(fMaxDeviation,fDeviation);
			}
			else if((fDeviation< 0.0) && (fDeviation < -fLimitDeviation))
			{
				bDeviation = true;
				nDeviation -= 1;
				fMaxDeviation = gex_max(fMaxDeviation,-fDeviation);
			}
		}
	}

	if(!bDeviation)
		return 0.0;

	if(nDeviation < 0)
		return -fMaxDeviation;
	else
		return fMaxDeviation;

}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Give the MaxDeviation if have a CircleDeviation
//
// Argument(s) :
//	pBatch
//	fLimitDeviation = x%
//	bUsePass = use Pass information
//	bSoftBin = if !bUsePass, use the SoftBin or the HardBin
//	nBinIndex = the Bin to use
//	iParameterIndex = if > -1 then use the parameter mean
//
// Return type : 0 if no deviation
//				 - if negative deviation
//				 + if positive deviation
/////////////////////////////////////////////////////////////////////////////////////////
float C_Gate_DataModel_WaferMap::WaferZoneHasDiagonalLeftTopDeviation( float fLimitDeviation, bool bUsePass, bool bSoftBin, int nBinIndex, int iParameterIndex)
{
	// have some stats
	float	fExecs[6];
	float	fPass[6];
	float	fYield[6];
	int		nNbZones;
	int		nZoneIndex, nIndex;
	float	fDeviation, fMaxDeviation;
	bool	bDeviation = false;	// if have a deviation
	int		nDeviation;			// indecate the way of the deviation

	if(m_pWaferZones == NULL)
		return 0.0;

	if((m_nNbParameters == -1) && (iParameterIndex > -1))
		return 0.0;

	// Initialization
	fMaxDeviation = 0.0;
	for(nIndex=0; nIndex<6; nIndex++)
	{
		fExecs[nIndex] = 0;
		fPass[nIndex] = 0;
		fYield[nIndex] = 0;
	}
	// parse all zones and find some variation

	// DIAGONAL Left-Top DEVIATION
	// information about each zones
	nNbZones = 6;
	nDeviation = 0;

	// Initialization
	for(nIndex=0; nIndex<6; nIndex++)
	{
		fExecs[nIndex] = 0;
		fPass[nIndex] = 0;
		fYield[nIndex] = 0;
	}

	nZoneIndex=1;
	for(nIndex=0; nIndex<3; nIndex++)
	{
		if(iParameterIndex < 0)
		{
			fExecs[2-nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_uiExecs);
			if(bUsePass)
				fPass[2-nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_uiPass)*100.0;
			else if(bSoftBin)
				fPass[2-nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pnSoftBin[nBinIndex])*100.0;
			else
				fPass[2-nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pnHardBin[nBinIndex])*100.0;
		}
		else
		{
			fExecs[2-nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pZoneParameterStats[iParameterIndex].m_uiExecs);
			if(bUsePass)
				fPass[2-nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pZoneParameterStats[iParameterIndex].m_uiPass)*100.0;
			else
				fPass[2-nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pZoneParameterStats[iParameterIndex].m_lfSumX);
		}
	}

	nZoneIndex=7;
	for(nIndex=0; nIndex<3; nIndex++)
	{
		if(iParameterIndex < 0)
		{
			fExecs[3+nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_uiExecs);
			if(bUsePass)
				fPass[3+nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_uiPass)*100.0;
			else if(bSoftBin)
				fPass[3+nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pnSoftBin[nBinIndex])*100.0;
			else
				fPass[3+nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pnHardBin[nBinIndex])*100.0;
		}
		else
		{
			fExecs[3+nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pZoneParameterStats[iParameterIndex].m_uiExecs);
			if(bUsePass)
				fPass[3+nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pZoneParameterStats[iParameterIndex].m_uiPass)*100.0;
			else
				fPass[3+nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pZoneParameterStats[iParameterIndex].m_lfSumX);
		}
	}

	for(nIndex=0; nIndex<nNbZones; nIndex++)
	{
		fYield[nIndex] = fPass[nIndex]/fExecs[nIndex];
		if(nIndex>0)
		{
			fDeviation = fYield[nIndex-1]-fYield[nIndex];
			if((fDeviation>=0.0) && (fDeviation > fLimitDeviation))
			{
				bDeviation = true;
				nDeviation += 1;
				fMaxDeviation = gex_max(fMaxDeviation,fDeviation);
			}
			else if((fDeviation< 0.0) && (fDeviation < -fLimitDeviation))
			{
				bDeviation = true;
				nDeviation -= 1;
				fMaxDeviation = gex_max(fMaxDeviation,-fDeviation);
			}
		}
	}




	if(!bDeviation)
		return 0.0;

	if(nDeviation < 0)
		return -fMaxDeviation;
	else
		return fMaxDeviation;

}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Give the MaxDeviation if have a CircleDeviation
//
// Argument(s) :
//	pBatch
//	fLimitDeviation = x%
//	bUsePass = use Pass information
//	bSoftBin = if !bUsePass, use the SoftBin or the HardBin
//	nBinIndex = the Bin to use
//	iParameterIndex = if > -1 then use the parameter mean
//
// Return type : 0 if no deviation
//				 - if negative deviation
//				 + if positive deviation
/////////////////////////////////////////////////////////////////////////////////////////
float C_Gate_DataModel_WaferMap::WaferZoneHasDiagonalLeftBottomDeviation(float fLimitDeviation, bool bUsePass, bool bSoftBin, int nBinIndex, int iParameterIndex)
{
	// have some stats
	float	fExecs[6];
	float	fPass[6];
	float	fYield[6];
	int		nNbZones;
	int		nZoneIndex, nIndex;
	float	fDeviation, fMaxDeviation;
	bool	bDeviation = false;	// if have a deviation
	int		nDeviation;			// indecate the way of the deviation

	if(m_pWaferZones == NULL)
		return 0.0;

	if((m_nNbParameters == -1) && (iParameterIndex > -1))
		return 0.0;

	// Initialization
	fMaxDeviation = 0.0;
	for(nIndex=0; nIndex<6; nIndex++)
	{
		fExecs[nIndex] = 0;
		fPass[nIndex] = 0;
		fYield[nIndex] = 0;
	}
	// parse all zones and find some variation

	// DIAGONAL Left-Bottom DEVIATION
	// information about each zones
	nNbZones = 6;
	nDeviation = 0;

	// Initialization
	for(nIndex=0; nIndex<6; nIndex++)
	{
		fExecs[nIndex] = 0;
		fPass[nIndex] = 0;
		fYield[nIndex] = 0;
	}

	nZoneIndex=4;
	for(nIndex=0; nIndex<3; nIndex++)
	{
		if(iParameterIndex < 0)
		{
			fExecs[2-nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_uiExecs);
			if(bUsePass)
				fPass[2-nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_uiPass)*100.0;
			else if(bSoftBin)
				fPass[2-nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pnSoftBin[nBinIndex])*100.0;
			else
				fPass[2-nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pnHardBin[nBinIndex])*100.0;
		}
		else
		{
			fExecs[2-nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pZoneParameterStats[iParameterIndex].m_uiExecs);
			if(bUsePass)
				fPass[2-nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pZoneParameterStats[iParameterIndex].m_uiPass)*100.0;
			else
				fPass[2-nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pZoneParameterStats[iParameterIndex].m_lfSumX);
		}
	}

	nZoneIndex=10;
	for(nIndex=0; nIndex<3; nIndex++)
	{
		if(iParameterIndex < 0)
		{
			fExecs[3+nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_uiExecs);
			if(bUsePass)
				fPass[3+nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_uiPass)*100.0;
			else if(bSoftBin)
				fPass[3+nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pnSoftBin[nBinIndex])*100.0;
			else
				fPass[3+nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pnHardBin[nBinIndex])*100.0;
		}
		else
		{
			fExecs[3+nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pZoneParameterStats[iParameterIndex].m_uiExecs);
			if(bUsePass)
				fPass[3+nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pZoneParameterStats[iParameterIndex].m_uiPass)*100.0;
			else
				fPass[3+nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pZoneParameterStats[iParameterIndex].m_lfSumX);
		}
	}

	for(nIndex=0; nIndex<nNbZones; nIndex++)
	{
		fYield[nIndex] = fPass[nIndex]/fExecs[nIndex];
		if(nIndex>0)
		{
			fDeviation = fYield[nIndex-1]-fYield[nIndex];
			if((fDeviation>=0.0) && (fDeviation > fLimitDeviation))
			{
				bDeviation = true;
				nDeviation += 1;
				fMaxDeviation = gex_max(fMaxDeviation,fDeviation);
			}
			else if((fDeviation< 0.0) && (fDeviation < -fLimitDeviation))
			{
				bDeviation = true;
				nDeviation -= 1;
				fMaxDeviation = gex_max(fMaxDeviation,-fDeviation);
			}
		}
	}




	if(!bDeviation)
		return 0.0;

	if(nDeviation < 0)
		return -fMaxDeviation;
	else
		return fMaxDeviation;

}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Give the MaxDeviation if have a CircleDeviation
//
// Argument(s) :
//	pBatch
//	fLimitDeviation = x%
//	bUsePass = use Pass information
//	bSoftBin = if !bUsePass, use the SoftBin or the HardBin
//	nBinIndex = the Bin to use
//	iParameterIndex = if > -1 then use the parameter mean
//
// Return type : 0 if no deviation
//				 - if negative deviation
//				 + if positive deviation
/////////////////////////////////////////////////////////////////////////////////////////
float C_Gate_DataModel_WaferMap::WaferZoneHasVerticalDeviation(float fLimitDeviation, bool bUsePass, bool bSoftBin, int nBinIndex, int iParameterIndex)
{
	// have some stats
	float	fExecs[6];
	float	fPass[6];
	float	fYield[6];
	int		nNbZones;
	int		nZoneIndex, nIndex;
	float	fDeviation, fMaxDeviation;
	bool	bDeviation = false;	// if have a deviation
	int		nDeviation;			// indecate the way of the deviation

	if(m_pWaferZones == NULL)
		return 0.0;

	if((m_nNbParameters == -1) && (iParameterIndex > -1))
		return 0.0;

	// Initialization
	fMaxDeviation = 0.0;
	for(nIndex=0; nIndex<6; nIndex++)
	{
		fExecs[nIndex] = 0;
		fPass[nIndex] = 0;
		fYield[nIndex] = 0;
	}
	// parse all zones and find some variation

	// VERTICAL DEVIATION
	// information about each zone
	nNbZones = 6;
	nDeviation = 0;

	// Initialization
	for(nIndex=0; nIndex<6; nIndex++)
	{
		fExecs[nIndex] = 0;
		fPass[nIndex] = 0;
		fYield[nIndex] = 0;
	}

	nZoneIndex=1;
	for(nIndex=0; nIndex<3; nIndex++)
	{
		if(iParameterIndex < 0)
		{
			fExecs[2-nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_uiExecs);
			if(bUsePass)
				fPass[2-nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_uiPass)*100.0;
			else if(bSoftBin)
				fPass[2-nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pnSoftBin[nBinIndex])*100.0;
			else
				fPass[2-nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pnHardBin[nBinIndex])*100.0;
		}
		else
		{
			fExecs[2-nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pZoneParameterStats[iParameterIndex].m_uiExecs);
			if(bUsePass)
				fPass[2-nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pZoneParameterStats[iParameterIndex].m_uiPass)*100.0;
			else
				fPass[2-nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pZoneParameterStats[iParameterIndex].m_lfSumX);
		}
	}

	nZoneIndex=10;
	for(nIndex=0; nIndex<3; nIndex++)
	{
		if(iParameterIndex < 0)
		{
			fExecs[2-nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_uiExecs);
			if(bUsePass)
				fPass[2-nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_uiPass)*100.0;
			else if(bSoftBin)
				fPass[2-nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pnSoftBin[nBinIndex])*100.0;
			else
				fPass[2-nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pnHardBin[nBinIndex])*100.0;
		}
		else
		{
			fExecs[2-nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pZoneParameterStats[iParameterIndex].m_uiExecs);
			if(bUsePass)
				fPass[2-nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pZoneParameterStats[iParameterIndex].m_uiPass)*100.0;
			else
				fPass[2-nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pZoneParameterStats[iParameterIndex].m_lfSumX);
		}
	}

	nZoneIndex=4;
	for(nIndex=0; nIndex<3; nIndex++)
	{
		if(iParameterIndex < 0)
		{
			fExecs[3+nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_uiExecs);
			if(bUsePass)
				fPass[3+nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_uiPass)*100.0;
			else if(bSoftBin)
				fPass[3+nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pnSoftBin[nBinIndex])*100.0;
			else
				fPass[3+nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pnHardBin[nBinIndex])*100.0;
		}
		else
		{
			fExecs[3+nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pZoneParameterStats[iParameterIndex].m_uiExecs);
			if(bUsePass)
				fPass[3+nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pZoneParameterStats[iParameterIndex].m_uiPass)*100.0;
			else
				fPass[3+nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pZoneParameterStats[iParameterIndex].m_lfSumX);
		}
	}

	nZoneIndex=7;
	for(nIndex=0; nIndex<3; nIndex++)
	{
		if(iParameterIndex < 0)
		{
			fExecs[3+nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_uiExecs);
			if(bUsePass)
				fPass[3+nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_uiPass)*100.0;
			else if(bSoftBin)
				fPass[3+nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pnSoftBin[nBinIndex])*100.0;
			else
				fPass[3+nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pnHardBin[nBinIndex])*100.0;
		}
		else
		{
			fExecs[3+nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pZoneParameterStats[iParameterIndex].m_uiExecs);
			if(bUsePass)
				fPass[3+nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pZoneParameterStats[iParameterIndex].m_uiPass)*100.0;
			else
				fPass[3+nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pZoneParameterStats[iParameterIndex].m_lfSumX);
		}
	}

	for(nIndex=0; nIndex<nNbZones; nIndex++)
	{
		fYield[nIndex] = fPass[nIndex]/fExecs[nIndex];
		if(nIndex>0)
		{
			fDeviation = fYield[nIndex-1]-fYield[nIndex];
			if((fDeviation>=0.0) && (fDeviation > fLimitDeviation))
			{
				bDeviation = true;
				nDeviation += 1;
				fMaxDeviation = gex_max(fMaxDeviation,fDeviation);
			}
			else if((fDeviation< 0.0) && (fDeviation < -fLimitDeviation))
			{
				bDeviation = true;
				nDeviation -= 1;
				fMaxDeviation = gex_max(fMaxDeviation,-fDeviation);
			}
		}
	}




	if(!bDeviation)
		return 0.0;

	if(nDeviation < 0)
		return -fMaxDeviation;
	else
		return fMaxDeviation;

}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Give the MaxDeviation if have a CircleDeviation
//
// Argument(s) :
//	pBatch
//	fLimitDeviation = x%
//	bUsePass = use Pass information
//	bSoftBin = if !bUsePass, use the SoftBin or the HardBin
//	nBinIndex = the Bin to use
//	iParameterIndex = if > -1 then use the parameter mean
//
// Return type : 0 if no deviation
//				 - if negative deviation
//				 + if positive deviation
/////////////////////////////////////////////////////////////////////////////////////////
float C_Gate_DataModel_WaferMap::WaferZoneHasHorizontalDeviation(float fLimitDeviation, bool bUsePass, bool bSoftBin, int nBinIndex, int iParameterIndex)
{
	// have some stats
	float	fExecs[6];
	float	fPass[6];
	float	fYield[6];
	int		nNbZones;
	int		nZoneIndex, nIndex;
	float	fDeviation, fMaxDeviation;
	bool	bDeviation = false;	// if have a deviation
	int		nDeviation;			// indecate the way of the deviation

	if(m_pWaferZones == NULL)
		return 0.0;

	if((m_nNbParameters == -1) && (iParameterIndex > -1))
		return 0.0;

	// Initialization
	fMaxDeviation = 0.0;
	for(nIndex=0; nIndex<6; nIndex++)
	{
		fExecs[nIndex] = 0;
		fPass[nIndex] = 0;
		fYield[nIndex] = 0;
	}
	// parse all zones and find some variation

	// HORIZONTAL DEVIATION
	// information about each circle
	nNbZones = 6;
	nDeviation = 0;
	nZoneIndex=1;
	for(nIndex=0; nIndex<3; nIndex++)
	{
		if(iParameterIndex < 0)
		{
			fExecs[2-nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_uiExecs);
			if(bUsePass)
				fPass[2-nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_uiPass)*100.0;
			else if(bSoftBin)
				fPass[2-nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pnSoftBin[nBinIndex])*100.0;
			else
				fPass[2-nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pnHardBin[nBinIndex])*100.0;
		}
		else
		{
			fExecs[2-nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pZoneParameterStats[iParameterIndex].m_uiExecs);
			if(bUsePass)
				fPass[2-nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pZoneParameterStats[iParameterIndex].m_uiPass)*100.0;
			else
				fPass[2-nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pZoneParameterStats[iParameterIndex].m_lfSumX);
		}
	}

	nZoneIndex=4;
	for(nIndex=0; nIndex<3; nIndex++)
	{
		if(iParameterIndex < 0)
		{
			fExecs[2-nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_uiExecs);
			if(bUsePass)
				fPass[2-nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_uiPass)*100.0;
			else if(bSoftBin)
				fPass[2-nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pnSoftBin[nBinIndex])*100.0;
			else
				fPass[2-nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pnHardBin[nBinIndex])*100.0;
		}
		else
		{
			fExecs[2-nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pZoneParameterStats[iParameterIndex].m_uiExecs);
			if(bUsePass)
				fPass[2-nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pZoneParameterStats[iParameterIndex].m_uiPass)*100.0;
			else
				fPass[2-nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pZoneParameterStats[iParameterIndex].m_lfSumX);
		}
	}

	nZoneIndex=7;
	for(nIndex=0; nIndex<3; nIndex++)
	{
		if(iParameterIndex < 0)
		{
			fExecs[3+nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_uiExecs);
			if(bUsePass)
				fPass[3+nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_uiPass)*100.0;
			else if(bSoftBin)
				fPass[3+nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pnSoftBin[nBinIndex])*100.0;
			else
				fPass[3+nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pnHardBin[nBinIndex])*100.0;
		}
		else
		{
			fExecs[3+nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pZoneParameterStats[iParameterIndex].m_uiExecs);
			if(bUsePass)
				fPass[3+nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pZoneParameterStats[iParameterIndex].m_uiPass)*100.0;
			else
				fPass[3+nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pZoneParameterStats[iParameterIndex].m_lfSumX);
		}
	}

	nZoneIndex=10;
	for(nIndex=0; nIndex<3; nIndex++)
	{
		if(iParameterIndex < 0)
		{
			fExecs[3+nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_uiExecs);
			if(bUsePass)
				fPass[3+nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_uiPass)*100.0;
			else if(bSoftBin)
				fPass[3+nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pnSoftBin[nBinIndex])*100.0;
			else
				fPass[3+nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pnHardBin[nBinIndex])*100.0;
		}
		else
		{
			fExecs[3+nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pZoneParameterStats[iParameterIndex].m_uiExecs);
			if(bUsePass)
				fPass[3+nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pZoneParameterStats[iParameterIndex].m_uiPass)*100.0;
			else
				fPass[3+nIndex] += (float)(m_pWaferZones[nZoneIndex+nIndex][0].m_pZoneParameterStats[iParameterIndex].m_lfSumX);
		}
	}

	for(nIndex=0; nIndex<nNbZones; nIndex++)
	{
		fYield[nIndex] = fPass[nIndex]/fExecs[nIndex];
		if(nIndex>0)
		{
			fDeviation = fYield[nIndex-1]-fYield[nIndex];
			if((fDeviation>=0.0) && (fDeviation > fLimitDeviation))
			{
				bDeviation = true;
				nDeviation += 1;
				fMaxDeviation = gex_max(fMaxDeviation,fDeviation);
			}
			else if((fDeviation< 0.0) && (fDeviation < -fLimitDeviation))
			{
				bDeviation = true;
				nDeviation -= 1;
				fMaxDeviation = gex_max(fMaxDeviation,-fDeviation);
			}
		}
	}


	if(!bDeviation)
		return 0.0;

	if(nDeviation < 0)
		return -fMaxDeviation;
	else
		return fMaxDeviation;

}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Initialize variables and allocate memory for WaferMap results
//
// Argument(s) :
//
//	C_Gate_DataModel_Batch * pBatch
//		Parent
//
// Return type : bool
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_WaferMap::InitializeWaferMap(C_Gate_DataModel_Batch* pBatch)
{
	C_Gate_DataModel_Site *pSite;
	unsigned int	uiIndex;
	int				nIndex;
	int				nXMax;
	int				nYMax;
	int				nXIndex, nYIndex;
	int				nMaxSoftBin, nMaxHardBin;
	int				nXCenter, nXMaxCenter, nXMinCenter;
	int				nYCenter, nYMaxCenter, nYMinCenter;


	// Here, all stats are already computed

	// First clear data in case
	ClearWaferMap();

	// Check if valid wafermap and parametric test



	// Initialize value
	// For each site



	// to find the dimensions and the center of the wafer

	nXMax		=  -GEX_PLUGIN_INVALID_VALUE_INT;
	nYMax		=  -GEX_PLUGIN_INVALID_VALUE_INT;
	nXMaxCenter = nXMinCenter = 0;
	nYMaxCenter = nYMinCenter = 0;
	nMaxSoftBin = -1;
	nMaxHardBin = -1;

	m_nXOffset	= GEX_PLUGIN_INVALID_VALUE_INT;
	m_nYOffset	= GEX_PLUGIN_INVALID_VALUE_INT;
	m_nXMax		= -GEX_PLUGIN_INVALID_VALUE_INT;
	m_nYMax		= -GEX_PLUGIN_INVALID_VALUE_INT;
	m_nNbParameters = -1;


	pSite = pBatch->GetFirstSite();
	while(pSite)
	{
		// Find the min and the max position for X and Y
		for(uiIndex=0; uiIndex<pSite->m_uiNbRuns; uiIndex++)
		{
			if((pSite->m_pPartResults[uiIndex].m_uiFlags & YIELD123_PARTRESULT_EXECUTED)
			&& ((pSite->m_pPartResults[uiIndex].m_nXLoc != -32768) && (pSite->m_pPartResults[uiIndex].m_nYLoc != -32768)))
			{

				m_nXOffset=gex_min(m_nXOffset,pSite->m_pPartResults[uiIndex].m_nXLoc);
				m_nYOffset=gex_min(m_nYOffset,pSite->m_pPartResults[uiIndex].m_nYLoc);
				m_nXMax=gex_max(m_nXMax,pSite->m_pPartResults[uiIndex].m_nXLoc);
				m_nYMax=gex_max(m_nYMax,pSite->m_pPartResults[uiIndex].m_nYLoc);
				nMaxSoftBin = gex_max(nMaxSoftBin, pSite->m_pPartResults[uiIndex].m_nSoftBin);
				nMaxHardBin = gex_max(nMaxHardBin, pSite->m_pPartResults[uiIndex].m_nHardBin);
				// find the max X and the corresponding Y line
				if(pSite->m_pPartResults[uiIndex].m_nXLoc > nXMax)
				{
					nXMax = pSite->m_pPartResults[uiIndex].m_nXLoc;
					nYMaxCenter = nYMinCenter = pSite->m_pPartResults[uiIndex].m_nYLoc;
				}
				else if(pSite->m_pPartResults[uiIndex].m_nXLoc == nXMax)
				{
					nYMaxCenter = gex_max(nYMaxCenter,pSite->m_pPartResults[uiIndex].m_nYLoc);
					nYMinCenter = gex_min(nYMinCenter,pSite->m_pPartResults[uiIndex].m_nYLoc);
				}
				// find the max Y and the corresponding Y line
				if(pSite->m_pPartResults[uiIndex].m_nYLoc > nYMax)
				{
					nYMax = pSite->m_pPartResults[uiIndex].m_nYLoc;
					nXMaxCenter = nXMinCenter = pSite->m_pPartResults[uiIndex].m_nXLoc;
				}
				else if(pSite->m_pPartResults[uiIndex].m_nYLoc == nYMax)
				{
					nXMaxCenter = gex_max(nXMaxCenter,pSite->m_pPartResults[uiIndex].m_nXLoc);
					nXMinCenter = gex_min(nXMinCenter,pSite->m_pPartResults[uiIndex].m_nXLoc);
				}
			}

		}

		m_nNbSites = gex_max(m_nNbSites, (int)pSite->m_uiSiteIndex+1);
		m_nNbParameters = gex_max(m_nNbParameters, (int) pSite->m_uiNbParameters);
		pSite = pBatch->GetNextSite();
	}

	// if no wafer information, exit
	if((m_nXOffset == GEX_PLUGIN_INVALID_VALUE_INT) && (m_nYOffset == GEX_PLUGIN_INVALID_VALUE_INT))
	{
		m_nXOffset	=	GEX_PLUGIN_INVALID_VALUE_INT;
		m_nYOffset	=	GEX_PLUGIN_INVALID_VALUE_INT;
		m_nXMax		=	-1;
		m_nYMax		=	-1;
		m_nNbZones = -1;
		m_nNbSites = -1;
		m_nNbParameters = -1;
		m_pWaferPoints = NULL;
		m_pWaferZones = NULL;

		return TRUE;
	}

	m_nXMax -= m_nXOffset;
	m_nYMax -= m_nYOffset;

	m_nXMax++;	// Invalid index
	m_nYMax++;	// Invalid index

	// recenter the wafer
	nXMaxCenter  -= m_nXOffset;
	nXMinCenter  -= m_nXOffset;
	nXCenter = (nXMaxCenter + nXMinCenter)/2;
	if(nXCenter*2 > m_nXMax)
		m_nXMax = nXCenter*2;

	nYMaxCenter  -= m_nYOffset;
	nYMinCenter  -= m_nYOffset;
	nYCenter = (nYMaxCenter + nYMinCenter)/2;
	if(nYCenter*2 > m_nYMax)
		m_nYMax = nYCenter*2;


	if(!(m_uiFlags & YIELD123_FLAGS_WAFERMAP_PARAMETRICS))
	{
		// if no wafermap parametrics
		m_nNbParameters = -1;
	}

	// Allocate memory for WaferMap
	m_pWaferPoints = new C_Gate_DataModel_WaferPoint*[m_nXMax];
	if(m_pWaferPoints == NULL)
	{
		GSET_ERROR0(C_Gate_DataModel_WaferMap, eMalloc, NULL);
		return FALSE;
	}
	for(nXIndex=0; nXIndex<m_nXMax; nXIndex++)
	{
		m_pWaferPoints[nXIndex]  = new C_Gate_DataModel_WaferPoint[m_nYMax];
		if(m_pWaferPoints[nXIndex] == NULL)
		{
			GSET_ERROR0(C_Gate_DataModel_WaferMap, eMalloc, NULL);
			return FALSE;
		}
		if(m_nNbParameters > 0)
		{
			for(nYIndex=0; nYIndex<m_nYMax; nYIndex++)
			{
				m_pWaferPoints[nXIndex][nYIndex].m_pnParameterPass = new int[m_nNbParameters];
				if(m_pWaferPoints[nXIndex][nYIndex].m_pnParameterPass == NULL)
				{
					GSET_ERROR0(C_Gate_DataModel_WaferMap, eMalloc, NULL);
					return FALSE;
				}
				for(nIndex=0; nIndex<m_nNbParameters; nIndex++)
					m_pWaferPoints[nXIndex][nYIndex].m_pnParameterPass[nIndex]=0;

				m_pWaferPoints[nXIndex][nYIndex].m_pfParameterValues = new float[m_nNbParameters];
				if(m_pWaferPoints[nXIndex][nYIndex].m_pfParameterValues == NULL)
				{
					GSET_ERROR0(C_Gate_DataModel_WaferMap, eMalloc, NULL);
					return FALSE;
				}
				for(nIndex=0; nIndex<m_nNbParameters; nIndex++)
					m_pWaferPoints[nXIndex][nYIndex].m_pfParameterValues[nIndex]=-1;

				m_pWaferPoints[nXIndex][nYIndex].m_pnParameterPlots = new int[m_nNbParameters];
				if(m_pWaferPoints[nXIndex][nYIndex].m_pnParameterPlots == NULL)
				{
					GSET_ERROR0(C_Gate_DataModel_WaferMap, eMalloc, NULL);
					return FALSE;
				}
				for(nIndex=0; nIndex<m_nNbParameters; nIndex++)
					m_pWaferPoints[nXIndex][nYIndex].m_pnParameterPlots[nIndex]=-1;
			}
		}
	}

	m_pWaferZones = new  C_Gate_DataModel_WaferZone*[m_nNbZones+1];
	if(m_pWaferZones == NULL)
	{
		GSET_ERROR0(C_Gate_DataModel_WaferMap, eMalloc, NULL);
		return FALSE;
	}
	for(nXIndex=0; nXIndex<(m_nNbZones+1); nXIndex++)
	{
		m_pWaferZones[nXIndex] = new  C_Gate_DataModel_WaferZone[m_nNbSites+1];
		if(m_pWaferZones[nXIndex] == NULL)
		{
			GSET_ERROR0(C_Gate_DataModel_WaferMap, eMalloc, NULL);
			return FALSE;
		}
		for(nYIndex=0; nYIndex<(m_nNbSites+1); nYIndex++)
		{
			if(!m_pWaferZones[nXIndex][nYIndex].InitWaferZone(nXIndex,nYIndex,nMaxSoftBin,nMaxHardBin,m_nNbZones,m_nNbSites, m_nNbParameters))
				return FALSE;
		}
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Update variables for WaferMap results
//
// Argument(s) :
//
//	C_Gate_DataModel_Batch * pBatch
//		Parent
//
// Return type : bool
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_WaferMap::UpdateWaferPoints(C_Gate_DataModel_Batch* pBatch)
{
	unsigned int	uiRunIndex;
	int				nParameterIndex;
	int				nXIndex;
	int				nYIndex;

	int				nZoneIndex;
	int				nNormalValue;

	// Here, all stats are already computed
	// Initialize WaferMap
	if(m_pWaferPoints == NULL)
		return true;

	if((m_nXOffset == GEX_PLUGIN_INVALID_VALUE_INT) && (m_nYOffset == GEX_PLUGIN_INVALID_VALUE_INT))
		return true;

	C_Gate_DataModel_Site *pSite;
	//TODO: unused pParameterSet ?
	//C_Gate_DataModel_ParameterSet *pParameterSet = pBatch->m_clBatchID.m_pParameterSet;

	pSite = pBatch->GetFirstSite();
	while(pSite)
	{
		// For each runs
		for(uiRunIndex=0; uiRunIndex<pSite->m_uiNbRuns; uiRunIndex++)
		{
			// Update progress dialog
			if(pBatch->m_pProgress != NULL)
				pBatch->m_pProgress->Increment();

			if((pSite->m_pPartResults[uiRunIndex].m_uiFlags & YIELD123_PARTRESULT_EXECUTED)
			&& ((pSite->m_pPartResults[uiRunIndex].m_nXLoc != -32768) && (pSite->m_pPartResults[uiRunIndex].m_nYLoc != -32768))
			&&!(pSite->m_pPartResults[uiRunIndex].m_uiFlags & YIELD123_PARTRESULT_HASBEENRETESTED))
			{
				nXIndex = pSite->m_pPartResults[uiRunIndex].m_nXLoc - m_nXOffset;
				nYIndex = pSite->m_pPartResults[uiRunIndex].m_nYLoc - m_nYOffset;
				if((nXIndex<0)      ||(nYIndex<0)
				|| (nXIndex>m_nXMax)||(nYIndex>m_nYMax))
				{
					GSET_ERROR2(C_Gate_DataModel_WaferMap, eWaferMapIndexOverflow, NULL, nXIndex, nYIndex);
					return FALSE;
				}
				nZoneIndex = GiveZoneIndex(nXIndex, nYIndex);
				unsigned int uiSoftBin	= pSite->m_pPartResults[uiRunIndex].m_nSoftBin;
				unsigned int uiHardBin	= pSite->m_pPartResults[uiRunIndex].m_nHardBin;

				if((pBatch->m_pGoodBins_SoftBins.Find(uiSoftBin) != -1) && (pBatch->m_pGoodBins_HardBins.Find(uiHardBin) != -1))
					m_pWaferPoints[nXIndex][nYIndex].m_nPass = 1;
				else
					m_pWaferPoints[nXIndex][nYIndex].m_nPass = -1;

				m_pWaferPoints[nXIndex][nYIndex].m_nZoneIndex = nZoneIndex;
				m_pWaferPoints[nXIndex][nYIndex].m_nRunIndex = (int)uiRunIndex;
				m_pWaferPoints[nXIndex][nYIndex].m_nSiteIndex = (int)pSite->m_uiSiteIndex;
				m_pWaferPoints[nXIndex][nYIndex].m_nSoftBin = pSite->m_pPartResults[uiRunIndex].m_nSoftBin;
				m_pWaferPoints[nXIndex][nYIndex].m_nHardBin = pSite->m_pPartResults[uiRunIndex].m_nHardBin;

				for(nParameterIndex=0; nParameterIndex<m_nNbParameters; nParameterIndex++)
				{
					// Pass or Fail
					if(	(pSite->m_pTestResults[nParameterIndex*pSite->m_uiNbRuns + uiRunIndex].m_uiFlags & YIELD123_TESTRESULT_EXECUTED) &&
						!(pSite->m_pTestResults[nParameterIndex*pSite->m_uiNbRuns + uiRunIndex].m_uiFlags & (YIELD123_TESTRESULT_OUTLIER_L | YIELD123_TESTRESULT_OUTLIER_H)))
					{

						m_pWaferPoints[nXIndex][nYIndex].m_pfParameterValues[nParameterIndex] = pSite->m_pTestResults[nParameterIndex*pSite->m_uiNbRuns + uiRunIndex].m_fResult;
						nNormalValue = (int)((m_pWaferPoints[nXIndex][nYIndex].m_pfParameterValues[nParameterIndex]
												- pSite->m_pParameterStats_Distribution[nParameterIndex].m_fMin)
											/ pSite->m_pParameterStats_Distribution[nParameterIndex].m_fRange * 100);
						m_pWaferPoints[nXIndex][nYIndex].m_pnParameterPlots[nParameterIndex] = nNormalValue;

						if(pSite->m_pTestResults[nParameterIndex*pSite->m_uiNbRuns + uiRunIndex].m_uiFlags & YIELD123_TESTRESULT_PASSFAILSTATUS_VALID)
						{
							if((pSite->m_pTestResults[nParameterIndex*pSite->m_uiNbRuns + uiRunIndex].m_uiFlags & YIELD123_TESTRESULT_PASSFAILSTATUS_FAILED)
							|| (pSite->m_pTestResults[nParameterIndex*pSite->m_uiNbRuns + uiRunIndex].m_uiFlags & YIELD123_TESTRESULT_VALUE_FAILED_L)
							|| (pSite->m_pTestResults[nParameterIndex*pSite->m_uiNbRuns + uiRunIndex].m_uiFlags & YIELD123_TESTRESULT_VALUE_FAILED_H))
								m_pWaferPoints[nXIndex][nYIndex].m_pnParameterPass[nParameterIndex] = -1;
							else
								m_pWaferPoints[nXIndex][nYIndex].m_pnParameterPass[nParameterIndex] = 1;


						}
						else
							m_pWaferPoints[nXIndex][nYIndex].m_pnParameterPass[nParameterIndex] = 0;
					}
					else
						m_pWaferPoints[nXIndex][nYIndex].m_pnParameterPass[nParameterIndex] = 0;

				}
			}
		}

		pSite = pBatch->GetNextSite();
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Update variables for WaferMap results
//
// Argument(s) :
//
//	C_Gate_DataModel_Batch * pBatch
//		Parent
//
// Return type : bool
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_WaferMap::UpdateWaferZone(C_Gate_DataModel_Batch* pBatch, int nZoneIndex, int nSiteIndex)
{
	unsigned int	uiRunIndex;
	int				nParameterIndex;
	int				nXIndex;
	int				nYIndex;

	int				nZoneNb;

	// Here, all stats are already computed
	// Initialize WaferMap
	if(m_pWaferZones == NULL)
		return true;

	C_Gate_DataModel_Site				*pSite;
	C_Gate_DataModel_TestResult			*pTestResult;
	C_Gate_DataModel_TestResultVector	*pTestResultVector = NULL;;
	//TODO: unused pParameterSet ?
	//C_Gate_DataModel_ParameterSet		*pParameterSet = pBatch->m_clBatchID.m_pParameterSet;

	if(m_nNbParameters > 0)
		pTestResultVector = new C_Gate_DataModel_TestResultVector[pBatch->m_uiNbParameters];

	for(nParameterIndex=0; nParameterIndex<m_nNbParameters; nParameterIndex++)
	{
		// First clear the vector
		(pTestResultVector[nParameterIndex]).ClearValues();

		// Size of vector is nb of samples, including outliers, even if vector will be filled only with non-outlier data
		//(pTestResultVector[nParameterIndex]).Resize(pBatch->m_uiNbRuns);
	}


	pSite = pBatch->GetFirstSite();
	while(pSite)
	{
		if((nSiteIndex == -1) || (pSite->m_nSiteNb == nSiteIndex))
		{
			// For each runs
			for(uiRunIndex=0; uiRunIndex<pSite->m_uiNbRuns; uiRunIndex++)
			{
				// Update progress dialog
				if(pBatch->m_pProgress != NULL)
					pBatch->m_pProgress->Increment();

				if((pSite->m_pPartResults[uiRunIndex].m_uiFlags & YIELD123_PARTRESULT_EXECUTED)
				&& ((pSite->m_pPartResults[uiRunIndex].m_nXLoc != -32768) && (pSite->m_pPartResults[uiRunIndex].m_nYLoc != -32768))
				&&!(pSite->m_pPartResults[uiRunIndex].m_uiFlags & YIELD123_PARTRESULT_HASBEENRETESTED))
				{

					nXIndex = pSite->m_pPartResults[uiRunIndex].m_nXLoc - m_nXOffset;
					nYIndex = pSite->m_pPartResults[uiRunIndex].m_nYLoc - m_nYOffset;
					if((nXIndex<0)      ||(nYIndex<0)
					|| (nXIndex>m_nXMax)||(nYIndex>m_nYMax))
					{
						GSET_ERROR2(C_Gate_DataModel_WaferMap, eWaferMapIndexOverflow, NULL, nXIndex, nYIndex);
						return FALSE;
					}
					nZoneNb = GiveZoneIndex(nXIndex, nYIndex);

					if((nZoneIndex == 0) || (nZoneIndex == nZoneNb))
					{
						for(nParameterIndex=0; nParameterIndex<m_nNbParameters; nParameterIndex++)
						{
							pTestResult = pSite->m_pTestResults + (nParameterIndex*pSite->m_uiNbRuns + uiRunIndex);
							if((pTestResult->m_uiFlags & YIELD123_TESTRESULT_EXECUTED)
							&& !(pTestResult->m_uiFlags & (YIELD123_TESTRESULT_OUTLIER_L | YIELD123_TESTRESULT_OUTLIER_H)))
							{
								//delete (pTestResultVector[nParameterIndex])[(pTestResultVector[nParameterIndex]).size()-1];
								(pTestResultVector[nParameterIndex]).InsertValue(new C_Gate_DataModel_TestResult_Item(pTestResult, uiRunIndex, nParameterIndex));
							}

						}
						m_pWaferZones[nZoneIndex][nSiteIndex+1].UpdateData(pBatch, pSite->m_pPartResults[uiRunIndex]);
					}

				}
			}
		}

		pSite = pBatch->GetNextSite();
	}


	// update WaferZone
	// for the specific zone and a specific site
	m_pWaferZones[nZoneIndex][nSiteIndex+1].UpdateStats(pBatch, pTestResultVector);

	for(nParameterIndex=0; nParameterIndex<m_nNbParameters; nParameterIndex++)
	{
		// clear the vector
		(pTestResultVector[nParameterIndex]).ClearValues();
	}
	delete []pTestResultVector;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Update variables for Parameter stat in WaferMap
//
// Argument(s) :
//
//	C_Gate_DataModel_Batch * pBatch
//		Parent
//
// Return type : bool
/////////////////////////////////////////////////////////////////////////////////////////
bool	C_Gate_DataModel_WaferMap::UpdateParameterStat( C_Gate_DataModel_ParameterStat* pParameterStat , const C_Gate_DataModel_ParameterStat* pInitParameterStat)
{

	double	lfExecs, lfNum, lfDenom;


	pParameterStat->m_uiExecs += pInitParameterStat->m_uiExecs;
	pParameterStat->m_uiPass += pInitParameterStat->m_uiPass;
	pParameterStat->m_uiFail += pInitParameterStat->m_uiFail;

	lfExecs = pParameterStat->m_uiExecs;

	if((pParameterStat->m_fMin == GEX_PLUGIN_INVALID_VALUE_FLOAT)
	|| (pParameterStat->m_fMin > pInitParameterStat->m_fMin))
		pParameterStat->m_fMin = pInitParameterStat->m_fMin;

	if((pParameterStat->m_fMax == GEX_PLUGIN_INVALID_VALUE_FLOAT)
	|| (pParameterStat->m_fMax < pInitParameterStat->m_fMax))
		pParameterStat->m_fMax = pInitParameterStat->m_fMax;

	pParameterStat->m_fRange = pParameterStat->m_fMax - pParameterStat->m_fMin;

	if(pParameterStat->m_lfSumX == GEX_PLUGIN_INVALID_VALUE_FLOAT)
		pParameterStat->m_lfSumX = pInitParameterStat->m_lfSumX;
	else
		pParameterStat->m_lfSumX += pInitParameterStat->m_lfSumX;

	if(pParameterStat->m_lfSumX2 == GEX_PLUGIN_INVALID_VALUE_FLOAT)
		pParameterStat->m_lfSumX2 = pInitParameterStat->m_lfSumX2;
	else
		pParameterStat->m_lfSumX2 += pInitParameterStat->m_lfSumX2;

	pParameterStat->m_fMean = pParameterStat->m_lfSumX/lfExecs;

	// Compute Sigma
	if((pParameterStat->m_fRange == 0.0)
	|| (pParameterStat->m_uiExecs < 2))
		pParameterStat->m_fSigma = 0.0;
	else
	{
		lfNum = lfExecs*pParameterStat->m_lfSumX2 - pow(pParameterStat->m_lfSumX, 2.0);
		lfDenom = lfExecs*(lfExecs-1.0);
		pParameterStat->m_fSigma = sqrt(fabs(lfNum/lfDenom));
	}

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////////////////
// SITE OBJECT
/////////////////////////////////////////////////////////////////////////////////////////
// ERROR MAP
GBEGIN_ERROR_MAP(C_Gate_DataModel_Site)
	GMAP_ERROR(eMalloc,"Failed to allocate memory for site %d")
	GMAP_ERROR(eParameterIndexOverflow,"Parameter index overflow (index=%d, nb of parameters=%d)")
	GMAP_ERROR(eRunIndexOverflow,"Run index overflow (index=%d, nb of runs=%d)")
	GMAP_ERROR(eNoMemForResults,"No memory allocated to store test results")
GEND_ERROR_MAP(C_Gate_DataModel_Site)

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Constructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_Site::C_Gate_DataModel_Site(C_Gate_DataModel_Batch *pParent, C_Gate_DataModel_Progress *pProgress) : C_Gate_DataModel_Object(eData_Site, pParent,pProgress)
{
	m_pTestResults = NULL;
	m_pPartResults = NULL;
	m_pParameterStats_AllData = NULL;
	m_pParameterStats_Outliers = NULL;
	m_pParameterStats_Distribution = NULL;

	ClearData();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Destructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_Site::~C_Gate_DataModel_Site()
{
	ClearData();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Clear object data (free allocated ressources)
//
// Argument(s) :
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_Site::ClearData()
{
	m_clEquipmentID.ClearData();

	// Free allocated ressources
	if(m_pTestResults != NULL)
	{
		delete [] m_pTestResults;
		m_pTestResults = NULL;
	}
	if(m_pPartResults != NULL)
	{
		delete [] m_pPartResults;
		m_pPartResults = NULL;
	}
	if(m_pParameterStats_AllData != NULL)
	{
		delete [] m_pParameterStats_AllData;
		m_pParameterStats_AllData = NULL;
	}
	if(m_pParameterStats_Distribution != NULL)
	{
		delete [] m_pParameterStats_Distribution;
		m_pParameterStats_Distribution = NULL;
	}
	if(m_pParameterStats_Outliers != NULL)
	{
		delete [] m_pParameterStats_Outliers;
		m_pParameterStats_Outliers = NULL;
	}
//	m_clResultSet_Failures.ClearData();
//	m_clResultSet_Outliers.ClearData();
//	m_clResultSet_Distribution.ClearData();

	m_pSoftBins.clearItems();
	m_pHardBins.clearItems();

	// Reset variables
	m_uiSiteIndex = 0;
	m_nHeadNb = -1;
	m_nSiteNb = -1;
	m_uiNbParameters = 0;
	m_uiNbRuns = 0;
	m_uiNbParts = 0;
	m_uiNbPassParts = 0;
	m_uiNbFailParts = 0;
	m_uiNbRetestParts = 0;
	m_uiNbFailPartsAfterRetest = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Cleanup data (remove empty structures...)
//
// Argument(s) :
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_Site::Cleanup()
{
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Initialize variables and allocate memory for test results
//
// Argument(s) :
//
//	int nSiteNb
//		Site nb
//
//	const Gate_SiteDescription* pSiteDescription
//		Site descritpion object from plug-in
//
//	const Gate_LotDef & LotDefinition
//		Lot definition object
//
//	const C_Gate_DataModel_ParameterSet* pParameterSet
//		Ptr on perameter set to be used for this lot
//
// Return type : bool
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_Site::Init(unsigned int uiSiteIndex, int nSiteNb, const Gate_SiteDescription* pSiteDescription, const Gate_LotDef & LotDefinition, const C_Gate_DataModel_ParameterSet* pParameterSet)
{
	unsigned int uiIndex;

	// First clear data in case
	ClearData();

	// Set Site ids
	m_uiSiteIndex	= uiSiteIndex;
	m_nSiteNb		= nSiteNb;
	m_nHeadNb		= pSiteDescription->m_nHeadNum;

	// Set Site equipment ID object
	m_clEquipmentID.m_strTesterName			= LotDefinition.m_strTesterName;
	m_clEquipmentID.m_strTesterType			= LotDefinition.m_strTesterType;
	m_clEquipmentID.m_strDibBoardID			= pSiteDescription->m_strDibBoardID;
	m_clEquipmentID.m_strExtraEquipmentID	= pSiteDescription->m_strExtraEquipmentID;
	m_clEquipmentID.m_strHandlerContactorID = pSiteDescription->m_strHandlerContactorID;
	m_clEquipmentID.m_strHandlerProberID	= pSiteDescription->m_strHandlerProberID;
	m_clEquipmentID.m_strInterfaceCableID	= pSiteDescription->m_strInterfaceCableID;
	m_clEquipmentID.m_strLaserID			= pSiteDescription->m_strLaserID;
	m_clEquipmentID.m_strLoadBoardID		= pSiteDescription->m_strLoadBoardID;
	m_clEquipmentID.m_strProbeCardID		= pSiteDescription->m_strProbeCardID;

	// Allocate memory for test results
	if(pParameterSet->m_uiNbParameters > 0)
	{
		m_pTestResults = new C_Gate_DataModel_TestResult[pParameterSet->m_uiNbParameters*LotDefinition.m_uiProgramRuns];
		if(m_pTestResults == NULL)
		{
			GSET_ERROR1(C_Gate_DataModel_Site, eMalloc, NULL, nSiteNb);
			return FALSE;
		}
	}

	// Allocate memory for part results
	m_pPartResults = new C_Gate_DataModel_PartResult[LotDefinition.m_uiProgramRuns];
	if(m_pPartResults == NULL)
	{
		GSET_ERROR1(C_Gate_DataModel_Site, eMalloc, NULL, nSiteNb);
		return FALSE;
	}
	// Initialize run index (useful if using a vector of ptrs and sorting it, so that when we run
	// through the sorted vector, we know the associated run index for each element)
	for(uiIndex=0; uiIndex<LotDefinition.m_uiProgramRuns; uiIndex++)
		m_pPartResults[uiIndex].m_uiRunIndex = uiIndex;

	// Allocate memory for Parameter stats on all data
	if(pParameterSet->m_uiNbParameters > 0)
	{
		m_pParameterStats_AllData = new C_Gate_DataModel_ParameterStat[pParameterSet->m_uiNbParameters];
		if(m_pParameterStats_AllData == NULL)
		{
			GSET_ERROR1(C_Gate_DataModel_Site, eMalloc, NULL, nSiteNb);
			return FALSE;
		}
		// Initialize parameter index (useful if using a vector of ptrs and sorting it, so that when we run
		// through the sorted vector, we know the associated parameter index for each element)
		for(uiIndex=0; uiIndex<pParameterSet->m_uiNbParameters; uiIndex++)
		{
			m_pParameterStats_AllData[uiIndex].m_uiParameterIndex = uiIndex;
			m_pParameterStats_AllData[uiIndex].m_pclParameter = &pParameterSet->m_pclParameters[uiIndex];
		}
	}

	// Allocate memory for Parameter stats on non-outlier data
	if(pParameterSet->m_uiNbParameters > 0)
	{
		m_pParameterStats_Distribution = new C_Gate_DataModel_ParameterStat[pParameterSet->m_uiNbParameters];
		if(m_pParameterStats_Distribution == NULL)
		{
			GSET_ERROR1(C_Gate_DataModel_Site, eMalloc, NULL, nSiteNb);
			return FALSE;
		}
		// Initialize parameter index (useful if using a vector of ptrs and sorting it, so that when we run
		// through the sorted vector, we know the associated parameter index for each element)
		for(uiIndex=0; uiIndex<pParameterSet->m_uiNbParameters; uiIndex++)
		{
			m_pParameterStats_Distribution[uiIndex].m_uiParameterIndex = uiIndex;
			m_pParameterStats_Distribution[uiIndex].m_pclParameter = &pParameterSet->m_pclParameters[uiIndex];
		}
	}

	// Allocate memory for Parameter stats on outlier data
	if(pParameterSet->m_uiNbParameters > 0)
	{
		m_pParameterStats_Outliers = new C_Gate_DataModel_ParameterStat[pParameterSet->m_uiNbParameters];
		if(m_pParameterStats_Outliers == NULL)
		{
			GSET_ERROR1(C_Gate_DataModel_Site, eMalloc, NULL, nSiteNb);
			return FALSE;
		}
		// Initialize parameter index (useful if using a vector of ptrs and sorting it, so that when we run
		// through the sorted vector, we know the associated parameter index for each element)
		for(uiIndex=0; uiIndex<pParameterSet->m_uiNbParameters; uiIndex++)
		{
			m_pParameterStats_Outliers[uiIndex].m_uiParameterIndex = uiIndex;
			m_pParameterStats_Outliers[uiIndex].m_pclParameter = &pParameterSet->m_pclParameters[uiIndex];
		}
	}

	// Init variables
	m_uiNbParameters = pParameterSet->m_uiNbParameters;
	m_uiNbRuns = LotDefinition.m_uiProgramRuns;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Set a result for a specific parameter / run nb
//
// Argument(s) :
//
//	const Gate_DataResult & DataResult
//		Result structure
//
//	C_Gate_DataModel_ParameterSet* pParameterSet
//		Ptr to current parameter set
//
// Return type : bool
//		TRUE if result successfully set, FALSE else
/////////////////////////////////////////////////////////////////////////////////////////
// Note: if we try to set a result that was alread set for this particular test/run, the
//       first value is kept, and all subsequent one will be ignored.
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_Site::SetTestResult(const Gate_DataResult & DataResult, C_Gate_DataModel_ParameterSet* pParameterSet)
{
	// Is memeory allocated?
	if((m_pTestResults == NULL) || (m_pParameterStats_AllData == NULL))
	{
		GSET_ERROR0(C_Gate_DataModel_Site, eNoMemForResults, NULL);
		return FALSE;
	}

	//if(m_pProgress)
	//	m_pProgress->ProcessEvents();

	// Check for Parameter index overflow
	int iParameterIndex = DataResult.m_nParameterIndex;
	unsigned int uiRunIndex = DataResult.m_uiProgramRunIndex;
	if (iParameterIndex < 0 ||
		static_cast<unsigned int>(iParameterIndex) >= m_uiNbParameters) {
		GSET_ERROR2(C_Gate_DataModel_Site, eParameterIndexOverflow, NULL,
					iParameterIndex, m_uiNbParameters);
		return FALSE;
	}

	// Check for Run index overflow
	if (uiRunIndex >= m_uiNbRuns) {
		GSET_ERROR2(C_Gate_DataModel_Site, eRunIndexOverflow, NULL, uiRunIndex, m_uiNbRuns);
		return FALSE;
	}


	float	fResult = DataResult.m_lfValue;
	long	lIndex = iParameterIndex * m_uiNbRuns+uiRunIndex;

	// Check if we already had a value for this test/run
	if(m_pTestResults[lIndex].m_uiFlags & YIELD123_TESTRESULT_EXECUTED)
		return TRUE;

	// Set test results
	m_pTestResults[lIndex].m_uiFlags |= YIELD123_TESTRESULT_EXECUTED;
	m_pTestResults[lIndex].m_fResult = fResult;

	// Set pass/fail status
	if(DataResult.m_bPassFailStatusValid)
	{
		m_pTestResults[lIndex].m_uiFlags |= YIELD123_TESTRESULT_PASSFAILSTATUS_VALID;
		if(DataResult.m_bTestFailed)
			m_pTestResults[lIndex].m_uiFlags |= YIELD123_TESTRESULT_PASSFAILSTATUS_FAILED;
	}

	// Specific case for Binning Parameter Result
	// Check conditions
	if (pParameterSet->m_pclParameters[iParameterIndex].m_bType == '-') {
		// if the BinResult is a good bin or a bad bin
		Gate_PartResult clPartResult;

		clPartResult.m_nSoftBin = (int)fResult;
		clPartResult.m_nHardBin = (int)fResult;
		clPartResult.m_bPassFailStatusValid = FALSE; // force IsPartFail to check the bin value
		m_pTestResults[lIndex].m_uiFlags |= YIELD123_TESTRESULT_PASSFAILSTATUS_VALID;
		if(GetParent()->IsPartFail(clPartResult))
			m_pTestResults[lIndex].m_uiFlags |= YIELD123_TESTRESULT_PASSFAILSTATUS_FAILED;
	}

	// Check if value outside spec
	unsigned int uiParameterDefFlags =
			pParameterSet->m_pclParameters[iParameterIndex].m_uiFlags;
	float fLL = pParameterSet->m_pclParameters[iParameterIndex].m_fLL;
	float fHL = pParameterSet->m_pclParameters[iParameterIndex].m_fHL;



	if(uiParameterDefFlags & YIELD123_PARAMETERDEF_HASLL)
	{
		if(uiParameterDefFlags & YIELD123_PARAMETERDEF_LLNOTSTRICT)
		{
			if(fResult < fLL)
				m_pTestResults[lIndex].m_uiFlags |= YIELD123_TESTRESULT_VALUE_FAILED_L;
		}
		else
		{
			if(fResult <= fLL)
				m_pTestResults[lIndex].m_uiFlags |= YIELD123_TESTRESULT_VALUE_FAILED_L;
		}
	}
	if(uiParameterDefFlags & YIELD123_PARAMETERDEF_HASHL)
	{
		if(uiParameterDefFlags & YIELD123_PARAMETERDEF_HLNOTSTRICT)
		{
			if(fResult > fHL)
				m_pTestResults[lIndex].m_uiFlags |= YIELD123_TESTRESULT_VALUE_FAILED_H;
		}
		else
		{
			if(fResult >= fHL)
				m_pTestResults[lIndex].m_uiFlags |= YIELD123_TESTRESULT_VALUE_FAILED_H;
		}
	}

	// Set final pass/fail status: consider test fail if either the fail flag is valid and set, or value outside limits
	if((m_pTestResults[lIndex].m_uiFlags & YIELD123_TESTRESULT_PASSFAILSTATUS_VALID) && (m_pTestResults[lIndex].m_uiFlags & YIELD123_TESTRESULT_PASSFAILSTATUS_FAILED))
		m_pTestResults[lIndex].m_uiFlags |= YIELD123_TESTRESULT_FINALSTATUS_FAILED;
	if((m_pTestResults[lIndex].m_uiFlags & YIELD123_TESTRESULT_VALUE_FAILED_L) || (m_pTestResults[lIndex].m_uiFlags & YIELD123_TESTRESULT_VALUE_FAILED_H))
		m_pTestResults[lIndex].m_uiFlags |= YIELD123_TESTRESULT_FINALSTATUS_FAILED;

	// Update test execution counter for current run
	(m_pPartResults[uiRunIndex].m_uiTestsExecuted)++;

	/////////////////////////////////////////////////////////////////////////////////////
	// Update parameter stats (cumuls, execution count, fail count...)
	/////////////////////////////////////////////////////////////////////////////////////
	// Execution count
	(m_pParameterStats_AllData[iParameterIndex].m_uiExecs)++;
	// Pass/Fail counters
	if(m_pTestResults[lIndex].m_uiFlags & YIELD123_TESTRESULT_FINALSTATUS_FAILED)
	{
		(m_pParameterStats_AllData[iParameterIndex].m_uiFail)++;
		if(m_pTestResults[lIndex].m_uiFlags & YIELD123_TESTRESULT_VALUE_FAILED_H)
			(m_pParameterStats_AllData[iParameterIndex].m_uiFail_H)++;
		else if(m_pTestResults[lIndex].m_uiFlags & YIELD123_TESTRESULT_VALUE_FAILED_L)
			(m_pParameterStats_AllData[iParameterIndex].m_uiFail_L)++;
	}
	else
	{
		(m_pParameterStats_AllData[iParameterIndex].m_uiPass)++;
	}
	// Cumuls, min, max...
	m_pParameterStats_AllData[iParameterIndex].m_lfSumX += fResult;
	m_pParameterStats_AllData[iParameterIndex].m_lfSumX2 += pow(fResult, 2.0F);
	if(m_pParameterStats_AllData[iParameterIndex].m_fMin == GEX_PLUGIN_INVALID_VALUE_FLOAT)
		m_pParameterStats_AllData[iParameterIndex].m_fMin = fResult;
	else
		m_pParameterStats_AllData[iParameterIndex].m_fMin = gex_min(m_pParameterStats_AllData[iParameterIndex].m_fMin, fResult);
	if(m_pParameterStats_AllData[iParameterIndex].m_fMax == GEX_PLUGIN_INVALID_VALUE_FLOAT)
		m_pParameterStats_AllData[iParameterIndex].m_fMax = fResult;
	else
		m_pParameterStats_AllData[iParameterIndex].m_fMax = gex_max(m_pParameterStats_AllData[iParameterIndex].m_fMax, fResult);
	m_pParameterStats_AllData[iParameterIndex].m_fRange =
			m_pParameterStats_AllData[iParameterIndex].m_fMax -
			m_pParameterStats_AllData[iParameterIndex].m_fMin;
	// Check if test result is an integer value
	if((float)((int)fResult) != fResult)
		m_pParameterStats_AllData[iParameterIndex].m_uiFlags |= YIELD123_PARAMETERSTATS_HASFRESULTS;

	if(DataResult.m_bValidValue)
		m_pParameterStats_AllData[iParameterIndex].m_uiFlags |= YIELD123_PARAMETERSTATS_HASVALIDRESULTS;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Set a result for a specific part / run nb
//
// Argument(s) :
//
//	const Gate_PartResult & PartResult
//		Result values
//
//	bool bPartIsFail
//		TRUE if Part is FAIL, FALSE else
//
// Return type : bool
//		TRUE if result successfully set, FALSE else
/////////////////////////////////////////////////////////////////////////////////////////
// Note: if we try to set a result that was alread set for this particular test/run, the
//       first value is kept, and all subsequent one will be ignored.
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_Site::SetPartResult(const Gate_PartResult & PartResult, bool bPartIsFail, bool bPartIsRetest)
{
	// Is memory allocated?
	if(m_pPartResults == NULL)
	{
		GSET_ERROR0(C_Gate_DataModel_Site, eNoMemForResults, NULL);
		return FALSE;
	}

	// Check for Run index overflow
	unsigned int uiRunIndex = PartResult.m_uiProgramRunIndex;
	if (uiRunIndex >= m_uiNbRuns) {
		GSET_ERROR2(C_Gate_DataModel_Site, eRunIndexOverflow, NULL, uiRunIndex, m_uiNbRuns);
		return FALSE;
	}

	// Check if we already had a value for this run
	if(m_pPartResults[uiRunIndex].m_uiFlags & YIELD123_PARTRESULT_EXECUTED)
		return TRUE;

	// Set part result
	m_pPartResults[uiRunIndex].m_uiFlags |= YIELD123_PARTRESULT_EXECUTED;
	m_pPartResults[uiRunIndex].m_nHardBin = PartResult.m_nHardBin;
	m_pPartResults[uiRunIndex].m_nSoftBin = PartResult.m_nSoftBin;
	m_pPartResults[uiRunIndex].m_nXLoc = PartResult.m_nPart_X;
	m_pPartResults[uiRunIndex].m_nYLoc = PartResult.m_nPart_Y;
	m_pPartResults[uiRunIndex].m_strPartID = PartResult.m_strPartID;
	if(bPartIsFail)
		m_pPartResults[uiRunIndex].m_uiFlags |= YIELD123_PARTRESULT_FAIL;
	if(bPartIsRetest)
		m_pPartResults[uiRunIndex].m_uiFlags |= YIELD123_PARTRESULT_RETEST;

	if((m_pPartResults[uiRunIndex].m_uiFlags & YIELD123_PARTRESULT_EXECUTED)
	&& ((m_pPartResults[uiRunIndex].m_nXLoc != -32768) && (m_pPartResults[uiRunIndex].m_nYLoc != -32768)))
		GetParent()->m_bWithWaferInfo = true;

	// Update binnings
	m_pSoftBins.AddBinning(PartResult.m_nSoftBin, !bPartIsFail, bPartIsRetest);
	m_pHardBins.AddBinning(PartResult.m_nHardBin, !bPartIsFail, bPartIsRetest);

	// Update site part counters
	m_uiNbParts++;
	if(bPartIsRetest == TRUE)
	{
		m_uiNbRetestParts++;
		if(bPartIsFail == FALSE)
			m_uiNbFailPartsAfterRetest++;
	}
	if(bPartIsFail == FALSE)
		m_uiNbPassParts++;
	else
		m_uiNbFailParts++;

	m_clPartVector.InsertValue(m_pPartResults + uiRunIndex);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Set a bin info
//
// Argument(s) :
//
//	const Gate_BinningDef & BinDef
//		Result structure from plug-in
//
// Return type : TRUE if binning correctly set, FALSE else
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_Site::SetBinning(const Gate_BinningDef & BinDef)
{
	// Increment ProgressBar
	if(m_pProgress)
		m_pProgress->Increment();

	// Update binnings
	if(BinDef.m_bBinType == 'H')
		m_pHardBins.InitBinning(BinDef.m_nBinNumber, BinDef.m_strBinName, BinDef.m_bBinCat);
	else
		m_pSoftBins.InitBinning(BinDef.m_nBinNumber, BinDef.m_strBinName, BinDef.m_bBinCat);

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////////////////
// Description : Set a result for a specific part / run nb
//
// Argument(s) :
//
//	const Gate_PartResult & PartResult
//		Result values
//
//	bool bPartIsFail
//		TRUE if Part is FAIL, FALSE else
//
// Return type : bool
//		TRUE if result successfully set, FALSE else
/////////////////////////////////////////////////////////////////////////////////////////
// Note: if we try to set a result that was alread set for this particular test/run, the
//       first value is kept, and all subsequent one will be ignored.
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_ParameterStatVector* C_Gate_DataModel_Site::GetParameterStatVector(C_Gate_DataModel_ParameterStatVector::SortOn eSortSelector, bool bAscending)
{
	if(m_clParameterStatVector.CountValues() == 0)
		m_clParameterStatVector.Fill(this, eParameter_Any);
	m_clParameterStatVector.Sort(eSortSelector,bAscending);

	return &m_clParameterStatVector;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Set a result for a specific part / run nb
//
// Argument(s) :
//
//	const C_Gate_DataModel_BinningVector & Binning
//		Result values
//
// Return type : bool
//		TRUE if result successfully set, FALSE else
/////////////////////////////////////////////////////////////////////////////////////////
// Note: if we try to set a result that was alread set for this particular test/run, the
//       first value is kept, and all subsequent one will be ignored.
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_BinningVector* C_Gate_DataModel_Site::GetHardBinningVector(C_Gate_DataModel_BinningVector::SortOn eSortSelector, bool bAscending)
{
	if(m_clHardBinningVector.CountValues() == 0)
		m_clHardBinningVector.Fill(this, eParameter_WithResults,true);
	m_clHardBinningVector.Sort(eSortSelector,bAscending);

	return &m_clHardBinningVector;
}

C_Gate_DataModel_BinningVector* C_Gate_DataModel_Site::GetSoftBinningVector(C_Gate_DataModel_BinningVector::SortOn eSortSelector, bool bAscending)
{
	if(m_clSoftBinningVector.CountValues() == 0)
		m_clSoftBinningVector.Fill(this, eParameter_WithResults,false);
	m_clSoftBinningVector.Sort(eSortSelector,bAscending);

	return &m_clSoftBinningVector;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Dumps the data structure to an ascii file (CSV format)
//
// Argument(s) :
//
//	const QTextStream & hDumpFile
//		Dump file stream
//
//	C_Gate_DataModel_ParameterSet* pParameterSet
//		Ptr on parameter set
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_Site::Dump(QTextStream & hDumpFile, C_Gate_DataModel_ParameterSet* pParameterSet)
{
	// Update ProgressBar
	if(m_pProgress)
		m_pProgress->Increment();

  //unsigned int uiIndex_X;
  unsigned int uiIndex_Y;
  //long lIndex;
	C_Gate_DataModel_Binning	*pBinning;

  //uiIndex_X=
  uiIndex_Y = 0;
  //lIndex = 0.0;


	// Write header
	hDumpFile << endl;
	hDumpFile << "SITE" << endl;
	hDumpFile << "====" << endl;
	hDumpFile << "Site Index" << CSV_SEPARATOR << QString::number(m_uiSiteIndex) << endl;
	hDumpFile << "Head Num" << CSV_SEPARATOR << QString::number(m_nHeadNb) << endl;
	hDumpFile << "Site Num" << CSV_SEPARATOR << QString::number(m_nSiteNb) << endl;
	m_clEquipmentID.Dump(hDumpFile);
	hDumpFile << "Nb Parameters" << CSV_SEPARATOR << QString::number(m_uiNbParameters) << endl;
	hDumpFile << "Nb Runs" << CSV_SEPARATOR << QString::number(m_uiNbRuns) << endl;
	hDumpFile << "Nb Parts" << CSV_SEPARATOR << QString::number(m_uiNbParts) << endl;
	hDumpFile << "Nb PASS Parts" << CSV_SEPARATOR << QString::number(m_uiNbPassParts) << endl;
	hDumpFile << "Nb FAIL Parts" << CSV_SEPARATOR << QString::number(m_uiNbFailParts) << endl;
	hDumpFile << "Nb RETEST Parts" << CSV_SEPARATOR << QString::number(m_uiNbRetestParts) << endl;
	hDumpFile << "Nb FAIL Parts after RETEST" << CSV_SEPARATOR << QString::number(m_uiNbFailPartsAfterRetest) << endl;
	hDumpFile << endl;

	// Write Rule results for failures analysis
//	if(m_clResultSet_Failures.m_pRuleResults != NULL)
//	{
//		hDumpFile << endl;
//		hDumpFile << "ResultSet for Failures analysis**********" << endl;
//		if(pParameterSet == NULL)
//		{
//			hDumpFile << "No parameters defined" << endl;
//		}
//		else
//			m_clResultSet_Failures.Dump(hDumpFile);
//	}

	// Write Rule results for outliers analysis
//	if(m_clResultSet_Outliers.m_pRuleResults != NULL)
//	{
//		hDumpFile << endl;
//		hDumpFile << "ResultSet for Outliers analysis**********" << endl;
//		if(pParameterSet == NULL)
//		{
//			hDumpFile << "No parameters defined" << endl;
//		}
//		else
//			m_clResultSet_Outliers.Dump(hDumpFile);
//	}

	// Write Rule results for outliers analysis
//	if(m_clResultSet_Distribution.m_pRuleResults != NULL)
//	{
//		hDumpFile << endl;
//		hDumpFile << "ResultSet for Distribution analysis**********" << endl;
//		if(pParameterSet == NULL)
//		{
//			hDumpFile << "No parameters defined" << endl;
//		}
//		else
//		 m_clResultSet_Distribution.Dump(hDumpFile);
//	}


#if 0
	// Write results
	hDumpFile << endl;
	hDumpFile << "Results**********" << endl;
	if(pParameterSet == NULL)
	{
		hDumpFile << "No parameters defined" << endl;
	}
	else
	{
		// Column titls
		hDumpFile << "Parameter" << CSV_SEPARATOR << "PARTID" << CSV_SEPARATOR << "TESTS_EXECUTED" << CSV_SEPARATOR << "SBIN" << CSV_SEPARATOR << "HBIN" << CSV_SEPARATOR << "DIE_X" << CSV_SEPARATOR << "DIE_Y";
		for(uiIndex_Y = 0; uiIndex_Y < pParameterSet->m_uiNbParameters; uiIndex_Y++)
			hDumpFile << CSV_SEPARATOR << GetCsvString(pParameterSet->m_pclParameters[uiIndex_Y].m_strName);
		hDumpFile << endl;
		// Test nb
		hDumpFile << "Test#" << CSV_SEPARATOR << "" << CSV_SEPARATOR << "" << CSV_SEPARATOR << "" << CSV_SEPARATOR << "" << CSV_SEPARATOR << "" << CSV_SEPARATOR << "";
		for(uiIndex_Y = 0; uiIndex_Y < pParameterSet->m_uiNbParameters; uiIndex_Y++)
			hDumpFile << CSV_SEPARATOR << QString::number(pParameterSet->m_pclParameters[uiIndex_Y].m_uiNumber);
		hDumpFile << endl;
		// Test units
		hDumpFile << "Unit" << CSV_SEPARATOR << "" << CSV_SEPARATOR << "" << CSV_SEPARATOR << "" << CSV_SEPARATOR << "" << CSV_SEPARATOR << "" << CSV_SEPARATOR << "";
		for(uiIndex_Y = 0; uiIndex_Y < pParameterSet->m_uiNbParameters; uiIndex_Y++)
			hDumpFile << CSV_SEPARATOR << pParameterSet->m_pclParameters[uiIndex_Y].m_strUnits;
		hDumpFile << endl;
		// Test limits
		hDumpFile << "HighL" << CSV_SEPARATOR << "" << CSV_SEPARATOR << "" << CSV_SEPARATOR << "" << CSV_SEPARATOR << "" << CSV_SEPARATOR << "" << CSV_SEPARATOR << "";
		for(uiIndex_Y = 0; uiIndex_Y < pParameterSet->m_uiNbParameters; uiIndex_Y++)
		{
			if(pParameterSet->m_pclParameters[uiIndex_Y].m_uiLimitFlags & YIELD123_PARAMETERDEF_HASHL)
				hDumpFile << CSV_SEPARATOR << QString::number(pParameterSet->m_pclParameters[uiIndex_Y].m_fHL);
			else
				hDumpFile << CSV_SEPARATOR << "";
		}
		hDumpFile << endl;
		hDumpFile << "LowL" << CSV_SEPARATOR << "" << CSV_SEPARATOR << "" << CSV_SEPARATOR << "" << CSV_SEPARATOR << "" << CSV_SEPARATOR << "" << CSV_SEPARATOR << "";
		for(uiIndex_Y = 0; uiIndex_Y < pParameterSet->m_uiNbParameters; uiIndex_Y++)
		{
			if(pParameterSet->m_pclParameters[uiIndex_Y].m_uiLimitFlags & YIELD123_PARAMETERDEF_HASLL)
				hDumpFile << CSV_SEPARATOR << QString::number(pParameterSet->m_pclParameters[uiIndex_Y].m_fLL);
			else
				hDumpFile << CSV_SEPARATOR << "";
		}
		hDumpFile << endl;
		// Test results
		for(uiIndex_X = 0; uiIndex_X < m_uiNbRuns; uiIndex_X++)
		{
			if(m_pPartResults[uiIndex_X].m_uiTestsExecuted == 0)
				continue;
			hDumpFile << "PID-" << QString::number(uiIndex_X+1) << CSV_SEPARATOR << m_pPartResults[uiIndex_X].m_strPartID << CSV_SEPARATOR << QString::number(m_pPartResults[uiIndex_X].m_uiTestsExecuted) << CSV_SEPARATOR << QString::number(m_pPartResults[uiIndex_X].m_nSoftBin) << CSV_SEPARATOR << QString::number(m_pPartResults[uiIndex_X].m_nHardBin) << CSV_SEPARATOR << QString::number(m_pPartResults[uiIndex_X].m_nXLoc) << CSV_SEPARATOR << QString::number(m_pPartResults[uiIndex_X].m_nYLoc);
			for(uiIndex_Y = 0; uiIndex_Y < m_uiNbParameters; uiIndex_Y++)
			{
				lIndex = uiIndex_Y*m_uiNbRuns+uiIndex_X;
				if(m_pTestResults[lIndex].m_uiFlags & YIELD123_TESTRESULT_EXECUTED)
					hDumpFile << CSV_SEPARATOR << QString::number(m_pTestResults[lIndex].m_fResult);
				else
					hDumpFile << CSV_SEPARATOR << "";
			}
			hDumpFile << endl;
		}
	}

#endif

	// Write statistics
	hDumpFile << endl;
	hDumpFile << "Statistics_AllData**********" << endl;
	if(pParameterSet == NULL)
	{
		hDumpFile << "No parameters defined" << endl;
	}
	else
	{
		// Column titles
		hDumpFile << "Parameter" << CSV_SEPARATOR;
		hDumpFile << "Test#" << CSV_SEPARATOR;
		hDumpFile << "Flags" << CSV_SEPARATOR;
		hDumpFile << "Outliers (H)" << CSV_SEPARATOR;
		hDumpFile << "Outliers (L)" << CSV_SEPARATOR;
		hDumpFile << "Outliers on good parts (H)" << CSV_SEPARATOR;
		hDumpFile << "Outliers on good parts (L)" << CSV_SEPARATOR;
		hDumpFile << "Outliers on latest retest (H)" << CSV_SEPARATOR;
		hDumpFile << "Outliers on latest retest (L)" << CSV_SEPARATOR;
		hDumpFile << "Execs" << CSV_SEPARATOR;
		hDumpFile << "Pass" << CSV_SEPARATOR;
		hDumpFile << "Fail" << CSV_SEPARATOR;
		hDumpFile << "Fail_L" << CSV_SEPARATOR;
		hDumpFile << "Fail_H" << CSV_SEPARATOR;
		hDumpFile << "Mean" << CSV_SEPARATOR;
		hDumpFile << "Min" << CSV_SEPARATOR;
		hDumpFile << "Max" << CSV_SEPARATOR;
		hDumpFile << "Range" << CSV_SEPARATOR;
		hDumpFile << "Sigma" << CSV_SEPARATOR;
		hDumpFile << "CpkL" << CSV_SEPARATOR;
		hDumpFile << "CpkH" << CSV_SEPARATOR;
		hDumpFile << "Cpk" << CSV_SEPARATOR;
		hDumpFile << "Cp" << CSV_SEPARATOR;
		hDumpFile << "Q1" << CSV_SEPARATOR;
		hDumpFile << "Q2" << CSV_SEPARATOR;
		hDumpFile << "Q3" << CSV_SEPARATOR;
		hDumpFile << "IQR" << CSV_SEPARATOR;
		hDumpFile << "IQS" << CSV_SEPARATOR;
		hDumpFile << "Robust Mean" << CSV_SEPARATOR;
		hDumpFile << "Robust Sigma" << CSV_SEPARATOR;
		hDumpFile << endl;
		for(uiIndex_Y = 0; uiIndex_Y < pParameterSet->m_uiNbParameters; uiIndex_Y++)
		{
			// Update ProgressBar
			if(m_pProgress)
				m_pProgress->Increment();

			hDumpFile << GetCsvString(pParameterSet->m_pclParameters[uiIndex_Y].m_strName) << CSV_SEPARATOR;
			hDumpFile << QString::number(pParameterSet->m_pclParameters[uiIndex_Y].m_uiNumber) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_AllData[uiIndex_Y].m_uiFlags) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_AllData[uiIndex_Y].m_uiOutliers_H) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_AllData[uiIndex_Y].m_uiOutliers_L) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_AllData[uiIndex_Y].m_uiOutliers_H_GoodPart) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_AllData[uiIndex_Y].m_uiOutliers_L_GoodPart) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_AllData[uiIndex_Y].m_uiOutliers_H_LastRetest) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_AllData[uiIndex_Y].m_uiOutliers_L_LastRetest) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_AllData[uiIndex_Y].m_uiExecs) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_AllData[uiIndex_Y].m_uiPass) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_AllData[uiIndex_Y].m_uiFail) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_AllData[uiIndex_Y].m_uiFail_L) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_AllData[uiIndex_Y].m_uiFail_H) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_AllData[uiIndex_Y].m_fMean) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_AllData[uiIndex_Y].m_fMin) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_AllData[uiIndex_Y].m_fMax) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_AllData[uiIndex_Y].m_fRange) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_AllData[uiIndex_Y].m_fSigma) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_AllData[uiIndex_Y].m_fCpkL) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_AllData[uiIndex_Y].m_fCpkH) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_AllData[uiIndex_Y].m_fCpk) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_AllData[uiIndex_Y].m_fCp) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_AllData[uiIndex_Y].m_fQ1) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_AllData[uiIndex_Y].m_fQ2) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_AllData[uiIndex_Y].m_fQ3) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_AllData[uiIndex_Y].m_fIQR) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_AllData[uiIndex_Y].m_fIQS) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_AllData[uiIndex_Y].m_fRobustMean) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_AllData[uiIndex_Y].m_fRobustSigma) << CSV_SEPARATOR;
			hDumpFile << endl;
		}
	}
	hDumpFile << endl;
	hDumpFile << "Statistics_Outliers**********" << endl;
	if(pParameterSet == NULL)
	{
		hDumpFile << "No parameters defined" << endl;
	}
	else
	{
		// Column titles
		hDumpFile << "Parameter" << CSV_SEPARATOR;
		hDumpFile << "Test#" << CSV_SEPARATOR;
		hDumpFile << "Flags" << CSV_SEPARATOR;
		hDumpFile << "Outliers (H)" << CSV_SEPARATOR;
		hDumpFile << "Outliers (L)" << CSV_SEPARATOR;
		hDumpFile << "Outliers on good parts (H)" << CSV_SEPARATOR;
		hDumpFile << "Outliers on good parts (L)" << CSV_SEPARATOR;
		hDumpFile << "Outliers on latest retest (H)" << CSV_SEPARATOR;
		hDumpFile << "Outliers on latest retest (L)" << CSV_SEPARATOR;
		hDumpFile << "Execs" << CSV_SEPARATOR;
		hDumpFile << "Pass" << CSV_SEPARATOR;
		hDumpFile << "Fail" << CSV_SEPARATOR;
		hDumpFile << "Fail_L" << CSV_SEPARATOR;
		hDumpFile << "Fail_H" << CSV_SEPARATOR;
		hDumpFile << "Mean" << CSV_SEPARATOR;
		hDumpFile << "Min" << CSV_SEPARATOR;
		hDumpFile << "Max" << CSV_SEPARATOR;
		hDumpFile << "Range" << CSV_SEPARATOR;
		hDumpFile << "Sigma" << CSV_SEPARATOR;
		hDumpFile << "CpkL" << CSV_SEPARATOR;
		hDumpFile << "CpkH" << CSV_SEPARATOR;
		hDumpFile << "Cpk" << CSV_SEPARATOR;
		hDumpFile << "Cp" << CSV_SEPARATOR;
		hDumpFile << "Q1" << CSV_SEPARATOR;
		hDumpFile << "Q2" << CSV_SEPARATOR;
		hDumpFile << "Q3" << CSV_SEPARATOR;
		hDumpFile << "IQR" << CSV_SEPARATOR;
		hDumpFile << "IQS" << CSV_SEPARATOR;
		hDumpFile << "Robust Mean" << CSV_SEPARATOR;
		hDumpFile << "Robust Sigma" << CSV_SEPARATOR;
		hDumpFile << endl;
		for(uiIndex_Y = 0; uiIndex_Y < pParameterSet->m_uiNbParameters; uiIndex_Y++)
		{
			// Update ProgressBar
			if(m_pProgress)
				m_pProgress->Increment();

			hDumpFile << GetCsvString(pParameterSet->m_pclParameters[uiIndex_Y].m_strName) << CSV_SEPARATOR;
			hDumpFile << QString::number(pParameterSet->m_pclParameters[uiIndex_Y].m_uiNumber) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Outliers[uiIndex_Y].m_uiFlags) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Outliers[uiIndex_Y].m_uiOutliers_H) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Outliers[uiIndex_Y].m_uiOutliers_L) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Outliers[uiIndex_Y].m_uiOutliers_H_GoodPart) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Outliers[uiIndex_Y].m_uiOutliers_L_GoodPart) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Outliers[uiIndex_Y].m_uiOutliers_H_LastRetest) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Outliers[uiIndex_Y].m_uiOutliers_L_LastRetest) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Outliers[uiIndex_Y].m_uiExecs) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Outliers[uiIndex_Y].m_uiPass) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Outliers[uiIndex_Y].m_uiFail) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Outliers[uiIndex_Y].m_uiFail_L) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Outliers[uiIndex_Y].m_uiFail_H) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Outliers[uiIndex_Y].m_fMean) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Outliers[uiIndex_Y].m_fMin) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Outliers[uiIndex_Y].m_fMax) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Outliers[uiIndex_Y].m_fRange) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Outliers[uiIndex_Y].m_fSigma) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Outliers[uiIndex_Y].m_fCpkL) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Outliers[uiIndex_Y].m_fCpkH) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Outliers[uiIndex_Y].m_fCpk) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Outliers[uiIndex_Y].m_fCp) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Outliers[uiIndex_Y].m_fQ1) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Outliers[uiIndex_Y].m_fQ2) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Outliers[uiIndex_Y].m_fQ3) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Outliers[uiIndex_Y].m_fIQR) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Outliers[uiIndex_Y].m_fIQS) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Outliers[uiIndex_Y].m_fRobustMean) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Outliers[uiIndex_Y].m_fRobustSigma) << CSV_SEPARATOR;
			hDumpFile << endl;
		}
	}
	hDumpFile << endl;
	hDumpFile << "Statistics_Distribution**********" << endl;
	if(pParameterSet == NULL)
	{
		hDumpFile << "No parameters defined" << endl;
	}
	else
	{
		// Column titles
		hDumpFile << "Parameter" << CSV_SEPARATOR;
		hDumpFile << "Test#" << CSV_SEPARATOR;
		hDumpFile << "Flags" << CSV_SEPARATOR;
		hDumpFile << "Outliers (H)" << CSV_SEPARATOR;
		hDumpFile << "Outliers (L)" << CSV_SEPARATOR;
		hDumpFile << "Outliers on good parts (H)" << CSV_SEPARATOR;
		hDumpFile << "Outliers on good parts (L)" << CSV_SEPARATOR;
		hDumpFile << "Outliers on latest retest (H)" << CSV_SEPARATOR;
		hDumpFile << "Outliers on latest retest (L)" << CSV_SEPARATOR;
		hDumpFile << "Execs" << CSV_SEPARATOR;
		hDumpFile << "Pass" << CSV_SEPARATOR;
		hDumpFile << "Fail" << CSV_SEPARATOR;
		hDumpFile << "Fail_L" << CSV_SEPARATOR;
		hDumpFile << "Fail_H" << CSV_SEPARATOR;
		hDumpFile << "Mean" << CSV_SEPARATOR;
		hDumpFile << "Min" << CSV_SEPARATOR;
		hDumpFile << "Max" << CSV_SEPARATOR;
		hDumpFile << "Range" << CSV_SEPARATOR;
		hDumpFile << "Sigma" << CSV_SEPARATOR;
		hDumpFile << "CpkL" << CSV_SEPARATOR;
		hDumpFile << "CpkH" << CSV_SEPARATOR;
		hDumpFile << "Cpk" << CSV_SEPARATOR;
		hDumpFile << "Cp" << CSV_SEPARATOR;
		hDumpFile << "Q1" << CSV_SEPARATOR;
		hDumpFile << "Q2" << CSV_SEPARATOR;
		hDumpFile << "Q3" << CSV_SEPARATOR;
		hDumpFile << "IQR" << CSV_SEPARATOR;
		hDumpFile << "IQS" << CSV_SEPARATOR;
		hDumpFile << "Robust Mean" << CSV_SEPARATOR;
		hDumpFile << "Robust Sigma" << CSV_SEPARATOR;
		hDumpFile << endl;
		for(uiIndex_Y = 0; uiIndex_Y < pParameterSet->m_uiNbParameters; uiIndex_Y++)
		{
			// Update ProgressBar
			if(m_pProgress)
				m_pProgress->Increment();

			hDumpFile << GetCsvString(pParameterSet->m_pclParameters[uiIndex_Y].m_strName) << CSV_SEPARATOR;
			hDumpFile << QString::number(pParameterSet->m_pclParameters[uiIndex_Y].m_uiNumber) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Distribution[uiIndex_Y].m_uiFlags) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Distribution[uiIndex_Y].m_uiOutliers_H) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Distribution[uiIndex_Y].m_uiOutliers_L) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Distribution[uiIndex_Y].m_uiOutliers_H_GoodPart) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Distribution[uiIndex_Y].m_uiOutliers_L_GoodPart) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Distribution[uiIndex_Y].m_uiOutliers_H_LastRetest) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Distribution[uiIndex_Y].m_uiOutliers_L_LastRetest) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Distribution[uiIndex_Y].m_uiExecs) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Distribution[uiIndex_Y].m_uiPass) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Distribution[uiIndex_Y].m_uiFail) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Distribution[uiIndex_Y].m_uiFail_L) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Distribution[uiIndex_Y].m_uiFail_H) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Distribution[uiIndex_Y].m_fMean) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Distribution[uiIndex_Y].m_fMin) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Distribution[uiIndex_Y].m_fMax) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Distribution[uiIndex_Y].m_fRange) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Distribution[uiIndex_Y].m_fSigma) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Distribution[uiIndex_Y].m_fCpkL) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Distribution[uiIndex_Y].m_fCpkH) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Distribution[uiIndex_Y].m_fCpk) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Distribution[uiIndex_Y].m_fCp) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Distribution[uiIndex_Y].m_fQ1) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Distribution[uiIndex_Y].m_fQ2) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Distribution[uiIndex_Y].m_fQ3) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Distribution[uiIndex_Y].m_fIQR) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Distribution[uiIndex_Y].m_fIQS) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Distribution[uiIndex_Y].m_fRobustMean) << CSV_SEPARATOR;
			hDumpFile << QString::number(m_pParameterStats_Distribution[uiIndex_Y].m_fRobustSigma) << CSV_SEPARATOR;
			hDumpFile << endl;
		}
	}
	hDumpFile << endl;

	// Write Binnings
	m_pSoftBins.Sort(C_Gate_DataModel_BinningList::eSortOnBinCount, FALSE);	// Sort on decreasing bin count
	m_pHardBins.Sort(C_Gate_DataModel_BinningList::eSortOnBinCount, FALSE);	// Sort on decreasing bin count
	hDumpFile << endl;
	hDumpFile << "Binnings**********" << endl;
	hDumpFile << "Soft Bin" << CSV_SEPARATOR;
	hDumpFile << "Count" << CSV_SEPARATOR;
	hDumpFile << "Retest Count" << CSV_SEPARATOR;
	hDumpFile << "Hard Bin" << CSV_SEPARATOR;
	hDumpFile << "Count" << CSV_SEPARATOR;
	hDumpFile << "Retest Count" << CSV_SEPARATOR;
	hDumpFile << endl;
	for (pBinning = m_pSoftBins.GetFirst(); pBinning; pBinning = m_pSoftBins.GetNext())
	{
		hDumpFile << pBinning->m_uiBinNb << CSV_SEPARATOR;
		hDumpFile << pBinning->m_uiBinCount << CSV_SEPARATOR;
		hDumpFile << pBinning->m_uiBinCount_Retest << CSV_SEPARATOR;
		hDumpFile << pBinning->m_uiBinNb << CSV_SEPARATOR;
		hDumpFile << pBinning->m_uiBinCount << CSV_SEPARATOR;
		hDumpFile << pBinning->m_uiBinCount_Retest << CSV_SEPARATOR;
		hDumpFile << endl;
	}
	hDumpFile << endl;
}

/////////////////////////////////////////////////////////////////////////////////////////
// EQUIPMENTID OBJECT
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// Description : Constructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_EquipmentID::C_Gate_DataModel_EquipmentID()
{
	ClearData();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Destructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_EquipmentID::~C_Gate_DataModel_EquipmentID()
{
	ClearData();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Comparison operator
//
// Argument(s) :
//
//	const C_Gate_DataModel_EquipmentID &s
//		reference to object to compare with this one
//
// Return type : TRUE if the 2 objects are identical, FALSE else
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_EquipmentID::operator==(const C_Gate_DataModel_EquipmentID &s)
{
	if(m_strTesterName != s.m_strTesterName)					return FALSE;
	if(m_strTesterType != s.m_strTesterType)					return FALSE;
	if(m_strHandlerProberID != s.m_strHandlerProberID)			return FALSE;
	if(m_strProbeCardID != s.m_strProbeCardID)					return FALSE;
	if(m_strLoadBoardID != s.m_strLoadBoardID)					return FALSE;
	if(m_strDibBoardID != s.m_strDibBoardID)					return FALSE;
	if(m_strInterfaceCableID != s.m_strInterfaceCableID)		return FALSE;
	if(m_strHandlerContactorID != s.m_strHandlerContactorID)	return FALSE;
	if(m_strLaserID != s.m_strLaserID)							return FALSE;
	if(m_strExtraEquipmentID != s.m_strExtraEquipmentID)		return FALSE;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Clear object data (free allocated ressources)
//
// Argument(s) :
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_EquipmentID::ClearData()
{
	// Free allocated ressources

	// Reset variables
	m_strTesterName			= "";
	m_strTesterType			= "";
	m_strHandlerProberID	= "";
	m_strProbeCardID		= "";
	m_strLoadBoardID		= "";
	m_strDibBoardID			= "";
	m_strInterfaceCableID	= "";
	m_strHandlerContactorID	= "";
	m_strLaserID			= "";
	m_strExtraEquipmentID	= "";
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Dumps the data structure to an ascii file (CSV format)
//
// Argument(s) :
//
//	const QTextStream & hDumpFile
//		Dump file stream
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_EquipmentID::Dump(QTextStream & hDumpFile)
{
	// Write header
	hDumpFile << endl;
	hDumpFile << "EQUIPMENT_ID**********" << endl;
	hDumpFile << "TesterType" << CSV_SEPARATOR << GetCsvString(m_strTesterType) << endl;
	hDumpFile << "TesterName" << CSV_SEPARATOR << GetCsvString(m_strTesterName) << endl;
	hDumpFile << "HandlerProberID" << CSV_SEPARATOR << GetCsvString(m_strHandlerProberID) << endl;
	hDumpFile << "ProbeCardID" << CSV_SEPARATOR << GetCsvString(m_strProbeCardID) << endl;
	hDumpFile << "LoadBoardID" << CSV_SEPARATOR << GetCsvString(m_strLoadBoardID) << endl;
	hDumpFile << "DibBoardID" << CSV_SEPARATOR << GetCsvString(m_strDibBoardID) << endl;
	hDumpFile << "InterfaceCableID" << CSV_SEPARATOR << GetCsvString(m_strInterfaceCableID) << endl;
	hDumpFile << "HandlerContactorID" << CSV_SEPARATOR << GetCsvString(m_strHandlerContactorID) << endl;
	hDumpFile << "LaserID" << CSV_SEPARATOR << GetCsvString(m_strLaserID) << endl;
	hDumpFile << "ExtraEquipmentID" << CSV_SEPARATOR << GetCsvString(m_strExtraEquipmentID) << endl;
}

/////////////////////////////////////////////////////////////////////////////////////////
// BATCHID OBJECT
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// Description : Constructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_BatchID::C_Gate_DataModel_BatchID()
{
	ClearData();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Destructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_BatchID::~C_Gate_DataModel_BatchID()
{
	ClearData();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Comparison operator
//
// Argument(s) :
//
//	const C_Gate_DataModel_BatchID &s
//		reference to object to compare with this one
//
// Return type : TRUE if the 2 objects are identical, FALSE else
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_BatchID::operator==(const C_Gate_DataModel_BatchID &s)
{
	// Lot information
	if(m_strLotID != s.m_strLotID)								return FALSE;
	if(m_strSublotID != s.m_strSublotID)						return FALSE;
	if(m_strWaferID != s.m_strWaferID)							return FALSE;

	// Test conditions
	if(*m_pParameterSet != *(s.m_pParameterSet))				return FALSE;
	if(m_uiNbSites != s.m_uiNbSites)							return FALSE;
	if(m_strOperatorName != s.m_strOperatorName)				return FALSE;
	if(m_strJobName != s.m_strJobName)							return FALSE;
	if(m_strJobRevision != s.m_strJobRevision)					return FALSE;
	if(m_strExecType != s.m_strExecType)						return FALSE;
	if(m_strExecVersion != s.m_strExecVersion)					return FALSE;
	if(m_strTestCode != s.m_strTestCode)						return FALSE;
	if(m_strTestTemperature != s.m_strTestTemperature)			return FALSE;

	// Equipment identification
	if(m_clEquipmentID != s.m_clEquipmentID)					return FALSE;

	 return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Clear object data (free allocated ressources)
//
// Argument(s) :
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_BatchID::ClearData()
{
	// Free allocated ressources (m_pParameterSet is not allocated here, it's just a ptr on an object handled elsewhere)

	// Reset variables
	// Lot information
	m_strLotID					= "";
	m_strSublotID				= "";
	m_strWaferID				= "";

	// Test conditions
	m_pParameterSet				= NULL;
	m_uiNbSites					= 0;
	m_strOperatorName			= "";
	m_strJobName				= "";
	m_strJobRevision			= "";
	m_strExecType				= "";
	m_strExecVersion			= "";
	m_strTestCode				= "";
	m_strTestTemperature		= "";

	// Equipment identification
	m_clEquipmentID.ClearData();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Dumps the data structure to an ascii file (CSV format)
//
// Argument(s) :
//
//	const QTextStream & hDumpFile
//		Dump file stream
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_BatchID::Dump(QTextStream & hDumpFile)
{
	// Write header
	hDumpFile << endl;
	hDumpFile << "BATCH_ID**********" << endl;
	hDumpFile << "LotID" << CSV_SEPARATOR << GetCsvString(m_strLotID) << endl;
	hDumpFile << "SublotID" << CSV_SEPARATOR << GetCsvString(m_strSublotID) << endl;
	hDumpFile << "WaferID" << CSV_SEPARATOR << GetCsvString(m_strWaferID) << endl;
	if(m_pParameterSet != NULL)
		hDumpFile << "ParameterSet Nb" << CSV_SEPARATOR << QString::number(m_pParameterSet->m_uiParameterSetNb) << endl;
	else
		hDumpFile << "ParameterSet Nb" << CSV_SEPARATOR << "n/a" << endl;
	hDumpFile << "Nb sites" << CSV_SEPARATOR << QString::number(m_uiNbSites) << endl;
	hDumpFile << "OperatorName" << CSV_SEPARATOR << GetCsvString(m_strOperatorName) << endl;
	hDumpFile << "JobName" << CSV_SEPARATOR << GetCsvString(m_strJobName) << endl;
	hDumpFile << "JobRevision" << CSV_SEPARATOR << GetCsvString(m_strJobRevision) << endl;
	hDumpFile << "ExecType" << CSV_SEPARATOR << GetCsvString(m_strExecType) << endl;
	hDumpFile << "ExecVersion" << CSV_SEPARATOR << GetCsvString(m_strExecVersion) << endl;
	hDumpFile << "TestCode" << CSV_SEPARATOR << GetCsvString(m_strTestCode) << endl;
	hDumpFile << "TestTemperature" << CSV_SEPARATOR << GetCsvString(m_strTestTemperature) << endl;
	m_clEquipmentID.Dump(hDumpFile);
}

/////////////////////////////////////////////////////////////////////////////////////////
// BATCH OBJECT
/////////////////////////////////////////////////////////////////////////////////////////
// ERROR MAP
GBEGIN_ERROR_MAP(C_Gate_DataModel_Batch)
	GMAP_ERROR(eMultipleHeads,"The data comes from several test heads (%d, %d)")
	GMAP_ERROR(eSiteNotInList,"Attempt to set a result for a site that is not in the site list (site %d)")
	GMAP_ERROR(eNoParameterSet,"Attempt to set a parameter result, but no parameters are defined")
	GMAP_ERROR(eSetTestResult,"Error setting test result")
	GMAP_ERROR(eSetPartResult,"Error setting part result")
GEND_ERROR_MAP(C_Gate_DataModel_Batch)

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Constructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_Batch::C_Gate_DataModel_Batch(C_Gate_DataModel_Sublot *pParent, C_Gate_DataModel_Progress *pProgress) : C_Gate_DataModel_Object(eData_Batch, pParent,pProgress)
{
	m_pGoodBins_SoftBins.AddBinning(1,true);
	m_pGoodBins_HardBins.AddBinning(1,true);

	m_bWithWaferInfo = false;
	m_pWaferMap = NULL;

	ClearData();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Destructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_Batch::~C_Gate_DataModel_Batch()
{
	ClearData();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Cleanup data (remove empty sites...)
//
// Argument(s) :
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_Batch::Cleanup()
{
	C_Gate_DataModel_Site *pSite;

	pSite = GetFirstSite();
	while(pSite)
	{
		if((pSite->m_uiNbParameters == 0) || (pSite->m_uiNbParts == 0))
		{
			pSite = RemoveCurrentSite();
		}
		else
			pSite = GetNextSite();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Comparison operator
//
// Argument(s) :
//
//	const C_Gate_DataModel_Batch &s
//		reference to object to compare with this one
//
// Return type : TRUE if the 2 objects are identical, FALSE else
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_Batch::operator==(const C_Gate_DataModel_Batch &s)
{
	return (m_clBatchID == s.m_clBatchID);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Clear object data (free allocated ressources)
//
// Argument(s) :
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_Batch::ClearData()
{
	m_clBatchID.ClearData();

	// Free allocated ressources
//	m_clResultSet_Multisite.ClearData();
//	m_clResultSet_Compliance.ClearData();
	ClearSites();

	m_pSoftBins.clearItems();
	m_pHardBins.clearItems();

	// Reset variables
	m_clFromDate.setTime_t(0);
	m_clToDate.setTime_t(0);
	m_nHeadNum = -1;
	m_uiNbParameters = 0;
	m_uiNbRuns = 0;
	m_uiNbParts = 0;
	m_uiNbPassParts = 0;
	m_uiNbFailParts = 0;
	m_uiNbRetestParts = 0;
	m_uiNbFailPartsAfterRetest = 0;

	if(m_pWaferMap != NULL)
		delete m_pWaferMap;
	m_pWaferMap = NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Init WaferMap
//
// Argument(s) :
//
//
// Return type : TRUE if wafermap result correctly set, FALSE else
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_Batch::InitWaferMap()
{

	if(m_pWaferMap != NULL)
		m_pWaferMap->ClearWaferMap();
	else
	{
		m_pWaferMap = new C_Gate_DataModel_WaferMap();
	}
	m_pWaferMap->InitializeWaferMap(this);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Return ptr on site with specified site index
//
// Argument(s) :
//
//	unsigned int uiSiteIndex
//		Index of the site to find
//
// Return type : Ptr on site if site found, NULL else
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_Site* C_Gate_DataModel_Batch::GetSiteFromSiteIndex(unsigned int uiSiteIndex)
{
	C_Gate_DataModel_Site*	pSite;

	pSite = GetFirstSite();
	while(pSite)
	{
		if(pSite->m_uiSiteIndex == uiSiteIndex)
			return pSite;
		pSite = GetNextSite();
	}

	// Site not found, return NULL
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Return ptr on site with specified site nb
//
// Argument(s) :
//
//	int nSiteNb
//		Nb of the site to find
//
// Return type : Ptr on site if site found, NULL else
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_Site* C_Gate_DataModel_Batch::GetSiteFromSiteNumber(int nSiteNb)
{
	C_Gate_DataModel_Site*	pSite;

	pSite = GetFirstSite();
	while(pSite)
	{
		if(pSite->m_nSiteNb == nSiteNb)
			return pSite;
		pSite = GetNextSite();
	}

	// Site not found, return NULL
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Udates From and To dates of the batch
//
// Argument(s) :
//
//	long lStartTime
//		Start time
//
//	long lFinishTime
//		Finish time
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_Batch::UpdateFromToDates(long lStartTime, long lFinishTime)
{
	if((m_clFromDate.toTime_t() == 0) || (lStartTime < (long)m_clFromDate.toTime_t()))
		m_clFromDate.setTime_t(lStartTime);
	if((m_clToDate.toTime_t() == 0) || (lFinishTime > (long)m_clToDate.toTime_t()))
		m_clToDate.setTime_t(lFinishTime);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Make sure data comes from a single test head (we don't support multi-head data)
//
// Argument(s) :
//
//	const Gate_SiteDescriptionMap* pSiteEquipmentIDMap
//		Map of Site description objects from plug-in
//
// Return type : TRUE if no multiple heads found, FALSE else
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_Batch::CheckHead(const Gate_SiteDescriptionMap* pSiteEquipmentIDMap)
{
	// Make sure we work on a single head (we don't support multiple heads in same dataset)
	if(pSiteEquipmentIDMap->count() != 0)
	{
		for(Gate_SiteDescriptionMap::const_iterator it = pSiteEquipmentIDMap->begin(); it != pSiteEquipmentIDMap->end(); ++it)
		{
			if(m_nHeadNum == -1)
				m_nHeadNum = (*it).m_nHeadNum;
			else if(m_nHeadNum != (*it).m_nHeadNum)
			{
				GSET_ERROR2(C_Gate_DataModel_Batch, eMultipleHeads, NULL, m_nHeadNum, (*it).m_nHeadNum);
				return FALSE;
			}
		}
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Make sure data comes from a single test head (we don't support multi-head data)
//
// Argument(s) :
//
//	const Gate_SiteDescriptionMap* pSiteEquipmentIDMap
//		Map of Site description objects from plug-in
//
// Return type : TRUE if no multiple heads found, FALSE else
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_Batch::CheckHead(int nHeadNum)
{
	// Make sure we work on a single head (we don't support multiple heads in same dataset)
	if(m_nHeadNum == -1)
		m_nHeadNum = nHeadNum;
	else if(m_nHeadNum != nHeadNum)
	{
		GSET_ERROR2(C_Gate_DataModel_Batch, eMultipleHeads, NULL, m_nHeadNum, nHeadNum);
		return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Set a test result for specified parameter
//
// Argument(s) :
//
//	const Gate_DataResult & DataResult
//		Result structure from plug-in
//
// Return type : TRUE if test result correctly set, FALSE else
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_Batch::SetTestResult(const Gate_DataResult & DataResult)
{
	// Make sure data comes from a single head (we don't support multiple heads)
	if(CheckHead(DataResult.m_nHeadNum) == FALSE)
		return FALSE;

	// Get relevant site
	C_Gate_DataModel_Site* pSite = GetFirstSite();
	while(pSite)
	{
		if(pSite->m_nSiteNb == DataResult.m_nSiteNum)
			break;
		pSite = GetNextSite();
	}

	// Did we find the site??
	if(!pSite)
	{
		GSET_ERROR1(C_Gate_DataModel_Batch, eSiteNotInList, NULL, DataResult.m_nSiteNum);
		return FALSE;
	}

	// Check if parameter set available
	if(m_clBatchID.m_pParameterSet == NULL)
	{
		GSET_ERROR0(C_Gate_DataModel_Batch, eNoParameterSet, NULL);
		return FALSE;
	}

	// Set result
	if(pSite->SetTestResult(DataResult, m_clBatchID.m_pParameterSet) == FALSE)
	{
		//GSET_ERROR0(C_Gate_DataModel_Batch, eSetTestResult, NULL);
		return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Check if Part is FAIL
//
// Argument(s) :
//
//	const Gate_PartResult & PartResult
//		Result structure from plug-in
//
// Return type : TRUE if part is FAIL, FALSE else
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_Batch::IsPartFail(const Gate_PartResult & PartResult)
{
	// We could check first if any test of this part failed...
	// This check is not currently done, some tests may be fail but with no impact on the Binning

	// Check if the Part's Fail flag is valid
	if(PartResult.m_bPassFailStatusValid == TRUE)
		return PartResult.m_bPartFailed;

	// Check if Binning is in Good Bin list
	unsigned int uiSoftBin = PartResult.m_nSoftBin;
	unsigned int uiHardBin = PartResult.m_nHardBin;
	if((m_pGoodBins_SoftBins.Find(uiSoftBin) != -1) && (m_pGoodBins_HardBins.Find(uiHardBin) != -1))
		return FALSE;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Check if Part is a retest
//
// Argument(s) :
//
//	const Gate_PartResult & PartResult
//		Result structure from plug-in
//
// Return type : TRUE if part is a retest, FALSE else
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_Batch::IsPartRetest(const Gate_PartResult & PartResult)
{
	C_Gate_DataModel_Site				*pSite;
	C_Gate_DataModel_PartResult			*pPartResult;
	unsigned int						uiIndex;
	C_Gate_DataModel_PartResultVector	PartVector;
	C_Gate_DataModel_TestingStage		*pTestingStage = GetParent()->GetParent()->GetParent();
	bool							bReturn = FALSE;

	if(!pTestingStage->IsWTest() && !pTestingStage->IsETest())
		return bReturn;

	// Go through all sites
	pSite = GetFirstSite();
	while(pSite)
	{
		for(uiIndex = 0; uiIndex < pSite->m_clPartVector.CountValues(); uiIndex++)
		{
			pPartResult = pSite->m_clPartVector.at(uiIndex);

			// If wafer, check if coordinates match
			if(	(pTestingStage->IsWTest() || pTestingStage->IsETest()) &&
				((pPartResult->m_nXLoc == PartResult.m_nPart_X) && (pPartResult->m_nYLoc == PartResult.m_nPart_Y)))
			{
				// Set flag to specify that this part has been retested
				pPartResult->m_uiFlags |= YIELD123_PARTRESULT_HASBEENRETESTED;
				bReturn = TRUE;
				// return the result now
				return bReturn;
			}

			// We currently don't use the PartID when checking for retest
		}
		/*PartVector.Fill(pSite, eRun_Any ,PartResult.m_nPart_X,PartResult.m_nPart_Y);
		for(uiIndex = 0; uiIndex < PartVector.CountValues(); uiIndex++)
		{
			pPartResult = PartVector.at(uiIndex);

			// If wafer, check if coordinates match
			if(	(pTestingStage->IsWTest() || pTestingStage->IsETest()) &&
				((pPartResult->m_nXLoc == PartResult.m_nPart_X) && (pPartResult->m_nYLoc == PartResult.m_nPart_Y)))
			{
				// Set flag to specify that this part has been retested
				pPartResult->m_uiFlags |= YIELD123_PARTRESULT_HASBEENRETESTED;
				bReturn = TRUE;
				// return the result now
				return bReturn;
			}

			// We currently don't use the PartID when checking for retest
		}
		*/

		pSite = GetNextSite();
	}

	return bReturn;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Set a part result for specified part
//
// Argument(s) :
//
//	const Gate_PartResult & PartResult
//		Result structure from plug-in
//
// Return type : TRUE if part result correctly set, FALSE else
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_Batch::SetPartResult(const Gate_PartResult & PartResult)
{
	bool bPartIsFail, bPartIsRetest;

	// Increment ProgressBar
	if(m_pProgress)
		m_pProgress->Increment();

	// Make sure data comes from a single head (we don't support multiple heads)
	if(CheckHead(PartResult.m_nHeadNum) == FALSE)
		return FALSE;

	// Get relevant site
	C_Gate_DataModel_Site* pSite = GetFirstSite();
	while(pSite)
	{
		if(pSite->m_nSiteNb == PartResult.m_nSiteNum)
			break;
		pSite = GetNextSite();
	}

	// Did we find the site??
	if(!pSite)
	{
		GSET_ERROR1(C_Gate_DataModel_Batch, eSiteNotInList, NULL, PartResult.m_nSiteNum);
		return FALSE;
	}

	// Check if part is FAIL
	bPartIsFail = IsPartFail(PartResult);

	// Check if part is a retest
	bPartIsRetest = IsPartRetest(PartResult);

	// Set result
	if(pSite->SetPartResult(PartResult, bPartIsFail, bPartIsRetest) == FALSE)
	{
		GSET_ERROR0(C_Gate_DataModel_Batch, eSetPartResult, NULL);
		return FALSE;
	}

	// Update binnings
	m_pSoftBins.AddBinning(PartResult.m_nSoftBin, !bPartIsFail, bPartIsRetest);
	m_pHardBins.AddBinning(PartResult.m_nHardBin, !bPartIsFail, bPartIsRetest);

	// Update batch part counters
	m_uiNbParts++;
	if(bPartIsRetest == TRUE)
	{
		m_uiNbRetestParts++;
		if(bPartIsFail == FALSE)
			m_uiNbFailPartsAfterRetest++;
	}
	if(bPartIsFail == FALSE)
		m_uiNbPassParts++;
	else
		m_uiNbFailParts++;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Set a bin info
//
// Argument(s) :
//
//	const Gate_BinningDef & BinDef
//		Result structure from plug-in
//
// Return type : TRUE if binning correctly set, FALSE else
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_Batch::SetBinning(const Gate_BinningDef & BinDef)
{
	// Increment ProgressBar
	if(m_pProgress)
		m_pProgress->Increment();

	// Get all site
	C_Gate_DataModel_Site* pSite = GetFirstSite();
	while(pSite)
	{
		pSite->SetBinning(BinDef);
		pSite = GetNextSite();
	}

	// Update binnings
	if(BinDef.m_bBinType == 'H')
		m_pHardBins.InitBinning(BinDef.m_nBinNumber, BinDef.m_strBinName, BinDef.m_bBinCat);
	else
		m_pSoftBins.InitBinning(BinDef.m_nBinNumber, BinDef.m_strBinName, BinDef.m_bBinCat);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// Note: return the Vector from the badest site depending to the eSortSelector
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_ParameterStatVector* C_Gate_DataModel_Batch::GetParameterStatVector(C_Gate_DataModel_ParameterStatVector::SortOn eSortSelector, bool bAscending)
{

	// Get all site
	C_Gate_DataModel_Site* pSite = GetFirstSite();
	C_Gate_DataModel_Site* pSelectedSite = pSite;

	C_Gate_DataModel_ParameterStat	*pParameterStat;
	unsigned int					uiParameterIndex;
	float							fValue;
	float							fSelectedValue = 0.0;

	while(pSite)
	{
		fValue = 0;
		// Now sort
		switch(eSortSelector)
		{
		case C_Gate_DataModel_ParameterStatVector::eSortOnParameterIndex:
			fValue = pSite->m_uiNbParameters;
			break;
		case C_Gate_DataModel_ParameterStatVector::eSortOnOutliers:
			// check all parameter stats
			for(uiParameterIndex = 0; uiParameterIndex < pSite->m_uiNbParameters; uiParameterIndex++)
			{
				pParameterStat = pSite->m_pParameterStats_AllData + uiParameterIndex;
				fValue += (float) (pParameterStat->m_uiOutliers_H + pParameterStat->m_uiOutliers_L);
			}
			break;
		case C_Gate_DataModel_ParameterStatVector::eSortOnOutliers_L:
			// check all parameter stats
			for(uiParameterIndex = 0; uiParameterIndex < pSite->m_uiNbParameters; uiParameterIndex++)
			{
				pParameterStat = pSite->m_pParameterStats_AllData + uiParameterIndex;
				fValue += (float) (pParameterStat->m_uiOutliers_L);
			}
			break;
		case C_Gate_DataModel_ParameterStatVector::eSortOnOutliers_H:
			// check all parameter stats
			for(uiParameterIndex = 0; uiParameterIndex < pSite->m_uiNbParameters; uiParameterIndex++)
			{
				pParameterStat = pSite->m_pParameterStats_AllData + uiParameterIndex;
				fValue += (float) (pParameterStat->m_uiOutliers_H);
			}
			break;
		case C_Gate_DataModel_ParameterStatVector::eSortOnOutliers_GoodRun:
			// check all parameter stats
			for(uiParameterIndex = 0; uiParameterIndex < pSite->m_uiNbParameters; uiParameterIndex++)
			{
				pParameterStat = pSite->m_pParameterStats_AllData + uiParameterIndex;
				fValue += (float) (pParameterStat->m_uiOutliers_L_GoodPart + pParameterStat->m_uiOutliers_H_GoodPart);
			}
			break;
		case C_Gate_DataModel_ParameterStatVector::eSortOnOutliers_LatestRetest:
			// check all parameter stats
			for(uiParameterIndex = 0; uiParameterIndex < pSite->m_uiNbParameters; uiParameterIndex++)
			{
				pParameterStat = pSite->m_pParameterStats_AllData + uiParameterIndex;
				fValue += (float) (pParameterStat->m_uiOutliers_L_LastRetest + pParameterStat->m_uiOutliers_H_LastRetest);
			}
			break;
		case C_Gate_DataModel_ParameterStatVector::eSortOnCpk:
			// check all parameter stats
			for(uiParameterIndex = 0; uiParameterIndex < pSite->m_uiNbParameters; uiParameterIndex++)
			{
				pParameterStat = pSite->m_pParameterStats_AllData + uiParameterIndex;
				if(pParameterStat->m_fCpk < GEX_PLUGIN_INFINITE_VALUE_FLOAT)
					fValue += (float) (pParameterStat->m_fCpk);
			}
			break;
		case C_Gate_DataModel_ParameterStatVector::eSortOnCp:
			// check all parameter stats
			for(uiParameterIndex = 0; uiParameterIndex < pSite->m_uiNbParameters; uiParameterIndex++)
			{
				pParameterStat = pSite->m_pParameterStats_AllData + uiParameterIndex;
				if(pParameterStat->m_fCp < GEX_PLUGIN_INFINITE_VALUE_FLOAT)
					fValue += (float) (pParameterStat->m_fCp);
			}
			break;
		case C_Gate_DataModel_ParameterStatVector::eSortOnMean:
			// check all parameter stats
			for(uiParameterIndex = 0; uiParameterIndex < pSite->m_uiNbParameters; uiParameterIndex++)
			{
				pParameterStat = pSite->m_pParameterStats_AllData + uiParameterIndex;
				if(pParameterStat->m_fMean < GEX_PLUGIN_INFINITE_VALUE_FLOAT)
					fValue += (float) (pParameterStat->m_fMean);
			}
			break;
		case C_Gate_DataModel_ParameterStatVector::eSortOnMin:
			// check all parameter stats
			for(uiParameterIndex = 0; uiParameterIndex < pSite->m_uiNbParameters; uiParameterIndex++)
			{
				pParameterStat = pSite->m_pParameterStats_AllData + uiParameterIndex;
				if(pParameterStat->m_fMin < GEX_PLUGIN_INFINITE_VALUE_FLOAT)
					fValue += (float) (pParameterStat->m_fMin);
			}
			break;
		case C_Gate_DataModel_ParameterStatVector::eSortOnMax:
			// check all parameter stats
			for(uiParameterIndex = 0; uiParameterIndex < pSite->m_uiNbParameters; uiParameterIndex++)
			{
				pParameterStat = pSite->m_pParameterStats_AllData + uiParameterIndex;
				if(pParameterStat->m_fMax < GEX_PLUGIN_INFINITE_VALUE_FLOAT)
					fValue += (float) (pParameterStat->m_fMax);
			}
			break;
		case C_Gate_DataModel_ParameterStatVector::eSortOnRange:
			// check all parameter stats
			for(uiParameterIndex = 0; uiParameterIndex < pSite->m_uiNbParameters; uiParameterIndex++)
			{
				pParameterStat = pSite->m_pParameterStats_AllData + uiParameterIndex;
				if(pParameterStat->m_fRange < GEX_PLUGIN_INFINITE_VALUE_FLOAT)
					fValue += (float) (pParameterStat->m_fRange);
			}
			break;
		case C_Gate_DataModel_ParameterStatVector::eSortOnSigma:
			// check all parameter stats
			for(uiParameterIndex = 0; uiParameterIndex < pSite->m_uiNbParameters; uiParameterIndex++)
			{
				pParameterStat = pSite->m_pParameterStats_AllData + uiParameterIndex;
				if(pParameterStat->m_fSigma < GEX_PLUGIN_INFINITE_VALUE_FLOAT)
					fValue += (float) (pParameterStat->m_fSigma);
			}
			break;
		case C_Gate_DataModel_ParameterStatVector::eSortOnQ1:
			// check all parameter stats
			for(uiParameterIndex = 0; uiParameterIndex < pSite->m_uiNbParameters; uiParameterIndex++)
			{
				pParameterStat = pSite->m_pParameterStats_AllData + uiParameterIndex;
				if(pParameterStat->m_fQ1 < GEX_PLUGIN_INFINITE_VALUE_FLOAT)
					fValue += (float) (pParameterStat->m_fQ1);
			}
			break;
		case C_Gate_DataModel_ParameterStatVector::eSortOnQ2:
			// check all parameter stats
			for(uiParameterIndex = 0; uiParameterIndex < pSite->m_uiNbParameters; uiParameterIndex++)
			{
				pParameterStat = pSite->m_pParameterStats_AllData + uiParameterIndex;
				if(pParameterStat->m_fQ2 < GEX_PLUGIN_INFINITE_VALUE_FLOAT)
					fValue += (float) (pParameterStat->m_fQ2);
			}
			break;
		case C_Gate_DataModel_ParameterStatVector::eSortOnQ3:
			// check all parameter stats
			for(uiParameterIndex = 0; uiParameterIndex < pSite->m_uiNbParameters; uiParameterIndex++)
			{
				pParameterStat = pSite->m_pParameterStats_AllData + uiParameterIndex;
				if(pParameterStat->m_fQ3 < GEX_PLUGIN_INFINITE_VALUE_FLOAT)
					fValue += (float) (pParameterStat->m_fQ3);
			}
			break;
		case C_Gate_DataModel_ParameterStatVector::eSortOnExecs:
			// check all parameter stats
			for(uiParameterIndex = 0; uiParameterIndex < pSite->m_uiNbParameters; uiParameterIndex++)
			{
				pParameterStat = pSite->m_pParameterStats_AllData + uiParameterIndex;
				if(pParameterStat->m_uiExecs < GEX_PLUGIN_INFINITE_VALUE_FLOAT)
					fValue += (float) (pParameterStat->m_uiExecs);
			}
			break;
		case C_Gate_DataModel_ParameterStatVector::eSortOnPass:
			// check all parameter stats
			for(uiParameterIndex = 0; uiParameterIndex < pSite->m_uiNbParameters; uiParameterIndex++)
			{
				pParameterStat = pSite->m_pParameterStats_AllData + uiParameterIndex;
				if(pParameterStat->m_uiPass < GEX_PLUGIN_INFINITE_VALUE_FLOAT)
					fValue += (float) (pParameterStat->m_uiPass);
			}
			break;
		case C_Gate_DataModel_ParameterStatVector::eSortOnFail:
			// check all parameter stats
			for(uiParameterIndex = 0; uiParameterIndex < pSite->m_uiNbParameters; uiParameterIndex++)
			{
				pParameterStat = pSite->m_pParameterStats_AllData + uiParameterIndex;
				if(pParameterStat->m_uiFail < GEX_PLUGIN_INFINITE_VALUE_FLOAT)
					fValue += (float) (pParameterStat->m_uiFail);
			}
			break;
		case C_Gate_DataModel_ParameterStatVector::eSortOnFail_L:
			// check all parameter stats
			for(uiParameterIndex = 0; uiParameterIndex < pSite->m_uiNbParameters; uiParameterIndex++)
			{
				pParameterStat = pSite->m_pParameterStats_AllData + uiParameterIndex;
				if(pParameterStat->m_uiFail_L < GEX_PLUGIN_INFINITE_VALUE_FLOAT)
					fValue += (float) (pParameterStat->m_uiFail_L);
			}
			break;
		case C_Gate_DataModel_ParameterStatVector::eSortOnFail_H:
			// check all parameter stats
			for(uiParameterIndex = 0; uiParameterIndex < pSite->m_uiNbParameters; uiParameterIndex++)
			{
				pParameterStat = pSite->m_pParameterStats_AllData + uiParameterIndex;
				if(pParameterStat->m_uiFail_H < GEX_PLUGIN_INFINITE_VALUE_FLOAT)
					fValue += (float) (pParameterStat->m_uiFail_H);
			}
			break;
		case C_Gate_DataModel_ParameterStatVector::eSortOnTestNumber:
			fValue = pSite->m_uiNbParameters;
			break;
		case C_Gate_DataModel_ParameterStatVector::eSortOnTestName:
			fValue = pSite->m_uiNbParameters;
			break;
		case C_Gate_DataModel_ParameterStatVector::eNotSorted:
			break;
		}

		if(pSite == pSelectedSite)
			fSelectedValue = fValue;

		if(fValue > fSelectedValue)
			pSelectedSite = pSite;

		pSite = GetNextSite();
	}

	return pSelectedSite->GetParameterStatVector(eSortSelector,bAscending);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Set a result for a specific part / run nb
//
// Argument(s) :
//
//	const C_Gate_DataModel_BinningVector & Binning
//		Result values
//
// Return type : bool
//		TRUE if result successfully set, FALSE else
/////////////////////////////////////////////////////////////////////////////////////////
// Note: if we try to set a result that was alread set for this particular test/run, the
//       first value is kept, and all subsequent one will be ignored.
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_BinningVector* C_Gate_DataModel_Batch::GetHardBinningVector(C_Gate_DataModel_BinningVector::SortOn eSortSelector, bool bAscending)
{
	if(m_clHardBinningVector.CountValues() == 0)
		m_clHardBinningVector.Fill(this, eParameter_WithResults,true);
	m_clHardBinningVector.Sort(eSortSelector,bAscending);

	return &m_clHardBinningVector;
}

C_Gate_DataModel_BinningVector* C_Gate_DataModel_Batch::GetSoftBinningVector(C_Gate_DataModel_BinningVector::SortOn eSortSelector, bool bAscending)
{
	if(m_clSoftBinningVector.CountValues() == 0)
		m_clSoftBinningVector.Fill(this, eParameter_WithResults,false);
	m_clSoftBinningVector.Sort(eSortSelector,bAscending);

	return &m_clSoftBinningVector;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Dumps the data structure to an ascii file (CSV format)
//
// Argument(s) :
//
//	const QTextStream & hDumpFile
//		Dump file stream
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_Batch::Dump(QTextStream & hDumpFile)
{
	// Update ProgressBar
	if(m_pProgress)
		m_pProgress->Increment();

	//C_Gate_DataModel_ParameterSet	*pParameterSet = m_clBatchID.m_pParameterSet;
	C_Gate_DataModel_Binning		*pBinning;

	// Write header
	hDumpFile << endl;
	hDumpFile << "BATCH" << endl;
	hDumpFile << "=====" << endl;
	m_clBatchID.Dump(hDumpFile);
	hDumpFile << "From" << CSV_SEPARATOR << m_clFromDate.toString("dd MMM yyyy hh:mm:ss") << endl;
	hDumpFile << "To" << CSV_SEPARATOR << m_clToDate.toString("dd MMM yyyy hh:mm:ss") << endl;
	hDumpFile << "Nb sites" << CSV_SEPARATOR << NbSites() << endl;
	hDumpFile << "Nb Runs" << CSV_SEPARATOR << QString::number(m_uiNbRuns) << endl;
	hDumpFile << "Nb Parts" << CSV_SEPARATOR << QString::number(m_uiNbParts) << endl;
	hDumpFile << "Nb PASS Parts" << CSV_SEPARATOR << QString::number(m_uiNbPassParts) << endl;
	hDumpFile << "Nb FAIL Parts" << CSV_SEPARATOR << QString::number(m_uiNbFailParts) << endl;
	hDumpFile << "Nb RETEST Parts" << CSV_SEPARATOR << QString::number(m_uiNbRetestParts) << endl;
	hDumpFile << "Nb FAIL Parts after RETEST" << CSV_SEPARATOR << QString::number(m_uiNbFailPartsAfterRetest) << endl;

	// Write Binnings
	m_pSoftBins.Sort(C_Gate_DataModel_BinningList::eSortOnBinCount, FALSE);	// Sort on decreasing bin count
	m_pHardBins.Sort(C_Gate_DataModel_BinningList::eSortOnBinCount, FALSE);	// Sort on decreasing bin count
	hDumpFile << endl;
	hDumpFile << "Binnings**********" << endl;
	hDumpFile << "Soft Bin" << CSV_SEPARATOR;
	hDumpFile << "Count" << CSV_SEPARATOR;
	hDumpFile << "Retest Count" << CSV_SEPARATOR;
	hDumpFile << "Hard Bin" << CSV_SEPARATOR;
	hDumpFile << "Count" << CSV_SEPARATOR;
	hDumpFile << "Retest Count" << CSV_SEPARATOR;
	hDumpFile << endl;
	for (pBinning = m_pSoftBins.GetFirst(); pBinning; pBinning = m_pSoftBins.GetNext())
	{
		hDumpFile << pBinning->m_uiBinNb << CSV_SEPARATOR;
		hDumpFile << pBinning->m_uiBinCount << CSV_SEPARATOR;
		hDumpFile << pBinning->m_uiBinCount_Retest << CSV_SEPARATOR;
		hDumpFile << pBinning->m_uiBinNb << CSV_SEPARATOR;
		hDumpFile << pBinning->m_uiBinCount << CSV_SEPARATOR;
		hDumpFile << pBinning->m_uiBinCount_Retest << CSV_SEPARATOR;
		hDumpFile << endl;
	}

	// Write Rule results for multisite analysis
//	if(m_clResultSet_Multisite.m_pRuleResults != NULL)
//	{
//		hDumpFile << endl;
//		hDumpFile << "Rule Results for Multisite analysis**********" << endl;
//		if(pParameterSet == NULL)
//		{
//			hDumpFile << "No parameters defined" << endl;
//		}
//		else
//		 m_clResultSet_Multisite.Dump(hDumpFile);
//	}

	// Write Rule results for multisite analysis
//	if(m_clResultSet_Compliance.m_pRuleResults != NULL)
//	{
//		hDumpFile << endl;
//		hDumpFile << "Rule Results for Compliance analysis**********" << endl;
//		if(pParameterSet == NULL)
//		{
//			hDumpFile << "No parameters defined" << endl;
//		}
//		else
//		 m_clResultSet_Compliance.Dump(hDumpFile);
//	}

	// Dump all Sites
	C_Gate_DataModel_Site* pSite = GetFirstSite();
	while(pSite)
	{
		pSite->Dump(hDumpFile, m_clBatchID.m_pParameterSet);
		pSite = GetNextSite();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// SUBLOT OBJECT
/////////////////////////////////////////////////////////////////////////////////////////
// ERROR MAP
GBEGIN_ERROR_MAP(C_Gate_DataModel_Sublot)
	GMAP_ERROR(eInitSite, "Site initialization error")
	GMAP_ERROR(eNoSite, "No site found in dataset")
	GMAP_ERROR(eNoParameterSet,"Attempt to add a batch, but no parameters are defined")
	GMAP_ERROR(eCheckHead, "Check head error.")
GEND_ERROR_MAP(C_Gate_DataModel_Sublot)

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Constructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_Sublot::C_Gate_DataModel_Sublot(C_Gate_DataModel_Lot *pParent, C_Gate_DataModel_Progress *pProgress) : C_Gate_DataModel_Object(eData_Sublot, pParent,pProgress)
{
	ClearData();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Constructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_Sublot::C_Gate_DataModel_Sublot(C_Gate_DataModel_Lot *pParent, const QString &strLotID, const QString &strSublotID, const QString &strWaferID, C_Gate_DataModel_Progress *pProgress) : C_Gate_DataModel_Object(eData_Sublot, pParent,pProgress)
{
	// Set variables passed as parameter
	m_strLotID = strLotID;
	m_strSublotID = strSublotID;
	m_strWaferID = strWaferID;

	ClearData();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Destructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_Sublot::~C_Gate_DataModel_Sublot()
{
	ClearData();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Comparison operator
//
// Argument(s) :
//
//	const C_Gate_DataModel_Sublot &s
//		reference to object to compare with this one
//
// Return type : TRUE if the 2 objects are identical, FALSE else
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_Sublot::operator==(const C_Gate_DataModel_Sublot &s)
{
	if(m_strLotID != s.m_strLotID)			return FALSE;
	if(m_strSublotID != s.m_strSublotID)	return FALSE;
	if(m_strWaferID != s.m_strWaferID)		return FALSE;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Clear object data (free allocated ressources)
//
// Argument(s) :
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_Sublot::ClearData()
{
	// Free allocated ressources
	ClearBatches();

	// Reset variables
	m_clFromDate.setTime_t(0);
	m_clToDate.setTime_t(0);
	m_strLotID = "";
	m_strSublotID = "";
	m_strWaferID = "";
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Cleanup data (remove empty batches...)
//
// Argument(s) :
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_Sublot::Cleanup()
{
	C_Gate_DataModel_Batch *pBatch;

	pBatch = GetFirstBatch();
	while(pBatch)
	{
		pBatch->Cleanup();
		if(pBatch->NbSites() == 0)
			pBatch = RemoveCurrentBatch();
		else
			pBatch = GetNextBatch();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Udates From and To dates of the sublot
//
// Argument(s) :
//
//	long lStartTime
//		Start time
//
//	long lFinishTime
//		Finish time
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_Sublot::UpdateFromToDates(long lStartTime, long lFinishTime)
{
	if((m_clFromDate.toTime_t() == 0) || (lStartTime < (long)m_clFromDate.toTime_t()))
		m_clFromDate.setTime_t(lStartTime);
	if((m_clToDate.toTime_t() == 0) || (lFinishTime > (long)m_clToDate.toTime_t()))
		m_clToDate.setTime_t(lFinishTime);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Adds a batch to be used to store data to be received.
//
// Argument(s) :
//
//	const Gate_LotDef & LotDefinition
//		Lot definition object (LotID, SublotID...)
//
//	const C_Gate_DataModel_ParameterSet* pParameterSet
//		Ptr to Parameter set
//
//	const Gate_SiteDescription* pGlobalEquipmentID
//		Ptr to Global equipment definition object (Loadboard, HandlerProber...)
//
//	const Gate_SiteDescriptionMap* pSiteEquipmentIDMap
//		Ptr to Per site equipment definition object (Loadboard, HandlerProber...)
//
// Return type : Ptr to batch object to be used, NULL if an error occured
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_Batch* C_Gate_DataModel_Sublot::AddBatch(const Gate_LotDef & LotDefinition, C_Gate_DataModel_ParameterSet* pParameterSet, const Gate_SiteDescription& clGlobalEquipmentID, const Gate_SiteDescriptionMap* pSiteEquipmentIDMap)
{
	// Make sure a parameter set is available
	if(pParameterSet == NULL)
	{
		GSET_ERROR0(C_Gate_DataModel_Sublot, eNoParameterSet, NULL);
		return NULL;
	}

	// Create a new Batch object
	C_Gate_DataModel_Batch* pBatch = new C_Gate_DataModel_Batch(this,m_pProgress);
	pBatch->m_uiNbParameters = pParameterSet->m_uiNbParameters;

	// Make sure data comes from a single head (we don't support multiple heads)
	if(pBatch->CheckHead(pSiteEquipmentIDMap) == FALSE)
	{
		GSET_ERROR0(C_Gate_DataModel_Sublot, eCheckHead, GGET_LASTERROR(C_Gate_DataModel_Batch, pBatch));
		delete pBatch;
		return NULL;
	}
	// Save GoodHardBinning and GoodSoftBinning
	QList<int>::const_iterator it;
	if(LotDefinition.m_lstGoodHardBins.count() > 0)
		pBatch->m_pGoodBins_HardBins.clear();
	for(it = LotDefinition.m_lstGoodHardBins.begin(); it != LotDefinition.m_lstGoodHardBins.end(); it++)
		pBatch->m_pGoodBins_HardBins.AddBinning(*it,true);

	if(LotDefinition.m_lstGoodSoftBins.count() > 0)
		pBatch->m_pGoodBins_SoftBins.clear();
	for(it = LotDefinition.m_lstGoodSoftBins.begin(); it != LotDefinition.m_lstGoodSoftBins.end(); it++)
		pBatch->m_pGoodBins_SoftBins.AddBinning(*it,true);


	// Create site list
	C_Gate_DataModel_Site*	pclSite;
	unsigned int		uiSiteIndex=0;
	for(unsigned int uiSite=0 ; uiSite<256 ; uiSite++)
	{
		if(LotDefinition.m_puiPartsPerSite[uiSite] != 0)
		{
			// This site has some results: add a new site to the site list
			pclSite = new C_Gate_DataModel_Site(pBatch,m_pProgress);
			const Gate_SiteDescription* pSiteDescription;
			Gate_SiteDescriptionMap::const_iterator it = pSiteEquipmentIDMap->find(uiSite);
			if(it == pSiteEquipmentIDMap->end())
				pSiteDescription = &clGlobalEquipmentID;
			else
				pSiteDescription = &(*it);

			// Initialize site
			if(pclSite->Init(uiSiteIndex++, uiSite, pSiteDescription, LotDefinition, pParameterSet) == FALSE)
			{
				GSET_ERROR0(C_Gate_DataModel_Sublot, eInitSite, GGET_LASTERROR(C_Gate_DataModel_Site, pclSite));
				delete pclSite; pclSite=0;
				delete pBatch; pBatch=0;
				return NULL;
			}
			pBatch->AddSite(pclSite);
		}
	}

	// Make sure list has at least one site
	if(pBatch->NbSites() == 0)
	{
		GSET_ERROR0(C_Gate_DataModel_Sublot, eNoSite, NULL);
		delete pBatch; pBatch=0;
		return NULL;
	}

	// Set nb of runs
	pBatch->m_uiNbRuns = LotDefinition.m_uiProgramRuns;

	// Fill batch identification object
	// Lot Information
	pBatch->m_clBatchID.m_strLotID					= m_strLotID;
	pBatch->m_clBatchID.m_strSublotID				= m_strSublotID;
	pBatch->m_clBatchID.m_strWaferID				= m_strWaferID;
	// Test conditions
	pBatch->m_clBatchID.m_pParameterSet				= pParameterSet;
	pBatch->m_clBatchID.m_uiNbSites					= LotDefinition.m_uiNbSites;
	pBatch->m_clBatchID.m_strOperatorName			= LotDefinition.m_strOperatorName;
	pBatch->m_clBatchID.m_strJobName				= LotDefinition.m_strJobName;
	pBatch->m_clBatchID.m_strJobRevision			= LotDefinition.m_strJobRevision;
	pBatch->m_clBatchID.m_strExecType				= LotDefinition.m_strExecType;
	pBatch->m_clBatchID.m_strExecVersion			= LotDefinition.m_strExecVersion;
	pBatch->m_clBatchID.m_strTestCode				= LotDefinition.m_strTestCode;
	pBatch->m_clBatchID.m_strTestTemperature		= LotDefinition.m_strTestTemperature;
	// Equipment ID
	pBatch->m_clBatchID.m_clEquipmentID.m_strTesterName = LotDefinition.m_strTesterName;
	pBatch->m_clBatchID.m_clEquipmentID.m_strTesterType = LotDefinition.m_strTesterType;
	pBatch->m_clBatchID.m_clEquipmentID.m_strDibBoardID = clGlobalEquipmentID.m_strDibBoardID;
	pBatch->m_clBatchID.m_clEquipmentID.m_strExtraEquipmentID = clGlobalEquipmentID.m_strExtraEquipmentID;
	pBatch->m_clBatchID.m_clEquipmentID.m_strHandlerContactorID = clGlobalEquipmentID.m_strHandlerContactorID;
	pBatch->m_clBatchID.m_clEquipmentID.m_strHandlerProberID = clGlobalEquipmentID.m_strHandlerProberID;
	pBatch->m_clBatchID.m_clEquipmentID.m_strInterfaceCableID = clGlobalEquipmentID.m_strInterfaceCableID;
	pBatch->m_clBatchID.m_clEquipmentID.m_strLaserID = clGlobalEquipmentID.m_strLaserID;
	pBatch->m_clBatchID.m_clEquipmentID.m_strLoadBoardID = clGlobalEquipmentID.m_strLoadBoardID;
	pBatch->m_clBatchID.m_clEquipmentID.m_strProbeCardID = clGlobalEquipmentID.m_strProbeCardID;

	// Append batch to list of batches
	AddBatch(pBatch);

	// Update batch's From and To dates
	pBatch->UpdateFromToDates(LotDefinition.m_lStartTime, LotDefinition.m_lFinishTime);

	return pBatch;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Dumps the data structure to an ascii file (CSV format)
//
// Argument(s) :
//
//	const QTextStream & hDumpFile
//		Dump file stream
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_Sublot::Dump(QTextStream & hDumpFile)
{
	// Write header
	hDumpFile << endl;
	hDumpFile << "SUBLOT" << endl;
	hDumpFile << "======" << endl;
	hDumpFile << "LotID" << CSV_SEPARATOR << GetCsvString(m_strLotID) << endl;
	hDumpFile << "SublotID" << CSV_SEPARATOR << GetCsvString(m_strSublotID) << endl;
	hDumpFile << "WaferID" << CSV_SEPARATOR << GetCsvString(m_strWaferID) << endl;
	hDumpFile << "From" << CSV_SEPARATOR << m_clFromDate.toString("dd MMM yyyy hh:mm:ss") << endl;
	hDumpFile << "To" << CSV_SEPARATOR << m_clToDate.toString("dd MMM yyyy hh:mm:ss") << endl;
	hDumpFile << "Nb batches" << CSV_SEPARATOR << NbBatches() << endl;

	// Dump all Batches
	C_Gate_DataModel_Batch* pBatch = GetFirstBatch();
	while(pBatch)
	{
		pBatch->Dump(hDumpFile);
		pBatch = GetNextBatch();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// LOT OBJECT
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// Description : Constructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_Lot::C_Gate_DataModel_Lot(C_Gate_DataModel_TestingStage *pParent, C_Gate_DataModel_Progress *pProgress) : C_Gate_DataModel_Object(eData_Lot, pParent,pProgress)
{
	ClearData();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Constructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_Lot::C_Gate_DataModel_Lot(C_Gate_DataModel_TestingStage *pParent, const QString &strLotID, C_Gate_DataModel_Progress *pProgress) : C_Gate_DataModel_Object(eData_Lot, pParent,pProgress)
{
	// Set variables passed as parameter
	m_strLotID = strLotID;

	ClearData();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Destructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_Lot::~C_Gate_DataModel_Lot()
{
	ClearData();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Comparison operator
//
// Argument(s) :
//
//	const C_Gate_DataModel_Lot &s
//		reference to object to compare with this one
//
// Return type : TRUE if the 2 objects are identical, FALSE else
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_Lot::operator==(const C_Gate_DataModel_Lot &s)
{
	if(m_strLotID != s.m_strLotID)			return FALSE;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Clear object data (free allocated ressources)
//
// Argument(s) :
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_Lot::ClearData()
{
	// Free allocated ressources
	ClearSublots();

	// Reset variables
	m_clFromDate.setTime_t(0);
	m_clToDate.setTime_t(0);
	m_strLotID = "";
	m_uiFlags = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Cleanup data (remove empty sublots...)
//
// Argument(s) :
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_Lot::Cleanup()
{
	C_Gate_DataModel_Sublot *pSublot;

	pSublot = GetFirstSublot();
	while(pSublot)
	{
		pSublot->Cleanup();
		if(pSublot->NbBatches() == 0)
			pSublot = RemoveCurrentSublot();
		else
			pSublot = GetNextSublot();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Udates From and To dates of the lot
//
// Argument(s) :
//
//	long lStartTime
//		Start time
//
//	long lFinishTime
//		Finish time
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_Lot::UpdateFromToDates(long lStartTime, long lFinishTime)
{
	if((m_clFromDate.toTime_t() == 0) || (lStartTime < (long)m_clFromDate.toTime_t()))
		m_clFromDate.setTime_t(lStartTime);
	if((m_clToDate.toTime_t() == 0) || (lFinishTime > (long)m_clToDate.toTime_t()))
		m_clToDate.setTime_t(lFinishTime);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Get sublot to be used to add data to be received.
//				 Create it if no corresponding sublot already in the list.
//
// Argument(s) :
//
//	const Gate_LotDef & LotDefinition
//		Lot definition object
//
// Return type : Ptr to sublot object to be used, NULL if an error occured
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_Sublot* C_Gate_DataModel_Lot::GetSublot(const Gate_LotDef & LotDefinition)
{
	// Check if we already have data for this sublot
	C_Gate_DataModel_Sublot* pSublot = GetFirstSublot();
	while(pSublot)
	{
		if((pSublot->m_strSublotID == LotDefinition.m_strSubLotID) && (pSublot->m_strWaferID == LotDefinition.m_strWaferID))
			break;
		pSublot = GetNextSublot();
	}

	// If no sublot found, insert a new one
	if(!pSublot)
	{
		pSublot = new C_Gate_DataModel_Sublot(this,m_pProgress);
		pSublot->m_strLotID = m_strLotID;
		pSublot->m_strSublotID = LotDefinition.m_strSubLotID;
		pSublot->m_strWaferID = LotDefinition.m_strWaferID;
		AddSublot(pSublot);
	}

	// Update sublot's From and To dates
	pSublot->UpdateFromToDates(LotDefinition.m_lStartTime, LotDefinition.m_lFinishTime);

	return pSublot;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Dumps the data structure to an ascii file (CSV format)
//
// Argument(s) :
//
//	const QTextStream & hDumpFile
//		Dump file stream
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_Lot::Dump(QTextStream & hDumpFile)
{
	// Write header
	hDumpFile << endl;
	hDumpFile << "LOT" << endl;
	hDumpFile << "===" << endl;
	hDumpFile << "LotID" << CSV_SEPARATOR << GetCsvString(m_strLotID) << endl;
	hDumpFile << "From" << CSV_SEPARATOR << m_clFromDate.toString("dd MMM yyyy hh:mm:ss") << endl;
	hDumpFile << "To" << CSV_SEPARATOR << m_clToDate.toString("dd MMM yyyy hh:mm:ss") << endl;
	hDumpFile << "Flags" << CSV_SEPARATOR << QString::number(m_uiFlags) << endl;
	hDumpFile << "Nb sublots" << CSV_SEPARATOR << NbSublots() << endl;

	// Dump all Sublots
	C_Gate_DataModel_Sublot* pSublot = GetFirstSublot();
	while(pSublot)
	{
		pSublot->Dump(hDumpFile);
		pSublot = GetNextSublot();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// PARAMETERDEF OBJECT
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// Description : Constructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_ParameterDef::C_Gate_DataModel_ParameterDef()
{
	ClearData();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Destructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_ParameterDef::~C_Gate_DataModel_ParameterDef()
{
	ClearData();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Assignment operator
//
// Argument(s) :
//
//	const C_Gate_DataModel_ParameterDef& source
//		reference to use for the assignement
//
// Return type : ptr to this object
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_ParameterDef& C_Gate_DataModel_ParameterDef::operator=(const C_Gate_DataModel_ParameterDef& source)
{
	m_uiNumber			= source.m_uiNumber;
	m_bType				= source.m_bType;
	m_strName			= source.m_strName;
	m_strUnits			= source.m_strUnits;
	m_uiPinmapIndex		= source.m_uiPinmapIndex;
	m_fLL				= source.m_fLL;
	m_nLLScaleFactor	= source.m_nLLScaleFactor;
	m_fHL				= source.m_fHL;
	m_nHLScaleFactor	= source.m_nHLScaleFactor;
	m_uiFlags			= source.m_uiFlags;
	m_uiLimitFlags		= source.m_uiLimitFlags;
	m_nResScaleFactor	= source.m_nResScaleFactor;

	return *this;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Comparison operator
//
// Argument(s) :
//
//	const C_Gate_DataModel_ParameterDef &s
//		reference to object to compare with this one
//
// Return type : TRUE if the 2 objects are identical, FALSE else
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_ParameterDef::operator==(const C_Gate_DataModel_ParameterDef &s)
{
	if(m_uiNumber != s.m_uiNumber)					return FALSE;
	if(m_bType != s.m_bType)						return FALSE;
	if(m_strName != s.m_strName)					return FALSE;
	if(m_strUnits != s.m_strUnits)					return FALSE;
	if(m_uiPinmapIndex != s.m_uiPinmapIndex)		return FALSE;
	if(m_fLL != s.m_fLL)							return FALSE;
	if(m_nLLScaleFactor != s.m_nLLScaleFactor)		return FALSE;
	if(m_fHL != s.m_fHL)							return FALSE;
	if(m_nHLScaleFactor != s.m_nHLScaleFactor)		return FALSE;
	if(m_uiFlags != s.m_uiFlags)					return FALSE;
	if(m_uiLimitFlags != s.m_uiLimitFlags)			return FALSE;
	if(m_nResScaleFactor != s.m_nResScaleFactor)	return FALSE;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Clear object data (free allocated ressources)
//
// Argument(s) :
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_ParameterDef::ClearData()
{
	// Free allocated ressources

	// Reset variables
	m_bType = ' ';
	m_uiNumber = 0;
	m_strName = "";
	m_strUnits = "";
	m_uiPinmapIndex = 0;
	m_fLL = -GEX_PLUGIN_INVALID_VALUE_FLOAT;
	m_nLLScaleFactor = 0;
	m_fHL = GEX_PLUGIN_INVALID_VALUE_FLOAT;
	m_nHLScaleFactor = 0;
	m_uiFlags = 0;
	m_uiLimitFlags = 0;
	m_nResScaleFactor = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : return the parameter type name
//
// Argument(s) :
//
// Return type : QString
/////////////////////////////////////////////////////////////////////////////////////////
QString C_Gate_DataModel_ParameterDef::GetParameterType(bool bShortDescription)
{
	QString strType = "";

	if(bShortDescription)
	{
		if(m_bType == 'P')
			strType = "P";

		if(m_bType == 'M')
			strType = "M";

		if(m_bType == 'F')
			strType = "F";

		if(m_bType == '*')
			strType = "Record";

		if(m_bType == '-')
			strType = "Bin";

		if(m_bType == 'x')
			strType = "Record";
	}
	else
	{
		if(m_bType == 'P')
			strType = "Parametric test";

		if(m_bType == 'M')
			strType = "Multi-Parametric test";

		if(m_bType == 'F')
			strType = "Functional test";

		if(m_bType == '*')
			strType = "Stdf Record";

		if(m_bType == '-')
			strType = "Bin result";

		if(m_bType == 'x')
			strType = "Deleted Record";
	}

	return strType;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : return the parameter type name
//
// Argument(s) :
//
// Return type : QString
/////////////////////////////////////////////////////////////////////////////////////////
void
C_Gate_DataModel_ParameterDef::
ComputeScaleFromStat(C_Gate_DataModel_ParameterStat* pParameterStat,
					 bool /*bUseLimitRange*/)
{
	// Check if allready computed ?
	if(m_nResScaleFactor != 0)
		return;

	m_nResScaleFactor = 0;

	// Compute the scale to applyf or each parameter value
	// From Range information

	float	fRange;
	float	fLowValue = pParameterStat->m_fMin;
	float	fHightValue = pParameterStat->m_fMax;
	int		nScaleStep = 3;

	if(m_strUnits.isEmpty())
		nScaleStep = 1;

	if(m_strUnits == "%")
	{
		nScaleStep = 1;

		// Check if the results are saved div 100 or not
		if(pParameterStat->m_fMax < 0)
			m_nResScaleFactor = 2;
	}


	if((m_uiLimitFlags&YIELD123_PARAMETERDEF_HASLL)
	&& (m_fLL < fLowValue))
		fLowValue = m_fLL;

	if((m_uiLimitFlags&YIELD123_PARAMETERDEF_HASHL)
	&& (m_fHL > fHightValue))
		fHightValue = m_fHL;

	fRange = fHightValue - fLowValue;

	fRange = fabs(fRange);
	int nScale = -12;

	if(fRange == 0)
	{
		m_nResScaleFactor = 0;
	}
	else
	{
		while(fRange > pow(10.0,nScale))
		{
			nScale += nScaleStep;
		}

		// adjust
		if((pow(10.0,nScale)-fRange) > (fRange - pow(10.0,nScale-nScaleStep)))
			nScale -= nScaleStep;

		m_nResScaleFactor = nScale;
	}


}


/////////////////////////////////////////////////////////////////////////////////////////
// Description : return the parameter type name
//
// Argument(s) :
//
// Return type : QString
/////////////////////////////////////////////////////////////////////////////////////////
QString C_Gate_DataModel_ParameterDef::PrintableValue(float fValue, int nNbDigit, bool bUseStatScale)
{

	QString strUnit = NormalizedUnit();
	QString strFormat, strScale;
	int		nScale = -12;
	int		nScaleStep = 3;

	if(strUnit.isEmpty())
		nScaleStep = 1;


	if(bUseStatScale)
		nScale = m_nResScaleFactor;
	else
	if(m_strUnits.isEmpty())
		nScale = 0;
	else
	if(fValue == 0.0)
		nScale = 0;
	else
	if(strUnit == "%")
		nScale = m_nResScaleFactor;
	else
	if(!strUnit.startsWith("Ohm") && strUnit.length() > 2)
		nScale = 0;
	else
	{
		while(fabs(fValue) > pow(10.0,nScale))
		{
			nScale += nScaleStep;
		}

		// adjust
		if((pow(10.0,nScale)-fabs(fValue)) > (fabs(fValue) - pow(10.0,nScale-nScaleStep)))
			nScale -= nScaleStep;
	}

	return FormatValue(fValue, nScale, !strUnit.isEmpty(), nNbDigit);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : return a formatted value
//
// Argument(s) :
//
// Return type : QString
/////////////////////////////////////////////////////////////////////////////////////////
QString C_Gate_DataModel_ParameterDef::FormatValue(float fValue)
{
	int nScale = m_nResScaleFactor;
	QString strUnit = NormalizedUnit();

	if(strUnit.isEmpty())
		nScale = 0;
	if(!strUnit.startsWith("Ohm") &&  strUnit.length() > 2)
		nScale = 0;

	if(fValue == 0.0)
		nScale = 0;

	return FormatValue(fValue, nScale);
}

QString C_Gate_DataModel_ParameterDef::FormatValue(float fValue, int nScale, bool bWithUnit, int nNbDigit)
{

	QString strFormat, strScale;
	float	fScaledValue;
	int nNbDecimal = -nNbDigit;

	if(fValue >= GEX_PLUGIN_INFINITE_VALUE_FLOAT)
		return "inf";

	if(fValue <= -GEX_PLUGIN_INFINITE_VALUE_FLOAT)
		return "-inf";

	fScaledValue = fValue / pow(10.0,nScale);

	while(fabs(fScaledValue) >= pow(10.0,nNbDecimal))
	{
		nNbDecimal++;
	}



	if(bWithUnit
	&& (m_strUnits == "%"))
		fScaledValue *= pow(10.0,nScale);

	// Add the sign char in the total
	if(	bWithUnit
	&& (fScaledValue < 0))
	{
		if(nNbDecimal > nNbDigit)
			strFormat = "%.1e";
		else
		if(nNbDecimal >= 0)
			strFormat = "%."+QString::number(nNbDigit-nNbDecimal)+"f";
		else
			strFormat = "%."+QString::number(nNbDigit-1)+"f";
	}
	else
	{
		if(nNbDecimal > nNbDigit)
			strFormat = "%.1e";
		else
		if(nNbDecimal >= 0)
			strFormat = "%."+QString::number(nNbDigit-nNbDecimal)+"f";
		else
			strFormat = "%."+QString::number(nNbDigit-1)+"f";
	}


	strFormat = strFormat.sprintf(strFormat.toLatin1(),fScaledValue);

	if(strFormat.contains(".") && !strFormat.contains("e"))
	{
		while(strFormat.endsWith("0"))
			strFormat = strFormat.left(strFormat.length()-1);

		if(strFormat.endsWith("."))
			strFormat = strFormat.left(strFormat.length()-1);
	}

	if(strFormat == "-0")
		strFormat = "0";

	if(bWithUnit)
		strFormat += " " + GetScaleUnitFromStat(nScale);

	return strFormat;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : return a normalized unit
//
// Argument(s) :
//
// Return type : QString
/////////////////////////////////////////////////////////////////////////////////////////
QString C_Gate_DataModel_ParameterDef::NormalizedUnit()
{
	QString strUnit = m_strUnits;

	if(strUnit.startsWith("sec",Qt::CaseInsensitive))
		strUnit = "s";
	else
	if(strUnit.startsWith("volt",Qt::CaseInsensitive))
		strUnit = "V";
	else
	if(strUnit.startsWith("amp",Qt::CaseInsensitive))
		strUnit = "A";
	else
	if(strUnit.startsWith("ohm",Qt::CaseInsensitive))
		strUnit = "Ohm";
	else
	if(strUnit.startsWith("newton",Qt::CaseInsensitive))
		strUnit = "N";
	else
	if(strUnit.startsWith("watt/h",Qt::CaseInsensitive))
		strUnit = "Wh";
	else
	if(strUnit.startsWith("watt",Qt::CaseInsensitive))
		strUnit = "W";
	else
	if(strUnit.startsWith("bel",Qt::CaseInsensitive))
		strUnit = "B";
	else
	if(strUnit.startsWith("decibel",Qt::CaseInsensitive))
		strUnit = "dB";
	else
	if(strUnit.startsWith("kelvin",Qt::CaseInsensitive))
		strUnit = "K";
	else
	if(strUnit.startsWith("celcius",Qt::CaseInsensitive))
		strUnit = "°C";
	else
	if(strUnit.startsWith("fahren",Qt::CaseInsensitive))
		strUnit = "°F";
	else
	if(strUnit.startsWith("joule",Qt::CaseInsensitive))
		strUnit = "J";
	else
	if(strUnit.startsWith("becquerel",Qt::CaseInsensitive))
		strUnit = "Bq";
	else
	if(strUnit.startsWith("henry",Qt::CaseInsensitive))
		strUnit = "H";
	else
	if(strUnit.startsWith("coulomb",Qt::CaseInsensitive))
		strUnit = "C";
	else
	if(strUnit.startsWith("tesla",Qt::CaseInsensitive))
		strUnit = "T";
	else
	if(strUnit.startsWith("gauss",Qt::CaseInsensitive))
		strUnit = "G";
	else
	if(strUnit.startsWith("maxwell",Qt::CaseInsensitive))
		strUnit = "Mx";
	else
	if(strUnit.startsWith("weber",Qt::CaseInsensitive))
		strUnit = "Wb";
	else
	if(strUnit.startsWith("hertz",Qt::CaseInsensitive))
		strUnit = "Hz";

	// Then take the good case (upper or Lower)
	if(strUnit.toLower() == "s")
		strUnit = "s";
	else
	if(strUnit.toLower() == "v")
		strUnit = "V";
	else
	if(strUnit.toLower() == "a")
		strUnit = "A";
	else
	if(strUnit.toLower() == "ohm")
		strUnit = "Ohm";
	else
	if(strUnit.toLower() == "n")
		strUnit = "N";
	else
	if(strUnit.toLower() == "wh")
		strUnit = "Wh";
	else
	if(strUnit.toLower() == "w")
		strUnit = "W";
	else
	if(strUnit.toLower() == "b")
		strUnit = "B";
	else
	if(strUnit.toLower() == "db")
		strUnit = "dB";
	else
	if(strUnit.toLower() == "k")
		strUnit = "K";
	else
	if(strUnit.toLower() == "°c")
		strUnit = "°C";
	else
	if(strUnit.toLower() == "°f")
		strUnit = "°F";
	else
	if(strUnit.toLower() == "j")
		strUnit = "J";
	else
	if(strUnit.toLower() == "bq")
		strUnit = "Bq";
	else
	if(strUnit.toLower() == "h")
		strUnit = "H";
	else
	if(strUnit.toLower() == "c")
		strUnit = "C";
	else
	if(strUnit.toLower() == "t")
		strUnit = "T";
	else
	if(strUnit.toLower() == "g")
		strUnit = "G";
	else
	if(strUnit.toLower() == "mx")
		strUnit = "Mx";
	else
	if(strUnit.toLower() == "wb")
		strUnit = "Wb";
	else
	if(strUnit.toLower() == "hz")
		strUnit = "Hz";

	return strUnit;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : return a formatted value
//
// Argument(s) :
//
// Return type : QString
/////////////////////////////////////////////////////////////////////////////////////////
QString C_Gate_DataModel_ParameterDef::GetScaleUnitFromStat()
{
	return GetScaleUnitFromStat(m_nResScaleFactor);
}

QString C_Gate_DataModel_ParameterDef::GetScaleUnitFromStat(int nScale)
{

	QString strScale;

	QString strUnit = NormalizedUnit();

	if(nScale == 0)
		return strUnit;

	if(strUnit.isEmpty())
	{
		strScale = "10e" + QString::number(nScale);
	}
	else
	if(!strUnit.startsWith("Ohm") && strUnit.length() > 2)
	{
		strScale = "10e" + QString::number(nScale);
	}
	else
	{
		switch(nScale)
		{
			case -15: // Fento
				strScale = 'f';
				break;
			case -12: // Pico
				strScale = 'p';
				break;
			case -9: // Nano
				strScale = 'n';
				break;
			case -6: // Micro
				strScale = 'u';
				break;
			case -3: // Milli
				strScale = 'm';
				break;
			case 3: // Kilo
				strScale = 'K';
				break;
			case 6: // Mega
				strScale = 'M';
				break;
			case 9: // Giga
				strScale = 'G';
				break;
			case 12: // Tera
				strScale = 'T';
				break;
		}
	}

	return strScale + strUnit;
}


/////////////////////////////////////////////////////////////////////////////////////////
// PARAMETERSET OBJECT
/////////////////////////////////////////////////////////////////////////////////////////
// ERROR MAP
GBEGIN_ERROR_MAP(C_Gate_DataModel_ParameterSet)
	GMAP_ERROR(eMalloc,"Failed to allocate memory for parameter definitions")
	GMAP_ERROR(eParameterIndexOverflow,"Parameter index overflow (index=%d, nb of parameters=%d)")
	GMAP_ERROR(eNotAllParametersDefined,"Not all parameters are defined: %d parameters declared, %d parameters defined")
GEND_ERROR_MAP(C_Gate_DataModel_ParameterSet)

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Constructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_ParameterSet::C_Gate_DataModel_ParameterSet()
{
	m_pclParameters = NULL;

	ClearData();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Destructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_ParameterSet::~C_Gate_DataModel_ParameterSet()
{
	ClearData();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Comparison operator
//
// Argument(s) :
//
//	const C_Gate_DataModel_ParameterSet &s
//		reference to object to compare with this one
//
// Return type : TRUE if the 2 objects are identical, FALSE else
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_ParameterSet::operator==(const C_Gate_DataModel_ParameterSet &s)
{
	if(m_uiNbParameters != s.m_uiNbParameters)				return FALSE;
	if(m_uiNbValidParameters != s.m_uiNbValidParameters)	return FALSE;

	for(unsigned int uiIndex=0; uiIndex<m_uiNbParameters; uiIndex++)
	{
		if(m_pclParameters[uiIndex] != s.m_pclParameters[uiIndex])
			return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Clear object data (free allocated ressources)
//
// Argument(s) :
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_ParameterSet::ClearData()
{
	// Free allocated ressources
	if(m_pclParameters != NULL)
	{
		delete [] m_pclParameters;
		m_pclParameters = NULL;
	}

	// Reset variables
	m_uiParameterSetNb = 0;
	m_uiNbParameters = 0;
	m_uiNbValidParameters = 0;
	m_uiNbCustomParameters = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Initialize parameter set
//
// Argument(s) :
//
//	unsigned int uiNbParameters
//		Nb of parameters
//
// Return type : TRUE if initialization successful, FALSE else
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_ParameterSet::Init(unsigned int uiNbParameters)
{
	// Init some members
	m_uiNbParameters = uiNbParameters;
	m_uiNbValidParameters = 0;
	m_uiNbCustomParameters = 0;

	// Allocate memory for parameters
	m_pclParameters = new C_Gate_DataModel_ParameterDef[uiNbParameters];
	if(m_pclParameters == NULL)
	{
		GSET_ERROR0(C_Gate_DataModel_ParameterSet, eMalloc, NULL);
		ClearData();
		return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Set parameter definition
//
// Argument(s) :
//
//	const Gate_ParameterDef & ParameterDefinition
//		Parameter definition structure from plugin event
//
// Return type : TRUE if parameter definition correctly set, FALSE else
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_ParameterSet::SetParameter(const Gate_ParameterDef & ParameterDefinition)
{
	// Check for overflow
	if((ParameterDefinition.m_nParameterIndex >= (int)m_uiNbParameters)
	|| (ParameterDefinition.m_nParameterIndex < 0))
	{
		GSET_ERROR2(C_Gate_DataModel_ParameterSet, eParameterIndexOverflow, NULL, ParameterDefinition.m_nParameterIndex, m_uiNbParameters);
		return FALSE;
	}

	int nIndex = ParameterDefinition.m_nParameterIndex;
	if(m_pclParameters[nIndex].m_bType == ' ')
		m_uiNbValidParameters++;

	m_pclParameters[nIndex].m_uiNumber = ParameterDefinition.m_nParameterNumber;
	m_pclParameters[nIndex].m_bType = ParameterDefinition.m_bTestType;
	m_pclParameters[nIndex].m_strName = ParameterDefinition.m_strName;
	m_pclParameters[nIndex].m_uiPinmapIndex = ParameterDefinition.m_nPinArrayIndex;
	m_pclParameters[nIndex].m_strUnits = ParameterDefinition.m_strUnits.simplified();
	m_pclParameters[nIndex].m_uiFlags = ParameterDefinition.m_uiFlags;

	if((m_pclParameters[nIndex].m_bType == '*') || (m_pclParameters[nIndex].m_bType == '-') || (m_pclParameters[nIndex].m_bType == 'x'))
		m_uiNbCustomParameters++;

	if((m_pclParameters[nIndex].m_bType == '*') && (m_pclParameters[nIndex].m_strName.indexOf("[Record#") >= 0))
	{
		m_pclParameters[nIndex].m_uiFlags = 0;
		SetStdfRecordInformation(m_pclParameters[nIndex].m_strName,
								 m_pclParameters[nIndex].m_uiNumber,
								 m_pclParameters[nIndex].m_strName,
								 m_pclParameters[nIndex].m_strUnits,
								 m_pclParameters[nIndex].m_uiFlags);
		return TRUE;
	}
	// Check between first parameter (*) if have some info
	for(nIndex = m_uiNbParameters-1; nIndex > ParameterDefinition.m_nParameterIndex; nIndex--)
	{
		if(m_pclParameters[nIndex].m_bType == '-')
			break;
		if(m_pclParameters[nIndex].m_bType == 'x')
			continue;
		if((m_pclParameters[nIndex].m_uiNumber == (unsigned int) ParameterDefinition.m_nParameterNumber)
		&& (m_pclParameters[nIndex].m_strName.toUpper() == ParameterDefinition.m_strName.toUpper()))
		{
			m_pclParameters[ParameterDefinition.m_nParameterIndex].m_uiFlags |= m_pclParameters[nIndex].m_uiFlags;
			// Desable this parameter
			m_pclParameters[nIndex].m_bType = 'x';
		}

	}

	// Restart the update
	nIndex = ParameterDefinition.m_nParameterIndex;


	if(ParameterDefinition.m_bHasLowL)
	{
		m_pclParameters[nIndex].m_uiLimitFlags |= YIELD123_PARAMETERDEF_HASLL;
		m_pclParameters[nIndex].m_fLL = ParameterDefinition.m_lfLowL;
	}

	if(!ParameterDefinition.m_bStrictLowL)
		m_pclParameters[nIndex].m_uiLimitFlags |= YIELD123_PARAMETERDEF_LLNOTSTRICT;
	if(ParameterDefinition.m_bHasHighL)
	{
		m_pclParameters[nIndex].m_uiLimitFlags |= YIELD123_PARAMETERDEF_HASHL;
		m_pclParameters[nIndex].m_fHL = ParameterDefinition.m_lfHighL;
	}
	if(!ParameterDefinition.m_bStrictHighL)
		m_pclParameters[nIndex].m_uiLimitFlags |= YIELD123_PARAMETERDEF_HLNOTSTRICT;
	m_pclParameters[nIndex].m_nLLScaleFactor = ParameterDefinition.m_nLowLScaleFactor;
	m_pclParameters[nIndex].m_nHLScaleFactor = ParameterDefinition.m_nHighLScaleFactor;
	m_pclParameters[nIndex].m_nResScaleFactor = ParameterDefinition.m_nResultScaleFactor;

	// Check if test has parametric units
	if(HasParametricUnits(ParameterDefinition.m_strUnits) == TRUE)
		m_pclParameters[nIndex].m_uiLimitFlags |= YIELD123_PARAMETERDEF_PARAMETRICUNITS;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Checks if specified units are of parametric type (ie V, A...)
//				 o Following units are considered parametric:
//				   volt (V), ampere (A), ohm (omega), farad (F), hertz (Hz), decibel (db),
//				   watt (W), second (s), kelvin (K), degree (°C), siemens (S), coulomb (C)
// Argument(s) :
//
//	const QString & strUnits
//		Units to check
//
// Return type : bool
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_ParameterSet::HasParametricUnits(const QString & strUnit)
{
	QString strCaptured;

	// Construct a string with all whitespaces removed
	QString	strString = strUnit.simplified();
	strString.remove(' ');

	// Construct regular expressions for unit checking
	QRegExp	rxUnitFullNames("(volt|ampere|ohm|farad|hertz|decibel|watt|second|kelvin|degree|siemens|coulomb)");
	QRegExp	rxUnitVariants("(volts|amperes|amp|amps|ohms|farads|hertzs|hz|hzs|decibels|db|dbs|watts|seconds|sec|secs|kelvins|degrees|deg|degs|coulombs)");
	QRegExp	rxScalePrefixes("^(f|femto|p|pico|n|nano|u|micro|m|milli|k|kilo|m|mega|g|giga|t|tera)");
	QRegExp rxUnitAbreviations("^(v|a|o|f|w|s|k|c)$");

	rxUnitFullNames.setCaseSensitivity(Qt::CaseInsensitive);
	rxUnitVariants.setCaseSensitivity(Qt::CaseInsensitive);
	rxScalePrefixes.setCaseSensitivity(Qt::CaseInsensitive);
	rxUnitAbreviations.setCaseSensitivity(Qt::CaseInsensitive);

	// Check if string contains unit names
	if(rxUnitFullNames.exactMatch(strString))
	{
		strCaptured = rxUnitFullNames.cap();
		return TRUE;
	}

	// Check if unit string contains unit variants
	if(rxUnitVariants.exactMatch(strString))
	{
		strCaptured = rxUnitVariants.cap();
		return TRUE;
	}

	// Check if unit string is an exact abreviation
	if(rxUnitAbreviations.exactMatch(strString))
	{
		strCaptured = rxUnitAbreviations.cap();
		return TRUE;
	}

	// Check if unit string starts with scale prefix
	if(rxScalePrefixes.exactMatch(strString))
	{
		strCaptured = rxScalePrefixes.cap();

		int nLength = rxScalePrefixes.matchedLength();
		QString strSubString = strString.right(strString.length()-nLength);

		// Check if unit sub-string is an exact abreviation
		if(rxUnitAbreviations.exactMatch(strSubString))
		{
			strCaptured = rxUnitAbreviations.cap();
			return TRUE;
		}
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_ParameterSet::SetStdfRecordInformation(const QString strWarning, uint &uiNumber, QString &strName, QString &strType, uint &uiFlags)
{

	// [Record#MIR]
	strType = strWarning.section("#",1).section("]",0,0);

	if(strWarning.indexOf("tests with the same definition") > 0)
	{
		// "Found parametric and functional tests with the same definition : test number %d (%s)."
		uiNumber = strWarning.section("test number",1).section("(",0,0).simplified().toUInt();
		strName = strWarning.section("(",1).section(")",0,0);
		uiFlags |= YIELD123_FLAGS_AUDITFILE_DuplicatedTests;
	}
	else
	if(strWarning.indexOf("Found record outside of PIR/PRR bloc") > 0)
	{
		// "Found record outside of PIR/PRR bloc. Some results were ignored for test number %d (%s)."
		uiNumber = strWarning.section("test number",1).section("(",0,0).simplified().toUInt();
		strName = strWarning.section("(",1).section(")",0,0);
		uiFlags |= YIELD123_FLAGS_AUDITFILE_TestsResultOutside;
	}
	else
	if(strWarning.indexOf("RTN_ICNT <> RSLT_CNT for the Multi-Parametric") > 0)
	{
		// "RTN_ICNT <> RSLT_CNT for the Multi-Parametric test number %d (%s)."
		uiNumber = strWarning.section("test number",1).section("(",0,0).simplified().toUInt();
		strName = strWarning.section("(",1).section(")",0,0);
		uiFlags |= YIELD123_FLAGS_AUDITFILE_MprIndex;
	}
	else
	if(strWarning.indexOf("differs from WRR count") > 0)
	{
		// "WIR count (%d) differs from WRR count (%d)."
		uiNumber = (uint) -1;
		strName = strType;
		uiFlags |= YIELD123_FLAGS_AUDITMISCELLANEOUS_RecordsWirWrrCount;
	}
	else
	if(strWarning.indexOf("More than one limit definition per site") > 0)
	{
		// "More than one limit definition per site for test number %d (%s)."
		uiNumber = strWarning.section("test number",1).section("(",0,0).simplified().toUInt();
		strName = strWarning.section("(",1).section(")",0,0);
		uiFlags |= YIELD123_FLAGS_AUDITFILE_DiscrepanciesTestLimits;
	}
	else
	if(strWarning.indexOf("mandatory records are missing") > 0)
	{
		// "Some mandatory records are missing in the STDF file (%s)."
		uiNumber = (uint) -1;
		strName = strWarning.section("(",1).section(")",0,0);
		uiFlags |= YIELD123_FLAGS_AUDITFILE_MissingRecords;
	}
	else
	if(strWarning.indexOf("mandatory fields are empty") > 0)
	{
		// "Some mandatory fields are empty (%s)."
		uiNumber = (uint) -1;
		strName = strType;
		if(strWarning.indexOf("PRODUCT_NAME") > 0)
			uiFlags |= YIELD123_FLAGS_AUDITMISCELLANEOUS_MissingProductID;
		if(strWarning.indexOf("LOT_ID") > 0)
			uiFlags |= YIELD123_FLAGS_AUDITMISCELLANEOUS_MissingLotID;
		if(strWarning.indexOf("WAFER_ID") > 0)
			uiFlags |= YIELD123_FLAGS_AUDITMISCELLANEOUS_MissingWaferID;
	}
	else
	if(strWarning.indexOf("Wafer without valid PART_X PART_Y") > 0)
	{
		// "Wafer without valid PART_X PART_Y."
		uiNumber = (uint) -1;
		strName = strType;
		uiFlags |= YIELD123_FLAGS_AUDITMISCELLANEOUS_MissingWaferCoordinates;
	}
	else
	if(strWarning.indexOf("Final test with valid PART_X PART_Y") > 0)
	{
		// "Final test with valid PART_X PART_Y."
		//uiNumber = (uint) -1;
		//strName = strType;
		//uiFlags |= YIELD123_FLAGS_AUDITMISCELLANEOUS_MissingWaferCoordinates;
	}
	else
	if(strWarning.indexOf("WaferID has been overloaded") > 0)
	{
		// "WaferID has been overloaded with value %s (through dbkeys mapping) for a multi-wafer file."
		uiNumber = (uint) -1;
		strName = strType;
		uiFlags |= YIELD123_FLAGS_AUDITMISCELLANEOUS_RecordsWirOverloaded;
	}
	else
	if(strWarning.indexOf("Multiple heads in the same data") > 0)
	{
		// "Multiple heads in the same data file is not supported."
		uiNumber = (uint) -1;
		strName = strWarning;
		uiFlags |= YIELD123_FLAGS_AUDITMISCELLANEOUS_RecordsWirMultihead;
	}
		else
	if(strWarning.indexOf("Multiple W-Sort wafers in the same data") > 0)
	{
		// "Multiple W-Sort wafers in the same data file is not supported."
		uiNumber = (uint) -1;
		strName = strWarning;
		uiFlags |= YIELD123_FLAGS_AUDITMISCELLANEOUS_RecordsWirCount;
	}
	else
	if(strWarning.indexOf("Multiple SDR records in the same data") > 0)
	{
		// "Multiple SDR records in the same data file is not supported."
		uiNumber = (uint) -1;
		strName = strWarning;
		uiFlags |= YIELD123_FLAGS_AUDITMISCELLANEOUS_RecordsSdrCount;
	}
	else
	if(strWarning.indexOf("Not same Parts count between PCR") > 0)
	{
		// "Not same Parts count between PCR(%d) record and Parts Results(%d)"
		uiNumber = (uint) -1;
		strName = strWarning;
		uiFlags |= YIELD123_FLAGS_AUDITMISCELLANEOUS_RecordsPcrPartsCount;
	}
	else
	if(strWarning.indexOf("MIR record appears multiple time") > 0)
	{
		// "MIR record appears multiple time. Only one MIR is allowed! This record was ignored."
		uiNumber = (uint) -1;
		strName = strWarning;
		uiFlags |= YIELD123_FLAGS_AUDITMISCELLANEOUS_RecordsMirCount;
	}


	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
QStringList C_Gate_DataModel_ParameterSet::GetStdfRecordInformation(const uint uiFlags, int uiParameterIndex)
{
	QStringList lstWarning;
	QString		strMessage;

	if(uiFlags & YIELD123_FLAGS_AUDITFILE_DuplicatedTests)
	{
		lstWarning.append("Found parametric and functional tests with the same definition (same name and same number)");
	}

	if(uiFlags & YIELD123_FLAGS_AUDITFILE_DiscrepanciesTestLimits)
	{
		lstWarning.append("More than one limit definition per site");
	}

	if(uiFlags & YIELD123_FLAGS_AUDITFILE_MissingTestLimits)
	{

		if(uiParameterIndex > -1)
		{
			if(m_pclParameters[uiParameterIndex].m_uiLimitFlags & YIELD123_PARAMETERDEF_HASHL)
				strMessage = "Missing Low Limit detected";
			else
			if(m_pclParameters[uiParameterIndex].m_uiLimitFlags & YIELD123_PARAMETERDEF_HASLL)
				strMessage = "Missing High Limit detected";
			else
				strMessage = "Missing High & Low Limits detected";
		}
		else
			strMessage = "Missing some limit definition";
		lstWarning.append(strMessage);
	}

	if(uiFlags & YIELD123_FLAGS_AUDITFILE_TestsResultOutside)
	{
		lstWarning.append("Found record outside of PIR/PRR bloc. Some results were ignored");
	}

	if(uiFlags & YIELD123_FLAGS_AUDITFILE_MprIndex)
	{
		lstWarning.append("RTN_ICNT <> RSLT_CNT");
	}

	if(uiFlags & YIELD123_FLAGS_AUDITMISCELLANEOUS_RecordsWirWrrCount)
	{
		lstWarning.append("WIR count differs from WRR count");
	}

	if(uiFlags & YIELD123_FLAGS_AUDITMISCELLANEOUS_RecordsWirOverloaded)
	{
		lstWarning.append("WaferID has been overloaded (through dbkeys mapping) for a multi-wafer file.");
	}

	if(uiFlags & YIELD123_FLAGS_AUDITMISCELLANEOUS_RecordsWirCount)
	{
		lstWarning.append("Multiple W-Sort wafers in the same data file is not supported.");
	}

	if(uiFlags & YIELD123_FLAGS_AUDITMISCELLANEOUS_RecordsWirMultihead)
	{
		lstWarning.append("Multiple heads in the same data file is not supported.");
	}

	if(uiFlags & YIELD123_FLAGS_AUDITMISCELLANEOUS_RecordsSdrCount)
	{
		lstWarning.append("Multiple SDR records in the same data file is not supported.");
	}

	if(uiFlags & YIELD123_FLAGS_AUDITMISCELLANEOUS_RecordsPcrPartsCount)
	{
		lstWarning.append("Not same Parts count between PCR record and Parts Results");
	}

	if(uiFlags & YIELD123_FLAGS_AUDITMISCELLANEOUS_RecordsMirCount)
	{
		lstWarning.append("MIR record appears multiple time. Duplicated records were ignored.");
	}

	if(uiFlags & YIELD123_FLAGS_AUDITMISCELLANEOUS_MissingWaferCoordinates)
	{
		lstWarning.append("Missing Die XY coordinates. Your file should have its fields PRR.X_COORD and PRR.Y_COORD set to valid die locations.");
	}

	if(uiFlags & YIELD123_FLAGS_AUDITFILE_MissingRecords)
	{
		lstWarning.append("Record is missing");
	}

	if(uiFlags & YIELD123_FLAGS_AUDITMISCELLANEOUS_MissingProductID)
	{
		lstWarning.append("No Product ID defined in your file! Your file should have its fields MIR.PART_TYP set to a valid Product name.");
	}

	if(uiFlags & YIELD123_FLAGS_AUDITMISCELLANEOUS_MissingLotID)
	{
		lstWarning.append("No LotID defined in your file! Your file should have its fields MIR.LOT_ID set to a valid Lot ID.");
	}

	if(uiFlags & YIELD123_FLAGS_AUDITMISCELLANEOUS_MissingWaferID)
	{
		lstWarning.append("No WaferID defined in your file! Your file should have its fields WIR.WAFER_ID set to a valid Wafer ID.");
	}

	if(uiFlags & YIELD123_FLAGS_AUDITFILE_InvalidRecords)
	{
		lstWarning.append("Invalid record");
	}

	if(uiFlags & YIELD123_FLAGS_AUDITFILE_EndRecords)
	{
		lstWarning.append("Unexpected end of record");
	}

	return lstWarning;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Check if Parameter set is OK
//
// Argument(s) :
//
// Return type : TRUE if Parameter set is OK, FALSE else
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_ParameterSet::Check()
{
	if(m_uiNbValidParameters != m_uiNbParameters)
	{
		GSET_ERROR2(C_Gate_DataModel_ParameterSet, eNotAllParametersDefined, NULL, m_uiNbParameters, m_uiNbValidParameters);
		return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Dumps the data structure to an ascii file (CSV format)
//
// Argument(s) :
//
//	const QTextStream & hDumpFile
//		Dump file stream
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_ParameterSet::Dump(QTextStream & hDumpFile)
{
	// Write header
	hDumpFile << endl;
	hDumpFile << "PARAMETERSET" << endl;
	hDumpFile << "============" << endl;
	hDumpFile << "ParameterSet Nb" << CSV_SEPARATOR << QString::number(m_uiParameterSetNb) << endl;
	hDumpFile << "Nb parameters" << CSV_SEPARATOR << QString::number(m_uiNbParameters) << endl;
	hDumpFile << "Nb valid parameters" << CSV_SEPARATOR << QString::number(m_uiNbValidParameters) << endl;
	hDumpFile << endl;

	//hDumpFile << "Index" << CSV_SEPARATOR;
	hDumpFile << "Number" << CSV_SEPARATOR;
	hDumpFile << "Type" << CSV_SEPARATOR;
	hDumpFile << "Name" << CSV_SEPARATOR;
	hDumpFile << "Units" << CSV_SEPARATOR;
	//hDumpFile << "Has LL" << CSV_SEPARATOR;
	hDumpFile << "LL" << CSV_SEPARATOR;
	//hDumpFile << "LL scale factor" << CSV_SEPARATOR;
	//hDumpFile << "Has HL" << CSV_SEPARATOR;
	hDumpFile << "HL" << CSV_SEPARATOR;
	//hDumpFile << "HL scale factor" << CSV_SEPARATOR;
	//hDumpFile << "Pinmap Index" << CSV_SEPARATOR;
	//hDumpFile << "Flags" << CSV_SEPARATOR;
	//hDumpFile << "Limit class" << CSV_SEPARATOR;
	//hDumpFile << "Res scale factor" << CSV_SEPARATOR;
	hDumpFile << endl;

	// Dump all Parameters
	for(unsigned int uiIndex=0; uiIndex<m_uiNbParameters; uiIndex++)
	{
		QChar cTestType((char)(m_pclParameters[uiIndex].m_bType));
		//hDumpFile << QString::number(uiIndex) << CSV_SEPARATOR;
		hDumpFile << QString::number(m_pclParameters[uiIndex].m_uiNumber) << CSV_SEPARATOR;
		hDumpFile << cTestType << CSV_SEPARATOR;
		hDumpFile << GetCsvString(m_pclParameters[uiIndex].m_strName) << CSV_SEPARATOR;
		hDumpFile << GetCsvString(m_pclParameters[uiIndex].m_strUnits) << CSV_SEPARATOR;
		//hDumpFile << ((m_pclParameters[uiIndex].m_uiLimitFlags & YIELD123_PARAMETERDEF_HASLL) ? 'Y' : 'N') << CSV_SEPARATOR;
		hDumpFile << GetCsvString(QString::number(m_pclParameters[uiIndex].m_fLL)) << CSV_SEPARATOR;
		//hDumpFile << QString::number(m_pclParameters[uiIndex].m_nLLScaleFactor) << CSV_SEPARATOR;
		//hDumpFile << ((m_pclParameters[uiIndex].m_uiLimitFlags & YIELD123_PARAMETERDEF_HASHL) ? 'Y' : 'N') << CSV_SEPARATOR;
		hDumpFile << GetCsvString(QString::number(m_pclParameters[uiIndex].m_fHL)) << CSV_SEPARATOR;
		//hDumpFile << QString::number(m_pclParameters[uiIndex].m_nHLScaleFactor) << CSV_SEPARATOR;
		//hDumpFile << QString::number(m_pclParameters[uiIndex].m_uiPinmapIndex) << CSV_SEPARATOR;
		//hDumpFile << QString::number(m_pclParameters[uiIndex].m_uiFlags);
		//hDumpFile << QString::number(m_pclParameters[uiIndex].m_uiLimitFlags);
		//hDumpFile << QString::number(m_pclParameters[uiIndex].m_nResScaleFactor) << CSV_SEPARATOR;
		hDumpFile << endl;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// PARAMETERSETLIST OBJECT
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// Description : Constructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_ParameterSetList::C_Gate_DataModel_ParameterSetList(): QList<C_Gate_DataModel_ParameterSet*>()
{
	// Init member variables
	m_uiParameterSetNb = 0;

	m_iCurrentPos = -1;
	// Set autodelete flag to true
	m_bAutoDelete = TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Sets the ParameterSet nb of the item and appends it to the ptrlist
//
// Argument(s) :
//
//	C_Gate_DataModel_ParameterSet* pParameterSet
//		ParameterSet to append
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_ParameterSetList::appendItem(C_Gate_DataModel_ParameterSet* pParameterSet)
{
	pParameterSet->m_uiParameterSetNb = m_uiParameterSetNb++;
	append(pParameterSet);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Resets ParameterSet nb, and clears all items of ptr list
//
// Argument(s) :
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_ParameterSetList::clearItems()
{
	m_uiParameterSetNb = 0;
	if(m_bAutoDelete)
	{
		while(!isEmpty())
			delete takeFirst();
	}

	clear();
	m_iCurrentPos = -1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// TESTINGSTAGE OBJECT
/////////////////////////////////////////////////////////////////////////////////////////
// ERROR MAP
GBEGIN_ERROR_MAP(C_Gate_DataModel_TestingStage)
	GMAP_ERROR(eUnknownDataOrigin,"Unknown data origin (%s). Couldn't determine testing stage")
GEND_ERROR_MAP(C_Gate_DataModel_TestingStage)

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Constructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_TestingStage::C_Gate_DataModel_TestingStage(C_Gate_DataModel *pParent, C_Gate_DataModel_Progress *pProgress) : C_Gate_DataModel_Object(eData_TestingStage, pParent,pProgress)
{
	ClearData();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Destructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_TestingStage::~C_Gate_DataModel_TestingStage()
{
	ClearData();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Comparison operator
//
// Argument(s) :
//
//	const C_Gate_DataModel_TestingStage &s
//		reference to object to compare with this one
//
// Return type : TRUE if the 2 objects are identical, FALSE else
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_TestingStage::operator==(const C_Gate_DataModel_TestingStage &s)
{
	return (m_uiStageID == s.m_uiStageID);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Clear object data (free allocated ressources)
//
// Argument(s) :
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_TestingStage::ClearData()
{
	// Free allocated ressources
	ClearLots();
	m_pParameterSets.clearItems();

	// Reset variables
	m_clFromDate.setTime_t(0);
	m_clToDate.setTime_t(0);
	m_uiStageID = YIELD123_TESTINGSTAGE_ID_UNKNOWN;
	m_strStageName = YIELD123_TESTINGSTAGE_NAME_UNKNOWN;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Cleanup testing stage (remove empty lots...)
//
// Argument(s) :
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_TestingStage::Cleanup()
{
	C_Gate_DataModel_Lot *pLot;

	pLot = GetFirstLot();
	while(pLot)
	{
		pLot->Cleanup();
		if(pLot->NbSublots() == 0)
			pLot = RemoveCurrentLot();
		else
			pLot = GetNextLot();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description :
//
// Argument(s) :
//
//	const QString & strDataOrigin
//		String describing the origin of the dataset
//
// Return type : TRUE if Stage ID correctly set, FALSE else
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataModel_TestingStage::SetStageID(const QString & strDataOrigin)
{
	if(strDataOrigin == "ETEST")
	{
		m_uiStageID = YIELD123_TESTINGSTAGE_ID_ETEST;
		m_strStageName = YIELD123_TESTINGSTAGE_NAME_ETEST;
		return TRUE;
	}

	if(strDataOrigin == "WTEST")
	{
		m_uiStageID = YIELD123_TESTINGSTAGE_ID_WTEST;
		m_strStageName = YIELD123_TESTINGSTAGE_NAME_WTEST;
		return TRUE;
	}

	if(strDataOrigin == "FTEST")
	{
		m_uiStageID = YIELD123_TESTINGSTAGE_ID_FTEST;
		m_strStageName = YIELD123_TESTINGSTAGE_NAME_FTEST;
		return TRUE;
	}

	// Undefined data origin
	GSET_ERROR1(C_Gate_DataModel_TestingStage, eUnknownDataOrigin, NULL, strDataOrigin.toLatin1().data());
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Udates From and To dates of the testing stage
//
// Argument(s) :
//
//	long lStartTime
//		Start time
//
//	long lFinishTime
//		Finish time
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_TestingStage::UpdateFromToDates(long lStartTime, long lFinishTime)
{
	if((m_clFromDate.toTime_t() == 0) || (lStartTime < (long)m_clFromDate.toTime_t()))
		m_clFromDate.setTime_t(lStartTime);
	if((m_clToDate.toTime_t() == 0) || (lFinishTime > (long)m_clToDate.toTime_t()))
		m_clToDate.setTime_t(lFinishTime);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Get lot to be used to add data to be received.
//				 Create it if no corresponding lot already in the list.
//
// Argument(s) :
//
//	const Gate_LotDef & LotDefinition
//		Lot definition object
//
// Return type : Ptr to lot object to be used, NULL if an error occured
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_Lot* C_Gate_DataModel_TestingStage::GetLot(const Gate_LotDef & LotDefinition)
{
	// Check if we already have data for this lot
	C_Gate_DataModel_Lot* pLot = GetFirstLot();
	while(pLot)
	{
		if(pLot->m_strLotID == LotDefinition.m_strLotID)
			break;
		pLot = GetNextLot();
	}

	// If no lot found, insert a new one
	if(!pLot)
	{
		pLot = new C_Gate_DataModel_Lot(this,m_pProgress);
		pLot->m_strLotID = LotDefinition.m_strLotID;
		AddLot(pLot);
	}

	// Update lot's From and To dates
	pLot->UpdateFromToDates(LotDefinition.m_lStartTime, LotDefinition.m_lFinishTime);

	return pLot;
}


/////////////////////////////////////////////////////////////////////////////////////////
// Description : Get Parameter set to be used to add data to be received.
//				 Add it if no corresponding Parameter set already in the list.
//
// Argument(s) :
//
//	const C_Gate_DataModel_ParameterSet* pNewParameterSet
//		New Parameter set
//
//	bool* pbAlreadyInList
//		Set to TRUE if the parameter set is already in the list
//
// Return type : Ptr to Parameter set object to be used, NULL if an error occured
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_ParameterSet* C_Gate_DataModel_TestingStage::GetParameterSet(C_Gate_DataModel_ParameterSet* pNewParameterSet, bool* pbAlreadyInList)
{
	*pbAlreadyInList = FALSE;

	// Check if this Parameter set is already in list
	C_Gate_DataModel_ParameterSet* pParameterSet = m_pParameterSets.GetFirst();
	while(pParameterSet)
	{
		if(*pParameterSet == *pNewParameterSet)
		{
			*pbAlreadyInList = TRUE;
			return pParameterSet;
		}
		pParameterSet = m_pParameterSets.GetNext();
	}

	// Parameter set not found in list, add the new one
	m_pParameterSets.appendItem(pNewParameterSet);
	return pNewParameterSet;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Dumps the data structure to an ascii file (CSV format)
//
// Argument(s) :
//
//	const QTextStream & hDumpFile
//		Dump file stream
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel_TestingStage::Dump(QTextStream & hDumpFile)
{
	// Write header
	hDumpFile << endl;
	hDumpFile << "TESTINGSTAGE" << endl;
	hDumpFile << "============" << endl;
	hDumpFile << "StageID" << CSV_SEPARATOR << QString::number(m_uiStageID) << endl;
	hDumpFile << "Stage Name" << CSV_SEPARATOR << m_strStageName << endl;
	hDumpFile << "From" << CSV_SEPARATOR << m_clFromDate.toString("dd MMM yyyy hh:mm:ss") << endl;
	hDumpFile << "To" << CSV_SEPARATOR << m_clToDate.toString("dd MMM yyyy hh:mm:ss") << endl;
	hDumpFile << "Nb parameter sets" << CSV_SEPARATOR << m_pParameterSets.count() << endl;
	hDumpFile << "Nb lots" << CSV_SEPARATOR << NbLots() << endl;

	// Dump all ParameterSets
	C_Gate_DataModel_ParameterSet* pParameterSet = m_pParameterSets.GetFirst();
	while(pParameterSet)
	{
		pParameterSet->Dump(hDumpFile);
		pParameterSet = m_pParameterSets.GetNext();
	}

	// Dump all lots
	C_Gate_DataModel_Lot* pLot = GetFirstLot();
	while(pLot)
	{
		pLot->Dump(hDumpFile);
		pLot = GetNextLot();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// DATA OBJECT
/////////////////////////////////////////////////////////////////////////////////////////
// ERROR MAP
GBEGIN_ERROR_MAP(C_Gate_DataModel)
	GMAP_ERROR(eMultipleProducts,"The data comes from several products (%s, %s)")
	GMAP_ERROR(eTestingStage,"Testing stage error.")
GEND_ERROR_MAP(C_Gate_DataModel)

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Constructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel::C_Gate_DataModel(C_Gate_DataModel_Progress *pProgress) : C_Gate_DataModel_Object(eData, NULL, pProgress)
{
	ClearData();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Destructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel::~C_Gate_DataModel()
{
	ClearData();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Clear object data (free allocated ressources)
//
// Argument(s) :
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel::ClearData()
{
	// Free allocated ressources
	ClearTestingStages();

	// Reset variables
	m_clFromDate.setTime_t(0);
	m_clToDate.setTime_t(0);
	m_strProductID = "";
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Cleanup data set (remove empty testing stages...)
//
// Argument(s) :
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel::Cleanup()
{
	C_Gate_DataModel_TestingStage *pTestingStage;

	pTestingStage = GetFirstTestingStage();
	while(pTestingStage)
	{
		pTestingStage->Cleanup();
		if(pTestingStage->NbLots() == 0)
			pTestingStage = RemoveCurrentTestingStage();
		else
			pTestingStage = GetNextTestingStage();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Return testing stage corresponding to a product ID and a specified data origin
//				 Create the testing stage if no corresponding found in the list.
//
// Argument(s) :
//
//	const Gate_LotDef & LotDefinition
//		Lot definition object
//
// Return type : ptr on corresponding testing stage, NULL if an error occured
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataModel_TestingStage* C_Gate_DataModel::GetTestingStage(const Gate_LotDef & LotDefinition)
{
	// Initialize ProgressBar
	if(m_pProgress)
		m_pProgress->Init("Loading data in progress ...", LotDefinition.m_uiProgramRuns+10);

	// Create a testing stage corresponding to specified data origin
	C_Gate_DataModel_TestingStage* pclTestingStage = new C_Gate_DataModel_TestingStage(this,m_pProgress);
	if(pclTestingStage->SetStageID(LotDefinition.m_strDatasetOrigin) == FALSE)
	{
		GSET_ERROR0(C_Gate_DataModel, eTestingStage, GGET_LASTERROR(C_Gate_DataModel_TestingStage, pclTestingStage));
		delete pclTestingStage;
		return NULL;
	}

	// Update from and to dates
	UpdateFromToDates(LotDefinition.m_lStartTime, LotDefinition.m_lFinishTime);
	pclTestingStage->UpdateFromToDates(LotDefinition.m_lStartTime, LotDefinition.m_lFinishTime);

	// Is this the first testing stage?
	if(NbTestingStages() == 0)
	{
		m_strProductID = LotDefinition.m_strProductID;
		AddTestingStage(pclTestingStage);

		return pclTestingStage;
	}

	// Make sure the data comes from same product ID
	if(LotDefinition.m_strProductID != m_strProductID)
	{
		GSET_ERROR2(C_Gate_DataModel, eMultipleProducts, NULL, LotDefinition.m_strProductID.toLatin1().data(), m_strProductID.toLatin1().data());
		return NULL;
	}

	// Look for a testing stage with same ID in the list
	C_Gate_DataModel_TestingStage* pScan = GetFirstTestingStage();
	while(pScan)
	{
		if(*pScan == *pclTestingStage)
		{
			// We found a testing stage, delete the one we just created, update From and To dates of the existing one,
			// and return ptr to the existing one
			delete pclTestingStage; pclTestingStage=0;
			pScan->UpdateFromToDates(LotDefinition.m_lStartTime, LotDefinition.m_lFinishTime);
			return pScan;
		}
		pScan = GetNextTestingStage();
	}

	// No testing with same ID found, add it
	AddTestingStage(pclTestingStage);
	return pclTestingStage;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Udates From and To dates of the data set
//
// Argument(s) :
//
//	long lStartTime
//		Start time
//
//	long lFinishTime
//		Finish time
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel::UpdateFromToDates(long lStartTime, long lFinishTime)
{
	if((m_clFromDate.toTime_t() == 0) || (lStartTime < (long)m_clFromDate.toTime_t()))
		m_clFromDate.setTime_t(lStartTime);
	if((m_clToDate.toTime_t() == 0) || (lFinishTime > (long)m_clToDate.toTime_t()))
		m_clToDate.setTime_t(lFinishTime);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Dumps the data structure to an ascii file (CSV format)
//
// Argument(s) :
//
//	const QString & strFileName
//		Dump file name
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataModel::Dump(QTextStream & hDumpFile)
{
	// Dump all testing stages
	int	nStep = 0;
	C_Gate_DataModel_TestingStage* pTestingStage = GetFirstTestingStage();
	C_Gate_DataModel_ParameterSet* pParameterSet;
	while(pTestingStage)
	{
		// Dump all ParameterSets
		pParameterSet = pTestingStage->m_pParameterSets.GetFirst();
		while(pParameterSet)
		{
			nStep += pParameterSet->m_uiNbParameters*3;
			pParameterSet = pTestingStage->m_pParameterSets.GetNext();
		}
		pTestingStage = GetNextTestingStage();
	}
	// Update ProgressBar
	if(m_pProgress)
		m_pProgress->Init("Dumping data in progress ...", nStep+10);

	// Write header
	hDumpFile << "DATA" << endl;
	hDumpFile << "====" << endl;
	hDumpFile << "ProductID" << CSV_SEPARATOR << GetCsvString(m_strProductID) << endl;
	hDumpFile << "From" << CSV_SEPARATOR << m_clFromDate.toString("dd MMM yyyy hh:mm:ss") << endl;
	hDumpFile << "To" << CSV_SEPARATOR << m_clToDate.toString("dd MMM yyyy hh:mm:ss") << endl;
	hDumpFile << "Nb testing stages" << CSV_SEPARATOR << NbTestingStages() << endl;

	// Dump all testing stages
	pTestingStage = GetFirstTestingStage();
	while(pTestingStage)
	{
		pTestingStage->Dump(hDumpFile);
		pTestingStage = GetNextTestingStage();
	}

}
