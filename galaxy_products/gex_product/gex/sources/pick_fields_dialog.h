#ifndef PICK_FIELDS_DIALOG_H
#define PICK_FIELDS_DIALOG_H

#include <QDialog>

class QTreeWidgetItem;

namespace Ui {
    class PickFieldsDialog;
}

struct PickFieldsData;

class PickFieldsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PickFieldsDialog(const QStringList& fieldList, QWidget *parent = 0);
    ~PickFieldsDialog();

    const QStringList&  selectedFieldList() const;
    void                setSelectedFieldList(const QStringList& fieldList);

protected:

private slots:

    void	onItemChanged(QTreeWidgetItem * pItem, int nColumn);
    void	onItemDoubleClicked(QTreeWidgetItem * pItem, int nColumn);

private:
    Ui::PickFieldsDialog *  ui;
    PickFieldsData *        mpPrivate;
};

#endif // PICK_FIELDS_DIALOG_H
