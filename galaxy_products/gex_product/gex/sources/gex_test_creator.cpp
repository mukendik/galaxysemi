#include "report_build.h"
#include "ctest.h"
#include "gex_test_to_create.h"
#include "gex_file_in_group.h"
#include <gqtl_log.h>
#include "gex_group_of_files.h"

// cstats.cpp
extern double	ScalingPower(int iPower);

#include "gex_test_creator.h"

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexTestCreator::GexTestCreator(CGexFileInGroup *pFileInGroup)
{
    m_pFileInGroup = pFileInGroup;
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexTestCreator::~GexTestCreator()
{

}

///////////////////////////////////////////////////////////
// Build test in file group
///////////////////////////////////////////////////////////
bool GexTestCreator::createTestInFileGroup(GexTestToCreate *testToCreate,
                                           CTest	**ptMergedTestList,
                                           int pin /*GEX_PTEST*/)
{
    // Use the test list pointer
    // ZB: In merge files, if the test exists in an other file in the group, we have to get it and don't create a new one
    // an example is when we attach the external map to a file that contains more than 1 wafer. Or, when we attach the
    // same external map to 2 different input files
    m_pFileInGroup->ptTestList = *ptMergedTestList;
    CGexGroupOfFiles* lGroupOfFiles = m_pFileInGroup->getParentGroup();
    const QList<CGexFileInGroup*>& lFiles = lGroupOfFiles->GetFilesList();
    QListIterator<CGexFileInGroup*> lFilesInGroup(lFiles);
    CGexFileInGroup *			lFile = NULL;
    while (lFilesInGroup.hasNext())
    {
        lFile = lFilesInGroup.next();
        // If Test aldready exist in list
        if (lFile->FindTestCell(testToCreate->number().toLong(), pin, &m_pCTestToCreate, false, false,
                                         (testToCreate->name() + " [Formula]").toLatin1().data()) == 1)
        {

            // No overloading of existing test from stdf files
            if (m_pCTestToCreate->m_eOrigin == CTest::OriginStdf)
                return false;
            break;
        }

    }

    // If test doesn't exist in list
    if (m_pCTestToCreate == NULL)
    {
        // if the test is create is from MPR test
        // Create test
        m_pFileInGroup->FindTestCell(testToCreate->number().toLong(), pin, &m_pCTestToCreate, true, true,
                                     (testToCreate->name() + " [Formula]").toLatin1().data());
        if (m_pCTestToCreate == NULL)
            return false; // failed to create
        // Units
        strcpy(m_pCTestToCreate->szTestUnits, testToCreate->unit().toLatin1().constData());
        // Limits
        if (testToCreate->lowLimit().isEmpty())
            m_pCTestToCreate->GetCurrentLimitItem()->bLimitFlag &= CTEST_LIMITFLG_NOLTL;
        else
        {
            m_pCTestToCreate->GetCurrentLimitItem()->lfLowLimit = testToCreate->lowLimit().toDouble();
            m_pCTestToCreate->GetCurrentLimitItem()->bLimitFlag &= ~CTEST_LIMITFLG_NOLTL;
        }
        if (testToCreate->highLimit().isEmpty())
            m_pCTestToCreate->GetCurrentLimitItem()->bLimitFlag &= CTEST_LIMITFLG_NOHTL;
        else
        {
            m_pCTestToCreate->GetCurrentLimitItem()->lfHighLimit = testToCreate->highLimit().toDouble();
            m_pCTestToCreate->GetCurrentLimitItem()->bLimitFlag &= ~CTEST_LIMITFLG_NOHTL;
        }
        // Results
        m_pCTestToCreate->ldSamplesExecs = (*ptMergedTestList)->ldSamplesExecs;

        // GCORE-5913: HTH
        // Allow virtual test created to store multi-result per run.
        QString lRes=m_pCTestToCreate->m_testResult.createResultTable(m_pCTestToCreate->ldSamplesExecs, true);
        if (lRes.startsWith("error"))
        {
            GSLOG(SYSLOG_SEV_WARNING, QString("create test in file group : failed to create result table: %1").arg( lRes).toLatin1().constData());
        }
        // Get sublot information
        for (int iNSubLot = 0; iNSubLot < (*ptMergedTestList)->pSamplesInSublots.count(); iNSubLot++)
            m_pCTestToCreate->pSamplesInSublots.append((*ptMergedTestList)->pSamplesInSublots.at(iNSubLot));

        m_pCTestToCreate->ldSamplesValidExecs = 1;
        // Type parametric
        GEX_ASSERT(m_pCTestToCreate->bTestType == 'P');
        // Origin : custom
        m_pCTestToCreate->m_eOrigin = CTest::OriginCustom;
    }
    m_strFormula = testToCreate->formula();

    // A test has been added to the local test group, it can be in the beginning of the list so
    // we assign the local test list pointer to the merge list pointer.
    *ptMergedTestList = m_pFileInGroup->ptTestList;

    return true; //Succes to create test or existing test is a Custom one
}



///////////////////////////////////////////////////////////
// Apply formula and load test result
///////////////////////////////////////////////////////////
QList<CTest*> GexTestCreator::GetTestsFromFormula()
{
    QList<CTest*> testInFormulat;
    testInFormulat.clear();

    QRegExp regExpTestNumber("(T|t)\\d+(\\.\\d+)?"); // eg. T1050 or T1050.1
    unsigned int lTestNumber;
    CTest *pTestFound = NULL;
    int iPos = 0;
    bool ok;

    // While find matching expression
    while ((iPos = regExpTestNumber.indexIn(m_strFormula, iPos)) != -1)
    {
        pTestFound = NULL;

        // Get test number
        lTestNumber = regExpTestNumber.cap().mid(1).toUInt(&ok);
        if (!ok)
        {
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("Unable to extract the test number from formula: %1").arg(m_strFormula)
                  .toLatin1().constData());
            return QList<CTest*>();
        }

        // Search test from the beginning of the list
        m_pFileInGroup->FindTestCell(lTestNumber, GEX_PTEST, &pTestFound, false, false,"",false);

        // If no result exit
        if (pTestFound == NULL)
        {
            GSLOG(SYSLOG_SEV_WARNING,
                  QString("Unable to find the test number %1 in current test list").arg(lTestNumber)
                  .toLatin1().constData());
            return QList<CTest*>();
        }
        testInFormulat.append(pTestFound);

        iPos += regExpTestNumber.matchedLength();
    }

    return testInFormulat;
}



