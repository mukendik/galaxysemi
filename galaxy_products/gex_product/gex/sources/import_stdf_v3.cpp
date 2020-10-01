//////////////////////////////////////////////////////////////////////
// import_csmc.cpp: Convert a STDF_V3 file to STDF V4.0
//////////////////////////////////////////////////////////////////////

#include <math.h>
#include <qmath.h>
#include <time.h>

#ifdef _WIN32
#include "windows.h"		// For 'GetWindowsDirectoryA' declaration
#endif

#include <qfileinfo.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlabel.h>
#include <QTextStream>
#include <QListIterator>

#include "engine.h"
#include "import_stdf_v3.h"
#include "import_constants.h"
#include "gex_algorithms.h"
#include "gqtl_log.h"

// main.cpp
extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar	*	GexProgressBar;		// Handle to progress bar in status bar


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGSTDFV3toSTDF::CGSTDFV3toSTDF()
{
    m_lStartTime = 0;
    mPTRList.clear();
    mPDRList.clear();
    mFDRList.clear();
    mFARWritten = false;
    mNbrePart = 0;
    mSBinPassFail.clear();
    mHBinPassFail.clear();
    mTSRTestTyp.clear();
}

CGSTDFV3toSTDF::~CGSTDFV3toSTDF()
{
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGSTDFV3toSTDF::GetLastError()
{
    mStrLastError = "Import STDF_V3: ";

    switch(mIntLastError)
    {
        default:
        case errNoError:
            mStrLastError += "No Error";
            break;
        case errOpenFail:
            mStrLastError += "Failed to open file";
            break;
        case errInvalidFormat:
            mStrLastError += "Invalid file format";
            break;
        case errInvalidFormatLowInRows:
            mStrLastError += "Invalid file format: 'Parameter' line too short, missing rows";
            break;
        case errWriteSTDF:
            mStrLastError += "Failed creating temporary file. Folder permission issue?";
            break;
        case errLicenceExpired:
            mStrLastError += "License has expired or Data file out of date...";
            break;
    }
    // Return Error Message
    return mStrLastError;
}



//////////////////////////////////////////////////////////////////////
// Convert 'FileName' STDF V3 file, to STDF V4 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGSTDFV3toSTDF::Convert(const char *Stdf_V3FileName, const char *strFileNameSTDFV4)
{
    // No erro (default)
    mIntLastError = errNoError;

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo fInput(Stdf_V3FileName);
    QFileInfo fOutput(strFileNameSTDFV4);

    QFile f( Stdf_V3FileName );
    if((f.exists() == true) && (fInput.lastModified() < fOutput.lastModified()))
        return true;

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    bool bHideProgressAfter=true;
    bool bHideLabelAfter=false;


    if(GexScriptStatusLabel != NULL)
    {
        if(GexScriptStatusLabel->isHidden())
        {
            bHideLabelAfter = true;
            GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(Stdf_V3FileName).fileName()+"...");
            GexScriptStatusLabel->show();
        }
    }

    if(GexProgressBar != NULL)
    {
        bHideProgressAfter = GexProgressBar->isHidden();
        GexProgressBar->setMaximum(200);
        GexProgressBar->setTextVisible(true);
        GexProgressBar->setValue(0);
        GexProgressBar->show();
    }
    QCoreApplication::processEvents();

    // Open STDF file to read...
    if(mStdfParse.Open(Stdf_V3FileName) == false)
    {
        // Error. Can't open STDF file in read mode!
        GSLOG(4, QString("Cannot open STDF '%1'").arg(Stdf_V3FileName).toLatin1().data() );
        return false;
    }

    int lRecordType;
    int lStatus;

    if(mStdfFile.Open((char*)strFileNameSTDFV4,STDF_WRITE) != GS::StdLib::Stdf::NoError)
    {
        // Failed importing CSV file into STDF database
        mIntLastError = errWriteSTDF;

        // Convertion failed.
        return false;
    }
    // Read one record from STDF file.
    lStatus = mStdfParse.LoadNextRecord(&lRecordType);
    while((lStatus == GQTL_STDF::StdfParse::NoError) || (lStatus == GQTL_STDF::StdfParse::UnexpectedRecord))
    {
//        m_nStdtTotalRecords++;
//        bool bStopDump = false;

        // Process STDF record read.
        switch(lRecordType)
        {
            case GQTL_STDF::Stdf_Record::Rec_FAR:
                if(ReadWriteFAR(mStdfV3FAR) != true)
                {
                    GSLOG(4, QString("Processing FAR failed").toLatin1().data() );
                    // File timestamp is invalid, and higher than expiration date!
                    // Close STDF file
                    mStdfParse.Close();
                    mIntLastError = errInvalidFormat;
                    return false;
                }
                break;
            case GQTL_STDF::Stdf_Record::Rec_MIR:
                if(ReadWriteMIR(mStdfV3MIR) != true)
                {
                    GSLOG(4, QString("Processing MIR failed").toLatin1().data() );
                    // File timestamp is invalid, and higher than expiration date!
                    // Close STDF file
                    mStdfParse.Close();
                    mIntLastError = errInvalidFormat;
                    return false;
                }
                break;
            case GQTL_STDF::Stdf_Record::Rec_MRR:
                if(ReadWriteMRR(mStdfV3MRR) != true)
                {
                    GSLOG(4, QString("Processing MRR failed").toLatin1().data() );
                    // File timestamp is invalid, and higher than expiration date!
                    // Close STDF file
                    mStdfParse.Close();
                    mIntLastError = errInvalidFormat;
                    return false;
                }
                break;
            case GQTL_STDF::Stdf_Record::Rec_HBR:
                if(ReadWriteHBR(mStdfV3HBR) != true)
                {
                    GSLOG(4, QString("Processing HBR failed").toLatin1().data() );
                    // File timestamp is invalid, and higher than expiration date!
                    // Close STDF file
                    mStdfParse.Close();
                    mIntLastError = errInvalidFormat;
                    return false;
                }
                break;
            case GQTL_STDF::Stdf_Record::Rec_SBR:
                if(ReadWriteSBR(mStdfV3SBR) != true)
                {
                    GSLOG(4, QString("Processing SBR failed").toLatin1().data() );
                    // File timestamp is invalid, and higher than expiration date!
                    // Close STDF file
                    mStdfParse.Close();
                    mIntLastError = errInvalidFormat;
                    return false;
                }
                break;
            case GQTL_STDF::Stdf_Record::Rec_PMR:
                if(ReadPMR(mStdfV3PMR) != true)
                {
                    GSLOG(4, QString("Processing PMR failed").toLatin1().data() );
                    // File timestamp is invalid, and higher than expiration date!
                    // Close STDF file
                    mStdfParse.Close();
                    mIntLastError = errInvalidFormat;
                    return false;
                }
                break;
            case GQTL_STDF::Stdf_Record::Rec_WIR:
                if(ReadWriteWIR(mStdfV3WIR) != true)
                {
                    GSLOG(4, QString("Processing WIR failed").toLatin1().data() );
                    // File timestamp is invalid, and higher than expiration date!
                    // Close STDF file
                    mStdfParse.Close();
                    mIntLastError = errInvalidFormat;
                    return false;
                }
                break;
            case GQTL_STDF::Stdf_Record::Rec_WRR:
                if(ReadWriteWRR(mStdfV3WRR) != true)
                {
                    GSLOG(4, QString("Processing WRR failed").toLatin1().data() );
                    // File timestamp is invalid, and higher than expiration date!
                    // Close STDF file
                    mStdfParse.Close();
                    mIntLastError = errInvalidFormat;
                    return false;
                }
                break;
            case GQTL_STDF::Stdf_Record::Rec_WCR:
                if(ReadWriteWCR(mStdfV3WCR) != true)
                {
                    GSLOG(4, QString("Processing WCR failed").toLatin1().data() );
                    // File timestamp is invalid, and higher than expiration date!
                    // Close STDF file
                    mStdfParse.Close();
                    mIntLastError = errInvalidFormat;
                    return false;
                }
                break;
            case GQTL_STDF::Stdf_Record::Rec_PIR:
                if(ReadWritePIR(mStdfV3PIR) != true)
                {
                    GSLOG(4, QString("Processing PIR failed").toLatin1().data() );
                    // File timestamp is invalid, and higher than expiration date!
                    // Close STDF file
                    mStdfParse.Close();
                    mIntLastError = errInvalidFormat;
                    return false;
                }
                break;
            case GQTL_STDF::Stdf_Record::Rec_PRR:
                if(ReadWritePRR(mStdfV3PRR) != true)
                {
                    GSLOG(4, QString("Processing PRR failed").toLatin1().data() );
                    // File timestamp is invalid, and higher than expiration date!
                    // Close STDF file
                    mStdfParse.Close();
                    mIntLastError = errInvalidFormat;
                    return false;
                }
                break;
            case GQTL_STDF::Stdf_Record::Rec_PDR:
                if(ReadPDR(mStdfV3PDR) != true)
                {
                    GSLOG(4, QString("Processing PDR failed").toLatin1().data() );
                    // File timestamp is invalid, and higher than expiration date!
                    // Close STDF file
                    mStdfParse.Close();
                    mIntLastError = errInvalidFormat;
                    return false;
                }
                break;
            case GQTL_STDF::Stdf_Record::Rec_FDR:
                if(ReadFDR(mStdfV3FDR) != true)
                {
                    GSLOG(4, QString("Processing FDR failed").toLatin1().data() );
                    // File timestamp is invalid, and higher than expiration date!
                    // Close STDF file
                    mStdfParse.Close();
                    mIntLastError = errInvalidFormat;
                    return false;
                }
                break;
            case GQTL_STDF::Stdf_Record::Rec_TSR:
                if(ReadWriteTSR(mStdfV3TSR) != true)
                {
                    GSLOG(4, QString("Processing TSR failed").toLatin1().data() );
                    // File timestamp is invalid, and higher than expiration date!
                    // Close STDF file
                    mStdfParse.Close();
                    mIntLastError = errInvalidFormat;
                    return false;
                }
                break;
            case GQTL_STDF::Stdf_Record::Rec_PTR:
                if(ReadWritePTR(mStdfV3PTR) != true)
                {
                    GSLOG(4, QString("Processing PTR failed").toLatin1().data() );
                    // File timestamp is invalid, and higher than expiration date!
                    // Close STDF file
                    mStdfParse.Close();
                    mIntLastError = errInvalidFormat;
                    return false;
                }
                break;
            case GQTL_STDF::Stdf_Record::Rec_FTR:
                if(ReadWriteFTR(mStdfV3FTR) != true)
                {
                    GSLOG(4, QString("Processing FTR failed").toLatin1().data() );
                    // File timestamp is invalid, and higher than expiration date!
                    // Close STDF file
                    mStdfParse.Close();
                    mIntLastError = errInvalidFormat;
                    return false;
                }
                break;
            case GQTL_STDF::Stdf_Record::Rec_BPS:
                if(ReadWriteBPS(mStdfV3BPS) != true)
                {
                    GSLOG(4, QString("Processing BPS failed").toLatin1().data() );
                    // File timestamp is invalid, and higher than expiration date!
                    // Close STDF file
                    mStdfParse.Close();
                    mIntLastError = errInvalidFormat;
                    return false;
                }
                break;
            case GQTL_STDF::Stdf_Record::Rec_EPS:
                if(ReadWriteEPS(mStdfV3EPS) != true)
                {
                    GSLOG(4, QString("Processing EPS failed").toLatin1().data() );
                    // File timestamp is invalid, and higher than expiration date!
                    // Close STDF file
                    mStdfParse.Close();
                    mIntLastError = errInvalidFormat;
                    return false;
                }
                break;
            case GQTL_STDF::Stdf_Record::Rec_SHB:
                if(ReadWriteSHB(mStdfV3SHB) != true)
                {
                    GSLOG(4, QString("Processing SHB failed").toLatin1().data() );
                    // File timestamp is invalid, and higher than expiration date!
                    // Close STDF file
                    mStdfParse.Close();
                    mIntLastError = errInvalidFormat;
                    return false;
                }
                break;
            case GQTL_STDF::Stdf_Record::Rec_SSB:
                if(ReadSSBWriteSSB(mStdfV3SSB) != true)
                {
                    GSLOG(4, QString("Processing SSB failed").toLatin1().data() );
                    // File timestamp is invalid, and higher than expiration date!
                    // Close STDF file
                    mStdfParse.Close();
                    mIntLastError = errInvalidFormat;
                    return false;
                }
                break;
            case GQTL_STDF::Stdf_Record::Rec_STS:
                if(ReadWriteSTS(mStdfV3STS) != true)
                {
                    GSLOG(4, QString("Processing STS failed").toLatin1().data() );
                    // File timestamp is invalid, and higher than expiration date!
                    // Close STDF file
                    mStdfParse.Close();
                    mIntLastError = errInvalidFormat;
                    return false;
                }
                break;
            case GQTL_STDF::Stdf_Record::Rec_SCR:
                if(ReadSCRWritePCR(mStdfV3SCR) != true)
                {
                    GSLOG(4, QString("Processing SCR failed").toLatin1().data() );
                    // File timestamp is invalid, and higher than expiration date!
                    // Close STDF file
                    mStdfParse.Close();
                    mIntLastError = errInvalidFormat;
                    return false;
                }
                break;
            case GQTL_STDF::Stdf_Record::Rec_GDR:
                if(ReadWriteGDR(mStdfV3GDR) != true)
                {
                    GSLOG(4, QString("Processing GDR failed").toLatin1().data() );
                    // File timestamp is invalid, and higher than expiration date!
                    // Close STDF file
                    mStdfParse.Close();
                    mIntLastError = errInvalidFormat;
                    return false;
                }
                break;
            case GQTL_STDF::Stdf_Record::Rec_DTR:
                if(ReadWriteDTR(mStdfV3DTR) != true)
                {
                    GSLOG(4, QString("Processing DTR failed").toLatin1().data() );
                    // File timestamp is invalid, and higher than expiration date!
                    // Close STDF file
                    mStdfParse.Close();
                    mIntLastError = errInvalidFormat;
                    return false;
                }
                break;
            default:
                mIntLastError = errInvalidFormat;
        }

        // Check if dump should be stopped
//        if(bStopDump == true)
//        {
//            // Force progress bar to completed
//            ResetProgress(true);
//            // Close STDF file
//            mStdfParse.Close();
//            // Close ATDF files
//            CloseAtdfFiles();
//            return true;
//        }

        // Read one record from STDF file.
        lStatus = mStdfParse.LoadNextRecord(&lRecordType);
    };

    // Close input STDF file between passes.
    mStdfParse.Close();
    mStdfFile.Close();

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


bool CGSTDFV3toSTDF::ReadWriteFAR(GQTL_STDF::Stdf_FAR_V3& record)
{
    // If the FAR is not the first record, return false
    if (mFARWritten == true)
    {
        GSLOG(4, QString("The FAR have to be the first record").toLatin1().data() );
        return false;
    }
    else
        mFARWritten = true;

    // The same FAR
    if (!mStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*)&record))
    {
        return false;
    }
    GQTL_STDF::Stdf_FAR_V4 lFarV4Record;
    if (record.IsFieldPresent(GQTL_STDF::Stdf_FAR_V3::eposCPU_TYPE))
        lFarV4Record.SetCPU_TYPE(record.m_u1CPU_TYPE);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_FAR_V3::eposSTDF_VER))
        lFarV4Record.SetSTDF_VER(record.m_u1STDF_VER);
    lFarV4Record.Write(mStdfFile);
    return true;
}


