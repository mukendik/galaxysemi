#ifdef GCORE15334
#include "message.h"
#include "engine.h"
#include "gqtl_log.h"
#include "pat_info.h"
#include "pat_recipe.h"
#include "pat_global.h"
#include "gex_report.h"
#include "patman_lib.h"
#include "pat_recipe_io.h"
#include "pat_definition.h"
#include "gqtl_global.h"
#include "browser_dialog.h"
#include "picktest_dialog.h"
#include "pat_recipe_editor.h"
#include "pat_limits_dialog.h"
#include "pat_widget_ft.h"
#include "csl/csl_engine.h"
#include "gex_report.h"
#include "gex_scriptengine.h"
#include "gex_pixmap_extern.h"
#include "mv_pat_test_edition.h"
#include "temporary_files_manager.h"
#include "gex_pat_constants_extern.h"
#include "pat_recipe_historical_data_gui.h"
#include "filter_dialog.h"
#include "tb_pat_recipe_wizard_dialog.h"
#include "stats_engine.h"


#include <QMessageBox>
#include <QFileDialog>
#include <QStandardItemModel>
#include <QItemDelegate>
#include <QVariantList>
#include <QProgressDialog>
#include <QInputDialog>
#include <QDropEvent>
#include <QIntValidator>


extern GexMainwindow *      pGexMainWindow;
extern QString      patlib_GetDistributionName(int iDistributionType);
extern CGexReport* gexReport;
extern GexScriptEngine *pGexScriptEngine;
extern QPixmap *pixGexApplication;

bool CompareTest(CPatDefinition *test1, CPatDefinition *test2)
{
    if (test1->m_lTestNumber == test2->m_lTestNumber)
        return (test1->mPinIndex < test2->mPinIndex);

     return (test1->m_lTestNumber < test2->m_lTestNumber);
}

extern void ConvertToScriptString(QString &strFile);
extern void ConvertFromScriptString(QString &strFile);

#undef	GEX_TPAT_CONFIG_FIELD_SBIN
#undef	GEX_TPAT_CONFIG_FIELD_DBIN
#undef	GEX_TPAT_CONFIG_FIELD_TNUM
#undef	GEX_TPAT_CONFIG_FIELD_TNAME
#undef	GEX_TPAT_CONFIG_FIELD_LSL
#undef	GEX_TPAT_CONFIG_FIELD_USL
#undef	GEX_TPAT_CONFIG_FIELD_SHAPE
#undef	GEX_TPAT_CONFIG_FIELD_CPK
#undef	GEX_TPAT_CONFIG_FIELD_RMEAN
#undef	GEX_TPAT_CONFIG_FIELD_RSIGMA
#undef	GEX_TPAT_CONFIG_FIELD_MEAN
#undef	GEX_TPAT_CONFIG_FIELD_SIGMA
#undef	GEX_TPAT_CONFIG_FIELD_RANGE
#undef	GEX_TPAT_CONFIG_FIELD_IGNOREDATA
#undef	GEX_TPAT_CONFIG_FIELD_KEEP
#undef	GEX_TPAT_CONFIG_FIELD_OUTLIER_LIMIT_SET
#undef	GEX_TPAT_CONFIG_FIELD_RULE
#undef	GEX_TPAT_CONFIG_FIELD_NFACTOR
#undef	GEX_TPAT_CONFIG_FIELD_TFACTOR
#undef GEX_TPAT_CONFIG_FIELD_SPC_PAT_DRIFT
#undef GEX_TPAT_CONFIG_FIELD_SPC_PAT_DRIFT_UNITS
#undef GEX_TPAT_CONFIG_FIELD_SPC_TEST_DRIFT
#undef GEX_TPAT_CONFIG_FIELD_SPC_TEST_DRIFT_UNITS
#undef GEX_TPAT_CONFIG_FIELD_SPC_TEST_CPK
#undef GEX_TPAT_CONFIG_FIELD_LLM_SCALE
#undef GEX_TPAT_CONFIG_FIELD_HLM_SCALE
#undef	GEX_TPAT_CONFIG_FIELD_SBINRULE
#undef	GEX_TPAT_CONFIG_FIELD_SPAT_NFACTOR
#undef	GEX_TPAT_CONFIG_FIELD_SPAT_TFACTOR
#undef GEX_TPAT_CONFIG_FIELD_DPAT_COMMENT
#undef GEX_TPAT_CONFIG_FIELD_SPAT_COMMENT
#undef	GEX_TPAT_CONFIG_FIELD_NNR_RULE
#undef	GEX_TPAT_CONFIG_FIELD_TTYPE
#undef	GEX_TPAT_CONFIG_FIELD_PINMAP_IDX
#undef GEX_TPAT_CONFIG_TOTAL_FIELDS

namespace GS
{
namespace Gex
{

class PATRecipeEditorPrivate
{
public:
    PATRecipeEditorPrivate();
    ~PATRecipeEditorPrivate();
    int GetFieldColumn(const QString &field, PATRecipeEditor::PATSettings);
    int GetMVFieldColmun(const QString &field);
    void ScaleLimits(QString &strScaledLimit,double &lfValue,int iPower,char cSeparator,QString &strUnits);
    QString PATDefKey(CTest *test);
    void GetItemIdx(QComboBox *combo, QTableWidget *tree, int &row, int &column);
    PATMultiVariateRule GetRule(QTreeWidgetItem *item);
    PATRecipe   mPATRecipe;
    QString     mRecipeName;
    bool        mModified;
    QStringList mUVBasicColumn;
    QStringList mUVSPATColumn;
    QStringList mUVDPATColumn;
    QStringList mUVNNRColumn;
    QStringList mUVSPCColumn;
    QStringList mMVColumns;
    QStringList mUVColumns;
    GS::QtLib::NumberFormat mFormat;
    QMap<QComboBox *,CPatDefinition*> mPatDefMapping;
    QMap<QTreeWidgetItem *,PATMultiVariateRule *> mPatRuleMapping;
    QMap<QWidget *,PATMultiVariateRule *> mPatRuleEditorMapping;
    static int mRuleIdx;

    class RuleNameValidator : public QValidator
    {
    public:
        RuleNameValidator(QTreeWidget *widget, int columIdx,QObject * parent = 0);
        ~RuleNameValidator();

