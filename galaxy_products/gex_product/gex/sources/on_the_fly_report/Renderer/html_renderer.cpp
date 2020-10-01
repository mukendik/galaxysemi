#include "html_renderer.h"
#include "section_element.h"
#include "ctest.h"
#include "gex_report.h"
#include "report_element.h"
#include "statistical_table.h"
#include "gqtl_log.h"
#include "report_options.h"
#include "engine.h"
#include "renderer_keys.h"
#include "gex_shared.h"
#include "report_options.h"
#include <stdio.h>


extern CGexReport* gexReport;
extern int RetrieveTopNWorstTests(int N, CTest* start, QList<CTest*> &output );


bool CompareComponent(Component* compo1, Component* compo2)
{
    return compo1->mIndexPosition < compo2->mIndexPosition;
}


HtmlRenderer::HtmlRenderer()
{
    mGalaxyImage = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString() +"/images/gex_application_32x32.png";
}

QString     HtmlRenderer::CreateHomePage(const QString& title, const QString& comment)
{
    QString lComment(comment);
    QString lAPplicationDir = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
    QString lImagePath =  lAPplicationDir + QString("/images/gex_logo_transparent_title_300x78.png");
    return QString("<br><br>" \
                   "<font color=\"#006699\">\n" \
                   "    <div align=\"center\"><img border=\"0\" src=\"%4\"></div>\n" \
                   "    <h6> </h6><h1 align=\"center\">Report Builder</h1>\n" \
                   "    <h6 align=\"center\">%2</h6>\n" \
                   "        <br><br>\n" \
                   "    <h1 align=\"center\">%1</h1>\n" \
                   "</font>\n" \
                   "<br><br><br><br>\n" \
                   "<table border=\"2\" cellspacing=\"40\" width=\"100%\" style=\"font-size:10pt; border-collapse:collapse\" bordercolor=\"#111111\" cellpadding=\"0\">\n" \
                   "    <tr>\n"\
                   "        <td>%3 \n" \
                   "        </td>\n"\
                   "    </tr>\n" \
                   "</table>\n" \
                   "<br clear=\"all\" style=\"page-break-before:always\" />")
                   .arg(title)
                   .arg(QCoreApplication::applicationVersion())
                   .arg(lComment.replace("\n", "<br>"))
                   .arg(lImagePath);
}

void        HtmlRenderer::CreateHTMLDoc(const Composite& root, QTemporaryFile& tempoFile)
{
    QString lHtmlDoc;

    lHtmlDoc =  "<!DOCTYPE html>\n"\
                "<html>\n"\
                "   <head>\n"\
                "       <title>" + root.GetName() + "</title> \n"   \
                "   </head>\n" \
                "   <body>\n";

    // Create home page
    lHtmlDoc += CreateHomePage(root.GetName(), root.GetComment());


    // Loop on all sections
    const QList<Component *> &lSections = root.GetElementsSorted();
    for (int i=0; i<lSections.size(); ++i)
    {
        SectionElement* lSection = static_cast<SectionElement*>(lSections[i]);
        if (!lSection)
        {
            continue;
        }

        // Add a page break between sections. Do not add this page break before the first section.
        if (i > 0)
            lHtmlDoc += BREAK_PAGE;

        NewSection(*lSection, lHtmlDoc);
    }

    // Create the appendix
    AppendixSection(lHtmlDoc);


    lHtmlDoc += "</body>\n";
    lHtmlDoc += "</html>\n";
    tempoFile.write(lHtmlDoc.toLatin1().constData());
}



QString    HtmlRenderer::CreateImageHTML(const QString& imagePath,
                                         const QString& name,
                                         const QString& ref,
                                         const QString& comment/*=""*/)
{

        return QString("<div class=\"image\">"\
                       " <div align=\"left\" ><b>%2</b></div>"
                       "  <img    height=\"300\" align=\"top\" border=\"0\"  src=\"%1\"> <a name=\"%2\"></a> <a href=\"%3\"></a> </img>"\
                       "  <div class=\"text\">"\
                       "    <p>%4</p>"\
                       "  </div>"
                       "</div> \n")
                        .arg(imagePath).arg(name).arg(ref).arg(comment);
}



QString  HtmlRenderer::CreateCellHTML(const QString& elt)
{
    return QString("<td width=\"50%\" valign=\"top\" align=\"center\">%1</td>").arg(elt);
}


