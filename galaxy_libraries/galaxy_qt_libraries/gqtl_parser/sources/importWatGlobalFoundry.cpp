
//////////////////////////////////////////////////////////////////////
// importSiTimeEtest.cpp: Convert a SiTimeEtest file to STDF V4.0
//////////////////////////////////////////////////////////////////////

#include <math.h>
#include <qmath.h>
#include <time.h>
#include <QFile>
#include <QDir>
#include <QVector>

#include "importWatGlobalFoundry.h"
#include "gqtl_log.h"


// Remove spaces from the header keys because they are not consistante in the input files
const QString cCustomerName = "customername";
const QString cCustomerPN = "customerpn";
const QString cPN = "pn";
const QString cLotId = "lotid(lll)";
const QString cULL = "ull";
const QString cCLL = "cll";
const QString cTimestampStart = "timestamp(start)";
const QString cTimestampEnd = "timestamp(end)";
const QString cFab ="fab";
const QString cTechnology = "technology";
const QString cProduct = "product";
const QString cTestProg = "testprogec";
const QString cTestProgram = "testprogram";
const QString cEquipmentId = "equipmentid";
const QString cParameterCount = "parametercount";
const QString cTemperature = "temperature";
const QString cFlatOrientation = "flatorientation";
const QString cWaferCount = "wafercount";
const QString cTestLevel = "testlevel";
const QString cSpecHigh = "SPEC HIGH";
const QString cSpecLow = "SPEC LOW";
const QString cPNP = "PNP";
const QString cWafer = "wafer";
const QString cPasFail = "Pass/Fail";
const QString cTestNumber = "Test Number";
const QString cUnits = "Units";
const QString cSlotId = "SlotId";
const QString cPNPIndex = "PnpIndex";


/// File format:
//Customer Name,INPHI INTERNATIONAL PTE. LTD.
//Customer PN,0000001LY051
//PN,0000001LY051
//Lot Id(LLL),D2U42000KL
//ULL,1NL3B00000
//CLL,
//Timestamp (Start),2018/6/18 11:49:14
//Timestamp (End),2018/6/19 1:24:34
//Fab,FAB9
//Technology,SIGE8XP
//Product,Mosis 899 P1.2
//Test Prog EC,SIGE8XPLM1
//Test Program,2929,2939,2989,3215,8189,8219,8322,
//Equipment Id,KDE1
//Parameter Count,113
//Temperature,25
//Flat Orientation,DOWN,DOWN,DOWN,DOWN,DOWN,DOWN,DOWN,
//Wafer Count,9
//Test Level (Metal Type),LAST_METAL
//,,,,PNP,2929,,,,,,,,,,,,,,,,,,,,PNP,2939,,,,,,,,,,,,,,,PNP,2989,,,,,,,,,,,,,,,,,,,,,,,,,PNP,3215,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,PNP,8189,,,,PNP,8219,,,,,,,,,,,,PNP,8322,,,,,,,,,,
//,,,,SiteCount,225,,,,,,,,,,,,,,,,,,,,SiteCount,225,,,,,,,,,,,,,,,SiteCount,225,,,,,,,,,,,,,,,,,,,,,,,,,SiteCount,225,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,SiteCount,225,,,,SiteCount,45,,,,,,,,,,,,SiteCount,225,,,,,,,,,,
//,,,,Parameter,19,,,,,,,,,,,,,,,,,,,,Parameter,14,,,,,,,,,,,,,,,Parameter,24,,,,,,,,,,,,,,,,,,,,,,,,,Parameter,33,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,Parameter,3,,,,Parameter,11,,,,,,,,,,,,Parameter,9,,,,,,,,,,
//Wafer,SlotId,Pass/Fail,SiteID,Site_X,Site_Y,Diffusion_Nwell_Rs,Diode_P+/NW_Ideality,Diode_P_NWell_Bounded_Vf,NFET_10x0.12_dense_IdSat,NFET_10x0.12_dense_Ioff,NFET_10x0.12_dense_VtSat,NFET_5x5_VtLin,NFET_Coverlap,NFET_Junction_Cap,NFET_Lpoly,NFET_PSRO_Delay,NFET_Tox_Inv,PFET_10x0.12_dense_IdSat,PFET_10x0.12_dense_Ioff,PFET_10x0.12_dense_VtSat,PFET_5x5_VtLin,PFET_Coverlap,PFET_Junction_Cap,PFET_Tox_Inv,Site_X,Site_Y,DG_NFET_5x0.24_IdSat,DG_NFET_5x0.24_Ioff,DG_NFET_5x0.24_VtSat,DG_NFET_5x5_VtLin,DG_NFET_Coverlap,DG_NFET_Tox_Inv,DG_PFET_5x0.24_IdSat,DG_PFET_5x0.24_Ioff,DG_PFET_5x0.24_VtSat,DG_PFET_5x5_VtLin,DG_PFET_Coverlap,DG_PFET_Tox_Inv,Resistor_Diffusion_N+_Rs,Resistor_Poly_P+_Rs,Site_X,Site_Y,Contacts_M1_to_N/P_PC_Rc,Contacts_M1_to_N_Diff_Rc,Contacts_M1_to_P_Diff_Rc,Contacts_Via_V1_Rc,Contacts_Via_V2_Rc
//,,,,Test Number,,10512,20528,10525,10120,20121,10123,10032,20380,20382,20363,20552,10340,10130,20131,10133,10047,20385,20387,10350,,,10060,20061,10063,10032,20380,10340,10075,20076,10078,10047,20385,10350,10518,10516,,,10376,10381,10375,10416,10446,10476,10555,20352,10621,20342,20404,10622,20401,10623,10624,20549,10627,20547,20318,20317,10274,10253,10290,10250,,,10215,10371,20222,20223,20226,20227,20229,20475,20474,20228,20761,20083,10142,10309,10310,10312,10313,10314,10375,20233,20234,20235,20236,20306,10079,20485,20484,20755,20444,20334,10031,10127,10159,,,10240,10250,10234,,,10761,10766,10431,20599,10548,20598,20595,10543,20594,10436,10656,,,10314,10567,10313,10321,20315,10303,20324,20327,20715,,
//,,,,Units,,Ohms/Sq,Ratio,V,uA/um,nA/um,V,V,fF/um,fF/um2,um,pS/Stage,nm,uA/um,nA/um,V,V,fF/um,fF/um2,nm,,,uA/um,nA/um,V,V,fF/um,nm,uA/um,nA/um,V,V,fF/um,nm,Ohms/Sq,Ohms/Sq,,,Ohms/CA,Ohms/CA,Ohms/CA,Ohms/Via,Ohms/Via,Ohms/Via,Ohms/Via,Percent,Ohms/um,Percent,Percent,Ohms/um,Percent,Ohms/um,Ohms/um,Percent,Ohms/um,Percent,Percent,Percent,Ohms/Sq,Ohms/Sq,Ohms/Sq,Ohms/Sq,,,V,V,V,Ratio,Unit,V,V,fF/um2,fF/um2,V,Percent,Ohms/Sq,Unit,V,Unit,Ohms,V,V,V,nA,uA,V,Ratio,Ratio,Ohms,fF/um2,fF/um2,Percent,Ohms,V,Ohms/Sq,Ohms/Sq,fF/um2,,,,,,,,Ohms/Via,Ohms/Via,fF/um2,Percent,Ohms/Sq,Percent,Percent,Ohms/Sq,Percent,fF/um2,Ohms/Sq,,,V,V,V,Unit,V,V,Ratio,Ohms,Percent,,

