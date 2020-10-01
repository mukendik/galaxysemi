#include <float.h>
#include <limits>
#include <limits.h>

#include <QMap>
#include <QSqlQuery>
#include <QSqlResult>

#include "cpart_info.h"
#include "gex_report_unit.h"
#include "gex_group_of_files.h"
#include "gex_file_in_group.h"
#include "shift_report_unit.h"
#include "gex_report.h"
#include "product_info.h"
#include <gqtl_log.h>
#include "engine.h"

// To avoid conflict between MicroSoft max macro and STL max() function
#undef max


bool sortGroupOfFile(CGexGroupOfFiles* first, CGexGroupOfFiles* second)
{
    if (first->strGroupName > second->strGroupName)
        return true;
    else
        return false;
}

namespace GS
{
namespace Gex
{
ShiftReportUnit::ShiftReportUnit(CGexReport* gr, const QString &key
                                 //CReportOptions &ro
                                 //const QList<CGexGroupOfFiles*> lgof
                                 )
    : ReportUnit(gr, key)
{
    //        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("GexReport:%1").arg( gr));
    mCalcMethod=mGexReport->GetOption("adv_shift", "calc_method").toString();
    mSortColumn = mGexReport->GetOption("adv_shift", "sort_column").toString();
    bool ok=false;
    mMaxNumberOfLinesPerSlideInPres=mGexReport->GetOption(
                "adv_shift", "max_lines_per_slide").toInt(&ok);
    if (!ok)
        mMaxNumberOfLinesPerSlideInPres=34;
    mTypeFilesInGroup = samples;
}

ShiftReportUnit::~ShiftReportUnit()
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "ShiftReportUnit destructor...");
    if (!mPartsAlertsMap.isEmpty())
        foreach(ShiftAlert* a, mPartsAlertsMap.values())
            if (a)
            { delete a; a=0; }
    mPartsAlertsMap.clear();
    // Shift alerts has been deleted in the last loop
    mTestsAlertsMap.clear();
    mPercentLSAlertsMap.clear();
}

QString ShiftReportUnit::PrepareSection(bool bValidSection)
{
    GSLOG(SYSLOG_SEV_NOTICE,
          QString("ValidSection = %1").arg( bValidSection?"true":"false")
          .toLatin1().constData());

    QString strOutputFormat=mGexReport->GetOption("output", "format").toString();

    if (strOutputFormat=="CSV") //if(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
    {
        // Generating .CSV report file.
        fprintf(mGexReport->getReportFile(),"\n\nShift analysis\n");
    }
    else
        if(mGexReport->getReportOptions()->isReportOutputHtmlBased())
            //if (strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
            //if(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
        {
            // Generating HTML report file.
            // Open advanced.htm file
            QString r=OpenFile();
            if(r.startsWith("error") )
                return "error : cant open file : "+r;	//Stdf::ReportFile;

            mGexReport->WriteHeaderHTML(mGexReport->getReportFile(),"#000000");	// Default: Text is Black

            // Title + bookmark
            WriteHtmlSectionTitle(mGexReport->getReportFile(),"all_advanced","More Reports: Shift analysis");
        }
    return "ok";
}

QString ShiftReportUnit::WriteGroupsSummaryTable(QString lTitlesAttributes, QString lTableAtts, QString lFirstTableLineAtts)
{
    mGexReport->BeginTable(lTableAtts);
    QList<QString> labels = QList<QString>() << "Groups summary" << "  " << "(Top)";
    QList<QString> anchors = QList<QString>() << "name=\"GroupsSummary\"" << "" << "href=\"#Top\"";
    mGexReport->WriteTableRow(labels, lTitlesAttributes, anchors);
    mGexReport->EndTable();
    mGexReport->EndLine();

    // Groups summary :
    mGexReport->BeginTable(lTableAtts);
    //mGexReport->WriteInfoLine(QString("Groups (%1): ").arg(mSamplesGroups.size()).toLatin1().data(), "<br>\n");
    QList<QString> l;
    l << "Group name" << "Part type" << "Total valid file" << "Number of tests" << "Parts count (from MIR)";
    mGexReport->WriteTableRow(l, lFirstTableLineAtts);
    foreach(CGexGroupOfFiles* gof, mSamplesGroups)
        if (gof)
        {
            l.clear();
            l << gof->strGroupName;
            l << (gof->pFilesList.size()>0?gof->pFilesList.at(0)->getMirDatas().szPartType:"?");
            l << QString::number(gof->GetTotalValidFiles());
            l << QString::number(gof->cMergedData.GetNumberOfTests());
            l << QString::number( gof->GetTotalMirDataPartCount() );
            mGexReport->WriteTableRow(l);
        }
        else
        {
            return "error : found a NULL GroupOfFiles";
        }
    mGexReport->EndTable();
    return "ok";
}




