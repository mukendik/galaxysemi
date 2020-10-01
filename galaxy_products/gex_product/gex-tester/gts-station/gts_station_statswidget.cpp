/****************************************************************************
** Deriven from gts_station_statswidget_base.cpp
****************************************************************************/
#include "gtl_core.h"
#include "gts_station_mainwindow.h"
#include "gts_station_statswidget.h"


// From main.cpp
extern GtsStationMainwindow *pMainWindow;		// Ptr to main station window

/////////////////////////////////////////////////////////////////////////////////////
// Constructs a GtsStationStatswidget as a child of 'parent', with the
// name 'name' and widget flags set to 'f'.
/////////////////////////////////////////////////////////////////////////////////////
GtsStationStatswidget::GtsStationStatswidget(QWidget* parent, Qt::WindowFlags fl)
    : QWidget(parent, fl)
{
    setupUi(this);

    // Set Tree widget column count and header row
    mYieldWidget->setColumnCount(7);
    QTreeWidgetItem *lHeaderItem = mYieldWidget->headerItem();
    lHeaderItem->setText(0, QApplication::translate("GtsStationStatswidget_base", "Site", 0));
    lHeaderItem->setText(1, QApplication::translate("GtsStationStatswidget_base", "Last Bin", 0));
    lHeaderItem->setText(2, QApplication::translate("GtsStationStatswidget_base", "Parts", 0));
    lHeaderItem->setText(3, QApplication::translate("GtsStationStatswidget_base", "GOOD", 0));
    lHeaderItem->setText(4, QApplication::translate("GtsStationStatswidget_base", "BAD", 0));
    lHeaderItem->setText(5, QApplication::translate("GtsStationStatswidget_base", "Yield", 0));
    lHeaderItem->setText(6, QApplication::translate("GtsStationStatswidget_base", "GTL state", 0));

    mBinningWidget->setColumnCount(4);
    lHeaderItem = mBinningWidget->headerItem();
    lHeaderItem->setText(0, QApplication::translate("GtsStationStatswidget_base", "Binning", 0));
    lHeaderItem->setText(1, QApplication::translate("GtsStationStatswidget_base", "Pass/Fail", 0));
    lHeaderItem->setText(2, QApplication::translate("GtsStationStatswidget_base", "Parts", 0));
    lHeaderItem->setText(3, QApplication::translate("GtsStationStatswidget_base", "Yield", 0));

	Reset();
}

/////////////////////////////////////////////////////////////////////////////////////
// Destroys the object and frees any allocated resources
/////////////////////////////////////////////////////////////////////////////////////
GtsStationStatswidget::~GtsStationStatswidget()
{
}

/////////////////////////////////////////////////////////////////////////////////////
// Reset widget data
/////////////////////////////////////////////////////////////////////////////////////
void GtsStationStatswidget::Reset()
{
	// Clear list views 
    mYieldWidget->clear();
    mBinningWidget->clear();

    // Enable sorting
    mYieldWidget->setSortingEnabled(true);
    mBinningWidget->setSortingEnabled(true);
}

/////////////////////////////////////////////////////////////////////////////////////
// Init widget data
/////////////////////////////////////////////////////////////////////////////////////
void GtsStationStatswidget::Init(GtsStation_SiteList & listSites)
{
    GtsStation_Binning* lBinning=NULL;
    QTreeWidgetItem*    lItem=NULL;
    int                 lIndex=0;

	// Clear list views 
    mYieldWidget->clear();
    mBinningWidget->clear();

    // Add items to Yield list view (1 line per site)
    for(lIndex=0; lIndex<listSites.size(); ++lIndex)
	{
        lItem = new QTreeWidgetItem(mYieldWidget);
        lItem->setText(0, QString::number(listSites.at(lIndex)->m_uiSiteNb));
        lItem->setText(1, "n/a");
        lItem->setText(2, "n/a");
        lItem->setText(3, "n/a");
        lItem->setText(4, "n/a");
        lItem->setText(5, "n/a");
        lItem->setText(6, "n/a");
	}

	// Add 'All' line
    lItem = new QTreeWidgetItem(mYieldWidget);
    lItem->setText(0, "All");
    lItem->setText(1, "");
    lItem->setText(2, "n/a");
    lItem->setText(3, "n/a");
    lItem->setText(4, "n/a");
    lItem->setText(5, "n/a");
    lItem->setText(6, "n/a");

	// Add items to Binning list view
    listSites.m_listSoftBinnings.Sort(GtsStation_BinningList::eSortOnBinCount, false);
    for(lIndex=0; lIndex<listSites.m_listSoftBinnings.size(); ++lIndex)
	{
        lBinning = listSites.m_listSoftBinnings.at(lIndex);
        lItem = new QTreeWidgetItem(mBinningWidget);
        lItem->setText(0, QString::number(lBinning->m_uiBinNb));
        if(lBinning->m_eBinStatus == GtsStation_Binning::eBinningPass)
            lItem->setText(1, "P");
		else
            lItem->setText(1, "F");
        lItem->setText(2, "n/a");
        lItem->setText(3, "n/a");
	}
}