//AGMTGQP,14,NA,1,10,59,535.81,1.0245,.64963,538.97,.52428,.33867,.17226,.32991,.88852,.08328,21.969,3.11716,-174.81,-.21523,-.32929,-.19101,.25442,1.09909,3.36093,10,59,705.34,.11451,.34228,.43017,.30933,5.98228,-257.25,-.00973,-.43701,-.44677,.28615,6.22898,80.733,344.58,10,59,10.908,12.522,12.335,1.09539,1.11498,1.08671,.613,100,.57058,100,100,.30717,100,.3054,.28319,100,.10427,100,100,100,6.92218,6.76878,6.12171,7.06003,10,59,11.862,3.27997,.70918,1.02695,605.83,2.54613,11.872,.79508,8.21776,33.962,100,2241.1,.06769,.7106,529.27,4.31466,5.90006,2.54043,1.79297,27.699,14.66,5.90741,1.0323,1.01929,42.62,3.82803,7.81256,0,3425.8,42.881,8.6453,1699.2,2.17787,10,59,.43652,.16699,11.529,10,79,.42113,.50272,3.18411,100,.00706,100,100,.02331,100,.99262,62.427,10,59,18.281,8.16038,2.87712,259.54,18.345,.79044,1.0094,69.382,100,
//AGMTGQP,14,NA,2,10,99,536.56,.99446,.65216,539.29,.58405,.33398,.16757,.32808,.88258,.08321,21.524,3.11939,-175.43,-.1605,-.33164,-.1957,.25394,1.10362,3.35594,10,99,693.65,.06556,.36181,.43017,.30764,5.99113,-259.03,-.00676,-.45654,-.44677,.28697,6.2295,80.487,343.04,10,99,11.054,12.753,12.532,1.0903,1.09021,1.09854,.61112,100,.55861,100,100,.29773,100,.30579,.28193,100,.10746,100,100,100,6.81063,6.67,6.09779,7.02995,10,99,11.968,3.33624,.70565,1.30728,314.92,1.74964,11.975,.78069,8.75148,34.588,100,2401.8,.06815,.70876,494.37,3.79053,5.91749,2.09637,1.84759,31.892,15.766,5.92577,1.0279,1.03626,45.41,-9999,-9999,0,3396.7,41.036,8.8362,1680.8,2.16966,10,99,.43945,.16699,11.547,32,159,.41882,.48924,3.21296,100,.00715,100,100,.02357,100,1.02154,62.317,10,99,18.349,8.16961,2.9321,251.07,18.392,.79084,1.00699,68.748,100,
//AGMTGQP,14,NA,3,21,39,540.76,.99508,.65234,551.61,.79531,.3246,.16289,.3336,.8755,.08413,22.45,3.09881,-180.81,-.21274,-.3246,-.18398,.26307,1.09977,3.34443,21,39,695.57,.06334,.36181,.43505,.30949,5.97534,-254.03,-.0067,-.43701,-.44677,.28914,6.21955,78.966,342.93,21,39,11.278,13.104,12.9,1.1376,1.21171,1.12236,.63332,100,.57012,100,100,.31077,100,.30576,.29609,100,.10196,100,100,100,7.00483,6.86995,6.11497,6.98596,21,39,11.858,3.27758,.70607,1.04546,453.15,1.83528,11.866,.79138,8.26532,35.773,100,2397.3,.0687,.7089,434.63,3.24855,5.90465,2.03256,1.7834,35.804,15.561,5.91109,1.03407,1.05001,-9999,-9999,-9999,0,3977.3,36.942,8.6674,1689.2,2.13413,21,39,.44238,.16699,11.513,43,119,.42549,.51969,3.16157,100,.00713,100,100,.02314,100,.99519,61.564,21,39,18.295,8.12736,2.992,266.36,18.36,.78963,1.01578,69.466,100,


