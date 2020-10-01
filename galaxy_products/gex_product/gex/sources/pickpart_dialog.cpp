#include "pickpart_dialog.h"
#include "browser_dialog.h"
#include "report_build.h"
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include "gex_report.h"
#include "cpart_info.h"
#include "stdfparse.h"
#include "stdfrecords_v4.h"
#include "stdf_head_and_site_number_decipher.h"
#include <gqtl_log.h>

extern CGexReport*	gexReport;			// Handle to report class

PickPartDialog::findPart PickPartDialog::m_eFindPartMode = PickPartDialog::usePartID;

///////////////////////////////////////////////////////////
// Constructor for PartInfo object
//PartInfo::PartInfo(long ldRunID, QString strPartID, QString strCustomPartID, QString strDieXY, int nSoftBin, int nHardBin, int nSiteNb)
PartInfo::PartInfo(long ldRunID, QString strPartID, QString strCustomPartID, QString strDieXY, int nSoftBin, int nHardBin, int nSiteNb, QString strWaferId)
{
	m_ldRunID			= ldRunID;
	m_strPartID			= strPartID;
	m_strCustomPartID	= strCustomPartID;
	m_strDieXY			= strDieXY;
	m_nSoftBin			= nSoftBin;
	m_nHardBin			= nHardBin;
	m_nSiteNb			= nSiteNb;
	m_strWaferId		= strWaferId;		// case 3935
}

///////////////////////////////////////////////////////////
// Constructor
PickPartDialog::PickPartDialog( QWidget* parent, bool modal, Qt::WindowFlags f )
	: QDialog( parent, f )
{
	setupUi(this);
	setModal(modal);

	// Apply Examinator palette
	GexMainwindow::applyPalette(this);

	QObject::connect(PushButtonOk,		SIGNAL(clicked()),									this, SLOT(accept()));
	QObject::connect(PushButtonCancel,	SIGNAL(clicked()),									this, SLOT(reject()));
	QObject::connect(editFindPart,		SIGNAL(textChanged(QString)),						this, SLOT(onFindPart()));
	QObject::connect(treeWidget,		SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)),	this, SLOT(accept()));
	QObject::connect(comboFindPart,		SIGNAL(activated(int)),								this, SLOT(onChangeFindPartMode(int)));
	QObject::connect(mUi_WaferIDFilterComboBox, SIGNAL(currentIndexChanged(QString)),		this, SLOT(OnWaferIdSelectionChanged(QString)));		// case 3935

	// Init some variables
	m_bFoundCustomPartIDs = false;

	// Empty Filter list
	treeWidget->clear();

	// Multiple selection mode allowed
	treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);

	// Fill the list with sorting disabled
	treeWidget->setSortingEnabled(false);

	// Set minimum size for the Value column
	treeWidget->resizeColumnToContents(0);
	treeWidget->resizeColumnToContents(1);
	treeWidget->resizeColumnToContents(2);
	treeWidget->resizeColumnToContents(3);
	treeWidget->resizeColumnToContents(4);

	// Initialise the combo box used to define how to find a part in the list
	comboFindPart->insertItem(0, "Find Part (PartID)",			QVariant(PickPartDialog::usePartID));
	comboFindPart->insertItem(1, "Find Part (Custom PartID)",	QVariant(PickPartDialog::useCustomPartID));
	comboFindPart->insertItem(2, "Find Die: X,Y",				QVariant(PickPartDialog::useXY));

	// Select the last item used
	comboFindPart->setCurrentIndex(comboFindPart->findData(QVariant(m_eFindPartMode)));
}

///////////////////////////////////////////////////////////
// Destructor
PickPartDialog::~PickPartDialog()
{
}

///////////////////////////////////////////////////////////
// keep the FindPartMode used by the customer
///////////////////////////////////////////////////////////
void PickPartDialog::onChangeFindPartMode(int nIndex)
{
	m_eFindPartMode = (PickPartDialog::findPart) comboFindPart->itemData(nIndex).toInt();
}

