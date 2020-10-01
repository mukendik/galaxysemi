///////////////////////////////////////////////////////////
// Database Transactions class
///////////////////////////////////////////////////////////

#ifndef GEX_DB_TRANSACTIONS_H
#define GEX_DB_TRANSACTIONS_H

#include <stdio.h>
#if defined unix || __MACH__
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#else
#include <direct.h>
#endif

#include <qcombobox.h>

#include "stdf.h"
#include "db_external_database.h"
#include "scripting_io.h"
#include "gex_constants.h"
#include "import_all.h"

// Forward declarations
class GexRemoteDatabase;
class GexDbPlugin_Filter;							// db_external_database.h

namespace GS
{
namespace DbPluginBase
{
class DatakeysEngine;
}
}


#endif
