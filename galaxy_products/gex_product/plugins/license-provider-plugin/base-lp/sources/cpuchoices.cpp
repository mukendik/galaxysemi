#include "cpuchoices.h"
#include "ui_cpuchoices.h"

namespace GS
{
namespace LPPlugin
{

CPUChoices::CPUChoices(const QString &lMessage, const QStringList &lAvailableProducts, bool activate, QWidget *parent) :
    QDialog(parent),
    mUi(new Ui::CPUChoices)
{
    mUi->setupUi(this);
    setWindowTitle("Available license choice");
    setWindowIcon(QIcon(":/gex/icons/gex_application.png"));
    mUi->mMessageError->setText(lMessage);
    mUi->mAvailableProducts->addItems(lAvailableProducts);

    if(!activate)
        mUi->mButtonBox->button(QDialogButtonBox::Ok)->hide();
    QObject::connect(mUi->mUseProduct, SIGNAL(clicked()), this, SLOT(OnUseButton()));
    if(activate)
        QObject::connect(mUi->mButtonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(onActivate()));
    QObject::connect(mUi->mButtonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(onExit()));

    if(activate)
        mUi->mButtonBox->button(QDialogButtonBox::Ok)->setText("Activate");
    mUi->mButtonBox->button(QDialogButtonBox::Cancel)->setText("Exit");

    mProductToBeUsed = "";
    mChoice = -1;

}
CPUChoices::~CPUChoices()
{
    delete mUi;
}

void CPUChoices::OnUseButton()
{
    mProductToBeUsed = mUi->mAvailableProducts->currentText();
    mChoice= 0;
    done(QDialog::Accepted);
}

void CPUChoices::onActivate()
{
    mChoice= 1;
    done(QDialog::Accepted);
}

void CPUChoices::onExit()
{
    mChoice= 2;
    done(QDialog::Rejected);
}

QString CPUChoices::GetProductToBeUsed()
{
    return mProductToBeUsed;
}

int CPUChoices::GetChoice()
{
    return mChoice;
}
void CPUChoices::SetChoice(int choice)
{
    mChoice = choice;
}

}
}