bool CGSTDFV3toSTDF::ReadWriteMIR(GQTL_STDF::Stdf_MIR_V3& record)
{
    // Some differences
    if (!mStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*)&record))
    {
        return false;
    }

    // Create also a Far if doesn't exist
    if (mFARWritten == false)
    {
        GQTL_STDF::Stdf_FAR_V4 lFarV4Record;
        if (record.IsFieldPresent(GQTL_STDF::Stdf_MIR_V3::eposCPU_TYPE))
            lFarV4Record.SetCPU_TYPE(record.m_u1CPU_TYPE);
        if (record.IsFieldPresent(GQTL_STDF::Stdf_MIR_V3::eposSTDF_VER))
            lFarV4Record.SetSTDF_VER(4);
        lFarV4Record.Write(mStdfFile);
        mFARWritten = true;
    }

    GQTL_STDF::Stdf_MIR_V4 lMIRV4Record;
    if (record.IsFieldPresent(GQTL_STDF::Stdf_MIR_V3::eposSETUP_T))
        lMIRV4Record.SetSETUP_T(record.m_u4SETUP_T);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_MIR_V3::eposSTART_T))
        lMIRV4Record.SetSTART_T(record.m_u4START_T);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_MIR_V3::eposSTAT_NUM))
        lMIRV4Record.SetSTAT_NUM(record.m_u1STAT_NUM);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_MIR_V3::eposMODE_COD))
        lMIRV4Record.SetMODE_COD(record.m_c1MODE_COD);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_MIR_V3::eposRTST_COD))
        lMIRV4Record.SetRTST_COD(record.m_c1RTST_COD);
    lMIRV4Record.SetBURN_TIM(65535);    // mandatory field in the stdf v4 and doesn't exist in stdf v3
    if (record.IsFieldPresent(GQTL_STDF::Stdf_MIR_V3::eposCMOD_COD))
        lMIRV4Record.SetCMOD_COD(record.m_c1PROT_COD);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_MIR_V3::eposLOT_ID))
        lMIRV4Record.SetLOT_ID(record.m_cnLOT_ID);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_MIR_V3::eposPART_TYP))
        lMIRV4Record.SetPART_TYP(record.m_cnPART_TYP);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_MIR_V3::eposNODE_NAM))
        lMIRV4Record.SetNODE_NAM(record.m_cnNODE_NAM);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_MIR_V3::eposTSTR_TYP))
        lMIRV4Record.SetTSTR_TYP(record.m_cnTSTR_TYP);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_MIR_V3::eposJOB_NAM))
        lMIRV4Record.SetJOB_NAM(record.m_cnJOB_NAM);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_MIR_V3::eposJOB_REV))
        lMIRV4Record.SetJOB_REV(record.m_cnJOB_REV);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_MIR_V3::eposSBLOT_ID))
        lMIRV4Record.SetSBLOT_ID(record.m_cnSBLOT_ID);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_MIR_V3::eposOPER_NAM))
        lMIRV4Record.SetOPER_NAM(record.m_cnOPER_NAM);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_MIR_V3::eposEXEC_TYP))
        lMIRV4Record.SetEXEC_TYP(record.m_cnEXEC_TYP);
    if (record.m_cnTEST_COD == "   ")
            lMIRV4Record.SetTEST_COD("");
    else
        if (record.IsFieldPresent(GQTL_STDF::Stdf_MIR_V3::eposTEST_COD))
            lMIRV4Record.SetTEST_COD(record.m_cnTEST_COD);
