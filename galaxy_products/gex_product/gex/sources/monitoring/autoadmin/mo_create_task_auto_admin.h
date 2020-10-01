#ifndef MO_CREATE_TASK_AUTO_ADMIN_H
#define MO_CREATE_TASK_AUTO_ADMIN_H

#include <QDialog>

#include "ui_mo_autoadmin_dialog.h"
#include "autoadmin/autoadmin_taskdata.h"

class CollapsibleButton;
class CGexMoTaskAutoAdmin;
class FilePathWidget;

const QString C_ShellPassKey = "pass";
const QString C_ShellStdKey = "std";
const QString C_ShellCritKey = "crit";

const QString C_YieldTask = "yield";
const QString C_SpmTask = "spm";
const QString C_SyaTask = "sya";
const QString C_PatTask = "pat";

class GexMoCreateTaskAutoAdmin: public QDialog, public Ui::monitor_autoadmin_basedialog
{
    Q_OBJECT

public:
    //! \brief Auto Admin task dialog box.
    GexMoCreateTaskAutoAdmin( QWidget* parent = 0, bool modal = false, Qt::WindowFlags fl = 0 );
public slots:
    //! \brief Read/Save all fields entered.
    void OnOk(void);
    //! \brief Cancel...
    void OnCancel(void);
    //! \brief Select mailing list for email notification
    void OnMailingList(void);
    //! \brief Load dialog box fields with specified data structure.
    void LoadFields(CGexMoTaskAutoAdmin *ptTaskItem);
    //! \brief Returns data
    GexMoAutoAdminTaskData& GetData();

private:
    //! \brief Save Shell settings into internal class fields
    void SaveShellSettings();

    CollapsibleButton *CreateCollapsibleArea(const QString& title,
            const QString& key);

    GexMoAutoAdminTaskData mAutoAdminData;
    CollapsibleButton* mSPMShellsButton;
    CollapsibleButton* mSYAShellsButton;
    CollapsibleButton* mYieldShellsButton;
    CollapsibleButton* mPATShellsButton;

    QMap<QString, FilePathWidget*> mShellPathWidgets;
};

#endif // MO_CREATE_TASK_AUTO_ADMIN_H
