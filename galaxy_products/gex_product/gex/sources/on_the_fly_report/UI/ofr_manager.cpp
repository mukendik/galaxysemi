#include <QFileDialog>
#include <QList>
#include <QLabel>
#include <QPushButton>
#include <QLayout>
#include <QSpacerItem>
#include <QMouseEvent>
#include <QAbstractScrollArea>
#include <QScrollBar>
#include <QMenu>
#include <QIcon>
#include <QMessageBox>
#include <QAction>
#include <QToolTip>

#include "gex_report.h"
#include "section_element.h"
#include "global_report_settings.h"
#include "item_report_settings.h"
#include "section_report_settings.h"
#include "button_collapse_settings.h"
#include "browser_dialog.h"

#include "ui_ofr_manager.h"
#include "ofr_controller.h"
#include "composite.h"
#include "gqtl_log.h"
#include "chart_element.h"
#include "tree_model_report.h"
#include "ofr_manager.h"
#include "message.h"

/*
 * background-color: qlineargradient(spread:pad, x1:0.965, y1:0.670864, x2:0, y2:0.107955, stop:0 #1f84ca, stop:1 #0038A8);
 *background-color: qlineargradient(spread:pad, x1:1, y1:1, x2:0, y2:0, stop:0 rgba(0, 56, 168, 255), stop:1 rgba(31, 132, 202, 255));
 *
 *
 * #1f84ca - RGB(31, 132, 202)
 * #0038A8  -  RGB(0, 56, 168)
 */

const QString defaultTitle = "MyReportBuilder";

OFRManager *OFRManager::mOFRManager = NULL;
extern CGexReport* gexReport;

OFRManager *OFRManager::GetInstance()
{
    if(!mOFRManager)
        mOFRManager = new OFRManager();

    return mOFRManager;
}

void OFRManager::Destroy()
{
    if(mOFRManager)
    {
        delete mOFRManager;
        mOFRManager = 0;
    }
}

OFRManager::OFRManager(QWidget *parent) :
    QDialog(parent, Qt::WindowCloseButtonHint),
    ui(new Ui::OFRManager),
    mComponent(0),
    mCurrentModelIndex(QModelIndex()),
    mCurrentSectionIndex(QModelIndex()),
    mTreeModelReport(0),
    mTreeContextualMenu(0),
    mTreeExpandMenu(0),
    mTreeClickedItem(0),
    mCurrentSelectedComponent(0),
    mCollapseGlobal(0),
    mCollapseSection(0),
    mCollapseItem(0),
    mGlobalReportSettings(0),
    mItemReportSettings(0),
    mSectionSettingsUI(0),
    mSaveIsNeeded(false)
{
   // this->setModal(false);
    ui->setupUi(this);
    mNoData = QPixmap(":/gex/icons/noData.png");
    InitGUI();
}

OFRManager::~OFRManager()
{
    delete ui;
}

