// Routines to implement dynamic PAT on TMT ASL testers with Galaxy
// Written by Patrick Hayes 
// $Rev: 1403 $ $Date: 2011-04-21 14:48:34 -0700 (Thu, 21 Apr 2011) $
// $LastChangedBy: phayes $ 
// $URL: http://svn-wts.dalsemi.com/repos/mtg/TMT_DPAT/Trunk/dpat.cpp $

#include "galaxy/include/gtl_core.h"
void galaxy_bin(test_function& func);
void galaxy_data(test_function& func, unsigned int uiSiteNb, long lTestNb, char *szTestName, double lfResult);
