#ifndef DL4_IMPORT
#define DL4_IMPORT

#include <qdatetime.h>

#include "dl4_db.h"
#include "stdfparse.h"

typedef struct  _DL4_SYSTEMTIME
    {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
    }	DL4_SYSTEMTIME;

typedef struct  _DL4_LARGE_INTEGER
    {
    gsint32 LowPart;
    gsint32 HighPart;
    }	DL4_LARGE_INTEGER;

//CHANGES BEGIN 21 September 2006
struct TestHeaderSupplement
{
    short version; 	  //version of the TestHeaderSupplement
    float test_time;  //total test time in mS per device (as reported in the datalog)

    //add new items here...
    char padding[128];//extra space for future needs
};
//CHANGES END 21 September 2006


#define DL4_ULONG gsuint32

#define LAST_N_YIELD 100


#define LONG_INVALID_DATA	4294967295UL

#define UNKNOWN_RESULT	0xFE	// Put into <passed_failed> before result known
#define FAILED_TEST		0xFF	// Put into <passed_failed> upon failure
#define MAX_SW_BINS     99		// goganesy FREmn10312

#define MAXIMUM_BINS MAX_SW_BINS //  goganesy FREmn10312

#define TEST_TYPE_PARAMETRIC	'P'
#define TEST_TYPE_FUNCTIONAL	'F'
#define TEST_TYPE_MULTIPLE		'M'
#define TEST_TYPE_UNKNOWN		' '

#define MIR_PRODUCT		'P'

#define HBR_PASS_BIN	'P'
#define HBR_FAIL_BIN	'F'

#define SBR_PASS_BIN	'P'
#define SBR_FAIL_BIN	'F'

#define ASL_CFG_3000	2

extern char power_translate[];

extern WORD		g_backplane;
extern STRING   g_UI_computer_name;

enum
{
    HAND_TYP,
    HAND_ID,
    CARD_TYP,
    CARD_ID,
    LOAD_TYP,
    LOAD_ID,
    DIB_TYP,
    DIB_ID,
    CABL_TYP,
    CABL_ID,
    CONT_TYP,
    CONT_ID,
    LASR_TYP,
    LASR_ID,
    EXTR_TYP,
    EXTR_ID
};

enum	// Variable Length Data Records In MIR
{
    LOT_ID,
    PART_TYP,
    NODE_NAM,
    TSTR_TYP,
    JOB_NAM,
    JOB_REV,
    SBLOT_ID,
    OPER_NAM,
    EXEC_TYP,
    EXEC_VER,
    TEST_COD,
    TST_TEMP,
    USER_TXT,
    AUX_FILE,
    PKG_TYP,
    FAMLY_ID,
    DATE_COD,
    FACIL_ID,
    FLOOR_ID,
    PROC_ID,
    OPER_FRQ,
    SPEC_NAM,
    SPEC_VER,
    FLOW_ID,
    SETUP_ID,
    DSGN_REV,
    ENG_ID,
    ROM_COD,
    SERL_NUM,
    SUPR_NAM,
};

// CHANGES BEGIN 28 JUNE 2006
class DLogPlotData;
class OneAxisPlotData;
// CHANGES END 28 JUNE 2006
class TEST_TIME_STATS;
class PER_TEST_STAT;
class TestLimits;
class subtest_result_info;
class DATALOG_VAR_LEN_DATA_PER_SUBTEST;
class TestHeader;
class DLOG_PROGRAM_INFO;
class LOT_SUMMARY_DATA;
class PROGRAM_FUNCTIONS_DATA;
class ONE_FUNCTION;
class SUBTEST_VALUE_ID;
class FUNCTION_SUBTEST_DATA;
class HANDLER_BIN;
class ResultsTypeID;
class FUNCTION_PARAM_VALUE;
class FUNCTION_PARAM_DATA;
class DLOG_DB_VLD;
class DLOG_DB_HDR;
class DLOG_DB_FLD;
class DataLogInfo;
class STDF_STRINGS;
class STDF_SUPPLEMENT;


struct dlog_test;

struct dlog_entry			// After the test_header this is written for each test.
{
    // 9 bytes storage. This must not change!!!!
    char					valid_value;
    char					passed_fail;
    float					measured_value;
    char					power;
    unsigned char			prescript_length;
    unsigned char			postscript_length;
    void reset()
    {
        valid_value			= 0;
        passed_fail			= 0;
        measured_value		= 0;
        power				= 0;
        prescript_length	= 0;
        postscript_length	= 0;
    }
};

struct dos_test_header			// Put out to file for each test performed
{
    // 13 bytes storage. This must not change!!!!
    unsigned char			write_type;
    time_t					now;
    unsigned char			hundreths;
    unsigned short			device_num;
    short					ret_val;
    unsigned char			bin_num;
    unsigned short			serial_num;
    void reset()
    {
        write_type	= 0;
        now			= 0;
        hundreths	= 0;
        device_num	= 0;
        ret_val		= 0;
        bin_num		= 0;
        serial_num	= 0;
    }
};

struct test_header			// Put out to file for each test performed
{
    // 21 bytes storage. This must not change!!!!
    unsigned char			write_type;
    time_t					now;
    unsigned char			hundreths;
    gsint32					device_num;
    short					ret_val;
    unsigned char			bin_num;
    gsint32					serial_num;
    short                   x_coordinate;
    short					y_coordinate;
//CHANGES BEGIN 21 September 2006
    TestHeaderSupplement    test_header_supplement;
//CHANGES END 21 September 2006
    void reset()
    {
        write_type	= 0;
        now			= 0;
        hundreths	= 0;
        device_num	= 0;
        ret_val		= 0;
        bin_num		= 0;
        serial_num	= 0;
        x_coordinate = 0;
        y_coordinate = 0;
        test_header_supplement.test_time = 0;
    }
};

typedef struct
{
    // 83 bytes storage. This must not change!!!!
    char name[13];
    short type;
    union
    {
        char c;
        char s[40];
        short i;
        WORD u;
        float f;
        double d;
    } val;
    double multiplier;
    char units[7];
    char c_name[13];
} develop_parameter;

enum // Define values that can be passed to display results to calculate power
{
    POWER_TERRA,
    POWER_GIGA,
    POWER_MEGA,
    POWER_KILO,
    POWER_UNIT,
    POWER_MILLI,
    POWER_MICRO,
    POWER_NANO,
    POWER_PICO,
    POWER_FEMTO,
    POWER_HEX			// Special Case for Datalogging Hex Values 18th July 1994
};




class CDL4toSTDF :
    public BinaryDatabase
{
public:
    CDL4toSTDF(void);
    ~CDL4toSTDF(void);

    bool			Convert(const char *szFileNameDl4, const char *szFileNameStdf);
    QString			GetLastError();
    void			ErrorMessage (const char *e0, const char *e1=0, const char *e2=0, const char *e3=0, const char *e5=0);

    static bool	IsCompatible(const char *szFileName);

private:

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    int	iProgressStep;
    int	iNextCount;
    int	iTotalCount;
    int	iCount;

    QString m_strLastErrorMsg;
    QString strFileNameDl4;
    QString strFileNameStdf;

    BOOL STDF_Convert ();

//---------------------------------------------------------------------------
    BOOL OpenAnExistingDatalog ();


    DLOG_DB_HDR *GetDBHeader (void);
    PROGRAM_FUNCTIONS_DATA *get_pfd (DLOG_DB_FLD *FLD =0);

    PROGRAM_FUNCTIONS_DATA *Get_PROGRAM_FUNCTIONS_DATA (DLOG_DB_FLD *FLD=0);


    BOOL STDF_PCR_Record_Write (
                            short sites_i, gsuint32 **bin_count,
                            gsuint32 *loop);
    BOOL STDF_SBR_Record_Write (LOT_SUMMARY_DATA *pLSD,
                            short sites_i,
                            gsuint32 **bin_count);
    BOOL STDF_HBR_Record_Write (LOT_SUMMARY_DATA *pLSD,
                            short sites_i,
                            gsuint32 **bin_count);


    BOOL STDF_TSR_Record_Write (short sites_i,
                            gsuint32 **fails,gsuint32 **exec);

    BOOL STDF_WRR_Record_Write (gsuint32 *last_time,
                            gsuint32 **bin_count, gsuint32 *loop,
                            short sites_i);

    BOOL STDF_MRR_Record_Write ( gsuint32 *last_time,
                            short sites_i);
    BOOL STDF_Open_File (const char *file_name);
    BOOL STDF_Create_Arrays (gsuint32 * &loop, gsuint32 ** &fails, gsuint32 ** &exec,
                         gsuint32 * &last_time, gsuint32 * &first_time,
                         gsuint32 ** &bin_count, short sites_i, short subtest_total);
    BOOL STDF_SDR_Record_Write (LOT_SUMMARY_DATA *pLSD,
                            short sites_i);
    BOOL STDF_MIR_Record_Write (const char *program_name, gsuint32 *first_time,
                            LOT_SUMMARY_DATA *pLSD,
                            short sites_i);
    BOOL STDF_ATR_Record_Write ();
    BOOL STDF_FAR_Record_Write ();
    BOOL STDF_Get_Start_Time (gsint32 start_rec, gsint32 nrec,
                          gsuint32 *first_time, short sites_i);
    BOOL STDF_WCR_Record_Write (short doing_site);
    BOOL STDF_WIR_Record_Write (time_t start_t, short doing_site);
    BOOL STDF_PIR_Record_Write (short doing_site);
    BOOL STDF_PTR_Record_Write (BOOL *semi_static_sent, gsint32 rec_num,
                            const dlog_entry *sub_test_results,
                            FUNCTION_SUBTEST_DATA *pFSD,
                            short doing_site, short func_i,	short subtest_i,
                            short subtest_count,gsuint32 **fails,gsuint32 **exec,
                            DLOG_DB_VLD *pVLD,
                            bool limits_changed);
    BOOL STDF_PRR_Record_Write (LOT_SUMMARY_DATA *pLSD, const test_header &header, short doing_site, short subtest_count);

    GQTL_STDF::StdfParse	m_cStdfParse;




    gsint32 GetLastDeviceNumber (void);
    gsint32 GetLastSerialNumber (void);



    DLOG_DB_HDR *m_HDR;
    DLOG_DB_FLD *m_last_FLD;
    DLOG_DB_VLD *m_last_VLD;

    gsuint32 m_serial_num;
    gsuint32 m_device_num;
//    unsigned char cpu_type;
    QDate	*m_pExpirationDate;

    // STDF analyses
    int		m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_COUNT];	// Array of records to display
};