namespace GS
{
namespace Parser
{

QVector<QString> WatGlobalFoundryEtesttoSTDF::mCompatibleKeys;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
WatGlobalFoundryEtesttoSTDF::WatGlobalFoundryEtesttoSTDF() : ParserBase(typeWatGlobalFoundry, "WatGlobalFoundry")
{
    mOutputFiles.clear();
    mTestsList = NULL;
    mListTestPrograms.clear();
    mTotalParameters = 0;
    mTotalColumns = 0;
    mExistSlotID = false;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
WatGlobalFoundryEtesttoSTDF::~WatGlobalFoundryEtesttoSTDF()
{
    // Destroy list of Parameters tables.
    if(mTestsList!=NULL)
    {
        delete [] mTestsList;
    }
    mOutputFiles.clear();
    mListTestPrograms.clear();
}


//////////////////////////////////////////////////////////////////////
// Check if File is compatible with SiTime format
//////////////////////////////////////////////////////////////////////
bool WatGlobalFoundryEtesttoSTDF::IsCompatible(const QString& aFileName)
{
    bool    lIsCompatible = false;
    QFile   lFile(aFileName);

    if(lFile.open(QIODevice::ReadOnly))
    {
        // Assign file I/O stream
        QTextStream lTxtStream(&lFile);

        QMap<QString, QString> lHeaders;
        QString lErrorMessqge;
        if (ReadHeader(lHeaders, lTxtStream, lErrorMessqge))
        {
            lIsCompatible = true;
        }
    }

    return lIsCompatible;
}


bool WatGlobalFoundryEtesttoSTDF::ReadHeader(QMap<QString, QString>& aHeaders, QTextStream &aTxtSTream, QString& aErrorMessage)
{
    bool lAbort = false;
    bool lHeaderCompleted = false;

    InitCompatibleKeys();

    QString lLine;
    do
    {
        lLine = aTxtSTream.readLine().simplified();

        if (lLine.isNull() == false && lLine.isEmpty() == false)
        {
            QString lKey = lLine.section(',', 0, 0).replace(" ", "").toLower();
            QString lValue = lLine.section(',', 1).simplified();

            if (lKey.contains(cTestLevel.toLower()))
            {
                aHeaders.insert(cTestLevel, lValue);
                lHeaderCompleted = true;
                // !!!! The header is red. We'll exit the while loop.
                break;
            }
            if (mCompatibleKeys.contains(lKey))
            {
                aHeaders.insert(lKey, lValue);
            }
            else
            {
                aErrorMessage = "Incompatible header key found [" + lKey + "]";
                lAbort = true;
            }
        }
    }
    while(!lAbort && !aTxtSTream.atEnd() &&!lHeaderCompleted);

    return (!lAbort && lHeaderCompleted);
}


void WatGlobalFoundryEtesttoSTDF::InitCompatibleKeys()
{
    if (mCompatibleKeys.isEmpty())
    {
        mCompatibleKeys.push_back(cCustomerName.toLower());
        mCompatibleKeys.push_back(cCustomerPN.toLower());
        mCompatibleKeys.push_back(cPN.toLower());
        mCompatibleKeys.push_back(cLotId.toLower());
        mCompatibleKeys.push_back(cULL.toLower());
        mCompatibleKeys.push_back(cCLL.toLower());
        mCompatibleKeys.push_back(cTimestampStart.toLower());
        mCompatibleKeys.push_back(cTimestampEnd.toLower());
        mCompatibleKeys.push_back(cFab.toLower());
        mCompatibleKeys.push_back(cTechnology.toLower());
        mCompatibleKeys.push_back(cProduct.toLower());
        mCompatibleKeys.push_back(cTestProg.toLower());
        mCompatibleKeys.push_back(cTestProgram.toLower());
        mCompatibleKeys.push_back(cEquipmentId.toLower());
        mCompatibleKeys.push_back(cParameterCount.toLower());
        mCompatibleKeys.push_back(cTemperature.toLower());
        mCompatibleKeys.push_back(cFlatOrientation.toLower());
        mCompatibleKeys.push_back(cWaferCount.toLower());
        mCompatibleKeys.push_back(cTestLevel.toLower());
    }
}

bool WatGlobalFoundryEtesttoSTDF::ProcessHeader(QTextStream &aTxtStream)
{
    QMap<QString, QString>  lHeaders;

    if (ReadHeader(lHeaders, aTxtStream, mLastErrorMessage) == false)
    {
        mLastError = errMissingData;
        return false;
    }

    mMIRRecord.SetPART_TYP(lHeaders.value(cCustomerPN));
    mMIRRecord.SetLOT_ID(lHeaders.value(cLotId));
    mMIRRecord.SetSBLOT_ID(lHeaders.value(cLotId));
    // Read start time
    QDateTime lReportTime = QDateTime::fromString(lHeaders.value(cTimestampStart), "yyyy/M/d h:m:s");
    lReportTime.setTimeSpec(Qt::UTC);
    mMIRRecord.SetSTART_T(lReportTime.toTime_t());
    mMIRRecord.SetSETUP_T(lReportTime.toTime_t());
    mWIRRecord.SetSTART_T(lReportTime.toTime_t());
    mATRRecord.SetMOD_TIM(QDateTime::currentDateTime().toTime_t());
    mATRRecord.SetCMD_LINE("Quantix (Global Foundries WAT converter)");

    // Read end time
    lReportTime = QDateTime::fromString(lHeaders.value(cTimestampEnd), "yyyy/M/d h:m:s");
    lReportTime.setTimeSpec(Qt::UTC);
    mMRRRecord.SetFINISH_T(lReportTime.toTime_t());
    mWRRRecord.SetFINISH_T(lReportTime.toTime_t());

    mMIRRecord.SetFACIL_ID(lHeaders.value(cFab));
    mMIRRecord.SetPROC_ID(lHeaders.value(cTechnology));
    mMIRRecord.SetFAMLY_ID(lHeaders.value(cProduct));
    mMIRRecord.SetJOB_REV(lHeaders.value(cTestProg));
    QString lequip = lHeaders.value(cEquipmentId);
    mMIRRecord.SetTSTR_TYP(lequip);
    mTestPrograms = lHeaders.value(cTestProgram);
    mFlatOrientations = lHeaders.value(cFlatOrientation).split(",", QString::SkipEmptyParts);
    mMIRRecord.SetTST_TEMP(lHeaders.value(cTemperature));
    mMIRRecord.SetTEST_COD(lHeaders.value(cTestLevel));
    QString	lUserTxt;
    lUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    lUserTxt += ":";
    lUserTxt += GEX_IMPORT_DATAORIGIN_ETEST;
    lUserTxt += ":GLOBAL FOUNDRIES WAT";
    mMIRRecord.SetUSER_TXT(lUserTxt);

    // Set inchanged fields
    mWIRRecord.SetHEAD_NUM(1);							// Test head
    mWIRRecord.SetSITE_GRP(255);						// Tester site (all)
    mWRRRecord.SetHEAD_NUM(1);
    mWRRRecord.SetSITE_GRP(255);
    mMIRRecord.SetBURN_TIM(65535);
    mMIRRecord.SetRTST_COD(' ');
    mMIRRecord.SetPROT_COD(' ');
    mMIRRecord.SetCMOD_COD(' ');
    mWCRRecord.SetWAFR_SIZ(0);
    mWCRRecord.SetDIE_HT(0);
    mWCRRecord.SetDIE_WID(0);
    mWCRRecord.SetWF_UNITS(0);
    mWCRRecord.SetWF_FLAT(' ');

    return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the SiTime file
//////////////////////////////////////////////////////////////////////
bool WatGlobalFoundryEtesttoSTDF::ReadWatGlobalFoundryFile(QTextStream& aTxtStream, const QString &aOutputSTDF)
{
    QString lString;

    // Read until the PNP section
    do
    {
        lString = aTxtStream.readLine();
    }
    while (!aTxtStream.atEnd() && !lString.contains(cPNP, Qt::CaseInsensitive));

    if (aTxtStream.atEnd())
    {
        mLastErrorMessage = "No PNP section in the input file";
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());
        mLastError = errInvalidFormatParameter;
        return false;
    }

    short lNUmberOfTestprogram = lString.count(cPNP, Qt::CaseInsensitive);
    if (lNUmberOfTestprogram != mFlatOrientations.size())
    {
        mLastErrorMessage = "The number of test programs in different from the number of flat orientation in the input file";
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());
        mLastError = errInvalidFormatParameter;
        return false;
    }

    // Find the biginning of each test program in the file
    // ,,,,PNP,2929,,,,,,,,,,,,,,,,,,,,PNP,2939,,,,,,,,,,,,,,,PNP,2959,,,,,PNP,2989,,,,,,,,,,,,,,,,,,,,,,,,,PNP,3215,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,PNP,8189,,,,PNP,8219,,,,,,,,,,,,PNP,8322,,,,,,,,,,
    // we'll go throw the line and get the part delimited by "PNP". Each PNP section will be written in a separate STDF file
    // Each section PNP represent a test program
    QStringList lTestProgramIndexes = lString.split(",");
    QString lTestProgram;
    for (int i=0; i<lTestProgramIndexes.size(); ++i)
    {
        if (lTestProgramIndexes.at(i) == cPNP)
        {
            // Get the PNP string (test program)
            if ((i+1) < lTestProgramIndexes.size())
            {
                lTestProgram = lTestProgramIndexes.at(i+1);
            }
            mListTestPrograms.push_back(QPair<int, QString>(i, lTestProgram));
        }
    }

    QStringList lTestPrograms = mTestPrograms.split(",", QString::SkipEmptyParts);
    // Check that we have the same number of test programs and orientations in the input file
    if (lTestPrograms.size() != mFlatOrientations.size() || mListTestPrograms.size() != lTestPrograms.size())
    {
        mLastErrorMessage = QString("The number of Test Program %1 is different from the number of Flat Orientation %2 or the number of PNP %3.")
                            .arg(lTestPrograms.size())
                            .arg(mFlatOrientations.size())
                            .arg(mListTestPrograms.size());
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());
        mLastError = errInvalidFormatParameter;
        return false;
    }

