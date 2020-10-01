#include "ftr_correlation_report.h"
#include "gex_group_of_files.h"
#include "gex_file_in_group.h"
#include "gex_report_unit.h"
#include "gex_constants.h"
#include "gex_report.h"
#include "gex_shared.h"
#include <gqtl_log.h>
#include "cstats.h"
#include "engine.h"
#include "product_info.h"

extern double	ScalingPower(int iPower);

//extern CGexReport* gexReport;

namespace GS
{
namespace Gex
{

FTRCorrelationReport::FTRCorrelationReport(CGexReport*poReport, const QString& cslkey)
    : ReportUnit(poReport, cslkey){

}

FTRCorrelationReport::~FTRCorrelationReport(){
    clear();
}

void FTRCorrelationReport::clear(){
    m_oTestNumberList.clear();
    qDeleteAll(m_oCommonFailList);
    m_oCommonFailList.clear();
}


void FTRCorrelationReport::TestCorrelationData::getTestFaillingPart (CTest *poTest, QList <int> &oTestFaillingPart, QList <QString> &oTestVectorIdxFaillingPart){

    foreach (const QString &strVect, poTest->mVectors.keys()){
        CFunctionalTest	cFunctTest = poTest->mVectors[strVect];
        if(cFunctTest.lFails){

            for(int iPart=0;iPart<poTest->m_testResult.count();++iPart){
                if( (poTest->m_testResult.isValidIndex(iPart)) &&
                        (poTest->ldSamplesValidExecs != 0))
                {
                    if(poTest->m_testResult.isValidResultAt(iPart))
                    {
                        // test result
                        double lfData = poTest->m_testResult.resultAt(iPart);
                        // Scale result to be normalized as test limits
                        lfData *= ScalingPower(poTest->res_scal);

                        // Check if failing value
                        if(poTest->isFailingValue(lfData, poTest->m_testResult.passFailStatus(iPart))){
                            oTestFaillingPart.append(iPart);
                            oTestVectorIdxFaillingPart.append(strVect);
                        }
                    }
                }
            }
        }
    }
}

FTRCorrelationReport::TestCorrelationData *FTRCorrelationReport::TestCorrelationData::buildFaillingPart(CTest *poTest1, CTest *poTest2){
    TestCorrelationData *poData = 0;
    if(poTest1->bTestType == 'F' && poTest2->bTestType == 'F' ){

        QList <int> oTest1FaillingPart;
        QList <QString> oTest1VectorIdxFaillingPart;
        TestCorrelationData::getTestFaillingPart(poTest1, oTest1FaillingPart, oTest1VectorIdxFaillingPart);
        if(oTest1FaillingPart.isEmpty())
            return poData;

        QList <int> oTest2FaillingPart;
        QList <QString> oTest2VectorIdxFaillingPart;
        TestCorrelationData::getTestFaillingPart(poTest2, oTest2FaillingPart, oTest2VectorIdxFaillingPart);
        if(oTest2FaillingPart.isEmpty())
            return poData;
        poData = new TestCorrelationData(poTest1->lTestNumber, poTest2->lTestNumber);
        for(int iFail=0; iFail<oTest1FaillingPart.count(); ++iFail)
        {
            int lSearchIdx = oTest2FaillingPart.indexOf(oTest1FaillingPart[iFail]);
            //if(oTest2FaillingPart.contains(oTest1FaillingPart[iFail]))
            if(lSearchIdx != -1)
            {
                poData->addFaillingPart(oTest1FaillingPart[iFail], oTest1VectorIdxFaillingPart[iFail], oTest2VectorIdxFaillingPart[lSearchIdx]);
            }
        }
    }
    return poData;
}

void FTRCorrelationReport::buildFTRCorrelation (QList <TestCorrelationData *> &oCommonFailList,
                                                QList<int> &oTestNumberList, CTest *poTestList,
                                                CGexTestRange *poRange, const QList<CTest *> &oSelectedTestList)
{
    //Filter over Functional tests and user choice.
    CTest *poTest = poTestList;
    QList <CTest *> oTestList;
    if(oSelectedTestList.isEmpty()){
        while(poTest){
            if(poTest->bTestType == 'F' || (poRange && poRange->IsTestInList(poTest->lTestNumber, poTest->lPinmapIndex))){
                oTestList.append(poTest);
            }
            poTest = poTest->GetNextTest();
        }
    }else
         oTestList = oSelectedTestList;

    if(oTestList.count()<2){
        return ;
    }

     for(int iT1=0; iT1 < oTestList.count()-1; ++iT1){
         bool bInsert = false;
         for(int iT2=iT1+1; iT2< oTestList.count(); ++iT2){
             TestCorrelationData *poObj = TestCorrelationData::buildFaillingPart(oTestList[iT1], oTestList[iT2]);
             if(poObj){
                 if(poObj->count()){
                     bInsert = true;
                     oCommonFailList.append(poObj);
                     if(!oTestNumberList.contains(oTestList[iT2]->lTestNumber))
                         oTestNumberList.append(oTestList[iT2]->lTestNumber);
                 }else{
                     delete poObj;
                     poObj = 0;
                 }
             }
         }
         if(!oTestNumberList.contains(oTestList[iT1]->lTestNumber) && bInsert){
             oTestNumberList.append(oTestList[iT1]->lTestNumber);
         }
     }

     if(!oTestNumberList.isEmpty())
         qSort(oTestNumberList);
}

FTRCorrelationReport::TestCorrelationData *FTRCorrelationReport::getCorrObj(QList <TestCorrelationData *> &oCommonFailList, TestCorrelationData *poCorr){
    foreach(TestCorrelationData* poData, oCommonFailList){
        if(*poCorr==*poData)
            return poData;

    }
    return 0;
}


/////////////////////////////////////////////////////////////////////////////
// Prepares the report section to be written (.CSV & .HTML)
/////////////////////////////////////////////////////////////////////////////
QString FTRCorrelationReport::PrepareSection(bool bValidSection)
{
    QString of=mGexReport->GetOption("output", "format").toString();

    // Creates the 'FTR' page & header
    if(of=="CSV")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
    {
        // Generating .CSV report file.
        fprintf(mGexReport->getReportFile(),"\n---- Ftr correlation report ----\nNOT Supported in .CSV format. You must switch to HTML report mode!\n\n");
    }
    else if(of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")	//(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
    {
        if(mGexReport->getGroupsList().count() > 1){
            fprintf(mGexReport->getReportFile(),"\n---- Ftr correlation report ----\nNOT Supported in .CSV format. You must switch to HTML report mode!\n\n");
            return  "error : FTR correlation report not supported with multiple file";
        }
        // Generating HTML report file.

        // Open advanced.htm file
        if(OpenFile().startsWith("error"))
            return "error : cant open file";


        mGexReport->WriteHeaderHTML(mGexReport->getReportFile(),"#000080");	// Default: Text is Dark Blue
        // Title + bookmark
        mGexReport->WriteHtmlSectionTitle(mGexReport->getReportFile(),"all_advanced","More Reports: FTR correlation");

        if(bValidSection == false)
        {
            fprintf(mGexReport->getReportFile(),"<p align=\"left\">&nbsp;</p>\n");
            fprintf(mGexReport->getReportFile(),"<p align=\"left\"><font color=\"#000000\" size=\"%d\">No FTR correlation data available !<br>\n",mGexReport->iHthmNormalFontSize);
        }
    }

    if(bValidSection == true)
    {
        // Get pointer to first group & first file (we always have them exist)
        CGexGroupOfFiles *pGroup = mGexReport->getGroupsList().isEmpty() ? NULL : mGexReport->getGroupsList().first();
        if(!pGroup)
            return  "error : FTR correlation report no file found";
        CGexFileInGroup *pFile  = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
        if(!pFile)
            return  "error : FTR correlation report no file found";

        if(of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")	//(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
            fprintf(mGexReport->getReportFile(),"<table border=\"0\" width=\"750\" cellspacing=\"1\">\n");

        if(mGexReport->getReportOptions()->iFiles == 1)
        {
            mGexReport->WriteInfoLine(mGexReport->getReportFile(), "Program",pFile->getMirDatas().szJobName);
            mGexReport->WriteInfoLine(mGexReport->getReportFile(), "Product",pFile->getMirDatas().szPartType);
            mGexReport->WriteInfoLine(mGexReport->getReportFile(), "Lot",pFile->getMirDatas().szLot);
        }

        QString lMessage;
        switch(mGexReport->getReportOptions()->getAdvancedReportSettings())
        {
            case GEX_ADV_FTR_CORRELATION_ALL: // list tests
            default:
                lMessage = "list all tests";
                break;
            case GEX_ADV_FTR_CORRELATION_LIST://  specific tests
                lMessage = mGexReport->getReportOptions()->pGexAdvancedRangeList->BuildTestListString("list tests: ");
                break;
        }
        mGexReport->WriteInfoLine(mGexReport->getReportFile(),
                                  "FTR correlation for ", lMessage.toLatin1().constData());
        if(of=="CSV")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
            fprintf(mGexReport->getReportFile(),"\n");
        else
        {
            // Wrting HTML file
            if( (of=="DOC")||(of=="PDF")||(of=="PPT") ) 	//(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_FLAT_HTML)
            {
                // Flat HTML file (for Word, PDF generation): simply close the table, but do not close the file.
                fprintf(mGexReport->getReportFile(),"</table>\n");
            }
            else
            if(of=="HTML")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_HTML)
            {
                // add link to advanced1.htm
                QByteArray lFileData;
                QString lFilePath(ReportOptions.strReportDirectory + "/index.htm");
                QFile lFile(lFilePath);
                if (lFile.exists() && lFile.open(QIODevice::ReadWrite))
                {
                    lFileData = lFile.readAll();
                    QString lText(lFileData);
                    lText.replace(QString("href=\"pages/advanced.htm\">skip"), QString("href=\"pages/advanced1.htm\">skip"));
                    lFile.seek(0);
                    lFile.write(lText.toLatin1());
                    lFile.close();
                }


                // Standard multi-pages HTML file
                fprintf(mGexReport->getReportFile(),"<tr>\n");
                fprintf(mGexReport->getReportFile(),"<td bgcolor=%s>Link to pages:</td>",szFieldColor);
                fprintf(mGexReport->getReportFile(),"<td bgcolor=%s><b><a href=\"advanced1.htm\">See FTR correlation pages</a> </b></td>",szDataColor);
                fprintf(mGexReport->getReportFile(),"</tr>\n");
                fprintf(mGexReport->getReportFile(),"</table>\n");
                fprintf(mGexReport->getReportFile(),C_HTML_FOOTER, GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );	// Writes HTML footer Application name/web site string
                fprintf(mGexReport->getReportFile(),"</body>\n");
                fprintf(mGexReport->getReportFile(),"</html>\n");
                CloseReportFile(mGexReport->getReportFile()); // Close  HTML file.

                // Create first  page...
                char szString[2048];
                sprintf(szString,"%s/pages/advanced1.htm",mGexReport->getReportOptions()->strReportDirectory.toLatin1().constData());
                mGexReport->setAdvancedReportFile(fopen(szString,"wt"));
                mGexReport->setReportFile(mGexReport->getAdvancedReportFile());
                if(mGexReport->getAdvancedReportFile() == NULL)
                     return "error : cant open file";

                fprintf(mGexReport->getAdvancedReportFile(),"<html>\n");
                fprintf(mGexReport->getAdvancedReportFile(),"<head>\n");
                fprintf(mGexReport->getAdvancedReportFile(),"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=ISO-8859-1\">\n");
                fprintf(mGexReport->getAdvancedReportFile(),"</head>\n");
                // Sets default background color = white, text color given in argument.
                fprintf(mGexReport->getAdvancedReportFile(),"<body bgcolor=\"#FFFFFF\" text=\"#000000\">\n");	// Default color in HTML page!

            }

            // Keep track of total HTML pages written
            mGexReport->lReportPageNumber++;

            // Title + bookmark
            mGexReport->WriteHtmlSectionTitle(mGexReport->getAdvancedReportFile(),"","FTR correlation summary:");
            fprintf(mGexReport->getAdvancedReportFile(),"<br><br>\n<table border=\"1\" width=\"98%%\"  cellspacing=\"0\" cellpadding=\"0\">\n");
        }
    }

    // Keep count of total HTML pages to create (used to compute the ProgreeGauge size)
    mGexReport->getReportOptions()->lAdvancedHtmlPages = 1;	// incremented as we generate the HTML pages...
    // Reset line counter...used to insert page breaks in Advanced HTML pages that get too long
    mGexReport->getReportOptions()->lAdvancedHtmlLinesInPage=0;

    return "ok";
}

/////////////////////////////////////////////////////////////////////////////
// closes the report section just written (.CSV & .HTML)
/////////////////////////////////////////////////////////////////////////////
QString	FTRCorrelationReport::CloseSection()
{

    QString of=mGexReport->getReportOptions()->GetOption("output", "format").toString();
    if(of=="CSV")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
    {
        // Generating .CSV report file: no specific task to perform when section written !
    }
    else
    if(of=="HTML")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_HTML)
    {
        // Standard multi-paes HTML file
        if(mGexReport->getAdvancedReportFile() == NULL)
            return "error : cant open file";
        // In case No tests ... then fill table with only empty line (workaround QT bug)
        fprintf(mGexReport->getAdvancedReportFile(),"<tr>\n<td> </td>\n</tr>\n");

        fprintf(mGexReport->getAdvancedReportFile(),"</table>\n");

        if(mGexReport->getReportOptions()->getAdvancedReport() != GEX_ADV_FTR_CORRELATION)
            fprintf(mGexReport->getAdvancedReportFile(),"<p align=\"left\"><font color=\"#000000\" size=\"%d\">FTR correlation report is disabled!<br>\n", mGexReport->iHthmNormalFontSize);


        fprintf(mGexReport->getAdvancedReportFile(),C_HTML_FOOTER, GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );	// Writes HTML footer Application name/web site string
        fprintf(mGexReport->getAdvancedReportFile(),"</body>\n");
        fprintf(mGexReport->getAdvancedReportFile(),"</html>\n");

        CloseReportFile(mGexReport->getAdvancedReportFile());
    }
    return "ok";
}

#include <QFile>
#include <QTextStream>

void FTRCorrelationReport::exportToCSV(const QString &strFileName)
{
    QFile oFile(strFileName);
    if(!oFile.open(QIODevice::Append))
        return ;

    QTextStream oStream;
    oStream.setDevice(&oFile);

    // Generating .CSV report file.
    oStream << "******************************************************";
    oStream << "******************************************************\n";
    oStream << "* " << GS::Gex::Engine::GetInstance().Get("AppFullName").toString() <<" - www.mentor.com\n";
    oStream << "* "<<C_STDF_COPYRIGHT<<". All rights reserved.\n";
    oStream <<"******************************************************";
    oStream <<"******************************************************\n";

    oStream <<"\n---- FTR correlation summary ----\n"  ;
    QString strCsvData;
    for(int iIdx=0; iIdx<m_oTestNumberList.count(); iIdx++){
        strCsvData += QString(",%1").arg(m_oTestNumberList[iIdx]);
    }
    oStream <<strCsvData<<"\n";

    TestCorrelationData *poCorr = new TestCorrelationData(-1,-1);
    QList <TestCorrelationData * > oFinalList;
    for(int iIdx=0; iIdx<m_oTestNumberList.count(); iIdx++){

        strCsvData = QString("%1").arg(m_oTestNumberList[iIdx]);

        for(int iCell =0 ; iCell<iIdx ; iCell++){
            poCorr->setPair(m_oTestNumberList[iIdx], m_oTestNumberList[iCell]);
            TestCorrelationData *poData = FTRCorrelationReport::getCorrObj(m_oCommonFailList, poCorr);
            int iFailCount = 0;
            if(poData){
                iFailCount = poData->count();
            }
             strCsvData += QString(",%1").arg(iFailCount);
            if(iFailCount){

                oFinalList.append(poData);
            }
        }

        for(int iCell =iIdx ; iCell<m_oTestNumberList.count() ; iCell++){
             strCsvData += QString(",");
        }
        oStream <<strCsvData<<"\n";
    }
    oStream <<"\n---- FTR correlation details ----\n"  ;

    foreach(FTRCorrelationReport::TestCorrelationData *poData, oFinalList){
        oStream << poData->generateCSVSection(mGexReport)<<"\n";
    }

    oFile.close();


}

QString FTRCorrelationReport::CreatePages(/*CReportOptions* ro*/)
{
    GSLOG(SYSLOG_SEV_NOTICE,
          QString(" iAdvancedReport = GEX_ADV_FTR_CORRELATION AdvTestList = %1")
          .arg( mGexReport->getAdvancedTestsListToReport().size())
          .toLatin1().constData());

    if (GS::LPPlugin::ProductInfo::getInstance()->getProductID() == GS::LPPlugin::LicenseProvider::eLtxcOEM)
    {
        QString m = ReportOptions.GetOption("messages", "upgrade").toString();
        mGexReport->WriteText(m);
        return "error : Your licence doesn't allow this function";
    }

    QString of=mGexReport->GetOption("output", "format").toString();
    if	(of=="CSV") //(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
        return "warning : FTR correlation not available in csv format !";
    CTest *poTestList = mGexReport->getGroupsList().first()->pFilesList.first()->ptTestList;
    CGexTestRange *poRange = mGexReport->getReportOptions()->pGexAdvancedRangeList;
    QList<CTest *> oSelectedTestList;
    buildFTRCorrelation(m_oCommonFailList, m_oTestNumberList, poTestList, poRange, oSelectedTestList);
    if(m_oCommonFailList.isEmpty())
        return "error: empty test list";
    // build the final test list to be shown in the table

    QString strHtmlData;

    strHtmlData +=  "<tr>\n";
    strHtmlData += QString("<td width=\"10%%\" bgcolor=\"#006699\" align=\"center\"><b>Tests</b></td>\n");
    for(int iIdx=0; iIdx<m_oTestNumberList.count(); iIdx++){
        strHtmlData += QString("<td width=\"10%%\" bgcolor=\"#CCECFF\" align=\"center\"><b>%1</b></td>\n").arg(m_oTestNumberList[iIdx]);
    }
    strHtmlData +=  "</tr>";

    TestCorrelationData *poCorr = new TestCorrelationData(-1,-1);
    QList <TestCorrelationData * > oFinalList;
    for(int iIdx=0; iIdx<m_oTestNumberList.count(); iIdx++){
        strHtmlData +=  "<tr>";
        strHtmlData += QString("<td width=\"10%%\" bgcolor=\"#CCECFF\" align=\"center\"><b>%1</b></td>\n").arg(m_oTestNumberList[iIdx]);

        for(int iCell =0 ; iCell<iIdx ; iCell++){
            poCorr->setPair(m_oTestNumberList[iIdx], m_oTestNumberList[iCell]);
            TestCorrelationData *poData = FTRCorrelationReport::getCorrObj(m_oCommonFailList, poCorr);
            int iFailCount = 0;
            if(poData){
                iFailCount = poData->count();
            }
            if(iFailCount){
                strHtmlData += QString("<td width=\"10%%\" bgcolor=\"#F8F8F8\" align=\"center\"><b><a name=\"%1\"></a><a href=\"#%2\">%3</a></b></td>\n")
                              .arg(poData->generateBookmark()/*"BK_NAME"*/).arg(poData->generateHref()/*"Destination_Link"*/).arg(iFailCount);
                oFinalList.append(poData);
            }
            else
                strHtmlData += QString("<td width=\"10%%\" bgcolor=\"#F8F8F8\" align=\"center\"><b> %1 </b></td>\n").arg(iFailCount);
        }

        for(int iCell =iIdx ; iCell<m_oTestNumberList.count() ; iCell++){
            strHtmlData += QString("<td width=\"10%%\" bgcolor=\"#006699\" align=\"center\"><b>   </b></td>\n");
        }

        strHtmlData +=  "</tr>\n";
    }

    strHtmlData += "</table>\n";
    fprintf(mGexReport->getAdvancedReportFile(),"%s",strHtmlData.toLatin1().constData());
    strHtmlData.clear();

    WriteHtmlSectionTitle(mGexReport->getAdvancedReportFile(),"","FTR correlation details:");
    foreach(FTRCorrelationReport::TestCorrelationData *poData, oFinalList){
        fprintf(mGexReport->getAdvancedReportFile(),"%s",poData->generateHtmlSection(mGexReport).toLatin1().constData());
    }

    fprintf(mGexReport->getAdvancedReportFile(), "<p><a href=\"_gex_export_ftr_correlation.html\"><img src=\"../images/save.png\" border=\"0\" width=\"14\" height=\"14\"></a> :");
    fprintf(mGexReport->getAdvancedReportFile(), "<a href=\"_gex_export_ftr_correlation.html\">Save</a> Export report data as CSV!</p>");

    return "ok";
}


FTRCorrelationReport::TestCorrelationData::TestCorrelationData(int iT1, int iT2){
    setPair(iT1, iT2);
}

void FTRCorrelationReport::TestCorrelationData::setPair(int iT1, int iT2){
    m_oTestCouple.first = iT1;
    m_oTestCouple.second = iT2;
}

FTRCorrelationReport::TestCorrelationData::~TestCorrelationData()
{

}

void FTRCorrelationReport::TestCorrelationData::addFaillingPart(int iPart, const QString &strT1VectIdx, const QString &strT2VectIdx){
    if(!m_oFaillingPartList.contains(iPart)){
        m_oFaillingPartList.append(iPart);
        m_oVectT1IdxList.append(strT1VectIdx);
        m_oVectT2IdxList.append(strT2VectIdx);

    }
}

int FTRCorrelationReport::TestCorrelationData::count() {
    return m_oFaillingPartList.count() ;
}

bool FTRCorrelationReport::TestCorrelationData::operator==(const TestCorrelationData& poObj) const
{
    return (m_oTestCouple.first == poObj.m_oTestCouple.first && m_oTestCouple.second == poObj.m_oTestCouple.second)
         ||(m_oTestCouple.first == poObj.m_oTestCouple.second && m_oTestCouple.second == poObj.m_oTestCouple.first);

}

bool FTRCorrelationReport::TestCorrelationData::operator==(const TestCorrelationData* &poObj) const
{
    return (m_oTestCouple.first == poObj->m_oTestCouple.first && m_oTestCouple.second == poObj->m_oTestCouple.second)
        ||(m_oTestCouple.first == poObj->m_oTestCouple.second && m_oTestCouple.second == poObj->m_oTestCouple.first);
}

QString FTRCorrelationReport::TestCorrelationData::generateBookmark(){
    return QString("%1_%2_correlation_bk").arg(m_oTestCouple.first).arg(m_oTestCouple.second);
}

QString FTRCorrelationReport::TestCorrelationData::generateHref(){
    return QString("%1_%2_correlation_href").arg(m_oTestCouple.first).arg(m_oTestCouple.second);
}

QString FTRCorrelationReport::TestCorrelationData::generateToolTip(){
    return QString();
}

QString FTRCorrelationReport::TestCorrelationData::generateCSVSection(CGexReport *poGexReport){

    CTest *poT1=0;
    if(poGexReport->getGroupsList().first()->pFilesList.first()->FindTestCell(m_oTestCouple.first,GEX_FTEST,&poT1, true, false) != 1)
        return QString();
    CTest *poT2=0;
    if(poGexReport->getGroupsList().first()->pFilesList.first()->FindTestCell(m_oTestCouple.second,GEX_FTEST,&poT2, true, false) != 1)
        return QString();

    QString strCsvData;
    strCsvData =  QString("\nFTR Test correlation for , T%1 (%2) & T%3 (%4)").arg(poT1->lTestNumber).arg(poT1->strTestName).arg(poT2->lTestNumber).arg(poT2->strTestName);

    //PART
    QString strFaillingPart = "\nEmpty failling part list";
    if(!m_oFaillingPartList.isEmpty()){
        strFaillingPart = QString::number(m_oFaillingPartList[0]);
        for(int iIdx=1; iIdx < m_oFaillingPartList.count(); ++iIdx)
            strFaillingPart += QString(", %1").arg(m_oFaillingPartList[iIdx]);
    }
    strCsvData += QString("\nCommon part failed , %1").arg(strFaillingPart);

    //Vectorname
    QString strVect1 ="\nEmpty vector name";
    QString strVect2 ="\nEmpty vector name";
    if(!m_oVectT1IdxList.isEmpty()){
        strVect1 = m_oVectT1IdxList[0];
        strVect2 = m_oVectT2IdxList[0];

        for(int iIdx=1; iIdx<m_oVectT1IdxList.count(); ++iIdx){

            if(!strVect1.contains(m_oVectT1IdxList[iIdx]))
                strVect1 += QString(", %1").arg(m_oVectT1IdxList[iIdx]);

            if(!strVect2.contains(m_oVectT2IdxList[iIdx]))
                strVect2 +=  QString(", %1").arg(m_oVectT2IdxList[iIdx]);

        }
    }
    strCsvData += QString("\nVector name for T%1 , %2").arg(poT1->lTestNumber).arg(strVect1);
    strCsvData += QString("\nVector name for T%1 , %2").arg(poT2->lTestNumber).arg(strVect2);
    strCsvData += "\n";
    return strCsvData;
}

QString FTRCorrelationReport::TestCorrelationData::generateHtmlSection(CGexReport *poGexReport){

    CTest *poT1=0;
    if(poGexReport->getGroupsList().first()->pFilesList.first()->FindTestCell(m_oTestCouple.first,GEX_FTEST,&poT1, true, false) != 1)
        return QString();
    CTest *poT2=0;
    if(poGexReport->getGroupsList().first()->pFilesList.first()->FindTestCell(m_oTestCouple.second,GEX_FTEST,&poT2, true, false) != 1)
        return QString();

    QString strHTMLSection;
    strHTMLSection = "<br><br>\n";
    strHTMLSection += "<table border=\"0\" width=\"98%%\"  cellspacing=\"0\" cellpadding=\"0\">\n";

    //Tests
    strHTMLSection += "<tr>\n";
    strHTMLSection += "<td width=\"24%%\" bgcolor=\"#CCECFF\"><b> FTR Test correlation for : </b></td>\n";
    strHTMLSection += QString("<td width=\"76%%\" bgcolor=\"#F8F8F8\" align=\"left\"><a name=\"%1\"></a> <a href=\"#%2\">%3</a></td>\n")
            .arg(generateHref()).arg(generateBookmark()).arg(QString("T%1 (%2) & T%3 (%4)").arg(poT1->lTestNumber).arg(poT1->strTestName).arg(poT2->lTestNumber).arg(poT2->strTestName));
    strHTMLSection += "</tr>\n";

    //PART
    strHTMLSection += "<tr>\n";
    strHTMLSection += "<td width=\"24%%\" bgcolor=\"#CCECFF\"><b> Common part failed : </b></td>\n";
    QString strFaillingPart = "Empty failling part list";
    if(!m_oFaillingPartList.isEmpty()){
        strFaillingPart = QString::number(m_oFaillingPartList[0]);
        for(int iIdx=1; iIdx < m_oFaillingPartList.count(); ++iIdx)
            strFaillingPart += QString(", %1").arg(m_oFaillingPartList[iIdx]);
    }
    strHTMLSection += QString("<td width=\"76%%\" bgcolor=\"#F8F8F8\" align=\"left\"> %1 </td>\n").arg(strFaillingPart);
    strHTMLSection += "</tr>\n";

    //Vectorname
    QString strVect1 ="Empty vector name";
    QString strVect2 ="Empty vector name";
    if(!m_oVectT1IdxList.isEmpty()){
        strVect1 = m_oVectT1IdxList[0];
        strVect2 = m_oVectT2IdxList[0];

        for(int iIdx=1; iIdx<m_oVectT1IdxList.count(); ++iIdx){

            if(!strVect1.contains(m_oVectT1IdxList[iIdx]))
                strVect1 += QString(", %1").arg(m_oVectT1IdxList[iIdx]);

            if(!strVect2.contains(m_oVectT2IdxList[iIdx]))
                strVect2 +=  QString(", %1").arg(m_oVectT2IdxList[iIdx]);

        }
    }
    strHTMLSection += "<tr>\n";
    strHTMLSection += QString("<td width=\"24%%\" bgcolor=\"#CCECFF\"><b> Vector name for T%1: </b></td>\n").arg(poT1->lTestNumber);
    strHTMLSection += QString("<td width=\"76%%\" bgcolor=\"#F8F8F8\" align=\"left\"> %1 </td>\n").arg(strVect1);
    strHTMLSection += "</tr>\n";

    strHTMLSection += "<tr>\n";
    strHTMLSection += QString("<td width=\"24%%\" bgcolor=\"#CCECFF\"><b> Vector name for T%1: </b></td>\n").arg(poT2->lTestNumber);
    strHTMLSection += QString("<td width=\"76%%\" bgcolor=\"#F8F8F8\" align=\"left\"> %1 </td>\n").arg(strVect2);
    strHTMLSection += "</tr>\n";


    strHTMLSection += "</table>\n";
    return strHTMLSection;
}


QPair <int, int> &FTRCorrelationReport::TestCorrelationData::getTestCouple () {
    return m_oTestCouple;
}

QList <int> &FTRCorrelationReport::TestCorrelationData::getFallingPart() {
    return m_oFaillingPartList;
}

QStringList &FTRCorrelationReport::TestCorrelationData::getVect1IdxList() {
    return m_oVectT1IdxList;
}

QStringList &FTRCorrelationReport::TestCorrelationData::getVect2IdxList() {
    return m_oVectT2IdxList;
}

} // namespace Gex
} // namespace GS
