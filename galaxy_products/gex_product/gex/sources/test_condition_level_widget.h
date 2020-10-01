#ifndef TEST_CONDITION_LEVEL_WIDGET_H
#define TEST_CONDITION_LEVEL_WIDGET_H

#include <QDialog>

namespace Ui {
class TestConditionLevelWidget;
}

namespace GS
{
namespace Gex
{

class TestConditionLevelWidget : public /*QDialog*/ QWidget
{
    Q_OBJECT

public:
    explicit TestConditionLevelWidget(QWidget *parent = 0);
    ~TestConditionLevelWidget();

    QStringList GetSelectedFields() const;

    void        AddSelectedField(const QString& field);
    void        AddSelectedFields(const QStringList &fields);
    void        SetAvailableFields(const QStringList& fields, bool keepSelection = false);
    void        SetSelectedFields(const QStringList& fields);

protected slots:

    void    OnEditAdd();
    void    OnEditRemove();
    void    OnTop();
    void    OnUp();
    void    OnDown();
    void    OnBottom();
    void    UpdateUI();

signals:

    void    levelChanged(const QStringList&);

private:

    Ui::TestConditionLevelWidget *  mUi;

    QStringList                     mFields;
};

}   // End namespace Gex
}   // End namespace GS
#endif // TEST_CONDITION_LEVEL_WIDGET_H