class TestHeader : public StoredObject
{
public:
//CHANGES BEGIN 21 September 2006
    TestHeader (void)
    {
        m_nt_test_header.reset();
        m_status_bit5 = 1; //added supplement struct to test_header
        m_status_bit6 = 1; //added XY coords in test_header
        m_status_bit7 = 1; //NT test_header
    };

    ~TestHeader (void){	};

    gsint32 GetStorageSizeBytes (void)
    {
        if (m_status_bit7)
        {
            if (m_status_bit6)
            {
                if(m_status_bit5)
                    return 21 + sizeof(short) + sizeof(float) + sizeof(char[128]); //sizeof (m_nt_test_header)
                else
                    return 21; //sizeof(m_nt_test_header) - sizeof(TestHeaderSupplement);
            }
            else
                return 21 - (sizeof (x_coordinate) + sizeof (y_coordinate)); //(sizeof (m_nt_test_header) - (sizeof (x_coordinate) + sizeof (y_coordinate)) - sizeof(TestHeaderSupplement));
        }
        else
        {
            return 13/*sizeof (m_dos_test_header)*/;
        }
    };


    gsint32 LoadFromBuffer (const void *buffer)
    {
        ASSERT_LOCATION(buffer);
        BYTE *b = (BYTE *) buffer;
        ReadByte(b, &m_status);

        if (m_status_bit7)
        {
            if (m_status_bit6)
            {
                if(m_status_bit5)
                {
                    int i = 0;
                    i += ReadByte(b+i, &m_nt_test_header.write_type);
                    i += ReadDword(b+i,reinterpret_cast<gsint32 *>(&m_nt_test_header.now));
                    i += ReadByte(b+i, &m_nt_test_header.hundreths);
                    i += ReadDword(b+i, &m_nt_test_header.device_num);
                    i += ReadWord(b+i, &m_nt_test_header.ret_val);
                    i += ReadByte(b+i, &m_nt_test_header.bin_num);
                    i += ReadDword(b+i, &m_nt_test_header.serial_num);
                    i += ReadWord(b+i, &m_nt_test_header.x_coordinate);
                    i += ReadWord(b+i, &m_nt_test_header.y_coordinate);
                    i += ReadWord(b+i, &m_nt_test_header.test_header_supplement.version);
                    i += ReadFloat(b+i, &m_nt_test_header.test_header_supplement.test_time);
                    int iIndex;
                    for(iIndex=0; iIndex<128; iIndex++)
                        i += ReadByte(b+i, (unsigned char*)&m_nt_test_header.test_header_supplement.padding[iIndex]);
                    return i;
                }
                else
                {
                    int i = 0;
                    i += ReadByte(b+i, &m_nt_test_header.write_type);
                    i += ReadDword(b+i,reinterpret_cast<gsint32 *>(&m_nt_test_header.now));
                    i += ReadByte(b+i, &m_nt_test_header.hundreths);
                    i += ReadDword(b+i, &m_nt_test_header.device_num);
                    i += ReadWord(b+i, &m_nt_test_header.ret_val);
                    i += ReadByte(b+i, &m_nt_test_header.bin_num);
                    i += ReadDword(b+i, &m_nt_test_header.serial_num);
                    i += ReadWord(b+i, &m_nt_test_header.x_coordinate);
                    i += ReadWord(b+i, &m_nt_test_header.y_coordinate);
                    return i;
                }
            }
            else
            {
                int i = 0; // ReadBuffer
                i += ReadByte(b+i, &m_nt_test_header.write_type);
                i += ReadDword(b+i,reinterpret_cast<gsint32 *>(&m_nt_test_header.now));
                i += ReadByte(b+i, &m_nt_test_header.hundreths);
                i += ReadDword(b+i, &m_nt_test_header.device_num);
                i += ReadWord(b+i, &m_nt_test_header.ret_val);
                i += ReadByte(b+i, &m_nt_test_header.bin_num);
                i += ReadDword(b+i, &m_nt_test_header.serial_num);
                return i;
            }
        }
        else
        {
            // Convert
            struct dos_test_header tmp_dos_test_header;
            int i=0; // ReadBuffer
            i += ReadByte(b+i, &tmp_dos_test_header.write_type);
            i += ReadDword(b+i, reinterpret_cast<gsint32 *>( &tmp_dos_test_header.now));
            i += ReadByte(b+i, &tmp_dos_test_header.hundreths);
            i += ReadWord(b+i, (short*)&tmp_dos_test_header.device_num);
            i += ReadWord(b+i, &tmp_dos_test_header.ret_val);
            i += ReadByte(b+i, &tmp_dos_test_header.bin_num);
            i += ReadWord(b+i, (short*)&tmp_dos_test_header.serial_num);

            // convert utility
            m_nt_test_header.write_type = tmp_dos_test_header.write_type;
            m_nt_test_header.now		= tmp_dos_test_header.now;
            m_nt_test_header.hundreths  = tmp_dos_test_header.hundreths;
            m_nt_test_header.device_num = tmp_dos_test_header.device_num;
            m_nt_test_header.ret_val    = tmp_dos_test_header.ret_val;
            m_nt_test_header.bin_num    = tmp_dos_test_header.bin_num;
            m_nt_test_header.serial_num = tmp_dos_test_header.serial_num;

//			ASSERT(i == 13);
            return i;
        }
    };
//CHANGES end 21 September 2006

public:
    union
    {
        struct
        {
            union
            {
                BYTE m_status;
                struct
                {
                    BYTE m_status_bit0 : 1;
                    BYTE m_status_bit1 : 1;
                    BYTE m_status_bit2 : 1;
                    BYTE m_status_bit3 : 1;
                    BYTE m_status_bit4 : 1;
                    BYTE m_status_bit5 : 1;
                    BYTE m_status_bit6 : 1;
                    BYTE m_status_bit7 : 1;
                };
            };
        };

        struct dos_test_header m_dos_test_header;
        union
        {
            struct test_header m_nt_test_header;
            struct
            {
                unsigned char			write_type;
                time_t					now;
                unsigned char			hundreths;
                gsint32					device_num;
                short					ret_val;
                unsigned char			bin_num;
                gsint32					serial_num;
                short                   x_coordinate;
                short					y_coordinate;
//CHANGES BEGIN 21 September 2006
                TestHeaderSupplement    test_header_supplement;
//CHANGES end 21 September 2006
            };
        };
    };
};


class DLOG_DB_HDR : public StoredObject
{
public:
    DLOG_DB_HDR (void);
    ~DLOG_DB_HDR (void);

    DLOG_PROGRAM_INFO *GetPrgInfo (gsint32 idx=-1);

    gsint32 GetLastDeviceNumber (void);
    gsint32 GetLastSerialNumber (void);

    gsint32 GetStorageSizeBytes (void);
    gsint32 LoadFromBuffer (const void *buffer);

    inline LOT_SUMMARY_DATA *Get_LOT_SUMMARY_DATA (void)
    {
        gsint32 n = m_lot_summary_data_array.GetCount ();
        return m_lot_summary_data_array.GetObjectPtr (n-1);
    }

    inline PtrSet<SO_DWORD> *GetSiteCountArray (void)
    {
        return &m_site_count_array;
    }

private:

    gsint32 m_reserved;

    PtrSet<DLOG_PROGRAM_INFO> m_prg_info_array;

    gsint32 m_active_prg_info_idx;

    gsint32 m_act_lot_summary;
    PtrSet<LOT_SUMMARY_DATA> m_lot_summary_data_array;

    PtrSet<SO_DWORD> m_site_count_array;

    union
    {
        WORD m_status;

        struct
        {
            WORD m_load_comments			    : 1;
            WORD m_load_header					: 1;
            WORD m_load_active_prg_info_idx		: 1;
            WORD m_load_auto_corr_status		: 1;
        };
    };

    short m_auto_corr_status;

public:
    STDF_STRINGS *m_STDF_STRINGS;		// Not Saved to Buffer
    STDF_SUPPLEMENT *m_STDF_SUPPLEMENT;	// Not Saved to Buffer
};

