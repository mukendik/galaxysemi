#ifndef SMSQLTABLEMODEL_H
#define SMSQLTABLEMODEL_H

#include <QSqlTableModel>

class CGexMoTaskStatisticalMonitoring;

class SMSQLTableModel : public QSqlTableModel
{
    Q_OBJECT
public:
    explicit SMSQLTableModel(CGexMoTaskStatisticalMonitoring* SMDraftTask,
                             QObject * parent = 0,
                             QSqlDatabase db = QSqlDatabase());
    QVariant data(const QModelIndex &index, int role) const;

private:
    CGexMoTaskStatisticalMonitoring*      mSMDraftTask;                  ///< Holds ptr to current draft

};

#endif // SMSQLTABLEMODEL_H
