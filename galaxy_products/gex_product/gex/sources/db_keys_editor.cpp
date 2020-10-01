#include <QLineEdit>
#include <QScrollArea>
#include <QFileDialog>
#include <QLabel>
#include <QDomDocument>
#include <QDropEvent>
#include <QUrl>
#include <QTime>
#include <QMessageBox>
#include <QMimeData>


#include <gqtl_log.h>

#include "ui_db_keys_editor.h"
#include "db_keys_editor.h"
#include "regexp_validator.h"
#include "db_key_data.h"
#include "db_key_dyn_row_items.h"
#include "db_key_static_row_items.h"
#include "engine.h"
#include "product_info.h"
#include "gex_shared.h"
#include "message.h"

namespace GS
{
namespace Gex
{

DbKeysEditor::DbKeysEditor(const QtLib::DatakeysContent& dbKeyContent,
                           const QStringList &filesGroup,
                           OpenMode mode,
                           QWidget *parent) :
    QDialog(parent,Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    mOriginalDbKeyContent(dbKeyContent),
    mFilesGroup(filesGroup),
    mOpenMode(mode),
    mUi(new Ui::DbKeysEditor)
{
    mUi->setupUi(this);
    mRegExpValidator    = 0;
    mUiStaticVLayout    = 0;
    mUiDynVLayout       = 0;
    mChangesSaved       = true;
    mIsDatasetValid     = true;
    mShowDetailsSelected= false;
    mDbKeysFilePath     = "new";
    mStdfFilePath       = mOriginalDbKeyContent.Get("SourceArchive").toString();

    // ...
    InitGui();
    // Load data
    LoadDefaultData();
    // Load view
    LoadView();
    // Connection to regexp button
    connect(mUi->pushButtonRegExp, SIGNAL(clicked()),
            this, SLOT(OnShowRegExpValidatorRequested()));
    // Connection to save button
    connect(mUi->pushButtonSave, SIGNAL(clicked()),
            this, SLOT(OnSaveRequested()));
    // Connection to save as button
    connect(mUi->pushButtonSaveAs, SIGNAL(clicked()),
            this, SLOT(OnSaveAsRequested()));
    // Connection to open button
    connect(mUi->pushButtonOpenFile, SIGNAL(clicked()),
            this, SLOT(OnLoadFromFileRequested()));
    // Connection to clear button
    connect(mUi->pushButtonClearAll, SIGNAL(clicked()),
            this, SLOT(OnClearAllRequested()));
    // Connection to load default button
    connect(mUi->pushButtonLoadDefault, SIGNAL(clicked()),
            this, SLOT(OnLoadDefaultPatternRequested()));
    // Connection to show details
    connect(mUi->pushButtonDetails, SIGNAL(toggled(bool)),
            this, SLOT(OnShowDetailsRequested(bool)));
    // Connection to apply button
    connect(mUi->pushButtonOk, SIGNAL(clicked()),
            this, SLOT(OnApplyRequested()));
}

DbKeysEditor::~DbKeysEditor()
{
    while (!mRowsStaticKey.isEmpty())
         delete mRowsStaticKey.takeFirst();
    while (!mDatasStaticKey.isEmpty())
         delete mDatasStaticKey.takeFirst();
    while (!mDatasDynamicKey.isEmpty())
         delete mDatasDynamicKey.takeFirst();
    while (!mRowsDynKey.isEmpty())
         delete mRowsDynKey.takeFirst();
    if (mRegExpValidator)
        delete mRegExpValidator;
    if (mUiStaticVLayout)
        delete mUiStaticVLayout;
    if (mUiDynVLayout)
        delete mUiDynVLayout;
    delete mUi;
}

void DbKeysEditor::dragEnterEvent(QDragEnterEvent *event)
{
    const QMimeData * mimeData = event->mimeData();
    if (mimeData->hasUrls())
        event->acceptProposedAction();
}

void DbKeysEditor::dropEvent(QDropEvent * event)
{
    QString lFileName;

    if (!event->mimeData()->hasUrls() ||
       (event->mimeData()->urls().count() != 1))
    {
        event->ignore();
        return;
    }

    lFileName = event->mimeData()->urls().first().toLocalFile();

    LoadFromFile(lFileName);

    event->acceptProposedAction();
}

void DbKeysEditor::keyPressEvent(QKeyEvent *e)
{
    bool lCtrlS = ((e->modifiers() & Qt::ControlModifier) &&
                   (e->key() == Qt::Key_S)); // CTRL+S

    if (lCtrlS)
        OnSaveRequested();
    else
        QDialog::keyPressEvent(e);
}

void DbKeysEditor::InitGui()
{
    QPalette lPalette = palette();
    lPalette.setColor(backgroundRole(), Qt::white);
    setPalette(lPalette);
    setAcceptDrops(true); // allow to drop config key file
    if (!mStdfFilePath.isEmpty())
        mUi->labelStdfFilePath->setText("File to insert: " +
                QDir::cleanPath(mStdfFilePath));
    InitStaticKeysView();
    InitDynamicKeysView();
    InitActionLabels();
    mUi->comboBoxSelectAction->addItem(mActionLabels.value(SELECT), SELECT);
    mUi->comboBoxSelectAction->addItem(mActionLabels.value(FILE), FILE);
    // If more than one file to import
    if (mFilesGroup.count() > 1)
        mUi->comboBoxSelectAction->addItem(mActionLabels.value(GROUP), GROUP);
    mUi->comboBoxSelectAction->addItem(mActionLabels.value(PRODUCT), PRODUCT);
    mUi->comboBoxSelectAction->addItem(mActionLabels.value(FOLDER), FOLDER);
    mUi->comboBoxSelectAction->addItem(mActionLabels.value(SKIP), SKIP);
    mUi->comboBoxSelectAction->addItem(mActionLabels.value(CANCEL), CANCEL);
    mUi->comboBoxSelectAction->setCurrentIndex(0);
    mUi->comboBoxSelectAction->setToolTip("Select one action");
    mUi->pushButtonOk->setToolTip("Apply selected action");
    mUi->pushButtonDetails->setToolTip("Show/Hide details on keys syntax");

    mUi->tabWidgetKeys->setCurrentIndex(0);

    if (mOpenMode == TOOLBOX)
    {
        mUi->frameBottom->setHidden(true);
        this->setModal(false);
    }

    RefreshUi();
}

void DbKeysEditor::InitActionLabels()
{
    mActionLabels.clear();
    mActionLabels.insert(SELECT, "--Select an action--");
    mActionLabels.insert(FILE, "Use for this file only");
    mActionLabels.insert(GROUP, "Use for this group of files");
    mActionLabels.insert(PRODUCT, "Use for this product in this folder");
    mActionLabels.insert(FOLDER, "Use for all files in this folder");
    mActionLabels.insert(SKIP, "Skip for this file");
    mActionLabels.insert(CANCEL, "Cancel insertion");
}

void DbKeysEditor::RefreshUi()
{
    mUi->pushButtonOk->setEnabled(mIsDatasetValid);
    mUi->pushButtonSave->setDisabled(mChangesSaved);
    mUi->pushButtonSaveAs->setDisabled(mChangesSaved);
    mUi->textEditLogs->setVisible(mShowDetailsSelected);
    mUi->pushButtonDetails->setChecked(mShowDetailsSelected);
    if (mChangesSaved && mDbKeysFilePath.startsWith("*"))
        mDbKeysFilePath.remove(0, 1);
    else if (!mChangesSaved && !mDbKeysFilePath.startsWith("*"))
        mDbKeysFilePath.prepend("*");
    setWindowTitle(mDbKeysFilePath + " - Database Keys Editor");
}

void DbKeysEditor::AddLog(const QString &logMessage)
{
    mUi->textEditLogs->append(QTime::currentTime().toString() +
                              ": "+
                              logMessage);
}

void DbKeysEditor::InitStaticKeysView()
{
    // create static keys grid layout
    QFrame *frame = new QFrame(this);
    mUiStaticVLayout = new QVBoxLayout(frame);
    frame->setLayout(mUiStaticVLayout);
    mUi->scrollAreaStaticKeys->setWidget(frame);

    // Insert labels
    QHBoxLayout *hLayout = new QHBoxLayout();
    QLabel *labelPtr = new QLabel("Key Name");
    labelPtr->setMinimumWidth(250);
    labelPtr->setMaximumHeight(26);
    hLayout->insertWidget(0, labelPtr);
    hLayout->insertWidget(1, new QLabel("Value"));
    hLayout->insertWidget(2, new QLabel("Expression"));
    hLayout->insertWidget(3, new QLabel("New Value"));
    hLayout->addSpacing(36);
    mUiStaticVLayout->insertLayout(0, hLayout);

    QSpacerItem* spacer = new QSpacerItem( 20, 20,
                                           QSizePolicy::Minimum,
                                           QSizePolicy::Expanding );
    mUiStaticVLayout->addItem(spacer);
}

void DbKeysEditor::InitDynamicKeysView()
{
    // create dynamic keys grid layout
    QFrame *frame = new QFrame(this);
    mUiDynVLayout = new QVBoxLayout(frame);
    frame->setLayout(mUiDynVLayout);
    mUi->scrollAreaDynKey->setWidget(frame);

    // Insert labels
    QHBoxLayout *hLayout = new QHBoxLayout();
    QLabel *labelPtr = new QLabel("Key Name");
    labelPtr->setMinimumWidth(250);
    hLayout->insertWidget(0, labelPtr);
    hLayout->insertWidget(1, new QLabel("Expression"));
    hLayout->addSpacing(36);

    mUiDynVLayout->insertLayout(0, hLayout);
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum,
      QSizePolicy::Expanding );
    mUiDynVLayout->addItem(spacer);
}


void DbKeysEditor::OnShowRegExpValidatorRequested()
{
    QString lRegExp;
    ShowRegExpValidator(lRegExp);
}

void DbKeysEditor::OnShowDetailsRequested(bool show)
{
    mShowDetailsSelected = show;

    RefreshUi();
}

bool DbKeysEditor::EvaluateStaticDataSet(int rowId/*=0*/)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->isNotSupportedCapability(GS::LPPlugin::ProductInfo::toolbox))
    {
      // OEM mode, refuse to run this function!
      GS::Gex::Message::information(
          "", "This function is disabled in teradyne mode\n\nContact " +
          QString(GEX_EMAIL_SALES) + " for more information!");
        return false;
    }