///////////////////////////////////////////////////////////
// Show a list of PartIDs (or other info if needed) found in the file, their Bin and Run#
///////////////////////////////////////////////////////////
QString PickPartDialog::getPartsList(int iRawId/*=0*/,char cSeparator/*=','*/)
{
	QString				strPartsList	= "";
	int					iSelectionCount	= 0;

	// Pointer to first selection (if any)
	QTreeWidgetItem * pTreeWidgetItem = treeWidget->topLevelItem(0);

	// Return empty string if no parts selected
	if(pTreeWidgetItem == NULL)
		return strPartsList;

	// Scan the list and compute the parts list (if any selection made)
	while(pTreeWidgetItem != NULL)
	{
		// For each group of selected tests
		if(pTreeWidgetItem->isSelected() == true)
		{
			if(iSelectionCount)
				strPartsList += cSeparator;	// Separator between multiple selections

			strPartsList += pTreeWidgetItem->text(iRawId);	// Return info found in column 'iRawId'

			// Update counter index
			iSelectionCount++;
		}

		// Move to next item...and update test bloc markers.
		pTreeWidgetItem = treeWidget->itemBelow(pTreeWidgetItem);
	};

	return strPartsList;
}

QString	PickPartDialog::GetCurrentWaferId() const		// case 3935
{
	if(mUi_WaferIDFilterComboBox->count()<=1)		// no wafer id filter if only one wafer
		return QString();
	else
		return mUi_WaferIDFilterComboBox->currentText();
}

///////////////////////////////////////////////////////////
// Show a list of PartIDs found in a given group...
///////////////////////////////////////////////////////////
bool PickPartDialog::setGroupId(int iGroupID)
{
	return setGroupAndFileID(iGroupID, 0);
}

///////////////////////////////////////////////////////////
// Show a list of PartIDs found in a given group and file...
///////////////////////////////////////////////////////////
bool PickPartDialog::setGroupAndFileID(int nGroupID, int nFileID)
{
	// No data in memory!
	if(gexReport == NULL)
		return false;

    if(nGroupID < 0 || nGroupID >= (int)gexReport->getGroupsList().count())
		nGroupID = 0;

	// Get Handle to relevant group.
    CGexGroupOfFiles	*pGroup = gexReport->getGroupsList().at(nGroupID);
	if(pGroup == NULL)
		return false;

	if(nFileID < 0 || nFileID >= (int)pGroup->pFilesList.count())
		nFileID = 0;

	CGexFileInGroup	*pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.at(nFileID);
	if(pFile == NULL)
		return false;

	// Disabled the sort during the insertion
	treeWidget->setSortingEnabled(false);

	mUi_qfPtrWaferIdSelectionFrame->hide();		// case 3935

	// Get handle to Die X,Y info
	CTest	*ptDieX,*ptDieY,*ptSite,*ptSoftBin,*ptHardBin;

    if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEX,GEX_PTEST,&ptDieX,true,false) != 1)
		return false;
    if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEY,GEX_PTEST,&ptDieY,true,false) != 1)
		return false;
    if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_TESTING_SITE,GEX_PTEST,&ptSite,true,false) != 1)
		return false;
    if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_SBIN,GEX_PTEST,&ptSoftBin,true,false) != 1)
		return false;
    if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_HBIN,GEX_PTEST,&ptHardBin,true,false) != 1)
		return false;

	// Find sublot bound
	int nBeginOffset;
	int nEndOffset;

	ptDieX->findSublotOffset(nBeginOffset, nEndOffset, nFileID);

	// Scan all parts tested.
	QString				strDieXY;
	QString				strPartID;
	long				lPartIndex, iDieX, iDieY, iSiteID, iSoftBin, iHardBin;
	QTreeWidgetItem *	pTreeWidgetItem = NULL;

	for(lPartIndex = nBeginOffset; lPartIndex < nEndOffset; lPartIndex++)
    {
        if( (!ptDieX->m_testResult.isValidIndex(lPartIndex))        ||
                (!ptDieY->m_testResult.isValidIndex(lPartIndex))    ||
                (!ptSite->m_testResult.isValidIndex(lPartIndex))    ||
                (!ptSoftBin->m_testResult.isValidIndex(lPartIndex)) ||
                (!ptHardBin->m_testResult.isValidIndex(lPartIndex)) )
        {
            GEX_ASSERT(false);
            continue;
        }

		if (ptSoftBin->m_testResult.isValidResultAt(lPartIndex))
		{
			iDieX		= (int) ptDieX->m_testResult.resultAt(lPartIndex);
			iDieY		= (int) ptDieY->m_testResult.resultAt(lPartIndex);
			strDieXY	= QString::number(iDieX) + "," + QString::number(iDieY) ;
			iSiteID		= (int) ptSite->m_testResult.resultAt(lPartIndex);
			iSoftBin	= (int) ptSoftBin->m_testResult.resultAt(lPartIndex);
			iHardBin	= (int) ptHardBin->m_testResult.resultAt(lPartIndex);
            strPartID	= pFile->pPartInfoList.at(lPartIndex-nBeginOffset)->getPartID();

			pTreeWidgetItem = new PickPartTreeWidgetItem(treeWidget);
			pTreeWidgetItem->setText(0, QString::number(lPartIndex+1));
			pTreeWidgetItem->setText(1, strPartID);
			pTreeWidgetItem->setText(2, "-");
			pTreeWidgetItem->setText(3, QString::number(iSiteID));
			pTreeWidgetItem->setText(4, strDieXY);
			pTreeWidgetItem->setText(5, QString::number(iSoftBin));
			pTreeWidgetItem->setText(6, QString::number(iHardBin));
		}
	}

	// Hide custom PartID column as not needed here
	treeWidget->hideColumn(2);
	comboFindPart->removeItem(1);	// Remove find on Custom PartID

	// Enable sorting on a invalid column so only sorting is enabled, but no sorting performed yet!
	treeWidget->sortByColumn(99);
	treeWidget->setSortingEnabled(true);

	return true;
}