class DLOG_PROGRAM_INFO : public StoredObject
{
public:

    DLOG_PROGRAM_INFO (void);
    ~DLOG_PROGRAM_INFO (void);

    BOOL operator < (DLOG_PROGRAM_INFO &);
    BOOL operator > (DLOG_PROGRAM_INFO &);

    gsint32 GetStorageSizeBytes (void);
    gsint32 LoadFromBuffer (const void *);

    PROGRAM_FUNCTIONS_DATA *PrgInfo (void);

    inline const char *GetName (void) { return m_program_name; }

    inline const char *GetLimitSetName (void) { return m_limit_set_name; }

    inline const char *GetDeviceName (void) { return m_device_name; }

    inline const char *GetProgramRevision (void) { return m_program_revision; }

    inline const char *GetProgramTestCode (void) { return m_program_test_code; }

    inline const char *GetOperationStepNumber (void) { return m_operation_step_number; }


private:

    STRING m_reserved_not_present; // if (m_reserved_not_present.HasLen () == true) -- Then m_reserved has not been stored

    PROGRAM_FUNCTIONS_DATA *m_prg_info;

    union
    {
        WORD m_reserved;

        struct
        {
            WORD m_device_name_used           : 1;
            WORD m_program_mode_used          : 1;
            WORD m_program_revision_used      : 1;
            WORD m_program_test_code_used     : 1;
            WORD m_operation_step_number_used : 1;
            WORD m_limit_set_name_qa_retest_used : 1;
            WORD m_fract_devices_to_log_1_minus_used : 1;
            WORD m_log_fraction_plus_fails_used : 1;
        };
    };

    STRING m_program_name;  // Not used unless m_reserved is present
    STRING m_limit_set_name;
    STRING m_device_name;
    STRING m_program_mode;
    STRING m_program_revision;
    STRING m_program_test_code;
    STRING m_operation_step_number;
    STRING m_limit_set_name_qa_retest;

public:
    gsint32 m_fract_devices_to_log_1_minus;
    BOOL m_log_fraction_plus_fails;

};

class LOT_SUMMARY_DATA : public StoredObject
{
public:
    LOT_SUMMARY_DATA (const void *buffer);
    LOT_SUMMARY_DATA();
    ~LOT_SUMMARY_DATA (void);

    gsint32 GetStorageSizeBytes (void);
    gsint32 LoadFromBuffer (const void *v);

    BOOL Add (LOT_SUMMARY_DATA &lsd);
    void Clear (void);
    void Reset (void);

    short BinsN (void);
    const char *BinName (short bin_i);
    const char *HardwareBin (short bin_i);

    gsint32 MultiSiteBinCount (short bin_i, short site_i);

    const char *GetLotIDName (void);
    BOOL SetLotIDName (const char *s);
    const char *GetSubLotIDName (void);
// CHANGES BEGIN 28 JUNE 2006
    const char *GetOperatorName (void);
    BOOL SetOperatorName (const char *s);
    const char *GetTheComputerName (void);
    BOOL SetTheComputerName (const char *s);
// CHANGES END 28 JUNE 2006

//CHANGES BEGIN 17 JULY 2006
    void GetProgramPath (STRING &temp, const char *s);
    BOOL SetProgramPath (const char *s);
//CHANGES END 17 JULY 2006

    const char *GetHandlerName (void);
    BOOL SetHandlerName (const char *s);

    gsint32 GetLastDeviceNumber (void);
    void  SetLastDeviceNumber (gsint32 n);
    gsint32 GetLastSerialNumber (void);

    void SetName (const char *name);
    BOOL operator > (LOT_SUMMARY_DATA *lsd);
    BOOL operator < (LOT_SUMMARY_DATA *lsd);

    BOOL GetStatusIncludesComments (void);
    PtrSet<STRING,short> &Comments (void);

    PtrSet<PER_TEST_STAT> &TestStats (void);

    short GetSitesN (void);

protected:

    void clear (void);
public:
    union
    {
        WORD m_schema_info_assignment_field;
        struct
        {
            WORD m_lot_id_name_included		   : 1; // bit0 // used
            WORD m_asl_dos_equivalent  		   : 1; // bit1 // not used?
            WORD m_names_initialized	       : 1; // bit2 //used
            WORD m_includes_comments		   : 1; // bit3 // used
            WORD m_includes_multi_site		   : 1; // bit4 // not used?
            WORD m_includes_per_test_stats	   : 1; // bit5 // not used?
            WORD m_includes_time_of_last_test  : 1; // bit6 // used
            WORD m_includes_hardware_bins      : 1; // bit7 // not used?
            WORD m_includes_display_multi      : 1; // bit8 // not used?
            WORD m_includes_last_hundred_yield : 1; // bit9 // not used?
            WORD m_includes_per_test_statistics_passed_devices_only : 1;// bit10 // not used?
            WORD m_includes_last_hundred_yield_qa_retest : 1;// bit11 // not used?
            WORD m_includes_flags : 1; // bit12 // not used?
            WORD m_includes_qa : 1; // bit13 // used
            WORD m_reserved : 1;    // JRC -  i think this should be two.
        };
    };


//CHANGES BEGIN 17 JULY 2006
    //goganesy 	FREmn10312
    //struct{ // this structure will be loaded and save into file if m_reserved equal to 1

        //1 = added 99 SW bin support
        //2 = added preservation of user/computer name
        //3 = added preservation of program path
        //4 = added preservation of handler name
        int m_AddinVersion; // use this field to define what to read from or save to file

        int m_Used_SW_Bins_count; // by default should be 32 to support old files

        STRING m_OperatorName; //preserve the operator name for loaded dl4's
        STRING m_ComputerName; //preserve the computer name for loaded dl4's
        STRING m_ProgramPath;  //preserve the program path for loaded dl4's
        STRING m_HandlerName;  //preserve the handler name for loaded dl4's

        // goganesy - add new fields here, increment  m_AddinVersion
        //			  and change GetStorageSizeBytes, SaveToBuffer, LoadFromBuffer functions
    //};//goganesy 	FREmn10312
//CHANGES END 17 JULY 2006

    STRING m_name;
    DL4_SYSTEMTIME m_time;
    time_t m_time_of_last_test;

public:
    short m_bins_n;
    short m_sites_n;

private:
    STRING *m_bin_name;
    STRING *m_hardware_bin;
    BOOL m_display_hardware_bins;
    BOOL m_display_multi_site;
    BOOL m_per_test_statistics_passed_devices_only;

public:
    gsint32 *m_bin_count;
    gsint32 *m_bin_count_qa;
    gsint32 *m_multi_site_count;
    BOOL m_is_qa_retest_view;

private:
    STRING m_lot_id_name;
    STRING m_sublot_id_name;

public:
    short m_good_devices_in_last_hundred;
    short m_good_devices_in_last_hundred_qa_retest;
    BOOL *m_last_hundred_array;
    short m_pointer_to_last_hundred_array;
    short *m_good_devices_in_last_hundred_by_site;
    BOOL **m_last_hundred_array_by_site;

public:
    short *m_pointer_to_last_hundred_array_by_site;

protected:
    TEST_TIME_STATS *m_test_time_stats;

    gsint32 m_passed_wafer_no;
    gsint32 m_summary_passed;
    gsint32 m_summary_failed;

    gsint32 m_last_device_num;
    gsint32 m_last_serial_num;

public:
    BYTE  m_last_bin_num;

public:
    gsint32 m_start_rec_in_dlog;

protected:
    gsint32 m_end_rec_in_dlog;


    PtrSet<STRING,short> m_comments;

    PtrSet<PER_TEST_STAT> m_per_test_stats;


public:

    gsint32 m_total_pass;
    gsint32 m_total_fail;

    gsint32 m_total_pass_qa_retest;
    gsint32 m_total_fail_qa_retest;

    gsint32 m_DLOG_INFO_INX;
    short site_count;

    union
    {
        gsint32 m_flags;
        struct
        {
            gsint32 m_wafer_testing : 1;
            gsint32 m_sublot_id_name_included : 1;
            gsint32 m_flags_reserved : 30;
        };
    };
};

class PROGRAM_FUNCTIONS_DATA : public StoredObject
{
public:

    PROGRAM_FUNCTIONS_DATA (void);
    ~PROGRAM_FUNCTIONS_DATA (void);

    gsint32 GetStorageSizeBytes (void);
    gsint32 LoadFromBuffer (const void *);

    short FunctionsN();

    ONE_FUNCTION *Function (const short func_i);

    gsint32 GetCumulativeSubTestsN (void);


private:

    short m_functions_n;

    union
    {
        gsint32 m_status;
        struct
        {
            gsint32 m_use_dll_time : 1;
            gsint32 m_use_prg_time : 1;
            gsint32 m_use_lst_time : 1;
            gsint32 m_use_src_time : 1;
            // ...
        };
    };

    gsint32 m_dll_time;
    gsint32 m_prg_time;
    gsint32 m_lst_time;
    gsint32 m_src_time;

    gsint32 m_reserved[15];

    ONE_FUNCTION **m_functions;
    gsint32 m_cumulative_subtests_n;
};

class ONE_FUNCTION : public StoredObject
{
public:

    ONE_FUNCTION (void);
    ~ONE_FUNCTION (void);

    gsint32 GetStorageSizeBytes (void);
    gsint32 LoadFromBuffer (const void *);

