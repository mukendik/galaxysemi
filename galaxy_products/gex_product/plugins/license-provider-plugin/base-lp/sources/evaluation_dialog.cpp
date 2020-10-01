#include "evaluation_dialog.h"
#include "ui_evaluation_dialog.h"

namespace GS
{
namespace LPPlugin
{
EvaluationDialog::EvaluationDialog(const QString &message, QWidget *parent) :
    QDialog(parent),
    mUi(new Ui::EvaluationDialog)
{
    mUi->setupUi(this);
    setWindowTitle("Evaluation license");
    setWindowIcon(QIcon(":/gex/icons/gex_application.png"));
    mUi->mEvalLicense->setText(message);

    mUi->mButtonBox->button(QDialogButtonBox::Cancel)->setText("Exit");

    QObject::connect(mUi->mActivate, SIGNAL(clicked()), this, SLOT(OnActivate()));
    QObject::connect(mUi->mButtonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(OnCancel()));
}

EvaluationDialog::~EvaluationDialog()
{
    delete mUi;
}

void EvaluationDialog::OnActivate()
{
    if(mUi->mOrderIDVal->text().isEmpty())
        return ;
    else
    {
        mOrderId = mUi->mOrderIDVal->text();
        done(QDialog::Accepted);
    }

}

void EvaluationDialog::OnCancel()
{
    done(QDialog::Rejected);
}

QString EvaluationDialog::GetOrderId()
{
    return mOrderId;
}

}
}