    bool lValidDataSet = true;
    QString lName, lExpression, lError, lValue;
    // Copy original keys content before starting cascade computing
    GS::QtLib::DatakeysContent tmpDbKeyContent(mOriginalDbKeyContent);

    for (int i = 0; i < mDatasStaticKey.size(); ++i)
    {
        // Disconnect to ensure not reload during cascade computing
        if (mDatasStaticKey.at(i))
            disconnect(mDatasStaticKey.at(i), SIGNAL(DataChanged(int)),
                   this, SLOT(EvaluateStaticDataSet(int)));
        lName = mDatasStaticKey.at(i)->Name();
        lValue = "";
        tmpDbKeyContent.GetDbKeyContent(lName, lValue);
        mDatasStaticKey.at(i)->SetValue(lValue);
        lExpression = mDatasStaticKey.at(i)->Expression();
        if (!lExpression.isEmpty())
        {
            if (GS::QtLib::DatakeysEngine::
                    evaluateDbKeys(tmpDbKeyContent, lName, lExpression, lError))
            {
                lValue = "";
                tmpDbKeyContent.GetDbKeyContent(lName, lValue);
                mDatasStaticKey.at(i)->SetEvaluatedValue(lValue, true);
            }
            else
            {
                mDatasStaticKey.at(i)->SetEvaluatedValue("", false);
                lValidDataSet = false;
                AddLog(lError);
            }
        }
        else
            mDatasStaticKey.at(i)->SetEvaluatedValue("", true);
        // Update finished reconnect...
        connect(mDatasStaticKey.at(i), SIGNAL(DataChanged(int)),
                this, SLOT(EvaluateStaticDataSet(int)));
    }
    UpdateStaticView(rowId);

