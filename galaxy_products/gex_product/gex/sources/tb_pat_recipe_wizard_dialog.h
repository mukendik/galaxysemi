#ifndef TB_PAT_RECIPE_WIZARD_H
#define TB_PAT_RECIPE_WIZARD_H

#include "ui_tb_pat_recipe_wizard_dialog.h"

#include <QTreeWidget>

class COptionsPat;

struct TestInfo {
        int         mNumber;
        int         mPinIndex;
        QString     mName;
};

/////////////////////////////////////////////////////////////////////////////
class PatRecipeWizardDialog : public QDialog, public Ui::PatRecipeWizardDialogBase
{
    Q_OBJECT

public:
    PatRecipeWizardDialog(QWidget* parent = 0, bool modal = false, Qt::WindowFlags f = 0 );
    bool setFile(QString strFile, const COptionsPat &patOptions, QString strFilterType="all", QString strFilterList="");
    bool hasParametricTests();
    QList<TestInfo> getDisabledTests(void);
    QList<TestInfo> getEnabledTests(void);

public slots:
    void OnDisableTests(void);
    void OnEnableTests(void);
    void contextMenuTestsToEnable(const QPoint &);
    void contextMenuTestsToDisable(const QPoint & pos);
    void OnButtonBack();

};

class PatRecipeListViewItem : public QTreeWidgetItem
{
public:

    PatRecipeListViewItem(QTreeWidget * pListView, const QString& testID);
    virtual ~PatRecipeListViewItem();

    const QString&  GetTestID() const;
    int             compare (QTreeWidgetItem * pOtherItem, int nCol, bool bAscending) const;

protected:

    QString mTestID;
};

#endif // TB_PAT_RECIPE_WIZARD_H
