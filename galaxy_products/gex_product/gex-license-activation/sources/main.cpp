#include "license_activation_dialog.h"
#include <QApplication>
#include <QMessageBox>
//#include <QPlastiqueStyle>
#include <QStyleFactory>


int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    QStyle *lFusion = QStyleFactory::create("Fusion");
    if(lFusion)
        a.setStyle(lFusion);

    GS::ActivationGui::LicenseActivation w;
    QString error;
    if(w.getError(error) != GS::ActivationGui::LicenseActivation::eNoError)
    {
        QMessageBox::critical(0, a.applicationName() + " - " + a.applicationVersion(),error);
        a.exit(1);
        return 1;
    }
    else
    {
        w.show();
        return a.exec();
    }

}