    short SubTestsN();

    FUNCTION_SUBTEST_DATA *SubTestData (const short subtest_i);

public:
    PROP_STRING_32 m_function_name;
    STRING m_original_name;//mh 2/24/00 Not persistent, used in Edit list to associate functions and source code files.
    BOOL m_bStubbed;//mh 2/29/00 Not persistent, used in Edit list to track that user has selected "no code body" at least once.
private:

    PROP_STRING_32 m_function_c_lang_name;

    short m_params_n;
    FUNCTION_PARAM_DATA *m_param_data;

    short m_subtests_n;
    FUNCTION_SUBTEST_DATA *m_subtest_data;

    gsint32 m_reserved[10];
};


class FUNCTION_SUBTEST_DATA : public StoredObject
{
public:
    enum {LIMITS_OK, BAD_MIN,BAD_MAX,MIN_GTE_MAX};

    inline FUNCTION_SUBTEST_DATA (void)
    {
        m_options0 = 0;
        m_reserved0 = 0;
        m_power = POWER_UNIT;
        MemSet (m_reserved_block,0,sizeof(m_reserved_block));
        m_subtest_name << "NEW_TEST";
        m_iSignature = 1;
        m_TST_TYP = 'P'; // TEST_TYPE_PARAMETRIC
    }

    inline ~FUNCTION_SUBTEST_DATA (void) { return; }

    gsint32 GetStorageSizeBytes (void);
    gsint32 LoadFromBuffer (const void *);

    inline BOOL operator < (FUNCTION_SUBTEST_DATA *fsd)
    {
        return IsLessThan (m_subtest_name,fsd->m_subtest_name,true);
    }

    inline BOOL operator > (FUNCTION_SUBTEST_DATA *fsd)
    {
        return IsGreaterThan (m_subtest_name,fsd->m_subtest_name,true);
    }

private:
// Fix me: remove this variable
//    BYTE m_above_storage_space;

public:

    PROP_STRING_16  m_subtest_name;

    PROP_SWITCH     m_max_limit_active;
    PROP_SWITCH     m_min_limit_active;

    PROP_SWITCH     m_limits_type_float;
    PROP_SWITCH     m_limits_type_ushort;

    PROP_FLOAT      m_max_limit[LIMITS_ARRAY_SIZE];
    PROP_FLOAT      m_min_limit[LIMITS_ARRAY_SIZE];

    PROP_STRING_8  m_units;
    PROP_STRING_16 m_note;
    PROP_STRING_80 m_pretest_comment;

    PROP_SWITCH m_display_results;

    PROP_CHAR	m_power;

    union
    {
        gsint32 m_options0;
        struct
        {
            gsint32 m_units_read_only		: 1; // Default is units dialog
            gsint32 m_units_direct_edit	: 1;
        };
    };

    BYTE m_reserved0;

    PROP_SWITCH m_delta_limit_active;

private:

    PROP_SWITCH m_reserved[4];

public:

    PROP_FLOAT m_delta_limit;

    PROP_STRING_16  m_subtest_user_number;
    int  m_iSignature;
    char m_TST_TYP;

private:

    BYTE m_reserved_block[128 - 8 - 20 - sizeof(int) - sizeof(char)];
//    BYTE m_reserved_block[128 - 8 /*sizeof(PROP_FLOAT)*/ - 20 /*sizeof(PROP_STRING_16)*/];

    // Fix me: unused variable
//    BYTE m_below_storage_space;
};

class ResultsTypeID : public StoredObject
{
public:

    ResultsTypeID (void) { return; }
    ~ResultsTypeID (void) { return; }

    inline gsint32 GetStorageSizeBytes (void)
    {
        gsint32 n = sizeof(m_reserved);
        n += m_results_name.GetStorageSizeBytes ();
        return n;
    }

    inline gsint32 LoadFromBuffer (const void *buf)
    {
        const BYTE *b = (const BYTE *) buf;
        gsint32 i=0;

        i += ReadByte(b+i, &m_reserved);

        i += m_results_name.LoadFromBuffer (b+i);
        return i;
    }

    inline BOOL operator < (ResultsTypeID &rti)
    {
        return (::IsLessThan (m_results_name,rti.m_results_name));
    }

    inline BOOL operator > (ResultsTypeID &rti)
    {
        return (::IsGreaterThan (m_results_name,rti.m_results_name));
    }

    BYTE   m_reserved;
    STRING m_results_name;
};

class FUNCTION_PARAM_VALUE : StoredObject
{
public:

    FUNCTION_PARAM_VALUE (void);
    ~FUNCTION_PARAM_VALUE (void);

    gsint32 GetStorageSizeBytes (void);
    gsint32 LoadFromBuffer (const void *buffer);

    BOOL SetValue (const void *, BYTE type);

    BOOL operator == (FUNCTION_PARAM_VALUE &);
    operator const char * ();

private:

    PROP_BYTE m_type;
    union
    {
        PROP_DOUBLE		*m_double;
        PROP_FLOAT		*m_float;
        PROP_SHORT		*m_short;
        PROP_WORD		*m_word;
        PROP_CHAR		*m_char;
        PROP_STRING_128 *m_string_128;
        void *m_address_of_data;
    };

    void clear (void);

public:
    // Power to use to display the parameter value
    PROP_BYTE m_power;
};

class FUNCTION_PARAM_DATA : public StoredObject
{
public:
    FUNCTION_PARAM_DATA (void);
    ~FUNCTION_PARAM_DATA (void);

    void Assign (FUNCTION_PARAM_DATA &);
    BOOL Assign (develop_parameter &, BOOL use_dos_name=false);

    gsint32 GetStorageSizeBytes (void);
    gsint32 LoadFromBuffer (const void *);

public:

    PROP_STRING_32 m_name;
    PROP_DOUBLE    m_multiplier;
    PROP_STRING_8  m_units;
    PROP_STRING_32 m_c_name;

    union
    {
        BYTE m_options0;
        struct
        {
            BYTE m_units_read_only		: 1; // Default is units dialog
            BYTE m_units_direct_edit	: 1;
        };
    };

    FUNCTION_PARAM_VALUE m_value;
};


struct dlog_test
{
    char					test_name[16];
    char                    user_test_number[16];
    union 	{
        float				f_measured_value;
        unsigned short		u_measured_value;
            };
    unsigned char			passed_fail;
    char					max_limit_active;
    char					min_limit_active;
    union	{
        float				f_max_limit_val[MAX_PASS_BINS];
        unsigned short		u_max_limit_val[MAX_PASS_BINS];
            };
    union	{
        float				f_min_limit_val[MAX_PASS_BINS];
        unsigned short		u_min_limit_val[MAX_PASS_BINS];
            };
    char			units[7];
    char			note[13];
    void			(*actual_test)();
    short			bin;
    short			sub_test_no;
    char			display_results;
    char			results_valid;
    char			power;
    // The following string has to be the last in the structure since when
    // loading a limits file it must not copied, and it is simpler if I can
    // take the size of the structure and subtract the length of
    // pretest_comment.
    char			pretest_comment[80];
};


class DLOG_DB_FLD : public StoredObject
{
public:
    DLOG_DB_FLD (gsint32 prg_info_idx = 0);
    ~DLOG_DB_FLD (void);

    gsint32 GetProgramInfoIdx (void);

    gsint32 GetStorageSizeBytes (void);
    gsint32 LoadFromBuffer (const void *);

private:

    gsint32 m_prg_info_idx;
};

class DLOG_DB_VLD : public StoredObject
{
public:

    DLOG_DB_VLD (PROGRAM_FUNCTIONS_DATA *);
    ~DLOG_DB_VLD (void);

    gsint32 GetStorageSizeBytes (void);
    gsint32 LoadFromBuffer (const void *);

    const test_header &Get_test_header(void);

    inline gsint32 Get_dlog_entry_array_n (void) { return m_dlog_entry_array_n; }
    const dlog_entry *Get_dlog_entry_array (void);
    DATALOG_VAR_LEN_DATA_PER_SUBTEST *Get_DATALOG_VAR_LEN_DATA_PER_SUBTEST (gsint32 subtest_i);

    BYTE GetSiteI (void);

    void Clear (void);

    BOOL ContainsData (void);


private:

    PROGRAM_FUNCTIONS_DATA *m_pfd;
    TestHeader m_test_header;
    dlog_entry *m_dlog_entry_array;
    gsint32 m_dlog_entry_array_n;
    DATALOG_VAR_LEN_DATA_PER_SUBTEST *m_dlog_entry_var_len_data;
    union
    {
        BYTE m_status01;

        struct
        {
            BYTE m_ex_status_exists : 1;
            BYTE m_site_i           : 7;
        };
    };

    union
    {
        gsint32 m_ex_status;

        struct
        {
            gsint32 m_results_type_array_exists : 1;
            gsint32 m_ex_status_reserved        : 31;
        };
    };

    PtrSet<ResultsTypeID,short> *m_results_type_array;
};


class DATALOG_VAR_LEN_DATA_PER_SUBTEST : public StoredObject
{
public:

    DATALOG_VAR_LEN_DATA_PER_SUBTEST (void);
    ~DATALOG_VAR_LEN_DATA_PER_SUBTEST (void);

    gsint32 GetStorageSizeBytes (void);
    gsint32 LoadFromBuffer (const void *buffer);

