#include "buttonmenucombobox.h"
#include <QStylePainter>
#include <QMenu>

ButtonMenuComboBox::ButtonMenuComboBox(QWidget *parent):QToolButton(parent)
{
    setPopupMode(QToolButton::MenuButtonPopup);
    setToolButtonStyle(Qt::ToolButtonTextOnly);
    setArrowType(Qt::NoArrow);
    connect(this, SIGNAL(clicked()), this, SLOT(ButtonClicked()));
}

ButtonMenuComboBox::~ButtonMenuComboBox()
{

}

void ButtonMenuComboBox::AddItem(const QString &action)
{
    if(!menu())
    {
        QMenu *lMenu = new QMenu(action);
        setMenu(lMenu);
        setText(action);
        connect(lMenu, SIGNAL(triggered(QAction *)), this, SLOT(ManageActions(QAction *)));
    }

    menu()->addAction(action);
}

void ButtonMenuComboBox::ManageActions(QAction *action)
{
    if (action)
        setText(action->text());
    // remove comment to enable execute action on action selection
    //    emit executeAction(text());
}

void ButtonMenuComboBox::ButtonClicked()
{
    emit executeAction(text());
}

void ButtonMenuComboBox::ClearItems()
{
    if(menu())
        menu()->clear();
}