///////////////////////////////////////////////////////////
// Show a list of PartIDs found in the file, their Bin and Run#
// BG - Oct 2006: Ember request to display custom PartIDs (read from DTR records)
// BG - Oct 2006: Add column with site nb
///////////////////////////////////////////////////////////
bool PickPartDialog::setFile(QString strFile)
{
    GS::StdLib::Stdf				StdfFile;
	QString				strDataFile = strFile, strCustomPartID;
    GS::StdLib::StdfRecordReadInfo	StdfRecordHeader;
	int					iStatus;
	bool				bHasCustomPartID=false;
	QList <PartInfo*>	pPartList;

	m_bFoundCustomPartIDs = false;

	// If selected file is a CSV temporary edited file, then read the associated STDF file!
	if(strDataFile.endsWith(GEX_TEMPORARY_CSV) == true)
	{
		// remove the custom extension from the temporary csv file...to rebuild the STDF original name!
		strDataFile = strDataFile.remove(GEX_TEMPORARY_CSV);
	}

	// File exists?
	if(QFile::exists(strDataFile) == false)
		return false;	// No!

	// Open STDF file to read until MIR record found...have a 50K cache buffer used (instead of 2M !)
	iStatus = StdfFile.Open(strDataFile.toLatin1().constData(),STDF_READ,1000000L);
    if(iStatus != GS::StdLib::Stdf::NoError)
		return false;

	// Disable the sort during the insertion
	treeWidget->setSortingEnabled(true);
	mUi_qfPtrWaferIdSelectionFrame->show();		// case 3935

	// Read all file...until end of file reached.
	char	szString[257];
	char	szStringWaferId[257];
	long	ldPartsSampledID=0;
	BYTE	bData,bSiteNb;
	long	lData;
	int		wData,wSoftBin,wHardBin;
	QString	strDieXY;
	QString strWaferId;

    while(StdfFile.LoadRecord(&StdfRecordHeader) == GS::StdLib::Stdf::NoError)
	{
        // Process STDF records: DTR, PRR...

        // GDR ??
        if( ( StdfRecordHeader.iRecordType == 50 ) && ( StdfRecordHeader.iRecordSubType == 10 ) )
        {
            // set a specific deciphering method for site number if needed
            GQTL_STDF::Stdf_GDR_V4 gdr;
            gdr.Read( StdfFile );

            GQTL_STDF::HeadAndSiteNumberDecipher::SetDecipheringModeInStdf
                ( gdr, StdfFile );
        }

		// DTR ??
		if((StdfRecordHeader.iRecordType == 50) && (StdfRecordHeader.iRecordSubType == 30))
		{
			// Read DTR line
            if(StdfFile.ReadString(szString)  == GS::StdLib::Stdf::NoError)
			{
				QString strString = szString;
				strString = strString.toLower();

				// Non-supported format, for customer Ember: "EUI number: xxxxxxxxxxxxxxxx". Indicates custom partID for next PRR.
				if(strString.startsWith("eui number:", Qt::CaseInsensitive))
				{
					// Retrieve custom partID to use when reading next PRR
					//strCustomPartID = strString.mid(11).trimmed();
					//bHasCustomPartID = true;
					strString = strString.mid(11).trimmed();
					strString = "<cmd> partid = " + strString;
				}

				// <cmd> partid = xxxxxxxxxxxxxxxx
				if(strString.startsWith("<cmd>"))
				{
					strString = strString.mid(5).trimmed();
					if(strString.startsWith("partid"))
					{
						strString = strString.mid(6).trimmed();
						if(strString.startsWith("="))
						{
							strCustomPartID = strString.mid(1).trimmed();
							bHasCustomPartID = true;
						}
					}
				}
			}
		}

		// PRR ??
		if((StdfRecordHeader.iRecordType == 5) && (StdfRecordHeader.iRecordSubType == 20))
		{
            unsigned short siteNumber = 0;

            // Reset DieXY variable
			strDieXY = "";
			 switch(StdfRecordHeader.iStdfVersion)
			 {
             default :
             break;

				  case GEX_STDFV4:
					StdfFile.ReadByte(&bData);		// head number
					StdfFile.ReadByte(&bSiteNb);	// site number

                    GQTL_STDF::Stdf_PRR_V4 prr;
                    prr.SetHEAD_NUM(bData);
                    prr.SetSITE_NUM(bSiteNb);

                    siteNumber =
                        GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
                        ( StdfFile, prr );

					StdfFile.ReadByte(&bData);		// part flag
					StdfFile.ReadWord(&wData);		// number of tests
					StdfFile.ReadWord(&wHardBin);	// HBIN
					StdfFile.ReadWord(&wSoftBin); 	// SBIN
					StdfFile.ReadWord(&wData);		// DieX
					if(wData >= 32768) wData -= 65536;
					if(wData != -32768)
						strDieXY = QString::number(wData) + ",";
					StdfFile.ReadWord(&wData);		// DieY
					if(wData >= 32768) wData -= 65536;
					if(wData != -32768)
						strDieXY += QString::number(wData);
					StdfFile.ReadDword(&lData);		// Execution time
					*szString = 0;
					StdfFile.ReadString(szString);	// PART ID.
					break;
			 }

			 // Keep track of run#
			 ldPartsSampledID++;

			 // Add entry to list
			 if(bHasCustomPartID)
			 {
                 pPartList.append(new PartInfo(ldPartsSampledID, szString, strCustomPartID, strDieXY, wSoftBin, wHardBin, (int)siteNumber, strWaferId));
				 m_bFoundCustomPartIDs = true;
				 bHasCustomPartID = false;
			 }
			 else
                 pPartList.append(new PartInfo(ldPartsSampledID, szString, "n/a", strDieXY, wSoftBin, wHardBin, (int)siteNumber, strWaferId));
		}

		// WIR		(case 3935)
		if((StdfRecordHeader.iRecordType == 2) && (StdfRecordHeader.iRecordSubType == 10))
		{
			// Reset runID count
			ldPartsSampledID = 0;

			switch(StdfRecordHeader.iStdfVersion)
			{
            default :
				break;

			case GEX_STDFV4:
				StdfFile.ReadByte(&bData);				// head number
				StdfFile.ReadByte(&bData);				// site group number
				StdfFile.ReadDword(&lData);				// start time
				StdfFile.ReadString(szStringWaferId);	// wafer id
				break;
			}

			strWaferId = QString(szStringWaferId);
		}


	};

	// End of file or error while reading...
	StdfFile.Close();	// Clean close.

	// Insert items into listView
	PartInfo *			pPartInfo;
	QTreeWidgetItem *	pTreeWidgetItem = NULL;
	QString				strConsideratedWaferId;
	QList<QString >		qlWaferIdList;

	while (!pPartList.isEmpty())
	{
		pPartInfo = pPartList.takeFirst();
		strConsideratedWaferId = pPartInfo->m_strWaferId;

		// Add item to list
		pTreeWidgetItem = new PickPartTreeWidgetItem(treeWidget);
		pTreeWidgetItem->setText(0, QString::number(pPartInfo->m_ldRunID));
		pTreeWidgetItem->setText(1, pPartInfo->m_strPartID);
		pTreeWidgetItem->setText(2, pPartInfo->m_strCustomPartID);
		pTreeWidgetItem->setText(3, QString::number(pPartInfo->m_nSiteNb));
		pTreeWidgetItem->setText(4, pPartInfo->m_strDieXY);
		pTreeWidgetItem->setText(5, QString::number(pPartInfo->m_nSoftBin));
		pTreeWidgetItem->setText(6, QString::number(pPartInfo->m_nHardBin));
		pTreeWidgetItem->setText(7, strConsideratedWaferId);	// (case 3935)

		if(!qlWaferIdList.contains(strConsideratedWaferId))
			qlWaferIdList.append(strConsideratedWaferId);

		delete pPartInfo;
		pPartInfo=0;
	}

	// Enable sorting on a invalid column so only sorting is enabled, but no sorting performed yet!
	treeWidget->sortByColumn(99);
	treeWidget->setSortingEnabled(true);

	// wafer id management		(case 3935)
	treeWidget->hideColumn(7);		// case 3935
	if(qlWaferIdList.count()>1)
	{
		mUi_WaferIDFilterComboBox->clear();
		QListIterator<QString > qliWaferIdIterator(qlWaferIdList);
		while(qliWaferIdIterator.hasNext())
			mUi_WaferIDFilterComboBox->addItem(qliWaferIdIterator.next());
		OnWaferIdSelectionChanged(qlWaferIdList.first());
	}
	else
	{
		mUi_qfPtrWaferIdSelectionFrame->hide();
	}


	// Hide custom PartID column if beed be
	if(m_bFoundCustomPartIDs == false)
	{
		treeWidget->hideColumn(2);		// Remove Custom PartID column
		comboFindPart->removeItem(1);	// Remove find on Custom PartID
	}

	return true;
}