    // Read the test name list
    // Wafer,SlotId,Pass/Fail,SiteID,Site_X,Site_Y,Diffusion_Nwell_Rs,Diode_P+/NW_Ideality,Diode_P_NWell_Bounded_Vf,NFET_10x0.12_dense_IdSat,NFET_10x0.12_dense_Ioff,NFET_10x0.12_dense_VtSat
    do
    {
        lString = aTxtStream.readLine();
    }
    while (!aTxtStream.atEnd() && !lString.contains(cWafer, Qt::CaseInsensitive));
    if (aTxtStream.atEnd())
    {
        mLastErrorMessage = QString("No test name defined in the file");
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());
        mLastError = errInvalidFormatParameter;
        return false;
    }

    // Allocate the appropriate memory space for the test list
    RemoveCommaAtTheEnd(lString);
    QStringList lStrCells = lString.split("," , QString::KeepEmptyParts);

    mExistSlotID = lStrCells.contains(cSlotId, Qt::CaseInsensitive);
    mParameterOffset = mExistSlotID ? 6 : 5;

    // The number of test = number of colum - Offset - number of columns used for Site_X, Site_Y
    // But we'll allocate also 2 cases for each site_X and Site_Y, it 'll be much easier to use after
    mTotalParameters = lStrCells.size() - mParameterOffset;
    try
    {
        mTestsList = new ParserParameter[mTotalParameters];	// List of parameters
    }
    catch (const std::bad_alloc& )
    {
        GSLOG(SYSLOG_SEV_ALERT, "Memory allocation exception caught ");
        return false;
    }
    catch (...)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Exception caught ");
        return false;
    }
    for (int i = 0; i < mTotalParameters; ++i)
    {
        mTestsList[i].SetTestName(lStrCells[mParameterOffset + i]);
    }

    // Read the test number list
    // ,,,,Test Number,,512,528,525,120,121,123,32,380,382,363,552,340,130,131,133,47,385,387,350,,,60,61,63,32,380,340,75,76,78,47,385,350,518,516,,,210,213,610,613,,,376,381,375,416,446,476,555,352,621,342,404,622,401,623,624,549,627,547,318,317,274,253,290,250,,,215,371,229,227,226,223,222,475,474,228,761,83,142,313,375,235,314,310,233,306,234,236,312,309,79,485,484,755,444,334,127,,,240,250,234,,,761,766,431,599,548,598,595,543,594,436,656,,,314,567,313,321,315,303,324,327,715,,
    lString = aTxtStream.readLine();
    RemoveCommaAtTheEnd(lString);
    lStrCells = lString.split("," , QString::KeepEmptyParts);
    if (mTotalParameters != lStrCells.size() - mParameterOffset)
    {
        mLastErrorMessage = QString("The number of test number %1 is different than the number of tests number %2.")
                            .arg(mTotalParameters)
                            .arg(lStrCells.size() - mParameterOffset);
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());
        mLastError = errInvalidFormatParameter;
        return false;
    }

    for (int i = 0; i < mTotalParameters; ++i)
    {
        mTestsList[i].SetTestNumber(lStrCells[mParameterOffset + i].toLong());
    }

    // Read the units list
    // ,,,,Units,,Ohms/Sq,Ratio,V,uA/um,nA/um,V,V,fF/um,fF/um2,um,pS/Stage,nm,uA/um,nA/um,V,V,fF/um,fF/um2,nm,,,uA/um,nA/um,V,V,fF/um,nm,uA/um,nA/um,V,V,fF/um,nm,Ohms/Sq,Ohms/Sq,,,uA/um,V,uA/um,V,,,Ohms/CA,Ohms/CA,Ohms/CA,Ohms/Via,Ohms/Via,Ohms/Via,Ohms/Via,Percent,Ohms/um,Percent,Percent,Ohms/um,Percent,Ohms/um,Ohms/um,Percent,Ohms/um,Percent,Percent,Percent,Ohms/Sq,Ohms/Sq,Ohms/Sq,Ohms/Sq,,,V,V,V,V,Unit,Ratio,V,fF/um2,fF/um2,V,Percent,Ohms/Sq,Unit,V,V,V,V,Unit,nA,Ratio,uA,Ratio,Ohms,V,Ohms,fF/um2,fF/um2,Percent,Ohms,V,Ohms/Sq,,,V,V,V,,,Ohms/Via,Ohms/Via,fF/um2,Percent,Ohms/Sq,Percent,Percent,Ohms/Sq,Percent,fF/um2,Ohms/Sq,,,V,V,V,Unit,V,V,Ratio,Ohms,Percent,,
    lString = aTxtStream.readLine();
    RemoveCommaAtTheEnd(lString);
    lStrCells = lString.split("," , QString::KeepEmptyParts);
    if (mTotalParameters != lStrCells.size() - mParameterOffset)
    {
        mLastErrorMessage = QString("The number of units %1 is different than the number of tests unit %2.")
                            .arg(mTotalParameters)
                            .arg(lStrCells.size() - mParameterOffset);
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());
        mLastError = errInvalidFormatParameter;
        return false;
    }

    for (int i = 0; i < mTotalParameters; ++i)
    {
        mTestsList[i].SetTestUnit(lStrCells[mParameterOffset + i]);
    }

    // Read until the end of the file to get the limit // SPEC HIGH,,,,,,650,1.016,,,25,25,
    do
    {
        lString = aTxtStream.readLine();
    }
    while (!aTxtStream.atEnd() && !lString.contains(cSpecHigh, Qt::CaseInsensitive));

    if (aTxtStream.atEnd())
    {
        mLastErrorMessage = QString("No high spec limit on the file");
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());
        mLastError = errInvalidFormatParameter;
        return false;
    }

    bool lOK(false);
    RemoveCommaAtTheEnd(lString);
    lStrCells = lString.split(",");
    if (mTotalParameters != lStrCells.size() - mParameterOffset)
    {
        mLastErrorMessage = QString("The number of units %1 is different than the number of high limits %2.")
                            .arg(mTotalParameters)
                            .arg(lStrCells.size() - mParameterOffset);
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());
        mLastError = errInvalidFormatParameter;
        return false;
    }

    QString lLimit;
    for (int i = 0; i < mTotalParameters; ++i)
    {
        lLimit = lStrCells.at(i + mParameterOffset);
        if (!lLimit.isEmpty())
        {
            mTestsList[i].SetHighLimit(lLimit.toDouble(&lOK));
            mTestsList[i].SetValidHighLimit(lOK);
            if (!lOK)
            {
                mLastErrorMessage = QString("The limit: %1 is not a number").arg(lStrCells.at(i+ mParameterOffset));
                GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());
                mLastError = errInvalidFormatParameter;
                return false;
            }
        }
    }
    lString = aTxtStream.readLine();
    if (!lString.contains(cSpecLow, Qt::CaseInsensitive))
    {
        mLastErrorMessage = QString("No low spec limit on the file");
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());
        mLastError = errInvalidFormatParameter;
        return false;
    }

    RemoveCommaAtTheEnd(lString);
    lStrCells = lString.split(",");
    if (mTotalParameters != lStrCells.size() - mParameterOffset)
    {
        mLastErrorMessage = QString("The number of units %1 is different than the number of low limits %2.")
                            .arg(mTotalParameters)
                            .arg(lStrCells.size() - mParameterOffset);
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());
        mLastError = errInvalidFormatParameter;
        return false;
    }

    for (int i = 0; i < mTotalParameters; ++i)
    {
        lLimit = lStrCells.at(i + mParameterOffset);
        if (!lLimit.isEmpty())
        {
            mTestsList[i].SetLowLimit(lLimit.toDouble(&lOK));
            mTestsList[i].SetValidLowLimit(lOK);
            if (!lOK)
            {
                mLastErrorMessage = QString("The limit: %1 is not a number").arg(lStrCells.at(i));
                GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());
                mLastError = errInvalidFormatParameter;
                return false;
            }
        }
    }

    /// Read and write an STDF file for each PNP.
    /// Do like if we have only a file that conatins the first columns (Wafer .. SiteID) and the tests
    /// We do like if we have a file that contain only the a PNP
    for (int i=0; i<mListTestPrograms.size(); ++i)
    {
        QString lOutputFile = QFileInfo(aOutputSTDF).absolutePath() + QDir::separator();
        lOutputFile += QFileInfo(aOutputSTDF).baseName() + "_" + mListTestPrograms[i].second;
        lOutputFile += "." + QFileInfo(aOutputSTDF).completeSuffix();
        mOutputFiles.append(lOutputFile);

        if (!WriteTestProgram(i, aTxtStream, lOutputFile))
        {
            RemoveOutputFiles();
            return false;
        }
    }
    return true;
}

