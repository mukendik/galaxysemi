#ifndef TREE_ITEM_REPORT_H
#define TREE_ITEM_REPORT_H

#include <QVariant>
#include <QFont>
#include <QIcon>
#include <QString>


enum T_VIEW_ITEM_TYPE { T_ROOT_ITEM =0,T_SECTION_ITEM , T_ELEMENT_ITEM, T_NO_ITEM};

class Component;
class TreeItemReport
{

public:

    explicit TreeItemReport(const QString &name=QString(), const QIcon& icon=QIcon(), TreeItemReport* parent = 0);
    ~TreeItemReport();

    /**
     * @brief AppendChild - add a TreeItemReport as a child to the current instance
     * @param item
     */
    void                    AppendChild (TreeItemReport *item);


    void                    Clear       () ;

    /**
     * @brief Child
     * @param row
     * @return if row out of bound null, otherwise the TreeItemReport at row
     */
    TreeItemReport*         Child       (int row);

    /**
     * @brief ChildCount
     * @return the number of childs
     */
    int                     ChildCount  () const;
    int                     ColumnCount () const;

    QVariant                Data        (int index) const;
    TreeItemReport*         FindChild   (const QString& name);
    TreeItemReport*         GetParent   () const;
    void                    SetParent   (TreeItemReport* parent);
    QIcon                   Icon        () const;
    QFont                   Font        () const;

    void                    InsertChild     (int indexOf, TreeItemReport *item);
    void                    RemoveChild     (int indexOf);

    /**
     * @brief Row
     * @return the row index of this TreeItemReport
     */
    int                     Row         () const;
    void                    SetData     (const QVariant & variant, int role);

    Component*              GetComponent() const;
    void                    SetComponent(Component* component);

    T_VIEW_ITEM_TYPE        GetType     () const;
    void                    SetItemType (T_VIEW_ITEM_TYPE itemType);
private:

    T_VIEW_ITEM_TYPE                mItemType;
    QList<QVariant>                 mData;
    QList<TreeItemReport*>          mChilds;


    QIcon                           mIcon;
    QFont                           mFont;
    TreeItemReport                  *mParent;
    Component                       *mComponent;

};

#endif // TREE_ELEMENT_H