    mIsDatasetValid = lValidDataSet;

    RefreshUi();

    return lValidDataSet;
}

bool DbKeysEditor::EvaluateDynDataSet(int rowId)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->isNotSupportedCapability(GS::LPPlugin::ProductInfo::toolbox))
    {
      // Evaluation mode or OEM mode, refuse to run this function!
      GS::Gex::Message::information(
          "", "This function is disabled in teradyne mode\n\nContact " +
          QString(GEX_EMAIL_SALES) + " for more information!");
        return false;
    }

    bool lValidDataSet = true;
    QString lExpression, lName, lError;

    for (int i = 0; i < mDatasDynamicKey.size(); ++i)
    {
        lName = mDatasDynamicKey.at(i)->Name();
        lExpression = mDatasDynamicKey.at(i)->Expression();

        // Empty line ignore it
        if (lName.isEmpty() && lExpression.isEmpty())
            continue;

        // Disconnect to ensure not reload during cascade computing
        if (mDatasDynamicKey.at(i))
            disconnect(mDatasDynamicKey.at(i), SIGNAL(DataChanged(int)),
                   this, SLOT(EvaluateDynDataSet(int)));

        GS::QtLib::DatakeysContent tmpDbKeyContent(mOriginalDbKeyContent);

        if (GS::QtLib::DatakeysContent::isValidDynamicKeys(lName, lError))
        {
            mDatasDynamicKey.at(i)->SetNameIsValid(true);
            if (!lExpression.isEmpty())
            {
                if (GS::QtLib::DatakeysEngine::
                        evaluateDbKeys(tmpDbKeyContent, lName, lExpression, lError))
                    mDatasDynamicKey.at(i)->SetExpressionIsValid(true);
                else
                {
                    lValidDataSet = false;
                    mDatasDynamicKey.at(i)->SetExpressionIsValid(false);
                    AddLog(lError);
                }
            }
            else
                mDatasDynamicKey.at(i)->SetExpressionIsValid(true);
        }
        else
        {
            lValidDataSet = false;
            mDatasDynamicKey.at(i)->SetNameIsValid(false);
            mDatasDynamicKey.at(i)->SetExpressionIsValid(false);
        }

        connect(mDatasDynamicKey.at(i), SIGNAL(DataChanged(int)),
                this, SLOT(EvaluateDynDataSet(int)));
    }

    UpdateDynamicView(rowId);

    mIsDatasetValid = lValidDataSet;

    RefreshUi();

    return lValidDataSet;
}

void DbKeysEditor::UpdateStaticView(int rowId)
{
    for (int i = rowId ; i < mRowsStaticKey.size() ;++i)
    {
        // update value
        mRowsStaticKey.at(i)->SetValue(mDatasStaticKey.at(i)->Value());
        // Update evaluated value
        mRowsStaticKey.at(i)->SetEvaluatedValue(
                    mDatasStaticKey.at(i)->EvaluatedValue());
    }
}

void DbKeysEditor::UpdateDynamicView(int rowId)
{
    for (int i = rowId ; i < mRowsDynKey.size() ;++i)
    {
        // Check field validity
        mRowsDynKey.at(i)->CheckValidity();
    }
}