        State validate(QString &, int &) const;
    private:
        QTreeWidget *mTree;
        int mColIdx;
    };
};

int PATRecipeEditorPrivate::mRuleIdx = 0;
PATRecipeEditor *PATRecipeEditor::mInstance = NULL;

PATRecipeEditor &PATRecipeEditor::GetInstance()
{
    GSLOG(SYSLOG_SEV_DEBUG, "GetInstance ...");

    if(!mInstance)
    {
        mInstance = new PATRecipeEditor(0);
        if (pGexScriptEngine)
        {
            QScriptValue lSV = pGexScriptEngine->newQObject(mInstance);
            if (!lSV.isNull())
            {
                GSLOG(5, "Registering PAT Recipe editor into Script Engine...");
                pGexScriptEngine->globalObject().setProperty("GSPATRecipeEditor", lSV);
            }

        }
    }

    return  *mInstance;
}

bool PATRecipeEditor::IsInstantiated()
{
    return (mInstance != NULL);
}

void PATRecipeEditor::DestroyInstance()
{
    if(mInstance)
    {
        delete mInstance;
        mInstance = NULL;
    }
}

const QString &PATRecipeEditor::GetRecipeName() const
{
    return mPrivate->mRecipeName;
}

PATRecipeEditor::PATRecipeEditor(QWidget *parent) : QDialog(parent)
{
    GSLOG(SYSLOG_SEV_DEBUG, "PATRecipeEditor ...");
    Initialize();
}

PATRecipeEditor::~PATRecipeEditor()
{
    GSLOG(SYSLOG_SEV_DEBUG, "~PATRecipeEditor ...");
    Clear();

    if (mPrivate)
    {
        delete mPrivate;
        mPrivate = NULL;
    }
}

void PATRecipeEditor::DetachRecipeEditor()
{
    GSLOG(SYSLOG_SEV_DEBUG, "detachWindow ...");

    if(parent())
    {
        pGexMainWindow->pScrollArea->layout()->removeWidget(this);
        setParent(NULL, Qt::Dialog);
        setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
        setWindowTitle("PAT Recipe Editor");
        move(QPoint(100, 100));
        if(pGexMainWindow != NULL)
            pGexMainWindow->ShowHtmlBrowser();
        show();

    }
    else
    {
        setParent(pGexMainWindow->centralWidget(), Qt::Dialog);
        pGexMainWindow->pScrollArea->layout()->addWidget(this);
        show();
        pGexMainWindow->ShowWizardDialog(GEXTB_EDIT_PAT_WIZARD_P1);
    }

}

void PATRecipeEditor::ReadRecipe()
{
    GSLOG(SYSLOG_SEV_DEBUG, "loadRecipe ...");
    QString lRecipeName = QFileDialog::getOpenFileName(this,
                                                       "Review & Edit Outlier / PAT Configuration File",
                                                       "",
                                                       "Recipe file (*.csv *.json)");
    if(!lRecipeName.isEmpty())
    {
        RefreshEditor(lRecipeName);
    }
}

void PATRecipeEditor::RefreshEditor(const QString &recipeFileName)
{
    GSLOG(SYSLOG_SEV_DEBUG, "Refresh PAT recipe editor...");
    if(recipeFileName.isEmpty())
    {
        //GS::Gex::Message::information(windowTitle(),"Empty recipe file name provided");
        return ;
    }
    InitializeData();
    mPrivate->mRecipeName = recipeFileName;
    QSharedPointer<PATRecipeIO> lRecipeIO(PATRecipeIO::CreateRecipeIo(recipeFileName));
    if (lRecipeIO.isNull() == false)
    {
        lRecipeIO->SetRecipeName(recipeFileName);
        setDisabled(true);
        bool lStatus = lRecipeIO->Read(mPrivate->mPATRecipe);
        setDisabled(false);
        if (lStatus == false)
        {
            GS::Gex::Message::warning(windowTitle(),
                       QString("Failed to read recipe : %1\nLast error: %2")
                                .arg(recipeFileName).arg(lRecipeIO->GetErrorMessage())
                                .toLatin1().constData());
            GSLOG(SYSLOG_SEV_ERROR, QString("Failed to read recipe : %1").arg(recipeFileName).toLatin1().constData());
            return;
        }
    }
    else
    {
        GS::Gex::Message::warning(windowTitle(),
                                  QString("Failed to read recipe : %1").arg(recipeFileName).toLatin1().constData());
        GSLOG(SYSLOG_SEV_ERROR, QString("Failed to read recipe : %1").arg(recipeFileName).toLatin1().constData());
        return;
    }

    RefreshEditor();
    mPrivate->mModified = false;
    EnableEditorButton(true);
    GSLOG(SYSLOG_SEV_DEBUG, "Refresh PAT recipe editor done");
}

void PATRecipeEditor::RefreshEditor()
{
    GSLOG(SYSLOG_SEV_DEBUG, "Refresh ...");
    InitPATSettings(PATRecipeEditor::DPAT);
    int lRange = 0;
    if(mPrivate->mPATRecipe.GetOptions().GetRecipeType() == GS::Gex::PAT::RecipeWaferSort)
    {
        lRange = mPrivate->mPATRecipe.GetUniVariateRules().count()+mPrivate->mPATRecipe.GetMultiVariateRules().count();
        GS::Gex::Engine::GetInstance().UpdateProgressStatus(true,
                                                            lRange,
                                                            0,true);
        mRecipeData->setTabEnabled(mRecipeData->indexOf(mMultivariate), true);
        UpdateUVTab();
        UpdateMVTab();
        GS::Gex::Engine::GetInstance().UpdateProgressStatus(true,
                                                            0,
                                                            lRange,false);
    }
    else if(mPrivate->mPATRecipe.GetOptions().GetRecipeType() == GS::Gex::PAT::RecipeFinalTest)
    {
        lRange = mPrivate->mPATRecipe.GetUniVariateRules().count();
        GS::Gex::Engine::GetInstance().UpdateProgressStatus(true,
                                                            lRange,
                                                            0,true);
        mRecipeData->setTabEnabled(mRecipeData->indexOf(mMultivariate), false);
        UpdateUVTab();
        GS::Gex::Engine::GetInstance().UpdateProgressStatus(true,
                                                            0,
                                                            lRange,false);
    }
    else
    {
        GS::Gex::Message::warning(windowTitle(), QString("Unknown Recipe type").toLatin1().constData());
        GSLOG(SYSLOG_SEV_ERROR, QString("Unknown Recipe type").toLatin1().constData());
        return;
    }

    GSLOG(SYSLOG_SEV_DEBUG, "Refresh done.");
}

void PATRecipeEditor::addUVEntry(CPatDefinition *def, const QString &key,int idx)
{
    if(!def)
        return ;
    CPatDefinition *lDef = def;
    int lIdx = idx;
    QString lKey = key;

    if (mPrivate->mPATRecipe.GetOptions().bStaticPAT)
        addUVEntry("Static Bin",lIdx,Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled,PATRecipeEditor::Basic, (qlonglong)lDef->m_lFailStaticBin);
    else
        addUVEntry("Static Bin",lIdx,Qt::ItemIsSelectable|Qt::ItemIsEditable,PATRecipeEditor::Basic, (qlonglong)lDef->m_lFailStaticBin);

    if (mPrivate->mPATRecipe.GetOptions().bDynamicPAT)
        addUVEntry("Dyn. Bin"  ,lIdx,Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled,PATRecipeEditor::Basic, (qlonglong)lDef->m_lFailDynamicBin );
    else
        addUVEntry("Dyn. Bin"  ,lIdx,Qt::ItemIsSelectable|Qt::ItemIsEditable,PATRecipeEditor::Basic, (qlonglong)lDef->m_lFailDynamicBin );

    addUVEntry("Test#"     ,lIdx,Qt::ItemIsSelectable|Qt::ItemIsEnabled,PATRecipeEditor::Basic, (qulonglong)lDef->m_lTestNumber);
    QTableWidgetItem *lItem = mUnivariateTree->item(lIdx, mPrivate->GetFieldColumn("Test#",PATRecipeEditor::Basic));
    lItem->setData(Qt::UserRole,lKey);

    addUVEntry("Test Type" ,lIdx,Qt::ItemIsSelectable|Qt::ItemIsEnabled,PATRecipeEditor::Basic, QString(lDef->GetTestTypeLegacy()));
    addUVEntry("PinIndex"  ,lIdx,Qt::ItemIsSelectable|Qt::ItemIsEnabled,PATRecipeEditor::Basic, (qlonglong)lDef->mPinIndex);
    addUVEntry("Test name" ,lIdx,Qt::ItemIsSelectable|Qt::ItemIsEnabled,PATRecipeEditor::Basic, lDef->m_strTestName);

    if(lDef->m_lfLowLimit > -GEX_TPAT_FLOAT_INFINITE)
        addUVEntry("LL.", lIdx,Qt::ItemIsSelectable|Qt::ItemIsEnabled,PATRecipeEditor::Basic, QString("%1 %2").arg(mPrivate->mFormat.formatNumericValue(lDef->m_lfLowLimit, true)).arg(lDef->m_strUnits), NULL, NULL, QVariant(lDef->m_lfLowLimit));
    else
        addUVEntry("LL.", lIdx,Qt::ItemIsSelectable|Qt::ItemIsEnabled,PATRecipeEditor::Basic, QString(""));

    if(lDef->m_lfHighLimit < GEX_TPAT_FLOAT_INFINITE)
        addUVEntry("HL.", lIdx,Qt::ItemIsSelectable|Qt::ItemIsEnabled,PATRecipeEditor::Basic, QString("%1 %2").arg(mPrivate->mFormat.formatNumericValue(lDef->m_lfHighLimit, true)).arg(lDef->m_strUnits), NULL, NULL, QVariant(lDef->m_lfHighLimit));
    else
        addUVEntry("HL.", lIdx,Qt::ItemIsSelectable|Qt::ItemIsEnabled,PATRecipeEditor::Basic, QString(""));

    addUVEntry("SPAT rule"  ,lIdx, Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled , PATRecipeEditor::SPAT , lDef->m_SPATRuleType,gexSpatRuleSetItemsGUI,lDef);

    addUVEntry("N Factor"   ,lIdx, Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled , PATRecipeEditor::SPAT , lDef->m_lfSpatOutlierNFactor);
    addUVEntry("T Factor"   ,lIdx, Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled , PATRecipeEditor::SPAT , lDef->m_lfSpatOutlierTFactor);
    addUVEntry("SPAT Notes" ,lIdx, Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled , PATRecipeEditor::SPAT , lDef->mSPATNote);

    addUVEntry("Samples to ignore" , lIdx, Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled , PATRecipeEditor::DPAT, lDef->m_SamplesToIgnore,gexIgnoreSamplesSetItemsGUI,lDef);

    addUVEntry("Outliers to keep"  , lIdx, Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled , PATRecipeEditor::DPAT, lDef->m_OutliersToKeep,gexKeepOutliersSetItemsGUI,lDef);

    addUVEntry("Outlier Limits set", lIdx, Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled , PATRecipeEditor::DPAT, lDef->m_iOutlierLimitsSet,gexOutlierLimitsSetItemsLongNamesGUI,lDef);

    addUVEntry("DPAT Rule"         , lIdx, Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled , PATRecipeEditor::DPAT, lDef->mOutlierRule,gexRuleSetItemsGUI,lDef);

    if (lDef->m_lfOutlierNFactor != 0)
        addUVEntry("N Factor"          , lIdx, Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled , PATRecipeEditor::DPAT, lDef->m_lfOutlierNFactor);
    else
        addUVEntry("N Factor"          , lIdx, Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled , PATRecipeEditor::DPAT, QString(""));

    if (lDef->m_lfOutlierTFactor)
        addUVEntry("T Factor"          , lIdx, Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled , PATRecipeEditor::DPAT, lDef->m_lfOutlierTFactor);
    else
        addUVEntry("T Factor"          , lIdx, Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled , PATRecipeEditor::DPAT, QString(""));

    addUVEntry("Shape"             , lIdx, Qt::ItemIsSelectable|Qt::ItemIsEnabled, PATRecipeEditor::DPAT, patlib_GetDistributionName(lDef->m_iDistributionShape));
    addUVEntry("DPAT Notes"        , lIdx, Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled , PATRecipeEditor::DPAT, lDef->mDPATNote);

    addUVEntry("NNR Rule" ,lIdx, Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled , PATRecipeEditor::NNR , lDef->m_iNrrRule,gexNnrRuleSetItemsGUI,lDef);

    addUVEntry("PAT-Median Drift alarm",lIdx, Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled , PATRecipeEditor::SPC, lDef->m_SPC_PatMedianDriftAlarm);
    addUVEntry("PAT-Median Drift units"           ,lIdx, Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled , PATRecipeEditor::SPC, lDef->m_SPC_PatMedianDriftAlarmUnits,gexMedianDriftUnitsSetItemsGUI,lDef);

    addUVEntry("Test Mean Drift alarm" ,lIdx, Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled , PATRecipeEditor::SPC, lDef->m_SPC_TestMeanDriftAlarm);
    addUVEntry("Test Mean Drift units"           ,lIdx, Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled , PATRecipeEditor::SPC, lDef->m_SPC_TestMeanDriftAlarmUnits, gexMedianDriftUnitsSetItemsGUI,lDef);

    addUVEntry("Cpk alarm"             ,lIdx, Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled , PATRecipeEditor::SPC, lDef->m_SPC_CpkAlarm);

    // enabled only in smart & adaptive mode
    if (lDef->mOutlierRule == GEX_TPAT_RULETYPE_SMARTID)
    {
        addUVEntry("Tail management",
                   lIdx,
                   Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled ,
                   PATRecipeEditor::DPAT,
                   lDef->mTailMngtRuleType,
                   gexTailMngtRuleSetItemsGUI,
                   lDef);
    }
    else
    {
        addUVEntry("Tail management",
                   lIdx,
                   Qt::ItemIsSelectable|Qt::ItemIsEditable,
                   PATRecipeEditor::DPAT,
                   GEX_TPAT_TAIL_MNGT_DISABLED,gexTailMngtRuleSetItemsGUI,lDef);
    }

    mPrivate->mModified = true;
}

void PATRecipeEditor::addUVEntry(const QString &label, int row, Qt::ItemFlags flag, PATRecipeEditor::PATSettings setting,
                                 QVariant value, const char*comboItems[], CPatDefinition *patDef, QVariant data)
{
    if(comboItems)
    {
        QComboBox *lCombo = new QComboBox;
        int iIndex  = 0;
        int lCol    = mPrivate->GetFieldColumn(label,setting);

        while(comboItems[iIndex])
        {
           lCombo->addItem(QString(comboItems[iIndex++]));
        }
        lCombo->setCurrentIndex(value.toInt());
        mUnivariateTree->setCellWidget(row, lCol, lCombo);
        if(patDef)
        {
            mPrivate->mPatDefMapping.insert(lCombo,patDef);
            lCombo->setProperty("Column",   QVariant(lCol));
        }
        connect(lCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateUVData(int)));
        lCombo->setEnabled((flag & Qt::ItemIsEnabled) ? true : false);
    }
    else
    {
        QTableWidgetItem *lItem = new QTableWidgetItem();
        lItem->setText(value.toString());
        if(!data.isNull())
        {
            lItem->setData(Qt::UserRole, data);
        }
        lItem->setFlags(flag);
        mUnivariateTree->setItem(row, mPrivate->GetFieldColumn(label,setting), lItem);
    }
}

void PATRecipeEditorPrivate::GetItemIdx(QComboBox *combo, QTableWidget *tree, int &row, int &col)
{
    row = -1;
    col = -1;
    for(int lRow=0; lRow < tree->rowCount(); ++lRow)
    {
        for(int lCol=mUVBasicColumn.count(); lCol < tree->columnCount(); ++lCol)
        {
            QComboBox *lCombo = qobject_cast<QComboBox *> (tree->cellWidget(lRow,lCol));
            if(lCombo == combo)
            {
                row = lRow;
                col = lCol;
            }
        }
    }
}

void PATRecipeEditor::UpdateUVData(int /*idx*/)
{
    QComboBox *lCombo = qobject_cast<QComboBox *> (sender());

    if(lCombo)
    {
        int row     = mUnivariateTree->rowAt(lCombo->y());
        int column  = lCombo->property("Column").toInt();
        CPatDefinition *lDef = 0;
        if(mPrivate->mPatDefMapping.contains(lCombo))
            lDef =  mPrivate->mPatDefMapping[lCombo];
        else
            return;

        if(column ==  mPrivate->GetFieldColumn("DPAT Rule"  ,PATRecipeEditor::DPAT))
        {
            lDef->mOutlierRule = lCombo->currentIndex();

            // Refresh GUI, keep current Factors.
            UpdateDpatGuiFactors(lDef->mOutlierRule, row, true);
        }
        if(column ==  mPrivate->GetFieldColumn("Tail management"  ,PATRecipeEditor::DPAT))
        {
            lDef->mTailMngtRuleType = lCombo->currentIndex();
        }
        else if(column ==  mPrivate->GetFieldColumn("SPAT rule"  ,PATRecipeEditor::SPAT))
        {
            lDef->m_SPATRuleType = lCombo->currentIndex();

            // Refresh GUI, keep current Factors.
            UpdateSpatGuiFactors(lDef->m_SPATRuleType, row, true);
        }
        else if(column ==  mPrivate->GetFieldColumn("Samples to ignore" , PATRecipeEditor::DPAT))
        {
            lDef->m_SamplesToIgnore = lCombo->currentIndex();
        }
        else if(column ==  mPrivate->GetFieldColumn("Outliers to keep"  , PATRecipeEditor::DPAT))
        {
            lDef->m_OutliersToKeep = lCombo->currentIndex();
        }
        else if(column ==  mPrivate->GetFieldColumn("Outlier Limits set", PATRecipeEditor::DPAT))
        {
            lDef->m_iOutlierLimitsSet = lCombo->currentIndex();
        }
        else if(column ==  mPrivate->GetFieldColumn("NNR Rule" ,PATRecipeEditor::NNR))
        {
            lDef->m_iNrrRule = lCombo->currentIndex();
        }
        else if(column ==  mPrivate->GetFieldColumn("PAT-Median Drift units"           ,PATRecipeEditor::SPC))
        {
            lDef->m_SPC_PatMedianDriftAlarmUnits = lCombo->currentIndex();
        }
        else if(column ==  mPrivate->GetFieldColumn("Test Mean Drift units"           ,PATRecipeEditor::SPC))
        {
            lDef->m_SPC_TestMeanDriftAlarmUnits = lCombo->currentIndex();
        }

        mPrivate->mModified = true;
    }

}

void PATRecipeEditor::UpdateUVData( int row, int column)
{
    QTableWidgetItem *lItem = mUnivariateTree->item(row,column);
    if(lItem)
    {
        QTableWidgetItem *lPatDefKey = mUnivariateTree->item(row,mPrivate->GetFieldColumn("Test#",PATRecipeEditor::Basic));
        QString lKey = lPatDefKey->data(Qt::UserRole).toString();
        CPatDefinition *lDef = mPrivate->mPATRecipe.GetUniVariateRules().value(lKey);

        if(column ==  mPrivate->GetFieldColumn("N Factor"   ,PATRecipeEditor::SPAT))
        {
            // Get SPAT rule type (AEC, N*Sigma,....)
            double lfValue = lItem->text().toDouble();

            if((lDef->m_SPATRuleType == GEX_TPAT_SPAT_ALGO_SIGMA) && (lfValue < -15 || lfValue > 15))
            {
                lItem->setText("15");
                GS::Gex::Message::information("", "Maximum allowed factor is 15*Sigma!");
            }
            else if((lDef->m_SPATRuleType == GEX_TPAT_SPAT_ALGO_RANGE) && (lfValue < 0))
            {
                lItem->setText(QString::number(fabs(lfValue)));
                GS::Gex::Message::information("", "Range must be positive!");
            }
            else
                lDef->m_lfSpatOutlierNFactor = lfValue;
        }
        else if(column ==  mPrivate->GetFieldColumn("T Factor"   ,PATRecipeEditor::SPAT))
        {
            // Get SPAT rule type (AEC, N*Sigma,....)
            double lfValue = lItem->text().toDouble();

            if((lDef->m_SPATRuleType == GEX_TPAT_SPAT_ALGO_SIGMA) && (lfValue < -15 || lfValue > 15))
            {
                lItem->setText("15");
                GS::Gex::Message::information("", "Maximum allowed factor is 15*Sigma!");
            }
            else if((lDef->m_SPATRuleType == GEX_TPAT_SPAT_ALGO_RANGE) && (lfValue < 0))
            {
                lItem->setText(QString::number(fabs(lfValue)));
                GS::Gex::Message::information("", "Range must be positive!");
            }
            else
                lDef->m_lfSpatOutlierTFactor = lfValue;
        }
        else if(column ==  mPrivate->GetFieldColumn("SPAT Notes" ,PATRecipeEditor::SPAT))
        {
            lDef->mSPATNote = lItem->text();
        }
        else if(column ==  mPrivate->GetFieldColumn("DPAT Rule"         , PATRecipeEditor::DPAT))
        {
            lDef->mOutlierRule = lItem->text().toDouble();;
        }
        else if(column ==  mPrivate->GetFieldColumn("N Factor"          , PATRecipeEditor::DPAT))
        {
            lDef->m_lfOutlierNFactor = lItem->text().toDouble();

            // Check current DPAT rule selected...
            double lfValue = lItem->text().toDouble();
            if((lDef->mOutlierRule == GEX_TPAT_RULETYPE_RANGEID) && (lfValue < 0))
            {
                lItem->setText(QString::number(fabs(lfValue)));
                GS::Gex::Message::information("", "Range must be positive!");
            }
        }
        else if(column ==  mPrivate->GetFieldColumn("T Factor"          , PATRecipeEditor::DPAT))
        {
            lDef->m_lfOutlierTFactor = lItem->text().toDouble();
        }
        else if(column ==  mPrivate->GetFieldColumn("DPAT Notes"         ,PATRecipeEditor::DPAT))
        {
            lDef->mDPATNote = lItem->text();
        }
        else if(column ==  mPrivate->GetFieldColumn("PAT-Median Drift alarm",PATRecipeEditor::SPC))
        {
            lDef->m_SPC_PatMedianDriftAlarm = lItem->text().toDouble();
        }
        else if(column ==  mPrivate->GetFieldColumn("Test Mean Drift alarm" ,PATRecipeEditor::SPC))
        {
            lDef->m_SPC_TestMeanDriftAlarm = lItem->text().toDouble();
        }
        else if(column ==  mPrivate->GetFieldColumn("Cpk alarm"             ,PATRecipeEditor::SPC))
        {
            lDef->m_SPC_CpkAlarm = lItem->text().toDouble();
        }
        else if(column ==  mPrivate->GetFieldColumn("Static Bin", PATRecipeEditor::Basic))
        {
            lDef->m_lFailStaticBin = lItem->text().toInt();
        }
        else if(column ==  mPrivate->GetFieldColumn("Dyn. Bin" ,PATRecipeEditor::Basic))
        {
            lDef->m_lFailDynamicBin = lItem->text().toInt();
        }
        else if(column ==  mPrivate->GetFieldColumn("LL." ,PATRecipeEditor::Basic))
        {
            lDef->m_lfLowLimit = lItem->text().section(" ",0,0).toDouble();
        }
        else if(column ==  mPrivate->GetFieldColumn("HL." ,PATRecipeEditor::Basic))
        {
            lDef->m_lfHighLimit = lItem->text().section(" ",0,0).toDouble();
        }

        mPrivate->mModified = true;
    }
}

void PATRecipeEditor::UpdateUVTab()
{
    GSLOG(SYSLOG_SEV_DEBUG, "Update UniVariate Tab...");
    QHash<QString, CPatDefinition *> &lUniVariateRules = mPrivate->mPATRecipe.GetUniVariateRules();
    mUnivariateTree->setRowCount(lUniVariateRules.count());
    mUnivariateTree->setColumnCount(mPrivate->mUVColumns.count());

    mUnivariateTree->disconnect(this, SLOT(UpdateUVData(int, int)));

    int lIdx = 0;
    QList<CPatDefinition *> lPatDef = lUniVariateRules.values();
    qSort(lPatDef.begin(), lPatDef.end(), CompareTest);

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Analyzing %1 Pat Definitions...").arg(lPatDef.size()).toLatin1().data() );
    QList<CPatDefinition *>::Iterator lDefIterator = lPatDef.begin();
    QString lStatus;
    while(lDefIterator != lPatDef.end())
    {
        if (lIdx%10==0)
        {
            GS::Gex::Engine::GetInstance().UpdateLabelStatus(lStatus.sprintf("Analyzing PAT definition %d on %d...",
                                                             lIdx, lPatDef.size())
                                                             );
            GS::Gex::Engine::GetInstance().UpdateProgressStatus(false,
                                                                -1,
                                                                lIdx,true);
            qApp->processEvents(); // to avoid freezing
        }
        CPatDefinition *lDef = *lDefIterator;
        QString lKey = lUniVariateRules.key(lDef);
        addUVEntry(lDef, lKey, lIdx);
        ++lDefIterator;
        ++lIdx;
    }
    GS::Gex::Engine::GetInstance().UpdateLabelStatus("RecipeEditor UniVariate updated");
    mUnivariateTree->resizeColumnsToContents();
    SettingsPATTypeChanged(PATRecipeEditor::DPAT);
    mUnivariateTree->connect(mUnivariateTree, SIGNAL(cellChanged(int,int)), this, SLOT(UpdateUVData(int, int)));
    GSLOG(SYSLOG_SEV_DEBUG, "Update UniVariate Tab done.");
}

void PATRecipeEditor::UpdateMVTab()
{
    QList<PATMultiVariateRule> &lMultiVariateRules = mPrivate->mPATRecipe.GetMultiVariateRules();
#ifdef _USE_NEW_RECIPE_EDITOR
    if(lMultiVariateRules.isEmpty())
    {
        PATMultiVariateRule lMVRule;
        lMVRule.SetEnabled(true);
        lMVRule.SetBin(5);
        lMVRule.SetCustomDistance(5);
        lMVRule.SetPrincipalComponents(4);
        lMVRule.SetName("Rule 1");
        lMVRule.SetOutlierDistanceMode(PAT::Medium);
        lMVRule.SetRule(PATMultiVariateRule::Generated);
        lMVRule.AddTestData(PATMultiVariateRule::MVTestData("Test1",11111,33));
        lMVRule.AddTestData(PATMultiVariateRule::MVTestData("Test2",11111,35));
        lMVRule.AddTestData(PATMultiVariateRule::MVTestData("Test3",11113,3));
        lMVRule.AddTestData(PATMultiVariateRule::MVTestData("Test4",11114,4));
        lMultiVariateRules.append(lMVRule);

        lMVRule.SetEnabled(true);
        lMVRule.SetBin(5);
        lMVRule.SetCustomDistance(5);
        lMVRule.SetPrincipalComponents(4);
        lMVRule.SetName("Rule 1");
        lMVRule.SetOutlierDistanceMode(PAT::Medium);
        lMVRule.SetRule(PATMultiVariateRule::Manual);
        lMVRule.AddTestData(PATMultiVariateRule::MVTestData("Test1",11111,33));
        lMVRule.AddTestData(PATMultiVariateRule::MVTestData("Test2",11111,35));
        lMVRule.AddTestData(PATMultiVariateRule::MVTestData("Test3",11113,3));
        lMVRule.AddTestData(PATMultiVariateRule::MVTestData("Test4",11114,4));
        lMultiVariateRules.append(lMVRule);

    }

#endif
    for(int lIdx=0; lIdx<lMultiVariateRules.count(); ++lIdx )
    {
        GS::Gex::Engine::GetInstance().UpdateProgressStatus(false,
                                                            -1,
                                                            lIdx,true);

        if(lMultiVariateRules[lIdx].GetName().isEmpty())
            lMultiVariateRules[lIdx].SetName(QString("Noname-Rule%1").arg(mPrivate->mRuleIdx++));
        addMVEntry(lMultiVariateRules[lIdx]);
    }
    for (int lIdx=0; lIdx<mMultivariateTree->columnCount(); ++lIdx)
        mMultivariateTree->resizeColumnToContents(lIdx);
}

void PATRecipeEditor::AddNewRule()
{
    PATMultiVariateRule lMVRule;

    lMVRule.SetName(QString("Custom Rule# %1").arg(mPrivate->mRuleIdx++));

    if (mPrivate->mPATRecipe.GetOptions().GetMVPATEnabled())
        lMVRule.SetBin(mPrivate->mPATRecipe.GetOptions().GetMVPATSoftBin());

    mPrivate->mPATRecipe.GetMultiVariateRules().append(lMVRule);
    addMVEntry(mPrivate->mPATRecipe.GetMultiVariateRules().last());
    for (int lIdx=0; lIdx<mMultivariateTree->columnCount(); ++lIdx)
        mMultivariateTree->resizeColumnToContents(lIdx);

}

void PATRecipeEditor::DuplicateRule()
{
    QList<QTreeWidgetItem*> lSlected = mMultivariateTree->selectedItems();
    if(lSlected.count() == 1)
    {
        QTreeWidgetItem *lItem = lSlected.first();
        if(lItem->parent())
        {
            lItem = lItem->parent();
        }
        if(lItem && (mMultivariateTree->indexOfTopLevelItem(lItem) !=-1) && mPrivate->mPatRuleMapping.contains(lItem))
        {
            mPrivate->mPATRecipe.GetMultiVariateRules().append(PATMultiVariateRule (*(mPrivate->mPatRuleMapping[lItem])));
            mPrivate->mPATRecipe.GetMultiVariateRules().last().SetRule(PATMultiVariateRule::Manual);
            QString lName = mPrivate->mPATRecipe.GetMultiVariateRules().last().GetName()+" - copy";
            QString lSuffix = "";
            int lSuffixIdx = 0;
            for(int lIdx=0; lIdx<mMultivariateTree->topLevelItemCount();lIdx++)
            {
                QTreeWidgetItem *lItem = mMultivariateTree->topLevelItem(lIdx);
                QLineEdit *lEdit = qobject_cast<QLineEdit*> (mMultivariateTree->itemWidget(lItem, mPrivate->GetMVFieldColmun("Rule Name")));
                if(lEdit->text() == QString(lName+lSuffix))
                {
                    lSuffixIdx++;
                    lSuffix = QString::number(lSuffixIdx);

                }
            }
            mPrivate->mPATRecipe.GetMultiVariateRules().last().SetName(lName+lSuffix);
            addMVEntry(mPrivate->mPATRecipe.GetMultiVariateRules().last());
        }

    }
}

void PATRecipeEditor::DeleteRule()
{
    QList<QTreeWidgetItem*> lSlected = mMultivariateTree->selectedItems();
    if(lSlected.count() == 1)
    {
        QTreeWidgetItem *lItem = lSlected.first();
        if(lItem->parent())
        {
            lItem = lItem->parent();
        }
        int lIndex = mMultivariateTree->indexOfTopLevelItem(lItem);
        if(lIndex == -1)
            return;
        if(lItem && mPrivate->mPatRuleMapping.contains(lItem))
        {
            if(lItem->text(mPrivate->GetMVFieldColmun("Type")) != "Manual")
                return;
            PATMultiVariateRule lMVRule = *(mPrivate->mPatRuleMapping[lItem]);
            mPrivate->mPATRecipe.GetMultiVariateRules().removeAll(lMVRule);
            mPrivate->mPatRuleMapping.remove(lItem);
            delete lItem;
        }

    }
}

void PATRecipeEditor::EditTestList()
{
    QList<QTreeWidgetItem*> lSlected = mMultivariateTree->selectedItems();
    if(lSlected.count() == 1)
    {
        QTreeWidgetItem *lItem = lSlected.first();
        if(lItem->parent())
        {
            lItem = lItem->parent();
        }
        if(mMultivariateTree->indexOfTopLevelItem(lItem) == -1)
            return;
        if(lItem->text(mPrivate->GetMVFieldColmun("Type")) != "Manual")
            return;
        if(lItem && mPrivate->mPatRuleMapping.contains(lItem))
        {
            PATMultiVariateRule *lRule = mPrivate->mPatRuleMapping[lItem];
            //            if(lRule->GetMVTestData().isEmpty())
            //            {
            //                GS::Gex::Message::warning(windowTitle(), QString("No rule selected to be edited").toLatin1().constData());
            //            }
            //            else
            //            {
            GS::Gex::MVPATTestEdition lEdition(this, lRule, mPrivate->mPATRecipe.GetUniVariateRules());
            if(lEdition.exec() == QDialog::Accepted)
            {
                while(lItem->childCount())
                {
                    lItem->removeChild(lItem->child(0));
                }

                const QList<PATMultiVariateRule::MVTestData> &lTestData = lRule->GetMVTestData();

                for(int lIdx=0; lIdx<lTestData.count(); ++lIdx)
                {
                    QTreeWidgetItem *lItemTest = new QTreeWidgetItem();
                    lItemTest->setText(mPrivate->GetMVFieldColmun("Tests list"),
                                       QString("%1 - %2 - %3")
                                       .arg(lTestData[lIdx].GetTestNumber())
                                       .arg(lTestData[lIdx].GetTestName())
                                       .arg(lTestData[lIdx].GetPinIdx()));
                    lItem->addChild(lItemTest);
                }

                mPrivate->mModified = true;
            }
        }
    }

}
void PATRecipeEditor::RemoveTestFromRule(QTreeWidgetItem *lRuleItem, QTreeWidgetItem *lTesItem)
{
    if(lRuleItem && mPrivate->mPatRuleMapping.contains(lRuleItem) && lTesItem)
    {
        PATMultiVariateRule *lRule = mPrivate->mPatRuleMapping[lRuleItem];
        QStringList lTestProperty =  lTesItem->text(mPrivate->GetMVFieldColmun("Tests list")).split("-",QString::SkipEmptyParts);
        if(lTestProperty.count() == 3)
        {
            PATMultiVariateRule::MVTestData lData(lTestProperty[1],lTestProperty[0].toInt(),lTestProperty[2].toInt());
            lRule->RemoveAllTestData(lData);
            delete lTesItem;

            mPrivate->mModified = true;
        }
    }
}

void PATRecipeEditor::RuleNameChanged(const QString & )
{
    QWidget *lWidget = qobject_cast<QWidget *>(sender());
    if(lWidget && mPrivate->mPatRuleEditorMapping.contains(lWidget))
    {
        QLineEdit *lCurrentEdit = qobject_cast<QLineEdit*> (lWidget);
        QString lText = lCurrentEdit->text();
        mPrivate->mPatRuleEditorMapping[lWidget]->SetName(lText);

        mPrivate->mModified = true;
    }
}

void PATRecipeEditor::RuleBinChanged(const QString & )
{
    QWidget *lWidget = qobject_cast<QWidget *>(sender());
    if(lWidget && mPrivate->mPatRuleEditorMapping.contains(lWidget))
    {
        QLineEdit *lCurrentEdit = qobject_cast<QLineEdit*> (lWidget);
        QString lText = lCurrentEdit->text();
        mPrivate->mPatRuleEditorMapping[lWidget]->SetBin(lText.toInt());

        mPrivate->mModified = true;
    }
}

void PATRecipeEditor::RuleOutlierDistanceChanged(int idx)
{
    QWidget *lWidget = qobject_cast<QWidget *>(sender());
    if(lWidget && mPrivate->mPatRuleEditorMapping.contains(lWidget))
    {
        QComboBox *lCombo = qobject_cast<QComboBox *>(sender());
        mPrivate->mPatRuleEditorMapping[lWidget]->SetOutlierDistanceMode((PAT::OutlierDistance)(lCombo->itemData(idx).toInt()));

        QTreeWidgetItem *lItem = 0;
        QList<QTreeWidgetItem *> lItems = mPrivate->mPatRuleMapping.keys(mPrivate->mPatRuleEditorMapping[lWidget]);
        if(!lItems.isEmpty())
        {
            lItem = lItems.first();
        }
        if(!lItem)
            return;

        QDoubleSpinBox * lDoubleSB = qobject_cast<QDoubleSpinBox *>(mMultivariateTree->itemWidget(lItem, mPrivate->GetMVFieldColmun("Custom Distance")));
        lDoubleSB->setEnabled((mPrivate->mPatRuleEditorMapping[lWidget]->GetOutlierDistanceMode() == GS::Gex::PAT::Custom));

        mPrivate->mModified = true;
    }
}

void PATRecipeEditor::RuleCustomDistanceChanged(double lValue)
{
    QWidget *lWidget = qobject_cast<QWidget *>(sender());
    if(lWidget && mPrivate->mPatRuleEditorMapping.contains(lWidget))
    {
        mPrivate->mPatRuleEditorMapping[lWidget]->SetCustomDistance(lValue);
        mPrivate->mModified = true;
    }
}

void PATRecipeEditor::RuleEnabledChanged(int check)
{
    QWidget *lWidget = qobject_cast<QWidget *>(sender());
    if(lWidget && mPrivate->mPatRuleEditorMapping.contains(lWidget))
    {
        bool lEnable = false;
        if(check == Qt::Checked)
            lEnable = true;
        mPrivate->mPatRuleEditorMapping[lWidget]->SetEnabled(lEnable);

        mPrivate->mModified = true;
    }
}

void PATRecipeEditor::addMVEntry(PATMultiVariateRule &mvRule, int topLevelIdx)
{
    QTreeWidgetItem *lItem = new QTreeWidgetItem;
    mPrivate->mPatRuleMapping.insert(lItem, &mvRule);
    //mPrivate->mPatRuleEditorMapping.insert(widget,&mvRule);

    if((topLevelIdx == -1) || (topLevelIdx == mMultivariateTree->topLevelItemCount()))
        mMultivariateTree->addTopLevelItem(lItem);
    else
    {
        mMultivariateTree->insertTopLevelItem(topLevelIdx,lItem);
    }

    lItem->setText(mPrivate->GetMVFieldColmun("Type"),mvRule.GetTypeString());

    QIcon lRuleIconType;
    if(mvRule.GetType() == PATMultiVariateRule::Manual)
        lRuleIconType.addFile(QStringLiteral(":/gex/icons/move.png"), QSize(16,16), QIcon::Normal, QIcon::Off);
    else
        lRuleIconType.addFile(QStringLiteral(":/gex/icons/generated_rule.png"), QSize(16,16), QIcon::Normal, QIcon::Off);

    lItem->setIcon(mPrivate->GetMVFieldColmun("Type"),lRuleIconType);

    QLineEdit *lEdit = new QLineEdit(mvRule.GetName());
    lEdit->setValidator(new PATRecipeEditorPrivate::RuleNameValidator(mMultivariateTree, mPrivate->GetMVFieldColmun("Rule Name"), this));
    mMultivariateTree->setItemWidget(lItem, mPrivate->GetMVFieldColmun("Rule Name"), lEdit);
    mPrivate->mPatRuleEditorMapping.insert(lEdit,&mvRule);
    connect(lEdit,SIGNAL(editingFinished()), this, SLOT(RuleNameChanged()));
    connect(lEdit,SIGNAL(textChanged(const QString &)), this, SLOT(RuleNameChanged(const QString &)));

    lEdit = new QLineEdit(QString::number(mvRule.GetBin()));
    lEdit->setValidator(new QIntValidator(this));
    lEdit->setEnabled(mPrivate->mPATRecipe.GetOptions().GetMVPATEnabled());
    mMultivariateTree->setItemWidget(lItem, mPrivate->GetMVFieldColmun("Bin"), lEdit);
    mPrivate->mPatRuleEditorMapping.insert(lEdit,&mvRule);
    connect(lEdit,SIGNAL(editingFinished()), this, SLOT(RuleBinChanged()));
    connect(lEdit,SIGNAL(textChanged(const QString &)), this, SLOT(RuleBinChanged(const QString &)));


    QComboBox *lOutlierDistance = new QComboBox;
    lOutlierDistance->insertItem(0, "Near", (int)GS::Gex::PAT::Near);
    lOutlierDistance->insertItem(1, "Medium", (int)GS::Gex::PAT::Medium);
    lOutlierDistance->insertItem(2, "Far", (int)GS::Gex::PAT::Far);
    lOutlierDistance->insertItem(3, "Custom", (int)GS::Gex::PAT::Custom);
    lOutlierDistance->setCurrentIndex(lOutlierDistance->findData((int)(mvRule.GetOutlierDistanceMode())));


    mMultivariateTree->setItemWidget(lItem, mPrivate->GetMVFieldColmun("Outlier Distance"), lOutlierDistance);
    mPrivate->mPatRuleEditorMapping.insert(lOutlierDistance,&mvRule);
    connect(lOutlierDistance,SIGNAL(currentIndexChanged(int)), this, SLOT(RuleOutlierDistanceChanged(int)));

    QDoubleSpinBox * lDoubleSB = new QDoubleSpinBox();
    lDoubleSB->setRange(0, 100);
    lDoubleSB->setValue(mvRule.GetCustomDistance());
    lDoubleSB->setSingleStep(1);
    lDoubleSB->setEnabled((mvRule.GetOutlierDistanceMode() == GS::Gex::PAT::Custom));

    mMultivariateTree->setItemWidget(lItem, mPrivate->GetMVFieldColmun("Custom Distance"), lDoubleSB);
    mPrivate->mPatRuleEditorMapping.insert(lDoubleSB,&mvRule);

    connect(lDoubleSB, SIGNAL(valueChanged(double)), this, SLOT(RuleCustomDistanceChanged(double)));

    QCheckBox *lCheck = new QCheckBox();
    lCheck->setChecked(mvRule.GetEnabled());
    mMultivariateTree->setItemWidget(lItem, mPrivate->GetMVFieldColmun("Enabled"), lCheck);
    mPrivate->mPatRuleEditorMapping.insert(lCheck,&mvRule);
    connect(lCheck,SIGNAL(stateChanged(int)), this, SLOT(RuleEnabledChanged(int)));

    const QList<PATMultiVariateRule::MVTestData> &lTestData = mvRule.GetMVTestData();

    for(int lIdx=0; lIdx<lTestData.count(); ++lIdx)
    {
        QTreeWidgetItem *lItemTest = new QTreeWidgetItem();
        lItemTest->setText(mPrivate->GetMVFieldColmun("Tests list"),
                           QString("%1 - %2 - %3")
                           .arg(lTestData[lIdx].GetTestNumber())
                           .arg(lTestData[lIdx].GetTestName())
                           .arg(lTestData[lIdx].GetPinIdx()));
        lItem->addChild(lItemTest);
    }

    mPrivate->mModified = true;
}

void PATRecipeEditor::WriteRecipe(bool lMajorRelease /*= false*/)
{
    GSLOG(SYSLOG_SEV_DEBUG, "saveRecipe ...");

    QString lOldRecipeName = mPrivate->mRecipeName;
    QString lNewRecipeName = mPrivate->mRecipeName;

    // If .debug extension exists, remove it!
    int iDebugExtension = lOldRecipeName.lastIndexOf(".debug");
    if(iDebugExtension > 0)
        lNewRecipeName.truncate(iDebugExtension);

    // Append .CSV extension if not already present.
    if(lNewRecipeName.endsWith(".json", Qt::CaseInsensitive) == false)
        lNewRecipeName += ".gextb.json";

    // If PAT-Man recipe, check if Revision control enabled!
    // Overload Recipe name if it includes a version#
    if(iDebugExtension > 0 || mPrivate->mModified || lMajorRelease)
        BuildRecipeVersion(lNewRecipeName, lMajorRelease);

    QString lRecipeName = QFileDialog::getSaveFileName(this,
                                                       "Review & Edit Outlier / PAT Configuration File",
                                                       lNewRecipeName,
                                                       "Recipe file (*.json)");
    if(!lRecipeName.isEmpty())
    {
        RefreshFile(lRecipeName);
    }

    // Ensure we remove this temporary recipe file on exit
    if(iDebugExtension > 0)
        GS::Gex::Engine::GetInstance().GetTempFilesManager().addFile(lOldRecipeName, TemporaryFile::CloseCheck);
}

void PATRecipeEditor::RefreshFile(const QString &recipeFileName)
{
    GSLOG(SYSLOG_SEV_DEBUG,
          QString("Refreshing PAT recipe file %1").arg(recipeFileName)
          .toLatin1().constData());

    // Create a json recipe
    QSharedPointer<PATRecipeIO> lRecipeIO(PATRecipeIO::CreateRecipeIo(PATRecipeIO::JSON));

    if (lRecipeIO.isNull())
    {
        GS::Gex::Message::warning(windowTitle(), "Failed to write recipe");
        GSLOG(SYSLOG_SEV_ERROR, "Failed to write recipe : Unable to instanciate Recipe Writer");
        return;
    }

    setDisabled(true);
    mPrivate->mRecipeName = recipeFileName;

    lRecipeIO->SetRecipeName(recipeFileName);
    bool lStatus = lRecipeIO->Write(mPrivate->mPATRecipe, recipeFileName);

    setDisabled(false);
    if (lStatus == false)
    {
        GS::Gex::Message::warning(windowTitle(), QString("Failed to write recipe : %1").arg(lRecipeIO->GetErrorMessage()).toLatin1().constData());
        GSLOG(SYSLOG_SEV_ERROR, QString("Failed to write recipe : %1").arg(lRecipeIO->GetErrorMessage()).toLatin1().constData());
        return;
    }

    mPrivate->mModified = false;
}

///////////////////////////////////////////////////////////
// Force processing this CSV config file over STDF file
// and apply PAT limits
///////////////////////////////////////////////////////////
void PATRecipeEditor::ApplyPAT()
{
    GSLOG(SYSLOG_SEV_DEBUG, "ApplyPAT ...");

    // Disable the 'PAT Process' button to avoid double-clicking on it!
    mApplyPAT->setEnabled(false);

    // Checks if recipe to used is currently edited and needs to be saved first
    CheckSaveForModifiedRecipe();

    // Force to detach PAT editor
    DetachRecipeEditor();

    if (mPrivate->mPATRecipe.GetOptions().GetRecipeType() == GS::Gex::PAT::RecipeFinalTest)
    {
        // Display 'FT PAT: Process file' wizard
        pGexMainWindow->Wizard_FT_PAT();

        if (pGexMainWindow->mWizardFTPAT)
        {
            // If Config file name not already selected, use the Editor file name
            if(mPrivate->mRecipeName.isEmpty() == false)
                pGexMainWindow->mWizardFTPAT->SetRecipeFile(mPrivate->mRecipeName);

            QString lTestDataFile = pGexMainWindow->mWizardFTPAT->GetTestDataFile();

            if(gexReport)
            {
                if(lTestDataFile.isEmpty())
                {
                    CGexGroupOfFiles *  lGroup  = gexReport->getGroupsList().isEmpty() ?
                                NULL : gexReport->getGroupsList().first();
                    CGexFileInGroup *   lFile   = NULL;

                    if(lGroup != NULL)
                    {
                        lFile = (lGroup->pFilesList.isEmpty()) ? NULL : lGroup->pFilesList.first();

                        if(lFile != NULL)
                            // Force to process the first data file available in our groups.
                            pGexMainWindow->mWizardFTPAT->SetTestDataFile(lFile->strFileName);
                    }
                }
            }

            pGexMainWindow->mWizardFTPAT->RunSimulator();
        }
    }
    else if (mPrivate->mPATRecipe.GetOptions().GetRecipeType() == GS::Gex::PAT::RecipeWaferSort)
    {
        // Display 'WS PAT: Process file' wizard
        pGexMainWindow->Wizard_GexTb_PAT();

        // If Config file name not already selected, use the Editor file name
        if(mPrivate->mRecipeName.isEmpty() == FALSE)
            pGexMainWindow->mWizardWSPAT->lineEditConfigFile->setText(mPrivate->mRecipeName);

        // If input data file to process not specified in the dialog box, take the first one just analyzed
        QString strTestDataFile;
        pGexMainWindow->mWizardWSPAT->strGetFile(strTestDataFile);

        if(gexReport)
        {
            if(strTestDataFile.isEmpty())
            {
                CGexGroupOfFiles *  pGroup  = gexReport->getGroupsList().isEmpty()?NULL:gexReport->getGroupsList().first();
                CGexFileInGroup *   pFile   = NULL;

                if(pGroup != NULL)
                {

                    pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

                    if(pFile != NULL)
                        // Force to process the first data file available in our groups.
                        pGexMainWindow->mWizardWSPAT->AddFile(pFile->strFileName);
                }
            }
        }

        // Send signal to process file.
        if(pGexMainWindow->mWizardWSPAT != NULL)
            pGexMainWindow->mWizardWSPAT->OnProcessFile();
    }
    else
        GS::Gex::Message::warning("", "Recipe type not defined, unable to run any PAT processing.");

    // Enable the 'PAT Process' button
    mApplyPAT->setEnabled(true);
}


void PATRecipeEditor::Initialize()
{
    GSLOG(SYSLOG_SEV_DEBUG, "init ...");
    setupUi(this);
    setObjectName("GSRecipeEditor");
    setModal(false);
    setWindowTitle("PAT Recipe Editor");
    setWindowIcon(*pixGexApplication);
    setAcceptDrops(true);
    mPrivate = new PATRecipeEditorPrivate;
    EnableEditorButton(false);
    mDetach->setChecked(true);
    mRecipeData->setCurrentWidget(mUnivariate);

    mMultivariateTree->setSelectionBehavior(QAbstractItemView::SelectRows);
    mMultivariateTree->setSelectionMode(QAbstractItemView::SingleSelection);

    mUnivariateTree->clear();
    mMultivariateTree->clear();
    mUnivariateTree->setColumnCount(mPrivate->mUVColumns.count());
    mUnivariateTree->setHorizontalHeaderLabels(mPrivate->mUVColumns);
    mMultivariateTree->setColumnCount(mPrivate->mMVColumns.count());
    mMultivariateTree->setHeaderLabels(mPrivate->mMVColumns);

    mUnivariateTree->resizeColumnsToContents();
    for(int lIdx=0; lIdx<mMultivariateTree->columnCount(); ++lIdx)
        mMultivariateTree->resizeColumnToContents(lIdx);
    connect(mDetach, SIGNAL(clicked()), this, SLOT(DetachRecipeEditor()));
    connect(mApplyPAT, SIGNAL(clicked()), this, SLOT(ApplyPAT()));
    connect(mEditRecipeOption, SIGNAL(clicked()), this, SLOT(EditRecipeOption()));
    connect(mReadRecipe, SIGNAL(clicked()), this, SLOT(ReadRecipe()));
    connect(mWriteRecipe, SIGNAL(clicked()), this, SLOT(WriteRecipe()));
    connect(mPATSettings, SIGNAL(activated(int)), this, SLOT(SettingsPATTypeChanged(int)));
    connect(mFindTest, SIGNAL( clicked() ), this, SLOT( OnFindTest() ) );
    connect(mExportStaticPatLimits, SIGNAL( clicked() ), this, SLOT( OnExportStaticPatLimits() ) );
    connect(mAddPatTests, SIGNAL( clicked() ), this, SLOT( OnAddPatTests() ) );
    connect(mAddMVRule, SIGNAL( clicked() ), this, SLOT( OnAddMVRules() ) );

    mUnivariateTree->setContextMenuPolicy(Qt::CustomContextMenu);
    connect( mUnivariateTree, SIGNAL( customContextMenuRequested(const QPoint&) ),
                                                    this, SLOT( OnContextualMenu(const QPoint&) ) );
    mMultivariateTree->setContextMenuPolicy(Qt::CustomContextMenu);
    connect( mMultivariateTree, SIGNAL( customContextMenuRequested(const QPoint&) ),
                                                    this, SLOT( OnRuleContextualMenu(const QPoint&)));

    connect(mNewRule, SIGNAL( clicked() ), this, SLOT( AddNewRule()));
    connect(mDuplicateRule, SIGNAL( clicked() ), this, SLOT( DuplicateRule()));
    connect(mDeleteRule, SIGNAL( clicked() ), this, SLOT( DeleteRule()));
    connect(mEditTestList, SIGNAL( clicked() ), this, SLOT( EditTestList()));

    connect(mRuleUp, SIGNAL( clicked() ), this, SLOT( RuleUp()));
    connect(mRuleDown, SIGNAL( clicked() ), this, SLOT( RuleDown()));

    connect(this, SIGNAL( sRefreshEditor(const QString &) ), this, SLOT( RefreshEditor(const QString &)));


    InitPATSettings(PATRecipeEditor::Basic);
}

void PATRecipeEditor::Clear()
{
    GSLOG(SYSLOG_SEV_DEBUG, "clear ...");
    disconnect();
}

void PATRecipeEditor::EnableEditorButton(bool enable)
{
    mApplyPAT->setEnabled(enable);
    mEditRecipeOption->setEnabled(enable);
    mWriteRecipe->setEnabled(enable);
}
void PATRecipeEditor::InitializeData()
{
    EnableEditorButton(false);
    mPrivate->mPATRecipe.GetMultiVariateRules().clear();
    qDeleteAll(mPrivate->mPATRecipe.GetUniVariateRules().values());
    mPrivate->mPATRecipe.GetUniVariateRules().clear();
    mUnivariateTree->clearContents();
    mMultivariateTree->clear();
    mPrivate->mRuleIdx = 0;
    InitPATSettings(PATRecipeEditor::DPAT);
}

void PATRecipeEditor::EditRecipeOption()
{
    GSLOG(SYSLOG_SEV_DEBUG, "Edit Recipe Option ...");

    GS::Gex::PatLimitsDialog lPatConfigGlobals(&(mPrivate->mPATRecipe.GetOptions()), this);

    // No 'save' will take place when the dialog box 'Ok' button is clicked, 'Save' only occurs when
    // user clicks the 'Save' button or 'Apply PAT' button
    lPatConfigGlobals.setEditMode(false, false);

    // Save current PAT modes enabled
    bool bStaticPAT     = mPrivate->mPATRecipe.GetOptions().bStaticPAT;
    int	 iStaticBin     = mPrivate->mPATRecipe.GetOptions().iFailStatic_SBin;
    bool bDynamicPAT    = mPrivate->mPATRecipe.GetOptions().bDynamicPAT;
    int  iDynamicBin    = mPrivate->mPATRecipe.GetOptions().iFailDynamic_SBin;

    bool lMVPATEnabled  = mPrivate->mPATRecipe.GetOptions().GetMVPATEnabled();
    int  lMVPATSoftBin  = mPrivate->mPATRecipe.GetOptions().GetMVPATSoftBin();
    bool lNNREnabled    = mPrivate->mPATRecipe.GetOptions().IsNNREnabled();


    if(lPatConfigGlobals.exec() == QDialog::Accepted)
    {
        mPrivate->mModified = true;
    }
    else // QDialog::Rejected
    {
        return;
    }


    //Update MV Pat bin
    if(lMVPATEnabled && !mPrivate->mPATRecipe.GetOptions().GetMVPATEnabled())
    {
        ResetMVBin();
    }

    if((!lMVPATEnabled && mPrivate->mPATRecipe.GetOptions().GetMVPATEnabled()) || (lMVPATSoftBin  != mPrivate->mPATRecipe.GetOptions().GetMVPATSoftBin()))
    {
        UpdateMVBin(lMVPATSoftBin);
    }

    // check if Static/Dynamic PAT mode changed (if so, need to update Bin# in the table)
    if(bStaticPAT && !mPrivate->mPATRecipe.GetOptions().bStaticPAT)
    {
        ResetStaticBin();
    }

    if((!bStaticPAT && mPrivate->mPATRecipe.GetOptions().bStaticPAT) || (iStaticBin != mPrivate->mPATRecipe.GetOptions().iFailStatic_SBin))
    {
        UpdateStaticBin (iStaticBin);

    }

    if(bDynamicPAT && !mPrivate->mPATRecipe.GetOptions().bDynamicPAT)
    {
        ResetDynamicBin();
    }

    if((!bDynamicPAT && mPrivate->mPATRecipe.GetOptions().bDynamicPAT) || (iDynamicBin != mPrivate->mPATRecipe.GetOptions().iFailDynamic_SBin))
    {
        UpdateDynamicBin (iDynamicBin);
    }

    if (lNNREnabled != mPrivate->mPATRecipe.GetOptions().IsNNREnabled())
    {
        UpdateNNR(mPrivate->mPATRecipe.GetOptions().IsNNREnabled());
    }
}

void PATRecipeEditor::InitPATSettings(PATRecipeEditor::PATSettings setting)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Init PAT settings...");
    // Clear combo settings
    mPATSettings->clear();

