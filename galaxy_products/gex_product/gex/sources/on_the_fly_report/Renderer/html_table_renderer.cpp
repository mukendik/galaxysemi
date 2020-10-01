#include "ctest.h"
#include "gex_report.h"
#include "pat_engine.h"
#include "patman_lib.h"
#include "renderer_keys.h"
#include "html_table_renderer.h"



extern CGexReport* gexReport;

HTMLTableRenderer::HTMLTableRenderer()
{

}

template<typename T_DATA>
QString HTMLTableRenderer::CreateCell(const T_DATA& value, const QString &color /*= DATA_COLOR*/) const
{
    return QString("<td bgcolor=%1align=\"center\">%2</td>\n").arg(color).arg(value);
}

template<typename T_DATA>
QString HTMLTableRenderer::CreateCell(const T_DATA& value, char result, const QString &color /*= DATA_COLOR*/) const
{
    if (result == 'F')
        return QString("<td bgcolor=%1 align=\"center\" >%2</td>\n").arg(color).arg(GEX_NA);
    else
    {
        return CreateCell<T_DATA>(value, color);
    }
}

QString HTMLTableRenderer::CreateHeader( const QList<QString>& fields) const
{
    // -- the test's number and name must be write for each table
    QString lHTMLTable = "<tr>\n"
                  "<td bgcolor=\"#CCECFF\" align=\"center\" ><b>" + columnsLabel[TEST_TNUMBER] + "</b></td>\n"
                  "<td bgcolor=\"#CCECFF\" align=\"center\" ><b>" + columnsLabel[TEST_TNAME] + "</b></td>\n";

    // -- write the header column
    QList<QString>::ConstIterator lIterBegin(fields.begin()), lIterEnd(fields.end());
    for(; lIterBegin != lIterEnd; ++lIterBegin)
    {
        // -- already managed above
        if( *lIterBegin == TEST_TNAME || *lIterBegin == TEST_TNUMBER)
            continue;
        else
            lHTMLTable += "<td bgcolor=\"#CCECFF\" align=\"center\" ><b>" + columnsLabel[*lIterBegin] + "</b></td>\n";
    }

   lHTMLTable += "</tr>\n";

   return lHTMLTable;
}

void HTMLTableRenderer::InitShiftColumnLabels() const
{
    bool lCompareAllGroups = (ReportOptions.GetOption("statistics","shift_with")).toString() == "shift_to_all";
    if (lCompareAllGroups)
    {
        int lNbGroups = gexReport->getGroupsList().size();
        for(int lIdx = 0; lIdx < lNbGroups; ++lIdx)
        {
            if (gexReport->getGroupsList()[lIdx])
            {
                QString lGroupName = gexReport->getGroupsList()[lIdx]->strGroupName;
                if (!columnsLabel.contains(TEST_MEANSHIFT + lGroupName))
                {
                    columnsLabel.insert(TEST_MEANSHIFT + lGroupName,
                                        columnsLabel.value(TEST_MEANSHIFT) + " (Ref: " + lGroupName + ")");
                    columnsLabel.insert(TEST_SIGMASHIFT + lGroupName,
                                        columnsLabel.value(TEST_SIGMASHIFT) + " (Ref: " + lGroupName + ")");
                    columnsLabel.insert(TEST_CPSHIFT + lGroupName,
                                        columnsLabel.value(TEST_CPSHIFT) + " (Ref: " + lGroupName + ")");
                    columnsLabel.insert(TEST_CRSHIFT + lGroupName,
                                        columnsLabel.value(TEST_CRSHIFT) + " (Ref: " + lGroupName + ")");
                    columnsLabel.insert(TEST_CPKSHIFT + lGroupName,
                                        columnsLabel.value(TEST_CPKSHIFT) + " (Ref: " + lGroupName + ")");
                }
            }
        }
    }
}

