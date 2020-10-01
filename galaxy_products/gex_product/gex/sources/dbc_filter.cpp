#include <QSortFilterProxyModel>

#include <gqtl_log.h>
#include "dbc_dataset_editor.h"
#include "dbc_filter.h"
#include "ui_dbc_filter.h"
#include "dbc_transaction.h"


int DbcFilter::m_iLastIndex;

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
DbcFilter::DbcFilter(ParamSqlTableModel *pTableModel, int iIndex, QWidget *parent) :
    QFrame(parent),
    m_ui(new Ui::DbcFilter)
{
    m_ui->setupUi(this);
    clearErrors();
    setTableModel(pTableModel);
    setIndex(iIndex);
    setParamNumber(0);
    setParamName("Parameter_name");
    setOperator("=");
    setParamValue("");
    m_ui->lineEditParamValue->setFocus();
    init();
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
DbcFilter::~DbcFilter()
{
    delete m_ui;
}

///////////////////////////////////////////////////////////
// Initialize
///////////////////////////////////////////////////////////
void DbcFilter::init()
{
    initComboParamNumber();
    initComboOperator();
    connect(m_ui->pushButtonDelete, SIGNAL(clicked()), this, SLOT(onAutoDelete()));
    connect(m_ui->pushButtonDelete, SIGNAL(clicked()), this, SIGNAL(sChanged()));
    connect(m_ui->pushButtonAdd, SIGNAL(clicked()), this, SIGNAL(sAdd()));
    connect(m_ui->pushButtonAdd, SIGNAL(clicked()), this, SIGNAL(sChanged()));
    connect(m_ui->lineEditParamValue, SIGNAL(textEdited(QString)), this, SIGNAL(sChanged()));
    connect(m_ui->comboBoxOperator, SIGNAL(currentIndexChanged(int)), this, SIGNAL(sChanged()));
    connect(m_ui->comboBoxParamNumber, SIGNAL(currentIndexChanged(int)), this, SIGNAL(sChanged()));
    connect(this, SIGNAL(sChanged()), this, SLOT(onChanged()));
}

///////////////////////////////////////////////////////////
// Init Index
///////////////////////////////////////////////////////////
void DbcFilter::initIndex()
{
    DbcFilter::m_iLastIndex = 0;
}

///////////////////////////////////////////////////////////
// Init combo box parameter
///////////////////////////////////////////////////////////
void DbcFilter::initComboOperator()
{
    // Operator list of possible values
    m_lstOperator.clear();
    m_lstOperator << "=";
    m_lstOperator << "!=";
    m_lstOperator << ">";
    m_lstOperator << "<";
    m_lstOperator << ">=";
    m_lstOperator << "<=";
    m_lstOperator << "LIKE";
    // Clear ui
    m_ui->comboBoxOperator->clear();
    m_ui->comboBoxOperator->addItems(m_lstOperator);
}

///////////////////////////////////////////////////////////
// Update label index
///////////////////////////////////////////////////////////
void DbcFilter::updateLabelIndex()
{
    if (m_iIndex == 0)
    {
        DbcFilter::m_iLastIndex++;
        m_iIndex = DbcFilter::m_iLastIndex;
    }
    m_ui->labelIndex->setText(QString::number(m_iIndex));
}

///////////////////////////////////////////////////////////
// Init combobox parameter
///////////////////////////////////////////////////////////
void DbcFilter::initComboParamNumber()
{
    m_pProxyModelParamNumber = new QSortFilterProxyModel(this);
    m_pProxyModelParamNumber->setSourceModel(m_pTableModel);
    m_ui->comboBoxParamNumber->setModel(m_pProxyModelParamNumber);
    m_ui->comboBoxParamNumber->setModelColumn(m_pTableModel->fieldIndex(QString(DBC_TEST_F_NAME)));
}

///////////////////////////////////////////////////////////
// Update Combobox parameter
///////////////////////////////////////////////////////////
void DbcFilter::updateComboParamNumber()
{
    if (m_ui->comboBoxParamNumber->findText(QString(m_strParamName + "-" + QString::number(m_iParamNumber))) >= 0)
        m_ui->comboBoxParamNumber->setCurrentIndex(m_ui->comboBoxParamNumber->findText(QString(m_strParamName + "-" + QString::number(m_iParamNumber))));
    else
        m_ui->comboBoxParamNumber->setCurrentIndex(0);
}

///////////////////////////////////////////////////////////
// Update combo box operator
///////////////////////////////////////////////////////////
void DbcFilter::updateComboOperator()
{
    int iSelectedOperator = m_lstOperator.indexOf(m_strOperator);
    m_ui->comboBoxOperator->setCurrentIndex(iSelectedOperator);
}

///////////////////////////////////////////////////////////
// Update param value
///////////////////////////////////////////////////////////
void DbcFilter::updateEditParamValue()
{
    m_ui->lineEditParamValue->setText(m_strParamValue);
}

///////////////////////////////////////////////////////////
// Called when changed detected on ui
///////////////////////////////////////////////////////////
void DbcFilter::onChanged()
{
    retrieveUIValue();
}

///////////////////////////////////////////////////////////
// Set members with ui values
///////////////////////////////////////////////////////////
void DbcFilter::retrieveUIValue()
{
    m_iParamNumber = m_pTableModel->index(m_ui->comboBoxParamNumber->currentIndex(), m_pTableModel->fieldIndex(QString(DBC_TEST_F_NUMBER))).data(Qt::DisplayRole).toInt();
    m_strParamName = m_pTableModel->index(m_ui->comboBoxParamNumber->currentIndex(), m_pTableModel->fieldIndex(QString(DBC_TEST_F_NAME))).data(Qt::DisplayRole).toString();
    m_strParamName.remove(- (QString::number(m_iParamNumber).size() + 1),QString::number(m_iParamNumber).size() + 1);
    m_strOperator = m_ui->comboBoxOperator->currentText();
    m_strParamValue = m_ui->lineEditParamValue->text();
}

///////////////////////////////////////////////////////////
// Escapes XML unavailable characters
// (only used for text without any markup)
///////////////////////////////////////////////////////////
QString DbcFilter::escapesXmlUnavailableChars(const QString& strText)
{
    QString strXmlValue = strText;
    // Replace "&" by "&amp;"
    strXmlValue.replace("&", "&amp;");
    // Replace "<" by "&lt;"
    strXmlValue.replace("<", "&lt;");
    // Replace ">" by "&gt;"
    strXmlValue.replace(">", "&gt;");
    // Replace "'" by "&apos;"
    strXmlValue.replace("'", "&apos;");
    // Replace """ by "&quot;"
    strXmlValue.replace("\"", "&quot;");

    return strXmlValue;
}

///////////////////////////////////////////////////////////
// Restore XML unavailable characters
///////////////////////////////////////////////////////////
QString DbcFilter::restoresXmlUnavailableChars(const QString& strText)
{
    QString strXmlValue = strText;
    // Replace "&amp;" by "&"
    strXmlValue.replace("&amp;", "&");
    // Replace "&lt;" by "<"
    strXmlValue.replace("&lt;", "<");
    // Replace "&gt;" by ">"
    strXmlValue.replace("&gt;", ">");
    // Replace "&apos;" by "'"
    strXmlValue.replace("&apos;", "'");
    // Replace "&quot;" by """
    strXmlValue.replace("&quot;", "\"");

    return strXmlValue;
}

///////////////////////////////////////////////////////////
// Set Table model
///////////////////////////////////////////////////////////
void DbcFilter::setTableModel(ParamSqlTableModel *pTableModel)
{
    if (pTableModel)
        m_pTableModel = pTableModel;
    else
    {
        m_lstError.append("Link to table not found");
        GSLOG(SYSLOG_SEV_ERROR, "no table model.");
    }
}

///////////////////////////////////////////////////////////
// Show/hide add button
///////////////////////////////////////////////////////////
void DbcFilter::setAddButtonVisible(bool bIsVisible)
{
    m_ui->pushButtonAdd->setVisible(bIsVisible);
}

///////////////////////////////////////////////////////////
// Set index
///////////////////////////////////////////////////////////
void DbcFilter::setIndex(int iIndex)
{
    m_iIndex = iIndex;
    if (m_iIndex > DbcFilter::m_iLastIndex)
        DbcFilter::m_iLastIndex = m_iIndex;
    updateLabelIndex();
}

///////////////////////////////////////////////////////////
// Set param number
///////////////////////////////////////////////////////////
void DbcFilter::setParamNumber(int iParamNumber)
{
    m_iParamNumber = iParamNumber;
    updateComboParamNumber();
}

///////////////////////////////////////////////////////////
// Set param name
///////////////////////////////////////////////////////////
void DbcFilter::setParamName(QString strParamName)
{
    m_strParamName = strParamName;
    updateComboParamNumber();
}

///////////////////////////////////////////////////////////
// Set operator
///////////////////////////////////////////////////////////
void DbcFilter::setOperator(QString strFilterOperator)
{
    m_strOperator = strFilterOperator;
    updateComboOperator();
}

///////////////////////////////////////////////////////////
// Set param value
///////////////////////////////////////////////////////////
void DbcFilter::setParamValue(QString strParamValue)
{
    m_strParamValue = strParamValue;
    updateEditParamValue();
}

///////////////////////////////////////////////////////////
// Emit a signal to delete object
///////////////////////////////////////////////////////////
void DbcFilter::onAutoDelete()
{
    emit sDelete(m_iIndex);
}
