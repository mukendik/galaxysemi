///////////////////////////////////////////////////////////
// All classes used for 'Analyze ONE file'
///////////////////////////////////////////////////////////

#ifndef SQL_QUERY_WIZARD_H
#define SQL_QUERY_WIZARD_H
#include "db_sql_query_basedialog.h"

#include <qdatetime.h>

class GexSqlQueryWizardPage1 : public sql_query_basedialog
{
public:
	GexSqlQueryWizardPage1( QWidget* parent = 0, const char* name = 0, bool modal = false, WindowFlags fl = 0 );
	~GexSqlQueryWizardPage1();

	void	ShowPage(void);

private:
	void UpdateSkin(int lProductID);


public slots:
	void	reject(void);		// Called if user hits the ESCAPE key.

};
#endif
