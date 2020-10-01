#ifndef NNR_DEFINITION_DIALOG
#define NNR_DEFINITION_DIALOG

#include "ui_nnr_definition_dialog.h"

class CNNR_Rule;
class COptionsPat;

class NnrDefinitionDialog : public QDialog, public Ui::NnrDefinitionDialogBase
{
    Q_OBJECT

public:
    NnrDefinitionDialog(QWidget* parent, bool modal, Qt::WindowFlags fl);

    /// \brief Init UI with default values
    void            InitGui(const COptionsPat &patOptions);
    /// \brief Load rule into UI
    void            LoadGUI(CNNR_Rule &rule);
    /// \brief Load rule into UI
    void            ReadGUI(CNNR_Rule &rule);

public slots:
    void            onOK(void);

protected slots:
    void            OnRuleNameChanged();

private:

    QStringList     mRules;         ///< holds existing rules name
};


#endif // NNR_DEFINITION_DIALOG

