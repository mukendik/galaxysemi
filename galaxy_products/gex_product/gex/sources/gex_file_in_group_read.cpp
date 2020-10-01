#include "gex_report.h"
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include "gexabstractdatalog.h"
#include "cpart_info.h"
#include "import_constants.h"
#include "product_info.h"
#include "report_options.h"
#include "gex_oem_constants.h"
#include "browser_dialog.h"
#include "message.h"
#include "plugin_base.h"
#include "stdfrecords_v4.h"
#include "stdf_head_and_site_number_decipher.h"

extern GexMainwindow *	pGexMainWindow;
extern CGexReport*		gexReport;			// Handle to report class

/////////////////////////////////////////////////////////////////////////////
// Read string from STDF MIR record, save in into relevant MIR buffer
/////////////////////////////////////////////////////////////////////////////
int	CGexFileInGroup::ReadStringToField(char *szField)
{
  char	szString[257];	// A STDF string is 256 bytes long max!

  *szField=0;

  if(StdfFile.ReadString(szString)  != GS::StdLib::Stdf::NoError)
    return -1;
  // Security: ensures we do not overflow destination buffer !
  szString[MIR_STRING_SIZE-1] = 0;
  strcpy(szField,szString);
  return 1;
}

/////////////////////////////////////////////////////////////////////////////
// Read STDF ATR record
/////////////////////////////////////////////////////////////////////////////
void CGexFileInGroup::ReadATR(void)
{
    if(lPass == 2)
        return;

    // ATR: STDF V4 only
    char	szString[257];	// A STDF string is 256 bytes long max!

    StdfFile.ReadDword((long *)&lData);			// MOD_TIM
    if(StdfFile.ReadString(szString)  != GS::StdLib::Stdf::NoError)
        return;	// CMD_LINE

    // ATR may include the tester signature. If this is the case, parse it!
    switch(GS::LPPlugin::ProductInfo::getInstance()->getProductID())
    {
        case GS::LPPlugin::LicenseProvider::eLtxcOEM:	// OEM-Examinator for LTXC
        {
            // Parse ATR string and check if it includes the Sapphire STDF signature.
            QString strString = szString;
            // E.G: Credence Systems Sapphire D Series - Release 1
            if(strString.startsWith("Credence Systems Sapphire D Series - Release") == false)
                break;

            //  Flag to tell 'Reject this file'...cleared below if file is fine!
            iStdfAtrTesterBrand = -2;

            // Clean the Revision string in case it starts with string "Rev" or Revision"
            QString strExecVersion = strString.section(' ',-1);	// Get Exec version info
            strExecVersion = strExecVersion.trimmed();
            if(strExecVersion.isEmpty())
                break;	// Missing OS revision...error

            // Check if this release accepts this version of Credence Sapphire OS...
            // Check that OS version is in format X.Y
            int iMajorRelease;
            int iStatus = sscanf(strExecVersion.toLatin1().constData(),"%d",&iMajorRelease);
            if(iStatus != 1)
                break;	// OS version string not valid

            // Check OS version is more recent than the one accepted by Examinator: user needs to upgrade Examinator!
            if(iMajorRelease > GEX_OEM_SAPPHIRE_OSVERSION)
                break;

            iStdfAtrTesterBrand = GS::LPPlugin::LicenseProvider::eLtxcOEM;
            break;
        }

        default:
            break;
    }
}