    inline TestLimits *GetTestLimitsObj (void)
    {
        return m_test_limits;
    }

private:

    struct
    {
        BYTE pretest_comment         : 1;
        BYTE includes_test_limit_obj : 1;
        BYTE posttest_comment        : 1;
        BYTE datalog_note            : 1;
        BYTE includes_plot_data		 : 1;
        BYTE reserved : 3;
    } m_included_data;

    short m_pretest_comment_len;
    char *m_pretest_comment;

    short m_posttest_comment_len;
    char *m_posttest_comment;

    short m_datalog_note_len;
    char *m_datalog_note;

    // This is used when test limits are overridden
    TestLimits *m_test_limits;
// CHANGES BEGIN 28 JUNE 2006
    DLogPlotData *m_plot_data;
// CHANGES END 28 JUNE 2006
};


class TestLimits : public StoredObject
{
public:

    inline TestLimits (void)
    {
        zero_members ();
    }


    inline TestLimits (const dlog_test *dt, gsint32 zero_based_test_no)
    {
        zero_members ();

        SetCumulativeTestNumber (zero_based_test_no);

        SetStatusMinLimitIsActive(dt->min_limit_active);
        SetStatusMaxLimitIsActive(dt->max_limit_active);

        if (dt->power == POWER_HEX)
        {
            SetNumberOfMaxLimits (MAX_PASS_BINS, 1);
            SetNumberOfMinLimits (MAX_PASS_BINS, 1);
            for (int i=0; i<MAX_PASS_BINS; i++)
            {
                SetMaxLimitValue (i,dt->u_max_limit_val[i]);
                SetMinLimitValue (i,dt->u_min_limit_val[i]);
            }
        }
        else
        {
            SetNumberOfMaxLimits (MAX_PASS_BINS, 0);
            SetNumberOfMinLimits (MAX_PASS_BINS, 0);
            for (int i=0; i<MAX_PASS_BINS; i++)
            {
                SetMaxLimitValue (i,dt->f_max_limit_val[i]);
                SetMinLimitValue (i,dt->f_min_limit_val[i]);
            }
        }
    }

    inline ~TestLimits (void)
    {
        delete_max_limit_array ();
        delete_min_limit_array ();

        m_status = 0;
        m_cumulative_test_no = -1;
    }

    inline BOOL SetCumulativeTestNumber (gsint32 zero_based_test_no)
    {
        if (zero_based_test_no >= 0)
        {
            m_cumulative_test_no = zero_based_test_no;
            return true;
        }

        return false;
    }

    inline gsint32 GetCumulativeTestNumber (void)
    {
        return m_cumulative_test_no;
    }

    ///////////////////////////////////////////////////////////
    // limit_type = 0 - float /////////////////////////////////
    // limit_type = 1 - WORD  /////////////////////////////////
    inline BOOL SetNumberOfMaxLimits (short n, short limit_type=0)
    {
        if (n > 0)
        {
            delete_max_limit_array ();
            if (limit_type == 0)
            {
                m_max_limits_f = new float[n];
//				ASSERT(m_max_limits_f);
                if (m_max_limits_f)
                {
                    m_number_of_max_limits = n;
                    m_contains_max_limits_u2 = false;
                    m_contains_max_limits_f = true;
                    return true;
                }
            }

            if (limit_type == 1)
            {
                m_max_limits_u2 = new WORD[n];
//				ASSERT(m_max_limits_u2);
                if (m_max_limits_u2)
                {
                    m_number_of_max_limits = n;
                    m_contains_max_limits_u2 = true;
                    m_contains_max_limits_f = false;
                    return true;
                }
            }
        }

        return false;
    }

    ///////////////////////////////////////////////////////////
    // limit_type = 0 - float /////////////////////////////////
    // limit_type = 1 - WORD  /////////////////////////////////
    inline BOOL SetNumberOfMinLimits (short n, short limit_type=0)
    {
        if (n > 0)
        {
            delete_min_limit_array ();
            if (limit_type == 0)
            {
                m_min_limits_f = new float[n];
//				ASSERT(m_min_limits_f);
                if (m_min_limits_f)
                {
                    m_number_of_min_limits = n;
                    m_contains_min_limits_u2 = false;
                    m_contains_min_limits_f = true;
                    return true;
                }
            }

            if (limit_type == 1)
            {
                m_min_limits_u2 = new WORD[n];
//				ASSERT(m_min_limits_u2);
                if (m_min_limits_u2)
                {
                    m_number_of_min_limits = n;
                    m_contains_min_limits_u2 = true;
                    m_contains_min_limits_f = false;
                    return true;
                }
            }
        }

        return false;
    }

    inline BOOL SetMaxLimitValue (short zero_based_idx, float val)
    {
        if (zero_based_idx < 0) return false;
        if (zero_based_idx >= m_number_of_max_limits) return false;
        if (!m_contains_max_limits_f) return false;
        if (!m_max_limits_f) return false;
        m_max_limits_f[zero_based_idx] = val;
        return true;
    }

    inline BOOL SetMinLimitValue (short zero_based_idx, float val)
    {
        if (zero_based_idx < 0) return false;
        if (zero_based_idx >= m_number_of_min_limits) return false;
        if (!m_contains_min_limits_f) return false;
        if (!m_min_limits_f) return false;
        m_min_limits_f[zero_based_idx] = val;
        return true;
    }

    inline BOOL SetMaxLimitValue (short zero_based_idx, WORD val)
    {
        if (zero_based_idx < 0) return false;
        if (zero_based_idx >= m_number_of_max_limits) return false;
        if (!m_contains_max_limits_u2) return false;
        if (!m_max_limits_u2) return false;
        m_max_limits_u2[zero_based_idx] = val;
        return true;
    }

    inline BOOL SetMinLimitValue (short zero_based_idx, WORD val)
    {
        if (zero_based_idx < 0) return false;
        if (zero_based_idx >= m_number_of_min_limits) return false;
        if (!m_contains_min_limits_u2) return false;
        if (!m_min_limits_u2) return false;
        m_min_limits_u2[zero_based_idx] = val;
        return true;
    }


    inline BOOL GetMaxLimitValue (short zero_based_idx, float &val)
    {
        if (zero_based_idx < 0) return false;
        if (zero_based_idx >= m_number_of_max_limits) return false;
        if (!m_contains_max_limits_f) return false;
        if (!m_max_limits_f) return false;
        val = m_max_limits_f[zero_based_idx];
        return true;
    }

    inline BOOL GetMinLimitValue (short zero_based_idx, float &val)
    {
        if (zero_based_idx < 0) return false;
        if (zero_based_idx >= m_number_of_min_limits) return false;
        if (!m_contains_min_limits_f) return false;
        if (!m_min_limits_f) return false;
        val = m_min_limits_f[zero_based_idx];
        return true;
    }


    //-------------------------------------


    inline void SetStatusMinLimitIsActive(char val)
    {
        m_min_limit_is_valid = val;
    }

    inline void SetStatusMaxLimitIsActive(char val)
    {
        m_max_limit_is_valid = val;
    }
    //-------------------------------------


    inline gsint32 GetStorageSizeBytes (void)
    {
        gsint32 i = 0;

        i += sizeof (m_status);
        i += sizeof (m_cumulative_test_no);
        i += sizeof (m_number_of_max_limits);
        i += sizeof (m_number_of_min_limits);

        if (m_contains_max_limits_f)
            i += sizeof (float) * m_number_of_max_limits;
        else if (m_contains_max_limits_u2)
            i += sizeof (WORD) * m_number_of_max_limits;

        if (m_contains_min_limits_f)
            i += sizeof (float) * m_number_of_min_limits;
        else if (m_contains_min_limits_u2)
            i += sizeof (WORD) * m_number_of_min_limits;

        //CHANGES BEGIN 27 June 2006
        //i += sizeof (m_max_limit_is_valid);
        //i += sizeof (m_min_limit_is_valid);

        return i;
    }

    inline gsint32 LoadFromBuffer (const void *buffer)
    {
        delete_max_limit_array ();
        delete_min_limit_array ();

        const BYTE *b = (const BYTE *) buffer;
        gsint32 i = 0;

        i += ReadWord(b+i,  (short*)&m_status);
        i += ReadDword(b+i, &m_cumulative_test_no);
        i += ReadWord(b+i, &m_number_of_max_limits);
        i += ReadWord(b+i, &m_number_of_min_limits);

        if (m_number_of_max_limits)
        {
            if (m_contains_max_limits_f)
            {
                m_max_limits_f = new float[m_number_of_max_limits];
//				ASSERT(m_max_limits_f);
                int iIndex;
                for(iIndex=0 ; iIndex<m_number_of_max_limits; iIndex++)
                    i += ReadFloat(b+i, &m_max_limits_f[iIndex]);
            }
            else if (m_contains_max_limits_u2)
            {
                m_max_limits_u2 = new WORD[m_number_of_max_limits];
//				ASSERT(m_max_limits_u2);
                int iIndex;
                for(iIndex=0 ; iIndex<m_number_of_max_limits; iIndex++)
                    i += ReadWord(b+i, (short*)&m_max_limits_u2[iIndex]);
            }
        }

        if (m_number_of_min_limits)
        {
            if (m_contains_min_limits_f)
            {
                m_min_limits_f = new float[m_number_of_min_limits];
//				ASSERT(m_min_limits_f);
                int iIndex;
                for(iIndex=0 ; iIndex<m_number_of_min_limits; iIndex++)
                    i += ReadFloat(b+i, &m_min_limits_f[iIndex]);
            }
            else if (m_contains_min_limits_u2)
            {
                m_min_limits_u2 = new WORD[m_number_of_min_limits];
//				ASSERT(m_min_limits_u2);
                for(int iIndex=0 ; iIndex<m_number_of_min_limits; iIndex++)
                    i += ReadWord(b+i, (short*)&m_min_limits_u2[iIndex]);
           }
        }

        //CHANGES BEGIN 27 June 2006
        //i += ReadByte(b+i, (BYTE*)&m_max_limit_is_valid);
        //i += ReadByte(b+i, (BYTE*)&m_min_limit_is_valid);

        return i;
    }

