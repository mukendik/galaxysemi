///////////////////////////////////////////////////////////
// Constants used in import functions
///////////////////////////////////////////////////////////
#ifndef GEX_IMPORT_CONSTANTS
#define	GEX_IMPORT_CONSTANTS        1

#include <QString>

// Constants used when writing data origin information into the MIR record
const QString GEX_IMPORT_DATAORIGIN_LABEL("[GALAXY_DATA_ORIGIN]");
const QString GEX_IMPORT_DATAORIGIN_ETEST("ETEST");
const QString GEX_IMPORT_DATAORIGIN_FTEST("FTEST");
const QString GEX_IMPORT_DATAORIGIN_ATETEST("ATETEST");

#define GEX_TEMPORARY_STDF	"_temp_gex"

// Test# offset for files with parameters specified without a test#!
#define	GEX_TESTNBR_OFFSET_PCM_TOWER        811000
#define	GEX_TESTNBR_OFFSET_FREESCALE        810000
#define	GEX_TESTNBR_OFFSET_SDI              809000
#define	GEX_TESTNBR_OFFSET_ETEST_FAB7       808000
#define	GEX_TESTNBR_OFFSET_HJTC             807000
#define	GEX_TESTNBR_OFFSET_WIF              806000
#define	GEX_TESTNBR_OFFSET_ACCO             805000
#define	GEX_TESTNBR_OFFSET_AMIDA            804000
#define	GEX_TESTNBR_OFFSET_SIPEX            803000
#define	GEX_TESTNBR_OFFSET_TESSERA          802000
#define	GEX_TESTNBR_OFFSET_SEQUOIA          801000
#define	GEX_TESTNBR_OFFSET_WAT_UMC          800000
#define	GEX_TESTNBR_OFFSET_PCM_DONGBU       799000
#define	GEX_TESTNBR_OFFSET_VERIGY_EDL       798000
#define	GEX_TESTNBR_OFFSET_EAGLE_DATALOG    797000
#define	GEX_TESTNBR_OFFSET_WAT_SMIC         796000
#define	GEX_TESTNBR_OFFSET_ASL1000          795000
#define	GEX_TESTNBR_OFFSET_WAT_ASMC         794000
#define	GEX_TESTNBR_OFFSET_PCM_X_FAB        793000
#define	GEX_TESTNBR_OFFSET_TERADYNE_IMAGE   792000
#define	GEX_TESTNBR_OFFSET_CSMC             791000
#define	GEX_TESTNBR_OFFSET_CSM              790000
#define	GEX_TESTNBR_OFFSET_WAT              789000
#define	GEX_TESTNBR_OFFSET_PCM              788000
#define	GEX_TESTNBR_OFFSET_CSV              787000

// Parameter files
#define GEX_FREESCALE_PARAMETERS            "/freescale_parameters.txt"
#define GEX_SDI_PARAMETERS                  "/sdi_parameters.txt"
#define GEX_ETEST_FAB7_PARAMETERS           "/etest_fab7_parameters.txt"
#define GEX_WIF_PARAMETERS                  "/wif_parameters.txt"
#define GEX_ACCO_PARAMETERS                 "/acco_parameters.txt"
#define GEX_AMIDA_PARAMETERS                "/amida_parameters.txt"
#define GEX_SIPEX_PARAMETERS                "/sipex_parameters.txt"
#define GEX_TESSERA_PARAMETERS              "/tessera_parameters.txt"
#define GEX_SEQUOIA_PARAMETERS              "/sequoia_parameters.txt"
#define GEX_ADVANTEST_T2000_STATS_PARAMETERS "/AdvantestT2000Stats_parameters.txt"
#define GEX_PCM_DONGBU_PARAMETERS           "/pcm_dongbu_parameters.txt"
#define GEX_YOKOGAWA_PARAMETERS             "/yokogawa_parameters.txt"
#define GEX_VERIGY_EDL_PARAMETERS           "/verigy_edl_parameters.txt"
#define GEX_EAGLE_DATALOG_PARAMETERS        "/eagle_datalog_parameters.txt"
#define GEX_WAT_SMIC_PARAMETERS             "/smic_wat_parameters.txt"
#define GEX_ASL1000_PARAMETERS              "/asl1000_parameters.txt"
#define GEX_CSM_PARAMETERS                  "/csm_parameters.txt"
#define GEX_CSM_TYPE_2_PARAMETERS           "/csm_type_2_parameters.txt"
#define GEX_CSMC_PARAMETERS                 "/csmc_parameters.txt"
#define GEX_WAT_PARAMETERS                  "/wat_parameters.txt"
#define GEX_WAT_UMC_PARAMETERS              "/wat_umc_parameters.txt"
#define GEX_PCM_PARAMETERS                  "/pcm_parameters.txt"
#define GEX_TERADYNE_IMAGE_PARAMETERS       "/teradyne_image_parameters.txt"
#define GEX_PCM_X_FAB_PARAMETERS            "/pcm_x_fab_parameters.txt"
#define GEX_WAT_ASMC_PARAMETERS             "/wat_asmc_parameters.txt"
#define GEX_CSV_PARAMETERS                  "/csv_parameters.txt"
#define GEX_GDF_PARAMETERS                  "/gdf_parameters.txt"
#define GEX_7C7_PARAMETERS                  "/7c7_parameters.txt"
#define GEX_PCM_HJTC_PARAMETERS             "/hjtc_parameters.txt"
#define GEX_PCM_TOWER_PARAMETERS            "/pcm_tower_parameters.txt"
#define GEX_TRI_INPHIPRESTO_PARAMETERS      "/inphipresto_parameters.txt"
#define GEX_TRI_INPHIBBA_PARAMETERS         "/inphibba_parameters.txt"
#define GEX_TRI_MICRO_PARAMETERS            "/micron_parameters.txt"
#define GEX_TRI_MICROSUM_PARAMETERS         "/microsum_parameters.txt"
#define GEX_SITIME_ETEST_PARAMETERS         "/sitime_parameters.txt"
#define GEX_TRI_QUINT_DC_PARAMETERS         "/tri_quintdc_parameters.txt"
#define GEX_TRI_QUINT_RF_PARAMETERS         "/tri_quintrf_parameters.txt"
#define GEX_WOBURN_PARAMETERS               "/woburn_parameters.txt"
#define GEX_SKYWORKS_CSV_PARAMETERS         "/skyworks_csv_parameters.txt"
#define GEX_VANGUARD_PCM_PARAMETERS         "/vanguard_pcm_parameters.txt"

#endif // GEX_IMPORT_CONSTANTS
