//////////////////////////////////////////////////////////////////////
// import_CSMC_SPDM.cpp: Convert a CSMC_SPDM (Process control Monitor).csv
// file to STDF V4.0
//////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#include "windows.h"		// For 'GetWindowsDirectoryA' declaration
#endif

#include <qfileinfo.h>
#include <qregexp.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlabel.h>

#include "engine.h"
#include "import_csmc_spdm.h"
#include "time.h"
#include "import_constants.h"

//NEW FORMATED WAFER SUMMARY: 0.9SPDM PROCESS FOR VERSION_1.0
//CODE: _BE248 LOT: AE57081
//TESTED 06/19/13 ON SYSTEM# AG40721 BY SZY
//
//WF# S# N+ 0.5*408 Rc       P+ 0.5*408 Rc       P1 0.5*520 RC       M1 800/0.6 RES      M2 800/0.7 RES      VIA .55*520 Rc      BVR GOX/NW@1uA      BVR GOX/PW@1uA      BVR HGOX/NW@1uA     BVR HGOX/PW@1uA     N+ 100/10 RES       P+ 100/10 RES       NW_F 100/10 RES     PP1 100/10 RES      P1 COMB 1/0.8       M1 COMB             M2 COMB             N 1x20 Vt           N 1x20 Ioff         N 1x20 Ion          N 1x20 BVd/sgt      NFOX 2.0x20 VTF     P 1x20 Vt           P 1x20 Ioff         P 1x20 Ion          P 1x20 BVd/sgt      PFOX 2.0x20 VTF     HNA 3x20 Vt         HNA 3x20 Ion        HNA 3x20 BVd/sgt    HPA 3x20 Vt         HPA 3x20 Ion        HPA 3x20 BVd/sgt    HNST 3x20 Vt        HNST 3x20 Ion       HNST 3x20 BVd/sgt   HNS 3x20 Vt         HNS 3x20 Ion        HNS 3x20 BVd/sgt    HPS 3x20 Vt         HPS 3x20 Ion        HPS 3x20 BVd/sgt    HNI 3x20 Vt         HNI 3x20 Ion        HNI 3x20 BVd/sgt    ZC 2.5/1.8 BV@10uA  HNAT 3x20 Vt        HNAT 3x20 Ion       HNAT 3x20 BVd/sgt   HPAT 3x20 Vt        HPAT 3x20 Ion       HPAT 3x20 BVd/sgt   HPST 3x20 Vt        HPST 3x20 Ion       HPST 3x20 BVd/sgt   HNIT 3x20 Vt        HNIT 3x20 Ion       HNIT 3x20 BVd/sgt   HiP1M 80/10         ZC 2.5/1.8 BV@1mA   NP1 100/10 RES      P1 100/10 RES       NWBN/PSUB BV        VN 5X5 HFE          VN 5X5 BVCEO        VN 5X5 Vbe          HNA 3x20 BVs/dgt
//Unit   ohm/CNT             ohm/CNT             ohm/CNT             ohm/sq              ohm/sq              ohm/CNT             VOLT                VOLT                VOLT                VOLT                ohm/sq              ohm/sq              ohm/sq              ohm/sq              #NAME?              #NAME?              #NAME?              VOLT                #NAME?              mA                  VOLT                VOLT                VOLT                #NAME?              mA                  VOLT                VOLT                VOLT                mA                  VOLT                VOLT                mA                  VOLT                VOLT                mA                  VOLT                VOLT                mA                  VOLT                VOLT                mA                  VOLT                VOLT                mA                  VOLT                VOLT                VOLT                mA                  VOLT                VOLT                mA                  VOLT                VOLT                mA                  VOLT                VOLT                mA                  VOLT                ohm/sq              VOLT                ohm/sq              ohm/sq              VOLT                GAIN                VOLT                VOLT                VOLT
//01  1          23.654              75.385              25.577               .0659               .0446              1.9231             20.2000            -21.6000             80.0000            -80.0000             37.9000             55.2000                1850             38.4000             10.4588             10.4630             10.5948               .7615             10.5384              6.3550             11.8000             15.9370              -.9463             10.4835             -3.2920             11.9000            -11.0930              2.7408              9.9865             55.4000             -2.1110             -4.8580            -56.2000               .6409              1.7940             55.6000              2.1139              8.6775             56.2000             -1.2461             -3.6865            -56.2000              2.7465             11.6250             44.4000              5.6400               .7102              2.8710             55.6000              -.8267             -1.1520            -49.8000              -.5299              -.6560            -49.6000               .6726              3.3010             31.6000                1125              5.7330             30.9000             27.5000             48.2000             221.350             12.0340               .6059             12.2000
//01  2          23.077              70.385              25.000               .0684               .0466              1.7308             20.4000            -21.8000             80.0000            -80.0000             37.8000             54.8000                1870             38.4000             10.5654             10.5502             10.6240               .7517             10.5620              6.4115             11.8000             15.9370              -.9368             10.5952             -3.4175             11.9000            -11.0150              2.7072             10.0400             56.2000             -2.1671             -4.7250            -56.8000               .6396              1.8040             56.0000              2.0682              8.6735             56.4000             -1.2611             -3.6850            -56.2000              2.7186             11.6350             44.4000              5.6530               .6939              2.9060             55.8000              -.8272             -1.1380            -51.4000              -.5325              -.6334            -50.2000               .6635              3.3445             32.4000                1113              5.7520             30.9000             27.6000             48.6000             223.400             11.9790               .6062             12.2000
// .........................................................
// .........................................................
//Max            25.385              82.500              27.308               .0692               .0478              2.1154             20.4000            -21.6000             80.0000            -80.0000             38.2000             55.7000                1900             39.5000             10.6414             10.5502             10.7126               .7781             10.5837              6.4440             11.9000             16.5620              -.9241             10.6340             -3.2400             12.0000            -10.8590              2.8152             10.0750             56.4000             -2.0469             -4.5790            -55.6000               .6569              1.8265             56.6000              2.1498              8.7035             56.8000             -1.2175             -3.5965            -55.2000              2.8118             11.7300             45.0000              5.6940               .7163              2.9240             56.4000              -.8201             -1.1145            -48.6000              -.5033              -.6263            -48.2000               .6911              3.3760             36.4000                1150              5.7870             31.7000             28.4000             48.6000             232.250             12.3740               .6071             12.5000
//Min            22.692              67.308              24.231               .0610               .0381              1.5385              5.0000            -21.8000             80.0000            -80.0000             37.5000             54.6000                1790             37.6000             10.2294             10.2692             10.3183               .7351              9.6247              6.2540             11.5000             15.4680              -.9690              9.6704             -3.4225             11.9000            -11.4060              2.6975              9.8475             54.2000             -2.2249             -4.8915            -58.0000               .6115              1.7625             54.4000              2.0264              8.4865             54.2000             -1.4067             -3.7560            -57.2000              2.6952             11.3750             43.6000              5.5810               .6794              2.8555             54.4000              -.8582             -1.1635            -51.4000              -.5647              -.6761            -50.8000               .6494              3.2595             31.6000                1100              5.6940             30.3000             27.1000             47.6000             211.900             11.7990               .6050             12.1000
//Ave            24.011              76.083              25.954               .0658               .0419              1.9077             20.0912            -21.6400             80.0000            -80.0000             37.8952             55.2432                1855             38.4880             10.3923             10.4070             10.4972               .7576             10.4466              6.3485             11.8064             16.0684              -.9416             10.3769             -3.3272             11.9008            -11.1320              2.7606              9.9561             55.2624             -2.1445             -4.7735            -56.5968               .6358              1.7925             55.3616              2.1055              8.5885             55.4224             -1.3035             -3.6860            -56.2480              2.7561             11.5159             44.1904              5.6413               .6995              2.8847             55.2688              -.8355             -1.1396            -50.0544              -.5360              -.6518            -49.6976               .6675              3.3119             33.5264                1127              5.7454             31.0400             27.7000             48.1424             220.827             12.1064               .6061             12.2776
//Std              .621               3.516                .839               .0016               .0028               .1373              1.3620               .0803              0.0000              0.0000               .1645               .2503                  22               .3907               .0755               .0621               .0705               .0082               .1008               .0417               .0716               .2105               .0080               .1168               .0393               .0089               .1092               .0278               .0447               .5227               .0360               .0717               .5255               .0118               .0152               .5072               .0232               .0418               .7054               .0373               .0362               .4875               .0276               .0708               .2312               .0207               .0075               .0170               .5175               .0079               .0116               .6751               .0118               .0131               .5007               .0079               .0237              1.3072                   9               .0174               .3220               .2745               .1811               3.867               .1167               .0004               .0831
//Hsl           100.000             180.000              60.000               .2000               .2000             10.0000             30.0000            -15.0000             90.0000            -70.0000             60.0000             80.0000                2300             60.0000             13.0000             13.0000             13.0000               .9000             13.0000              7.3000             16.0000             24.0000              -.8000             13.0000             -2.5000             15.0000            -10.0000              3.2500             11.6000             65.0000             -1.8000              3.6000            -46.0000               .8000              2.3000             65.0000              2.7000             10.0000             65.0000             -1.0500             -2.8000            -46.0000              3.2500             13.0000             60.0000              6.2000               .9000              3.4000             65.0000              -.6800              -.9000            -46.0000              -.3500              -.5000            -46.0000               .9000              3.8000             50.0000                1250              6.4000             38.0000             34.0000             65.0000             270.000             25.0000               .6400             20.0000
//Lsl             0.000               0.000               0.000              0.0000              0.0000              0.0000             15.0000            -30.0000             70.0000            -90.0000             20.0000             40.0000                1400             20.0000              9.0000              9.0000              9.0000               .6000              9.0000              5.3000              9.0000             10.0000             -1.1000              9.0000             -4.3000              9.0000            -20.0000              2.3500              7.6000             46.0000             -2.8000             -5.8000            -70.0000               .5000              1.5000             46.0000              1.7000              7.0000             46.0000             -1.7500             -4.6000            -70.0000              2.3500              9.0000             40.0000              5.4000               .6000              2.4000             46.0000              -.9800             -1.3000            -70.0000              -.6300              -.8000            -70.0000               .6000              2.8000             32.0000                 910              5.6000             26.0000             22.0000             42.0000             170.000              9.0000               .5600              9.0000
//  *** INDICATES THIS PARAMETER WAS OUT OF SPEC LIMIT
//   -- INDICATES THIS PARAMETER WAS OUT OF VALID LIMIT