QString HtmlRenderer::CreateHTMLTitle(const QString& title, int fontSize, const QString& HexaColor/*=#006699*/)
{
    return QString ("<a name=\"%1\"></a><h%4 align=\"left\"><font color=\"%2\"><a href=\"http://support.galaxysemi.com\">"\
                    "<img border=\"0\" src=\"%3\"></a>&nbsp; %1</font></h%4>\n")
                    .arg(title).arg(HexaColor).arg(mGalaxyImage).arg(fontSize);

}


QString HtmlRenderer::OpenHTMLTable(const QString& testNumber, const QString& testName)
{

    QString lTestHeader;
    if(testNumber.isEmpty() == false && testNumber.isEmpty() == false)
        lTestHeader = QString("<H3>Test %1: %2</H3>\n").arg(testNumber).arg(testName);

    return QString("%1"\
                   "<table border=\"0\" cellspacing=\"10\" width=\"100%\" style=\"font-size:10pt; border-collapse:"\
                   "collapse\" bordercolor=\"#111111\" cellpadding=\"0\">\n").arg(lTestHeader);
}

QString HtmlRenderer::CreateHTMLTables(const StatisticalTable* aTable,
                                       const CTestContainer& aTests,
                                       const QList<Group *> aGroups,
                                       bool aSplitByGroup) const
{
    if(aTable == 0)
        return "";

    int lCutPos = 9; //-- 9 comes from the number of colum for the capability table (Magic Number until the pdf renderer)
    const QList<QString>& lFields = aTable->GetFieldsList();

    if(lFields.size() <= lCutPos)
        return mHTMTableRenderer.CreateTable(aTests, aTable->GetFieldsList(), aGroups, aSplitByGroup);

    else
    {
        QString lHTMLTables;
        int lPos = 0;

        bool lDone = false;
        while(lDone == false)
        {
            QList<QString> lSubListFields;
            if(lPos + lCutPos >= lFields.size())
            {
                lSubListFields = lFields.mid(lPos);
            }
            else
            {
                lSubListFields = lFields.mid(lPos, lCutPos);
            }
            lPos += lCutPos;

            lHTMLTables += mHTMTableRenderer.CreateTable(aTests, lSubListFields, aGroups, aSplitByGroup);
            lHTMLTables += "\n\n";

            if(lPos >= lFields.size())
                lDone = true;
        }

        return lHTMLTables;
    }
}

void HtmlRenderer::AppendixSection(QString& aHtmlDoc)
{
    aHtmlDoc += BREAK_PAGE;
    // Add a title for the current section
    aHtmlDoc += CreateHTMLTitle("Appendix", 2);

    if(gexReport->getProcessingFile() == false)
        WriteGlobalPageExaminatorDB(aHtmlDoc);
    else
        WriteGlobalPageExaminator(aHtmlDoc);	// File report while running in Database mode.
}

