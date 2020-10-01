#include "sm_sql_table_model.h"
#include "spm_task.h"
#include <QColor>
#include <QIcon>
#include <QDateTime>

SMSQLTableModel::SMSQLTableModel(CGexMoTaskStatisticalMonitoring* SMDraftTask, QObject * parent, QSqlDatabase db): QSqlTableModel(parent, db), mSMDraftTask(SMDraftTask)
{

}

QVariant SMSQLTableModel::data(const QModelIndex &index, int role) const
{
    if(index.isValid())
    {
        if(role == Qt::DecorationRole)
        {
            if(index.column() == 1)
            {
                int lSpmId = QSqlTableModel::data(index, Qt::DisplayRole).toInt();
                if(lSpmId== mSMDraftTask->GetActiveProdVersionId())
                     return QVariant(QIcon(":/gex/icons/clean_undo_stack.png"));
            }
        }
        else if(role == Qt::DisplayRole)
        {
            QString lColumnName = headerData(index.column(), Qt::Horizontal).toString().toLower();
            if (lColumnName.contains("date") || lColumnName.contains("computed "))
            {
                return QSqlTableModel::data(index, Qt::DisplayRole).toDateTime().toString(Qt::ISODate);
            }
            else
            {
                return QSqlTableModel::data(index, Qt::DisplayRole);
            }
        }
        else if(role == Qt::ToolTipRole)
        {
            return QSqlTableModel::data(index, Qt::DisplayRole);
        }
    }
    return QVariant();
}
