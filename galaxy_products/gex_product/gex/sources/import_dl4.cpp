
#include <float.h>
#include <string.h>

#include <qfileinfo.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlabel.h>


#include "import_dl4.h"
#include "read_system_info.h"
#include "import_constants.h"
#include "engine.h"

extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar	*	GexProgressBar;		// Handle to progress bar in status bar

WORD			g_backplane;
STRING			g_UI_computer_name;

char			power_translate[] = {-12,-9,-6,-3,0,3,6,9,12,15};
//CHANGES BEGIN 6 July 2006
//STDF_STRINGS	*g_STDF_STRINGS = NULL;
//CHANGES END 6 July 2006

QString			g_strCurrentStep;

//CHANGES BEGIN 28 June 2006
//extern enum PRR_PART_ID_OPTIONS;
// CHANGES END 28 JUNE 2006

CDL4toSTDF::CDL4toSTDF(void)
{
    ReadSystemInfo cSystemInfo;		// All system info: hostname, host id, board id, etc...

    strFileNameDl4 = "";
    strFileNameStdf = "";
    m_strLastErrorMsg = "";

//CHANGES BEGIN 6 July 2006
    //g_STDF_STRINGS = new STDF_STRINGS;
    //ASSERT(g_STDF_STRINGS);
//CHANGES END 6 July 2006

    if(cSystemInfo.isSystemInfoAvailable() != READ_SYS_INFO_ERROR_HOSTNAME)
        g_UI_computer_name = cSystemInfo.strHostName.toLatin1().data();

    g_backplane = ASL_CFG_3000;

//--- from DataLogDoc

    m_HDR = NULL;
    m_last_VLD = 0;
    m_last_FLD = 0;


    m_serial_num = 1;
    m_device_num = 1;
    m_pExpirationDate = NULL;
}

CDL4toSTDF::~CDL4toSTDF(void)
{

    try {
//CHANGES BEGIN 6 July 2006
        //if(g_STDF_STRINGS)
        //	delete g_STDF_STRINGS;
//CHANGES BEGIN 6 July 2006
        if (m_HDR)
        {
            if(m_HDR->m_STDF_STRINGS)
                delete m_HDR->m_STDF_STRINGS;
            m_HDR->m_STDF_STRINGS = NULL;
            if(m_HDR->m_STDF_SUPPLEMENT)
                delete m_HDR->m_STDF_SUPPLEMENT;
            m_HDR->m_STDF_SUPPLEMENT = NULL;
            delete m_HDR;
        }

        m_HDR = NULL;

        if(m_last_VLD)
            delete m_last_VLD;
        m_last_VLD = NULL;

        if(m_last_FLD)
            delete m_last_FLD;
        m_last_FLD = NULL;
    } // try
    catch(...)
    {
        // No error
    } // catch
}

QString CDL4toSTDF::GetLastError()
{
    if(m_strLastErrorMsg.isEmpty())
        return BinaryDatabase::GetLastError();
    return m_strLastErrorMsg;
}

void CDL4toSTDF::ErrorMessage (const char *e0, const char *e1, const char *e2, const char *e3,const char *e4)
{
    if(!m_strLastErrorMsg.isEmpty())
        m_strLastErrorMsg += " ; ";
    m_strLastErrorMsg += e0;
    if (e1)
    {
        m_strLastErrorMsg += " ; ";
        m_strLastErrorMsg += e1;
        if (e2)
        {
            m_strLastErrorMsg += " ; ";
            m_strLastErrorMsg += e2;
            if (e3)
            {
                m_strLastErrorMsg += " ; ";
                m_strLastErrorMsg += e3;
                if (e4)
                {
                    m_strLastErrorMsg += " ; ";
                    m_strLastErrorMsg += e4;
                }
            }
        }
    }

}

//////////////////////////////////////////////////////////////////////
// Check if File is compatible with DL4 format
//////////////////////////////////////////////////////////////////////
bool CDL4toSTDF::IsCompatible(const char *szFileName)
{
    QFileInfo cFileInfo(szFileName);
    // Only check if have the good extension !!!

    return (cFileInfo.suffix().toLower() == "dl4");
}

//////////////////////////////////////////////////////////////////////
// Convert Dl4 to Stdf file format
//////////////////////////////////////////////////////////////////////
bool CDL4toSTDF::Convert(const char *szFileNameDl4, const char *szFileNameStdf)
{
    bool	iret = false;
    QString strMessage, strString;

    m_strLastErrorMsg = "";

    strFileNameDl4 = szFileNameDl4;
    strFileNameStdf = szFileNameStdf;

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    iProgressStep = 0;
    iNextCount = 0;
    iTotalCount = 0;
    iCount = 0;

    bool bHideProgressAfter	= true;
    bool bHideLabelAfter	= false;
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
            GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(szFileNameDl4).fileName()+"...");
            GexScriptStatusLabel->show();
        }
    }
    QCoreApplication::processEvents();

    iret = false;
   try
    {
        iret = OpenAnExistingDatalog ();

   } // try
   catch(char* sz)
    {
        strMessage.sprintf("Pos in DL4 file(%d) - Error message(%s)", BinaryDatabase::GetCurrentPos(),sz);
        ErrorMessage("ERROR FROM Dl4_Convert","Dl4 file corrupted",strMessage.toLatin1().constData());
   } // catch
   catch(...)
    {
        strMessage.sprintf("Pos in DL4 file(%d) - Error message(%s)", BinaryDatabase::GetCurrentPos(),"An exception occurs during the parse of the Dl4 file");
        ErrorMessage("ERROR FROM Dl4_Convert","Dl4 file corrupted",strMessage.toLatin1().constData());
   } // catch

    if (iret != true)
        return false;

    iret = false;
    try {
        iret = STDF_Convert();
    } // try
    catch(char* sz) {
        strMessage = sz;
        ErrorMessage("ERROR FROM Dl4_Convert",strMessage.toLatin1().constData(),BinaryDatabase::GetLastError().toLatin1().constData());
    } // catch
    catch(...) {
        ErrorMessage("ERROR FROM Dl4_Convert","An exception occurs during the convertion of the Dl4 file",BinaryDatabase::GetLastError().toLatin1().constData());
    } // catch

    if(iret == false)
    {
        // WHERE THE STDF CONVERTION FAIL
        int i;
        strMessage="";
        for(i=0; i<GQTL_STDF::Stdf_Record::Rec_COUNT; i++)
        {
            if(m_nStdfRecordsCount[i] == 0)
                continue;
            switch(i)
            {
                case GQTL_STDF::Stdf_Record::Rec_FAR:
                    strMessage += strString.sprintf("FAR(%d) ",m_nStdfRecordsCount[i]);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_ATR:
                    strMessage += strString.sprintf("ATR(%d) ",m_nStdfRecordsCount[i]);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_MIR:
                    strMessage += strString.sprintf("MIR(%d) ",m_nStdfRecordsCount[i]);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_MRR:
                    strMessage += strString.sprintf("MRR (%d)", m_nStdfRecordsCount[i]);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_PCR:
                    strMessage += strString.sprintf("PCR(%d) ",m_nStdfRecordsCount[i]);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_HBR:
                    strMessage += strString.sprintf("HBR(%d) ",m_nStdfRecordsCount[i]);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_SBR:
                    strMessage += strString.sprintf("SBR(%d) ",m_nStdfRecordsCount[i]);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_PMR:
                    strMessage += strString.sprintf("PMR(%d) ",m_nStdfRecordsCount[i]);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_PGR:
                    strMessage += strString.sprintf("PGR(%d) ",m_nStdfRecordsCount[i]);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_PLR:
                    strMessage += strString.sprintf("PLR(%d) ",m_nStdfRecordsCount[i]);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_RDR:
                    strMessage += strString.sprintf("RDR(%d) ",m_nStdfRecordsCount[i]);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_SDR:
                    strMessage += strString.sprintf("SDR(%d) ",m_nStdfRecordsCount[i]);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_WIR:
                    strMessage += strString.sprintf("WIR(%d) ",m_nStdfRecordsCount[i]);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_WRR:
                    strMessage += strString.sprintf("WRR(%d) ",m_nStdfRecordsCount[i]);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_WCR:
                    strMessage += strString.sprintf("WCR(%d) ",m_nStdfRecordsCount[i]);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_PIR:
                    strMessage += strString.sprintf("PIR(%d) ",m_nStdfRecordsCount[i]);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_PRR:
                    strMessage += strString.sprintf("PRR(%d) ",m_nStdfRecordsCount[i]);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_TSR:
                    strMessage += strString.sprintf("TSR(%d) ",m_nStdfRecordsCount[i]);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_PTR:
                    strMessage += strString.sprintf("PTR(%d) ",m_nStdfRecordsCount[i]);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_MPR:
                    strMessage += strString.sprintf("MPR(%d) ",m_nStdfRecordsCount[i]);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_FTR:
                    strMessage += strString.sprintf("FTR(%d) ",m_nStdfRecordsCount[i]);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_BPS:
                    strMessage += strString.sprintf("BPS(%d) ",m_nStdfRecordsCount[i]);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_EPS:
                    strMessage += strString.sprintf("EPS(%d) ",m_nStdfRecordsCount[i]);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_GDR:
                    strMessage += strString.sprintf("GDR(%d) ",m_nStdfRecordsCount[i]);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_DTR:
                    strMessage += strString.sprintf("DTR(%d) ",m_nStdfRecordsCount[i]);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_RESERVED_IMAGE:
                    strMessage += strString.sprintf("Reserved for use by Image(%d) ",m_nStdfRecordsCount[i]);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_RESERVED_IG900:
                    strMessage += strString.sprintf("Reserved for use by IG900(%d) ",m_nStdfRecordsCount[i]);
                    break;
                case GQTL_STDF::Stdf_Record::Rec_UNKNOWN:
                    strMessage += strString.sprintf("UNKNOWN!!!!(%d) ",m_nStdfRecordsCount[i]);
                    break;
            }
        }

        if(strMessage.isEmpty())
            strMessage = "Import DL4: Invalid file format: File corrupted, no data extracted";
        else
        {
            // 2011 10 19 => REJECT ALL FILES WITH INCOMPLET DATA
            strMessage.insert(0,"Import DL4: Invalid file format: File corrupted, some data cannot be extracted ; ** Records parsed: ");
            /*
            // force the status
            iret = true;
            strMessage.insert(0,"Import DL4: Warning: File corrupted, not all data extracted ; ** Records generated: ");
            */
        }

        strMessage+=" ; ";
        m_strLastErrorMsg.insert(0,strMessage);

    }

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    if((GexProgressBar != NULL)
    && bHideProgressAfter)
        GexProgressBar->hide();

    if((GexScriptStatusLabel != NULL)
    && bHideLabelAfter)
        GexScriptStatusLabel->hide();

    return iret;
}

BOOL CDL4toSTDF::OpenAnExistingDatalog ()
{

    void *adv;
    char *adtag;

    if(!BinaryDatabase::OpenDatabaseStationaryMode (strFileNameDl4.toLatin1().constData()))
        return false;

    if (!m_HDR)
        m_HDR = new DLOG_DB_HDR;
    ASSERT_LOCATION(m_HDR);
    if (!m_HDR)
    {
        ErrorMessage("Can't allocate enough memory to perform database operation.");
        return false;
    }

    if(BinaryDatabase::ReadHDR (*m_HDR) == 0)
        return false;

    DLOG_PROGRAM_INFO *dpi = m_HDR->GetPrgInfo ();
    if (!dpi)
    {
        ErrorMessage("Can't allocate enough memory to perform database operation.");
        return false;
    }

    PROGRAM_FUNCTIONS_DATA *pfd = dpi->PrgInfo ();
    ASSERT_LOCATION(pfd);

    m_last_VLD = new DLOG_DB_VLD (pfd);
    ASSERT_LOCATION(m_last_VLD);
    if (!m_last_VLD)
    {
        ErrorMessage("Can't allocate enough memory to perform database operation.");
        return false;
    }

    m_last_FLD = new DLOG_DB_FLD;
    ASSERT_LOCATION(m_last_FLD);
    if (!m_last_FLD)
    {
        ErrorMessage("Can't allocate enough memory to perform database operation.");
        return false;
    }

    if(!BinaryDatabase::ReadFLD (*m_last_FLD, BinaryDatabase::ItemsTotalN() - 1))
        return false;
    if(!BinaryDatabase::ReadVLD (*m_last_VLD, BinaryDatabase::ItemsTotalN() - 1))
        return false;

    while ((adtag= BinaryDatabase::ReadAD(adv)) != NULL) {
        if (strcmp(adtag, ADTAG_STDF_STRINGS) == 0) {
//CHANGES BEGIN 6 July 2006
            //m_HDR->m_STDF_STRINGS = new STDF_STRINGS;
            //ASSERT_LOCATION(m_HDR->m_STDF_STRINGS);
//CHANGES END 6 July 2006

            m_HDR->m_STDF_STRINGS->LoadFromBuffer(adv);
        } else if (strcmp(adtag, ADTAG_STDF_SUPPLEMENT) == 0) {
            m_HDR->m_STDF_SUPPLEMENT = new STDF_SUPPLEMENT(m_HDR);
            ASSERT_LOCATION(m_HDR->m_STDF_SUPPLEMENT);
            m_HDR->m_STDF_SUPPLEMENT->LoadFromBuffer(adv);
        }
        delete [] (BYTE *)adv;
    }

    m_device_num = GetLastDeviceNumber () + 1;
    m_serial_num = GetLastSerialNumber () + 1;

    return true;
}

gsint32 CDL4toSTDF::GetLastDeviceNumber (void)
{
    if (!m_HDR) return 0;
    return m_HDR->GetLastDeviceNumber ();
}

gsint32 CDL4toSTDF::GetLastSerialNumber (void)
{
    if (!m_HDR) return 0;
    return m_HDR->GetLastSerialNumber ();
}

BOOL CDL4toSTDF::STDF_Convert ()
{

    BOOL bReturnCode = true;
    gsuint32 *loop, **fails, **exec, *last_time, *first_time, **bin_count;
    time_t *wafer_start_times = NULL;
    bool wafer_testing = false;
    gsint32 start_rec;
    gsint32 nrec, i;
    QDateTime qTime;

    // Init table of records to process
    for(i=0; i<GQTL_STDF::Stdf_Record::Rec_COUNT; i++)
        m_nStdfRecordsCount[i] = 0;

    // test for datalog entries
    if (!ItemsTotalN ())
        return true;

    // get datalog DB header
    DLOG_DB_HDR *pDBH = GetDBHeader();
    ASSERT_LOCATION(pDBH);

    // get lot summary data
    LOT_SUMMARY_DATA *pLSD = pDBH->Get_LOT_SUMMARY_DATA ();
    ASSERT_LOCATION(pLSD);

    // get program name from program info
    DLOG_PROGRAM_INFO *pDPI = pDBH->GetPrgInfo ();
    ASSERT_LOCATION(pDPI);

    const char *program_name = pDPI->GetName ();

    wafer_testing = (pLSD->m_wafer_testing) ? true : false;

    // determine the number of subtests
    PROGRAM_FUNCTIONS_DATA *pPFD = Get_PROGRAM_FUNCTIONS_DATA ();
    ASSERT_LOCATION(pPFD);

    short subtest_total = 0;
    for (short func_i = 0; func_i < pPFD->FunctionsN (); func_i++)
    {
        ONE_FUNCTION *pOF = pPFD->Function (func_i);
        ASSERT_LOCATION(pOF);

        for (short subtest_i = 0; subtest_i < pOF->SubTestsN (); subtest_i++)
            subtest_total++;
    }

    // create a boolean array to record that "semi-static" data has been sent in a PTR record
    BOOL *semi_static_sent = new BOOL [subtest_total];
    ASSERT_LOCATION(semi_static_sent);
    if(!semi_static_sent)
    {
        ErrorMessage("Can't allocate enough memory to perform database operation.");
        return false;
    }

    for (short icount = 0; icount < subtest_total; icount++) // initialize
        semi_static_sent[icount] = false;

    // determine the number of sites
    short sites_i = (short) pDBH->GetSiteCountArray()->GetCount();

    // create arrays to accumulate values
    if(!STDF_Create_Arrays (loop, fails, exec, last_time, first_time, bin_count, sites_i, subtest_total))
    {bReturnCode = false; goto CleanupTag;}
    wafer_start_times = new time_t[sites_i];
    ASSERT_LOCATION(wafer_start_times);
    memset(wafer_start_times, 0, sizeof(time_t) * sites_i);

    start_rec = 0;

    nrec = ItemsTotalN ();

    if(!STDF_Get_Start_Time (  start_rec, nrec, first_time,sites_i)) {bReturnCode = false; goto CleanupTag;}


    // open output file(s)
    if(!STDF_Open_File(strFileNameStdf.toLatin1().constData()))
    {bReturnCode = false; goto CleanupTag;}

    // begin writing STDF records
    if(!STDF_FAR_Record_Write ()) {bReturnCode = false; goto CleanupTag;}
    if(!STDF_ATR_Record_Write ()) {bReturnCode = false; goto CleanupTag;}

    if(!STDF_MIR_Record_Write (program_name, first_time,   pLSD, sites_i)) {bReturnCode = false; goto CleanupTag;}

    if(!STDF_SDR_Record_Write (  pLSD, sites_i)) {bReturnCode = false; goto CleanupTag;}


    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    iTotalCount = nrec - start_rec;
    iNextCount = 0;
    iCount = 0;

    short doing_site;
    short subtest_count;

    for (i = start_rec; i < start_rec + nrec; i++)
    {
        //////////////////////////////////////////////////////////////////////
        // For ProgressBar
        iCount++;
        if(GexProgressBar != NULL)
        {
            while(iCount > iNextCount)
            {
                iProgressStep += 100/iTotalCount + 1;
                iNextCount  += iTotalCount/100 + 1;
                GexProgressBar->setValue(iProgressStep);
            }
        }
        QCoreApplication::processEvents();

        DLOG_DB_FLD *pFLD = new DLOG_DB_FLD;
        ASSERT_LOCATION(pFLD);

        if(!BinaryDatabase::ReadFLD (*pFLD,i))
            {bReturnCode = false; goto CleanupTag;}

        PROGRAM_FUNCTIONS_DATA *pPFD = Get_PROGRAM_FUNCTIONS_DATA (pFLD);
        ASSERT_LOCATION(pPFD);

        DLOG_DB_VLD *pVLD = new DLOG_DB_VLD(pPFD);
        ASSERT_LOCATION(pVLD);

        if(!BinaryDatabase::ReadVLD (*pVLD,i))
            {bReturnCode = false; goto CleanupTag;}

        if(pVLD->ContainsData ())
        {
            const test_header src_test_header = pVLD->Get_test_header ();
            const dlog_entry *sub_test_results = pVLD->Get_dlog_entry_array ();
            doing_site = pVLD->GetSiteI ();

            last_time[doing_site] = (DL4_ULONG)src_test_header.now;  // get last time for MRR record

            if (wafer_testing && (wafer_start_times[0] == 0))
            {
                if(!STDF_WCR_Record_Write(  doing_site)) {bReturnCode = false; goto CleanupTag;}
                if(!STDF_WIR_Record_Write(  src_test_header.now, doing_site)) {bReturnCode = false; goto CleanupTag;}
                wafer_start_times[0] = src_test_header.now;
            }
            if(!STDF_PIR_Record_Write (doing_site)) {bReturnCode = false; goto CleanupTag;}

            subtest_count = 0;

            // FREmn07540 - Old method for testing for datalog sampling did not work if the
            // sample interval was changed during testing.
            //
            // When datalog sampling is enabled, sub-test results are only saved for the
            // sampled parts.  For parts with no datalog, the VLD::m_dlog_entry_array is
            // initialized to the cleared state: valid_value == false && passed_failed ==
            // UNKNOWN_RESULTS for all sub-tests.   To determine if the part has no sub-tests
            // results (i.e. was not sampled), check for the cleared state in the VLD.
            // If in the cleared state, skip writing PTR records.
            // GPZ 06/02/02
            bool valid = false;
            for (int ii = 0; valid == false && ii < pVLD->Get_dlog_entry_array_n(); ++ii)
            {
                if (sub_test_results[ii].valid_value != false ||
                    sub_test_results[ii].passed_fail != (char)UNKNOWN_RESULT)
                {
                    valid = true;
                }
            }

            if (valid)
            {
                for (short func_i = 0; func_i < pPFD->FunctionsN (); func_i++)
                {
                    ONE_FUNCTION *pOF = pPFD->Function (func_i);
                    ASSERT_LOCATION(pOF);
                    //-------------------------------
                    const dlog_entry *sub_test_results_first_record;

                    DLOG_DB_FLD *pFLD_first_record = new DLOG_DB_FLD;
                    ASSERT_LOCATION(pFLD_first_record);

                    if(!BinaryDatabase::ReadFLD (*pFLD_first_record,0))	{bReturnCode = false; goto CleanupTag;}
                    PROGRAM_FUNCTIONS_DATA *pPFD_first_record = Get_PROGRAM_FUNCTIONS_DATA (pFLD_first_record);
                    ASSERT_LOCATION(pPFD_first_record);

                    DLOG_DB_VLD *pVLD_first_record = new DLOG_DB_VLD(pPFD_first_record);
                    ASSERT_LOCATION(pVLD_first_record);

                    if(!BinaryDatabase::ReadVLD (*pVLD_first_record,0))
                    {
                        delete pFLD_first_record;
                        delete pVLD_first_record;
                        bReturnCode = false;
                        goto CleanupTag;
                    }//first record entry

                    sub_test_results_first_record = pVLD_first_record->Get_dlog_entry_array();
                    ONE_FUNCTION *pOF_first_record = pPFD_first_record->Function (func_i);
                    ASSERT_LOCATION(pOF_first_record);

                    bool limits_changed = false;

                    //-------------------------------
                    for (short subtest_i = 0; subtest_i < pOF->SubTestsN (); subtest_i++)
                    {
                        //current subtest data
                        FUNCTION_SUBTEST_DATA *pFSD = pOF->SubTestData (subtest_i);
                        ASSERT_LOCATION(pFSD);

                        //first record subtest data
                        FUNCTION_SUBTEST_DATA *pFSD_First_record = pOF_first_record->SubTestData (subtest_i);
                        ASSERT_LOCATION(pFSD_First_record);

                        //Since the first record in the STDF file sets the default value for limits,
                        //we need to compare the current subtest result to that of the subtest result in the first record so
                        //we can set the opt_flag to correctly handle different grades of passing limits
                        if( sub_test_results->passed_fail != sub_test_results_first_record->passed_fail)
                            limits_changed = true;

                        if(!STDF_PTR_Record_Write (  semi_static_sent, i, sub_test_results,
                                            pFSD, doing_site,func_i, subtest_i,
                                            subtest_count, fails, exec,
                                            pVLD, limits_changed))
                        {
                            delete pFLD_first_record;
                            delete pVLD_first_record;
                            bReturnCode = false;
                            goto CleanupTag;
                        }
                        subtest_count++;
                    }

                    delete pFLD_first_record;
                    delete pVLD_first_record;

                }
            }

            if(!STDF_PRR_Record_Write (pLSD,src_test_header, doing_site, subtest_count))
            {
                delete pVLD;
                delete pFLD;
                bReturnCode = false;
                goto CleanupTag;
            }
        }

        delete pVLD;
        delete pFLD;
    }

    if (sites_i <= pLSD->GetSitesN ())
    {
        for (short i = 0; i < sites_i; i++)
        {
            loop[i] = 0;
            for (short j = 0; j < pLSD->BinsN (); j++)
            {
                bin_count[i][j] = pLSD->MultiSiteBinCount (j, i);
                loop[i] += bin_count[i][j];
            }
        }
        PtrSet <PER_TEST_STAT> &ptrset = pLSD->TestStats ();
        if (ptrset.GetCount ())
        {
            for (short i = 0; i < ptrset.GetCount(); i++)
            {
                PER_TEST_STAT *pts = ptrset.GetObjectPtr(i);
                if (pts)
                {
                    for (short j = 0; j < sites_i; j++)
                    {
                        fails[j][i] = pts->GetFailureCount (j);
                        exec [j][i] = pts->GetDeviceCount (j);
                    }
                }
            }
        }
    }

    if(!STDF_HBR_Record_Write (pLSD, sites_i, bin_count)) {bReturnCode = false; goto CleanupTag;}
    if(!STDF_SBR_Record_Write (pLSD, sites_i, bin_count)) {bReturnCode = false; goto CleanupTag;}
    if(!STDF_TSR_Record_Write (sites_i, fails, exec)) {bReturnCode = false; goto CleanupTag;}
    if(!STDF_PCR_Record_Write (sites_i, bin_count, loop)) {bReturnCode = false; goto CleanupTag;}

    if (wafer_testing)
    {
        if(!STDF_WRR_Record_Write (  last_time, bin_count, loop, sites_i))
            {bReturnCode = false; goto CleanupTag;}
    }

    if(!STDF_MRR_Record_Write (  last_time, sites_i)) {bReturnCode = false; goto CleanupTag;}

    CleanupTag:
    // cleanup
    if(semi_static_sent) delete [] semi_static_sent;
    semi_static_sent = NULL;

    if(last_time) delete [] last_time;
    last_time = NULL;

    if(first_time) delete [] first_time;
    first_time = NULL;

    if(wafer_start_times) delete [] wafer_start_times;
    wafer_start_times = NULL;

    if(loop) delete loop;
    loop = NULL;

    for (i = 0; i < sites_i; i++)
    {
        if(fails[i]) delete [] fails[i];
        if(exec[i]) delete [] exec[i];
        if(bin_count[i]) delete [] bin_count[i];
    }

    if(fails) delete [] fails;
    fails = NULL;

    if(exec) delete [] exec;
    exec = NULL;

    if(bin_count) delete [] bin_count;
    bin_count = NULL;

    m_cStdfParse.Close();

    return bReturnCode;
}

DLOG_DB_HDR *CDL4toSTDF::GetDBHeader (void)
{
    return m_HDR;
}


PROGRAM_FUNCTIONS_DATA *CDL4toSTDF::Get_PROGRAM_FUNCTIONS_DATA (DLOG_DB_FLD *FLD)
{
    return get_pfd (FLD);
}

BOOL CDL4toSTDF::STDF_Open_File (const char *file_name)
{
#if defined (i386) || defined (__i386__) || defined (_M_IX86) || defined (vax) || defined (__alpha)
    if(!m_cStdfParse.Open((char*) file_name,STDF_WRITE,2))
    {
        ErrorMessage(GGET_LASTERRORMSG(StdfParse, &m_cStdfParse));
        return false;
    }
#else
    if(!m_cStdfParse.Open((char*) file_name,STDF_WRITE,1))
    {
        ErrorMessage(GGET_LASTERRORMSG(StdfParse, &m_cStdfParse));
        return false;
    }
#endif
    return true;
}

