#ifndef CUSTOMDIALOGBOX_H
#define CUSTOMDIALOGBOX_H

#include <QDialog>
#include <QPushButton>
#include <QGridLayout>
#include <QDialogButtonBox>

class CustomDialogBox : public QDialog{
    Q_OBJECT
public:
    CustomDialogBox(QWidget *poParent, const QString &strTitle);
    virtual ~CustomDialogBox();
    void addWidget(QWidget *poWidget);
    QPushButton *addButtonBox(const QString &strText, QDialogButtonBox::ButtonRole eRole);
    QPushButton *clickedButton();
    QDialogButtonBox::ButtonRole buttonRole(QPushButton *poButton);
protected:
    QGridLayout *m_poGridLayout;
    QDialogButtonBox *m_poButtonBox;
    QPushButton *m_poSelectedButton;
private slots:
    void selectedButton(QAbstractButton * poButton);
};

#endif // CUSTOMDIALOGBOX_H
