#include <QMessageBox>
#include <gqtl_log.h>
#include "gex_database_entry.h"
#include "db_engine.h"
#include "gex_shared.h"
#include <libgexpb.h>
#include "admin_gui.h"
#include "browser_dialog.h"
#include "pickproduct_idsql_dialog.h"
#include "picktest_dialog.h"
#include "pickbin_single_dialog.h"
#include "statistical_monitoring_manual_tests_widget.h"
#include "ui_statistical_monitoring_settings_widget.h"
#include "statistical_monitoring_settings_widget.h"
#include "sm_filters.h"
#include "statistical_monitoring_taskdata.h"
#include "message.h"

extern GexScriptEngine*	pGexScriptEngine;
extern GexMainwindow* pGexMainWindow;

namespace GS
{
namespace Gex
{

StatisticalMonitoringSettingsWidget::StatisticalMonitoringSettingsWidget(QWidget *parent) :
    QWidget(parent),
    mUI(new Ui::StatisticalMonitoringSettingsWidget),
    mFrameLayout(0),
    mFilters(0),
    mButtonShowHide(0),
    mSettingsPB(0),
    mButtonProperties(0),
    mCacheData(0),
    mSettingsChanged(false)
{
    mUI->setupUi(this);
    mNewTests.clear();
}

StatisticalMonitoringSettingsWidget::~StatisticalMonitoringSettingsWidget()
{
    delete mUI;
}


bool StatisticalMonitoringSettingsWidget::initializeUI(const int databaseType, bool aIsNewTask)
{
    // Apply Examinator palette
    GexMainwindow::applyPalette(this);

    // Load list of databases
    pGexMainWindow->pWizardAdminGui->ListDatabasesInCombo(mUI->comboTDR, databaseType, DB_TDR_YM_PROD);

    mUI->comboTDR->setEnabled( aIsNewTask );
    if(aIsNewTask)
        onDatabaseChanged();

    ConnectEditSignals();

    // Create property browser and load controls from XML definition file
    if(!mSettingsPB)
    {
        mSettingsPB = loadSpecializedPropertyBrowserFromXML();
        if(!mSettingsPB)
            return false;
    }
    mSettingsPB->SlotSetExpanded(true);

    mButtonProperties = new CollapsibleButton( "Properties", T_Informative, 11 );
    mButtonProperties->addWidget(mSettingsPB);
    mUI->frameProperties->layout()->addWidget(mButtonProperties);
    mUI->frameProperties->layout()->setAlignment(Qt::AlignTop);

    mUI->frameProperties->layout()->setContentsMargins(0, 0, 0, 0);

    // Rule Name has focus
    mUI->editRuleName->setFocus();

    QIcon lIcon;
    lIcon.addPixmap(QPixmap(QString::fromUtf8(":/gex/icons/spm_rules.png")));
    QAction* lCreateManual = new QAction(lIcon, "Save and create manual limits", this );
    connect(lCreateManual, SIGNAL(triggered()), this, SLOT(onCreateManualLimitsClicked()), Qt::UniqueConnection);

    connect( mUI->toolButtonSave, SIGNAL(clicked(bool)), this, SLOT(onComputeClicked()), Qt::UniqueConnection);

    mUI->toolButtonSave->addAction(lCreateManual);

    mUI->comboConsoType->addItem("Raw data", QVariant(GEXDB_CONSO_TYPE_RAW));
    mUI->comboConsoType->addItem("Consolidated data", QVariant(GEXDB_CONSO_TYPE_CONSO));
    mUI->comboConsoLevel->addItem("Test insertion", QVariant(GEXDB_CONSO_LEVEL_INSERT));
    mUI->comboConsoLevel->addItem("Test flow", QVariant(GEXDB_CONSO_LEVEL_FLOW));

    initializeSpecializedUI();

    initFiltersUI();

    return true;
}

void StatisticalMonitoringSettingsWidget::initFiltersUI()
{
    if(!mButtonShowHide)
    {
        mButtonShowHide = new CollapsibleButton( "Filters", T_Informative, 11 );
        mButtonShowHide->close();
        mUI->frameFilters->layout()->addWidget( mButtonShowHide );
        mUI->frameFilters->layout()->setAlignment(Qt::AlignTop);
    }

    // get all filter key from DB if db is correctly set-up
    QString db_name = mUI->comboTDR->currentText();
    QStringList filter_keys;

    GexDatabaseEntry* db_entry = NULL;
    if(! db_name.isEmpty())
    {
        // GCORE-14130: non connected DB are also useful
        db_entry=Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(db_name, false);

        if( ! db_entry )
        {
            GSLOG( SYSLOG_SEV_ERROR, "FATAL : unable to get the database entry" );
            return;
        }
        db_entry->m_pExternalDatabase->GetConsolidatedLabelFilterChoices(mUI->comboTestingStage->currentText(), filter_keys);
    }

    if(!mFilters)
    {
        mFilters = new smFilters( NULL, filter_keys );
        mButtonShowHide->addWidget(mFilters);
    }
    else
    {
        mFilters->set_filter_keys(filter_keys);
    }

    if(db_entry && (db_entry->StatusFlags() & STATUS_CONNECTED))
    {
        mFilters->setEnabled(true);
        mFilters->setToolTip("");
    }
    else
    {
        mFilters->setEnabled(false);
        mFilters->setToolTip("TDR not connected");
    }


    mFilters->set_database_name( db_name );
    mFilters->set_testing_stage( mUI->comboTestingStage->currentText() );
    mFilters->set_products( mUI->editProducts->text() );

    QObject::connect( mFilters, SIGNAL(reset_requested()),
                      this, SLOT(onResetFilters()),Qt::UniqueConnection);
}

void StatisticalMonitoringSettingsWidget::onResetFilters()
{
    bool lReset;
    GS::Gex::Message::request("Reset task filters",
                              "This will reset all filters set for this task.\nConfirm ?", lReset);
    if (! lReset)
    {
        return;
    }

    mFilters->reset_filter_lines();
}

void StatisticalMonitoringSettingsWidget::onDatabaseChanged()
{
    // Get database name
    QString	lDBName = getCurrentDBName();
    if( lDBName.isEmpty() )
    {
        GSLOG( SYSLOG_SEV_ERROR, "FATAL : unable to get the database name" );
        return;
    }

    // GCORE-14130: non connected DB are also useful
    GexDatabaseEntry* lDBEntry = Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(lDBName, false);

    if(!lDBEntry)
    {
        GSLOG( SYSLOG_SEV_ERROR, "FATAL : unable to get the database entry" );
        return;
    }

    checkDatabasesConsistency( lDBEntry );

    QStringList lTS;
    if(lDBEntry->m_pExternalDatabase && lDBEntry->m_pExternalDatabase->IsTestingStagesSupported())
        lDBEntry->m_pExternalDatabase->GetSupportedTestingStages(lTS);

    QString lCurrentTS = lTS.isEmpty() ? "None" : lTS.first();
    if (mCacheData) lCurrentTS = mCacheData->GetAttribute(C_TESTINGSTAGE).toString();

    mUI->comboTestingStage->clear();
    mUI->comboTestingStage->addItems(lTS);
    // Select previous testing stage if still part of the list
    if(lTS.contains(lCurrentTS))
        mUI->comboTestingStage->setCurrentIndex(mUI->comboTestingStage->findText(lCurrentTS));

    // We could clear only if the new DB doesn't have the selected products and parameters
    mUI->editProducts->clear();
    mUI->editItemNums->clear();
    mUI->editFlow->clear();
    mUI->editInsertion->clear();

    if( mFilters != NULL )
    {
        QStringList filter_keys;
        lDBEntry->m_pExternalDatabase->GetConsolidatedLabelFilterChoices(mUI->comboTestingStage->currentText(), filter_keys);

        mFilters->set_filter_keys(filter_keys);
        mFilters->set_database_name(mUI->comboTDR->currentText());
        mFilters->reset_filter_lines();
    }

    onSettingsChanged();
}

void StatisticalMonitoringSettingsWidget::createManualLimitsClicked(const QString& labelNumber, const QString& labelName )
{
    StatisticalMonitoringManualTestsWidget lManualLimits(labelNumber, labelName);
    int lRes =lManualLimits.exec();
    if( lRes == QDialog::Accepted)
    {
        lManualLimits.GetNewElements(mNewTests);
        DumpFields(mCacheData);
        emit sCreateLimitsClicked();
    }
}

QList<QPair<int, QString> >& StatisticalMonitoringSettingsWidget::GetNewTests()
{
    return mNewTests;
}

QString StatisticalMonitoringSettingsWidget::getCurrentDBName()
{
   return mUI->comboTDR->currentText();

}

bool StatisticalMonitoringSettingsWidget::loadFields(GexMoStatisticalMonitoringTaskData* taskData)
{
    if(!taskData || !mSettingsPB)
    {
        return false;
    }

    mCacheData = taskData;

    DisconnectEditSignals();

    QVariant lValue;

    // Title/Rule name
    lValue = taskData->GetAttribute(C_TITLE);
    if(!lValue.isNull())
        mUI->editRuleName->setText(lValue.toString());

    // Database
    lValue = taskData->GetAttribute(C_DATABASE);
    if(!lValue.isNull())
    {
        int lIndex = mUI->comboTDR->findText(lValue.toString());
        if(lIndex != -1)
        {
            mUI->comboTDR->setCurrentIndex(lIndex);
            onDatabaseChanged();
        }
    }

    // Testing stage
    lValue = taskData->GetAttribute(C_TESTINGSTAGE);
    if(!lValue.isNull())
    {
        int lIndex = mUI->comboTestingStage->findText(lValue.toString());
        if(lIndex != -1)
        {
            mUI->comboTestingStage->setCurrentIndex(lIndex);
        }
    }

    // Products RegExp
    lValue = taskData->GetAttribute(C_PRODUCTREGEXP);
    if(!lValue.isNull())
    {
        mUI->editProducts->setText(lValue.toString());
        if(mFilters)
        {
            mFilters->set_products(lValue.toString());
        }
    }

    // Item type
    lValue = taskData->GetAttribute(C_MONITOREDITEMTYPE);
    if(!lValue.isNull())
    {
        int lIndex = mUI->comboItemType->findData(QVariant(lValue.toString()));
        if(lIndex != -1)
            mUI->comboItemType->setCurrentIndex(lIndex);
    }

    // Test flow
    lValue = taskData->GetAttribute(C_TESTFLOW);
    if(!lValue.isNull())
    {
        mUI->editFlow->setText(lValue.toString());
    }

    // Consolidation type
    lValue = taskData->GetAttribute(C_CONSOLIDATION_TYPE);
    if(!lValue.isNull())
    {
        int lIndex = mUI->comboConsoType->findData(QVariant(lValue.toString()));
        if(lIndex != -1)
            mUI->comboConsoType->setCurrentIndex(lIndex);
    }

    // Consolidation level
    lValue = taskData->GetAttribute(C_CONSOLIDATION_LEVEL);
    if(!lValue.isNull())
    {
        int lIndex = mUI->comboConsoLevel->findData(QVariant(lValue.toString()));
        if(lIndex != -1)
            mUI->comboConsoLevel->setCurrentIndex(lIndex);
    }

    mUI->editInsertion->setEnabled(lValue == GEXDB_CONSO_LEVEL_INSERT);
    mUI->buttonPickInsertion->setEnabled(lValue == GEXDB_CONSO_LEVEL_INSERT);

    // Consolidation name RegExp
    lValue = taskData->GetAttribute(C_INSERTION);
    if(!lValue.isNull())
        mUI->editInsertion->setText(lValue.toString());

    // Item RegExp
    lValue = taskData->GetAttribute(C_MONITOREDITEMREGEXP);
    if(!lValue.isNull())
    {
        mUI->editItemNums->setText(lValue.toString());
    }

    // filters
    initFiltersUI();

    if (taskData->filtersMetaData.isEmpty() == false)
        mButtonShowHide->open();

    mFilters->load_from( taskData->filtersMetaData, true );

    // Property Browser fields
    QMap<QString, int>::const_iterator i;
    for(i = mSettingKeyToPropID.constBegin(); i != mSettingKeyToPropID.constEnd(); ++i)
    {
        lValue = taskData->GetAttribute(i.key());
        if(!lValue.isNull())
            mSettingsPB->SetValue(i.value(), lValue);
    }

    lValue = taskData->GetAttribute(C_VERSION_COMPUTEFROM);
    if(!lValue.isNull())
    {
        mUI->dateEditFrom->setDate(lValue.toDateTime().date());
    }

    lValue = taskData->GetAttribute(C_VERSION_COMPUTETO);
    if(!lValue.isNull())
    {
        mUI->dateEditTo->setDate(lValue.toDateTime().date());
    }

    // Tell fields have been loaded
    onSettingsLoaded();

    ConnectEditSignals();

    return true;
}

bool StatisticalMonitoringSettingsWidget::DumpFields(GexMoStatisticalMonitoringTaskData * taskData)
{

    if(taskData == 0)
        return false;

    QVariant lValue;

    // saves filters if any
    if( mFilters )
      mFilters->copy_filters_in( taskData->filtersMetaData );

    // Title/Rule name
    taskData->UpdateAttribute(C_TITLE, mUI->editRuleName->text());
    // Database
    taskData->UpdateAttribute(C_DATABASE, mUI->comboTDR->currentText());
    // Testing stage
    taskData->UpdateAttribute(C_TESTINGSTAGE, mUI->comboTestingStage->currentText());
    // Products RegExp
    taskData->UpdateAttribute(C_PRODUCTREGEXP, mUI->editProducts->text());
    // Item type
    taskData->UpdateAttribute(C_MONITOREDITEMTYPE, mUI->comboItemType->currentData());
    // Item RegExp
    taskData->UpdateAttribute(C_MONITOREDITEMREGEXP, mUI->editItemNums->text());
    // test flow
    taskData->UpdateAttribute(C_TESTFLOW, mUI->editFlow->text());
    // Consolidation type
    taskData->UpdateAttribute(C_CONSOLIDATION_TYPE, mUI->comboConsoType->currentData());
    // Consolidation level
    taskData->UpdateAttribute(C_CONSOLIDATION_LEVEL, mUI->comboConsoLevel->currentData());
    // Consolidation name RegExp
    taskData->UpdateAttribute(C_INSERTION, mUI->editInsertion->text());

    // Property Browser fields
    QMap<QString, int>::const_iterator i;
    for(i = mSettingKeyToPropID.constBegin(); i != mSettingKeyToPropID.constEnd(); ++i)
    {
        lValue = mSettingsPB->GetCurrentValue(i.value());
        if(!lValue.isNull())
            taskData->UpdateAttribute(i.key(), lValue);
    }

    // Compute from
    taskData->UpdateAttribute(C_VERSION_COMPUTEFROM, mUI->dateEditFrom->dateTime());
    // Compute to
    taskData->UpdateAttribute(C_VERSION_COMPUTETO, mUI->dateEditTo->dateTime());

    return true;
}

void StatisticalMonitoringSettingsWidget::ConnectEditSignals()
{
    // Signal/slots
    connect(mUI->editRuleName, SIGNAL(textChanged(const QString&)),
                     this, SLOT(onSettingsChanged()), Qt::UniqueConnection);

    connect(mUI->comboTDR, SIGNAL(currentIndexChanged(const QString &)),
                     this, SLOT(onDatabaseChanged()));

    connect(mUI->comboTestingStage, SIGNAL(currentIndexChanged(const QString &)),
                     this, SLOT(onTestingStageChanged()),Qt::UniqueConnection);
    connect(mUI->buttonPickProduct, SIGNAL(clicked()),
                     this, SLOT(onPickProducts()),Qt::UniqueConnection);
    connect(mUI->editProducts, SIGNAL(textChanged(const QString &)),
                     this, SLOT(onProductsChanged()),Qt::UniqueConnection);
    connect(mUI->comboItemType, SIGNAL(currentIndexChanged(const QString &)),
                     this, SLOT(onItemTypeChanged()),Qt::UniqueConnection);
    connect(mUI->buttonPickFlow, SIGNAL(clicked()),
                     this, SLOT(onPickFlow()),Qt::UniqueConnection);
    connect(mUI->editFlow, SIGNAL(textChanged(const QString &)),
                     this, SLOT(onFlowChanged()),Qt::UniqueConnection);
    connect(mUI->comboConsoType, SIGNAL(currentIndexChanged(const QString &)),
                     this, SLOT(onConsoTypeLevelChanged()),Qt::UniqueConnection);
    connect(mUI->comboConsoLevel, SIGNAL(currentIndexChanged(const QString &)),
                     this, SLOT(onConsoTypeLevelChanged()),Qt::UniqueConnection);
    connect(mUI->buttonPickInsertion, SIGNAL(clicked()),
                     this, SLOT(onPickInsertion()),Qt::UniqueConnection);
    connect(mUI->editInsertion, SIGNAL(textChanged(const QString &)),
                     this, SLOT(onSettingsChanged()),Qt::UniqueConnection);
    connect(mUI->buttonPickTests, SIGNAL(clicked()),
                     this, SLOT(onPickItems()),Qt::UniqueConnection);
    connect(mUI->editItemNums, SIGNAL(textChanged(const QString &)),
                     this, SLOT(onSettingsChanged()),Qt::UniqueConnection);
    connect(mSettingsPB, SIGNAL(SignalGPbPropertyValueChanged(int,const QVariant&)),
                     this, SLOT(onPropertyChanged(int,const QVariant&)), Qt::UniqueConnection);
    connect(mUI->dateEditFrom, SIGNAL(dateChanged(QDate)),
                     this, SLOT(onSettingsChanged()), Qt::UniqueConnection);
    connect(mUI->dateEditTo, SIGNAL(dateChanged(QDate)), this,
                     SLOT(onSettingsChanged()), Qt::UniqueConnection);
    connect(mUI->pushButtonSave, SIGNAL(clicked()),
                     this, SLOT(onSaveClicked()), Qt::UniqueConnection);
    connect(mUI->pushButtonDiscard, SIGNAL(clicked()),
                     this, SLOT(onDiscardClicked()), Qt::UniqueConnection);
}

void StatisticalMonitoringSettingsWidget::DisconnectEditSignals()
{
    // Signal/slots
    disconnect(mUI->editRuleName, SIGNAL(textChanged(const QString&)),
                     this, SLOT(onSettingsChanged()));

    disconnect(mUI->comboTDR, SIGNAL(currentIndexChanged(const QString &)),
                     this, SLOT(onDatabaseChanged()));

    disconnect(mUI->comboTestingStage, SIGNAL(currentIndexChanged(const QString &)),
                     this, SLOT(onTestingStageChanged()));
    disconnect(mUI->buttonPickProduct, SIGNAL(clicked()),
                     this, SLOT(onPickProducts()));
    disconnect(mUI->editProducts, SIGNAL(textChanged(const QString &)),
                     this, SLOT(onProductsChanged()));
    disconnect(mUI->comboItemType, SIGNAL(currentIndexChanged(const QString &)),
                     this, SLOT(onItemTypeChanged()));
    disconnect(mUI->buttonPickTests, SIGNAL(clicked()),
                     this, SLOT(onPickItems()));
    disconnect(mUI->editItemNums, SIGNAL(textChanged(const QString &)),
                     this, SLOT(onSettingsChanged()));
    disconnect(mUI->buttonPickFlow, SIGNAL(clicked()),
                     this, SLOT(onPickFlow()));
    disconnect(mUI->editFlow, SIGNAL(textChanged(const QString &)),
                     this, SLOT(onSettingsChanged()));
    disconnect(mUI->comboConsoType, SIGNAL(currentIndexChanged(const QString &)),
                     this, SLOT(onConsoTypeLevelChanged()));
    disconnect(mUI->comboConsoLevel, SIGNAL(currentIndexChanged(const QString &)),
                     this, SLOT(onConsoTypeLevelChanged()));
    disconnect(mUI->buttonPickInsertion, SIGNAL(clicked()),
                     this, SLOT(onPickConsoName()));
    disconnect(mUI->editInsertion, SIGNAL(textChanged(const QString &)),
                     this, SLOT(onSettingsChanged()));
    disconnect(mSettingsPB, SIGNAL(SignalGPbPropertyValueChanged(int,const QVariant&)),
                     this, SLOT(onPropertyChanged(int,const QVariant&)));
    disconnect(mUI->dateEditFrom, SIGNAL(dateChanged(QDate)),
                     this, SLOT(onSettingsChanged()));
    disconnect(mUI->dateEditTo, SIGNAL(dateChanged(QDate)), this,
                     SLOT(onSettingsChanged()));
    disconnect(mUI->pushButtonSave, SIGNAL(clicked()),
                     this, SLOT(onSaveClicked()));
    disconnect(mUI->pushButtonDiscard, SIGNAL(clicked()),
                     this, SLOT(onDiscardClicked()));
}

void StatisticalMonitoringSettingsWidget::onTestingStageChanged()
{
    // We could clear only if the new DB doesn't have the selected products and parameters
    mUI->editProducts->clear();
    mUI->editItemNums->clear();
    mUI->editFlow->clear();
    mUI->editInsertion->clear();
    if( mFilters != NULL )
    {
        QString db_name = mUI->comboTDR->currentText();
        QStringList filter_keys;

        if(! db_name.isEmpty())
        {
          // GCORE-14130: non connected DB are also useful
          GexDatabaseEntry* db_entry=
            Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry
            (db_name, false);

          if( ! db_entry )
          {
            GSLOG( SYSLOG_SEV_ERROR, "FATAL : unable to get the database entry" );
            return;
          }

          db_entry->m_pExternalDatabase->GetConsolidatedLabelFilterChoices
            (mUI->comboTestingStage->currentText(), filter_keys);
        }

        mFilters->set_filter_keys(filter_keys);
        mFilters->set_testing_stage( mUI->comboTestingStage->currentText() );
        mFilters->reset_filter_lines();
    }

    if(mCacheData)
    {
        mCacheData->ResetDefault(mUI->comboTestingStage->currentText());

        loadFields(mCacheData);
    }

    onSettingsChanged();
}

void StatisticalMonitoringSettingsWidget::onProductsChanged()
{
    mUI->editItemNums->clear();
    mUI->editFlow->clear();
    mUI->editInsertion->clear();
    if( mFilters != NULL )
    {
        mFilters->set_products( mUI->editProducts->text() );
        mFilters->reset_filter_lines();
    }
    onSettingsChanged();
}

void StatisticalMonitoringSettingsWidget::onItemTypeChanged()
{
    mUI->editFlow->clear();
    mUI->editInsertion->clear();
    mUI->editItemNums->clear();
    onSettingsChanged();
}

void StatisticalMonitoringSettingsWidget::onFlowChanged()
{
    mUI->editInsertion->clear();
    mUI->editItemNums->clear();
    onSettingsChanged();
}

void StatisticalMonitoringSettingsWidget::onConsoTypeLevelChanged()
{
    mUI->editInsertion->clear();
    mUI->editInsertion->setEnabled(mUI->comboConsoLevel->currentData().toString() == GEXDB_CONSO_LEVEL_INSERT);
    mUI->buttonPickInsertion->setEnabled(mUI->comboConsoLevel->currentData().toString() == GEXDB_CONSO_LEVEL_INSERT);
    onSettingsChanged();
}

void StatisticalMonitoringSettingsWidget::onSettingsLoaded()
{
    // Settings are not considered changed after load from TaskData
    mSettingsChanged = false;
    emit sSettingsLoaded();
}


void StatisticalMonitoringSettingsWidget::onSettingsChanged()
{
    mSettingsChanged = true;
    emit sSettingsChanged();
}

bool StatisticalMonitoringSettingsWidget::onSaveSettings()
{
    mSettingsChanged = false;
    return true;
}

void StatisticalMonitoringSettingsWidget::onSaveClicked()
{
    DumpFields(mCacheData);
    emit sSaveClicked();
}

void StatisticalMonitoringSettingsWidget::onComputeClicked()
{
    DumpFields(mCacheData);
    emit sComputeClicked();
}

void StatisticalMonitoringSettingsWidget::onDiscardClicked()
{
    if(mCacheData)
    {
        loadFields(mCacheData);
    }
}


CGexPB* StatisticalMonitoringSettingsWidget::loadPropertyBrowserFromXML(QString type,
                                                                        QString firstElement,
                                                                        QString fileName)
{
    // Create PB
    //CGexPB* lPB = CGexPB::CreatePropertyBrowser(pGexScriptEngine, this, 40);
    //if (!lPB)
        //return 0;
    CGexPB*	lPB = new CGexPB(this, pGexScriptEngine, 40);


    // Set minimum margins
    lPB->setContentsMargins(0,0,0,0);

    // Openn settings XML definition file
    QFile oFile(fileName);
    if(!oFile.open(QIODevice::ReadOnly))
    {
        GSLOG(SYSLOG_SEV_ERROR,
              QString("Could not open %1 settings definition XML").arg(type).toLatin1().constData());
        delete lPB; CGexPB::DestroyPropertyBrowser(lPB);
        return 0;
    }

    // Load XML
    QDomDocument lDoc(firstElement);
    QString errorMsg; int errorLine=0;
    if (!lDoc.setContent(&oFile, &errorMsg, &errorLine))
    {
         GSLOG(SYSLOG_SEV_WARNING,
               QString("%1 settings definition XML not compliant at line %2 : %3").arg(type).
               arg(errorLine).arg(errorMsg).toLatin1().constData());
         //CGexPB::DestroyPropertyBrowser(lPB);
          delete lPB;
         oFile.close();
         return 0;
    }

    // Find firstElement element
    QDomElement lSettings = lDoc.firstChildElement(firstElement);
    if (lSettings.isNull())
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Cannot find at least one %1 "
                                  "element in %2 settings definition XML!").
              arg(firstElement).arg(type).toLatin1().constData());
        //CGexPB::DestroyPropertyBrowser(lPB);
         delete lPB;
        oFile.close();
        return 0;
    }

    // Go through groups and settings
    QDomElement lGroup;
    for(lGroup = lSettings.firstChildElement("group"); !lGroup.isNull();
        lGroup = lGroup.nextSiblingElement("group"))
    {
        QMap<QString, QString> lAttributes;
        lAttributes.insert("type", "Group");
        for (int i=0; i<lGroup.attributes().size(); i++)
            lAttributes.insert(lGroup.attributes().item(i).nodeName(),
                               lGroup.attributes().item(i).nodeValue());
        int lGroupID = lPB->AddProperty(lAttributes, -1);
        if(lGroupID == -1)
        {
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("%1 settings: cannot add group \"%2\" to Property Browser")
                  .arg(type).arg(lAttributes.value("label", "")).toLatin1().constData());
            //CGexPB::DestroyPropertyBrowser(lPB);
             delete lPB;
            oFile.close();
            return 0;
        }
        QDomElement lSetting;
        QString     lNodeName, lNodeValue;
        for(lSetting = lGroup.firstChildElement("setting"); !lSetting.isNull();
            lSetting = lSetting.nextSiblingElement("setting"))
        {
            lAttributes.clear();
            for (int i=0; i<lSetting.attributes().size(); i++)
            {
                lNodeName = lSetting.attributes().item(i).nodeName();
                lNodeValue = lSetting.attributes().item(i).nodeValue();
                rawReplace(lNodeName, lNodeValue);
                lAttributes.insert(lNodeName, lNodeValue);
            }
            int lSettingID = lPB->AddProperty(lAttributes, lGroupID);
            if(lSettingID == -1)
            {
                GSLOG(SYSLOG_SEV_ERROR, QString("%1 settings: cannot add property \"%2\" to Property Browser")
                      .arg(type).arg(lAttributes.value("label", "")).toLatin1().constData());
                //CGexPB::DestroyPropertyBrowser(lPB);
                 delete lPB;
                oFile.close();
                return 0;
            }

            if(lAttributes.contains("key"))
                mSettingKeyToPropID.insert(lAttributes.value("key"), lSettingID);
        }
    }

    // Close XML and return PB
    oFile.close();
    return lPB;
}

