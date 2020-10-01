// ----------------------------------------------------------------------------------------------------------
// Copyright Galaxy
// This computer program is protected by copyright law
// and international treaties. Unauthorized reproduction or 
// distribution of this program, or any portion of it,may 
// result in severe civil and criminal penalties, and will be 
// prosecuted to the maximum extent possible under the low.
// ----------------------------------------------------------------------------------------------------------

#ifndef _GEXDB_PLUGIN_MEDTRONIC_OPTIONS_HEADER_
#define _GEXDB_PLUGIN_MEDTRONIC_OPTIONS_HEADER_

// Standard includes

// Qt includes
#include <qstring.h>
#include <qstringlist.h>

// Galaxy modules includes

// Local includes

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GexDbPlugin_Medtronic_Options
// Holds options specific to this plug-in
/////////////////////////////////////////////////////////////////////////////////////////
class GexDbPlugin_Medtronic_Options
{
// CONSTRUCTOR/DESTRUCTOR
public:
	GexDbPlugin_Medtronic_Options();

	enum TestMergeOption
	{
		// For a test nb having multiple results per run:
		eKeepFirstResult,	// Keep only the first result
		eKeepLastResult,	// Keep only the last result
		eKeepAllResults		// Keep all results (create as many tests as results)
	};

// PUBLIC METHODS
public:
	void	Init(const QString & strOptionsString);

// PROTECTED METHODS
protected:
	void	Reset();

// PUBLIC DATA
public:
	TestMergeOption	m_eTestMergeOption;	// Option on how to handle tests with multiple results per run
};

#endif // _GEXDB_PLUGIN_MEDTRONIC_OPTIONS_HEADER_
