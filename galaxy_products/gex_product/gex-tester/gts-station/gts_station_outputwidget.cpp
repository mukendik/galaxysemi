/****************************************************************************
** Deriven from gts_station_outputwidget_base.cpp
****************************************************************************/
#include "gts_station_mainwindow.h"
#include "gts_station_outputwidget.h"
#include "gts_station_setupwidget.h"


// From main.cpp
extern GtsStationMainwindow *pMainWindow;		// Ptr to main station window

/////////////////////////////////////////////////////////////////////////////////////
// Constructs a GtsStationOutputwidget as a child of 'parent', with the
// name 'name' and widget flags set to 'f'.
/////////////////////////////////////////////////////////////////////////////////////
GtsStationOutputwidget::GtsStationOutputwidget(GtsStationSetupwidget *pageSetup, QWidget* parent, Qt::WindowFlags fl)
    : QWidget(parent, fl)
{
    // Setup UI
    setupUi(this);
    // No rich text, only plain text
    editOutput->setAcceptRichText(false);

	m_pageSetup = pageSetup;
	
    // CHECKME: does not exist anymore in QT4
    //editOutput->setTextFormat(Qt::LogText);
    //editOutput->setMaxLogLines(50);
}

/////////////////////////////////////////////////////////////////////////////////////
// Destroys the object and frees any allocated resources
/////////////////////////////////////////////////////////////////////////////////////
GtsStationOutputwidget::~GtsStationOutputwidget()
{
}

/////////////////////////////////////////////////////////////////////////////////////
// Reset widget data
/////////////////////////////////////////////////////////////////////////////////////
void GtsStationOutputwidget::Reset()
{
	editOutput->clear();
}

/////////////////////////////////////////////////////////////////////////////////////
// Add a command to the output edit widget (use prompt)
/////////////////////////////////////////////////////////////////////////////////////
void GtsStationOutputwidget::Command(const QString & lCommand)
{
    if(pMainWindow->isHidden())
        return;

    if(m_pageSetup->IsOutputEnabled())
	{
        // CHECKME: does not exist anymore in QT4
        //editOutput->setMaxLogLines(m_pageSetup->GexMaxLines());
        Append(QString("\nprompt> %1").arg(lCommand));
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// Add a message to the output edit widget
/////////////////////////////////////////////////////////////////////////////////////
void GtsStationOutputwidget::Printf(const QString & lMessage)
{
    if(pMainWindow->isHidden())
        return;

    if(pMainWindow->isRunningModeBench())
        return;

    if(m_pageSetup->IsOutputEnabled())
	{
        // CHECKME: does not exist anymore in QT4
        //editOutput->setMaxLogLines(m_pageSetup->GexMaxLines());
        Append(lMessage);
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// Add a message to the output edit widget
/////////////////////////////////////////////////////////////////////////////////////
void GtsStationOutputwidget::Append(const QString & lMessage)
{
    if(pMainWindow->isHidden())
        return;

    if(pMainWindow->isRunningModeBench())
        return;

#if 0
    QString strText = editOutput->toPlainText();
	if(strText.endsWith("\n"))
		strText.truncate(strText.length()-1);
    strText += lMessage;
	editOutput->setText(strText);
#else
    QString lText = lMessage;
    while(!lText.isEmpty() && lText.endsWith("\n"))
        lText.truncate(lText.length()-1);
    editOutput->append(lText);
#endif

    editOutput->textCursor().movePosition(QTextCursor::End);
}

/////////////////////////////////////////////////////////////////////////////////////
// Update widget data for specified sites
/////////////////////////////////////////////////////////////////////////////////////
void GtsStationOutputwidget::UpdateGUI(GtsStation_SiteList & listSites)
{
    if(pMainWindow->isHidden())
        return;

    if(pMainWindow->isRunningModeBench())
        return;

    GtsStation_Site*    lSite=NULL;
	QString				strString;

	// Display column headers
    strString = "Site    Device #    Bin #\n";
    Printf(strString);
	
	// Display binnings... per site
    for(int lIndex=0; lIndex<listSites.size(); ++lIndex)
	{
        lSite = listSites.at(lIndex);
        if(lSite->m_nBinning_Hard != -1)
        {
            strString.sprintf(" %03d        %04d    %05d\n", lSite->m_uiSiteNb, lSite->m_uiPartCount, lSite->m_nBinning_Soft);
            Printf(strString);
        }
	}
}