///////////////////////////////////////////////////////////
// Find part by PartID or Custom PartID
// BG - Nov 2006: Ember request to quickly find a part
///////////////////////////////////////////////////////////
void PickPartDialog::onFindPart()
{
	QString						strPartID				= editFindPart->text();
	QTreeWidgetItem *			pTreeWidgetFirstItem	= treeWidget->topLevelItem(0);
	QTreeWidgetItem *			pTreeWidgetCurrentItem	= treeWidget->currentItem();
	QList<QTreeWidgetItem *>	lstTreeWidgetItem;

	if(pTreeWidgetFirstItem)
		treeWidget->setCurrentItem(pTreeWidgetFirstItem);

	// Check find mode
	if(m_eFindPartMode == usePartID)
		// Find on PartID
		lstTreeWidgetItem = treeWidget->findItems(strPartID, Qt::MatchStartsWith, 1);
	else
	if(m_eFindPartMode == useCustomPartID)
		// Find on Custom PartID
		lstTreeWidgetItem = treeWidget->findItems(strPartID, Qt::MatchStartsWith, 2);
	else
	if(m_eFindPartMode == useXY)
		// Find on Custom PartID
		lstTreeWidgetItem = treeWidget->findItems(strPartID, Qt::MatchStartsWith, 4);

	// Select item
	if(lstTreeWidgetItem.size() > 0)
	{
		treeWidget->clearSelection();
		treeWidget->setCurrentItem(lstTreeWidgetItem.at(0));
		lstTreeWidgetItem.at(0)->setSelected(true);
	}
	else if(pTreeWidgetCurrentItem)
		treeWidget->setCurrentItem(pTreeWidgetCurrentItem);
}


