#ifndef PAT_GDBN_WEIGHTING_MATRIX_WIDGET_H
#define PAT_GDBN_WEIGHTING_MATRIX_WIDGET_H

#include <QItemDelegate>
#include <QWidget>
#include "pat_gdbn_weighting_ring.h"

namespace Ui {
class PatGdbnWeightingMatrixWidget;
}

class PatGdbnWeightingRingDelegate : public QItemDelegate
 {
     Q_OBJECT

 public:

     PatGdbnWeightingRingDelegate(QObject *parent = 0);

     QWidget *  createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                const QModelIndex &index) const;

     void       setEditorData(QWidget *editor, const QModelIndex &index) const;
     void       setModelData(QWidget *editor, QAbstractItemModel *model,
                                const QModelIndex &index) const;

     void       updateEditorGeometry(QWidget *editor,
                                     const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const;
 };

class PatGdbnWeightingMatrixWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PatGdbnWeightingMatrixWidget(QWidget *parent = 0);
    ~PatGdbnWeightingMatrixWidget();

    QList<PatGdbnWeightingRing> rings();

    void                        setRings(const QList<PatGdbnWeightingRing> rings);

protected:

    void                        updateMatrixSize(unsigned int matrixSize);

private slots:

    void                        onComboMatrixChanged(int);
    void                        onMatrixNxNChanged(int matrixSize);

private:

    Ui::PatGdbnWeightingMatrixWidget *ui;
};

#endif // PAT_GDBN_WEIGHTING_MATRIX_WIDGET_H