void HtmlRenderer::WriteGlobalPageExaminatorDB(QString& aHtmlDoc)
{
    GexDatabaseQuery* lQuery;
    CGexGroupOfFiles* lGroup;
    CGexFileInGroup*  lFile;
    // Get pointer to first group & first file (we always have them exist)
    QListIterator<CGexGroupOfFiles*>	lItGroupsList(gexReport->getGroupsList());

    QString lHTMLTable = "<table border=\"0\" cellspacing=\"0\" width=\"50%\" style=\"font-size:10pt; border-collapse: collapse\"" \
                         "bordercolor=\"#111111\" cellpadding=\"0\">\n";
    lHTMLTable += "<tr>\n"
                  "<td align=\"left\" ><b>Dataset(s) Analyzed </b></td>\n"
                  "</tr>\n";
    aHtmlDoc += lHTMLTable;

    QString lDBName, lLot, lSubLot, lWaferID, lPartType, lProcessBins;
    QString lDBNameOld, lLotOld, lSubLotOld, lWaferIDOld, lPartTypeOld, lProcessBinsOld;

    // Get pointer to first group & first file (we always have them exist)
    while(lItGroupsList.hasNext())
    {
        // Get pointer to Group Query definition
        lGroup		= lItGroupsList.next();
        lQuery		= &(lGroup->cQuery);
        QListIterator<CGexFileInGroup*> itFilesList(lGroup->pFilesList);

        // Add the Query name if we have more than 2 grpups (compare DBs)
        if (gexReport->getGroupsList().size() >= 2)
        {
            aHtmlDoc += "<tr><td>=================================================</td><td>=================================================</td></tr>";
            aHtmlDoc += "<tr> <td width=\"10%\">Query name</td> <td  align=\"left\">"
                        + lQuery->strTitle + "</td></tr>\n";
            aHtmlDoc += "<tr><td>=================================================</td><td>=================================================</td></tr>";
        }

        while (itFilesList.hasNext())
        {
            lFile = itFilesList.next();
            lDBName = lQuery->strDatabaseLogicalName;
            lLot = QString(lFile->getMirDatas().szLot);
            lSubLot = QString(lFile->getMirDatas().szSubLot);
            lWaferID = QString(lFile->getWaferMapData().szWaferID);
            lPartType = QString(lFile->getMirDatas().szPartType);
            lProcessBins = gexReport->GetPartFilterReportType(lFile->iProcessBins);
            if (   lDBName      != lDBNameOld
                || lLot         != lLotOld
                || lSubLot      != lSubLotOld
                || lWaferID     != lWaferIDOld
                || lPartType    != lPartTypeOld
                || lProcessBins != lProcessBinsOld)
            {
                aHtmlDoc += "<tr> <td width=\"10%\">Database</td> <td  align=\"left\">"
                            + lDBName + "</td></tr>\n";
                aHtmlDoc += "<tr> <td width=\"10%\">LotID</td> <td  align=\"left\">"
                            + lLot + "</td></tr>\n";
                aHtmlDoc += "<tr> <td width=\"10%\">SubLotID</td> <td  align=\"left\">"
                            + lSubLot + "</td></tr>\n";
                aHtmlDoc += "<tr> <td width=\"10%\">WaferID</td> <td  align=\"left\">"
                            + lWaferID + "</td></tr>\n";
                aHtmlDoc += "<tr> <td width=\"10%\">Product</td> <td  align=\"left\">"
                            + lPartType + "</td></tr>\n";
                aHtmlDoc += "<tr> <td>Parts</td> <td>"
                            + lProcessBins + "</td></tr>";
                aHtmlDoc += "<tr><td>-------------------------------------------------</td><td>-------------------------------------------------</td></tr>";

                lDBNameOld = lQuery->strDatabaseLogicalName;
                lLotOld = QString(lFile->getMirDatas().szLot);
                lSubLotOld = QString(lFile->getMirDatas().szSubLot);
                lWaferIDOld = QString(lFile->getWaferMapData().szWaferID);
                lPartTypeOld = QString(lFile->getMirDatas().szPartType);
                lProcessBinsOld = gexReport->GetPartFilterReportType(lFile->iProcessBins);
            }
        }
    }

    aHtmlDoc += "</table>";
}

void HtmlRenderer::WriteGlobalPageExaminator(QString& aHtmlDoc)
{
    // Get pointer to first group & first file (we always have them exist)
    QString lHTMLTable = "<table border=\"0\" cellspacing=\"0\" width=\"98%\" style=\"font-size:10pt; border-collapse: collapse\"" \
                         "bordercolor=\"#111111\" cellpadding=\"0\">\n";

    QListIterator<CGexGroupOfFiles*>	lItGroupsList(gexReport->getGroupsList());
    CGexGroupOfFiles *					lGroup = NULL;
    CGexFileInGroup *					lFile  = NULL;

    // If single file analysis
    if (gexReport->getReportOptions()->iFiles == 1)
    {
        // Get pointer to first group & first file (we always have them exist)
        lGroup = gexReport->getGroupsList().isEmpty() ? NULL : gexReport->getGroupsList().first();
        lFile  = (lGroup->pFilesList.isEmpty()) ? NULL : lGroup->pFilesList.first();
        if (lFile)
        {
            lHTMLTable += "<tr>\n"
                          "<td align=\"left\" ><b>Files Analyzed </b></td>\n"
                          "<td align=\"left\" ><b>Parts</b></td>\n"
                          "</tr>\n";
            aHtmlDoc += lHTMLTable;
            aHtmlDoc += "<tr>\n";
            QFileInfo lFileInfo(lFile->strFileName);
            aHtmlDoc += "<td>" + lFileInfo.fileName() + "</td>";
            aHtmlDoc += "<td>" + lFile->BuildPartsToProcess() + "</td>";
            aHtmlDoc += "</tr>\n";
        }

    }
    else
    {
        lHTMLTable += "<tr>\n"
                      "<td align=\"left\" ><b>Files Analyzed </b></td>\n"
                      "<td align=\"left\" ><b>Group Name</b></td>\n"
                      "<td align=\"left\" ><b>Parts</b></td>\n"
                      "</tr>\n";
        aHtmlDoc += lHTMLTable;

        while(lItGroupsList.hasNext())
        {
            lGroup	= lItGroupsList.next();
            QListIterator<CGexFileInGroup*> itFilesList(lGroup->pFilesList);

            while (itFilesList.hasNext())
            {
                lFile = itFilesList.next();
                if (lFile)
                {
                    aHtmlDoc += "<tr>\n";
                    QFileInfo lFileInfo(lFile->strFileName);
                    aHtmlDoc += "<td>" + lFileInfo.fileName() + "</td>";
                    aHtmlDoc += "<td>" + lGroup->strGroupName + "</td>";
                    aHtmlDoc += "<td>" + lFile->BuildPartsToProcess() + "</td>";
                    aHtmlDoc += "</tr>\n";
                }
            }
        }
    }
    aHtmlDoc += "</table>";
}