    inline BOOL operator > (TestLimits &t)
    {
        return GetCumulativeTestNumber () > t.GetCumulativeTestNumber ();
    }

    inline BOOL operator < (TestLimits &t)
    {
        return GetCumulativeTestNumber () < t.GetCumulativeTestNumber ();
    }

    inline BOOL operator == (TestLimits &t)
    {
        return GetCumulativeTestNumber () == t.GetCumulativeTestNumber ();
    }

protected:

    inline void zero_members (void)
    {
        m_status = 0;
        m_cumulative_test_no = -1;
        m_number_of_max_limits = 0;
        m_number_of_min_limits = 0;
        m_max_limits_f = 0;
        m_min_limits_f = 0;

        m_max_limit_is_valid = 1;
        m_min_limit_is_valid = 1;

        //CHANGES BEGIN 27 June 2006
        m_RESERVED_FOR_NEW_INFO_BLOCK = 0;

    }

    inline void delete_max_limit_array (void)
    {
        if (!m_number_of_max_limits) return;

        if (m_contains_max_limits_f)
        {
            delete m_max_limits_f;
            m_max_limits_f = 0;
            m_contains_max_limits_f = 0;
            m_number_of_max_limits = 0;
            return;
        }

        if (m_contains_max_limits_u2)
        {
            delete m_max_limits_u2;
            m_max_limits_u2 = 0;
            m_contains_max_limits_u2 = 0;
            m_number_of_max_limits = 0;
            return;
        }
    }

    inline void delete_min_limit_array (void)
    {
        if (!m_number_of_min_limits) return;

        if (m_contains_min_limits_f)
        {
            delete m_min_limits_f;
            m_min_limits_f = 0;
            m_contains_min_limits_f = 0;
            m_number_of_min_limits = 0;
            return;
        }

        if (m_contains_min_limits_u2)
        {
            delete m_min_limits_u2;
            m_min_limits_u2 = 0;
            m_contains_min_limits_u2 = 0;
            m_number_of_min_limits = 0;
            return;
        }
    }

    union
    {
        WORD m_status;

        struct
        {
            WORD m_contains_max_limits_f : 1;
            WORD m_contains_min_limits_f : 1;
            WORD m_contains_max_limits_u2 : 1;
            WORD m_contains_min_limits_u2 : 1;

            //CHANGES BEGIN 27 June 2006
            WORD m_max_limit_is_valid			: 1;
            WORD m_min_limit_is_valid			: 1;
            WORD m_RESERVED_FOR_NEW_INFO_BLOCK   : 1;
        };
    };

    gsint32 m_cumulative_test_no;

    short m_number_of_max_limits;
    short m_number_of_min_limits;

    //CHANGES BEGIN 27 June 2006
    //char m_max_limit_is_valid;
    //char m_min_limit_is_valid;

    union
    {
        float *m_max_limits_f;
        WORD *m_max_limits_u2;
    };

    union
    {
        float *m_min_limits_f;
        WORD *m_min_limits_u2;
    };

};

class PER_TEST_STAT : public StoredObject
{
public:

    PER_TEST_STAT (short sites = MAX_SITE_IN_ASLHANDLER); //PR#599 (DDTS#FREmn07270) JR 4/4/02
    ~PER_TEST_STAT ();

    gsint32 GetStorageSizeBytes (void);
    gsint32 LoadFromBuffer (const void *v);

    gsint32 GetDeviceCount (short site_i);
    gsint32 GetFailureCount (short site_i);

protected:
    char   *m_power;
    double *m_max_value;
    double *m_min_value;
    gsint32   *m_device_count;
    double *m_sum_values;
    double *m_sq_sum_values;
    gsint32   *m_failure_count;
    gsint32   *m_fail_under_count;
    gsint32   *m_fail_over_count;
    gsuint32  *m_consecutive_fails_count;  // not persistent
    gsuint32  *m_consecutive_fails_count_by_bin;  // not persistent PR#579
    gsuint32  *m_consecutive_fails_count_by_bin_alarm_count;  // not persistent PR#579
    short   m_site_count;

    union
    {
        BYTE m_status;
        struct
        {
            BYTE m_status_bit0 : 1;
            BYTE m_status_bit1 : 1;
            BYTE m_status_bit2 : 1;
            BYTE m_status_bit3 : 1;
            BYTE m_status_bit4 : 1;
            BYTE m_status_bit5 : 1;
            BYTE m_status_bit6 : 1; // indicates changes added for IR, failed under and over
            BYTE m_status_bit7 : 1; // indicates version 4.6 and above; sites > 4
        };
    };
};


class TEST_TIME_STATS : public StoredObject
{
public:

    TEST_TIME_STATS (void);
    ~TEST_TIME_STATS (void);

    gsint32 GetStorageSizeBytes (void);
    gsint32 LoadFromBuffer (const void *v);

public:
    //////////////////////////////////////////////
    // Need all of this information in order to
    // be compatible with ASL for dos
    gsint32  m_average_test_time_usec;
    gsint32  m_max_test_time_usec;
    gsint32  m_min_test_time_usec;
    DL4_LARGE_INTEGER m_prog_run_time;
    gsint32  m_average_wait_for_handler_usec;
    gsint32  m_max_wait_for_handler_usec;
    gsint32  m_min_wait_for_handler_usec;
    DL4_LARGE_INTEGER  m_handler_down_time_usec;
};

// CHANGES BEGIN 28 JUNE 2006
class SO_BUFFER
{
public:
    inline SO_BUFFER (void)
    {
        m_buffer = 0;
        m_reserved = 0;
        m_buffer_size = 0;
    }

    inline ~SO_BUFFER (void)
    {
        if (m_buffer)
        {
            delete m_buffer;
            m_buffer = 0;
        }
    }

    inline gsint32 GetStorageSizeBytes (void)
    {
        gsint32 n = 0;
        n += sizeof (m_reserved);
        n += sizeof (m_buffer_size);
        n += m_buffer_size;
        return n;
    }

    inline gsint32 LoadFromBuffer (const void *v)
    {
        if (m_buffer)
        {
            delete m_buffer;
            m_buffer = 0;
            m_buffer_size = 0;
        }

        const BYTE *b = (BYTE *) v;

        gsint32 i=0;

        MemMove (&m_reserved,b+i,sizeof(m_reserved));
        i += sizeof(m_reserved);

        MemMove (&m_buffer_size,b+i,sizeof(m_buffer_size));
        i += sizeof(m_buffer_size);

        m_buffer = new BYTE[m_buffer_size];
        if (!m_buffer)
            return i;

        MemMove (m_buffer,b+i,m_buffer_size);
        i += m_buffer_size;

        return i;
    }

protected:

    BYTE m_reserved;
    gsint32 m_buffer_size;
    BYTE *m_buffer;
};

class OneAxisPlotData
{
public:
    inline OneAxisPlotData (void)
    {
        m_status = 0;
    }

    inline ~OneAxisPlotData (void)
    {
    }

    inline gsint32 GetStorageSizeBytes (void)
    {
        gsint32 n = sizeof (m_status);
        n += m_npts.GetStorageSizeBytes ();
        n += m_buffer.GetStorageSizeBytes ();
        n += m_data_name.GetStorageSizeBytes ();

        return n;
    }

    inline gsint32 LoadFromBuffer (const void *v)
    {
        BYTE *b = (BYTE *) v;
        gsint32 i=0;

        MemMove (&m_status,b+i,sizeof(m_status));
        i += sizeof(m_status);

        i += m_npts.LoadFromBuffer (b+i);
        i += m_buffer.LoadFromBuffer (b+i);
        i += m_data_name.LoadFromBuffer (b+i);

        return i;
    }

protected:

    union
    {
        gsint32 m_status;

        struct
        {
            gsint32 m_reserved;
        };
    };

    SO_DWORD m_npts;
    SO_BUFFER m_buffer;
    STRING m_data_name;
};

class DLogPlotData
{
public:
    inline DLogPlotData (void)
    {
        m_status = 0;
        m_this_object_exists = true; // DO NOT CHANGE THIS - IT IS NECESSARY
        m_y_axes.SetStatusDeleteObjects (true);
    }

    inline ~DLogPlotData (void)
    {
    }