//        lMIRV4Record.SetTST_TEMP("");
//        lMIRV4Record.SetUSER_TXT("");
//        lMIRV4Record.SetAUX_FILE("");
//        lMIRV4Record.SetPKG_TYP("");
//        lMIRV4Record.SetFAMLY_ID("");
//        lMIRV4Record.SetDATE_COD("");
//        lMIRV4Record.SetFACIL_ID("");
//        lMIRV4Record.SetFLOOR_ID("");
    if (record.IsFieldPresent(GQTL_STDF::Stdf_MIR_V3::eposPROC_ID))
        lMIRV4Record.SetPROC_ID(record.m_cnPROC_ID);
//    if (record.IsFieldPresent(GQTL_STDF::Stdf_MIR_V3::eposOPER_FRQ))
//        lMIRV4Record.SetOPER_FRQ(record.m_cnOPER_FRQ);
    // The rest of fields are optional and didn't exist in the stdf v3. So, no utility to set them.
    lMIRV4Record.Write(mStdfFile);
    return true;
}


bool CGSTDFV3toSTDF::ReadWriteMRR(GQTL_STDF::Stdf_MRR_V3& record)
{
    // V3 has more items than V4
    if (!mStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*)&record))
    {
        return false;
    }

    // Write PCR before the MRR
    GQTL_STDF::Stdf_PCR_V4 lPCRV4Record;
    lPCRV4Record.SetHEAD_NUM(255);
    lPCRV4Record.SetSITE_NUM(1);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_MRR_V3::eposPART_CNT))
    lPCRV4Record.SetPART_CNT(record.m_u4PART_CNT);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_MRR_V3::eposRTST_CNT))
        lPCRV4Record.SetRTST_CNT(static_cast<stdf_type_u4>((record.m_i4RTST_CNT!=-1)?record.m_i4RTST_CNT:INVALID_INT));
    if (record.IsFieldPresent(GQTL_STDF::Stdf_MRR_V3::eposABRT_CNT))
        lPCRV4Record.SetABRT_CNT(static_cast<stdf_type_u4>((record.m_i4ABRT_CNT!=-1)?record.m_i4ABRT_CNT:INVALID_INT));
    if (record.IsFieldPresent(GQTL_STDF::Stdf_MRR_V3::eposGOOD_CNT))
        lPCRV4Record.SetGOOD_CNT(static_cast<stdf_type_u4>((record.m_i4GOOD_CNT!=-1)?record.m_i4GOOD_CNT:INVALID_INT));
    if (record.IsFieldPresent(GQTL_STDF::Stdf_MRR_V3::eposFUNC_CNT))
        lPCRV4Record.SetFUNC_CNT(static_cast<stdf_type_u4>((record.m_i4FUNC_CNT!=-1)?record.m_i4FUNC_CNT:INVALID_INT));
    lPCRV4Record.Write(mStdfFile);

    GQTL_STDF::Stdf_MRR_V4 lMRRV4Record;
    if (record.IsFieldPresent(GQTL_STDF::Stdf_MRR_V3::eposFINISH_T))
        lMRRV4Record.SetFINISH_T(record.m_u4FINISH_T);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_MRR_V3::eposDISP_COD))
        lMRRV4Record.SetDISP_COD(record.m_c1DISP_COD);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_MRR_V3::eposUSR_DESC))
        lMRRV4Record.SetUSR_DESC(record.m_cnUSR_DESC);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_MRR_V3::eposEXC_DESC))
        lMRRV4Record.SetEXC_DESC(record.m_cnEXC_DESC);
    lMRRV4Record.Write(mStdfFile);
    return true;
}