bool WatGlobalFoundryEtesttoSTDF::WriteTestProgram(int aPNPIndex, QTextStream& aTxtStream, const QString &aOutputSTDF)
{
    QString lString;
    QStringList lStrCells;

    GQTL_STDF::Stdf_PIR_V4 lPIRRecord;
    GQTL_STDF::Stdf_PRR_V4 lPRRRecord;
    GQTL_STDF::Stdf_DTR_V4 lDTRRecord;

    bool lFirstWafer(true);
    unsigned int lPartCount(1);
    int lPassBin(0), lFailBin(0);
    mMIRRecord.SetJOB_NAM(mListTestPrograms[aPNPIndex].second);

    // Write Test results for each line read.
    QStringList lFields;

    if(mStdfParse.Open(aOutputSTDF.toLatin1().constData(),STDF_WRITE) == false)
    {
        // Failed importing TriQuintDC file into STDF database
        mLastError = errWriteSTDF;
        mLastErrorMessage = QString ("can't open the file: %1 for writing").arg(aOutputSTDF);
        QFile::remove(aOutputSTDF);
        // Convertion failed.
        return false;
    }

    // Return to the beginning of the input file
    aTxtStream.seek(0);

    do
    {
        lString = aTxtStream.readLine();
    }
    while (!aTxtStream.atEnd() && !lString.contains(cPasFail, Qt::CaseInsensitive));
    if (aTxtStream.atEnd())
    {
        // This test is done above, it is here just to check
        return false;
    }

    do
    {
        lString = aTxtStream.readLine();
    }
    while (!aTxtStream.atEnd() && !lString.contains(cUnits, Qt::CaseInsensitive));

    if (aTxtStream.atEnd())
    {
        // This test is done above, it is here just to check
        return false;
    }

    // Write ATR
    mStdfParse.WriteRecord(&mATRRecord);

    // Write MIR
    mStdfParse.WriteRecord(&mMIRRecord);

    // Write DTR for PnpIndex: DTR:{"KEY": "PnpIndex", "TYPE": "MD","VALUE": "1"}
    lDTRRecord.Reset();
    AddMetaDataToDTR(cPNPIndex, QString::number(aPNPIndex), &lDTRRecord);
    mStdfParse.WriteRecord(&lDTRRecord);

    // Write WCR/WIR of new Wafer.
    QString lOrientation = mFlatOrientations[aPNPIndex].simplified().trimmed();
    char lOrientationChar;
    if (lOrientation.isEmpty())
    {
        lOrientationChar =' ';
    }
    else
    {
        lOrientationChar = lOrientation.at(0).toLatin1();
    }
    mWCRRecord.SetWF_FLAT(lOrientationChar);

    int lFirstResultIndex = mListTestPrograms[aPNPIndex].first;
    int lLastResultIndex;
    // if last item, take the last index of the file
    if (mListTestPrograms.size() == (aPNPIndex + 1))
    {
        RemoveCommaAtTheEnd(lString);
        lStrCells = lString.split(",");
        lLastResultIndex = lStrCells.size();
    }
    else
    {
        lLastResultIndex = (mListTestPrograms[aPNPIndex+1]).first;
    }

    short lShifColumnForSlotID(0);
    if (mExistSlotID)
    {
        lShifColumnForSlotID = 1;
    }

    /// Read results
    QString lStrString, lWaferID, lPreviousWaferID;
    // Write all Parameters read on this file : WIR,PIR,PTR...,PRR,WRR WIR,PIR,PTR..., PRR
    while (!aTxtStream.atEnd())
    {
        lStrString = ReadLine(aTxtStream).simplified();
        if (lStrString.isEmpty() || lStrString.startsWith("max", Qt::CaseInsensitive))
        {
            break;
        }

        lFields = lStrString.split(',');
        if (lFields.size() < lLastResultIndex)
        {
            mStdfParse.Close();
            mLastError = errInvalidFormatParameter;
            mLastErrorMessage = QString("The line %1 doesn't have enough parameters. It has to have at least %2 parameters")
                                .arg(lStrString).arg(lLastResultIndex);
            GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());
            return false;
        }

        lWaferID = lFields[0];

        bool lFail(false);
        int lSiteNum = lFields[lShifColumnForSlotID + 2].toShort();;
        for (int lResultIndex=(lFirstResultIndex+2); lResultIndex < lLastResultIndex; ++lResultIndex)
        {
            QString lValueString = lFields[lResultIndex];
            // Don't write a PTR if it is empty
            if (lValueString.isEmpty())
            {
                continue;
            }
            else if (lResultIndex == (lFirstResultIndex+2))
            {
                // Create a new WIR/WRR section if we find a new wafer ID
                if (lWaferID != lPreviousWaferID && lFirstWafer == false)
                {
                    // close the previous WRR
                    mWRRRecord.SetPART_CNT(lPartCount - 1);
                    mWRRRecord.SetWAFER_ID(lPreviousWaferID);
                    mStdfParse.WriteRecord(&mWRRRecord);

                    // Open the new WIR
                    mWIRRecord.SetWAFER_ID(lWaferID);
                    mStdfParse.WriteRecord(&mWIRRecord);

                    // Write a SlotId if the column exists
                    if (mExistSlotID)
                    {
                        lDTRRecord.Reset();
                        AddMetaDataToDTR(cSlotId, lFields[1], &lDTRRecord);
                        mStdfParse.WriteRecord(&lDTRRecord);
                    }
                    lPreviousWaferID = lWaferID;
                    // For each wafer, have to write limit in the first PTR
                    for(int lIndex=0; lIndex<mTotalParameters; ++lIndex)
                    {
                        mTestsList[lIndex].SetStaticHeaderWritten(false);
                    }
                    lPartCount = 1;
                }

                if (lFirstWafer == true)
                {
                    lFirstWafer = false;
                    // Open the new WIR
                    mStdfParse.WriteRecord(&mWCRRecord);
                    mWIRRecord.SetWAFER_ID(lWaferID);
                    mStdfParse.WriteRecord(&mWIRRecord);

                    lPreviousWaferID = lWaferID;

                    // Write a SlotId if the column exists
                    if (mExistSlotID)
                    {
                        lDTRRecord.Reset();
                        AddMetaDataToDTR(cSlotId, lFields[1], &lDTRRecord);
                        mStdfParse.WriteRecord(&lDTRRecord);
                    }
                }


                lPRRRecord.SetX_COORD(lFields[lFirstResultIndex].toDouble());
                lPRRRecord.SetY_COORD(lFields[lFirstResultIndex + 1].toDouble());
                lPRRRecord.SetSITE_NUM(lSiteNum);
                lPIRRecord.SetSITE_NUM(lSiteNum);
                lPIRRecord.SetHEAD_NUM(1);
                mStdfParse.WriteRecord(&lPIRRecord);
            }

            GQTL_STDF::Stdf_PTR_V4 lPTRRecord;
            ParserParameter& lTest = mTestsList[lResultIndex - mParameterOffset];
            lPTRRecord.SetTEST_NUM(lTest.GetTestNumber());
            lPTRRecord.SetHEAD_NUM(1);
            lPTRRecord.SetSITE_NUM(lSiteNum);

            gsfloat64 lValue = lValueString.toDouble();
            lPTRRecord.SetRESULT(lValue);
            if ((lTest.GetValidLowLimit() && (lTest.GetLowLimit() >= lValue))
                    || (lTest.GetValidHighLimit() && (lValue >= lTest.GetHighLimit())))
            {
                lFail = true;
                lPTRRecord.SetTEST_FLG(stdf_type_b1(0x80));
            }

            lTest.IncrementExecTest();
            if (!lTest.GetStaticHeaderWritten())
            {
                lPTRRecord.SetTEST_TXT(lTest.GetTestName());
                gsuchar lOptFlg = 0x02;
                if (lTest.GetValidLowLimit())
                {
                    lPTRRecord.SetLO_LIMIT(lTest.GetLowLimit());// R*4 Low test limit value OPT_FLAGbit 4 or 6 = 1
                }
                else
                {
                    lOptFlg |= 0x50;
                }
                if (lTest.GetValidHighLimit())
                {
                    lPTRRecord.SetHI_LIMIT(lTest.GetHighLimit());// R*4 High test limit value OPT_FLAGbit 5 or 7 = 1
                }
                else
                {
                    lOptFlg |= 0xA0;
                }

                lPTRRecord.SetUNITS(lTest.GetTestUnits());
                // No valid spec limits
                lOptFlg |= 0x0E;
                lPTRRecord.SetPARM_FLG(stdf_type_b1(0x0));

                lPTRRecord.SetOPT_FLAG(lOptFlg);
                lTest.SetStaticHeaderWritten(true);
            }

            mStdfParse.WriteRecord(&lPTRRecord);
        }
        if (lFail == true)
        {
            lPRRRecord.SetPART_FLG(0x08);
            lPRRRecord.SetSOFT_BIN(1);
            lPRRRecord.SetHARD_BIN(1);
            ++lFailBin;
        }
        else
        {
            lPRRRecord.SetPART_FLG(0);
            lPRRRecord.SetSOFT_BIN(0);
            lPRRRecord.SetHARD_BIN(0);
            ++lPassBin;
        }
        lPRRRecord.SetNUM_TEST(lLastResultIndex - (lFirstResultIndex+2));
        lPRRRecord.SetPART_ID(QString::number(lPartCount));
        ++lPartCount;
        mStdfParse.WriteRecord(&lPRRRecord);
    }

    mWRRRecord.SetWAFER_ID(lPreviousWaferID);
    mWRRRecord.SetPART_CNT(lPartCount - 1);
    mStdfParse.WriteRecord(&mWRRRecord);

    GQTL_STDF::Stdf_SBR_V4 lSBRRecord;
    lSBRRecord.SetHEAD_NUM(255);
    lSBRRecord.SetSBIN_NAM("PASS");
    lSBRRecord.SetSITE_NUM(1);
    lSBRRecord.SetSBIN_NUM(0);
    lSBRRecord.SetSBIN_CNT(lPassBin);
    lSBRRecord.SetSBIN_PF('P');
    mStdfParse.WriteRecord(&lSBRRecord);

    lSBRRecord.SetSBIN_NUM(1);
    lSBRRecord.SetSBIN_NAM("FAIL");
    lSBRRecord.SetSBIN_CNT(lFailBin);
    lSBRRecord.SetSBIN_PF('F');
    mStdfParse.WriteRecord(&lSBRRecord);

    GQTL_STDF::Stdf_HBR_V4 lHBRRecord;
    lHBRRecord.SetHEAD_NUM(255);
    lHBRRecord.SetSITE_NUM(1);
    lHBRRecord.SetHBIN_NAM("PASS");
    lHBRRecord.SetHBIN_NUM(0);
    lHBRRecord.SetHBIN_CNT(lPassBin);
    lHBRRecord.SetHBIN_PF('P');
    mStdfParse.WriteRecord(&lHBRRecord);

    lHBRRecord.SetHBIN_NUM(1);
    lHBRRecord.SetHBIN_CNT(lFailBin);
    lHBRRecord.SetHBIN_NAM("FAIL");
    lHBRRecord.SetHBIN_PF('F');
    mStdfParse.WriteRecord(&lHBRRecord);

    mStdfParse.WriteRecord(&mMRRRecord);
    mStdfParse.Close();
    // Success
    return true;
}

//////////////////////////////////////////////////////////////////////
// Convert 'FileName' SiTime file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool WatGlobalFoundryEtesttoSTDF::ConvertoStdf(const QString &aInputFileName, QString &aStdfFileName)
{
    // No erro (default)
    mLastError = errNoError;

    // Open log2 file
    QFile lFile(aInputFileName);
    if(!lFile.open( QIODevice::ReadOnly ))
    {
        // Failed Opening CSV file
        mLastError = errOpenFail;
        mLastErrorMessage = "Unable to open file " + aInputFileName;

        // Convertion failed.
        return false;
    }

    // Assign file I/O stream
    QTextStream lTxtStream(&lFile);

    // Process Header section
    if (ProcessHeader(lTxtStream) == false)
    {
        lFile.close();
        return false;
    }

    if(ReadWatGlobalFoundryFile(lTxtStream, aStdfFileName) != true)
    {
        lFile.close();
        return false;	// Error reading SiTime file
    }

    // Convertion successful
    return true;
}

void WatGlobalFoundryEtesttoSTDF::RemoveCommaAtTheEnd(QString& aString)
{
    while (aString.endsWith(","))
    {
        aString = aString.left(aString.size() - 1);
    }
}


}
}
