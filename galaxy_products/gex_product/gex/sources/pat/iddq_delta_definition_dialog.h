#ifndef IDDQ_DELTA_DEFINITION_DIALOG
#define IDDQ_DELTA_DEFINITION_DIALOG

#include "ui_iddq_definition_dialog.h"

class CIDDQ_Delta_Rule;

class IddqDeltaDefinitionDialog : public QDialog, public Ui::IddqDeltaDefinitionDialogBase
{
public:
    IddqDeltaDefinitionDialog(QWidget* parent, bool modal, Qt::WindowFlags fl);
    void	fillGUI(CIDDQ_Delta_Rule &rule);
    void	readGUI(CIDDQ_Delta_Rule &rule);

};

#endif // IDDQ_DELTA_DEFINITION_DIALOG

