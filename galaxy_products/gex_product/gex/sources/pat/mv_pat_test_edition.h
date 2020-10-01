#include <QDialog>
#include "pat_mv_rule.h"
#include "ui_rule_test_edition.h"
#include <QHash>
#include <QString>

#ifndef MV_PAT_TEST_EDITION_H
#define MV_PAT_TEST_EDITION_H
class CPatDefinition;

namespace GS
{
namespace Gex
{

class MVPATTestEdition : public QDialog , public Ui::MVPATRule
{
    Q_OBJECT
public:
    MVPATTestEdition(QWidget *parent, PATMultiVariateRule *rule, QHash<QString, CPatDefinition *> &univarTests);
    virtual ~MVPATTestEdition();
public slots:
    void IncludedTest();
    void ExcludedTest();
    void OnOk();
private:
    PATMultiVariateRule *mRule;
    PATMultiVariateRule mEditedRule;
    QHash<QString, CPatDefinition *> mUVTests;
};

}
}

#endif // MV_PAT_TEST_EDITION_H