BOOL CDL4toSTDF::STDF_Create_Arrays (gsuint32 * &loop, gsuint32 ** &fails, gsuint32 ** &exec,
                         gsuint32 * &last_time, gsuint32 * &first_time,
                         gsuint32 ** &bin_count, short sites_i, short subtest_total)
{
    last_time = new gsuint32[sites_i];
    ASSERT_LOCATION(last_time);
    if (!last_time)
    {
        ErrorMessage("Can't allocate enough memory to perform database operation.");
        return false;
    }

    first_time = new gsuint32[sites_i];
    ASSERT_LOCATION(first_time);
    if (!first_time)
    {
        ErrorMessage("Can't allocate enough memory to perform database operation.");
        return false;
    }

    bin_count = (gsuint32 **)new gsuint32 *[sites_i];
    ASSERT_LOCATION(bin_count);
    if (!bin_count)
    {
        ErrorMessage("Can't allocate enough memory to perform database operation.");
        return false;
    }
    fails = (gsuint32 **)new gsuint32 *[sites_i];
    ASSERT_LOCATION(fails);
    if (!fails)
    {
        ErrorMessage("Can't allocate enough memory to perform database operation.");
        return false;
    }
    exec = (gsuint32 **)new gsuint32 *[sites_i];
    ASSERT_LOCATION(exec);
    if (!exec)
    {
        ErrorMessage("Can't allocate enough memory to perform database operation.");
        return false;
    }
    loop = new gsuint32[sites_i];
    ASSERT_LOCATION(loop);
    if (!loop)
    {
        ErrorMessage("Can't allocate enough memory to perform database operation.");
        return false;
    }

    short i, j;
    for (i = 0; i < sites_i; i++)
    {
        //it's important these 2 arrays get initialized to 0. Otherwise, the MIR and MRR start/finish
        //times will report the wrong times.
        last_time[i]  = 0;
        first_time[i] = 0;

        fails[i] = new gsuint32[subtest_total];
        ASSERT_LOCATION(fails[i]);
        if (!fails[i])
        {
            ErrorMessage("Can't allocate enough memory to perform database operation.");
            return false;
        }
        exec[i]  = new gsuint32[subtest_total];
        ASSERT_LOCATION(exec[i]);
        if (!exec[i])
        {
            ErrorMessage("Can't allocate enough memory to perform database operation.");
            return false;
        }
        bin_count[i] = new gsuint32[MAX_SW_BINS]; //goganesy FREmn10312
        ASSERT_LOCATION(bin_count[i]);
        if (!bin_count[i])
        {
            ErrorMessage("Can't allocate enough memory to perform database operation.");
            return false;
        }
        loop[i] = 0;

        for (short j = 0; j < subtest_total; j++)
        {
            exec[i][j] = fails[i][j] = 0L;
        }
    }

    for (i = 0; i < MAX_SW_BINS; i++)//goganesy FREmn10312
    {
        for (j = 0; j < sites_i; j++)
        {
            bin_count[j][i] = 0L;
        }
    }
    return true;
}

BOOL CDL4toSTDF::STDF_FAR_Record_Write ()
{

    m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_FAR]++;
    // read when create the STDF file
    return true;

    //Stdf_FAR_V4 clFar;
    //int iValue;
    //m_cStdfParse.GetCpuType(&iValue);
    //clFar.SetCPU_TYPE(iValue);
    //m_cStdfParse.GetVersion(&iValue);
    //clFar.SetSTDF_VER(iValue);
    //return m_cStdfParse.WriteRecord(&clFar);
}

BOOL CDL4toSTDF::STDF_ATR_Record_Write ()
{
    bool	bSave = false;
    GQTL_STDF::Stdf_ATR_V4 clAtr;
    //CHANGES BEGIN 6 July 2006
    STDF_STRINGS *ss  = GetDBHeader()->m_STDF_STRINGS;
    //CHANGES END 6 July 2006


    if (ss != NULL && ss->Get_ATR_CMD_LINE() != NULL)
    {
        bSave = true;
        clAtr.SetCMD_LINE(ss->Get_ATR_CMD_LINE());
    }
    else
        clAtr.SetCMD_LINE("");

    clAtr.SetMOD_TIM(0);

    // Save only if data
    if(bSave)
    {
        m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_ATR]++;
        return m_cStdfParse.WriteRecord(&clAtr);
    }
    else
        return true;
}