void OFRManager::InitGUI()
{
    setWindowIconText("Report Builder");
    connect(ui->pushButtonLoad,      SIGNAL(clicked()),                      this,       SLOT(LoadFile()));
    connect(ui->pushButtonSaveAs,    SIGNAL(clicked()),                      this,       SLOT(SaveAs()));
    connect(ui->pushButtonSave,      SIGNAL(clicked()),                      this,       SLOT(SaveWithoutAsking()));
    connect(ui->pushButtonBuild,     SIGNAL(clicked()),                      this,       SLOT(Build()));
    connect(ui->pushButtonDeleteAll, SIGNAL(clicked(bool)),                  this,       SLOT(OnClear()));
    connect(ui->pushButtonExit,      SIGNAL(clicked(bool)),                  this,       SLOT(OnExit()));
    connect(ui->pushButtonCsl,       SIGNAL(clicked(bool)),                  this,       SLOT(OnGenerateCsl()));
    ui->pushButtonLoad->setAutoDefault(false);
    ui->pushButtonSaveAs->setAutoDefault(false);
    ui->pushButtonSave->setAutoDefault(false);
    ui->pushButtonDeleteAll->setAutoDefault(false);
    ui->pushButtonCsl->setAutoDefault(false);

    mTreeModelReport = new TreeModelReport();
    ui->treeView->setModel(mTreeModelReport);
    QWidget* lSettings = new QWidget();

    mCollapseGlobal         = new button_collapse_settings("Global");
    mGlobalReportSettings   = new global_report_settings();
    mCollapseGlobal->addWidget(mGlobalReportSettings);
    mGlobalReportSettings->setStyleSheet("");

    mCollapseSection    = new button_collapse_settings("Section");
    mSectionSettingsUI  = new section_report_settings();
    mCollapseSection->addWidget(mSectionSettingsUI);
    mSectionSettingsUI->setStyleSheet("");

    mCollapseItem       = new button_collapse_settings("Item");
    mItemReportSettings = new item_report_settings();
    mCollapseItem->addWidget(mItemReportSettings);
    mItemReportSettings->setStyleSheet("");

    mCollapseGlobal->setEnabled(false);
    mCollapseSection->setEnabled(false);
    mCollapseItem->setEnabled(false);

   // ui->pushButtonSave->setEnabled(false);

    // -- add the button_collapse to the OFR GUI
    QVBoxLayout* lLayout = new QVBoxLayout();

    lLayout->addWidget(mCollapseGlobal);
    lLayout->addWidget(mCollapseSection);
    lLayout->addWidget(mCollapseItem);

    lLayout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));

    setMouseTracking(true);
    lSettings->setLayout(lLayout);

     ui->scrollArea->setWidget(lSettings);
     ui->scrollArea->setLayout(new QVBoxLayout());
     SetupTreeView();
     connect(mGlobalReportSettings,SIGNAL(titleReportChanged(const QString&) ),     this,   SLOT(UpdateTitleReport(const QString&)));
     connect(mSectionSettingsUI,   SIGNAL(sectionNameChanged(const QString&)),      this,   SLOT(UpdateSectionName(const QString&)));
     connect(mSectionSettingsUI,   SIGNAL(testsSet(Component*, bool)),              this,   SLOT(UpdateUIItemSettings(Component*, bool)));
     connect(mItemReportSettings,  SIGNAL(tieChanged(ReportElement*, bool)),        this,   SLOT(UpdateElementTie(ReportElement*, bool)));
     connect(mItemReportSettings,  SIGNAL(reportElementNameChanged(const QString&)),this,   SLOT(UpdateElementReportName(const QString&)));
     connect(mGlobalReportSettings,SIGNAL(changesHasBeenMade()),                    this,   SLOT(SaveIsNeeded()));
     connect(mSectionSettingsUI,   SIGNAL(changesHasBeenMade()),                    this,   SLOT(SaveIsNeeded()));
     connect(mItemReportSettings,  SIGNAL(changesHasBeenMade()),                    this,   SLOT(SaveIsNeeded()));
     connect(mTreeModelReport,     SIGNAL(changedHasBeenMade()),                    this,   SLOT(SaveIsNeeded()));
//     connect(mItemReportSettings,  SIGNAL(iconUpdate()),                            this,   SLOT(UpdateElementReportIcon()));

    // ui->pushButtonDeleteAll->setEnabled(false);
     ui->previewZone->setPixmap(mNoData);
     EnabledPushButton(false);
     raise();
}

void OFRManager::paintEvent(QPaintEvent *)
{
    if(gexReport == 0)
       setDisabled(true);
     else
       setDisabled(false);
}

void OFRManager::EnabledPushButton(bool enabled)
{
    ui->pushButtonBuild->setEnabled(enabled);
    ui->pushButtonDeleteAll->setEnabled(enabled);
    ui->pushButtonSave->setEnabled(enabled);
    ui->pushButtonSaveAs->setEnabled(enabled);
    ui->pushButtonCsl->setEnabled(enabled);
}

void OFRManager::OnClear()
{
    QMessageBox msgBox;
    msgBox.setWindowIcon(QPixmap(":/gex/icons/gex_application_48x48.png"));

    QString lMessage = "Are you sure you want to clear this report ?";

    if(mSaveIsNeeded)
    {
       lMessage +="\n There are unsaved modifications that will be lost." ;
    }

    msgBox.setText(lMessage);
    msgBox.setIcon(QMessageBox::Critical);

    if(mSaveIsNeeded)
    {
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Save | QMessageBox::Cancel);
    }
    else
    {
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    }

    msgBox.setDefaultButton(QMessageBox::Cancel);
    int ret = msgBox.exec();

    if(ret == QMessageBox::Yes)
    {
       Reset();
    }
    else if(ret == QMessageBox::Save)
    {
       SaveWithoutAsking();
       Reset();

    }
}