/////////////////////////////////////////////////////////////////////////////
// Read STDF MIR record
/////////////////////////////////////////////////////////////////////////////
void CGexFileInGroup::ReadMIR(void)
{
  // MIR

  if(lPass == 2)
  {
    // Reset variables used when reading WIR
    bFirstWafer = true;
    strPreviousWaferID = "";
    uiAutoWaferID = 0;

    return;
  }

  switch(StdfRecordHeader.iStdfVersion)
  {
  default :
    break;

  case GEX_STDFV4:

    long lData;
    StdfFile.ReadDword(&lData);			// Setup_T
    MirData.lSetupT = (time_t)lData;
    StdfFile.ReadDword(&lData);			// Start_T
    MirData.lStartT = (time_t)lData;
    MirData.lEndT = MirData.lStartT;	// Just in case the 'end time' info is missing...
    StdfFile.ReadByte(&MirData.bStation);	// stat #
    StdfFile.ReadByte(&MirData.bModeCode);			// mode_code
    StdfFile.ReadByte(&MirData.bRtstCode);			// rtst_code
    StdfFile.ReadByte(&MirData.bProdCode);			// prot_cod #
    StdfFile.ReadWord(&MirData.iBurnTime);			// burn_time
    StdfFile.ReadByte(&MirData.cModeCode);			// cmode_code
    if(ReadStringToField(MirData.szLot) != 1)
      return;
    if(ReadStringToField(MirData.szPartType) != 1)
      return;
    if(ReadStringToField(MirData.szNodeName) != 1)
      return;
    if(ReadStringToField(MirData.szTesterType) != 1)
      return;
    if(ReadStringToField(MirData.szJobName) != 1)
      return;
    if(ReadStringToField(MirData.szJobRev) != 1)
      return;
    if(ReadStringToField(MirData.szSubLot) != 1)
      return;
    if(ReadStringToField(MirData.szOperator) != 1)
      return;
    if(ReadStringToField(MirData.szExecType) != 1)
      return;
    if(ReadStringToField(MirData.szExecVer) != 1)
      return;
    if(ReadStringToField(MirData.szTestCode) != 1)
      return;
    if(ReadStringToField(MirData.szTestTemperature) != 1)
      return;
    if(ReadStringToField(MirData.szUserText) != 1)
      return;
    if(ReadStringToField(MirData.szAuxFile) != 1)
      return;
    if(ReadStringToField(MirData.szPkgType) != 1)
      return;
    if(ReadStringToField(MirData.szFamilyID) != 1)
      return;
    if(ReadStringToField(MirData.szDateCode) != 1)
      return;
    if(ReadStringToField(MirData.szFacilityID) != 1)
      return;
    if(ReadStringToField(MirData.szFloorID) != 1)
      return;
    if(ReadStringToField(MirData.szProcID) != 1)
      return;
    if(ReadStringToField(MirData.szperFrq) != 1)
      return;
    if(ReadStringToField(MirData.szSpecName) != 1)
      return;
    if(ReadStringToField(MirData.szSpecVersion) != 1)
      return;
    if(ReadStringToField(MirData.szFlowID) != 1)
      return;
    if(ReadStringToField(MirData.szSetupID) != 1)
      return;
    if(ReadStringToField(MirData.szDesignRev) != 1)
      return;
    if(ReadStringToField(MirData.szEngID) != 1)
      return;
    if(ReadStringToField(MirData.szROM_Code) != 1)
      return;
    if(ReadStringToField(MirData.szSerialNumber) != 1)
      return;
    if(ReadStringToField(MirData.szSuprName) != 1)
      return;
    break;
  }
}

