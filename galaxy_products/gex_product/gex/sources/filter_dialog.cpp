#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif

#include <stdio.h>
#include <QDialog>
#include <QFileDialog>
#include <QDesktopServices>

#if defined unix || __MACH__
#include <unistd.h>
#include <stdlib.h>
#endif

#include "browser_dialog.h"
#include "filter_dialog.h"
#include "report_options.h"
#include "stdf.h"
#include "import_all.h"			// ATDF, WAT, PCM, CSV, etc...=> to STDF converter
#include "engine.h"
#include "temporary_files_manager.h"
#include <gqtl_log.h>
#include "pickname_dialog.h"
#include "pickpart_dialog.h"
#include "pickbin_dialog.h"
#include "pickcoord_dialog.h"
#include "stdf_content_utils.h"

// Galaxy QT libraries
#include "gqtl_sysutils.h"
#include "message.h"

// main.cpp
extern GexMainwindow *	pGexMainWindow;

extern CReportOptions	ReportOptions;		// Holds options (report_build.h)


///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
FilterDialog::FilterDialog(QTreeWidget * pTreeWidgetFiles,
                           QWidget* parent,
                           bool modal,
                           Qt::WindowFlags f )
    : QDialog( parent, f )
{
    setupUi(this);
    setModal(modal);

    // Apply Examinator palette
    GexMainwindow::applyPalette(this);

    QObject::connect(comboProcess,				SIGNAL(activated(QString)), this, SLOT(OnComboProcessChange()));
    QObject::connect(comboSites,				SIGNAL(activated(QString)), this, SLOT(OnComboSitesChange()));
    QObject::connect(comboGroupType,			SIGNAL(activated(QString)), this, SLOT(OnComboGroupTypeChange()));
    QObject::connect(PushButtonCancel,			SIGNAL(clicked()),			this, SLOT(reject()));
    QObject::connect(pushButtonMapTests,		SIGNAL(clicked()),			this, SLOT(OnMapTests()));
    QObject::connect(PushButtonOk,				SIGNAL(clicked()),			this, SLOT(accept()));
    QObject::connect(pushButtonCreateMapTests,	SIGNAL(clicked()),			this, SLOT(OnCreateMapTests()));
    QObject::connect(pushButtonSuggestName,		SIGNAL(clicked()),			this, SLOT(OnSuggestDatasetName()));

    mTreeWidgetFiles = pTreeWidgetFiles;
    comboGroupType->setToolTip("If set to ‘Yes’ this dataset will be a control group which will be compared"
                               "\nagainst the sample group in the shift analysis report."
                               "\nIf set to ‘No’ this dataset will be a samples group.");

    // Fills Filter combo box list (sorting disabled)
    int nItem = 0;
    while (ProcessPartsItems[nItem] != 0)
    {
        comboProcess->insertItem(nItem, ProcessPartsItems[nItem]);
        nItem++;
    }

    // Edit field maximum allowed length
    editProcess->setMaxLength(32000);

    // Temperature edit field only accepts number
    editTemperature->setValidator(new QDoubleValidator( -500.0, 999.0, 2,editTemperature));

    // Case 3935: Hide WaferID filter (shown only if multiple wafers in same file, after pick parts filter has been executed)
    mUi_qfPtrWaferDisplayFrame->hide();
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
FilterDialog::~FilterDialog()
{

}

///////////////////////////////////////////////////////////
// Reads a string from STDF file
///////////////////////////////////////////////////////////
void FilterDialog::ReadStringToField(GS::StdLib::Stdf *pStdfFile,char *szField)
{
    char	szString[257];	// A STDF string is 256 bytes long max!

    // Empties string.
    *szField=0;

    // Read string from STDF file.
    if(pStdfFile->ReadString(szString)  != GS::StdLib::Stdf::NoError)
      return;
    // Security: ensures we do not overflow destination buffer !
    szString[MIR_STRING_SIZE-1] = 0;
    strcpy(szField,szString);
}

///////////////////////////////////////////////////////////
// Reads STDF MIR record.
///////////////////////////////////////////////////////////
void FilterDialog::GetDataFileHeader(int iSelectedItems)
{
    GS::StdLib::Stdf				StdfFile;
    GS::StdLib::StdfRecordReadInfo	StdfRecordHeader;
    int					iStatus;
    QTreeWidgetItem *	pTreeWidgetItem = NULL;

    // Clear+Disable sorting mode, so data are listed as they are entered!
    treeWidget->clear();
    treeWidget->setSortingEnabled(false);

    // If file doesn't exist, simply quietly return!
    if(QFile::exists(mFile) == false)
        return;

    // If  multiple files selection, can't show a MIR!
    if(iSelectedItems > 1)
    {
        pTreeWidgetItem = new QTreeWidgetItem(treeWidget);
        pTreeWidgetItem->setText(0, "No description available...");
        pTreeWidgetItem->setText(1, "*INFO*");

        return;
    }

    // If selected file is a CSV temporary edited file, then read the associated STDF file!
    if(mFile.endsWith(GEX_TEMPORARY_CSV) == true)
    {
        // remove the custom extension from the temporary csv file...to rebuild the STDF original name!
        mFile = mFile.remove(GEX_TEMPORARY_CSV);
    }

    // Copy test data file name to read.
    mStdfFile = mFile;

    bool bAllowWizard=true;

    // If not a STDF file (eg.: ATDF, GDF, CSV,...)...convert it to STDF!;
    GS::Gex::ConvertToSTDF  StdfConvert;
    QString                 strErrorMessage;
    bool                    bFileCreated;
    int                     nConvertStatus;
    QStringList             lstStdfFile;
    if(mTreeWidgetFiles == NULL)
    {
        nConvertStatus = StdfConvert.Convert(false,bAllowWizard,false,false,mFile, mStdfFile,GEX_TEMPORARY_STDF,bFileCreated,strErrorMessage);
        lstStdfFile.append(mStdfFile);
    }
    else
    {
        nConvertStatus = StdfConvert.Convert(false,bAllowWizard,false,false,mFile, lstStdfFile,GEX_TEMPORARY_STDF,bFileCreated,strErrorMessage);
    }
    mStdfFile = lstStdfFile.first();
    if(lstStdfFile.count()>1)
    {
        // Add new entry file to the current list
        int					iFileIndex			= 0;			// pos of the file in this list
        QTreeWidgetItem *	pTreeWidgetItem		= mTreeWidgetFiles->currentItem();
        QTreeWidgetItem *	pTreeWidgetNewItem	= pTreeWidgetItem;
        QString				strGroup			= pTreeWidgetItem->text(0);

        for(iFileIndex=0; iFileIndex<8; iFileIndex++)
        {
            if(pTreeWidgetItem->text(iFileIndex).contains(mFile, Qt::CaseInsensitive))
                break;
        }

        // And change the FileName
        mFile = mStdfFile;
        pTreeWidgetItem->setText(iFileIndex,mFile);

        for(int i=1; i<lstStdfFile.count(); i++)
        {
            // Update list of temporary STDF files created if needed.
            if(bFileCreated == true)
                GS::Gex::Engine::GetInstance().GetTempFilesManager().addFile(lstStdfFile[i], TemporaryFile::BasicCheck);
            // Check if have to increment Group Name
            if(!strGroup.isEmpty() && (mTreeWidgetFiles->headerItem()->text(0).contains("Group",Qt::CaseInsensitive)))
                strGroup = "DataSet_"+QString::number(mTreeWidgetFiles->topLevelItemCount());

            pTreeWidgetNewItem = new QTreeWidgetItem(mTreeWidgetFiles, pTreeWidgetNewItem);
            pTreeWidgetNewItem->setText(0, strGroup);
            pTreeWidgetNewItem->setText(1, pTreeWidgetItem->text(1));
            pTreeWidgetNewItem->setText(2, pTreeWidgetItem->text(2));
            pTreeWidgetNewItem->setText(3, pTreeWidgetItem->text(3));
            pTreeWidgetNewItem->setText(4, pTreeWidgetItem->text(4));
            pTreeWidgetNewItem->setText(5, pTreeWidgetItem->text(5));

            pTreeWidgetNewItem->setText(iFileIndex, lstStdfFile[i]);
        }

    }

    // Failed converting to STDF file
    if(nConvertStatus == GS::Gex::ConvertToSTDF::eConvertError)
    {
        // Remove leading \r & \n if any
        strErrorMessage.replace("\r","");
        strErrorMessage.replace("\n","");

        pTreeWidgetItem = new QTreeWidgetItem(treeWidget);
        pTreeWidgetItem->setText(0, strErrorMessage);
        pTreeWidgetItem->setText(1, "*ERROR*");
        return;
    }

    // Update list of temporary STDF files created if needed.
    if(bFileCreated == true)
        GS::Gex::Engine::GetInstance().GetTempFilesManager().addFile(mStdfFile, TemporaryFile::BasicCheck);

    iStatus = StdfFile.Open(mStdfFile.toLatin1().constData(),STDF_READ,1000000L);
    if(iStatus != GS::StdLib::Stdf::NoError)
    {
        pTreeWidgetItem = new QTreeWidgetItem(treeWidget);
        pTreeWidgetItem->setText(0, "No description available...");
        pTreeWidgetItem->setText(1, "*INFO*");
        return;
    }

    // Read all file...until MIR is found.
    do
    {
        // Read one record from STDF file.
        iStatus = StdfFile.LoadRecord(&StdfRecordHeader);
        if(iStatus != GS::StdLib::Stdf::NoError)
        {
            // End of file or error while reading...but no MIR found!
            pTreeWidgetItem = new QTreeWidgetItem(treeWidget);
            pTreeWidgetItem->setText(0, "No description available...");
            pTreeWidgetItem->setText(1, "*INFO*");

            StdfFile.Close();	// Clean close.

            return;
        }

        // Process STDF record read.
        if((StdfRecordHeader.iRecordType == 1) && (StdfRecordHeader.iRecordSubType == 10))
        {
            BYTE	bData,bStation = BYTE();
            int		wData;
            time_t	tSetupTime=0,tStartTime=0;
            char	szString[MIR_STRING_SIZE];
            char	szLot[MIR_STRING_SIZE];
            char	szPartType[MIR_STRING_SIZE];
            char	szNodeName[MIR_STRING_SIZE];
            char	szTesterType[MIR_STRING_SIZE];
            char	szJobName[MIR_STRING_SIZE];
            char	szJobRev[MIR_STRING_SIZE];
            char	szSubLot[MIR_STRING_SIZE];
            char	szOperator[MIR_STRING_SIZE];
            char	szExecType[MIR_STRING_SIZE];
            char	szExecVer[MIR_STRING_SIZE];
            char	*ptChar;

            // File name
            pTreeWidgetItem = new QTreeWidgetItem(treeWidget);
            pTreeWidgetItem->setText(0, "File name");
            pTreeWidgetItem->setText(1, mFile);

            // MIR found, get data!
            switch(StdfRecordHeader.iStdfVersion)
            {
            default :
                break;

              case GEX_STDFV4:
                StdfFile.ReadDword((long *)&tSetupTime);			// Setup_T
                StdfFile.ReadDword((long *)&tStartTime);			// Start_T
                StdfFile.ReadByte(&bStation);		// stat #
                StdfFile.ReadByte(&bData);			// mode_code
                StdfFile.ReadByte(&bData);			// rtst_code
                StdfFile.ReadByte(&bData);			// prot_cod #
                StdfFile.ReadWord(&wData);			// burn_time
                StdfFile.ReadByte(&bData);			// cmode_code
                ReadStringToField(&StdfFile,szLot);
                ReadStringToField(&StdfFile,szPartType);
                ReadStringToField(&StdfFile,szNodeName);
                ReadStringToField(&StdfFile,szTesterType);
                ReadStringToField(&StdfFile,szJobName);
                ReadStringToField(&StdfFile,szJobRev);
                ReadStringToField(&StdfFile,szSubLot);
                ReadStringToField(&StdfFile,szOperator);
                ReadStringToField(&StdfFile,szExecType);
                ReadStringToField(&StdfFile,szExecVer);

                pTreeWidgetItem = new QTreeWidgetItem(treeWidget);
                pTreeWidgetItem->setText(0, "STDF Version");
                pTreeWidgetItem->setText(1, "4.0");

                 break;
            }

            // Fills list with MIR data read.
            strcpy(szString,TimeStringUTC(tSetupTime));
            ptChar = strchr(szString,'\n');
            if(ptChar != NULL) *ptChar = 0;	// Remove leading '\n'

            pTreeWidgetItem = new QTreeWidgetItem(treeWidget);
            pTreeWidgetItem->setText(0, "Setup time");
            pTreeWidgetItem->setText(1, szString);

            strcpy(szString,TimeStringUTC(tStartTime));
            ptChar = strchr(szString,'\n');
            if(ptChar != NULL)
                *ptChar = 0;	// Remove leading '\n'

            pTreeWidgetItem = new QTreeWidgetItem(treeWidget);
            pTreeWidgetItem->setText(0, "Start time");
            pTreeWidgetItem->setText(1, szString);

            pTreeWidgetItem = new QTreeWidgetItem(treeWidget);
            pTreeWidgetItem->setText(0, "Program");
            pTreeWidgetItem->setText(1, szJobName);

            pTreeWidgetItem = new QTreeWidgetItem(treeWidget);
            pTreeWidgetItem->setText(0, "Revision");
            pTreeWidgetItem->setText(1, szJobRev);

            pTreeWidgetItem = new QTreeWidgetItem(treeWidget);
            pTreeWidgetItem->setText(0, "Lot");
            pTreeWidgetItem->setText(1, szLot);

            pTreeWidgetItem = new QTreeWidgetItem(treeWidget);
            pTreeWidgetItem->setText(0, "Sub-Lot");
            pTreeWidgetItem->setText(1, szSubLot);

            pTreeWidgetItem = new QTreeWidgetItem(treeWidget);
            pTreeWidgetItem->setText(0, "Tester name");
            pTreeWidgetItem->setText(1, szNodeName);

            pTreeWidgetItem = new QTreeWidgetItem(treeWidget);
            pTreeWidgetItem->setText(0, "Tester type");
            pTreeWidgetItem->setText(1, szTesterType);

            sprintf(szString,"%d",(int)bStation);

            pTreeWidgetItem = new QTreeWidgetItem(treeWidget);
            pTreeWidgetItem->setText(0, "Station");
            pTreeWidgetItem->setText(1, szString);

            pTreeWidgetItem = new QTreeWidgetItem(treeWidget);
            pTreeWidgetItem->setText(0, "Part type");
            pTreeWidgetItem->setText(1, szPartType);

            pTreeWidgetItem = new QTreeWidgetItem(treeWidget);
            pTreeWidgetItem->setText(0, "Operator");
            pTreeWidgetItem->setText(1, szOperator);

            pTreeWidgetItem = new QTreeWidgetItem(treeWidget);
            pTreeWidgetItem->setText(0, "Exec_type");
            pTreeWidgetItem->setText(1, szExecType);

            pTreeWidgetItem = new QTreeWidgetItem(treeWidget);
            pTreeWidgetItem->setText(0, "Exec version");
            pTreeWidgetItem->setText(1, szExecVer);

            StdfFile.Close();	// Clean close.
            return;
        }
    }
    while(1);	// Loop until MIR or end of file found
}

///////////////////////////////////////////////////////////
// Called to specify information to list in filter
///////////////////////////////////////////////////////////
void FilterDialog::SetParameters(int selectedItems,
                                 bool editGroupName,
                                 int columnOffset,
                                 QTreeWidgetItem * treeWidgetItemSelection,
                                 int flags/*=0*/, QStringList sampleGroups)
{
    QString lSite,lParts,lRange,lTemperature;

    // Option to split each site as an individual chart
    if( flags & GEX_FILE_FILTER_ALLOWSPLIT)
    {
        // Enable option to split each site in a file (user has to do it manually)
        TextLabelMultiSites->show();
        comboMultiSites->show();
    }
    else
    {
        // Disable option to split each site in a file (user has to do it manually)
        TextLabelMultiSites->hide();
        comboMultiSites->hide();
    }
    // Disable site filtering?
    if( flags & GEX_FILE_FILTER_NOSITE)
    {
        comboSites->setEnabled(false);
        editSites->hide();
        comboMultiSites->setEnabled(false);
    }

    // Disable Group name selection?
    if( flags & GEX_FILE_FILTER_NOGROUPNAME)
    {
        TextLabelGroupName->hide();
        LineEditGroupName->hide();
        pushButtonSuggestName->hide();
    }

    // Disable Advanced options (tab widget?
    if( flags & GEX_FILE_FILTER_NOADVANCEDTAB)
    {
        tabWidget->removeTab(1);
    }

    if( flags & GEX_FILE_FILTER_ALLOWGROUPTYPE)
    {
        mIsGroupTypeEditable = true;
    }
    else
    {
        mIsGroupTypeEditable = false;
    }

    if (flags & GEX_FILE_FILTER_NOCONTROL)
    {
        TextLabelGroupType->hide();
        comboBoxSampleGroup->hide();
        comboGroupType->hide();
        labelSampleGroup->hide();
    }


    // Reset flags
    mIsSamplesEdited            = false;
    // Saves pointer to file structure for back-update
    mTreeWidgetItemFileFilter	= treeWidgetItemSelection;
    mColumnOffset				= columnOffset;
    mIsGroupNameEditable		= editGroupName;
    mSelectedItems				= selectedItems;

    // No file selected, so disable all file hader Dump info.
    if(treeWidgetItemSelection == NULL)
    {
        PickParts->hide();
        tabWidget->hide();
        // Ensures fields are in the correct enable/disable mode
        OnComboProcessChange();
        OnComboSitesChange();
        return;
    }

    // check if the GroupName edit box has to be seen (Ignored when multiple files selected!)
    if(mIsGroupNameEditable==true)
    {
        LineEditGroupName->show();
        TextLabelGroupName->show();
        pushButtonSuggestName->show();
        // Get group name...load it into the TextEdit field.
        mGroupName    = treeWidgetItemSelection->text(mColumnOffset);

        if (mTreeWidgetFiles->indexOfTopLevelItem(treeWidgetItemSelection) == 0 && mGroupName.right(6) == " (Ref)")
            mGroupName.truncate(mGroupName.length()-6);

        LineEditGroupName->setText(mGroupName);
        LineEditGroupName->setFocus();
        LineEditGroupName->selectAll();
        // Now that we have the group, now point on the site/parts/range/file name structure!
        columnOffset++;
    }

    if ((mIsGroupNameEditable == false) || (mSelectedItems > 1))
    {
        LineEditGroupName->hide();
        TextLabelGroupName->hide();
        pushButtonSuggestName->hide();
    }

    // Extract filter info from list columns.
    lSite		= treeWidgetItemSelection->text(columnOffset);
    lSite		= lSite.trimmed();	// remove leading spaces.
    lParts		= treeWidgetItemSelection->text(columnOffset+1);
    lParts		= lParts.trimmed();	// remove leading spaces.
    lRange		= treeWidgetItemSelection->text(columnOffset+2);
    lTemperature= treeWidgetItemSelection->text(columnOffset+3);

    // Copy to GUI.
    mFile		= mOriginalFile = treeWidgetItemSelection->text(columnOffset+4);
    mMapTests	= treeWidgetItemSelection->text(columnOffset+5);

    if (mIsGroupTypeEditable && (mSelectedItems == 1))
    {
        if (sampleGroups.isEmpty())
        {
            TextLabelGroupType->show();
            comboGroupType->show();
            comboGroupType->setEnabled(false);
            labelSampleGroup->hide();
            comboBoxSampleGroup->hide();
        }
        else
        {
            QString lIsGroupControls= treeWidgetItemSelection->text(columnOffset + 7);
            lIsGroupControls = lIsGroupControls.trimmed();
            TextLabelGroupType->show();
            comboGroupType->show();
            QString lIsControl = lIsGroupControls.startsWith("yes", Qt::CaseInsensitive) ? "Yes" : "No";
            comboGroupType->setCurrentIndex(comboGroupType->findData(lIsControl, Qt::DisplayRole));
            comboBoxSampleGroup->clear();
            comboBoxSampleGroup->insertItem(0, "-- Select sample group --");
            comboBoxSampleGroup->insertItems(1, sampleGroups);
            QString lSampleGroup = lIsGroupControls.section(" - ", 1, 1).trimmed();
            if (!lSampleGroup.isEmpty())
            {
                comboBoxSampleGroup->setCurrentIndex(comboBoxSampleGroup->findData(lSampleGroup, Qt::DisplayRole));
            }
            if (lIsGroupControls.startsWith("yes", Qt::CaseInsensitive) && !sampleGroups.isEmpty())
            {
                labelSampleGroup->show();
                comboBoxSampleGroup->show();
            }
            else
            {
                labelSampleGroup->hide();
                comboBoxSampleGroup->hide();
            }
        }
    }
    else
    {
        TextLabelGroupType->hide();
        comboGroupType->hide();
        labelSampleGroup->hide();
        comboBoxSampleGroup->hide();
    }

    LineEditMapTests->setText(mMapTests);
    editTemperature->setText(lTemperature);

    // Find the Type of parts to process
    int iIndex=0;
    while(ProcessPartsItems[iIndex] != 0)
    {
        if( lParts == ProcessPartsItems[iIndex])
            break;	// found matching string
        iIndex++;
    };	// loop until we have found the string entry.

    comboProcess->setCurrentIndex(iIndex);
    editProcess->setText(lRange);

    // Type of sites to filter
    if(!qstricmp(lSite.toLatin1().constData(),"all"))
        comboSites->setCurrentIndex(0);	// ALL sites
    else
    {
        comboSites->setCurrentIndex(1);	// Specific site
        editSites->setText(lSite);	// Update Edit box with site#
    }

    // Ensures fields are in the correct enable/disable mode
    OnComboProcessChange();
    OnComboSitesChange();

    // Read Data file header (STDF MIR record)
    GetDataFileHeader(mSelectedItems);
}

///////////////////////////////////////////////////////////
// User Cancels dialog box...checks if DataEditor was executed
///////////////////////////////////////////////////////////
void FilterDialog::selectFileName(QString &strFileName)
{
    mFile  = mOriginalFile = strFileName;

    // Read Data file header (STDF MIR record)
    GetDataFileHeader(mSelectedItems);
}

///////////////////////////////////////////////////////////
// User Cancels dialog box...checks if DataEditor was executed
///////////////////////////////////////////////////////////
void FilterDialog::reject()
{
    // If Editor not launched, or already used multiple times, just return.
    if(mIsSamplesEdited == false)
    {
        done(0);
        return;
    }

    if(mOriginalFile.endsWith(GEX_TEMPORARY_CSV) == true)
    {
        done(0);
        return;
    }

    // User use the Data editor, and a CSV file was created...ask if abort?
    bool lOk;
    GS::Gex::Message::request("",
        "You have clicked the 'Cancel' button, but you may\n"
        "have modified your data with the Data Editor...\n"
        "Do you want to ignore your edits ?", lOk);
    if (! lOk)
    {
        // User confirmed: then perform a 'Ok' action instead of 'Cancel'
        accept();
    }
    else
    {
        done(0);
    }
}

///////////////////////////////////////////////////////////
// User has changed filter: update its data structure
///////////////////////////////////////////////////////////
void FilterDialog::accept()
{
    QString lSite, lIsControlsGroup, lParts, lRange, lTemperature, lSampleGroup;

    lParts = ProcessPartsItems[comboProcess->currentIndex()];
    if(editProcess->isEnabled() == true)
    {
        lRange = editProcess->text();
        // Replace comma with semi-colon
        lRange = lRange.replace(',',';');
        // Remove leading \r & \n if any
        lRange.replace("\r","");
        lRange.replace("\n","");
    }
    else
        lRange = "";

    if(comboSites->currentIndex() == 0)
        lSite = "All";
    else
        lSite = editSites->text();

    // Add one leading space to ease reading in list!
    lSite  += " ";
    lParts += " ";

    // Get temperature and extract numerical value from it.
    lTemperature = editTemperature->text();
    lTemperature = lTemperature.trimmed();

    // check if the GroupName edit box is visible and has to be saved...
    if(mIsGroupNameEditable==true)
    {
        // Get group name...save it back to structure...unless multiple-files selected!
        mGroupName    = LineEditGroupName->text();
        if((mSelectedItems == 1) && mTreeWidgetItemFileFilter)
            mTreeWidgetItemFileFilter->setText(mColumnOffset,mGroupName);

        // Now that we have saved the group, point to the site/parts/range/file name structure!
        mColumnOffset++;
    }

    if(mTreeWidgetItemFileFilter)
    {

        mTreeWidgetItemFileFilter->setText(mColumnOffset,lSite);
        mTreeWidgetItemFileFilter->setText(mColumnOffset + 1,lParts);
        mTreeWidgetItemFileFilter->setText(mColumnOffset + 2,lRange);
        mTreeWidgetItemFileFilter->setText(mColumnOffset + 3,lTemperature);
    }

    // If Data samples edited...make sure we use the new input file
    if((mIsSamplesEdited == true) && mTreeWidgetItemFileFilter)
        mTreeWidgetItemFileFilter->setText(mColumnOffset + 4,mFile);

    // Tests mapped?
    mMapTests = LineEditMapTests->text().trimmed();
    // Clean input file name strings in case they come from drag & drop (start with string 'file:///')
    if(mMapTests.startsWith("file:///"))
        mMapTests = QUrl(mMapTests).toLocalFile();

    // Remove leading \r & \n if any
    mMapTests.replace("\r","");
    mMapTests.replace("\n","");

    if(mTreeWidgetItemFileFilter)
        mTreeWidgetItemFileFilter->setText(mColumnOffset + 5,mMapTests);

    if (mIsGroupTypeEditable)
    {
        lIsControlsGroup = comboGroupType->itemData(
                    comboGroupType->currentIndex(),
                    Qt::DisplayRole).toString();
        if ((lIsControlsGroup != "Yes") && (lIsControlsGroup != "No"))
        {
            lIsControlsGroup = "";
            GSLOG(SYSLOG_SEV_ERROR, QString("invalid file type!").toLatin1().constData());
        }
        if (lIsControlsGroup == "Yes") // retrieve sample group if any
        {
            lSampleGroup = comboBoxSampleGroup->itemData(
                        comboBoxSampleGroup->currentIndex(),
                        Qt::DisplayRole).toString();
            if (!lSampleGroup.startsWith("-- select", Qt::CaseInsensitive))
            {
                lIsControlsGroup += " - " + lSampleGroup;
            }
        }
        if(mTreeWidgetItemFileFilter)
            mTreeWidgetItemFileFilter->setText(mColumnOffset + 7,lIsControlsGroup);
        ++mColumnOffset;
    }

    done(1);
}

///////////////////////////////////////////////////////////
// Enable/Disable List of binnings-parts to process
///////////////////////////////////////////////////////////
void FilterDialog::OnComboProcessChange(void)
{
    int iComboPosition=comboProcess->currentIndex();

    if( !((iComboPosition==GEX_PROCESSPART_PARTLIST)||(iComboPosition==GEX_PROCESSPART_EXPARTLIST)) )
    {
        SetWaferIdFilter(QString());
    }

    switch(iComboPosition)
    {
    case GEX_PROCESSPART_ALL:		// Process ALL Bins
    case GEX_PROCESSPART_GOOD:		// Process Good bins only
    case GEX_PROCESSPART_FAIL:		// Process Fail bins only
    case GEX_PROCESSPART_ODD:		// Process Odd parts only (1,3,5...)
    case GEX_PROCESSPART_EVEN:		// Process Odd parts only (2,4,6...)
    case GEX_PROCESSPART_NO_SAMPLES: // Only use bins (ignore samples)
        editProcess->setEnabled(false);
        PickParts->hide();
        break;

    case GEX_PROCESSPART_PARTLIST:	// Process list of parts.
    case GEX_PROCESSPART_EXPARTLIST:// All parts except...
        editProcess->setEnabled(true);
        editProcess->setFocus();
        PickParts->show();
        // Reset signal handler
        PickParts->disconnect();
        connect( PickParts, SIGNAL( clicked() ), this, SLOT( OnPickParts() ) );
        break;

    case GEX_PROCESSPART_SBINLIST:	 // Process list of Soft binnings
    case GEX_PROCESSPART_EXSBINLIST: // All Soft bins except...
        editProcess->setEnabled(true);
        editProcess->setFocus();
        PickParts->show();
        // Reset signal handler
        PickParts->disconnect();
        connect( PickParts, SIGNAL( clicked() ), this, SLOT( OnPickSoftBins() ) );
        break;

    case GEX_PROCESSPART_HBINLIST:	 // Process list of Hard binnings
    case GEX_PROCESSPART_EXHBINLIST: // All Hard bins except...
        editProcess->setEnabled(true);
        editProcess->setFocus();
        PickParts->show();
        // Reset signal handler
        PickParts->disconnect();
        connect( PickParts, SIGNAL( clicked() ), this, SLOT( OnPickHardBins() ) );
        break;

    case GEX_PROCESSPART_PARTSINSIDE:	// Parts inside...
    case GEX_PROCESSPART_PARTSOUTSIDE:	// Parts outside...
        editProcess->setEnabled(true);
        editProcess->setFocus();
        PickParts->show();
        // Reset signal handler
        PickParts->disconnect();
        connect( PickParts, SIGNAL( clicked() ), this, SLOT( OnPickCoord() ) );
        break;

    case GEX_PROCESSPART_FIRSTINSTANCE:		// Process first test instance
    case GEX_PROCESSPART_LASTINSTANCE:		// Process first test instance
        editProcess->setEnabled(false);
        PickParts->hide();
        break;
    }

    // If no file selected, always disable this button
    if(mTreeWidgetItemFileFilter == NULL)
        PickParts->hide();
}

///////////////////////////////////////////////////////////
// Enable/Disable List of Sites to process
///////////////////////////////////////////////////////////
void FilterDialog::OnComboSitesChange(void)
{
    int iComboPosition=comboSites->currentIndex();
    switch(iComboPosition)
    {
    case GEX_PROCESSSITE_ALL:		// Process all sites
        editSites->setEnabled(false);
        break;
    case GEX_PROCESSSITE_SITELIST:	// Process specific sites
        editSites->setEnabled(true);
        editSites->setFocus();
        break;
    }
}

void FilterDialog::OnComboGroupTypeChange()
{
    QString lIsControlsGroup = comboGroupType->itemData(
                comboGroupType->currentIndex(),
                Qt::DisplayRole).toString();
    bool lShowSampleSelect = (lIsControlsGroup == "Yes") ? true : false;

    labelSampleGroup->setVisible(lShowSampleSelect);
    comboBoxSampleGroup->setVisible(lShowSampleSelect);
}

///////////////////////////////////////////////////////////
// Select a .CSV Test Number mapping file...
///////////////////////////////////////////////////////////
void FilterDialog::OnMapTests()
{
    QString fn;

    // User wants to select a Test# mapping file...
    fn = QFileDialog::getOpenFileName(this, "Select dataset config File",
                           "", "Dataset config file (*.csv *.txt *.xml)");

    if(fn.isEmpty())
        return;

    LineEditMapTests->setText(fn);
}

///////////////////////////////////////////////////////////
// Create an empty a .CSV Test Number mapping file...
///////////////////////////////////////////////////////////
void FilterDialog::OnCreateMapTests()
{
    QString	strFilePath;

    // Current Map file in the Edit field...
    strFilePath = LineEditMapTests->text();
    strFilePath = strFilePath.trimmed();
    if(strFilePath.isEmpty() == true)
    {
        // No file assigned...use template.
        strFilePath = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
        strFilePath += "/samples/gex_dataset_config.xml";
    }

    if (ReportOptions.GetOption("preferences","use_os_default").toBool() && QDesktopServices::openUrl(QUrl(QString("file:///%1").arg(strFilePath)))) //Case 4619 - Use OS's default program to open file
      return;

#ifdef _WIN32
    // If file includes a space, we need to batch it between " "
    if(strFilePath.indexOf(' ') != -1)
    {
        strFilePath = "\"" + strFilePath;
        strFilePath = strFilePath + "\"";
    }
    ShellExecuteA(NULL,
           "open",
        ReportOptions.GetOption("preferences", "text_editor").toString().toLatin1().data(), //  ReportOptions.strTextEditor.toLatin1().constData(),
           strFilePath.toLatin1().constData(),
           NULL,
           SW_SHOWNORMAL);
#endif
#if defined unix || __MACH__
    char	szString[2048];
    sprintf(szString,"%s %s&",
        ReportOptions.GetOption("preferences", "text_editor").toString().toLatin1().constData(),	//ReportOptions.strTextEditor.toLatin1().constData(),
        strFilePath.toLatin1().constData());
    if (system(szString) == -1) {
      //FIXME: send error
    }
#endif
}


///////////////////////////////////////////////////////////
// Show a list of dataset names that are meaningful (eg:lotID,...)
///////////////////////////////////////////////////////////
void FilterDialog::OnSuggestDatasetName()
{
    PickNameDialog cPickName;

    // Empty suggestion list
    cPickName.clear();

    // Fill the list with suggestions.
    QTreeWidgetItem * pTreeWidgetItem = treeWidget->topLevelItem(0);

    while(pTreeWidgetItem != NULL)
    {
        // Fill suggestion list
        cPickName.insert(pTreeWidgetItem->text(0), pTreeWidgetItem->text(1));

        // Move to next item.
        pTreeWidgetItem = treeWidget->itemBelow(pTreeWidgetItem);
    };

    // Prompt dialog box, let user pick suggested name from the list
    if(cPickName.exec() != 1)
        return;	// User 'Abort'

    // Save the list item selected into the edit field...
    QString strDatasetName = LineEditGroupName->text();
    if(cPickName.isAppendMode())
        strDatasetName += QString(" ") + cPickName.nameList();
    else
        strDatasetName = cPickName.nameList();

    LineEditGroupName->setText(strDatasetName);
}

///////////////////////////////////////////////////////////
// Tells if split file over each site#
///////////////////////////////////////////////////////////
bool FilterDialog::SpitValidSites(void)
{
    if(comboMultiSites->currentIndex() == 0)
        return false;	// No split, keep all sites merged (default)
    return true;	// Split each site.
}

///////////////////////////////////////////////////////////
// Build the list of valid sites
///////////////////////////////////////////////////////////
bool FilterDialog::GetValidSites(QString strDataFile, bool bQuietMode /*=true*/)
{
    // 30/082011, pyc, case 5088
    QString strDataProcessingStdfCompliancy = ReportOptions.GetOption(QString("dataprocessing"), QString("stdf_compliancy")).toString();
    QList<int> qlFlexibleSitesList;

    // Define STDF file to read
    if(strDataFile.isEmpty() == false)
        mStdfFile = strDataFile;	// File to read given in argument

    // Reset Sites list
    mSitesList.clear();

    std::vector<int> lSite;
    std::vector<int> lSiteList;
    if (GS::Gex::StdfContentUtils::GetSites((std::string)(mStdfFile.toLatin1().constData()), lSite, lSiteList))
    {
        qlFlexibleSitesList = QList<int>::fromVector( QVector<int>::fromStdVector(lSite));
        mSitesList = QList<int>::fromVector( QVector<int>::fromStdVector(lSiteList));
        if ((mSitesList.count() != qlFlexibleSitesList.count()))
        {
            if( (strDataProcessingStdfCompliancy == QString("stringent")) && (!bQuietMode) )
            {
                QString strMsgBoxMessage;
                strMsgBoxMessage = QString("Valid sites list would be different if you change the option 'Data processing', 'Handling STDF compliancy issues' to 'flexible' mode");
                strMsgBoxMessage += QString("\n\nDo you want to change the option now ?");
                bool lOk;
                GS::Gex::Message::request("", strMsgBoxMessage, lOk);
                if (lOk)
                {
                    strDataProcessingStdfCompliancy = QString("flexible");
                    /// TODO : GSLOG msg
                    GS::Gex::OptionsHandler optionsHandler(GS::Gex::Engine::GetInstance().GetOptionsHandler());

                    optionsHandler.SetOption(QString("dataprocessing"), QString("stdf_compliancy"), strDataProcessingStdfCompliancy);
                    GS::Gex::Engine::GetInstance().SetOptionsMap(optionsHandler.GetOptionsMap());

                    ReportOptions.SetOption(QString("dataprocessing"), QString("stdf_compliancy"), strDataProcessingStdfCompliancy);
                }
            }

            if ( strDataProcessingStdfCompliancy == QString("flexible") ) // could have just been changed by user
            {
                mSitesList = qlFlexibleSitesList;
            }
        }

        return true;
    }

    return false;
}

// Case 3935:
QString FilterDialog::GetWaferIdFilter() const
{return mWaferIdToFilter;}

// Case 3935:
void FilterDialog::SetWaferIdFilter(QString strWaferIDFilter)
{
    mWaferIdToFilter = strWaferIDFilter;
    mUi_qlPtrWaferIDDisplayLabel->setText(mWaferIdToFilter);
    // Update GUI
    if(mWaferIdToFilter.isEmpty())
        mUi_qfPtrWaferDisplayFrame->hide();
    else
        mUi_qfPtrWaferDisplayFrame->show();
}

///////////////////////////////////////////////////////////
// Show a list of PartIDs found in the file, their Bin and Run#
///////////////////////////////////////////////////////////
void FilterDialog::OnPickParts()
{
    PickPartDialog		cPickPart;

    if(cPickPart.setFile(mStdfFile) == false)
        return;

    // case 3935
    if(!mWaferIdToFilter.isEmpty())
        cPickPart.OnWaferIdSelectionChanged(mWaferIdToFilter);

    if(cPickPart.exec() != 1)
        return;

    // Get list of parts selected, and insert it to the Part list edit field.
    editProcess->setText(cPickPart.getPartsList());
    SetWaferIdFilter(cPickPart.GetCurrentWaferId());
}

///////////////////////////////////////////////////////////
// Show a list of Softbins
///////////////////////////////////////////////////////////
void FilterDialog::OnPickSoftBins()
{
    PickBinDialog		cPickBins;

    if(cPickBins.setFile(mStdfFile,true) == false)
        return;

    if(cPickBins.exec() != 1)
        return;

    // Get list of soft bins selected, and insert it to the edit field.
    editProcess->setText(cPickBins.getBinsList());
}

///////////////////////////////////////////////////////////
// Show a list of Hardbins
///////////////////////////////////////////////////////////
void FilterDialog::OnPickHardBins()
{
    PickBinDialog		cPickBins;

    if(cPickBins.setFile(mStdfFile,false) == false)
        return;

    if(cPickBins.exec() != 1)
        return;

    // Get list of soft bins selected, and insert it to the edit field.
    editProcess->setText(cPickBins.getBinsList());
}

///////////////////////////////////////////////////////////
// Choose coord min and max to filter on
///////////////////////////////////////////////////////////
void FilterDialog::OnPickCoord()
{
    PickCoordDialog		cPickCoord;

    if(cPickCoord.setFile(mStdfFile))
    {
        // Set the coordinates already entered
        cPickCoord.setCoordinates(editProcess->text());

        if(cPickCoord.exec() == 1)
            // Get coordinates selected, and insert it to the edit field.
            editProcess->setText(cPickCoord.coordinates());
    }
}