bool CGSTDFV3toSTDF::ReadWriteHBR(GQTL_STDF::Stdf_HBR_V3& record)
{
    // Merge it with the SHB to generate a V4's HBR
    if (!mStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*)&record))
    {
        return false;
    }
    GQTL_STDF::Stdf_HBR_V4 lHBRV4Record;
    lHBRV4Record.SetHEAD_NUM(255);
    lHBRV4Record.SetSITE_NUM(1);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_HBR_V3::eposHBIN_NUM))
        lHBRV4Record.SetHBIN_NUM(record.m_u2HBIN_NUM);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_HBR_V3::eposHBIN_CNT))
        lHBRV4Record.SetHBIN_CNT(record.m_u4HBIN_CNT);
    if ((record.IsFieldPresent(GQTL_STDF::Stdf_HBR_V3::eposHBIN_NUM))
        && (mHBinPassFail.contains(record.m_u2HBIN_NUM)))
            lHBRV4Record.SetHBIN_PF(mHBinPassFail.value(record.m_u2HBIN_NUM));
    else
        lHBRV4Record.SetHBIN_PF(' ');
    if (record.IsFieldPresent(GQTL_STDF::Stdf_HBR_V3::eposHBIN_NAM))
        lHBRV4Record.SetHBIN_NAM(record.m_cnHBIN_NAM);
    lHBRV4Record.Write(mStdfFile);
    return true;
}


bool CGSTDFV3toSTDF::ReadWriteSBR(GQTL_STDF::Stdf_SBR_V3& record)
{
    // Merge it with the SSB to generate a V4's SBR
    mStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*)&record);

    GQTL_STDF::Stdf_SBR_V4 lSBRV4Record;
    lSBRV4Record.SetHEAD_NUM(255);
    lSBRV4Record.SetSITE_NUM(1);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_SBR_V3::eposSBIN_NUM))
        lSBRV4Record.SetSBIN_NUM(record.m_u2SBIN_NUM);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_SBR_V3::eposSBIN_CNT))
        lSBRV4Record.SetSBIN_CNT(record.m_u4SBIN_CNT);
    if ((record.IsFieldPresent(GQTL_STDF::Stdf_SBR_V3::eposSBIN_NUM))
        && (mSBinPassFail.contains(record.m_u2SBIN_NUM)))
            lSBRV4Record.SetSBIN_PF(mSBinPassFail.value(record.m_u2SBIN_NUM));
    else
        lSBRV4Record.SetSBIN_PF(' ');
    if (record.IsFieldPresent(GQTL_STDF::Stdf_SBR_V3::eposSBIN_NAM))
        lSBRV4Record.SetSBIN_NAM(record.m_cnSBIN_NAM);
    lSBRV4Record.Write(mStdfFile);
    return true;
}

/// For the moment, do not generate a V4 PMR
bool CGSTDFV3toSTDF::ReadPMR(GQTL_STDF::Stdf_PMR_V3& record)
{
    // Not the same
    if (!mStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*)&record))
    {
        return false;
    }
    return true;
}


bool CGSTDFV3toSTDF::ReadWriteWIR(GQTL_STDF::Stdf_WIR_V3& record)
{
    // Very close
    if (!mStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*)&record))
    {
        return false;
    }
    GQTL_STDF::Stdf_WIR_V4 lWIRV4Record;
    if (record.IsFieldPresent(GQTL_STDF::Stdf_WIR_V3::eposHEAD_NUM))
        lWIRV4Record.SetHEAD_NUM(record.m_u1HEAD_NUM);
    lWIRV4Record.SetSITE_GRP(255);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_WIR_V3::eposSTART_T))
        lWIRV4Record.SetSTART_T(record.m_u4START_T);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_WIR_V3::eposWAFER_ID))
        lWIRV4Record.SetWAFER_ID(record.m_cnWAFER_ID);
    lWIRV4Record.Write(mStdfFile);
    return true;
}


bool CGSTDFV3toSTDF::ReadWriteWRR(GQTL_STDF::Stdf_WRR_V3& record)
{
    // Very close
    if (!mStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*)&record))
    {
        return false;
    }
    GQTL_STDF::Stdf_WRR_V4 lWRRV4Record;
    if (record.IsFieldPresent(GQTL_STDF::Stdf_WRR_V3::eposHEAD_NUM))
        lWRRV4Record.SetHEAD_NUM(record.m_u1HEAD_NUM);
    lWRRV4Record.SetSITE_GRP(255);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_WRR_V3::eposFINISH_T))
        lWRRV4Record.SetFINISH_T(record.m_u4FINISH_T);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_WRR_V3::eposPART_CNT))
        lWRRV4Record.SetPART_CNT(record.m_u4PART_CNT);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_WRR_V3::eposRTST_CNT))
        lWRRV4Record.SetRTST_CNT(static_cast<stdf_type_u4>((record.m_i4RTST_CNT!=-1)?record.m_i4RTST_CNT:INVALID_INT));
    if (record.IsFieldPresent(GQTL_STDF::Stdf_WRR_V3::eposABRT_CNT))
        lWRRV4Record.SetABRT_CNT(static_cast<stdf_type_u4>((record.m_i4ABRT_CNT!=-1)?record.m_i4ABRT_CNT:INVALID_INT));
    if (record.IsFieldPresent(GQTL_STDF::Stdf_WRR_V3::eposGOOD_CNT))
        lWRRV4Record.SetGOOD_CNT(static_cast<stdf_type_u4>((record.m_i4GOOD_CNT!=-1)?record.m_i4GOOD_CNT:INVALID_INT));
    if (record.IsFieldPresent(GQTL_STDF::Stdf_WRR_V3::eposFUNC_CNT))
        lWRRV4Record.SetFUNC_CNT(static_cast<stdf_type_u4>((record.m_i4FUNC_CNT!=-1)?record.m_i4FUNC_CNT:INVALID_INT));
    if (record.IsFieldPresent(GQTL_STDF::Stdf_WRR_V3::eposWAFER_ID))
        lWRRV4Record.SetWAFER_ID(record.m_cnWAFER_ID);
//        lWRRV4Record.SetFABWF_ID("");
//        lWRRV4Record.SetFRAME_ID("");
//        lWRRV4Record.SetMASK_ID("");
    if (record.IsFieldPresent(GQTL_STDF::Stdf_WRR_V3::eposUSR_DESC))
        lWRRV4Record.SetUSR_DESC(record.m_cnUSR_DESC);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_WRR_V3::eposEXC_DESC))
        lWRRV4Record.SetEXC_DESC(record.m_cnEXC_DESC);
    lWRRV4Record.Write(mStdfFile);
    return true;
}


bool CGSTDFV3toSTDF::ReadWriteWCR(GQTL_STDF::Stdf_WCR_V3& record)
{
    // The same
    if (!mStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*)&record))
    {
        return false;
    }
    GQTL_STDF::Stdf_WCR_V4 lWCRV4Record;
    if (record.IsFieldPresent(GQTL_STDF::Stdf_WCR_V3::eposWAFR_SIZ))
        lWCRV4Record.SetWAFR_SIZ(record.m_r4WAFR_SIZ);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_WCR_V3::eposDIE_HT))
        lWCRV4Record.SetDIE_HT(record.m_r4DIE_HT);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_WCR_V3::eposDIE_WID))
        lWCRV4Record.SetDIE_WID(record.m_r4DIE_WID);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_WCR_V3::eposWF_UNITS))
        lWCRV4Record.SetWF_UNITS(record.m_u1WF_UNITS);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_WCR_V3::eposWF_FLAT))
        lWCRV4Record.SetWF_FLAT(record.m_c1WF_FLAT);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_WCR_V3::eposCENTER_X))
        lWCRV4Record.SetCENTER_Y(record.m_i2CENTER_X);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_WCR_V3::eposCENTER_Y))
        lWCRV4Record.SetCENTER_Y(record.m_i2CENTER_Y);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_WCR_V3::eposPOS_X))
        lWCRV4Record.SetPOS_X(record.m_c1POS_X);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_WCR_V3::eposPOS_Y))
        lWCRV4Record.SetPOS_Y(record.m_c1POS_Y);
    lWCRV4Record.Write(mStdfFile);
    return true;
}