void OFRManager::Reset()
{
    mTreeModelReport->clear();

    mGlobalReportSettings->clear();
    mGlobalReportSettings->setEnabled(false);

    mSectionSettingsUI->clear();
    mSectionSettingsUI->setEnabled(false);

    mItemReportSettings->clear();
    mItemReportSettings->setEnabled(false);

    mCollapseGlobal->close();
    mCollapseSection->close();
    mCollapseItem->close();

    EnabledPushButton(false);

    GS::Gex::OFR_Controller::GetInstance()->Reset();
    mComponent = 0;

    ui->previewZone->setPixmap(mNoData);

    ChartElement::mCurrentId = 0;

    mCurrentFileName = "";
    mTitle = "";
    mSaveIsNeeded = false;
    UpdateHeader();
}

void OFRManager::SetCurrentSelectedComponent(Component* selectedUIComponent)
{
    mCurrentSelectedComponent = selectedUIComponent;
}

void OFRManager::SetupTreeView()
{
    ui->treeView->setSelectionMode(QAbstractItemView::SingleSelection );
    ui->treeView->setDragEnabled(true);
    ui->treeView->setAcceptDrops(true);
    ui->treeView->setDropIndicatorShown(true);
    ui->treeView->setHorizontalScrollBar(new QScrollBar());
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);

    // -- for activating the horizontalScrollBar
    ui->treeView->setHorizontalScrollMode (QAbstractItemView::ScrollPerPixel);
    ui->treeView->setHorizontalScrollBarPolicy (Qt::ScrollBarAsNeeded);
    ui->treeView->header()->setResizeMode (QHeaderView :: ResizeToContents);
    //ui->treeView->setColumnWidth(0, 500);
    ui->treeView->header()->setStretchLastSection (false);


    connect(ui->treeView,                   SIGNAL(customContextMenuRequested   (const QPoint&)),                            this, SLOT(PreparedContextualMenu(const QPoint)));
    connect(ui->treeView->selectionModel(), SIGNAL(currentChanged               (const QModelIndex& ,const QModelIndex&)),   this, SLOT(SelectedItem(const QModelIndex&, const QModelIndex&)));
//    connect(ui->treeView,                   SIGNAL(clicked                      (const QModelIndex&)),                       this, SLOT(SelectedItem(const QModelIndex&)));
//    connect(ui->treeView,                   SIGNAL(activated                    (const QModelIndex&)),                       this, SLOT(SelectedItem(const QModelIndex&)));
//    connect(ui->treeView,                   SIGNAL(entered                      (QModelIndex)),                              this, SLOT(SelectedItem(const QModelIndex&)));
}

bool OFRManager::AddElementInView(Component* component)
{
    TreeItemReport* lItemSection = mTreeModelReport->addSection(component->GetName(), component);

    QModelIndex lIndex = mTreeModelReport->indexOfItem(lItemSection);
    ui->treeView->setExpanded(lIndex, true);
    Component* lElement = component->GetElements().back();
    mTreeModelReport->addItemToSection(lItemSection,  lElement->GetName(), lElement);

    SaveIsNeeded();

    EnabledPushButton(true);
    return true;
}

bool OFRManager::LoadUI(Composite* component)
{
    mComponent = component;

    setWindowIcon(QPixmap(":/gex/icons/gex_application_48x48.png"));

    mTreeModelReport->clear();
    // -- init the tree
    // -- Iterate over the list of sections
    QList<Component*> lSections = mComponent->GetElements();
    QList<Component *>::const_iterator lIterBeginSection(lSections.begin()), lIterEndSection(lSections.end());
    for (; lIterBeginSection<lIterEndSection; ++lIterBeginSection)
    {
        TreeItemReport* lItemSection = mTreeModelReport->addSection((*lIterBeginSection)->GetName(), *lIterBeginSection);

        //-- Iterate on list to insert new elements
        QList<Component*> lElements = (*lIterBeginSection)->GetElements();
        QList<Component *>::const_iterator lIterBegin(lElements.begin()), lIterEnd(lElements.end());
        for (; lIterBegin<lIterEnd; ++lIterBegin)
        {
           mTreeModelReport->addItemToSection(lItemSection,  (*lIterBegin)->GetName(), *lIterBegin);
        }
    }
    ui->treeView->reset();
    ui->treeView->setModel(mTreeModelReport);
    ExpandTreeView(true);

    if(lSections.count() > 0)
    {
        mCollapseGlobal->setEnabled(true);
        mGlobalReportSettings->setEnabled(true);
        EnabledPushButton(true);
    }

    mGlobalReportSettings->loadReportElement(mComponent);
    return true;
}