// This function will check if all manatory fields are populated
bool StatisticalMonitoringSettingsWidget::onCheckMandatoryFields()
{
    if(mUI->editRuleName->text().isEmpty())
    {
        QMessageBox::warning(this, "Statistical Monitoring editor",
                             QString("Please select a task name before saving your task."),
                             QMessageBox::Ok);
        mUI->editRuleName->setFocus();
        return false;
    }

    if(mUI->comboTDR->currentText().isEmpty())
    {
        QMessageBox::warning(this, "Statistical Monitoring editor",
                             QString("Please select a TDR database before saving your task."),
                             QMessageBox::Ok);
        mUI->comboTDR->setFocus();
        return false;
    }
    // Check if the TDR is compatible for an update
    // Get database pointer
    // GCORE-14130: non connected DB are also useful
    GexDatabaseEntry* lDBEntry = Engine::GetInstance().GetDatabaseEngine(). FindDatabaseEntry(
                mUI->comboTDR->currentText(), false);
    if(lDBEntry && lDBEntry->m_pExternalDatabase)
    {
        bool            bDbUpToDate=false;
        QString         lErrorMessage;
        QString         lCurrentVersionName, lLastSupportedVersionName;
        unsigned int    lCurrentVersionNumber, lLastSupportedVersionNumber;
        if (!lDBEntry->m_pExternalDatabase->IsDbUpToDateForInsertion(&bDbUpToDate,
                                                                     lCurrentVersionName,
                                                                     &lCurrentVersionNumber,
                                                                     lLastSupportedVersionName,
                                                                     &lLastSupportedVersionNumber))
            lErrorMessage="Error : impossible to retrieve database version !";
        if (!bDbUpToDate)
            lErrorMessage="Your database is NOT uptodate. Please update your database to a newer version.";
        if(!bDbUpToDate)
        {
            QMessageBox::warning(this, "",
                    QString("%1\n\nYour changes will not be saved.").arg(lErrorMessage),
                    QMessageBox::Ok);
            return false;
        }
    }

    if(mUI->comboTestingStage->currentText().isEmpty())
    {
        QMessageBox::warning(this, "Statistical Monitoring editor",
                             QString("Please select a testing stage before saving your task."),
                             QMessageBox::Ok);
        mUI->comboTestingStage->setFocus();
        return false;
    }

    if(mUI->editProducts->text().isEmpty())
    {
        QMessageBox::warning(this, "Statistical Monitoring editor",
                             QString("Please select one or more products before saving your task."),
                             QMessageBox::Ok);
        mUI->editProducts->setFocus();
        return false;
    }

    return true;
}

