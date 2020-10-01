/* (C) Copyright 1996-2012 by Maxim Integrated Products

THIS SOFTWARE/COMPUTER CODE IS PROPRIETARY AND CANNOT BE USED OR DISCLOSED IN WHOLE OR IN PART
FOR ANY PURPOSE; WITHOUT THE EXPRESS WRITTEN CONSENT OF MAXIM INTEGRATED PRODUCTS.

//            MM   MM       AA  XX   XX  II       MM   MM
//           MMM  MMM      AAA   XX XX   II      MMM  MMM
//          MMMM MMMM     AAAA    XXX    II     MMMM MMMM
//         MM MMMM MM    AA AA    XXX    II    MM MMMM MM
//        MM  MMM  MM   AA  AA   XX XX   II   MM  MMM  MM
//       MM   MM   MM  AA  AAA  XX   XX  II  MM   MM   MM

/********************************************************************
  Version	  Date	    	Change		           Eng
---------------------------------------------------------------------
    1.0		 11/27/02      INITIAL RELEASE	           RWF
    1.02	 04/05/05      Function counter	           BRS
    1.05	 07/12/06      Auto save summary feature   GT MPOC
    1.10	 07/24/06      Added MVXML Code to file	   SBK	
    1.11     08/24/06      Cleaned up MVXML Code 	   SBK
    1.12	 02/02/07      Added Get_Current_Time function  SBK	
    1.2      03/27/07      Changed the path to production   
			               from lists for auto summary save  SBK
    1.3      03/10/07      Changes to popup box and HW Checker 
			               by D. Smith 	
   	1.5      10/22/07      DS made changes to the badge log tool.  SBK
	1.6		 01/8/08	   Michael Urhausen added average test time functions  MU
						   Darryl Smith made changes to the function counter code.
						   Javier Contreras made changes to standardize the open/short binning code.
	2.34     9-10-08       Changed rev for better control of ezrev compiles. 
	2.37    03-12-10	   Added Maxim DPAT library
	2.38    04-06-12       Changed the Thailand TMT server path
	2.39    04-16-12       Resolved an issue with the DOS Maxuser.h file SBK
	2.40    04-27-12       Changed made to the Thailand TMT server path in the LotInfoFromComets.h file
*********************************************************************/
#define VERSION 2.40   // rev 40 released 04-27-12

#define U_TMT30 //Uncomment this line for ASL 3.x software

#define ASL_NT

// Disable warning C4244 "conversion from 'const double' to 'float', possible loss of data
#pragma warning (disable : 4244 4305)

#ifdef USER_FILE
#undef USER_FILE
#define U_EXT
#else
#define U_EXT extern
#endif
#include "gage_form.h"	//grem
#ifdef MAXIM_DPAT
#include "dpat.h"		//PHH
#endif
#ifdef GALAXY_DPAT
#include "galaxy.h"
#endif
// Declare global variables

U_EXT int u_device_fail, u_debug_test;
U_EXT int u_device, u_testflow, u_temp, u_grade, u_option;
U_EXT int u_clock_delta_flag;
U_EXT float u_clock_previous;
extern int u_func_counter, u_num_func, u_ctr_enable;
extern float u_bin_stat[32];
U_EXT int u_bin_nt;
U_EXT char Badge_nt[50];//DGS 09/14/07

// Generic definitions

/*** Options for u_debug_test ******************************/

#define DEBUG_NONE  0
#define DEBUG_FAIL -1
#define DEBUG_ALL  -2
#define DEBUG_STOP -3

/*** Options when running program **************************/

#define HWSKIP    1
#define ALLTEST   2
#define CHARACT   4
#define TIMETESTS 8

                /*  Limits for Condition    u_testflow            */
                /*  --------------------    ------------------    */
#define                WAFER                    1
#define                FINAL                    2
#define                QA                       3

                /*  Temperature               u_temp              */
                /*  ------------------------  ------------------  */  
#define                ROOM                         1 
#define                COLD                         2
#define                HOT                          3
#define                HOTCOLD                      4
#define                ALL                          5

                /*  Grade                     u_grade              */
                /*  ------------------------  -------------------- */  
#define                COM                          1  
#define                IND                          2  
#define                EXT                          3  
#define                MIL                          4  


// Define some units that I may want to use ****************************

#define  V *1.0
#define mV *1.0E-3
#define  A *1.0
#define mA *1.0E-3
#define uA *1.0E-6
#define nA *1.0E-9
#define  S *1.0
#define mS *1.0E-3
#define uS *1.0E-6
#define nS *1.0E-9

// Type cast generic functions which are defined in user.c??
int u_test(test_function& func, float value, char unit, int fail_bin, int test_no, int site_num= 1);
void u_test_trap(int);
void u_zap_write_dac_v(Zap*, float);
void u_zap_link(Zap*, int, int, int);
void u_clock_time_tests();
void u_clock_delta();
void u_clock_cumulative();
void u_clock_start();
float u_clock_read();



float u_version();
void u_screen_save();
void u_screen_restore();
void u_ramp_v(Dvi* dvi, unsigned char dvi_channel, float v_start,
              float v_stop, int time, unsigned char v_range = 0,
              int t_step = 100, char v_compensation = FAST_VOLTAGE_MODE);
void u_ramp_i(Dvi* dvi, unsigned char dvi_channel, float i_start,
              float i_stop, int time, unsigned char i_range = 0,
              int t_step = 100, char i_half = FALSE);

void u_soft_connect( Dvi* dvi, int i, float f);
void u_soft_init( Dvi* dvi);
void u_func_check_count(void);
extern void u_check_bin_stat(char *msg, int bin, float flag_pct = 5.0, int num_devices = 100);

void AutoSaveSummary(void);
void MoveSaveSummary(void);


void SetHWCStatus(char *my_product_name, int status);
long GetHWCStatus(char *product_name);

extern short Message_dialog_nt(char *topmessage, char *bottommessage, char *fieldmessage);//jez, DGS 09/14/07
void LogOperatorBadge(char *msg, int bin, float flag_pct, int num_devices);//DGS 09/14/07

/*************************************/
void u_log_ttime(int skip_bin1 = 0, int skip_bin2 = 0, int num_tests = 100);
void u_start_ttime(void);
float u_update_avg_ttime(float time, int num_tests);
void u_clear_ttimes(void);
bool u_UpdateLotSummaryComment(STRING comment);
long u_GetSerialNum(void);
void u_SetSerialNum(long num);
/************************************/

/* Maxim User Executive Functions  Added to incorporate Galaxy and Maxim STDF driver*/
void maxim_user_init(void);
void maxim_user_load(void);
void maxim_user_start_lot(void);
void maxim_user_start_test(void);
void maxim_user_before_binning(void);
void maxim_user_end_test(void);
void maxim_user_next_device(void);
void maxim_user_end_lot(void);
void maxim_user_exit(void);
/*Maxim User Executive Functions*/
#ifdef GALAXY_DPAT
int asl_get_bin(const int site_no);
#endif