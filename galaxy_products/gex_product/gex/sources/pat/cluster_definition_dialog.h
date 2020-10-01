#ifndef CLUSTER_DEFINITION_DIALOG
#define CLUSTER_DEFINITION_DIALOG

#include "ui_cluster_definition_dialog.h"

class CClusterPotatoRule;
class COptionsPat;

class ClusterDefinitionDialog : public QDialog, public Ui::ClusterDefinitionDialogBase
{
    Q_OBJECT

public:
    ClusterDefinitionDialog(QWidget* parent, bool modal, Qt::WindowFlags fl);
    void	fillGUI(CClusterPotatoRule &cRule, const COptionsPat *patOptions);
    void	readGUI(CClusterPotatoRule &cRule);

public slots:
    void	changeOutlineMatrixSize(int);
    void	OnWeightingEdgeDie(void);
};


#endif // CLUSTER_DEFINITION_DIALOG

