#ifndef GEXCONFIGKEYSEDITOR_H
#define GEXCONFIGKEYSEDITOR_H

#include <QDialog>
#include <QList>
#include "gqtl_datakeys.h"

class QVBoxLayout;


namespace Ui {
class DbKeysEditor;
}
namespace GS
{
namespace Gex
{
class RegExpValidator;
class DbKeyStaticRowItems;
class DbKeyData;
class DbKeyDynRowItems;
/*! \class DbKeysEditor
 * \brief Gui tool to build a gexdbkey file
 *
 * Static keys and dynamic keys are stored in their own list
 *
 * Each Gui row has a pointer to a member of these lists
 *
 * As soon as a value is manually changed in one of these GUI rows
 * the DATA member is updated what triggered an update of all other
 * DATA members of this row and then a cascading recomputing
 * of all keys in the DATA list
 * Once it s done
 * all GUI rows are updated with the right value
 *
 * Same thing for insert or remove before the DATA list
 * and then the GUI is updated
 */
class DbKeysEditor : public QDialog
{
    Q_OBJECT

public:

    enum OpenMode
    {
        TOOLBOX,
        FILEINSERTION
    };

    enum ActionScope
    {
        SELECT,
        FILE,
        GROUP,
        PRODUCT,
        FOLDER,
        SKIP,
        CANCEL
    };

    /// \brief Constructor
    /// \param dbKeyContent used to get the value a each key coming from the stdf file
    /// \param filesGroup used to get the list of stdf files to import with this one
    /// \param mode used to define from which flow the editor is open
    DbKeysEditor(const QtLib::DatakeysContent &dbKeyContent,
                 const QStringList& filesGroup,
                 OpenMode mode,
                 QWidget *parent = 0);
    /// \brief Destructor
    virtual ~DbKeysEditor();

protected:
    void keyPressEvent(QKeyEvent *e);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

signals:
    /// \brief emitted to signal that the DATA list has been updated
    void DatasetEvaluated(bool);

private slots:
    /// \brief open the windows to get a file to load and call loadFile
    void OnLoadFromFileRequested();
    /// \brief if a file name is recorded save to this file, else call save as
    void OnSaveRequested();
    /// \brief open the windows to set a file where to save and call saveFile
    void OnSaveAsRequested();
    /// \brief load a default set of key to overload
    void OnLoadDefaultPatternRequested();
    /// \brief Show the reg exp validator
    void OnShowRegExpValidatorRequested();
    /// \brief Show logs
    void OnShowDetailsRequested(bool show);
    /// \brief Evaluate dataset using cascaded values starting from rowId
    bool EvaluateStaticDataSet(int rowId = 0);
    /// \brief Evaluate dataset checking key validity without cascading
    bool EvaluateDynDataSet(int rowId = 0);
    /// \brief Insert a new empty key in dataset
    void InsertEmptyStaticKey(int rowId);
    /// \brief Insert a new empty key in dataset
    void InsertEmptyDynKey(int rowId);
    /// \brief If it's not the last line call remove key
    void OnRemoveStaticKeyRequested(int rowId);
    /// \brief If it's not the last line call remove key
    void OnRemoveDynKeyRequested(int rowId);
    /// \brief remove all data from gui and insert one empty key
    void OnClearAllRequested();
    /// \brief save the file wi the right name and folder and close the dialog
    void OnApplyRequested();
    /// \brief call to update members on changes
    void OnDataChanged();

private:
    Q_DISABLE_COPY(DbKeysEditor)

    // ****** GENERAL ******
    /// \brief remove all data
    void clearAll();
    /// \brief save the DATA lists to a gexdbkey file
    bool SaveToFile(const QString &filePath);
    /// \brief load the DATA lists from a gexdbkey file
    bool LoadFromFile(const QString &filePath);
    /// \brief show the regexp validator
    void ShowRegExpValidator(const QString &strRegExp);
    /// \brief remove a key from the dialog
    bool RemoveDynKey(int rowId);
    /// \brief remove a key from the dialog
    bool RemoveStaticKey(int rowId);
    /// \return the map of static keys expression tool tips
    QMap<QString, QString> MapOfStaticKeysExpressionToolTips() const;
    /// \return the map of functions expression tool tips
    QMap<QString, QString> MapOfFunctionToolTips();

    // ****** DATA *******
    // Loading
    /// \brief load default data lists
    void LoadDefaultData();
    /// \brief load default static keys data list
    void LoadDefaultStaticKeys();
    /// \brief load default dynamic keys data list
    void LoadDefaultDynamicKeys();
    // Insert
    /// \brief Insert new Key in static data list
    void InsertStaticKeyData(DbKeyData *keyData);
    /// \brief Insert new Key in dyn data list
    void InsertDynamicKeyData(DbKeyData *keyData);

    // ****** GUI *******
    /// \brief init all GUI details
    void InitGui();
    /// \brief init action labels
    void InitActionLabels();
    /// \brief refresh GUI according to members
    void RefreshUi();
    /// \brief insert log into log box
    void AddLog(const QString &logMessage);
    /// \brief init the static row list
    void InitStaticKeysView();
    /// \brief init the dyn row list
    void InitDynamicKeysView();
    /// \brief load the all list
    void LoadView();
    /// \brief Draw the list of static DB keys into the tab widget
    void LoadStaticKeysView();
    /// \brief Draw the list of dyn DB keys into the tab widget
    void LoadDynamicKeysView();
    /// \brief instanciates new line insert it at row
    bool InsertStaticKeyRow(DbKeyData *key);
    /// \brief instanciates new line insert it at row
    bool InsertDynamicKeyRow(DbKeyData *key);
    void UpdateStaticView(int rowId = 0);
    void UpdateDynamicView(int rowId = 0);

    // ****** MEMBERS ******
    QMap<ActionScope, QString>      mActionLabels;          ///<  List of action labels
    QList<DbKeyData*>               mDatasStaticKey;        ///<  List of static keys
    QList<DbKeyData*>               mDatasDynamicKey;       ///<  List of dynamic keys
    QList<DbKeyStaticRowItems *>    mRowsStaticKey;         ///<  List of layout (static key) lines
    QList<DbKeyDynRowItems *>       mRowsDynKey;            ///<  List of layout (dynamic key) lines
    GS::QtLib::DatakeysContent      mOriginalDbKeyContent;  ///<  Copy of original key content
    QStringList                     mFilesGroup;            ///<  Group of files to import with the original stdf
    RegExpValidator                 *mRegExpValidator;      ///<  Ptr to regexp validator
    QVBoxLayout                     *mUiStaticVLayout;      ///<  Ptr to layout contaning static widgets
    QVBoxLayout                     *mUiDynVLayout;         ///<  Ptr to layout contaning dyn widgets
    OpenMode                        mOpenMode;              ///<  to know if it has been opened while inserting stdf or with the toolbox
    Ui::DbKeysEditor                *mUi;                   ///<  Ptr to UI
    QString                         mStdfFilePath;          ///<  stores original stdf file path
    QString                         mDbKeysFilePath;        ///<  stores path to db key file
    bool                            mChangesSaved;          ///<  true if all changes have been saved
    bool                            mIsDatasetValid;        ///<  true if no error in the dataset evaluation
    bool                            mShowDetailsSelected;   ///<  if true show the details edit box
};

} // END Gex
} // END GS

#endif // GEXCONFIGKEYSEDITOR_H
