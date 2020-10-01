#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QWeakPointer>
#include "thread.h"

class Widget: public QWidget
{
    Q_OBJECT

    public:
        Widget( QWeakPointer<Thread> p1):QWidget(0), mLabel("0000", this), mTWP(p1)
        {
            setLayout(new QVBoxLayout());
            setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            show();
        }
    public slots:
        void UpdateGUI();
    private:
        QLabel mLabel;
        QWeakPointer<Thread> mTWP;
};

#endif // WIDGET_H
