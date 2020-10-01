#ifndef MULTI_SELECT_INPUT_DIALOG_H
#define MULTI_SELECT_INPUT_DIALOG_H

#include <QDialog>

namespace Ui {
class MultiSelectInputDialog;
}

class MultiSelectInputDialog : public QDialog
{
    Q_OBJECT

public:

    static QList<QStringList> GetSelectedItems(QWidget *parent,
                                        const QString& title,
                                        const QString& label,
                                        const QList<QStringList>& inputItems,
                                        const QStringList& header,
                                        bool *ok);

private:
    MultiSelectInputDialog(QWidget *parent = 0);
    ~MultiSelectInputDialog();
    void SetLabel(const QString& label);
    void SetHeader(const QStringList& header);
    void SetInputItems(const QList<QStringList> &items);
    QList<QStringList> GetSelectedItems();

    Ui::MultiSelectInputDialog *mUi;
    QStringList mHeader;
};

#endif // MULTI_SELECT_INPUT_DIALOG_H