///////////////////////////////////////////////////////////
// Apply formula and load test result
///////////////////////////////////////////////////////////
void GexTestCreator::applyFormula(int iNSublot, QList<CTest*>& testInFormulat, int pin /*==GEX_PTEST*/)
{
    int iBeginIndex, iEndIndex, iSampleIndex;
    //Get indexs of the current sublot
    m_pCTestToCreate->findSublotOffset(iBeginIndex, iEndIndex, iNSublot);

    for (iSampleIndex = iBeginIndex; iSampleIndex < iEndIndex; ++iSampleIndex)
    {
        if (evaluateVars(iSampleIndex, testInFormulat, pin) == true)
        {
            CExEv cExpression;
            Variant vres;
            double dRes;
            ExpressionEvaluatorOnGex eectx(m_lstVars);
            cExpression.setContext(&eectx);
            int iStatus = cExpression.evalExpression(m_strFormula, vres);

			if(iStatus == CExEv::E_OK)
			{
				vres.asReal(dRes);
				m_pCTestToCreate->m_testResult.pushResultAt(iSampleIndex, dRes);
				// Update SamplesMin,SampleMax variables!
				m_pCTestToCreate->lfSamplesMin = gex_min(dRes,m_pCTestToCreate->lfSamplesMin);
				m_pCTestToCreate->lfSamplesMax = gex_max(dRes,m_pCTestToCreate->lfSamplesMax);
				m_pCTestToCreate->lfSamplesTotal += dRes;				// Sum of X
				m_pCTestToCreate->lfSamplesTotalSquare += (dRes*dRes);	// Sum of X*X
			}
            else
            {
				//m_pCTestToCreate->m_testResult.pushResultAt(iSampleIndex, GEX_C_DOUBLE_NAN);
				m_pCTestToCreate->m_testResult.invalidateResultAt(iSampleIndex);
			}
		}
        else
        {
			//m_pCTestToCreate->m_testResult.pushResultAt(iSampleIndex, GEX_C_DOUBLE_NAN);
			m_pCTestToCreate->m_testResult.invalidateResultAt(iSampleIndex);
		}
	}
}


///////////////////////////////////////////////////////////
// Evaluate variable, add it in m_lstVars
///////////////////////////////////////////////////////////
bool GexTestCreator::evaluateVars(int iSampleIndex, QList<CTest*>& testInFormulat, int pin/* =GEX_PTEST*/)
{
    CTest *pTestFound = NULL;
    m_lstVars.clear();

    // While find test in the same formula
    for (int i=0; i< testInFormulat.size(); ++i)
    {
        pTestFound = testInFormulat[i];
        unsigned int ltestNumber = pTestFound->lTestNumber;
        // MPR test
        if(pTestFound->lPinmapIndex == GEX_MPTEST)
        {
            int i = 0;
            while(pTestFound != NULL
                  && pTestFound->lTestNumber == ltestNumber
                  && i <= pin)
            {
                pTestFound = pTestFound->GetNextTest();
                i++;
            }
        }
        // If no result exit
        if ( (pTestFound == NULL) || (!pTestFound->m_testResult.isValidIndex(iSampleIndex))
             || (!pTestFound->m_testResult.isValidResultAt(iSampleIndex)) )
            return false;

        // Add the "T" to be conform to the regExp used by the function getValue
        QString key = "T" + QString::number(pTestFound->lTestNumber);
        double value = pTestFound->m_testResult.resultAt(iSampleIndex) * ScalingPower(pTestFound->res_scal);
        m_lstVars.insert(key, value);
    }

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//												CLASS ExpressionEvaluatorOnGex								//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
ExpressionEvaluatorOnGex::ExpressionEvaluatorOnGex(QMap<QString, double> &lstVars) :
        CExEvCtx(),
        m_lstVars(lstVars)
{

}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
ExpressionEvaluatorOnGex::~ExpressionEvaluatorOnGex()
{

}

///////////////////////////////////////////////////////////
// OverRide to return evaluate unknown var value in lstVars
///////////////////////////////////////////////////////////
bool ExpressionEvaluatorOnGex::getValue(const char *cIdent, Variant& result)
{
    QString strIdent(cIdent);
    QRegExp regExpColPosition("(C|c)\\d+");
    QRegExp regExpRowPosition("(R|r)\\d+");
    QRegExp regExpTestNumber("(T|t)\\d+(.\\d+)?");

    // If it's a test number
    if (regExpTestNumber.exactMatch(strIdent))
    {
        result.setReal(m_lstVars[strIdent]);
            return true;
    }
    // If it's a column number
    else if (regExpColPosition.exactMatch(strIdent))
    {
        result.setReal(m_lstVars[strIdent]);
            return true;
    }
    // If it's a row number
    else if(regExpRowPosition.exactMatch(strIdent))
    {
        result.setReal(m_lstVars[strIdent]);
            return true;
    }

    return CExEvCtx::getValue(cIdent, result);
}

