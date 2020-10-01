#ifndef CPUCHOICES_H
#define CPUCHOICES_H

#include <QDialog>

namespace Ui
{
class CPUChoices;
}
namespace GS
{
namespace LPPlugin
{

class CPUChoices : public QDialog
{
    Q_OBJECT

public:
    explicit CPUChoices(const QString &lMessage, const QStringList &lAvailableProducts,bool actuvate,QWidget *parent = 0);
    ~CPUChoices();
public slots:
    // slot called when Use button is called to use the chosen alternative product
    void OnUseButton();
    // slot called when activate button is called to run galaxy-la and activate
    void onActivate();
    // slot called by exit button to exit
    void onExit();
    // slot returning the the alternative product to be used
    QString GetProductToBeUsed();
    // slot returning the user choice
    int GetChoice();
    // slot to set user choice
    void SetChoice(int choice);

private:
    Ui::CPUChoices *mUi;
    // alternative product to be used
    QString mProductToBeUsed;
    // user choice
    int mChoice;
};

}
}
#endif // CPUCHOICES_H
