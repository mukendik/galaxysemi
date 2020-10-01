#ifndef MASK_DEFINITION_DIALOG
#define MASK_DEFINITION_DIALOG

#include "ui_mask_definition_dialog.h"

class CMask_Rule;

class MaskDefinitionDialog : public QDialog, public Ui::MaskDefinitionDialogBase
{
    Q_OBJECT

public:
    MaskDefinitionDialog(bool bCreate, QWidget* parent, bool modal, Qt::WindowFlags fl);
    void	fillGUI(CMask_Rule &cRule);
    void	readGUI(CMask_Rule &cRule);

public slots:
    void	changeWorkingArea(int);
};


#endif // MASK_DEFINITION_DIALOG

