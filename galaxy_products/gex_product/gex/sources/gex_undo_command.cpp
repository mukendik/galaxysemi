#include "ctest.h"
#include "gex_report.h"
#include "drill_chart.h"
#include "browser_dialog.h"
#include "gex_undo_command.h"
#include "gex_options_handler.h"

extern GexMainwindow	*pGexMainWindow;
extern CGexReport* gexReport;
extern double       ScalingPower(int iPower);


TestRemoveResultCommand::TestRemoveResultCommand(const QString &strAction, QList<TestRemovedResult *> &oRemovedResult, QUndoCommand *poParent):QUndoCommand(poParent){

    QString strTest("On tests: ");
    m_oRemovedResult = oRemovedResult;
    if(!m_oRemovedResult.isEmpty()){
        foreach(TestRemovedResult *poObj, m_oRemovedResult){
            strTest += QString("%1,").arg(poObj->getTest()->lTestNumber);
        }
    }
    setText(strAction + QString(" ") + strTest);
    m_bFirstRun = true;
}

TestRemoveResultCommand::~TestRemoveResultCommand(){
    if(!m_oRemovedResult.isEmpty()){
        qDeleteAll(m_oRemovedResult);
        m_oRemovedResult.clear();
    }
}

void TestRemoveResultCommand::undo()
{
    TestRemovedResult* poTestRemovedResult;

    for (int i = m_oRemovedResult.size() - 1; i >= 0; --i)
    {
        poTestRemovedResult = m_oRemovedResult.at(i);
        CTest *poTest = poTestRemovedResult->getTest();
        foreach(int iIdxResult, poTestRemovedResult->getRunIdx()){
            bool isMultiResult = poTest->m_testResult.isMultiResultAt(iIdxResult);
            if(!isMultiResult){
                //poTest->m_testResult.invalidateResultAt(iIdxResult);// what is done
                poTest->m_testResult.validateResultAt(iIdxResult);//what is undone
            }else{
                foreach(int iIdxSubResult, poTestRemovedResult->getMultiResultRunIdx(iIdxResult)){
                    //poTest->m_testResult.at(iIdxResult)->invalidateResultAt(iIdxSubResult);// what is done
                    poTest->m_testResult.at(iIdxResult)->validateResultAt(iIdxSubResult);//what is undone
                }
            }
        }
        //poTestRemovedResult->saveTestsStats(&poTestRemovedResult->getSavedResult(), poTest);
        poTest->GetCurrentLimitItem()->ldOutliers = poTestRemovedResult->getOutlierRemoval();
        updateTest(poTest);
        //Check if we need to upadte group
        if(!poTestRemovedResult->getRunIdx().isEmpty() && poTestRemovedResult->getGroup() && poTestRemovedResult->getRemoveAssociatedParts()){
            updateGroup(poTestRemovedResult->getGroup(), poTestRemovedResult->getRunIdx(),true);
        }
    }
    updateChart();
}

void TestRemoveResultCommand::redo(){
    if(m_bFirstRun){
        m_bFirstRun = false;
        return;
    }

    foreach(TestRemovedResult *poTestRemovedResult, m_oRemovedResult){
        CTest *poTest = poTestRemovedResult->getTest();
        foreach(int iIdxResult, poTestRemovedResult->getRunIdx()){
            bool isMultiResult = poTest->m_testResult.isMultiResultAt(iIdxResult);
            if(!isMultiResult){
                poTest->m_testResult.invalidateResultAt(iIdxResult);// what is done
                //poTest->m_testResult.validateResultAt(iIdxResult);//what is undone
            }else{
                foreach(int iIdxSubResult, poTestRemovedResult->getMultiResultRunIdx(iIdxResult)){
                    poTest->m_testResult.at(iIdxResult)->invalidateResultAt(iIdxSubResult);// what is done
                    //poTest->m_testResult.at(iIdxResult)->validateResultAt(iIdxSubResult);//what is undone
                }
            }
        }
        //poTestRemovedResult->saveTestsStats(&poTestRemovedResult->getSavedResult(), poTest);
        poTest->GetCurrentLimitItem()->ldOutliers = poTestRemovedResult->getOutlierRemoval();
        updateTest(poTest);
        //Check if we need to upadte group
        if(!poTestRemovedResult->getRunIdx().isEmpty() && poTestRemovedResult->getGroup() && poTestRemovedResult->getRemoveAssociatedParts()){
            updateGroup(poTestRemovedResult->getGroup(), poTestRemovedResult->getRunIdx(), false);
        }
    }
    updateChart();
}

void TestRemoveResultCommand::updateChart(){

   pGexMainWindow->ReloadWizardHandler();
   /*foreach(TestRemovedResult *poTestRemovedResult, m_oRemovedResult){
       pGexMainWindow->ReloadWizardHandler(poTestRemovedResult->getTest());
   }*/
}

void TestRemoveResultCommand::updateTest(CTest *poTest){
    gexReport->m_cStats.ComputeLowLevelTestStatistics(poTest,ScalingPower(poTest->res_scal));
    gexReport->m_cStats.ComputeBasicTestStatistics(poTest,true);
    gexReport->m_cStats.RebuildHistogramArray(poTest,GEX_HISTOGRAM_OVERDATA);
    QString pf= gexReport->getReportOptions()->GetOption("statistics", "cp_cpk_computation").toString();
    gexReport->m_cStats.ComputeAdvancedDataStatistics(poTest,true,pf=="percentile"?true:false);
}

