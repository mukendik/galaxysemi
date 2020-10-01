#ifndef GTL_OUTPUT_H
#define GTL_OUTPUT_H

#include "gtl_constants.h"
#include "gtl_testlist.h"
//#include "gtl_message.h"

 extern int gtl_InitOutput();

 extern int gtl_UpdateOutput(SiteItem *lSite, int lOriginalHBin, int lOriginalSBin, int lHBin,int lSBin,
                            const char* lPartID, char PatBinType, char lOrgBinType);

 extern int gtl_OutputNewEvent(char* lEventType, char* lEventSubtype, char* lEventMessage);
 extern int gtl_OutputNewMessage(GTL_Message*);

 extern int gtl_SQLiteCloseSplitlot();

 extern int gtl_OutputCloseSplitlot();
 extern int gtl_OutputNewSplitlot();

 extern int gtl_CloseOutput();

#endif // GTL_OUTPUT_H