    mPATSettings->addItem("SPAT Settings", QVariant(PATRecipeEditor::SPAT));
    mPATSettings->addItem("DPAT Settings", QVariant(PATRecipeEditor::DPAT));

    // NNR only available for WaferSort recipe
    if (mPrivate->mPATRecipe.GetOptions().GetRecipeType() == GS::Gex::PAT::RecipeWaferSort)
        mPATSettings->addItem("NNR Settings", QVariant(PATRecipeEditor::NNR));

    int lSettingsIndex = mPATSettings->findData(QVariant(setting));

    if (lSettingsIndex != -1)
        mPATSettings->setCurrentIndex(lSettingsIndex);
    else
        mPATSettings->setCurrentIndex(0);

    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Init PAT settings done.");
}

void PATRecipeEditor::SettingsPATTypeChanged(int )
{
    int lSetting = mPATSettings->itemData(mPATSettings->currentIndex()).toInt();
    int lFrom = -1, lTo = -1;

    //mUVColumns = mUVBasicColumn + mUVSPATColumn + mUVDPATColumn + mUVNNRColumn + mUVSPCColumn;
    if(lSetting == PATRecipeEditor::SPAT)
    {
        lFrom = mPrivate->mUVBasicColumn.count();
        lTo =   mPrivate->mUVBasicColumn.count() + mPrivate->mUVSPATColumn.count() - 1;
    }
    else if(lSetting == PATRecipeEditor::DPAT)
    {
        lFrom = mPrivate->mUVBasicColumn.count() + mPrivate->mUVSPATColumn.count() ;
        lTo =   mPrivate->mUVBasicColumn.count() + mPrivate->mUVSPATColumn.count() +  mPrivate->mUVDPATColumn.count() - 1;

    }
    else if(lSetting == PATRecipeEditor::NNR)
    {
        lFrom = mPrivate->mUVBasicColumn.count() + mPrivate->mUVSPATColumn.count() + mPrivate->mUVDPATColumn.count();
        lTo =   mPrivate->mUVBasicColumn.count() + mPrivate->mUVSPATColumn.count() + mPrivate->mUVDPATColumn.count() +  mPrivate->mUVNNRColumn.count() - 1;
    }
    else if(lSetting == PATRecipeEditor::SPC)
    {
        lFrom = mPrivate->mUVBasicColumn.count() + mPrivate->mUVSPATColumn.count() + mPrivate->mUVDPATColumn.count() + mPrivate->mUVNNRColumn.count();
        lTo =   mPrivate->mUVBasicColumn.count() + mPrivate->mUVSPATColumn.count() +  mPrivate->mUVDPATColumn.count() +  mPrivate->mUVNNRColumn.count() + mPrivate->mUVSPCColumn.count() - 1;
    }

    for (int lIdx =mPrivate->mUVBasicColumn.count();lIdx<mUnivariateTree->columnCount();++lIdx)
    {
        //int lIdx=lFrom;lIdx<lTo;++lIdx
        if((lIdx >= lFrom) && (lIdx <= lTo))
        {
            mUnivariateTree->showColumn(lIdx);
        }
        else
            mUnivariateTree->hideColumn(lIdx);
    }
}

///////////////////////////////////////////////////////////
// Export recipe Static PAT limits into CSV file (so to be sued as program test limits)
// Called from within Editor.
///////////////////////////////////////////////////////////
void PATRecipeEditor::OnExportStaticPatLimits(void)
{
    // Let's user tell where to save the table.
    QString strOldName = mPrivate->mRecipeName.section("_patman_config.csv",0,0);
    if(strOldName.isEmpty())
        strOldName =  mPrivate->mRecipeName;
    strOldName += "_SPAT_Limits.csv";

    QString strFile = QFileDialog::getSaveFileName(this,
                    "Save Static PAT Limits to...",
                    strOldName,
                    "Spreadsheet CSV(*.csv)",
                    NULL,
                    QFileDialog::DontConfirmOverwrite);

    // If no file selected, ignore command.
    if(strFile.isEmpty())
        return;

    // Open data file
    QFile cFile(strFile);
    if(cFile.exists())
    {
        bool lOk;
        GS::Gex::Message::request("", "File already exists. Overwrite it ?", lOk);
        if (! lOk)
        {
            return;
        }
    }

    if(!cFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        // Error. Can't create CSV file!
        GS::Gex::Message::information(
            "", "Failed to create CSV data file\n"
            "Disk full or write protection issue.");
        return;
    }

    // Save Static limits to file.
    QTextStream hCsvTableFile(&cFile);
    double lfSpatLL,lfSpatHL;
    QString	strLowLimitString,strHighLimitString;;
    QString	strWarning;
    hCsvTableFile << "Test#,Test name, SPAT Low L.,Units,SPAT High L,Units" << endl;

    for(int lRow = 0; lRow < mUnivariateTree->rowCount(); ++lRow)
    {
        // Clear warning message
        strWarning = "";

        unsigned long lTestNumber = (unsigned long)(mUnivariateTree->item(lRow,mPrivate->GetFieldColumn("Test#",PATRecipeEditor::Basic))->text().toLongLong());
        QString lTestName = mUnivariateTree->item(lRow,mPrivate->GetFieldColumn("Test name",PATRecipeEditor::Basic))->text();
        long lPinIdx = mUnivariateTree->item(lRow,mPrivate->GetFieldColumn("PinIndex",PATRecipeEditor::Basic))->text().toLong();

        CPatDefinition *lPatDef = mPrivate->mPATRecipe.FindUniVariateRule(lTestNumber, lPinIdx, lTestName);
        if(!lPatDef)
            continue;

        // Get & save Test#
        hCsvTableFile << lPatDef->m_lTestNumber << ",";

        // Get & save test name
        hCsvTableFile << lPatDef->m_strTestName << ",";

        // Check SPAT rule defined...if SBIN = -1, assume SPAT rule = DISABLED, and export STD limits
        // The ComputeStaticLimits() function would assign "GEX_TPAT_DOUBLE_INFINITE" limits, so to
        // keep previous behaviour of the export function, let's force STD limits here
        if((lPatDef->m_lFailStaticBin<0) || (lPatDef->m_SPATRuleType == GEX_TPAT_SPAT_ALGO_IGNORE))
        {
            lfSpatLL = lPatDef->m_lfLowLimit;
            lfSpatHL = lPatDef->m_lfHighLimit;
            strWarning = " *** NOTE: SPAT Disabled for this test.,";
        }
        else
        {
            // Compute SPAT limits
            lPatDef->ComputeStaticLimits();
            // Check if SPAT limits must noot exceed original test limits.
            lPatDef->CheckStaticLimits(mPrivate->mPATRecipe.GetOptions());

            lfSpatLL = lPatDef->m_lfLowStaticLimit;
            lfSpatHL = lPatDef->m_lfHighStaticLimit;

            if((lPatDef->m_SPATRuleType == GEX_TPAT_SPAT_ALGO_ROBUSTSIGMA) && (lPatDef->m_lfRobustSigma == 0.0))
                strWarning = " *** WARNING *** : Sigma=0. All test values identical.,";

            if((lPatDef->m_SPATRuleType == GEX_TPAT_SPAT_ALGO_SIGMA) && (lPatDef->m_lfSigma == 0.0))
                strWarning = " *** WARNING *** : Sigma=0. All test values identical.,";
        }

        // Format limit strings with scaled values
        mPrivate->ScaleLimits(strLowLimitString, lfSpatLL, lPatDef->m_llm_scal, ',', lPatDef->m_strUnits);
        mPrivate->ScaleLimits(strHighLimitString, lfSpatHL, lPatDef->m_hlm_scal, ',', lPatDef->m_strUnits);

        // Write scaled Low & High Limits
        hCsvTableFile << strLowLimitString << ",";
        hCsvTableFile << strHighLimitString << ",";

        // Write Warning (if any)
        hCsvTableFile << strWarning << "," << endl;
    }

    // Close file.
    cFile.close();
}


void PATRecipeEditor::ResetMVBin()
{
    int lColumn = mPrivate->GetMVFieldColmun("Bin");
    if(lColumn == -1)
        return;
    for(int lItemIdx=0; lItemIdx<mMultivariateTree->topLevelItemCount();++lItemIdx)
    {
        QTreeWidgetItem *lItem = mMultivariateTree->topLevelItem(lItemIdx);
        if(lItem)
        {
            QWidget *lWidget = mMultivariateTree->itemWidget(lItem, lColumn);
            QLineEdit *lEdit = qobject_cast<QLineEdit *>(lWidget);
            if(lEdit)
            {
                lEdit->setText("-1");
                lEdit->setEnabled(false);
            }
        }
    }
}

void PATRecipeEditor::UpdateMVBin(int multivariateBin)
{
    int lColumn = mPrivate->GetMVFieldColmun("Bin");
    if(lColumn == -1)
        return;
    for(int lItemIdx=0; lItemIdx<mMultivariateTree->topLevelItemCount();++lItemIdx)
    {
        QTreeWidgetItem *lItem = mMultivariateTree->topLevelItem(lItemIdx);
        if(lItem)
        {
            QWidget *lWidget = mMultivariateTree->itemWidget(lItem, lColumn);
            QLineEdit *lEdit = qobject_cast<QLineEdit *>(lWidget);
            if(lEdit)
            {
                int lBin = lEdit->text().toInt();
                if(lBin < 0 || lBin == multivariateBin)
                    lEdit->setText(QString::number(mPrivate->mPATRecipe.GetOptions().GetMVPATSoftBin()));

                lEdit->setEnabled(true);
            }
        }
    }
}

