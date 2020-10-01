#ifndef TREE_MODEL_REPORT_H
#define TREE_MODEL_REPORT_H


#include <QAbstractItemModel>
#include <QMimeData>
#include <QKeyEvent>
#include "component.h"
#include <tree_item_report.h>


class TreeMimeData : public QMimeData
{
    Q_OBJECT
  public:
    TreeMimeData():QMimeData(),mDragged(0), mParent(0), mComponent(0), mRow(-1){}
    ~TreeMimeData(){}

    T_VIEW_ITEM_TYPE    mType;
    QString             mText;
    TreeItemReport*     mDragged;
    TreeItemReport*     mParent;
    Component*          mComponent;
    QFont               mFont;
    int                 mRow;


};

class TreeModelReport : public QAbstractItemModel
{
    Q_OBJECT

    public:
       TreeModelReport(QObject *parent = 0);
       ~TreeModelReport();


       /**
        * @brief addSection - add a section in the tree
        * @param sectionName the ne section name
        * @param component the component to add
        * @return the TreeItemReport corresponding to the new section
        */
       TreeItemReport*          addSection(const QString& sectionName, Component *component);


       /**
        * @brief appendChild - create and add a TreeItemReport init from its name
        * @param nameItem
        * @return the created TreeItemReport
        */
       TreeItemReport*          appendChild(const QString& nameItem);


       TreeItemReport*          addItemToSection(TreeItemReport* section, const QString& elementName, Component* component);
       /**
        * @brief Empty the view
        */
       void                     clear  ();

       /**
        * @brief getItemAt - return the TrreItemReport at index if it exists
        * @param index
        * @return 0 if no item at this index
        */
       TreeItemReport*          getItemAt(QModelIndex index);


       void                     removeItem(TreeItemReport* item);

       /**
        * @brief reloadData - refresh the display of the indexModel
        * @param index - the index we want to reload
        * @param role - either Qt::DisplayRole or Qt::DecorationRole other are ignored
        */
       void                     reloadData(QModelIndex index, Qt::ItemDataRole role);

       /**
        * @brief retrieveIndexPosition set the current index position to the compoment element
        */
       void                     retrieveIndexPosition();

       void                     updateDataSection(QModelIndex index);

       /**
        * @brief updateData - Called this function for updating the name of an index
        * @param index      - the current selected index
        * @param name
        * @param type       - type of the element that has been changed
        */
        void                  updateDataName(QModelIndex index, const QString& name, T_VIEW_ITEM_TYPE type);

        QModelIndex           indexOfItem  (TreeItemReport* item, const QModelIndex& parent=QModelIndex());

        TreeItemReport*       sectionAt    (int index);
        int                   sectionsCount () const;
       /**
        * @brief Subclassing QAbstractItemModel methods (see Qt Doc)
        * http://doc.qt.io/qt-4.8/model-view-programming.html#using-drag-and-drop-with-item-views
        */
       QVariant         data                (const QModelIndex &index, int role) const;
       bool             dropMimeData        (const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
       QModelIndex      index               (int row, int column, const QModelIndex &parent = QModelIndex()) const;
       bool             insertRow           (int row, const QModelIndex & parent = QModelIndex()) ;
       QMimeData *      mimeData            (const QModelIndexList &indexes) const;
       QStringList      mimeTypes           () const;
       QModelIndex      parent              (const QModelIndex &index) const;
       bool             removeRows          (int row, int count, const QModelIndex & parent = QModelIndex());
       int              rowCount            (const QModelIndex &parent = QModelIndex()) const;
       int              columnCount         (const QModelIndex &parent= QModelIndex()) const;
       Qt::ItemFlags    flags               (const QModelIndex &index) const;
       QVariant         headerData          (int section, Qt::Orientation orientation, int role) const;
       bool             setData             (const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);

       Qt::DropActions  supportedDropActions() const;

 signals:
       void indexChanged        (const QModelIndex&);
       void signalChanged       ();
       void changedHasBeenMade  ();

private:
       QIcon&           initIcon        (T_Component type);
       QFont            initFont        (T_Component type);
       TreeItemReport*  insertNewItem   (int row,
                                         const QString& name,
                                         const QIcon& icon,
                                         const QFont &font,
                                         const QModelIndex& parent=QModelIndex());

       static QMap<T_Component, QIcon>    mIcons;
       TreeItemReport                     mRoot;
       QModelIndex                        mSelectedIndexOnDrop;
};


#endif // TREE_VIEW_REPORT_H

