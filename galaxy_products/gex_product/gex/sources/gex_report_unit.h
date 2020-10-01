#ifndef GEX_REPORT_UNIT_H
#define GEX_REPORT_UNIT_H

#include <stdio.h>
#include <QList>
#include <QString>
#include "report_options.h"

class CReportOptions;
class CTest;
class CGexGroupOfFiles;
class CGexReport;

namespace GS
{
namespace Gex
{
    /*
        This is the base virtual class for all ReportUnits :
        - adv reports (Pearson, Trend, Scatter, probplot, boxplot, histo, multichart, outlierRemoval, ...)
        - std reports (Global, Histo, WaferMap, Bin, datalog, ...)
    */

    class ReportUnit : public QObject
    {
        Q_OBJECT

        Q_DISABLE_COPY(ReportUnit)

        const QList<CTest*>				mTestsListToReport; // ?

        protected:

            CGexReport*					mGexReport;

            //FILE*               mReportFile;	// Do we have to pen an new one or to use a given one ?
            const QString				mKey;	// csl key
            long                mReportPageNumber;	// Holds current report page# created; incremented at each new page
        public:
            // constructor
            // key is the csl key : 'stats', 'wafer', 'histogram', 'adv_histogram',...
            ReportUnit(CGexReport*, const QString &key);
            //
            virtual ~ReportUnit();

            // _________________________________________________________
            // Pure virtual
            // Prepare the report unit
            // bValidSection is
            virtual QString PrepareSection(bool bValidSection)=0;

            // Check Me : is there any computations between Prepare and Create
            // Creates ALL pages for the given report unit
            virtual QString     CreatePages()=0;	//

            // closes the report section just written (.CSV & .HTML)
            // should call CloseReportFile();
            virtual QString CloseSection()=0;

            //____________________________________________________________________________________

            const QString GetKey() { return mKey; }
            // will set/open mReportFile
            QString	OpenFile(const QString& strFile = QString());
            // Close the Report file
            void CloseReportFile(FILE *hFile=NULL);

            // Prepares the report section to be written (.CSV & .HTML)
            // do we have to give the ReportOptions to work on  ?
            /**
            void WriteHeaderHTML(FILE *hHtmlReportFile, const char *szColor,
                                 const char *szBackground="#FFFFFF",
                                 QString strHeadLines="",
                                 bool bReportFile=true,
                                 bool bWriteTOC=true);
            */
            // Write the HTML section title (<h1> type) + Quantix icon button + hyperlink (if flat HTML)
            void WriteHtmlSectionTitle(FILE *hReportFile,QString strBookmarkName,QString strSectionTitle);
            // Write the HTML code to display the DRILL ZoomIn/out toolbar in the report and other optionnal links.
            void WriteHtmlToolBar(int iSizeX, bool bAllowInteractive, QString strArgument, QString strText2="", QString strImage2="", QString strLink2="", QString strText3="", QString strImage3="", QString strLink3="");
            // Writes a page break in document (flat HTML file)
            void WritePageBreak(FILE *hFile=NULL);
            // Actions to first do when preparing PPT convertion (create few HTML + XML files)
            void WritePowerPointBegin(void);
            // Build Header page to include in MS-Office report or PDF file.
            void BuildHeaderFooterText(QString &strHeader,QString &strFooter);

            //
            //CreateChartImageEx(QMap<QVariant, QVariant> options); // ToDo
            //
            //void BuildImageUniqueName(...);
        public slots:
            // Updates the process bar.
            //void UpdateProcessBar(bool bSet,int iRange,int iStep,bool bShow);
        signals:
            // emit this signal to inform about the progress of your process : example : "Reading record 12563..."
            void UpdateProcessString(const QString&);
    };

} // namespace Gex
} // namespace GS

#endif // GEX_REPORT_UNIT_H