void PATRecipeEditor::UpdateNNR(bool lEnabled)
{
    int         lRuleCol    = mPrivate->GetFieldColumn("NNR Rule",PATRecipeEditor::NNR);
    QComboBox * lCombo      = NULL;

    // Step#1: count how many tests will be
    for(int lRow=0;lRow < mUnivariateTree->rowCount() ; ++lRow)
    {
        lCombo = qobject_cast<QComboBox *>(mUnivariateTree->cellWidget(lRow, lRuleCol));

        // Disable this test
        if(lCombo)
        {
            if (lEnabled)
                lCombo->setCurrentIndex(GS::Gex::PAT::GetNNRRuleIndex("Enabled"));
            else
                lCombo->setCurrentIndex(GS::Gex::PAT::GetNNRRuleIndex("Disabled"));
        }
    }
}

void PATRecipeEditor::ResetStaticBin()
{
     // Static PAT just disabled: force all Static bin column to '-1'
     //mExcelTable->item(row,GEX_TPAT_CONFIG_FIELD_SBIN)->setText("-1");
    int lColumn = mPrivate->GetFieldColumn("Static Bin",PATRecipeEditor::Basic);
    if(lColumn == -1)
        return;
    for(int lRow=0; lRow<mUnivariateTree->rowCount();++lRow)
    {
        QTableWidgetItem *lItem = mUnivariateTree->item(lRow, lColumn);
        if(lItem)
        {
            lItem->setText("-1");
            lItem->setFlags(lItem->flags()&~Qt::ItemIsEnabled);
        }
    }
}

void PATRecipeEditor::UpdateStaticBin (int staticBin)
{
    // Static PAT just enabled: force all Static bin column to the StaticPatBin (except for cells with already a valid bin > 0)
    int lColumn = mPrivate->GetFieldColumn("Static Bin",PATRecipeEditor::Basic);
    if(lColumn == -1)
        return;
    for(int lRow=0; lRow<mUnivariateTree->rowCount();++lRow)
    {
        QTableWidgetItem *lItem = mUnivariateTree->item(lRow, lColumn);
        if(lItem)
        {
            int lBin = lItem->text().toInt();
            if(lBin < 0 || lBin == staticBin)
                lItem->setText(QString::number(mPrivate->mPATRecipe.GetOptions().iFailStatic_SBin));

            lItem->setFlags(lItem->flags()|Qt::ItemIsEnabled);
        }
    }
}


void PATRecipeEditor::ResetDynamicBin()
{
    // Dynamic PAT just disabled: force all Dynamic bin column to '-1'
    //  mExcelTable->item(row,GEX_TPAT_CONFIG_FIELD_DBIN)->setText("-1");
    //  UpdateDpatGuiFactors(GEX_TPAT_RULETYPE_IGNOREID,row,false);
    int lColumn = mPrivate->GetFieldColumn("Dyn. Bin",PATRecipeEditor::Basic);
    if(lColumn == -1)
        return;
    for(int lRow=0; lRow<mUnivariateTree->rowCount();++lRow)
    {
        QTableWidgetItem *lItem = mUnivariateTree->item(lRow, lColumn);
        if(lItem)
        {
            QTableWidgetItem *lItem = mUnivariateTree->item(lRow, lColumn);
            if(lItem)
            {
                lItem->setText("-1");
                lItem->setFlags(lItem->flags()&~Qt::ItemIsEnabled);
            }
        }
    }
}

void PATRecipeEditor::UpdateDynamicBin (int iDynamicBin)
{
    // Dynamic PAT just enabled: force all Dynamic bin column to the DynamicPatBin (except for cells with already a valid bin > 0)
    int lColumn = mPrivate->GetFieldColumn("Dyn. Bin",PATRecipeEditor::Basic);
    for(int lRow=0; lRow<mUnivariateTree->rowCount();++lRow)
    {
        QTableWidgetItem *lItem = mUnivariateTree->item(lRow, lColumn);
        if(lItem)
        {
            QTableWidgetItem *lItem = mUnivariateTree->item(lRow, lColumn);
            if(lItem)
            {
                int lBin = lItem->text().toInt();
                if(lBin < 0 || lBin == iDynamicBin)
                {
                     lItem->setText(QString::number(mPrivate->mPATRecipe.GetOptions().iFailDynamic_SBin));
                }

                lItem->setFlags(lItem->flags()|Qt::ItemIsEnabled);

            }
        }
    }
}

///////////////////////////////////////////////////////////
// Rule combo-box toggled, update relevant DPAT N factor & Tail factor
void	PATRecipeEditor::UpdateDpatGuiFactors(
        int iRuleID,int row,bool bShowError/*=true*/,bool bResetDefaultFactors/*=true*/)
{
    GSLOG(7, "ToolBox Update Dpat Gui Factors");
    // Sets the N & T fields
    bool bDisabledDynamic=false;
    switch(iRuleID)
    {
        case GEX_TPAT_RULETYPE_SEPARATORID:		// Separator: same as 'disabled'
        {
            // force N (head) and T (tail) factors.
            mUnivariateTree->item(row, mPrivate->GetFieldColumn("N Factor", PATRecipeEditor::DPAT))->setText("");
            mUnivariateTree->item(row, mPrivate->GetFieldColumn("T Factor", PATRecipeEditor::DPAT))->setText("");

            iRuleID = GEX_TPAT_RULETYPE_IGNOREID;
            bDisabledDynamic = true;	// Disabled Dynamic PAT
            break;
        }

        case GEX_TPAT_RULETYPE_SIGMAID:			// Rule set: N*Sigma
        case GEX_TPAT_RULETYPE_ROBUSTSIGMAID:	// Rule set: N*RobustSigma
            // force N (head) and T (tail) factors.
            if(!bResetDefaultFactors)
                break;	// Keep current factors
            mUnivariateTree->item(row,mPrivate->GetFieldColumn("N Factor", PATRecipeEditor::DPAT))->setText("6.0");
            mUnivariateTree->item(row,mPrivate->GetFieldColumn("T Factor", PATRecipeEditor::DPAT))->setText("");
            break;

        case GEX_TPAT_RULETYPE_Q1Q3IQRID:			// Rule set: Q1-N*IQR, Q3+N*IQR
            // force N (head) and T (tail) factors.
            if(!bResetDefaultFactors)
                break;	// Keep current factors
            mUnivariateTree->item(row,mPrivate->GetFieldColumn("N Factor", PATRecipeEditor::DPAT))->setText("4.0");
            mUnivariateTree->item(row,mPrivate->GetFieldColumn("T Factor", PATRecipeEditor::DPAT))->setText("");
            break;

        case GEX_TPAT_RULETYPE_RANGEID:			// Rule set: Range
            // force N (head) and T (tail) factors.
            if(!bResetDefaultFactors)
                break;	// Keep current factors
            mUnivariateTree->item(row,mPrivate->GetFieldColumn("N Factor", PATRecipeEditor::DPAT))->setText("0.0");
            mUnivariateTree->item(row,mPrivate->GetFieldColumn("T Factor", PATRecipeEditor::DPAT))->setText("");
            break;

        case GEX_TPAT_RULETYPE_SMARTID:			// Rule set: Smart & adaptive
            // force N (head) and T (tail) factors.
            mUnivariateTree->item(row,mPrivate->GetFieldColumn("N Factor", PATRecipeEditor::DPAT))->setText("");
            mUnivariateTree->item(row,mPrivate->GetFieldColumn("T Factor", PATRecipeEditor::DPAT))->setText("");
            break;

        case GEX_TPAT_RULETYPE_IGNOREID:		// Rule set: Disabled
            // force N (head) and T (tail) factors.
            mUnivariateTree->item(row,mPrivate->GetFieldColumn("N Factor", PATRecipeEditor::DPAT))->setText("");
            mUnivariateTree->item(row,mPrivate->GetFieldColumn("T Factor", PATRecipeEditor::DPAT))->setText("");
            bDisabledDynamic = true;	// Disabled Dynamic PAT
            break;

        case GEX_TPAT_RULETYPE_GAUSSIANID:		// Rule: Gaussian distribution
            // force N (head) and T (tail) factors.
            if(!bResetDefaultFactors)
                break;	// Keep current factors
            mUnivariateTree->item(row,mPrivate->GetFieldColumn("N Factor", PATRecipeEditor::DPAT))->setText(QString::number(fabs(mPrivate->mPATRecipe.GetOptions().lfSmart_HeadGaussian[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR])));
            mUnivariateTree->item(row,mPrivate->GetFieldColumn("T Factor", PATRecipeEditor::DPAT))->setText(QString::number(fabs(mPrivate->mPATRecipe.GetOptions().lfSmart_TailGaussian[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR])));
            break;
        case GEX_TPAT_RULETYPE_GAUSSIANTAILID:	// Rule: Gaussian+Tail distribution
            // force N (head) and T (tail) factors.
            if(!bResetDefaultFactors)
                break;	// Keep current factors
            mUnivariateTree->item(row,mPrivate->GetFieldColumn("N Factor", PATRecipeEditor::DPAT))->setText(QString::number(fabs(mPrivate->mPATRecipe.GetOptions().lfSmart_HeadGaussianTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR])));
            mUnivariateTree->item(row,mPrivate->GetFieldColumn("T Factor", PATRecipeEditor::DPAT))->setText(QString::number(fabs(mPrivate->mPATRecipe.GetOptions().lfSmart_TailGaussianTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR])));
            break;
        case GEX_TPAT_RULETYPE_GAUSSIANDUALTAILID:	// Rule: Gaussian+2 Tails distribution
            // force N (head) and T (tail) factors.
            if(!bResetDefaultFactors)
                break;	// Keep current factors
            mUnivariateTree->item(row,mPrivate->GetFieldColumn("N Factor", PATRecipeEditor::DPAT))->setText(QString::number(fabs(mPrivate->mPATRecipe.GetOptions().lfSmart_HeadGaussianDoubleTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR])));
            mUnivariateTree->item(row,mPrivate->GetFieldColumn("T Factor", PATRecipeEditor::DPAT))->setText(QString::number(fabs(mPrivate->mPATRecipe.GetOptions().lfSmart_TailGaussianDoubleTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR])));
            break;
        case GEX_TPAT_RULETYPE_LOGNORMALID:		// Rule: LogNormal distribution
            // force N (head) and T (tail) factors.
            if(!bResetDefaultFactors)
                break;	// Keep current factors
            mUnivariateTree->item(row,mPrivate->GetFieldColumn("N Factor", PATRecipeEditor::DPAT))->setText(QString::number(fabs(mPrivate->mPATRecipe.GetOptions().lfSmart_HeadLogNormal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR])));
            mUnivariateTree->item(row,mPrivate->GetFieldColumn("T Factor", PATRecipeEditor::DPAT))->setText(QString::number(fabs(mPrivate->mPATRecipe.GetOptions().lfSmart_TailLogNormal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR])));
            break;
        case GEX_TPAT_RULETYPE_BIMODALID:		// Rule: Bi-Modal distribution (clearly two modes apart)
            // force N (head) and T (tail) factors.
            if(!bResetDefaultFactors)
                break;	// Keep current factors
            mUnivariateTree->item(row,mPrivate->GetFieldColumn("N Factor", PATRecipeEditor::DPAT))->setText(QString::number(fabs(mPrivate->mPATRecipe.GetOptions().lfSmart_HeadMultiModal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR])));
            mUnivariateTree->item(row,mPrivate->GetFieldColumn("T Factor", PATRecipeEditor::DPAT))->setText(QString::number(fabs(mPrivate->mPATRecipe.GetOptions().lfSmart_TailMultiModal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR])));
            break;
        case GEX_TPAT_RULETYPE_MULTIMODALID:		// Rule: Multi-Modal distribution
            // force N (head) and T (tail) factors.
            if(!bResetDefaultFactors)
                break;	// Keep current factors
            mUnivariateTree->item(row,mPrivate->GetFieldColumn("N Factor", PATRecipeEditor::DPAT))->setText(QString::number(fabs(mPrivate->mPATRecipe.GetOptions().lfSmart_HeadMultiModal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR])));
            mUnivariateTree->item(row,mPrivate->GetFieldColumn("T Factor", PATRecipeEditor::DPAT))->setText(QString::number(fabs(mPrivate->mPATRecipe.GetOptions().lfSmart_TailMultiModal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR])));
            break;
        case GEX_TPAT_RULETYPE_CLAMPEDID:		// Rule: Clamped distribution
            // force N (head) and T (tail) factors.
            if(!bResetDefaultFactors)
                break;	// Keep current factors
            mUnivariateTree->item(row,mPrivate->GetFieldColumn("N Factor", PATRecipeEditor::DPAT))->setText(QString::number(fabs(mPrivate->mPATRecipe.GetOptions().lfSmart_HeadClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR])));
            mUnivariateTree->item(row,mPrivate->GetFieldColumn("T Factor", PATRecipeEditor::DPAT))->setText(QString::number(fabs(mPrivate->mPATRecipe.GetOptions().lfSmart_TailClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR])));
            break;
        case GEX_TPAT_RULETYPE_DUALCLAMPEDID:	// Rule: DoubleClamped distribution
            // force N (head) and T (tail) factors.
            if(!bResetDefaultFactors)
                break;	// Keep current factors
            mUnivariateTree->item(row,mPrivate->GetFieldColumn("N Factor", PATRecipeEditor::DPAT))->setText(QString::number(fabs(mPrivate->mPATRecipe.GetOptions().lfSmart_HeadDoubleClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR])));
            mUnivariateTree->item(row,mPrivate->GetFieldColumn("T Factor", PATRecipeEditor::DPAT))->setText(QString::number(fabs(mPrivate->mPATRecipe.GetOptions().lfSmart_TailDoubleClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR])));
            break;
        case GEX_TPAT_RULETYPE_CATEGORYID:		// Rule: Categories distribution
            // force N (head) and T (tail) factors.
            if(!bResetDefaultFactors)
                break;	// Keep current factors
            mUnivariateTree->item(row,mPrivate->GetFieldColumn("N Factor", PATRecipeEditor::DPAT))->setText(QString::number(fabs(mPrivate->mPATRecipe.GetOptions().lfSmart_HeadCategory[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR])));
            mUnivariateTree->item(row,mPrivate->GetFieldColumn("T Factor", PATRecipeEditor::DPAT))->setText(QString::number(fabs(mPrivate->mPATRecipe.GetOptions().lfSmart_TailCategory[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR])));
            break;

        case GEX_TPAT_RULETYPE_LIMITSID:	// Rule set: % of limits
            // check that the TWO test limits fields are valid!
            if(!bResetDefaultFactors)
                break;	// Keep current factors
            if(mUnivariateTree->item(row,mPrivate->GetFieldColumn("LL.", PATRecipeEditor::Basic))->text().isEmpty() ||
              mUnivariateTree->item(row,mPrivate->GetFieldColumn("HL.", PATRecipeEditor::Basic))->text().isEmpty())
            {
                if(bShowError)
                {
                    GS::Gex::Message::critical(
                        "",
                        "You can only select this rule for parameters\n"
                        "with both Low and High spec limits");
                }
                iRuleID = GEX_TPAT_RULETYPE_IGNOREID;

            }
            else
            {
                mUnivariateTree->item(row,mPrivate->GetFieldColumn("N Factor", PATRecipeEditor::DPAT))->setText("70%");
                mUnivariateTree->item(row,mPrivate->GetFieldColumn("T Factor", PATRecipeEditor::DPAT))->setText("");
            }
            break;

        case GEX_TPAT_RULETYPE_NEWLIMITSID:	// Rule set: Force new limits
            // force N (head) and T (tail) factors to Low & high limits
            if(!bResetDefaultFactors)
                break;	// Keep current factors
            mUnivariateTree->item(row,mPrivate->GetFieldColumn("N Factor", PATRecipeEditor::DPAT))->setText(QString::number(mUnivariateTree->item(row,mPrivate->GetFieldColumn("LL.", PATRecipeEditor::Basic))->data(Qt::UserRole).toFloat()));
            mUnivariateTree->item(row,mPrivate->GetFieldColumn("T Factor", PATRecipeEditor::DPAT))->setText(QString::number(mUnivariateTree->item(row,mPrivate->GetFieldColumn("HL.", PATRecipeEditor::Basic))->data(Qt::UserRole).toFloat()));
            break;
    }

    // Force to repaint 'Test#' cell as it may change its background color (red if disabled, white otherwise)
    mUnivariateTree->item(row,mPrivate->GetFieldColumn("Test#", PATRecipeEditor::Basic))->setText(mUnivariateTree->item(row,mPrivate->GetFieldColumn("Test#", PATRecipeEditor::Basic))->text());

    // Update the Dynamic PAT bin if needed
    if(bDisabledDynamic == true)
    {
        // Force dynamic PAT bin to -1.
        mUnivariateTree->item(row,mPrivate->GetFieldColumn("Dyn. Bin", PATRecipeEditor::Basic))->setText("-1");
    }
    else
    {
        // Enabled Dynamic PAT bin: Force it unless already defined (not -1)
        if(mUnivariateTree->item(row,mPrivate->GetFieldColumn("Dyn. Bin", PATRecipeEditor::Basic))->text().toLong() < 0 &&
           mPrivate->mPATRecipe.GetOptions().bDynamicPAT)
        {
            mUnivariateTree->item(row,mPrivate->GetFieldColumn("Dyn. Bin", PATRecipeEditor::Basic))->setText(QString::number(mPrivate->mPATRecipe.GetOptions().iFailDynamic_SBin));
        }
    }

    QComboBox * lCombo = NULL;
    // Sets 'Outliers to keep' cell
    // if Separator: same as 'disabled' or Rule set: Disabled
    if (GEX_TPAT_RULETYPE_SEPARATORID == iRuleID || GEX_TPAT_RULETYPE_IGNOREID == iRuleID)
    {

        lCombo = qobject_cast<QComboBox *>(mUnivariateTree->cellWidget(row, mPrivate->GetFieldColumn("DPAT Rule", PATRecipeEditor::DPAT)));
        if (lCombo)
            lCombo->setCurrentIndex(iRuleID);

        // Reset combo box & selection: 'Samples to Ignore'
        lCombo = qobject_cast<QComboBox *>(mUnivariateTree->cellWidget(row, mPrivate->GetFieldColumn("Samples to ignore", PATRecipeEditor::DPAT)));
        if (lCombo)
            lCombo->setCurrentIndex(GEX_TPAT_IGNOREDATA_NONEID);

        // Reset combo box & selection: 'Outliers to keep'
        lCombo = qobject_cast<QComboBox *>(mUnivariateTree->cellWidget(row, mPrivate->GetFieldColumn("Outliers to keep", PATRecipeEditor::DPAT)));
        if (lCombo)
            lCombo->setCurrentIndex(GEX_TPAT_KEEPTYPE_NONEID);

        // Reset combo box & selection: 'Outliers Limits set'
        lCombo = qobject_cast<QComboBox *>(mUnivariateTree->cellWidget(row, mPrivate->GetFieldColumn("Outlier Limits set", PATRecipeEditor::DPAT)));
        if (lCombo)
            lCombo->setCurrentIndex(GEX_TPAT_LIMITSSET_NEAR);
    }

    // Enable/Disable Tail management
    bool lEnableTailMngt = (iRuleID == GEX_TPAT_RULETYPE_SMARTID) ? true : false;
    lCombo = qobject_cast<QComboBox *>(
                mUnivariateTree->cellWidget(
                    row,
                    mPrivate->GetFieldColumn("Tail management", PATRecipeEditor::DPAT))
                );
    if (lCombo)
    {
        if (!lEnableTailMngt)
            lCombo->setCurrentIndex(GEX_TPAT_TAIL_MNGT_DISABLED);
        else
            lCombo->setCurrentIndex(GEX_TPAT_TAIL_MNGT_LIBERAL);
        lCombo->setEnabled(lEnableTailMngt);
    }
}

///////////////////////////////////////////////////////////
// Rule combo-box toggled, update relevant SPAT N factor & Tail factor
///////////////////////////////////////////////////////////
void PATRecipeEditor::UpdateSpatGuiFactors(int iRuleID,
                                        int row,
                                        bool /*bShowError = true*/,
                                        bool /*bResetDefaultFactors = true*/)
{
    GSLOG(7, "ToolBox Update Spat Gui Factors");
    QString strString;
    bool	bNumber;
    switch(iRuleID)
    {
        case GEX_TPAT_SPAT_ALGO_ROBUSTSIGMA:	// Rule set: N*RobustSigma
            strString = mUnivariateTree->item(row,mPrivate->GetFieldColumn("N Factor", PATRecipeEditor::SPAT))->text();
            if(strString.isEmpty())
                mUnivariateTree->item(row,mPrivate->GetFieldColumn("N Factor", PATRecipeEditor::SPAT))->setText("");
            strString = mUnivariateTree->item(row,mPrivate->GetFieldColumn("T Factor", PATRecipeEditor::SPAT))->text();
            if(strString.isEmpty())
                mUnivariateTree->item(row,mPrivate->GetFieldColumn("T Factor", PATRecipeEditor::SPAT))->setText("");
            break;

        case GEX_TPAT_SPAT_ALGO_SIGMA:			// Rule set: N*Sigma
            strString = mUnivariateTree->item(row,mPrivate->GetFieldColumn("N Factor", PATRecipeEditor::SPAT))->text();
            strString.toFloat(&bNumber);
            if(strString.isEmpty() || !bNumber)
                mUnivariateTree->item(row,mPrivate->GetFieldColumn("N Factor", PATRecipeEditor::SPAT))->setText("6.0");

            mUnivariateTree->item(row,mPrivate->GetFieldColumn("T Factor", PATRecipeEditor::SPAT))->setText("");
            break;

        case GEX_TPAT_SPAT_ALGO_NEWLIMITS:
            // force N (head) and T (tail) factors to Low & high limits
            strString = mUnivariateTree->item(row,mPrivate->GetFieldColumn("N Factor", PATRecipeEditor::SPAT))->text();
            if(strString.isEmpty())
                mUnivariateTree->item(row,mPrivate->GetFieldColumn("N Factor", PATRecipeEditor::SPAT))->setText(QString::number(mUnivariateTree->item(row,mPrivate->GetFieldColumn("LL.", PATRecipeEditor::Basic))->data(Qt::UserRole).toFloat()));
            strString = mUnivariateTree->item(row,mPrivate->GetFieldColumn("T Factor", PATRecipeEditor::SPAT))->text();
            if(strString.isEmpty())
                mUnivariateTree->item(row,mPrivate->GetFieldColumn("T Factor", PATRecipeEditor::SPAT))->setText(QString::number(mUnivariateTree->item(row,mPrivate->GetFieldColumn("HL.", PATRecipeEditor::Basic))->data(Qt::UserRole).toFloat()));
            break;

        case GEX_TPAT_SPAT_ALGO_RANGE:			// Rule set: Range
            strString = mUnivariateTree->item(row,mPrivate->GetFieldColumn("N Factor", PATRecipeEditor::SPAT))->text();
            if(strString.isEmpty())
                mUnivariateTree->item(row,mPrivate->GetFieldColumn("N Factor", PATRecipeEditor::SPAT))->setText("");
            mUnivariateTree->item(row,mPrivate->GetFieldColumn("T Factor", PATRecipeEditor::SPAT))->setText("");
            break;

        case GEX_TPAT_SPAT_ALGO_IGNORE:	// SPAT disabled
            mUnivariateTree->item(row,mPrivate->GetFieldColumn("Static Bin", PATRecipeEditor::Basic))->setText("-1");
            mUnivariateTree->item(row,mPrivate->GetFieldColumn("N Factor", PATRecipeEditor::SPAT))->setText("");
            mUnivariateTree->item(row,mPrivate->GetFieldColumn("T Factor", PATRecipeEditor::SPAT))->setText("");
            // REbuild + update dropdown
//            QStandardItemModel *model = (QStandardItemModel*)mUnivariateTree->model();
//            QModelIndex index =  model->index(row, mPrivate->GetFieldColmun("Static Bin", PATRecipeEditor::Basic), QModelIndex());
//            model->setData(index,  QVariant(gexSpatRuleSetItemsGUI[GEX_TPAT_SPAT_ALGO_IGNORE]));
            break;
    }

    // 6180
    // Enabled PAT bin: Force it unless already defined (not -1)
    if( iRuleID != GEX_TPAT_SPAT_ALGO_IGNORE &&
        mUnivariateTree->item(row,mPrivate->GetFieldColumn("Static Bin", PATRecipeEditor::Basic))->text().toLong() < 0 &&
        mPrivate->mPATRecipe.GetOptions().bStaticPAT)
    {
        mUnivariateTree->item(row, mPrivate->GetFieldColumn("Static Bin", PATRecipeEditor::Basic))->setText(QString::number(mPrivate->mPATRecipe.GetOptions().iFailStatic_SBin));
    }
}
///////////////////////////////////////////////////////////
// Find test in recipe and select its line; allows quick search in long recipe list!
///////////////////////////////////////////////////////////
void	PATRecipeEditor::OnFindTest()
{
    // Show TestList: Single selection mode.
    PickTestDialog dPickTest;

    // Disable multiselection
    dPickTest.setMultipleSelection(false);

    // Disable selection of multiple layers: user can only pick a test from a given layer
    dPickTest.setMultipleGroups(false,false);

    // Fill list; format is <test#> <test name> <test type>

    QStringList strTestList;
    QStringList strPinList;

    // Get list of tests in recipe...
    QHash<QString, CPatDefinition *> &lUniVariateRules = mPrivate->mPATRecipe.GetUniVariateRules();
    QHash<QString, CPatDefinition *>::iterator lPatDefIter = lUniVariateRules.begin();
    while(lPatDefIter != lUniVariateRules.end())
    {
        CPatDefinition *lDef = lPatDefIter.value();//lUniVariateRules.value(lKeys[lIdx]);
        strTestList << QString::number(lDef->m_lTestNumber);
        strTestList << lDef->m_strTestName;
        strTestList << QString(lDef->GetTestTypeLegacy());
        strPinList << QString::number(lDef->mPinIndex);
        ++lPatDefIter;
    }


    if (dPickTest.fillParameterList(strTestList,strPinList))
    {
        // Prompt dialog box, let user pick ONE test from the list
        if(dPickTest.exec() == QDialog::Accepted)
        {
            // Get test# and test name selected
            QString strTestNumber = dPickTest.testItemizedList();
            QStringList strTestName = dPickTest.testItemizedListNames();
            mUnivariateTree->clearSelection();
            mUnivariateTree->setFocus();

            for(int lRow=0; lRow < mUnivariateTree->rowCount();++lRow)
            {

                if((mUnivariateTree->item(lRow,mPrivate->GetFieldColumn("Test#", PATRecipeEditor::Basic))->text() == strTestNumber )
                        && (mUnivariateTree->item(lRow,mPrivate->GetFieldColumn("Test name", PATRecipeEditor::Basic))->text() == strTestName.first()))
                {
                    mUnivariateTree->selectRow(lRow);
                    mUnivariateTree->scrollToItem(mUnivariateTree->item(lRow,0));
                }
            }
        }
    }
}