///////////////////////////////////////////////////////////////////
/// 1/ Get the first test in the reference group (we take it from the first group because we have the same list of tests)
/// 2/ Loop on all tests, and for each test (test_ref):
///     i)   create a list that contains all groups and the corresponding tests in these groups to the reference test
///          which is the test_ref
///     ii)  get the list of parts from the first element of the previous list => the parts of the first file
///     iii) loop on all parts, and for each part:
///         a. find all corresponding tests in all groups to this part and this test
///         b. compute the max delta for the current test using the list created below
///         c. find the max percentage of shift
///         d. fill the shift alert if exists
///
////////////////////////////////////////////////////////////
QString ShiftReportUnit::SearchForShiftAlerts()
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString("Searching for shift alert on %1 groups...")
          .arg( mSamplesGroups.size()).toLatin1().constData());

    emit UpdateProcessString(" Building Shift report...");

    mPartsAlertsMap.clear();
    mTestsAlertsMap.clear();
    mPercentLSAlertsMap.clear();

    // This code is just for debug
    //QFile lCSVFile(QString("/tmp") + QDir::separator() + "shiftAlerts.csv");
    //lCSVFile.open(QIODevice::WriteOnly);
    //QTextStream lCVSTempOutput(&lCSVFile);
    float lLimitSpacePercent=mGexReport->GetOption("adv_shift", "limit_space_percent").toFloat();

    // All Groups should have the same list of CTest. Let's consider the first group as reference.
    CTest* lListOfTests=mSamplesGroups.at(0)->cMergedData.ptMergedTestList;
    CTest* ptTestCell=lListOfTests;
    while(ptTestCell != NULL)
    {
        if ( ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL
             ||   ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL
             ||  ptTestCell->GetCurrentLimitItem()->lfHighLimit==-C_INFINITE
             || ptTestCell->GetCurrentLimitItem()->lfLowLimit==C_INFINITE
             )
        {
            ptTestCell = ptTestCell->GetNextTest();
            continue;
        }
        QMap<CGexGroupOfFiles*, CTest*> lTestsByGroup;
        lTestsByGroup.clear();

        CGexGroupOfFiles* lGroupOfFile = NULL;
        // Insert, for the current test, a list of <test group, the test that has the same test number>
        for(int i=0; i < mSamplesGroups.count(); i++)
        {
            lGroupOfFile = mSamplesGroups.at(i);
            if (!lGroupOfFile)
                continue;

            // GCORE-4612 HTH
            // Retrieve the corresponding test in each group. Only one test per group can match.
            CTest * lTest = lGroupOfFile->FindTestCell(ptTestCell->lTestNumber,
                                                       ptTestCell->strTestName, ptTestCell->lPinmapIndex);

            if (lTest == NULL)
                continue;

            // Insert the test found for current group
            lTestsByGroup.insert( lGroupOfFile, lTest);
        }

        if (lTestsByGroup.size()!=mSamplesGroups.size())
        {
            GSLOG(SYSLOG_SEV_WARNING,
                  QString("The test %1 is not available in all groups. Ignoring.")
                  .arg( ptTestCell->lTestNumber).toLatin1().constData());
            ptTestCell = ptTestCell->GetNextTest();
            continue;
        }
        // Let s search for list of parts info
        QList<CPartInfo*> lPartsInfo;
        lPartsInfo.clear();
        QList<CGexGroupOfFiles*> lGexGroupFile = lTestsByGroup.keys();
        qSort(lGexGroupFile.begin(), lGexGroupFile.end(), sortGroupOfFile);
        lGroupOfFile = NULL;
        for(int comptGOF=0; comptGOF < lGexGroupFile.size(); comptGOF++)
        {
            lGroupOfFile = lGexGroupFile.at(comptGOF);
            if (!lGroupOfFile)
                return "error : the group of file is null";
            CGexFileInGroup* lFileInGroup = lGroupOfFile->pFilesList.at(0);
            if (lFileInGroup)
            {
                lPartsInfo=lFileInGroup->pPartInfoList;
                break;
            }
        }
        if (lPartsInfo.isEmpty())
        {
            // next test
            GSLOG(SYSLOG_SEV_WARNING, "no PartInfo found for this test");
            ptTestCell = ptTestCell->GetNextTest();
            continue;
        }

        int index=-1;
        QMap<CGexGroupOfFiles*, double> testResultPerGroup;
        CPartInfo* lPartInfo = NULL;
        // Loop on all parts and find all shift
        for (int comptPI=0; comptPI<lPartsInfo.count(); ++comptPI)
        {
            lPartInfo = lPartsInfo.at(comptPI);
            testResultPerGroup.clear(); // clear items of previous part (5920)
            index++; // test result index in TestResults. Thanks to the dataprocessing clean_samples option, no more retest noises.
            lGexGroupFile = lTestsByGroup.keys();
            qSort(lGexGroupFile.begin(), lGexGroupFile.end(), sortGroupOfFile);
            lGroupOfFile = NULL;
            // Find all corresponding tests in all groups to this part and this test
            for(int comptGOF=0; comptGOF < lGexGroupFile.size(); comptGOF++)
            {
                lGroupOfFile = lGexGroupFile.at(comptGOF);
                if (!lGroupOfFile)
                    continue;
                CTest* lTest =lGroupOfFile->FindTestCell(
                            ptTestCell->lTestNumber, ptTestCell->strTestName, ptTestCell->lPinmapIndex);
                if (!lTest)
                    continue;

                if (lTest->m_testResult.count()==0)
                {
                    //GSLOG(SYSLOG_SEV_WARNING, QString("Test %1 has no result in group %2").arg( t->lTestNumber, gof->strGroupName).toLatin1().constData() );.arg(                     //GSLOG(SYSLOG_SEV_WARNING, "Test %1 has no result in group %2").arg( t->lTestNumber.arg( gof->strGroupName).toLatin1().constData() );
                    continue; // next group
                }

                if (lTest->m_testResult.count()<=index)
                {
                    // appear when for a given group there is no test result for this test/part (often because of stop on first fail)
                    //GSLOG(SYSLOG_SEV_WARNING, QString("No more data for this group (%1) at this index (%2). Cancelling.").arg(gof->strGroupName).toLatin1().constData(), index));.arg(                     //GSLOG(SYSLOG_SEV_WARNING, "No more data for this group (%1) at this index (%2). Cancelling.").arg(gof->strGroupName).toLatin1().constData().arg( index);
                    break;
                }

                if (!lTest->m_testResult.isValidResultAt(index))
                    continue;

                double v=GEX_C_DOUBLE_NAN;
                v=lTest->m_testResult.resultAt(index); //int i=t->FindValidResult(index, v); // cant use FindValidResult() because it will jump over NAN results.

                testResultPerGroup.insert(lGroupOfFile, v);
            }

            if (testResultPerGroup.size()<1)
                continue;  // continue to next part info (5920)

            CGexGroupOfFiles* mingroup=0, *maxgroup=0;
            CGexGroupOfFiles* ref_group = mSamplesGroups.at(0);
            QMap<int, double> lTestPerGroup;
            QMapIterator<CGexGroupOfFiles*, double> lIt(testResultPerGroup);
            while (lIt.hasNext()) {
                lIt.next();
                lTestPerGroup.insert(lIt.key()->GetGroupId(), lIt.value());
            }

            double lMaxDelta = ComputeMaxDelta(lTestPerGroup, mingroup, maxgroup, ref_group->GetGroupId());
            if (lMaxDelta==GEX_C_DOUBLE_NAN || lMaxDelta==0)
                continue; // next Part ID

            // Find the max percentage of shift
            double lPourcentOfShift(0.0);
            if (mCalcMethod == "measuring_value_of_baseline")
            {
                double lRefTestResult = testResultPerGroup.value(ref_group);
                if (lRefTestResult != 0)
                    lPourcentOfShift = lMaxDelta/lRefTestResult;
            }
            else
            {
                double lMaxValue=qAbs(ptTestCell->GetCurrentLimitItem()->lfHighLimit - ptTestCell->GetCurrentLimitItem()->lfLowLimit); // (5921)
                if (lMaxValue!=0)
                    lPourcentOfShift=lMaxDelta/lMaxValue;
            }

            if ( lPourcentOfShift*100. > lLimitSpacePercent)
            {
                ShiftAlert* lShiftAlert = new ShiftAlert(ptTestCell, lPartInfo);
                lShiftAlert->mTestResultsPerGroup=testResultPerGroup;
                lShiftAlert->mParams.insert("maxdelta", lMaxDelta);
                lShiftAlert->mParams.insert("percent", lPourcentOfShift);
                lShiftAlert->mMaxGroup = maxgroup;
                lShiftAlert->mMinGroup = mingroup;
                if (testResultPerGroup.value(maxgroup) >
                    testResultPerGroup.value(mingroup))
                {
                    lShiftAlert->mPolarity = positive;
                }
                else
                {
                    lShiftAlert->mPolarity = negative;
                }

                // for shift alert debug file
                //lCVSTempOutput << ptTestCell->lTestNumber << ";"
                //               << lPartInfo->getPartID()<< ";"
                //               << lMaxDelta << ";"
                //               << lPourcentOfShift << ";";
                //lCVSTempOutput << (maxgroup?maxgroup->strGroupName:"?")+QString(" vs ")+(mingroup?mingroup->strGroupName:"?")<< ";" << testResultPerGroup.value(maxgroup) << ";" << testResultPerGroup.value(mingroup) <<"\n";
                mPartsAlertsMap.insert(lPartInfo->getPartID(), lShiftAlert);
                mTestsAlertsMap.insert(ptTestCell, lShiftAlert);
                mPercentLSAlertsMap.insert(lPourcentOfShift, lShiftAlert);
            }
        } // for all Part
        ptTestCell = ptTestCell->GetNextTest();
    } // for all test

    //lCVSTempOutput.flush();
    //lCSVFile.close();

    return "ok";
}





QString ShiftReportUnit::WriteOptionsTable(QString lTableAtts, QString lFirstTableLineAtts)
{
    QList< QString > l;
    mGexReport->BeginTable(lTableAtts);
    l.clear(); l<<"Option"<<"Value";
    mGexReport->WriteTableRow(l, lFirstTableLineAtts);
    l.clear(); l<<"Calculation method"<<mCalcMethod;
    mGexReport->WriteTableRow(l, " style=\"text-align:center\"");
    l.clear();
    float lsp=mGexReport->GetOption("adv_shift", "limit_space_percent").toFloat();
    l<<"Limit space percent cutoff"<<QString::number(lsp);
    mGexReport->WriteTableRow(l, " style=\"text-align:center\"");
    mGexReport->EndTable();
    return "ok";
}



