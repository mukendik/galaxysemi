#ifndef CONDITIONS_WIDGET_H
#define CONDITIONS_WIDGET_H

#include <QWidget>
#include <QMap>
#include <QString>
#include <QVariant>

class ConditionsWidget : public QWidget
{
public:
    ConditionsWidget(const  QMap<QString, QVariant> &mapListCondition, QWidget *poParent = 0, const QStringList &strListCondition=QStringList());
    ConditionsWidget(const QStringList &strListCondition, QWidget *poParent = 0);
    virtual ~ConditionsWidget();
    void buildConditionsWidget(const  QMap<QString, QVariant> &mapListCondition, const QStringList &strListCondition=QStringList());
    void buildConditionsWidget(const QStringList &strListCondition);
    QMap<QString, QVariant> getConditionsValues();
};

#endif // CONDITIONS_WIDGET_H
