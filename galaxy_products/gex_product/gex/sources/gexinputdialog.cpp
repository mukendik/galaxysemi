///////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////
#include "gexinputdialog.h"
#include "browser_dialog.h"

///////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////
#include <QLabel>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QVBoxLayout>

///////////////////////////////////////////////////////////////////////////////////
// Class GexAbstractInputItem - class which
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexAbstractInputItem::GexAbstractInputItem(const QVariant& varDefault, const QString& strLabel, const QString& strTooltip)
{
	m_strLabel		= strLabel;
	m_varValue		= varDefault;
	m_strTooltip	= strTooltip;
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexAbstractInputItem::~GexAbstractInputItem()
{

}

///////////////////////////////////////////////////////////////////////////////////
// Class GexDoubleInputItem - class which
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexDoubleInputItem::GexDoubleInputItem(double dDefaultValue, const QString& strLabel /*=QString()*/, const QString& strTooltip /*=QString()*/)
		: GexAbstractInputItem(QVariant(dDefaultValue), strLabel, strTooltip)
{
	m_dMaximum		= 1e99;
	m_dMinimum		= -1e99;
	m_nDecimals		= 10;
	m_dSingleStep	= 1.0;
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexDoubleInputItem::~GexDoubleInputItem()
{
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	QWidget * createInputItem(QWidget * pParent)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
QWidget * GexDoubleInputItem::createInputItem(QWidget * pParent)
{
	QWidget *		pWidget				= new QWidget(pParent);
	QHBoxLayout *	pHorizontalLayout	= new QHBoxLayout(pWidget);

	// Layout has no margin
	pHorizontalLayout->setContentsMargins(0, 0, 0, 0);

	// add label on the left side of the spin box if not empty
	if (label().isEmpty() == false)
	{
		QLabel * pLabel = new QLabel(pWidget);
		pLabel->setText(label());

		QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		pLabel->setSizePolicy(sizePolicy);
		pLabel->setMinimumSize(QSize(100, 0));
		pLabel->setMaximumSize(QSize(100, 16777215));

    	pHorizontalLayout->addWidget(pLabel);
	}

	// add spin box widget
    QDoubleSpinBox * pSpinBox = new QDoubleSpinBox(pWidget);
	pSpinBox->setMaximum(m_dMaximum);
	pSpinBox->setMinimum(m_dMinimum);
	pSpinBox->setDecimals(m_nDecimals);
	pSpinBox->setValue(value().toDouble());
	pSpinBox->setSingleStep(m_dSingleStep);

	// add tooltip if not empty
	if (tooltip().isEmpty() == false)
		pSpinBox->setToolTip(tooltip());

    pHorizontalLayout->addWidget(pSpinBox);

	// connect signal to get the value
	connect(pSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onValueChanged(double)));

	return pWidget;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void onValueChanged(double dValue)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexDoubleInputItem::onValueChanged(double dValue)
{
	// sets the new value
	setValue(QVariant(dValue));
}

///////////////////////////////////////////////////////////////////////////////////
// Class GexBoolInputItem - class which
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexBoolInputItem::GexBoolInputItem(bool bDefaultValue, const QString& strLabel /*=QString()*/, const QString& strTooltip /*=QString()*/)
		: GexAbstractInputItem(QVariant(bDefaultValue), strLabel, strTooltip)
{
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexBoolInputItem::~GexBoolInputItem()
{
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	QWidget * createInputItem(QWidget * pParent)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
QWidget * GexBoolInputItem::createInputItem(QWidget * pParent)
{
	// create check box widget
	QCheckBox * pCheckBox = new QCheckBox(pParent);

	// sets text label and default value
	pCheckBox->setText(label());
	pCheckBox->setChecked(value().toBool());

	// add tooltip if not empty
	if (tooltip().isEmpty() == false)
		pCheckBox->setToolTip(tooltip());

	// connect signal to get the new value
	connect(pCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onValueChanged(int)));

	return pCheckBox;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void onValueChanged(int nValue)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexBoolInputItem::onValueChanged(int nValue)
{
	setValue(QVariant((bool) nValue));
}

///////////////////////////////////////////////////////////////////////////////////
// Class GexStringInputItem - class which
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexStringInputItem::GexStringInputItem(const QString &strValue, const QString &strLabel /*=QString()*/, const QString &strTooltip /*=QString()*/)
    : GexAbstractInputItem(QVariant(strValue), strLabel, strTooltip), m_pTextEdit(NULL)
{
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexStringInputItem::~GexStringInputItem()
{
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	QWidget * createInputItem(QWidget * pParent)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
QWidget * GexStringInputItem::createInputItem(QWidget * pParent)
{
    QWidget *		pWidget				= new QWidget(pParent);
	QHBoxLayout *	pHorizontalLayout	= new QHBoxLayout(pWidget);

	// Layout has no margin
	pHorizontalLayout->setContentsMargins(0, 0, 0, 0);

	// add label on the left side of the spin box if not empty
	if (label().isEmpty() == false)
	{
		QLabel * pLabel = new QLabel(pWidget);
		pLabel->setText(label());

		QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		pLabel->setSizePolicy(sizePolicy);
		pLabel->setMinimumSize(QSize(100, 0));
		pLabel->setMaximumSize(QSize(100, 25));

    	pHorizontalLayout->addWidget(pLabel);
	}

	// create text edit widget
	m_pTextEdit = new QTextEdit(pParent);
    m_pTextEdit->setText(value().toString());
    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_pTextEdit->setSizePolicy(sizePolicy);
    m_pTextEdit->setMinimumSize(QSize(100, 0));
    m_pTextEdit->setMaximumSize(QSize(1000, 25));

    // add tooltip if not empty
	if (tooltip().isEmpty() == false)
		m_pTextEdit->setToolTip(tooltip());

    pHorizontalLayout->addWidget(m_pTextEdit);

	// connect signal to get the value
    connect(m_pTextEdit, SIGNAL(textChanged()), this, SLOT(onValueChanged()));

	return pWidget;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void onValueChanged()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexStringInputItem::onValueChanged()
{
    setValue(QVariant(m_pTextEdit->toPlainText()));
}

///////////////////////////////////////////////////////////////////////////////////
// Class GexInputDialog - class which
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexInputDialog::GexInputDialog(const QString& strTitle, const QString& strLabel, QWidget * pParent /* = 0 */, Qt::WindowFlags wFlags /* = 0 */)
	: QDialog(pParent, wFlags)
{
	// Apply Examinator palette
	GexMainwindow::applyPalette(this);

	setWindowTitle(strTitle);

	resize(400, 100);

	QGridLayout * pGridLayout = new QGridLayout(this);

	m_pVerticalLayout	= new QVBoxLayout();

	// Label is not empty, add a label widget
	if (strLabel.isEmpty() == false)
		m_pVerticalLayout->addWidget(new QLabel(strLabel, this));

	// add the vertical layout which manages input items
	pGridLayout->addLayout(m_pVerticalLayout, 0, 0, 1, 1);

	// add a vertical spacer
	pGridLayout->addItem(new QSpacerItem(20, 100, QSizePolicy::Minimum, QSizePolicy::Expanding), 1, 0, 1, 1);

	// add ok and cancel buttons
	QDialogButtonBox * pButtonBox = new QDialogButtonBox(this);
    pButtonBox->setOrientation(Qt::Horizontal);
    pButtonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);

	pGridLayout->addWidget(pButtonBox, 2, 0, 1, 1);

	// connect signal from buttons
	QObject::connect(pButtonBox, SIGNAL(accepted()), this, SLOT(accept()));
    QObject::connect(pButtonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexInputDialog::~GexInputDialog()
{
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void addInputItem(GexAbstractInputItem * pInputItem)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexInputDialog::addInputItem(GexAbstractInputItem * pInputItem)
{
	if (pInputItem)
        m_pVerticalLayout->addWidget(pInputItem->createInputItem(this));
}


