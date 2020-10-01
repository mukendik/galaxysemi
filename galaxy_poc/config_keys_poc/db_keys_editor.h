#ifndef GEXCONFIGKEYSEDITOR_H
#define GEXCONFIGKEYSEDITOR_H

#include <QMainWindow>

class RegExpValidator;
class GexDatabaseKeysContent;
class DbKeyData;

namespace Ui {
class DbKeysEditor;
}

class DbKeysEditor : public QMainWindow
{
    Q_OBJECT
    
public:
    DbKeysEditor(/*DbKeysEngine *dbKeysEngine, */ QWidget *parent = 0);
    ~DbKeysEditor();

private slots:
    void OnShowRegExpValidatorRequested();

private:
    void LoadGui();

    // ***** REGEXP VALID ******
    void ShowRegExpValidator(const QString &strRegExp);

    // ****** MODEL *******
    // Loading
    bool LoadDefaultData();  // load default datamodel
    bool LoadDefaultStaticKeys();  // load default static keys datamodel
    bool LoadDefaultDynamicKeys(); // load default dynamic keys datamodel
    // Insert
    bool InsertStaticKeyData(/*DbKeyInfo info, int row = -1*/); // Insert new Key in static data model
    bool InsertDynamicKeyData(/*DbKeyInfo info, int row = -1*/); // Insert new Key in dyn data model
    // Update
    bool UpdateStaticKeysData(/*QString value, int row, int column*/); // Update Key in static data model
    bool UpdateDynamicKeysData(/*QString value, int row, int column*/); // Update Key in dyn data model
    // Remove
    bool RemoveStaticKeyData(/*int row*/); // Remove Key from static data model
    bool RemoveDynamicKeyData(/*int row*/); // Remove Key from dyn data model

    // ****** VIEW *******
    // Build
    bool BuildStaticKeysView(); // Draw the list of static DB keys into the tab widget
    bool InsertStaticKeyRow(/*DbKeyInfo*/); // instanciates new line insert it at row
    bool BuildDynamicKeysView(); // Draw the list of dyn DB keys into the tab widget
    bool InsertDynamicKeyRow(/*DbKeyInfo*/); // instanciates new line insert it at row
    // Refresh


    /**/   // List of static keys
    /**/   // List of dynamic keys

    /**/   // List of layout (static key) lines
    /**/   // List of layout (dynamic key) lines

    GexDatabaseKeysContent *mOriginalDbKeyContent; // ptr to original key content
    GexDatabaseKeysContent *mCurrentDbKeyContent; // ptr to current key content


    RegExpValidator * mRegExpValidator;
    Ui::DbKeysEditor *mUi;
};

#endif // GEXCONFIGKEYSEDITOR_H