QString ShiftReportUnit::WriteDeviceReport(QString lTitlesAttributes, QString lTableAtts, QString lFirstTableLineAtts)
{
    unsigned lNumberOfLinesWritten=0;
    QString outputFormat = mGexReport->GetOption("output", "format").toString();

    mGexReport->BeginTable(lTableAtts);
    QList<QString> labels = QList<QString>() << "Summary by device" << "  " << "(Top)";
    QList<QString> anchors = QList<QString>() << "name=\"SummaryByDevice\"" << "" << "href=\"#Top\"";
    mGexReport->WriteTableRow(labels, lTitlesAttributes, anchors);
    mGexReport->EndTable();
    mGexReport->EndLine();

    mGexReport->BeginTable(lTableAtts);
    QList<QString> lLabels, lAnchors, title;
    lLabels.clear();
    title.clear();
    if(outputFormat=="HTML")
        title<<"PartID"<<"Number of Tests<br>over limit space threshold";
    else
        title<<"PartID"<<"Number of Tests over limit space threshold";
    mGexReport->WriteTableRow(title, lFirstTableLineAtts);

    QMap<int, QString> NumOfAlertsPartIDsMap;
    foreach(const QString &partid, mPartsAlertsMap.uniqueKeys())
    {
        NumOfAlertsPartIDsMap.insertMulti(mPartsAlertsMap.count(partid), partid);
    }

    QMap<int, QString>::iterator it=NumOfAlertsPartIDsMap.end(); --it;
    for (; it!=NumOfAlertsPartIDsMap.end(); --it)
    {
        lLabels.clear(); lAnchors.clear();
        lLabels << it.value(); //partid;
        lAnchors <<  QString("href=\"#ByDeviceP%1\"").arg(it.value());
        lLabels<<QString::number(it.key());     //lPartsAlertsMap.count(partid));
        mGexReport->WriteTableRow(lLabels, "style=\"text-align:center\"", lAnchors); //, QString("parent.location='#%1'").arg(it.value()));

        // if the output format is HTML or CSV, don't add empty line
        if (/*outputFormat != "HTML" && */outputFormat !="CSV")
        {
            lNumberOfLinesWritten++;
            if (lNumberOfLinesWritten>mMaxNumberOfLinesPerSlideInPres)
            {
                lNumberOfLinesWritten=0;
                mGexReport->EndTable();
                mGexReport->EndLine();
                //mGexReport->WritePageBreak(mGexReport->getReportFile(), false);
                mGexReport->WritePageBreak(); // actually flush pres (PPT, ODP) pages
                mGexReport->BeginTable(lTableAtts);
                mGexReport->WriteTableRow(title, lFirstTableLineAtts);
            }
        }

        if (it==NumOfAlertsPartIDsMap.begin())
            break;
    }

    mGexReport->EndTable();
    mGexReport->EndLine();

    mGexReport->BeginTable(lTableAtts);
    labels = QList<QString>() << "Details by device" << "  " << "(Top)";
    anchors = QList<QString>() << "name=\"DetailsByDevice\"" << "" << "href=\"#Top\"";
    mGexReport->WriteTableRow(labels, lTitlesAttributes, anchors);
    mGexReport->EndTable();
    mGexReport->EndLine();

    mGexReport->BeginTable(lTableAtts);
    lLabels.clear();
    title.clear();
    if (mCalcMethod != "measuring_value_of_baseline")
    {
        title << "PartID" << "Test" << "Test name" <<"LL";
        if (outputFormat=="CSV")
            title <<"Units" << "HL" << "Units" << "Max delta" << "Units";
        else
            title << "HL" << "Max delta";
        title <<"Groups with max delta" << "Percent of limit space";
    }
    else
    {
        title << "Test #" << "Test name" << "PartID" << "LL";
        if (outputFormat=="CSV")
            title <<"Units" << "HL" << "Units";
        else
            title << "HL";
        title << "Polarity" << "Guilty groups" << "Percent of Actual Shift";
    }
    mGexReport->WriteTableRow(title, lFirstTableLineAtts);

    it=NumOfAlertsPartIDsMap.end(); it--;
    for (; it!=NumOfAlertsPartIDsMap.end(); it--) // (5919)
    {
        QString pid=it.value();
        QList< ShiftAlert* > values = mPartsAlertsMap.values(pid);
        qSort(values.begin(), values.end(), ShiftAlert::percentGreatThan);
        for (int i=0; i < values.size(); ++i)
        {
            ShiftAlert* sa=values.at(i);
            if (!sa || !sa->mTest)
                continue;
            lLabels.clear(); lAnchors.clear();
            if (mCalcMethod != "measuring_value_of_baseline")
            {
                lLabels << pid; lAnchors << QString("name=\"ByDeviceP%1\"").arg(pid);
                lLabels << QString::number(sa->mTest->lTestNumber);    //l<<m.value("test number").toString() << m.value("test name").toString();
                QString href=QString("#_gex_drill--drill_chart=adv_trend--data=%1").arg(sa->mTest->lTestNumber);
                lAnchors << QString("href=\"%1\"").arg(href);
                lLabels << sa->mTest->strTestName;
                lAnchors << QString("href=\"%1\"").arg(href);
                lLabels << sa->mTest->GetCurrentLimitItem()->szLowL;
                lLabels << sa->mTest->GetCurrentLimitItem()->szHighL;
                if (outputFormat=="CSV")
                {
                    lLabels << sa->mParams.value("maxdelta").toString();
                    lLabels << QString(sa->mTest->szTestUnits);
                }
                else
                {
                    lLabels << sa->mParams.value("maxdelta").toString() + " " + QString(sa->mTest->szTestUnits);
                }
            }
            else
            {
                lLabels << QString::number(sa->mTest->lTestNumber);
                QString href=QString("#_gex_drill--drill_chart=adv_trend--data=%1").arg(sa->mTest->lTestNumber);
                lAnchors << QString("href=\"%1\"").arg(href);
                lLabels << sa->mTest->strTestName;
                lAnchors << QString("href=\"%1\"").arg(href);
                lLabels << pid; lAnchors << QString("name=\"ByDeviceP%1\"").arg(pid);
                lLabels << sa->mTest->GetCurrentLimitItem()->szLowL;
                lLabels << sa->mTest->GetCurrentLimitItem()->szHighL;
                lLabels << ((sa->mPolarity == positive)? "+" : "-");
            }
            lLabels << (sa->mMinGroup?sa->mMinGroup->strGroupName:"?")+QString(" vs ")+(sa->mMaxGroup?sa->mMaxGroup->strGroupName:"?");    //l<<m.value("mingroup").toString()+" vs "+m.value("maxgroup").toString();
            lLabels << QString::number((double)(sa->mParams.value("percent").toDouble()*100.),'g', 3)+"%";
            mGexReport->WriteTableRow(lLabels, "style=\"text-align:center\"", lAnchors);
            // if the output format is HTML or CSV, not make line break
            if (/*outputFormat != "HTML" && */outputFormat !="CSV")
            {
                lNumberOfLinesWritten++;
                if (lNumberOfLinesWritten>mMaxNumberOfLinesPerSlideInPres)
                {
                    lNumberOfLinesWritten=0;
                    mGexReport->EndTable();
                    mGexReport->EndLine();
                    //mGexReport->WritePageBreak(mGexReport->getReportFile(), false);
                    mGexReport->WritePageBreak(); // actually flush pres (PPT, ODP) pages
                    mGexReport->BeginTable(lTableAtts);
                    mGexReport->WriteTableRow(title, lFirstTableLineAtts);
                }
            }
        }
        if (it==NumOfAlertsPartIDsMap.begin())
            break;
    }
    mGexReport->EndTable();
    mGexReport->EndLine();
    return "ok";
}

