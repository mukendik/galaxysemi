//////////////////////////////////////////////////////////////////////
// import_spd.cpp: Convert a .SPD / TMT Credence file to STDF V4.0
//////////////////////////////////////////////////////////////////////

#include "gqtl_global.h"
#include <qmath.h>

#ifdef _WIN32
#include "windows.h"		// For 'GetWindowsDirectoryA' declaration
#endif

#include <qfileinfo.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlabel.h>

#include "import_spd.h"
#include "import_constants.h"
#include "engine.h"

// File format:
// Spreadsheet,Format,,,,,,,,,,,,,
// Test, Program:,PW29152K (Default) No Version#,,,,,,,,,,,,,
// Lot ID:,29150AA_024,,,,,,,,,,,,,
// Operator:,operator,,,,,,,,,,,,,
// Computer:,WM-TMT-08,,,,,,,,,,,,,
// Date:,08/23/04 17:02:32,,,,,,,,,,,,,
// ,,1.01.01,1.01.02,1.02.01,1.02.02,1.02.03,1.02.04,1.03.01,1.03.02,1.03.03,1.03.04,1.03.05,1.03.06,1.03.07
// ,,GND-IN,GND-OUT,LSB zener,2nd LSB ,3rd LSB,MSB Zener,PreTrimVo,AllTrimVo,Sim Vo,PostTrmVo,Zap Code       ,Vout @ Vin high,Lineregulation
// ,,short only,short only,To Zener com,To Zener com,To Zener com,To Zener com,,Simulated,Trial Code,+/-.5%,real code,Vin = 26V    ,
// ,,3.5000    ,3.5000    ,-0.2000   ,-0.2000   ,-0.2000   ,-0.2000   ,1.5000    ,1.2512    ,none,1.2512    ,none,1.2574    ,250.0000
// ,,2.5000    ,2.5000    ,-1.0000   ,-1.0000   ,-1.0000   ,-1.0000   ,1.2387    ,0.0000,none,1.2387    ,none,1.2325    ,-250.0000
// Serial#,Bin#, V, V, V, V, V, V, V, V, V, V,        , V   ,m%
// 1,1,3.0036    ,3.0047    ,-0.6890   ,-0.6878   ,-0.6846   ,-0.6816   ,1.2717    ,1.1902    ,1.2478    ,1.2481    ,2.0000    ,1.2503    ,174.3899
// 6,1,3.0036    ,3.0050    ,-0.6882   ,-0.6871   ,-0.6841   ,-0.6809   ,1.2669    ,1.1863    ,1.2433    ,1.2435    ,2.0000    ,1.2460    ,193.2966