void OFRManager::ExpandTreeView(bool status)
{
    int lNbSections = mTreeModelReport->sectionsCount();

    for(int i = 0; i< lNbSections; ++i)
    {
        TreeItemReport* lItemSection = mTreeModelReport->sectionAt(i);

        QModelIndex lIndex = mTreeModelReport->indexOfItem(lItemSection);
        ui->treeView->setExpanded(lIndex, status);
    }
}

void OFRManager::LoadFile()
{
   QString lFileLoadName = QFileDialog::getOpenFileName(this,
                                             "Open Report Builder Config file...",
                                             mFolderLoadName,
                                             "Report Builder config file(*.json)",
                                             NULL);
    if(lFileLoadName.isEmpty() ==false)
    {

        if(GS::Gex::OFR_Controller::GetInstance()->OpenJsonFile(lFileLoadName))
        {
            Reset();
            if(GS::Gex::OFR_Controller::GetInstance()->LoadJsonFile(lFileLoadName) == true)
            {
                mFileLoadName = lFileLoadName;
                mFolderLoadName = QFileInfo(mFileLoadName).path() + "/";

                mFileSaveName       = mFileLoadName;
                mFolderSaveName     = mFolderLoadName;
                mFolderBuildName    = mFolderLoadName;
                LoadUI(static_cast<Composite*>(GS::Gex::OFR_Controller::GetInstance()->GetRoot())) ;

                ui->label_2->setToolTip(mFileLoadName);
                ui->label_3->setToolTip(mFileLoadName);

                UpdateFileReport( mFileLoadName);
                ui->pushButtonCsl->setEnabled(true);
                ui->pushButtonBuild->setEnabled(true);
                ui->pushButtonSaveAs->setEnabled(true);
            }
        }
        else
        {
            GS::Gex::Message::critical("", QString("Can't load file : " + lFileLoadName + ". File corrupted "));
        }
    }
    raise();
}

void OFRManager::UpdateTitleReport(const QString& reportTitle)
{
    mTitle = reportTitle;
    UpdateHeader();
}

void OFRManager::UpdateFileReport(const QString& fileName)
{
    mCurrentFileName = fileName;
    UpdateHeader();
}

void OFRManager::UpdateHeader()
{
    QString lTitle;
    if(mCurrentFileName.isEmpty() == false)
    {
        QFileInfo lFile(mCurrentFileName);
        lTitle = lFile.baseName() ;
    }

    if(mCurrentFileName.isEmpty() == false && mTitle.isEmpty() == false)
    {
        lTitle += " - ";
    }

    if(mTitle.isEmpty() == false)
    {
       lTitle += mTitle;
    }
    if(lTitle.isEmpty())
    {
        lTitle = defaultTitle;
    }

    if(mSaveIsNeeded)
    {
        lTitle +="*";
    }

    setWindowTitle(lTitle);
}


void OFRManager::UpdateElementTie(ReportElement* element, bool tieSet)
{
    element->SetTieWithSection(tieSet);
    UpdateTreeViewSection();

    DisplayPreview(static_cast<ChartElement*>(element));
}

void OFRManager::UpdateUIItemSettings(Component* section, bool tieSet)
{
    if(section->GetType() != T_SECTION)
        return;

    //-- update list of element child of the section
    QList<Component*> lListOfItems = static_cast<SectionElement*>(section)->GetElements();

    QList<Component*>::iterator lIterBegin(lListOfItems.begin()), lIterEnd(lListOfItems.end());
    for(; lIterBegin != lIterEnd; ++ lIterBegin)
    {
        ReportElement* reportElement =  static_cast<ReportElement*>(*lIterBegin);
        reportElement->SetSectionSettingsControlActivated(tieSet);
    }

    //-- reload the item settings GUI
    mItemReportSettings->reLoad();

    //-- reload the section treeView
    UpdateTreeViewSection();

    //--Update the preview
    UpdatePreview();


}


