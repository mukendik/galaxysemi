#include <QColorDialog>

#include "browser_dialog.h"
#include "pickserie_dialog.h"
#include "report_build.h"
#include "db_onequery_wizard.h"
#include "message.h"

// in main.cpp
extern GexMainwindow *	pGexMainWindow;

// in classes.cpp
extern bool SetCurrentComboItem(QComboBox *pCombo, const QString & strItem, bool bInsertIfMissing=false);

//////////////////////////////////////////////////////////
// Constructor
PickSerieDialog::PickSerieDialog(QTreeWidget* qtwPtrSQLYieldWizardSeriesView, QWidget* parent, bool modal, Qt::WindowFlags f )
    : QDialog( parent, f )
{
    setupUi(this);
    setModal(modal);

    // Apply Examinator palette
    GexMainwindow::applyPalette(this);

    QObject::connect(PushButtonOk,					SIGNAL(clicked()),		this, SLOT(OnButtonOK()));
    QObject::connect(PushButtonCancel,				SIGNAL(clicked()),		this, SLOT(reject()));
    QObject::connect(comboSQL_Serie_Binnings,		SIGNAL(activated(int)), this, SLOT(OnBinType()));
    QObject::connect(buttonSQL_Serie_PickBinList,	SIGNAL(clicked()),		this, SLOT(OnPickBinList()));

    // Make sure label will not be wrapped
    labelSQL_Serie->setAlignment((Qt::Alignment)(Qt::AlignTop | Qt::AlignLeft | Qt::TextSingleLine));

    // Init list view pointer
    m_qtwPtrSeriesView = qtwPtrSQLYieldWizardSeriesView;

    // Set default serie name to "Serie <serie nb>" if new serie
    if(qtwPtrSQLYieldWizardSeriesView)
        editSQL_Serie_Name->setText("Serie " + QString::number(qtwPtrSQLYieldWizardSeriesView->topLevelItemCount()+1));

    // Make sure 'Serie Name' edit has the focus and select all text
    editSQL_Serie_Name->setFocus();
    editSQL_Serie_Name->selectAll();

    // Set other control defaults
    SetCurrentComboItem(comboSQL_Serie_Binnings, "PASS Bins only");
    SetCurrentComboItem(comboSQL_Serie_DataLabels, "Top");
    SetCurrentComboItem(comboSQL_Serie_LineSpots, "Circle");
    SetCurrentComboItem(comboSQL_Serie_TableData, "Yield & Volume");

    // Init variables
    m_WorkOverSOFT_BIN = true;	// Work over SOFT bins

    // Set some validators
    m_pYieldLimitValidator = new QDoubleValidator(0.0, 100.0, 2, this);
    editSQL_Serie_YieldThreshold->setValidator(m_pYieldLimitValidator);

    // Ensure GUI shown/hidden fields is accurate
    RefreshGuiFields();
}

///////////////////////////////////////////////////////////
// Ensure all fields are hidden/shown as expected
///////////////////////////////////////////////////////////
void PickSerieDialog::RefreshGuiFields(void)
{
    OnBinType();
}

///////////////////////////////////////////////////////////
// Define if working over SoftBIN or HardBIN
///////////////////////////////////////////////////////////
void PickSerieDialog::setBinningType(bool bSoftBin)
{
    m_WorkOverSOFT_BIN = bSoftBin;
}

///////////////////////////////////////////////////////////
// User selecting bin type to plot (All bins, PASS only, FAIL only, or bin list...)
///////////////////////////////////////////////////////////
void PickSerieDialog::OnBinType(void)
{
    switch(comboSQL_Serie_Binnings->currentIndex())
    {
        case 0:	// ALL bins
        case 1:	// PASS only bins
        case 2:	// FAIL only bins
            editSQL_Serie_Binlist->hide();
            buttonSQL_Serie_PickBinList->hide();
            break;
        case 3:	// Specific bins...
            editSQL_Serie_Binlist->show();
            buttonSQL_Serie_PickBinList->show();
            // If dialog box is visible, then set focus to edit field.
            if(editSQL_Serie_Name->isVisible())
                editSQL_Serie_Binlist->setFocus();
            break;
    }
}

///////////////////////////////////////////////////////////
// User clicked 'bin list' icon picker: display bin list available.
///////////////////////////////////////////////////////////
void PickSerieDialog::OnPickBinList(void)
{
    QString strBinningList;
    strBinningList = pGexMainWindow->pWizardOneQuery->SqlPickBinningList(false, m_WorkOverSOFT_BIN);

    if(!strBinningList.isEmpty())
        editSQL_Serie_Binlist->setText(strBinningList);
}