void DbKeysEditor::InsertEmptyStaticKey(int rowId)
{
    DbKeyData *newKey = new DbKeyData("", "", rowId);
    // insert into data model
    InsertStaticKeyData(newKey);
    // Add new row to view
    InsertStaticKeyRow(newKey);
    // Update rowid of next keys
    for (int i = rowId + 1; i < mDatasStaticKey.size(); ++i)
        mDatasStaticKey.at(i)->SetFlowId(mDatasStaticKey.at(i)->FlowId() + 1);
}

void DbKeysEditor::OnRemoveStaticKeyRequested(int rowId)
{
    // Do not remove last row
    if (mDatasStaticKey.size() > 1)
        RemoveStaticKey(rowId);
    else
        GSLOG(SYSLOG_SEV_INFORMATIONAL, "Last row will not be removed");
}

bool DbKeysEditor::RemoveStaticKey(int rowId)
{
    if (rowId < 0 || rowId >= mDatasStaticKey.size())
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Row %1 not in range of existing rows, unable to remove").
               arg(QString::number(rowId)).toLatin1().data());
        return false;
    }

    DbKeyData *key = mDatasStaticKey.takeAt(rowId);
    delete key; key = 0;

    // Update row id
    for (int i = rowId; i < mDatasStaticKey.size(); ++i)
        mDatasStaticKey.at(i)->SetFlowId(mDatasStaticKey.at(i)->FlowId() - 1);
    mUiStaticVLayout->takeAt(rowId + 1);

    DbKeyStaticRowItems *row = mRowsStaticKey.takeAt(rowId);
    delete row; row = 0;

    EvaluateStaticDataSet(rowId);

    return true;
}

QMap<QString, QString> DbKeysEditor::MapOfStaticKeysExpressionToolTips() const
{

    QStringList lKeys = GS::QtLib::DataKeysDefinitionLoader::GetInstance().GetStaticKeys();
    QMap<QString, QString> lToolTips;
    for(int lIdx=0; lIdx<lKeys.count(); ++lIdx)
    {
        QString lKey = lKeys[lIdx];
        GS::QtLib::DataKeysData lData = GS::QtLib::DataKeysDefinitionLoader::GetInstance().GetDataKeysData(lKey);
        lToolTips.insert(lKey,
                QString("<b>%1:</b><br><br>"
                        "Description / Typical content:<br> %2<br><br>"
                        "STDF field:<br> %3").
                arg(lData.GetKeyName()).
                arg(lData.GetDescription()).
                arg(lData.GetStdfField()));


    }

    return lToolTips;
//

//    QDomDocument doc;
//    QFile file(":/gex/xml/gexdbkeys_definition.xml");
//    if (!file.open(QIODevice::ReadOnly))
//    {
//        GSLOG(SYSLOG_SEV_ERROR, "Unable to open tool tips source file");
//        return toolTips;
//    }
//    if (!doc.setContent(&file))
//    {
//        GSLOG(SYSLOG_SEV_ERROR, "Unable to load tool tips source file");
//        file.close();
//        return toolTips;
//    }
//    file.close();

//    QDomElement docElt = doc.documentElement();

//    QDomNode node = docElt.firstChildElement("key");
//    QString desc, stdfField, name;
//    while (!node.isNull())
//    {
//        desc = stdfField = name = "";
//        QDomElement elt = node.toElement(); // try to convert the node to an element.
//        name = elt.attribute("name", "");
//        if (!name.isEmpty() && !toolTips.contains(name))
//        {
//            QDomElement elt1 = elt.firstChildElement("description");
//            if (!elt1.isNull())
//                desc = elt1.text();
//            elt1 = elt.firstChildElement("stdf_field");
//            if (!elt1.isNull())
//                stdfField = elt1.text();
//            toolTips.insert(name,
//                            QString("<b>%1:</b><br><br>"
//                                    "Description / Typical content:<br> %2<br><br>"
//                                    "STDF field:<br> %3").
//                            arg(name).
//                            arg(desc).
//                            arg(stdfField));
//        }
//        node = node.nextSibling();
//    }

//    return toolTips;
}

QMap<QString, QString> DbKeysEditor::MapOfFunctionToolTips()
{
    QMap<QString, QString> toolTips;
    toolTips.insert("Section",
                    "<b>Section:</b>:<br><br>"
                    "Syntax: &lt;fieldsrc&gt;.Section(X-Y)<br>"
                    "This syntax allows extracting a sub-string from &lt;fieldsrc&gt;, starting at character# X, up to character# Y.<br>"
                    "i.e.:Lot , FileName.Section(1-10): this line extracts the first 10 characters of the file name and save them in the Lot ID<br><br>"
                    "Syntax: &lt;fieldsrc&gt;.Section(%nC)<br>"
                    "Allows extracting the nth substring, where C is the separator.<br>"
                    "i.e.:Lot , FileName.Section(%17_): this line extracts the 17th sub-string of the file name, using the '_' character as the sub-string delimiter.");
    toolTips.insert("RegExp",
                    "<b>RegExp:</b><br><br>"
                    "Syntax: &lt;fieldsrc&gt;.RegExp(&lt;reg_expression&gt;)<br>"
                    "This syntax loads the &lt;fielddest&gt; based on the &lt;reg_expression&gt; formula.<br>"
                    "If such formula is not verified (ie: returns no result), then &lt;fielddest&gt; remains unchanged.<br><br>"
                    "Syntax: &lt;fieldsrc&gt;.RegExp(&lt;reg_expression&gt;,&lt;s1&gt;,&lt;s2&gt;)<br>"
                    "This syntax loads &lt;fielddest&gt; with &lt;s1&gt; string if &lt;reg_expression&gt; is verified.<br>"
                    "Otherwise, &lt;s2&gt; it loaded into &lt;fielddest&gt;.");
    return toolTips;
}

