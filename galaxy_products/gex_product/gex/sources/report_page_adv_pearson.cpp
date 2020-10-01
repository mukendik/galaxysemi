#include <QLabel>
#include <gqtl_log.h>

#include "browser.h"
#include "report_build.h"
#include "report_options.h"
#include "test_list_pearson.h"
#include "gex_file_in_group.h"
#include "gex_shared.h"
#include "gexscatterchart.h" // For Linear Regression
#include "gex_group_of_files.h"
#include "gex_report.h"
#include "engine.h"
#include "product_info.h"
#include <QCoreApplication>

// report_build.cpp
extern CReportOptions	ReportOptions;		// Holds options (report_build.h)
extern QString			formatHtmlImageFilename(const QString& strImageFileName);

int CGexReport::ComputeMaxShiftBetweenXandYtest(double &d, CTest* ptCellX, CTest* ptCellY)
{
    if(ptCellX == NULL || ptCellY == NULL)
        return -1;

    long ldTotalSamples = gex_min(ptCellX->m_testResult.count(),ptCellY->m_testResult.count());
    if(ldTotalSamples <= 0)
        return 0;

    d=0.;
    long nVS=0;
    double lfValueX=0., lfValueY=0.;

    for(long lIndex=0;lIndex < ldTotalSamples; lIndex++)
    {
        if( (ptCellX->m_testResult.isValidResultAt(lIndex)) && (ptCellY->m_testResult.isValidResultAt(lIndex)) )
        {
            nVS++;
            lfValueX = ptCellX->m_testResult.scaledResultAt(lIndex, ptCellX->res_scal );
            //lfValueX = ptCellX->m_testResult.resultAt(lIndex);
            lfValueY = ptCellY->m_testResult.scaledResultAt(lIndex, ptCellY->res_scal );
            //lfValueY = ptCellY->m_testResult.resultAt(lIndex);
            if ((fabs(lfValueX-lfValueY))>d)
                d=fabs(lfValueX-lfValueY);
        }
    }
    if (nVS<1)
        return 0;
    return nVS;
}


double	CGexReport::ComputePearsonValue(CTest *ptCellX, CTest *ptCellY, int &count)
{
    // Needs two axis to be defined to apply pearson's formula.
    if(ptCellX == NULL || ptCellY == NULL)
        return 0;

    long	ldTotalSamples = 0;
    long	ldTotalValidSamples=0;
    double	lfValueX,lfValueY;
    double	lfSumX=0,lfSumY=0;
    double	lfSumX2=0,lfSumY2=0;
    double	lfSumXY=0;

    // to be sure to use a valid index
    ldTotalSamples = gex_min(ptCellX->m_testResult.count(),ptCellY->m_testResult.count());

    // Need samples to compute Pearson's correlation value!
    if(ldTotalSamples <= 0)
        return 0;

    // If test is an Examinator internal test (eg: Die location, etc): ignore it
    if((ptCellX->bTestType == '-') || (ptCellY->bTestType == '-'))
        return 0;

    // If functional test: ignore it
    if((ptCellX->bTestType == 'F') || (ptCellY->bTestType == 'F'))
        return 0;

    // If no test results available : ignore it
    if((ptCellX->m_testResult.count() == 0) || (ptCellY->m_testResult.count() == 0))
        return 0;

    // Compute Sum(X*Y)
    for(long lIndex=0;lIndex < ldTotalSamples;lIndex++)
    {
        if( (ptCellX->m_testResult.isValidResultAt(lIndex)) && (ptCellY->m_testResult.isValidResultAt(lIndex)) )
        {
            lfValueX = ptCellX->m_testResult.resultAt(lIndex);
            lfValueY = ptCellY->m_testResult.resultAt(lIndex);

            // Compute Sums
            lfSumX += lfValueX;
            lfSumY += lfValueY;
            lfSumXY+= lfValueX*lfValueY;

            // Computes Square
            lfSumX2 += lfValueX*lfValueX;
            lfSumY2 += lfValueY*lfValueY;

            // Keep total of valid X,Y test pairs
            ldTotalValidSamples++;
        }
    }
    count=ldTotalValidSamples;

    int minsamples=m_pReportOptions->GetOption("adv_pearson", "min_samples").toInt();
    if (ldTotalValidSamples<minsamples)
    {
        //GSLOG(SYSLOG_SEV_WARNING, "no valid XY pair found !");
        return GEX_C_DOUBLE_NAN;
    }

    // Compute numerator of 'r' Pearson's correlation index
    double lfNumerator = lfSumXY -(lfSumX*lfSumY/ldTotalValidSamples);

    // Compute denominator of 'r' Pearson's correlation index
    double lfDenominatorX = lfSumX2 - (lfSumX*lfSumX / ldTotalValidSamples);
    double lfDenominatorY = lfSumY2 - (lfSumY*lfSumY / ldTotalValidSamples);
    double lfDenominator = sqrt(lfDenominatorX*lfDenominatorY);

    // Check if we can compute the division...if not, we can't check correlation!
    if(lfDenominator == 0)
        return 0;

    // Pearson's correlation index
    double lfValue = fabs(lfNumerator / lfDenominator);
    if(lfValue > 1)
        lfValue = 1;

    return lfValue;;
}

