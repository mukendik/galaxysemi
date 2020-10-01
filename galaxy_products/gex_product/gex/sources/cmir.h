#ifndef CMIR_H
#define CMIR_H

#include <QList>
#include "stdf.h"
#include "gex_constants.h" // for MIR_STRING_SIZE,...

class	CMir
{
public:
	CMir();


	char	szLot[MIR_STRING_SIZE];
	char	szPartType[MIR_STRING_SIZE];
	char	szNodeName[MIR_STRING_SIZE];
	char	szTesterType[MIR_STRING_SIZE];
	char	szJobName[MIR_STRING_SIZE];
	char	szJobRev[MIR_STRING_SIZE];
	char	szSubLot[MIR_STRING_SIZE];
	char	szOperator[MIR_STRING_SIZE];
	char	szExecType[MIR_STRING_SIZE];
	char	szExecVer[MIR_STRING_SIZE];
	char	szTestCode[MIR_STRING_SIZE];
	char	szTestTemperature[MIR_STRING_SIZE];
	char	szUserText[MIR_STRING_SIZE];
	char	szAuxFile[MIR_STRING_SIZE];
	char	szPkgType[MIR_STRING_SIZE];
	char	szFamilyID[MIR_STRING_SIZE];
	char	szDateCode[MIR_STRING_SIZE];
	char	szFacilityID[MIR_STRING_SIZE];
	char	szFloorID[MIR_STRING_SIZE];
	char	szProcID[MIR_STRING_SIZE];
	char	szperFrq[MIR_STRING_SIZE];
	char	szSpecName[MIR_STRING_SIZE];
	char	szSpecVersion[MIR_STRING_SIZE];
	char	szFlowID[MIR_STRING_SIZE];
	char	szSetupID[MIR_STRING_SIZE];
	char	szDesignRev[MIR_STRING_SIZE];
	char	szEngID[MIR_STRING_SIZE];
	char	szROM_Code[MIR_STRING_SIZE];
	char	szSerialNumber[MIR_STRING_SIZE];
	char	szSuprName[MIR_STRING_SIZE];
	char	szHandlerProberID[MIR_STRING_SIZE];
	char	szProbeCardID[MIR_STRING_SIZE];


    time_t	lSetupT;
    time_t	lStartT;
    time_t	lEndT;
    long	lFirstPartResult;	// First part# collected for Advanced charting
    int		iBurnTime;
    bool bMrrRecord;		// Set to true if file includes a MRR (meaning it is complete).
	// PCR data (get them from WRR if needed)
    /*bool bMergedPcrDone;	// Set once PCRs of merged sites is read, and filter=all sites
	bool bFirstPcrRecord;	// Cleared after first PCR seen (and PCR structures reset...so to overwrite WRR temporary values)

    long lPartCount;
	int	lRetestCount;
	int	lAbortCount;
	int	lGoodCount;
    int	lFuncCount;*/

    BYTE	bStation;
    BYTE	bModeCode;
    BYTE	bRtstCode;
    BYTE	bProdCode;

    BYTE	cModeCode;

	// If multiple Wafers in a single file, get the list of WaferIDs so we can parse the correct data.
    QList<QString>	m_lstWaferID;// List of WaferIDs in STDF file
};

class	CPcr
{
public:
    CPcr();

    // PCR data (get them from WRR if needed)
    bool bMergedPcrDone;	// Set once PCRs of merged sites is read, and filter=all sites
    bool bFirstPcrRecord;	// Cleared after first PCR seen (and PCR structures reset...so to overwrite WRR temporary values)
   // bool bMrrRecord;		// Set to true if file includes a MRR (meaning it is complete).
    long lPartCount;
    int	lRetestCount;
    int	lAbortCount;
    int	lGoodCount;
    int	lFuncCount;
};
#endif // CMIR_H
