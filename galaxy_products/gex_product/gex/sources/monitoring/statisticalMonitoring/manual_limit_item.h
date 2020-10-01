#ifndef MANUAL_LIMIT_ITEM_H
#define MANUAL_LIMIT_ITEM_H

#include <QWidget>
#include "gs_types.h"

namespace Ui {
class manual_limit_item;
}

class ManualLimitItem : public QWidget
{
    Q_OBJECT

public:
    explicit ManualLimitItem(int index, int num, QString name, QWidget *parent = 0);
    ~ManualLimitItem();

    int GetIndex() const;
    gsint32 GetTestNumber() const;
    QString GetTestName();

    void setPlaceHolderLabelNumber(const QString& label);
    void setPlaceHolderLabelName(const QString& label);

public slots:
    void onAddItem();
    void onDeleteItem();
private:
    Ui::manual_limit_item *ui;
    int             mIndex;
signals:
    void sDeleteItem(int);
    void sAddItem();
};

#endif // LIMIT_ITEM_H