bool CGSTDFV3toSTDF::ReadWritePIR(GQTL_STDF::Stdf_PIR_V3& record)
{
    // Stdf V3 has more fields
    if (!mStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*)&record))
    {
        return false;
    }
    GQTL_STDF::Stdf_PIR_V4 lPIRV4Record;
    if (record.IsFieldPresent(GQTL_STDF::Stdf_PIR_V3::eposHEAD_NUM))
        lPIRV4Record.SetHEAD_NUM(record.m_u1HEAD_NUM);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_PIR_V3::eposSITE_NUM))
        lPIRV4Record.SetSITE_NUM(record.m_u1SITE_NUM);
    lPIRV4Record.Write(mStdfFile);
    return true;
}


bool CGSTDFV3toSTDF::ReadWritePRR(GQTL_STDF::Stdf_PRR_V3& record)
{
    // Similar but not the same
    if (!mStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*)&record))
    {
        return false;
    }
    GQTL_STDF::Stdf_PRR_V4 lPRRV4Record;
    if (record.IsFieldPresent(GQTL_STDF::Stdf_PRR_V3::eposHEAD_NUM))
        lPRRV4Record.SetHEAD_NUM(record.m_u1HEAD_NUM);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_PRR_V3::eposSITE_NUM))
        lPRRV4Record.SetSITE_NUM(record.m_u1SITE_NUM);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_PRR_V3::eposPART_FLG))
        lPRRV4Record.SetPART_FLG(record.m_b1PART_FLG);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_PRR_V3::eposNUM_TEST))
        lPRRV4Record.SetNUM_TEST(record.m_u2NUM_TEST);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_PRR_V3::eposHARD_BIN))
        lPRRV4Record.SetHARD_BIN(record.m_u2HARD_BIN);
    // In the SFTD v3, the default value is 0, in the stdf v4 it is 65535
    if (record.IsFieldPresent(GQTL_STDF::Stdf_PRR_V3::eposSOFT_BIN))
        lPRRV4Record.SetSOFT_BIN((record.m_u2SOFT_BIN != 0)?record.m_u2SOFT_BIN : STDF_MAX_U2);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_PRR_V3::eposX_COORD))
        lPRRV4Record.SetX_COORD(record.m_i2X_COORD);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_PRR_V3::eposY_COORD))
        lPRRV4Record.SetY_COORD(record.m_i2Y_COORD);
//    lPRRV4Record.SetTEST_T(0);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_PRR_V3::eposPART_ID))
        lPRRV4Record.SetPART_ID(record.m_cnPART_ID);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_PRR_V3::eposPART_TXT))
        lPRRV4Record.SetPART_TXT(record.m_cnPART_TXT);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_PRR_V3::eposPART_FIX))
    {
        lPRRV4Record.m_bnPART_FIX.m_bLength = record.m_bnPART_FIX.m_bLength;
        for (int i=0; i<lPRRV4Record.m_bnPART_FIX.m_bLength; ++i)
            lPRRV4Record.m_bnPART_FIX.m_pBitField[i] = record.m_bnPART_FIX.m_pBitField[i];
        lPRRV4Record.SetPART_FIX();
    }

    lPRRV4Record.Write(mStdfFile);

    // Add pass fail status to sbin and hbin fields for the SBR and HBR
    if (record.m_u2SOFT_BIN <= 32767)
        mSBinPassFail.insert(record.m_u2SOFT_BIN, ((record.m_b1PART_FLG&0x08)?'F':'P'));
    mHBinPassFail.insert(record.m_u2HARD_BIN, ((record.m_b1PART_FLG&0x08)?'F':'P'));
    return true;
}


bool CGSTDFV3toSTDF::ReadPDR(GQTL_STDF::Stdf_PDR_V3& record)
{
    // used to write the PTR
    mStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*)&record);
    if (!mPDRList.contains(record.m_u4TEST_NUM))
        mPDRList.insert(record.m_u4TEST_NUM, record);
    else
    {
        mPDRList.remove(record.m_u4TEST_NUM);
        mPDRList.insert(record.m_u4TEST_NUM, record);
    }
    return true;
}


bool CGSTDFV3toSTDF::ReadFDR(GQTL_STDF::Stdf_FDR_V3& record)
{
    // Used to write the FTR
    mStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*)&record);
//    QString lKey = QString::number(record.m_u4TEST_NUM) + record.m_cnTEST_NAM;
    if (!mFDRList.contains(record.m_u4TEST_NUM))
        mFDRList.insert(record.m_u4TEST_NUM, record);
    else
    {
        mFDRList.remove(record.m_u4TEST_NUM);
        mFDRList.insert(record.m_u4TEST_NUM, record);
    }
    return true;
}


bool CGSTDFV3toSTDF::ReadWriteTSR(GQTL_STDF::Stdf_TSR_V3& record)
{
    // Similar but not the same
    // use also the STS
    if (!mStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*)&record))
    {
        return false;
    }
    GQTL_STDF::Stdf_TSR_V4 lTSRV4Record;
    lTSRV4Record.SetHEAD_NUM(255);
    lTSRV4Record.SetSITE_NUM(1);
    if (mTSRTestTyp.contains(record.m_u4TEST_NUM))
        lTSRV4Record.SetTEST_TYP(mTSRTestTyp[record.m_u4TEST_NUM]);
    else
        lTSRV4Record.SetTEST_TYP(' ');
    if (record.IsFieldPresent(GQTL_STDF::Stdf_TSR_V3::eposTEST_NUM))
        lTSRV4Record.SetTEST_NUM(record.m_u4TEST_NUM);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_TSR_V3::eposEXEC_CNT))
        lTSRV4Record.SetEXEC_CNT(static_cast<stdf_type_u4>((record.m_i4EXEC_CNT!=-1)?record.m_i4EXEC_CNT:INVALID_INT));
    if (record.IsFieldPresent(GQTL_STDF::Stdf_TSR_V3::eposFAIL_CNT))
        lTSRV4Record.SetFAIL_CNT(static_cast<stdf_type_u4>((record.m_i4FAIL_CNT!=-1)?record.m_i4FAIL_CNT:INVALID_INT));
    if (record.IsFieldPresent(GQTL_STDF::Stdf_TSR_V3::eposALRM_CNT))
        lTSRV4Record.SetALRM_CNT(static_cast<stdf_type_u4>((record.m_i4ALRM_CNT!=-1)?record.m_i4ALRM_CNT:INVALID_INT));
    if (record.IsFieldPresent(GQTL_STDF::Stdf_TSR_V3::eposTEST_NAM))
        lTSRV4Record.SetTEST_NAM(record.m_cnTEST_NAM);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_TSR_V3::eposSEQ_NAME))
        lTSRV4Record.SetSEQ_NAME(record.m_cnSEQ_NAME);
//    lTSRV4Record.SetTEST_LBL("");
    if (record.IsFieldPresent(GQTL_STDF::Stdf_TSR_V3::eposOPT_FLAG))
    {
        // convert the optional flag from v3 to v4
        stdf_type_b1 lOptFlag;
        lOptFlag = record.m_b1OPT_FLAG | 0xC8;
        lOptFlag &= 0xFB;
        lTSRV4Record.SetOPT_FLAG(lOptFlag);
    }
