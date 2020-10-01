#ifndef HTMLRENDERER_H
#define HTMLRENDERER_H

#include <QString>
#include <QList>
#include <QTemporaryFile>
#include "renderer_keys.h"
#include "component.h"
#include "html_table_renderer.h"

class Composite;
class SectionElement;
class CTest;
class StatisticalTable;

class HtmlRenderer
{
public:
    HtmlRenderer();
    QString CreateImageHTML(const QString &imagePath, const QString &name, const QString &ref, const QString &comment="");
    QString CreateCellHTML(const QString &elt);
    void CreateHTMLDoc(const Composite &root, QTemporaryFile &tempoFile);

private:
    bool BuildCTestList(QList<Test *> tests, CTestContainer &lTestList);
    void NewSection(const SectionElement &section, QString &htmlDoc);

    QString mGalaxyImage;
    QString CreateHTMLTitle(const QString &title, int fontSize, const QString &HexaColor="#006699");
    QString OpenHTMLTable(const QString &testNumber="", const QString &testName="");

    QString CreateHomePage(const QString &title, const QString &comment);


    /**
     * @brief CreateHTMLTables Create as many html tables as necessary in order to adjust the size of the the table
     * to be printe as a pdf Portait page
     * @return the html string build
     */
     QString CreateHTMLTables  (const StatisticalTable *aTable,
                                const CTestContainer &aTests,
                                const QList<Group *> aGroups,
                                bool aSplitByGroup) const;


     /** hanlde the renderer for a table */
     HTMLTableRenderer mHTMTableRenderer;

     /**
      * @brief AppendixSection Create the appendix section to contains some files infos
      * @param htmlDoc the html document
      */
     void AppendixSection(QString &aHtmlDoc);

     /**
      * @brief WriteGlobalPageExaminator Create the appendix section to contains when we come from file analyzes or compare
      * @param htmlDoc the html document
      */
     void WriteGlobalPageExaminator(QString &aHtmlDoc);

     /**
      * @brief WriteGlobalPageExaminatorDB Create the appendix section to contains when we come from DB analyzes or compare
      * @param htmlDoc the html document
      */
     void WriteGlobalPageExaminatorDB(QString &aHtmlDoc);
};

#endif // HTMLRENDERER_H