QString ShiftReportUnit::WriteTestReport(const QString &lTitlesAttributes,
                                         const QString &lTableAtts,
                                         const QString &lFirstTableLineAtts)
{
    int lNumberOfLinesWritten=0;
    QString outputFormat = mGexReport->GetOption("output", "format").toString();

    // Write Test summary
    // If we have Samples and Controls
    // 1/ Add the controls header to the table's header
    // 2/ Read all samples, and for each sample
    //      i)   Write the sample result
    //      ii)  Calculate the shift of the controls occarding to the references group
    //      iii) Write the controls results (only the current line)
    //      iv)  Calculate the new mesurement and write it

    mGexReport->BeginTable(lTableAtts);
    QList<QString> labels = QList<QString>() << "Summary by test" << "  " << "(Top)";
    QList<QString> anchors = QList<QString>() << "name=\"SummaryByTest\"" << "" << "href=\"#Top\"";
    mGexReport->WriteTableRow(labels, lTitlesAttributes, anchors);
    mGexReport->EndTable();
    mGexReport->EndLine();

    mGexReport->BeginTable(lTableAtts);
    QList< QString > lLabels;
    QList< QString > lAnchors;
    lLabels.clear();

    QList< QString > lTitle, lHearder;
    QString lMaxDelta;
    QString lLimitSpace;
    lTitle<<"Test number"<<"Test name";
    if(outputFormat=="HTML")
    {
        if (mCalcMethod == "measuring_value_of_baseline")
            lLimitSpace = "# of parts over threshold";
        else
            lLimitSpace = "# of parts over <br>limit space threshold";
        lMaxDelta = "Max % <br>of limit space delta" ;
    }
    else
    {
        if (mCalcMethod == "measuring_value_of_baseline")
            lLimitSpace = "# of parts over threshold";
        else
            lLimitSpace = "# of parts over limit space threshold";
        lMaxDelta = "Max % of limit space delta" ;
    }

    // Add Unit only in CSV format
    if (outputFormat=="CSV")
        lHearder << lLimitSpace << "Min LL" << "Units" << "Min HL" << "Units" ;
    else
        lHearder << lLimitSpace << "Min LL" << "Min HL" ;


    if (mCalcMethod == "measuring_value_of_baseline")
    {
        lHearder << "Polarity" << "Max %" << "Worst part ID" << "Guilty groups";
    }
    else
    {
        // case 6286 and case 7214
        lHearder << lMaxDelta << "Polarity" << "Worst delta" << "Worst part ID" << "Guilty groups";
    }

    // Write the controls's header only if we have both controls and samples
    if (mTypeFilesInGroup == both)
    {
        lTitle << lHearder << lHearder << "Max Delta Normalized" << "Normalized shift";
    }
    else
    {
        lTitle << lHearder;
    }
    lLabels<<lTitle;

    // Write the header in the output file
    mGexReport->WriteTableRow(lLabels, lFirstTableLineAtts);

    // Insert <shift value, test> in the map ordered by the option choosen in the option
    QMap<double, CTest*> NumOfAlertsPerTestMap;
    if (mSortColumn == "nbr_limit_threshold")
    {
        foreach(CTest* t, mTestsAlertsMap.uniqueKeys())
        {
            NumOfAlertsPerTestMap.insertMulti(mTestsAlertsMap.count(t), t);
        }
    }
    else
    {
        foreach(CTest* t, mTestsAlertsMap.uniqueKeys())
        {
            QList< ShiftAlert* > values = mTestsAlertsMap.values(t);
            if (!values.empty())
            {
                // Sort shift alert values to add only the greatest one to the map
                qSort(values.begin(), values.end(), ShiftAlert::percentGreatThan);
                ShiftAlert* lShiftAlert=values.first();
                if (lShiftAlert)
                {
                    double val = (double)(lShiftAlert->mParams.value("percent").toDouble()*100.);
                    NumOfAlertsPerTestMap.insertMulti(val, t);
                }
            }
        }
    }


    // Write the lines from the highest to the lowest
    QMap<double, CTest*>::iterator it2=NumOfAlertsPerTestMap.end(); --it2;
    for (; it2!=NumOfAlertsPerTestMap.end(); --it2)
    {
        lLabels.clear(); lAnchors.clear();
        CTest* lTest=it2.value();
        if (!lTest)
            continue;
        lLabels << QString::number(lTest->lTestNumber);
        lAnchors << QString("href=\"#ByTestT%1\"").arg(lTest->lTestNumber);

        lLabels << lTest->strTestName;
        lAnchors << QString("href=\"#ByTestT%1\"").arg(lTest->lTestNumber);

        lLabels << QString::number(mTestsAlertsMap.count(lTest)/*it2.key()*/);
        lAnchors << "";

        double lMaxDeltaSamples(0.0);

        QList< ShiftAlert* > values = mTestsAlertsMap.values(lTest);
        double lPercentSamples(0.0);
        if (values.empty())
            lLabels << "n/a";
        else
        {
            qSort(values.begin(), values.end(), ShiftAlert::percentGreatThan);
            ShiftAlert* lShiftAlert = values.first();
            if (lShiftAlert)
            {
                lLabels << lTest->GetCurrentLimitItem()->szLowL;
                lLabels << lTest->GetCurrentLimitItem()->szHighL;
                lPercentSamples = (double)(lShiftAlert->mParams.value("percent").toDouble()*100.);
                if (mCalcMethod == "measuring_value_of_baseline")
                {
                    lLabels << ((lShiftAlert->mPolarity == positive)? "+" : "-");
                    lLabels << QString::number(lPercentSamples,'g',3)+"%";
                }
                else
                {
                    lLabels << QString::number(lPercentSamples,'g',3)+"%";
                    lLabels << ((lShiftAlert->mPolarity == positive)? "+" : "-");
                    lMaxDeltaSamples = lShiftAlert->mParams.value("maxdelta").toDouble();
                    lLabels << QString::number(lMaxDeltaSamples);
                }
                lLabels << lShiftAlert->mPartInfo->getPartID(); // Worst part id
                lLabels << (lShiftAlert->mMinGroup?lShiftAlert->mMinGroup->strGroupName:"?")+QString(" vs ")
                           +(lShiftAlert->mMaxGroup?lShiftAlert->mMaxGroup->strGroupName:"?");
            }
            else
                lLabels << "n/a";
        }



        // Add the controls information if both Controls and Samples are selected
        if (mTypeFilesInGroup == both)
        {
            if (values.empty())
                lLabels << "n/a";
            else
            {
                ShiftAlert* lShiftAlert = values.first();
                ShiftAlert* lControlShiftAlert;
                if (lShiftAlert)
                {
                    unsigned short lNbreShiftAlert(0);
                    if(!CalculateMaxControlsDelta(lShiftAlert, lControlShiftAlert, lNbreShiftAlert))
                    {
                        GSLOG(SYSLOG_SEV_ERROR, "Problem in the calculation of the max delta");
                        return "error: Error in the calculation of the Shift";
                    }
                    lLabels << QString::number(lNbreShiftAlert);
                    lLabels << lTest->GetCurrentLimitItem()->szLowL;
                    lLabels << lTest->GetCurrentLimitItem()->szHighL;
                    double lPercentControl = (double)(lControlShiftAlert->mParams.value("percent").toDouble());
                    double lMaxDeltaControl(0.0);
                    if (mCalcMethod == "measuring_value_of_baseline")
                    {
                        lLabels << ((lControlShiftAlert->mPolarity == positive)? "+" : "-");
                        lLabels << QString::number(lPercentControl,'g',3)+"%";
                    }
                    else
                    {
                        lLabels << QString::number(lPercentControl,'g',3)+"%";
                        lLabels << ((lControlShiftAlert->mPolarity == positive)? "+" : "-");
                        lMaxDeltaControl = lControlShiftAlert->mParams.value("maxdelta").toDouble();
                        lLabels << QString::number(lMaxDeltaControl);
                    }
                    if (lControlShiftAlert->mPartInfo)
                    {
                        lLabels << lControlShiftAlert->mPartInfo->getPartID(); // Worst part id
                    }
                    lLabels << (lControlShiftAlert->mMinGroup?lControlShiftAlert->mMinGroup->strGroupName:"?")+QString(" vs ")
                               +(lControlShiftAlert->mMaxGroup?lControlShiftAlert->mMaxGroup->strGroupName:"?");

                    // Add the new columns Normalized shift
                    if (lControlShiftAlert->mPolarity != lShiftAlert->mPolarity)
                    {
                        if (mCalcMethod != "measuring_value_of_baseline")
                        {
                            lLabels << QString::number(lMaxDeltaControl + lMaxDeltaSamples);
                        }
                        lLabels << QString::number(lPercentControl + lPercentSamples,'g',3)+"%";
                    }
                    else
                    {
                        if (mCalcMethod != "measuring_value_of_baseline")
                        {
                            lLabels << QString::number(lMaxDeltaControl - lMaxDeltaSamples);
                        }
                        lLabels << QString::number(qAbs(lPercentControl - lPercentSamples),'g',3)+"%";
                    }
                }
                else
                {
                    lLabels << "n/a";
                }

            }
        }

        mGexReport->WriteTableRow(lLabels, "style=\"text-align:center\"", lAnchors); //, QString("parent.location='#%1'").arg(it.value()));

        // if the output format is HTML or CSV, not make line break
        if (/*outputFormat != "HTML" && */outputFormat !="CSV")
        {
            lNumberOfLinesWritten++;
            // New table in new page
            if (lNumberOfLinesWritten>(int)mMaxNumberOfLinesPerSlideInPres)
            {
                lNumberOfLinesWritten=0;
                mGexReport->EndTable();
                mGexReport->EndLine();
                //mGexReport->WritePageBreak(mGexReport->getReportFile(), false);
                mGexReport->WritePageBreak(); // actually flush pres (PPT, ODP) pages
                mGexReport->BeginTable(lTableAtts);
                mGexReport->WriteTableRow(lTitle, lFirstTableLineAtts);
            }
        }
        if (it2 == NumOfAlertsPerTestMap.begin())
            break;
    }
    mGexReport->EndTable();
    mGexReport->EndLine();

    mGexReport->WritePageBreak(); //mGexReport->getReportFile(), false);

    // Write Test details
    mGexReport->BeginTable(lTableAtts);
    labels = QList<QString>() << "Details by test" << "  " << "(Top)";
    anchors = QList<QString>() << "name=\"DetailsByTest\"" << "" << "href=\"#Top\"";
    mGexReport->WriteTableRow(labels, lTitlesAttributes, anchors);
    mGexReport->EndTable();
    mGexReport->EndLine();

    mGexReport->BeginTable(lTableAtts);
    lAnchors.clear(); lLabels.clear();lTitle.clear();

    lTitle<<"Test #"<<"Test name"<<"Part ID"<<"LL";
    if (outputFormat=="CSV")
    {
        lTitle << "Units"<<"HL"<<"Units";
        if (mCalcMethod != "measuring_value_of_baseline")
            lTitle <<"Max delta"<<"Units";
    }
    else
    {
        lTitle <<"HL";
        if (mCalcMethod != "measuring_value_of_baseline")
            lTitle <<"Max delta";
    }
    lTitle <<"Polarity" <<"Guilty groups";
    if (mCalcMethod == "measuring_value_of_baseline")
    {
        lTitle <<"Percent of Actual Shift";
    }
    else
    {
        lTitle <<"Percent of limit space delta";
    }
    lLabels<<lTitle;
    mGexReport->WriteTableRow(lLabels, lFirstTableLineAtts);



    // insert tests in the map ordered by the option choosen in the option
    if (mCalcMethod == "measuring_value_of_baseline")
    {
        QMap <double, QPair<CTest*, ShiftAlert*> > lAlertsSortedByPercent;
        NumOfAlertsPerTestMap.clear();
        foreach(CTest* t, mTestsAlertsMap.uniqueKeys())
        {
            QList< ShiftAlert* > values = mTestsAlertsMap.values(t);
            for (int i=0; i<values.size(); ++i)
            {
                ShiftAlert* sa = values[i];
                double val = (double)(sa->mParams.value("percent").toDouble()*100.);
                lAlertsSortedByPercent.insertMulti(val, qMakePair(t, sa));
            }
        }

        QMap <double, QPair<CTest*, ShiftAlert*> >::iterator it3 = lAlertsSortedByPercent.end();
        --it3;
        for (; it3 != lAlertsSortedByPercent.end(); it3--)
        {
            QPair<CTest*, ShiftAlert*> lPairShiftTest = it3.value();
            CTest* t=lPairShiftTest.first;
            if (!t)
                continue;
            ShiftAlert* sa=lPairShiftTest.second;
            if (!sa || !sa->mTest)
                continue;
            lLabels.clear(); lAnchors.clear();
            QString href=QString("#_gex_drill--drill_chart=adv_trend--data=%1").arg(t->lTestNumber);
            lLabels << QString::number(t->lTestNumber);
            lAnchors << QString("name=\"ByTestT%1\" href=\"%2\"").arg(t->lTestNumber).arg(href);
            lLabels << t->strTestName;
            lAnchors << QString("name=\"ByTestT%1\" href=\"%2\"").arg(t->lTestNumber).arg(href);
            lLabels << sa->mPartInfo->getPartID();
            lLabels << t->GetCurrentLimitItem()->szLowL;
            lLabels << t->GetCurrentLimitItem()->szHighL;
            lLabels << ((sa->mPolarity == positive)? "+" : "-");
            lLabels << (sa->mMinGroup?sa->mMinGroup->strGroupName:"?")+QString(" vs ")+(sa->mMaxGroup?sa->mMaxGroup->strGroupName:"?");
            lLabels << QString::number((double)(sa->mParams.value("percent").toDouble()*100.),'g',3)+"%";
            mGexReport->WriteTableRow(lLabels, "style=\"text-align:center\"", lAnchors);
            // if the output format is HTML or CSV, not make line break
            if (outputFormat !="CSV")
            {
                lNumberOfLinesWritten++;
                if (lNumberOfLinesWritten>(int)mMaxNumberOfLinesPerSlideInPres)
                {
                    lNumberOfLinesWritten=0;
                    mGexReport->EndTable();
                    mGexReport->EndLine();
                    mGexReport->WritePageBreak(); // actually flush pres (PPT, ODP) pages
                    mGexReport->BeginTable(lTableAtts);
                    mGexReport->WriteTableRow(lTitle, lFirstTableLineAtts);
                }
            }
            if (it3==lAlertsSortedByPercent.begin())
                break;
        }
    }
    else
    {
        it2=NumOfAlertsPerTestMap.end(); it2--;
        for (; it2!=NumOfAlertsPerTestMap.end(); it2--)
        {
            CTest* t=it2.value();
            if (!t)
                continue;
            QList< ShiftAlert* > values = mTestsAlertsMap.values(t);
            qSort(values.begin(), values.end(), ShiftAlert::percentGreatThan);
            for (int i=0; i < values.size(); ++i)
            {
                ShiftAlert* sa=values.at(i);
                if (!sa || !sa->mTest)
                    continue;
                lLabels.clear(); lAnchors.clear();
                QString href=QString("#_gex_drill--drill_chart=adv_trend--data=%1").arg(t->lTestNumber);
                lLabels << QString::number(t->lTestNumber);
                lAnchors << QString("name=\"ByTestT%1\" href=\"%2\"").arg(t->lTestNumber).arg(href);
                lLabels << t->strTestName;
                lAnchors << QString("name=\"ByTestT%1\" href=\"%2\"").arg(t->lTestNumber).arg(href);
                lLabels << sa->mPartInfo->getPartID();
                lLabels << t->GetCurrentLimitItem()->szLowL;
                lLabels << t->GetCurrentLimitItem()->szHighL;
                if (outputFormat=="CSV")
                {
                    lLabels << sa->mParams.value("maxdelta").toString();
                    lLabels << QString(t->szTestUnits);
                }
                else
                {
                    lLabels << sa->mParams.value("maxdelta").toString() + " " + QString(t->szTestUnits);
                }
                lLabels << ((sa->mPolarity == positive)? "+" : "-");
                lLabels << (sa->mMinGroup?sa->mMinGroup->strGroupName:"?")+QString(" vs ")+(sa->mMaxGroup?sa->mMaxGroup->strGroupName:"?");
                lLabels << QString::number((double)(sa->mParams.value("percent").toDouble()*100.),'g',3)+"%";
                mGexReport->WriteTableRow(lLabels, "style=\"text-align:center\"", lAnchors);
                // if the output format is HTML or CSV, not make line break
                if (/*outputFormat != "HTML" && */outputFormat !="CSV")
                {
                    lNumberOfLinesWritten++;
                    if (lNumberOfLinesWritten>(int)mMaxNumberOfLinesPerSlideInPres)
                    {
                        lNumberOfLinesWritten=0;
                        mGexReport->EndTable();
                        mGexReport->EndLine();
                        //mGexReport->WritePageBreak(mGexReport->getReportFile(), false);
                        mGexReport->WritePageBreak(); // actually flush pres (PPT, ODP) pages
                        mGexReport->BeginTable(lTableAtts);
                        mGexReport->WriteTableRow(lTitle, lFirstTableLineAtts);
                    }
                }
            }

            if (it2==NumOfAlertsPerTestMap.begin())
                break;
        }
    }


    mGexReport->EndTable();
    mGexReport->EndLine();
    mGexReport->WritePageBreak(); // actually flush pres (PPT, ODP) pages
    return "ok";
}