//    lTSRV4Record.SetTEST_TIM(0.0);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_TSR_V3::eposTEST_MIN))
        lTSRV4Record.SetTEST_MIN(record.m_r4TEST_MIN);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_TSR_V3::eposTEST_MAX))
        lTSRV4Record.SetTEST_MAX(record.m_r4TEST_MAX);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_TSR_V3::eposTST_SUMS))
        lTSRV4Record.SetTST_SUMS(record.m_r4TST_SUMS);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_TSR_V3::eposTST_SQRS))
        lTSRV4Record.SetTST_SQRS(record.m_r4TST_SQRS);
    lTSRV4Record.Write(mStdfFile);
    return true;
}


bool CGSTDFV3toSTDF::ReadWritePTR(GQTL_STDF::Stdf_PTR_V3& record)
{
    // Some differences
    if (!mStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*)&record))
    {
        return false;
    }
    GQTL_STDF::Stdf_PTR_V4 lPTRV4Record;
    if (record.IsFieldPresent(GQTL_STDF::Stdf_PTR_V3::eposTEST_NUM))
        lPTRV4Record.SetTEST_NUM(record.m_u4TEST_NUM);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_PTR_V3::eposHEAD_NUM))
        lPTRV4Record.SetHEAD_NUM(record.m_u1HEAD_NUM);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_PTR_V3::eposSITE_NUM))
        lPTRV4Record.SetSITE_NUM(record.m_u1SITE_NUM);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_PTR_V3::eposTEST_FLG))
        lPTRV4Record.SetTEST_FLG(record.m_b1TEST_FLG);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_PTR_V3::eposPARM_FLG))
        lPTRV4Record.SetPARM_FLG(record.m_b1PARM_FLG);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_PTR_V3::eposRESULT))
        lPTRV4Record.SetRESULT(record.m_r4RESULT);

    GQTL_STDF::Stdf_PDR_V3 lPDRRecord;
    if (mPDRList.contains(record.m_u4TEST_NUM))
    {
        // If the list of the PDR contains many test_number, the PTR must have the test_nam
        if (mPDRList.count(record.m_u4TEST_NUM) > 1)
        {
            if (!record.IsFieldPresent(GQTL_STDF::Stdf_PTR_V3::eposTEST_TXT))
                return false;
            else
            {
                QList<GQTL_STDF::Stdf_PDR_V3> lPDRList = mPDRList.values(record.m_u4TEST_NUM);
                for(int i=0; i<lPDRList.size(); ++i)
                {
                    if (lPDRList[i].m_cnTEST_NAM == record.m_cnTEST_TXT)
                    {
                        lPDRRecord = lPDRList[i];
                    }
                }
            }
        }
        else
            lPDRRecord = mPDRList.value(record.m_u4TEST_NUM);
    }
    else
        lPDRRecord.Reset();
    if (!mPTRList.contains(record.m_u4TEST_NUM))
    {
        mPTRList.append(record.m_u4TEST_NUM);
        // if the options in the PTR is null then those existing in the PDR
        if (record.IsFieldPresent(GQTL_STDF::Stdf_PTR_V3::eposTEST_TXT))
            lPTRV4Record.SetTEST_TXT(record.m_cnTEST_TXT);
        else if (lPDRRecord.IsFieldPresent(GQTL_STDF::Stdf_PDR_V3::eposTEST_NAM))
            lPTRV4Record.SetTEST_TXT(lPDRRecord.m_cnTEST_NAM);
//    lPTRV4Record.SetALARM_ID("");
        if (record.IsFieldPresent(GQTL_STDF::Stdf_PTR_V3::eposOPT_FLAG))
            lPTRV4Record.SetOPT_FLAG(record.m_b1OPT_FLAG);
        // convert the opt flag
        else if (lPDRRecord.IsFieldPresent(GQTL_STDF::Stdf_PTR_V3::eposOPT_FLAG))
        {
            stdf_type_b1 lOPT_FLAG = 0x02;
            if (lPDRRecord.m_b1OPT_FLAG & 0x01) lOPT_FLAG |= 0x54;    //1010100
            if (lPDRRecord.m_b1OPT_FLAG & 0x02) lOPT_FLAG |= 0xA8;    //10101000
            lPTRV4Record.SetOPT_FLAG(lOPT_FLAG);
        }

        if (record.IsFieldPresent(GQTL_STDF::Stdf_PTR_V3::eposRES_SCAL))
            lPTRV4Record.SetRES_SCAL(record.m_i1RES_SCAL);
        else if (lPDRRecord.IsFieldPresent(GQTL_STDF::Stdf_PTR_V3::eposRES_SCAL))
            lPTRV4Record.SetRES_SCAL(lPDRRecord.m_i1RES_SCAL);

        if (record.IsFieldPresent(GQTL_STDF::Stdf_PTR_V3::eposLLM_SCAL))
            lPTRV4Record.SetLLM_SCAL(record.m_i1LLM_SCAL);
        else if (lPDRRecord.IsFieldPresent(GQTL_STDF::Stdf_PTR_V3::eposLLM_SCAL))
            lPTRV4Record.SetLLM_SCAL(lPDRRecord.m_i1LLM_SCAL);

        if (record.IsFieldPresent(GQTL_STDF::Stdf_PTR_V3::eposHLM_SCAL))
            lPTRV4Record.SetHLM_SCAL(record.m_i1HLM_SCAL);
        else if (lPDRRecord.IsFieldPresent(GQTL_STDF::Stdf_PTR_V3::eposHLM_SCAL))
            lPTRV4Record.SetHLM_SCAL(lPDRRecord.m_i1HLM_SCAL);

        if (record.IsFieldPresent(GQTL_STDF::Stdf_PTR_V3::eposLO_LIMIT))
            lPTRV4Record.SetLO_LIMIT(record.m_r4LO_LIMIT);
        else if (lPDRRecord.IsFieldPresent(GQTL_STDF::Stdf_PTR_V3::eposLO_LIMIT))
            lPTRV4Record.SetLO_LIMIT(lPDRRecord.m_r4LO_LIMIT);

        if (record.IsFieldPresent(GQTL_STDF::Stdf_PTR_V3::eposHI_LIMIT))
            lPTRV4Record.SetHI_LIMIT(record.m_r4HI_LIMIT);
        else if (lPDRRecord.IsFieldPresent(GQTL_STDF::Stdf_PTR_V3::eposHI_LIMIT))
            lPTRV4Record.SetHI_LIMIT(lPDRRecord.m_r4HI_LIMIT);

        if (record.IsFieldPresent(GQTL_STDF::Stdf_PTR_V3::eposUNITS))
            lPTRV4Record.SetUNITS(record.m_cnUNITS);
        else if (lPDRRecord.IsFieldPresent(GQTL_STDF::Stdf_PTR_V3::eposUNITS))
            lPTRV4Record.SetUNITS(lPDRRecord.m_cnUNITS);

//        lPTRV4Record.SetC_RESFMT("");
//        lPTRV4Record.SetC_LLMFMT("");
//        lPTRV4Record.SetC_HLMFMT("");
//        lPTRV4Record.SetLO_SPEC(0.0);
//        lPTRV4Record.SetHI_SPEC(0.0);
    }
    lPTRV4Record.Write(mStdfFile);

    mTSRTestTyp.insert(record.m_u4TEST_NUM, 'P');
    return true;
}