///////////////////////////////////////////////////////////
// Rule combo-box toggled, update relevant NNR rule
///////////////////////////////////////////////////////////
void	PATRecipeEditor::UpdateNnrGuiFactors(int /*iRuleID*/,int /*row*/)
{
    // Rebuild + update dropdown
//    QStandardItemModel *model = (QStandardItemModel*)mUnivariateTree->model();
//    QModelIndex index = model->index(row, mPrivate->GetFieldColmun("NNR Rule", PATRecipeEditor::NNR), QModelIndex());
//    model->setData(index,  QVariant(gexNnrRuleSetItemsGUI[iRuleID]));
}

int PATRecipeEditorPrivate::GetFieldColumn(const QString &field, PATRecipeEditor::PATSettings setting)
{
    if(mUVColumns.indexOf(field)==-1)
        return -1;
    //mUVColumns = mUVBasicColumn + mUVSPATColumn + mUVDPATColumn + mUVNNRColumn + mUVSPCColumn;
    if(setting == PATRecipeEditor::SPAT)
    {
        int lIdx = mUVSPATColumn.indexOf(field);
        if(lIdx == -1)
            return -1;
        return mUVBasicColumn.count() + lIdx;
    }
    else if(setting == PATRecipeEditor::DPAT)
    {
        int lIdx = mUVDPATColumn.indexOf(field);
        if(lIdx == -1)
            return -1;
        return mUVBasicColumn.count() + mUVSPATColumn.count() + lIdx;
    }
    else if(setting == PATRecipeEditor::NNR)
    {
        int lIdx = mUVNNRColumn.indexOf(field);
        if(lIdx == -1)
            return -1;
        return mUVBasicColumn.count() + mUVSPATColumn.count() + mUVDPATColumn.count() + lIdx;
    }
    else if(setting == PATRecipeEditor::SPC)
    {
        int lIdx = mUVSPCColumn.indexOf(field);
        if(lIdx == -1)
            return -1;
        return mUVBasicColumn.count() + mUVSPATColumn.count() + mUVDPATColumn.count() + mUVNNRColumn.count() + lIdx;
    }
    else
    {
        int lIdx = mUVBasicColumn.indexOf(field);
        if(lIdx == -1)
            return -1;
        return mUVBasicColumn.indexOf(field);
    }
}

int PATRecipeEditorPrivate::GetMVFieldColmun(const QString &field)
{
    return mMVColumns.indexOf(field);
}

PATRecipeEditorPrivate::PATRecipeEditorPrivate()
{

    //Basic column
    mUVBasicColumn = QStringList()
    << "Static Bin"
    << "Dyn. Bin"
    << "Test#"
    << "Test Type"
    << "PinIndex"
    << "Test name"
    << "LL."
    << "HL.";

    //SPAT columns
    mUVSPATColumn = QStringList()
    << "SPAT rule"
    << "N Factor"
    << "T Factor"
    << "SPAT Notes";

    //DPAT columns
    mUVDPATColumn = QStringList()
    << "Shape"
    << "Samples to ignore"
    << "Outliers to keep"
    << "Outlier Limits set"
    << "DPAT Rule"
    << "Tail management"
    << "N Factor"
    << "T Factor"
    << "DPAT Notes";

    //NNR columns
    mUVNNRColumn = QStringList()
    << "NNR Rule";

    //SPC columns
    mUVSPCColumn = QStringList()
    << "PAT-Median Drift alarm"
    << "PAT-Median Drift units"
    << "Test Mean Drift alarm"
    << "Test Mean Drift units"
    << "Cpk alarm";

    mUVColumns = mUVBasicColumn + mUVSPATColumn + mUVDPATColumn + mUVNNRColumn + mUVSPCColumn;
    /*
    //Ignore Field
    = QStringList()
    << "Historical Cpk"
    << "R.Mean"
    << "R.Sigma"
    << "Mean"
    << "Sigma"
    << "Range"
    */

    mMVColumns = QStringList()
    <<  "Type"
    <<  "Rule Name"
    <<  "Bin"
    <<  "Outlier Distance"
    <<  "Custom Distance"
    <<  "Tests list"
    <<  "Enabled"
    <<  "MV PAT notes";

}

PATRecipeEditorPrivate::~PATRecipeEditorPrivate()
{

}

///////////////////////////////////////////////////////////////////////////////
// DelegateItem: constructure
///////////////////////////////////////////////////////////////////////////////
EditorComboTableDelegate::EditorComboTableDelegate(const char* comboItems[], int sizeTable, QObject * parent/* = 0*/):
    QItemDelegate(parent)
{
     mComboItems.clear();
     int iIndex = 0;
     while(comboItems[iIndex])
     {
        mComboItems.append(comboItems[iIndex++]);
     }
     mSizeTable = sizeTable;
}

///////////////////////////////////////////////////////////////////////////////
// DelegateItem: Overload of create editor function that create a combo box
///////////////////////////////////////////////////////////////////////////////
QWidget * EditorComboTableDelegate::createEditor ( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    QComboBox * cb = new QComboBox(parent);
    if (index.row()>1 && index.row()<mSizeTable-1)
    {
        for(int i = 0; i < mComboItems.size(); ++i)
        {
           cb->addItem(mComboItems[i]);
        }
    }
    else
    {
        return QItemDelegate::createEditor(parent, option, index);
    }
    connect(cb, SIGNAL(activated(int)), this, SLOT(emitCommitData()));
    return cb;

}


///////////////////////////////////////////////////////////////////////////////
// DelegateItem: Overload of editor data setter function : Set data in the combo Box
///////////////////////////////////////////////////////////////////////////////
void EditorComboTableDelegate::setEditorData(QWidget */*editor*/, const QModelIndex &/*index*/) const
{
    /*
    if (index.row()>1 && index.row()<mSizeTable-1)
    {
        QString     qs          = index.model()->data(index, Qt::DisplayRole).toString();
        QComboBox * comboBox    = static_cast<QComboBox*>(editor);
        int         lIndex      = -1;

        switch(index.column())
        {
            case GEX_TPAT_CONFIG_FIELD_IGNOREDATA:
                lIndex = GS::Gex::PAT::GetSamplesToIgnoreIndex(qs);
                break;

            case GEX_TPAT_CONFIG_FIELD_OUTLIER_LIMIT_SET:
                lIndex = GS::Gex::PAT::GetOutlierLimitsSetIndex(qs);
                break;

            case GEX_TPAT_CONFIG_FIELD_KEEP:
                lIndex = GS::Gex::PAT::GetOutlierToKeepIndex(qs);
                break;

            case GEX_TPAT_CONFIG_FIELD_SBINRULE:
                lIndex = GS::Gex::PAT::GetSPATRuleIndex(qs);
                break;

            case GEX_TPAT_CONFIG_FIELD_RULE:
                lIndex = GS::Gex::PAT::GetRuleIndex(qs);
                break;

            case GEX_TPAT_CONFIG_FIELD_SPC_TEST_DRIFT_UNITS:
            case GEX_TPAT_CONFIG_FIELD_SPC_PAT_DRIFT_UNITS:
                lIndex = GS::Gex::PAT::GetMedianDriftUnitsIndex(qs);
                break;

            case GEX_TPAT_CONFIG_FIELD_NNR_RULE:
                lIndex = GS::Gex::PAT::GetNNRRuleIndex(qs);
                break;

            default:
                break;
        }

        comboBox->setCurrentIndex(lIndex);
    }
    else
    {

    }
    QItemDelegate::setEditorData(editor, index);
    */
}



///////////////////////////////////////////////////////////////////////////////
// DelegateItem: Overload of model data setter function
///////////////////////////////////////////////////////////////////////////////
void EditorComboTableDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const
{
    if (index.row()>1 && index.row()<mSizeTable-1)
    {
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        model->setData(index, comboBox->itemText(comboBox->currentIndex()), Qt::EditRole);
    }
    else
    {
        QItemDelegate::setModelData(editor, model, index);
    }
}

///////////////////////////////////////////////////////////
// DelegateItem: Overload of update editor geomety function
///////////////////////////////////////////////////////////
void EditorComboTableDelegate::updateEditorGeometry(QWidget *editor,
     const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
     editor->setGeometry(option.rect);
}


///////////////////////////////////////////////////////////
// DelegateItem: Overload of commit data signal
///////////////////////////////////////////////////////////
void EditorComboTableDelegate::emitCommitData()
{
    emit commitData(qobject_cast<QWidget *>(sender()));
}


void PATRecipeEditor::initExcelTableColumnEditor(int /*row*/,int /*col*/)
{
    /*
    QAbstractItemDelegate *lColumnDelegate = mExcelTable->itemDelegateForColumn(col);
    EditorComboTableDelegate *lDelegateCombo = NULL;
    int lExcelTableSize = mExcelTable->rowCount();
    if(!lColumnDelegate)
    {
        if(col == GEX_TPAT_CONFIG_FIELD_IGNOREDATA)
        {
            // Insert combo box in all tests 'Samples to Ignore' column.
            lDelegateCombo = new EditorComboTableDelegate(gexIgnoreSamplesSetItemsGUI,
                                                          lExcelTableSize, this);
            mExcelTable->setItemDelegateForColumn(GEX_TPAT_CONFIG_FIELD_IGNOREDATA, lDelegateCombo);

        }
        else if(col == GEX_TPAT_CONFIG_FIELD_KEEP)
        {
            // Insert combo box in all tests 'Outliers to Keep' column.
            lDelegateCombo = new EditorComboTableDelegate(gexKeepOutliersSetItemsGUI, lExcelTableSize, this);
            mExcelTable->setItemDelegateForColumn(GEX_TPAT_CONFIG_FIELD_KEEP, lDelegateCombo);

        }
        else if(col == GEX_TPAT_CONFIG_FIELD_OUTLIER_LIMIT_SET)
        {
            // Insert combo box in all tests 'Limits set (Near / Medim / Far outliers)' column.
            lDelegateCombo = new EditorComboTableDelegate(gexOutlierLimitsSetItemsLongNamesGUI, lExcelTableSize, this);
            mExcelTable->setItemDelegateForColumn(GEX_TPAT_CONFIG_FIELD_OUTLIER_LIMIT_SET, lDelegateCombo);
        }
        else if(col == GEX_TPAT_CONFIG_FIELD_SBINRULE)
        {
            // Insert dropdown SPAT RuleSet list & set the correct item (as found in table)
            lDelegateCombo = new EditorComboTableDelegate(gexSpatRuleSetItemsGUI, lExcelTableSize, this);
            mExcelTable->setItemDelegateForColumn(GEX_TPAT_CONFIG_FIELD_SBINRULE, lDelegateCombo);

        }
        else if(col == GEX_TPAT_CONFIG_FIELD_RULE)
        {
            // Insert combo box in all tests 'rule-set' column.
            lDelegateCombo = new EditorComboTableDelegate(gexRuleSetItemsGUI, lExcelTableSize, this);
            mExcelTable->setItemDelegateForColumn(GEX_TPAT_CONFIG_FIELD_RULE, lDelegateCombo);

        }
        else if(col == GEX_TPAT_CONFIG_FIELD_SPC_TEST_DRIFT_UNITS)
        {
            // Fill String list with list of 'PAT-Median drift units' & 'Test Mean drift' sets available.
            lDelegateCombo = new EditorComboTableDelegate(gexMedianDriftUnitsSetItemsGUI, lExcelTableSize, this);
            mExcelTable->setItemDelegateForColumn(GEX_TPAT_CONFIG_FIELD_SPC_TEST_DRIFT_UNITS, lDelegateCombo);

        }
        else if(col == GEX_TPAT_CONFIG_FIELD_SPC_PAT_DRIFT_UNITS)
        {
            // Fill String list with list of 'PAT-Median drift units' & 'Test Mean drift' sets available.
            lDelegateCombo = new EditorComboTableDelegate(gexMedianDriftUnitsSetItemsGUI, lExcelTableSize, this);
            mExcelTable->setItemDelegateForColumn(GEX_TPAT_CONFIG_FIELD_SPC_PAT_DRIFT_UNITS, lDelegateCombo);

        } else if(col == GEX_TPAT_CONFIG_FIELD_NNR_RULE)
        {
            // Insert combo box in all tests 'NNR' column.
            lDelegateCombo = new EditorComboTableDelegate(gexNnrRuleSetItemsGUI, lExcelTableSize, this);
            mExcelTable->setItemDelegateForColumn(GEX_TPAT_CONFIG_FIELD_NNR_RULE, lDelegateCombo);
        }
        else
            return ;
    }

    // Check how to optimze this in a separate case.
    // mExcelTable->openPersistentEditor(mExcelTable->item(row, col));
    */
}

void PATRecipeEditor::OnRuleContextualMenu(const QPoint& /*pos*/)
{
//    "contextual menu on Automatic Rule item:
//     - Add new custom rule
//     - Duplicate rule "

//    "contextual menu on custom Rule item:
//     - Add new custom rule
//     - Duplicate rule
//     - Edit test list
//     - Remove custom rule"

//    "Contextual menu on Test item:
//     - Add new rule
//     - Edit test list
//     - Remove test"

//    "Short cut available on Custom Rule
//     - del ? remove the rule
//     - enter ? edit the test list"

//    "Short cut available on Test item in Custom Rule
//     - del ? remove the test
//    "

    QList<QTreeWidgetItem*> lSelected = mMultivariateTree->selectedItems();
    if(lSelected.isEmpty())
        return;

    QTreeWidgetItem *lItem = lSelected.first();
    bool bRuleItem = false;
    bool bTestItem = false;

    if(mMultivariateTree->indexOfTopLevelItem(lItem)!=-1)
        bRuleItem = true;
    QTreeWidgetItem *lItemParent = lItem->parent();
    if(lItemParent)
    {
        bTestItem = true;
    }

    QAction *lAddNewRule = 0;
    QAction *lDuplicateRule = 0;
    QAction *lEditTestList = 0;
    QAction *lRemoveCustomRule = 0;
    QAction *lRemoveTest = 0;

    QMenu lMenu(this);
    lAddNewRule = lMenu.addAction("Add new custom rule");
    lDuplicateRule = lMenu.addAction("Duplicate rule");

    if(bRuleItem)
    {
        if(lItem->text(mPrivate->GetMVFieldColmun("Type")) == "Manual")
        {
            lEditTestList = lMenu.addAction("Edit test list");
            lRemoveCustomRule = lMenu.addAction("Remove custom rule");

        }
    }
    else if(bTestItem)
    {
        if(lItemParent->text(mPrivate->GetMVFieldColmun("Type")) == "Manual")
        {
            lEditTestList = lMenu.addAction("Edit test list");
            lRemoveTest = lMenu.addAction("Remove test");
        }
    }

    lMenu.setMouseTracking(true);
    QAction *lAction = lMenu.exec(QCursor::pos());
    lMenu.setMouseTracking(true);

    if(lAction == lAddNewRule)
    {
        AddNewRule();
    }
    if(lAction == lDuplicateRule)
    {
        DuplicateRule();
    }
    if(lAction == lEditTestList)
    {
        EditTestList();
    }
    if(lAction == lRemoveCustomRule)
    {
        DeleteRule();
    }
    if(lAction == lRemoveTest)
    {
        RemoveTestFromRule(lItemParent, lItem);
    }
}


