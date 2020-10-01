/****************************************************************************
** Deriven from g-trigger_dialogbase.h
****************************************************************************/

#ifndef G_TRIGGERDIALOG_H
#define G_TRIGGERDIALOG_H

#include <QDialog>

#include "ui_g-trigger_dialogbase.h"

namespace GS
{
namespace GTrigger
{

class GTriggerEngine;

class GTriggerDialog : public QDialog, public Ui::G_TriggerDialogBase
{
    Q_OBJECT

public:
    GTriggerDialog(const QString & AppName, const QString & AppPath, QWidget* parent = 0, Qt::WindowFlags fl = 0);
    ~GTriggerDialog();

protected:
    void closeEvent ( QCloseEvent * e );

private:
    // Application name
    QString         mAppName;
    // Application path
    QString         mAppPath;
    // GUI functions
    void            InsertStatusLog(const QString & Line);
    void            InsertSettingsLog(const QString & Line);

signals:
    void            sAboutToClose();

protected slots:
    void            OnInsertStatusLog(const QString & Line);
    void            OnInsertSettingsLog(const QString & Line);
    void            Close();
};

} // namespace GS
} // namespace GTrigger

#endif // G_TRIGGERDIALOG_H