bool CGSTDFV3toSTDF::ReadWriteFTR(GQTL_STDF::Stdf_FTR_V3& record)
{
    // Some differences
    if (!mStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*)&record))
    {
        return false;
    }
    GQTL_STDF::Stdf_FTR_V4 lFTRV4Record;
    if (record.IsFieldPresent(GQTL_STDF::Stdf_FTR_V3::eposTEST_NUM))
        lFTRV4Record.SetTEST_NUM(record.m_u4TEST_NUM);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_FTR_V3::eposHEAD_NUM))
        lFTRV4Record.SetHEAD_NUM(record.m_u1HEAD_NUM);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_FTR_V3::eposSITE_NUM))
        lFTRV4Record.SetSITE_NUM(record.m_u1SITE_NUM);
    lFTRV4Record.SetTEST_FLG(record.m_b1TEST_FLG & 0xFD); // bit 1 reserved for the future, must be 0
    stdf_type_b1 lOptionalFlag = 0xFF;
    lOptionalFlag &= record.m_b1OPT_FLAG;
    if ((record.m_b1OPT_FLAG & 0x1) == 0) // Vect_ADR
        lOptionalFlag &= 0xFD;
    if ((record.m_b1OPT_FLAG & 0x2) == 0) // CYCL_CNT
        lOptionalFlag &= 0xFE;
    if ((record.m_b1OPT_FLAG & 0x20) == 0) // CYCL_CNT
        lOptionalFlag &= 0xFB;
    if ((record.m_b1OPT_FLAG & 0x10) == 0) // NUL_FAIL
        lOptionalFlag &= 0xF7;
    lFTRV4Record.SetOPT_FLAG(lOptionalFlag);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_FTR_V3::eposCYCL_CNT))
        lFTRV4Record.SetCYCL_CNT(record.m_u4CYCL_CNT);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_FTR_V3::eposVECT_ADR))
        lFTRV4Record.SetREL_VADR(record.m_u4VECT_ADR);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_FTR_V3::eposREPT_CNT))
        lFTRV4Record.SetREPT_CNT(record.m_u2REPT_CNT);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_FTR_V3::eposNUM_FAIL))
        lFTRV4Record.SetNUM_FAIL(record.m_u4NUM_FAIL);
    lFTRV4Record.m_dnFAIL_PIN.m_uiLength = static_cast<unsigned int>(record.m_bnFAIL_PIN.m_bLength);
    for (int lIndex=0; lIndex<record.m_bnFAIL_PIN.m_bLength; ++lIndex)
        lFTRV4Record.m_dnFAIL_PIN.m_pBitField[lIndex] = record.m_bnFAIL_PIN.m_pBitField[lIndex];
    if (record.IsFieldPresent(GQTL_STDF::Stdf_FTR_V3::eposFAIL_PIN))
        lFTRV4Record.SetFAIL_PIN();
    if (record.IsFieldPresent(GQTL_STDF::Stdf_FTR_V3::eposTIME_SET))
        lFTRV4Record.SetTIME_SET(QString::number(record.m_u1TIME_SET));
    if (record.IsFieldPresent(GQTL_STDF::Stdf_FTR_V3::eposTEST_TXT))
        lFTRV4Record.SetTEST_TXT(record.m_cnTEST_TXT);
    else
        if (mFDRList.contains(record.m_u4TEST_NUM)
            && (mFDRList[record.m_u4TEST_NUM]).IsFieldPresent(GQTL_STDF::Stdf_FDR_V3::eposTEST_NAM))
            lFTRV4Record.SetTEST_TXT(mFDRList[record.m_u4TEST_NUM].m_cnTEST_NAM);

    if (record.IsFieldPresent(GQTL_STDF::Stdf_FTR_V3::eposRPIN_MAP))
    {
        lFTRV4Record.m_dnSPIN_MAP.m_uiLength = static_cast<unsigned int>(record.m_bnRPIN_MAP.m_bLength);
        for (int lIndex=0; lIndex<record.m_bnRPIN_MAP.m_bLength; ++lIndex)
            lFTRV4Record.m_dnSPIN_MAP.m_pBitField[lIndex] = record.m_bnRPIN_MAP.m_pBitField[lIndex];
        lFTRV4Record.SetSPIN_MAP();
    }
    lFTRV4Record.Write(mStdfFile);

    mTSRTestTyp.insert(record.m_u4TEST_NUM, 'F');
    return true;
}


bool CGSTDFV3toSTDF::ReadWriteBPS(GQTL_STDF::Stdf_BPS_V3& record)
{
    // the same
    if (!mStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*)&record))
    {
        return false;
    }
    GQTL_STDF::Stdf_BPS_V4 lBPSV4Record;
    if (record.IsFieldPresent(GQTL_STDF::Stdf_BPS_V3::eposSEQ_NAME))
        lBPSV4Record.SetSEQ_NAME(record.m_cnSEQ_NAME);
    lBPSV4Record.Write(mStdfFile);
    return true;
}


bool CGSTDFV3toSTDF::ReadWriteEPS(GQTL_STDF::Stdf_EPS_V3& record)
{
    // the same
    if (!mStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*)&record))
    {
        return false;
    }
    GQTL_STDF::Stdf_EPS_V4 lEPSV4Record;
    lEPSV4Record.Write(mStdfFile);
    return true;
}


bool CGSTDFV3toSTDF::ReadWriteSHB(GQTL_STDF::Stdf_SHB_V3& record)
{
    // Merge it with the HBR to generate a V4's HBR
    if (!mStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*)&record))
    {
        return false;
    }
    GQTL_STDF::Stdf_HBR_V4 lHBRV4Record;
    if (record.IsFieldPresent(GQTL_STDF::Stdf_SHB_V3::eposHEAD_NUM))
        lHBRV4Record.SetHEAD_NUM(record.m_u1HEAD_NUM);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_SHB_V3::eposSITE_NUM))
        lHBRV4Record.SetSITE_NUM(record.m_u1SITE_NUM);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_SHB_V3::eposHBIN_NUM))
        lHBRV4Record.SetHBIN_NUM(record.m_u2HBIN_NUM);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_SHB_V3::eposHBIN_CNT))
        lHBRV4Record.SetHBIN_CNT(record.m_u4HBIN_CNT);
    if (mHBinPassFail.contains(record.m_u2HBIN_NUM))
        lHBRV4Record.SetHBIN_PF(mHBinPassFail.value(record.m_u2HBIN_NUM));
    else
        lHBRV4Record.SetHBIN_PF(' ');
    if (record.IsFieldPresent(GQTL_STDF::Stdf_SHB_V3::eposHBIN_NAM))
        lHBRV4Record.SetHBIN_NAM(record.m_cnHBIN_NAM);
    lHBRV4Record.Write(mStdfFile);
    return true;
}


bool CGSTDFV3toSTDF::ReadSSBWriteSSB(GQTL_STDF::Stdf_SSB_V3& record)
{
    // Merge it with the SBR to generate a V4's SBR
    if (!mStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*)&record))
    {
        return false;
    }
    GQTL_STDF::Stdf_SBR_V4 lSBRV4Record;
    if (record.IsFieldPresent(GQTL_STDF::Stdf_SSB_V3::eposHEAD_NUM))
        lSBRV4Record.SetHEAD_NUM(record.m_u1HEAD_NUM);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_SSB_V3::eposSITE_NUM))
        lSBRV4Record.SetSITE_NUM(record.m_u1SITE_NUM);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_SSB_V3::eposSBIN_NUM))
        lSBRV4Record.SetSBIN_NUM(record.m_u2SBIN_NUM);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_SSB_V3::eposSBIN_CNT))
        lSBRV4Record.SetSBIN_CNT(record.m_u4SBIN_CNT);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_SSB_V3::eposSBIN_NUM)
        && (mSBinPassFail.contains(record.m_u2SBIN_NUM)))
            lSBRV4Record.SetSBIN_PF(mSBinPassFail.value(record.m_u2SBIN_NUM));
        else
            lSBRV4Record.SetSBIN_PF(' ');
    if (record.IsFieldPresent(GQTL_STDF::Stdf_SSB_V3::eposSBIN_NAM))
        lSBRV4Record.SetSBIN_NAM(record.m_cnSBIN_NAM);
    lSBRV4Record.Write(mStdfFile);
    return true;
}