///////////////////////////////////////////////////////////
// Contextual menu Table clicked
//////////////////////////////////////////////////////////////////////
void PATRecipeEditor::OnContextualMenu(const QPoint& /*pos*/)
{

    QAction *	pActionForceStaticPat				= NULL;
    QAction *	pActionClearStaticPat				= NULL;
    QAction *	pActionForceDynamicPat				= NULL;
    QAction *	pActionClearDynamicPat				= NULL;
    QAction *	pActionSelect						= NULL;
    QAction *	pActionProperties					= NULL;
    QAction *	pActionSave							= NULL;
    QAction *	pActionDeleteLines					= NULL;
    QAction *	pActionDeleteCols					= NULL;
    QAction *	pActionForceFactor					= NULL;
    QAction *	pActionClearFactor					= NULL;
    QAction *	pActionForceCpkAlarm				= NULL;
    QAction *	pActionClearCpkAlarm				= NULL;
    QAction *	pActionForcePatMedianDriftAlarm		= NULL;
    QAction *	pActionClearPatMedianDriftAlarm		= NULL;
    QAction *	pActionForceTestMeanDriftAlarm		= NULL;
    QAction *	pActionClearTestMeanDriftAlarm		= NULL;
    QAction *	pActionDisableAllTests				= NULL;
    QAction *	pActionDisableTestsWithoutLimits	= NULL;
    QAction *   pActionEditTests					= NULL;
    QList<QAction*> lActionOutliersRules;
    QList<QAction*> lActionTailManagement;
    QString		lTmpStr;

    QMenu		menu(this);

    int row = mUnivariateTree->currentRow();
    int col = mUnivariateTree->currentColumn();

    // Build menu.
    pActionSelect					= menu.addAction(*pixOpen,"Select file to edit...");
    pActionProperties				= menu.addAction(*pixProperties,"Properties...");
    pActionSave						= menu.addAction(*pixSave,"Save edits to spreadsheet CSV file");

    // If PAT editor, and rules selected, allow to toggle selection...
    int	iIndex;
    int	iRuleCol=-1;

    int lSettingsType = -1;

    if (mPATSettings->currentIndex() >= 0)
        lSettingsType = mPATSettings->itemData(mPATSettings->currentIndex()).toInt();

    menu.addSeparator();
    // Fill list of toggles available.
    //col = mPrivate->GetFieldColmun("Static Bin",PATRecipeEditor::Basic)

    if(col == mPrivate->GetFieldColumn("Static Bin",PATRecipeEditor::Basic))
    {
        pActionForceStaticPat = menu.addAction("Force 'Static PAT-Bin' to...");
        pActionClearStaticPat = menu.addAction("Disable cells from 'Static PAT'");
    }
    else if(col == mPrivate->GetFieldColumn("SPAT rule",PATRecipeEditor::SPAT))
    {
        for(iIndex = 0; gexSpatRuleSetItemsGUI[iIndex]; iIndex++)
        {
            lTmpStr = "SPAT Outlier rule: ";
            lTmpStr += gexSpatRuleSetItemsGUI[iIndex];
            if(iIndex != GEX_TPAT_RULETYPE_SEPARATORID)
                menu.addAction(lTmpStr);
        }

    }
    else if(col == mPrivate->GetFieldColumn("Dyn. Bin",PATRecipeEditor::Basic))
    {
        pActionForceDynamicPat = menu.addAction("Force 'Dynamic PAT-Bin' to...");
        pActionClearDynamicPat = menu.addAction("Disable cells from 'Dynamic PAT'");
    }
    else if(col == mPrivate->GetFieldColumn("Samples to ignore",PATRecipeEditor::DPAT))
    {
        for(iIndex = 0; gexIgnoreSamplesSetItemsGUI[iIndex]; iIndex++)
        {
            lTmpStr = "Samples to ignore: ";
            lTmpStr += gexIgnoreSamplesSetItemsGUI[iIndex];
            menu.addAction(lTmpStr);
        }
    }
    else if(col == mPrivate->GetFieldColumn("Outliers to keep",PATRecipeEditor::DPAT))
    {
        for(iIndex = 0; gexKeepOutliersSetItemsGUI[iIndex]; iIndex++)
        {
            lTmpStr = "Outliers to keep: ";
            lTmpStr += gexKeepOutliersSetItemsGUI[iIndex];
            menu.addAction(lTmpStr);
        }
    }
    else if(col == mPrivate->GetFieldColumn("Outlier Limits set",PATRecipeEditor::DPAT))
    {
        for(iIndex = 0; gexOutlierLimitsSetItemsLongNamesGUI[iIndex]; iIndex++)
        {
            lTmpStr = "Outlier Limits set: ";
            lTmpStr += gexOutlierLimitsSetItemsLongNamesGUI[iIndex];
            menu.addAction(lTmpStr);
        }
    }
    else if((col == mPrivate->GetFieldColumn("N Factor",PATRecipeEditor::DPAT))
            || (col == mPrivate->GetFieldColumn("T Factor",PATRecipeEditor::DPAT)))
    {
        pActionForceFactor = menu.addAction("Force 'Factor' to...");
        pActionClearFactor = menu.addAction("Clear 'Factor' cells");
    }
    else if(col == mPrivate->GetFieldColumn("PAT-Median Drift alarm",PATRecipeEditor::SPC))
    {
        pActionForcePatMedianDriftAlarm = menu.addAction("Force 'PAT-Median drift alarm' to...");
        pActionClearPatMedianDriftAlarm = menu.addAction("Clear 'PAT-Median drift alarm' cells");
    }
    else if(col == mPrivate->GetFieldColumn("PAT-Median Drift units",PATRecipeEditor::SPC))
    {
        for(iIndex = 0; gexMedianDriftUnitsSetItemsGUI[iIndex]; iIndex++)
        {
            lTmpStr = "Median drift units: ";
            lTmpStr += gexMedianDriftUnitsSetItemsGUI[iIndex];
            menu.addAction(lTmpStr);
        }

    }
    else  if(col == mPrivate->GetFieldColumn("Test Mean Drift alarm",PATRecipeEditor::SPC))
    {
        pActionForceTestMeanDriftAlarm = menu.addAction("Force 'Test Mean drift alarm' to...");
        pActionClearTestMeanDriftAlarm = menu.addAction("Clear 'Test Mean drift alarm' cells");
    }
    else  if(col == mPrivate->GetFieldColumn("Test Mean Drift units",PATRecipeEditor::SPC))
    {
        for(iIndex = 0; gexMedianDriftUnitsSetItemsGUI[iIndex]; iIndex++)
        {
            lTmpStr = "Mean drift units: ";
            lTmpStr += gexMedianDriftUnitsSetItemsGUI[iIndex];
            menu.addAction(lTmpStr);
        }
    }
    else if(col == mPrivate->GetFieldColumn("Cpk alarm",PATRecipeEditor::SPC))
    {
        pActionForceCpkAlarm = menu.addAction("Force 'Cpk alarm' to...");
        pActionClearCpkAlarm = menu.addAction("Clear 'Cpk alarm' cells");
    }

    if(lSettingsType  == PATRecipeEditor::DPAT)
    {
        QMenu* lSubMenuOutlierRule = new QMenu("Outlier rule");
        for(iIndex = 0; gexRuleSetItemsGUI[iIndex]; iIndex++)
        {
            lTmpStr = gexRuleSetItemsGUI[iIndex];
            if(iIndex != GEX_TPAT_RULETYPE_SEPARATORID)
            {
                lActionOutliersRules.append(lSubMenuOutlierRule->addAction(lTmpStr));
            }
            menu.addMenu(lSubMenuOutlierRule);
        }
        QMenu* lSubMenuTailMngt = new QMenu("Tail management");
        for(iIndex = 0; gexTailMngtRuleSetItemsGUI[iIndex]; iIndex++)
        {
            lTmpStr = gexTailMngtRuleSetItemsGUI[iIndex];
            lActionTailManagement.append(lSubMenuTailMngt->addAction(lTmpStr));
            menu.addMenu(lSubMenuTailMngt);
        }
    }
    else if(lSettingsType  == PATRecipeEditor::NNR)
    {
        for(iIndex = 0; gexNnrRuleSetItemsGUI[iIndex]; iIndex++)
        {
            lTmpStr = "NNR rule: ";
            lTmpStr += gexNnrRuleSetItemsGUI[iIndex];
            menu.addAction(lTmpStr);
        }
    }



    if(lSettingsType == PATRecipeEditor::SPAT)
    {
        iRuleCol = mPrivate->GetFieldColumn("SPAT rule",PATRecipeEditor::SPAT);
    }
    else if(lSettingsType == PATRecipeEditor::DPAT)
    {
        iRuleCol = mPrivate->GetFieldColumn("DPAT Rule",PATRecipeEditor::DPAT);
    }
    else if(lSettingsType == PATRecipeEditor::NNR)
    {
        iRuleCol = mPrivate->GetFieldColumn("NNR Rule",PATRecipeEditor::NNR);
    }

    if(iRuleCol >= 0)
    {
        // Global short-cuts....
        pActionDisableAllTests = menu.addAction(*pixRemove,"Disable ALL tests");
        pActionDisableTestsWithoutLimits = menu.addAction(*pixRemove,"Disable tests missing both 'Test Limits'");
    }

    // Under PAT-Man editor, the 'Delete line' is only available for the line covering test settings, not the other lines (header & footer XML markers)!
    bool	bDeleteItem=true;

//    if( sToolBoxTask == GEXTB_EDIT_PAT_WIZARD_P1
//            && (row <= mLineTestNumberParameter || row > mLastTestNumberParameterLine))
//        bDeleteItem = false;

    // Advanced proerties...
//    if(sToolBoxTask != GEXTB_EDIT_PAT_WIZARD_P1)
//    {
//        menu.addSeparator();
//        pActionEditTests = menu.addAction("Edit Test(s)...", this, SLOT(onEditTests()));
//        pSortByPartID = menu.addAction("Sort By Part-ID");
//    }

    if(bDeleteItem)
    {
        menu.addSeparator();
        pActionDeleteLines = menu.addAction(*pixRemove,"Delete selected rows");
    }

    // Toolbox (except whan in PAT Editor mode) accepts to add test and delete columns
//    if(sToolBoxTask != GEXTB_EDIT_PAT_WIZARD_P1)
//    {
//        pActionDeleteCols = menu.addAction(*pixRemove,"Delete selected columns");
//        menu.addAction("Add Test before selection...", this, SLOT(onAddTestBeforeSelected()));
//        menu.addAction("Add Test after selection...", this, SLOT(onAddTestAfterSelected()));
//    }

    menu.setMouseTracking(true);
    QAction * pActionResult = menu.exec(QCursor::pos());
    menu.setMouseTracking(true);

    // Menu cancelled?
    if(pActionResult == NULL)
        return;

    // Process Scale/Offset parameters
    if (pActionResult == pActionEditTests)
        return;

    // Process menu selection
    if(pActionResult == pActionSelect)
    {
        ReadRecipe();
        return;
    }

    if(pActionResult == pActionProperties)
    {
        OnProperties();
        return;
    }

    if(pActionResult == pActionSave)
    {
        WriteRecipe();
        return;
    }

    if(pActionResult == pActionDeleteLines)
    {
        OnDelete(true);
        return;
    }

    if(pActionResult == pActionDeleteCols)
    {
        OnDelete(false);
        return;
    }

//    if(pActionResult == pSortByPartID)
//    {
//        // Sort by Part ID (ascending)
//        OnSortByPartID(true);
//        return;
//    }

    // PAT: User forces factor (N or T) to all selected lines.
    double	lfForceValue=1.5f;
    int		iForceValue=140;
    bool ok;
    if(pActionResult == pActionForceStaticPat)
    {
        // Force Static PAT Bin to...
        iForceValue = mPrivate->mPATRecipe.GetOptions().iFailStatic_SBin;
        // Get user value
        iForceValue = QInputDialog::getInteger(this,"Set Static PAT-Bin","Enter Bin value (e.g: 14):",iForceValue,
                                               -1, 65535, 1, &ok);
        if(!ok)
            return;
    }

    // Force dynamic PAT bin to selected tests
    if(pActionResult == pActionForceDynamicPat)
    {
        // Force Static PAT Bin to...
        iForceValue = mPrivate->mPATRecipe.GetOptions().iFailDynamic_SBin;
        // Get user value
        iForceValue = QInputDialog::getInteger(this,
                                               "Set Dynamic PAT-Bin", "Enter Bin value (e.g: 14):",iForceValue,-1, 65535, 1, &ok);
        if(!ok)
            return;
    }

    // Forcing a PAT-Median drift alarm (Final Test PAT Monitoring feature)
    if(pActionResult == pActionForcePatMedianDriftAlarm)
    {
        // Default value
        lfForceValue=1.5f;
        // Get user value
        lfForceValue = QInputDialog::getDouble(this, "PAT-Median Drift Alarm",
                                               "Enter PAT-Median drift alarm (e.g: 1.5):", lfForceValue,0.0f, 1.0e99, 2, &ok);
        if(!ok)
            return;
    }
    // Forcing a Test Mean drift alarm (Final Test PAT Monitoring feature)
    if(pActionResult == pActionForceTestMeanDriftAlarm)
    {
        // Default value
        lfForceValue=1.5f;
        // Get user value
        lfForceValue = QInputDialog::getDouble(this, "Test Mean Drift Alarm", "Enter Test Mean drift alarm (e.g: 1.5):",
                                               lfForceValue,0.0f, 1.0e99, 2, &ok);
        if(!ok)
            return;
    }
    // Forcing a Cpk Alarm (Final Test PAT Monitoring feature)
    if(pActionResult == pActionForceCpkAlarm)
    {
        // Default value
        lfForceValue=1.67f;
        // Get user value
        lfForceValue = QInputDialog::getDouble(this, "Cpk Alarm", "Enter Cpk alarm level (e.g: 1.33):",
                                               lfForceValue,0.0f, 200.0f, 2, &ok);
        if(!ok)
            return;
    }

    // Forcing a Cpk Alarm (Final Test PAT Monitoring feature)
    if(pActionResult == pActionForceFactor)
    {
        // Default value
        lfForceValue=6.0f;
        // Get user value
        lfForceValue = QInputDialog::getDouble(this, "N/T Factors", "Enter Factor (e.g: 8.00):", lfForceValue,
                                               0.0f, 200.0f, 2, &ok);
        if(!ok)
            return;
    }

    // Disable ALL tests
    if(pActionResult == pActionDisableAllTests)
    {
        int	iTotalDisabled=0;

        // Step#1: count how many tests will be
        for(int lRow=0;lRow < mUnivariateTree->rowCount() ; ++lRow)
        {
            QComboBox *lCombo = NULL;
            lCombo = qobject_cast<QComboBox *>(mUnivariateTree->cellWidget(lRow, iRuleCol));
            if(!lCombo)
                continue;
            // Disable this test!
            if(iRuleCol == mPrivate->GetFieldColumn("SPAT rule",PATRecipeEditor::SPAT))
            {
                //mUnivariateTree->item(lRow, iRuleCol)->setText("-1");
                lCombo->setCurrentIndex(GS::Gex::PAT::GetSPATRuleIndex("Disabled"));
                UpdateSpatGuiFactors(GEX_TPAT_SPAT_ALGO_IGNORE,lRow,false);
            }
            else if(iRuleCol == mPrivate->GetFieldColumn("DPAT Rule",PATRecipeEditor::DPAT))
            {
                lCombo->setCurrentIndex(GS::Gex::PAT::GetRuleIndex("Disabled"));
                //mUnivariateTree->item(lRow, iRuleCol)->setText("-1");
                UpdateDpatGuiFactors(GEX_TPAT_RULETYPE_IGNOREID,lRow,false);
            }
            else if(iRuleCol == mPrivate->GetFieldColumn("NNR Rule",PATRecipeEditor::NNR))
            {
                lCombo->setCurrentIndex(GS::Gex::PAT::GetNNRRuleIndex("Disabled"));
                UpdateNnrGuiFactors(GEX_TPAT_NNR_DISABLED,lRow);
            }
            // Keep track of total count
            iTotalDisabled++;
        }

        // Work completed, report status.
        if(iTotalDisabled)
        {
            lTmpStr = "Total tests disabled: " + QString::number(iTotalDisabled);
            GS::Gex::Message::information("", lTmpStr);

            // Flag table is modified
            mPrivate->mModified = true;
        }
        else
            GS::Gex::Message::information("", "No test disabled");
        return;
    }

    // Disable ALL tests that are missing both test limits
    if(pActionResult == pActionDisableTestsWithoutLimits)
    {
        QString strLowL,strHighL;
        int	iTotalDisabled=0;

        // Step#1: count how many tests will be
        for(int lRow=0;lRow < mUnivariateTree->rowCount() ; ++lRow)
        {
            // Get Low Limit field
            strLowL = mUnivariateTree->item(lRow,mPrivate->GetFieldColumn("LL.", PATRecipeEditor::Basic))->text().trimmed();

            // Get HighLimit field
            strHighL = mUnivariateTree->item(lRow,mPrivate->GetFieldColumn("HL.", PATRecipeEditor::Basic))->text().trimmed();

            // Check if both limits are missing
            if(strLowL.isEmpty() && strHighL.isEmpty())
            {
                // Disable this test!
                mUnivariateTree->item(lRow,mPrivate->GetFieldColumn("Dyn. Bin" ,PATRecipeEditor::Basic) )->setText("-1");
                UpdateDpatGuiFactors(GEX_TPAT_RULETYPE_IGNOREID,lRow,false);

                // Add comment line
                mUnivariateTree->item(lRow,mPrivate->GetFieldColumn("DPAT Notes",PATRecipeEditor::DPAT))->setText("**Disabled** Because missing test limits");

                // Keep track of total count
                iTotalDisabled++;
            }
        }

        // Work completed, report status.
        if(iTotalDisabled)
        {
            lTmpStr = "Total tests disabled: " + QString::number(iTotalDisabled);
            GS::Gex::Message::information("", lTmpStr);

            // Flag table is modified
            mPrivate->mModified = true;
        }
        else
            GS::Gex::Message::information(
                        "", "No test disabled...they all have test limits!");
        return;
    }

    // Execute below code ONLY if editing PAT recipe...
//    if(sToolBoxTask != GEXTB_EDIT_PAT_WIZARD_P1)
//        return;

    // Toggle selected (Outlier rule, or outlier filter, or data filter,...)
    QString lMenuSelection = pActionResult->text();
    lMenuSelection = lMenuSelection.section(':',1);
    lMenuSelection = lMenuSelection.trimmed();

    int	iSelectionID;

    int lCurrentSettings = mPATSettings->itemData(mPATSettings->currentIndex()).toInt();

    QList<QTableWidgetSelectionRange> selectedRegion = mUnivariateTree->selectedRanges();
    QList<QTableWidgetSelectionRange>::iterator itTableRange;
    for(itTableRange = selectedRegion.begin(); itTableRange != selectedRegion.end(); ++itTableRange)
    {
        int lRow = (*itTableRange).topRow();
        do
        {
            if(!lMenuSelection.isEmpty() && lCurrentSettings  == PATRecipeEditor::SPAT)
            {
                iSelectionID = GS::Gex::PAT::GetSPATRuleIndex(lMenuSelection,NULL);	// Get rule ID associated with the rule string.
                UpdateSpatGuiFactors(iSelectionID,lRow,false);
                QComboBox *lCombo = qobject_cast<QComboBox *>(mUnivariateTree->cellWidget(lRow, mPrivate->GetFieldColumn("SPAT rule",PATRecipeEditor::SPAT)));
                if(lCombo)
                {
                    lCombo->setCurrentIndex(iSelectionID);
                }
            }
            if(lCurrentSettings  == PATRecipeEditor::DPAT)
            {
                // Update outlier rules if needed
                if (lActionOutliersRules.contains(pActionResult))
                {
                    iSelectionID = GS::Gex::PAT::GetRuleIndex(
                                pActionResult->text().trimmed(),
                                NULL);
                    UpdateDpatGuiFactors(iSelectionID,lRow,false);
                    QComboBox *lCombo = qobject_cast<QComboBox *>(
                                mUnivariateTree->cellWidget(
                                    lRow,
                                    mPrivate->GetFieldColumn("DPAT Rule",PATRecipeEditor::DPAT))
                                );
                    if(lCombo)
                    {
                        lCombo->setCurrentIndex(iSelectionID);
                    }
                }
                // Update tail management rules if needed
                // Do not change if not smart & adaptive
                else if (lActionTailManagement.contains(pActionResult))
                {
                    QComboBox *lDPatCombo = qobject_cast<QComboBox *>(
                                mUnivariateTree->cellWidget(
                                    lRow,
                                    mPrivate->GetFieldColumn("DPAT Rule",PATRecipeEditor::DPAT))
                                );
                    if (lDPatCombo)
                    {
                        int lRuleType = lDPatCombo->currentIndex();
                        if (lRuleType == GEX_TPAT_RULETYPE_SMARTID)
                        {
                            iSelectionID = GS::Gex::PAT::GetTailMngtRuleIndex(
                                        pActionResult->text().trimmed(),
                                        NULL);
                            QComboBox *lCombo = qobject_cast<QComboBox *>(
                                        mUnivariateTree->cellWidget(
                                            lRow,
                                            mPrivate->GetFieldColumn("Tail management",PATRecipeEditor::DPAT))
                                        );
                            if(lCombo)
                            {
                                lCombo->setCurrentIndex(iSelectionID);
                            }
                        }
                    }
                }
            }
            if(!lMenuSelection.isEmpty() && lCurrentSettings  == PATRecipeEditor::NNR)
            {
//                case GEX_TPAT_CONFIG_FIELD_NNR_RULE:
                // Create combo item list to insert into cell
                iSelectionID	= GS::Gex::PAT::GetNNRRuleIndex(lMenuSelection,NULL);	// Get selection ID
                UpdateNnrGuiFactors(iSelectionID,lRow);
                QComboBox *lCombo = qobject_cast<QComboBox *>(mUnivariateTree->cellWidget(lRow, mPrivate->GetFieldColumn("NNR Rule",PATRecipeEditor::NNR)));
                if(lCombo)
                {
                    lCombo->setCurrentIndex(iSelectionID);
                }
            }

//            if ((lRow > mLineTestNumberParameter) && (lRow <= mLastTestNumberParameterLine))
//            {
                // check if we ignore those line (if starts with a '#')
//                strString = mExcelTable->item(lRow,0)->text();
//                if(strString.startsWith("#"))
//                    continue;

                //QStandardItemModel *model = (QStandardItemModel*)mUnivariateTree->model();
                //QModelIndex index = model->index(lRow, col, QModelIndex());
                //mPrivate->GetFieldColmun("DPAT Notes",PATRecipeEditor::DPAT)
                if(col == mPrivate->GetFieldColumn("Static Bin", PATRecipeEditor::Basic))
                {
//                case GEX_TPAT_CONFIG_FIELD_SBIN:
                    // Insert/Erase Factor value.
                    if(pActionResult == pActionForceStaticPat)
                        mUnivariateTree->item(lRow,col)->setText(QString::number(iForceValue));
                    else
                        if(pActionResult == pActionClearStaticPat)
                            mUnivariateTree->item(lRow,col)->setText("-1");
                    // Flag table is modified
                    mPrivate->mModified = true;
                }

                    // SPAT custom N and T factors: do nothing!
//                case GEX_TPAT_CONFIG_FIELD_SPAT_NFACTOR:
//                case GEX_TPAT_CONFIG_FIELD_SPAT_TFACTOR:
//                    break;
                if(col == mPrivate->GetFieldColumn("SPAT rule", PATRecipeEditor::SPAT))
                {
//                case GEX_TPAT_CONFIG_FIELD_SBINRULE:
                    // Create combo item list to insert into cell
                    iSelectionID = GS::Gex::PAT::GetSPATRuleIndex(lMenuSelection,NULL);
                    //model->setData(index,  QVariant(gexSpatRuleSetItemsGUI[iSelectionID]));
                    // Flag table is modified
                    mPrivate->mModified = true;
                }
                if(col == mPrivate->GetFieldColumn("Dyn. Bin", PATRecipeEditor::DPAT))
                {
//                case GEX_TPAT_CONFIG_FIELD_DBIN:
                    //model->setData(index,  QVariant(""));
                    // Insert/Erase Factor value.
                    if(!mUnivariateTree->item(lRow,col))
                        mUnivariateTree->setItem(lRow,col, new QTableWidgetItem());
                    if(pActionResult == pActionForceDynamicPat)
                        mUnivariateTree->item(lRow,col)->setText(QString::number(iForceValue));
                    else
                        if(pActionResult == pActionClearDynamicPat)
                        {
                            mUnivariateTree->item(lRow,col)->setText("-1");
                            UpdateDpatGuiFactors(GEX_TPAT_RULETYPE_IGNOREID,row,false);
                        }
                    // Flag table is modified
                    mPrivate->mModified = true;
//                    break;
                }
                if(col == mPrivate->GetFieldColumn("Samples to ignore", PATRecipeEditor::Basic))
                {
//                case GEX_TPAT_CONFIG_FIELD_IGNOREDATA:
                    // Create combo item list to insert into cell
                    iSelectionID	= GS::Gex::PAT::GetSamplesToIgnoreIndex(lMenuSelection,NULL);	// Get selection ID
                    //model->setData(index,  QVariant(gexIgnoreSamplesSetItemsGUI[iSelectionID]));
                    // Flag table is modified
                    mPrivate->mModified = true;
//                    break;
                }
                if(col == mPrivate->GetFieldColumn("Outliers to keep", PATRecipeEditor::DPAT))
                {
//                case GEX_TPAT_CONFIG_FIELD_KEEP:
                    // Create combo item list to insert into cell
                    iSelectionID	= GS::Gex::PAT::GetOutlierToKeepIndex(lMenuSelection,NULL);	// Get selection ID
                    //model->setData(index,  QVariant(gexKeepOutliersSetItemsGUI[iSelectionID]));
                    // Flag table is modified
                    mPrivate->mModified = true;
//                    break;
                }
                if(col == mPrivate->GetFieldColumn("Outlier Limits set", PATRecipeEditor::DPAT))
                {
//                case GEX_TPAT_CONFIG_FIELD_OUTLIER_LIMIT_SET:
                    // Create combo item list to insert into cell
                    iSelectionID	= GS::Gex::PAT::GetOutlierLimitsSetIndex(lMenuSelection,NULL);	// Get selection ID
                    //model->setData(index,  QVariant(gexOutlierLimitsSetItemsLongNamesGUI[iSelectionID]));
                    // Flag table is modified
                    mPrivate->mModified = true;
//                    break;
                }
                if(col == mPrivate->GetFieldColumn("N Factor", PATRecipeEditor::DPAT)
                        || col == mPrivate->GetFieldColumn("T Factor", PATRecipeEditor::DPAT ))
                {
//                case GEX_TPAT_CONFIG_FIELD_NFACTOR:
//                case GEX_TPAT_CONFIG_FIELD_TFACTOR:
                    //model->setData(index,  QVariant(""));
                    // Insert/Erase Factor value.
                    if(!mUnivariateTree->item(lRow,col))
                        mUnivariateTree->setItem(lRow,col, new QTableWidgetItem());
                    if(pActionResult == pActionForceFactor)
                        mUnivariateTree->item(lRow,col)->setText(QString::number(lfForceValue));
                    else
                        if(pActionResult == pActionClearFactor)
                            mUnivariateTree->item(lRow,col)->setText("");
                    // Flag table is modified
                    mPrivate->mModified = true;
//                    break;
                }
                if(col == mPrivate->GetFieldColumn("PAT-Median Drift alarm", PATRecipeEditor::SPC)
                        || col == mPrivate->GetFieldColumn("Test Mean Drift alarm", PATRecipeEditor::SPC))
                {
//                case GEX_TPAT_CONFIG_FIELD_SPC_PAT_DRIFT:
//                case GEX_TPAT_CONFIG_FIELD_SPC_TEST_DRIFT:
                    // Insert/Erase Factor value.
                    if(!mUnivariateTree->item(lRow,col))
                        mUnivariateTree->setItem(lRow,col, new QTableWidgetItem());

                    if( pActionResult == pActionForcePatMedianDriftAlarm
                            || pActionResult == pActionForceTestMeanDriftAlarm)
                        mUnivariateTree->item(lRow,col)->setText(QString::number(lfForceValue));
                    else if( pActionResult == pActionClearPatMedianDriftAlarm
                             || pActionResult == pActionClearTestMeanDriftAlarm)
                    {
                        mUnivariateTree->item(lRow,col)->setText("");
                        // Show dropdown list on table cell.
                        if(col == mPrivate->GetFieldColumn("PAT-Median Drift alarm", PATRecipeEditor::SPC))
                        {
                            QComboBox *lCombo = qobject_cast<QComboBox *>(mUnivariateTree->cellWidget(lRow,mPrivate->GetFieldColumn("PAT-Median Drift alarm", PATRecipeEditor::SPC)));
                            if(lCombo)
                                lCombo->setCurrentIndex(GEX_TPAT_DRIFT_UNITS_NONE);
//                            QModelIndex indexTPatConfig = model->index(lRow, col, QModelIndex());
//                            model->setData(indexTPatConfig,  QVariant(gexMedianDriftUnitsSetItemsGUI[GEX_TPAT_DRIFT_UNITS_NONE]));
                        }
                        else if(col == mPrivate->GetFieldColumn("Test Mean Drift alarm", PATRecipeEditor::SPC))
                        {
                            QComboBox *lCombo = qobject_cast<QComboBox *>(mUnivariateTree->cellWidget(lRow,mPrivate->GetFieldColumn("Test Mean Drift alarm", PATRecipeEditor::SPC)));
                            if(lCombo)
                                lCombo->setCurrentIndex(GEX_TPAT_DRIFT_UNITS_NONE);
//                            QModelIndex indexTPatConfig = model->index(lRow, col, QModelIndex());
//                            model->setData(indexTPatConfig,  QVariant(gexMedianDriftUnitsSetItemsGUI[GEX_TPAT_DRIFT_UNITS_NONE]));
                        }

                    }
                    // Flag table is modified
                    mPrivate->mModified = true;
//                    break;
                }
                //col == mPrivate->GetFieldColmun("PAT-Median Drift alarm", PATRecipeEditor::SPC)
                if((col == mPrivate->GetFieldColumn("PAT-Median Drift units", PATRecipeEditor::SPC))
                        ||(col == mPrivate->GetFieldColumn("Test Mean Drift units", PATRecipeEditor::SPC)))
                {
//                case GEX_TPAT_CONFIG_FIELD_SPC_PAT_DRIFT_UNITS:
//                case GEX_TPAT_CONFIG_FIELD_SPC_TEST_DRIFT_UNITS:
                    // Create combo item list to insert into cell
                    iSelectionID = GS::Gex::PAT::GetMedianDriftUnitsIndex(lMenuSelection,NULL);	// Get selection ID
                    //model->setData(index,  QVariant(gexMedianDriftUnitsSetItemsGUI[iSelectionID]));
                    // Clear 'Drift alarm' field if no units defined.
                    if(iSelectionID == GEX_TPAT_DRIFT_UNITS_NONE)
                    {
                        if(col == mPrivate->GetFieldColumn("PAT-Median Drift units", PATRecipeEditor::SPC))
                        {
                            QComboBox *lCombo = qobject_cast<QComboBox*>(mUnivariateTree->cellWidget(lRow,mPrivate->GetFieldColumn("PAT-Median Drift units", PATRecipeEditor::SPC)));
                            if(lCombo)
                            {
                                lCombo->setCurrentIndex(GS::Gex::PAT::GetMedianDriftUnitsIndex("None"));
                            }
                            //mUnivariateTree->item(lRow,mPrivate->GetFieldColmun("PAT-Median Drift units", PATRecipeEditor::SPC))->setText("");
                        }
                        else if(col == mPrivate->GetFieldColumn("Test Mean Drift units", PATRecipeEditor::SPC))
                        {
                            QComboBox *lCombo = qobject_cast<QComboBox*>(mUnivariateTree->cellWidget(lRow,mPrivate->GetFieldColumn("Test Mean Drift units", PATRecipeEditor::SPC)));
                            if(lCombo)
                            {
                                //gexMedianDriftUnitsSetItemsGUI
                                lCombo->setCurrentIndex(GS::Gex::PAT::GetMedianDriftUnitsIndex("None"));
                            }
                            //mUnivariateTree->item(lRow,mPrivate->GetFieldColmun("Test Mean Drift units", PATRecipeEditor::SPC))->setText("");
                        }
                    }
                    // Flag table is modified
                    mPrivate->mModified = true;
//                    break;
                }
                if(col == mPrivate->GetFieldColumn("Cpk alarm", PATRecipeEditor::SPC))
                {
//                case GEX_TPAT_CONFIG_FIELD_SPC_TEST_CPK:
                    // Insert/Erase Factor value.
                    if(pActionResult == pActionForceCpkAlarm)
                        mUnivariateTree->item(lRow,col)->setText(QString::number(lfForceValue));
                    else
                        if(pActionResult == pActionClearCpkAlarm)
                            mUnivariateTree->item(lRow,col)->setText("");
                    // Flag table is modified
                    mPrivate->mModified = true;
//                    break;
                }
//                }
            //}
            ++ lRow;
        }
        while (lRow <= (*itTableRange).bottomRow());
    }


}