void StatisticalMonitoringSettingsWidget::onPropertyChanged(int settingID, const QVariant &newValue)
{
    Q_UNUSED(settingID)
    Q_UNUSED(newValue)
    onSettingsChanged();
}

void StatisticalMonitoringSettingsWidget::pickProducts(QStringList &productList)
{
     // Fill Filter list with relevant strings
    PickProductIdSQLDialog lPickFilter;
    lPickFilter.fillList(productList, true, false);

    // Prompt dialog box, let user pick Filter string from the list
    productList.clear();
    if(lPickFilter.exec() != 1)
        return;	// User 'Abort'

    // Set selected list
    lPickFilter.getProductList(productList);

    // If a product has been selected, display it
    if(productList.count() > 0)
    {
        // Set ProductID
        mUI->editProducts->setText(productList.join(","));
        mUI->editProducts->setFocus();
    }

    mUI->editItemNums->setText("");
}

void StatisticalMonitoringSettingsWidget::pickTests(QStringList &testList)
{
    // Fill Dialog box with parameter list...
    // Show TestList
    PickTestDialog lPickTest;

    // Allow Multiple selections.
    lPickTest.setMultipleSelection(true);

    // Check if List was successfuly loaded
    if (lPickTest.fillParameterList(testList))
    {
        // Prompt dialog box, let user pick tests from the list
        if(lPickTest.exec() == QDialog::Accepted)
        {
            // Save the list selected into the edit field...
            mUI->editItemNums->setText(lPickTest.testList());
        }
    }
}

