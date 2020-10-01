#ifndef OFR_MANAGER_H
#define OFR_MANAGER_H

#include <QModelIndex>
#include <QHBoxLayout>
#include <QLabel>
#include <QDialog>


class Component;
class Composite;
class TreeItemReport;
class ReportElement;
class QMenu;

namespace Ui {
class OFRManager;
}

class TreeModelReport;
class button_collapse_settings;
class global_report_settings;
class item_report_settings;
class section_report_settings;
class ChartElement;

/// \class this class implements the OFR manager UI
class OFRManager : public QDialog
{
    Q_OBJECT

public:

    /// \brief Destructor
    ~OFRManager                     ();

    /**
     * \fn bool LoadUI(Composite *component);
     * \brief this function loads the UI of the OFR manager.
     * \param component: the composite element
     * \return true if the load has been done with success. Otherwise return false.
     */
    bool            LoadUI          (Composite *component);

    bool            AddElementInView(Component* component);



    /**
     * @brief Update the current selection if any
     */
    void            UpdateCurrentSelection();

    void            Reset           ();
    /**
     * @brief SetupTreeView -  set several TreeView options
     */
    void            SetupTreeView   ();

    void            SetCurrentSelectedComponent(Component* selectedUIComponent);

    void            keyPressEvent   (QKeyEvent * event);

    void            paintEvent(QPaintEvent *);
    static OFRManager *GetInstance  ();
    static void Destroy             ();

public slots:

    void SaveHasBeenDone            ();
    void SaveIsNeeded               ();
    void OnClear                    ();
    void OnExit                     ();

    void UpdatePreview              (ReportElement* element);
    /// \brief this function pops up a dialog box to load the OFR config file (*.json)
    void LoadFile                   ();

    /// \brief this function pops up a dialog box to save the OFR config file (*.json)
    void SaveWithoutAsking          ();
    void SaveAs                     ();
    void Save                       ();
    void OnGenerateCsl              ();

     /// \brief this function calls the build of the report
    void Build                      ();

    void DeleteTreeClickedItem      ();

    void UpdatePreview              ();
    void UpdateTitleReport          (const QString& reportTitle);
    void UpdateFileReport           (const QString& fileName );
    void UpdateElementTie           (ReportElement* element, bool tieSet);
    void UpdateUIItemSettings          (Component* section, bool tieSet);
    void PreparedContextualMenu     (const QPoint& point);

    /// \brief called when an item is selected on the treeview. Send the corresponding QModelIndex
    void SelectedItem               (const QModelIndex& index, const QModelIndex &);

    void ExpandTreeView             ();
    void CollapseTreeView           ();

    /// \brief Called when tree item name has been updated
    void    UpdateSectionName       (const QString& name);
    void    UpdateElementReportName (const QString& name);
    void    UpdateElementReportIcon ();
    void    UpdateTreeViewSection   ();
    void    ExpandTreeView          (bool status);

    void reject() { }  // Ignore the ESCAPE key

protected:
    void closeEvent(QCloseEvent *);
private:
    /// \brief Constructor
    explicit OFRManager         (QWidget *parent = 0);

    void InitGUI                ();
    void DisplayPreview         (ChartElement *element);

    void DisabledPushButtonInput();
    void ProcessSectionSelected (TreeItemReport* section);
    void ProcessItemSelected    (TreeItemReport* item);
    void UpdateHeader           ();
    void EnabledPushButton      (bool enabled);

    static OFRManager*          mOFRManager;

    QString                     mFolderLoadName;
    QString                     mFileLoadName;
    QString                     mFolderSaveName;
    QString                     mFileSaveName;
    QString                     mFolderBuildName;
    QString                     mFileBuildName;
    QString                     mCurrentFileName;
    QString                     mTitle;
    Composite*                  mComponent;         /// Do not delete this pointer
    Component*                  mCurrentSelectedComponent;
    bool                        mSaveIsNeeded;

    /// \brief graphical parameters
    Ui::OFRManager *            ui;
    button_collapse_settings*   mCollapseGlobal;
    button_collapse_settings*   mCollapseSection;
    button_collapse_settings*   mCollapseItem;
    global_report_settings*     mGlobalReportSettings;
    item_report_settings*       mItemReportSettings;
    section_report_settings*        mSectionSettingsUI;
    QModelIndex                 mCurrentModelIndex;
    QModelIndex                 mCurrentSectionIndex;
    QPixmap                     mNoData;
    TreeModelReport*            mTreeModelReport;
    QMenu*                      mTreeContextualMenu;
    QMenu*                      mTreeExpandMenu;
    TreeItemReport*             mTreeClickedItem;
};

#endif // OFR_MANAGER_H
