#ifndef DBC_FORM_PARAM_RESULTS_H
#define DBC_FORM_PARAM_RESULTS_H

#include <QWidget>
#include <QLineEdit>
#include <QVariant>
#include <QDoubleSpinBox>
#include <QItemDelegate>

class Gate_DataResult;
class Gate_ParameterDef;
class DbcTransaction;
class QSqlTableModel;
class QSortFilterProxyModel;
class ParamSqlTableModel;

namespace Ui {
    class DbcFormParamResults;
}

class DbcFormParamResults : public QWidget {
    Q_OBJECT
public:
    DbcFormParamResults(DbcTransaction& dbTransac, Gate_ParameterDef& paramInfo, QSqlTableModel *pTableModel, ParamSqlTableModel *pTableModelInfo, QWidget *parent = 0);
    ~DbcFormParamResults();
	void setDbcTransac(DbcTransaction& dbTransac);
	void setParamInfo(Gate_ParameterDef& paramInfo);
	void setResultsModel(QSqlTableModel *pSqlTableModel);
	void setInfoModel(ParamSqlTableModel *pParamTableModel);
	void initModel();
	void initGui();
	void resizeTableView();

public slots:
	void refreshGui();
	void onCustomContextMenuRequested();
	void onEditParameterRequested();
	
signals:
	void sChanged();

private:
    Ui::DbcFormParamResults *m_ui;
	DbcTransaction			*m_pDbcTransac;
	Gate_ParameterDef		*m_paramInfo;
	QSortFilterProxyModel	*m_pProxyModel;
	QSqlTableModel			*m_pTableModelResults;
	ParamSqlTableModel		*m_pTableModelInfo;
};

class ParamResultDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    ParamResultDelegate(QObject *parent)
            : QItemDelegate(parent)
    {
    }

//    void paint(QPainter *painter, const QStyleOptionViewItem &option,
//               const QModelIndex &index) const
//    {
//        int idateTimeEdit = qVariantValue<int>(index.data());
//        drawDisplay(painter, option, option.rect, QDateTime::fromTime_t(idateTimeEdit).toString("yyyy-MM-dd"));
//    }

//	QSize sizeHint(const QStyleOptionViewItem& /*option*/,
//                   const QModelIndex &index) const
//    {
//        int idateTimeEdit = qVariantValue<int>(index.data());
//        QDateTimeEdit dateTimeEdit(QDateTime::fromTime_t(idateTimeEdit));
//        return dateTimeEdit.sizeHint();
//    }
	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/, const QModelIndex &index) const
	{
		bool bIsdouble = false;
		QVariant variant = index.data(Qt::EditRole);
		variant.toDouble(&bIsdouble);
//		if (bIsdouble)
//		{
//			QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
//			editor->setCorrectionMode(QDoubleSpinBox::CorrectToNearestValue);
//			editor->setRange(-999999999.000000, 999999999.000000);
//			editor->setButtonSymbols(QAbstractSpinBox::NoButtons);
//			editor->setDecimals(20);
//			return editor;
//		}
//		else
//		{
			QLineEdit *editor = new QLineEdit(parent);
			return editor;
//		}
	}
	
	void setEditorData(QWidget *editor, const QModelIndex &index) const
	{
		QVariant variant = index.data(Qt::EditRole);
		bool bIsdouble = false;
		variant.toDouble(&bIsdouble);
//		if (bIsdouble)
//		{
//			QDoubleSpinBox *widget = static_cast<QDoubleSpinBox *>(editor);
//			widget->setValue(variant.toDouble());
//		}
//		else
//		{
			QLineEdit *widget = static_cast<QLineEdit *>(editor);
			widget->setText(variant.toString());
//		}
	}
	
	void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
	{
		bool bIsdouble = false;
		index.data(Qt::EditRole).toDouble(&bIsdouble);
//		if (bIsdouble)
//		{
//			QDoubleSpinBox *widget = static_cast<QDoubleSpinBox *>(editor);
//			widget->interpretText();
//			model->setData(index, widget->value(), Qt::EditRole);
//		}
//		else
//		{
			QLineEdit *widget = static_cast<QLineEdit *>(editor);
			model->setData(index, widget->text(), Qt::EditRole);
//		}
	}
	
	void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/*index*/) const
	{
		editor->setGeometry(option.rect);
	}
};

#endif // DBC_FORM_PARAM_RESULTS_H
