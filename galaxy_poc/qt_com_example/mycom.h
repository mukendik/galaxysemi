#ifndef MY_COM_H
#define MY_COM_H

#include <QObject>
#include <QAxObject>
#include <QString>

class MyCOM : public QObject
{
    Q_OBJECT

public:
    MyCOM();
    ~MyCOM();

private:
    QAxObject*  mComObject;
    QAxObject*  mComSubObject;

signals:
    void    sComInitialized(const QString & doc);
    void    sSubObjectInitialized(const QString & doc);
    void    sStatus(const QString & status);

public slots:
    void    onInit();
    void    onQuery();
};

#endif // MY_COM_H