int	CGexReport::PrepareSection_AdvPearson(BOOL /*bValidSection*/)
{
    QString of = ReportOptions.GetOption("output", "format").toString();
    double 	dPearsonCutOff = ReportOptions.GetOption("adv_pearson", "cutoff").toDouble();

    // Creates the 'Adv report' page & header
    if(of=="CSV")
    {
        // Generating .CSV report file.
        fprintf(hReportFile,"\n\nTests Correlation (Pearson's correlation - r^2): Test time optimization\n");
        fprintf(hReportFile,"Correlation threshold defined (r^2) : %g\n", dPearsonCutOff);
    }
    else
        if (ReportOptions.isReportOutputHtmlBased()) //(of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
    {
        // Generating HTML report file.
        // Open advanced.htm file
        if(OpenFile_Advanced() != GS::StdLib::Stdf::NoError)
            return GS::StdLib::Stdf::ReportFile;

        // Most of functions keep writing to the 'hReportFile' file.
        hReportFile = hAdvancedReport;

        WriteHeaderHTML(hReportFile,"#000000");	// Default: Text is Black

        // Title + bookmark
        WriteHtmlSectionTitle(hReportFile,"all_advanced","More Reports: Pearson's Correlation for 'Test time optimization'");

        // If PowerPoint slides, do not write the legend as we're tight with space!
        if(of!="PPT")
        {
            fprintf(hReportFile,"<p><align=\"left\">Discover tests that strongly correlate (Pearson's correlation - r^2).\n");
            fprintf(hReportFile,"A correlation value of 0.7 or higher means tests strongly correlate.<br>\n");
            fprintf(hReportFile,"<b>About Test time optimization:</b> When two tests strongly correlate, you may consider removing one of them and therefore reduce testing time.<br>\n");
        }

        fprintf(hReportFile,"<br>Correlation limit defined: <b>%.2lf</b> (Can be edited from Examinator's <a href=\"_gex_options.htm\">'Options'</a> tab)</p>\n", dPearsonCutOff);
    }
    return GS::StdLib::Stdf::NoError;
}

/////////////////////////////////////////////////////////////////////////////
// closes the report section just written (.CSV & .HTML)
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::CloseSection_AdvPearson(void)
{
    QString of=m_pReportOptions->GetOption("output", "format").toString();
    if(of=="CSV")
    {
        // Generating .CSV report file: no specific task to perform when section written !
    }
    else if (m_pReportOptions->isReportOutputHtmlBased())
    {
        // Close last Histogram page created...so we can now build the index.
        if(hReportFile == NULL)
            return GS::StdLib::Stdf::NoError;	// Just in case file was closed on memory allocation error while collecting trend data!

        // close page...if this section was created.
        if((IfHtmlSectionCreated(GEX_HTMLSECTION_ADVANCED) == true) && (of=="HTML"))
        {
            // Close index page (unless creating one flat HTML file with all sections (eg: when creating a Word document)
            fprintf(hReportFile,C_HTML_FOOTER, GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );	// Writes HTML footer Application name/web site string
            fprintf(hReportFile,"</body>\n");
            fprintf(hReportFile,"</html>\n");
            CloseReportFile();	// Close report file
        }

        // Write page break (ignored if not writing a flat HTML document)
        WritePageBreak();
    }
    return GS::StdLib::Stdf::NoError;
}