void HtmlRenderer::NewSection(const SectionElement& section, QString& htmlDoc)
{
    // Add a title for the current section
    htmlDoc += CreateHTMLTitle(section.GetName(), 2);

    htmlDoc += "<p>" + section.GetComment() + "</p><br>\n";

    // Get the test list if exists
    T_TestListFilter lFilter = section.GetTestListFilterType();

    // if no elements are tied to the section settings, we force the type to NOFILTER
    // since the list of tests won't be applied to any element
    if(section.HasTiedChildElements() == false)
       lFilter =  T_TESTS_NOFILTER;

    CTestContainer lTestList;
    switch(lFilter)
    {
    case T_TESTS_LIST:
    {
        BuildCTestList(section.GetTestsListFilter(), lTestList);
        // Add a small description in the section
        htmlDoc += "<p>This section contains this list of tests: ";
        CTestContainerIter lIterBegin(lTestList.begin()), lIterEnd(lTestList.end());
        for (; lIterBegin != lIterEnd; ++lIterBegin)
        {
            if (lIterBegin->second == NULL)
            {
                continue;
            }
            htmlDoc += lIterBegin->second->GetTestNumber() + "  ";
        }
        htmlDoc += "</p>\n";
        break;
    }
    case T_TESTS_TOPN:
    {
        if(gexReport->getGroupsList().isEmpty())
            return;

        int lTopN = section.GetTopNValue();

        QList<CGexGroupOfFiles*>::iterator  lIterBegin(gexReport->getGroupsList().begin()),
                                            lIterEnd(gexReport->getGroupsList().end());
        htmlDoc += "<p>This section contains the top " + QString::number(lTopN) + " failing tests. The test list: ";

        for(;lIterBegin != lIterEnd; ++lIterBegin)
        {
            //--retrieve the topN for this group
            QList<CTest*> lCTests;
            RetrieveTopNWorstTests(lTopN, (*lIterBegin)->cMergedData.ptMergedTestList, lCTests);

            //-- loop over the Ctest list to
            //-- 1. add it on the groupID/CTest list
            //-- 2. create the comment in the doc
            QList<CTest*>::iterator lIterTestBegin(lCTests.begin()), lIterTestEnd(lCTests.end());
            for(;lIterTestBegin != lIterTestEnd; ++lIterTestBegin)
            {
                lTestList.push_back(qMakePair((*lIterBegin)->GetGroupId(), *lIterTestBegin ));
                htmlDoc += (*lIterTestBegin)->GetTestNumber() + "  ";
            }
        }
        htmlDoc += "</p>\n";
        break;
    }
    case T_TESTS_NOFILTER:
    {
        if (section.ContainsTable())
        {
            BuildCTestList(static_cast<StatisticalTable*>(section.GetElements()[0])->GetTestList(), lTestList);
        }
        break;
    }

    default:
        return;
    }


    // Loop on report elements and draw them
    const QList<Component*> &lElements = section.GetElementsSorted();
    int lEltListSize = lElements.size();
    if (section.ContainsTable())
    {
        htmlDoc += CreateHTMLTables(static_cast<StatisticalTable*>(section.GetElements()[0]),
                                    lTestList,
                                    section.GetGroupsListFilter(),
                                    section.IsSplitByGroup());
    }
    else if (section.HasTiedChildElements() && lTestList.size() >0)
    {
        // Find the number of column in the table
        // fi we have more than 2 elts, we have to display them 2 by 2 in lines (2 columns by line)
        int lNbColumn = (lEltListSize >= 2) ? 2:1;

        // Fill the table line by line
        CTestContainerIter lIterBegin(lTestList.begin()), lIterEnd(lTestList.end());
        for (; lIterBegin != lIterEnd; ++lIterBegin)
        {
            CTest* lCTest =  lIterBegin->second;
            if (lCTest == NULL)
            {
                continue;
            }
            htmlDoc += "\n";

            // Open the table
            htmlDoc += OpenHTMLTable(lCTest->GetTestNumber(), lCTest->GetTestName());

            htmlDoc += OPEN_TABLE_ROW;
            bool lFirstLine(true), lClosedRow(false);
            for (int lElt=0; lElt<lEltListSize; ++lElt)
            {
                if (lFirstLine == false && (lElt % 2 == 0))
                {
                    htmlDoc += OPEN_TABLE_ROW;
                    lClosedRow = false;
                }
                lFirstLine = false;

                QString lImagePath;
                // Check if the element contains a test
                Component* lComponent = lElements[lElt];
                QString lComment = lComponent->GetComment();
                if (!static_cast<ReportElement* >(lComponent)->IsTieWithSection())
                {
                    lComponent->DrawSection(lImagePath, GEX_CHARTSIZE_LARGE);
                }
                else
                {
                    lComponent->DrawSection(lImagePath, GEX_CHARTSIZE_LARGE, lCTest);
                    lComment = "";
                }

                // Add image
                htmlDoc += CreateCellHTML(CreateImageHTML(lImagePath,
                                                          lComponent->GetName(),
                                                          QString("#"+lComponent->GetName()),
                                                          lComment.replace("\n", "<br>")));

                if ((lElt % lNbColumn) != 0)
                {
                    htmlDoc += CLOSE_TABLE_ROW;
                    lClosedRow = true;
                }
            }
            if (lClosedRow == false)
            {
                htmlDoc += CLOSE_TABLE_ROW;
            }
            htmlDoc += CLOSE_TABLE;
        }
    }
    else
    {
        // Open the table
        htmlDoc += OpenHTMLTable();

        for (int lEltIndex=0; lEltIndex<lElements.size(); ++lEltIndex)
        {
            if (lEltIndex % 2 == 0)
            {
                htmlDoc += OPEN_TABLE_ROW;
            }

            QString lImagePath;
            lElements[lEltIndex]->DrawSection(lImagePath, GEX_CHARTSIZE_LARGE);
            QString lComment("");
            if (!static_cast<ReportElement* >(lElements[lEltIndex])->IsTieWithSection())
            {
                lComment = lElements[lEltIndex]->GetComment();
            }
            htmlDoc += CreateCellHTML(CreateImageHTML(lImagePath,
                                                      lElements[lEltIndex]->GetName(),
                                                      "#"+lElements[lEltIndex]->GetName(),
                                                      lComment.replace("\n", "<br>")));

            if (lEltIndex % 2 != 0)
            {
                htmlDoc += CLOSE_TABLE_ROW;
            }
        }
        htmlDoc += CLOSE_TABLE;
    }

    // GCORE-7663:
    // This line must not be abbed systematically at the end of the section. When added at the very bottom of the html and nothing else exist
    // after, it makes htmldoc to crash (only on US computer :) )
    // Instead, this page break should be added between two sections and ONLY in this situation.
//    htmlDoc += BREAK_PAGE;
}