// OR for WAFER SORT DATA
//Spreadsheet,Format,,,,,,,,,,,,,,,,,,,,,,,,,,,,
//Test, Program:,ZXLD1350_P09 (Default) No Version#,,,,,,,,,,,,,,,,,,,,,,,,,,,,
//Lot ID:,9H15946_21,,,,,,,,,,,,,,,,,,,,,,,,,,,,
//Operator:,Administrator,,,,,,,,,,,,,,,,,,,,,,,,,,,,
//Computer:,7AS3K04,,,,,,,,,,,,,,,,,,,,,,,,,,,,
//Date:,10/07/06 03:45:11,,,,,,,,,,,,,,,,,,,,,,,,,,,,
//,,1.01.01,1.01.02,1.02.01,1.02.02,1.02.03,1.02.04,1.03.01,1.03.02,1.03.03,1.04.01,1.04.02,1.05.01,1.05.02,1.05.03,1.05.04,1.06.01,1.07.01,1.08.01,1.08.02,1.09.01,1.09.02,1.10.01,1.11.01,1.12.01,1.12.02,1.12.03,1.12.04,1.13.01
//,,Xcoord         ,Ycoord         ,Cont_Lx        ,Cont_Adj       ,Cont_Isense    ,Cont_Vin       ,Vin_th_rise    ,Vin_th_fall    ,Vin_th_hyst    ,Iinqoff_12V    ,Iinqon_12V     ,VTH_12V        ,VTL_12V        ,VT_Mean_12V    ,VT_Hyst_12V    ,Isense_12V     ,Isense_11V8    ,Vadj(nom)_12V  ,Radj_12V       ,Vadjoff_12V    ,Vadjon_12V     ,Ron_Lx_12V     ,Ileak_Lx_12V   ,Dlx_IsH_500_12V,Vlx_IsL_500_12V,Vlx_IsH_16k_12V,Vlx_IsL_16k_12V,FLx_12V
//,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
//,,none,none,0.9800    ,0.9800    ,0.9800    ,0.9800    ,6.3000    ,5.8000    ,1.7000    ,19.6000   ,496.0000  ,123.4000  ,93.8000   ,103.8000  ,38.4000   ,9.4000    ,9.4000    ,1.2821    ,240.0000  ,0.2350    ,0.2850    ,10.0000   ,0.9400    ,48.4000   ,5.0800    ,0.4800    ,5.0800    ,580.0000
//,,none,none,0.2200    ,0.4200    ,0.4200    ,0.3200    ,4.2000    ,3.7000    ,0.3500    ,10.4000   ,104.0000  ,101.6000  ,76.2000   ,96.2000   ,11.6000   ,0.6000    ,0.6000    ,1.2176    ,140.0000  ,0.1650    ,0.2150    ,0.1600    ,-0.9400   ,30.6000   ,4.9200    ,-0.0800   ,4.9200    ,120.0000
//Serial#,Bin#, UNIT, UNIT, V   , V   , V   , V   , V   , V   , V   ,uA   ,uA   ,mV   ,mV   ,mV   , %,uA   ,uA   , V   ,KOhm, V   , V   , Ohms,uA   , %   , V, V, V,KHz
//1,1,-54.0000  ,-23.0000  ,0.3733    ,0.5913    ,0.5938    ,0.6035    ,4.8200    ,4.3900    ,0.4300    ,13.0076   ,331.6428  ,113.7504  ,88.4998   ,101.1251  ,24.9697   ,1.0537    ,1.0567    ,1.2322    ,183.6563  ,0.2100    ,0.2500    ,1.7814    ,0.0140    ,35.4441   ,4.9996    ,0.0181    ,4.9996    ,251.7287
//11,1,-64.0000  ,-23.0000  ,0.4682    ,0.5914    ,0.5943    ,0.6035    ,4.8400    ,4.4200    ,0.4200    ,12.8576   ,338.2194  ,113.7504  ,88.4998   ,101.1251  ,24.9697   ,1.0827    ,1.0594    ,1.2319    ,184.1224  ,0.2050    ,0.2400    ,1.8569    ,0.0135    ,35.0570   ,4.9995    ,0.0187    ,4.9995    ,255.5182

// Note1: Some SPD file have each line between quotes characters. " <line> "
// in main.cpp
extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar	*	GexProgressBar;		// Handle to progress bar in status bar

static bool m_bWaferSort;