/////////////////////////////////////////////////////////////////////////////
// Writes the header of the table to fill
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::BuildPearsonHtmlTable(CGexGroupOfFiles* pGroup1, CGexGroupOfFiles* pGroup2)
{
    QString of=m_pReportOptions->GetOption("output", "format").toString();
    if(of=="CSV")
    {
        // Generating .CSV report file.
        fprintf(hReportFile,"Correlation level,Between this test..., and this test...\n");
    }
    else
        if (m_pReportOptions->isReportOutputHtmlBased()) //if(of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
    {
        // Open HTML table with correlation results
        WriteHtmlOpenTable(98,2);	// HTML code to open table, 98%, cell spacing=1 (or 2 if creating PDF file)
        fprintf(hReportFile,"<tr>\n");

            fprintf(hReportFile,"<td width=\"40%%\"  bgcolor=%s align=\"center\"><b>Scatter plot</b></td>\n",szFieldColor);

            // width=\"10%%\"
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\"><b>Correlation</b><br><b>r^2</td>\n",szFieldColor);
            if (!pGroup2)
                fprintf(hReportFile,"<td width=\"25%%\" bgcolor=%s align=\"left\"><b>Between Test...</b></td>\n",szFieldColor );
            else
                fprintf(hReportFile,"<td width=\"25%%\" bgcolor=%s align=\"left\"><b>Between %s Test...</b></td>\n",
                    szFieldColor, pGroup1->strGroupName.toLatin1().data());
            if (!pGroup2)
                fprintf(hReportFile,"<td width=\"25%%\"  bgcolor=%s align=\"left\"><b>And Test...</b></td>\n",szFieldColor);
            else
                fprintf(hReportFile,"<td width=\"25%%\"  bgcolor=%s align=\"left\"><b>And %s Test...</b></td>\n",
                    szFieldColor, pGroup2->strGroupName.toLatin1().data());

        fprintf(hReportFile,"</tr>\n");

        // If PowerPoint slide to create, set its name
        SetPowerPointSlideName("Test Time Optimization");
    }
}