void OFRManager:: UpdatePreview()
{
    //-- update the preview only if this is an item that is selected
    ChartElement* lCurrentElement = dynamic_cast<ChartElement*>(mCurrentSelectedComponent);

    if(lCurrentElement)
    {
        DisplayPreview(lCurrentElement);
    }
}


void OFRManager::SaveWithoutAsking()
{
    if(mFileSaveName.isEmpty())
    {
        SaveAs();
    }
    else
    {
        Save();
    }
}

void OFRManager::OnGenerateCsl()
{
    GS::Gex::OFR_Controller::GetInstance()->SaveReportScript();
}

void OFRManager::SaveAs()
{
    // Build full path to Recipe file
    QString lDefaultName = mFileSaveName;
    if(mFileSaveName.isEmpty())
    {
        lDefaultName = mFolderSaveName + defaultTitle;
    }

    QString lFileSaveName = QFileDialog::getSaveFileName(this,
                                             "Save Report Builder Config file...",
                                             lDefaultName,
                                             "Report Builder config file(*.json)",
                                             NULL);
    if(lFileSaveName.isEmpty() == false)
    {
        mFileSaveName = lFileSaveName;
        Save();
    }
}


void OFRManager::Save()
{
    if(mFileSaveName.isEmpty())
        return;

    if(QFileInfo(mFileSaveName).completeSuffix().isEmpty())
    {
        mFileSaveName+=".json";
    }

    mFolderSaveName = QFileInfo(mFileSaveName).path() + "/";

    //-- retrieve the index postion in the tree
    mTreeModelReport->retrieveIndexPosition();

    GS::Gex::OFR_Controller::GetInstance()->SaveJsonObject(mFileSaveName);

    ui->label_2->setToolTip(mFileSaveName);
    ui->label_3->setToolTip(mFileSaveName);

    UpdateFileReport(mFileSaveName);

    SaveHasBeenDone();
}

void OFRManager::keyPressEvent(QKeyEvent * event)
{
    if(event->key() == Qt::Key_S && QApplication::keyboardModifiers() && Qt::ControlModifier && mSaveIsNeeded)
    {
        SaveWithoutAsking();
        event->accept();
    }
    else
    {
        event->ignore();
    }

    QDialog::keyPressEvent(event);
}

void OFRManager::Build()
{

    // No data avaliable
    if(gexReport && gexReport->getReportOptions() == 0)
    {
        GS::Gex::Message::warning("", "No Data to build the report.");
        return;
    }

    QString lFileBuildName;

    // the user has to choose a file destination to save its buit report
    if (mFileBuildName.isEmpty())
    {
        lFileBuildName = QDir::homePath() + "/untitled.pdf";
    }
    else
    {
        lFileBuildName = mFolderBuildName + '/' + mFileBuildName;
    }

    // In the first version, we write only PDF
    /* QString::fromStdString( report_builder.GetReportFileExtension() ) */
    QString report_file_extension("PDF");

    lFileBuildName =
        QFileDialog::getSaveFileName(this,
                                     "Built report file name...",
                                     lFileBuildName,
                                     "Built report file (*." +
                                     report_file_extension + ")",
                                     0);
    if (lFileBuildName.isEmpty())
    {
        GSLOG(SYSLOG_SEV_DEBUG, "Empty file name!");
        return;
    }
    if (QFileInfo(lFileBuildName).completeSuffix().isEmpty())
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString("File name: %1").arg(lFileBuildName).toLatin1().data());
        lFileBuildName += ".pdf";
    }

    mFileBuildName = lFileBuildName;
    mFolderBuildName = QFileInfo(mFileBuildName).path();

    // update the index of the component item due to the tree changes
    mTreeModelReport->retrieveIndexPosition();
    // Build the PDF report
    GS::Gex::OFR_Controller::GetInstance()->GenerateReport(mFileBuildName);
}


void OFRManager::ProcessSectionSelected(TreeItemReport* section)
{
    //-- update GUI enable/disable
    //mCollapseSection->open();
    mCollapseSection->setEnabled(true);
    mSectionSettingsUI->setEnabled(true);
    mCollapseItem->close();
    mCollapseItem->setEnabled(false);
    mCollapseItem->setToolTip("Select an element to edit item settings");

   //-- fill the GUI input
    SectionElement* lSectionElement = static_cast<SectionElement*>(section->GetComponent());
    mSectionSettingsUI->loadSectionElement(lSectionElement);

    ui->previewZone->setPixmap(mNoData);
}

