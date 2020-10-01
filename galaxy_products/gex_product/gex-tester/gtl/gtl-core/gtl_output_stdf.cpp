
#include <gstdl_systeminfo.h> // just to test if compilation ok on all platforms
#include "gtl_constants.h"
//#include <hashtable.h>
// #include <hash_map> // deprecated
// #include <unordered_map> // needs STL ISO 2011 : -std=c++11

extern GTL_ProdInfo	gtl_stProdInfo;	/* Tester Production information */

//static CGStdfFile sStdfFile;

/*
// deprecated, non STL standard
struct eqstr
{
  bool operator()(const char* s1, const char* s2) const
  {
    return strcmp(s1, s2) == 0;
  }
};
std::hash_map<const char*, void*, hash<const char*>, eqstr> sTestNameToTestItemHash;
*/

extern "C"
{
    int gtl_InitSTDFOutput()
    {
        /*
        bool lR=sStdfFile.Open("gtl.stdf", 2);
        if (!lR)
            return -1;

        CGStdf4_FAR lFAR;
        lFAR.m_bSTDF_VER=4;
        lFAR.Write(sStdfFile, 1);

        CGStdf4_MIR lMIR;
        sprintf(lMIR.m_szLOT_ID, gtl_stProdInfo.szLotID);
        sprintf(lMIR.m_szPART_TYP, gtl_stProdInfo.szProductID);
        lR=lMIR.Write(sStdfFile, 1); // iNumberOfFields ?

        sStdfFile.Close();
        */

        return 0;
    }

}
