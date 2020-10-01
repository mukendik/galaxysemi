#include <QString>
#include <QTextStream>
#include <QFile>
#include <QFileInfo>

#include "gqtl_log.h"
#include "importWat.h"




// File format:
//                                                  W.A.T. DATA ATTACHED
// TYPE NO :TME391                        PROCESS  :018TW30LF1                    PCM SPEC:T.B.D         QTY: 1 pcs
// LOT ID  :D62748.05                     DATE     :10/01/2001
// WAF SITE    VT_N4 N1u   Isat_N4     BV_N4       VT_P4 N1u   Isat_P4     BV_P4       VT_N43 N1u  Isat_N43    BV_N43      VT_P43 N1u
// ID  ID      V 10/.18    mA 10/.18   V 10/.18    V 10/.18    mA 10/.18   V 10/.18    V 10/.35    mA 10/.35   V 10/.35    V 10/.3
// 7  -1        0.421       6.512       4.047      -0.469      -3.066      -5.168       0.678       6.663       7.000      -0.667
// 7  -2        0.425       6.497       4.020      -0.474      -3.023      -5.195       0.681       6.588       7.000      -0.673
// 7  -3        0.424       6.477       4.047      -0.479      -2.984      -5.168       0.671       6.686       7.000      -0.668
// 7  -4        0.426       6.478       4.020      -0.459      -3.141      -5.250       0.669       6.595       7.000      -0.665
// 7  -5        0.427       6.420       4.047      -0.471      -3.027      -5.250       0.669       6.560       7.000      -0.667
// .....
// -------------------------------------------------------------------------------------------------------------------------------
// AVERAGE      0.424       6.477       4.036      -0.470      -3.048      -5.206       0.674       6.618       7.000      -0.668
// STD DEV     2.347E-03    0.035       0.015      7.493E-03    0.059       0.041      5.560E-03    0.053       0.000      3.029E-03
// SPEC HI      0.520       6.900       12.000     -0.400      -2.210      -3.600       0.820       6.900       20.000     -0.640
// SPEC LO      0.320       5.100       3.600      -0.600      -2.990      -12.000      0.620       5.100       6.000      -0.840
// -------------------------------------------------------------------------------------------------------------------------------
// WAF SITE    Isat_P43    BV_P43      VTFpo_N+    VTFpo_P+    Rs_NW(STI)  Rs_N+       Rs_P+       Rs_N+Po     Rs_P+Po     Rc_N+
// ID  ID      mA 10/.3    V 10/.3     V .28/128   V .28/128   Ohm/Sq 20   Ohm/Sq .22  Ohm/Sq .22  Ohm/Sq .18  Ohm/Sq .18  Ohm/Se .22
// ....