void StatisticalMonitoringSettingsWidget::pickFlow( QStringList &flowList)
{
    // Fill Dialog box with parameter list...
    // Show TestList
    PickProductIdSQLDialog lPickFilter;
    lPickFilter.fillList(flowList, true, false);

    flowList.clear();
    if(lPickFilter.exec() != 1)
        return;	// User 'Abort'

    // Set selected list
    lPickFilter.getProductList(flowList);

    // If a retest phase has been selected, display it
    if(flowList.count() > 0)
    {
        // Set ProductID
        mUI->editFlow->setText(flowList.join(","));
        mUI->editFlow->setFocus();
    }
}

void StatisticalMonitoringSettingsWidget::pickInsertion( QStringList &insertionList)
{
    // Fill Dialog box with parameter list...
    // Show TestList
    PickProductIdSQLDialog lPickFilter;
    lPickFilter.fillList(insertionList, true, false);

    insertionList.clear();
    if(lPickFilter.exec() != 1)
        return;	// User 'Abort'

    // Set selected list
    lPickFilter.getProductList(insertionList);

    // If a retest phase has been selected, display it
    if(insertionList.count() > 0)
    {
        // Set ProductID
        mUI->editInsertion->setText(insertionList.join(","));
        mUI->editInsertion->setFocus();
    }
}

void StatisticalMonitoringSettingsWidget::pickBins(QStringList &binList)
{
    // Fill Filter list with relevant strings
    PickBinSingleDialog dPickFilter;
    dPickFilter.fillList(binList);
    dPickFilter.setMultipleSelection(true);

    // Prompt dialog box, let user pick Filter string from the list
    if(dPickFilter.exec() != 1)
        return;	// User 'Abort'

    // Return the list selected (with '|' replaced by ',', which is more appropriate for a binlist)
    mUI->editItemNums->setText(dPickFilter.getBinsList().replace('|',','));
    mUI->editItemNums->setFocus();
}

} // namespace Gex
} // namespace GS
