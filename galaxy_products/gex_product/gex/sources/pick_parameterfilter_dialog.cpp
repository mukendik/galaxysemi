#include "pick_parameterfilter_dialog.h"
#include "browser_dialog.h"

///////////////////////////////////////////////////////////
// Constructor
PickParameterFilterDialog::PickParameterFilterDialog( QWidget* parent, bool modal, Qt::WindowFlags f ) 
	: QDialog( parent, f )
{
	setupUi(this);
	setModal(modal);
		
	// Apply Examinator palette
	GexMainwindow::applyPalette(this);

	QObject::connect(PushButtonOk,			SIGNAL(clicked()),									this, SLOT(accept()));
    QObject::connect(PushButtonCancel,		SIGNAL(clicked()),									this, SLOT(reject()));
    QObject::connect(treeWidgetParameter,	SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),	this, SLOT(accept()));
    QObject::connect(lineEditNameMask,		SIGNAL(textChanged(QString)),						this, SLOT(OnNameMask(QString)));
    QObject::connect(comboBox,				SIGNAL(activated(QString)),							this, SLOT(OnFilterSelection(QString)));
    QObject::connect(lineEditLimit,			SIGNAL(textChanged(QString)),						this, SLOT(OnLimit(QString)));
}

///////////////////////////////////////////////////////////
// User is selecting a specific filtering type.
// Eg: All parameters, or Failing parameters, or specfic name,...
///////////////////////////////////////////////////////////
void
PickParameterFilterDialog::
OnFilterSelection(const QString& /*strComboSelection*/)
{
}

///////////////////////////////////////////////////////////
// User specifying a filter based on a name string. E.g: Iddq*
///////////////////////////////////////////////////////////
void
PickParameterFilterDialog::
OnNameMask(const QString& /*strStringMask*/)
{
}

///////////////////////////////////////////////////////////
// User specifying a Cp or Cpk limit.
///////////////////////////////////////////////////////////
void
PickParameterFilterDialog::
OnLimit(const QString& /*strStringLimit*/)
{
}

///////////////////////////////////////////////////////////
// Return Filter selection made.
///////////////////////////////////////////////////////////
void
PickParameterFilterDialog::
getFilterSelection(QString& /*strParameterList*/)
{
}