QString ShiftReportUnit::WriteParetoReport(QString lTitlesAttributes, QString lTableAtts, QString lFirstTableLineAtts)
{
    unsigned lNumberOfLinesWritten=0;
    QString outputFormat = mGexReport->GetOption("output", "format").toString();

    // % of LS pareto
    mGexReport->BeginTable(lTableAtts);
    QString lTableTitle;
    QList<QString> anchors;
    if(mCalcMethod != "measuring_value_of_baseline")
    {
        lTableTitle = "Percent of LimitSpace pareto";
        anchors = QList<QString>() << "name=\"PercentOfLimitSpaceParetoTable\"" << "" << "href=\"#Top\"";
    }
    else
    {
        lTableTitle = "Actual Value Shift pareto";
        anchors = QList<QString>() << "name=\"ActualValueShiftParetoTable\"" << "" << "href=\"#Top\"";
    }
    QList<QString> labels = QList<QString>() << lTableTitle << "  " << "(Top)";
    mGexReport->WriteTableRow(labels, lTitlesAttributes, anchors);
    mGexReport->EndTable();
    mGexReport->EndLine();

    mGexReport->BeginTable(lTableAtts);
    QList< QString > lanchors;
    QList< QString > l, title;
    title.clear();l.clear();
    if(mCalcMethod != "measuring_value_of_baseline")
    {
        title<<"PartID" << "Test #"<<"Test name"<<"LL";
        if (outputFormat=="CSV")
            title << "Units" <<"HL" << "Units" <<"Max delta"<< "Units";
        else
            title <<"HL" <<"Max delta";
        title << "Polarity" <<"Guilty groups"<<"Percent of limit space";
    }
    else
    {
        title << "Test #" << "Test name" << "PartID" << "LL";
        if (outputFormat=="CSV")
            title << "Units" <<"HL" << "Units";
        else
            title <<"HL";
        title << "Polarity" <<"Guilty groups"<<"Percent of Actual Shift";
    }
    mGexReport->WriteTableRow(title, lFirstTableLineAtts);
    QMultiMap<double, ShiftAlert* >::iterator it3=mPercentLSAlertsMap.end(); it3--;
    for (; it3!=mPercentLSAlertsMap.end(); it3--)
    {
        ShiftAlert* a=it3.value();
        if (!a || !a->mTest || !a->mPartInfo)
            continue;
        l.clear(); lanchors.clear();
        if(mCalcMethod != "measuring_value_of_baseline")
        {
            l <<a->mPartInfo->getPartID(); lanchors<<"";
            QString href=QString("#_gex_drill--drill_chart=adv_trend--data=%1").arg(a->mTest->lTestNumber);
            l <<QString::number(a->mTest->lTestNumber);
            lanchors<<QString("name=\"T%1\" href=\"%2\"").arg(a->mTest->lTestNumber).arg(href);
            l << a->mTest->strTestName;
            lanchors<<QString("name=\"T%1\" href=\"%2\"").arg(a->mTest->lTestNumber).arg(href);
            l <<a->mTest->GetCurrentLimitItem()->szLowL;
            l <<a->mTest->GetCurrentLimitItem()->szHighL;
            if (outputFormat=="CSV")
            {
                l <<a->mParams.value("maxdelta").toString();
                l << QString(a->mTest->szTestUnits);
            }
            else
            {
                l <<a->mParams.value("maxdelta").toString() + " " +  QString(a->mTest->szTestUnits);
            }
        }
        else
        {
            QString href=QString("#_gex_drill--drill_chart=adv_trend--data=%1").arg(a->mTest->lTestNumber);
            l <<QString::number(a->mTest->lTestNumber);
            lanchors<<QString("name=\"T%1\" href=\"%2\"").arg(a->mTest->lTestNumber).arg(href);
            l << a->mTest->strTestName;
            lanchors<<QString("name=\"T%1\" href=\"%2\"").arg(a->mTest->lTestNumber).arg(href);
            l <<a->mPartInfo->getPartID(); lanchors<<"";
            l <<a->mTest->GetCurrentLimitItem()->szLowL;
            l <<a->mTest->GetCurrentLimitItem()->szHighL;
        }
        l << ((a->mPolarity == positive)? "+" : "-");
        l <<(a->mMinGroup?a->mMinGroup->strGroupName:"?")+QString(" vs ")+(a->mMaxGroup?a->mMaxGroup->strGroupName:"?");
        l <<QString::number((double)(a->mParams.value("percent").toDouble()*100.),'g', 3)+"%";
        mGexReport->WriteTableRow(l, "style=\"text-align:center\"", lanchors);

        // if the output format is HTML or CSV, not make line break
        if (/*outputFormat != "HTML" && */outputFormat !="CSV")
        {
            lNumberOfLinesWritten++;
            if (lNumberOfLinesWritten>mMaxNumberOfLinesPerSlideInPres)
            {
                lNumberOfLinesWritten=0;
                mGexReport->EndTable();
                mGexReport->EndLine();
                //mGexReport->WritePageBreak(mGexReport->getReportFile(), false);
                mGexReport->WritePageBreak(); // actually flush pres (PPT, ODP) pages
                mGexReport->BeginTable(lTableAtts);
                mGexReport->WriteTableRow(title, lFirstTableLineAtts);
            }
        }

        if (it3==mPercentLSAlertsMap.begin())
            break;
    }
    mGexReport->EndTable();

    return "ok";
}