void DbKeysEditor::InsertEmptyDynKey(int rowId)
{
    DbKeyData *newKey = new DbKeyData("", "", rowId);
    // insert into data model
    InsertDynamicKeyData(newKey);
    // Add new row to view
    InsertDynamicKeyRow(newKey);
    // Update rowid of next keys
    for (int i = rowId + 1; i < mDatasDynamicKey.size(); ++i)
        mDatasDynamicKey.at(i)->SetFlowId(mDatasDynamicKey.at(i)->FlowId() + 1);
}

void DbKeysEditor::OnRemoveDynKeyRequested(int rowId)
{
    // Do not remove last row
    if (mDatasDynamicKey.size() > 1)
        RemoveDynKey(rowId);
    else
        GSLOG(SYSLOG_SEV_INFORMATIONAL, "Last row will not be removed");
}

bool DbKeysEditor::RemoveDynKey(int rowId)
{
    if (rowId < 0 || rowId >= mDatasDynamicKey.size())
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Row %1 not in range of existing rows, unable to remove").
               arg(QString::number(rowId)).toLatin1().data());
        return false;
    }

    DbKeyData *key = mDatasDynamicKey.takeAt(rowId);
    delete key; key = 0;

    // Update row id
    for (int i = rowId; i < mDatasDynamicKey.size(); ++i)
        mDatasDynamicKey.at(i)->SetFlowId(mDatasDynamicKey.at(i)->FlowId() - 1);
    mUiDynVLayout->takeAt(rowId + 1);

    DbKeyDynRowItems *row = mRowsDynKey.takeAt(rowId);
    delete row; row = 0;

    return true;
}

bool DbKeysEditor::SaveToFile(const QString &filePath)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->isNotSupportedCapability(GS::LPPlugin::ProductInfo::toolbox))
    {
      // Evaluation mode or OEM mode, refuse to run this function!
      GS::Gex::Message::information(
          "", "This function is disabled in teradyne mode\n\nContact " +
          QString(GEX_EMAIL_SALES) + " for more information!");
        return false;
    }

    GS::QtLib::DatakeysFile   keyFile(filePath);
    GS::QtLib::DatakeysError  keyError;
    QString lMsg;

    // Insert static keys into keyfile
    for (int i = 0; i < mDatasStaticKey.size(); ++i)
    {
        if (!mDatasStaticKey.at(i)->Name().isEmpty() &&
                !mDatasStaticKey.at(i)->Expression().isEmpty())
        {
            keyFile.InsertStaticKey(mDatasStaticKey.at(i)->Name(),
                                    mDatasStaticKey.at(i)->Expression(),
                                    i);
        }
        else
        {
            lMsg = QString("Static Key #%1 will not be saved, "
                           "expression or key name is empty").
                    arg(QString::number(i+1));
            AddLog(lMsg);
        }
    }

    // Insert dynamic keys into keyfile
    for (int i = 0; i < mDatasDynamicKey.size(); ++i)
    {
        if (!mDatasDynamicKey.at(i)->Name().isEmpty() &&
                !mDatasDynamicKey.at(i)->Expression().isEmpty())
        {
            keyFile.InsertDynamicKey(mDatasDynamicKey.at(i)->Name(),
                                     mDatasDynamicKey.at(i)->Expression(),
                                     i);
        }
        else
        {
            lMsg = QString("Dynamic Key #%1 will not be saved, "
                           "expression or key name is empty").
                    arg(QString::number(i+1));
            AddLog(lMsg);
        }
    }

    if (!keyFile.Write(keyError))
    {
        lMsg = QString("Unable to write %1: %2")
                .arg(filePath).arg(keyError.mMessage);
        AddLog(lMsg);
        GSLOG(SYSLOG_SEV_ERROR, lMsg.toLatin1().data());
        return false;
    }

    lMsg = QString("File %1 successfully saved").arg(filePath);
    AddLog(lMsg);
    GSLOG(SYSLOG_SEV_INFORMATIONAL, lMsg.toLatin1().data());

    mDbKeysFilePath = filePath;

    mChangesSaved = true;

    RefreshUi();

    return true;
}

