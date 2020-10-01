#ifndef TASKS_MANAGER_GUI_H
#define TASKS_MANAGER_GUI_H

#include <QDialog>

namespace Ui {
    class TasksManagerGui;
}

namespace GS
{
namespace Gex
{

class TasksManagerGui : public QDialog
{
    Q_OBJECT
public :
    TasksManagerGui(QWidget *parent = 0);
    virtual ~TasksManagerGui();

signals :
    /// \brief signal that cancel button has been pushed
    void	sCancelled();

public slots :
    /// \brief Refresh the task list view
    void    OnRefreshTaskList(const QStringList &taskList);//
    /// \brief Show dialog
    void    OnShowDialog();

private slots:
    /// \brief emit cancel and hide dialog
    void	OnButtonCancel();

private :
    Q_DISABLE_COPY(TasksManagerGui);
    Ui::TasksManagerGui	*mUi; ///< pointer to GUI object
};

} // END Gex
} // END GS

#endif // TASKS_MANAGER_GUI_H