bool CGSTDFV3toSTDF::ReadWriteSTS(GQTL_STDF::Stdf_STS_V3& record)
{
    // Used in the TSR
    if (!mStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*)&record))
    {
        return false;
    }
    GQTL_STDF::Stdf_TSR_V4 lTSRV4Record;
    if (record.IsFieldPresent(GQTL_STDF::Stdf_STS_V3::eposHEAD_NUM))
        lTSRV4Record.SetHEAD_NUM(record.m_u1HEAD_NUM);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_STS_V3::eposSITE_NUM))
        lTSRV4Record.SetSITE_NUM(record.m_u1SITE_NUM);
//    if (record.IsFieldPresent(GQTL_STDF::Stdf_TSR_V3::epos))
        if (mTSRTestTyp.contains(/*QString::number(*/record.m_u4TEST_NUM/*) + record.m_cnTEST_NAM)*/))
            lTSRV4Record.SetTEST_TYP(mTSRTestTyp[/*QString::number(*/record.m_u4TEST_NUM/*) + record.m_cnTEST_NAM*/]);
        else
            lTSRV4Record.SetTEST_TYP(' ');
    if (record.IsFieldPresent(GQTL_STDF::Stdf_STS_V3::eposTEST_NUM))
        lTSRV4Record.SetTEST_NUM(record.m_u4TEST_NUM);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_STS_V3::eposEXEC_CNT))
        lTSRV4Record.SetEXEC_CNT(static_cast<stdf_type_u4>((record.m_i4EXEC_CNT!=-1)?record.m_i4EXEC_CNT:INVALID_INT));
    if (record.IsFieldPresent(GQTL_STDF::Stdf_STS_V3::eposFAIL_CNT))
        lTSRV4Record.SetFAIL_CNT(static_cast<stdf_type_u4>((record.m_i4FAIL_CNT!=-1)?record.m_i4FAIL_CNT:INVALID_INT));
    if (record.IsFieldPresent(GQTL_STDF::Stdf_STS_V3::eposALRM_CNT))
        lTSRV4Record.SetALRM_CNT(static_cast<stdf_type_u4>((record.m_i4ALRM_CNT!=-1)?record.m_i4ALRM_CNT:INVALID_INT));
    if (record.IsFieldPresent(GQTL_STDF::Stdf_STS_V3::eposTEST_NAM))
        lTSRV4Record.SetTEST_NAM(record.m_cnTEST_NAM);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_STS_V3::eposSEQ_NAME))
        lTSRV4Record.SetSEQ_NAME(record.m_cnSEQ_NAME);
//        lTSRV4Record.SetTEST_LBL("");
    if (record.IsFieldPresent(GQTL_STDF::Stdf_STS_V3::eposOPT_FLAG))
    {
        // convert the  optional flag from v3 to v4
        stdf_type_b1 lOptFlag;
        lOptFlag = record.m_b1OPT_FLAG | 0xC8;
        lOptFlag &= 0xFB;
        lTSRV4Record.SetOPT_FLAG(lOptFlag);
    }
//        lTSRV4Record.SetTEST_TIM(0.0);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_STS_V3::eposTEST_MIN))
        lTSRV4Record.SetTEST_MIN(record.m_r4TEST_MIN);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_STS_V3::eposTEST_MAX))
        lTSRV4Record.SetTEST_MAX(record.m_r4TEST_MAX);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_STS_V3::eposTST_SUMS))
        lTSRV4Record.SetTST_SUMS(record.m_r4TST_SUMS);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_STS_V3::eposTST_SQRS))
        lTSRV4Record.SetTST_SQRS(record.m_r4TST_SQRS);
    lTSRV4Record.Write(mStdfFile);
    return true;
}


bool CGSTDFV3toSTDF::ReadSCRWritePCR(GQTL_STDF::Stdf_SCR_V3& record)
{
    // Very close to the PCR
    // read the SCR record from the stdf v3
    if (!mStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*)&record))
    {
        return false;
    }
    GQTL_STDF::Stdf_PCR_V4 lPCRV4Record;
    if (record.IsFieldPresent(GQTL_STDF::Stdf_SCR_V3::eposHEAD_NUM))
        lPCRV4Record.SetHEAD_NUM(record.m_u1HEAD_NUM);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_SCR_V3::eposSITE_NUM))
        lPCRV4Record.SetSITE_NUM(record.m_u1SITE_NUM);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_SCR_V3::eposPART_CNT))
        lPCRV4Record.SetPART_CNT(record.m_u4PART_CNT);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_SCR_V3::eposRTST_CNT))
        lPCRV4Record.SetRTST_CNT(static_cast<stdf_type_u4>((record.m_i4RTST_CNT!=-1)?record.m_i4RTST_CNT:INVALID_INT));
    if (record.IsFieldPresent(GQTL_STDF::Stdf_SCR_V3::eposABRT_CNT))
        lPCRV4Record.SetABRT_CNT(static_cast<stdf_type_u4>((record.m_i4ABRT_CNT!=-1)?record.m_i4ABRT_CNT:INVALID_INT));
    if (record.IsFieldPresent(GQTL_STDF::Stdf_SCR_V3::eposGOOD_CNT))
        lPCRV4Record.SetGOOD_CNT(static_cast<stdf_type_u4>((record.m_i4GOOD_CNT!=-1)?record.m_i4GOOD_CNT:INVALID_INT));
    if (record.IsFieldPresent(GQTL_STDF::Stdf_SCR_V3::eposFUNC_CNT))
        lPCRV4Record.SetFUNC_CNT(static_cast<stdf_type_u4>((record.m_i4FUNC_CNT!=-1)?record.m_i4FUNC_CNT:INVALID_INT));
    lPCRV4Record.Write(mStdfFile);
    return true;
}


bool CGSTDFV3toSTDF::ReadWriteGDR(GQTL_STDF::Stdf_GDR_V3& record)
{
    // The same
    if (!mStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*)&record))
    {
        return false;
    }
    GQTL_STDF::Stdf_GDR_V4 lGDRV4Record;
    if (record.IsFieldPresent(GQTL_STDF::Stdf_GDR_V3::eposFLD_CNT))
        lGDRV4Record.SetFLD_CNT(record.m_u2FLD_CNT);
    if (record.IsFieldPresent(GQTL_STDF::Stdf_GDR_V3::eposGEN_DATA))
        lGDRV4Record.SetGEN_DATA(record.m_vnGEN_DATA);
    lGDRV4Record.Write(mStdfFile);
    return true;
}


bool CGSTDFV3toSTDF::ReadWriteDTR(GQTL_STDF::Stdf_DTR_V3& record)
{
    // the same
    if (!mStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*)&record))
    {
        return false;
    }
    GQTL_STDF::Stdf_DTR_V4 lDTRV4Record;
    if (record.IsFieldPresent(GQTL_STDF::Stdf_DTR_V3::eposTEXT_DAT))
        lDTRV4Record.SetTEXT_DAT(record.m_cnTEXT_DAT);
    lDTRV4Record.Write(mStdfFile);
    return true;
}