    inline gsint32 GetStorageSizeBytes (void)
    {
        gsint32 n = 0;
        n += sizeof(m_status);
        n += m_title.GetStorageSizeBytes ();
        n += m_x_axis_title.GetStorageSizeBytes ();
        n += m_y_axis_title.GetStorageSizeBytes ();
        n += m_comment.GetStorageSizeBytes ();
        n += m_window_ht_pixels.GetStorageSizeBytes ();
        n += m_window_wt_pixels.GetStorageSizeBytes ();
        n += m_plot_ht_pixels.GetStorageSizeBytes ();
        n += m_plot_wt_pixels.GetStorageSizeBytes ();
        n += m_x_axis.GetStorageSizeBytes ();
        n += m_y_axes.GetStorageSizeBytes ();
        return n;
    }

    inline gsint32 LoadFromBuffer (const void *v)
    {
        const BYTE *b = (const BYTE *) v;

        gsint32 i = 0;

        MemMove (&m_status,b+i,sizeof(m_status));
        i += sizeof(m_status);

        i += m_title.LoadFromBuffer (b+i);
        i += m_x_axis_title.LoadFromBuffer (b+i);
        i += m_y_axis_title.LoadFromBuffer (b+i);
        i += m_comment.LoadFromBuffer (b+i);
        i += m_window_ht_pixels.LoadFromBuffer (b+i);
        i += m_window_wt_pixels.LoadFromBuffer (b+i);
        i += m_plot_ht_pixels.LoadFromBuffer (b+i);
        i += m_plot_wt_pixels.LoadFromBuffer (b+i);
        i += m_x_axis.LoadFromBuffer (b+i);
        i += m_y_axes.LoadFromBuffer (b+i);

        return i;
    }

protected:

    union
    {
        gsint32 m_status;

        struct
        {
            gsint32 m_this_object_exists : 1;
            gsint32 m_reserved_1_thru_31 : 31;
        };
    };

    STRING m_title;

    STRING m_x_axis_title;
    STRING m_y_axis_title;

    STRING m_comment;

    SO_DWORD m_window_ht_pixels;
    SO_DWORD m_window_wt_pixels;

    SO_DWORD m_plot_ht_pixels;
    SO_DWORD m_plot_wt_pixels;

    OneAxisPlotData m_x_axis;
    PtrSet<OneAxisPlotData> m_y_axes;
};
// CHANGES END 28 JUNE 2006

///////////////////////////////////////// STDF_STRINGS /////////////////////////////////////////

#define ADTAG_STDF_STRINGS	"STDF_STRINGS"

//CHANGES BEGIN 28 June 2006
enum PRR_PART_ID_OPTIONS
{
    SERIAL_NUM_AND_SITE_NUM,
    SERIAL_NUM_ONLY
    //Add more options here...
};
// CHANGES END 28 JUNE 2006

class STDF_STRINGS : public StoredObject {

public:
    STDF_STRINGS(void) {
        Init();
    }
    ~STDF_STRINGS(void) {
        Free();
    }

    void Reset(BOOL counters_only) {
        if(counters_only)
            Init_Counters_Only();
        else
        {
            Free();
            Init();
        }
    }
    // ATR Record
    char *m_ATR_CMD_LINE;

    void Init_Counters_Only(void);

//CHANGES BEGIN 28 June 2006
    int m_PRR_PART_ID_OPTIONS;
// CHANGES END 28 JUNE 2006
    gsint32 GetStorageSizeBytes(void);
    gsint32 LoadFromBuffer(const void *);

private:
    void Init(void);
    void Free(void);
    gsint32 FormatStrings(char *);

private:
    // MIR Record
    unsigned char m_MIR_STAT_NUM;
    char m_MIR_MODE_COD;
    char m_MIR_RTST_COD;
    char m_MIR_PROT_COD;
    unsigned short m_MIR_BURN_TIM;
    char m_MIR_CMOD_COD;
    char *m_MIR_LOT_ID;
    char *m_MIR_PART_TYP;
    char *m_MIR_NODE_NAM;
    char *m_MIR_TSTR_TYP;
    char *m_MIR_JOB_NAM;
    char *m_MIR_JOB_REV;
    char *m_MIR_SBLOT_ID;
    char *m_MIR_OPER_NAM;
    char *m_MIR_EXEC_TYP;
    char *m_MIR_EXEC_VER;
    char *m_MIR_TEST_COD;
    char *m_MIR_TST_TEMP;
    char *m_MIR_USER_TXT;
    char *m_MIR_AUX_FILE;
    char *m_MIR_PKG_TYP;
    char *m_MIR_FAMLY_ID;
    char *m_MIR_DATE_COD;
    char *m_MIR_FACIL_ID;
    char *m_MIR_FLOOR_ID;
    char *m_MIR_PROC_ID;
    char *m_MIR_OPER_FRQ;
    char *m_MIR_SPEC_NAM;
    char *m_MIR_SPEC_VER;
    char *m_MIR_FLOW_ID;
    char *m_MIR_SETUP_ID;
    char *m_MIR_DSGN_REV;
    char *m_MIR_ENG_ID;
    char *m_MIR_ROM_COD;
    char *m_MIR_SERL_NUM;
    char *m_MIR_SUPR_NAM;

    // MRR Record
    char m_MRR_DISP_COD;
    char *m_MRR_USR_DESC;
    char *m_MRR_EXC_DESC;

    // PCR Record
    gsuint32 m_PCR_PART_CNT;
    gsuint32 m_PCR_RTST_CNT;
    gsuint32 m_PCR_ABRT_CNT;
    gsuint32 m_PCR_GOOD_CNT;
    gsuint32 m_PCR_FUNC_CNT;

    // SDR Record
    char *m_SDR_HAND_TYP;
    char *m_SDR_HAND_ID;
    char *m_SDR_CARD_TYP;
    char *m_SDR_CARD_ID;
    char *m_SDR_LOAD_TYP;
    char *m_SDR_LOAD_ID;
    char *m_SDR_DIB_TYP;
    char *m_SDR_DIB_ID;
    char *m_SDR_CABL_TYP;
    char *m_SDR_CABL_ID;
    char *m_SDR_CONT_TYP;
    char *m_SDR_CONT_ID;
    char *m_SDR_LASR_TYP;
    char *m_SDR_LASR_ID;
    char *m_SDR_EXTR_TYP;
    char *m_SDR_EXTR_ID;

    // WIR Record
    char *m_WIR_WAFER_ID;

    // WRR Record
    gsuint32 m_WRR_PART_CNT;
    gsuint32 m_WRR_RTST_CNT;
    gsuint32 m_WRR_ABRT_CNT;
    gsuint32 m_WRR_GOOD_CNT;
    gsuint32 m_WRR_FUNC_CNT;
    char *m_WRR_WAFER_ID;
    char *m_WRR_FABWF_ID;
    char *m_WRR_FRAME_ID;
    char *m_WRR_MASK_ID;
    char *m_WRR_USR_DESC;
    char *m_WRR_EXC_DESC;

    // WCR Record
    float m_WCR_WAFR_SIZ;
    float m_WCR_DIE_HT;
    float m_WCR_DIE_WID;
    unsigned char m_WCR_WF_UNITS;
    char m_WCR_WF_FLAT;
    short m_WCR_CENTER_X;
    short m_WCR_CENTER_Y;
    char m_WCR_POS_X;
    char m_WCR_POS_Y;

    // PTR Record
    char *m_PTR_C_RESFMT;
    char *m_PTR_C_LLMFMT;
    char *m_PTR_C_HLMFMT;

public:

    // ATR Get Functions
    char *Get_ATR_CMD_LINE(void);

    // ATR Set Functions
    void Set_ATR_CMD_LINE(char *value);

//CHANGES BEGIN 28 June 2006
    // PRR Get/Set Functions
    int Get_PRR_PART_ID_OPTIONS(void);
    void Set_PRR_PART_ID_OPTIONS(int val);
// CHANGES END 28 JUNE 2006

    // MIR Get Functions
    unsigned char Get_MIR_STAT_NUM(void);
    char Get_MIR_MODE_COD(void);
    char Get_MIR_RTST_COD(void);
    char Get_MIR_PROT_COD(void);
    unsigned short Get_MIR_BURN_TIM(void);
    char Get_MIR_CMOD_COD(void);
    char *Get_MIR_LOT_ID(void);
    char *Get_MIR_PART_TYP(void);
    char *Get_MIR_NODE_NAM(void);
    char *Get_MIR_TSTR_TYP(void);
    char *Get_MIR_JOB_NAM(void);
    char *Get_MIR_JOB_REV(void);
    char *Get_MIR_SBLOT_ID(void);
    char *Get_MIR_OPER_NAM(void);
    char *Get_MIR_EXEC_TYP(void);
    char *Get_MIR_EXEC_VER(void);
    char *Get_MIR_TEST_COD(void);
    char *Get_MIR_TST_TEMP(void);
    char *Get_MIR_USER_TXT(void);
    char *Get_MIR_AUX_FILE(void);
    char *Get_MIR_PKG_TYP(void);
    char *Get_MIR_FAMLY_ID(void);
    char *Get_MIR_DATE_COD(void);
    char *Get_MIR_FACIL_ID(void);
    char *Get_MIR_FLOOR_ID(void);
    char *Get_MIR_PROC_ID(void);
    char *Get_MIR_OPER_FRQ(void);
    char *Get_MIR_SPEC_NAM(void);
    char *Get_MIR_SPEC_VER(void);
    char *Get_MIR_FLOW_ID(void);
    char *Get_MIR_SETUP_ID(void);
    char *Get_MIR_DSGN_REV(void);
    char *Get_MIR_ENG_ID(void);
    char *Get_MIR_ROM_COD(void);
    char *Get_MIR_SERL_NUM(void);
    char *Get_MIR_SUPR_NAM(void);