QString	CGexReport::ComputePearsonsBetweenGroups(
        CGexGroupOfFiles* pGroup1, CGexGroupOfFiles* pGroup2,
        CTestListPearson &list, const float	dPearsonCutOff,
        QStringList &warns)
{
    if (!pGroup1 || !pGroup2)
        return "error : group(s) are/is NULL !";

    if (pGroup1->pFilesList.size()!=pGroup2->pFilesList.size())
        return QString("error : groups have a different number of files : %1 for %2 vs %3 for %4 !")
                .arg(pGroup1->pFilesList.size()).arg(pGroup1->strGroupName)
                .arg(pGroup2->pFilesList.size()).arg(pGroup2->strGroupName);

    // 7230 : There is no way to know which stage is for each group !!! : if (pGroup1->)

    int i=0;
    for (i=0; i<pGroup1->pFilesList.size(); i++)
    {
        CGexFileInGroup* f1=pGroup1->pFilesList.at(i);
        if (!f1)
            return QString("error : group 1 (%1) has a NULL file !").arg(pGroup1->strGroupName);
        CGexFileInGroup* f2=pGroup2->pFilesList.at(i);
        if (!f2)
            return QString("error : group 2 (%1) has a NULL file !").arg(pGroup2->strGroupName);

        if (f1->getPcrDatas().lPartCount!=f2->getPcrDatas().lPartCount)
        {
            QString w=QString("Both datasets SHOULD have the same number of tested parts for the Pearsons method to be executable : %1 vs %2.")
              .arg(f1->getPcrDatas().lPartCount)
              .arg(f2->getPcrDatas().lPartCount);
            warns.append(w);
            GSLOG(SYSLOG_SEV_WARNING, w.toLatin1().data());
        }

        if (f1->ldTotalPartsSampled!=f2->ldTotalPartsSampled)
        {
            QString m=QString("Both datasets SHOULD have the same number of total parts sampled (tests and retests) for the Pearsons method to be executable (%1 vs %2).")
                  .arg(f1->ldTotalPartsSampled).arg(f2->ldTotalPartsSampled);
            GSLOG(SYSLOG_SEV_WARNING, m.toLatin1().data());
            warns.append(m);
            //return "error :"+m;
        }

        QString w1(f1->getWaferMapData().szWaferID);
        QString w2(f2->getWaferMapData().szWaferID);
        if (w1==w2)
            continue;

        // these tests seem superfluous
        /*
        bool ok1=false;
        bool ok2=false;
        int wid1=w1.toInt(&ok1);
        int wid2=w2.toInt(&ok2);
        if ( (!ok1) || (!ok2) )
        {
            QString w = QString("for file number %1 : Wafer '%2' seems very different than " \
                                "Wafer '%3'. Illegal correlation possible.").arg(i).arg(w1).arg(w2);
            warns.append(w);

            GSLOG(SYSLOG_SEV_WARNING, w.toLatin1().data() );
        }

        if (wid1 !=  wid2)
        {
            QString lMessage = QString("WaferID of group 1 (%1) seems not the same than WaferID " \
                                       "of group 2 (%2)").arg(wid1).arg(wid2);

            GSLOG(SYSLOG_SEV_ERROR, lMessage.toLatin1().constData());

            return "error : " + lMessage;
        }
        */

        foreach(const QString &k, f1->m_mapPartID.keys() )
        {
            if (!f2->m_mapPartID.contains(k))
            {
                QString m=QString("PartID %1 can not be found in the second group." \
                  "For the Pearson's' report to be licit, both groups SHOULD have the same parts list.").arg(k);

                return "error: "+m; // error or just warning ???
            }
        }

        foreach(const QString &k, f2->m_mapPartID.keys() )
        {
            if (!f1->m_mapPartID.contains(k))
            {
                QString m=QString("PartID %1 can not be found in the first group." \
                   "For the Pearson's' report to be executable, both groups SHOULD have the same parts list.").arg(k);

                return "error: "+m; // error or just warning ???
            }
        }
    }

    int minsamples=m_pReportOptions->GetOption("adv_pearson","min_samples").toInt();
    QString test_combination=m_pReportOptions->GetOption("adv_pearson","test_combination").toString();


    GSLOG(6, QString("Compute between %1 and %2 with a min num of samples of %3")
           .arg(pGroup1->strGroupName).arg(pGroup2->strGroupName).arg(minsamples)
           .toLatin1().data()
           );
    CTest	*ptTestCellRef=NULL;			// Points to Test to compare with others
    CTest	*ptTestCellWith=NULL;		// Points to tests before the reference.
    double	lfPearson=0;				// Pearson's correlation index.(r)
    double	lfPearsonSquared=0;		// Pearson's correlation index (r-squared).
    CPearsonTest ptPearsonTest;	// Used to create a Person test entry
    int count=0;

    ptTestCellRef = pGroup1->cMergedData.ptMergedTestList;

    while(ptTestCellRef != NULL)
    {
        // If not a parametric / multiparametric (eg: functional) test, ignore!
        if(ptTestCellRef->bTestType == 'F')
            goto next_cell_ref;

        // ignore test swithout enough samples
        if(ptTestCellRef->ldSamplesValidExecs<minsamples)
            goto next_cell_ref;

        // Compare with all other cells (after it, as the correlation is symetrical!).
        ptTestCellWith = pGroup2->cMergedData.ptMergedTestList;

        GS::Gex::Engine::GetInstance().UpdateLabelStatus(QString(" Computing Pearsons between group 1 & 2 : Test %1").arg(ptTestCellRef->lTestNumber));
        QCoreApplication::instance()->QCoreApplication::processEvents();

        while(ptTestCellWith != NULL)
        {
            // If not a parametric / multiparametric (eg: functional) test, ignore!
            if(ptTestCellWith->bTestType == 'F')
                goto next_cell_with;

            // ignore test swithout enough samples
            if(ptTestCellWith->ldSamplesValidExecs<minsamples)
                goto next_cell_with;

            if (test_combination=="only_when_testnumber_match")
                if (ptTestCellRef->lTestNumber!=ptTestCellWith->lTestNumber)
                    goto next_cell_with;

            // ignore if not the same number of execs
            // case 5309 : let s try to go on even if not the same number of Execs...
            //if (ptTestCellWith->ldExecs!=ptTestCellRef->ldExecs)
            //goto next_cell_with;

            // Compute Correlation coefficient.
            if(ptTestCellRef == ptTestCellWith) // Should be impossible but anyway
                goto next_cell_with;

            count=0;
            lfPearson = ComputePearsonValue(ptTestCellRef, ptTestCellWith, count);
            if (lfPearson==GEX_C_DOUBLE_NAN)
                goto next_cell_with;

            lfPearsonSquared	= lfPearson * lfPearson;

            if(lfPearsonSquared >= dPearsonCutOff)
            {
                //ptPearsonTest = new CPearsonTest;
                ptPearsonTest.lfPearsonRSquaredScore	= lfPearsonSquared;
                ptPearsonTest.ptTestX					= ptTestCellRef;
                ptPearsonTest.ptTestY					= ptTestCellWith;
                ptPearsonTest.mData.insert("count", count);

                // d/W as defined by JDevice
                double d = 0.;
                if (ComputeMaxShiftBetweenXandYtest(d, ptTestCellRef, ptTestCellWith)>0)
                {
                    // Limits should be always normalized
                    double LimitWidth = ptTestCellRef->GetCurrentLimitItem()->lfHighLimit-ptTestCellRef->GetCurrentLimitItem()->lfLowLimit;
                    if (LimitWidth!=0.)
                    {
                        double dW=0.;
                        dW= d / LimitWidth * 100.;
                        ptPearsonTest.mData.insert("d/W", dW );
                    }
                }
                // Add entry
                list.append(ptPearsonTest);
            }

            next_cell_with:
                // Next cell to compare with our 'Reference' one. Stop when we reach the reference
                ptTestCellWith = ptTestCellWith->GetNextTest();
        }

        next_cell_ref:
        // Move to the next Test to compare to others
        ptTestCellRef = ptTestCellRef->GetNextTest();
    }

    GSLOG(6, QString("%1 useful Pearsons tests computed").arg(list.count()).toLatin1().data() );

    return "ok";
}