/////////////////////////////////////////////////////////////////////////////
// Read STDF SDR record
/////////////////////////////////////////////////////////////////////////////
void CGexFileInGroup::ReadSDR(void)
{
  // SDR
  BYTE                  bData;
  int					nSiteCount, nHeadNum, nSite;
  GP_SiteDescription	clSite;
  char                  szString[256];
  QVector<int>          pSiteNumbers;

  // Nothing to do in Pass 2
  if(lPass == 2)
    return;

  // SDR record present only in STDF V4
  if(StdfRecordHeader.iStdfVersion != GEX_STDFV4)
    return;

  // Read SDR SDTF V4
  StdfFile.ReadByte(&bData);		// Head nb
  nHeadNum = (int)bData;
  StdfFile.ReadByte(&bData);		// Site group nb
  StdfFile.ReadByte(&bData);		// Site count
  nSiteCount = (int)bData;
  if(nSiteCount != 0)
  {
    // Keep track of site numbers
    pSiteNumbers.resize(nSiteCount);
    for(nSite=0; nSite<nSiteCount; nSite++)
    {
      StdfFile.ReadByte(&bData);			// Site#
      pSiteNumbers[nSite] = (int)bData;
    }
  }

  if(ReadStringToField(szString) != 1)		// Handler type
    goto RecordEnd;
  clSite.m_strHandlerType = szString;

  // If reach this point then we have at least one SDR valid record, even if nSiteCount states otherwise!
  if(nSiteCount == 0)
  {
    // Overload site count
    nSiteCount = 1;
    // Overload site map ID
    nSite = 0;

    // Fill buffer
    pSiteNumbers.resize(nSiteCount);
    pSiteNumbers[nSite] = 1;
  }

  if(ReadStringToField(szString) != 1)		// Handler ID
    goto RecordEnd;
  clSite.m_strHandlerProberID = szString;
  if(ReadStringToField(szString) != 1)		// Prober card type
    goto RecordEnd;
  clSite.m_strProbeCardType = szString;
  if(ReadStringToField(szString) != 1)		// Prober card ID
    goto RecordEnd;
  clSite.m_strProbeCardID = szString;
  if(ReadStringToField(szString) != 1)		// Loadboard type
    goto RecordEnd;
  clSite.m_strLoadBoardType = szString;
  if(ReadStringToField(szString) != 1)		// Loadboard ID
    goto RecordEnd;
  clSite.m_strLoadBoardID = szString;
  if(ReadStringToField(szString) != 1)		// DIBboard type
    goto RecordEnd;
  if(ReadStringToField(szString) != 1)		// DIBboard ID
    goto RecordEnd;
  clSite.m_strDibBoardID = szString;
  if(ReadStringToField(szString) != 1)		// Interface cable type
    goto RecordEnd;
  if(ReadStringToField(szString) != 1)		// Interface cable ID
    goto RecordEnd;
  clSite.m_strInterfaceCableID = szString;
  if(ReadStringToField(szString) != 1)		// Handler contactor type
    goto RecordEnd;
  if(ReadStringToField(szString) != 1)		// Handler contactor ID
    goto RecordEnd;
  clSite.m_strHandlerContactorID = szString;
  if(ReadStringToField(szString) != 1)		// Laser type
    goto RecordEnd;
  if(ReadStringToField(szString) != 1)		// Laser ID
    goto RecordEnd;
  clSite.m_strLaserID = szString;
  if(ReadStringToField(szString) != 1)		// Extra equipment type
    goto RecordEnd;
  if(ReadStringToField(szString) != 1)		// Extra equipment ID
    goto RecordEnd;
  clSite.m_strExtraEquipmentID = szString;

  RecordEnd:
  // Update global equipment object
  m_pGlobalEquipmentID->m_nHeadNum = 255;
  m_pGlobalEquipmentID->m_nSiteNum = 255;
  *m_pGlobalEquipmentID += clSite;

  // Update individual sites
  for(nSite=0; nSite<nSiteCount; nSite++)
  {
    clSite.m_nHeadNum = nHeadNum;
    clSite.m_nSiteNum = pSiteNumbers[nSite];
    m_pSiteEquipmentIDMap->insert(clSite.m_nSiteNum, clSite);
  }
}