void PickPartDialog::OnWaferIdSelectionChanged(QString strSelectedWaferId) //int nWaferIndexInCombo)
{
//	if(mUi_WaferIDFilterComboBox->currentText() == strSelectedWaferId)
//		return;

    int i = mUi_WaferIDFilterComboBox->findText(strSelectedWaferId);
    if (i != -1)
        mUi_WaferIDFilterComboBox->setCurrentIndex(i);
    else if (mUi_WaferIDFilterComboBox->isEditable())
        mUi_WaferIDFilterComboBox->setEditText(strSelectedWaferId);
    else
        mUi_WaferIDFilterComboBox->setItemText(mUi_WaferIDFilterComboBox->currentIndex(), strSelectedWaferId);
//	mUi_WaferIDFilterComboBox->setCurrentText(strSelectedWaferId);


	//QString strSelectedWaferId = mUi_WaferIDFilterComboBox->text(nWaferIndexInCombo);
	QTreeWidgetItem* qtwiPtrConsideratedItem = NULL;
	for(int ii=0; ii<treeWidget->topLevelItemCount(); ii++)
	{
		qtwiPtrConsideratedItem = treeWidget->topLevelItem(ii);
		if(!qtwiPtrConsideratedItem)
		{
			GEX_ASSERT(false);
			return;
		}

		if( strSelectedWaferId == (qtwiPtrConsideratedItem->text(7)) )
		{
			//qtwiPtrConsideratedItem->setFlags(Qt::ItemIsEnabled);
			qtwiPtrConsideratedItem->setHidden(false);
		}
		else
		{
			// qtwiPtrConsideratedItem->setFlags(Qt::NoItemFlags);
			qtwiPtrConsideratedItem->setHidden(true);
		}
	}

}

