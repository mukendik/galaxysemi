#ifndef CHARAC_BOX_WHISKER_TEMPLATE_DIALOG_H
#define CHARAC_BOX_WHISKER_TEMPLATE_DIALOG_H

#include "charac_box_whisker_template.h"

#include <QDialog>

class QTreeWidgetItem;

namespace Ui {
class CharacBoxWhiskerTemplateDialog;
}

namespace GS
{
namespace Gex
{

class CharacBoxWhiskerTemplateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CharacBoxWhiskerTemplateDialog(QWidget *parent = 0);
    ~CharacBoxWhiskerTemplateDialog();

    const CharacBoxWhiskerTemplate&  GetTemplate() const;
    void        SetTemplate(const CharacBoxWhiskerTemplate& lTemplate);
    void        SetConditionsLevel(const QStringList &conditionsLevel);
    void        SetConditionsValues(const QList<QMap<QString, QString> >& conditionsValues);

    void        FillGui();

public slots:

    void        accept();

protected:

    void        FillTemplateFromGUI();
    void        FillDefaultTemplate();

protected slots:

    void        OnAggregateDoubleClicked(QTreeWidgetItem *,int);

private:

    Ui::CharacBoxWhiskerTemplateDialog *    mUi;
    QStringList                             mConditionsLevel;
    QList<QMap<QString, QString> >          mConditionsValues;
    CharacBoxWhiskerTemplate                mTemplate;
};

}   // namespace Gex
}   // namespace GS

#endif // CHARAC_BOX_WHISKER_TEMPLATE_DIALOG_H
