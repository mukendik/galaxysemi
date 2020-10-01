#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif

#include <stdio.h>
#include <QDialog>

#if defined unix || __MACH__
#include <unistd.h>
#include <stdlib.h>
#endif

#include "browser_dialog.h"
#include "pickbin_dialog.h"
#include "bin_info.h"
#include "stdf.h"

// Galaxy QT libraries
#include "gqtl_sysutils.h"

// main.cpp
extern GexMainwindow *	pGexMainWindow;

///////////////////////////////////////////////////////////
// Constructor
PickBinDialog::PickBinDialog( QWidget* parent, bool modal, Qt::WindowFlags f )
    : QDialog( parent, f )
{
    setupUi(this);
    setModal(modal);

    // Apply Examinator palette
    GexMainwindow::applyPalette(this);

    QObject::connect(PushButtonOk,		SIGNAL(clicked()),									this, SLOT(accept()));
    QObject::connect(PushButtonCancel,	SIGNAL(clicked()),									this, SLOT(reject()));
    QObject::connect(treeWidget,		SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),	this, SLOT(accept()));

    // Empty Filter list
    treeWidget->clear();

    // Multiple selection mode allowed, Fill the list with sorting enabled
    treeWidget->header()->setSortIndicatorShown(false);
    treeWidget->header()->setSortIndicator(2, Qt::AscendingOrder);

    // Set minimum size for the Value column
    treeWidget->setColumnWidth(0, 50);
}

///////////////////////////////////////////////////////////
// Show a list of Bins (SoftBin) found in the file
///////////////////////////////////////////////////////////
QString PickBinDialog::getBinsList(void)
{
    QString								strBinsList		= "";
    int									iSelectionCount	= 0;
    QList<QTreeWidgetItem*>				selectedItems	= treeWidget->selectedItems();
    QList<QTreeWidgetItem*>::iterator	itBegin			= selectedItems.begin();
    QList<QTreeWidgetItem*>::iterator	itEnd			= selectedItems.end();

    while(itBegin != itEnd)
    {
        if(iSelectionCount)
            strBinsList += ",";	// Separator between multiple selections

        if(bGetSofBin)
            strBinsList += (*itBegin)->text(0);	// Extracting Soft bins
        else
            strBinsList += (*itBegin)->text(2);	// Extracting Hard bins

        // Update counter index
        iSelectionCount++;

        itBegin++;
    }

    return strBinsList;
}