// main.cpp
extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar	*	GexProgressBar;		// Handle to progress bar in status bar



//////////////////////////////////////////////////////////////////////
// Check if File is compatible with NEW FORMATED WAFER SUMMARY: SPDM format
//////////////////////////////////////////////////////////////////////
bool CGCSMC_SPDMtoSTDF::IsCompatible(const char *lFileName)
{
    QString lStrString;

    // Open hCsmFile file
    QFile lFile( lFileName );
    if(!lFile.open( QIODevice::ReadOnly ))
    {
        // Failed Opening ASL1000 file
        return false;
    }
    // Assign file I/O stream
    QTextStream lSpdmFile(&lFile);

    // Check if first line is the correct SPDM header...
    //NEW FORMATED WAFER SUMMARY: 0.9SPDM PROCESS FOR VERSION_1.0
    //CODE: _BE248 LOT: AE57081
    do
        lStrString = lSpdmFile.readLine();
    while(!lStrString.isNull() && lStrString.isEmpty());

    lFile.close();

    lStrString = lStrString.trimmed();	// remove leading spaces.

    // extract Lot Number
    if( lStrString.startsWith("NEW FORMATED WAFER SUMMARY", Qt::CaseInsensitive) == false
        || (!lStrString.contains("SPDM") && !lStrString.contains("DPDM")) )
    {
        // Incorrect header...this is not a SCMC_SPDM file!
        return false;
    }

    do
        lStrString = lSpdmFile.readLine();
    while(!lStrString.isNull() && lStrString.isEmpty());

    // CODE and LOT
    if(!lStrString.contains("CODE") || !lStrString.contains("LOT"))
    {
        // Incorrect header...this is not a SCMC_SPDM file!
        return false;
    }

    // read at max 10 lines to find the WF# and S#
    bool lCorrectHeader(false);
    for (int i=0; i<10; ++i)
    {
        lStrString = lSpdmFile.readLine();
        if (lStrString.contains("WF#") && lStrString.contains("S#"))
        {
            lCorrectHeader = true;
            break;
        }
    }
    if(lCorrectHeader == false)
    {
        // Incorrect header...this is not a SCMC_SPDM file!
        return false;
    }

    return true;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGCSMC_SPDMtoSTDF::CGCSMC_SPDMtoSTDF()
{
    // Default: CSMC_SPDM parameter list on disk includes all known CSMC_SPDM parameters...
    m_bNewCsmcPmdParameterFound = false;
    m_lStartTime = 0;

    m_pCGPcmSPDMParameter = NULL;
    mCsmcType = "SPDM";
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
CGCSMC_SPDMtoSTDF::~CGCSMC_SPDMtoSTDF()
{
    // Destroy list of Parameters tables.
    if(m_pCGPcmSPDMParameter!=NULL)
        delete [] m_pCGPcmSPDMParameter;
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGCSMC_SPDMtoSTDF::GetLastError()
{
    strLastError = "Import CSMC_" + mCsmcType + ": ";

    switch(iLastError)
    {
        default:
        case errNoError:
            strLastError += "No Error";
            break;
        case errOpenFail:
            strLastError += "Failed to open file";
            break;
        case errInvalidFormat:
            strLastError += "Invalid file format";
            break;
        case errInvalidFormatLowInRows:
            strLastError += "Invalid file format: 'Parameter' line too short, missing rows";
            break;
        case errNoLimitsFound:
            strLastError += "Invalid file format: Specification Limits not found";
            break;
        case errWriteSTDF:
            strLastError += "Failed creating temporary file. Folder permission issue?";
            break;
        case errLicenceExpired:
            strLastError += "License has expired or Data file out of date...";
            break;
    }
    // Return Error Message
    return strLastError;
}

//////////////////////////////////////////////////////////////////////
// Load CSMC_SPDM Parameter table from DISK
//////////////////////////////////////////////////////////////////////
void CGCSMC_SPDMtoSTDF::LoadParameterIndexTable(void)
{
    QString	lCsmcSpdmTableFile;
    QString	lStrString;

    lCsmcSpdmTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
    lCsmcSpdmTableFile += GEX_CSMC_PARAMETERS;

    // Open CSMC_SPDM Parameter table file
    QFile lFile( lCsmcSpdmTableFile );
    if(!lFile.open( QIODevice::ReadOnly ))
        return;

    // Assign file I/O stream
    QTextStream hCsmcSpdmTableFile(&lFile);

    // Skip comment lines
    do
    {
      lStrString = hCsmcSpdmTableFile.readLine();
    }
    while((lStrString.indexOf("----------------------") < 0) && (hCsmcSpdmTableFile.atEnd() == false));

    // Read lines
    m_pFullCsmcSpdmParametersList.clear();
    lStrString = hCsmcSpdmTableFile.readLine();
    while (lStrString.isNull() == false)
    {
        // Save Parameter name in list
        m_pFullCsmcSpdmParametersList.append(lStrString);
        // Read next line
        lStrString = hCsmcSpdmTableFile.readLine();
    };

    // Close file
    lFile.close();
}

//////////////////////////////////////////////////////////////////////
// Save CSMC_SPDM Parameter table to DISK
//////////////////////////////////////////////////////////////////////
void CGCSMC_SPDMtoSTDF::DumpParameterIndexTable(void)
{
    QString		strCsmcSpdmTableFile;

    strCsmcSpdmTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
    strCsmcSpdmTableFile += GEX_CSMC_PARAMETERS;

    // Open CSMC_SPDM Parameter table file
    QFile lFile( strCsmcSpdmTableFile );
    if(!lFile.open( QIODevice::WriteOnly ))
        return;

    // Assign file I/O stream
    QTextStream hCsmcSpdmTableFile(&lFile);

    // First few lines are comments:
    hCsmcSpdmTableFile << "############################################################" << endl;
    hCsmcSpdmTableFile << "# DO NOT EDIT THIS FILE!" << endl;
    hCsmcSpdmTableFile << "# Quantix Examinator: CSMC_"<< mCsmcType << " Parameters detected" << endl;
    hCsmcSpdmTableFile << "# www.mentor.com" << endl;
    hCsmcSpdmTableFile << "# Quantix Examinator reads and writes into this file..." << endl;
    hCsmcSpdmTableFile << "-----------------------------------------------------------" << endl;

    // Write lines
    // m_pFullCsmcSpdmParametersList.sort();
    for (QStringList::const_iterator
         iter  = m_pFullCsmcSpdmParametersList.begin();
         iter != m_pFullCsmcSpdmParametersList.end(); ++iter) {
        // Write line
        hCsmcSpdmTableFile << *iter << endl;
    }

    // Close file
    lFile.close();
}

//////////////////////////////////////////////////////////////////////
// If Examinator doesn't have this CSMC_SPDM parameter in his dictionnary, have it added.
//////////////////////////////////////////////////////////////////////
void CGCSMC_SPDMtoSTDF::UpdateParameterIndexTable(QString strParamName)
{
    // Check if the table is empty...if so, load it from disk first!
    if(m_pFullCsmcSpdmParametersList.isEmpty() == true)
    {
        // Load CSMC_SPDM parameter table from disk...
        LoadParameterIndexTable();
    }

    // Check if Parameter name already in table...if not, add it to the list
    // the new full list will be dumped to the disk at the end.
    if(m_pFullCsmcSpdmParametersList.indexOf(strParamName) < 0)
    {
        // Update list
        m_pFullCsmcSpdmParametersList.append(strParamName);

        // Set flag to force the current CSMC_SPDM table to be updated on disk
        m_bNewCsmcPmdParameterFound = true;
    }
}


//////////////////////////////////////////////////////////////////////
// Read and Parse the CSMC_SPDM file
//////////////////////////////////////////////////////////////////////
bool CGCSMC_SPDMtoSTDF::ReadCsmcSpdmFile(const char *CsmcSpdmFileName,const char *strFileNameSTDF)
{
    QString lStrString;
    QString strSection;
    bool	bStatus;
    int		iIndex;				// Loop index

    // Open CSV file
    QFile f( CsmcSpdmFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening CSMC_SPDM file
        iLastError = errOpenFail;

        // Convertion failed.
        return false;
    }

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    iNextFilePos = 0;
    iProgressStep = 0;
    iFileSize = f.size() + 1;

    // We have to read the file twice because the spec limits are in the end of file
    /// first pass
    // Assign file I/O stream
    QTextStream hCsmcSpdmFile(&f);

    // Check if first line is the correct CSMC_SPDM header...
    //NEW FORMATED WAFER SUMMARY: 0.9SPDM PROCESS FOR VERSION_1.0
    //CODE: _BE248 LOT: AE57081
    //TESTED 06/19/13 ON SYSTEM# AG40721 BY SZY
    while (!lStrString.contains("NEW FORMATED WAFER SUMMARY"))
        lStrString = ReadLine(hCsmcSpdmFile);
    if (lStrString.contains("SPDM"))
        mCsmcType = QString("SPDM");
    else
        mCsmcType = QString("DPDM");
    lStrString = ReadLine(hCsmcSpdmFile);
    lStrString = lStrString.trimmed();	// remove leading spaces.

    // Extract Program Name
    if(lStrString.startsWith("CODE", Qt::CaseInsensitive) == false)
    {
        // Incorrect header...this is not a CSMC_SPDM file!
        iLastError = errInvalidFormat;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }
    lStrString = lStrString.section(":",1).trimmed();
    m_strProgramID = lStrString.section(' ',0,0).trimmed();

    // Extract LOT number
    lStrString = lStrString.section("LOT",1);
    m_strLotID = lStrString.section(":", 1).trimmed();

    lStrString = ReadLine(hCsmcSpdmFile);
    // Upload Time
    if(lStrString.startsWith("TESTED", Qt::CaseInsensitive) == false)
    {
        // Incorrect header...this is not a CSMC_SPDM file!
        iLastError = errInvalidFormat;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    QString strDate  = lStrString.mid(7, 8);

    // Date format: MM/dd/yy
    QDate lDate = QDate::fromString(strDate, "MM/dd/yy");
    lDate = lDate.addYears(100); // because the date begin with 1900 in qt
    QDateTime lDateTime(lDate);
    lDateTime.setTimeSpec(Qt::UTC);
    m_lStartTime = lDateTime.toTime_t();

    // Read test names
    //WF# S# N+ 0.5*408 Rc       P+ 0.5*408 Rc       P1 0.5*520 RC       M1 800/0.6 RES      M2 800/0.7 RES      VIA .55*520 Rc      BVR GOX/NW@1uA      BVR GOX/PW@1uA      BVR HGOX/NW@1uA     BVR HGOX/PW@1uA     N+ 100/10 RES       P+ 100/10 RES       NW_F 100/10 RES     PP1 100/10 RES      P1 COMB 1/0.8       M1 COMB             M2 COMB             N 1x20 Vt           N 1x20 Ioff         N 1x20 Ion          N 1x20 BVd/sgt      NFOX 2.0x20 VTF     P 1x20 Vt           P 1x20 Ioff         P 1x20 Ion          P 1x20 BVd/sgt      PFOX 2.0x20 VTF     HNA 3x20 Vt         HNA 3x20 Ion        HNA 3x20 BVd/sgt    HPA 3x20 Vt         HPA 3x20 Ion        HPA 3x20 BVd/sgt    HNST 3x20 Vt        HNST 3x20 Ion       HNST 3x20 BVd/sgt   HNS 3x20 Vt         HNS 3x20 Ion        HNS 3x20 BVd/sgt    HPS 3x20 Vt         HPS 3x20 Ion        HPS 3x20 BVd/sgt    HNI 3x20 Vt         HNI 3x20 Ion        HNI 3x20 BVd/sgt    ZC 2.5/1.8 BV@10uA  HNAT 3x20 Vt        HNAT 3x20 Ion       HNAT 3x20 BVd/sgt   HPAT 3x20 Vt        HPAT 3x20 Ion       HPAT 3x20 BVd/sgt   HPST 3x20 Vt        HPST 3x20 Ion       HPST 3x20 BVd/sgt   HNIT 3x20 Vt        HNIT 3x20 Ion       HNIT 3x20 BVd/sgt   HiP1M 80/10         ZC 2.5/1.8 BV@1mA   NP1 100/10 RES      P1 100/10 RES       NWBN/PSUB BV        VN 5X5 HFE          VN 5X5 BVCEO        VN 5X5 Vbe          HNA 3x20 BVs/dgt
    while (!lStrString.contains("WF#"))
        lStrString = ReadLine(hCsmcSpdmFile);

    // read the wafer number and site number
    QStringList lstSections;
    lstSections.append(lStrString.mid(0, 3).trimmed());
    lstSections.append(lStrString.mid(4, 2).trimmed());
    lStrString = lStrString.mid(8).trimmed();
    do
    {
        lstSections.append(lStrString.mid(0, 18).trimmed());
        lStrString = lStrString.mid(19).trimmed();
    }while(!lStrString.isEmpty());

    // Count the number of parameters specified in the line
    // Do not count first 2 fields
    m_iTotalParameters=lstSections.count() - 3;
    // If no parameter specified...ignore!
    if(m_iTotalParameters <= 0)
    {
        // Incorrect header...this is not a valid CSMC_SPDM file!
        iLastError = errInvalidFormat;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }


    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    iNextFilePos = 0;
    iProgressStep = 0;
    iFileSize = f.size() + 1;

    // Allocate the buffer to hold the N parameters & results.
    m_pCGPcmSPDMParameter = new CGPcmSPDMParameter[m_iTotalParameters+1];	// List of parameters

    // Extract the N column names
    for(iIndex=0;iIndex<=m_iTotalParameters;iIndex++)
    {
        strSection = lstSections[iIndex+2].trimmed();	// Remove spaces
        m_pCGPcmSPDMParameter[iIndex].strName = strSection;
        UpdateParameterIndexTable(strSection);		// Update Parameter master list if needed.
        m_pCGPcmSPDMParameter[iIndex].bStaticHeaderWritten = false;
    }

    // Read units
    while (!lStrString.startsWith("Unit", Qt::CaseInsensitive))
    {
        lStrString = ReadLine(hCsmcSpdmFile).trimmed();
    }

    lstSections.clear();
    lStrString = lStrString.mid(7).trimmed();
    do
    {
        lstSections.append(lStrString.mid(0, 8).trimmed());
        lStrString = lStrString.mid(9).trimmed();
    }while(!lStrString.isEmpty());


    // Extract the N column units
    for(iIndex=0;iIndex<=m_iTotalParameters;iIndex++)
    {
        strSection = lstSections[iIndex].trimmed();	// Remove spaces
        m_pCGPcmSPDMParameter[iIndex].strUnits = strSection;
    }


    // Read HighLimit
    int	lHighLimits =0;
    while (!lStrString.startsWith("Hsl", Qt::CaseInsensitive)
           && !hCsmcSpdmFile.atEnd())
    {
        lStrString = ReadLine(hCsmcSpdmFile).trimmed();
    }

    if (hCsmcSpdmFile.atEnd())
    {
        for(iIndex=0;iIndex<=m_iTotalParameters;iIndex++)
        {
            m_pCGPcmSPDMParameter[iIndex].bValidHighLimit = false;
            m_pCGPcmSPDMParameter[iIndex].bValidLowLimit = false;
        }
    }
    else
    {

        // found the HIGH limits
        lHighLimits |= 1;
        lstSections.clear();
        lStrString = lStrString.mid(7).trimmed();
        do
        {
            lstSections.append(lStrString.mid(0, 8).trimmed());
            lStrString = lStrString.mid(9).trimmed();
        }while(!lStrString.isEmpty());

        // Check if have the good count
        if(lstSections.count() < m_iTotalParameters+1)
        {
            iLastError = errInvalidFormatLowInRows;

            // Convertion failed.
            // Close file
            f.close();
            return false;
        }

        // Extract the N column Upper Limits
        for(iIndex=0;iIndex<=m_iTotalParameters;iIndex++)
        {
            strSection = lstSections[iIndex].trimmed();
            m_pCGPcmSPDMParameter[iIndex].fHighLimit = strSection.toFloat(&bStatus);
            m_pCGPcmSPDMParameter[iIndex].bValidHighLimit = bStatus;
        }

        // Read LowLimit
        while (!lStrString.startsWith("Lsl", Qt::CaseInsensitive)
               && !hCsmcSpdmFile.atEnd())
        {
            lStrString = ReadLine(hCsmcSpdmFile).trimmed();
        }

        if (hCsmcSpdmFile.atEnd())
        {
            for(iIndex=0;iIndex<=m_iTotalParameters;iIndex++)
            {
                m_pCGPcmSPDMParameter[iIndex].bValidLowLimit = false;
            }
            return true;
        }

        // found the Low limits
        lHighLimits |= 2;
        lstSections.clear();
        lStrString = lStrString.mid(7).trimmed();
        do
        {
            lstSections.append(lStrString.mid(0, 8).trimmed());
            lStrString = lStrString.mid(9).trimmed();
        }while(!lStrString.isEmpty());

        // Check if have the good count
        if(lstSections.count() < m_iTotalParameters+1)
        {
            iLastError = errInvalidFormatLowInRows;

            // Convertion failed.
            // Close file
            f.close();
            return false;
        }

        // Extract the N column Upper Limits
        for(iIndex=0;iIndex<=m_iTotalParameters;iIndex++)
        {
            strSection = lstSections[iIndex].trimmed();
            m_pCGPcmSPDMParameter[iIndex].fLowLimit = strSection.toFloat(&bStatus);
            m_pCGPcmSPDMParameter[iIndex].bValidLowLimit = bStatus;
        }
    }

    // second pass
    // Close file
    f.close();
    f.open(QIODevice::ReadOnly);
    QTextStream lCsmcSpdmFile(&f);
    // Loop reading file until end is reached & generate STDF file dynamically.
    bStatus = WriteStdfFile(&lCsmcSpdmFile,strFileNameSTDF);
    if(!bStatus)
        QFile::remove(strFileNameSTDF);

    // Close file
    f.close();

    // All CSMC_SPDM file read...check if need to update the CSMC_SPDM Parameter list on disk?
    if(bStatus && (m_bNewCsmcPmdParameterFound == true))
        DumpParameterIndexTable();

    // Success parsing CSMC_SPDM file
    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from CSMC_SPDM data parsed
//////////////////////////////////////////////////////////////////////
bool CGCSMC_SPDMtoSTDF::WriteStdfFile(QTextStream *lCsmcSpdmFile, const char *strFileNameSTDF)
{
    // now generate the STDF file...
    GS::StdLib::Stdf StdfFile;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    if(StdfFile.Open((char*)strFileNameSTDF,STDF_WRITE) != GS::StdLib::Stdf::NoError)
    {
        // Failed importing CSV file into STDF database
        iLastError = errWriteSTDF;

        // Convertion failed.
        return false;
    }

    // Write FAR
    RecordReadInfo.iRecordType = 0;
    RecordReadInfo.iRecordSubType = 10;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(1);					// SUN CPU type
    StdfFile.WriteByte(4);					// STDF V4
    StdfFile.WriteRecord();

    if(m_lStartTime <= 0)
        m_lStartTime = QDateTime::currentDateTime().toTime_t();

    // Write MIR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 10;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteDword(m_lStartTime);			// Setup time
    StdfFile.WriteDword(m_lStartTime);			// Start time
    StdfFile.WriteByte(1);						// Station
    StdfFile.WriteByte((BYTE) 'P');				// Test Mode = PRODUCTION
    StdfFile.WriteByte((BYTE) ' ');				// rtst_cod
    StdfFile.WriteByte((BYTE) ' ');				// prot_cod
    StdfFile.WriteWord(65535);					// burn_tim
    StdfFile.WriteByte((BYTE) ' ');				// cmod_cod
    StdfFile.WriteString(m_strLotID.toLatin1().constData());		// Lot ID
    StdfFile.WriteString(m_strProductID.toLatin1().constData());	// Part Type / Product ID
    StdfFile.WriteString("");					// Node name
    StdfFile.WriteString("");					// Tester Type
    StdfFile.WriteString(m_strProgramID.toLatin1().constData());	// Job name
    StdfFile.WriteString("");					// Job rev
    StdfFile.WriteString("");					// sublot-id
    StdfFile.WriteString("");					// operator
    StdfFile.WriteString("");					// exec-type
    StdfFile.WriteString("");					// exe-ver
    StdfFile.WriteString("WAFER");				// test-cod
    StdfFile.WriteString("");					// test-temperature
    // Construct custom Galaxy USER_TXT
    QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    strUserTxt += GEX_IMPORT_DATAORIGIN_ETEST;
    strUserTxt += ":CSMC_" + mCsmcType;
    StdfFile.WriteString(strUserTxt.toLatin1().constData());	// user-txt
    StdfFile.WriteString("");							// aux-file
    StdfFile.WriteString("");							// package-type
    StdfFile.WriteString(m_strProductID.toLatin1().constData());	// familyID
    StdfFile.WriteString("");							// Date-code
    StdfFile.WriteString("");							// Facility-ID
    StdfFile.WriteString("");							// FloorID
    StdfFile.WriteString("");	// ProcessID	StdfFile.WriteRecord();
    StdfFile.WriteRecord();

    // Write Test results for each line read.
    QString lStrString;
    char	szString[257];
    QString strSection;
    QString	strWaferID;
    float	fValue;				// Used for readng floating point numbers.
    int		iIndex;				// Loop index
    int		iSiteNumber;
    BYTE		bData;
    WORD		wSoftBin,wHardBin;
    long		iTotalGoodBin,iTotalFailBin;
    long		iTestNumber,iTotalTests,iPartNumber;
    bool		bStatus,bPassStatus;
    QStringList	lstSections;
    // Reset counters
    iTotalGoodBin=iTotalFailBin=0;
    iPartNumber=0;

    lStrString = ReadLine(*lCsmcSpdmFile);
    // Read until the unit line
    while (!lCsmcSpdmFile->atEnd()
           && !lStrString.contains("Unit", Qt::CaseInsensitive))
    {
        lStrString = ReadLine(*lCsmcSpdmFile);
    }
    lStrString = ReadLine(*lCsmcSpdmFile);

    // Write all Parameters read on this wafer.: WIR.PIR,PTR....PTR, PRR, PIR, PTR,PTR...PRR,   ... WRR
    while(lCsmcSpdmFile->atEnd() == false
          && !lStrString.startsWith("Max", Qt::CaseInsensitive)
          && !lStrString.startsWith("Mix", Qt::CaseInsensitive)
          && !lStrString.startsWith("Ave", Qt::CaseInsensitive)
          && !lStrString.startsWith("Std", Qt::CaseInsensitive)
          && !lStrString.startsWith("Hsl", Qt::CaseInsensitive)
          && !lStrString.startsWith("Lsl", Qt::CaseInsensitive)
          && !lStrString.contains("INDICATES THIS PARAMETER WAS OUT OF", Qt::CaseInsensitive)
          )
    {

        // Part number
        iPartNumber++;

        lstSections.clear();
        lstSections.append(lStrString.mid(0,3).trimmed());
        lstSections.append(lStrString.mid(4,2).trimmed());
        lStrString = lStrString.mid(7).trimmed();
        do
        {
            lstSections.append(lStrString.mid(0, 14).trimmed());
            lStrString = lStrString.mid(15).trimmed();
        }while(!lStrString.isEmpty());

        // Check if have the good count
        if(lstSections.count() < m_iTotalParameters+1)
        {
            iLastError = errInvalidFormatLowInRows;
            StdfFile.Close();
            // Convertion failed.
            return false;
        }

        // Extract WaferID
        strSection = lstSections[0].trimmed();
        if(strSection != strWaferID)
        {
            // Write WRR in case we have finished to write wafer records.
            if(strWaferID.isEmpty() == false)
            {
                // WRR
                RecordReadInfo.iRecordType = 2;
                RecordReadInfo.iRecordSubType = 20;
                StdfFile.WriteHeader(&RecordReadInfo);
                StdfFile.WriteByte(1);						// Test head
                StdfFile.WriteByte(255);					// Tester site (all)
                StdfFile.WriteDword(m_lStartTime);			// Time of last part tested
                StdfFile.WriteDword(iTotalGoodBin+iTotalFailBin);	// Parts tested: always 5
                StdfFile.WriteDword(0);						// Parts retested
                StdfFile.WriteDword(0);						// Parts Aborted
                StdfFile.WriteDword(iTotalGoodBin);			// Good Parts
                StdfFile.WriteDword((DWORD)-1);				// Functionnal Parts
                StdfFile.WriteString(strWaferID.toLatin1().constData());	// WaferID
                StdfFile.WriteRecord();
            }

            iTotalGoodBin=iTotalFailBin=0;
            // For each wafer, have to write limit in the first PTR
            for(iIndex=0;iIndex<=m_iTotalParameters;iIndex++)
                m_pCGPcmSPDMParameter[iIndex].bStaticHeaderWritten = false;

            // Write WIR of new Wafer.
            strWaferID = strSection;
            RecordReadInfo.iRecordType = 2;
            RecordReadInfo.iRecordSubType = 10;
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteByte(1);								// Test head
            StdfFile.WriteByte(255);							// Tester site (all)
            StdfFile.WriteDword(m_lStartTime);					// Start time
            StdfFile.WriteString(strWaferID.toLatin1().constData());	// WaferID
            StdfFile.WriteRecord();
        }

        // Reset Pass/Fail flag.
        bPassStatus = true;

        // Reset counters
        iTotalTests = 0;

        // Extract Site
        iSiteNumber = lstSections[1].trimmed().toInt();

        // Write PIR for parts in this Wafer site
        RecordReadInfo.iRecordType = 5;
        RecordReadInfo.iRecordSubType = 10;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);								// Test head
        StdfFile.WriteByte(iSiteNumber);					// Tester site
        StdfFile.WriteRecord();

        // Read Parameter results for this record
        for(iIndex=0;iIndex<=m_iTotalParameters;iIndex++)
        {
            // Write the PTR
            iTotalTests++;

            RecordReadInfo.iRecordType = 15;
            RecordReadInfo.iRecordSubType = 10;
            StdfFile.WriteHeader(&RecordReadInfo);

            // Compute Test# (add user-defined offset)
            iTestNumber = (long) m_pFullCsmcSpdmParametersList.indexOf(m_pCGPcmSPDMParameter[iIndex].strName);
            iTestNumber += GEX_TESTNBR_OFFSET_CSMC;     // Test# offset
            StdfFile.WriteDword(iTestNumber);			// Test Number
            StdfFile.WriteByte(1);						// Test head
            StdfFile.WriteByte(iSiteNumber);			// Tester site#

            strSection = lstSections[iIndex+2].trimmed();
            fValue = strSection.toFloat(&bStatus);
            if(bStatus == true) // Valid test result
            {
                if(((m_pCGPcmSPDMParameter[iIndex].bValidLowLimit==true) && (fValue < m_pCGPcmSPDMParameter[iIndex].fLowLimit)) ||
                   ((m_pCGPcmSPDMParameter[iIndex].bValidHighLimit==true) && (fValue > m_pCGPcmSPDMParameter[iIndex].fHighLimit)))
                {
                    bData = 0200;	// Test Failed
                    bPassStatus = false;
                }
                else
                {
                    bData = 0;		// Test passed
                }
                StdfFile.WriteByte(bData);							// TEST_FLG
                StdfFile.WriteByte(0x40|0x80);						// PARAM_FLG
                StdfFile.WriteFloat(fValue);						// Test result
            }
            else    // test fail or invalid
            {
                strSection = strSection.mid(1);
                fValue = strSection.toFloat(&bStatus);
                if (true == bStatus)    // *value
                {
                    bData = 0x80;	// 10000000 : Test Failed and Invalid test result
                    bPassStatus = false;
                    StdfFile.WriteByte(bData);					// TEST_FLG
                    StdfFile.WriteByte(0x40|0x80);				// PARAM_FLG
                    StdfFile.WriteFloat(fValue);						// Test result
                }
                 else    // Invalid test result (*** or ---)
                {
                    bData = 0x86;	// 10000110 : Test Failed and Invalid test result
                    bPassStatus = false;
                    StdfFile.WriteByte(bData);					// TEST_FLG
                    StdfFile.WriteByte(0x40|0x80);				// PARAM_FLG
                    StdfFile.WriteFloat(0);						// Test result
                }
            }

            if(m_pCGPcmSPDMParameter[iIndex].bStaticHeaderWritten == false)
            {
                StdfFile.WriteString(m_pCGPcmSPDMParameter[iIndex].strName.toLatin1().constData());	// TEST_TXT
                StdfFile.WriteString("");							// ALARM_ID
                bData = 2;	// Valid data.
                if(m_pCGPcmSPDMParameter[iIndex].bValidLowLimit==false)
                    bData |=0x40;
                if(m_pCGPcmSPDMParameter[iIndex].bValidHighLimit==false)
                    bData |=0x80;
                StdfFile.WriteByte(bData);							// OPT_FLAG
                StdfFile.WriteByte(0);								// RES_SCALE
                StdfFile.WriteByte(0);								// LLM_SCALE
                StdfFile.WriteByte(0);								// HLM_SCALE
                StdfFile.WriteFloat(m_pCGPcmSPDMParameter[iIndex].fLowLimit);			// LOW Limit
                StdfFile.WriteFloat(m_pCGPcmSPDMParameter[iIndex].fHighLimit);		// HIGH Limit
                StdfFile.WriteString(m_pCGPcmSPDMParameter[iIndex].strUnits.toLatin1().constData());							// Units
                m_pCGPcmSPDMParameter[iIndex].bStaticHeaderWritten = true;
            }
            StdfFile.WriteRecord();
        }		// Read all results on line

        // Write PRR
        RecordReadInfo.iRecordType = 5;
        RecordReadInfo.iRecordSubType = 20;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);			// Test head
        StdfFile.WriteByte(iSiteNumber);// Tester site#:1
        if(bPassStatus == true)
        {
            StdfFile.WriteByte(0);				// PART_FLG : PASSED
            wSoftBin = wHardBin = 1;
            iTotalGoodBin++;
        }
        else
        {
            StdfFile.WriteByte(8);				// PART_FLG : FAILED
            wSoftBin = wHardBin = 0;
            iTotalFailBin++;
        }
        StdfFile.WriteWord((WORD)iTotalTests);	// NUM_TEST
        StdfFile.WriteWord(wHardBin);           // HARD_BIN
        StdfFile.WriteWord(wSoftBin);           // SOFT_BIN
        switch(iSiteNumber)
        {
                case 1:	// Left
                    StdfFile.WriteWord(1);			// X_COORD
                    StdfFile.WriteWord(4);			// Y_COORD
                    break;
                case 2:	// Down
                    StdfFile.WriteWord(5);			// X_COORD
                    StdfFile.WriteWord(6);			// Y_COORD
                    break;
                case 3:	// Center
                    StdfFile.WriteWord(5);			// X_COORD
                    StdfFile.WriteWord(3);			// Y_COORD
                    break;
                case 4:	// Top
                    StdfFile.WriteWord(5);			// X_COORD
                    StdfFile.WriteWord(1);			// Y_COORD
                    break;
                case 5:	// Right
                    StdfFile.WriteWord(8);			// X_COORD
                    StdfFile.WriteWord(4);			// Y_COORD
                    break;
                case 6:	// Upper-Right corner
                    StdfFile.WriteWord(7);			// X_COORD
                    StdfFile.WriteWord(2);			// Y_COORD
                    break;
                case 7:	// Upper-Left corner
                    StdfFile.WriteWord(3);			// X_COORD
                    StdfFile.WriteWord(2);			// Y_COORD
                    break;
                case 8:	// Lower-Left corner
                    StdfFile.WriteWord(3);			// X_COORD
                    StdfFile.WriteWord(5);			// Y_COORD
                    break;
                case 9:	// Lower-Right corner
                    StdfFile.WriteWord(7);			// X_COORD
                    StdfFile.WriteWord(5);			// Y_COORD
                    break;
            default: // More than 5 sites?....give 0,0 coordonates
                StdfFile.WriteWord(0);			// X_COORD
                StdfFile.WriteWord(0);			// Y_COORD
                break;
        }
        StdfFile.WriteDword(0);				// No testing time known...
        sprintf(szString,"%ld",iPartNumber);
        StdfFile.WriteString(szString);		// PART_ID
        StdfFile.WriteString("");			// PART_TXT
        StdfFile.WriteString("");			// PART_FIX
        StdfFile.WriteRecord();

        // Read line
        lStrString = ReadLine(*lCsmcSpdmFile);
    };			// Read all lines with valid data records in file

    // Write WRR for last wafer inserted
    RecordReadInfo.iRecordType = 2;
    RecordReadInfo.iRecordSubType = 20;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(1);						// Test head
    StdfFile.WriteByte(255);					// Tester site (all)
    StdfFile.WriteDword(m_lStartTime);			// Time of last part tested
    StdfFile.WriteDword(iTotalGoodBin+iTotalFailBin);	// Parts tested: always 5
    StdfFile.WriteDword(0);						// Parts retested
    StdfFile.WriteDword(0);						// Parts Aborted
    StdfFile.WriteDword(iTotalGoodBin);			// Good Parts
    StdfFile.WriteDword((DWORD)-1);				// Functionnal Parts
    StdfFile.WriteString(strWaferID.toLatin1().constData());	// WaferID
    StdfFile.WriteRecord();

    // Write MRR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 20;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteDword(m_lStartTime);			// File finish-time.
    StdfFile.WriteRecord();

    // Close STDF file.
    StdfFile.Close();

    // Success
    return true;
}

//////////////////////////////////////////////////////////////////////
// Convert 'FileName' CSMC_SPDM file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGCSMC_SPDMtoSTDF::Convert(const char *CsmcSpdmFileName, const char *strFileNameSTDF)
{
    // No erro (default)
    iLastError = errNoError;

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo fInput(CsmcSpdmFileName);
    QFileInfo fOutput(strFileNameSTDF);

    QFile f( strFileNameSTDF );
    if((f.exists() == true) && (fInput.lastModified() < fOutput.lastModified()))
        return true;

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    bool bHideProgressAfter=true;
    bool bHideLabelAfter=false;
    if(GexProgressBar != NULL)
    {
        bHideProgressAfter = GexProgressBar->isHidden();
        GexProgressBar->setMaximum(100);
        GexProgressBar->setTextVisible(true);
        GexProgressBar->setValue(0);
        GexProgressBar->show();
    }

    if(GexScriptStatusLabel != NULL)
    {
        if(GexScriptStatusLabel->isHidden())
        {
            bHideLabelAfter = true;
            GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(CsmcSpdmFileName).fileName()+"...");
            GexScriptStatusLabel->show();
        }
        GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(CsmcSpdmFileName).fileName()+"...");
        GexScriptStatusLabel->show();
    }
    qApp->QCoreApplication::processEvents();

    if(ReadCsmcSpdmFile(CsmcSpdmFileName,strFileNameSTDF) != true)
    {
        //////////////////////////////////////////////////////////////////////
        // For ProgressBar
        if((GexProgressBar != NULL)
        && bHideProgressAfter)
            GexProgressBar->hide();

        if((GexScriptStatusLabel != NULL)
        && bHideLabelAfter)
            GexScriptStatusLabel->hide();
        return false;	// Error reading CSMC_SPDM file
    }

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    if((GexProgressBar != NULL)
    && bHideProgressAfter)
        GexProgressBar->hide();

    if((GexScriptStatusLabel != NULL)
    && bHideLabelAfter)
        GexScriptStatusLabel->hide();

    // Convertion successful
    return true;
}


//////////////////////////////////////////////////////////////////////
// Read line : skip empty line
//////////////////////////////////////////////////////////////////////
QString CGCSMC_SPDMtoSTDF::ReadLine(QTextStream& hFile)
{
    QString lStrString;

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    if(GexProgressBar != NULL)
    {
        while((int) hFile.device()->pos() > iNextFilePos)
        {
            iProgressStep += 100/iFileSize + 1;
            iNextFilePos  += iFileSize/100 + 1;
            GexProgressBar->setValue(iProgressStep);
        }
    }
    qApp->QCoreApplication::processEvents();

    do
        lStrString = hFile.readLine();
    while(!lStrString.isNull() && lStrString.isEmpty());

    return lStrString;

}