QString	CGexReport::ComputePearsonsInGroup(CGexGroupOfFiles* pGroup, CTestListPearson &list,
                                           const float	dPearsonCutOff)
{
    if (!pGroup)
        return "error : the first group of the list is NULL !";

    int minsamples=m_pReportOptions->GetOption("adv_pearson", "min_samples").toInt();
    GSLOG(6, QString("considering tests with at least %1 samples and correlation higher than %2")
           .arg(minsamples).arg(dPearsonCutOff).toLatin1().data() );

    CTest	*ptTestCellRef=NULL;			// Points to Test to compare with others
    CTest	*ptTestCellWith=NULL;		// Points to tests before the reference.
    double	lfPearson;				// Pearson's correlation index.(r)
    double	lfPearsonSquared;		// Pearson's correlation index (r-squared).
    CPearsonTest ptPearsonTest;	// Used to create a Person test entry
    int count=0;

    ptTestCellRef = ptTestCellWith = pGroup->cMergedData.ptMergedTestList;

    while(ptTestCellRef != NULL)
    {
        // If not a parametric / multiparametric (eg: functional) test, ignore!
        if(ptTestCellRef->bTestType == 'F')
            goto next_cell_ref;

        // ignore test swithout enough samples
        if(ptTestCellRef->ldSamplesValidExecs<minsamples)
            goto next_cell_ref;


        // Compare with all other cells (after it, as the correlation is symetrical!).
        ptTestCellWith = ptTestCellRef->GetNextTest();

        while(ptTestCellWith != NULL)
        {
            // If not a parametric / multiparametric (eg: functional) test, ignore!
            if(ptTestCellWith->bTestType == 'F')
                goto next_cell_with;

            // ignore test swithout enough samples
            if(ptTestCellWith->ldSamplesValidExecs<minsamples)
                goto next_cell_with;

            // Compute Correlation coefficient.
            if(ptTestCellRef != ptTestCellWith)
            {
                lfPearson			= ComputePearsonValue(ptTestCellRef,ptTestCellWith, count);
                if (lfPearson==GEX_C_DOUBLE_NAN)
                    goto next_cell_with;

                lfPearsonSquared	= lfPearson * lfPearson;

                if(lfPearsonSquared >= dPearsonCutOff)
                {
                    ptPearsonTest.lfPearsonRSquaredScore	= lfPearsonSquared;
                    ptPearsonTest.ptTestX					= ptTestCellRef;
                    ptPearsonTest.ptTestY					= ptTestCellWith;
                    ptPearsonTest.mData.insert("count", count);
                    // Add entry
                    list.append(ptPearsonTest);
                }
            }

next_cell_with:
            // Next cell to compare with our 'Reference' one. Stop when we reach the reference
            ptTestCellWith = ptTestCellWith->GetNextTest();
        };

next_cell_ref:
        // Move to the next Test to compare to others
        ptTestCellRef = ptTestCellRef->GetNextTest();
    };

    return "ok";
}

bool GetTestStats(QString &o, CTest &t, CGexFileInGroup *f)
{
    if (f)
     o=QString(
        "<br>Mean=%1<br>Sigma=%2<br>HighLimit=%3<br>LowLimit=%4<br>Q1=%5<br>Q2=%6<br>Q3=%7<br>Max=%8<br>Min=%9<br>Cpk=%10")
         .arg( f->FormatTestResult(&t, t.lfMean, t.res_scal, false) )
         .arg( f->FormatTestResult(&t, t.lfSigma, t.res_scal, false) )
         .arg( f->FormatTestResult(&t, t.GetCurrentLimitItem()->lfHighLimit, t.res_scal, false) )
         .arg( f->FormatTestResult(&t, t.GetCurrentLimitItem()->lfLowLimit, t.res_scal, false) )
         .arg( f->FormatTestResult(&t, t.lfSamplesQuartile1, t.res_scal, false) )
         .arg( f->FormatTestResult(&t, t.lfSamplesQuartile2, t.res_scal, false) )
         .arg( f->FormatTestResult(&t, t.lfSamplesQuartile3, t.res_scal, false) )
         .arg( f->FormatTestResult(&t, t.lfMax, t.res_scal, false) )
         .arg( f->FormatTestResult(&t, t.lfMin, t.res_scal, false) )
         .arg( t.GetCurrentLimitItem()->lfCpk )
        ;
    else
        o=QString("<br>Mean=%1<br>Sigma=%2<br>HighLimit=%3<br>LowLimit=%4<br>Q1=%5<br>Q2=%6<br>Q3=%7<br>Cpk=%8")
         .arg(t.lfMean)
         .arg(t.lfSigma)
         .arg(t.GetCurrentLimitItem()->lfHighLimit) .arg(t.GetCurrentLimitItem()->lfLowLimit)
         .arg(t.lfSamplesQuartile1).arg(t.lfSamplesQuartile2) .arg(t.lfSamplesQuartile3)
         .arg(t.GetCurrentLimitItem()->lfCpk)
        ;

    return "true";
}