///////////////////////////////////////////////////////////
// Read data file and enumerate list of bins found.
///////////////////////////////////////////////////////////
bool PickBinDialog::setFile(QString strFile,bool bSoftBin)
{
    GS::StdLib::Stdf				StdfFile;
    GS::StdLib::StdfRecordReadInfo	StdfRecordHeader;
    int					iStatus;

    // Flag telling if extracting Soft bins or Hard bins.
    bGetSofBin = bSoftBin;

    QString	strDataFile = strFile;
    // If selected file is a CSV temporary edited file, then read the associated STDF file!
    if(strDataFile.endsWith(GEX_TEMPORARY_CSV) == true)
    {
        // remove the custom extension from the temporary csv file...to rebuild the STDF original name!
        strDataFile = strDataFile.remove(GEX_TEMPORARY_CSV);
    }

    // Open STDF file to read until MIR record found...have a 50K cache buffer used (instead of 2M !)
    iStatus = StdfFile.Open(strDataFile.toLatin1().constData(),STDF_READ,1000000L);
    if(iStatus != GS::StdLib::Stdf::NoError)
        return false;

    // Read all file...until MRR is found.
    BYTE							bData;
    int								wData,wSoftBin,wHardBin;
    long							dwData;
    QMap<int, int>					mBinMap;
    QMap<int, BinInfo>				mSBIN, mHBIN;
    QMap<int, BinInfo>::Iterator	itBinInfo;
    char							szString[255];

    do
    {
        // Read one record from STDF file.
        iStatus = StdfFile.LoadRecord(&StdfRecordHeader);
        if(iStatus != GS::StdLib::Stdf::NoError)
        {
            // End of file or error while reading...
            StdfFile.Close();	// Clean close.
            break;
        }

        // Process STDF record read PRR records
        if((StdfRecordHeader.iRecordType == 5) && (StdfRecordHeader.iRecordSubType == 20))
        {
             switch(StdfRecordHeader.iStdfVersion)
             {
             default :
               break;

                  case GEX_STDFV4:
                    StdfFile.ReadByte(&bData);		// head number
                    StdfFile.ReadByte(&bData);		// site number
                    StdfFile.ReadByte(&bData);		// part flag
                    StdfFile.ReadWord(&wData);		// number of tests
                    StdfFile.ReadWord(&wHardBin);	// HBIN
                    StdfFile.ReadWord(&wSoftBin); 	// SBIN
                    break;
             }
             if(bGetSofBin)
                mBinMap[wSoftBin] = wHardBin;	// Focus is on Soft bins
             else
                mBinMap[wHardBin] = wSoftBin;	// Focus is on Hard bins
        }
        // Process STDF record read HBR records
        if((StdfRecordHeader.iRecordType == 1) && (StdfRecordHeader.iRecordSubType == 40))
        {
            switch(StdfRecordHeader.iStdfVersion)
            {
            default :
                break;

                case GEX_STDFV4:
                    StdfFile.ReadByte(&bData);		// head number
                    StdfFile.ReadByte(&bData);		// site number
                    StdfFile.ReadWord(&wHardBin);	// Bin nb
                    StdfFile.ReadDword(&dwData);	// Skip Bin count
                    StdfFile.ReadByte(&bData);		// Pass/Fail status
                    StdfFile.ReadString(szString);	// Bin name
                    szString[254] = '\0';
                    break;
            }
            BinInfo	clBinInfo;
            clBinInfo.m_cBinCat = bData;
            if(*szString != 0)
                clBinInfo.m_strBinName = szString;
            mHBIN[wHardBin] = clBinInfo;
        }
        // Process STDF record read SBR records
        if((StdfRecordHeader.iRecordType == 1) && (StdfRecordHeader.iRecordSubType == 50))
        {
            switch(StdfRecordHeader.iStdfVersion)
            {
            default :
                    break;

                case GEX_STDFV4:
                    StdfFile.ReadByte(&bData);		// head number
                    StdfFile.ReadByte(&bData);		// site number
                    StdfFile.ReadWord(&wHardBin);	// Bin nb
                    StdfFile.ReadDword(&dwData);	// Skip Bin count
                    StdfFile.ReadByte(&bData);		// Pass/Fail status
                    StdfFile.ReadString(szString);	// Bin name
                    szString[254] = '\0';
                    break;
            }
            BinInfo	clBinInfo;
            clBinInfo.m_cBinCat = bData;
            if(*szString != 0)
                clBinInfo.m_strBinName = szString;
            mSBIN[wHardBin] = clBinInfo;
        }
    }
    while(1);	// Loop until MRR or end of file found

    // Create list of Soft & Hard bins found in file....
    QMap<int, int>::Iterator	it;
    QString						strSoftBin="?", strHardBin="?";
    for ( it = mBinMap.begin(); it != mBinMap.end(); ++it )
    {
        if(bGetSofBin)
        {
            // Focus is on Soft bins
            wSoftBin = it.key();
            wHardBin = it.value();
        }
        else
        {
            // Focus is on Hard bins
            wHardBin = it.key();
            wSoftBin = it.value();
        }
        itBinInfo = mSBIN.find(wSoftBin);
        if(itBinInfo != mSBIN.end())
            strSoftBin = itBinInfo.value().m_strBinName;
        itBinInfo = mHBIN.find(wHardBin);
        if(itBinInfo != mHBIN.end())
            strHardBin = itBinInfo.value().m_strBinName;

        QTreeWidgetItem * pTreeItem = new QTreeWidgetItem(treeWidget);
        pTreeItem->setText(0, QString::number(wSoftBin));
        pTreeItem->setText(1, strSoftBin);
        pTreeItem->setText(2, QString::number(wHardBin));
        pTreeItem->setText(3, strHardBin);
    }

    return true;
}