void TestRemoveResultCommand::updateGroup(CGexGroupOfFiles *poGroup, QList<int> lstParts,bool bUndo){

    CTest *						ptTest	= poGroup->cMergedData.ptMergedTestList;
    QList<int>::const_iterator   lPartOffset;
    while(ptTest)
    {
        // Erase ALL samples tested at given X,Y die coordinates (if retests, allows removing all retests instances at once!)
        if(ptTest->m_testResult.count() > 0)
        {
            // Mark samples as deleted for all parts tested at this DieX,Y location
            for (lPartOffset = lstParts.constBegin(); lPartOffset != lstParts.constEnd(); ++lPartOffset){
                if(!bUndo)
                    ptTest->m_testResult.deleteResultAt(*lPartOffset);
                else
                    ptTest->m_testResult.undeleteResultAt(*lPartOffset);

            }
        }

        // Next test
        ptTest = ptTest->GetNextTest();
    };

      // Get the first test in the testlist
      int nTestCellRsltCnt = 0;
      ptTest	= poGroup->cMergedData.ptMergedTestList;
      while(ptTest)
      {
          nTestCellRsltCnt = ptTest->m_testResult.count();

          //if (ptTest->bTestType == 'F' && ptTest->m_testResult.count() > 0)
          if (ptTest->bTestType == 'F' && nTestCellRsltCnt > 0)
          {
              // clear the existing one
              ptTest->mVectors.clear();

              //Compute execs and fails
              CFunctionalTest cVectorResult;
              //for (int iIndex = 0; iIndex < ptTest->ldSamplesExecs; iIndex++)
              for (int iIndex = 0; iIndex < nTestCellRsltCnt; iIndex++)
              {
                  if (ptTest->m_testResult.isValidResultAt(iIndex))
                  {
                      cVectorResult.lExecs++;

                      if (ptTest->m_testResult.resultAt(iIndex) == 0)
                          cVectorResult.lFails++;
                  }
              }

              ptTest->mVectors[cVectorResult.strVectorName] = cVectorResult;
          }

          //if(ptTest->m_testResult.count() > 0)
          if(nTestCellRsltCnt > 0)
          {
              float lfExponent = ScalingPower(ptTest->res_scal);
              poGroup->getStats().ComputeLowLevelTestStatistics(ptTest, lfExponent);
              poGroup->getStats().ComputeBasicTestStatistics(ptTest, true);
              poGroup->getStats().RebuildHistogramArray(ptTest, ReportOptions.iHistogramType);
              QString pf=ReportOptions.GetOption("statistics", "cp_cpk_computation").toString();
              poGroup->getStats().ComputeAdvancedDataStatistics(ptTest, true, pf=="percentile"?true:false); //cStats.ComputeAdvancedDataStatistics(ptTest, true, ReportOptions.bStatsCpCpkPercentileFormula);
          }

          // Next test
          ptTest = ptTest->GetNextTest();
      }

      poGroup->UpdateBinTables(true);
      poGroup->UpdateBinTables(false);

}

TestRemoveResultCommand::TestRemovedResult::TestRemovedResult(CTest *poTest, CGexGroupOfFiles *poGroup, bool bRemoveAssociatedParts){
    m_poTest = poTest;
    m_iOutlierRemoval = 0;
    m_poGroup = poGroup;
    m_bRemoveAssociatedParts = bRemoveAssociatedParts;
}

TestRemoveResultCommand::TestRemovedResult::~TestRemovedResult(){
    m_poTest = 0;
    m_oRunIdx.clear();
    m_oMultiResultRunIdx.clear();
}

void TestRemoveResultCommand::TestRemovedResult::addRemovedIdx(int iRunIdx){
    m_oRunIdx.append(iRunIdx);
}

void TestRemoveResultCommand::TestRemovedResult::addRemovedIdx(int iRunIdx, int iSubRunIdx){
    if(!m_oMultiResultRunIdx.contains(iRunIdx)){
        m_oMultiResultRunIdx.insert(iRunIdx, QList<int>());
    }
    m_oMultiResultRunIdx[iRunIdx].append(iSubRunIdx);
}

#if 0


OptionChangedCommand::OptionChangedCommand(const QString &strSection, const QString &strField, const QVariant &oOld, const QVariant &oNew,QUndoCommand *poParent)
    :QUndoCommand(poParent){

    m_strSection = strSection;
    m_strField = strField ;
    m_oOld = oOld;
    m_oNew = oNew;

    setText(QString("Change option value ""%1/%2"" from ""%3"" to ""%4"" ").arg(strSection).arg(strField).arg(oOld.toString()).arg(oNew.toString()));
    m_bFirstRun = true;
}

OptionChangedCommand::~OptionChangedCommand(){

}

void OptionChangedCommand::undo(){
    //Set to old value
    if(pGexMainWindow && pGexMainWindow->getOptionsHandler())
        pGexMainWindow->getOptionsHandler()->SetOption(m_strSection, m_strField, m_oOld.toString());
}

void OptionChangedCommand::redo(){
    if(m_bFirstRun){
        m_bFirstRun = false;
        return;
    }
    //SET to new value
    if(pGexMainWindow && pGexMainWindow->getOptionsHandler())
        pGexMainWindow->getOptionsHandler()->SetOption(m_strSection, m_strField, m_oNew.toString());
}
#endif


