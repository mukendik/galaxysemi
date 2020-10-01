#ifndef GDBN_DEFINITION_DIALOG
#define GDBN_DEFINITION_DIALOG

#include "ui_gdbn_definition_dialog.h"

class CGDBN_Rule;
class COptionsPat;

class GdbnDefinitionDialog : public QDialog, public Ui::GdbnDefinitionDialogBase
{
    Q_OBJECT

public:
    GdbnDefinitionDialog(QWidget* parent, bool modal, Qt::WindowFlags fl);
    void	fillGUI(CGDBN_Rule &rule, const COptionsPat *patOptions);
    void	readGUI(CGDBN_Rule &rule);
public slots:
    void	OnBadClusterSize(void);
    void	OnNeighbourhoodAlgorithm(void);
    void	OnWeightingEdgeDie(void);
};

#endif // GDBN_DEFINITION_DIALOG