BOOL CDL4toSTDF::STDF_MIR_Record_Write (const char *program_name, gsuint32 *first_time,
                            LOT_SUMMARY_DATA *pLSD,
                            short sites_i)
{
    char str[1024];//, *site_name;
    const char	*limit_name=0, *file_name=0, *device_name=0, *prog_rev=0, *prog_testcode=0, *oper_stepnumber=0;
    //CHANGES BEGIN 6 July 2006
    STDF_STRINGS *ss  = GetDBHeader()->m_STDF_STRINGS;
    //CHANGES END 6 July 2006

    m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_MIR]++;

    if (GetDBHeader ())
    {
        limit_name = (const char *)GetDBHeader()->GetPrgInfo()->GetLimitSetName();
        device_name = (const char *)GetDBHeader()->GetPrgInfo()->GetDeviceName();
        prog_rev = (const char *)GetDBHeader()->GetPrgInfo()->GetProgramRevision();
        prog_testcode = (const char *)GetDBHeader()->GetPrgInfo()->GetProgramTestCode();
        oper_stepnumber = (const char *)GetDBHeader()->GetPrgInfo()->GetOperationStepNumber();
    }

    GQTL_STDF::Stdf_MIR_V4 clMir;

    file_name = BinaryDatabase::GetDatabaseFileName ();
    QFileInfo finder(file_name);
    BOOL bFound = finder.exists();
    if (bFound)
    {
        QDateTime qTime = finder.created();
        clMir.SetSETUP_T((gsuint32)qTime.toTime_t());
    }
    else
        clMir.SetSETUP_T(0);

    for (int i=0; i<sites_i; i++)
    {
        if(first_time[i])
        {
            clMir.SetSTART_T(first_time[i]);
            break;
        }
    }

    if (ss != NULL && ss->Get_MIR_STAT_NUM() != 0) {
        clMir.SetSTAT_NUM(ss->Get_MIR_STAT_NUM());
    } else {
        clMir.SetSTAT_NUM(1);
    }

    if (ss != NULL && ss->Get_MIR_MODE_COD() != 0) {
        clMir.SetMODE_COD(ss->Get_MIR_MODE_COD());
    } else {
        clMir.SetMODE_COD(MIR_PRODUCT);
    }

    if (ss != NULL && ss->Get_MIR_RTST_COD() != 0) {
        clMir.SetRTST_COD(ss->Get_MIR_RTST_COD());
    } else {
        clMir.SetRTST_COD(' ');
    }

    if (ss != NULL && ss->Get_MIR_PROT_COD() != 0) {
        clMir.SetPROT_COD(ss->Get_MIR_PROT_COD());
    } else {
        clMir.SetPROT_COD(' ');
    }

    if (ss != NULL && ss->Get_MIR_BURN_TIM() != 0) {
        clMir.SetBURN_TIM(ss->Get_MIR_BURN_TIM());
    } else {
        clMir.SetBURN_TIM(65535);
    }

    if (ss != NULL && ss->Get_MIR_CMOD_COD() != 0) {
        clMir.SetCMOD_COD(ss->Get_MIR_CMOD_COD());
    } else {
        clMir.SetCMOD_COD(' ');
    }

    if (ss != NULL && ss->Get_MIR_LOT_ID() != NULL) {
        clMir.SetLOT_ID(ss->Get_MIR_LOT_ID());
    } else {
        clMir.SetLOT_ID((char *)pLSD->GetLotIDName());
    }

    if (ss != NULL && ss->Get_MIR_PART_TYP() != NULL) {
        clMir.SetPART_TYP(ss->Get_MIR_PART_TYP());
    } else {
        clMir.SetPART_TYP(device_name);
    }

    if (ss != NULL && ss->Get_MIR_NODE_NAM() != NULL) {
        clMir.SetNODE_NAM(ss->Get_MIR_NODE_NAM());
    } else {
//CHANGES BEGIN 17 JULY 2006
        clMir.SetNODE_NAM((char *)pLSD->GetTheComputerName());// TBD
//CHANGES END 17 JULY 2006
    }

    QString strTstrTyp;
    if (ss != NULL && ss->Get_MIR_TSTR_TYP() != NULL)
        strTstrTyp = ss->Get_MIR_TSTR_TYP();
    else
    {
        unsigned short temp = g_backplane; //so we can debug
        switch(temp)
        {
        case 1: //ASL2000
            strTstrTyp = "ASL2000";
            break;
        case 2: //ASL3000
            strTstrTyp = "ASL3000";
            break;
        case 0: //ASL1000
        default:
            strTstrTyp = "ASL1000";
            break;
        }
    }
    // force TSTR_TYP to "DL4_Source: <original value>" for Quantix check
    clMir.SetTSTR_TYP("DL4_Source: " + strTstrTyp);

    if (ss != NULL && ss->Get_MIR_JOB_NAM() != NULL) {
        clMir.SetJOB_NAM(ss->Get_MIR_JOB_NAM());
    } else {
        sprintf(str, "%s.%s", program_name, limit_name);
        clMir.SetJOB_NAM(str);
    }

    // Optionals Fields

    // Beginning with the last field for initialization
    BOOL	bInitPreviousField = false;

    if (ss != NULL && ss->Get_MIR_SUPR_NAM() != NULL) {
        clMir.SetSUPR_NAM(ss->Get_MIR_SUPR_NAM());
        bInitPreviousField = true;
    }

    if (ss != NULL && ss->Get_MIR_SERL_NUM() != NULL) {
        clMir.SetSERL_NUM(ss->Get_MIR_SERL_NUM());
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clMir.SetSERL_NUM("");

    if (ss != NULL && ss->Get_MIR_ROM_COD() != NULL) {
        clMir.SetROM_COD(ss->Get_MIR_ROM_COD());
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clMir.SetROM_COD("");

    if (ss != NULL && ss->Get_MIR_ENG_ID() != NULL) {
        clMir.SetENG_ID(ss->Get_MIR_ENG_ID());
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clMir.SetENG_ID("");

    if (ss != NULL && ss->Get_MIR_DSGN_REV() != NULL) {
        clMir.SetDSGN_REV(ss->Get_MIR_DSGN_REV());
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clMir.SetDSGN_REV("");

    if (ss != NULL && ss->Get_MIR_SETUP_ID() != NULL) {
        clMir.SetSETUP_ID(ss->Get_MIR_SETUP_ID());
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clMir.SetSETUP_ID("");

    if (ss != NULL && ss->Get_MIR_FLOW_ID() != NULL) {
        clMir.SetFLOW_ID(ss->Get_MIR_FLOW_ID());
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clMir.SetFLOW_ID("");

    if (ss != NULL && ss->Get_MIR_SPEC_VER() != NULL) {
        clMir.SetSPEC_VER(ss->Get_MIR_SPEC_VER());
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clMir.SetSPEC_VER("");

    if (ss != NULL && ss->Get_MIR_SPEC_NAM() != NULL) {
        clMir.SetSPEC_NAM(ss->Get_MIR_SPEC_NAM());
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clMir.SetSPEC_NAM("");

    if (ss != NULL && ss->Get_MIR_OPER_FRQ() != NULL) {
        clMir.SetOPER_FRQ(ss->Get_MIR_OPER_FRQ());
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clMir.SetOPER_FRQ("");

    if (ss != NULL && ss->Get_MIR_PROC_ID() != NULL) {
        clMir.SetPROC_ID(ss->Get_MIR_PROC_ID());
        bInitPreviousField = true;
    } else if(strcmp(oper_stepnumber, "") != 0) {
        clMir.SetPROC_ID(oper_stepnumber);
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clMir.SetPROC_ID("");

    if (ss != NULL && ss->Get_MIR_FLOOR_ID() != NULL) {
        clMir.SetFLOOR_ID(ss->Get_MIR_FLOOR_ID());
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clMir.SetFLOOR_ID("");

    if (ss != NULL && ss->Get_MIR_FACIL_ID() != NULL) {
        clMir.SetFACIL_ID(ss->Get_MIR_FACIL_ID());
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clMir.SetFACIL_ID("");

    if (ss != NULL && ss->Get_MIR_DATE_COD() != NULL) {
        clMir.SetDATE_COD(ss->Get_MIR_DATE_COD());
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clMir.SetDATE_COD("");

    if (ss != NULL && ss->Get_MIR_FAMLY_ID() != NULL) {
        clMir.SetFAMLY_ID(ss->Get_MIR_FAMLY_ID());
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clMir.SetFAMLY_ID("");

    if (ss != NULL && ss->Get_MIR_PKG_TYP() != NULL) {
        clMir.SetPKG_TYP(ss->Get_MIR_PKG_TYP());
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clMir.SetPKG_TYP("");

    if (ss != NULL && ss->Get_MIR_AUX_FILE() != NULL) {
        clMir.SetAUX_FILE(ss->Get_MIR_AUX_FILE());
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clMir.SetAUX_FILE("");

    if (ss != NULL && ss->Get_MIR_USER_TXT() != NULL) {
        clMir.SetUSER_TXT(ss->Get_MIR_USER_TXT());
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clMir.SetUSER_TXT("");

    if (ss != NULL && ss->Get_MIR_TST_TEMP() != NULL) {
        clMir.SetTST_TEMP(ss->Get_MIR_TST_TEMP());
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clMir.SetTST_TEMP("");

    // get temperature from lot summary comments
    if (pLSD && pLSD->GetStatusIncludesComments ())
    {
        int obj_num = 0;
        const char *s = *(pLSD->Comments ().GetObjectPtr (obj_num));
        QString temp (s);
        int npos;
        if ((npos = temp.indexOf ("Temperature")) != -1)
        {
            npos = temp.indexOf(":", npos);
            temp = temp.right (temp.length () - (npos + 1));
            temp = temp.trimmed();
            clMir.SetTST_TEMP(temp.toLatin1().constData());
            bInitPreviousField = true;
        }
    }

    if (ss != NULL && ss->Get_MIR_TEST_COD() != NULL) {
        clMir.SetTEST_COD(ss->Get_MIR_TEST_COD());
        bInitPreviousField = true;
    } else if(strcmp(prog_testcode, "") != 0){
        clMir.SetTEST_COD(prog_testcode);
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clMir.SetTEST_COD("");

    if (ss != NULL && ss->Get_MIR_EXEC_VER() != NULL) {
        clMir.SetEXEC_VER(ss->Get_MIR_EXEC_VER());
        bInitPreviousField = true;
    } else
    {
        clMir.SetEXEC_VER("Rev not encoded");
        bInitPreviousField = true;
    }
    // force EXEC_VER to QuantixVer for Quantix check
    clMir.SetEXEC_VER("Quantix Converter");
    bInitPreviousField = true;

    if (ss != NULL && ss->Get_MIR_EXEC_TYP() != NULL) {
        clMir.SetEXEC_TYP(ss->Get_MIR_EXEC_TYP());
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clMir.SetEXEC_TYP("visualATE");

    if (ss != NULL && ss->Get_MIR_OPER_NAM() != NULL) {
        clMir.SetOPER_NAM(ss->Get_MIR_OPER_NAM());
        bInitPreviousField = true;
    } else
    {
//CHANGES BEGIN 17 JULY 2006
        clMir.SetOPER_NAM((char *)pLSD->GetOperatorName());
//CHANGES END 17 JULY 2006
        bInitPreviousField = true;
    }

    if (ss != NULL && ss->Get_MIR_SBLOT_ID() != NULL) {
        clMir.SetSBLOT_ID(ss->Get_MIR_SBLOT_ID());
        bInitPreviousField = true;
    } else if(strcmp(pLSD->GetSubLotIDName(), "") != 0){
        clMir.SetSBLOT_ID((char *)pLSD->GetSubLotIDName());
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clMir.SetSBLOT_ID("");

    if (ss != NULL && ss->Get_MIR_JOB_REV() != NULL) {
        clMir.SetJOB_REV(ss->Get_MIR_JOB_REV());
        bInitPreviousField = true;
    } else if(strcmp(prog_rev, "") != 0){
        clMir.SetJOB_REV(prog_rev);
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clMir.SetJOB_REV("");

    // Construct custom Galaxy USER_TXT
    QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
    strUserTxt += ":DL4";
    clMir.SetUSER_TXT(strUserTxt);

    return m_cStdfParse.WriteRecord(&clMir);
}

BOOL CDL4toSTDF::STDF_SDR_Record_Write (LOT_SUMMARY_DATA *pLSD,
                            short sites_i)
{
    ASSERT_LOCATION(pLSD);

    //CHANGES BEGIN 6 July 2006
    STDF_STRINGS *ss  = GetDBHeader()->m_STDF_STRINGS;
    //CHANGES END 6 July 2006

    GQTL_STDF::Stdf_SDR_V4 clSdr;

    m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_SDR]++;

    // Initialization
    clSdr.SetHEAD_NUM(1);
    clSdr.SetSITE_GRP(1);
    clSdr.SetSITE_CNT(stdf_type_u1(sites_i));

    for(unsigned int i=0; i<clSdr.m_u1SITE_CNT; i++)
    {
        clSdr.SetSITE_NUM(i,stdf_type_u1(i+1));
    }

    // Optionals Fields

    // Beginning with the last field for initialization
    BOOL	bInitPreviousField = false;

    if (ss != NULL && ss->Get_SDR_EXTR_ID() != NULL) {
        clSdr.SetEXTR_ID(ss->Get_SDR_EXTR_ID());
        bInitPreviousField = true;
    }

    if (ss != NULL && ss->Get_SDR_EXTR_TYP() != NULL) {
        clSdr.SetEXTR_TYP(ss->Get_SDR_EXTR_TYP());
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clSdr.SetEXTR_TYP("");

    if (ss != NULL && ss->Get_SDR_LASR_ID() != NULL) {
        clSdr.SetLASR_ID(ss->Get_SDR_LASR_ID());
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clSdr.SetLASR_ID("");

    if (ss != NULL && ss->Get_SDR_LASR_TYP() != NULL) {
        clSdr.SetLASR_TYP(ss->Get_SDR_LASR_TYP());
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clSdr.SetLASR_TYP("");

    if (ss != NULL && ss->Get_SDR_CONT_ID() != NULL) {
        clSdr.SetCONT_ID(ss->Get_SDR_CONT_ID());
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clSdr.SetCONT_ID("");

    if (ss != NULL && ss->Get_SDR_CONT_TYP() != NULL) {
        clSdr.SetCONT_TYP(ss->Get_SDR_CONT_TYP());
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clSdr.SetCONT_TYP("");

    if (ss != NULL && ss->Get_SDR_CABL_ID() != NULL) {
        clSdr.SetCABL_ID(ss->Get_SDR_CABL_ID());
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clSdr.SetCABL_ID("");

    if (ss != NULL && ss->Get_SDR_CABL_TYP() != NULL) {
        clSdr.SetCABL_TYP(ss->Get_SDR_CABL_TYP());
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clSdr.SetCABL_TYP("");

    if (ss != NULL && ss->Get_SDR_DIB_ID() != NULL) {
        clSdr.SetDIB_ID(ss->Get_SDR_DIB_ID());
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clSdr.SetDIB_ID("");

    if (ss != NULL && ss->Get_SDR_DIB_TYP() != NULL) {
        clSdr.SetDIB_TYP(ss->Get_SDR_DIB_TYP());
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clSdr.SetDIB_TYP("");

    if (ss != NULL && ss->Get_SDR_LOAD_ID() != NULL) {
        clSdr.SetLOAD_ID(ss->Get_SDR_LOAD_ID());
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clSdr.SetLOAD_ID("");

    if (ss != NULL && ss->Get_SDR_LOAD_TYP() != NULL) {
        clSdr.SetLOAD_TYP(ss->Get_SDR_LOAD_TYP());
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clSdr.SetLOAD_TYP("");

    if (ss != NULL && ss->Get_SDR_CARD_ID() != NULL) {
        clSdr.SetCARD_ID(ss->Get_SDR_CARD_ID());
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clSdr.SetCARD_ID("");

    if (ss != NULL && ss->Get_SDR_CARD_TYP() != NULL) {
        clSdr.SetCARD_TYP(ss->Get_SDR_CARD_TYP());
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clSdr.SetCARD_TYP("");

    if (ss != NULL && ss->Get_SDR_HAND_ID() != NULL) {
        clSdr.SetHAND_ID(ss->Get_SDR_HAND_ID());
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clSdr.SetHAND_ID("");

    if (ss != NULL && ss->Get_SDR_HAND_TYP() != NULL) {
        clSdr.SetHAND_TYP(ss->Get_SDR_HAND_TYP());
        bInitPreviousField = true;
    } else if (strcmp(pLSD->GetHandlerName(), "") != 0) {
        clSdr.SetHAND_TYP((char *)pLSD->GetHandlerName());
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clSdr.SetHAND_TYP("");

    return m_cStdfParse.WriteRecord(&clSdr);
}

BOOL CDL4toSTDF::STDF_Get_Start_Time (gsint32 start_rec, gsint32 nrec,
                          gsuint32 *first_time, short sites_i)
{
    test_header header;

    for (short s_i = 0; s_i < sites_i; s_i++)
        first_time[s_i] = 0;

    short site_count = 0;

    for (gsint32 i = start_rec; i < start_rec + nrec; i++)
    {
        DLOG_DB_FLD *pFLD = new DLOG_DB_FLD;
        ASSERT_LOCATION(pFLD);

        if(!BinaryDatabase::ReadFLD (*pFLD,i))
        {
            delete pFLD;
            return false;
        }

        PROGRAM_FUNCTIONS_DATA *pPFD = Get_PROGRAM_FUNCTIONS_DATA (pFLD);
        ASSERT_LOCATION(pPFD);

        DLOG_DB_VLD *pVLD = new DLOG_DB_VLD(pPFD);
        ASSERT_LOCATION(pVLD);

        if(!BinaryDatabase::ReadVLD (*pVLD,i))
        {
            delete pFLD;
            delete pVLD;
            return false;
        }

        if(pVLD->ContainsData ())
        {
            test_header src_test_header = pVLD->Get_test_header ();


            header.write_type	= src_test_header.write_type;
            header.now			= src_test_header.now;
            header.hundreths	= src_test_header.hundreths;
            header.device_num	= src_test_header.device_num;
            header.ret_val		= src_test_header.ret_val;
            header.bin_num		= src_test_header.bin_num;
            header.serial_num	= src_test_header.serial_num;
            header.x_coordinate	= src_test_header.x_coordinate;
            header.y_coordinate	= src_test_header.y_coordinate;

            short doing_site = pVLD->GetSiteI ();
            if (first_time[doing_site] == 0)
            {
                first_time[doing_site] = (DL4_ULONG)header.now;
                site_count++;
            }
            if (site_count >= sites_i)
            {
                delete pVLD;
                delete pFLD;
                return true;
            }
        }
        delete pVLD;
        delete pFLD;
    }
    return true;
}

BOOL CDL4toSTDF::STDF_WCR_Record_Write(short /*doing_site*/)
{
    //CHANGES BEGIN 6 July 2006
    STDF_STRINGS *ss  = GetDBHeader()->m_STDF_STRINGS;
    //CHANGES END 6 July 2006
    GQTL_STDF::Stdf_WCR_V4 clWcr;

    m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_WCR]++;

    // Optionals Fields

    // Beginning with the last field for initialization
    BOOL	bInitPreviousField = false;

    if (ss != NULL && ss->Get_WCR_POS_Y() != '\0') {
        clWcr.SetPOS_Y(ss->Get_WCR_POS_Y());
        bInitPreviousField = true;
    }

    if (ss != NULL && ss->Get_WCR_POS_X() != '\0') {
        clWcr.SetPOS_X(ss->Get_WCR_POS_X());
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clWcr.SetPOS_X(' ');

    if (ss != NULL && ss->Get_WCR_CENTER_Y() != 0) {
        clWcr.SetCENTER_Y(stdf_type_i2(ss->Get_WCR_CENTER_Y()));
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clWcr.SetCENTER_Y(stdf_type_i2(-32768));

    if (ss != NULL && ss->Get_WCR_CENTER_X() != 0) {
        clWcr.SetCENTER_X(stdf_type_i2(ss->Get_WCR_CENTER_X()));
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clWcr.SetCENTER_X(stdf_type_i2(-32768));

    if (ss != NULL && ss->Get_WCR_WF_FLAT() != '\0') {
        clWcr.SetWF_FLAT(ss->Get_WCR_WF_FLAT());
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clWcr.SetWF_FLAT(' ');

    if (ss != NULL && ss->Get_WCR_WF_UNITS() != 0.0) {
        clWcr.SetWF_UNITS(stdf_type_u1(ss->Get_WCR_WF_UNITS()));
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clWcr.SetWF_UNITS(stdf_type_u1(0));

    if (ss != NULL && ss->Get_WCR_DIE_WID() != 0.0) {
        clWcr.SetDIE_WID(stdf_type_r4(ss->Get_WCR_DIE_WID()));
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clWcr.SetDIE_WID(stdf_type_r4(0));

    if (ss != NULL && ss->Get_WCR_DIE_HT() != 0.0) {
        clWcr.SetDIE_HT(stdf_type_r4(ss->Get_WCR_DIE_HT()));
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clWcr.SetDIE_HT(stdf_type_r4(0));

    if (ss != NULL && ss->Get_WCR_WAFR_SIZ() != 0) {
        clWcr.SetWAFR_SIZ(stdf_type_r4(ss->Get_WCR_WAFR_SIZ()));
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clWcr.SetWAFR_SIZ(stdf_type_r4(0));

    // check if have init some fields
    if(bInitPreviousField)
        return m_cStdfParse.WriteRecord(&clWcr);
    else
        return true;
}

BOOL CDL4toSTDF::STDF_WIR_Record_Write(time_t start_t, short /*doing_site*/)
{
    GQTL_STDF::Stdf_WIR_V4 clWir;
    STDF_STRINGS *ss = GetDBHeader()->m_STDF_STRINGS;//g_STDF_STRINGS;

    m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_WIR]++;

    clWir.SetHEAD_NUM(1);
    clWir.SetSITE_GRP(255);

    clWir.SetSTART_T((DL4_ULONG)start_t);
    if (ss != NULL && ss->Get_WIR_WAFER_ID() != NULL) {
        clWir.SetWAFER_ID(ss->Get_WIR_WAFER_ID());
    }

    return m_cStdfParse.WriteRecord(&clWir);
}

BOOL CDL4toSTDF::STDF_PIR_Record_Write (short doing_site)
{
    GQTL_STDF::Stdf_PIR_V4 clPir;

    m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PIR]++;

    //Initialization
    clPir.SetHEAD_NUM(1);

    clPir.SetSITE_NUM(doing_site + 1);
    return m_cStdfParse.WriteRecord(&clPir);
}

BOOL CDL4toSTDF::STDF_PTR_Record_Write (BOOL *semi_static_sent, gsint32 rec_num,
                            const dlog_entry *sub_test_results,
                            FUNCTION_SUBTEST_DATA *pFSD,
                            short doing_site, short func_i,	short subtest_i,
                            short subtest_count,gsuint32 **fails,gsuint32 **exec,
                            DLOG_DB_VLD *pVLD,
                            bool limits_changed)
{

    ASSERT_LOCATION(pFSD);
    ASSERT_LOCATION(pVLD);

    GQTL_STDF::Stdf_PTR_V4 clPtr;

    m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PTR]++;

    // Initialization
    clPtr.SetTEST_NUM(1);
    clPtr.SetHEAD_NUM(1);
    clPtr.SetSITE_NUM(1);
    clPtr.SetTEST_FLG(0);
    clPtr.SetPARM_FLG(0);
    clPtr.SetRESULT(0);


    float limit;
    //CHANGES BEGIN 6 July 2006
    STDF_STRINGS *ss  = GetDBHeader()->m_STDF_STRINGS;
    //CHANGES END 6 July 2006

    clPtr.SetTEST_NUM(((func_i + 1) * 1000) + (subtest_i + 1));
    clPtr.SetSITE_NUM(doing_site + 1);

    stdf_type_b1 test_flags = 0;
    if(sub_test_results[subtest_count].passed_fail == (char)UNKNOWN_RESULT)
    {
        test_flags |= STDF_MASK_BIT1;
        test_flags |= STDF_MASK_BIT2;
        test_flags |= STDF_MASK_BIT4;
        test_flags |= STDF_MASK_BIT5;
        test_flags |= STDF_MASK_BIT6;
    }
    else
    {

        test_flags &= ~STDF_MASK_BIT1;
        test_flags &= ~STDF_MASK_BIT2;
        test_flags &= ~STDF_MASK_BIT4;
        test_flags &= ~STDF_MASK_BIT5;
        test_flags &= ~STDF_MASK_BIT6;
        ++exec[doing_site][subtest_count];

        if(sub_test_results[subtest_count].passed_fail == (char)FAILED_TEST)
        {
            test_flags |= STDF_MASK_BIT7;
            ++fails[doing_site][subtest_count];
        }
    }
    test_flags &= ~STDF_MASK_BIT3;
    clPtr.SetTEST_FLG(test_flags);

    clPtr.SetRESULT(sub_test_results[subtest_count].measured_value);


    stdf_type_b1 opt_flag = STDF_MASK_BIT1;
    stdf_type_b1 parm_flag = (stdf_type_b1) (STDF_MASK_BIT6|STDF_MASK_BIT7); // Low and High Limit not strict
    char pass_fail = sub_test_results[subtest_count].passed_fail;
    if (pass_fail > 3 || pass_fail < 0)
        pass_fail = 3;

    //Default opt_flag settings (3F)
    opt_flag |= STDF_MASK_BIT0;
    opt_flag |= STDF_MASK_BIT1;
    opt_flag |= STDF_MASK_BIT2;
    opt_flag |= STDF_MASK_BIT3;
    opt_flag |= STDF_MASK_BIT4;
    opt_flag |= STDF_MASK_BIT5;
    opt_flag &= ~STDF_MASK_BIT6;
    opt_flag &= ~STDF_MASK_BIT7;

    //FREmn08525 Set low and high limits to Max float value
    if (rec_num == 0
        || !semi_static_sent[subtest_count]
        || pVLD->Get_DATALOG_VAR_LEN_DATA_PER_SUBTEST (subtest_count)->GetTestLimitsObj ()
        || limits_changed)
    {

        opt_flag &= ~STDF_MASK_BIT0;
        //=================== LOW LIMIT =========================
        opt_flag &= ~STDF_MASK_BIT4;
        if(pFSD->m_min_limit_active)
        {
            opt_flag &= ~STDF_MASK_BIT6;
        }
        else
        {
            //MIN Limit is set to "none"
            opt_flag &= ~STDF_MASK_BIT4;
            opt_flag |= STDF_MASK_BIT6;
        }
        //=================== HIGH LIMIT =========================
        opt_flag &= ~STDF_MASK_BIT5;
        if(pFSD->m_max_limit_active)
        {
            opt_flag &= ~STDF_MASK_BIT7;

        }
        else
        {
            //MAX limit is set to "none"
            opt_flag &= ~STDF_MASK_BIT5;
            opt_flag |= STDF_MASK_BIT7;
        }
        // if some result to save
        if((opt_flag & 0x00ff) != 0x00ff)
        {
            clPtr.SetTEST_TXT("");
            clPtr.SetALARM_ID("");

            clPtr.SetOPT_FLAG(0);
            clPtr.SetRES_SCAL(0);
            clPtr.SetLLM_SCAL(0);
            clPtr.SetHLM_SCAL(0);
            clPtr.SetLO_LIMIT(stdf_type_r4(FLT_MAX*(-1)));
            clPtr.SetHI_LIMIT(stdf_type_r4(FLT_MAX));
            clPtr.SetUNITS("");
            clPtr.SetC_RESFMT("");
            clPtr.SetC_LLMFMT("");
            clPtr.SetC_HLMFMT("");

            if(sub_test_results[subtest_count].power == POWER_HEX)
                clPtr.SetRES_SCAL(0);  // Set to Units
            else
                clPtr.SetRES_SCAL(power_translate[(int)(sub_test_results[subtest_count].power)]);
            //=================== LOW LIMIT =========================
            opt_flag &= ~STDF_MASK_BIT4;
            if(pFSD->m_min_limit_active)
            {
                opt_flag &= ~STDF_MASK_BIT6;
                clPtr.SetLLM_SCAL(power_translate[(int)(sub_test_results[subtest_count].power)]);

                if((unsigned char)sub_test_results[subtest_count].passed_fail < 4)
                {
                    if(sub_test_results[subtest_count].power == POWER_HEX)
                        limit = (float)pFSD->m_min_limit[(int)(sub_test_results[subtest_count].passed_fail)];
                    else
                        limit = pFSD->m_min_limit[(int)(sub_test_results[subtest_count].passed_fail)];
                }
                else
                {
                    parm_flag |= STDF_MASK_BIT4;
                    if(sub_test_results[subtest_count].power == POWER_HEX)
                        limit = (float)pFSD->m_min_limit[3];
                    else
                        limit = pFSD->m_min_limit[3];
                }

                // Override displayed limit value if applicable
                if (pVLD->Get_DATALOG_VAR_LEN_DATA_PER_SUBTEST (subtest_count))
                {
                    TestLimits *overridden_limits = pVLD->Get_DATALOG_VAR_LEN_DATA_PER_SUBTEST (subtest_count)->GetTestLimitsObj ();
                    if (overridden_limits) // Contains overriden test limits
                        overridden_limits->GetMinLimitValue ((short)pass_fail,limit);
                }
                clPtr.SetLO_LIMIT(limit);
            }
            else
            {
                //MIN Limit is set to "none"
                opt_flag &= ~STDF_MASK_BIT4;
                opt_flag |= STDF_MASK_BIT6;
            }

            //=================== HIGH LIMIT =========================
            opt_flag &= ~STDF_MASK_BIT5;
            if(pFSD->m_max_limit_active)
            {
                opt_flag &= ~STDF_MASK_BIT7;
                clPtr.SetHLM_SCAL(power_translate[(int)(sub_test_results[subtest_count].power)]);
                if((unsigned char)sub_test_results[subtest_count].passed_fail < 4)
                {
                    if(sub_test_results[subtest_count].power == POWER_HEX)
                        limit = (float)pFSD->m_max_limit[(int)(sub_test_results[subtest_count].passed_fail)];
                    else
                        limit = pFSD->m_max_limit[(int)(sub_test_results[subtest_count].passed_fail)];
                }
                else
                {
                    parm_flag |= STDF_MASK_BIT3;
                    if(sub_test_results[subtest_count].power == POWER_HEX)
                        limit = (float)pFSD->m_max_limit[3];
                    else
                        limit = pFSD->m_max_limit[3];
                }
                // Override displayed limit value if applicable
                if (pVLD->Get_DATALOG_VAR_LEN_DATA_PER_SUBTEST (subtest_count))
                {
                    TestLimits *overridden_limits = pVLD->Get_DATALOG_VAR_LEN_DATA_PER_SUBTEST (subtest_count)->GetTestLimitsObj ();
                    if (overridden_limits) // Contains overriden test limits
                        overridden_limits->GetMaxLimitValue ((short)pass_fail,limit);
                }
                clPtr.SetHI_LIMIT(limit);
            }
            else
            {
                //MAX limit is set to "none"
                opt_flag &= ~STDF_MASK_BIT5;
                opt_flag |= STDF_MASK_BIT7;
            }

            QString name = (const char *)(pFSD->m_subtest_name);
            QString note = (const char *)(pFSD->m_note);
//CHANGES BEGIN 14 SEPTEMBER 2006
            if(!note.isEmpty())
            {
                name += ", ";
                name += note;
            }
//CHANGES END 14 SEPTEMBER 2006
            QString units = (const char *)(pFSD->m_units);

            clPtr.SetTEST_TXT(name.toLatin1().constData());
            clPtr.SetUNITS(units.toLatin1().constData());

            if(sub_test_results[subtest_count].power == POWER_HEX)
            {
                const char *hex = "%04X";

                clPtr.SetC_RESFMT(hex);
                clPtr.SetC_LLMFMT(hex);
                clPtr.SetC_HLMFMT(hex);
            }
            else
            {
                const char *flt = "%1.6e";

                if (ss != NULL && ss->Get_PTR_C_RESFMT() != NULL)
                    clPtr.SetC_RESFMT(ss->Get_PTR_C_RESFMT());
                else
                    clPtr.SetC_RESFMT(flt);

                if (ss != NULL && ss->Get_PTR_C_LLMFMT() != NULL)
                    clPtr.SetC_LLMFMT(ss->Get_PTR_C_LLMFMT());
                else
                    clPtr.SetC_LLMFMT(flt);

                if (ss != NULL && ss->Get_PTR_C_HLMFMT() != NULL)
                    clPtr.SetC_HLMFMT(ss->Get_PTR_C_HLMFMT());
                else
                    clPtr.SetC_HLMFMT(flt);

            }
            semi_static_sent[subtest_count] = true;

            clPtr.SetOPT_FLAG(opt_flag);
            clPtr.SetPARM_FLG(parm_flag);
        }
    }
    else
    {

        //default data
        BOOL min = pFSD->m_min_limit_active;
        BOOL max = pFSD->m_max_limit_active;

        if(min && max)
        {
            opt_flag |= STDF_MASK_BIT4;
            opt_flag |= STDF_MASK_BIT5;
            opt_flag &= ~STDF_MASK_BIT6;
            opt_flag &= ~STDF_MASK_BIT7;
        }
        else if(!min && !max)
        {
            opt_flag |= STDF_MASK_BIT4;
            opt_flag |= STDF_MASK_BIT5;
            opt_flag |= STDF_MASK_BIT6;
            opt_flag |= STDF_MASK_BIT7;

        }
        else if(!min && max)
        {
            opt_flag |= STDF_MASK_BIT4;
            opt_flag |= STDF_MASK_BIT5;
            opt_flag |= STDF_MASK_BIT6;
            opt_flag &= ~STDF_MASK_BIT7;
        }
        else if(min && !max)
        {
            opt_flag |= STDF_MASK_BIT4;
            opt_flag |= STDF_MASK_BIT5;
            opt_flag &= ~STDF_MASK_BIT6;
            opt_flag |= STDF_MASK_BIT7;
        }

        if((opt_flag & 0x00ff) != 0x00ff)
        {
            clPtr.SetTEST_TXT("");
            clPtr.SetALARM_ID("");

            if(sub_test_results[subtest_count].power == POWER_HEX)
                clPtr.SetRES_SCAL(0);  // Set to Units
            else
                clPtr.SetRES_SCAL(power_translate[(int)(sub_test_results[subtest_count].power)]);

            clPtr.SetOPT_FLAG(opt_flag);
            clPtr.SetPARM_FLG(parm_flag);
        }
    }


    return m_cStdfParse.WriteRecord(&clPtr);
}
BOOL CDL4toSTDF::STDF_PRR_Record_Write (LOT_SUMMARY_DATA *pLSD,const test_header &header, short doing_site, short subtest_count)
{
    char str[80];
    GQTL_STDF::Stdf_PRR_V4 clPrr;

    m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PRR]++;

    //CHANGES BEGIN 6 July 2006
    STDF_STRINGS *ss  = GetDBHeader()->m_STDF_STRINGS;
    //CHANGES END 6 July 2006

    clPrr.SetHEAD_NUM(1);
    clPrr.SetSITE_NUM(doing_site + 1);
    stdf_type_b1 part_flg = 0;
    if(header.ret_val != 1)
        part_flg |= STDF_MASK_BIT3;

    clPrr.SetPART_FLG(part_flg);
    clPrr.SetNUM_TEST(subtest_count);

    clPrr.SetHARD_BIN(atoi( pLSD->HardwareBin(header.bin_num-1) ));

    clPrr.SetSOFT_BIN(header.bin_num);

    // Beginning with the last field for initialization
    BOOL	bInitPreviousField = false;

    //sprintf(str,"%lu.%hu",header.serial_num, doing_site + 1);
//CHANGES BEGIN 28 June 2006
    if (ss != NULL)
    {
        switch(ss->Get_PRR_PART_ID_OPTIONS())
        {
        case SERIAL_NUM_AND_SITE_NUM:
        default:
            sprintf(str,"%u.%u",header.serial_num, doing_site + 1);
            break;

        case SERIAL_NUM_ONLY:
            sprintf(str,"%u",header.serial_num);
            break;
        }
    }
    else
        sprintf(str,"%u.%u",header.serial_num, doing_site + 1);
// CHANGES END 28 JUNE 2006

    if (strcmp(str, "") != 0) {
        clPrr.SetPART_ID(str);
        bInitPreviousField = true;
    }

    if(bInitPreviousField)
        clPrr.SetTEST_T(0);

    if(header.y_coordinate != -32768){
        clPrr.SetY_COORD(header.y_coordinate);
        bInitPreviousField = true;
    }else if(bInitPreviousField)
        clPrr.SetY_COORD(-32768);

    if(header.x_coordinate != -32768){
        clPrr.SetX_COORD(header.x_coordinate);
        bInitPreviousField = true;
    }else if(bInitPreviousField)
        clPrr.SetX_COORD(-32768);

//CHANGES BEGIN 21 September 2006
    if(header.test_header_supplement.test_time != 0){
        clPrr.SetTEST_T((int)(header.test_header_supplement.test_time));
        bInitPreviousField = true;
    }else if(bInitPreviousField)
        clPrr.SetTEST_T(0);
//CHANGES END 21 September 2006

    return m_cStdfParse.WriteRecord(&clPrr);
}

BOOL CDL4toSTDF::STDF_HBR_Record_Write (LOT_SUMMARY_DATA *pLSD,
                            short sites_i,
                            gsuint32 **bin_count)
{
    GQTL_STDF::Stdf_HBR_V4 clHbr;

    m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_HBR]++;

    clHbr.SetHEAD_NUM(255);
    clHbr.SetSITE_NUM(255);
    clHbr.SetHBIN_NUM(0);
    clHbr.SetHBIN_CNT(0);

    gsint32 **hbin_counts, *hp;
    bool hbin_exists[MAXIMUM_BINS];
    char hbin_pf[MAXIMUM_BINS];
    int i, site, sbin, hbin;
    const char *binname;

    hbin_counts = (gsint32 **)calloc(sites_i + 1, sizeof(gsint32 *));
    for (i = 0; i < sites_i + 1; ++i)
    {
        hbin_counts[i] = (gsint32 *)calloc(MAXIMUM_BINS, sizeof(gsint32));
    }

    for(i = 0; i < MAXIMUM_BINS; ++i)
    {
        hbin_exists[i] = false;
    }

    for (sbin = 0; sbin < pLSD->BinsN(); ++sbin)
    {
        binname = pLSD->HardwareBin(sbin);
        if (binname == NULL || *binname == '\0' || (hbin = atoi(binname)) < 0 || hbin >= MAXIMUM_BINS)
        {
            continue;
        }
        hbin_exists[hbin] = true;
        for (site = 0; site < sites_i; ++site)
        {
            hp = hbin_counts[0];
            *(hp + hbin) += bin_count[site][sbin];
            hp = hbin_counts[site + 1];
            *(hp + hbin) += bin_count[site][sbin];
        }
        if (sbin < 4)
        {
            hbin_pf[hbin] = HBR_PASS_BIN;
        }
        else
        {
            hbin_pf[hbin] = HBR_FAIL_BIN;
        }
    }

    for (hbin = 0; hbin < MAXIMUM_BINS; ++hbin)
    {
        if (hbin_exists[hbin] == false)
        {
            continue;
        }

        clHbr.SetHEAD_NUM(255);
        clHbr.SetSITE_NUM(255);
        clHbr.SetHBIN_NUM(hbin);
        if(hbin_pf[hbin] != ' ')
            clHbr.SetHBIN_PF(hbin_pf[hbin]);

        hp = hbin_counts[0];
        clHbr.SetHBIN_CNT(*(hp + hbin));

        if(!m_cStdfParse.WriteRecord(&clHbr)) return false;

        if (sites_i > 1) {
            clHbr.SetHEAD_NUM(1);
            for (short file_site = 0; file_site < sites_i; ++file_site) {
                clHbr.SetSITE_NUM(file_site + 1);
                hp = hbin_counts[file_site + 1];
                clHbr.SetHBIN_CNT(*(hp + hbin));
                if(!m_cStdfParse.WriteRecord(&clHbr)) return false;
            }
        }
        clHbr.Reset();
    }

    for (i = 0; i < sites_i + 1; ++i) {
        free(hbin_counts[i]);
    }
    free(hbin_counts);

    return true;
}

BOOL CDL4toSTDF::STDF_SBR_Record_Write (LOT_SUMMARY_DATA *pLSD,
                            short sites_i,
                            gsuint32 **bin_count)
{
    GQTL_STDF::Stdf_SBR_V4 clSbr;

    m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_SBR]++;

    clSbr.SetHEAD_NUM(255);
    clSbr.SetSITE_NUM(255);
    clSbr.SetSBIN_NUM(0);
    clSbr.SetSBIN_CNT(0);

    for(int i = 0; i < pLSD->BinsN (); i++)
    {
        clSbr.SetHEAD_NUM(255);
        clSbr.SetSITE_NUM(255) ; // reset required for multiple site

        clSbr.SetSBIN_NUM(i + 1);
        if((strcmp(pLSD->BinName (i), "") != 0) && (strcmp(pLSD->BinName (i), " ") != 0))
            clSbr.SetSBIN_NAM(pLSD->BinName (i));
        if(i <= 3)
            clSbr.SetSBIN_PF(SBR_PASS_BIN);
        else
            clSbr.SetSBIN_PF(SBR_FAIL_BIN);


        int iCnt = 0;
        for(short file_site = 0; file_site < sites_i; file_site++)
            iCnt += bin_count[file_site][i];
        clSbr.SetSBIN_CNT(iCnt);
        if (strlen (pLSD->BinName (i)) != 0 || iCnt != 0)
        {
            if(!m_cStdfParse.WriteRecord(&clSbr)) return false;
        }

        if (sites_i > 1)
        {
            clSbr.SetHEAD_NUM(1);
            for(short file_site = 0; file_site < sites_i; file_site++)
            {
                if (strlen (pLSD->BinName (i)) != 0 || bin_count[file_site][i] != 0)
                {
                    clSbr.SetSITE_NUM(file_site + 1);
                    clSbr.SetSBIN_CNT(bin_count[file_site][i]);
                    if(!m_cStdfParse.WriteRecord(&clSbr)) return false;
                }
            }
        }
        clSbr.Reset();
    }

    return true;
}

BOOL CDL4toSTDF::STDF_PCR_Record_Write (short sites_i, gsuint32 **bin_count,
                            gsuint32 *loop)
{
    GQTL_STDF::Stdf_PCR_V4 clPcr;
    //CHANGES BEGIN 6 July 2006
    STDF_STRINGS *ss  = GetDBHeader()->m_STDF_STRINGS;
    //CHANGES END 6 July 2006

    m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PCR]++;

    if(!ss)
    {
        ErrorMessage("Can't allocate enough memory to perform database operation.");
        return false;
    }


    // Note: Count values only used for total record.
    // Site values not available.
    gsuint32 part_cnt = (ss) ? ss->Get_PCR_PART_CNT() : 0L;
    gsuint32 rtst_cnt = (ss) ? ss->Get_PCR_RTST_CNT() : LONG_INVALID_DATA;
    gsuint32 abrt_cnt = (ss) ? ss->Get_PCR_ABRT_CNT() : LONG_INVALID_DATA;
    gsuint32 good_cnt = (ss) ? ss->Get_PCR_GOOD_CNT() : 0L;
    gsuint32 func_cnt = (ss) ? ss->Get_PCR_FUNC_CNT() : 0L;

    clPcr.SetHEAD_NUM(255);
    clPcr.SetSITE_NUM(255);
    clPcr.SetPART_CNT(0L);

    gsuint32 pcr_part_cnt = 0L;
    gsuint32 pcr_good_cnt = 0L;
    gsuint32 pcr_func_cnt = 0L;
    //FIRST, do an aggregate of each site
    for(short file_site = 0; file_site < sites_i; file_site++)
    {
        pcr_good_cnt += bin_count[file_site][0] + bin_count[file_site][1] +
                         bin_count[file_site][2] + bin_count[file_site][3];
        pcr_part_cnt += loop[file_site];
        pcr_func_cnt += loop[file_site];
    }
    if (part_cnt != 0L)
        clPcr.SetPART_CNT(part_cnt);
    else
        clPcr.SetPART_CNT(pcr_part_cnt);


    // Optionals Fields

    // Beginning with the last field for initialization
    BOOL	bInitPreviousField = false;

    if (func_cnt != 0L) {
        clPcr.SetFUNC_CNT(func_cnt);
        bInitPreviousField = true;
    } else if (pcr_func_cnt != 0L) {
        clPcr.SetFUNC_CNT(pcr_func_cnt);
        bInitPreviousField = true;
    }


    if (good_cnt != 0L){
        clPcr.SetGOOD_CNT(good_cnt);
        bInitPreviousField = true;
    } else if (pcr_good_cnt != 0L){
        clPcr.SetGOOD_CNT(pcr_good_cnt);
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clPcr.SetGOOD_CNT((stdf_type_u4)LONG_INVALID_DATA);

    if (abrt_cnt != LONG_INVALID_DATA) {
        clPcr.SetABRT_CNT(abrt_cnt);
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clPcr.SetABRT_CNT((stdf_type_u4)LONG_INVALID_DATA);

    if (rtst_cnt != LONG_INVALID_DATA)
        clPcr.SetRTST_CNT(rtst_cnt);
    else if(bInitPreviousField)
        clPcr.SetRTST_CNT((stdf_type_u4)LONG_INVALID_DATA);

    if(!m_cStdfParse.WriteRecord(&clPcr)) return false;

    //NEXT: do each site individually
    if (sites_i > 1)
    {
        for(short file_site = 0; file_site < sites_i; file_site++)
        {
            clPcr.Reset();
            bInitPreviousField = false;
            clPcr.SetHEAD_NUM(1);
            clPcr.SetSITE_NUM(file_site + 1);
            clPcr.SetPART_CNT(loop[file_site]);
            good_cnt =	bin_count[file_site][0] + bin_count[file_site][1] +
                                bin_count[file_site][2] + bin_count[file_site][3];
            func_cnt = loop[file_site];

            if (func_cnt != 0L) {
                clPcr.SetFUNC_CNT(func_cnt);
                bInitPreviousField = true;
            }

            if (good_cnt != 0L){
                clPcr.SetGOOD_CNT(good_cnt);
                bInitPreviousField = true;
            } else if(bInitPreviousField)
                clPcr.SetGOOD_CNT((stdf_type_u4)LONG_INVALID_DATA);

            if(bInitPreviousField)
                clPcr.SetABRT_CNT((stdf_type_u4)LONG_INVALID_DATA);

            if(bInitPreviousField)
                clPcr.SetRTST_CNT((stdf_type_u4)LONG_INVALID_DATA);


            if(!m_cStdfParse.WriteRecord(&clPcr)) return false;
        }
    }

    return true;
}

BOOL CDL4toSTDF::STDF_MRR_Record_Write ( gsuint32 *last_time,
                            short sites_i)
{
    GQTL_STDF::Stdf_MRR_V4 clMrr;

    m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_MRR]++;

    //Since data from all sites will be written to one .std file, we
    //just want to get the latest finish time regardless of which site
    //it occured on
    gsuint32 utime = last_time[0];
    for(int s=0; s<sites_i; s++)
    {
        if(last_time[s] > utime)
            utime = last_time[s];
    }

    clMrr.SetFINISH_T(utime);

    // Optionals Fields

    // Beginning with the last field for initialization
    BOOL	bInitPreviousField = false;
    //CHANGES BEGIN 6 July 2006
    STDF_STRINGS *ss  = GetDBHeader()->m_STDF_STRINGS;
    //CHANGES END 6 July 2006

    if (ss != NULL && ss->Get_MRR_EXC_DESC() != NULL) {
        clMrr.SetEXC_DESC(ss->Get_MRR_EXC_DESC());
        bInitPreviousField = true;
    }

    if (ss != NULL && ss->Get_MRR_USR_DESC() != NULL) {
        clMrr.SetUSR_DESC(ss->Get_MRR_USR_DESC());
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clMrr.SetUSR_DESC("");

    if (ss != NULL && ss->Get_MRR_DISP_COD() != 0) {
        clMrr.SetDISP_COD(ss->Get_MRR_DISP_COD());
    } else if(bInitPreviousField)
        clMrr.SetDISP_COD(' ');

    return m_cStdfParse.WriteRecord(&clMrr);
}

BOOL CDL4toSTDF::STDF_TSR_Record_Write (short sites_i,
                            gsuint32 **fails,gsuint32 **exec)
{
    GQTL_STDF::Stdf_TSR_V4 clTsr;

    m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_TSR]++;

    clTsr.SetHEAD_NUM(255);
    clTsr.SetSITE_NUM(255);
    clTsr.SetTEST_TYP(' ');
    clTsr.SetTEST_NUM(255);

    PROGRAM_FUNCTIONS_DATA *pPFD = Get_PROGRAM_FUNCTIONS_DATA ();
    ASSERT_LOCATION(pPFD);

    STDF_SUPPLEMENT *pSS = GetDBHeader()->m_STDF_SUPPLEMENT;
    // pSS can be NULL

    gsuint32 *cnt;
    short subtest_total = 0;
    for (short func_i = 0; func_i < pPFD->FunctionsN (); func_i++)
    {
        ONE_FUNCTION *pOF = pPFD->Function (func_i);
        ASSERT_LOCATION(pOF);

        for (short subtest_i = 0; subtest_i < pOF->SubTestsN (); subtest_i++)
        {
            // Optionals Fields

            // Beginning with the last field for initialization
            BOOL	bInitPreviousField = false;


            FUNCTION_SUBTEST_DATA *pFSD = pOF->SubTestData (subtest_i);
            ASSERT_LOCATION(pFSD);

            // At this moment in time no calculations are done on the test results,
            // so the option flags are unused.

            gsuint32 tsr_exec_cnt = 0;
            gsuint32 tsr_fail_cnt = 0;
            QString name = (const char *)(pFSD->m_subtest_name);
            QString note = (const char *)(pFSD->m_note);


            clTsr.SetHEAD_NUM(255);
            clTsr.SetSITE_NUM(255);
            clTsr.SetTEST_TYP(TEST_TYPE_PARAMETRIC);
            clTsr.SetTEST_NUM(((func_i + 1) * 1000) + (subtest_i + 1));
            if (pFSD->m_TST_TYP == TEST_TYPE_FUNCTIONAL) clTsr.SetTEST_TYP(TEST_TYPE_FUNCTIONAL); else
            if (pFSD->m_TST_TYP == TEST_TYPE_MULTIPLE  ) clTsr.SetTEST_TYP(TEST_TYPE_MULTIPLE)  ; else
                                                         clTsr.SetTEST_TYP(TEST_TYPE_PARAMETRIC);

            short doing_site;
            for (doing_site = 0; doing_site < sites_i; doing_site++)
            {
                if (pSS != NULL)
                {
                    cnt = pSS->m_tsr_exec_cnts[doing_site];
                    tsr_exec_cnt += (cnt != NULL) ? cnt[subtest_total] : 0;
                    cnt = pSS->m_tsr_fail_cnts[doing_site];
                    tsr_fail_cnt += (cnt != NULL) ? cnt[subtest_total] : 0;
                }
                else
                {
                    tsr_exec_cnt += exec[doing_site][subtest_total];
                    tsr_fail_cnt += fails[doing_site][subtest_total];
                }
            }

            if(note != "") {
                clTsr.SetTEST_LBL(note.toLatin1().constData());
                bInitPreviousField = true;
            }

            if(bInitPreviousField)
                clTsr.SetSEQ_NAME("");

            if(name != "")
                clTsr.SetTEST_NAM(name.toLatin1().constData());
            else if(bInitPreviousField)
                clTsr.SetTEST_NAM("");

            clTsr.SetEXEC_CNT(tsr_exec_cnt);
            clTsr.SetFAIL_CNT(tsr_fail_cnt);
            clTsr.SetALRM_CNT(0L);

            if(!m_cStdfParse.WriteRecord(&clTsr))
            {
                delete pPFD;
                delete pSS;
                delete pOF;
                return false;
            }

            if (sites_i > 1)
            {
                for (doing_site = 0; doing_site < sites_i; doing_site++)
                {
                    clTsr.SetHEAD_NUM(1);
                    clTsr.SetSITE_NUM(doing_site + 1);
                    if (pSS != NULL)
                    {
                        cnt = pSS->m_tsr_exec_cnts[doing_site];
                        clTsr.SetEXEC_CNT((cnt != NULL) ? cnt[subtest_total] : 0);
                        cnt = pSS->m_tsr_fail_cnts[doing_site];
                        clTsr.SetFAIL_CNT((cnt != NULL) ? cnt[subtest_total] : 0);
                    }
                    else
                    {
                        clTsr.SetEXEC_CNT(exec[doing_site][subtest_total]);
                        clTsr.SetFAIL_CNT(fails[doing_site][subtest_total]);
                    }
                    if(!m_cStdfParse.WriteRecord(&clTsr))
                    {
                        delete pPFD;
                        delete pSS;
                        delete pOF;
                        return false;
                    }
                }
            }
            clTsr.Reset();

            subtest_total++;
        }
    }
    return true;
}

BOOL CDL4toSTDF::STDF_WRR_Record_Write ( gsuint32 *last_time,
                            gsuint32 **bin_count, gsuint32 *loop,
                            short sites_i)
{
    GQTL_STDF::Stdf_WRR_V4 clWrr;

    m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_WRR]++;

    clWrr.SetHEAD_NUM(1);
    clWrr.SetSITE_GRP(1);
    clWrr.SetFINISH_T(1);
    clWrr.SetPART_CNT(0);

    STDF_STRINGS *ss = GetDBHeader()->m_STDF_STRINGS;//g_STDF_STRINGS;
    gsuint32 part_cnt = (ss) ? ss->Get_WRR_PART_CNT() : 0L;
    gsuint32 rtst_cnt = (ss) ? ss->Get_WRR_RTST_CNT() : LONG_INVALID_DATA;
    gsuint32 abrt_cnt = (ss) ? ss->Get_WRR_ABRT_CNT() : LONG_INVALID_DATA;
    gsuint32 good_cnt = (ss) ? ss->Get_WRR_GOOD_CNT() : 0L;
    gsuint32 func_cnt = (ss) ? ss->Get_WRR_FUNC_CNT() : 0L;

    gsuint32 wrr_part_cnt = 0;
    gsuint32 wrr_good_cnt = 0;
    gsuint32 wrr_func_cnt = 0;
    //Since data from all sites will be written to one .std file, we
    //just want to get the latest finish time regardless of which site
    //it occured on
    gsuint32 utime = last_time[0];
    for(short file_site = 0; file_site < sites_i; file_site++)
    {
        if(last_time[file_site] > utime)
            utime = last_time[file_site];

        wrr_good_cnt += bin_count[file_site][0] +
                                 bin_count[file_site][1] +
                                 bin_count[file_site][2] +
                                 bin_count[file_site][3];
        wrr_part_cnt += loop[file_site];
        wrr_func_cnt += loop[file_site];
    }
    clWrr.SetFINISH_T(utime);

    if (part_cnt != 0L)
        clWrr.SetPART_CNT(part_cnt);
    else
        clWrr.SetPART_CNT(wrr_part_cnt);


    // Optionals Fields

    // Beginning with the last field for initialization
    BOOL	bInitPreviousField = false;

    if (ss != NULL && ss->Get_WRR_EXC_DESC() != NULL) {
        clWrr.SetEXC_DESC(ss->Get_WRR_EXC_DESC());
        bInitPreviousField = true;
    }

    if (ss != NULL && ss->Get_WRR_USR_DESC() != NULL) {
        clWrr.SetUSR_DESC(ss->Get_WRR_USR_DESC());
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clWrr.SetUSR_DESC("");

    if (ss != NULL && ss->Get_WRR_MASK_ID() != NULL) {
        clWrr.SetMASK_ID(ss->Get_WRR_MASK_ID());
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clWrr.SetMASK_ID("");

    if (ss != NULL && ss->Get_WRR_FRAME_ID() != NULL) {
        clWrr.SetFRAME_ID(ss->Get_WRR_FRAME_ID());
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clWrr.SetFRAME_ID("");

    if (ss != NULL && ss->Get_WRR_FABWF_ID() != NULL) {
        clWrr.SetFABWF_ID(ss->Get_WRR_FABWF_ID());
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clWrr.SetFABWF_ID("");

    if (ss != NULL && ss->Get_WRR_WAFER_ID() != NULL) {
        clWrr.SetWAFER_ID(ss->Get_WRR_WAFER_ID());
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clWrr.SetWAFER_ID("");


    if (func_cnt != 0L) {
        clWrr.SetFUNC_CNT(func_cnt);
        bInitPreviousField = true;
    } else if (wrr_func_cnt != 0L) {
        clWrr.SetFUNC_CNT(wrr_func_cnt);
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clWrr.SetFUNC_CNT((stdf_type_u4)LONG_INVALID_DATA);

    if (good_cnt != 0L) {
        clWrr.SetGOOD_CNT(good_cnt);
        bInitPreviousField = true;
    } else if (wrr_good_cnt != 0L) {
        clWrr.SetGOOD_CNT(wrr_good_cnt);
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clWrr.SetGOOD_CNT((stdf_type_u4)LONG_INVALID_DATA);

    if (abrt_cnt != LONG_INVALID_DATA) {
        clWrr.SetABRT_CNT(abrt_cnt);
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clWrr.SetABRT_CNT((stdf_type_u4)LONG_INVALID_DATA);

    if (rtst_cnt != LONG_INVALID_DATA) {
        clWrr.SetRTST_CNT(rtst_cnt);
        bInitPreviousField = true;
    } else if(bInitPreviousField)
        clWrr.SetRTST_CNT((stdf_type_u4)LONG_INVALID_DATA);

    return m_cStdfParse.WriteRecord(&clWrr);
}


PROGRAM_FUNCTIONS_DATA *CDL4toSTDF::get_pfd (DLOG_DB_FLD *FLD)
{
    if (FLD == 0)
    {
        if (!m_last_FLD)
        {
            m_last_FLD = new DLOG_DB_FLD;
            ASSERT_LOCATION(m_last_FLD);
            if (!m_last_FLD)
            {
                ErrorMessage("Can't allocate enough memory to perform database operation.");
                return NULL;
            }

            if(!BinaryDatabase::ReadFLD (*m_last_FLD,0))
            {
                ErrorMessage(BinaryDatabase::GetLastError().toLatin1().constData());
                delete m_last_FLD;
                return NULL;
            }
        }

        FLD = m_last_FLD;
    }

    gsint32 idx = FLD->GetProgramInfoIdx ();
    DLOG_PROGRAM_INFO *dpi = m_HDR->GetPrgInfo (idx);
    if (!dpi)
    {
        ErrorMessage("Can't allocate enough memory to perform database operation.");
        return NULL;
    }

    PROGRAM_FUNCTIONS_DATA *pfd = dpi->PrgInfo ();
    ASSERT_LOCATION(pfd);

    return pfd;
}

///////////////////////////////////////// STDF_STRINGS /////////////////////////////////////////
// SAME AS THE ORIGINAL STDF_Strings.cpp
////////////////////////////////////////////////////////////////////////////////////////////////

#define STDF_CFREE(member) \
    if (member != NULL) { \
        free(member); \
        member = NULL; \
    }

#define STDF_CSET(member, value) \
    STDF_CFREE(member) \
    if (value) { \
        member = strdup(value); \
    }

void STDF_STRINGS::Init_Counters_Only(void)
{
    m_PCR_PART_CNT = 0L;
    m_PCR_RTST_CNT = LONG_INVALID_DATA;//0L;
    m_PCR_ABRT_CNT = LONG_INVALID_DATA;//0L;
    m_PCR_GOOD_CNT = 0L;
    m_PCR_FUNC_CNT = 0L;

    m_WRR_PART_CNT = 0L;
    m_WRR_RTST_CNT = LONG_INVALID_DATA;//0L;
    m_WRR_ABRT_CNT = LONG_INVALID_DATA;//0L;
    m_WRR_GOOD_CNT = 0L;
    m_WRR_FUNC_CNT = 0L;
}

void STDF_STRINGS::Init (void)
{
    m_ATR_CMD_LINE = NULL;

//CHANGES BEGIN 28 June 2006
    m_PRR_PART_ID_OPTIONS = SERIAL_NUM_AND_SITE_NUM;
// CHANGES END 28 JUNE 2006

    m_MIR_STAT_NUM = '\0';
    m_MIR_MODE_COD = '\0';
    m_MIR_RTST_COD = '\0';
    m_MIR_PROT_COD = '\0';
    m_MIR_BURN_TIM = 0;
    m_MIR_CMOD_COD = '\0';
    m_MIR_LOT_ID = NULL;
    m_MIR_PART_TYP = NULL;
    m_MIR_NODE_NAM = NULL;
    m_MIR_TSTR_TYP = NULL;
    m_MIR_JOB_NAM = NULL;
    m_MIR_JOB_REV = NULL;
    m_MIR_SBLOT_ID = NULL;
    m_MIR_OPER_NAM = NULL;
    m_MIR_EXEC_TYP = NULL;
    m_MIR_EXEC_VER = NULL;
    m_MIR_TEST_COD = NULL;
    m_MIR_TST_TEMP = NULL;
    m_MIR_USER_TXT = NULL;
    m_MIR_AUX_FILE = NULL;
    m_MIR_PKG_TYP = NULL;
    m_MIR_FAMLY_ID = NULL;
    m_MIR_DATE_COD = NULL;
    m_MIR_FACIL_ID = NULL;
    m_MIR_FLOOR_ID = NULL;
    m_MIR_PROC_ID = NULL;
    m_MIR_OPER_FRQ = NULL;
    m_MIR_SPEC_NAM = NULL;
    m_MIR_SPEC_VER = NULL;
    m_MIR_FLOW_ID = NULL;
    m_MIR_SETUP_ID = NULL;
    m_MIR_DSGN_REV = NULL;
    m_MIR_ENG_ID = NULL;
    m_MIR_ROM_COD = NULL;
    m_MIR_SERL_NUM = NULL;
    m_MIR_SUPR_NAM = NULL;

    m_MRR_DISP_COD = 0;
    m_MRR_USR_DESC = NULL;
    m_MRR_EXC_DESC = NULL;

    m_PCR_PART_CNT = 0L;
    m_PCR_RTST_CNT = LONG_INVALID_DATA;//0L;
    m_PCR_ABRT_CNT = LONG_INVALID_DATA;//0L;
    m_PCR_GOOD_CNT = 0L;
    m_PCR_FUNC_CNT = 0L;

    m_SDR_HAND_TYP = NULL;
    m_SDR_HAND_ID = NULL;
    m_SDR_CARD_TYP = NULL;
    m_SDR_CARD_ID = NULL;
    m_SDR_LOAD_TYP = NULL;
    m_SDR_LOAD_ID = NULL;
    m_SDR_DIB_TYP = NULL;
    m_SDR_DIB_ID = NULL;
    m_SDR_CABL_TYP = NULL;
    m_SDR_CABL_ID = NULL;
    m_SDR_CONT_TYP = NULL;
    m_SDR_CONT_ID = NULL;
    m_SDR_LASR_TYP = NULL;
    m_SDR_LASR_ID = NULL;
    m_SDR_EXTR_TYP = NULL;
    m_SDR_EXTR_ID = NULL;

    m_WIR_WAFER_ID = NULL;

    m_WRR_PART_CNT = 0L;
    m_WRR_RTST_CNT = LONG_INVALID_DATA;//0L;
    m_WRR_ABRT_CNT = LONG_INVALID_DATA;//0L;
    m_WRR_GOOD_CNT = 0L;
    m_WRR_FUNC_CNT = 0L;
    m_WRR_WAFER_ID = NULL;
    m_WRR_FABWF_ID = NULL;
    m_WRR_FRAME_ID = NULL;
    m_WRR_MASK_ID = NULL;
    m_WRR_USR_DESC = NULL;
    m_WRR_EXC_DESC = NULL;

    m_WCR_WAFR_SIZ = 0.0;
    m_WCR_DIE_HT = 0.0;
    m_WCR_DIE_WID = 0.0;
    m_WCR_WF_UNITS = '\0';
    m_WCR_WF_FLAT = '\0';
    m_WCR_CENTER_X = 0;
    m_WCR_CENTER_Y = 0;
    m_WCR_POS_X = '\0';
    m_WCR_POS_Y = '\0';

    m_PTR_C_RESFMT = NULL;
    m_PTR_C_LLMFMT = NULL;
    m_PTR_C_HLMFMT = NULL;
}

void
STDF_STRINGS::Free(void)
{
    STDF_CFREE(m_ATR_CMD_LINE);

    STDF_CFREE(m_MIR_LOT_ID);
    STDF_CFREE(m_MIR_PART_TYP);
    STDF_CFREE(m_MIR_NODE_NAM);
    STDF_CFREE(m_MIR_TSTR_TYP);
    STDF_CFREE(m_MIR_JOB_NAM);
    STDF_CFREE(m_MIR_JOB_REV);
    STDF_CFREE(m_MIR_SBLOT_ID);
    STDF_CFREE(m_MIR_OPER_NAM);
    STDF_CFREE(m_MIR_EXEC_TYP);
    STDF_CFREE(m_MIR_EXEC_VER);
    STDF_CFREE(m_MIR_TEST_COD);
    STDF_CFREE(m_MIR_TST_TEMP);
    STDF_CFREE(m_MIR_USER_TXT);
    STDF_CFREE(m_MIR_AUX_FILE);
    STDF_CFREE(m_MIR_PKG_TYP);
    STDF_CFREE(m_MIR_FAMLY_ID);
    STDF_CFREE(m_MIR_DATE_COD);
    STDF_CFREE(m_MIR_FACIL_ID);
    STDF_CFREE(m_MIR_FLOOR_ID);
    STDF_CFREE(m_MIR_PROC_ID);
    STDF_CFREE(m_MIR_OPER_FRQ);
    STDF_CFREE(m_MIR_SPEC_NAM);
    STDF_CFREE(m_MIR_SPEC_VER);
    STDF_CFREE(m_MIR_FLOW_ID);
    STDF_CFREE(m_MIR_SETUP_ID);
    STDF_CFREE(m_MIR_DSGN_REV);
    STDF_CFREE(m_MIR_ENG_ID);
    STDF_CFREE(m_MIR_ROM_COD);
    STDF_CFREE(m_MIR_SERL_NUM);
    STDF_CFREE(m_MIR_SUPR_NAM);

    STDF_CFREE(m_SDR_HAND_TYP);
    STDF_CFREE(m_SDR_HAND_ID);
    STDF_CFREE(m_SDR_CARD_TYP);
    STDF_CFREE(m_SDR_CARD_ID);
    STDF_CFREE(m_SDR_LOAD_TYP);
    STDF_CFREE(m_SDR_LOAD_ID);
    STDF_CFREE(m_SDR_DIB_TYP);
    STDF_CFREE(m_SDR_DIB_ID);
    STDF_CFREE(m_SDR_CABL_TYP);
    STDF_CFREE(m_SDR_CABL_ID);
    STDF_CFREE(m_SDR_CONT_TYP);
    STDF_CFREE(m_SDR_CONT_ID);
    STDF_CFREE(m_SDR_LASR_TYP);
    STDF_CFREE(m_SDR_LASR_ID);
    STDF_CFREE(m_SDR_EXTR_TYP);
    STDF_CFREE(m_SDR_EXTR_ID);

    STDF_CFREE(m_MRR_USR_DESC);
    STDF_CFREE(m_MRR_EXC_DESC);

    STDF_CFREE(m_WIR_WAFER_ID);

    STDF_CFREE(m_WRR_WAFER_ID);
    STDF_CFREE(m_WRR_FABWF_ID);
    STDF_CFREE(m_WRR_FRAME_ID);
    STDF_CFREE(m_WRR_MASK_ID);
    STDF_CFREE(m_WRR_USR_DESC);
    STDF_CFREE(m_WRR_EXC_DESC);

    STDF_CFREE(m_PTR_C_RESFMT);
    STDF_CFREE(m_PTR_C_LLMFMT);
    STDF_CFREE(m_PTR_C_HLMFMT);
}

gsint32
STDF_STRINGS::FormatStrings (char *buffer)
{
    gsint32	size = 0, len;
    char	localb[1024];

#define STDF_FORMAT(format, member) \
    len = sprintf(localb, format, member); \
    if (buffer) { \
        memcpy(buffer + size, localb, len); \
    } \
    size += len

        // Write ATR fields
    if (m_ATR_CMD_LINE != NULL) {
        STDF_FORMAT("ATR:CMD_LINE:%s\n", m_ATR_CMD_LINE);
    }

//CHANGES BEGIN 28 June 2006
    // Write PRR fields
    STDF_FORMAT("PRR:PART_ID_OPTIONS:%d\n", m_PRR_PART_ID_OPTIONS);
// CHANGES END 28 JUNE 2006

    // Write MIR fields
    if (m_MIR_STAT_NUM != 0) {
        STDF_FORMAT("MIR:STAT_NUM:%d\n", m_MIR_STAT_NUM);
    }
    if (m_MIR_MODE_COD != 0) {
        STDF_FORMAT("MIR:MODE_COD:%c\n", m_MIR_MODE_COD);
    }
    if (m_MIR_RTST_COD != 0) {
        STDF_FORMAT("MIR:RTST_COD:%c\n", m_MIR_RTST_COD);
    }
    if (m_MIR_PROT_COD != 0) {
        STDF_FORMAT("MIR:PROT_COD:%c\n", m_MIR_PROT_COD);
    }
    if (m_MIR_BURN_TIM != 0) {
        STDF_FORMAT("MIR:BURN_TIM:%d\n", m_MIR_BURN_TIM);
    }
    if (m_MIR_CMOD_COD != 0) {
        STDF_FORMAT("MIR:CMOD_COD:%c\n", m_MIR_CMOD_COD);
    }
    if (m_MIR_LOT_ID != NULL) {
        STDF_FORMAT("MIR:LOT_ID:%s\n", m_MIR_LOT_ID);
    }
    if (m_MIR_PART_TYP != NULL) {
        STDF_FORMAT("MIR:PART_TYP:%s\n", m_MIR_PART_TYP);
    }
    if (m_MIR_NODE_NAM != NULL) {
        STDF_FORMAT("MIR:NODE_NAM:%s\n", m_MIR_NODE_NAM);
    }
    if (m_MIR_TSTR_TYP != NULL) {
        STDF_FORMAT("MIR:TSTR_TYP:%s\n", m_MIR_TSTR_TYP);
    }
    if (m_MIR_JOB_NAM != NULL) {
        STDF_FORMAT("MIR:JOB_NAM:%s\n", m_MIR_JOB_NAM);
    }
    if (m_MIR_JOB_REV != NULL) {
        STDF_FORMAT("MIR:JOB_REV:%s\n", m_MIR_JOB_REV);
    }
    if (m_MIR_SBLOT_ID != NULL) {
        STDF_FORMAT("MIR:SBLOT_ID:%s\n", m_MIR_SBLOT_ID);
    }
    if (m_MIR_OPER_NAM != NULL) {
        STDF_FORMAT("MIR:OPER_NAM:%s\n", m_MIR_OPER_NAM);
    }
    if (m_MIR_EXEC_TYP != NULL) {
        STDF_FORMAT("MIR:EXEC_TYP:%s\n", m_MIR_EXEC_TYP);
    }
    if (m_MIR_EXEC_VER != NULL) {
        STDF_FORMAT("MIR:EXEC_VER:%s\n", m_MIR_EXEC_VER);
    }
    if (m_MIR_TEST_COD != NULL) {
        STDF_FORMAT("MIR:TEST_COD:%s\n", m_MIR_TEST_COD);
    }
    if (m_MIR_TST_TEMP != NULL) {
        STDF_FORMAT("MIR:TST_TEMP:%s\n", m_MIR_TST_TEMP);
    }
    if (m_MIR_USER_TXT != NULL) {
        STDF_FORMAT("MIR:USER_TXT:%s\n", m_MIR_USER_TXT);
    }
    if (m_MIR_AUX_FILE != NULL) {
        STDF_FORMAT("MIR:AUX_FILE:%s\n", m_MIR_AUX_FILE);
    }
    if (m_MIR_PKG_TYP != NULL) {
        STDF_FORMAT("MIR:PKG_TYP:%s\n", m_MIR_PKG_TYP);
    }
    if (m_MIR_FAMLY_ID != NULL) {
        STDF_FORMAT("MIR:FAMLY_ID:%s\n", m_MIR_FAMLY_ID);
    }
    if (m_MIR_DATE_COD != NULL) {
        STDF_FORMAT("MIR:DATE_COD:%s\n", m_MIR_DATE_COD);
    }
    if (m_MIR_FACIL_ID != NULL) {
        STDF_FORMAT("MIR:FACIL_ID:%s\n", m_MIR_FACIL_ID);
    }
    if (m_MIR_FLOOR_ID != NULL) {
        STDF_FORMAT("MIR:FLOOR_ID:%s\n", m_MIR_FLOOR_ID);
    }
    if (m_MIR_PROC_ID != NULL) {
        STDF_FORMAT("MIR:PROC_ID:%s\n", m_MIR_PROC_ID);
    }
    if (m_MIR_OPER_FRQ != NULL) {
        STDF_FORMAT("MIR:OPER_FRQ:%s\n", m_MIR_OPER_FRQ);
    }
    if (m_MIR_SPEC_NAM != NULL) {
        STDF_FORMAT("MIR:SPEC_NAM:%s\n", m_MIR_SPEC_NAM);
    }
    if (m_MIR_SPEC_VER != NULL) {
        STDF_FORMAT("MIR:SPEC_VER:%s\n", m_MIR_SPEC_VER);
    }
    if (m_MIR_FLOW_ID != NULL) {
        STDF_FORMAT("MIR:FLOW_ID:%s\n", m_MIR_FLOW_ID);
    }
    if (m_MIR_SETUP_ID != NULL) {
        STDF_FORMAT("MIR:SETUP_ID:%s\n", m_MIR_SETUP_ID);
    }
    if (m_MIR_DSGN_REV != NULL) {
        STDF_FORMAT("MIR:DSGN_REV:%s\n", m_MIR_DSGN_REV);
    }
    if (m_MIR_ENG_ID != NULL) {
        STDF_FORMAT("MIR:ENG_ID:%s\n", m_MIR_ENG_ID);
    }
    if (m_MIR_ROM_COD != NULL) {
        STDF_FORMAT("MIR:ROM_COD:%s\n", m_MIR_ROM_COD);
    }
    if (m_MIR_SERL_NUM != NULL) {
        STDF_FORMAT("MIR:SERL_NUM:%s\n", m_MIR_SERL_NUM);
    }
    if (m_MIR_SUPR_NAM != NULL) {
        STDF_FORMAT("MIR:SUPR_NAM:%s\n", m_MIR_SUPR_NAM);
    }

    // Write MRR fields
    if (m_MRR_DISP_COD != 0) {
        STDF_FORMAT("MRR:DISP_COD:%c\n", m_MRR_DISP_COD);
    }
    if (m_MRR_USR_DESC != NULL) {
        STDF_FORMAT("MRR:USR_DESC:%s\n", m_MRR_USR_DESC);
    }
    if (m_MRR_EXC_DESC != NULL) {
        STDF_FORMAT("MRR:EXC_DESC:%s\n", m_MRR_EXC_DESC);
    }

    // Write PCR fields
    if (m_PCR_PART_CNT != 0L) {
        STDF_FORMAT("PCR:PART_CNT:%d\n", m_PCR_PART_CNT);
    }
    if (m_PCR_RTST_CNT != LONG_INVALID_DATA/*0L*/) {
        STDF_FORMAT("PCR:RTST_CNT:%u\n", m_PCR_RTST_CNT);
    }
    if (m_PCR_ABRT_CNT != LONG_INVALID_DATA/*0L*/) {
        STDF_FORMAT("PCR:ABRT_CNT:%u\n", m_PCR_ABRT_CNT);
    }
    if (m_PCR_GOOD_CNT != 0L) {
        STDF_FORMAT("PCR:GOOD_CNT:%d\n", m_PCR_GOOD_CNT);
    }
    if (m_PCR_FUNC_CNT != 0L) {
        STDF_FORMAT("PCR:FUNC_CNT:%d\n", m_PCR_FUNC_CNT);
    }

    // Write SDR fields
    if (m_SDR_HAND_TYP != NULL) {
        STDF_FORMAT("SDR:HAND_TYP:%s\n", m_SDR_HAND_TYP);
    }
    if (m_SDR_HAND_ID != NULL) {
        STDF_FORMAT("SDR:HAND_ID:%s\n", m_SDR_HAND_ID);
    }
    if (m_SDR_CARD_TYP != NULL) {
        STDF_FORMAT("SDR:CARD_TYP:%s\n", m_SDR_CARD_TYP);
    }
    if (m_SDR_CARD_ID != NULL) {
        STDF_FORMAT("SDR:CARD_ID:%s\n", m_SDR_CARD_ID);
    }
    if (m_SDR_LOAD_TYP != NULL) {
        STDF_FORMAT("SDR:LOAD_TYP:%s\n", m_SDR_LOAD_TYP);
    }
    if (m_SDR_LOAD_ID != NULL) {
        STDF_FORMAT("SDR:LOAD_ID:%s\n", m_SDR_LOAD_ID);
    }
    if (m_SDR_DIB_TYP != NULL) {
        STDF_FORMAT("SDR:DIB_TYP:%s\n", m_SDR_DIB_TYP);
    }
    if (m_SDR_DIB_ID != NULL) {
        STDF_FORMAT("SDR:DIB_ID:%s\n", m_SDR_DIB_ID);
    }
    if (m_SDR_CABL_TYP != NULL) {
        STDF_FORMAT("SDR:CABL_TYP:%s\n", m_SDR_CABL_TYP);
    }
    if (m_SDR_CABL_ID != NULL) {
        STDF_FORMAT("SDR:CABL_ID:%s\n", m_SDR_CABL_ID);
    }
    if (m_SDR_CONT_TYP != NULL) {
        STDF_FORMAT("SDR:CONT_TYP:%s\n", m_SDR_CONT_TYP);
    }
    if (m_SDR_CONT_ID != NULL) {
        STDF_FORMAT("SDR:CONT_ID:%s\n", m_SDR_CONT_ID);
    }
    if (m_SDR_LASR_TYP != NULL) {
        STDF_FORMAT("SDR:LASR_TYP:%s\n", m_SDR_LASR_TYP);
    }
    if (m_SDR_LASR_ID != NULL) {
        STDF_FORMAT("SDR:LASR_ID:%s\n", m_SDR_LASR_ID);
    }
    if (m_SDR_EXTR_TYP != NULL) {
        STDF_FORMAT("SDR:EXTR_TYP:%s\n", m_SDR_EXTR_TYP);
    }
    if (m_SDR_EXTR_ID != NULL) {
        STDF_FORMAT("SDR:EXTR_ID:%s\n", m_SDR_EXTR_ID);
    }

    // Write WIR fields
    if (m_WIR_WAFER_ID != NULL) {
        STDF_FORMAT("WIR:WAFER_ID:%s\n", m_WIR_WAFER_ID);
    }

    // Write WRR fields
    if (m_WRR_PART_CNT != 0L) {
        STDF_FORMAT("WRR:PART_CNT:%d\n", m_WRR_PART_CNT);
    }
    if (m_WRR_RTST_CNT != LONG_INVALID_DATA/*0L*/) {
        STDF_FORMAT("WRR:RTST_CNT:%u\n", m_WRR_RTST_CNT);
    }
    if (m_WRR_ABRT_CNT != LONG_INVALID_DATA/*0L*/) {
        STDF_FORMAT("WRR:ABRT_CNT:%u\n", m_WRR_ABRT_CNT);
    }
    if (m_WRR_GOOD_CNT != 0L) {
        STDF_FORMAT("WRR:GOOD_CNT:%d\n", m_WRR_GOOD_CNT);
    }
    if (m_WRR_FUNC_CNT != 0L) {
        STDF_FORMAT("WRR:FUNC_CNT:%d\n", m_WRR_FUNC_CNT);
    }
    if (m_WRR_WAFER_ID != NULL) {
        STDF_FORMAT("WRR:WAFER_ID:%s\n", m_WRR_WAFER_ID);
    }
    if (m_WRR_FABWF_ID != NULL) {
        STDF_FORMAT("WRR:FABWF_ID:%s\n", m_WRR_FABWF_ID);
    }
    if (m_WRR_FRAME_ID != NULL) {
        STDF_FORMAT("WRR:FRAME_ID:%s\n", m_WRR_FRAME_ID);
    }
    if (m_WRR_MASK_ID != NULL) {
        STDF_FORMAT("WRR:MASK_ID:%s\n", m_WRR_MASK_ID);
    }
    if (m_WRR_USR_DESC != NULL) {
        STDF_FORMAT("WRR:USR_DESC:%s\n", m_WRR_USR_DESC);
    }
    if (m_WRR_EXC_DESC != NULL) {
        STDF_FORMAT("WRR:EXC_DESC:%s\n", m_WRR_EXC_DESC);
    }

    // Write WCR fields
    if (m_WCR_WAFR_SIZ != 0.0) {
        STDF_FORMAT("WCR:WAFR_SIZ:%f\n",  m_WCR_WAFR_SIZ);
    }
    if (m_WCR_DIE_HT != 0.0) {
        STDF_FORMAT("WCR:DIE_HT:%f\n",  m_WCR_DIE_HT);
    }
    if (m_WCR_DIE_WID != 0.0) {
        STDF_FORMAT("WCR:DIE_WID:%f\n",  m_WCR_DIE_WID);
    }
    if (m_WCR_WF_UNITS != '\0') {
        STDF_FORMAT("WCR:WF_UNITS:%d\n",  m_WCR_WF_UNITS);
    }
    if (m_WCR_WF_FLAT != '\0') {
        STDF_FORMAT("WCR:WF_FLAT:%c\n", m_WCR_WF_FLAT);
    }
    if (m_WCR_CENTER_X != 0) {
        STDF_FORMAT("WCR:CENTER_X:%d\n", m_WCR_CENTER_X);
    }
    if (m_WCR_CENTER_Y != 0) {
        STDF_FORMAT("WCR:CENTER_Y:%d\n", m_WCR_CENTER_Y);
    }
    if (m_WCR_POS_X != '\0') {
        STDF_FORMAT("WCR:POS_X:%c\n", m_WCR_POS_X);
    }
    if (m_WCR_POS_Y != '\0') {
        STDF_FORMAT("WCR:POS_Y:%c\n", m_WCR_POS_Y);
    }

    // Write PTR fields
    if (m_PTR_C_RESFMT != NULL) {
        STDF_FORMAT("PTR:C_RESFMT:%s\n", m_PTR_C_RESFMT);
    }
    if (m_PTR_C_LLMFMT != NULL) {
        STDF_FORMAT("PTR:C_LLMFMT:%s\n", m_PTR_C_LLMFMT);
    }
    if (m_PTR_C_HLMFMT != NULL) {
        STDF_FORMAT("PTR:C_HLMFMT:%s\n", m_PTR_C_HLMFMT);
    }

    // Write EOR
    STDF_FORMAT("%s", "$\n");

    return(size);
}

gsint32
STDF_STRINGS::GetStorageSizeBytes (void)
{
    return(FormatStrings(NULL));
}

gsint32
STDF_STRINGS::LoadFromBuffer (const void *buffer)
{
    char	*bp = (char *)buffer, *ep, *record, *field, *value;
    gsint32	i = 0;

    while (*bp != '$') {
        record = bp;
        bp = strchr(bp, ':');
        *bp++ = '\0';
        field = bp;
        bp = strchr(bp, ':');
        *bp++ = '\0';
        value = bp;
        ep = strchr(bp, '\n');
        *ep++ ='\0';
        i += (gsint32)(ep - bp);
        bp = ep;

        if(strcmp(record, "ATR") == 0){
            if (strcmp(field, "CMD_LINE") == 0)
                Set_ATR_CMD_LINE(value);
        }

//CHANGES BEGIN 28 June 2006
        if(strcmp(record, "PRR") == 0)
        {
            if (strcmp(field, "PART_ID_OPTIONS") == 0)
            {
                Set_PRR_PART_ID_OPTIONS(atoi(value));
            }
        }
// CHANGES END 28 JUNE 2006

        if (strcmp(record, "MIR") == 0) {
            if (strcmp(field, "STAT_NUM") == 0) {
                Set_MIR_STAT_NUM(atoi(value));
            } else if (strcmp(field, "MODE_COD") == 0) {
                Set_MIR_MODE_COD(*value);
            } else if (strcmp(field, "RTST_COD") == 0) {
                Set_MIR_RTST_COD(*value);
            } else if (strcmp(field, "PROT_COD") == 0) {
                Set_MIR_PROT_COD(*value);
            } else if (strcmp(field, "BURN_TIM") == 0) {
                Set_MIR_BURN_TIM(atoi(value));
            } else if (strcmp(field, "CMOD_COD") == 0) {
                Set_MIR_CMOD_COD(*value);
            } else if (strcmp(field, "LOT_ID") == 0) {
                Set_MIR_LOT_ID(value);
            } else if (strcmp(field, "PART_TYP") == 0) {
                Set_MIR_PART_TYP(value);
            } else if (strcmp(field, "NODE_NAM") == 0) {
                Set_MIR_NODE_NAM(value);
            } else if (strcmp(field, "TSTR_TYP") == 0) {
                Set_MIR_TSTR_TYP(value);
            } else if (strcmp(field, "JOB_NAM") == 0) {
                Set_MIR_JOB_NAM(value);
            } else if (strcmp(field, "JOB_REV") == 0) {
                Set_MIR_JOB_REV(value);
            } else if (strcmp(field, "SBLOT_ID") == 0) {
                Set_MIR_SBLOT_ID(value);
            } else if (strcmp(field, "OPER_NAM") == 0) {
                Set_MIR_OPER_NAM(value);
            } else if (strcmp(field, "EXEC_TYP") == 0) {
                Set_MIR_EXEC_TYP(value);
            } else if (strcmp(field, "EXEC_VER") == 0) {
                Set_MIR_EXEC_VER(value);
            } else if (strcmp(field, "TEST_COD") == 0) {
                Set_MIR_TEST_COD(value);
            } else if (strcmp(field, "TST_TEMP") == 0) {
                Set_MIR_TST_TEMP(value);
            } else if (strcmp(field, "USER_TXT") == 0) {
                Set_MIR_USER_TXT(value);
            } else if (strcmp(field, "AUX_FILE") == 0) {
                Set_MIR_AUX_FILE(value);
            } else if (strcmp(field, "PKG_TYP") == 0) {
                Set_MIR_PKG_TYP(value);
            } else if (strcmp(field, "FAMLY_ID") == 0) {
                Set_MIR_FAMLY_ID(value);
            } else if (strcmp(field, "DATE_COD") == 0) {
                Set_MIR_DATE_COD(value);
            } else if (strcmp(field, "FACIL_ID") == 0) {
                Set_MIR_FACIL_ID(value);
            } else if (strcmp(field, "FLOOR_ID") == 0) {
                Set_MIR_FLOOR_ID(value);
            } else if (strcmp(field, "PROC_ID") == 0) {
                Set_MIR_PROC_ID(value);
            } else if (strcmp(field, "OPER_FRQ") == 0) {
                Set_MIR_OPER_FRQ(value);
            } else if (strcmp(field, "SPEC_NAM") == 0) {
                Set_MIR_SPEC_NAM(value);
            } else if (strcmp(field, "SPEC_VER") == 0) {
                Set_MIR_SPEC_VER(value);
            } else if (strcmp(field, "FLOW_ID") == 0) {
                Set_MIR_FLOW_ID(value);
            } else if (strcmp(field, "SETUP_ID") == 0) {
                Set_MIR_SETUP_ID(value);
            } else if (strcmp(field, "DSGN_REV") == 0) {
                Set_MIR_DSGN_REV(value);
            } else if (strcmp(field, "ENG_ID") == 0) {
                Set_MIR_ENG_ID(value);
            } else if (strcmp(field, "ROM_COD") == 0) {
                Set_MIR_ROM_COD(value);
            } else if (strcmp(field, "SERL_NUM") == 0) {
                Set_MIR_SERL_NUM(value);
            } else if (strcmp(field, "SUPR_NAM") == 0) {
                Set_MIR_SUPR_NAM(value);
            }
        } else if (strcmp(record, "MRR") == 0) {
            if (strcmp(field, "DISP_COD") == 0) {
                Set_MRR_DISP_COD(*value);
            } else if (strcmp(field, "USR_DESC") == 0) {
                Set_MRR_USR_DESC(value);
            } else if (strcmp(field, "EXC_DESC") == 0) {
                Set_MRR_EXC_DESC(value);
            }
        } else if (strcmp(record, "PCR") == 0) {
            if (strcmp(field, "PART_CNT") == 0) {
                Set_PCR_PART_CNT(atol(value));
            } else if (strcmp(field, "RTST_CNT") == 0) {
                Set_PCR_RTST_CNT(atol(value));
            } else if (strcmp(field, "ABRT_CNT") == 0) {
                Set_PCR_ABRT_CNT(atol(value));
            } else if (strcmp(field, "GOOD_CNT") == 0) {
                Set_PCR_GOOD_CNT(atol(value));
            } else if (strcmp(field, "FUNC_CNT") == 0) {
                Set_PCR_FUNC_CNT(atol(value));
            }
        } else if (strcmp(record, "SDR") == 0) {
            if (strcmp(field, "HAND_TYP") == 0) {
                Set_SDR_HAND_TYP(value);
            } else if (strcmp(field, "HAND_ID") == 0) {
                Set_SDR_HAND_ID(value);
            } else if (strcmp(field, "CARD_TYP") == 0) {
                Set_SDR_CARD_TYP(value);
            } else if (strcmp(field, "CARD_ID") == 0) {
                Set_SDR_CARD_ID(value);
            } else if (strcmp(field, "LOAD_TYP") == 0) {
                Set_SDR_LOAD_TYP(value);
            } else if (strcmp(field, "LOAD_ID") == 0) {
                Set_SDR_LOAD_ID(value);
            } else if (strcmp(field, "DIB_TYP") == 0) {
                Set_SDR_DIB_TYP(value);
            } else if (strcmp(field, "DIB_ID") == 0) {
                Set_SDR_DIB_ID(value);
            } else if (strcmp(field, "CABL_TYP") == 0) {
                Set_SDR_CABL_TYP(value);
            } else if (strcmp(field, "CABL_ID") == 0) {
                Set_SDR_CABL_ID(value);
            } else if (strcmp(field, "CONT_TYP") == 0) {
                Set_SDR_CONT_TYP(value);
            } else if (strcmp(field, "CONT_ID") == 0) {
                Set_SDR_CONT_ID(value);
            } else if (strcmp(field, "LASR_TYP") == 0) {
                Set_SDR_LASR_TYP(value);
            } else if (strcmp(field, "LASR_ID") == 0) {
                Set_SDR_LASR_ID(value);
            } else if (strcmp(field, "EXTR_TYP") == 0) {
                Set_SDR_EXTR_TYP(value);
            } else if (strcmp(field, "EXTR_ID") == 0) {
                Set_SDR_EXTR_ID(value);
            }
        } else if (strcmp(record, "WIR") == 0) {
            if (strcmp(field, "WAFER_ID") == 0) {
                Set_WIR_WAFER_ID(value);
            }
        } else if (strcmp(record, "WRR") == 0) {
            if (strcmp(field, "PART_CNT") == 0) {
                Set_WRR_PART_CNT(atol(value));
            } else if (strcmp(field, "RTST_CNT") == 0) {
                Set_WRR_RTST_CNT(atol(value));
            } else if (strcmp(field, "ABRT_CNT") == 0) {
                Set_WRR_ABRT_CNT(atol(value));
            } else if (strcmp(field, "GOOD_CNT") == 0) {
                Set_WRR_GOOD_CNT(atol(value));
            } else if (strcmp(field, "FUNC_CNT") == 0) {
                Set_WRR_FUNC_CNT(atol(value));
            } else if (strcmp(field, "WAFER_ID") == 0) {
                Set_WRR_WAFER_ID(value);
            } else if (strcmp(field, "FABWF_ID") == 0) {
                Set_WRR_FABWF_ID(value);
            } else if (strcmp(field, "FRAME_ID") == 0) {
                Set_WRR_FRAME_ID(value);
            } else if (strcmp(field, "MASK_ID") == 0) {
                Set_WRR_MASK_ID(value);
            } else if (strcmp(field, "USR_DESC") == 0) {
                Set_WRR_USR_DESC(value);
            } else if (strcmp(field, "EXC_DESC") == 0) {
                Set_WRR_EXC_DESC(value);
            }
        } else if (strcmp(record, "WCR") == 0) {
            if (strcmp(field, "WAFR_SIZ") == 0) {
                Set_WCR_WAFR_SIZ((float)atof(value));
            } else if (strcmp(field, "DIE_HT") == 0) {
                Set_WCR_DIE_HT((float)atof(value));
            } else if (strcmp(field, "DIE_WID") == 0) {
                Set_WCR_DIE_WID((float)atof(value));
            } else if (strcmp(field, "WF_UNITS") == 0) {
                Set_WCR_WF_UNITS(atoi(value));
            } else if (strcmp(field, "WF_FLAT") == 0) {
                Set_WCR_WF_FLAT(*value);
            } else if (strcmp(field, "CENTER_X") == 0) {
                Set_WCR_CENTER_X(atoi(value));
            } else if (strcmp(field, "CENTER_Y") == 0) {
                Set_WCR_CENTER_Y(atoi(value));
            } else if (strcmp(field, "POS_X") == 0) {
                Set_WCR_POS_X(*value);
            } else if (strcmp(field, "POS_Y") == 0) {
                Set_WCR_POS_Y(*value);
            }
        } else if (strcmp(record, "PTR") == 0) {
            if (strcmp(field, "C_RESFMT") == 0) {
                Set_PTR_C_RESFMT(value);
            } else if (strcmp(field, "C_LLMFMT") == 0) {
                Set_PTR_C_LLMFMT(value);
            } else if (strcmp(field, "C_HLMFMT") == 0) {
                Set_PTR_C_HLMFMT(value);
            }
        }
    }
    return(i);
}

// ATR Set Functions
void STDF_STRINGS::Set_ATR_CMD_LINE(char *value) { STDF_CSET(m_ATR_CMD_LINE, value); }

// ATR Get Functions
char *STDF_STRINGS::Get_ATR_CMD_LINE(void) { return(m_ATR_CMD_LINE); }

//CHANGES BEGIN 28 June 2006
// PRR Get/Set Functions
void STDF_STRINGS::Set_PRR_PART_ID_OPTIONS(int val) { m_PRR_PART_ID_OPTIONS = val; }
int STDF_STRINGS::Get_PRR_PART_ID_OPTIONS(void) { return(m_PRR_PART_ID_OPTIONS); }
// CHANGES END 28 JUNE 2006

// MIR Get Functions
unsigned char STDF_STRINGS::Get_MIR_STAT_NUM(void) { return(m_MIR_STAT_NUM); }
char STDF_STRINGS::Get_MIR_MODE_COD(void) { return(m_MIR_MODE_COD); }
char STDF_STRINGS::Get_MIR_RTST_COD(void) { return(m_MIR_RTST_COD); }
char STDF_STRINGS::Get_MIR_PROT_COD(void) { return(m_MIR_PROT_COD); }
unsigned short STDF_STRINGS::Get_MIR_BURN_TIM(void) { return(m_MIR_BURN_TIM); }
char STDF_STRINGS::Get_MIR_CMOD_COD(void) { return(m_MIR_CMOD_COD); }
char *STDF_STRINGS::Get_MIR_LOT_ID(void) { return(m_MIR_LOT_ID); }
char *STDF_STRINGS::Get_MIR_PART_TYP(void) { return(m_MIR_PART_TYP); }
char *STDF_STRINGS::Get_MIR_NODE_NAM(void) { return(m_MIR_NODE_NAM); }
char *STDF_STRINGS::Get_MIR_TSTR_TYP(void) { return(m_MIR_TSTR_TYP); }
char *STDF_STRINGS::Get_MIR_JOB_NAM(void) { return(m_MIR_JOB_NAM); }
char *STDF_STRINGS::Get_MIR_JOB_REV(void) { return(m_MIR_JOB_REV); }
char *STDF_STRINGS::Get_MIR_SBLOT_ID(void) { return(m_MIR_SBLOT_ID); }
char *STDF_STRINGS::Get_MIR_OPER_NAM(void) { return(m_MIR_OPER_NAM); }
char *STDF_STRINGS::Get_MIR_EXEC_TYP(void) { return(m_MIR_EXEC_TYP); }
char *STDF_STRINGS::Get_MIR_EXEC_VER(void) { return(m_MIR_EXEC_VER); }
char *STDF_STRINGS::Get_MIR_TEST_COD(void) { return(m_MIR_TEST_COD); }
char *STDF_STRINGS::Get_MIR_TST_TEMP(void) { return(m_MIR_TST_TEMP); }
char *STDF_STRINGS::Get_MIR_USER_TXT(void) { return(m_MIR_USER_TXT); }
char *STDF_STRINGS::Get_MIR_AUX_FILE(void) { return(m_MIR_AUX_FILE); }
char *STDF_STRINGS::Get_MIR_PKG_TYP(void) { return(m_MIR_PKG_TYP); }
char *STDF_STRINGS::Get_MIR_FAMLY_ID(void) { return(m_MIR_FAMLY_ID); }
char *STDF_STRINGS::Get_MIR_DATE_COD(void) { return(m_MIR_DATE_COD); }
char *STDF_STRINGS::Get_MIR_FACIL_ID(void) { return(m_MIR_FACIL_ID); }
char *STDF_STRINGS::Get_MIR_FLOOR_ID(void) { return(m_MIR_FLOOR_ID); }
char *STDF_STRINGS::Get_MIR_PROC_ID(void) { return(m_MIR_PROC_ID); }
char *STDF_STRINGS::Get_MIR_OPER_FRQ(void) { return(m_MIR_OPER_FRQ); }
char *STDF_STRINGS::Get_MIR_SPEC_NAM(void) { return(m_MIR_SPEC_NAM); }
char *STDF_STRINGS::Get_MIR_SPEC_VER(void) { return(m_MIR_SPEC_VER); }
char *STDF_STRINGS::Get_MIR_FLOW_ID(void) { return(m_MIR_FLOW_ID); }
char *STDF_STRINGS::Get_MIR_SETUP_ID(void) { return(m_MIR_SETUP_ID); }
char *STDF_STRINGS::Get_MIR_DSGN_REV(void) { return(m_MIR_DSGN_REV); }
char *STDF_STRINGS::Get_MIR_ENG_ID(void) { return(m_MIR_ENG_ID); }
char *STDF_STRINGS::Get_MIR_ROM_COD(void) { return(m_MIR_ROM_COD); }
char *STDF_STRINGS::Get_MIR_SERL_NUM(void) { return(m_MIR_SERL_NUM); }
char *STDF_STRINGS::Get_MIR_SUPR_NAM(void) { return(m_MIR_SUPR_NAM); }

// MIR Set Functions
void STDF_STRINGS::Set_MIR_STAT_NUM(unsigned char value) { m_MIR_STAT_NUM = value; }
void STDF_STRINGS::Set_MIR_MODE_COD(char value) { m_MIR_MODE_COD = value; }
void STDF_STRINGS::Set_MIR_RTST_COD(char value) { m_MIR_RTST_COD = value; }
void STDF_STRINGS::Set_MIR_PROT_COD(char value) { m_MIR_PROT_COD = value; }
void STDF_STRINGS::Set_MIR_BURN_TIM(unsigned short value) { m_MIR_BURN_TIM = value; }
void STDF_STRINGS::Set_MIR_CMOD_COD(char value) { m_MIR_CMOD_COD = value; }
void STDF_STRINGS::Set_MIR_LOT_ID(char *value) { STDF_CSET(m_MIR_LOT_ID, value); }
void STDF_STRINGS::Set_MIR_PART_TYP(char *value) { STDF_CSET(m_MIR_PART_TYP, value); }
void STDF_STRINGS::Set_MIR_NODE_NAM(char *value) { STDF_CSET(m_MIR_NODE_NAM, value); }
void STDF_STRINGS::Set_MIR_TSTR_TYP(char *value) { STDF_CSET(m_MIR_TSTR_TYP, value); }
void STDF_STRINGS::Set_MIR_JOB_NAM(char *value) { STDF_CSET(m_MIR_JOB_NAM, value); }
void STDF_STRINGS::Set_MIR_JOB_REV(char *value) { STDF_CSET(m_MIR_JOB_REV, value); }
void STDF_STRINGS::Set_MIR_SBLOT_ID(char *value) { STDF_CSET(m_MIR_SBLOT_ID, value); }
void STDF_STRINGS::Set_MIR_OPER_NAM(char *value) { STDF_CSET(m_MIR_OPER_NAM, value); }
void STDF_STRINGS::Set_MIR_EXEC_TYP(char *value) { STDF_CSET(m_MIR_EXEC_TYP, value); }
void STDF_STRINGS::Set_MIR_EXEC_VER(char *value) { STDF_CSET(m_MIR_EXEC_VER, value); }
void STDF_STRINGS::Set_MIR_TEST_COD(char *value) { STDF_CSET(m_MIR_TEST_COD, value); }
void STDF_STRINGS::Set_MIR_TST_TEMP(char *value) { STDF_CSET(m_MIR_TST_TEMP, value); }
void STDF_STRINGS::Set_MIR_USER_TXT(char *value) { STDF_CSET(m_MIR_USER_TXT, value); }
void STDF_STRINGS::Set_MIR_AUX_FILE(char *value) { STDF_CSET(m_MIR_AUX_FILE, value); }
void STDF_STRINGS::Set_MIR_PKG_TYP(char *value) { STDF_CSET(m_MIR_PKG_TYP, value); }
void STDF_STRINGS::Set_MIR_FAMLY_ID(char *value) { STDF_CSET(m_MIR_FAMLY_ID, value); }
void STDF_STRINGS::Set_MIR_DATE_COD(char *value) { STDF_CSET(m_MIR_DATE_COD, value); }
void STDF_STRINGS::Set_MIR_FACIL_ID(char *value) { STDF_CSET(m_MIR_FACIL_ID, value); }
void STDF_STRINGS::Set_MIR_FLOOR_ID(char *value) { STDF_CSET(m_MIR_FLOOR_ID, value); }
void STDF_STRINGS::Set_MIR_PROC_ID(char *value) { STDF_CSET(m_MIR_PROC_ID, value); }
void STDF_STRINGS::Set_MIR_OPER_FRQ(char *value) { STDF_CSET(m_MIR_OPER_FRQ, value); }
void STDF_STRINGS::Set_MIR_SPEC_NAM(char *value) { STDF_CSET(m_MIR_SPEC_NAM, value); }
void STDF_STRINGS::Set_MIR_SPEC_VER(char *value) { STDF_CSET(m_MIR_SPEC_VER, value); }
void STDF_STRINGS::Set_MIR_FLOW_ID(char *value) { STDF_CSET(m_MIR_FLOW_ID, value); }
void STDF_STRINGS::Set_MIR_SETUP_ID(char *value) { STDF_CSET(m_MIR_SETUP_ID, value); }
void STDF_STRINGS::Set_MIR_DSGN_REV(char *value) { STDF_CSET(m_MIR_DSGN_REV, value); }
void STDF_STRINGS::Set_MIR_ENG_ID(char *value) { STDF_CSET(m_MIR_ENG_ID, value); }
void STDF_STRINGS::Set_MIR_ROM_COD(char *value) { STDF_CSET(m_MIR_ROM_COD, value); }
void STDF_STRINGS::Set_MIR_SERL_NUM(char *value) { STDF_CSET(m_MIR_SERL_NUM, value); }
void STDF_STRINGS::Set_MIR_SUPR_NAM(char *value) { STDF_CSET(m_MIR_SUPR_NAM, value); }

// MRR Get Functions
char STDF_STRINGS::Get_MRR_DISP_COD(void) { return m_MRR_DISP_COD; }
char *STDF_STRINGS::Get_MRR_USR_DESC(void) { return m_MRR_USR_DESC; }
char *STDF_STRINGS::Get_MRR_EXC_DESC(void) { return m_MRR_EXC_DESC;  }

// MRR Set Functions
void STDF_STRINGS::Set_MRR_DISP_COD(char value) { m_MRR_DISP_COD = value; }
void STDF_STRINGS::Set_MRR_USR_DESC(char *value) { STDF_CSET(m_MRR_USR_DESC, value); }
void STDF_STRINGS::Set_MRR_EXC_DESC(char *value) { STDF_CSET(m_MRR_EXC_DESC, value); }

// PCR Get Functions
gsuint32 STDF_STRINGS::Get_PCR_PART_CNT(void) { return m_PCR_PART_CNT; }
gsuint32 STDF_STRINGS::Get_PCR_RTST_CNT(void) { return m_PCR_RTST_CNT; }
gsuint32 STDF_STRINGS::Get_PCR_ABRT_CNT(void) { return m_PCR_ABRT_CNT; }
gsuint32 STDF_STRINGS::Get_PCR_GOOD_CNT(void) { return m_PCR_GOOD_CNT; }
gsuint32 STDF_STRINGS::Get_PCR_FUNC_CNT(void) { return m_PCR_FUNC_CNT; }

// PCR Set Functions
void STDF_STRINGS::Set_PCR_PART_CNT(gsuint32 value) { m_PCR_PART_CNT = value; }
void STDF_STRINGS::Set_PCR_RTST_CNT(gsuint32 value) { m_PCR_RTST_CNT = value; }
void STDF_STRINGS::Set_PCR_ABRT_CNT(gsuint32 value) { m_PCR_ABRT_CNT = value; }
void STDF_STRINGS::Set_PCR_GOOD_CNT(gsuint32 value) { m_PCR_GOOD_CNT = value; }
void STDF_STRINGS::Set_PCR_FUNC_CNT(gsuint32 value) { m_PCR_FUNC_CNT = value; }

// SDR Get Functions
char *STDF_STRINGS::Get_SDR_HAND_TYP(void) { return m_SDR_HAND_TYP; }
char *STDF_STRINGS::Get_SDR_HAND_ID(void) { return m_SDR_HAND_ID ; }
char *STDF_STRINGS::Get_SDR_CARD_TYP(void) { return m_SDR_CARD_TYP; }
char *STDF_STRINGS::Get_SDR_CARD_ID(void) { return m_SDR_CARD_ID; }
char *STDF_STRINGS::Get_SDR_LOAD_TYP(void) { return m_SDR_LOAD_TYP; }
char *STDF_STRINGS::Get_SDR_LOAD_ID(void) { return m_SDR_LOAD_ID; }
char *STDF_STRINGS::Get_SDR_DIB_TYP(void) { return m_SDR_DIB_TYP; }
char *STDF_STRINGS::Get_SDR_DIB_ID(void) { return m_SDR_DIB_ID; }
char *STDF_STRINGS::Get_SDR_CABL_TYP(void) { return m_SDR_CABL_TYP; }
char *STDF_STRINGS::Get_SDR_CABL_ID(void) { return m_SDR_CABL_ID; }
char *STDF_STRINGS::Get_SDR_CONT_TYP(void) { return m_SDR_CONT_TYP; }
char *STDF_STRINGS::Get_SDR_CONT_ID(void) { return m_SDR_CONT_ID ; }
char *STDF_STRINGS::Get_SDR_LASR_TYP(void) { return m_SDR_LASR_TYP; }
char *STDF_STRINGS::Get_SDR_LASR_ID(void) { return m_SDR_LASR_ID; }
char *STDF_STRINGS::Get_SDR_EXTR_TYP(void) { return m_SDR_EXTR_TYP; }
char *STDF_STRINGS::Get_SDR_EXTR_ID(void) { return m_SDR_EXTR_ID; }

// SDR Set Functions
void STDF_STRINGS::Set_SDR_HAND_TYP(char *value) { STDF_CSET(m_SDR_HAND_TYP, value); }
void STDF_STRINGS::Set_SDR_HAND_ID(char *value) { STDF_CSET(m_SDR_HAND_ID , value); }
void STDF_STRINGS::Set_SDR_CARD_TYP(char *value) { STDF_CSET(m_SDR_CARD_TYP, value); }
void STDF_STRINGS::Set_SDR_CARD_ID(char *value) { STDF_CSET(m_SDR_CARD_ID, value); }
void STDF_STRINGS::Set_SDR_LOAD_TYP(char *value) { STDF_CSET(m_SDR_LOAD_TYP, value); }
void STDF_STRINGS::Set_SDR_LOAD_ID(char *value) { STDF_CSET(m_SDR_LOAD_ID, value); }
void STDF_STRINGS::Set_SDR_DIB_TYP(char *value) { STDF_CSET(m_SDR_DIB_TYP, value); }
void STDF_STRINGS::Set_SDR_DIB_ID(char *value) { STDF_CSET(m_SDR_DIB_ID, value); }
void STDF_STRINGS::Set_SDR_CABL_TYP(char *value) { STDF_CSET(m_SDR_CABL_TYP, value); }
void STDF_STRINGS::Set_SDR_CABL_ID(char *value) { STDF_CSET(m_SDR_CABL_ID, value); }
void STDF_STRINGS::Set_SDR_CONT_TYP(char *value) { STDF_CSET(m_SDR_CONT_TYP, value); }
void STDF_STRINGS::Set_SDR_CONT_ID(char *value) { STDF_CSET(m_SDR_CONT_ID , value); }
void STDF_STRINGS::Set_SDR_LASR_TYP(char *value) { STDF_CSET(m_SDR_LASR_TYP, value); }
void STDF_STRINGS::Set_SDR_LASR_ID(char *value) { STDF_CSET(m_SDR_LASR_ID, value); }
void STDF_STRINGS::Set_SDR_EXTR_TYP(char *value) { STDF_CSET(m_SDR_EXTR_TYP, value); }
void STDF_STRINGS::Set_SDR_EXTR_ID(char *value) { STDF_CSET(m_SDR_EXTR_ID, value); }

// WIR Get Functions
char *STDF_STRINGS::Get_WIR_WAFER_ID(void) { return m_WIR_WAFER_ID; }

// WIR Set Functions
void STDF_STRINGS::Set_WIR_WAFER_ID(char *value) { STDF_CSET(m_WIR_WAFER_ID, value); }

// WRR Get Functions
gsuint32 STDF_STRINGS::Get_WRR_PART_CNT(void) { return m_WRR_PART_CNT; }
gsuint32 STDF_STRINGS::Get_WRR_RTST_CNT(void) { return m_WRR_RTST_CNT; }
gsuint32 STDF_STRINGS::Get_WRR_ABRT_CNT(void) { return m_WRR_ABRT_CNT; }
gsuint32 STDF_STRINGS::Get_WRR_GOOD_CNT(void) { return m_WRR_GOOD_CNT; }
gsuint32 STDF_STRINGS::Get_WRR_FUNC_CNT(void) { return m_WRR_FUNC_CNT; }
char *STDF_STRINGS::Get_WRR_WAFER_ID(void) { return m_WRR_WAFER_ID; }
char *STDF_STRINGS::Get_WRR_FABWF_ID(void) { return m_WRR_FABWF_ID; }
char *STDF_STRINGS::Get_WRR_FRAME_ID(void) { return m_WRR_FRAME_ID; }
char *STDF_STRINGS::Get_WRR_MASK_ID(void) { return m_WRR_MASK_ID; }
char *STDF_STRINGS::Get_WRR_USR_DESC(void) { return m_WRR_USR_DESC; }
char *STDF_STRINGS::Get_WRR_EXC_DESC(void) { return m_WRR_EXC_DESC; }

// WRR Set Functions
void STDF_STRINGS::Set_WRR_PART_CNT(gsuint32 value) { m_WRR_PART_CNT = value; }
void STDF_STRINGS::Set_WRR_RTST_CNT(gsuint32 value) { m_WRR_RTST_CNT = value; }
void STDF_STRINGS::Set_WRR_ABRT_CNT(gsuint32 value) { m_WRR_ABRT_CNT = value; }
void STDF_STRINGS::Set_WRR_GOOD_CNT(gsuint32 value) { m_WRR_GOOD_CNT = value; }
void STDF_STRINGS::Set_WRR_FUNC_CNT(gsuint32 value) { m_WRR_FUNC_CNT = value; }
void STDF_STRINGS::Set_WRR_WAFER_ID(char *value) { STDF_CSET(m_WRR_WAFER_ID, value) ; }
void STDF_STRINGS::Set_WRR_FABWF_ID(char *value) { STDF_CSET(m_WRR_FABWF_ID, value) ; }
void STDF_STRINGS::Set_WRR_FRAME_ID(char *value) { STDF_CSET(m_WRR_FRAME_ID, value) ; }
void STDF_STRINGS::Set_WRR_MASK_ID(char *value) { STDF_CSET(m_WRR_MASK_ID, value) ; }
void STDF_STRINGS::Set_WRR_USR_DESC(char *value) { STDF_CSET(m_WRR_USR_DESC, value) ; }
void STDF_STRINGS::Set_WRR_EXC_DESC(char *value) { STDF_CSET(m_WRR_EXC_DESC, value) ; }

// WCR Get Functions
float STDF_STRINGS::Get_WCR_WAFR_SIZ(void) { return m_WCR_WAFR_SIZ; }
float STDF_STRINGS::Get_WCR_DIE_HT(void) { return m_WCR_DIE_HT; }
float STDF_STRINGS::Get_WCR_DIE_WID(void) { return m_WCR_DIE_WID; }
unsigned char STDF_STRINGS::Get_WCR_WF_UNITS(void) { return m_WCR_WF_UNITS; }
char STDF_STRINGS::Get_WCR_WF_FLAT(void) { return m_WCR_WF_FLAT; }
short STDF_STRINGS::Get_WCR_CENTER_X(void) { return m_WCR_CENTER_X; }
short STDF_STRINGS::Get_WCR_CENTER_Y(void) { return m_WCR_CENTER_Y; }
char STDF_STRINGS::Get_WCR_POS_X(void) { return m_WCR_POS_X; }
char STDF_STRINGS::Get_WCR_POS_Y(void) { return m_WCR_POS_Y; }

// WCR Set Functions
void STDF_STRINGS::Set_WCR_WAFR_SIZ(float value) { m_WCR_WAFR_SIZ = value; }
void STDF_STRINGS::Set_WCR_DIE_HT(float value) { m_WCR_DIE_HT = value; }
void STDF_STRINGS::Set_WCR_DIE_WID(float value) { m_WCR_DIE_WID = value; }
void STDF_STRINGS::Set_WCR_WF_UNITS(unsigned char value) { m_WCR_WF_UNITS = value; }
void STDF_STRINGS::Set_WCR_WF_FLAT(char value) { m_WCR_WF_FLAT = value; }
void STDF_STRINGS::Set_WCR_CENTER_X(short value) { m_WCR_CENTER_X = value; }
void STDF_STRINGS::Set_WCR_CENTER_Y(short value) { m_WCR_CENTER_Y = value; }
void STDF_STRINGS::Set_WCR_POS_X(char value) { m_WCR_POS_X = value; }
void STDF_STRINGS::Set_WCR_POS_Y(char value) { m_WCR_POS_Y = value; }

// PTR Get Functions
char *STDF_STRINGS::Get_PTR_C_RESFMT(void) { return m_PTR_C_RESFMT; }
char *STDF_STRINGS::Get_PTR_C_LLMFMT(void) { return m_PTR_C_LLMFMT; }
char *STDF_STRINGS::Get_PTR_C_HLMFMT(void) { return m_PTR_C_HLMFMT; }

// PTR Set Functions
void STDF_STRINGS::Set_PTR_C_RESFMT(char *value) { STDF_CSET(m_PTR_C_RESFMT, value); }
void STDF_STRINGS::Set_PTR_C_LLMFMT(char *value) { STDF_CSET(m_PTR_C_LLMFMT, value); }
void STDF_STRINGS::Set_PTR_C_HLMFMT(char *value) { STDF_CSET(m_PTR_C_HLMFMT, value); }

///////////////////////////////////////// STDF_STRINGS /////////////////////////////////////////

///////////////////////////////////////// STDF_SUPPLEMENT //////////////////////////////////////
// NOT SAME AS ORIGINAL !!!
////////////////////////////////////////////////////////////////////////////////////////////////

STDF_SUPPLEMENT::STDF_SUPPLEMENT (DLOG_DB_HDR *hdr)
{
    m_flags = 0;
    m_Nsites = 0;
    m_Nsubtests = hdr->GetPrgInfo()->PrgInfo()->GetCumulativeSubTestsN();
    memset(&m_reserved_data[0], 0, sizeof(m_reserved_data));

    m_includes_tsr_counts = 1;
    memset(&m_tsr_exec_cnts[0], 0, sizeof(m_tsr_exec_cnts));
    memset(&m_tsr_fail_cnts[0], 0, sizeof(m_tsr_fail_cnts));
    for (int i = 0; i < m_Nsites; ++i) {

    }
}

STDF_SUPPLEMENT::~STDF_SUPPLEMENT (void)
{
    for (int i = 0; i < m_Nsites; ++i) {
        free(m_tsr_exec_cnts[i]);
        m_tsr_exec_cnts[i] = NULL;
        free(m_tsr_fail_cnts[i]);
        m_tsr_fail_cnts[i] = NULL;
    }
}

gsint32
STDF_SUPPLEMENT::GetStorageSizeBytes(void)
{
    gsint32 i = 0;

    i += sizeof(m_flags);
    i += sizeof(m_Nsites);
    i += sizeof(m_Nsubtests);

    i += sizeof(m_reserved_data);

    if (m_includes_tsr_counts) {
        i += 2 * m_Nsites * m_Nsubtests * sizeof(gsuint32);
    }

    return i;
}

gsint32
STDF_SUPPLEMENT::LoadFromBuffer(const void *v)
{
    gsint32 i = 0;

    const BYTE *b = (const BYTE *)v;

    i += ReadDword(b+i,(gsint32*)&m_flags);
    i += ReadDword(b+i,(gsint32*)&m_Nsites);
    i += ReadDword(b+i,(gsint32*)&m_Nsubtests);

    MemMove (&m_reserved_data, b+i, sizeof(m_reserved_data)); // ReadBuffer
    i += sizeof(m_reserved_data);

    if (m_includes_tsr_counts) {
        int size = m_Nsubtests * sizeof(gsuint32);
        for (int j = 0; j < m_Nsites; ++j) {
            if (m_tsr_exec_cnts[j] != NULL) {
                free(m_tsr_exec_cnts[j]);
            }
            m_tsr_exec_cnts[j] = (gsuint32 *)malloc(size);
            if (!m_tsr_exec_cnts[j])
                return -1;

            for(int ij=0; ij < m_Nsubtests; ij++){
                i += ReadDword(b+i,(gsint32*)&m_tsr_exec_cnts[j][ij]);
            }
            if (m_tsr_fail_cnts[j] != NULL) {
                free(m_tsr_fail_cnts[j]);
            }
            m_tsr_fail_cnts[j] = (gsuint32 *)malloc(size);
            if (!m_tsr_fail_cnts[j])
                return -1;

            for(int ik=0; ik < m_Nsubtests; ik++){
                i += ReadDword(b+i,(gsint32*)&m_tsr_fail_cnts[j][ik]);
            }
        }
    }

    return i;
}
///////////////////////////////////////// STDF_SUPPLEMENT /////////////////////////////////////////



/////////////////////////////////// GLOBALS ///////////////////////////////////

/////////////////////////////////// GLOBALS ///////////////////////////////////

/////////////////////////////////// OTHER ///////////////////////////////////

BOOL IsDebugUnassignedMemory (const BYTE *b, gsint32 count);
DLOG_DB_FLD::DLOG_DB_FLD (gsint32 prg_info_idx /* = 0 */)
{
    m_prg_info_idx = prg_info_idx;
}

DLOG_DB_FLD::~DLOG_DB_FLD (void)
{
}

gsint32 DLOG_DB_FLD::GetProgramInfoIdx (void)
{
    return m_prg_info_idx;
}

gsint32 DLOG_DB_FLD::LoadFromBuffer (const void *v)
{
    return ReadDword((BYTE*)v, &m_prg_info_idx);
}

gsint32 DLOG_DB_FLD::GetStorageSizeBytes (void)
{
    return sizeof (m_prg_info_idx);
}


//---------------------------------------



BOOL DLOG_DB_VLD::ContainsData (void)
{
    if (m_dlog_entry_array) return true;
    return false;
}
gsint32 DLOG_DB_VLD::GetStorageSizeBytes (void)
{
    gsint32 len = 0;

    len += m_test_header.GetStorageSizeBytes ();

    gsint32 subtests_total_n = m_pfd->GetCumulativeSubTestsN ();
    len += 9/*sizeof (dlog_entry)*/ * subtests_total_n;

    for (gsint32 i=0; i<subtests_total_n; i++)
    {
        len += m_dlog_entry_var_len_data[i].GetStorageSizeBytes ();
    }

    len += sizeof (m_status01);

    if (m_ex_status_exists)
    {
        len += sizeof (m_ex_status);

        if (m_results_type_array_exists)
        {
            len += m_results_type_array->GetStorageSizeBytes ();
        }
    }

    return len;
}

DATALOG_VAR_LEN_DATA_PER_SUBTEST *DLOG_DB_VLD::Get_DATALOG_VAR_LEN_DATA_PER_SUBTEST (gsint32 subtest_i)
{
    if (subtest_i < 0 || subtest_i >= m_pfd->GetCumulativeSubTestsN ())
        return NULL;
    return &m_dlog_entry_var_len_data[subtest_i];
}

DLOG_DB_VLD::DLOG_DB_VLD (PROGRAM_FUNCTIONS_DATA *pfd)
{
    if(!pfd)
        return;
    m_pfd = pfd;

    gsint32 subtests_total_n = pfd->GetCumulativeSubTestsN ();

    m_dlog_entry_array = new dlog_entry[subtests_total_n];
    ASSERT_LOCATION(m_dlog_entry_array);
    m_dlog_entry_array_n = subtests_total_n;

    m_dlog_entry_var_len_data = new DATALOG_VAR_LEN_DATA_PER_SUBTEST[subtests_total_n];
    ASSERT_LOCATION(m_dlog_entry_var_len_data);
    m_status01 = 0; // GPZ::UMR
    m_site_i = 0;
    m_results_type_array = NULL;
    m_ex_status_exists = false;
    m_ex_status = 0;  // GPZ::UMR
    m_results_type_array_exists = false;

}

DLOG_DB_VLD::~DLOG_DB_VLD (void)
{

    delete [] m_dlog_entry_array;
    m_dlog_entry_array = 0;

    delete [] m_dlog_entry_var_len_data;
    m_dlog_entry_var_len_data = 0;
    delete m_results_type_array;
}

BYTE DLOG_DB_VLD::GetSiteI (void)
{
    return m_site_i;
}

const dlog_entry *DLOG_DB_VLD::Get_dlog_entry_array (void)
{
    return m_dlog_entry_array;
}

const test_header &DLOG_DB_VLD::Get_test_header (void)
{
    return m_test_header.m_nt_test_header;
}

gsint32 DLOG_DB_VLD::LoadFromBuffer (const void *v)
{
    ASSERT_LOCATION(v);
    const BYTE *b = (const BYTE *) v;
    gsint32 i = 0;

    i += m_test_header.LoadFromBuffer (b+i);

    gsint32 subtests_total_n = m_pfd->GetCumulativeSubTestsN ();

    BOOL alloc_rqd = false;
    if (m_dlog_entry_array_n != subtests_total_n)
    {
        if (m_dlog_entry_array)
            delete m_dlog_entry_array;

        m_dlog_entry_array = new dlog_entry[subtests_total_n];
        ASSERT_LOCATION(m_dlog_entry_array);
        if (!m_dlog_entry_array)
            return 0;

        alloc_rqd = true;

        m_dlog_entry_array_n = subtests_total_n;
    }

    int iIndex;
    for(iIndex=0; iIndex < subtests_total_n; iIndex++)
    {
        i += ReadByte(b+i, (BYTE*)&m_dlog_entry_array[iIndex].valid_value);
        i += ReadByte(b+i, (BYTE*)&m_dlog_entry_array[iIndex].passed_fail);
        i += ReadFloat(b+i, (float*)&m_dlog_entry_array[iIndex].measured_value);
        i += ReadByte(b+i, (BYTE*)&m_dlog_entry_array[iIndex].power);
        i += ReadByte(b+i, (BYTE*)&m_dlog_entry_array[iIndex].prescript_length);
        i += ReadByte(b+i, (BYTE*)&m_dlog_entry_array[iIndex].postscript_length);
    }


    if (alloc_rqd)
    {
        if (m_dlog_entry_var_len_data)
            delete [] m_dlog_entry_var_len_data;

        m_dlog_entry_var_len_data = new DATALOG_VAR_LEN_DATA_PER_SUBTEST[m_dlog_entry_array_n];
        ASSERT_LOCATION(m_dlog_entry_var_len_data);
        if (!m_dlog_entry_var_len_data)
            return 0;
    }

    for (gsint32 j=0; j<subtests_total_n; j++)
    {
        i += m_dlog_entry_var_len_data[j].LoadFromBuffer (b+i);
    }

    i += ReadByte(b+i, &m_status01);

    if (m_ex_status_exists)
    {
        i += ReadDword(b+i, (gsint32*)&m_ex_status);

        if (m_results_type_array_exists)
        {
            if (m_results_type_array)
                delete m_results_type_array;
            m_results_type_array = new PtrSet <ResultsTypeID, short>;
            ASSERT_LOCATION(m_results_type_array);
            if (!m_results_type_array)
                return 0;
            m_results_type_array->SetStatusDeleteObjects (true);
            i += m_results_type_array->LoadFromBuffer (b+i);
        }
    }
    return i;
}

//--------------------------------------------------------------------


gsint32 DATALOG_VAR_LEN_DATA_PER_SUBTEST::GetStorageSizeBytes (void)
{
    gsint32 size = 1/*sizeof (m_included_data)*/;
    if (m_included_data.pretest_comment)
    {
        size += sizeof (m_pretest_comment_len);
        size += m_pretest_comment_len;
    }

    if (m_test_limits)
    {
        size += m_test_limits->GetStorageSizeBytes ();
    }

    if (m_included_data.posttest_comment)
    {
        size += sizeof (m_posttest_comment_len);
        size += m_posttest_comment_len;
    }

    if (m_included_data.datalog_note)
    {
        size += sizeof (m_datalog_note_len);
        size += m_datalog_note_len;
    }

// CHANGES BEGIN 28 JUNE 2006
    if (m_plot_data)
    {
        size += m_plot_data->GetStorageSizeBytes ();
    }
// CHANGES END 28 JUNE 2006

    return size;
}
DATALOG_VAR_LEN_DATA_PER_SUBTEST::DATALOG_VAR_LEN_DATA_PER_SUBTEST (void)
{
    MemSet (&m_included_data,0,sizeof(m_included_data));
    m_pretest_comment_len = 0;
    m_pretest_comment = 0;

    m_posttest_comment_len = 0;
    m_posttest_comment = 0;

    m_datalog_note_len = 0;
    m_datalog_note = 0;

    m_test_limits = 0;

// CHANGES BEGIN 28 JUNE 2006
    m_plot_data = 0;
// CHANGES END 28 JUNE 2006
}

DATALOG_VAR_LEN_DATA_PER_SUBTEST::~DATALOG_VAR_LEN_DATA_PER_SUBTEST (void)
{
    if (m_included_data.pretest_comment)
    {
        delete [] m_pretest_comment;
        m_pretest_comment = 0;
        m_pretest_comment_len = 0;
    }
    if (m_included_data.posttest_comment)
    {
        delete [] m_posttest_comment;
        m_posttest_comment = 0;
        m_posttest_comment_len = 0;
    }

    if (m_included_data.datalog_note)
    {
        delete [] m_datalog_note;
        m_datalog_note = 0;
        m_datalog_note_len = 0;
    }

    if (m_test_limits)
    {
        delete m_test_limits;
        m_test_limits = 0;
    }

// CHANGES BEGIN 28 JUNE 2006
    if (m_plot_data)
    {
        delete m_plot_data;
        m_plot_data = 0;
    }
// CHANGES END 28 JUNE 2006
}

gsint32 DATALOG_VAR_LEN_DATA_PER_SUBTEST::LoadFromBuffer (const void *buffer)
{
    const BYTE *b = (const BYTE *) buffer;

    gsint32 i=0;
    gsint32 len = 1/*sizeof (m_included_data)*/;

    MemMove (&m_included_data,b+i,len); i += len; // ReadBuffer

    if (m_pretest_comment)
    {
        delete m_pretest_comment;
        m_pretest_comment = 0;
    }

    if (m_posttest_comment)
    {
        delete m_posttest_comment;
        m_posttest_comment = 0;
    }
    if (m_datalog_note)
    {
        delete m_datalog_note;
        m_datalog_note = 0;
    }

    m_pretest_comment_len = 0;
    m_posttest_comment_len = 0;
    m_datalog_note_len = 0;

    if (m_included_data.pretest_comment)
    {
        i += ReadWord(b+i, &m_pretest_comment_len);
        ASSERT_TAB_INDEX(m_pretest_comment_len);

        m_pretest_comment = new char[m_pretest_comment_len+1];
        ASSERT_LOCATION(m_pretest_comment);

        MemMove (m_pretest_comment,b+i,m_pretest_comment_len);  // ReadBuffer
        m_pretest_comment[m_pretest_comment_len] = 0;

        i += m_pretest_comment_len;
    }

    if (m_included_data.includes_test_limit_obj)
    {
        m_test_limits = new TestLimits;
        ASSERT_LOCATION(m_test_limits);
        i += m_test_limits->LoadFromBuffer (b+i);
    }

    if (m_included_data.posttest_comment)
    {
        i += ReadWord(b+i, &m_posttest_comment_len);
        ASSERT_TAB_INDEX(m_posttest_comment_len);

        m_posttest_comment = new char[m_posttest_comment_len+1];
        ASSERT_LOCATION(m_posttest_comment);

        MemMove (m_posttest_comment,b+i,m_posttest_comment_len);  // ReadBuffer
        m_posttest_comment[m_posttest_comment_len] = 0;
        i += m_posttest_comment_len;
    }

    if (m_included_data.datalog_note)
    {
        i += ReadWord(b+i, &m_datalog_note_len);
        ASSERT_TAB_INDEX(m_datalog_note_len);

        m_datalog_note = new char[m_datalog_note_len+1];
        ASSERT_LOCATION(m_datalog_note);

        MemMove (m_datalog_note,b+i,m_datalog_note_len);  // ReadBuffer
        m_datalog_note[m_datalog_note_len] = 0;
        i += m_datalog_note_len;
    }

 // CHANGES BEGIN 28 JUNE 2006
    if (m_plot_data)
    {
        delete m_plot_data;
        m_plot_data = 0 ;
    }

    if (m_included_data.includes_plot_data)
    {
        m_plot_data = new DLogPlotData;
        i += m_plot_data->LoadFromBuffer (b+i);
    }
// CHANGES END 28 JUNE 2006
   return i;
}

//-----------------------------------------------------------
PROGRAM_FUNCTIONS_DATA::PROGRAM_FUNCTIONS_DATA (void)
{
    m_functions_n = 0;
    m_functions = 0;

    m_status = 0;

    m_dll_time = 0;
    m_prg_time = 0;
    m_lst_time = 0;
    m_src_time = 0;

    MemSet (m_reserved,0,sizeof(m_reserved));

    m_cumulative_subtests_n = 0;
}

PROGRAM_FUNCTIONS_DATA::~PROGRAM_FUNCTIONS_DATA (void)
{
    for (short i=0; i<m_functions_n; i++) delete m_functions[i];
    delete [] m_functions;
    m_functions = 0;
    m_functions_n = 0;
}

gsint32 PROGRAM_FUNCTIONS_DATA::GetCumulativeSubTestsN (void)
{
    if (m_cumulative_subtests_n) return m_cumulative_subtests_n;

    for (short func_i=0; func_i<m_functions_n; func_i++)
    {
        m_cumulative_subtests_n += m_functions[func_i]->SubTestsN ();
    }

    return m_cumulative_subtests_n;
}

short PROGRAM_FUNCTIONS_DATA::FunctionsN()
{
    return m_functions_n;
}

ONE_FUNCTION *PROGRAM_FUNCTIONS_DATA::Function (const short i)
{
    if (i < 0 || i >= m_functions_n) return 0;
    return m_functions[i];
}
gsint32 PROGRAM_FUNCTIONS_DATA::GetStorageSizeBytes (void)
{
    gsint32 len = 0;

    len += sizeof (m_functions_n);
    len += sizeof (m_status);
    len += sizeof (m_dll_time);
    len += sizeof (m_prg_time);
    len += sizeof (m_lst_time);
    len += sizeof (m_src_time);
    len += sizeof (m_reserved);

    for (short i=0; i<m_functions_n; i++)
        len += m_functions[i]->GetStorageSizeBytes ();

    return len;
}

gsint32 PROGRAM_FUNCTIONS_DATA::LoadFromBuffer (const void *buffer)
{
    if (m_functions)
    {
        for (short func_i=0; func_i<m_functions_n; func_i++)
            delete m_functions[func_i];

        delete m_functions;  m_functions = 0;
    }

    const BYTE *b = (const BYTE *) buffer;

    gsint32 i = 0;

    i += ReadWord(b+i, &m_functions_n);

    // This is required in order to undo a previous bug
    if (IsDebugUnassignedMemory (b+i,78))
    {
        i += 80;
    }
    else
    {
        i += ReadDword(b+i, (gsint32*)&m_status);
        i += ReadDword(b+i, (gsint32*)&m_dll_time);
        i += ReadDword(b+i, (gsint32*)&m_prg_time);
        i += ReadDword(b+i, (gsint32*)&m_lst_time);
        i += ReadDword(b+i, (gsint32*)&m_src_time);
        for(int iIndex=0; iIndex<15; iIndex++)
            i += ReadDword(b+i, (gsint32*)&m_reserved[iIndex]);
    }

    m_functions = new ONE_FUNCTION*[m_functions_n];
    ASSERT_LOCATION(m_functions);
    if (!m_functions) return 0;

    for (short func_i=0; func_i<m_functions_n; func_i++)
    {
        m_functions[func_i] = new ONE_FUNCTION;
        ASSERT_LOCATION(m_functions[func_i]);
        if (!m_functions[func_i]) return 0;
        i += m_functions[func_i]->LoadFromBuffer (b+i);
    }

    return i;
}
BOOL IsDebugUnassignedMemory (const BYTE *b, gsint32 count)
{
    for (gsint32 i=0; i<count; i++)
    {
        if (b[i] != 0xCD)
            return false;
    }

    return true;
}

//--------------------------------------------------------------
void LOT_SUMMARY_DATA::SetName (const char *name)
{
    m_name = name;
}

BOOL LOT_SUMMARY_DATA::SetLotIDName (const char *s)
{
    m_lot_id_name = s;
    m_lot_id_name_included = true;
    return true;
}

LOT_SUMMARY_DATA::LOT_SUMMARY_DATA ()//ProgramDoc *prg_doc/*=0*/,
                                    //DataLogDoc *dlog_doc/*=0*/,
                                    //long record_start,
                                    //long record_end,
                                    //BOOL is_qa_retest_view) //[QA RETEST]
{
    gsint32 record_start=-1;
    gsint32 record_end=-1;
    clear ();

    m_bins_n = MAXIMUM_BINS;
    m_sites_n = site_count = MAX_SITE_IN_ASLHANDLER; // PR 599 support of 128 sites.  JR 3/1/02


    m_bin_name = new STRING[MAXIMUM_BINS];
    ASSERT_LOCATION(m_bin_name);
    m_hardware_bin = new STRING[MAXIMUM_BINS];
    ASSERT_LOCATION(m_hardware_bin);
    m_bin_count = new gsint32[MAXIMUM_BINS];
    ASSERT_LOCATION(m_bin_count);
    m_bin_count_qa = new gsint32[MAXIMUM_BINS];
    ASSERT_LOCATION(m_bin_count_qa);
    m_multi_site_count = new gsint32[m_bins_n*m_sites_n];
    ASSERT_LOCATION(m_multi_site_count);

    short bin_i;
    for (bin_i=0; bin_i<m_bins_n; bin_i++)
    {
        m_bin_count[bin_i] = 0;
        m_bin_count_qa[bin_i] = 0;
    }

    for (bin_i=0; bin_i<m_bins_n*m_sites_n; bin_i++)
        m_multi_site_count[bin_i] = 0;


    if (record_start >= 0)
        m_start_rec_in_dlog = record_start;

    if (record_end >= 0)
        m_end_rec_in_dlog = record_end;

    // Set the name and the time whenever a new lot summary is created
    STRING s;


    SetLotIDName (s);
    SetName (s);

//CHANGES BEGIN 28 June 2006
    m_OperatorName = "";
    m_ComputerName = "";
// CHANGES END 28 JUNE 2006

//CHANGES BEGIN 17 JULY 2006
    m_ProgramPath  = "";
//CHANGES END 17 JULY 2006

    m_HandlerName = "";

    m_DLOG_INFO_INX = 0;

    //time_t lTime;
    //localtime(lTime);

    QDateTime clDateTime;
    QDate clDate;
    QTime clTime;

    clDateTime = QDateTime::currentDateTime();
    clDate = clDateTime.date();
    clTime = clDateTime.time();

    m_time.wDay = clDate.day();
    m_time.wDayOfWeek = clDate.dayOfWeek();
    m_time.wMonth = clDate.month();
    m_time.wYear = clDate.year();

    m_time.wMilliseconds = clTime.msec();
    m_time.wSecond = clTime.second();
    m_time.wMinute = clTime.minute();
    m_time.wHour = clTime.hour();

    m_last_hundred_array = new BOOL[LAST_N_YIELD];
    ASSERT_LOCATION(m_last_hundred_array);
    for (short i = 0; i < LAST_N_YIELD; i++)
        m_last_hundred_array [i] = false;

    m_good_devices_in_last_hundred_by_site = new short[m_sites_n];
    ASSERT_LOCATION(m_good_devices_in_last_hundred_by_site);
    m_pointer_to_last_hundred_array_by_site = new short[m_sites_n];
    ASSERT_LOCATION(m_pointer_to_last_hundred_array_by_site);
    m_last_hundred_array_by_site = new BOOL * [m_sites_n];
    ASSERT_LOCATION(m_last_hundred_array_by_site);

    short sites_i;
    for (sites_i=0; sites_i<m_sites_n; sites_i++)
    {
        m_good_devices_in_last_hundred_by_site[sites_i] = 0;
        m_pointer_to_last_hundred_array_by_site[sites_i] = 0;
        m_last_hundred_array_by_site[sites_i] = new BOOL[LAST_N_YIELD];
        ASSERT_LOCATION(m_last_hundred_array_by_site);
        for (short j = 0; j < LAST_N_YIELD; j++)
            m_last_hundred_array_by_site [sites_i][j] = false;
    }
}

LOT_SUMMARY_DATA::~LOT_SUMMARY_DATA (void)
{
    delete [] m_bin_name;
    delete [] m_hardware_bin;
    delete [] m_bin_count;
    delete [] m_bin_count_qa;
    delete [] m_multi_site_count;
    delete m_test_time_stats;
    delete [] m_last_hundred_array;
    delete [] m_good_devices_in_last_hundred_by_site;
    delete [] m_pointer_to_last_hundred_array_by_site;
    for (short sites_i=0; sites_i<m_sites_n; sites_i++)
    {
        if (m_last_hundred_array_by_site[sites_i])
            delete [] m_last_hundred_array_by_site[sites_i];
    }
    if (m_last_hundred_array_by_site)
        delete [] m_last_hundred_array_by_site;

    clear ();
}


void LOT_SUMMARY_DATA::clear (void)
{
    m_bins_n = 0;
    m_bin_name = 0;
    m_hardware_bin = 0;
    m_bin_count = 0;
    m_bin_count_qa = 0;
    m_multi_site_count = 0;

    m_schema_info_assignment_field = 0;  // GPZ::UMR
    m_lot_id_name_included = true;
    m_asl_dos_equivalent = false;
    m_names_initialized = false;
    m_includes_comments = false;
    m_includes_multi_site = true;
    m_includes_per_test_stats = true;
    m_includes_time_of_last_test = false;
    m_includes_hardware_bins = true;
    m_display_hardware_bins = false;
    m_includes_display_multi = true;
    m_display_multi_site = false;
    m_includes_last_hundred_yield = true;
    m_includes_last_hundred_yield_qa_retest = true;
    m_includes_per_test_statistics_passed_devices_only = false;
    m_includes_flags = true;
    m_includes_qa = false;
    m_reserved = 1; // goganesy - now m_reserved should be always  = 1

//CHANGES BEGIN 17 JULY 2006
    m_AddinVersion = 3; // goganesy - addin version
//CHANGES END 17 JULY 2006
    m_Used_SW_Bins_count = 32; // by default - goganesy

    m_test_time_stats = 0;
    m_time_of_last_test = 0;

    m_passed_wafer_no = 0;
    m_summary_passed = 0;
    m_summary_failed = 0;

    m_name = ""; //Default";
    m_lot_id_name = "Default";
    m_last_device_num = 0;
    m_last_serial_num = 0;
    m_last_bin_num = 0;

    m_start_rec_in_dlog = 1;
    m_end_rec_in_dlog = 1;

    m_comments.SetStatusDeleteObjects (true);
    m_comments.RemoveAll ();
    m_per_test_stats.SetStatusDeleteObjects (true);
    m_per_test_stats.RemoveAll ();

    m_total_pass = 0xFFFFFFFF;
    m_total_fail = 0xFFFFFFFF;
    m_total_pass_qa_retest = 0xFFFFFFFF;
    m_total_fail_qa_retest = 0xFFFFFFFF;

    m_good_devices_in_last_hundred = 0;
    m_good_devices_in_last_hundred_qa_retest = 0;

    m_pointer_to_last_hundred_array = 0;
    m_good_devices_in_last_hundred_by_site = 0;
    m_pointer_to_last_hundred_array_by_site = 0;
    m_last_hundred_array_by_site = 0;

    m_flags  = 0L;
}

LOT_SUMMARY_DATA::LOT_SUMMARY_DATA (const void *buffer)
{
    clear ();
    LoadFromBuffer (buffer);
}

gsint32 LOT_SUMMARY_DATA::GetStorageSizeBytes (void)
{
    gsint32 n = 0;

    n += sizeof (m_schema_info_assignment_field);

    n += sizeof (m_bins_n);

    for (gsint32 i=0; i<m_bins_n; i++)
        n += StrLen (m_bin_name[i])+1;

    n += sizeof (m_bin_count[0])*m_bins_n;

    if (m_lot_id_name_included)
    {
        n += StrLen (m_lot_id_name) + 1;
    }

    if (m_asl_dos_equivalent == true)
    {
        if (!m_test_time_stats) return 0;
        n += m_test_time_stats->GetStorageSizeBytes ();

        n += sizeof (m_passed_wafer_no);
        n += sizeof (m_summary_passed);
        n += sizeof (m_summary_failed);
    }

    n += m_name.GetStorageSizeBytes ();
    n += sizeof (m_time);
    n += sizeof (m_last_device_num);
    n += sizeof (m_last_serial_num);
    n += sizeof (m_last_bin_num);
    n += sizeof (m_start_rec_in_dlog);
    n += sizeof (m_end_rec_in_dlog);

    if (m_includes_comments)
    {
        n += m_comments.GetStorageSizeBytes ();
    }

    if (m_includes_multi_site)
    {
        n += sizeof (m_sites_n);

        n += sizeof (m_multi_site_count[0])*m_bins_n*m_sites_n;
    }

    if (m_includes_per_test_stats)
    {
        n += m_per_test_stats.GetStorageSizeBytes ();
        n += sizeof (m_DLOG_INFO_INX);
    }

    if (m_includes_time_of_last_test)
        n += sizeof (m_time_of_last_test);

    if (m_includes_hardware_bins)
    {
        for (gsint32 i=0; i<m_bins_n; i++)
            n += StrLen (m_hardware_bin[i])+1;

        n += sizeof (m_display_hardware_bins);
    }

    if (m_includes_display_multi)
        n += sizeof (m_display_multi_site);

    if (m_includes_per_test_statistics_passed_devices_only)
        n += sizeof (m_per_test_statistics_passed_devices_only);

    if (m_includes_last_hundred_yield)
    {
        n += sizeof (m_good_devices_in_last_hundred);
        n += sizeof (m_pointer_to_last_hundred_array);
        n += sizeof	(m_last_hundred_array[0])*LAST_N_YIELD;

        n += sizeof (m_good_devices_in_last_hundred_by_site[0])*m_sites_n;
        n += sizeof (m_pointer_to_last_hundred_array_by_site[0])*m_sites_n;
        n += sizeof (m_last_hundred_array_by_site[0][0])*m_sites_n*LAST_N_YIELD;
    }

    if (m_includes_last_hundred_yield_qa_retest)
    {
        n += sizeof (m_good_devices_in_last_hundred_qa_retest);
    }

    if (m_includes_flags)
    {
        n += sizeof (m_flags);
    }

    if (m_includes_flags && m_sublot_id_name_included)
    {
        n += StrLen (m_sublot_id_name) + 1;
    }
    if (m_includes_qa)
        n += sizeof (m_bin_count_qa[0])*m_bins_n;

//CHANGES BEGIN 28 June 2006
    if (m_reserved) // goganesy - write new information to lot summary
    {
        n +=  sizeof (m_AddinVersion);
        if (m_AddinVersion >= 1) // do not chage this, just add new if (m_AddinVersion == N) bellow this block
        {
            n +=  sizeof (m_Used_SW_Bins_count);
        }

        if (m_AddinVersion >= 2)
        {
            n += StrLen(m_OperatorName) + 1;
            n += StrLen(m_ComputerName) + 1;
        }
//CHANGES BEGIN 17 JULY 2006
        if (m_AddinVersion >= 3)
        {
            n += StrLen(m_ProgramPath)  + 1;
        }
//CHANGES END 17 JULY 2006
        if (m_AddinVersion >= 4)
        {
            n += StrLen(m_HandlerName)  + 1;
        }
        //if (m_AddinVersion == N) // for future - goganesy
    }
// CHANGES END 28 JUNE 2006

    return n;
}


gsint32 LOT_SUMMARY_DATA::LoadFromBuffer (const void *v)
{
    const BYTE *b = (const BYTE *) v;

    gsint32 i=0;
    gsint32 len;
    int iIndex;

    i += ReadWord(b+i, (short*)&m_schema_info_assignment_field);
    i += ReadWord(b+i, &m_bins_n);

    if (m_bin_name) delete [] m_bin_name;
    m_bin_name = new STRING[MAXIMUM_BINS];
    ASSERT_LOCATION(m_bin_name);
    if (!m_bin_name)
        return 0;

    for (gsint32 j=0; j<m_bins_n; j++)
    {
        const char *name = (const char *)(b+i);
        len = StrLen (name)+1;
        m_bin_name[j] = name;
        i += len;
    }

    if (m_bin_count) delete [] m_bin_count;
    m_bin_count = new gsint32[MAXIMUM_BINS];
    ASSERT_LOCATION(m_bin_count);
    for(iIndex=0; iIndex<m_bins_n; iIndex++)
        i += ReadDword(b+i, (gsint32*)&m_bin_count[iIndex]);

    if (m_lot_id_name_included)
    {
        m_lot_id_name = (const char *) (b+i);
        i += StrLen (m_lot_id_name) + 1;
    }

    if (m_asl_dos_equivalent == true)
    {
        if (!m_test_time_stats)
        {
            m_test_time_stats = new TEST_TIME_STATS;
            ASSERT_LOCATION(m_test_time_stats);
            if (!m_test_time_stats)
                return 0;
        }

        i += m_test_time_stats->LoadFromBuffer (b+i);

        i += ReadDword(b+i, (gsint32*)&m_passed_wafer_no);
        i += ReadDword(b+i, (gsint32*)&m_summary_passed);
        i += ReadDword(b+i, (gsint32*)&m_summary_failed);
    }

    i += m_name.LoadFromBuffer (b+i);

    i += ReadWord(b+i, (short*)&m_time.wYear);
    i += ReadWord(b+i, (short*)&m_time.wMonth);
    i += ReadWord(b+i, (short*)&m_time.wDayOfWeek);
    i += ReadWord(b+i, (short*)&m_time.wDay);
    i += ReadWord(b+i, (short*)&m_time.wHour);
    i += ReadWord(b+i, (short*)&m_time.wMinute);
    i += ReadWord(b+i, (short*)&m_time.wSecond);
    i += ReadWord(b+i, (short*)&m_time.wMilliseconds);

    i += ReadDword(b+i, (gsint32*)&m_last_device_num);
    i += ReadDword(b+i, (gsint32*)&m_last_serial_num);
    i += ReadByte(b+i, &m_last_bin_num);
    i += ReadDword(b+i, &m_start_rec_in_dlog);
    i += ReadDword(b+i, &m_end_rec_in_dlog);

    if (m_includes_comments)
        i += m_comments.LoadFromBuffer (b+i);

    short old_sites_n = m_sites_n;
    if (m_includes_multi_site)
    {
        i += ReadWord(b+i, &m_sites_n);

        if (m_multi_site_count) delete m_multi_site_count;
        m_multi_site_count = new gsint32[MAXIMUM_BINS*m_sites_n];
        ASSERT_LOCATION(m_multi_site_count);
        if (!m_multi_site_count)
            return 0;

        for(iIndex=0; iIndex<m_bins_n*m_sites_n; iIndex++)
            i += ReadDword(b+i, (gsint32*)&m_multi_site_count[iIndex]);
    }

    if (m_includes_per_test_stats)
    {
        i += m_per_test_stats.LoadFromBuffer (b+i);

        i += ReadDword(b+i, &m_DLOG_INFO_INX);
    }

    if (m_includes_time_of_last_test)
    {
        i += ReadDword(b+i,reinterpret_cast<gsint32 *> (&m_time_of_last_test));
    }

  gsint32 tmp_dword;

    if (m_includes_hardware_bins)
    {
        if (m_bin_name) delete [] m_hardware_bin;
        m_hardware_bin = new STRING[MAXIMUM_BINS];
        ASSERT_LOCATION(m_hardware_bin);
        if (!m_hardware_bin)
            return 0;

        for (gsint32 j=0; j<m_bins_n; j++)
        {
            const char *name = (const char *)(b+i);
            len = StrLen (name)+1;
            m_hardware_bin[j] = name;
            i += len;
        }

        i += ReadDword(b+i, &tmp_dword);
        memcpy(&m_display_hardware_bins, &tmp_dword, 4);
    }

    if (m_includes_display_multi)
    {
        i += ReadDword(b+i, &tmp_dword);
        memcpy(&m_display_multi_site, &tmp_dword, 4);
    }

    if (m_includes_per_test_statistics_passed_devices_only)
    {
        i += ReadDword(b+i, &tmp_dword);
        memcpy(&m_per_test_statistics_passed_devices_only, &tmp_dword, 4);
    }

    if (m_includes_last_hundred_yield)
    {
        i += ReadWord(b+i, &m_good_devices_in_last_hundred);
        i += ReadWord(b+i, &m_pointer_to_last_hundred_array);
        if (m_last_hundred_array) delete [] m_last_hundred_array;
        m_last_hundred_array = new BOOL[LAST_N_YIELD];
        ASSERT_LOCATION(m_last_hundred_array);
        for(iIndex=0; iIndex<LAST_N_YIELD ;iIndex++)
            i += ReadDword(b+i, (gsint32*)&m_last_hundred_array[iIndex]);

        if (m_good_devices_in_last_hundred_by_site) delete [] m_good_devices_in_last_hundred_by_site;

        m_good_devices_in_last_hundred_by_site = new short[m_sites_n];
        ASSERT_LOCATION(m_good_devices_in_last_hundred_by_site);
        for(iIndex=0; iIndex<m_sites_n ;iIndex++)
            i += ReadWord(b+i, &m_good_devices_in_last_hundred_by_site[iIndex]);

        if (m_pointer_to_last_hundred_array_by_site) delete [] m_pointer_to_last_hundred_array_by_site;

        m_pointer_to_last_hundred_array_by_site = new short[m_sites_n];
        ASSERT_LOCATION(m_pointer_to_last_hundred_array_by_site);
        for(iIndex=0; iIndex<m_sites_n ;iIndex++)
            i += ReadWord(b+i, &m_pointer_to_last_hundred_array_by_site[iIndex]);

        if (m_last_hundred_array_by_site)
        {
            // mod
            for (short sites_i=0; sites_i<old_sites_n; sites_i++)
            {
                delete [] m_last_hundred_array_by_site[sites_i];
            }
            delete [] m_last_hundred_array_by_site;
        }

        m_last_hundred_array_by_site = new BOOL * [m_sites_n];
        ASSERT_LOCATION(m_last_hundred_array_by_site);

        short sites_i;
        for (sites_i=0; sites_i<m_sites_n; sites_i++)
        {
            m_last_hundred_array_by_site[sites_i] = new BOOL[LAST_N_YIELD];
            ASSERT_LOCATION(m_last_hundred_array_by_site[sites_i]);
            for (short j = 0; j < LAST_N_YIELD; j++)
                m_last_hundred_array_by_site [sites_i][j] = false;
        }

        len = sizeof (m_last_hundred_array_by_site[0][0])*LAST_N_YIELD;
        for (sites_i = 0; sites_i < m_sites_n; sites_i++)
        {
            for(iIndex=0; iIndex<LAST_N_YIELD; iIndex++)
                i += ReadDword(b+i, (gsint32*)&m_last_hundred_array_by_site[sites_i][iIndex]);
        }
    }

    if (m_includes_last_hundred_yield_qa_retest)
    {
        i += ReadWord(b+i, &m_good_devices_in_last_hundred_qa_retest);
    }

    if (m_includes_flags)
    {
        i += ReadDword(b+i, (gsint32*)&m_flags);
    }

    if (m_includes_flags && m_sublot_id_name_included)
    {
        m_sublot_id_name = (const char *) (b+i);
        i += StrLen (m_sublot_id_name) + 1;
    }

    if (m_includes_qa)
    {
        if (m_bin_count_qa) delete m_bin_count_qa;
        m_bin_count_qa = new gsint32[MAXIMUM_BINS];
        ASSERT_LOCATION(m_bin_count_qa);
        for(iIndex=0; iIndex<m_bins_n ;iIndex++)
            i += ReadDword(b+i, (gsint32*)&m_bin_count_qa[iIndex]);
    }

//CHANGES BEGIN 28 June 2006
    if (m_reserved) // goganesy - write new information to lot summary
    {
        if (m_AddinVersion >= 1 ) // do not chage this, just add new if (m_AddinVersion == N) bellow this block
        {
            len = ReadDword(b+i, &tmp_dword);
            memcpy(&m_Used_SW_Bins_count, &tmp_dword, 4);
            //len = sizeof (m_Used_SW_Bins_count);
            //MemMove (&m_Used_SW_Bins_count,b+i,len);

            if (m_Used_SW_Bins_count != 32 && m_Used_SW_Bins_count != 99) // in thus case it is Addin Version
            {
                i += ReadDword(b+i, &tmp_dword);
                memcpy(&m_AddinVersion, &tmp_dword, 4);
                //MemMove (&m_AddinVersion,b+i,len); // get Addin version from file
                //i += len;
                ReadDword(b+i, &tmp_dword);
                memcpy(&m_Used_SW_Bins_count, &tmp_dword, 4);
                //MemMove (&m_Used_SW_Bins_count,b+i,len);
            }
            else
                m_AddinVersion = 1; // here should be always 1!

            i += len;
        }

        if (m_AddinVersion >= 2) // do not chage this, just add new if (m_AddinVersion == N) bellow this block
        {
            m_OperatorName = (const char *) (b+i);
            i += StrLen (m_OperatorName) + 1;

            m_ComputerName = (const char *) (b+i);
            i += StrLen (m_ComputerName) + 1;
        }
//CHANGES BEGIN 17 JULY 2006
        if (m_AddinVersion >= 3) // do not chage this, just add new if (m_AddinVersion == N) bellow this block
        {
            m_ProgramPath = (const char *) (b+i);
            i += StrLen (m_ProgramPath) + 1;
        }
//CHANGES END 17 JULY 2006
        if (m_AddinVersion >= 4) // do not chage this, just add new if (m_AddinVersion == N) bellow this block
        {
            m_HandlerName = (const char *) (b+i);
            i += StrLen (m_HandlerName) + 1;
        }
        //if (m_AddinVersion == N) // for future - goganesy
    }
// CHANGES END 28 JUNE 2006
    return i;
}

BOOL LOT_SUMMARY_DATA::SetHandlerName (const char *s)
{
    m_HandlerName = s;
    return true;
}
const char *LOT_SUMMARY_DATA::GetHandlerName (void)
{
    if( (m_AddinVersion >= 4) && (strlen(m_HandlerName)) )
        return m_HandlerName;
    else
    {
        //!!!IMPLEMENTATION DIFFERENCE ALERT!!!!
        //The VisualATE version of this API returns "n/a" if a handler is not saved
        //in the datalog. For the STDF converter, want "missing/invalid" data ("")instead.
        return "";
    }
}

//CHANGES BEGIN 17 JULY 2006
BOOL LOT_SUMMARY_DATA::SetProgramPath (const char *s)
{
    STRING temp;
    temp.LoadFileName (s,true,false,false);
    m_ProgramPath = temp;
    return true;
}

void LOT_SUMMARY_DATA::GetProgramPath (STRING &temp, const char *s)
{
    //This function will try to extract the prog_path from the LotSummaryData object (m_AddinVersion >= 3).
    //If it's not there (old dl4 format, error, etc..), we'll try to derive the path. As a last
    //resort, we'll print "n/a".

    temp = "";

    //Check to see if the prog_path is stored in the LotSummaryData object
    if(m_AddinVersion >= 3)
    {
        if( !strlen(m_ProgramPath) )
        {
            //Nope, it's not there (but it should be). Try to derive the path.
            if( !strlen(s) )
            {
                //oh well, we tried. We should never get here!
                temp = "n/a";
            }
            else
            {
                //an error occured (prog_path) should be there. Derive the path.
                temp.LoadFileName (s,true,false,false);
            }
        }
        else
            temp = m_ProgramPath; //we got it!
    }
    else
    {
        //Nope, the prog_path is not stored. Now check to see if we can derive the path
        if( !strlen(s) )
        {
            //in previous versions of VATE, the computer name was printed in this situation (BUG!)
            //Let's print something a little bit more meaningful.
            temp = "n/a";
        }
        else
        {
            //not sure if we'll ever get here.
            temp.LoadFileName (s,true,false,false);
        }
    }
}
//CHANGES END 17 JULY 2006
//CHANGES BEGIN 28 June 2006
BOOL LOT_SUMMARY_DATA::SetTheComputerName (const char *s)
{
    m_ComputerName = s;
    return true;
}
const char *LOT_SUMMARY_DATA::GetTheComputerName (void)
{
//CHANGES BEGIN 17 JULY 2006
    if( (m_AddinVersion >= 2) && (strlen(m_ComputerName)) )
        return m_ComputerName;
    else
        return g_UI_computer_name;

//CHANGES END 17 JULY 2006
}

BOOL LOT_SUMMARY_DATA::SetOperatorName (const char *s)
{
    m_OperatorName = s;
    return true;
}
const char *LOT_SUMMARY_DATA::GetOperatorName (void)
{
//CHANGES BEGIN 17 JULY 2006
    if( (m_AddinVersion >= 2) && (strlen(m_OperatorName)) )
        return m_OperatorName;
    else
        return "";
//CHANGES END 17 JULY 2006
}
// CHANGES END 28 JUNE 2006

const char *LOT_SUMMARY_DATA::BinName (short bin_i)
{
    if (bin_i < 0 || bin_i >= m_bins_n) return 0;
    return m_bin_name[bin_i];
}

const char *LOT_SUMMARY_DATA::HardwareBin (short bin_i)
{
    if (bin_i < 0 || bin_i >= m_bins_n) return 0;
    return m_hardware_bin[bin_i];
}

const char *LOT_SUMMARY_DATA::GetLotIDName (void)
{
    return m_lot_id_name;
}

const char *LOT_SUMMARY_DATA::GetSubLotIDName (void)
{
    return m_sublot_id_name;
}

BOOL LOT_SUMMARY_DATA::GetStatusIncludesComments (void)
{
    return m_includes_comments;
}

PtrSet<STRING,short> &LOT_SUMMARY_DATA::Comments (void)
{
    return m_comments;
}

PtrSet<PER_TEST_STAT> &LOT_SUMMARY_DATA::TestStats (void)
{
    return m_per_test_stats;
}

gsint32 LOT_SUMMARY_DATA::MultiSiteBinCount (short bin_i, short site_i)
{
    if (bin_i < 0 || bin_i >= m_bins_n) return 0;
    if (site_i < 0 || site_i >= m_sites_n) return 0;
    return m_multi_site_count[site_i*m_bins_n+bin_i];
}

short LOT_SUMMARY_DATA::BinsN (void)
{
    return m_bins_n;
}

short LOT_SUMMARY_DATA::GetSitesN (void)
{
    return m_sites_n;
}

gsint32 LOT_SUMMARY_DATA::GetLastDeviceNumber (void)
{
    return m_last_device_num;
}

void  LOT_SUMMARY_DATA::SetLastDeviceNumber (gsint32 n)
{
    m_last_device_num = n;
}

gsint32 LOT_SUMMARY_DATA::GetLastSerialNumber (void)
{
    return m_last_serial_num;
}


//-----------------------------------------------
short ONE_FUNCTION::SubTestsN()
{
    return m_subtests_n;
}

FUNCTION_SUBTEST_DATA *ONE_FUNCTION::SubTestData (const short subtest_i)
{
    if (subtest_i < 0 || subtest_i >= m_subtests_n) return 0;
    return &m_subtest_data[subtest_i];
}

ONE_FUNCTION::~ONE_FUNCTION (void)
{
    delete [] m_param_data; m_param_data = 0;
    delete [] m_subtest_data; m_subtest_data = 0;
    m_params_n = 0;
    m_subtests_n = 0;
}

ONE_FUNCTION::ONE_FUNCTION (void)
{
    m_params_n = 0;
    m_param_data = 0;
    m_subtests_n = 0;
    m_subtest_data = 0;
    for (short i=0; i<10; i++) m_reserved[i] = 0;
    m_function_name << "NEW_FUNCTION";
    m_bStubbed = false;
}

gsint32 ONE_FUNCTION::LoadFromBuffer (const void *buffer)
{
    const BYTE *b = (const BYTE *) buffer;

    gsint32 i = 0;

    gsint32 len = 36;//sizeof m_function_name;
    MemMove (&m_function_name,b+i,len);			i += len; // ReadBuffer

    len = 36;//sizeof (m_function_c_lang_name);
    MemMove (&m_function_c_lang_name,b+i,len);	i += len; // ReadBuffer

    i += ReadWord(b+i, &m_params_n);

    if (m_param_data) { delete [] m_param_data; }
    m_param_data = new FUNCTION_PARAM_DATA[m_params_n];
    if (!m_param_data) return 0;

    short j;
    for (j=0; j<m_params_n; j++)
    {
        len = m_param_data[j].LoadFromBuffer (b+i);	i += len;
    }

    i += ReadWord(b+i, &m_subtests_n);

    if (m_subtest_data) delete [] m_subtest_data;
    m_subtest_data = new FUNCTION_SUBTEST_DATA[m_subtests_n];
    if (!m_subtest_data) return 0;

    for (j=0; j<m_subtests_n; j++)
    {
        len = m_subtest_data[j].LoadFromBuffer (b+i);	i += len;
    }

    for(int iIndex=0; iIndex<10; iIndex++)
        i += ReadDword(b+i, (gsint32*)&m_reserved[iIndex]);

    return i;
}


gsint32 ONE_FUNCTION::GetStorageSizeBytes (void)
{
    gsint32 len = 0;

    len += 36;//sizeof m_function_name;
    len += 36;//sizeof m_function_c_lang_name;

    len += sizeof m_params_n;
    short i;
    for (i=0; i<m_params_n; i++)
        len += m_param_data[i].GetStorageSizeBytes ();

    len += sizeof m_subtests_n;
    for (i=0; i<m_subtests_n; i++)
        len += m_subtest_data[i].GetStorageSizeBytes ();

    len += sizeof m_reserved;

    return len;
}


//------------------------------------------------------------
gsint32 PER_TEST_STAT::GetStorageSizeBytes (void)
{
    gsint32 n = 0;

    n += sizeof (m_status);
    n += sizeof (m_site_count);
    n += sizeof (m_power[0])*m_site_count;
    n += sizeof (m_max_value[0])*m_site_count;
    n += sizeof (m_min_value[0])*m_site_count;
    n += sizeof (m_device_count[0])*m_site_count;
    n += sizeof (m_sum_values[0])*m_site_count;
    n += sizeof (m_sq_sum_values[0])*m_site_count;
    n += sizeof (m_failure_count[0])*m_site_count;
    n += sizeof (m_fail_under_count[0])*m_site_count;
    n += sizeof (m_fail_over_count[0])*m_site_count;

    return n;
}

gsint32 PER_TEST_STAT::LoadFromBuffer (const void *v)
{
    const BYTE *b = (const BYTE *) v;

    gsint32 i=0;

    int iIndex;
    i += ReadByte(b+i, &m_status);

    if (m_status_bit7)
    {

        i += ReadWord(b+i, &m_site_count);

        delete [] m_power;
        delete [] m_max_value;
        delete [] m_min_value;
        delete [] m_device_count;
        delete [] m_sum_values;
        delete [] m_sq_sum_values;
        delete [] m_failure_count;
        delete [] m_fail_under_count;
        delete [] m_fail_over_count;

        m_power = new char [m_site_count];
        m_max_value = new double [m_site_count];
        m_min_value = new double [m_site_count];
        m_device_count = new gsint32 [m_site_count];
        m_sum_values = new double [m_site_count];
        m_sq_sum_values = new double [m_site_count];
        m_failure_count = new gsint32 [m_site_count];
        m_fail_under_count = new gsint32 [m_site_count];
        m_fail_over_count = new gsint32 [m_site_count];
    }


    for(iIndex=0; iIndex<m_site_count; iIndex++)
        i += ReadByte(b+i, (BYTE*)&m_power[iIndex]);
    for(iIndex=0; iIndex<m_site_count; iIndex++)
        i += ReadDouble(b+i, &m_max_value[iIndex]);
    for(iIndex=0; iIndex<m_site_count; iIndex++)
        i += ReadDouble(b+i, &m_min_value[iIndex]);
    for(iIndex=0; iIndex<m_site_count; iIndex++)
        i += ReadDword(b+i, &m_device_count[iIndex]);
    for(iIndex=0; iIndex<m_site_count; iIndex++)
        i += ReadDouble(b+i, &m_sum_values[iIndex]);
    for(iIndex=0; iIndex<m_site_count; iIndex++)
        i += ReadDouble(b+i, &m_sq_sum_values[iIndex]);
    for(iIndex=0; iIndex<m_site_count; iIndex++)
        i += ReadDword(b+i, &m_failure_count[iIndex]);
    if (m_status_bit6)
    {
        for(iIndex=0; iIndex<m_site_count; iIndex++)
            i += ReadDword(b+i, &m_fail_under_count[iIndex]);
        for(iIndex=0; iIndex<m_site_count; iIndex++)
            i += ReadDword(b+i, &m_fail_over_count[iIndex]);
    }

    return i;
}

PER_TEST_STAT::PER_TEST_STAT (short sites /* = 4 */)
{
    m_site_count = sites;
    m_power = new char [m_site_count];
    m_max_value = new double [m_site_count];
    m_min_value = new double [m_site_count];
    m_device_count = new gsint32 [m_site_count];
    m_sum_values = new double [m_site_count];
    m_sq_sum_values = new double [m_site_count];
    m_failure_count = new gsint32 [m_site_count];
    m_consecutive_fails_count = new gsuint32 [m_site_count];  // not persistent
    m_consecutive_fails_count_by_bin = new gsuint32 [MAX_SW_BINS];  // not persistent //PR#579

    m_consecutive_fails_count_by_bin_alarm_count = new gsuint32 [MAX_SW_BINS];  // not persistent //PR#579
    m_fail_under_count = new gsint32 [m_site_count];
    m_fail_over_count = new gsint32 [m_site_count];

    short i;
    for (i = 0; i < m_site_count; i++)
    {
        if (m_max_value)
            m_max_value[i]		 = -1.0e30F;
        if (m_min_value)
            m_min_value[i]		 = 1.0e30F;
        if (m_device_count)
            m_device_count[i]	 = 0;
        if (m_sum_values)
            m_sum_values[i]		 = 0;
        if (m_sq_sum_values)
            m_sq_sum_values[i]	 = 0;
        if (m_power)
            m_power[i]			 = 0;
        if (m_failure_count)
            m_failure_count[i]	 = 0;
        if (m_fail_under_count)
            m_fail_under_count[i]	 = 0;
        if (m_fail_over_count)
            m_fail_over_count[i]	 = 0;
        if (m_consecutive_fails_count)
            m_consecutive_fails_count[i] = 0;
    }
    //Begin PR#579
    if (m_consecutive_fails_count_by_bin)
    {
        for (i = 0; i <MAX_SW_BINS ; i++) //	MAX_FAILING_BINS
        {
            m_consecutive_fails_count_by_bin[i] = 0;
            m_consecutive_fails_count_by_bin_alarm_count[i] = 0;
        }
    }
    //End PR#579

    m_status = 0;
    m_status_bit7 = 1;  // indicates version 4.6 and above; sites > 4
    m_status_bit6 = 1;  // indicates changes added for IR, failed under and over
}

PER_TEST_STAT::~PER_TEST_STAT ()
{
    delete [] m_power;
    delete [] m_max_value;
    delete [] m_min_value;
    delete [] m_device_count;
    delete [] m_sum_values;
    delete [] m_sq_sum_values;
    delete [] m_failure_count;
    delete [] m_fail_under_count;
    delete [] m_fail_over_count;
    delete [] m_consecutive_fails_count;  // not persistent
    delete [] m_consecutive_fails_count_by_bin;  // not persistent //PR#579
    delete [] m_consecutive_fails_count_by_bin_alarm_count;  // not persistent //PR#579
}

gsint32 PER_TEST_STAT::GetFailureCount (short site_i)
{
    return m_failure_count [site_i];
}

gsint32 PER_TEST_STAT::GetDeviceCount (short site_i)
{
    return m_device_count [site_i];
}

//----------------------------------------------------------------
gsint32 DLOG_DB_HDR::GetStorageSizeBytes (void)
{
    gsint32 i = 0;

    i += sizeof (m_reserved);
    i += sizeof (m_status);
    i += m_prg_info_array.GetStorageSizeBytes ();
    i += sizeof (m_act_lot_summary);
    i += m_lot_summary_data_array.GetStorageSizeBytes ();
    i += m_site_count_array.GetStorageSizeBytes ();
    if (m_load_active_prg_info_idx)
        i += sizeof (m_active_prg_info_idx);
    if (m_load_auto_corr_status)
        i += sizeof (m_auto_corr_status);

    return i;
}

gsint32 DLOG_DB_HDR::LoadFromBuffer (const void *v)
{
    const BYTE *b = (const BYTE *) v;

    gsint32 i = 0;

    i += ReadDword(b+i, (gsint32*)&m_reserved);
    i += ReadWord(b+i, (short*)&m_status);

    i += m_prg_info_array.LoadFromBuffer (b+i);

    i += ReadDword(b+i, &m_act_lot_summary);

    i += m_lot_summary_data_array.LoadFromBuffer (b+i);

    i += m_site_count_array.LoadFromBuffer (b+i);

    if (m_load_active_prg_info_idx)
    {
        i += ReadDword(b+i, &m_active_prg_info_idx);
    }

    if (m_load_auto_corr_status)
    {
        i += ReadWord(b+i, &m_auto_corr_status);
    }

    return i;
}

DLOG_DB_HDR::DLOG_DB_HDR (void)
{
    m_reserved = 0;
    m_status = 0;
    m_act_lot_summary = 0;
    m_load_active_prg_info_idx = true;
    m_active_prg_info_idx = 0;
    m_load_auto_corr_status = true;
    m_auto_corr_status = 0;

    LOT_SUMMARY_DATA *lsd = new LOT_SUMMARY_DATA;

    m_lot_summary_data_array.Append (lsd);

    m_prg_info_array.SetStatusDeleteObjects (true);
    m_lot_summary_data_array.SetStatusDeleteObjects (true);

//CHANGES BEGIN 6 July 2006
    m_STDF_STRINGS = new STDF_STRINGS;
//CHANGES END 6 July 2006
    m_STDF_SUPPLEMENT = NULL;


}

DLOG_DB_HDR::~DLOG_DB_HDR (void)
{
//CHANGES BEGIN 6 July 2006
    if (m_STDF_STRINGS != NULL ) {
        delete m_STDF_STRINGS;
    }
//CHANGES END 6 July 2006

    if (m_STDF_SUPPLEMENT)
    {
        delete m_STDF_SUPPLEMENT;
    }
}
gsint32 DLOG_DB_HDR::GetLastSerialNumber (void)
{
    LOT_SUMMARY_DATA *lsd = Get_LOT_SUMMARY_DATA ();
    return lsd->GetLastSerialNumber ();
}

gsint32 DLOG_DB_HDR::GetLastDeviceNumber (void)
{
    LOT_SUMMARY_DATA *lsd = Get_LOT_SUMMARY_DATA ();
    return lsd->GetLastDeviceNumber ();
}


DLOG_PROGRAM_INFO *DLOG_DB_HDR::GetPrgInfo (gsint32 idx/*=-1*/)
{
    if (idx == -1)
        idx = m_active_prg_info_idx;

    return m_prg_info_array.GetPtr (idx);
}

//------------------------------------
DLOG_PROGRAM_INFO::DLOG_PROGRAM_INFO (void)
{
    m_program_name = "Default";
    m_prg_info = 0;
    m_reserved = 0;
    m_device_name_used = 1;
    m_program_mode_used = 1;
    m_program_revision_used = 1;
    m_program_test_code_used = 1;
    m_operation_step_number_used = 1;
    m_fract_devices_to_log_1_minus_used = 1;
    m_log_fraction_plus_fails_used = 1;
    m_fract_devices_to_log_1_minus = 1;
    m_log_fraction_plus_fails = false;
// serialization	m_limit_set_name_qa_retest_used = 1;
}


DLOG_PROGRAM_INFO::~DLOG_PROGRAM_INFO (void)
{
    if (m_prg_info)
    {
        delete m_prg_info;
        m_prg_info = 0;
    }
}

gsint32 DLOG_PROGRAM_INFO::GetStorageSizeBytes (void)
{
    gsint32 i = m_reserved_not_present.GetStorageSizeBytes ();

    if (m_prg_info)
        i += m_prg_info->GetStorageSizeBytes ();

    i += sizeof (m_reserved);

    i += m_program_name.GetStorageSizeBytes ();

    i += m_limit_set_name.GetStorageSizeBytes ();

    i += m_device_name.GetStorageSizeBytes ();

    i += m_program_mode.GetStorageSizeBytes ();
    i += m_program_revision.GetStorageSizeBytes ();
    i += m_program_test_code.GetStorageSizeBytes ();
    i += m_operation_step_number.GetStorageSizeBytes ();
    if (m_fract_devices_to_log_1_minus_used) {
        i += sizeof(m_fract_devices_to_log_1_minus);
    }
    if (m_log_fraction_plus_fails_used) {
        i += sizeof(m_log_fraction_plus_fails);
    }
// serialization	i += m_limit_set_name_qa_retest.GetStorageSizeBytes ();

    return i;
}

gsint32 DLOG_PROGRAM_INFO::LoadFromBuffer (const void *v)
{
    gsint32 i = 0;

    const BYTE *b = (const BYTE *) v;

    i += m_reserved_not_present.LoadFromBuffer (b+i);

    if (!m_prg_info)
        m_prg_info = new PROGRAM_FUNCTIONS_DATA;

    ASSERT_LOCATION(m_prg_info);

    i += m_prg_info->LoadFromBuffer (b+i);


    if (m_reserved_not_present.HasLen () == true)
    {
        m_program_name = m_reserved_not_present;
        m_reserved_not_present.Clear ();
    }
    else
    {
        i += ReadWord(b+i, (short*)&m_reserved);

        i += m_program_name.LoadFromBuffer (b+i);
        i += m_limit_set_name.LoadFromBuffer (b+i);
        if (m_device_name_used)
            i += m_device_name.LoadFromBuffer (b+i);

        if (m_program_mode_used)
            i += m_program_mode.LoadFromBuffer (b+i);
        if (m_program_revision_used)
            i += m_program_revision.LoadFromBuffer (b+i);
        if (m_program_test_code_used)
            i += m_program_test_code.LoadFromBuffer (b+i);
        if (m_operation_step_number_used)
            i += m_operation_step_number.LoadFromBuffer (b+i);
        if (m_fract_devices_to_log_1_minus_used) {
            i += ReadDword(b+i, (gsint32*)&m_fract_devices_to_log_1_minus);
        }
        if (m_log_fraction_plus_fails_used) {
      gsint32 tmp_dword;
            i += ReadDword(b+i, &tmp_dword);
            memcpy(&m_log_fraction_plus_fails, &tmp_dword, 4);
        }
    }

    return i;
}

PROGRAM_FUNCTIONS_DATA *DLOG_PROGRAM_INFO::PrgInfo (void)
{
    return m_prg_info;
}

//----------------------------------------------------------
TEST_TIME_STATS::TEST_TIME_STATS (void)
{
    m_average_test_time_usec = 0;
    m_max_test_time_usec = 0;
    m_min_test_time_usec = 0;
    m_prog_run_time.LowPart = 0;
    m_prog_run_time.HighPart = 0;
    m_average_wait_for_handler_usec = 0;
    m_average_wait_for_handler_usec = 0;
    m_max_wait_for_handler_usec = 0;
    m_min_wait_for_handler_usec = 0;
    m_handler_down_time_usec.LowPart = 0;
    m_handler_down_time_usec.HighPart = 0;
}

TEST_TIME_STATS::~TEST_TIME_STATS (void)
{
    return;
}



gsint32 TEST_TIME_STATS::GetStorageSizeBytes (void)
{
    gsint32 n = 0;

    n += sizeof (m_average_test_time_usec);
    n += sizeof (m_max_test_time_usec);
    n += sizeof (m_min_test_time_usec);
    n += sizeof (m_prog_run_time);
    n += sizeof (m_average_wait_for_handler_usec);
    n += sizeof (m_average_wait_for_handler_usec);
    n += sizeof (m_max_wait_for_handler_usec);
    n += sizeof (m_min_wait_for_handler_usec);
    n += sizeof (m_handler_down_time_usec);

    return n;
}

gsint32 TEST_TIME_STATS::LoadFromBuffer (const void *v)
{
    const BYTE *b = (const BYTE *) v;
    gsint32 i=0;

    i += ReadDword(b+i, (gsint32*)&m_average_test_time_usec);
    i += ReadDword(b+i, (gsint32*)&m_max_test_time_usec);
    i += ReadDword(b+i, (gsint32*)&m_min_test_time_usec);
    i += ReadDword(b+i, (gsint32*)&m_prog_run_time.LowPart);
    i += ReadDword(b+i, (gsint32*)&m_prog_run_time.HighPart);
    i += ReadDword(b+i, (gsint32*)&m_average_wait_for_handler_usec);
    i += ReadDword(b+i, (gsint32*)&m_average_wait_for_handler_usec);
    i += ReadDword(b+i, (gsint32*)&m_max_wait_for_handler_usec);
    i += ReadDword(b+i, (gsint32*)&m_min_wait_for_handler_usec);
    i += ReadDword(b+i, (gsint32*)&m_handler_down_time_usec.LowPart);
    i += ReadDword(b+i, (gsint32*)&m_handler_down_time_usec.HighPart);

    return i;
}

//-----------------------------------------------------------
FUNCTION_PARAM_DATA::FUNCTION_PARAM_DATA (void)
{
    m_multiplier = 1.0;
    m_options0 = 0;
    m_name << "NEW_PARAM";
    m_units << "";
    m_c_name << "NEW_PARAM";
}

FUNCTION_PARAM_DATA::~FUNCTION_PARAM_DATA (void) { return; }

gsint32 FUNCTION_PARAM_DATA::GetStorageSizeBytes (void)
{
    gsint32 len = StrLen (m_name) + 1;
    len += 12;//sizeof (m_multiplier);
    len += StrLen (m_units) + 1;
    len += StrLen (m_c_name) + 1;
    len += sizeof (m_options0);
    len += m_value.GetStorageSizeBytes ();

    return len;
}

gsint32 FUNCTION_PARAM_DATA::LoadFromBuffer (const void *v)
{
    const char *b = (const char *) v;

    gsint32 n=0;
    gsint32 i=0;

    n = StrLen (b+i)+1;
    m_name << (const char *) (b+i);		i += n;

    n = 12;//sizeof (m_multiplier);
    i += ReadDouble(b+i,&m_multiplier.d);
  gsint32 tmp_dword;
    i += ReadDword(b+i, &tmp_dword);
    memcpy(&m_multiplier.properties, &tmp_dword, 4);
    n = StrLen (b+i)+1;
    m_units << (const char *) (b+i);	i += n; // ReadBuffer

    n = StrLen (b+i)+1;
    m_c_name << (const char *) (b+i);	i += n; // ReadBuffer

    i += ReadByte(b+i, &m_options0);

    i += m_value.LoadFromBuffer (b+i);

    return i;
}

//--------------------------------------------------------------
FUNCTION_PARAM_VALUE::FUNCTION_PARAM_VALUE (void)
{
    m_type = PROP_TYPE_SHORT;
    m_address_of_data = 0;
    m_power = POWER_UNIT;
    m_short = new PROP_SHORT;
}

FUNCTION_PARAM_VALUE::~FUNCTION_PARAM_VALUE (void)
{
    clear ();
}

void FUNCTION_PARAM_VALUE::clear (void)
{
    if (!m_address_of_data) return;

    switch (m_type)
    {
        case PROP_TYPE_CHAR:		delete m_char;		break;
        case PROP_TYPE_STRING_128:	delete m_string_128;	break;
        case PROP_TYPE_SHORT:		delete m_short;		break;
        case PROP_TYPE_WORD:		delete m_word;		break;
        case PROP_TYPE_FLOAT:		delete m_float;		break;
        case PROP_TYPE_DOUBLE:		delete m_double;	break;
    }

    m_address_of_data = 0;

    m_type = PROP_TYPE_NODATA;
}

gsint32 FUNCTION_PARAM_VALUE::GetStorageSizeBytes (void)
{
    switch (m_type)
    {
        case PROP_TYPE_NODATA:		return 1;
        case  PROP_TYPE_CHAR:		return 1 + 5;//sizeof (PROP_CHAR);
        case  PROP_TYPE_STRING_128: return 2+StrLen (*m_string_128);
        case  PROP_TYPE_SHORT:		return 1 + 6;//sizeof (PROP_SHORT);
        case  PROP_TYPE_WORD:		return 1 + 6;//sizeof (PROP_WORD);
        case  PROP_TYPE_FLOAT:		return 1 + 8;//sizeof (PROP_FLOAT);
        case  PROP_TYPE_DOUBLE:		return 1 + 12;//sizeof (PROP_DOUBLE);
    }

    return 0;
}

gsint32 FUNCTION_PARAM_VALUE::LoadFromBuffer (const void *buffer)
{
    clear ();

    BYTE *b = (BYTE *) buffer;

    // The toLower 4 bits are m_type
    m_type = (BYTE)(0x0F & b[0]);

    if (m_type == PROP_TYPE_BYTE) return 0;

    // The upper 4 bits are m_power
    m_power = (0xF0 & b[0]) >> 4;

    SetValue ((void *) &b[1],m_type);
    return GetStorageSizeBytes ();
}


BOOL FUNCTION_PARAM_VALUE::SetValue (const void *buffer, BYTE type)
{
  gsint32 tmp_dword;
    clear ();

    m_type = type;

    switch (type)
    {
        case PROP_TYPE_NODATA:
            return true;

        case  PROP_TYPE_CHAR:
            m_char = new PROP_CHAR;
            MemMove (m_char,buffer,1); // ReadBuffer
            return true;

        case  PROP_TYPE_STRING_128:
            m_string_128 = new PROP_STRING_128;
            *m_string_128 << (const char *) buffer; // ReadBuffer
            return true;

        case  PROP_TYPE_SHORT:
            m_short = new PROP_SHORT;
            ReadWord(buffer,&m_short->value);
            return true;

        case  PROP_TYPE_WORD:
            m_word = new PROP_WORD;
            ReadWord(buffer,(short*)&m_word->value);
            return true;

        case  PROP_TYPE_FLOAT:
            m_float = new PROP_FLOAT;
            ReadDword(buffer, &tmp_dword);
            memcpy(&m_float->f, &tmp_dword, 4);
            return true;

        case  PROP_TYPE_DOUBLE:
            m_double = new PROP_DOUBLE;
            ReadDword(buffer, &tmp_dword);
            memcpy(&m_double->d, &tmp_dword, 4);
            return true;
    }
    return false;
}

//-----------------------------------------------------------------
gsint32 FUNCTION_SUBTEST_DATA::GetStorageSizeBytes (void)
{
    //long len = (long)((&m_below_storage_space - &m_above_storage_space) - 1);
    return 388 + sizeof(int) + sizeof(char);//len;
}

gsint32 FUNCTION_SUBTEST_DATA::LoadFromBuffer (const void *v)
{
    unsigned int iIndex, i = 0;
    BYTE *b = (BYTE *) v;
  gsint32 tmp_dword;

    MemMove (&m_subtest_name,b+i,20);		i+=20;
    MemMove (&m_max_limit_active,b+i,5);		i+=5;
    MemMove (&m_min_limit_active,b+i,5);		i+=5;
    MemMove (&m_limits_type_float,b+i,5);	i+=5;
    MemMove (&m_limits_type_ushort,b+i,5);	i+=5;
    for(iIndex=0; iIndex<LIMITS_ARRAY_SIZE; iIndex++)
    {
        i += ReadDword(b+i, &tmp_dword);
        memcpy(&m_max_limit[iIndex].f, &tmp_dword, 4);
        i += ReadDword(b+i, &tmp_dword);
        memcpy(&m_max_limit[iIndex].properties, &tmp_dword, 4);
    }
    for(iIndex=0; iIndex<LIMITS_ARRAY_SIZE; iIndex++)
    {
        i += ReadDword(b+i, &tmp_dword);
        memcpy(&m_min_limit[iIndex].f, &tmp_dword, 4);
        i += ReadDword(b+i, &tmp_dword);
        memcpy(&m_min_limit[iIndex].properties, &tmp_dword, 4);
    }
    MemMove (&m_units,b+i,12);				i+=12;
    MemMove (&m_note,b+i,20);				i+=20;
    MemMove (&m_pretest_comment,b+i,84);		i+=84;
    MemMove (&m_display_results,b+i,5);		i+=5;
    MemMove (&m_power,b+i,5);				i+=5;
    i += ReadDword(b+i,(gsint32*)&m_options0);
    i += ReadByte(b+i,&m_reserved0);
    MemMove (&m_delta_limit_active,b+i,5);	i+=5;
    for(iIndex=0; iIndex<4; iIndex++)
    {
        MemMove (&m_reserved[iIndex],b+i,5);	i+=5;
    }
    i += ReadDword(b+i, &tmp_dword);
    memcpy(&m_delta_limit.f, &tmp_dword, 4);
    i += ReadDword(b+i, &tmp_dword);
    memcpy(&m_delta_limit.properties, &tmp_dword, 4);
    MemMove (&m_subtest_user_number,b+i,20);	i+=20;
    MemMove (&m_iSignature,b+i,sizeof(int));	i+=sizeof(int);
    MemMove (&m_TST_TYP,b+i,sizeof(char));	i+=sizeof(char);
    for(iIndex=0; iIndex<(128 - 8 /*sizeof(PROP_FLOAT)*/ - 20 /*sizeof(PROP_STRING_16)*/ - sizeof(int) - sizeof(char)); iIndex++)
    {
        MemMove (&m_reserved_block[iIndex],b+i,1);			i+=1;
    }
    if (m_iSignature == 0) m_TST_TYP = 'P'; // TEST_TYPE_PARAMETRIC
    return i;
}
/////////////////////////////////// OTHER ///////////////////////////////////