void DbKeysEditor::OnSaveRequested()
{
    if(GS::LPPlugin::ProductInfo::getInstance()->isNotSupportedCapability(GS::LPPlugin::ProductInfo::toolbox))
    {
      // Evaluation mode or OEM mode, refuse to run this function!
      GS::Gex::Message::information(
          "", "This function is disabled in teradyne mode\n\nContact " +
          QString(GEX_EMAIL_SALES) + " for more information!");
        return;
    }

    QRegExp lNewFileTemplate("^\\*?new$");
    if (!mDbKeysFilePath.isEmpty() &&
            !(lNewFileTemplate.exactMatch(mDbKeysFilePath)))
    {
        if (mDbKeysFilePath.startsWith("*"))
            mDbKeysFilePath.remove(0, 1);
        SaveToFile(mDbKeysFilePath);
    }
    else
        OnSaveAsRequested();
}

void DbKeysEditor::OnSaveAsRequested()
{
    QString filePath = QFileDialog::getSaveFileName(this,
                                                    "Save GexDB Keys to...",
                                                    GS::Gex::Engine::GetInstance().GetLastAccessedFodler(),
                                                    "GexDB Keys (*.gexdbkeys)");
    if (filePath.isEmpty())
    {
        QString lMsg = "Unable to save, file name is empty";
        AddLog(lMsg);
        GSLOG(SYSLOG_SEV_WARNING, lMsg.toLatin1().data());
        return;
    }

    GS::Gex::Engine::GetInstance().UpdateLastAccessedFolder(QFileInfo(filePath).absolutePath());

    SaveToFile(filePath);
}


bool DbKeysEditor::LoadFromFile(const QString &filePath)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->isNotSupportedCapability(GS::LPPlugin::ProductInfo::toolbox))
    {
      // Evaluation mode or OEM mode, refuse to run this function!
      GS::Gex::Message::information(
          "", "This function is disabled in teradyne mode\n\nContact " +
          QString(GEX_EMAIL_SALES) + " for more information!");
        return false;
    }

    // Clear existing model
    clearAll();

    // Read file
    GS::QtLib::DatakeysFile           lKeyFile(filePath);
    QList<GS::QtLib::DatakeysError>   lKeyErrors;
    QString                            lMsg;

    if (!lKeyFile.Read(lKeyErrors))
    {
        lMsg = QString("Error while loading %1").
                arg(filePath);
        AddLog(lMsg);
        GSLOG(SYSLOG_SEV_ERROR, lMsg.toLatin1().data());
        mIsDatasetValid = false;

        if (lKeyErrors.at(0).mLine == -1)
        {
            lMsg = lKeyErrors.at(0).mMessage;
            AddLog(lMsg);
            GSLOG(SYSLOG_SEV_ERROR, lMsg.toLatin1().data());
            OnClearAllRequested();
            return false;
        }
        else
        {
            for (int i = 0; i < lKeyErrors.count(); ++i)
            {
                lMsg = QString("at line %1: %2").
                        // starts from 0, so +1
                        arg(QString::number(lKeyErrors.at(i).mLine + 1)).
                        arg(lKeyErrors.at(i).mMessage);
                AddLog(lMsg);
                GSLOG(SYSLOG_SEV_ERROR, lMsg.toLatin1().data());
            }
        }
    }

    // Load data from file to key file
    GS::QtLib::DatakeysInfo keyInfo(-1, "", "");
    // static ones...
    for (int i = 0; i < lKeyFile.CountStaticKeys(); ++i)
    {
        if (lKeyFile.GetStaticKeysAt(keyInfo, i))
        {
            InsertStaticKeyData(new DbKeyData(keyInfo.mKeyName,"", i));
            mDatasStaticKey.at(i)->SetExpression(keyInfo.mKeyExpression);
        }
    }
    if (mDatasStaticKey.isEmpty()) // Insert empty row if needed
        InsertStaticKeyData(new DbKeyData("","", 0));

    // dynamic ones...
    for (int i = 0; i < lKeyFile.CountDynamicKeys(); ++i)
    {
        if (lKeyFile.GetDynamicKeysAt(keyInfo, i))
        {
            InsertDynamicKeyData(new DbKeyData(keyInfo.mKeyName,"", i));
            mDatasDynamicKey.at(i)->SetExpression(keyInfo.mKeyExpression);
        }
    }
    if (mDatasDynamicKey.isEmpty()) // Insert empty row if needed
        InsertDynamicKeyData(new DbKeyData("","", 0));

    // Reload view
    LoadView();

    EvaluateStaticDataSet(0);
    EvaluateDynDataSet(0);

    mDbKeysFilePath = filePath;
    mChangesSaved = true;

    lMsg = QString("File %1 loaded").arg(filePath);
    AddLog(lMsg);
    GSLOG(SYSLOG_SEV_INFORMATIONAL, lMsg.toLatin1().data());

    RefreshUi();

    return true;
}

void DbKeysEditor::OnLoadFromFileRequested()
{
    // Get file path
    QString filePath = QFileDialog::getOpenFileName(this,
                                                    "Open GexDB Keys file...",
                                                    GS::Gex::Engine::GetInstance().GetLastAccessedFodler(),
                                                    "GexDB Keys (*.gexdbkeys)");

    if (filePath.isEmpty())
    {
        QString lMsg = "Unable to load, file name is empty";
        AddLog(lMsg);
        GSLOG(SYSLOG_SEV_WARNING, lMsg.toLatin1().data());
        return;
    }

    GS::Gex::Engine::GetInstance().UpdateLastAccessedFolder(QFileInfo(filePath).absolutePath());

    LoadFromFile(filePath);
}

