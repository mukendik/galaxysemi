#include "customdialogbox.h"

CustomDialogBox::CustomDialogBox(QWidget *poParent, const QString &strTitle):
    QDialog(poParent,Qt::Dialog){
   setWindowTitle(strTitle);
   m_poGridLayout = new QGridLayout(this);
   m_poButtonBox = 0;
   m_poSelectedButton = 0;
}

CustomDialogBox::~CustomDialogBox(){

}

void CustomDialogBox::addWidget(QWidget *poWidget){

    m_poGridLayout->addWidget(poWidget,m_poGridLayout->rowCount(),0,1,1);
}


QPushButton *CustomDialogBox::addButtonBox(const QString &strText, QDialogButtonBox::ButtonRole eRole){
    if(!m_poButtonBox){
        m_poButtonBox = new QDialogButtonBox(this);
        addWidget(m_poButtonBox);
        connect(m_poButtonBox, SIGNAL(clicked(QAbstractButton *)), this, SLOT(selectedButton(QAbstractButton *)));
    }
    QPushButton *poButton = m_poButtonBox->addButton(strText, eRole);
    connect(poButton, SIGNAL(clicked()), this, SLOT(accept()));
    return poButton;
}

void CustomDialogBox::selectedButton(QAbstractButton * poButton){
    m_poSelectedButton = qobject_cast<QPushButton*>(poButton);
}

QPushButton *CustomDialogBox::clickedButton(){
    return m_poSelectedButton;
}

QDialogButtonBox::ButtonRole CustomDialogBox::buttonRole(QPushButton *poButton){
    return m_poButtonBox->buttonRole(poButton);
}
