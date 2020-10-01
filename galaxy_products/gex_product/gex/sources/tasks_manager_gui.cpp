#include <gqtl_log.h>

#include "tasks_manager_gui.h"
#include "ui_tasks_manager_gui.h"

namespace GS
{
namespace Gex
{

///////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////
TasksManagerGui::TasksManagerGui(QWidget *parent) :
    QDialog(parent,Qt::CustomizeWindowHint | Qt::WindowTitleHint),
    mUi(new Ui::TasksManagerGui)
{
    mUi->setupUi(this);
    connect(mUi->pushButtonCancel, SIGNAL(clicked()), this, SLOT(OnButtonCancel()));
    setWindowTitle("Task Manager");
}

///////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////
TasksManagerGui::~TasksManagerGui()
{
    delete mUi;
}

///////////////////////////////////////////////////////////////////
// On click on button cancel
////////////////////////////////////////////////////////////////////
void TasksManagerGui::OnButtonCancel()
{
    emit sCancelled();
    hide();
}

///////////////////////////////////////////////////////////////////
// Refresh the task list view
////////////////////////////////////////////////////////////////////
void TasksManagerGui::OnRefreshTaskList(const QStringList &taskList)
{
    // Clear the view
    mUi->treeWidgetTaskList->clear();

    if (taskList.isEmpty())
    {
        hide();
        return;
    }

    // Have some pending tasks
    QStringList::const_iterator lItTask = taskList.begin();
    QList<QTreeWidgetItem *> lTreeItems;
    while (lItTask != taskList.end())
    {
        // Add a task to the list view
        lTreeItems.append(new QTreeWidgetItem((QTreeWidget*)0, (*lItTask).split(";")));
        ++lItTask;
    }

    mUi->treeWidgetTaskList->insertTopLevelItems(0, lTreeItems);
}

void TasksManagerGui::OnShowDialog()
{
    if (mUi->treeWidgetTaskList->topLevelItemCount() == 0)
        hide();
    else
        show();
}

} // END Gex
} // END GS