///////////////////////////////////////////////////////////////////////////////////
// Class PickPartTreeWidgetItem - class which represents an part item
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
PickPartTreeWidgetItem::PickPartTreeWidgetItem(QTreeWidget * pTreeWidget) : QTreeWidgetItem(pTreeWidget)
{
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
PickPartTreeWidgetItem::~PickPartTreeWidgetItem()
{
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool operator< ( const QTreeWidgetItem & other ) const
//
// Description	:	Override the comparison methods
//
///////////////////////////////////////////////////////////////////////////////////
bool PickPartTreeWidgetItem::operator< ( const QTreeWidgetItem & other ) const
{
	int		nCol		= treeWidget() ? treeWidget()->sortColumn() : 0;
	QString strLeft		= text(nCol);
	QString strRight	= other.text(nCol);

	switch(nCol)
	{
		case 0 :
		case 3 :
		case 5 :
		case 6 :	return sortInteger(strLeft, strRight);

		case 1 :
		case 2 :	return sortPartID(strLeft, strRight);

		case 4 :	return sortCoordinates(strLeft, strRight);

		default:	return false;
	}
}

bool PickPartTreeWidgetItem::sortInteger(const QString& strLeft, const QString& strRight) const
{
	return strLeft.toInt() < strRight.toInt();
}

bool PickPartTreeWidgetItem::sortPartID(const QString& strLeft, const QString& strRight) const
{
	bool	bOkLeft			= false;
	bool	bOkRight		= false;
	int		nLeftPartID		= strLeft.toInt(&bOkLeft);
	int		nRightPartID	= strRight.toInt(&bOkRight);

	if (bOkLeft && bOkRight)
		return nLeftPartID < nRightPartID;

	return (strLeft.compare(strRight) < 0);
}

bool PickPartTreeWidgetItem::sortCoordinates(const QString& strLeft, const QString& strRight) const
{
	int nLeftXDie	= strLeft.section(',', 0, 0).toInt();
	int nRightXDie	= strRight.section(',', 0, 0).toInt();
	int nLeftYDie	= strLeft.section(',', 1, 1).toInt();
	int nRightYDie	= strRight.section(',', 1, 1).toInt();

	if (nLeftXDie < nRightXDie)
		return true;

	if (nLeftXDie == nRightXDie && nLeftYDie < nRightYDie)
		return true;

	return false;
}