/////////////////////////////////////////////////////////////////////////////
// Read STDF PRR record
/////////////////////////////////////////////////////////////////////////////
void CGexFileInGroup::ReadPRR(void)
{
    CTest	*ptTestCell=0;
    CTest *ptFailTestCell = NULL;
    CPartInfo PartInfo;		// Structure to hold part info: DieXY, Exec time, Tests executed, partID, etc...
    BYTE	cPassFail;
    int		iBin=0;
    static char	szString[1024];
    bool	bIgnoreSite=false;
    bool	bIgnorePart=false;	// Will remain true if site# filter not matching, or if part must not be processed
    //  bool	bPassFailStatusValid=true,
    bool bPartFailed=false;
    unsigned uMapIndex=0;
    long	lPartID				= -1;		// Will receive PRR partID number (unless not available).
    long	lInternalPartID		= -1;
    bool	bNumericalPartID	= false;

    BYTE bSiteNumber = 0;

    // PRR
    *szString = 0;	// Reset Part ID.


    StdfFile.ReadByte(&PartInfo.bHead);	// head number
    StdfFile.ReadByte(&bSiteNumber);	// site number
    PartInfo.m_site = bSiteNumber;


    // get the deciphering mode
    if(StdfFile.GetHeadAndSiteNumberDecipheringMode() != GS::StdLib::no_deciphering)
    {
        PartInfo.m_site = GQTL_STDF::HeadAndSiteNumberDecipher::DecipherSiteNumberForAdvantest(PartInfo.bHead, PartInfo.m_site);
        PartInfo.bHead = GQTL_STDF::HeadAndSiteNumberDecipher::DecipherHeadNumberForAdvantest( PartInfo.bHead);
    }

    uMapIndex = (int)(PartInfo.bHead << 16) | PartInfo.m_site;	// Compute index for Head&Site...as in multi-head/sites, we have to save multiple results before the PRR

    if (lMappingSiteHead.find(uMapIndex) == lMappingSiteHead.end())
    {
        if (m_eStdfCompliancy == CGexFileInGroup::STRINGENT)
        {
            GS::Gex::Message::warning(
                "",
                QString("Error parsing file %1:\n"
                        "PIR and PRR do not match.\n\n"
                        "You may check the file content using "
                        "'Toolbox->STDF Records Dump'.").
                arg(strFileName));
        }
    }
    //  if(lMappingSiteHead.find(uMapIndex) != lMappingSiteHead.end() && lMappingSiteHead[uMapIndex] == 0){
    //      // PIR is missing PIR/PRR block not closed
    //      //show a warning
    //  }
    //  if(lPass == 1){
    //      m_PRRListing.append(sPirPrrMapping());
    //      m_PRRListing.last().bHead = PartInfo.bHead;
    //      m_PRRListing.last().bSite = PartInfo.bSite;
    //      m_PRRListing.last().iIdx = m_iPRRCount;
    //  }
    //
    lMappingSiteHead[uMapIndex]= 0;		// Reset the flag saying we enter in a PIR/PRR block

    // Check if a test failied in this flow.
    if(m_map_ptFailTestCell.contains(PartInfo.m_site))
        ptFailTestCell = m_map_ptFailTestCell[PartInfo.m_site];

    switch(StdfRecordHeader.iStdfVersion)
    {
    default :
        break;

        ///////// ********* STDF V4 ********
    case GEX_STDFV4:
    {
        if((iProcessSite >=0) && (iProcessSite != (int)PartInfo.m_site))
        {
            bIgnoreSite = true;
            break;	// Site filter not matching.
        }

        StdfFile.ReadByte(&bData);	// part flag
        //      bPassFailStatusValid = !((int)bData & 0x0010);
        bPartFailed = (int)bData & 0x0008;
        if(bPartFailed)
        {
            PartInfo.bPass = false;	// Run was FAIL
            cPassFail = 'F';
        }
        else
        {
            PartInfo.bPass = true;	// Run was PASS
            cPassFail = 'P';
        }

        StdfFile.ReadWord(&wData);	// number of tests
        PartInfo.iTestsExecuted = wData;
        if(PartInfo.iTestsExecuted == 65535)	// Some testers (ie Eagle) set this field to 65535 instead of 0 when no tests were executed (only limits defined)
            PartInfo.iTestsExecuted = 0;
        StdfFile.ReadWord(&wData);	// HBIN
        PartInfo.iHardBin = wData;
        StdfFile.ReadWord(&wData);	// SBIN
        PartInfo.iSoftBin = wData;

        if(lPass ==1)
            m_oSBinHBinMap.insertMulti(PartInfo.iSoftBin , PartInfo.iHardBin );

        // Get bin result
        if(PartInfo.iSoftBin != 65535)
            iBin = PartInfo.iSoftBin;
        else
            iBin = PartInfo.iHardBin;

        // Save part #
        PartInfo.lPartNumber = PartProcessed.partNumber(PartInfo.m_site);

        // Update part counter, etc...
        bIgnorePart = UpdateBinResult(&PartInfo);

        // Check if Have to Collect Binning Trend data (normally done in HBR, S BR, but if Summary is disabled, we have to do it within PRRs!)
        //QString strOptionStorageDevice;
        //strOptionStorageDevice = (pReportOptions->GetOption("binning","computation")).toString();
        // if((bIgnorePart==false) && pReportOptions->bBinningUseSamplesOnly)
        if((bIgnorePart==false) && m_eBinComputation==SAMPLES )
            //if((bIgnorePart==false) && (strOptionStorageDevice == "samples"))
        {
            AdvCollectBinningTrend(&m_ParentGroup->cMergedData.ptMergedSoftBinList,PartInfo.iSoftBin,1);
            AdvCollectBinningTrend(&m_ParentGroup->cMergedData.ptMergedHardBinList,PartInfo.iHardBin,1);
        }

        // Save Soft-Bin parameter info.
        if(PartInfo.iSoftBin != 65535)
        {
            // Save failing bin into relevant test structure if this run failed.
            if(ptFailTestCell != NULL)
            {
                ptFailTestCell->iFailBin = PartInfo.iSoftBin;
                ptFailTestCell = NULL;
            }
        }

        // Save Hard-Bin parameter info.
        if(PartInfo.iHardBin != 65535)
        {
            // Save failing bin into relevant test structure if this run failed.
            if(ptFailTestCell != NULL)
            {
                ptFailTestCell->iFailBin = PartInfo.iHardBin;
                ptFailTestCell = NULL;
            }
        }

        if(StdfFile.ReadWord(&wData) != GS::StdLib::Stdf::NoError)
            wData = -32768;	// no DIE X
        PartInfo.iDieX = wData;

        if(StdfFile.ReadWord(&wData) != GS::StdLib::Stdf::NoError)
            wData = -32768;	// no DIE Y
        PartInfo.iDieY = wData;

        // 2-Aug-2004 PhL: Test removed as ITS9000 testers sometimes have this field set to 0 and still valid wafermap/Bin info.
#if 0
        // If no tests performed, force Die location = Unrelevant
        //if(PartInfo.iTestsExecuted <= 0)
        //	PartInfo.iDieX = PartInfo.iDieY = -32768;
#endif
        switch(pReportOptions->iWafermapType)
        {
            case GEX_WAFMAP_SOFTBIN:
            case GEX_WAFMAP_STACK_SOFTBIN:
            case GEX_WAFMAP_ZONAL_SOFTBIN:
                // Wafer map on: SOFT  bin
                iBin = PartInfo.iSoftBin;
                break;
            case GEX_WAFMAP_HARDBIN:
            case GEX_WAFMAP_STACK_HARDBIN:
            case GEX_WAFMAP_ZONAL_HARDBIN:
                // Wafer map on: HARD  bin
                iBin = PartInfo.iHardBin;
                break;
        }
        // Read test time, and get test time info if GOOD binning (bit3,4=0)
        if(StdfFile.ReadDword(&lData) == GS::StdLib::Stdf::NoError)
        {
            // Save execution time
            PartInfo.lExecutionTime = lData;

            lAverageTestTime_All += lData;	// Counts in Ms.
            lTestTimeParts_All++;			// Number of parts used to compute test time.

            // If Passing part, then use this exec time to compute the everage testing time of good devices.
            if((lPass == 1) && ((bData & 0x18) == 0) && (lData > 0))
            {
                lAverageTestTime_Good += lData;	// Counts in Ms.
                lTestTimeParts_Good++;			// Number of parts used to compute test time.
            }
        }
        else
        {
            // No more data in record
            PartInfo.lExecutionTime=0;
        }

        // Create Parameter entry for Testing site
        UpdateCustomParameter(bIgnorePart, "Testing_Site parameter",GEX_TESTNBR_OFFSET_EXT_TESTING_SITE,(float)PartInfo.m_site,&PartInfo);

        // Create Parameter entry for TestTemperature (if a custom value was specified from the 'File Properties' page)
        if(m_lfTemperature > C_NO_TEMPERATURE)
            UpdateCustomParameter(bIgnorePart, "Test_Temperature parameter",GEX_TESTNBR_OFFSET_EXT_TEMPERATURE,(float)m_lfTemperature,&PartInfo);

        // Create Parameter entry for Soft_Bin
        if(PartInfo.iSoftBin != 65535 || (m_eStdfCompliancy==CGexFileInGroup::FLEXIBLE))
            UpdateCustomParameter(bIgnorePart, "Soft_Bin parameter",GEX_TESTNBR_OFFSET_EXT_SBIN,(float)PartInfo.iSoftBin,&PartInfo);

        // Create Parameter entry for Hard_Bin
        if(PartInfo.iHardBin != 65535 || (m_eStdfCompliancy==CGexFileInGroup::FLEXIBLE))
            UpdateCustomParameter(bIgnorePart, "Hard_Bin parameter",GEX_TESTNBR_OFFSET_EXT_HBIN,(float)PartInfo.iHardBin,&PartInfo);

        // Create Parameter entry for Test time (in sec.).
        if(lData)
            UpdateCustomParameter(bIgnorePart, "Test_Time parameter",GEX_TESTNBR_OFFSET_EXT_TTIME,((float)lData)/1e3,&PartInfo);

        *szString = 0;
        StdfFile.ReadString(szString);// PART ID.
        PartInfo.setPartID(szString);

        lPartID = PartInfo.getPartID().toLong(&bNumericalPartID);

        if (m_mapPartID.contains(PartInfo.getPartID()))
            lInternalPartID = m_mapPartID.value(PartInfo.getPartID());
        else
        {
            lInternalPartID = m_lCurrentInternalPartID++;

            if (PartInfo.getPartID().isEmpty() == false)
                m_mapPartID.insert(PartInfo.getPartID(), lInternalPartID);
        }

        // Update partID list into results list (if PartID list is enabled)
        if(bNumericalPartID == false)
            lPartID = lInternalPartID;	// If no PartID defined, then force it to run# instead.

        if (PartInfo.getPartID().isEmpty())
            PartInfo.setPartID(QString::number(lPartID));

        // Save PartID
        UpdateCustomParameter(bIgnorePart, "Part_ID parameter",GEX_TESTNBR_OFFSET_EXT_PARTID,lInternalPartID,&PartInfo);

        StdfFile.ReadString(szString);// PART TXT.
        PartInfo.setPartText(szString);
        if(*szString)
            m_strPart_TXT += szString;		// Keep list of PartTXT strings.

        StdfFile.ReadString(szString);// PART FIX.
        PartInfo.strPartRepairInfo = szString;

        // Update wafermap info if part note filtered, or if wafermap must be FULL wafermap (then ignore part filtering!)
        if((bIgnorePart==false) || m_bFullWafermap)
            UpdateWaferMap(iBin,&PartInfo, lInternalPartID);

        // If no binning summary found, or we must use PRR for Bin count, we have to build it now!
        if( (bIgnorePart==false) && (lPass==2) &&
                ( (bBinningSummary == false)
                  || (m_eBinComputation==SAMPLES)
                  || (m_eBinComputation==WAFER_MAP)
                  )
                )
        {
            // Add Soft Bin inf to list
            AddBinCell(&m_ParentGroup->cMergedData.ptMergedSoftBinList,PartInfo.m_site,
                       PartInfo.iDieX,PartInfo.iDieY,PartInfo.iSoftBin,PartInfo.iHardBin,
                       cPassFail,1,true,true,"",false);
            // Add Hard Bin inf to list
            AddBinCell(&m_ParentGroup->cMergedData.ptMergedHardBinList,PartInfo.m_site,
                       PartInfo.iDieX,PartInfo.iDieY,PartInfo.iSoftBin,PartInfo.iHardBin,
                       cPassFail,1,true,true,"",true);
        }

        // Create Parameter entry for Die_X
        if(PartInfo.iDieX >= 32768)
            fData = PartInfo.iDieX - 65536.0;
        else
            fData = PartInfo.iDieX;
        UpdateCustomParameter(bIgnorePart, "Die_X parameter",GEX_TESTNBR_OFFSET_EXT_DIEX,fData,&PartInfo);

        // Create Parameter entry for Die_Y
        if(PartInfo.iDieY >= 32768)
            fData = PartInfo.iDieY - 65536.0;
        else
            fData = PartInfo.iDieY;
        UpdateCustomParameter(bIgnorePart, "Die_Y parameter",GEX_TESTNBR_OFFSET_EXT_DIEY,fData,&PartInfo);

        // If Wafersort & filtering over First test instance or Last test instance...
        if(lPass == 2 && (iProcessBins == GEX_PROCESSPART_FIRSTINSTANCE || iProcessBins == GEX_PROCESSPART_LASTINSTANCE))
        {
            int nRunIndex = PartProcessed.runIndexFromSite(PartInfo.m_site);

            if(nRunIndex >= 0)
            {
                QString strKey;

                PartIdentification lPartIdentification = mPartIdentifiation;

                if (mPartIdentifiation == AUTO)
                {
                    if (getWaferMapData().bWirExists)
                        lPartIdentification = XY;
                    else
                        lPartIdentification = PARTID;
                }

                if (lPartIdentification == PARTID)
                    strKey = PartInfo.getPartID();
                else if (lPartIdentification == XY)
                {
                    if ((PartInfo.iDieX != 32768) && (PartInfo.iDieY != 32768))
                        strKey = QString::number(PartInfo.iDieX) + "-" + QString::number(PartInfo.iDieY);
                }

                if (strKey.isEmpty() == false)
                {
                    // First instance
                    if(m_First_InstanceDies.contains(strKey) == false)
                        m_First_InstanceDies[strKey] = nRunIndex;	// Run Index for first instance of this die.

                    // Instance already present, overload it.
                    m_Last_InstanceDies[strKey] = nRunIndex;	// Run Index for last instance of this die.
                }
            }
        }

    }
        break;
    }

    // If this site must be ignored, simply do not save temporary Min/Max/Execs values for this site...
    if(bIgnoreSite == false && bIgnorePart == false)
    {
        // Datalog Binning if activated.
        if (lPass == 2)
        {
            if (GexAbstractDatalog::existInstance())
                GexAbstractDatalog::pushPartInfo(&PartInfo);
            else if((pReportOptions->getAdvancedReport() == GEX_ADV_DATALOG) && (lTestsInDatalog > 0) && (hAdvancedReport != NULL))
                WriteDatalogBinResult(iBin,&PartInfo);// Bin + Part ID+DieLocation
        }

        // Keep track of the number of samples in this sub-lot
        if (lPass == 2)
            m_lSamplesInSublot++;
    }

    if(lPass == 1)
    {
        ptTestCell = ptTestList;
        // If part is an outlier...
        if(bIgnorePart == true)
        {
            // Don't reinitialize parameters until parsing the last PRR in the case of nested PRR
            if (PartProcessed.PIRNestedLevel() == 1)
            {
                // Reset 'tested' flag for each test in the test list
                while(ptTestCell != NULL)
                {
                    ptTestCell->bTestExecuted = false;
                    ptTestCell->ldTmpSamplesExecs=0;
                    ptTestCell->lfTmpSamplesMin = ptTestCell->lfSamplesMin;
                    ptTestCell->lfTmpSamplesMax = ptTestCell->lfSamplesMax;

                    // If we exit from the last PRR in the PIR/PRR block, clear the multi result count for this test
                    if (PartProcessed.PIRNestedLevel() == 1)
                        ptTestCell->clearMultiResultCount();

                    ptTestCell = ptTestCell->GetNextTest();
                };
            }
        }
        else
        {
            // This part belongs to our filter...so update min/max and count values
            long	lTestsInPart=0;
            while(ptTestCell != NULL)
            {
                // check if update fields or not !
                if(ptTestCell->bTestExecuted == true)
                {
                    // count total of datalogs in this run
                    lTestsInPart++;

                    // This test was executed...if this part matches the filter
                    ptTestCell->ldSamplesValidExecs   += ptTestCell->ldTmpSamplesExecs;	// Updates sample count for this test...test may have been executed more than once in a run!
                    ptTestCell->ldTmpSamplesExecs     = 0;
                    ptTestCell->GetCurrentLimitItem()->lfHistogramMin        = ptTestCell->lfTmpHistogramMin;
                    ptTestCell->lfSamplesMin          = ptTestCell->lfTmpSamplesMin;
                    ptTestCell->GetCurrentLimitItem()->lfHistogramMax        = ptTestCell->lfTmpHistogramMax;
                    ptTestCell->lfSamplesMax          = ptTestCell->lfTmpSamplesMax;
                    ptTestCell->mHistogramData        = GS::Gex::HistogramData(mHistoBarsCount, ptTestCell->GetCurrentLimitItem()->lfHistogramMin, ptTestCell->GetCurrentLimitItem()->lfHistogramMax) ;

                    // Reset 'tested' flag.
                    ptTestCell->bTestExecuted = false;
                }
                else
                {
                    // No valid result for this run, only increment buffer size for NaN value
                    ptTestCell->ldSamplesExecs++;
                }

                // If we exit from the last PRR in the PIR/PRR block, clear the multi result count for this test
                if (PartProcessed.PIRNestedLevel() == 1)
                    ptTestCell->clearMultiResultCount();

                // Resets temporary samples buffer...to make sure to restrat from clean situation
                ptTestCell = ptTestCell->GetNextTest();
            };

            // If datalogs in this run OR PRR says tests in run, then consider this as a valid sample...
            if(lTestsInPart || PartInfo.iTestsExecuted)
                ldTotalPartsSampled++;
        }

        // Save Pass/Fail flag so we can create the lilst of PASS Bins and FAIL bins
        m_ParentGroup->cMergedData.mSBinPassFailFlag[PartInfo.iSoftBin] = (int) PartInfo.bPass;
        m_ParentGroup->cMergedData.mHBinPassFailFlag[PartInfo.iHardBin] = (int) PartInfo.bPass;
    }

    if(lPass == 2)
    {
        m_ParentGroup->cMergedData.UpdateBinSiteCounters(PartInfo);
    }

    if (bIgnoreSite == false)
        // Increment the part number
        PartProcessed.nextPartNumber();
}

