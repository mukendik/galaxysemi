#ifndef _GEX_INPUT_DIALOG_H_
#define _GEX_INPUT_DIALOG_H_

///////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////
#include <QDialog>
#include <QString>
#include <QVariant>

class QWidget;
class QVBoxLayout;
class QTextEdit;

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexAbstractInputItem
//
// Description	:	Abstract class used to manage data with an input item
//
///////////////////////////////////////////////////////////////////////////////////
class GexAbstractInputItem : public QObject
{
public:

	GexAbstractInputItem(const QVariant& varDefault, const QString& strLabel, const QString& strTooltip);
	virtual ~GexAbstractInputItem();

	virtual QWidget *	createInputItem(QWidget * pParent) = 0;					// create the widget which manages this item

	const QVariant&		value() const							{ return m_varValue; }

protected:

	const QString&		label() const							{ return m_strLabel; }
	const QString&		tooltip() const							{ return m_strTooltip; }

	void				setValue(const QVariant& varValue)		{ m_varValue = varValue; }

private:

	QVariant			m_varValue;												// item value
	QString				m_strLabel;												// label associated with the item
	QString				m_strTooltip;											// tooltip associated with the item
};

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexDoubleInputItem
//
// Description	:	class used to manage double with an input item
//
///////////////////////////////////////////////////////////////////////////////////
class GexDoubleInputItem : public GexAbstractInputItem
{
	Q_OBJECT

public:

	GexDoubleInputItem(double dDefaultValue, const QString& strLabel = QString(), const QString& strTooltip = QString());
	virtual ~GexDoubleInputItem();

	QWidget *			createInputItem(QWidget * pParent);						// create the widget which manages this item

	void				setMaximum(double dMaximum)						{ m_dMaximum = dMaximum; }
	void				setMinimum(double dMinimum)						{ m_dMinimum = dMinimum; }
	void				setRange(double dMinimum, double dMaximum)		{ m_dMaximum = dMaximum; m_dMinimum = dMinimum; }
	void				setDecimals(int nPrec)							{ m_nDecimals = nPrec; }
	void				setSingleStep(double dSingleStep)				{ m_dSingleStep = dSingleStep; }

protected slots:

	void				onValueChanged(double dValue);							// value has changed

private:

	double				m_dMaximum;												// maximum value allowed
	double				m_dMinimum;												// minimum value allowed
	int					m_nDecimals;											// precision of the value in decimals
	double				m_dSingleStep;											// holds the step value
};

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexBoolInputItem
//
// Description	:	class used to manage boolean with an input item
//
///////////////////////////////////////////////////////////////////////////////////
class GexBoolInputItem : public GexAbstractInputItem
{
	Q_OBJECT

public:

	GexBoolInputItem(bool bDefaultValue, const QString& strLabel = QString(), const QString& strTooltip = QString());
	virtual ~GexBoolInputItem();

	QWidget *			createInputItem(QWidget * pParent);						// create the widget which manages this item

protected slots:

	void				onValueChanged(int nState);								// value has changed
};

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexStringInputItem
//
// Description	:	class used to manage string with an input item
//
///////////////////////////////////////////////////////////////////////////////////
class GexStringInputItem : public GexAbstractInputItem
{
	Q_OBJECT

public:

	GexStringInputItem(const QString& strValue, const QString& strLabel = QString(), const QString& strTooltip = QString());
	virtual ~GexStringInputItem();

	QWidget *			createInputItem(QWidget * pParent);						// create the widget which manages this item

protected slots:

	void				onValueChanged();                                       // value has changed

private:

    QTextEdit *         m_pTextEdit;
};

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexInputDialog
//
// Description	:	Dialog which allows to input some data from various type
//
///////////////////////////////////////////////////////////////////////////////////
class GexInputDialog : public QDialog
{
	Q_OBJECT

public:

	GexInputDialog(const QString& strTitle, const QString& strLabel, QWidget * pParent = 0, Qt::WindowFlags wFlags = 0);
	~GexInputDialog();

	void				addInputItem(GexAbstractInputItem * pInputItem);		// add an input item in the input dialog box

private:

	QString				m_strTitle;												// Windows title
	QString				m_strLabel;												// First label shown in the dialog box
	QVBoxLayout	*		m_pVerticalLayout;										// Vertical layout used to manage all input item

};
#endif // _GEX_INPUT_DIALOG_H_
