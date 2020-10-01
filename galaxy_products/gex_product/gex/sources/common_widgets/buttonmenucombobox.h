#ifndef BUTTONMENUCOMBOBOX_H
#define BUTTONMENUCOMBOBOX_H

#include <QToolButton>

class QAction;

class ButtonMenuComboBox : public QToolButton
{
    Q_OBJECT
public:
        ButtonMenuComboBox(QWidget *parent=0);
        ~ButtonMenuComboBox();
public:
        void AddItem(const QString &action);
        void ClearItems();
protected slots:
        void ManageActions (QAction *action);
        void ButtonClicked();
signals :
        void executeAction(const QString &action);
};

#endif // BUTTONMENUCOMBOBOX_H