void DbKeysEditor::OnLoadDefaultPatternRequested()
{
    clearAll();

    LoadDefaultData();

    LoadView();

    mIsDatasetValid = true;
    mChangesSaved = false;

    RefreshUi();
}

void DbKeysEditor::OnClearAllRequested()
{
    clearAll();
    InsertStaticKeyData(new DbKeyData("","", 0));
    InsertDynamicKeyData(new DbKeyData("","", 0));

    // Reload view
    LoadView();
}

void DbKeysEditor::OnApplyRequested()
{
    QString lNewConfigFileName;
    ActionScope lAction = static_cast<ActionScope>(
                mUi->comboBoxSelectAction->itemData(mUi->comboBoxSelectAction->currentIndex()).toInt());

    if (lAction == SELECT)
    {
        QMessageBox::information(this, "No action selected", "You have to select an action to continue!");
        return;
    }
    else if (lAction == FILE)
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL, "Apply to this file only requested");
        lNewConfigFileName = mOriginalDbKeyContent.Get("SourceArchive").toString() +
                                        ".gexdbkeys";
        SaveToFile(lNewConfigFileName);
    }
    else if (lAction == GROUP)
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL,
               "Apply to this group of files requested");
        for (int i = 0; i < mFilesGroup.count(); ++i)
        {
            lNewConfigFileName = mFilesGroup.at(i) + ".gexdbkeys";
            SaveToFile(lNewConfigFileName);
        }
    }
    else if (lAction == PRODUCT)
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL,
               "Use for all files with the same product"
               "in this folder requested");
        lNewConfigFileName = QDir::cleanPath(
                            QFileInfo(mOriginalDbKeyContent.Get("SourceArchive").toString()).
                            absolutePath() +
                            QDir::separator() +
                            mOriginalDbKeyContent.Get("Product").toString() +
                            ".gexdbkeys");
        SaveToFile(lNewConfigFileName);
    }
    else if (lAction == FOLDER)
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL,
               "Use for all files in this folder requested");
        lNewConfigFileName = QDir::cleanPath(
                            QFileInfo(mOriginalDbKeyContent.Get("SourceArchive").toString()).
                            absolutePath() +
                            QDir::separator() +
                            GS::QtLib::DatakeysContent::defaultConfigFileName());
        SaveToFile(lNewConfigFileName);
    }
    else if (lAction == SKIP)
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL, "Do not overload this file requested");
        QDialog::accept();
        return;
    }
    else if (lAction == CANCEL)
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL, "Cancel insertion requested");
        QDialog::reject();
        return;
    }

    QDialog::accept();
}

void DbKeysEditor::OnDataChanged()
{
    mChangesSaved = false;

    RefreshUi();
}

void DbKeysEditor::clearAll()
{
    // Clear static keys
    for (int i = mDatasStaticKey.count() -1; i >= 0; --i)
        RemoveStaticKey(i);

    // Clear dynamic keys
    for (int i = mDatasDynamicKey.count() -1; i >= 0; --i)
        RemoveDynKey(i);
}

void DbKeysEditor::ShowRegExpValidator(const QString &strRegExp)
{
    if (!mRegExpValidator)
        mRegExpValidator = new RegExpValidator(strRegExp, this);

     GSLOG(SYSLOG_SEV_INFORMATIONAL, "Show Regular Expression validator");
    // Show it
    mRegExpValidator->show();
}

void DbKeysEditor::LoadDefaultData()
{
    LoadDefaultStaticKeys();
    LoadDefaultDynamicKeys();
}

void DbKeysEditor::LoadDefaultStaticKeys()
{
    QString value, name;
    int rowId = -1;
    /// TODO: Could be loaded from a default settings file to be more clean

    name = "Product"; value = "";
    mOriginalDbKeyContent.GetDbKeyContent(name, value);
    InsertStaticKeyData(new DbKeyData(name, value, ++rowId));
    // lot
    name = "Lot"; value = "";
    mOriginalDbKeyContent.GetDbKeyContent(name, value);
    InsertStaticKeyData(new DbKeyData(name, value, ++rowId));
    // sublot
    name = "SubLot"; value = "";
    mOriginalDbKeyContent.GetDbKeyContent(name, value);
    InsertStaticKeyData(new DbKeyData(name, value, ++rowId));
    // proddata
    name = "ProdData"; value = "";
    mOriginalDbKeyContent.GetDbKeyContent(name, value);
    InsertStaticKeyData(new DbKeyData(name, value, ++rowId));
    // retestindex
    name = "RetestIndex"; value = "";
    mOriginalDbKeyContent.GetDbKeyContent(name, value);
    InsertStaticKeyData(new DbKeyData(name, value, ++rowId));
    // retestbinlist
    name = "RetestBinList"; value = "";
    mOriginalDbKeyContent.GetDbKeyContent(name, value);
    InsertStaticKeyData(new DbKeyData(name, value, ++rowId));
    // wafer
    name = "Wafer"; value = "";
    mOriginalDbKeyContent.GetDbKeyContent(name, value);
    InsertStaticKeyData(new DbKeyData(name, value, ++rowId));
    // trackinglot
    name = "TrackingLot"; value = "";
    mOriginalDbKeyContent.GetDbKeyContent(name, value);
    InsertStaticKeyData(new DbKeyData(name, value, ++rowId));

    // Add empty DbKeyData
    InsertStaticKeyData(new DbKeyData("", "", ++rowId));
}