int GetGTLSiteState(int lSiteNb)
{
    if (gtl_set(GTL_KEY_DESIRED_SITE_NB, QString::number(lSiteNb).toLatin1().data())!=0)
        return -1;
    char lSiteStateString[1024]="?";
    if (gtl_get(GTL_KEY_SITE_STATE, lSiteStateString)!=0)
        return -1;
    return QString(lSiteStateString).toInt();
}

/////////////////////////////////////////////////////////////////////////////////////
// Update widget data for specified site
/////////////////////////////////////////////////////////////////////////////////////
void GtsStationStatswidget::UpdateGUI(GtsStation_SiteList & listSites)
{
    if(pMainWindow->isRunningModeBench())
        return;

    GtsStation_Site*    lSite=NULL;
    GtsStation_Binning* lBinning=NULL;
    QTreeWidgetItem*    lItem;
    float				lYield;
    QString				lText;
    int                 lIndex=0;

	// Clear list views 
    mYieldWidget->clear();
    mBinningWidget->clear();

	// Add items to Yield list view (1 line per site)
    for(lIndex=0; lIndex<listSites.size(); ++lIndex)
	{
        lSite = listSites.at(lIndex);
        if(lSite->m_uiPartCount)
        {
            // Compute yield
            lYield = ((float)lSite->m_uiPassCount/(float)lSite->m_uiPartCount)*100.0F;

            lItem = new QTreeWidgetItem(mYieldWidget);

            lItem->setText(0, QString::number(lSite->m_uiSiteNb));
            lItem->setText(1, QString::number(lSite->m_nBinning_Soft));
            lItem->setText(2, QString::number(lSite->m_uiPartCount));
            lItem->setText(3, QString::number(lSite->m_uiPassCount));
            lItem->setText(4, QString::number(lSite->m_uiFailCount));
            lText.sprintf("%05.2f%%", lYield);
            lItem->setText(5, lText);

            /* Display GTL state for this site */
            // gcore-1180
            //switch(gtl_get_site_state(lSite->m_uiSiteNb))
            switch(GetGTLSiteState(lSite->m_uiSiteNb))
            {
                case GTL_SITESTATE_BASELINE:
                    lItem->setText(6, QString("GTL_SITESTATE_BASELINE"));
                    break;
                case GTL_SITESTATE_DPAT:
                    lItem->setText(6, QString("GTL_SITESTATE_DPAT"));
                    break;
                default:
                    lItem->setText(6, QString("UNKNOWN"));
                    break;
            }
        }
        else
        {
            lItem = new QTreeWidgetItem(mYieldWidget);
            lItem->setText(0, QString::number(lSite->m_uiSiteNb));
            lItem->setText(1, "n/a");
            lItem->setText(2, "n/a");
            lItem->setText(3, "n/a");
            lItem->setText(4, "n/a");
            lItem->setText(5, "n/a");
            lItem->setText(6, "n/a");
        }
	}

	// Add 'All' line
	if(listSites.PartCount())
	{
        lYield = ((float)listSites.PassCount()/(float)listSites.PartCount())*100.0F;
        lItem = new QTreeWidgetItem(mYieldWidget);
        lItem->setText(0, "All");
        lItem->setText(1, "");
        lItem->setText(2, QString::number(listSites.PartCount()));
        lItem->setText(3, QString::number(listSites.PassCount()));
        lItem->setText(4, QString::number(listSites.FailCount()));
        lText.sprintf("%05.2f%%", lYield);
        lItem->setText(5, lText);
        lItem->setText(6, "");
    }
	else
	{
        lItem = new QTreeWidgetItem(mYieldWidget);
        lItem->setText(0, "All");
        lItem->setText(1, "");
        lItem->setText(2, "n/a");
        lItem->setText(3, "n/a");
        lItem->setText(4, "n/a");
        lItem->setText(5, "n/a");
        lItem->setText(6, "n/a");
    }
	
	// Add items to Binning list view
    listSites.m_listSoftBinnings.Sort(GtsStation_BinningList::eSortOnBinCount, false);
    for(lIndex=0; lIndex<listSites.m_listSoftBinnings.size(); ++lIndex)
	{
        lBinning = listSites.m_listSoftBinnings.at(lIndex);
		if(listSites.PartCount())
		{
            lYield = ((float)lBinning->m_uiBinCount/(float)listSites.PartCount())*100.0F;

            lItem = new QTreeWidgetItem(mBinningWidget);
            lItem->setText(0, QString::number(lBinning->m_uiBinNb));
            if(lBinning->m_eBinStatus == GtsStation_Binning::eBinningPass)
                lItem->setText(1, "P");
			else
                lItem->setText(1, "F");
            lItem->setText(2, QString::number(lBinning->m_uiBinCount));
            lText.sprintf("%05.2f%%", lYield);
            lItem->setText(3, lText);
		}
		else
		{
            lItem = new QTreeWidgetItem(mBinningWidget);
            lItem->setText(0, QString::number(lBinning->m_uiBinNb));
            if(lBinning->m_eBinStatus == GtsStation_Binning::eBinningPass)
                lItem->setText(1, "P");
			else
                lItem->setText(1, "F");
            lItem->setText(2, "n/a");
            lItem->setText(3, "n/a");
		}
	}
}
