#ifdef GCORE15334
#ifndef PAT_RECIPE_EDITOR_H
#define PAT_RECIPE_EDITOR_H

#include <QDialog>
#include <QVariant>
#include <QItemDelegate>
#include "ui_pat_recipe_gui.h"
#include <QVariantList>

class CPatDefinition;
namespace GS
{
namespace Gex
{

class PATRecipe;
class PATMultiVariateRule;

class PATRecipeEditor : public QDialog , public Ui::PatRecipeGUI
{
    Q_OBJECT
public:
    // Enum specifying the PAT Settings to apply
    enum PATSettings
    {
        Basic,
        SPAT,
        DPAT,
        NNR,
        SPC
    };
    //! \brief constructor
    PATRecipeEditor(QWidget *parent);
    // destructor
    virtual ~PATRecipeEditor();
    // Return Sigleton instance
    Q_INVOKABLE static PATRecipeEditor &GetInstance();
    Q_INVOKABLE static bool             IsInstantiated();

    // Destroy Sigleton instance
    Q_INVOKABLE static void DestroyInstance();

    Q_INVOKABLE const QString& GetRecipeName() const;

public slots:
    // Open and Read Recipe file
    void ReadRecipe();
    // Refresh view data by reading recipeFileName
    void RefreshEditor(const QString &recipeFileName);
    // Create and write recipe file
    void WriteRecipe(bool lMajorRelease = false);
    // Open and write in the recipeFileName the data from the editor gui
    void RefreshFile(const QString &recipeFileName);
    // Detach gui editor
    void DetachRecipeEditor();
    // Apply pat using the recipe
    void ApplyPAT();
    // Edit the recipe option section
    void EditRecipeOption();
    // slot to chnage the recipe tye
    void SettingsPATTypeChanged(int );
    // Contextual menu  for the univariate view
    void OnContextualMenu(const QPoint& /*pos*/);
    // Update object data changed in the editor
    void UpdateUVData(int, int);
    // Update object data changed in the editor
    void UpdateUVData(int idx);
    // Find test in the recipe viewer
    void OnFindTest();
    // Export static pat limits
    void OnExportStaticPatLimits(void);
    // Add tests from test file to the recipe
    void OnAddPatTests();
    // Add MV Rules from historical data
    void OnAddMVRules();
    // Add a new MV Rule
    void AddNewRule();
    // Duplicate and add an MV Rule
    void DuplicateRule();
    // Delete a Rule
    void DeleteRule();
    // Edit MV Rule test list
    void EditTestList();
    // Change the rule name
    void RuleNameChanged(const QString &text=QString());
    // Change the bin value
    void RuleBinChanged(const QString &text=QString());
    // change the OutlierDistance
    void RuleOutlierDistanceChanged(int idx);
    // Change the Custom Distance
    void RuleCustomDistanceChanged(double lValue);
    // Change the Enabled value
    void RuleEnabledChanged(int check);
    // Contextual menu  for the mv view
    void OnRuleContextualMenu(const QPoint& /*pos*/);
    //Move Up a rule
    void RuleUp();
    //Move Down a rule
    void RuleDown();
    // Recipe has been modified
    void OnRecipeModified();
    // Check recipe is modified, if so save it with .debug extension
    QString CheckSaveForModifiedRecipe();

protected:
    void dropEvent(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
protected slots:
private:
    Q_DISABLE_COPY(PATRecipeEditor)
    // Object initializer
    void Initialize();
    // Object clean
    void Clear();
    // Update Editor buttons
    void EnableEditorButton(bool enable);
    // Data initializer
    void InitializeData();
    // Update the GUI with the PATSettings specified
    void InitPATSettings(PATRecipeEditor::PATSettings);
    // Refresh editor view
    void RefreshEditor();
    // Update UV Tab
    void UpdateUVTab();
    // Update MV Tab
    void UpdateMVTab();
    // Update Dynamic bin with iDynamicBin
    void UpdateDynamicBin(int iDynamicBin);
    // Reset Dynamic bin
    void ResetDynamicBin();
    // Update Static bin with staticBin
    void UpdateStaticBin(int staticBin);
    // Reset Static bin with Default value
    void ResetStaticBin();
    // Reset MV Bin
    void ResetMVBin();
    // Update MV Bin
    void UpdateMVBin(int );
    // Update NNR status
    void UpdateNNR(bool lEnabled);
    // Update DPAT Factor
    void UpdateDpatGuiFactors(int iRule,int row,bool bShowError=true,
                             bool bResetDefaultFactors=true);
    // Update SPAT Factor
    void UpdateSpatGuiFactors(int iRule,int row,bool bShowError=true,
                             bool bResetDefaultFactors=true);
    // Update NNR Factor
    void UpdateNnrGuiFactors(int iRuleID,int row);
    // Add UV Entry
    void addUVEntry(const QString &label, int row, Qt::ItemFlags flag, PATRecipeEditor::PATSettings setting, QVariant value, const char *comboLabel [] = NULL, CPatDefinition *patDef=NULL, QVariant data = QVariant());
    // Add UV Entry
    void addUVEntry(CPatDefinition *def, const QString &key,int idx);
    // Add MV Entry
    void addMVEntry(PATMultiVariateRule &mvRule, int topLevelIdx=-1);
    // Show recipe file property
    void OnProperties();
    // Delete UV entry
    void OnDelete(bool bLines = true);
    // Remove test rule
    void RemoveTestFromRule(QTreeWidgetItem *lRuleItem, QTreeWidgetItem *lTesItem);
    // Build the new recipe file name with updated major and/or minor version
    bool BuildRecipeVersion(QString &lRecipeName, bool lMajorRelease);

    void initExcelTableColumnEditor(int /*row*/,int /*col*/);
    static PATRecipeEditor *mInstance;
    class  PATRecipeEditorPrivate *mPrivate;


signals:
    void sRefreshEditor(const QString &recipeFileName);

};

// in tb_pat_outlier_removal.h
// This class is an implementation of QAbstractItemDelegate to contain combobox in table' items
class EditorComboTableDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    EditorComboTableDelegate(const char* comboItems[], int sizeTable, QObject * parent = 0);
    virtual ~EditorComboTableDelegate(){}
    QWidget * createEditor ( QWidget * parent,
                             const QStyleOptionViewItem & option,
                             const QModelIndex & index ) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                               const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor,
                                    const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const;

    void SetItemData(QString data);

private:
   int mSizeTable;
   QStringList mComboItems;

private slots:
    void emitCommitData();
};

}
}
Q_DECLARE_METATYPE(GS::Gex::PATRecipeEditor::PATSettings);


#endif // PAT_RECIPE_EDITOR_H
#endif