void OFRManager::ProcessItemSelected(TreeItemReport* item)
{
    //-- Update the Section parent
    TreeItemReport* lSection = item->GetParent();
    ProcessSectionSelected(lSection);

    //--Update the unfolde buttonxx
    mCollapseItem->open();
    mCollapseItem->setEnabled(true);
    mCollapseItem->setToolTip("");

    //-- update the Report Item Settings
    mItemReportSettings->setEnabled(true);
    mItemReportSettings->loadReportElement(static_cast<ChartElement*>(item->GetComponent()));

    //-- Update the preview
    DisplayPreview(static_cast<ChartElement*>(item->GetComponent()));

    ui->scrollArea->verticalScrollBar()->setValue(ui->scrollArea->verticalScrollBar()->maximum());
}

void OFRManager::DisplayPreview(ChartElement* element)
{
    if(element->IsTieWithSection() == false || element->IsSectionSettingsControlActivated() == false)
    {
        // -- tempo put the createPixmap at component level
        QPixmap lPixmap = element->CreatePixmap();
        if(lPixmap.size().isEmpty() == false)
        {
            ui->previewZone->setPixmap(lPixmap);
        }
    }
    else
    {
        ui->previewZone->setPixmap(mNoData);
    }
}

void OFRManager::UpdatePreview(ReportElement* element)
{
    DisplayPreview(static_cast<ChartElement*>(element));
}

void OFRManager::UpdateElementReportIcon()
{
    if(mTreeModelReport)
    {
        //mTreeModelReport->reloadData(mCurrentModelIndex, Qt::DecorationRole);
        mTreeModelReport->reloadData(mCurrentModelIndex, Qt::FontRole);
    }
}

void OFRManager::UpdateSectionName(const QString& name)
{
    if(mTreeModelReport)
    {
        mTreeModelReport->updateDataName(mCurrentModelIndex, name, T_SECTION_ITEM);
    }
}

void OFRManager::UpdateElementReportName(const QString& name)
{
    if(mTreeModelReport)
    {
        mTreeModelReport->updateDataName(mCurrentModelIndex, name, T_ELEMENT_ITEM);
    }
}

void OFRManager::UpdateTreeViewSection()
{
    //-- get the current section element
    SectionElement* lSection = mSectionSettingsUI->getSectionElement();
    if(lSection && mCurrentSectionIndex.isValid())
    {
        if(mTreeModelReport)
        {
            mTreeModelReport->updateDataSection(mCurrentSectionIndex);
        }
    }
}

void OFRManager::SaveHasBeenDone()
{
    if(mSaveIsNeeded == true)
    {
        mSaveIsNeeded = false;
        ui->pushButtonSave->setEnabled(false);
        QString lTitle = windowTitle();
        if(lTitle[lTitle.size() - 1] == '*')
        {
            setWindowTitle(lTitle.remove(lTitle.size() - 1, 1));
        }
    }
}

void OFRManager::SaveIsNeeded()
{
    ui->pushButtonSave->setEnabled(true);

    if(mSaveIsNeeded == false)
    {
        mSaveIsNeeded = true;
        QString lTitle = windowTitle();
        lTitle+="*";
        setWindowTitle(lTitle);
    }
}

void OFRManager::DeleteTreeClickedItem()
{
    if(mTreeClickedItem)
    {
        // -- remove the model component
        Component *componentToRemove = mTreeClickedItem->GetComponent();
        if(componentToRemove)
        {
            Component* componentParent = componentToRemove->Parent();
            componentParent->EraseChild(componentToRemove);
        }

        // -- when deleting a section, we must update the index of the last updated section
        // -- used for the contextual menu
        if(mTreeClickedItem->GetType() == T_SECTION_ITEM)
            GS::Gex::OFR_Controller::GetInstance()->UpdateLastIndexUsedAfterADeletion(mTreeClickedItem->Row());

        // -- Update the tree view
        mTreeModelReport->removeItem(mTreeClickedItem);
        mTreeClickedItem = 0;

        //-- no more element, disable some input
        if(mTreeModelReport->rowCount() == 0)
        {
            EnabledPushButton(false);
        }
        else
        {
            SaveIsNeeded();
        }
    }
}