void DbKeysEditor::LoadDefaultDynamicKeys()
{
    // Only insert empty row
    InsertDynamicKeyData(new DbKeyData("", "", 0));
}

void DbKeysEditor::LoadView()
{
    LoadStaticKeysView();
    LoadDynamicKeysView();
}


void DbKeysEditor::LoadStaticKeysView()
{
    // For each key data insert a new row
    for (int i = 0; i < mDatasStaticKey.size(); ++i)
        InsertStaticKeyRow(mDatasStaticKey.at(i));
}

bool DbKeysEditor::InsertStaticKeyRow(DbKeyData *key)
{
    if (!key)
        return false;

    int rowId = key->FlowId();
    // Get allowed static keys
    QStringList allowedStaticKeys = mOriginalDbKeyContent.allowedStaticDbKeys();

    // Set tool tips map
    QMap<QString, QString> toolTipsStatic;
    toolTipsStatic.insert("","Click here and hit Ctrl+SPACE to get possible values");
    toolTipsStatic.unite(MapOfStaticKeysExpressionToolTips());
    toolTipsStatic.unite(MapOfFunctionToolTips());

    DbKeyStaticRowItems *newRow = new DbKeyStaticRowItems(*key,
                                                          allowedStaticKeys,
                                                          toolTipsStatic);

    // Insert item in rows list
    mRowsStaticKey.insert(rowId, newRow);

    mUiStaticVLayout->insertLayout(rowId + 1, newRow);

    connect(newRow, SIGNAL(AddRow(int)),
            this, SLOT(InsertEmptyStaticKey(int)));
    connect(newRow, SIGNAL(RemoveRow(int)),
            this, SLOT(OnRemoveStaticKeyRequested(int)));

    return true;
}

void DbKeysEditor::LoadDynamicKeysView()
{
    // For each key data insert a new row
    for (int i = 0; i < mDatasDynamicKey.size(); ++i)
        InsertDynamicKeyRow(mDatasDynamicKey.at(i));
}

bool DbKeysEditor::InsertDynamicKeyRow(DbKeyData *key)
{
    if (!key)
        return false;

    int rowId = key->FlowId();
    QStringList allowedStaticKeys = mOriginalDbKeyContent.allowedStaticDbKeys();

    DbKeyDynRowItems *newRow = new DbKeyDynRowItems(*key, allowedStaticKeys);

    QMap<QString, QString> toolTipsDyn;
    toolTipsDyn.insert("","Click here and hit Ctrl+SPACE to get possible values");
    /// TODO move this into xml config file
    toolTipsDyn.insert("testCondition",
                    QString("Description/ Typical content:\n  %1").
                       arg("Used to define test condition. Syntax: testCondition[<condition name>]."));
    toolTipsDyn.insert("test",
                    QString("Description/ Typical content:\n  %1").
                       arg("Used to overload test number or test name. Syntax: test[<name or number>]."));
    ///

    // Set tool tips map
    newRow->SetNameToolTips(toolTipsDyn);
    // Set tool tips map
    toolTipsDyn.unite(MapOfStaticKeysExpressionToolTips()).
            unite(MapOfFunctionToolTips());
    newRow->SetExpressionToolTips(toolTipsDyn);
    // Insert item in rows list
    mRowsDynKey.insert(rowId, newRow);

    mUiDynVLayout->insertLayout(rowId + 1, newRow);

    connect(newRow, SIGNAL(AddRow(int)),
            this, SLOT(InsertEmptyDynKey(int)));
    connect(newRow, SIGNAL(RemoveRow(int)),
            this, SLOT(OnRemoveDynKeyRequested(int)));

    return true;
}

void DbKeysEditor::InsertStaticKeyData(DbKeyData *keyData)
{
    mDatasStaticKey.insert(keyData->FlowId(), keyData);
    connect(keyData, SIGNAL(DataChanged(int)),
            this, SLOT(EvaluateStaticDataSet(int)));
    connect(keyData, SIGNAL(DataChanged(int)),
            this, SLOT(OnDataChanged()));
}

void DbKeysEditor::InsertDynamicKeyData(DbKeyData *keyData)
{
    mDatasDynamicKey.insert(keyData->FlowId(), keyData);
    connect(keyData, SIGNAL(DataChanged(int)),
            this, SLOT(EvaluateDynDataSet(int)));
    connect(keyData, SIGNAL(DataChanged(int)),
            this, SLOT(OnDataChanged()));
}

} // END Gex
} // END GS
