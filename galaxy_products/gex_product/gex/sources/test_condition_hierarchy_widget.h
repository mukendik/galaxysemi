#ifndef TEST_CONDITION_HIERACHY_WIDGET_H
#define TEST_CONDITION_HIERACHY_WIDGET_H

#include <QDialog>
#include <QMap>
#include <QHash>

namespace Ui
{
    class TestConditionHierarchyWidget;
}   // end namespace Ui

class QTreeWidgetItem;

namespace GS
{
namespace Gex
{

class TestConditionHierarchyWidget : public QWidget
{
    Q_OBJECT

public:

    TestConditionHierarchyWidget(QWidget * parent = NULL);
    virtual ~TestConditionHierarchyWidget();

    QStringList                     GetItems() const;
    QList<QMap<QString, QString> >  GetHierarchy() const;

    void            Clear();
    void            SetConditionsGroups(const QList<QMap<QString, QString> >& conditionsGroups);
    void            SetFilters(const QHash<QString, QString>& filters);

    static bool     OrderConditionsGroups(const QMap<QString,QString>& lMap1,
                                          const QMap<QString,QString>& lMap2);

signals:
    void sEmptyCond(QString aCondition);

public slots:

    void            SetConditionLevel(const QStringList& conditionLevel);

protected slots:

    void            OnMoveUp();
    void            OnMoveDown();
    void            OnMoveTop();
    void            OnMoveBottom();
    void            InitUI();

private:

    int                     GetItemLevel(QTreeWidgetItem * pItem);
    QList<QTreeWidgetItem*> GetItemsAtLevel(int lLevel);
    void                    HierarchyChanged(QTreeWidgetItem * pItem);
    void                    ApplyLevelHierarchy(const QStringList& lHierarchy,
                                                int lLevel);
    void                    AddItem(const QMap<QString, QString>& conditionsGroup);
    void                    RemoveItem(const QString &lHashKey);
    void                    ReloadHierarchy(const QStringList &lHierarchy,
                                            const QStringList &lOldConditionLevel);

    bool                    IsFiltered(const QString& field, const QString& value);
    bool                    IsMatchingFilter(const QMap<QString, QString>& conditionsGroup);

    QStringList             GetHierarchy();

    QString                 MakeHashKey(const QMap<QString, QString>& conditionsGroup,
                                        const QList<QString> &conditionLevel);

private:

    Ui::TestConditionHierarchyWidget    *   mUi;
    QStringList                             mConditionLevel;
    QList<QMap<QString, QString> >          mConditionsGroup;
    QHash<QString, QString>                 mFilters;
    QHash<QString, QTreeWidgetItem*>        mVisibleNode;
};

}   // end namespace Gex
}   // end namespace GS
#endif // TEST_CONDITION_HIERACHY_WIDGET_H