///////////////////////////////////////////////////////////
// Load GUI fields
///////////////////////////////////////////////////////////
void PickSerieDialog::setFields(CPickSerieFields &cFields)
{
    // Title
    editSQL_Serie_Name->setText(cFields.strTitle);

    // Binning type & list
    SetCurrentComboItem(comboSQL_Serie_Binnings, cFields.strBinType);
    editSQL_Serie_Binlist->setText(cFields.strBinList);

    // Plotting options
    groupSQL_Serie_PlotSerie->setChecked(cFields.bPlotSerie);
    SetCurrentComboItem(comboSQL_Serie_Style, cFields.strPlotStyle);
    buttonSQL_Serie_Color->setActiveColor(cFields.cSerieColor);
    SetCurrentComboItem(comboSQL_Serie_DataLabels, cFields.strDataLabels);
    SetCurrentComboItem(comboSQL_Serie_LineStyle, cFields.strLineStyle);
    SetCurrentComboItem(comboSQL_Serie_LineSpots, cFields.strLineSpots);
    SetCurrentComboItem(comboSQL_Serie_LineProperty, cFields.strLineProperty);

    // Binning pareto over yield alarm
    groupSQL_Serie_ParetoOverException->setChecked(cFields.bBinningParetoOverException);
    if(cFields.strAlarmType.isEmpty() == false)
        SetCurrentComboItem(comboSQL_Serie_YieldException, cFields.strAlarmType);	// 'Yield less than...','Yield Greater than...'
    QString strYieldLimit;
    if(cFields.fAlarmLimit < 0)
        cFields.fAlarmLimit = 100.0;	// If no alarm level defined yet, enter 100%
    strYieldLimit.sprintf("%.2f", cFields.fAlarmLimit);
    editSQL_Serie_YieldThreshold->setText(strYieldLimit);	// number in 0-100

    // Table options
    SetCurrentComboItem(comboSQL_Serie_TableData, cFields.strTableData);

    // Ensure GUI shown/hidden fields is accurate
    RefreshGuiFields();
}

///////////////////////////////////////////////////////////
// Dump/Save GUI fields
///////////////////////////////////////////////////////////
void PickSerieDialog::getFields(CPickSerieFields &cFields)
{
    // Title
    cFields.strTitle = editSQL_Serie_Name->text();

    // Binning type & list
    cFields.strBinType = comboSQL_Serie_Binnings->currentText();
    if(comboSQL_Serie_Binnings->currentIndex() == 3)
        cFields.strBinList = editSQL_Serie_Binlist->text();
    else
        cFields.strBinList = "";

    // Plotting options
    cFields.bPlotSerie = groupSQL_Serie_PlotSerie->isChecked();
    cFields.strPlotStyle = comboSQL_Serie_Style->currentText();
    cFields.cSerieColor = buttonSQL_Serie_Color->activeColor();
    cFields.strDataLabels = comboSQL_Serie_DataLabels->currentText();
    cFields.strLineStyle = comboSQL_Serie_LineStyle->currentText();
    cFields.strLineSpots = comboSQL_Serie_LineSpots->currentText();
    cFields.strLineProperty = comboSQL_Serie_LineProperty->currentText();

    // Binning Pareto enabled?
    cFields.bBinningParetoOverException = groupSQL_Serie_ParetoOverException->isChecked();
    if(cFields.bBinningParetoOverException)
    {
        cFields.strAlarmType = comboSQL_Serie_YieldException->currentText();
        QString strYieldLimit = editSQL_Serie_YieldThreshold->text();
        if(strYieldLimit.isEmpty())
            strYieldLimit = "-1";
        sscanf(strYieldLimit.toLatin1().constData(),"%f", &cFields.fAlarmLimit);
        if(cFields.fAlarmLimit < 0)
            cFields.fAlarmLimit = 0;
        if(cFields.fAlarmLimit > 100)
            cFields.fAlarmLimit = 100;
    }
    else
    {
        cFields.strAlarmType = "";
        cFields.fAlarmLimit = -1;
    }

    // Table options
    cFields.strTableData = comboSQL_Serie_TableData->currentText();
}

///////////////////////////////////////////////////////////
// User clicked the OK button: do some validation.
///////////////////////////////////////////////////////////
void PickSerieDialog::OnButtonOK()
{
    // Check if the serie name is not already in use (if new serie)
    if(m_qtwPtrSeriesView)
    {
        QTreeWidgetItemIterator	qtwiiSeriesIterator(m_qtwPtrSeriesView);
        while(*qtwiiSeriesIterator)
        {
            if((*qtwiiSeriesIterator)->text(0).toLower() == editSQL_Serie_Name->text().toLower())
            {
                QString strMessage = "A serie with the same name has already be defined.\nPlease modify the serie name.";
                GS::Gex::Message::warning("Serie", strMessage);
                return;
            }

            // Move to next item in listview
            ++qtwiiSeriesIterator;
        };
    }

    accept();
}