namespace GS
{
namespace Parser
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
WatWafer::~WatWafer()
{
    foreach(WatParameter* lWatParameter,  mParameterList)
        delete lWatParameter;
}

WatToStdf::WatToStdf():ParserBase(typeWat, "Wat"), mStartTime(0), mTotalParameters(0)
{
    mParameterDirectory.SetFileName(GEX_WAT_PARAMETERS);
}

WatToStdf::WatToStdf(ParserType type, const QString& name):ParserBase(type, name), mStartTime(0), mTotalParameters(0)
{
    mParameterDirectory.SetFileName(GEX_WAT_PARAMETERS);
}

WatToStdf::~WatToStdf()
{
    foreach(WatWafer* lWafer,  mWaferList)
        delete lWafer;
}

//////////////////////////////////////////////////////////////////////
// Check if File is compatible with WAT format
//////////////////////////////////////////////////////////////////////
bool WatToStdf::IsCompatible(const QString &fileName)
{
    // Open hCsmFile file
    QFile f( fileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening ASL1000 file
        return false;
    }

    // Find the correct WAT header in the 10 first lines ...
    QTextStream hWatFile(&f);
    QString strString;
    for(int i=0; i<10; ++i)
    {
        do
        {
            strString = hWatFile.readLine();
        }
        while(!strString.isNull() && strString.isEmpty());

        strString = strString.trimmed();	// remove leading spaces.
        if(strString.startsWith("W.A.T. DATA ATTACHED", Qt::CaseInsensitive))
        {
            f.close();
            return true;
        }
    }

    // Incorrect header...this is not a WAT file!
    f.close();
    return false;
}

//////////////////////////////////////////////////////////////////////
// Convert the WAT file to a STDF file
//////////////////////////////////////////////////////////////////////
bool WatToStdf::ConvertoStdf(const QString &fileName, QString &stdfFileName)
{
    UpdateProgressMessage(QString("Converting data (Parsing WAT file "+ QFileInfo(fileName).fileName()+")") );
    if(ParseFile(fileName))
    {
        UpdateProgressMessage(QString("Converting data (Writing STDF file "+ QFileInfo(stdfFileName).fileName()+")") );
        if(WriteStdfFile(stdfFileName.toStdString().c_str()))
        {
            StopProgressBar();
            return true;
        }
         QFile::remove(stdfFileName);
    }
    StopProgressBar();
    return false;

}

//////////////////////////////////////////////////////////////////////
// Parse the WAT file
//////////////////////////////////////////////////////////////////////
bool WatToStdf::ParseFile(const QString &fileName)
{

    QString strMessage, strString;
    QString strSection;
    QString	strSite;
    QString	strParameters[10];	// Holds the 10 Parameters name (WAT file is organized by sets of 10 parameters columns)
    QString	strUnits[10];		// Holds the 10 Parameters Units
    int		iWaferID;			// WaferID processed
    int		iSiteID;			// SiteID processed
    int		iIndex;				// Loop index

    // Debug trace
    strMessage = "---- WATtoSTDF::ReadWatFile(): reading wat file (";
    strMessage += fileName;
    strMessage += ")";
    GSLOG(SYSLOG_SEV_DEBUG, strMessage.toLatin1().constData());

    // Open WAT file
    QFile lFile( fileName );
    if(!lFile.open( QIODevice::ReadOnly ))
    {
        // Failed Opening WAT file
        mLastError = errOpenFail;
        return false;
    }
    // Assign file I/O stream
    QTextStream hWatFile(&lFile);

    // Find the correct WAT header in the 10 first lines ...
    for(int i=0; i!=10; i++)
    {
        strString = ReadLine(hWatFile);

        strString = strString.trimmed();	// remove leading spaces.
        if(strString.startsWith("W.A.T. DATA ATTACHED", Qt::CaseInsensitive))
            break;
    }

    if(!strString.startsWith("W.A.T. DATA ATTACHED", Qt::CaseInsensitive))
    {
        // Incorrect header...this is not a WAT file!
        mLastError = errInvalidFormatParameter;
        lFile.close();
        return false;
    }

    // Read line : " TYPE NO :TMF671                        PROCESS  :0184W30LF1                    PCM SPEC:TBD           QTY:25 pcs"
    // KEY WORD : VALUE_WITHOUT_SPACE  (only for 25 pcs)
    // Read line until "WAF SITE"
    QString strKeyWord, strValue;
    strString = "";
    while (hWatFile.atEnd() == false)
    {
        if(strString.isEmpty())
            strString = ReadLine(hWatFile);					// Read ProductID, ProcessID
        if(strString.toUpper().indexOf("WAF SITE") >= 0)
            break;

        strKeyWord = strString.section(":",0,0).trimmed();
        strString = strString.section(":",1).trimmed();
        strValue = strString.section(" ",0,0);
        if((strString.indexOf(":") > 0)
        && (strValue.section(":",0,0).trimmed() == strString.section(":",0,0).trimmed()))
        {
            // empty value
            strValue = "";
        }
        else
            strString = strString.section(" ",1).trimmed();

        if(strKeyWord.toUpper() == "TYPE NO")
            mProductID = strValue;
        else
        if(strKeyWord.toUpper() == "PROCESS")
            mProcessID = strValue;
        else
        if(strKeyWord.toUpper() == "PCM SPEC")
            mSpecID = strValue;
        else
        if(strKeyWord.endsWith("QTY",Qt::CaseInsensitive))
            strString = "";
        else
        if(strKeyWord.endsWith("PASS",Qt::CaseInsensitive))
            strString = "";
        else
        if(strKeyWord.toUpper() == "LOT ID")
            mLotID = strValue;
        else
        if(strKeyWord.toUpper() == "DATE")
        {
            // Extract TimeStamp, convert to UINT (nbr of seconds since 1-1-1970)
            int iDay,iMonth,iYear;
            strSection = strValue.section('/',0,0);	// First field is Month
            iMonth = strSection.toInt();
            strSection = strValue.section('/',1,1);	// Second field is Day
            iDay = strSection.toInt();
            strSection = strValue.section('/',2,2);	// Last field is Year
            iYear = strSection.toInt();
            QDate WatDate(iYear,iMonth,iDay);
            QDateTime WateDateTime(WatDate);
            WateDateTime.setTimeSpec(Qt::UTC);
            mStartTime = WateDateTime.toTime_t();

            strString = "";
        }
    }

    // ALREADY READ - Read line with list of Parameters names
    // eg: " WAF SITE    VT_N4 N1u   Isat_N4     BV_N4       VT_P4 N1u   Isat_P4     BV_P4       VT_N43 N1u  Isat_N43    BV_N43      VT_P43 N1u"
    //
    // For some specific WAT, have an additional offset of 3 spaces between each test line
    // Have to found -------- line, and if exist have to add offset
    //WAF SITE   BV_N2 A        BV_N2 S        BV_N3          BV_P2 A        BV_P2 S        BV_P4          CONTI_M1       CONTI_M2
    //ID  ID     V @1uA         V @1uA         V @0.1uA       V @1uA         V @1uA         V @-0.1uA      OHM  0.6um     OHM  0.7um
    //--- ----   ------------   ------------   ------------   ------------   ------------   ------------   ------------   ------------


    // Loop reading file until end is reached
    QString strLineParam1, strLineParam2;
    int nOffset = 0;
    int nStart = 12;
    do
    {
        strLineParam1 = strString;
        strString = ReadLine(hWatFile);
        strLineParam2 = strString;
        strString = ReadLine(hWatFile);
        // Check if have some offset
        if(strString.indexOf("------------") > 0)
        {

            nStart = strString.indexOf("------------");
            strString = strString.section("------------",1);
            nOffset = strString.section("------------",0,0).length();
            strString = ReadLine(hWatFile);
        }

        // Extract the 10 column names (or less if not all are filled)
        for(iIndex=0;iIndex<10;iIndex++)
        {
            strSection = strLineParam1.mid(nStart+iIndex*(12+nOffset),12);
            strSection = strSection.trimmed();	// Remove spaces
            if (!strSection.isEmpty())
                strParameters[iIndex] = strSection;
        }

        // Extract the 10 column Parameters second half of the name (usually contains units)
        for(iIndex=0;iIndex<10;iIndex++)
        {
            strSection = strLineParam2.mid(nStart+iIndex*(12+nOffset),12);
            strSection = strSection.trimmed();	// Remove spaces
            if (!strSection.isEmpty())
            {
                strParameters[iIndex] += " " + strSection;
                strSection.replace("(","").replace(")", ""); // replace ( and ) by empty char
                QStringList lListUnits = strSection.split(" ");
                if (lListUnits.size() > 0)
                    strUnits[iIndex] = lListUnits[0].trimmed();  // the unit is the part before the space
                else
                    strUnits[iIndex] = " ";
            }
        }


        while(strString.indexOf("----------------------") < 0)
        {
            // Read line of Parameter data unless end of data reached (reaching Param. Mean, Sigma and limits)
            // eg: " 7  -1        0.421       6.512       4.047      -0.469      -3.066      -5.168       0.678       6.663       7.000      -0.667"
            // or may be end of data, in which case the line is full of '------'

            // Extract WaferID, and save it to the parameter list as a parameter itself!
            strSection = strString.mid(0,3);
            strSection.remove ('-');	// Remove leading '-' if exists
            iWaferID = strSection.toInt();

            // Extract SiteID
            strSite = strString.mid(5,4);
            strSite.remove ('-');	// Remove leading '-' if exists
            strSite.remove ('#');	// Remove leading '#' if exists...sometimes used to flag a site with failing parameters
            iSiteID = strSite.toInt();

            if(strSection.isEmpty() != true)
                SaveParameterResult(iWaferID, iSiteID, "WAF ID", "", strSection);

            // For each column, extract parameter value and save it
            for(iIndex=0;iIndex<10;iIndex++)
            {
                strSection = strString.mid(nStart+iIndex*(12+nOffset),12);
                    // Save parameter result in buffer
                if(strSection.isNull() != true)
                    SaveParameterResult(iWaferID, iSiteID, strParameters[iIndex], strUnits[iIndex],strSection);
            }

            // Check if last line processed (should normally never happen as statistcs should follow...)
            if(hWatFile.atEnd())
                break;

            // Read next line in file...
            strString = ReadLine(hWatFile);
        };

        // Have reached the statistics section:
        // -------------------------------------------------------------------------------------------------------------------------------
        // AVERAGE      0.424       6.477       4.036      -0.470      -3.048      -5.206       0.674       6.618       7.000      -0.668
        // STD DEV     2.347E-03    0.035       0.015      7.493E-03    0.059       0.041      5.560E-03    0.053       0.000      3.029E-03
        // SPEC HI      0.520       6.900       12.000     -0.400      -2.210      -3.600       0.820       6.900       20.000     -0.640
        // SPEC LO      0.320       5.100       3.600      -0.600      -2.990      -12.000      0.620       5.100       6.000      -0.840
        // -------------------------------------------------------------------------------------------------------------------------------

        // Skip line starting with 'AVERAGE'
        ReadLine(hWatFile);
        // Skip line starting with 'STD DEV'
        ReadLine(hWatFile);

        // Read Parameters Spec HIGH limits.
        strString = ReadLine(hWatFile);
        for(iIndex=0;iIndex<10;iIndex++)
        {
            strSection = strString.mid(nStart+iIndex*(12+nOffset),12);
            if(strSection.isNull() != true)
            {
                // Remove spaces
                strSection = strSection.trimmed();
                // Save parameter result in buffer
                SaveParameterLimit(strParameters[iIndex], strSection ,eHighLimit, "");
            }
        }

        // Read Parameters Spec LOW limits.
        strString = ReadLine(hWatFile);
        for(iIndex=0;iIndex<10;iIndex++)
        {
            strSection = strString.mid(nStart+iIndex*(12+nOffset),12);
            if(strSection.isNull() != true)
            {
                // Remove spaces
                strSection = strSection.trimmed();
                // Save parameter result in buffer
                SaveParameterLimit(strParameters[iIndex], strSection ,eLowLimit, "");
            }
        }

        // Skip line starting with '---------'
        ReadLine(hWatFile);

        // Should point to next bloc of parameters..unless end of file!
        do
        {
          strString = ReadLine(hWatFile);
        }
        while((strString.indexOf("WAF SITE") < 0) && (hWatFile.atEnd() == false));

    }
    while (hWatFile.atEnd() == false);

    UpdateProgressBar(hWatFile);
    // Close file
    lFile.close();

    // All WAT file read...check if need to update the WAT Parameter list on disk?
    if(mParameterDirectory.GetNewParameterFound() == true)
        mParameterDirectory.DumpParameterIndexTable();

    // Success parsing WAT file
    return true;
}

//////////////////////////////////////////////////////////////////////
// Save WAT parameter result...
//////////////////////////////////////////////////////////////////////
void WatToStdf::SaveParameterResult(int waferID, int siteID, QString name, QString units, QString& value)
{
    if(name.isNull() == true)
        return;	// Ignore empty entry!

    // Find Pointer to the Parameter cell
    WatParameter *ptParam = FindParameterEntry(waferID, siteID, name, units);

    // Save the Test value in it
    value.remove ('x');	// Remove any 'x' character if found (sometimes present to flag entry out of specs)
    value.remove ('?');	// Remove any '?' character if found (sometimes present to flag entry out of specs)
    value = value.trimmed();
    ptParam->mValue[siteID] = value.toFloat();
}

//////////////////////////////////////////////////////////////////////
// Save WAT parameter High or Low Limit result...
//////////////////////////////////////////////////////////////////////
void WatToStdf::SaveParameterLimit(const QString &name, QString value, int limit, QString units)
{
    if(name.isNull() == true)
        return;	// Ignore empty entry!

    WatParameter *                  ptParam		= 0;					// List of parameters
    QList<WatWafer*>::iterator      lWafer(mWaferList.begin()), lWaferEnd(mWaferList.end());

    for(lWafer = mWaferList.begin(); lWafer != lWaferEnd; ++lWafer)
    {
        // Get Parameter pointer...save limits in ALL WaferEntry.
        ptParam = FindParameterEntry((*lWafer)->GetWaferID(), 0, name, units);
        switch(limit)
        {
            case eHighLimit:
                ptParam->SetHighLimit(value.toFloat() * ptParam->rescale());
                ptParam->SetValidHighLimit(true);		// High limit defined
                break;
            case eLowLimit:
                ptParam->SetLowLimit(value.toFloat() * ptParam->rescale());
                ptParam->SetValidLowLimit(true);		// Low limit defined
                break;
            default:break;
        }
    }
}

//////////////////////////////////////////////////////////////////////
// Find or create Parameter entry, return handle to it.
//////////////////////////////////////////////////////////////////////
WatParameter *WatToStdf::FindParameterEntry(int iWaferID,int iSiteID, QString paramName, QString units)
{
    WatWafer *						ptWafers	= 0;		// List of Wafers in WAT file
    WatParameter *					ptParam		= 0;		// List of parameters

    QList<WatWafer*>::iterator	itWafer = std::find_if(mWaferList.begin(),
                                                       mWaferList.end(),
                                                       std::bind2nd(WatWaferMatch(), iWaferID));

    if(itWafer != mWaferList.end())
    {
        ptWafers = (*itWafer);
        // Update Min/Max SiteID
        if(iSiteID > 0)
        {
            ptWafers->SetLowestSiteID(std::min(ptWafers->GetLowestSiteID(),iSiteID));				// Lowest WaferID found in WAT file
            ptWafers->SetHighestSiteID(std::max(ptWafers->GetHighestSiteID(),iSiteID));				// Highest WaferID found in WAT file
        }

        QList<WatParameter*>::iterator	itParam = std::find_if(ptWafers->mParameterList.begin(),
                                                               ptWafers->mParameterList.end(),
                                                               std::bind2nd(WatParameterMatch(), paramName));
        if(itParam != ptWafers->mParameterList.end())
            return *itParam;

    }
    else
    {
        // WaferID entry doesn't exist yet...so create it now!
        ptWafers = new WatWafer;
        ptWafers->mWaferID = iWaferID;

        // Update Min/Max SiteID
        if(iSiteID > 0)
        {
            ptWafers->mLowestSiteID=iSiteID;				// Lowest WaferID found in WAT file
            ptWafers->mHighestSiteID=iSiteID;				// Highest WaferID found in WAT file
        }

        // Add Wafer entry to existing list.
        mWaferList.append(ptWafers);
    }

    ptParam = CreateWatParameter(paramName, units);
    ptWafers->mParameterList.append(ptParam);
    mTotalParameters++;

    // If Examinator doesn't have this WAT parameter in his dictionnary, have it added.
    mParameterDirectory.UpdateParameterIndexTable(paramName);

    // Return pointer to the Parameter
    return ptParam;
}


WatParameter*   WatToStdf::CreateWatParameter(const QString& paramName, const QString& units)
{
    WatParameter* ptParam           = new WatParameter();
    ptParam->SetTestName(paramName);
    ptParam->SetTestUnit(units);
    ptParam->SetStaticHeaderWritten(false);

    return ptParam;
}

void WatToStdf::SetWIRRecord(GQTL_STDF::Stdf_WIR_V4 &lWIRV4Record, int waferID )
{
    char		szString[257];
    lWIRV4Record.SetHEAD_NUM(1);                    // Test head
    lWIRV4Record.SetSITE_GRP(255);					// Tester site (all)
    lWIRV4Record.SetSTART_T(mStartTime);			// Start time
    sprintf(szString,"%d",waferID);
    lWIRV4Record.SetWAFER_ID(szString);				// WaferID
}

void WatToStdf::SetWRRRecord(GQTL_STDF::Stdf_WRR_V4 &lWRRV4Record, int waferID, int totalBin , int totalGoodBin)
{

    lWRRV4Record.SetHEAD_NUM(1);                            // Test head
    lWRRV4Record.SetSITE_GRP(255);                          // Tester site (all)
    lWRRV4Record.SetFINISH_T(mStartTime);                   // Time of last part tested
    lWRRV4Record.SetPART_CNT(totalBin);	// Parts tested: always 5
    lWRRV4Record.SetRTST_CNT(0);                            // Parts retested
    lWRRV4Record.SetABRT_CNT(0);                            // Parts Aborted
    lWRRV4Record.SetGOOD_CNT(totalGoodBin);                // Good Parts
    lWRRV4Record.SetFUNC_CNT((DWORD)-1);                    // Functionnal Parts

    char		szString[257];
    sprintf(szString,"%d", waferID);
    lWRRV4Record.SetWAFER_ID(szString);				// WaferID
}

void WatToStdf::SetMIRRecord(GQTL_STDF::Stdf_MIR_V4 &lMIRV4Record, const char* userType, bool eTest)
{
    lMIRV4Record.SetSETUP_T(mStartTime);                                // Setup time
    lMIRV4Record.SetSTART_T(mStartTime);                                // Start time
    lMIRV4Record.SetSTAT_NUM(1);                                        // Station
    lMIRV4Record.SetMODE_COD((BYTE) 'P');                               // Test Mode = PRODUCTION
    lMIRV4Record.SetRTST_COD((BYTE) ' ');                               // rtst_cod
    lMIRV4Record.SetPROT_COD((BYTE) ' ');                               // prot_cod
    lMIRV4Record.SetBURN_TIM(65535);                                    // burn_tim
    lMIRV4Record.SetCMOD_COD((BYTE) ' ');                               // cmod_cod
    lMIRV4Record.SetLOT_ID(mLotID);                                     // Lot ID
    lMIRV4Record.SetPART_TYP(mProductID);                               // Part Type / Product ID
    lMIRV4Record.SetNODE_NAM("");                                       // Node name
    lMIRV4Record.SetTSTR_TYP(mTesterType);                              // Tester Type
    lMIRV4Record.SetJOB_NAM(mJobName);                                  // Job name
    lMIRV4Record.SetJOB_REV(mProgRev);                                  // Job rev
    lMIRV4Record.SetSBLOT_ID(mSubLotID);                                // sublot-id
    lMIRV4Record.SetOPER_NAM(mOperatorName);                            // operator
    lMIRV4Record.SetEXEC_TYP("");                                       // exec-type
    lMIRV4Record.SetEXEC_VER("");                                       // exe-ver
    lMIRV4Record.SetTEST_COD(mTestCode);                                // test-cod
    lMIRV4Record.SetTST_TEMP(mTemperature);                             // test-temperature
    // Construct custom Galaxy USER_TXT
    QString	strUserTxt(GEX_IMPORT_DATAORIGIN_LABEL);
    strUserTxt.append(":");
    if(eTest)
        strUserTxt.append(GEX_IMPORT_DATAORIGIN_ETEST);
    else
        strUserTxt.append(GEX_IMPORT_DATAORIGIN_ATETEST);
    strUserTxt.append(userType);
    lMIRV4Record.SetUSER_TXT(strUserTxt.toLatin1().data());                   // user-txt

    lMIRV4Record.SetAUX_FILE("");                                   // aux-file
    lMIRV4Record.SetPKG_TYP(mPackageId);                            // package-type
    lMIRV4Record.SetFAMLY_ID(mFamilyID);                            // familyID
    lMIRV4Record.SetDATE_COD("");                                   // Date-code
    lMIRV4Record.SetFACIL_ID("");                                   // Facility-ID
    lMIRV4Record.SetFLOOR_ID("");                                   // FloorID
    lMIRV4Record.SetPROC_ID(mProcessID);                            // ProcessID
}


//////////////////////////////////////////////////////////////////////
// Normalize test limits when writing into PTR.
//////////////////////////////////////////////////////////////////////
void WatToStdf::NormalizeLimits(WatParameter *pParameter)
{
    if(pParameter->GetTestUnits().length() <= 1)
    {
        // units too short to include a prefix, then keep it 'as-is'
        pParameter->SetResultScale(0);
        return;
    }
    if(pParameter->GetTestUnits().startsWith("MIRCO", Qt::CaseInsensitive))
    {
        // exception
        pParameter->SetResultScale (0);
        return;
    }

    int	value_scale=0;
    QChar cPrefix = pParameter->GetTestUnits()[0];
    switch(cPrefix.toLatin1())
    {
        case 'm': // Milli
            value_scale = -3;
            break;
        case 'u': // Micro
            value_scale = -6;
            break;
        case 'n': // Nano
            value_scale = -9;
            break;
        case 'p': // Pico
            value_scale = -12;
            break;
        case 'f': // Fento
            value_scale = -15;
            break;
        case 'K': // Kilo
            value_scale = 3;
            break;
        case 'M': // Mega
            value_scale = 6;
            break;
        case 'G': // Giga
            value_scale = 9;
            break;
        case 'T': // Tera
            value_scale = 12;
            break;
    }
    pParameter->SetResultScale(value_scale);

    int lLowLimit  = pParameter->GetLowLimit() * pParameter->rescale();
    int lHighLimit = pParameter->GetHighLimit() * pParameter->rescale();
    pParameter->SetLowLimit(lLowLimit);
    pParameter->SetHighLimit(lHighLimit);

    if(value_scale)
        pParameter->SetTestUnit(pParameter->GetTestUnits().mid(1));	// Take all characters after the prefix.
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from WAT data parsed
//////////////////////////////////////////////////////////////////////
bool WatToStdf::WriteStdfFile(const char *strFileNameSTDF)
{

    QString strMessage("Converting data (Writing STDF file "+ QFileInfo(strFileNameSTDF).fileName());

    if(mStdfParse.Open(strFileNameSTDF,STDF_WRITE) == false)
    {
        // Failed importing WAT file into STDF database
        mLastError = errWriteSTDF;

        // Convertion failed.
        return false;
    }


    if(mStartTime <= 0)
        mStartTime = QDateTime::currentDateTime().toTime_t();

    // Write MIR
    GQTL_STDF::Stdf_MIR_V4 lMIRV4Record;
    mTestCode = "WAFER";
    mFamilyID = mProductID;
    SetMIRRecord(lMIRV4Record, ":VIS");
    lMIRV4Record.SetOPER_FRQ("");                                   // Operation
    lMIRV4Record.SetSPEC_NAM("");                                   // spec_nam
    lMIRV4Record.SetSPEC_VER(mSpecID.toLatin1().constData());       // spec_ver
    mStdfParse.WriteRecord(&lMIRV4Record);

    // Write Test results for each waferID
    char		szString[257];
    BYTE		iSiteNumber,bData;
    WORD		wSoftBin,wHardBin;
    long		iTotalGoodBin,iTotalFailBin;
    long		iTestNumber,iPartNumber=0;
    bool		bPassStatus;

    WatParameter *					ptParam		= 0;
    WatWafer *						ptWafers	= 0;		// List of Wafers in WAT file
    QList<WatWafer*>::iterator		itWafer		= mWaferList.begin();
    QList<WatParameter*>::iterator	itParam;

    GQTL_STDF::Stdf_WIR_V4 lWIRV4Record;
    StartProgressStatus(mWaferList.size(), strMessage + ")");

    while(itWafer != mWaferList.end())
    {
        ptWafers = (*itWafer);

        // Write WIR
        SetWIRRecord(lWIRV4Record,  ptWafers->mWaferID);
        mStdfParse.WriteRecord(&lWIRV4Record);
        lWIRV4Record.Reset();

        // Write all Parameters read on this wafer.: PTR....PTR, PRR
        iTotalGoodBin=iTotalFailBin=0;

        // Write PTRs for EACH of the X sites
        GQTL_STDF::Stdf_PIR_V4 lPIRV4Record;
        for(iSiteNumber = (BYTE)ptWafers->mLowestSiteID; iSiteNumber <= (BYTE)ptWafers->mHighestSiteID; ++iSiteNumber)
        {
            // Part number
            ++iPartNumber;
            bPassStatus = true;
            itParam = ptWafers->mParameterList.begin();	// First test in list

            // Write PIR for parts in this Wafer site
            lPIRV4Record.SetHEAD_NUM(1);								// Test head
            lPIRV4Record.SetSITE_NUM(iSiteNumber);					// Tester site
            mStdfParse.WriteRecord(&lPIRV4Record);
            lPIRV4Record.Reset();

            GQTL_STDF::Stdf_PTR_V4 lPTRV4Record;
            while(itParam != ptWafers->mParameterList.end())
            {
                ptParam = (*itParam);
                UpdateProgressBar();
                // Write the PTR if it exists for this site...
                if(ptParam->mValue.contains (iSiteNumber) == true)
                {
                    // Compute Test# (add user-defined offset)
                    iTestNumber = (long) mParameterDirectory.GetFullParametersList().indexOf(ptParam->GetTestName());
                    iTestNumber += GEX_TESTNBR_OFFSET_WAT;		    // Test# offset

                    lPTRV4Record.SetTEST_NUM(iTestNumber);			// Test Number
                    lPTRV4Record.SetHEAD_NUM(1);				    // Test head
                    lPTRV4Record.SetSITE_NUM(iSiteNumber);			// Tester site:1,2,3,4 or 5, etc.
                    if((ptParam->GetValidLowLimit()) && (ptParam->mValue[iSiteNumber] < ptParam->GetLowLimit()))
                    {
                        bData = 0200;	// Test Failed
                        bPassStatus = false;
                    }
                    else if((ptParam->GetValidHighLimit()) && (ptParam->mValue[iSiteNumber] > ptParam->GetHighLimit()))
                    {
                        bData = 0200;	// Test Failed
                        bPassStatus = false;
                    }
                    else
                    {
                        bData = 0;		// Test passed
                    }
                    lPTRV4Record.SetTEST_FLG(bData);							// TEST_FLG
                    lPTRV4Record.SetPARM_FLG(static_cast<stdf_type_b1>(0x40|0x80));						// PARAM_FLG
                    lPTRV4Record.SetRESULT(ptParam->mValue[iSiteNumber]);       // Test result
                    if(ptParam->GetStaticHeaderWritten() == false)
                    {
                        lPTRV4Record.SetTEST_TXT(ptParam->GetTestName().toLatin1().constData());	// TEST_TXT
                        lPTRV4Record.SetALARM_ID("");							// ALARM_ID

                        bData = 2;	// Valid data.
                        if(ptParam->GetValidLowLimit()==false)
                            bData |=0x40;
                        if(ptParam->GetValidHighLimit()==false)
                            bData |=0x80;
                        lPTRV4Record.SetOPT_FLAG(bData);                                    // OPT_FLAG

                        lPTRV4Record.SetRES_SCAL(0);                                        // RES_SCALE
                        lPTRV4Record.SetLLM_SCAL(0);                                        // LLM_SCALE
                        lPTRV4Record.SetHLM_SCAL(0);                                        // HLM_SCALE
                        lPTRV4Record.SetLO_LIMIT(ptParam->GetLowLimit());                       // LOW Limit
                        lPTRV4Record.SetHI_LIMIT(ptParam->GetHighLimit());                      // HIGH Limit
                        lPTRV4Record.SetUNITS(ptParam->GetTestUnits().toLatin1().constData());	// Units
                        ptParam->SetStaticHeaderWritten (true);
                    }
                    mStdfParse.WriteRecord(&lPTRV4Record);
                    lPTRV4Record.Reset();
                }

                // Next Parameter!
                ++itParam;
            };


            // Write PRR
            GQTL_STDF::Stdf_PRR_V4 lPRRV4Record;
            lPRRV4Record.SetHEAD_NUM(1);					// Test head
            lPRRV4Record.SetSITE_NUM(iSiteNumber);			// Tester site:1,2,3,4 or 5
            if(bPassStatus == true)
            {
                lPRRV4Record.SetPART_FLG(0);				// PART_FLG : PASSED
                wSoftBin = wHardBin = 1;
                ++iTotalGoodBin;
            }
            else
            {
                lPRRV4Record.SetPART_FLG(8);				// PART_FLG : FAILED
                wSoftBin = wHardBin = 0;
                ++iTotalFailBin;
            }
            lPRRV4Record.SetNUM_TEST((WORD)ptWafers->mParameterList.count());		// NUM_TEST
            lPRRV4Record.SetHARD_BIN(wHardBin);            // HARD_BIN
            lPRRV4Record.SetSOFT_BIN(wSoftBin);            // SOFT_BIN
            // 200mm wafers, usually have 5 sites, 300mm wafers usually have 9 sites
            switch(iSiteNumber)
            {
                case 1:	// Center
                    lPRRV4Record.SetX_COORD(1);			// X_COORD
                    lPRRV4Record.SetY_COORD(1);			// Y_COORD
                    break;
                case 2:	// Down
                    lPRRV4Record.SetX_COORD(1);			// X_COORD
                    lPRRV4Record.SetY_COORD(2);			// Y_COORD
                    break;
                case 3:	// Left
                    lPRRV4Record.SetX_COORD(0);			// X_COORD
                    lPRRV4Record.SetY_COORD(1);			// Y_COORD
                    break;
                case 4:	// Top
                    lPRRV4Record.SetX_COORD(1);			// X_COORD
                    lPRRV4Record.SetY_COORD(0);			// Y_COORD
                    break;
                case 5:	// Right
                    lPRRV4Record.SetX_COORD(2);			// X_COORD
                    lPRRV4Record.SetY_COORD(1);			// Y_COORD
                    break;
                case 6:	// Lower-Right corner
                    lPRRV4Record.SetX_COORD(2);			// X_COORD
                    lPRRV4Record.SetY_COORD(2);			// Y_COORD
                    break;
                case 7:	// Lower-Left corner
                    lPRRV4Record.SetX_COORD(0);			// X_COORD
                    lPRRV4Record.SetY_COORD(2);			// Y_COORD
                    break;
                case 8:	// toUpper-Left corner
                    lPRRV4Record.SetX_COORD(0);			// X_COORD
                    lPRRV4Record.SetY_COORD(0);			// Y_COORD
                    break;
                case 9:	// toUpper-Right corner
                    lPRRV4Record.SetX_COORD(2);			// X_COORD
                    lPRRV4Record.SetY_COORD(0);			// Y_COORD
                    break;
                default: // More than 9 sites?....give 0,0 coordonates
                    lPRRV4Record.SetX_COORD(0);			// X_COORD
                    lPRRV4Record.SetY_COORD(0);			// Y_COORD
                    break;
            }
            lPRRV4Record.SetTEST_T(0);				// No testing time known...
            sprintf(szString,"%ld",iPartNumber);
            lPRRV4Record.SetPART_ID(szString);		// PART_ID
            lPRRV4Record.SetPART_TXT("");			// PART_TXT
            lPRRV4Record.SetPART_FIX();             // PART_FIX
            mStdfParse.WriteRecord(&lPRRV4Record);
            lPRRV4Record.Reset();
        }

        // Write WRR
        GQTL_STDF::Stdf_WRR_V4 lWRRV4Record;
        SetWRRRecord(lWRRV4Record, ptWafers->mWaferID, iTotalGoodBin+iTotalFailBin , iTotalGoodBin) ;
        mStdfParse.WriteRecord(&lWRRV4Record);
        lWRRV4Record.Reset();

#if 0
        // NO SUMMARY FOR ETEST
        // Write SBR Bin0 (FAIL)
        WriteSBR(0,iTotalFailBin, false);

        // Write SBR Bin1 (PASS)
        WriteSBR(1,iTotalGoodBin, true);

        // Write HBR Bin0 (FAIL)
        WriteHBR(0, iTotalFailBin, false);

        // Write HBR Bin1 (PASS)
        WriteHBR(1,iTotalGoodBin, true);

#endif
        // Not the WaferID we need...see next one.
        itWafer++;
    };

    // Write MRR
    GQTL_STDF::Stdf_MRR_V4 lMRRV4Record;
    lMRRV4Record.SetFINISH_T(mStartTime);			// File finish-time.
    mStdfParse.WriteRecord(&lMRRV4Record);

    // Close STDF file.
    mStdfParse.Close();

    // Success
    return true;
}

void WatToStdf::WriteHBR(int binNumber, int totalBins, bool isPassed, int testHead, int testSite )
{
    mHBRV4Record.SetHEAD_NUM(testHead);			    // Test Head = ALL
    mHBRV4Record.SetSITE_NUM(testSite);					    // Test sites = ALL
    mHBRV4Record.SetHBIN_NUM(binNumber);			// SBIN = 0
    mHBRV4Record.SetHBIN_CNT(totalBins);			// Total Bins
    if(isPassed)
        mHBRV4Record.SetHBIN_PF('P');
    else
        mHBRV4Record.SetHBIN_PF('F');
    mStdfParse.WriteRecord(&mHBRV4Record);
    mHBRV4Record.Reset();
}

void WatToStdf::WriteSBR(int binNumber, int totalBins, bool isPassed, int testHead, int testSite )
{
    mSBRV4Record.SetHEAD_NUM(testHead);			    // Test Head = ALL
    mSBRV4Record.SetSITE_NUM(testSite);					    // Test sites = ALL
    mSBRV4Record.SetSBIN_NUM(binNumber);			// SBIN = 0
    mSBRV4Record.SetSBIN_CNT(totalBins);			// Total Bins
    if(isPassed)
        mSBRV4Record.SetSBIN_PF('P');
    else
        mSBRV4Record.SetSBIN_PF('F');
    mStdfParse.WriteRecord(&mSBRV4Record);
    mSBRV4Record.Reset();
}

}
}