/////////////////////////////////////////////////////////////////////////////
// Writes list of Tests that higly correlate (.CSV & .HTML)
/////////////////////////////////////////////////////////////////////////////
QString	CGexReport::WriteAdvPersonReport(CReportOptions* ro)
{
    if (!ro)
        return "error : CReportOptions NULL !";

    GS::Gex::Engine::GetInstance().UpdateLabelStatus(" Building Pearson report...");

    GSLOG(6, QString("Write pearson report for %1 groups sorting on %2...").arg(getGroupsList().size())
             .arg(ro->GetOption("adv_pearson", "sorting").toString()).toLatin1().data() );

    // Enable/disable some features...
    if(GS::LPPlugin::ProductInfo::getInstance()->isOEM())
    {
        QString m=ReportOptions.GetOption("messages", "upgrade").toString();
        fprintf(hReportFile, "*ERROR* %s", m.toLatin1().data() );
        return "error : Your licence doesn't allow this function";
    }

    // If samples collection disabled
    if(ro->bSpeedCollectSamples == false)
    {
        fprintf(hReportFile,
          "*ERROR* Samples collection is currently disabled! Review your selections under the 'Speed Optimization' section in the 'Options' tab.");
        return "error : Samples collection is currently disabled ! ";
    }

    // Compute test-to-test 'r' correlation
    CTest	*ptTestCellRef=0;			// Points to Test to compare with others
    CTest	*ptTestCellWith=0;		// Points to tests before the reference.
    double	lfPearsonSquared;		// Pearson's correlation index (r-squared).
    CGexChartOverlays pChartLayers;	// Used to build Scatter plot image
    CGexSingleChart	*pLayer=0;		// Used to build Scatter plot image
    // CTestListPearson is the new QList class to use
    CTestListPearson TestListPearson(ro);	// Used to hold the list of tests with a score higher than the threshold.

    CPearsonTest *ptPearsonTest=NULL;	// Used to create a Person test entry

    QString strOutputFormat	= ro->GetOption("output", "format").toString();
    float	dPearsonCutOff	= ro->GetOption("adv_pearson", "cutoff").toDouble();


    // Get pointer to first group & first file (we always have them exist)
    CGexGroupOfFiles* pGroup1 = getGroupsList().isEmpty()?NULL:getGroupsList().first();
    if (!pGroup1)
        return "error : the first group of the list is NULL !";
    CGexGroupOfFiles* pGroup2=getGroupsList().size()>1?getGroupsList().at(1):NULL;

    // Creates the table header to fill
    BuildPearsonHtmlTable(pGroup1, pGroup2);

    QString r;
    QStringList warns;

    if ( getGroupsList().size()<2 || getGroupsList().at(1)==NULL )
    {
        r=ComputePearsonsInGroup(pGroup1, TestListPearson, dPearsonCutOff);
    }
    else
    {
        GS::Gex::Engine::GetInstance().UpdateLabelStatus(" Computing Pearsons between group 1 & 2...");
        QCoreApplication::instance()->QCoreApplication::processEvents();
        r=ComputePearsonsBetweenGroups(pGroup1, pGroup2, TestListPearson, dPearsonCutOff, warns);
    }

    foreach(const QString &s, warns)
      WriteText(s);  //fprintf(hReportFile,"- %s\n", s.toLatin1().data());

    if (!r.startsWith("ok"))
    {
        WriteText( "Error while computing correlation : "+r, "div", "style=\"color:#FF0000\"");
        //fprintf(hReportFile," Error while computing correlation : %s\n", r.toLatin1().data());
        return QString("error while computing correlation : %1").arg(r);
    }

    if (TestListPearson.size()==0)
    {
        WriteText( "No pearsons correlation found higher than desired threshold.", "div", "style=\"color:#0000FF\"");
        //fprintf(hReportFile,"No pearsons correlation found higher than desired threshold.\n");
        return "error : no pearsons correlation found higher than threshold !";
    }

    // Sort the list based on the sorting criteria
    TestListPearson.Sort(); // qt4

    // Read list in Descending order
    QList<CPearsonTest>::iterator it=TestListPearson.begin();
    ptPearsonTest = &(*it);

    QString strStatsDetail              = ro->GetOption("adv_pearson", "test_stats").toString();
    QString strOptionsCorrelationChart  = ro->GetOption("adv_correlation", "chart_type").toString();

    CGexFileInGroup* pFile1=pGroup1->pFilesList.isEmpty()?0:pGroup1->pFilesList.at(0);
    CGexFileInGroup* pFile2=pGroup2?(pGroup2->pFilesList.isEmpty()?pFile1:pGroup2->pFilesList.at(0)):pFile1;

    int	iChartNumber=0;
    while(ptPearsonTest != NULL)
    {
        // Chart ID
        iChartNumber++;

        GS::Gex::Engine::GetInstance().UpdateLabelStatus(QString("Generating Pearsons Report (chart %1)...").arg(iChartNumber));
        QCoreApplication::instance()->QCoreApplication::processEvents();

        ptTestCellRef		= ptPearsonTest->ptTestX;
        ptTestCellWith		= ptPearsonTest->ptTestY;
        lfPearsonSquared	= ptPearsonTest->lfPearsonRSquaredScore;

        if(strOutputFormat=="CSV")
        {
            // Generating .CSV report file.
            fprintf(hReportFile,"%.2lf,",lfPearsonSquared);
            fprintf(hReportFile,"%s: %s,",ptTestCellRef->szTestLabel,ptTestCellRef->strTestName.toLatin1().constData());
            fprintf(hReportFile,"%s: %s\n",ptTestCellWith->szTestLabel,buildDisplayName(ptTestCellWith).toLatin1().constData());
        }
        else if (ro->isReportOutputHtmlBased())
                //(strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
        {
            // Write Correlation info
            fprintf(hReportFile,"<tr>\n");

            // Move the chart to the left in order to looks like  JDevice old reports and to compute linear regression
            // Build Image full path where to save the chart plot.
            QString strImage = "adv" + QString::number(iChartNumber) + ".png";	// image to create: 'advXXXX.png'
            QString strImagePath = ro->strReportDirectory;
            strImagePath += "/images/";
            strImagePath += strImage;
            // Create the layer to plot
            pLayer = new CGexSingleChart;
            pLayer->bBellCurve = pLayer->bBoxBars = pLayer->bBox3DBars = pLayer->bFittingCurve = pLayer->bLines = false;
            pLayer->bSpots = true;
            pLayer->bVisible = true;
            if ( getGroupsList().size()<2 || getGroupsList().at(1)==NULL )
                pLayer->iGroupX = pLayer->iGroupY = 0;
            else
            {
                pLayer->iGroupX = 0;
                pLayer->iGroupY = 1;
            }
            pLayer->setLimitsColor(Qt::red); // For JDevice/DanKing
            pLayer->set12SigmaLineWidth(0);
            pLayer->set2SigmaLineWidth(0);
            pLayer->set3SigmaLineWidth(0);
            pLayer->set6SigmaLineWidth(0);
            pLayer->setLimitsLineWidth(1); // For JDevice/DanKing
            pLayer->setMaxLineWidth(0);
            pLayer->setMeanLineWidth(0);
            pLayer->setMedianLineWidth(0);
            pLayer->setMinLineWidth(0);

            pLayer->iTestNumberX	= ptTestCellRef->lTestNumber;
            pLayer->iPinMapX		= ptTestCellRef->lPinmapIndex;
            pLayer->strTestLabelX.sprintf("T%d : ",
                                          ptTestCellRef->lTestNumber);
            pLayer->strTestLabelX += ptTestCellRef->strTestName;
            pLayer->strTestNameX    = ptTestCellRef->strTestName;

            pLayer->iTestNumberY	= ptTestCellWith->lTestNumber;
            pLayer->iPinMapY		= ptTestCellWith->lPinmapIndex;
            pLayer->strTestLabelY.sprintf("T%d : ",
                                          ptTestCellWith->lTestNumber);
            pLayer->strTestLabelY += ptTestCellWith->strTestName;
            pLayer->strTestNameY    = ptTestCellWith->strTestName;

            int iChartType = (strOptionsCorrelationChart   == "boxwhisker") ? GexAbstractChart::chartTypeBoxWhisker : GexAbstractChart::chartTypeScatter;
            pChartLayers.getViewportRectangle()[iChartType].lfLowX		= ptTestCellRef->lfSamplesMin;
            pChartLayers.getViewportRectangle()[iChartType].lfHighX	= ptTestCellRef->lfSamplesMax;
            pChartLayers.getViewportRectangle()[iChartType].lfLowY		= ptTestCellWith->lfSamplesMin;
            pChartLayers.getViewportRectangle()[iChartType].lfHighY	= ptTestCellWith->lfSamplesMax;
            pChartLayers.clear();
            pChartLayers.addChart(pLayer);
            pChartLayers.mBackgroundColor = ReportOptions.cBkgColor;
            //pChartLayers.cChartOptions. // ToDo : change background color to white
            // 6237 : plot over option as given by iAdvancedReportSettings       // Plot Scatter over test results.
            //ro->iAdvancedReportSettings = GEX_ADV_CORR_OVERDATA;

            // Create Chart (paint into Widget)
            if (strOptionsCorrelationChart   == "boxwhisker")
                CreateAdvCorrelationBoxWhiskerChartImageEx(&pChartLayers, ptTestCellRef, ptTestCellWith,
                                                           GEX_CHARTSIZE_MEDIUM, strImagePath);
            else
                CreateAdvScatterChartImageEx(&pChartLayers, ptTestCellRef, ptTestCellWith,
                                             GEX_CHARTSIZE_MEDIUM, strImagePath);

            // Open cell that hold image + drill hyperlink
            fprintf(hReportFile, "<td width=\"40%%\" bgcolor=%s align=\"center\">", szDataColor);

            // Have the ToolBar line written in the HTML file (unless it's flat HTML, meaning final format is PDF, Word, PPT,...)
            if(strOutputFormat=="HTML")
            {
                QString strDrillArgument= "drill_chart=";
                if (strOptionsCorrelationChart == "boxwhisker")
                    strDrillArgument += "adv_boxwhisker";
                else
                    strDrillArgument += "adv_scatter";
                strDrillArgument += "--data=";
                strDrillArgument += ptTestCellRef->szTestLabel;	// X parameter
                strDrillArgument += "--data=";
                strDrillArgument += ptTestCellWith->szTestLabel;	// Y parameter
                //fprintf(hReportFile,"<a href=\"#_gex_drill--%s\"><img src=\"../images/zoom_in.png\" border=\"0\">Interactive Drill<br>",strDrillArgument.toLatin1().constData());
                // in order to get more charts on 1 page for JDevice, let s remove the zoom logo
                fprintf(hReportFile,"<a href=\"#_gex_drill--%s\">Interactive Drill<br>",strDrillArgument.toLatin1().constData());
            }

            // Write image
            fprintf(hReportFile,"<img border=\"0\" src=\"../images/%s\">\n",
                    formatHtmlImageFilename(strImage).toLatin1().constData());

            // If Toolbar included in HTML page, close the hyperlink definition
            if(strOutputFormat=="HTML")
                fprintf(hReportFile,"</a>\n");
            // Close chart table data
            fprintf(hReportFile,"</td>\n");

            static char buf[1024]="";
            sprintf(buf, "<td width=\"10%%\" bgcolor=%s align=\"center\">", szDataColor); // szFieldColor,
            float a=0.f, b=0.f;
            if (GexScatterChart::getLinearRegressionResults(ptTestCellRef, ptTestCellWith, a, b))
                sprintf(buf, "%s<b>%.2lf</b><br>from %d results<br>Y=%f X + %f <br>\n", buf,
                   lfPearsonSquared, ptPearsonTest->mData.value("count").toInt(), b, a );
            else
                sprintf(buf, "%s <b>%.2lf</b><br>\n", buf, lfPearsonSquared );

            if (ptPearsonTest->mData.find("d/W")!=ptPearsonTest->mData.end())
                sprintf(buf,"%s d/W=%f %%", buf, ptPearsonTest->mData.value("d/W").toFloat());

            sprintf(buf,"%s</td>\n", buf);

            fprintf(hReportFile, buf, NULL);

            QString CellRefStats;
            if (strStatsDetail=="full")
            {
                GetTestStats(CellRefStats, *ptTestCellRef, pFile1);
            }

            fprintf(hReportFile,"<td width=\"25%%\" bgcolor=%s align=\"left\">%s: %s %s</td>\n",
                    szDataColor,
                    ptTestCellRef->szTestLabel,
                    buildDisplayName(ptTestCellRef).toLatin1().constData(),
                    CellRefStats.toLatin1().data()
                    );

            QString CellWithStats;
            if (strStatsDetail=="full")
            {
                GetTestStats(CellWithStats, *ptTestCellWith, pFile2);
            }

            fprintf(hReportFile,"<td width=\"25%%\" bgcolor=%s align=\"left\">%s: %s %s</td>\n",
                    szDataColor,
                    ptTestCellWith->szTestLabel,
                    buildDisplayName(ptTestCellWith).toLatin1().constData(),
                    CellWithStats.toLatin1().data()
                    );

            fprintf(hReportFile,"</tr>\n");
        }// Line type: HTML

        // If PowerPoint slides created: insert page break every X
        if(	(strOutputFormat=="PPT") && ((iChartNumber % 5) == 0))
        {
            // Close table
            fprintf(hReportFile,"</table>\n");
            // Write this page as a slide (image)
            WritePageBreak();
            // Reopen the table for the next N report lines to show in next PPT slide.
            BuildPearsonHtmlTable(pGroup1, pGroup2);
        }

        // Get next Test in list
        //ptPearsonTest = cTestList.next();
        //
        it++;
        if (it==TestListPearson.end())
         ptPearsonTest = NULL;
        else
            ptPearsonTest = &(*it) ;
    };

    if (ro->isReportOutputHtmlBased())
        //(strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
        fprintf(hReportFile,"</table>\n");

    return QString("ok");
}
