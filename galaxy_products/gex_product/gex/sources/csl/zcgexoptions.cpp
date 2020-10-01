#include <QString>
#include "zcsl.hpp"
#include "../scripting_io.h"
#include "../report_options.h"

extern float m_fCslVersion;			// Csl version
extern ZString gexOptions_v1(ZCsl* csl);
extern ZString gexOptions_v2(ZCsl* csl);
extern ZString gexOptions_v2_1(ZCsl* csl);
extern CReportOptions	ReportOptions;	// Holds options (report_build.h)

//extern void ConvertFromScriptString(QString &strFile);		Not used

////////////////////////////////////////////////////////////////////////////////
// Function gexOption: Let's user control all GEX report options
// Call : gexOption(section, field, value);
// For csl file version 1.0
////////////////////////////////////////////////////////////////////////////////
//static
ZString gexOptions(ZCsl* csl)
{
    if (m_fCslVersion<=1.0f)
		return gexOptions_v1(csl);			// .CSL Legacy mode: allow reading OLD .CSL syntax
    else if(m_fCslVersion == 2.0f)
		return gexOptions_v2(csl);			// .CSL syntax version 2
	else
    {
        // currently version 2.1
        ZString s=gexOptions_v2_1(csl);
        //if (s==0)
          //  GSLOG(SYSLOG_SEV_ERROR, "gexOptions failed ");
        return s;
    }
}
