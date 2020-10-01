#include "widget.h"

void Widget::UpdateGUI()
{
    QSharedPointer<Thread> lSP=mTWP.toStrongRef();
    if (lSP.isNull())
    {
        qDebug("Cannot strongify WeakPointer");
        return;
    }
    mLabel.setText(lSP->objectName() + " : " + lSP->GetCurrentvalue());
    //QApplication::processEvents();
}