bool HtmlRenderer::BuildCTestList(QList<Test*> tests, CTestContainer &lTestList)
{
    if (tests.size() <= 0 || gexReport == 0)
        return false;

    //-- loop over the json Test list
    QList<Test*>::iterator lIterBegin(tests.begin()), lIterEnd(tests.end());
    for(;lIterBegin != lIterEnd; ++lIterBegin)
    {
        Test* lTest = *lIterBegin;

        if (lTest)
        {
            //-- retrieve the corresponding group
            if(lTest->mGroupId >= gexReport->getGroupsList().size())
                continue;

            CGexGroupOfFiles* lGroup = gexReport->getGroupsList()[lTest->mGroupId];
            if(lGroup && lTest->mFileIndex < lGroup->GetFilesList().size())
            {
                //-- retrieve the corresponding fileInGroup
                CGexFileInGroup* lFile = lGroup->GetFilesList()[lTest->mFileIndex];
                if(lFile)
                {
                    //-- search the corresponding CTest
                    CTest* lCTest = 0;
                    lFile->FindTestCell((lTest->mNumber).toInt(),
                                        (lTest->mPinIndex).toInt(),
                                        &lCTest,
                                        true,
                                        false,
                                        lTest->mName);

                    //-- if found, add it to the list by creating a pair with the groupID
                    if (lCTest)
                        lTestList.push_back(qMakePair(lTest->mGroupId, lCTest) );
                }
            }
        }
    }

    return lTestList.size() > 0;
}

