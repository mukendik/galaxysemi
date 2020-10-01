///////////////////////////////////////////////////////////
// All classes used for ToolBox 'Merge Test & Retest'
///////////////////////////////////////////////////////////

#ifndef TB_PAT_OUTLIER_REMOVAL_H
#define TB_PAT_OUTLIER_REMOVAL_H
#include "classes.h"
#include "stdf.h"
#include "patman_lib.h"


/////////////////////////////////////////////////////////////////////////////
// Class used for sorting PAtResults (pareto based on failures)
/////////////////////////////////////////////////////////////////////////////
typedef QList<CPatDefinition*>			tdPatResultList;
typedef QListIterator<CPatDefinition*>	tdPatResultListIterator;

#define	GEX_PAT_RESULT_TNUM		0			// Sort by test number
#define	GEX_PAT_RESULT_SSCORE	1			// Sort by severity mode


#endif