QString ShiftReportUnit::CreatePages()
{
    if (!mGexReport)
    {
        return "error : mGexReport null";
    }

    mGexReport->WriteText("Shift analysis", "h1", "style=\"color:#006699\"");

    // check the type of the groups
    if(mGexReport->getGroupsList().at(0)
       && mGexReport->getGroupsList().at(0)->pFilesList.at(0))
    {
        if (mGexReport->getGroupsList().at(0)->pFilesList.at(0)->mSampleGroup != "")
            mTypeFilesInGroup = controls;
        else
            mTypeFilesInGroup = samples;
    }
    else
    {
        return "error : error in the list of files";
    }

    if (mGexReport->getGroupsList().size() > 1)
    {
        for (int i=1; i<mGexReport->getGroupsList().size(); ++i)
        {
            if (!mGexReport->getGroupsList().at(i)
                 || !mGexReport->getGroupsList().at(i)->pFilesList.at(0))
            {
                return "error : error in the list of files";
            }
            if (mTypeFilesInGroup == samples
                    && mGexReport->getGroupsList().at(i)->pFilesList.at(0)->mSampleGroup != "")
            {
                mTypeFilesInGroup = both;
                break;
            }
            if (mTypeFilesInGroup == controls
                    && mGexReport->getGroupsList().at(i)->pFilesList.at(0)->mSampleGroup == "")
            {
                mTypeFilesInGroup = both;
                break;
            }

        }
    }
    mGexReport->EndLine();

    // if only samples or controls, the list is the same, otherwise, add only samples to calculate the shift alert
    if(mTypeFilesInGroup != both)
    {
        mSamplesGroups = mGexReport->getGroupsList();
    }
    else
    {
        for (int i=0; i<mGexReport->getGroupsList().size(); ++i)
        {
            if (mGexReport->getGroupsList().at(i)->pFilesList.at(0)->mSampleGroup == "")
            {
                mSamplesGroups.append(mGexReport->getGroupsList().at(i));
            }
        }
    }


    // Todo : disable if license not allowed
    if (!GS::LPPlugin::ProductInfo::getInstance()->isDBPluginAllowed() ||
        GS::LPPlugin::ProductInfo::getInstance()->getProductID() == GS::LPPlugin::LicenseProvider::eLtxcOEM)
    {
        QString m=ReportOptions.GetOption("messages", "upgrade").toString();
        mGexReport->WriteText(m);
        return "ok";
    }

    if (mSamplesGroups.isEmpty())
    {
        mGexReport->WriteText("Error : no group found.\n");
        return "error : no group found";
    }

    if (mSamplesGroups.size()<2)
    {
        mGexReport->WriteText( "Error : not enough groups.\n" );
        return "error : not enough group";
    }

    QString lTitlesAttributes=" style=\"font-weight:bold;text-align:center\"";
    QString lFirstTableLineAtts=" style=\"background-color:#CCECFF; font-weight:bold; text-align:center \""; // border:medium solid blue;
    // atts of tables : // empty-cells:show  ?
    QString lTableAtts="align=center style=\"align:center; background-color:#F8F8F8; border:1; border-color:#111111; border-collapse:collapse; cellspacing:2; cellpadding:0; \"";

    WriteOptionsTable(lTableAtts, lFirstTableLineAtts);
    mGexReport->EndLine();

    QString lResult = SearchForShiftAlerts();

    if (lResult.startsWith("err"))
        GSLOG(SYSLOG_SEV_ERROR, lResult.toLatin1().constData());

    if (mPartsAlertsMap.count()==0)
    {
        mGexReport->WriteText(
                    "No parts found with a too high max delta. Lower the limit space threshold option to catch some parts.\n", "div", "align:center");
        return "ok";
    }

    QVector< QVector<QVariant> > results;
    lResult = mGexReport->ExecQuery("DROP table shift_alerts", results);
    lResult = mGexReport->ExecQuery("create table shift_alerts (id integer primary key, " \
                                    "test_number integer, test_name varchar, test_unit, test_ll varchar, test_hl varchar,"\
                                    "part_id varchar, delta number, delta_percent number," \
                                    "min_group varchar, max_group varchar" \
                                    ")", results);

    if (lResult.startsWith("err"))
    {
        GSLOG(SYSLOG_SEV_ERROR, "Cannot create shift alerts table in report DB");
    }
    else
    {
        int i=0;
        foreach(ShiftAlert* sa, mPartsAlertsMap.values())
        {
            if (!sa)
                continue;
            lResult = mGexReport->ExecQuery(
                        QString("insert into shift_alerts values(%1, %2, '%3', '%4', '%5', '%6', '%7', '%8', '%9','%10', '%11')")
                        .arg(i++)
                        .arg(sa->mTest->lTestNumber).arg(sa->mTest->strTestName).arg(sa->mTest->szTestUnits)
                        .arg(sa->mTest->GetCurrentLimitItem()->szLowL).arg(sa->mTest->GetCurrentLimitItem()->szHighL)
                        .arg(sa->mPartInfo->getPartID())
                        .arg(sa->mParams.value("maxdelta").toFloat())
                        .arg(sa->mParams.value("percent").toDouble()*100.)
                        .arg(sa->mMinGroup?sa->mMinGroup->strGroupName:"?")
                        .arg(sa->mMaxGroup?sa->mMaxGroup->strGroupName:"?")
                        , results);
            if (lResult.startsWith("err"))
                GSLOG(SYSLOG_SEV_WARNING, lResult.toLatin1().data());
        }
    }

    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString("%1 alerts found").arg( mPartsAlertsMap.count()).toLatin1().constData());

    QString of=mGexReport->GetOption("output", "format").toString();
    if(of!="CSV")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
    {
        QString lParetoTableName, lParetoTableLabel;
        if(mCalcMethod != "measuring_value_of_baseline")
        {
            lParetoTableName = "href=\"#PercentOfLimitSpaceParetoTable\"";
            lParetoTableLabel = " Percent of LimitSpace pareto";
        }
        else
        {
            lParetoTableName = "href=\"#ActualValueShiftParetoTable\"";
            lParetoTableLabel = "Actual Value Shift Pareto";
        }
        mGexReport->BeginTable(lTableAtts);
        QList<QString> labels = QList<QString>()
                << "Summary by test" << "   "
                << " Details by test" << "   "
                << " Summary by device" << "   "
                << " Details by device" << "   "
                << lParetoTableLabel << "   "
                << " Groups summary";
        QList<QString> a = QList<QString>()
                << "href=\"#SummaryByTest\"" << ""
                << "href=\"#DetailsByTest\"" << ""
                << "href=\"#SummaryByDevice\"" << ""
                << "href=\"#DetailsByDevice\"" << ""
                << lParetoTableName << ""
                << "href=\"#GroupsSummary\"";
        mGexReport->WriteTableRow(labels, lTitlesAttributes + " name=\"Top\"", a);
        mGexReport->EndTable();
    }
    mGexReport->EndLine();

    lResult = WriteTestReport(lTitlesAttributes, lTableAtts, lFirstTableLineAtts);
    if (lResult.startsWith("err"))
        GSLOG(SYSLOG_SEV_WARNING, lResult.toLatin1().data());

    mGexReport->WritePageBreak(); //mGexReport->getReportFile(), false);

    lResult = WriteDeviceReport(lTitlesAttributes, lTableAtts, lFirstTableLineAtts);

    mGexReport->WritePageBreak();

    lResult = WriteParetoReport(lTitlesAttributes, lTableAtts, lFirstTableLineAtts);

    mGexReport->EndLine();

    mGexReport->WritePageBreak();


    QString extra_tables=mGexReport->GetOption("adv_shift", "extra_queries").toString();
    if (!extra_tables.isEmpty())
    {
        foreach(const QString &q, extra_tables.split(";"))
        {
            results.clear();
            if (q.isEmpty())
                continue;
            QString r=mGexReport->ExecQuery(q, results);
            if (r.startsWith("err"))
            {
                GSLOG(SYSLOG_SEV_WARNING, r.toLatin1().data());
                continue;
            }

            if (results.size()==0)
            {
                GSLOG(SYSLOG_SEV_WARNING,
                      QString("No result for extra query %1").arg( q).toLatin1().constData());
                continue;
            }

            GSLOG(SYSLOG_SEV_INFORMATIONAL,
                  QString("Extra query returned %1 results...").arg( results.size())
                  .toLatin1().constData());

            mGexReport->BeginTable(lTableAtts);
            QList<QString> QueryLabels = QList<QString>() << "Extra table for query"<<"  "<< "(Top)";
            QList<QString> anchors = QList<QString>() << "name=\"ExtraTableForQuery" << "" << "href=\"#Top\"";
            mGexReport->WriteTableRow(QueryLabels, lTitlesAttributes, anchors);
//            mGexReport->EndTable();
            mGexReport->EndLine();

//            mGexReport->WriteText("Extra table for query ");
//            mGexReport->EndLine();
            mGexReport->BeginTable(lTableAtts);

            QList<QString> labels;

            for (int i=0; i<results.size(); i++)
            {
                labels.clear();
                foreach(const QVariant &v, results.at(i))
                    labels.append(v.toString());
                mGexReport->WriteTableRow(labels, i==0?lFirstTableLineAtts:"style=\"text-align:center\"" );
            }

            mGexReport->EndTable();
        }
        mGexReport->EndLine();
    }

    mGexReport->WritePageBreak();


    lResult = WriteGroupsSummaryTable(lTitlesAttributes, lTableAtts, lFirstTableLineAtts);
    mGexReport->EndLine();
    mGexReport->WritePageBreak();
    return "ok";
}