QString HTMLTableRenderer::CreateItem(const QString& field, CTest* test,
                                      CGexGroupOfFiles* group, CGexFileInGroup* file ) const
{
    QString lStringToAdd;

    //- alarm_cp_cr_cpk shifts
    double  lOptionAlarmCp    =   0.0;
    bool    lOptionRsltCp   =   false;
    lOptionAlarmCp = (ReportOptions.GetOption("statistics","alarm_cp")).toDouble(&lOptionRsltCp);
    double  lOptionAlarmCr    =   0.0;
    bool    lOptionRsltCr   =   false;
    lOptionAlarmCr = (ReportOptions.GetOption("statistics","alarm_cr")).toDouble(&lOptionRsltCr);
    double  lOptionAlarmCpk    =   0.0;
    bool    lOptionRsltCpk   =   false;
    lOptionAlarmCpk = (ReportOptions.GetOption("statistics","alarm_cpk")).toDouble(&lOptionRsltCpk);

    // test_type
    if(field == TEST_TYPE)
    {
        return CreateCell(QString(test->bTestType));
    }

    // The table has to contain test_number and test_name
    // Low limit
    else if(field == TEST_LTL)
    {
        if((test->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
            return CreateCell(QString::number(test->GetCurrentLimitItem()->lfLowLimit), test->bTestType);
        else
            return CreateCell(GEX_NA);
    }

    // High limit
    else if(field == TEST_HTL)
    {
        if((test->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
            return CreateCell(QString::number(test->GetCurrentLimitItem()->lfHighLimit), test->bTestType);
        else
            return CreateCell(GEX_NA);
    }

    // Low spec limit
    else if(field == TEST_LSL)
    {
        if((test->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLSL) == 0)
            return CreateCell(QString::number(test->lfLowSpecLimit), test->bTestType);
        else
            return CreateCell(GEX_NA);
    }

    // High spec limit
    else if(field == TEST_HSL)
    {
        if((test->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHSL) == 0)
            return CreateCell(QString::number(test->lfHighSpecLimit), test->bTestType);
        else
            return CreateCell(GEX_NA);
    }

    // Exec
    else if(field == TEST_EXEC)
        return CreateCell(test->ldExecs, test->bTestType);

    // Write mean
    else if(field == TEST_MEAN)
    {
        QString lString;
        QString lBackgroundColor(DATA_COLOR);
        if (test->bTestType == 'F')
        {
            lString = GEX_NA;
        }
        else
        {
            // Highlight in red if mean outside of limits!
            if( test->lfMean != GEX_C_DOUBLE_NAN &&
                ((((test->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
                    && (test->lfMean < test->GetCurrentLimitItem()->lfLowLimit)) ||
                (((test->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
                    && (test->lfMean > test->GetCurrentLimitItem()->lfHighLimit))) )
            {
                lBackgroundColor = ALARM_COLOR;
            }
            else
            {
                lBackgroundColor = DATA_COLOR;
            }
            lString = file->FormatTestResult(test, test->lfMean, test->res_scal);
        }
        return CreateCell(lString, test->bTestType, lBackgroundColor);
    }

    // Sigma
    else if(field == TEST_SIGMA)
    {
        // Write sigma
        return CreateCell(file->FormatTestResult(test, test->lfSigma, test->res_scal), test->bTestType);
    }

    // CP
    else if(field == TEST_CP)
    {
        QString lBackgroundColor(DATA_COLOR);
        double lfRedAlarmCpValue = 0.0, lfYellowAlarmCpValue= 0.0;
        lfRedAlarmCpValue = ReportOptions.GetOption("statistics", "alarm_test_cp").toDouble();
        lfYellowAlarmCpValue = ReportOptions.GetOption("statistics", "warning_test_cp").toDouble();

        // Highlight in red if Cp Alarm!
        bool lRedAlarmCpValidity = (lfRedAlarmCpValue >=0)
                                   && (test->GetCurrentLimitItem()->lfCp != C_NO_CP_CPK)
                                   && (test->GetCurrentLimitItem()->lfCp < lfRedAlarmCpValue);
        bool lYellowAlarmCpValidity = (lfYellowAlarmCpValue >=0)
                                      && (test->GetCurrentLimitItem()->lfCp != C_NO_CP_CPK)
                                      && (test->GetCurrentLimitItem()->lfCp < lfYellowAlarmCpValue);

            if(lRedAlarmCpValidity)
                lBackgroundColor = ALARM_COLOR;		// Alarm
            else if(lYellowAlarmCpValidity)
                lBackgroundColor = WARNING_COLOR;	// Warning

        return CreateCell(gexReport->CreateResultStringCpCrCpk(test->GetCurrentLimitItem()->lfCp),
                          test->bTestType,
                          lBackgroundColor);
    }

    // CPK
    else if(field == TEST_CPK)
    {
        // Write Cpk
        QString lString;
        QString lBackgroundColor(DATA_COLOR);
        if (test->bTestType == 'F')
            lString = GEX_NA;
        else
        {
            // Highlight in red if Cpk Alarm!
            bool lRedAlarmCpkValidity, lYellowAlarmCpkValidity;
            double lfRedAlarmCpkValue, lfYellowAlarmCpkValue;

            CReportOptions *lReportOptions = gexReport->getReportOptions();
            if(lReportOptions == 0)
                return "";

            lfRedAlarmCpkValue      = lReportOptions->GetOption("statistics", "alarm_test_cpk")
                                        .toDouble(&lRedAlarmCpkValidity);
            lfYellowAlarmCpkValue   = lReportOptions->GetOption("statistics", "warning_test_cpk")
                                        .toDouble(&lYellowAlarmCpkValidity);

            lRedAlarmCpkValidity = (lfRedAlarmCpkValue >=0)
                                   && (test->GetCurrentLimitItem()->lfCpk != C_NO_CP_CPK)
                                   && (test->GetCurrentLimitItem()->lfCpk < lfRedAlarmCpkValue);
            lYellowAlarmCpkValidity = (lfYellowAlarmCpkValue >=0)
                                       && (test->GetCurrentLimitItem()->lfCpk != C_NO_CP_CPK)
                                       && (test->GetCurrentLimitItem()->lfCpk < lfYellowAlarmCpkValue);

            if(lRedAlarmCpkValidity)
                lBackgroundColor = ALARM_COLOR;	// ALARM!: Cpk under Alarm level.
            else if(lYellowAlarmCpkValidity)
                lBackgroundColor = WARNING_COLOR;	// Warning!: Cpk under Warning level

            lString = gexReport->CreateResultStringCpCrCpk(test->GetCurrentLimitItem()->lfCpk);
        }
        return CreateCell(lString, test->bTestType, lBackgroundColor);
    }

    //CPKL
    else if(field == TEST_CPKL)
    {
        // Write Cpl
        return CreateCell(gexReport->CreateResultStringCpCrCpk(test->GetCurrentLimitItem()->lfCpkLow),
                               test->bTestType);
    }

    //CPKH (CPKU)
    else if(field == TEST_CPKH)
    {
        return CreateCell(gexReport->CreateResultStringCpCrCpk(test->GetCurrentLimitItem()->lfCpkHigh),
                               test->bTestType);
    }

    // Group name / Dataset
    else if (field == TEST_GROUP)
    {
        return  CreateCell(group->strGroupName, test->bTestType);
    }

    // Test/Spec limits drift: both Test & spec limits must exist
    else if(field == TEST_DRIFTL)
    {
        return CreateCell(gexReport->CreateTestDrilTL(test), test->bTestType);
    }

    // TEST_DRIFTLOW
    else if(field == TEST_DRIFTLOW)
    {
        return CreateCell(gexReport->CreateTestDrilFlow(test), test->bTestType);
    }

    // TEST_DRIFTHIGH
    else if(field == TEST_DRIFTHIGH)
    {
        return CreateCell(gexReport->CreateTestDriftHigh(test), test->bTestType);
    }

    // TEST_SHAPE
    else if(field == TEST_SHAPE)
    {
        CPatInfo* lPatInfo = GS::Gex::PATEngine::GetInstance().GetContext();
        if (lPatInfo)
        {
            lStringToAdd = patlib_GetDistributionName(patlib_GetDistributionType(test,
                                                                lPatInfo->GetRecipeOptions().iCategoryValueCount,
                                                                lPatInfo->GetRecipeOptions().bAssumeIntegerCategory,
                                                                lPatInfo->GetRecipeOptions().mMinConfThreshold));
        }
        else
        {
            lStringToAdd = patlib_GetDistributionName(patlib_GetDistributionType(test));
        }
        return CreateCell(lStringToAdd, test->bTestType);
    }

    // TEST_STATS_SRC
    else if(field == TEST_STATS_SRC)
    {
        return CreateCell((test->bStatsFromSamples) ? "Samples": "Summary" , test->bTestType);
    }

    // TEST_FAIL
    else if(field == TEST_FAIL)
    {
        return CreateCell(gexReport->CreateResultString(test->GetCurrentLimitItem()->ldFailCount),test->bTestType);
    }

    // TEST_FAILPERCENT
    else if(field == TEST_FAILPERCENT)
    {
        gexReport->CreateTestFailPercentage(test, lStringToAdd);
        return CreateCell(lStringToAdd, test->bTestType);
    }

    // TEST_FAILBIN
    else if(field == TEST_FAILBIN)
    {
        QString lFail("0");
        if(test->iFailBin >= 0)
        {
            lFail = QString::number(test->iFailBin);
        }
        return CreateCell(lFail, test->bTestType);
    }

    // TEST_TESTFLOWID
    else if(field == TEST_TESTFLOWID)
    {
        QString lTestFlowId(GEX_NA);
        if(test->lTestFlowID > 0)
            lTestFlowId = QString::number(test->lTestFlowID);
        return CreateCell(lTestFlowId, test->bTestType);
    }

    // TEST_OUTLIER
    else if(field == TEST_OUTLIER)
    {
        return CreateCell(gexReport->CreateResultString(test->GetCurrentLimitItem()->ldOutliers), test->bTestType);
    }

    // TEST_MEANSHIFT
    else if(field.startsWith(TEST_MEANSHIFT))
    {
        QString lRefGroupName = field.right(field.size() - TEST_MEANSHIFT.size());
        TestShift lTestShift = test->mTestShifts.value(lRefGroupName);
        GS::Core::MLShift lMLShift = lTestShift.GetMlShift(test->GetCurrentLimitItem());
        QString lMeanShift(GEX_NA);
        bool lAlarmRaised(false);
        if(test->bTestType != 'F')
        {
                QString strTemp, strFormatDouble;
                strTemp.sprintf("%.2f", lMLShift.mMeanShiftPct);
                strFormatDouble = gexReport->getNumberFormat()->formatNumericValue(lMLShift.mMeanShiftPct,
                                                                                   false,
                                                                                   strTemp);
                lMeanShift.sprintf("%s %%",strFormatDouble.toLatin1().constData());

                //- alarm_mean
                double  lOptionAlarmMean   =   0.0;
                bool    lOptionMean      =   false;
                lOptionAlarmMean = (ReportOptions.GetOption("statistics","alarm_mean")).toDouble(&lOptionMean);
                lAlarmRaised = (fabs(lMLShift.mMeanShiftPct) >= lOptionAlarmMean) && (lOptionMean == true);
        }

        // Color the cell if needed
        QString lBackgroundColor(DATA_COLOR);
        if(lAlarmRaised)
        {
           lBackgroundColor = ALARM_COLOR;
        }
        return CreateCell(lMeanShift, test->bTestType, lBackgroundColor);
    }

    // TEST_T_TEST
    else if(field == TEST_T_TEST)
    {
        return CreateCell(gexReport->getNumberFormat()->formatNumericValue(test->getP_Value(), false)
                                                                            .toLatin1().constData(),
                          test->bTestType);
    }

    // TEST_SIGMASHIFT
    else if(field.startsWith(TEST_SIGMASHIFT))
    {
        QString lRefGroupName = field.right(field.size() - TEST_SIGMASHIFT.size());
        TestShift lTestShift = test->mTestShifts.value(lRefGroupName);
        QString lSigma(GEX_NA);
        bool lAlarmRaised(false);
        if(test->bTestType != 'F')
        {
            QString strTemp, strFormatDouble;
            strTemp.sprintf("%.2f", lTestShift.mSigmaShiftPercent);
            strFormatDouble = gexReport->getNumberFormat()->formatNumericValue(lTestShift.mSigmaShiftPercent,
                                                                               false,
                                                                               strTemp);
            lSigma.sprintf("%s %%",strFormatDouble.toLatin1().constData());
            bool lRsltSigma(false);
            double lfOptionAlarmSigma = 0.0;
            lfOptionAlarmSigma = (ReportOptions.GetOption("statistics", "alarm_sigma")).toDouble(&lRsltSigma);
            lAlarmRaised = (fabs(lTestShift.mSigmaShiftPercent) >= lfOptionAlarmSigma) && (lRsltSigma == true);
        }

        // Color the cell if needed
        QString lBackgroundColor(DATA_COLOR);
        if(lAlarmRaised)
        {
           lBackgroundColor = ALARM_COLOR;
        }
        return CreateCell(lSigma, test->bTestType, lBackgroundColor);
    }

    // TEST_2_SIGMA
    else if(field == TEST_2_SIGMA)
    {
        QString l2Sigma(GEX_NA);
        if(test->bTestType != 'F')
        {
            double lfData = 2.0* test->lfSigma;
            l2Sigma = file->FormatTestResult(test, lfData, test->res_scal, false);
        }
        return CreateCell(l2Sigma, test->bTestType);
    }

    // TEST_3_SIGMA
    else if(field == TEST_3_SIGMA)
    {
        QString l3Sigma(GEX_NA);
        if(test->bTestType != 'F')
        {
            double lfData = 3.0* test->lfSigma;
            l3Sigma = file->FormatTestResult(test, lfData, test->res_scal, false);
        }
        return CreateCell(l3Sigma, test->bTestType);
    }

    // TEST_6_SIGMA
    else if(field == TEST_6_SIGMA)
    {
        QString l6Sigma(GEX_NA);
        if(test->bTestType != 'F')
        {
            double lfData = 6.0* test->lfSigma;
            l6Sigma = file->FormatTestResult(test, lfData, test->res_scal, false);
        }
        return CreateCell(l6Sigma, test->bTestType);
    }

    // TEST_MIN
    else if(field == TEST_MIN)
    {
        QString lMin(GEX_NA);
        double lfData = test->lfMin;
        if(lfData != C_INFINITE && test->bTestType != 'F')
            lMin = file->FormatTestResult(test, lfData, test->res_scal, false);
        return CreateCell(lMin, test->bTestType);
    }

    // TEST_MAX
    else if(field == TEST_MAX)
    {
        QString lMax(GEX_NA);
        double lfData = test->lfMax;
        if(lfData != C_INFINITE && test->bTestType != 'F')
            lMax = file->FormatTestResult(test, lfData, test->res_scal, false);
        return CreateCell(lMax, test->bTestType);
    }

    // TEST_RANGE
    else if(field == TEST_RANGE)
    {
        QString lRange(GEX_NA);
        double lfData = test->lfRange;
        if(lfData != C_INFINITE && test->bTestType != 'F')
            lRange = file->FormatTestResult(test, lfData, test->res_scal, false);
        return CreateCell(lRange, test->bTestType);
    }

    // TEST_MAX_RANGE
    else if(field == TEST_MAX_RANGE)
    {
        QString lMaxRange;
        // Max. Range (only if comparing datasets)
        if (test->bTestType == 'F')
            lMaxRange = GEX_NA;
        else
        {
            if(group->GetGroupId())
            {
                // 2nd group and higher.
                lMaxRange = " ";
            }
            else
            {
                // First group
                double lfData = test->lfMaxRange;
                lMaxRange = file->FormatTestResult(test,lfData,test->res_scal, false);
            }
        }
        return CreateCell(lMaxRange, test->bTestType);
    }

    // TEST_CR
    else if(field == TEST_CR)
    {
        QString lCpString=gexReport->CreateResultStringCpCrCpk(test->GetCurrentLimitItem()->lfCp);
        QString lCrString=GEX_NA;
        if (lCpString != GEX_NA && test->GetCurrentLimitItem()->lfCp != 0.0)
            lCrString=gexReport->CreateResultStringCpCrCpk(1/test->GetCurrentLimitItem()->lfCp);
        return CreateCell(lCrString, test->bTestType);
    }

    // TEST_CPSHIFT
    else if(field.startsWith(TEST_CPSHIFT))
    {
        QString lRefGroupName = field.right(field.size() - TEST_CPSHIFT.size());
        TestShift lTestShift = test->mTestShifts.value(lRefGroupName);
        GS::Core::MLShift lMLShift = lTestShift.GetMlShift(test->GetCurrentLimitItem());
        QString strString;
        bool lAlarmRaised(false);
        if (test->bTestType == 'F')
            strString = GEX_NA;
        else
        {
            strString.sprintf("%.2f", lMLShift.mCpShift);
            QString strFormatDouble = gexReport->getNumberFormat()->formatNumericValue(lMLShift.mCpShift,
                                                                                       false,
                                                                                       strString);
            strString.sprintf("%s %%",strFormatDouble.toLatin1().constData());
            lAlarmRaised = (fabs(lMLShift.mCpShift) >= lOptionAlarmCp) && (lOptionRsltCp == true);
        }
        // Color the cell if needed
        QString lBackgroundColor(DATA_COLOR);
        if(lAlarmRaised)
        {
           lBackgroundColor = ALARM_COLOR;
        }
        return CreateCell(strString, test->bTestType, lBackgroundColor);
    }

    // TEST_CRSHIFT
    else if(field.startsWith(TEST_CRSHIFT))
    {
        QString lRefGroupName = field.right(field.size() - TEST_CRSHIFT.size());
        TestShift lTestShift = test->mTestShifts.value(lRefGroupName);
        GS::Core::MLShift lMLShift = lTestShift.GetMlShift(test->GetCurrentLimitItem());
        QString strString;
        bool lAlarmRaised(false);
        if (test->bTestType == 'F')
            strString = GEX_NA;
        else
        {
            strString.sprintf("%.2f", lMLShift.mCrShift);
            QString strFormatDouble = gexReport->getNumberFormat()->formatNumericValue(lMLShift.mCrShift,
                                                                                       false,
                                                                                       strString);
            strString.sprintf("%s %%",strFormatDouble.toLatin1().constData());
            lAlarmRaised = (fabs(lMLShift.mCrShift) >= lOptionAlarmCr) && (lOptionRsltCr == true);
        }
        // Color the cell if needed
        QString lBackgroundColor(DATA_COLOR);
        if(lAlarmRaised)
        {
           lBackgroundColor = ALARM_COLOR;
        }
        return CreateCell(strString, test->bTestType, lBackgroundColor);
    }

    // TEST_CPKSHIFT
    else if(field.startsWith(TEST_CPKSHIFT))
    {
        QString lRefGroupName = field.right(field.size() - TEST_CPKSHIFT.size());
        TestShift lTestShift = test->mTestShifts.value(lRefGroupName);
        GS::Core::MLShift lMLShift = lTestShift.GetMlShift(test->GetCurrentLimitItem());
        QString strString;
        bool lAlarmRaised(false);
        if (test->bTestType == 'F')
            strString = GEX_NA;
        else
        {
            strString.sprintf("%.2f", lMLShift.mCpkShift);
            QString strFormatDouble = gexReport->getNumberFormat()->formatNumericValue(
                        lMLShift.mCpkShift, false, strString);
            strString.sprintf("%s %%",strFormatDouble.toLatin1().constData());
            lAlarmRaised = (fabs(lMLShift.mCpkShift) >= lOptionAlarmCpk) && (lOptionRsltCpk == true);
        }
        // Color the cell if needed
        QString lBackgroundColor(DATA_COLOR);
        if(lAlarmRaised)
        {
           lBackgroundColor = ALARM_COLOR;
        }
        return CreateCell(strString, test->bTestType, lBackgroundColor);
    }

    // TEST_YIELD
    else if(field == TEST_YIELD)
    {
        QString szString(GEX_NA);
        double lfData(0);
        if(test->ldExecs > 0)
        {
            lfData = 100.0 -(100.0 * test->GetCurrentLimitItem()->ldFailCount / test->ldExecs);
            szString.sprintf("%.2f",lfData);
            QString strFormatDouble = gexReport->getNumberFormat()->formatNumericValue(lfData, false,szString);
            szString.sprintf("%s",strFormatDouble.toLatin1().constData());
        }

        // Highlight in red if Yield Alarm!
        //- alarm_test_yield
        double lYieldAlarm = 0.0, lYieldWarning = 0.0;
        bool lYieldAlarmGetOption = false, lYieldWarningGetOption = false;

        lYieldAlarm = (ReportOptions.GetOption("statistics","alarm_test_yield")).toDouble(&lYieldAlarmGetOption);
        lYieldWarning = (ReportOptions.GetOption("statistics","warning_test_yield")).toDouble(&lYieldWarningGetOption);
        lYieldAlarmGetOption = (lYieldAlarm >= 0) && (lfData < lYieldAlarm);
        lYieldWarningGetOption = (lYieldWarning >= 0) && (lfData < lYieldWarning);

        QString lBackgroundColor(DATA_COLOR);
        if(test->ldSamplesValidExecs && lYieldAlarmGetOption)
        {
            lBackgroundColor = ALARM_COLOR;		// Alarm
        }
        else if( test->ldSamplesValidExecs && lYieldWarningGetOption && (!lYieldAlarmGetOption) )
        {
            lBackgroundColor = WARNING_COLOR;	// Warning
        }
        return CreateCell(szString, test->bTestType, lBackgroundColor);
    }

    // TEST_GAGE_EV
    if ((field == TEST_GAGE_AV)
        || (field == TEST_GAGE_EV)
        || (field == TEST_GAGE_RR)
        || (field == TEST_GAGE_GB)
        || (field == TEST_GAGE_PV)
        || (field == TEST_GAGE_TV))
    {
        QString strString;
        double lData, lfLimitSpace(0.0);
        if (group->GetGroupId())
        {
            strString = "";
        }
        else
        {
            double  lfPercent=0;
            bool bBoxplotShiftOverTV = (ReportOptions.GetOption("adv_boxplot","delta").toString() == "over_tv");
            if((test->GetCurrentLimitItem()->bLimitFlag & (CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL)) == 0)
            {
                lfLimitSpace = test->GetCurrentLimitItem()->lfHighLimit - test->GetCurrentLimitItem()->lfLowLimit;
            }
            if(field == TEST_GAGE_EV)
            {
                // Gage EV (only display value for first group.)
                if(test->pGage == NULL)
                {
                    strString = "";
                }
                else
                {
                    // Check if % computed over TV or Limit space
                    if (bBoxplotShiftOverTV)
                        lData = (test->pGage->lfTV) ? test->pGage->lfTV : 1;
                    else
                        lData = (lfLimitSpace) ? lfLimitSpace : 1;

                    strString = gexReport->ValueAndPercentageString(test->pGage->lfEV, lData, lfPercent);
                }
                return CreateCell(strString, test->bTestType);
            }

            // TEST_GAGE_AV
            else if(field == TEST_GAGE_AV)
            {
                // Gage AV (only display value for first group.)
                if(test->pGage == NULL)
                {
                    strString = "";
                }
                else
                {
                    // Check if % computed over TV or Limit space
                    if(bBoxplotShiftOverTV)
                        lData = (test->pGage->lfTV) ? test->pGage->lfTV : 1;
                    else
                        lData = (lfLimitSpace) ? lfLimitSpace : 1;

                    strString = gexReport->ValueAndPercentageString(test->pGage->lfAV, lData, lfPercent);
                }
                return CreateCell(strString, test->bTestType);
            }

            // TEST_GAGE_RR
            else if(field == TEST_GAGE_RR)
            {
                QString strString;
                double lData;
                QString lBackgroundColor(DATA_COLOR);
                if(test->pGage == NULL)
                {
                    strString = "";
                }
                else
                {
                    if(bBoxplotShiftOverTV)
                        lData = (test->pGage->lfTV) ? test->pGage->lfTV : 1;
                    else
                        lData = (lfLimitSpace) ? lfLimitSpace : 1;
                    strString = gexReport->ValueAndPercentageString(test->pGage->lfRR, lData, lfPercent);
                    // Check if R&R% alarm
                    QColor lColor; // it is not used now, we have to use it in the future
                    gexReport->getR_R_AlarmColor(test, lfPercent, lBackgroundColor, lColor/*, pCell->mCustomBkColor*/);
                }
                return CreateCell(strString, test->bTestType, lBackgroundColor);
            }

            // TEST_GAGE_GB
            else if(field == TEST_GAGE_GB)
            {
                QString strString;
                double lData;
                // Gage GB (only display value for first group.)
                if(test->pGage == NULL)
                {
                    strString = "";
                }
                else
                {
                    // Check if % computed over TV or Limit space
                    if(bBoxplotShiftOverTV)
                        lData = (test->pGage->lfGB) ? test->pGage->lfGB : 1;
                    else
                        lData = (lfLimitSpace) ? lfLimitSpace : 1;

                    strString = gexReport->ValueAndPercentageString(test->pGage->lfGB,-1,lfPercent);
                }
                return CreateCell(strString, test->bTestType);
            }

            // TEST_GAGE_PV
            else if(field == TEST_GAGE_PV)
            {
                QString strString;
                double lData;
                // Gage PV (only display value for first group.)
                if(test->pGage == NULL)
                {
                    strString = "";
                }
                else
                {
                    // Check if % computed over TV or Limit space
                    if(bBoxplotShiftOverTV)
                        lData = (test->pGage->lfTV) ? test->pGage->lfTV : 1;
                    else
                        lData = (lfLimitSpace) ? lfLimitSpace : 1;

                    strString = gexReport->ValueAndPercentageString(test->pGage->lfPV,lData,lfPercent);
                }
                return CreateCell(strString, test->bTestType);
            }

            // TEST_GAGE_TV
            else if(field == TEST_GAGE_TV)
            {
                QString strString;
                double lData;
                // Gage TV (only display value for first group.)
                if(test->pGage == NULL)
                {
                    strString = "";
                }
                else
                {
                    // Check if % computed over TV or Limit space
                    if(bBoxplotShiftOverTV)
                        lData = (test->pGage->lfTV) ? test->pGage->lfTV : 1;
                    else
                        lData = (lfLimitSpace) ? lfLimitSpace : 1;

                    strString = gexReport->ValueAndPercentageString(test->pGage->lfTV,lData,lfPercent);
                }
                return CreateCell(strString, test->bTestType);
            }

        }
    }
    // TEST_GAGE_P_T
    else if(field == TEST_GAGE_P_T)
    {
        QString lTempString = "";
        if(test->pGage != NULL)
        {
            lTempString = QString::number(test->pGage->lfP_T,'f',2);
            lTempString = gexReport->getNumberFormat()->formatNumericValue(test->pGage->lfP_T, false, lTempString)
                          + QString("%");
        }
        return CreateCell(lTempString, test->bTestType);
    }

    // TEST_SKEW
    else if(field == TEST_SKEW)
    {
        QString lTempString = ((test->lfSamplesSkew == -C_INFINITE) || (test->bTestType == 'F')) ? GEX_NA :
                                gexReport->getNumberFormat()->formatNumericValue( test->lfSamplesSkew,
                                                            false, QString::number(test->lfSamplesSkew));
        return CreateCell(lTempString, test->bTestType);
    }

    // TEST_KURTOSIS
    else if(field == TEST_KURTOSIS)
    {
        QString lTempString = ((test->lfSamplesKurt == -C_INFINITE) || (test->bTestType == 'F')) ? GEX_NA :
                                gexReport->getNumberFormat()->formatNumericValue( test->lfSamplesKurt,
                                                            false, QString::number(test->lfSamplesKurt));
        return CreateCell(lTempString, test->bTestType);
    }

    // TEST_P0_5
    else if(field == TEST_P0_5)
    {
        QString lTempString(GEX_NA);
        if (test->bTestType != 'F')
        {
            lTempString = file->FormatTestResult(test, test->lfSamplesP0_5, test->res_scal, false);
        }
        return CreateCell(lTempString, test->bTestType);
    }

    // TEST_P2_5
    else if(field == TEST_P2_5)
    {
        QString lTempString(GEX_NA);
        if (test->bTestType != 'F')
        {
            lTempString = file->FormatTestResult(test, test->lfSamplesP2_5, test->res_scal, false);
        }
        return CreateCell(lTempString, test->bTestType);
    }

    // TEST_P10
    else if(field == TEST_P10)
    {
        QString lTempString(GEX_NA);
        if (test->bTestType != 'F')
        {
            lTempString = file->FormatTestResult(test, test->lfSamplesP10, test->res_scal, false);
        }
        return CreateCell(lTempString, test->bTestType);
    }

    // TEST_Q1
    else if(field == TEST_Q1)
    {
        QString lTempString(GEX_NA);
        if (test->bTestType != 'F')
        {
            lTempString = file->FormatTestResult(test, test->lfSamplesQuartile1, test->res_scal, false);
        }
        return CreateCell(lTempString, test->bTestType);
    }

    // TEST_Q2
    else if(field == TEST_Q2)
    {
        QString lTempString(GEX_NA);
        if (test->bTestType != 'F')
        {
            lTempString = file->FormatTestResult(test, test->lfSamplesQuartile2, test->res_scal, false);
        }
        return CreateCell(lTempString, test->bTestType);
    }

    // TEST_Q3
    else if(field == TEST_Q3)
    {
        QString lTempString(GEX_NA);
        if (test->bTestType != 'F')
        {
            lTempString = file->FormatTestResult(test, test->lfSamplesQuartile3, test->res_scal, false);
        }
        return CreateCell(lTempString, test->bTestType);
    }

    // TEST_P90
    else if(field == TEST_P90)
    {
        QString lTempString(GEX_NA);
        if (test->bTestType != 'F')
        {
            lTempString = file->FormatTestResult(test, test->lfSamplesP90, test->res_scal, false);
        }
        return CreateCell(lTempString, test->bTestType);
    }

    // TEST_P97_5
    else if(field == TEST_P97_5)
    {
        QString lTempString(GEX_NA);
        if (test->bTestType != 'F')
        {
            lTempString = file->FormatTestResult(test, test->lfSamplesP97_5, test->res_scal, false);
        }
        return CreateCell(lTempString, test->bTestType);
    }

    // TEST_P99_5
    else if(field == TEST_P99_5)
    {
        QString lTempString(GEX_NA);
        if (test->bTestType != 'F')
        {
            lTempString = file->FormatTestResult(test, test->lfSamplesP99_5, test->res_scal, false);
        }
        return CreateCell(lTempString, test->bTestType);
    }

    // TEST_IQR
    else if(field == TEST_IQR)
    {
        QString lTempString(GEX_NA);
        if (test->bTestType != 'F')
        {
            lTempString = file->FormatTestResult(test,
                                                 test->lfSamplesQuartile3 - test->lfSamplesQuartile1,
                                                 test->res_scal,
                                                 false);
        }
        return CreateCell(lTempString, test->bTestType);
    }

    // TEST_SIGMAIQR
    else if(field == TEST_SIGMAIQR)
    {
        QString lTempString(GEX_NA);
        if (test->bTestType != 'F')
        {
            lTempString = file->FormatTestResult(test,
                                                 test->lfSamplesSigmaInterQuartiles,
                                                 test->res_scal,
                                                 false);
        }
        return CreateCell(lTempString, test->bTestType);
    }

    return "";
}

QString HTMLTableRenderer::CreateRow(const QList<QString>& fields,
                                     CTest* test,
                                     CGexGroupOfFiles* group,
                                     CGexFileInGroup* file) const
{
    QString lHTMLRow = "<tr>\n"\
        "<td bgcolor=\"#CCECFF\" align=\"center\" ><b><a name=\"StatT1000\">" + test->GetTestNumber() + "</a></b></td>\n"\
        "<td bgcolor=\"#F8F8F8\" align=\"left\" >" + test->GetTestName() + "</td>\n";

    QList<QString>::ConstIterator lIterBegin(fields.begin()), lIterEnd(fields.end());
    for (;lIterBegin != lIterEnd; ++lIterBegin)
    {
        // already managed above
        if( *lIterBegin == TEST_TNAME || *lIterBegin == TEST_TNUMBER)
            continue;
        lHTMLRow += CreateItem(*lIterBegin, test, group, file );
    }


    lHTMLRow +="</tr>\n";
    return lHTMLRow;
}

QString HTMLTableRenderer::CreateTable(const CTestContainer&  tests,
                                      const QList<QString>& fields,
                                       const QList<Group *> groups,
                                       bool splitByGroup) const
{
    QString lHTMLTable = "<table border=\"0\" cellspacing=\"0\" width=\"98%\" style=\"font-size:10pt; border-collapse: collapse\"" \
                         "bordercolor=\"#111111\" cellpadding=\"0\">\n";
    QMap<int, QList<CTest*> > lTestByGroup;

    InitShiftColumnLabels();

    CTestContainerConstIter lIterBegin(tests.begin()), lIterEnd(tests.end());

    QList<int> lGroupsId;
    for (int lGroupIter = 0; lGroupIter < groups.size(); ++ lGroupIter)
    {
        lGroupsId << groups.at(lGroupIter)->mNumber.toInt();
    }

    if (splitByGroup)
    {
        // create Map with list of tests by group
        for(;lIterBegin != lIterEnd; ++lIterBegin)
        {
            if (!lTestByGroup.keys().contains(lIterBegin->first))
            {
                // Only add groups if included or no filter
                if (lGroupsId.contains(lIterBegin->first) || lGroupsId.isEmpty())
                {
                    QList<CTest*> lNewList;
                    lNewList.append(lIterBegin->second);
                    lTestByGroup.insert(lIterBegin->first, lNewList);
                }
            }
            else
            {
                lTestByGroup[lIterBegin->first].append(lIterBegin->second);
            }
        }
        // draw the table
        QMapIterator<int, QList<CTest*> > lIter(lTestByGroup);
        while (lIter.hasNext())
        {
            lIter.next();
            if (!gexReport->getGroupsList()[lIter.key()])
                continue;
            // write Group name
            lHTMLTable += "<tr><td>&nbsp;</td></tr>"
                            "<tr>\n<td bgcolor=\"#CCECFF\" align=\"center\" ><b>" +
                            gexReport->getGroupsList()[lIter.key()]->strGroupName + "</b></td></tr>\n";
            // write header line
            lHTMLTable += CreateHeader(fields);
            // write data
            for (int lTestId = 0; lTestId < lIter.value().size(); ++lTestId)
            {
                lHTMLTable += CreateRow(fields,
                                        lIter.value().at(lTestId),
                                        gexReport->getGroupsList()[lIter.key()],
                                        gexReport->getGroupsList()[lIter.key()]->GetFilesList()[0]);
            }
        }
    }
    else
    {
        lHTMLTable += CreateHeader(fields);
        for(;lIterBegin != lIterEnd; ++lIterBegin)
        {
            // Only add groups if included or no filter
            if (lGroupsId.contains(lIterBegin->first) || lGroupsId.isEmpty())
            {
                lHTMLTable += CreateRow(fields,
                                    lIterBegin->second,                                                  // CTest *
                                    gexReport->getGroupsList()[lIterBegin->first],                       // Group
                                    gexReport->getGroupsList()[lIterBegin->first]->GetFilesList()[0]);   // pFile
            }
        }
    }


    lHTMLTable +="</table>\n";

    return lHTMLTable;
}