///////////////////////////////////////////////////////////
// Scale limit according to scale factor. Reslt in QString.
///////////////////////////////////////////////////////////
void PATRecipeEditorPrivate::ScaleLimits(QString &strScaledLimit,double &lfValue,int iPower,char cSeparator,QString &strUnits)
{
    char cScaleUnits=0;
    switch(iPower)
    {
        default:
            lfValue *=GS_POW(10.0,iPower);
            break;

        case 0:
            break;

        case 253:	// for unsigned -3
        case -3:
            cScaleUnits = 'K';
            lfValue *=1e-3;
            break;
        case 250:	// for unsigned -6
        case -6:
            cScaleUnits = 'M';
            lfValue *=1e-6;
            break;
        case 247:	// for unsigned -9
        case -9:
            cScaleUnits = 'G';
            lfValue *=1e-9;
            break;
        case 244:	// for unsigned -13
        case -12:
            cScaleUnits = 'T';
            lfValue *=1e-12;
            break;
        case 2:
            if(strUnits[0] != '%')
                cScaleUnits = '%';
            lfValue *=1e2;
            break;
        case 3:
            cScaleUnits = 'm';
            lfValue *=1e3;
            break;
        case 6:
            cScaleUnits = 'u';
            lfValue *=1e6;
            break;
        case 9:
            cScaleUnits = 'n';
            lfValue *=1e9;
            break;
        case 12:
            cScaleUnits = 'p';
            lfValue *=1e12;
            break;
        case 15:
            cScaleUnits = 'f';
            lfValue *=1e15;
            break;
    }

    if(cScaleUnits)
        strScaledLimit.sprintf("%g%c%c",lfValue,cSeparator,cScaleUnits);
    else
        strScaledLimit.sprintf("%g%c",lfValue,cSeparator);

    strScaledLimit += strUnits;
}

void	PATRecipeEditor::OnAddMVRules()
{
    // Get data files selected (historical data files)
    QStringList::Iterator   it;
    QStringList             lFilesSelected;
    GS::Gex::PATRecipeHistoricalDataGui lDataSourceDialog(this);

    lDataSourceDialog.setWindowTitle("Select Historical Data for Multi-Variate rules creation");

    if (mPrivate->mPATRecipe.GetOptions().GetRecipeType() == GS::Gex::PAT::RecipeWaferSort)
    {
        lDataSourceDialog.SetTestingStage("Wafer Sort");
    }
    else if (mPrivate->mPATRecipe.GetOptions().GetRecipeType() == GS::Gex::PAT::RecipeFinalTest)
    {
        lDataSourceDialog.SetTestingStage("Final Test");
    }
    else
    {
        GS::Gex::Message::warning("", QString("Unknown testing stage defined in the recipe"));
        return;
    }

    lDataSourceDialog.SetTestKeyEnabled(false);
    switch (mPrivate->mPATRecipe.GetOptions().mTestKey)
    {
        case GEX_TBPAT_KEY_TESTNUMBER:
            lDataSourceDialog.SetTestKey(PATRecipeHistoricalDataGui::TestNumber);
            break;

        case GEX_TBPAT_KEY_TESTNAME:
            lDataSourceDialog.SetTestKey(PATRecipeHistoricalDataGui::TestName);
            break;

        case GEX_TBPAT_KEY_TESTMIX:
            lDataSourceDialog.SetTestKey(PATRecipeHistoricalDataGui::TestMix);
            break;

        default:
            break;
    }

    lDataSourceDialog.SetDataSource(GS::Gex::PATRecipeHistoricalDataGui::DataFromFiles);

    if (lDataSourceDialog.exec() == QDialog::Rejected)
        return;

    // Get historical data files selected
    lFilesSelected = lDataSourceDialog.GetHistoricalData();

    if(lFilesSelected.isEmpty() == TRUE)
    {
        GS::Gex::Message::warning("", QString("No historical data selected"));
        return;	// Empty list...ignore task!
    }

    // Filter data?
    QMessageBox mb(
        GS::Gex::Engine::GetInstance().Get("AppFullName").toString(),
        "Read all the data files or filter samples?\nYou can ignore/filter some parts or bins if needed...\n\nE.g.: Use GOOD parts only.",
    QMessageBox::Question,
    QMessageBox::Yes | QMessageBox::Escape | QMessageBox::Default,
    QMessageBox::No,
    QMessageBox::NoButton,this);
    mb.setWindowIcon(QPixmap(":/gex/icons/gex_application_48x48.png"));
    mb.setButtonText( QMessageBox::Yes, "Read ALL" );
    mb.setButtonText( QMessageBox::No, "Filter data..." );

    // User want to select a filter...
    QString strFilterType="all";
    QString	strFilterList="";
    if (mb.exec() == QMessageBox::No)
    {
        // Show selection made...
        FilterDialog *dFilter = new FilterDialog();
        dFilter->SetParameters(0,false,0,NULL,
                               GEX_FILE_FILTER_NOSITE |
                               GEX_FILE_FILTER_NOGROUPNAME |
                               GEX_FILE_FILTER_NOADVANCEDTAB);
        dFilter->adjustSize();

        // Prompt Filter dialog box
        if(dFilter->exec() == 1)
        {
            // Extract filer info
            strFilterType = gexFileProcessPartsItems[dFilter->comboProcess->currentIndex()];
            strFilterList = dFilter->editProcess->text();
        }
    }

    // Get files analysed: create script file and have it executed!
    QString	strFileName;

    // Create script that will read data file + compute all statistics (but NO report created)
    QString strErrorMessage;
    FILE *hFile = fopen(GS::Gex::Engine::GetInstance().GetAssistantScript().toLatin1().constData(),"w");

    if(hFile == NULL)
    {
        strErrorMessage = "  > Failed to create script file: " + GS::Gex::Engine::GetInstance().GetAssistantScript();
        GS::Gex::Message::information("", strErrorMessage);
        return;
    }

    // Creates 'SetOptions' section
    if(!ReportOptions.WriteOptionSectionToFile(hFile))
    {
        strErrorMessage=QString("error : can't write option section");
        GSLOG(3, "WriteOptionSectionToFile failed");
        return;
    }

    // Creates 'SetProcessData' section
    fprintf(hFile,"SetProcessData()\n");
    fprintf(hFile,"{\n");
    fprintf(hFile,"  var group_id;\n");
    fprintf(hFile,"  gexGroup('reset','all');\n");
    fprintf(hFile,"  gexQuery('db_report','DataSet_1');\n");
    fprintf(hFile,"  group_id = gexGroup('insert','DataSet_1');\n");

    // Write all files to merge in analysis
    for (it = lFilesSelected.begin(); it != lFilesSelected.end(); ++it )
    {
        // Get file name
        strFileName = *it;
        ConvertToScriptString(strFileName);	// Make sure any '\' in string is doubled.

        // Write script line
        fprintf(hFile,"  gexFile(group_id,'insert','%s','All','%s',' %s','','','','');\n",
                strFileName.toLatin1().constData(),
                strFilterType.toLatin1().constData(),
                strFilterList.toLatin1().constData());
    }
    fprintf(hFile,"}\n\n");

    // Creates 'main' section
    fprintf(hFile,"main()\n");
    fprintf(hFile,"{\n");
    fprintf(hFile,"  SetOptions();\n");

    // Ensure continue on fail
    fprintf(hFile,"  gexOptions('dataprocessing', 'fail_count', 'all');\n");

    fprintf(hFile,"  gexOptions('dataprocessing','used_limits','standard_limits_only');\n");
    // Default: keep test values as is (no scaling)
    fprintf(hFile,"  gexOptions('dataprocessing', 'scaling', 'normalized');\n");
    fprintf(hFile,"  gexOptions('output','format','interactive');\n");                  // Avoids editor to be launched if output options is set to Word or SPreadsheet CSV!
    fprintf(hFile,"  gexOptions('dataprocessing', 'duplicate_test', 'merge');\n");      // Merge tests with same test number (even if test number is different)
    fprintf(hFile,"  gexOptions('speed','adv_stats','always');\n");                     // Always compute Quartiles & Skew & Kurtosis.
    fprintf(hFile,"  gexOptions('statistics','computation','samples_only');\n");        // Force to compute statistics from samples, ignore summary.

    fprintf(hFile,"  gexOptions('dataprocessing','data_cleaning_mode','none');\n");   // Disable outlier removal so to make sure all values are taken into account.
    fprintf(hFile,"  SetProcessData();\n");
    fprintf(hFile,"  gexOptions('report','build','false');\n");	// Only data analysis, no report created!
    fprintf(hFile,"  gexBuildReport('none','0');\n");		// Build report.
    fprintf(hFile,"}\n\n");
    fclose(hFile);

    // Progress bar...
    QProgressDialog progress( "Reading data...", "Abort", 0, 1,this);
    progress.setModal(true);
    progress.show();

    // Execute script.
    if (GS::Gex::CSLEngine::GetInstance().RunScript(GS::Gex::Engine::GetInstance().GetAssistantScript()).IsFailed())
        return;

    // Identify all tests not already in the recipe list....
    CGexGroupOfFiles * pGroup = gexReport->getGroupsList().isEmpty() ? NULL : gexReport->getGroupsList().first();
    if(pGroup == NULL)
        return;

    CGexFileInGroup *   pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
    if(pFile == NULL)
        return;

    // IDendify list of tests not in recipe.
    CTest *             ptTestCell  = pGroup->cMergedData.ptMergedTestList;
    CPatDefinition *    lPatDef     = NULL;
    QList<TestInfo>     lTestList;
    TestInfo            lTestInfo;
    while(ptTestCell)
    {
        // Check if this test has valid data...
        // Or, if Muti-result parametric test, do not show master test record
        // Or, if not a parametric / multiparametric (eg: functional) test, ignore!
        if ( (ptTestCell->ldExecs == 0) || (ptTestCell->lResultArraySize > 0) ||
             (ptTestCell->bTestType == 'F' || ptTestCell->bTestType == '-'))
        {
            ptTestCell = ptTestCell->GetNextTest();
            continue;
        }

        // Check if this test is already in recipe
        lPatDef = mPrivate->mPATRecipe.FindUniVariateRule(ptTestCell->lTestNumber,
                                                          ptTestCell->lPinmapIndex, ptTestCell->strTestName);
        if(lPatDef)
        {
            lTestInfo.mNumber   = ptTestCell->lTestNumber;
            lTestInfo.mName     = ptTestCell->strTestName;
            lTestInfo.mPinIndex = ptTestCell->lPinmapIndex;

            lTestList.append(lTestInfo);
        }

        ptTestCell = ptTestCell->GetNextTest();
    };

    progress.hide();

    // Keep existing MV Rule
    QList<PATMultiVariateRule> lOldMVRule = mPrivate->mPATRecipe.GetMultiVariateRules();

    // Compute new MV Rules
    QString lAppDir = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
    QString lError;
    GS::SE::StatsEngine * lStatsEngine = GS::SE::StatsEngine::GetInstance(lAppDir, lError);
    if (lStatsEngine)
    {
        pGexMainWindow->ComputeMVPATGroups(lTestList, mPrivate->mPATRecipe, lStatsEngine);
        GS::SE::StatsEngine::ReleaseInstance();
    }
    else
    {
        GSLOG(SYSLOG_SEV_CRITICAL, QString("Unable to instantiate StatsEngine: %1. Application will now exit.")
              .arg(lError).toLatin1().data());
        exit(EXIT_FAILURE);
    }

    // Append custom rule to the new Generated MV Rule
    for (int lIdx = 0; lIdx < lOldMVRule.count(); ++lIdx)
    {
        if (lOldMVRule.at(lIdx).GetType() == PATMultiVariateRule::Manual)
            mPrivate->mPATRecipe.GetMultiVariateRules().append(lOldMVRule.at(lIdx));
    }

    // Clear current MV Tab
    mMultivariateTree->clear();
    mPrivate->mPatRuleMapping.clear();

    // Refresh the MV Tab
    UpdateMVTab();
}