QString ShiftReportUnit::CloseSection()
{
    QString of=mGexReport->GetOption("output", "format").toString();
    if(of=="CSV")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
    {
        // Generating .CSV report file: no specific task to perform when section written !
    }
    else
        //if(of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
        //(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
        if(mGexReport->getReportOptions()->isReportOutputHtmlBased())
        {
            if (of == "HTML")
                CloseReportFile();	// Close report file
        }
    return "ok"; //GS::StdLib::Stdf::NoError;
}

double ShiftReportUnit::ComputeMaxDelta(QMap<int, double> testResultPerGroup,
                                        CGexGroupOfFiles* &mingroup,
                                        CGexGroupOfFiles* &maxgroup,
                                        int refGroupId)
{
    double lMaxDelta=GEX_C_DOUBLE_NAN;
    //double min=std::num max<double>; //numeric_limits<double>;
    double lMin=std::numeric_limits<double>::max();
    QList<CGexGroupOfFiles*> lGroups = mGexReport->getGroupsList();
    //min=-min;
    double lMax=-1*std::numeric_limits<double>::max();

    QMap<int, double>::const_iterator lIter;
    for (lIter = testResultPerGroup.begin();
         lIter != testResultPerGroup.end(); ++lIter)
    {
        int lKey = lIter.key();
        double lValue = lIter.value();
        if (lValue < lMin)
        {
            lMin = lValue;
            if (lKey < lGroups.size())
            {
                mingroup = lGroups.at(lKey);
            }
        }
        if (lValue > lMax)
        {
            lMax = lValue;
            if (lKey < lGroups.size())
            {
                maxgroup = lGroups.at(lKey);
            }
        }
    }

    /* This condition is choosen in the options -> shift report -> Calculate Method */
    if ((mCalcMethod=="cmp_to_ref")
         || (mCalcMethod == "measuring_value_of_baseline"))
    {
        lMaxDelta=0;
        double ref_value=testResultPerGroup.value(refGroupId);
        if (fabs(ref_value-lMin)<fabs(lMax-ref_value))
        {
            lMaxDelta = fabs(lMax-ref_value);
        }
        else
        {
            lMaxDelta=fabs(ref_value-lMin);
            maxgroup=mingroup; // case 5918
        }
        mingroup=lGroups.at(refGroupId); // case 5918
    }
    else
        if (mCalcMethod=="cmp_all_to_all")
        {
            lMaxDelta=lMax-lMin;
        }
        else
            GSLOG(SYSLOG_SEV_WARNING,
                  QString("Unknown calc method %1").arg(mCalcMethod).toLatin1().constData() );

    return lMaxDelta;
}