    // MIR Set Functions
    void Set_MIR_STAT_NUM(unsigned char value);
    void Set_MIR_MODE_COD(char value);
    void Set_MIR_RTST_COD(char value);
    void Set_MIR_PROT_COD(char value);
    void Set_MIR_BURN_TIM(unsigned short value);
    void Set_MIR_CMOD_COD(char value);
    void Set_MIR_LOT_ID(char *value);
    void Set_MIR_PART_TYP(char *value);
    void Set_MIR_NODE_NAM(char *value);
    void Set_MIR_TSTR_TYP(char *value);
    void Set_MIR_JOB_NAM(char *value);
    void Set_MIR_JOB_REV(char *value);
    void Set_MIR_SBLOT_ID(char *value);
    void Set_MIR_OPER_NAM(char *value);
    void Set_MIR_EXEC_TYP(char *value);
    void Set_MIR_EXEC_VER(char *value);
    void Set_MIR_TEST_COD(char *value);
    void Set_MIR_TST_TEMP(char *value);
    void Set_MIR_USER_TXT(char *value);
    void Set_MIR_AUX_FILE(char *value);
    void Set_MIR_PKG_TYP(char *value);
    void Set_MIR_FAMLY_ID(char *value);
    void Set_MIR_DATE_COD(char *value);
    void Set_MIR_FACIL_ID(char *value);
    void Set_MIR_FLOOR_ID(char *value);
    void Set_MIR_PROC_ID(char *value);
    void Set_MIR_OPER_FRQ(char *value);
    void Set_MIR_SPEC_NAM(char *value);
    void Set_MIR_SPEC_VER(char *value);
    void Set_MIR_FLOW_ID(char *value);
    void Set_MIR_SETUP_ID(char *value);
    void Set_MIR_DSGN_REV(char *value);
    void Set_MIR_ENG_ID(char *value);
    void Set_MIR_ROM_COD(char *value);
    void Set_MIR_SERL_NUM(char *value);
    void Set_MIR_SUPR_NAM(char *value);

    // MRR Get Functions
    char Get_MRR_DISP_COD(void);
    char *Get_MRR_USR_DESC(void);
    char *Get_MRR_EXC_DESC(void);

    // MRR Set Functions
    void Set_MRR_DISP_COD(char value);
    void Set_MRR_USR_DESC(char *value);
    void Set_MRR_EXC_DESC(char *value);

    // PCR Get Functions
    gsuint32 Get_PCR_PART_CNT(void);
    gsuint32 Get_PCR_RTST_CNT(void);
    gsuint32 Get_PCR_ABRT_CNT(void);
    gsuint32 Get_PCR_GOOD_CNT(void);
    gsuint32 Get_PCR_FUNC_CNT(void);

    // PCR Set Functions
    void Set_PCR_PART_CNT(gsuint32 value);
    void Set_PCR_RTST_CNT(gsuint32 value);
    void Set_PCR_ABRT_CNT(gsuint32 value);
    void Set_PCR_GOOD_CNT(gsuint32 value);
    void Set_PCR_FUNC_CNT(gsuint32 value);

    // SDR Get Functions
    char *Get_SDR_HAND_TYP(void);
    char *Get_SDR_HAND_ID(void);
    char *Get_SDR_CARD_TYP(void);
    char *Get_SDR_CARD_ID(void);
    char *Get_SDR_LOAD_TYP(void);
    char *Get_SDR_LOAD_ID(void);
    char *Get_SDR_DIB_TYP(void);
    char *Get_SDR_DIB_ID(void);
    char *Get_SDR_CABL_TYP(void);
    char *Get_SDR_CABL_ID(void);
    char *Get_SDR_CONT_TYP(void);
    char *Get_SDR_CONT_ID(void);
    char *Get_SDR_LASR_TYP(void);
    char *Get_SDR_LASR_ID(void);
    char *Get_SDR_EXTR_TYP(void);
    char *Get_SDR_EXTR_ID(void);

    // SDR Set Functions
    void Set_SDR_HAND_TYP(char *value);
    void Set_SDR_HAND_ID(char *value);
    void Set_SDR_CARD_TYP(char *value);
    void Set_SDR_CARD_ID(char *value);
    void Set_SDR_LOAD_TYP(char *value);
    void Set_SDR_LOAD_ID(char *value);
    void Set_SDR_DIB_TYP(char *value);
    void Set_SDR_DIB_ID(char *value);
    void Set_SDR_CABL_TYP(char *value);
    void Set_SDR_CABL_ID(char *value);
    void Set_SDR_CONT_TYP(char *value);
    void Set_SDR_CONT_ID(char *value);
    void Set_SDR_LASR_TYP(char *value);
    void Set_SDR_LASR_ID(char *value);
    void Set_SDR_EXTR_TYP(char *value);
    void Set_SDR_EXTR_ID(char *value);

    // WIR Get Functions
    char *Get_WIR_WAFER_ID(void);

    // WIR Set Functions
    void Set_WIR_WAFER_ID(char *value);

    // WRR Get Functions
    gsuint32 Get_WRR_PART_CNT(void);
    gsuint32 Get_WRR_RTST_CNT(void);
    gsuint32 Get_WRR_ABRT_CNT(void);
    gsuint32 Get_WRR_GOOD_CNT(void);
    gsuint32 Get_WRR_FUNC_CNT(void);
    char *Get_WRR_WAFER_ID(void);
    char *Get_WRR_FABWF_ID(void);
    char *Get_WRR_FRAME_ID(void);
    char *Get_WRR_MASK_ID(void);
    char *Get_WRR_USR_DESC(void);
    char *Get_WRR_EXC_DESC(void);

    // WRR Set Functions
    void Set_WRR_PART_CNT(gsuint32 value);
    void Set_WRR_RTST_CNT(gsuint32 value);
    void Set_WRR_ABRT_CNT(gsuint32 value);
    void Set_WRR_GOOD_CNT(gsuint32 value);
    void Set_WRR_FUNC_CNT(gsuint32 value);
    void Set_WRR_WAFER_ID(char *value);
    void Set_WRR_FABWF_ID(char *value);
    void Set_WRR_FRAME_ID(char *value);
    void Set_WRR_MASK_ID(char *value);
    void Set_WRR_USR_DESC(char *value);
    void Set_WRR_EXC_DESC(char *value);

    // WCR Get Functions
    float Get_WCR_WAFR_SIZ(void);
    float Get_WCR_DIE_HT(void);
    float Get_WCR_DIE_WID(void);
    unsigned char Get_WCR_WF_UNITS(void);
    char Get_WCR_WF_FLAT(void);
    short Get_WCR_CENTER_X(void);
    short Get_WCR_CENTER_Y(void);
    char Get_WCR_POS_X(void);
    char Get_WCR_POS_Y(void);

    // WCR Set Functions
    void Set_WCR_WAFR_SIZ(float value);
    void Set_WCR_DIE_HT(float value);
    void Set_WCR_DIE_WID(float value);
    void Set_WCR_WF_UNITS(unsigned char value);
    void Set_WCR_WF_FLAT(char value);
    void Set_WCR_CENTER_X(short value);
    void Set_WCR_CENTER_Y(short value);
    void Set_WCR_POS_X(char value);
    void Set_WCR_POS_Y(char value);

    // PTR Get Functions
    char *Get_PTR_C_RESFMT(void);
    char *Get_PTR_C_LLMFMT(void);
    char *Get_PTR_C_HLMFMT(void);

    // PTR Set Functions
    void Set_PTR_C_RESFMT(char *value);
    void Set_PTR_C_LLMFMT(char *value);
    void Set_PTR_C_HLMFMT(char *value);
};

///////////////////////////////////////// STDF_STRINGS /////////////////////////////////////////

///////////////////////////////////////// STDF_SUPPLEMENT /////////////////////////////////////////
class DLOG_DB_HDR;

#define ADTAG_STDF_SUPPLEMENT	"STDF_SUPPLEMENT"

class STDF_SUPPLEMENT : public StoredObject
{
public:
    STDF_SUPPLEMENT (DLOG_DB_HDR *hdr);
    ~STDF_SUPPLEMENT (void);

    gsint32 GetStorageSizeBytes (void);
    // returns -1 on error
    gsint32 LoadFromBuffer (const void *v);

    // Generic Data
    union {
        gsuint32 m_flags;
        struct {
            gsuint32 m_includes_tsr_counts	: 1;
            gsuint32 m_reserved_flags		: 31;
        };
    };

    gsint32 m_Nsites;
    gsint32 m_Nsubtests;

    char m_reserved_data[512];

    // TSR Counts.  Need to save full counts to the dl4 file.  If doing
    // datalog sampling, subtest results will be missing and counts
    // cannot be determined from test data.
    gsuint32 *m_tsr_exec_cnts[MAX_SITE_IN_ASLHANDLER];
    gsuint32 *m_tsr_fail_cnts[MAX_SITE_IN_ASLHANDLER];
};

#endif // DL4_IMPORT

///////////////////////////////////////// STDF_SUPPLEMENT /////////////////////////////////////////