//////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////
CGSPDtoSTDF::CGSPDtoSTDF()
{
    m_pCGSpdParameter = NULL;
    m_lStartTime = 0;

    // Set default delimiter character
    m_cDelimiter=',';
    m_iBurninTime = 65535;
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
CGSPDtoSTDF::~CGSPDtoSTDF()
{
    // Destroy list of Parameters tables.
    if(m_pCGSpdParameter!=NULL)
        delete [] m_pCGSpdParameter;
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGSPDtoSTDF::GetLastError()
{
    m_strLastError = "Import SPD (Credence TMT): ";

    switch(m_iLastError)
    {
        default:
        case errNoError:
            m_strLastError += "No Error";
            break;
        case errOpenFail:
            m_strLastError += "Failed to open file";
            break;
        case errInvalidHeader:
            m_strLastError += "Invalid file format: Unexpected header format";
            break;
        case errInvalidFormatParameter:
            m_strLastError += "Invalid file format: Didn't find 'Parameter' line";
            break;
        case errInvalidFormatLowInRows:
            m_strLastError += "Invalid file format: 'Parameter' line too short, missing rows";
            break;
        case errWriteSTDF:
            m_strLastError += "Failed creating temporary file. Folder permission issue?";
            break;
        case errLicenceExpired:
            m_strLastError += "License has expired or Data file out of date...";
            break;
    }
    // Return Error Message
    return m_strLastError;
}

//////////////////////////////////////////////////////////////////////
// Check if File is compatible with SPD format
//////////////////////////////////////////////////////////////////////
bool CGSPDtoSTDF::IsCompatible(const char *szFileName)
{
    QString strString;
    QString strSection;

    // Open hCsmFile file
    QFile f( szFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening ASL1000 file
        return false;
    }
    // Assign file I/O stream
    QTextStream hSpdFile(&f);

    // Check if valid CSV header...extract header data.

    // 1st line: 'Spreadsheet,Format,,,,,,,,,,,,,'
    do
        strString = hSpdFile.readLine();
    while(!strString.isNull() && strString.isEmpty());

    // Close file
    f.close();

    strSection = strString.section(",",0,0);
    strSection = strSection.trimmed();	// remove leading spaces.
    strSection.remove('"');	// If line starts with a '"', remove it!
    if(strSection.startsWith("Spreadsheet")==false)
    {
        // Invalid header
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the SPD file
//////////////////////////////////////////////////////////////////////
bool CGSPDtoSTDF::ReadSpdFile(const char *SpdFileName,const char *strFileNameSTDF)
{
    QString strString;
    QString strSection;
    bool	bStatus;
    int		iIndex;				// Loop index

    // Open CSV file
    QFile f( SpdFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening CSV file
        m_iLastError = errOpenFail;

        // Convertion failed.
        return false;
    }
    // Assign file I/O stream
    QTextStream hSpdFile(&f);

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    iProgressStep = 0;
    iNextFilePos = 0;
    iFileSize = f.size() + 1;
    QCoreApplication::processEvents();

    // Check if valid CSV header...extract header data.

    // 1st line: 'Spreadsheet,Format,,,,,,,,,,,,,'
    strString = ReadLine(hSpdFile);
    strSection = strString.section(m_cDelimiter,0,0);
    strSection = strSection.trimmed();	// remove leading spaces.
    strSection.remove('"');	// If line starts with a '"', remove it!
    if(strSection.startsWith("Spreadsheet")==false)
    {
        // Invalid header
        m_iLastError = errInvalidHeader;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    // Now check what the delimiter is...
    if(strString.indexOf(",") > 0)
        m_cDelimiter = ',';
    else
    if(strString.indexOf("\t") > 0)
        m_cDelimiter = '\t';
    else
    {
        strSection = strString.trimmed();
        QChar cChar = strSection.at(11);	// Character just after the 'Spreadsheet' string.
        m_cDelimiter = cChar.toLatin1();
    }

    strSection = strString.section(m_cDelimiter,1,1);
    strSection = strSection.trimmed();	// remove leading spaces.
    if(strSection != "Format")
    {
        // Invalid header
        m_iLastError = errInvalidHeader;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    // Read next line
    strString = ReadLine(hSpdFile);
    strSection = strString.section(m_cDelimiter,0,0);
    strSection = strSection.trimmed();	// remove leading spaces.
    strSection.remove('"');	// If line starts with a '"', remove it!

    // Optional line: Product:,ASC7531M10,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
    if(strSection == "Product:")
    {
        // Read Product name
        m_strProductName = strString.section(m_cDelimiter,1,1);
        m_strProductName = m_strProductName.trimmed();	// remove leading spaces.

        // Read next line
        strString = ReadLine(hSpdFile);
        strSection = strString.section(m_cDelimiter,0,0);
        strSection = strSection.trimmed();	// remove leading spaces.
        strSection.remove('"');	// If line starts with a '"', remove it!
    }

    // 2nd line: 'Test, Program:,PW29152K (Default) No Version#,,,,,,,,,,,,,'
    // or 'Test Program:,PW29152K (Default) No Version#,,,,,,,,,,,,,'
    if(strSection == "Test")
    {
        strSection = strString.section(m_cDelimiter,1,1);
        strSection = strSection.trimmed();	// remove leading spaces.
        if(strSection != "Program:")
        {
            // Invalid header
            m_iLastError = errInvalidHeader;

            // Convertion failed.
            // Close file
            f.close();
            return false;
        }
        // Read Test program name
        m_strProgramName = strString.section(m_cDelimiter,2,2);
        m_strProgramName = m_strProgramName.trimmed();	// remove leading spaces.
    }
    else
    if(strSection == "Test Program:")
    {
        // Read Test program name
        m_strProgramName = strString.section(m_cDelimiter,1,1);
        m_strProgramName = m_strProgramName.trimmed();	// remove leading spaces.
    }
    else
    {
        // Invalid header
        m_iLastError = errInvalidHeader;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }


    // 3rd line: 'Lot ID:,29150AA_024,,,,,,,,,,,,,'
    strString = ReadLine(hSpdFile);
    strSection = strString.section(m_cDelimiter,0,0);
    strSection = strSection.trimmed();	// remove leading spaces.
    strSection.remove('"');	// If line starts with a '"', remove it!
    if(strSection != "Lot ID:")
    {
        // Invalid header
        m_iLastError = errInvalidHeader;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }
    // Read LotID
    m_strLotID = strString.section(m_cDelimiter,1,1);
    m_strLotID = m_strLotID.trimmed();	// remove leading spaces.

    // 4th line: 'Operator:,operator,,,,,,,,,,,,,'
    strString = ReadLine(hSpdFile);
    strSection = strString.section(m_cDelimiter,0,0);
    strSection = strSection.trimmed();	// remove leading spaces.
    strSection.remove('"');	// If line starts with a '"', remove it!
    if(strSection != "Operator:")
    {
        // Invalid header
        m_iLastError = errInvalidHeader;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }
    // Read Operator name
    m_strOperator = strString.section(m_cDelimiter,1,1);
    m_strOperator = m_strOperator.trimmed();	// remove leading spaces.

    // 5th line: 'Computer:,WM-TMT-08,,,,,,,,,,,,,'
    strString = ReadLine(hSpdFile);
    strSection = strString.section(m_cDelimiter,0,0);
    strSection = strSection.trimmed();	// remove leading spaces.
    strSection.remove('"');	// If line starts with a '"', remove it!
    if(strSection != "Computer:")
    {
        // Invalid header
        m_iLastError = errInvalidHeader;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }
    // Read Tester name
    m_strTesterName = strString.section(m_cDelimiter,1,1);
    m_strTesterName = m_strTesterName.trimmed();	// remove leading spaces.
    m_strTesterType = "TMT";

    // 6th line: 'Date:,08/23/04 17:02:32,,,,,,,,,,,,,'
    strString = ReadLine(hSpdFile);
    strSection = strString.section(m_cDelimiter,0,0);
    strSection = strSection.trimmed();	// remove leading spaces.
    strSection.remove('"');	// If line starts with a '"', remove it!
    if(strSection != "Date:")
    {
        // Invalid header
        m_iLastError = errInvalidHeader;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }
    // Extract Date & Time
    strSection = strString.section(m_cDelimiter,1,1);// Date format: MM/DD/YY HH:MM:SS
    strSection = strSection.trimmed();
    // Extract TimeStamp, convert to UINT (nbr of seconds since 1-1-1970)
    int iDay,iMonth,iYear;
    int	iHours,iMinutes,iSeconds;
    QString	strDate,strTime;
    strDate = strSection.section(' ',0,0);
    strTime = strSection.section(' ',1,1);
    strSection = strDate.section('/',0,0);	// First field is Month
    iMonth = strSection.toInt();
    strSection = strDate.section('/',1,1);	// Second field is Day
    iDay = strSection.toInt();
    strSection = strDate.section('/',2,2);	// Third field is Year
    iYear = strSection.toInt();
    if(iYear <100){
        if(iYear >= 80)
            iYear += 1900;
        else
            iYear += 2000;
    }
    strSection = strTime.section(':',0,0);	// First field is Hours
    iHours = strSection.toInt();
    strSection = strTime.section(':',1,1);	// Second field is Minutes
    iMinutes = strSection.toInt();
    strSection = strTime.section(':',2,2);	// Third field is Seconds
    iSeconds = strSection.toInt();

    QDate SpdDate(iYear,iMonth,iDay);
    QTime CsvTime(iHours,iMinutes,iSeconds);
    QDateTime SpdDateTime(SpdDate);
    SpdDateTime.setTime(CsvTime);
    SpdDateTime.setTimeSpec(Qt::UTC);
    m_lStartTime = SpdDateTime.toTime_t();

    if(hSpdFile.atEnd() == true)
    {
        // Incorrect header...this is not a valid CSV file!
        m_iLastError = errInvalidFormatParameter;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    // Count the number of parameters specified in the line
    QStringList strCells;
    m_iTotalParameters=0;

    strCells = strString.split(m_cDelimiter, QString::KeepEmptyParts);
    m_iTotalParameters = strCells.count();
    // If no parameter specified...ignore!
    if(m_iTotalParameters <= 0)
    {
        // Incorrect header...this is not a valid CSV file!
        m_iLastError = errInvalidFormatLowInRows;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    // Allocate the buffer to hold the N parameters & results.
    m_pCGSpdParameter = new CGSpdParameter[m_iTotalParameters];	// List of parameters

    // Extract Test number index.
    strString = ReadLine(hSpdFile);
    strCells = strString.split(m_cDelimiter, QString::KeepEmptyParts);
    // Check if have the good count
    if(strCells.count() < m_iTotalParameters)
    {
        // Incorrect header...this is not a valid CSM file!
        m_iLastError = errInvalidFormatLowInRows;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    for(iIndex=2;iIndex<m_iTotalParameters;iIndex++)
    {
        // Remove space & '.' and " characters
        strSection = strCells[iIndex].trimmed();	// Get field and remove spaces
        strSection = strSection.remove('.');
        strSection = strSection.remove('"');
        m_pCGSpdParameter[iIndex].lTestNumber = strSection.toLong();
        m_pCGSpdParameter[iIndex].bStaticHeaderWritten = false;
    }

    // Extract Test names (may be on two lines....)
    strString = ReadLine(hSpdFile);
    strString = strString.remove('"');
    strCells = strString.split(m_cDelimiter,QString::KeepEmptyParts);
    // Check if have the good count
    if(strCells.count() < m_iTotalParameters)
    {
        // Incorrect header...this is not a valid CSM file!
        m_iLastError = errInvalidFormatLowInRows;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    for(iIndex=2;iIndex<m_iTotalParameters;iIndex++)
        m_pCGSpdParameter[iIndex].strName = strCells[iIndex].trimmed();

    // Check if this is Wafer-Sort data
    m_bWaferSort = false;
    if(m_iTotalParameters > 3)
    {
        if((m_pCGSpdParameter[2].strName == "Xcoord") && (m_pCGSpdParameter[3].strName == "Ycoord"))
            m_bWaferSort = true;
        if((m_pCGSpdParameter[2].strName == "X") && (m_pCGSpdParameter[3].strName == "Y"))
            m_bWaferSort = true;
    }

    // Next lines is either:
    // a) 2nd half of test name , then HighLimit, then LowLimit or
    // b) HighLimit, then LowLimit or
    bool bOk = false;
    strString = ReadLine(hSpdFile);
    strString = strString.remove('"');
    strCells = strString.split(m_cDelimiter, QString::KeepEmptyParts);
    // Check if have the good count
    if(strCells.count() > 2)
    {
        strCells[2].toFloat(&bOk);
        if(!bOk)
            bOk = strCells[2].startsWith("none",Qt::CaseInsensitive);
        if(bOk == false)
        {
            // This line includes the second half of the test name...read it!
            for(iIndex=2;iIndex<m_iTotalParameters;iIndex++)
            {
                strSection = "";
                if(strCells.count() > iIndex)
                    strSection = strCells[iIndex].trimmed();
                if(!strSection.isEmpty())
                    m_pCGSpdParameter[iIndex].strName += " "+ strSection;
            }
        }
    }

    // Read High Limits line.
    // Extract the N column Upper Limits
    if(!bOk)
        strString = ReadLine(hSpdFile);
    strString = strString.remove('"');
    strCells = strString.split(m_cDelimiter, QString::KeepEmptyParts);
    for(iIndex=2;iIndex<m_iTotalParameters;iIndex++)
    {
        strSection = "";
        if(strCells.count() > iIndex)
            strSection = strCells[iIndex];
        m_pCGSpdParameter[iIndex].fHighLimit = strSection.trimmed().toFloat(&bStatus);
        m_pCGSpdParameter[iIndex].bValidHighLimit = bStatus;
    }

    // Read Low Limits line.
    // Extract the N column Lower Limits
    strString = ReadLine(hSpdFile);
    strString = strString.remove('"');
    strCells = strString.split(m_cDelimiter, QString::KeepEmptyParts);
    for(iIndex=2;iIndex<m_iTotalParameters;iIndex++)
    {
        strSection = "";
        if(strCells.count() > iIndex)
            strSection = strCells[iIndex];
        m_pCGSpdParameter[iIndex].fLowLimit = strSection.trimmed().toFloat(&bStatus);
        m_pCGSpdParameter[iIndex].bValidLowLimit = bStatus;

        // Note: if LL = HL = 0, then limits are disabled!
        if(m_pCGSpdParameter[iIndex].bValidLowLimit && m_pCGSpdParameter[iIndex].bValidHighLimit &&
            (m_pCGSpdParameter[iIndex].fLowLimit == m_pCGSpdParameter[iIndex].fHighLimit))
        {
            m_pCGSpdParameter[iIndex].bValidHighLimit = false;
            m_pCGSpdParameter[iIndex].bValidLowLimit  = false;
        }

    }

    // Extract the N column Units
    strString = ReadLine(hSpdFile);
    strString = strString.remove('"');
    strCells = strString.split(m_cDelimiter, QString::KeepEmptyParts);
    // Check if have the good count
    if(strCells.count() <= 1)
    {
        // Incorrect header...this is not a valid CSM file!
        m_iLastError = errInvalidFormatLowInRows;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    if((strCells[0] != "Serial#") || (strCells[1] != "Bin#"))
    {
        // Incorrect header...this is not a valid CSV file!
        m_iLastError = errInvalidFormatLowInRows;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    // Extract units & compute scae factor (so to normalize limits & units)
    for(iIndex=2;iIndex<m_iTotalParameters;iIndex++)
    {
        strSection = "";
        if(strCells.count() > iIndex)
            strSection = strCells[iIndex];
        NormalizeLimits(iIndex,strSection.trimmed());
    }

    // Loop reading file until end is reached & generate STDF file dynamically.
    bStatus = WriteStdfFile(&hSpdFile,strFileNameSTDF);
    if(!bStatus)
        QFile::remove(strFileNameSTDF);

    // Close file
    f.close();

    // Success parsing CSV file
    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Normalize test limits when writing into PTR.
//////////////////////////////////////////////////////////////////////
void CGSPDtoSTDF::NormalizeLimits(int iIndex, const QString& strUnits)
{
    int	value_scale=0;
    if(strUnits.length() <= 1)
    {
        // units too short to include a prefix, then keep it 'as-is'
        m_pCGSpdParameter[iIndex].scale = 0;
        m_pCGSpdParameter[iIndex].strUnits = strUnits;
        return;
    }

    QChar cPrefix = strUnits[0];
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
    m_pCGSpdParameter[iIndex].fLowLimit *= GS_POW(10.0,value_scale);
    m_pCGSpdParameter[iIndex].fHighLimit *= GS_POW(10.0,value_scale);
    m_pCGSpdParameter[iIndex].scale = value_scale;
    m_pCGSpdParameter[iIndex].strUnits = strUnits.mid(1);	// Take all characters after the prefix.
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from CSV data parsed
//////////////////////////////////////////////////////////////////////
bool CGSPDtoSTDF::WriteStdfFile(QTextStream *hSpdFile, const char *strFileNameSTDF)
{
    // now generate the STDF file...
    GS::StdLib::Stdf StdfFile;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    if(StdfFile.Open((char*)strFileNameSTDF,STDF_WRITE) != GS::StdLib::Stdf::NoError)
    {
        // Failed importing CSV file into STDF database
        m_iLastError = errWriteSTDF;

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
    StdfFile.WriteDword(m_lStartTime);						// Setup time
    StdfFile.WriteDword(m_lStartTime);						// Start time
    StdfFile.WriteByte(1);									// Station
    StdfFile.WriteByte((BYTE) 'P');							// Test Mode = PRODUCTION
    StdfFile.WriteByte((BYTE) ' ');							// rtst_cod
    StdfFile.WriteByte((BYTE) ' ');							// prot_cod
    StdfFile.WriteWord(m_iBurninTime);						// burn_tim
    StdfFile.WriteByte((BYTE) ' ');							// cmod_cod
    StdfFile.WriteString(m_strLotID.toLatin1().constData());			// Lot ID
    StdfFile.WriteString(m_strProductName.toLatin1().constData());	// Part Type / Product ID
    StdfFile.WriteString(m_strTesterName.toLatin1().constData());	// Node name
    StdfFile.WriteString(m_strTesterType.toLatin1().constData());	// Tester Type
    StdfFile.WriteString(m_strProgramName.toLatin1().constData());	// Job name
    StdfFile.WriteString("");								// Job rev
    StdfFile.WriteString("");								// sublot-id
    StdfFile.WriteString(m_strOperator.toLatin1().constData());		// operator
    StdfFile.WriteString("");								// exec-type
    StdfFile.WriteString("");								// exe-ver
    StdfFile.WriteString("");								// test-cod
    StdfFile.WriteString("");								// test-temperature
    // Construct custom Galaxy USER_TXT
    QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
    strUserTxt += ":TMT Credence";
    StdfFile.WriteString((char *)strUserTxt.toLatin1().constData());	// user-txt
    StdfFile.WriteString("");								// aux-file
    StdfFile.WriteString("");								// package-type
    StdfFile.WriteString("");								// familyID
    StdfFile.WriteString("");								// Date-code
    StdfFile.WriteString("");								// Facility-ID
    StdfFile.WriteString("");								// FloorID
    StdfFile.WriteString("");								// ProcessID
    StdfFile.WriteString("");								// Frequency/Step
    StdfFile.WriteRecord();

    // Write Test results for each line read.
    QStringList strCells;
    QString strString;
    QString strSection;
    float	fValue=0.0F;		// Used for readng floating point numbers.
    int		iIndex;				// Loop index
    BYTE	bData;
    WORD	wBin;
    int		iDie_X,iDie_Y;
    long	iTotalGoodBin,iTotalFailBin;
    long	iTestNumber,iTotalTests,iPartNumber;
    bool bStatus;
        //FIXME: not used ?
        //bool bPassStatus;
    int		value_scale;	// Scale factor for limots & results.

    // Reset counters
    iTotalGoodBin=iTotalFailBin=0;
    iPartNumber=0;

    // Write all Parameters read on this wafer.: PTR....PTR, PRR
    while(hSpdFile->atEnd() == false)
    {
        // Read line
        strString = ReadLine(*hSpdFile);
        strString = strString.remove('"');

        if(strString.isEmpty() == false)
        {
            // Part number tested
            iPartNumber++;

            // Pass/Fail flag.
                        //FIXME: not used ?
                        //bPassStatus = true;

            // Reset counters
            iTotalTests = 0;

            // Split line
            strCells = strString.split(m_cDelimiter, QString::KeepEmptyParts);

            // Extract Die location (if wafer sort)
            iDie_X = -32768;
            iDie_Y = -32768;
            if(m_bWaferSort)
            {
                // WAFER SORT Test data
                strSection = strCells[2].trimmed();
                iDie_X = (int) strSection.toFloat();
                strSection = strCells[3].trimmed();
                iDie_Y = (int) strSection.toFloat();
            }

            // Write PIR
            RecordReadInfo.iRecordType = 5;
            RecordReadInfo.iRecordSubType = 10;
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteByte(1);			// Test head
            StdfFile.WriteByte(1);			// Tester site#
            StdfFile.WriteRecord();

            // Read Parameter results for this record
            for(iIndex = 2; iIndex<m_iTotalParameters; iIndex++)
            {
                if(strCells.count() <= iIndex)
                    break;

                // Extract test result....if string is "0.0000" with NO trailing, spaces, then test was NOT excuted.
                strSection = strCells[iIndex];
                if(strSection == "0.0000")
                    bStatus = false;
                else
                {
                    // We have a result specified...convert it to a floating point value. could be "0.0000    "!!!!!
                    strSection = strSection.trimmed();
                    fValue = strSection.toFloat(&bStatus);
                }
                if(bStatus == true)
                {
                    // Valid test result...write the PTR
                    iTotalTests++;

                    RecordReadInfo.iRecordType = 15;
                    RecordReadInfo.iRecordSubType = 10;
                    StdfFile.WriteHeader(&RecordReadInfo);
                    iTestNumber = m_pCGSpdParameter[iIndex].lTestNumber;

                    StdfFile.WriteDword(iTestNumber);			// Test Number
                    StdfFile.WriteByte(1);						// Test head

                    // Write Site# (if known, or 1 otherwise)
                    StdfFile.WriteByte(1);					// Tester site#

                    // Normalize result
                    value_scale = m_pCGSpdParameter[iIndex].scale;
                    fValue*= GS_POW(10.0,value_scale);

                    // check If test fails limits
                    if(((m_pCGSpdParameter[iIndex].bValidLowLimit==true) && (fValue < m_pCGSpdParameter[iIndex].fLowLimit)) ||
                       ((m_pCGSpdParameter[iIndex].bValidHighLimit==true) && (fValue > m_pCGSpdParameter[iIndex].fHighLimit)))
                    {
                        bData = 0200;	// Test Failed
                                          //FIXME: not used ?
                                          //bPassStatus = false;
                    }
                    else
                    {
                        bData = 0;		// Test passed
                    }
                    StdfFile.WriteByte(bData);							// TEST_FLG
                    StdfFile.WriteByte(0x40|0x80);						// PARAM_FLG
                    StdfFile.WriteFloat(fValue);						// Test result
                    if(m_pCGSpdParameter[iIndex].bStaticHeaderWritten == false)
                    {
                        StdfFile.WriteString(m_pCGSpdParameter[iIndex].strName.toLatin1().constData());	// TEST_TXT
                        StdfFile.WriteString("");							// ALARM_ID
                        bData = 2;	// Valid data.
                        if(m_pCGSpdParameter[iIndex].bValidLowLimit==false)
                            bData |=0x40;
                        if(m_pCGSpdParameter[iIndex].bValidHighLimit==false)
                            bData |=0x80;
                        StdfFile.WriteByte(bData);							// OPT_FLAG
                        StdfFile.WriteByte(0);								// RES_SCALE
                        StdfFile.WriteByte(-value_scale);					// LLM_SCALE
                        StdfFile.WriteByte(-value_scale);					// HLM_SCALE
                        StdfFile.WriteFloat(m_pCGSpdParameter[iIndex].fLowLimit);		// LOW Limit
                        StdfFile.WriteFloat(m_pCGSpdParameter[iIndex].fHighLimit);	// HIGH Limit
                        StdfFile.WriteString(m_pCGSpdParameter[iIndex].strUnits.toLatin1().constData());	// Units
                        m_pCGSpdParameter[iIndex].bStaticHeaderWritten = true;
                    }
                    StdfFile.WriteRecord();
                }	// Valid test result
            };	// Read all results on line

            // Binning result
            strSection = strCells[1].trimmed();
            wBin = strSection.toLong();

            // Write PRR
            RecordReadInfo.iRecordType = 5;
            RecordReadInfo.iRecordSubType = 20;
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteByte(1);			// Test head
            // Tester site#
            StdfFile.WriteByte(1);	// Tester site#
            if(wBin == 1)
            {
                StdfFile.WriteByte(0);				// PART_FLG : PASSED
                iTotalGoodBin++;
            }
            else
            {
                StdfFile.WriteByte(8);				// PART_FLG : FAILED
                iTotalFailBin++;
            }


            StdfFile.WriteWord((WORD)iTotalTests);	// NUM_TEST
            StdfFile.WriteWord(wBin);		        // HARD_BIN
            StdfFile.WriteWord(wBin);	            // SOFT_BIN
            StdfFile.WriteWord((WORD)iDie_X);		// X_COORD
            StdfFile.WriteWord((WORD)iDie_Y);		// Y_COORD
            StdfFile.WriteDword(0);					// Testing time (if known, otherwise: 0)
            strSection = strString.section(m_cDelimiter,0,0);
            StdfFile.WriteString((char *)strSection.toLatin1().constData());	// PART_ID
            StdfFile.WriteRecord();
        }
    }	// Read all lines in file

    // Write PCR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 30;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(255);					// Test Head = ALL
    StdfFile.WriteByte(255);					// Test sites = ALL
    StdfFile.WriteDword(iPartNumber);			// Total Parts tested
    StdfFile.WriteDword(0);						// Total Parts re-tested
    StdfFile.WriteDword(0);						// Total Parts aborted
    StdfFile.WriteDword(iTotalGoodBin);			// Total GOOD Parts
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
// Convert 'FileName' SPD (TMT Credence) file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGSPDtoSTDF::Convert(const char *SpdFileName, const char *strFileNameSTDF)
{
    // No erro (default)
    m_iLastError = errNoError;

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo fInput(SpdFileName);
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
            GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(SpdFileName).fileName()+"...");
            GexScriptStatusLabel->show();
        }
    }
    QCoreApplication::processEvents();

    if(ReadSpdFile(SpdFileName,strFileNameSTDF) != true)
    {
        //////////////////////////////////////////////////////////////////////
        // For ProgressBar
        if((GexProgressBar != NULL)
        && bHideProgressAfter)
            GexProgressBar->hide();

        if((GexScriptStatusLabel != NULL)
        && bHideLabelAfter)
            GexScriptStatusLabel->hide();
        return false;	// Error reading SPD file
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
QString CGSPDtoSTDF::ReadLine(QTextStream& hFile)
{
    QString strString;

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
    QCoreApplication::processEvents();

    do
        strString = hFile.readLine();
    while(!strString.isNull() && strString.isEmpty());

    return strString;

}
