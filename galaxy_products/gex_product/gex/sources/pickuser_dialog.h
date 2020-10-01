#ifndef PICKUSER_DIALOG_H
#define PICKUSER_DIALOG_H

#include "ui_pickuser_dialog.h"

/////////////////////////////////////////////////////////////////////////////
class PickUserDialog : public QDialog, public Ui::PickUserDialogBase
{
    Q_OBJECT

public:

    PickUserDialog(QWidget* parent = 0, bool modal = false, Qt::WindowFlags f = 0 );

    void	setLoadMode(bool bLoad);
    void	getSelectedUserName(QString &strUserName, QString &strProfileFile);

private:

    void	FillList(void);
    void	writeDefaultProfile(const QString& strDefaultProfile);

    QString m_strDefaultUserName;

private slots:

    void	OnRemoveUser(void);
    void	OnProperties(void);
    void	onItemClicked(void);
    void	onItemDoubleClicked(void);
    void	onItemChanged(QTreeWidgetItem * pItem, int nColumn);
};

#endif // PICKUSER_DIALOG_H
