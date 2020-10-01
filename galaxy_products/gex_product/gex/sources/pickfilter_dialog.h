#ifndef GEX_PICKFILTER_H
#define GEX_PICKFILTER_H

#include "ui_pickfilter_dialog.h"
#include "browser_dialog.h"
#include "db_transactions.h"
#include "db_gexdatabasequery.h"

class GexDatabaseFilter;

/////////////////////////////////////////////////////////////////////////////
class PickFilterDialog : public QDialog, public Ui::PickFilterDialogBase
{
    Q_OBJECT

public:
    PickFilterDialog( QWidget* parent=0, bool modal=false, Qt::WindowFlags f=0 );
    void	fillList(QStringList & cList, bool bAddStar=true);
    void	fillList(const GexDatabaseFilter& dbFilter, bool bAddStar=true);
    void	fillList(const GexDatabaseQuery& dbQuery, QString & strFilterName);
    bool	isEmpty(void);
    QString filterList(void);
    void	setMultipleSelection(bool bAllowMultiSelections);
};

#endif