///////////////////////////////////////////////////////////
// Add tests to current recipe
// Typically used to update recipe with new tests (from new files)
///////////////////////////////////////////////////////////
void	PATRecipeEditor::OnAddPatTests()
{
    // Get data files selected (historical data files)
    QStringList::Iterator   it;
    QStringList             lFilesSelected;
    GS::Gex::PATRecipeHistoricalDataGui lDataSourceDialog(this);

    lDataSourceDialog.setWindowTitle("Select Historical Data for Uni-Variate rules creation");

    if (mPrivate->mPATRecipe.GetOptions().GetRecipeType() == GS::Gex::PAT::RecipeWaferSort)
    {
        lDataSourceDialog.SetTestingStage("Wafer Sort");
    }
    else if (mPrivate->mPATRecipe.GetOptions().GetRecipeType() == GS::Gex::PAT::RecipeFinalTest)
    {
        lDataSourceDialog.SetTestingStage("Final Test");
    }
    else
    {
        GS::Gex::Message::warning("", QString("Unknown testing stage defined in the recipe"));
        return;
    }

    lDataSourceDialog.SetTestKeyEnabled(false);
    switch (mPrivate->mPATRecipe.GetOptions().mTestKey)
    {
        case GEX_TBPAT_KEY_TESTNUMBER:
            lDataSourceDialog.SetTestKey(PATRecipeHistoricalDataGui::TestNumber);
            break;

        case GEX_TBPAT_KEY_TESTNAME:
            lDataSourceDialog.SetTestKey(PATRecipeHistoricalDataGui::TestName);
            break;

        case GEX_TBPAT_KEY_TESTMIX:
            lDataSourceDialog.SetTestKey(PATRecipeHistoricalDataGui::TestMix);
            break;

        default:
            break;
    }

    lDataSourceDialog.SetDataSource(GS::Gex::PATRecipeHistoricalDataGui::DataFromFiles);

    if (lDataSourceDialog.exec() == QDialog::Rejected)
        return;

    // Get historical data files selected
    lFilesSelected = lDataSourceDialog.GetHistoricalData();

    if(lFilesSelected.isEmpty() == TRUE)
    {
        GS::Gex::Message::warning("", QString("No historical data selected"));
        return;	// Empty list...ignore task!
    }

    // Filter data?
    QMessageBox mb(
        GS::Gex::Engine::GetInstance().Get("AppFullName").toString(),
        "Read all the data files or filter samples?\nYou can ignore/filter some parts or bins if needed...\n\nE.g.: Use GOOD parts only.",
    QMessageBox::Question,
    QMessageBox::Yes | QMessageBox::Escape | QMessageBox::Default,
    QMessageBox::No,
    QMessageBox::NoButton,this);
    mb.setWindowIcon(QPixmap(":/gex/icons/gex_application_48x48.png"));
    mb.setButtonText( QMessageBox::Yes, "Read ALL" );
    mb.setButtonText( QMessageBox::No, "Filter data..." );

    // User want to select a filter...
    QString strFilterType="all";
    QString	strFilterList="";
    if (mb.exec() == QMessageBox::No)
    {
        // Show selection made...
        FilterDialog *dFilter = new FilterDialog();
        dFilter->SetParameters(0,false,0,NULL,
                               GEX_FILE_FILTER_NOSITE |
                               GEX_FILE_FILTER_NOGROUPNAME |
                               GEX_FILE_FILTER_NOADVANCEDTAB);
        dFilter->adjustSize();

        // Prompt Filter dialog box
        if(dFilter->exec() == 1)
        {
            // Extract filer info
            strFilterType = gexFileProcessPartsItems[dFilter->comboProcess->currentIndex()];
            strFilterList = dFilter->editProcess->text();
        }
    }

    // If no files selected in the 'Process files', then load these files into it to (NOT in merge mode).
    if (mPrivate->mPATRecipe.GetOptions().GetRecipeType() == GS::Gex::PAT::RecipeFinalTest)
    {
        pGexMainWindow->ShowWizardDialog(GEX_FT_PAT_PROCESS_WIZARD_P1);	// Ensures PatWizard window is created
        pGexMainWindow->ShowWizardDialog(GEX_BROWSER);

        if (lFilesSelected.count())
            // Add Data file name to list of files to process
            pGexMainWindow->mWizardFTPAT->SetTestDataFile(lFilesSelected.at(0));
    }
    else
    {
        pGexMainWindow->ShowWizardDialog(GEX_WS_PAT_PROCESS_WIZARD_P1);	// Ensures PatWizard window is created

        pGexMainWindow->ShowWizardDialog(GEX_BROWSER);

        for (it = lFilesSelected.begin(); it != lFilesSelected.end(); ++it )
        {
            // Add Data file name to list of files to process
            pGexMainWindow->mWizardWSPAT->AddFile(*it,(it == lFilesSelected.begin()));
        }
    }

    // Get files analysed: create script file and have it executed!
    QString	strFileName;

    // Create script that will read data file + compute all statistics (but NO report created)
    QString strErrorMessage;
    FILE *hFile = fopen(GS::Gex::Engine::GetInstance().GetAssistantScript().toLatin1().constData(),"w");

    if(hFile == NULL)
    {
        strErrorMessage = "  > Failed to create script file: " + GS::Gex::Engine::GetInstance().GetAssistantScript();
        GS::Gex::Message::information("", strErrorMessage);
        return;
    }

    // Creates 'SetOptions' section
    if(!ReportOptions.WriteOptionSectionToFile(hFile))
    {
        strErrorMessage=QString("error : can't write option section");
        GSLOG(3, "WriteOptionSectionToFile failed");
        return;
    }

    // Creates 'SetProcessData' section
    fprintf(hFile,"SetProcessData()\n");
    fprintf(hFile,"{\n");
    fprintf(hFile,"  var group_id;\n");
    fprintf(hFile,"  gexGroup('reset','all');\n");
    fprintf(hFile,"  gexQuery('db_report','DataSet_1');\n");
    fprintf(hFile,"  group_id = gexGroup('insert','DataSet_1');\n");

    // Write all files to merge in analysis
    for (it = lFilesSelected.begin(); it != lFilesSelected.end(); ++it )
    {
        // Get file name
        strFileName = *it;
        ConvertToScriptString(strFileName);	// Make sure any '\' in string is doubled.

        // Write script line
        fprintf(hFile,"  gexFile(group_id,'insert','%s','All','%s',' %s','','','','');\n",
                strFileName.toLatin1().constData(),
                strFilterType.toLatin1().constData(),
                strFilterList.toLatin1().constData());
    }
    fprintf(hFile,"}\n\n");

    // Creates 'main' section
    fprintf(hFile,"main()\n");
    fprintf(hFile,"{\n");
    fprintf(hFile,"  SetOptions();\n");

    // Ensure continue on fail
    fprintf(hFile,"  gexOptions('dataprocessing', 'fail_count', 'all');\n");

    fprintf(hFile,"  gexOptions('output','format','interactive');\n");                  // Avoids editor to be launched if output options is set to Word or spreadsheet CSV!
    fprintf(hFile,"  gexOptions('speed','adv_stats','always');\n");                     // Always compute Quartiles & Skew & Kurtosis.
    fprintf(hFile,"  gexOptions('statistics','computation','samples_only');\n");        // Force to compute statistics from samples, ignore summary.

    fprintf(hFile,"  gexOptions('dataprocessing','data_cleaning_mode','none');\n");   // Disable outlier removal so to make sure all values are taken into account.

    // Specific Final Test options
    if(mPrivate->mPATRecipe.GetOptions().GetRecipeType() == GS::Gex::PAT::RecipeFinalTest)
    {
        // Choose test merge option based on recipe test key option
        switch (mPrivate->mPATRecipe.GetOptions().mTestKey)
        {
            case GEX_TBPAT_KEY_TESTNUMBER:
                // Merge tests with same test number (even if test name is different)
                fprintf(hFile,"  gexOptions('dataprocessing', 'duplicate_test', 'merge');\n");
                break;

            case GEX_TBPAT_KEY_TESTNAME:
                // Merge tests with same test name (even if test number is different)
                fprintf(hFile,"  gexOptions('dataprocessing', 'duplicate_test', 'merge_name');\n");
                break;

            case GEX_TBPAT_KEY_TESTMIX:
                // Merge tests with same test number and test name
                fprintf(hFile,"  gexOptions('dataprocessing', 'duplicate_test', 'no_merge');\n");
                break;

            default:
                // Use default option from options tab
                break;
        }
    }
    else
    {
        // Merge tests with same test number (even if test name is different)
        fprintf(hFile,"  gexOptions('dataprocessing', 'duplicate_test', 'merge');\n");
    }

    fprintf(hFile,"  SetProcessData();\n");
    fprintf(hFile,"  gexOptions('report','build','false');\n");	// Only data analysis, no report created!
    fprintf(hFile,"  gexBuildReport('none','0');\n");		// Build report.
    fprintf(hFile,"}\n\n");
    fclose(hFile);

    // Progress bar...
    QProgressDialog progress( "Reading data...", "Abort", 0, 1,this);
    progress.setModal(true);
    progress.show();

    // Execute script.
    if (GS::Gex::CSLEngine::GetInstance().RunScript(GS::Gex::Engine::GetInstance().GetAssistantScript()).IsFailed())
        return;

    // Get list of tests in recipe...
//    unsigned long ldTestNumber;
//    QList<unsigned long> vTestNumbersInRecipe;
//    QMap <unsigned long,unsigned long> mpTestNumbersInRecipe_RowOffset;
//    QStringList	strTestNumbersNotInRecipe;
//    int	lRow;
//    for(lRow=mLastTestNumberParameterLine;lRow > mLineTestNumberParameter ; lRow--)
//    {
//        // Save test# if in a visible line (not deleted)
//        if(mExcelTable->isRowHidden(lRow) == false)
//        {
//            ldTestNumber = mExcelTable->item(lRow,GEX_TPAT_CONFIG_FIELD_TNUM)->text().trimmed().toULong();
//            vTestNumbersInRecipe += ldTestNumber;	// Save list of tests in recipe
//            mpTestNumbersInRecipe_RowOffset[ldTestNumber] = lRow;	// Keep track of test entry offset in table.
//        }
//    }

    // Identify all tests not already in the recipe list....
    CGexGroupOfFiles	*pGroup=gexReport->getGroupsList().isEmpty()?NULL:gexReport->getGroupsList().first();
    if(pGroup == NULL)
        return;
    CGexFileInGroup		*pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
    if(pFile == NULL)
        return;

    // IDendify list of tests not in recipe.
    CTest *ptTestCell = pGroup->cMergedData.ptMergedTestList;
    QMap <QString, CTest *> lTestListNotInRecipe;
    QStringList lTestNumbersNotInRecipe;
    QStringList lTestPinNotInRecipe;
    while(ptTestCell)
    {
        // Check if this test has valid data...
        // Or, if Muti-result parametric test, do not show master test record
        // Or, if not a parametric / multiparametric (eg: functional) test, ignore!
        if ( (ptTestCell->ldExecs == 0)
             || (ptTestCell->lResultArraySize > 0)
             || (ptTestCell->bTestType == 'F' || ptTestCell->bTestType == '-')
            )
        {
            ptTestCell = ptTestCell->GetNextTest();
            continue;
        }

        // Check if this test is already in recipe...if not, add
        // <test#> <Test name> <Test type>
        CPatDefinition *lPatDef = mPrivate->mPATRecipe.FindUniVariateRule(ptTestCell->lTestNumber,
                                                                          ptTestCell->lPinmapIndex,
                                                                          ptTestCell->strTestName);
        if(!lPatDef)
        {
            lTestListNotInRecipe.insertMulti(QString::number(ptTestCell->lTestNumber),ptTestCell);
            lTestNumbersNotInRecipe.append(QString::number(ptTestCell->lTestNumber));
            lTestNumbersNotInRecipe.append(ptTestCell->strTestName);
            lTestNumbersNotInRecipe.append(QString(QChar(ptTestCell->bTestType)));
            lTestPinNotInRecipe.append(QString::number(ptTestCell->lPinmapIndex));
        }
        ptTestCell = ptTestCell->GetNextTest();
    };

    progress.hide();

    // Show Recipe editor page...
    pGexMainWindow->Wizard_GexTb_EditPAT_Limits();

    // Check if any test not already in recipe
    if(lTestListNotInRecipe.count() == 0)
    {
        GS::Gex::Message::information(
            "", "No new test detected.\nAll tests already in your recipe!");
        return;
    }

    QString strString;
    QString strTestList;

    // Display list of tests not in recipe, let user select which ones to import.
    PickTestDialog dPickTest(pGexMainWindow);

    // Disable selection of multiple layers: user can only pick a test from a given layer
    dPickTest.setMultipleGroups(true,false);

    // Allow multiple selection
    dPickTest.setMultipleSelection(true);

    // Load list with list of tests NOT already in recipe
    if (dPickTest.fillParameterList(lTestNumbersNotInRecipe, lTestPinNotInRecipe) && dPickTest.exec() == QDialog::Accepted)
    {
        // Get list of tests selected to be added to the recipe.
        strTestList = dPickTest.testItemizedList(false);
        QStringList lTestList =  strTestList.split(',');
        int lTotalTestAdded = 0;
        mUnivariateTree->disconnect(this, SLOT(UpdateUVData(int, int)));
        for(int lIdx=0; lIdx < lTestList.count(); ++lIdx )
        {
            QString lTestNumber = lTestList[lIdx];
            QList <CTest *> lTestListObject = lTestListNotInRecipe.values(lTestNumber);
            for(int lTest=0; lTest < lTestListObject.count(); ++lTest)
            {
                CTest *lCTest = lTestListObject[lTest];
                CPatDefinition *lPatDef = new CPatDefinition();
                if (mPrivate->mPATRecipe.GetUniVariateRules().size() > 0)
                {
                    lPatDef->m_lFailStaticBin  = mPrivate->mPATRecipe.GetUniVariateRules().begin().value()->m_lFailStaticBin;
                    lPatDef->m_lFailDynamicBin = mPrivate->mPATRecipe.GetUniVariateRules().begin().value()->m_lFailDynamicBin;
                }
                else
                {
                    if (mPrivate->mPATRecipe.GetOptions().bDynamicPAT)
                        lPatDef->m_lFailDynamicBin = mPrivate->mPATRecipe.GetOptions().iFailDynamic_SBin;
                    if (mPrivate->mPATRecipe.GetOptions().bStaticPAT)
                        lPatDef->m_lFailStaticBin = mPrivate->mPATRecipe.GetOptions().iFailStatic_SBin;
                }
                lPatDef->m_strTestName = lCTest->strTestName;
                lPatDef->m_lTestNumber = lCTest->lTestNumber;
                lPatDef->m_lfLowLimit = lCTest->GetCurrentLimitItem()->lfLowLimit;
                lPatDef->m_lfHighLimit = lCTest->GetCurrentLimitItem()->lfHighLimit;
                lPatDef->m_iDistributionShape = patlib_GetDistributionType(lCTest,
                                                                           mPrivate->mPATRecipe.GetOptions().iCategoryValueCount,
                                                                           mPrivate->mPATRecipe.GetOptions().bAssumeIntegerCategory,
                                                                           mPrivate->mPATRecipe.GetOptions().mMinConfThreshold);
                lPatDef->m_lfMean = lCTest->lfMean;
                lPatDef->m_lfRange = lCTest->lfRange;
                lPatDef->m_lfSigma = lCTest->lfSigma;
                lPatDef->m_lfRobustSigma = ((lCTest->lfSamplesQuartile3-lCTest->lfSamplesQuartile1)/1.35);
                lPatDef->m_SamplesToIgnore = GS::Gex::PAT::GetSamplesToIgnoreIndex(
                            gexIgnoreSamplesSetItemsKeywords[GEX_TPAT_IGNOREDATA_NONEID],NULL);
                lPatDef->m_OutliersToKeep = GS::Gex::PAT::GetOutlierToKeepIndex(gexKeepOutliersSetItemsKeywords[GEX_TPAT_KEEPTYPE_NONEID],NULL);
                lPatDef->m_iOutlierLimitsSet = GS::Gex::PAT::GetOutlierToKeepIndex(gexKeepOutliersSetItemsKeywords[GEX_TPAT_KEEPTYPE_NONEID],NULL);
                lPatDef->mOutlierRule = GS::Gex::PAT::GetRuleIndex(gexRuleSetItemsKeywords[GEX_TPAT_RULETYPE_SMARTID],NULL);

                lPatDef->m_iNrrRule = GS::Gex::PAT::GetNNRRuleIndex(gexNnrRuleSetItemsKeywords[GEX_TPAT_NNR_ENABLED],NULL);
                QString lString = "Test added: " + QDateTime::currentDateTime().toString("dd-MMM-yyyy hh:mm:ss");
                lPatDef->mDPATNote = lString;
                lPatDef->mSPATNote = lString;
                lTotalTestAdded++;
                QString lKeyWord = mPrivate->PATDefKey(lCTest);
                mPrivate->mPATRecipe.GetUniVariateRules().insert(lKeyWord, lPatDef);
                int lInsertedRow = mUnivariateTree->rowCount();
                mUnivariateTree->insertRow(lInsertedRow);
                addUVEntry(lPatDef, lKeyWord, lInsertedRow);
            }
        }
        mUnivariateTree->connect(mUnivariateTree, SIGNAL(cellChanged(int,int)), this, SLOT(UpdateUVData(int, int)));

        // Display job done.
        if(lTotalTestAdded)
        {
            strString = "Success adding tests to recipe file.\nTotal tests added: " + QString::number(lTotalTestAdded);
        }
        else
            strString = "No test added to recipe file!";
        GS::Gex::Message::information("", strString);
    }

    // Update test limits?
    bool lOk;
    GS::Gex::Message::request("", "Update recipe test spec. limits ?\n"
                                  "Note: Makes recipe reload test spec. limits from data file", lOk);
    if (! lOk)
    {
        return;
    }

    // Update recipe test limits...
    // Identify all tests not already in the recipe list....
    pGroup=gexReport->getGroupsList().isEmpty()?NULL:gexReport->getGroupsList().first();
    pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

    // Scan list of tests not in recipe.
    ptTestCell = pGroup->cMergedData.ptMergedTestList;
    while(ptTestCell)
    {
        // Check if this test has valid data...
        // Or, if Muti-result parametric test, do not show master test record
        // Or, if not a parametric / multiparametric (eg: functional) test, ignore!
        if ( (ptTestCell->ldExecs == 0)
             || (ptTestCell->lResultArraySize > 0)
             || (ptTestCell->bTestType == 'F' || ptTestCell->bTestType == '-')
             )
        {
            ptTestCell = ptTestCell->GetNextTest();
            continue;
        }

        // Check if this test is already in recipe...if not, add
        // <test#> <Test name> <Test type>

        for(int lRow=0; lRow < mUnivariateTree->rowCount();++lRow)
        {
            QString strTestName = ptTestCell->strTestName;
            QString strTestNumber = QString::number(ptTestCell->lTestNumber);

            if((mUnivariateTree->item(lRow,mPrivate->GetFieldColumn("Test#", PATRecipeEditor::Basic))->text() == strTestNumber )
                    && (mUnivariateTree->item(lRow,mPrivate->GetFieldColumn("Test name", PATRecipeEditor::Basic))->text() == strTestName))
            {
                mUnivariateTree->item(lRow,mPrivate->GetFieldColumn("LL.", PATRecipeEditor::Basic))->setText(QString("%1 %2")
                        .arg(mPrivate->mFormat.formatNumericValue(ptTestCell->GetCurrentLimitItem()->lfLowLimit, true))
                        .arg(ptTestCell->szTestUnits));
                mUnivariateTree->item(lRow,mPrivate->GetFieldColumn("LL.", PATRecipeEditor::Basic))->setData(Qt::UserRole, ptTestCell->GetCurrentLimitItem()->lfLowLimit);
                mUnivariateTree->item(lRow,mPrivate->GetFieldColumn("HL.", PATRecipeEditor::Basic))->setText(QString("%1 %2")
                       .arg(mPrivate->mFormat.formatNumericValue(ptTestCell->GetCurrentLimitItem()->lfHighLimit, true))
                       .arg(ptTestCell->szTestUnits));
                mUnivariateTree->item(lRow,mPrivate->GetFieldColumn("HL.", PATRecipeEditor::Basic))->setData(Qt::UserRole, ptTestCell->GetCurrentLimitItem()->lfHighLimit);
            }
        }
        ptTestCell = ptTestCell->GetNextTest();
    };

    GS::Gex::Message::information("", "Test Spec limits updated");
}

//// See if we consider tests based on the 'Test name' or 'test number' (the access key)
QString PATRecipeEditorPrivate::PATDefKey(CTest *test)
{
    QString lKeyword;
    switch(mPATRecipe.GetOptions().mTestKey)
    {
        case GEX_TBPAT_KEY_TESTNUMBER:
            lKeyword = QString::number(test->lTestNumber);
            if(test->lPinmapIndex >= 0)
                lKeyword += "." + QString::number(test->lPinmapIndex);
            break;
        case GEX_TBPAT_KEY_TESTNAME:
            lKeyword = test->strTestName.trimmed();
            if(test->lPinmapIndex >= 0)
                lKeyword += "." + QString::number(test->lPinmapIndex);
            break;
        case GEX_TBPAT_KEY_TESTMIX:
            lKeyword = test->strTestName.trimmed();
            lKeyword += "." + QString::number(test->lTestNumber);
            if(test->lPinmapIndex >= 0)
                lKeyword += "." + QString::number(test->lPinmapIndex);
            break;
    }
    return lKeyword;
}

///////////////////////////////////////////////////////////
// Show Conversion status for files selected.
///////////////////////////////////////////////////////////
void PATRecipeEditor::OnProperties(void)
{
    QString strMessage;
    QFile f;
    // File
    // Size
    // Lines
    strMessage = "* Editing Data: Information*\n";

    // Input file + size
    strMessage += "\nFile: " + mPrivate->mRecipeName;
    f.setFileName(mPrivate->mRecipeName);
    if(f.exists() == false)
        return;
    strMessage += "\nTable size: " + QString::number(f.size()) + " Bytes";

    // Total# of Rows & lines
    strMessage += "\nTotal Rows : " + QString::number(mUnivariateTree->rowCount());
    GS::Gex::Message::information("", strMessage);
}

///////////////////////////////////////////////////////////
// Delete the selected lines
///////////////////////////////////////////////////////////
void PATRecipeEditor::OnDelete(bool bLines)
{
    // Get the list of al selected element in the table
    QList<QTableWidgetSelectionRange> selectedRegion = mUnivariateTree->selectedRanges();

    // Check if selection affects ALL the table...
    QString strConfirm;
    if(!selectedRegion.empty() && (selectedRegion.first().rowCount()==mUnivariateTree->rowCount()))
    {
        // Confirm to delete FULL table
        strConfirm = "Do you confirm to delete the FULL table?";
    }
    else
    {
        // Confirm to delete some lines/columns?
        if(bLines)
            strConfirm = "Do you confirm to delete the selected rows?";
        else
            strConfirm = "Do you confirm to delete the selected columns?";
    }
    bool lOk;
    GS::Gex::Message::request("", strConfirm, lOk);
    if (! lOk)
    {
        return;
    }

    if(bLines)
    {
        mUnivariateTree->disconnect(this, SLOT(UpdateUVData(int, int)));
        // Delete lines: Hide selected rows
        QList<QTableWidgetSelectionRange>::iterator itTableRange;
        QList<int> lRowToDelete;

        for(itTableRange = selectedRegion.begin(); itTableRange != selectedRegion.end(); ++itTableRange)
        {
            int selectedRow = (*itTableRange).topRow();
            do
            {
                // hide row only if it not begins with "<" because they separte section in some PAT' files
                if(lRowToDelete.indexOf(selectedRow) == -1 )
                    lRowToDelete.append(selectedRow);
                ++selectedRow;
            }
            while(selectedRow <= (*itTableRange).bottomRow ());
        }
        qSort(lRowToDelete);
        for(int lIdx=lRowToDelete.count()-1; lIdx>=0;--lIdx)
        {
            int lRow = lRowToDelete[lIdx];
            QTableWidgetItem *lPatDefKey = mUnivariateTree->item(lRow,mPrivate->GetFieldColumn("Test#",PATRecipeEditor::Basic));
            QString lKey = lPatDefKey->data(Qt::UserRole).toString();
            CPatDefinition *lDef = mPrivate->mPATRecipe.GetUniVariateRules().value(lKey);
            mPrivate->mPATRecipe.GetUniVariateRules().remove(lKey);
            delete lDef;
            mUnivariateTree->removeRow(lRow);

        }

        if (lRowToDelete.isEmpty() == false)
            mPrivate->mModified = true;

        mUnivariateTree->connect(mUnivariateTree, SIGNAL(cellChanged(int,int)), this, SLOT(UpdateUVData(int, int)));
    }
}

void PATRecipeEditor::dropEvent(QDropEvent *event)
{
    if(!event->mimeData()->hasUrls())
    {
        event->ignore();
        return;
    }
    if(event->mimeData()->urls().isEmpty())
    {
        event->ignore();
        return;
    }

    QString lFileName = event->mimeData()->urls().first().toLocalFile();
    emit sRefreshEditor(lFileName);
    event->acceptProposedAction();
}

void PATRecipeEditor::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void PATRecipeEditor::RuleUp()
{
    QList<QTreeWidgetItem*> lSlected = mMultivariateTree->selectedItems();
    if(lSlected.count() == 1)
    {
        QTreeWidgetItem *lItem = lSlected.first();
        if(lItem->parent())
        {
            lItem = lItem->parent();
        }
        int lCurrentIdx = mMultivariateTree->indexOfTopLevelItem(lItem);
        if(lCurrentIdx == -1)
            return;
        if(lItem && mPrivate->mPatRuleMapping.contains(lItem))
        {
            if(lCurrentIdx == 0)
                return;
            PATMultiVariateRule *lMVRule = mPrivate->mPatRuleMapping[lItem];
            mPrivate->mPatRuleMapping.remove(lItem);
            delete lItem;
            addMVEntry(*lMVRule,lCurrentIdx-1);

            mMultivariateTree->setCurrentItem(mMultivariateTree->topLevelItem(lCurrentIdx-1));
            mMultivariateTree->scrollToItem(mMultivariateTree->topLevelItem(lCurrentIdx-1));
            mMultivariateTree->collapseItem(mMultivariateTree->topLevelItem(lCurrentIdx-1));
        }
    }

}

void PATRecipeEditor::RuleDown()
{
    QList<QTreeWidgetItem*> lSlected = mMultivariateTree->selectedItems();
    if(lSlected.count() == 1)
    {
        QTreeWidgetItem *lItem = lSlected.first();
        if(lItem->parent())
        {
            lItem = lItem->parent();
        }
        int lCurrentIdx = mMultivariateTree->indexOfTopLevelItem(lItem);
        if(lCurrentIdx == -1)
            return;
        if(lItem && mPrivate->mPatRuleMapping.contains(lItem))
        {
            if(lCurrentIdx == (mMultivariateTree->topLevelItemCount()-1))
                return;
            PATMultiVariateRule *lMVRule = mPrivate->mPatRuleMapping[lItem];
            mPrivate->mPatRuleMapping.remove(lItem);
            delete lItem;
            addMVEntry(*lMVRule,lCurrentIdx+1);
            mMultivariateTree->setCurrentItem(mMultivariateTree->topLevelItem(lCurrentIdx+1));
            mMultivariateTree->scrollToItem(mMultivariateTree->topLevelItem(lCurrentIdx+1));
            mMultivariateTree->collapseItem(mMultivariateTree->topLevelItem(lCurrentIdx+1));
        }
    }

}

void PATRecipeEditor::OnRecipeModified()
{
    mPrivate->mModified = true;
}

QString PATRecipeEditor::CheckSaveForModifiedRecipe()
{
    // Checks if recipe to used is currently edited and needs to be saved first
    if(mPrivate->mRecipeName.isEmpty() == false)
    {
        // If current recipe modified (and has a Vesrion#), save it under new Version#
        if(mPrivate->mModified)
        {
            if(mPrivate->mRecipeName.endsWith(".debug",Qt::CaseInsensitive) == false)
                mPrivate->mRecipeName += ".debug";

            RefreshFile(mPrivate->mRecipeName);
        }
    }

    return mPrivate->mRecipeName;
}

///////////////////////////////////////////////////////////
// Update recipe name (revision#) if Version# enabled!
// bNewMajorRelease: ;	// If true, increment major release, force build to 0.
///////////////////////////////////////////////////////////
bool PATRecipeEditor::BuildRecipeVersion(QString & lRecipeName, bool lMajorRelease)
{
    QFileInfo   lFileInfo(lRecipeName);
    QDir        lDir(lFileInfo.path());
    QString     lRecipePrefix;
    bool        lUseNamingConvention = false;
    int         lMajorVersion = mPrivate->mPATRecipe.GetOptions().iRecipeVersion;	// Current recipe version# as found in recipe file.
    int         lMinorVersion = mPrivate->mPATRecipe.GetOptions().iRecipeBuild;     // Current recipe Build#
    int         lIndex        = lFileInfo.fileName().indexOf("_Version",0,Qt::CaseInsensitive);

    if (lIndex >= 0)
    {
        lUseNamingConvention = true;

        // This recipe includes a version number...then check which revision to use.
        // Find all files with name <recipe_name>_Version*
        lRecipePrefix = lFileInfo.fileName().left(lIndex);
        QString strSearchName  = lRecipePrefix + "*";
        lDir.setFilter(QDir::Files);	// Non-recursive: ONLY import this folder

        // Find files: '<recipe_name>_version*'
        QStringList strFiles = lDir.entryList(QDir::nameFiltersFromString(strSearchName));

        // Extract highest version# from files '<recipe_name>_VersionXXX_patman_config.csv'
        QString strExistingRecipe;
        QString strString;

        int     lVersion;
        bool	lFlag;

        for (QStringList::const_iterator iter  = strFiles.begin(); iter != strFiles.end(); ++iter)
        {
            // Get file name
            strExistingRecipe = *iter;

            // Extract string to the right of 'xxx_version'
            strString = strExistingRecipe.mid(lIndex+8);
            // Extract subs-string to only hold the version#, either "X" or "X.Y"
            strString = strString.section('_',0,0);
            // If Build exists, ignore it.
            strString = strString.section('.',0,0);
            // Extract version#
            lVersion = strString.toInt(&lFlag);
            // If valid version parsed, update highest version# found in list
            if(lFlag)
                lMajorVersion = qMax(lMajorVersion, lVersion);
        }
    }

    if(lMajorRelease)
    {
        // Ensure first version starts at index 1!
        if(lMajorVersion <=0 )
            lMajorVersion = 1;
        else
            lMajorVersion++; // Increment highest version#

        lMinorVersion = 0;

    }
    else
    {
        // Only increment build#
        lMinorVersion++;
        // Ensure we always start from Build 1
        if(lMinorVersion < 0)
            lMinorVersion = 0;
    }

    if (lUseNamingConvention)
    {
        // Build new recipe name.
        lRecipeName = lFileInfo.path() + "/" + lRecipePrefix + "_Version" + QString::number(lMajorVersion);
        if(lMinorVersion >= 0)
            lRecipeName += "." + QString::number(lMinorVersion);

        lRecipeName += "_patman_config.json";
    }

    // Keep track of latest recipe rev#
    mPrivate->mPATRecipe.GetOptions().iRecipeVersion    = lMajorVersion;
    mPrivate->mPATRecipe.GetOptions().iRecipeBuild      = lMinorVersion;

    return true;
}

PATRecipeEditorPrivate::RuleNameValidator::RuleNameValidator(QTreeWidget *widget, int columIdx,QObject * parent):QValidator(parent)
{
    mColIdx = columIdx;
    mTree = widget;

}

PATRecipeEditorPrivate::RuleNameValidator::~RuleNameValidator()
{

}

QValidator::State PATRecipeEditorPrivate::RuleNameValidator::validate(QString &rulename, int &) const
{
    if(rulename.isEmpty())
        return QValidator::Invalid;
    for(int lIdx=0; lIdx<mTree->topLevelItemCount();lIdx++)
    {
        QTreeWidgetItem *lItem = mTree->topLevelItem(lIdx);
        QLineEdit *lEdit = qobject_cast<QLineEdit*> (mTree->itemWidget(lItem, mColIdx));
        if(this != lEdit->validator())
        {
            if(lEdit->text() == rulename)
            {
                return QValidator::Invalid;
            }
        }
    }
    return QValidator::Acceptable;
}


}
}
#endif