void OFRManager::PreparedContextualMenu(const QPoint& point)
{
    QModelIndex lIndex = ui->treeView->indexAt(point);
    if(lIndex.isValid())
    {
        TreeItemReport* lItemReport = mTreeModelReport->getItemAt(lIndex);
        if(lItemReport)
        {
            Component* lComponent = lItemReport->GetComponent();
            if(lComponent)
            {
                mTreeClickedItem = lItemReport;
                QString lName = "Delete [" + lComponent->GetName()+ "]";

                if(mTreeContextualMenu == 0)
                {
                    mTreeContextualMenu = new QMenu();
                    QAction* lAction = mTreeContextualMenu->addAction(QIcon(":/gex/icons/trash.png"), lName);
                    connect (lAction, SIGNAL(triggered(bool)), this, SLOT(DeleteTreeClickedItem()));
                }

                if(mTreeContextualMenu->isHidden() == false)
                {
                    mTreeContextualMenu->hide();
                }

                mTreeContextualMenu->actions()[0]->setText(lName);
                mTreeContextualMenu->popup(QCursor::pos());
            }
        }
    }
    else
    {
        if(mTreeModelReport->sectionsCount() > 0)
        {
            if(mTreeExpandMenu == 0)
            {
                mTreeExpandMenu = new QMenu();
                QAction* lAction = mTreeExpandMenu->addAction(QIcon(":/gex/icons/up.png"), "Collapse the view");
                connect (lAction, SIGNAL(triggered(bool)), this, SLOT(CollapseTreeView()));

                lAction = mTreeExpandMenu->addAction(QIcon(":/gex/icons/down.png"), "Expand the view");
                connect (lAction, SIGNAL(triggered(bool)), this, SLOT(ExpandTreeView()));
            }

            if(mTreeExpandMenu->isHidden() == false)
            {
                mTreeExpandMenu->hide();
            }

            mTreeExpandMenu->popup(QCursor::pos());
        }
    }
}

void OFRManager::SelectedItem(const QModelIndex& index, const QModelIndex&)
{
    if(index.isValid() == false)
        return;

    mCollapseSection->setEnabled(true);

    mCurrentModelIndex = index;
    TreeItemReport* lItem = static_cast<TreeItemReport*>(index.internalPointer());

    SetCurrentSelectedComponent(lItem->GetComponent());
    T_Component lType = lItem->GetComponent()->GetType();

    switch(lType)
    {
    case T_SECTION:
        mCurrentSectionIndex = mCurrentModelIndex;
        ProcessSectionSelected(lItem);
        break;
    case T_HISTO:
    case T_BOXPLOT:
    case T_PROBA:
    case T_TREND:
    case T_WAFER:
    case T_TABLE:
    case T_HISTO_CONNECTED:
    case T_BOXPLOT_CONNECTED:
    case T_PROBA_CONNECTED:
    case T_TREND_CONNECTED:
    case T_WAFER_CONNECTED:
    {
        mCurrentSectionIndex = mCurrentModelIndex.parent();
        //-- process the item with display of the preview
        ProcessItemSelected(lItem);
        break;
    }
    default :
        break;
    }
}

void OFRManager::UpdateCurrentSelection()
{
    if(mCurrentModelIndex.isValid())
    {
        SelectedItem(mCurrentModelIndex, QModelIndex());
    }
}

void OFRManager::ExpandTreeView()
{
    ExpandTreeView(true);
}

void OFRManager::CollapseTreeView()
{
    ExpandTreeView(false);
}

void OFRManager::closeEvent(QCloseEvent* event)
{
    event->ignore();
    OnExit();
}

void OFRManager::OnExit()
{
    if(mSaveIsNeeded == false)
    {
        done(0);
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setWindowIcon(QPixmap(":/gex/icons/gex_application_48x48.png"));

        QString lMessage = "Close without saving ?";

        msgBox.setText(lMessage);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Cancel |  QMessageBox::Close);

        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();

        if(ret == QMessageBox::Save)
        {
            SaveWithoutAsking();
            done(0);
        }
        else if(ret == QMessageBox::Close)
        {
            ui->pushButtonDeleteAll->setEnabled(false);
            done(0);
        }
    }
}
