#ifndef DB_DATAKEYS_DIALOG_H
#define DB_DATAKEYS_DIALOG_H

#include "ui_db_datakeys_dialog.h"
#include "gqtl_datakeys.h"

class GexDatabaseEditKeys : public QDialog, public Ui::db_datakeys_basedialog
{
    Q_OBJECT

public:

    GexDatabaseEditKeys( QWidget* parent = 0, bool modal = false, Qt::WindowFlags fl = 0 );

    void    setKeys(GS::QtLib::DatakeysContent *pKeys);
    void    getKeys(GS::QtLib::DatakeysContent *pKeys);
    bool    loadConfigFileKeys(GS::QtLib::DatakeysContent *pKeys,bool bEraseConfigFile,bool &bFailedValidationStep,int *pnLineNb,QString &strError, QString strMoTaskSpoolingDir="");
    static  void deleteConfigFileKeys(GS::QtLib::DatakeysContent *pKeys);
    static  void renameConfigFileKeys(GS::QtLib::DatakeysContent *pKeys, const QString& newExtension);
    static  void moveConfigFileKeys(GS::QtLib::DatakeysContent *pKeys, const QString& destDir);

    static  bool parseValue(GS::QtLib::DatakeysContent *pKeys,QString &strExpression,QString &strResult,QString &strKeyValue,bool &bMatch,bool &bValidResult,QString &strError);

private:

    bool                    bEdited;        // 'true' if some keys have been edited.
    GS::QtLib::DatakeysContent  cDefaultKeys;   // Holds a copy of the default keys

    static bool    parseValueFieldSection(GS::QtLib::DatakeysContent *pKeys,QString &strExpression,QString &strResult,QString &strKeyValue,QString &strError);
    static bool    parseValueFieldRegExp(GS::QtLib::DatakeysContent *pKeys,QString &strExpression,QString &strResult,QString &strKeyValue,bool &bMatch,bool &bValidResult,QString &strError);
    static bool    parseValueDate(GS::QtLib::DatakeysContent *pKeys,QString &strExpression,QString &strResult,QString &strKeyValue,QString &strError);

public slots:

    void    OnEdits(const QString&);
    void    OnLoadFromFile(void);
    void    OnReloadDefault(void);
};

#endif // DB_DATAKEYS_DIALOG_H