/////////////////////////////////////////////////////////////////////////////
// Read STDF WIR record
/////////////////////////////////////////////////////////////////////////////
void CGexFileInGroup::ReadWIR(void)
{
  char	szString[257];	// A STDF string is 256 bytes long max!

  // WIR
  StdfFile.ReadByte(&bData);	// Head
  StdfFile.ReadByte(&bData);	// pad (stdf V3), test site (stdf V4)
  StdfFile.ReadDword(&lData);	// START_T
  // WaferID
  if(ReadStringToField(szString) < 0)
  {
      // Incomplete record: assign n/a WaferID
      // case 7850
      sprintf(szString, "%s", "n/a");
      //    sprintf(szString, "%02d", ++uiAutoWaferID);
  }
  // LTX-STDF bug fix: if empty WaferID, assume it is the same as previous one found!
  if(*szString == 0)
    strcpy(szString,getWaferMapData().szWaferIDFilter);
  // Check if we read this wafer or not...
  if(*getWaferMapData().szWaferIDFilter == 0)
  {
    // No filter defined...so this is the first WaferID we find in the file...
    bIgnoreThisSubLot = false;	// Set Filter to read all data for this wafer.
    // Mark that this is the first Wafer/Sub-lot processed in this file
    getWaferMapData().bFirstSubLotInFile=true;
  }
  else
  {
    // A filter is defined...so probably we must not read this Wafer data...
    if(qstricmp(szString,getWaferMapData().szWaferIDFilter))
    {
      bIgnoreThisSubLot = true;	// Mismatch with filter: must ignore this wafer and its following related records
      if((lPass == 1) && (getWaferMapData().bFirstSubLotInFile == true))
      {
        // Pass1: While we scan the file for the first time (processing 1st entry in group)
        // we have to create a new File entry for each new waferID detected
        // so we skip it now be will process it alone later in the group.
        int iFileID = gexReport->addFile(lGroupID,strFileNameSTDF,iProcessSite,iProcessBins,
                                         strRangeList.toLatin1().data(),strMapTests,
                                         strWaferToExtract,m_lfTemperature,strDatasetName,mSampleGroup);
        // Set WaferID filter for this file.
        CGexGroupOfFiles *pGroup;
        CGexFileInGroup *pFile;
        pGroup = (lGroupID < 0 || lGroupID >= gexReport->getGroupsList().size()) ? NULL : gexReport->getGroupsList().at(lGroupID);
        if (!pGroup->pFilesList.isEmpty() && iFileID < pGroup->pFilesList.size())
        {
          pFile = pGroup->pFilesList.at(iFileID);
          strcpy(pFile->getWaferMapData().szWaferIDFilter,szString);
          pFile->getWaferMapData().bFirstSubLotInFile = false;
        }
      }
    }
    else
    {
      bIgnoreThisSubLot = false;	// Filter match: read next records (related to this waferID)
    }
  }

  if((strWaferToExtract.isEmpty() == false)
    && (isMatchingWafer(strWaferToExtract,szString) == false))
    bIgnoreThisSubLot = true;	// Query requested to only read one specific WaferID...do so!

  // If we read this wafer, save the TimeStamp & WaferID
  if(bIgnoreThisSubLot == false)
  {
    getWaferMapData().lWaferStartTime = lData;			// Save TimeStamp
    strcpy(getWaferMapData().szWaferID,szString);		// Save WaferID
    strcpy(getWaferMapData().szWaferIDFilter,szString);	// Wafer Filter = This WaferID
    // Overwrite MIR Time/date data with Wafer Time info (other wise all wafers in file would have same Time info!).
    getMirDatas().lStartT = lData;
  }

  // Flag this Data file if it has a WIR record
  getWaferMapData().bWirExists = true;
}
