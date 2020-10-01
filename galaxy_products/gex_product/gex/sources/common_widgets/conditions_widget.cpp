#include "conditions_widget.h"
#include <QLabel>
#include <QLineEdit>
#include <QGridLayout>

ConditionsWidget::ConditionsWidget(const  QMap<QString, QVariant> &mapListCondition, QWidget *poParent, const QStringList &strListCondition):QWidget(poParent){
    buildConditionsWidget(mapListCondition,strListCondition);
}

ConditionsWidget::ConditionsWidget(const QStringList &strListCondition, QWidget *poParent):QWidget(poParent){
    buildConditionsWidget(strListCondition);

}

ConditionsWidget::~ConditionsWidget(){
}


void ConditionsWidget::buildConditionsWidget(const  QMap<QString, QVariant> &mapListCondition, const QStringList &strListCondition){

    if(mapListCondition.isEmpty())
        return;
    if(strListCondition.isEmpty()){
        QGridLayout *poGridLayout = new QGridLayout(this);
        int iIdx=0;
        foreach(const QString &strKey, mapListCondition.keys()){
            QLabel *poLabel = new QLabel(strKey + QString(" :"),this);
            poGridLayout->addWidget(poLabel, iIdx, 0, 1, 1);
            QLineEdit *poLineEdit = new QLineEdit(this);
            poLineEdit->setText(mapListCondition[strKey].toString());
            poGridLayout->addWidget(poLineEdit, iIdx, 1, 1, 1);
            poLineEdit->setProperty("conditions",strKey);
            iIdx++;
        }
    }else {
        buildConditionsWidget(strListCondition);
        QList<QLineEdit *> oLineEditList = findChildren<QLineEdit*>();
        for(int iIdx=0;iIdx<oLineEditList.count();iIdx++){
            QString strKey = oLineEditList[iIdx]->property("conditions").toString();
            oLineEditList[iIdx]->setText(mapListCondition[strKey].toString());
        }
    }
}


void ConditionsWidget::buildConditionsWidget(const QStringList &strListCondition){
    if(strListCondition.isEmpty())
        return;
    QGridLayout *poGridLayout = new QGridLayout(this);

    for(int iIdx=0; iIdx<strListCondition.count();iIdx++){
        QLabel *poLabel = new QLabel(strListCondition[iIdx] + QString(" :") ,this);
        poGridLayout->addWidget(poLabel, iIdx, 0, 1, 1);
        QLineEdit *poLineEdit = new QLineEdit(this);
        poGridLayout->addWidget(poLineEdit, iIdx, 1, 1, 1);
        poLineEdit->setProperty("conditions",strListCondition[iIdx]);
    }
}

QMap<QString, QVariant> ConditionsWidget::getConditionsValues(){

    QMap<QString, QVariant> oMapConditions ;
    QList<QLineEdit *> oLineEditList = findChildren<QLineEdit*>();
    for(int iIdx=0;iIdx<oLineEditList.count();iIdx++){
        QString strKey = oLineEditList[iIdx]->property("conditions").toString();
        oMapConditions[strKey] =  oLineEditList[iIdx]->text();
    }
    return oMapConditions;

}
