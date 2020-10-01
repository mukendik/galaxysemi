#ifndef EVALUATION_DIALOG_H
#define EVALUATION_DIALOG_H

#include <QDialog>

namespace Ui {
class EvaluationDialog;
}
namespace GS
{
namespace LPPlugin
{


class EvaluationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EvaluationDialog(const QString &message, QWidget *parent = 0);
    ~EvaluationDialog();
public slots :
    // Methode called when Activate button is called
    void OnActivate();
    // Methode called when cancel button is called
    void OnCancel();
    // Get order id string
    QString GetOrderId();
private:
    Ui::EvaluationDialog *mUi;
    // OrderId attribute
    QString mOrderId;
};

}
}

#endif // EVALUATION_DIALOG_H