////////////////////////////////////////////////////////////////////////////////
/// 1/ Get the max and min group from the shift alert (the max of the samples)
/// 2/ For the min group:
///     i)   Get the first file in the min group
///     ii)  Get the list of dies from the first file in the min group
///     iii) Get the corresponding test from the first and second group which match with the test in the shift alert
//////////////////////////////////////////////////////////////////////////////////
bool ShiftReportUnit::CalculateMaxControlsDelta(ShiftAlert* sampleShiftAlert,
                                                ShiftAlert *&controlShiftAlert,
                                                unsigned short &nbreShiftAlert)
{
    controlShiftAlert = NULL;
    /// 1/ get the max and min group from the shift alert (the max of the samples)
    CTest* lOriginalTest = sampleShiftAlert->mTest;

    /// 2/ get all files from the each group (min and max)

    QList<CGexGroupOfFiles*> lListGroupOfFiles = mGexReport->getGroupsList();
    CGexGroupOfFiles* lMinGroup = NULL;
    CGexGroupOfFiles* lMaxGroup = NULL;
    for (int lGroup=0; lGroup<lListGroupOfFiles.size(); ++lGroup)
    {
        if (lListGroupOfFiles[lGroup]
            && (lListGroupOfFiles[lGroup]->pFilesList.size() > 0))
        {

            if (lListGroupOfFiles[lGroup]->pFilesList.at(0)->mSampleGroup.startsWith(sampleShiftAlert->mMinGroup->strGroupName))
            {
                lMinGroup = lListGroupOfFiles[lGroup];
            }
            if(lListGroupOfFiles[lGroup]->pFilesList.at(0)->mSampleGroup.startsWith(sampleShiftAlert->mMaxGroup->strGroupName))
            {
                lMaxGroup = lListGroupOfFiles[lGroup];
            }
        }
    }
    float lLimitSpacePercent=mGexReport->GetOption("adv_shift", "limit_space_percent").toFloat();

    if ((lMinGroup == NULL) || (lMaxGroup == NULL))
    {
        GSLOG(SYSLOG_SEV_ERROR, "the group of file is null");
        return false;
    }

    /// i) Get the first file in the min group
    if (lMinGroup->pFilesList.size() <= 0)
    {
        GSLOG(SYSLOG_SEV_ERROR, "The group of file doesn't contian any file");
        return false;
    }
    CGexFileInGroup* lFileInMinGroup = lMinGroup->pFilesList.at(0);

    /// ii) Get the list of dies from the first file in the min group
    QList<CPartInfo*> lPartsInfo;
    lPartsInfo.clear();
    if (lFileInMinGroup)
    {
        lPartsInfo = lFileInMinGroup->pPartInfoList;
    }
    if (lPartsInfo.isEmpty())
    {
        // next test
        GSLOG(SYSLOG_SEV_WARNING, QString("no PartInfo found for this test %1")
                                  .arg(lOriginalTest->lTestNumber).toLatin1().constData());
        return false;
    }

    /// iii) Get the corresponding test from the first and second group which match with the test in the shift alert
    CTest* lTestMinGroup = lMinGroup->FindTestCell(lOriginalTest->lTestNumber,
                                                                  lOriginalTest->strTestName,
                                                                  lOriginalTest->lPinmapIndex);

    CTest* lTestMaxGroup = lMaxGroup->FindTestCell(lOriginalTest->lTestNumber,
                                                                    lOriginalTest->strTestName,
                                                                    lOriginalTest->lPinmapIndex);

    // if the first group or the second one doesn't contain the requested test, return false and write "-" in the report
    if (!lTestMinGroup
        || !lTestMaxGroup
        || (lTestMinGroup->m_testResult.count()==0)
        || (lTestMaxGroup->m_testResult.count()==0))
    {
        return false;
    }

    int index=-1;
//    QMap<CGexGroupOfFiles*, double> testResultPerGroup;
    CPartInfo/** lPartInfo = NULL,*/ *lMaxDeltaPartInfo = NULL;
    double lMaxDelta=-GEX_C_DOUBLE_NAN, lMinGroupValue=-GEX_C_DOUBLE_NAN, lMaxGroupValue=-GEX_C_DOUBLE_NAN;
    double lPercent=-GEX_C_DOUBLE_NAN;
    Polarity lPolarity = positive;
    double lMaxValue=qAbs(lTestMinGroup->GetCurrentLimitItem()->lfHighLimit - lTestMinGroup->GetCurrentLimitItem()->lfLowLimit);
    // Loop on all parts and find all shift
    for (int lPartInfoIndex=0; lPartInfoIndex<lPartsInfo.count(); ++lPartInfoIndex)
    {
//        lPartInfo = lPartsInfo.at(lPartInfoIndex);
//        testResultPerGroup.clear(); // clear items
        index++; // test result index in TestResults.
//        lGexGroupFile = lTestsByGroup.keys();
//        qSort(lGexGroupFile.begin(), lGexGroupFile.end(), sortGroupOfFile);
//        lGroupOfFile = NULL;

        if (lTestMinGroup->m_testResult.count()<=index
            || lTestMaxGroup->m_testResult.count()<=index)
        {
            // appear when for a given group there is no test result for this test/part (often because of stop on first fail)
            //GSLOG(SYSLOG_SEV_WARNING, QString("No more data for this group (%1) at this index (%2). Cancelling.").arg(gof->strGroupName).toLatin1().constData(), index));.arg(                     //GSLOG(SYSLOG_SEV_WARNING, "No more data for this group (%1) at this index (%2). Cancelling.").arg(gof->strGroupName).toLatin1().constData().arg( index);
            break;
        }

        if (!lTestMinGroup->m_testResult.isValidResultAt(index)
            || !lTestMaxGroup->m_testResult.isValidResultAt(index))
        {
            continue;
        }

        lMinGroupValue = lTestMinGroup->m_testResult.resultAt(index); // can't use FindValidResult() because it will jump over NAN results.
        lMaxGroupValue = lTestMaxGroup->m_testResult.resultAt(index);
        if (fabs(lMinGroupValue - lMaxGroupValue) > lMaxDelta)
        {
            lMaxDelta = fabs(lMinGroupValue - lMaxGroupValue);
            if (lMinGroupValue > lMaxGroupValue)
                lPolarity = negative;
            else
                lPolarity = positive;
            lMaxDeltaPartInfo = lPartsInfo.at(lPartInfoIndex);
            lPercent = (lMaxDelta / lMaxValue) * 100.0;
            if (lPercent > lLimitSpacePercent)
                ++nbreShiftAlert;
        }

    }

    if (lMaxDeltaPartInfo)
    {
        controlShiftAlert = new ShiftAlert(lTestMinGroup, lMaxDeltaPartInfo);
        //    controlShiftAlert->mTestResultsPerGroup=testResultPerGroup;
        controlShiftAlert->mParams.insert("maxdelta", lMaxDelta);
        controlShiftAlert->mParams.insert("percent", lPercent);
        controlShiftAlert->mMaxGroup = lMaxGroup;
        controlShiftAlert->mMinGroup = lMinGroup;
        controlShiftAlert->mPolarity = lPolarity;
    }
    return true;
}

} // namespace Gex
} // namespace GS
