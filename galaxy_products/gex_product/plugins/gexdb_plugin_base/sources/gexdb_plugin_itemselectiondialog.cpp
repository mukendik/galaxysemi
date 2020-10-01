#include "gexdb_plugin_itemselectiondialog.h"

GexDbPlugin_ItemSelectionDialog::GexDbPlugin_ItemSelectionDialog( const QString & strDescription, const QStringList & strlTables, CGexSkin * pGexSkin, QWidget* parent, bool modal, Qt::WindowFlags fl )
    : QDialog( parent, fl )
{
    setupUi(this);
    setModal(modal);

    // Set Examinator skin
    pGexSkin->applyPalette(this);

    QObject::connect(buttonCancel,		SIGNAL(clicked()),						this, SLOT(reject()));
    QObject::connect(buttonOk,			SIGNAL(clicked()),						this, SLOT(accept()));
    QObject::connect(m_listWidget,		SIGNAL(doubleClicked(QModelIndex*)),	this, SLOT(accept()));

    // Init items
    labelDescription->setText(strDescription);
    //listBoxItemName->insertStringList(strlTables);
    m_listWidget->insertItems(0, strlTables);

    // Sort list view
    //listBoxItemName->sort();
    m_listWidget->sortItems();
}

/*
 *  Destroys the object and frees any allocated resources
 */
GexDbPlugin_ItemSelectionDialog::~GexDbPlugin_ItemSelectionDialog()
{
}
