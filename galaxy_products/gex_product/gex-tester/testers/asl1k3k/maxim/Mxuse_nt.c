/* (C) Copyright 1996-2008 by Maxim Integrated Products

THIS SOFTWARE/COMPUTER CODE IS PROPRIETARY AND CANNOT BE USED OR DISCLOSED IN WHOLE OR IN PART
FOR ANY PURPOSE; WITHOUT THE EXPRESS WRITTEN CONSENT OF MAXIM INTEGRATED PRODUCTS.

//            MM   MM       AA  XX   XX  II       MM   MM
//           MMM  MMM      AAA   XX XX   II      MMM  MMM
//          MMMM MMMM     AAAA    XXX    II     MMMM MMMM
//         MM MMMM MM    AA AA    XXX    II    MM MMMM MM
//        MM  MMM  MM   AA  AA   XX XX   II   MM  MMM  MM
//       MM   MM   MM  AA  AAA  XX   XX  II  MM   MM   MM

/********************************************************************
  Version	  Date	    	Change		              Eng
---------------------------------------------------------------------
    1.0	      11/27/02      INITIAL RELEASE		      RWF
    1.01      12/21/04      Added soft connect functions      ??? 			
    1.05      07/12/06      Auto save summary feature         GT MPOC
    1.10      07/24/06	    Added MVXML Code to file	      SBK
    1.11      08/24/06	    Cleaned up MVXML Code 	      SBK
    1.12      02/02/07      Added Get_Current_Time function   SBK
    1.2	      04/13/07      Chande to autosave path and DOS 
			                NT conversion code changes.       SBK
    1.3	      05/10/07      Changes to popup box and HW Checker 
			                by D. Smith 	
	1.4		  08/23/07		Added dual binning in u_test FJ. Contreras
	1.5       10/22/07      DS made changes to the badge log tool.  SBK
	1.6		  01/8/08	    Michael Urhausen added average test time functions  MU
							Darryl Smith made changes to the function counter code.	
							Javier Contreras made changes to standardize the open/short binning code.
   	1.46      11-25-08      Version 1.46 Changes to the u_ramp code to prevent overshoots.
	2.37	03-12-10	Added Maxim DPAT library functions
********************************************************************/

#include <string.h>
#include "ModalDialogDescription.h" //jez, DGS 09/14/07
#include "gage_form.c"				//grem
#ifdef MAXIM_DPAT
#include "dpat.cpp"					//PHH
#endif
#ifdef GALAXY_DPAT
#include "galaxy.cpp"
#define MX_SITES 32
int u_bin_num[MX_SITES]; //keep track of current bin 
#endif

#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))

#ifdef Multisite_lib_h
#define MULTI_SITE
#endif

int u_func_counter, u_num_func, u_ctr_enable;
float u_bin_stat[32];
unsigned long start_Time;
Time_calc time_calc;

int u_time_tests = 0;

// ********************************************************************
// Following functions are fairly generic and will probably be used in
// several test functions.  They may even become a library one day.

//***************************************************************************

//** Used to datalog test results.

int u_test(test_function& func, float value, char unit, int fail_bin,
	   int test_no, int site_num)
{
  static char* second = "S";
  char string[20], message[50];
  int fail_flag = 0;
 
  
#ifdef U_TMT30
  func.dlog->set_test_no(test_no, site_num);
#else
  func.dlog->set_test_no(test_no);
#endif

  //Initialize the Bin Flag
  if((u_device_fail == FALSE) && (u_bin_nt != 1))
    u_bin_nt = 1; 

  if(u_time_tests)
    func.dlog->power = POWER_UNIT;
  else
    func.dlog->power = unit;
  func.dlog->test_val(value);

if(ag_correlate)		//grem
	agf_RunStoreValue(func, value, test_no);	//grem

  if(u_time_tests)
    {
      func.dlog->set_unit(second);
      func.dlog->power = POWER_UNIT;
      func.dlog->tests[test_no-1].f_measured_value = u_clock_read();
      func.dlog->set_bin(16);
    }

  if(func.dlog->tests[test_no-1].passed_fail == FAILED_TEST)
    {
		/////////////////////////////////////////////////////////////////
		//  Two contact bins assuming testing normal esd diode:
		//  "short" is fail_bin ,							   
		//  "open" is fail_bin-1 added 8/23/07 J. Contreras   rev 1-21-08
		/////////////////////////////////////////////////////////////////
		if(fail_bin < 0)
		{
		  fail_bin = fabs(fail_bin);
			if(fabs(value) > fabs(func.dlog->tests[test_no-1].f_min_limit_val[0]) ||
			   fabs(value) > fabs(func.dlog->tests[test_no-1].f_max_limit_val[0]))
			fail_bin = fail_bin - 1;
		}
		/////////////////////////////////////////////////////////////////
		//  Two contact bins assuming testing normal esd diode:
		//  "short" is fail_bin ,							   
		//  "open" is fail_bin-1 added 8/23/07 J. Contreras   rev 1-21-08
		/////////////////////////////////////////////////////////////////
      func.dlog->set_bin(fail_bin);
      fail_flag = 1;
	  //On the First Failure ONLY, set the Bin Flag
	  if(u_device_fail == FALSE)
		{
		  u_bin_nt = fail_bin; 
#ifdef GALAXY_DPAT
		  u_bin_num[site_num-1]=fail_bin;
#endif
		}
      u_device_fail = TRUE;
    }

  /////////////////////////////////////////////////////////////////////
  //   MVXML added 7-24-06
  ///////////////////////////////////////////////////////////////////////
#ifdef _MVXML_H_
  /* Maxim programs must used site_num-1 (Dallas just uses site_num) */
  mvxml_dlog(func, test_no, value, fail_bin, site_num-1,
	     NULL, //func.dlog->datalog_notes(notes), /* is this correct? */
	     func.dlog->power);
#endif

  /////////////////////////////////////////////////////////////////////
  //   MVXML added 7-24-06
  ///////////////////////////////////////////////////////////////////////

  if(func.dlog->tests[test_no-1].display_results) // check for datalog on
    func.dlog->display_results();

  if(( u_debug_test == test_no)                    ||
     ( u_debug_test == DEBUG_ALL)                  ||
     ((u_debug_test == DEBUG_FAIL) && fail_flag))
    {
      strcpy(message, func.dlog->tests[test_no-1].test_name);
      strcat(message, "          Value measured: ");
      sprintf(string, "%g", value);
      strcat(message, string);
    }

//////////////////////////////////////////////////////////////////////////
  //DPAT added 3-1-2010
  ////////////////////////////////////////////////////////////////////////
#ifdef MAXIM_DPAT
	DPAT_data(func, value, unit, test_no, site_num);
#endif
//Galaxy DPAT, Need to add enable switch somewhere)
#ifdef GALAXY_DPAT
	galaxy_data(func,site_num-1,func.dlog->test_offset*1000+test_no, func.dlog->tests[test_no-1].test_name, value);
#endif

  return(fail_flag);
}

//***************************************************************************

//** Returns the current version of maxtools
float u_version()
{
  return VERSION;
}

//***************************************************************************

//** Used to set the test trap variable

void u_test_trap(int test_trap)
{
  u_debug_test = test_trap;
}

/**************************************************************/
/*** Cause the time to be returned as a cumulative time from the
  clock_start call.
  -- Brad Sitton 1/14/97 */

void u_clock_cumulative()
{
  u_clock_delta_flag = 0;
}

/**************************************************************/
/*** Cause the time to be returned as a delta from the previous
  clock call
  -- Brad Sitton 1/14/97 */

void u_clock_delta()
{
  u_clock_delta_flag = 1;
}

/**************************************************************/
/*** Read the clock and return time in milliseconds
  -- Brad Sitton 11/16/95 */

float u_clock_read()
{

  float time;					// In micro secs.
  const float fudge = 1050;		// There is a 1.05 milli sec error in time

  time = time_calc.stop_and_calculate();
  time = time - fudge;
  time = time/1000000;

  return (time);

}

/**************************************************************/
/*** Resets the clock to 0 and starts it running
  -- Brad Sitton 11/16/95 */

void u_clock_start()
{

  // Insert call that will read the start time of the clock.
  // Insert new clock reading function here
  //Going to have to do some processing in order to use the time.

  time_calc.start();

}

/**************************************************************/
/*** Cause the time to be data logged each time u_test is called
  -- Brad Sitton 11/16/95 */

void u_clock_time_tests()
{
  u_clock_start();
  u_time_tests = 1;
}

// ********************************************************************
// Program the DAC on the zap card to the specified voltage
// -- Brad Sitton 8/15/96

void u_zap_write_dac_v(Zap* zap, float v_dac)
{
  zap->write_dac(0, (unsigned int) (v_dac/(52.0/65535.0)));
}

// ********************************************************************
// Ramp the current on a DVI for a specified amount of time.
//   -- Brad Sitton 11/26/96

void u_ramp_i(Dvi* dvi, unsigned char dvi_channel, float i_start,
              float i_stop, int time, unsigned char i_range, int t_step,
              char i_half)
{
  float i_dvi, i_step, sign;

  // Only use the user specified range if it is large enough to
  //   accommodate start and stop voltage

  if(fabs(max(i_start, i_stop)) < 20.0E-6)
    i_range= max(i_range, MICRO_20_AMP);
  else if(fabs(max(i_start, i_stop)) < 200.0E-6)
    i_range= max(i_range, MICRO_200_AMP);
  else if(fabs(max(i_start, i_stop)) < 2.0E-3)
    i_range= max(i_range, MILLI_2_AMP);
  else if(fabs(max(i_start, i_stop)) < 20.0E-3)
    i_range= max(i_range, MILLI_20_AMP);
  else if(fabs(max(i_start, i_stop)) < 200.0E-3)
    i_range= max(i_range, MILLI_200_AMP);

  time /=10;
  t_step /=10;
  if(t_step < 1) t_step = 1;  // Minimum allowed delay is 10 uS
  //   which is 1 step

  i_step = (float) (i_stop - i_start) * t_step / time;
  i_dvi = i_start;
  sign = (i_step > 0.0) ? 1.0 : -1.0;
  i_step = sign*max( fabs(i_step), 1.0e-6);
  // extremely small values of i_step (e.g. 1e-7) can cause infinite loop

  while(sign*i_dvi <= sign*i_stop)
    {
      i_dvi += i_step;
      dvi->set_current(dvi_channel, i_dvi, i_range, i_half);
      wait.delay_10_us(t_step);
    }

  dvi->set_current(dvi_channel, i_stop, i_range, i_half);

}

// ********************************************************************
// Ramp the voltage on a DVI for a specified amount of time.
//   -- Brad Sitton 11/26/96

void u_ramp_v(Dvi* dvi, unsigned char dvi_channel, float v_start,
              float v_stop, int time, unsigned char v_range, int t_step,
              char v_compensation)
{
  float v_dvi, v_step, sign;

  // Only use the user specified range if it is large enough to
  //   accommodate start and stop voltage
  if(max(v_start, v_stop) <  1.0) v_range= max(v_range, VOLT_1_RANGE);
  else if(max(v_start, v_stop) <  2.0) v_range= max(v_range, VOLT_2_RANGE);
  else if(max(v_start, v_stop) <  5.0) v_range= max(v_range, VOLT_5_RANGE);
  else if(max(v_start, v_stop) < 10.0) v_range= max(v_range, VOLT_10_RANGE);
  else if(max(v_start, v_stop) < 20.0) v_range= max(v_range, VOLT_20_RANGE);
  else if(max(v_start, v_stop) < 50.0) v_range= max(v_range, VOLT_50_RANGE);

  time /=10;
  t_step /=10;
  if(t_step < 1) t_step = 1;  // Minimum allowed delay is 10 uS
  //   which is 1 step

  v_step = (float) (v_stop - v_start) * t_step / time;
  v_dvi = v_start;
  sign = (v_step > 0.0) ? 1.0 : -1.0;
  v_step = sign*max( fabs(v_step), 0.125e-3);

  // execute ramp only if start and stop are different
  // extremely small values of v_step (e.g. 1e-7) can cause infinite loop
  while(sign*v_dvi < sign*v_stop)
    {
      v_dvi += v_step;

	if(sign*v_dvi > sign*v_stop)
	{
		v_dvi = v_stop;
	}

      dvi->set_voltage(dvi_channel, v_dvi, v_range, v_compensation);
      wait.delay_10_us(t_step);
    }

  dvi->set_voltage(dvi_channel, v_stop, v_range, v_compensation);

}

// ********************************************************************
// Zap the specified link.  The relay that is listed first will be
//   connected to positive voltage, the other relay will go to ground.
//   -- Brad Sitton 8/15/96

void u_zap_link(Zap* zap, int relay1, int relay2, int zap_time)
{
  if(relay1%2 == relay2%2)  return;  // One must be even, one must be odd

  if(relay1%2 == 1)  zap->open_relay(RET_BUS_A);
  else               zap->close_relay(RET_BUS_A);

  zap->close_relay(relay1);
  zap->close_relay(relay2);
  delay(10);

  zap->close_relay(ZAP_FUSE);
  delay(zap_time);
  zap->open_relay(ZAP_FUSE);
  delay(2);

  zap->open_relay(relay1);
  zap->open_relay(relay2);

  zap->open_relay(RET_BUS_A);  // Always leave with odd relays connected
  //   to positive voltage
  return;
}

/*************************************************************/
/*** Initialize the DVI with minimized glitches due that occur
  when the force relay gets closed.
  -- Brad Sitton 11/15/04  */

void u_soft_init(Dvi* dvi)
{
  dvi->open_relay(CONN_FORCE0);
  dvi->open_relay(CONN_FORCE1);
  wait.delay_10_us(10);
  dvi->open_relay(CONN_SENSE0);
  dvi->open_relay(CONN_SENSE1);
  wait.delay_10_us(10);
  dvi->close_relay(BUS_SENSE0);
  dvi->close_relay(BUS_FORCE0);
  dvi->close_relay(BUS_SENSE1);
  dvi->close_relay(BUS_FORCE1);
  delay(1);
  dvi->init();
  delay(1);
  dvi->open_relay(BUS_FORCE0);
  dvi->open_relay(BUS_SENSE0);
  dvi->open_relay(BUS_FORCE1);
  dvi->open_relay(BUS_SENSE1);

}

/*************************************************************/
/*** Closes the force and sense relay on a DVI with minimized
  glitches due that occur when the force relay gets closed.
  -- Brad Sitton 11/15/04  */

void u_soft_connect(Dvi* dvi, int channel, float vout)
{
  switch (channel)
    {
    case DVI_CHANNEL_0 :
      dvi->open_relay(CONN_FORCE0);
      wait.delay_10_us(10);
      dvi->open_relay(CONN_SENSE0);
      wait.delay_10_us(10);
      dvi->close_relay(BUS_SENSE0);
      dvi->close_relay(BUS_FORCE0);
      delay(1);
      dvi->set_voltage(channel, vout);
      dvi->close_relay(CONN_SENSE0);
      dvi->close_relay(CONN_FORCE0);
      delay(1);
      dvi->open_relay(BUS_FORCE0);
      dvi->open_relay(BUS_SENSE0);
      break;
    case DVI_CHANNEL_1 :
      dvi->open_relay(CONN_FORCE1);
      wait.delay_10_us(10);
      dvi->open_relay(CONN_SENSE1);
      wait.delay_10_us(10);
      dvi->close_relay(BUS_SENSE1);
      dvi->close_relay(BUS_FORCE1);
      delay(1);
      dvi->set_voltage(channel, vout);
      dvi->close_relay(CONN_SENSE1);
      dvi->close_relay(CONN_FORCE1);
      delay(1);
      dvi->open_relay(BUS_FORCE1);
      dvi->open_relay(BUS_SENSE1);
      break;
    }
}

void u_func_check_count(void)
{
//    -- DGS updated 05/08/07 with MessageBox
//    -- DGS updated 09/14/07 with LogOperatorBadge requiring the operator to input their badge number and writes the Badge to a text file

  Text_box box;

  //Make sure the correct number of fuctions were run (make sure none
  //were erased or corrupted!
  //Only check if the part passes, indicating that program has run all the way to the end
  if( (u_func_counter != u_num_func)
      && u_device_fail==FALSE
      && u_ctr_enable==TRUE)
    {
      //box.error("This program is corrupt.\nAt least one function is missing or corrupt!\nProgram should be reloaded from archive or server."); 
      MessageBox(NULL, "This program is corrupt.\nAt least one function is missing or corrupt!\nProgram should be reloaded from archive or server.", "ATTENTION!", MB_OK | MB_SYSTEMMODAL | MB_ICONWARNING); //Use MessageBox with NT instead of box.error to prevent operators from checking the box which stops this message from popping up again

      LogOperatorBadge("This program is corrupt.\nAt least one function is missing or corrupt!\nProgram should be reloaded from archive or server.", 0, 0, 0);
	}
	if(ag_correlate)
		ag_gage->agf_correlate();
}

// **************************************************************************
// Stops tester if there are too many failures to a specific bin.
//    -- BRS 12/01/04, 
//    -- DGS updated 03/23/07 for NT, 
//    -- DGS updated 05/08/07 with MessageBox
//    -- DGS updated 09/14/07 with LogOperatorBadge requiring the operator to input their badge number and writes the Badge to a text file

void u_check_bin_stat(char *msg, int bin, float flag_pct, int num_devices)
{
	Text_box box;

   if (bin == u_bin_nt)
    u_bin_stat[bin-1] = (float) ((num_devices-1)*u_bin_stat[bin-1] + 1)/num_devices; //increase failing percentage
   else
    u_bin_stat[bin-1] = (float) ((num_devices-1)*u_bin_stat[bin-1] + 0)/num_devices; //decrease failing percentage

    if (u_bin_stat[bin-1] > flag_pct/100.0)
    {
		//"Failure rate for bin %d is over %f %% for the last %d devices, check setup\n", bin, flag_pct, num_devices
		MessageBox(NULL, msg, "ATTENTION!", MB_OK | MB_SYSTEMMODAL | MB_ICONWARNING); //Use MessageBox with NT instead of box.error to prevent operators from checking the box which stops this message from popping up again
		
		LogOperatorBadge(msg, bin, flag_pct, num_devices);

       //for (int i = 0; i<32; i++) u_bin_stat[i] = 0.0; //clear ALL bin stats
		u_bin_stat[bin-1] = 0.0; //clear ONLY current bin stat
    }
}
/***************************************************************************************************
Created By: Gremar G. Taganile -WWTS MPOC
Date:		July 13, 2006
Function:	AutoSaveSummary()
This function automatically save LS4 file. The lotid_date_time.ls4 is the filename.
This will help production to eliminate lot re-screen during VATE hangs up, crash or exits.
========================================U P D A T E S===============================================
April 4, 2007
	Solved the problem encountered when program is running on productionVATE. 

/****************************************************************************************************/
STRING prog_name, limits_name, lotid;
short move_on=0, summary_cnt=0, init_var=1;
short recent_min,previous_min;
char dir[200];
void AutoSaveSummary(void)
{
  //variable initialization
  STRING final_name, ldl;
  char path[200]="d:\\summaries\\";
  char directory[200]="c:\\asl_nt\\users";
  char date[100],time1[100],lotid1[50];
  move_on=1;								//to avoid error when going to vc++
  Func_Lib *c = (Func_Lib*)g_program;
  //get time and date
  time_t ltime;
  struct tm *today;
  struct _timeb;
  time(&ltime);
  today=localtime(&ltime);
  _strtime(time1);
  _strdate(date);
  recent_min=today->tm_min;
  if((summary_cnt>=3) && (recent_min!=previous_min))
    {
      MoveSaveSummary();
      summary_cnt=0;
    }
  //change date and time format
  strncpy(date+2,"-",1);
  strncpy(date+5,"-",1);
  strncpy(time1+2,"-",1);
  time1[strcspn(time1,":")]=0;
  strcat(date,"_");
  strcat(date,time1);
  //Get LotID, Limit set name, and Program name
  GetLotIDName(lotid);
  GetLimitSetName(limits_name);
  if(init_var)
    {
	  c->GetListDirLoc(ldl);
	  strcpy(dir,ldl);
      GetProgramName(prog_name);
      init_var=0;
    }
  //create senddatalog folder and datalog folder.Needed by visual ate to save the *.ls4 file
  if(dir[16]=='l' || dir[16]=='L')
  strcat(directory,"\\lists");
  if(dir[16]=='p' || dir[16]=='P')
  strcat(directory,"\\production");
  CreateDirectory(directory,NULL);		//create senddatalog folder
  strcat(directory,"\\senddatalog");
  CreateDirectory(directory,NULL);		//create datalog folder  
  strcat(directory,"\\datalog");
  CreateDirectory(directory,NULL);		//create datalog folder

  CreateDirectory(path,NULL);			//create summaries folder
  strcat(path,prog_name);				//added program name to path
  CreateDirectory(path,NULL);			//Create program name folder
  strcat(path,"\\");
  strcat(path,limits_name);				//added limit set name to path
  CreateDirectory(path,NULL);			//Create limit set name folder

  strcpy(lotid1,lotid);					//store lotid to lotid1
  strcat(lotid1,"_");
  strcat(lotid1,date);					//added date to lotid1

  final_name=_strdup(lotid1);			//copy lotid1 to final_name
  SendSummaryToFile(final_name);		//save *.ls4 to senddatalog folder

  summary_cnt++;
  previous_min=today->tm_min;

}
int Func_Lib::GetListDirLoc(STRING &ldl)
{
	ldl=_strdup(m_ListDirectoryLocation);
	return 0;
}
/***************************************************************************************************
Created By: Gremar G. Taganile -WWTS MPOC
Date:		July 13, 2006
Function:	MoveSaveSummary()
This function moves the saved summaries to d:\summaries\program name\limit set name folder
/****************************************************************************************************/
void MoveSaveSummary(void)
{
  //variable initialization
  char path[200]="d:\\summaries\\";
  char path1[200]="move c:\\asl_nt\\users";
  Text_box box;
   if(dir[16]=='l' || dir[16]=='L')
  strcat(path1,"\\lists");
  if(dir[16]=='p' || dir[16]=='P')
  strcat(path1,"\\production");
  strcat(path1,"\\senddatalog\\datalog\\");
  if(move_on==1)								//to avoid error when going to vc++
    {
      strcat(path1,lotid);
      strcat(path1,"*.ls4 ");
      strcat(path,prog_name);					//added program name to path
      strcat(path,"\\");
      strcat(path,limits_name);				//added limit set name to path
      strcat(path1,path);						//added path to path1
      system(path1);							//move *.ls4 to summaries
    }
}
/******************************************************************************************************/



/***************************************************************************************************
Created By: Steve B. Kavros - WWTS HQ
Date:		February 2,2007
Function:	SetHWCStatus(char,int)
This function writes the product name, pass//fail time and pass/fail time/date status to a text file 
//	SetHWCStatus("product name",pass or fail status); 
//  Sample code  SetHWCStatus("PP66",1); 
// Arguments product name = character string, pass or fail status = 1 for PASS and 0 for FAIL
/****************************************************************************************************/

void SetHWCStatus(char *my_product_name, int status)
{

FILE *log;

if ((log = fopen("\\ASL_NT\\HW_Checker_Status.txt","w+t"))  == NULL)  

{fprintf(stderr, "Cannot open input file.\n");}

time_t t;
t = time(NULL);

  fprintf(log, "%s\n", my_product_name); // write the product name
  fprintf(log, "%ld\n", t ); // write the current time
  if (status==1)     // write the PASS or FAIL status
  {  fprintf(log, "%s\n", "PASS" ); }
 
  else 	 
	  
  { fprintf(log, "%s\n", "FAIL" ); }  

fclose(log);

}

/*******************************************************************************************/

/***************************************************************************************************
Created By: Steve B. Kavros - WWTS HQ
Date:		February 6,2007
Function:	GetHWCStatus(char,int)
This function writes the product name, pass//fail time and pass/fail time/date status to a text file 
// Arguments product_name is used to compare the product running to the last product tested.
// Rev Feb 6, 2007 1548

//    -- DGS updated 05/08/07 
// SBK 5-10-07
  

//Sample code below
//Use this to log your part type and pass or fail status
getHWCStatus("SBK1966",1);  // syntax char part_type , int 1= Pass all else = fail

//setup variable to return number of hours since last pass to 
long hours;   //number of hours since last pass will be passed to "hours."
	 
// Use this code with a limit of how many hours you want to wait until your HW checker
// is run again.
hours=GetHWCStatus("SBK1966");   // char part_type
// returns the number of hours since your HW checker last "passed"
// If your HW checker did not pass or the part number doesn't match, 
// one of the two codes below will be returned.
// returns 991 if part number doesn't match the one in the log file
// returns 992 if hardware checker failed last time

/****************************************************************************************************/

long GetHWCStatus(char *product_name)
{

 long Logged_Time;
 char status_flag[5]; //="PASS"; 
 char status_pass[5]="PASS";
 //char status_fail[5]="FAIL";
 //char sample[5]="PASS";
 long hours;
 int status1;
 char my_product_name[10];
 int quick_exit=0;
 hours=999;

 FILE *log;

 if ((log = fopen("\\ASL_NT\\HW_Checker_Status.txt","r+t"))  == NULL)  
 {
  fprintf(stderr, "Cannot open input file.\n");
  quick_exit=1;
 }


 if (quick_exit!=1)
 {
  fscanf(log, "%s", my_product_name); 
  fscanf(log, "%ld",&Logged_Time);  
  fscanf(log, "%s", status_flag);  
  fclose(log);
 }	


 if(strcmp(my_product_name,product_name) == 0)   
 {
  status1=1;               // Two items do match
  time_t ct;
  ct = time(NULL);         //get current time
  hours=(ct-Logged_Time);  //in seconds
  hours=(hours/60);  	  //in minutes	
  hours=(hours/60);		  // in hours	

  if(strcmp(status_flag,status_pass)  )     // == 0)   
  {
   status1=0;   //part number matched but the HW checker failed
   hours=992;
  }

 }
 else 
 {
  status1=0;   //Part number doesn't matched the log file
  hours=991;
 }

 return(hours);
}

/***************************************************************************************************
Created By: JEZ and Darryl Smith(SVL HQ)
Date:		September 14,2007
Function:	short Message_dialog_nt(char *topmessage, char *bottommessage, char *fieldmessage)
This function pops up a dialog message box with topmessage and bottom message.  User can enter characters in the field box

//jez, DGS 09/14/07

//Sample code below
//Message_dialog_nt(" Please Fix the setup and enter your BADGE number", " Hit <ENTER> to continue	", Badge_nt);
/****************************************************************************************************/

short Message_dialog_nt(char *topmessage, char *bottommessage, char *fieldmessage)
{
	ModalDialogDescription mdd;
	mdd.SetStatusDialogHasYesButton(1);
	mdd.SetStatusDialogHasNoButton(1);
	mdd.SetDialogTopMessage (topmessage);
	mdd.SetDialogBottomMessage (bottommessage);
	mdd.SetDialogEditFieldInitializationText(fieldmessage);
	BOOL iret1 = RunModalDialog (&mdd);
	if(strlen(mdd.GetEditFieldText())>50)
	{
	 strcpy(fieldmessage, "");
	 return 50;
	}
	else
	 strcpy(fieldmessage, mdd.GetEditFieldText());
	
	if(mdd.GetStatusDialogYesButtonHasBeenPushed())
		return 1;
	else 
		if(mdd.GetStatusDialogNoButtonHasBeenPushed())
			return 0;
		else 
			return -1;
}

/*******************************************************************************************/

/***************************************************************************************************
Created By: Darryl Smith - SVL HQ
Date:		September 14,2007
Function:	LogOperatorBadge(msg, bin, flag_pct, num_devices);
This function writes the time, program name, lot ID, limits name, operator badge, message, failing bin, flag percent, and number of devices to a text file 
//  Sample code  LogOperatorBadge("Failed Contact Open", 25, 5, 100); 
/****************************************************************************************************/

void LogOperatorBadge(char *msg, int bin, float flag_pct, int num_devices)
{

 FILE *log;
 STRING prog_name, limits_name, lotid, local_time;
 char msg_cleaned[301], temp_string[81], local_time_str[81];
 char log_file_name[80] = "\\ASL_NT\\LogOperatorBadge_";
 int i, num_digits;
 time_t t, ltime;

 char tester_id[MAX_COMPUTERNAME_LENGTH + 1] = "";
 DWORD tester_id_size                        = MAX_COMPUTERNAME_LENGTH+1;


 do
 {
   strcpy(Badge_nt,"");
   Message_dialog_nt(" Please Fix the setup and enter your BADGE number", " Hit <ENTER> to continue	", Badge_nt);
   num_digits = 0;
   while((isdigit(Badge_nt[num_digits])))
   {
    temp_string[num_digits] = *(Badge_nt+num_digits);
    num_digits++;
   }
   temp_string[num_digits] = '\0';
   if((int)strlen(Badge_nt) != num_digits || num_digits < 3 || num_digits > 10)
    MessageBox(NULL, "The Number You Entered Does Not Appear To Be Valid", "ATTENTION!", MB_OK | MB_SYSTEMMODAL | MB_ICONWARNING); //Use MessageBox with NT instead of box.error to prevent operators from checking the box which stops this message from popping up again
 } while((int)strlen(Badge_nt) != num_digits || num_digits < 3 || num_digits > 10);
 //mvxml_read_bins();

 //system ("echo %computername% >>C:\LogOperatorBadge_%computername%.txt");
 GetComputerName(tester_id, &tester_id_size);

 GetProgramName(prog_name);
 GetLimitSetName(limits_name);
 GetLotIDName(lotid);

 strcat(log_file_name, tester_id);
 strcat(log_file_name, ".txt");

 if ((log = fopen(log_file_name,"a+t"))  == NULL)
	 {fprintf(stderr, "Cannot open input file.\n");}


 //WRITE TO TEXT FILE: 
 //time, local time, program name, lot ID, limits name, operator badge, failing bin, flag percent, number of devices, message 


 //TIME IN CODE AND SECONDS
 t = time(NULL);
 fprintf(log, "Time=%12ld, ", t); // write the current time


 //LOCAL TIME IN READABE FORMAT
 ltime = time(&ltime);
 local_time = ctime(&ltime);
 //remove newline and space from local_time
 i=0;
 while(local_time[i] != '\0' && i<80)
 {
  if(local_time[i] == '\n')
   local_time_str[i] = '~';
  else
   local_time_str[i] = local_time[i];
  i++;
 }
 local_time_str[i] = '\0';
 i=strlen(local_time_str);
 while(i<30) //Standardize Length of String for Datalog Readability
 {
	 local_time_str[i] = ' ';
	 i++;
 }
 local_time_str[i] = '\0';
 fprintf(log, "LocalTime=%s, ", (const char *) local_time_str); // write the current LOCAL time in readable format


 //PROGRAM NAME
 strcpy(temp_string, prog_name);
 i=strlen(temp_string);
 while(i<40) //Standardize Length of String for Datalog Readability
 {
	 temp_string[i] = ' ';
	 i++;
 }
 temp_string[i] = '\0';
 fprintf(log, "ProgramName=%s, ", (const char *) temp_string); // write the program name


 //LIMIT PROGRAM NAME
 strcpy(temp_string, limits_name);
 i=strlen(temp_string);
 while(i<40) //Standardize Length of String for Datalog Readability
 {
	 temp_string[i] = ' ';
	 i++;
 }
 temp_string[i] = '\0';
 fprintf(log, "LimitsName=%s, ", (const char *) temp_string); // write the limits_name


 //LOT ID NUMBER
 strcpy(temp_string, lotid);
 i=strlen(temp_string);
 while(i<40) //Standardize Length of String for Datalog Readability
 {
	 temp_string[i] = ' ';
	 i++;
 }
 temp_string[i] = '\0';
 fprintf(log, "LotID=%s, ", (const char *) temp_string); // write the lotid


 //TESTER ID NAME
 strcpy(temp_string, tester_id);
 i=strlen(temp_string);
 while(i<40) //Standardize Length of String for Datalog Readability
 {
	 temp_string[i] = ' ';
	 i++;
 }
 temp_string[i] = '\0';
 fprintf(log, "TESTER ID=%s, ", (const char *) temp_string); // write the lotid


 //OPERATOR BADGE NUMBER
 strcpy(temp_string, Badge_nt);
 i=strlen(temp_string);
 while(i<20) //Standardize Length of String for Datalog Readability
 {
	 temp_string[i] = ' ';
	 i++;
 }
 temp_string[i] = '\0';
 fprintf(log, "Badge=%s, ", temp_string); // write the Badge


 //FAILING BIN NUMBER
 fprintf(log, "FailingBin=%3ld, ", bin); // write the failing bin


 //FAILING BIN FLAG PERCENTAGE(TRIP POINT)
 sprintf(temp_string, "%5.5f", flag_pct);
 i=12;
 num_digits=i-strlen(temp_string); //Number of Leading Spaces to Insert
 if(num_digits>0)
 {
  while(i>=num_digits) //Standardize Length of String for Datalog Readability
  {
	 temp_string[i] = temp_string[i-num_digits];
	 i--;
  }
  while(i>=0) //Standardize Length of String for Datalog Readability
  {
	 temp_string[i] = ' ';
	 i--;
  }
 }
 fprintf(log, "FlagPercent=%s, ", temp_string); // write the flag percent


 //FAILING BIN NUMBER OF DEVICES FOR COMPARISON
 fprintf(log, "NumberOfDevices=%5ld, ", num_devices); // write the number of devices


 //FAILING BIN MESSAGE TO OPERATOR
 //remove newlines and commas from msg
 i=0;
 //msg_cleaned[] = "";
 while(*(msg+i) != '\0' && i<300)
 {
  if(*(msg+i) == '\n')
   msg_cleaned[i] = '~';
  else if(*(msg+i) == ',')
   msg_cleaned[i] = '_';
  else
   msg_cleaned[i] = *(msg+i);
  i++;
 }
 msg_cleaned[i] = '\0';
 i=strlen(msg_cleaned);
 while(i<300) //Standardize Length of String for Datalog Readability
 {
	 msg_cleaned[i] = ' ';
	 i++;
 }
 msg_cleaned[i] = '\0';
 fprintf(log, "Message=%s", msg_cleaned); // write the message


 //NEWLINE
 fprintf(log, "\n"); // write a newline


 fclose(log);

}

/*******************************************
			Test Time Functions

	author: Michael Urhausen

	description: functions for managing the information in the datalog / lot summary
	version history:
		last updated --	1.7.08

********************************************/

float MU_time = 0;
float MU_avg_ttime = 0;

//updates MU_time to current time
void u_start_ttime(void)
{
	MU_time = u_clock_read();
}

//logs the test time to the lot summary
//skip_bin1, skip_bin2, and num_tests have default values of 
//0,0, and 100 respectfully
void u_log_ttime(int skip_bin1, int skip_bin2, int num_tests)
{
	float time = u_clock_read();
	float ttime = time - MU_time;
	
	if(skip_bin1 != u_bin_nt || skip_bin2 != u_bin_nt)
	{
		if(time > MU_time)
			ttime = time - MU_time;
		else
			ttime = (4295 - MU_time) + time;
		
		STRING comment = "Average test time: ";
		ttime = u_update_avg_ttime(ttime, num_tests);
		comment += ttime;
		comment += "s";
	
		BOOL result = u_UpdateLotSummaryComment(comment);
	}
	MU_time = time;
}
//Keeps track of average test time.
//avgerage is over num test tests.
//returns -1 on an error
float u_update_avg_ttime(float time, int num_tests)
{
	if(time <= 0 || num_tests <= 0)
		return -1;

	if(MU_avg_ttime != 0)
		MU_avg_ttime = MU_avg_ttime + ((time - MU_avg_ttime) / num_tests);
	else
		MU_avg_ttime = time;

	return MU_avg_ttime;
}
//Clear test times.
void u_clear_ttimes()
{
	MU_avg_ttime = 0;
}
//Appends a string to the lot summary comments.
//Comments written previously through write_comment are not preserved.
bool u_UpdateLotSummaryComment(STRING comment)
{
	return ClearLotSummaryComments() && AppendLotSummaryComment(comment);
}
//Gets the current serial number.
long u_GetSerialNum(void)
{
	STRING s_devNum;

	BOOL result = GetSerialNum(s_devNum);
	return (long)(s_devNum);
	
}
//Sets the serial number in the datalog for the current part
void u_SetSerialNum(long num)
{
	STRING s_num;

	s_num = (long)num;
	BOOL result = SetSerialNum(s_num);
}
/**************************************************/

/* Maxim User Executive Functions  Added to incorporate Galaxy and Maxim STDF driver*/
void maxim_user_init(void)
{
}

void maxim_user_load(void)
{
#ifdef GALAXY_DPAT
	char szFullConfigFileName[256];
	char szFullRecipeFileName[256];

	strcpy(szFullConfigFileName, "C:\\Galaxy\\gtl_tester.conf");
	strcpy(szFullRecipeFileName, "C:\\Galaxy\\maxim_debug_dpat4.csv");

#ifdef _DEBUG 
	printf("**** DPAT: calling gtl_init_lib(\"%s\", \"%s\")\n", szFullConfigFileName, szFullRecipeFileName);
#endif
	int nStatus = gtl_init_lib(szFullConfigFileName, szFullRecipeFileName);
#ifdef _DEBUG 
	printf("**** DONE: gtl_init_lib() returned status=%d\n", nStatus);
#endif
#endif
}

void maxim_user_start_lot(void)
{
}

void maxim_user_start_test(void)
{
#ifdef GALAXY_DPAT  
#ifdef _DEBUG 
	printf("**** DPAT: calling gtl_beginjob_lib()\n");
#endif
#ifdef MULTI_SITE
	for (int iSite=0; iSite<CSC_LAST_SITE;iSite++)
	{
		if (active_site[iSite])
			u_bin_num[0]=1;
		else
			u_bin_num[0]=0;
	}
#else
	u_bin_num[0]=1;
#endif
	{
		u_bin_num[iSite]=1;
	}
    gtl_beginjob_lib(NULL, NULL);
#ifdef _DEBUG 
	printf("**** DONE\n");
#endif
#endif
}

void maxim_user_before_binning(void)
{
//Galaxy binning
#ifdef GALAXY_DPAT
#ifdef _DEBUG 
	// THIS IS WHERE THE gtl_binning_lib() FUNCTION SHOULD BE CALLED
	// FOR EVERY PART (Pass & Fail), and eventually overload the
	// binning
	int current_bin=1, new_bin=1;
	for(int site_num=0; site_num < CSC_MAX_SITES; site_num++)
	{
		current_bin = asl_get_bin(site_num);
		// Get new bin (can be overloaded with PAT bin if PAT failures)
#ifdef _DEBUG 
		printf("**** DPAT: calling gtl_binning_lib(%d, %d)\n", site_num, current_bin);
#endif
		new_bin = (float)gtl_binning_lib(site_num, current_bin);
#ifdef _DEBUG 
		printf("**** DONE\n");
#endif
	}
#endif
#endif
}

void maxim_user_end_test(void)
{
#ifdef GALAXY_DPAT  
#ifdef _DEBUG 
	printf("**** DPAT: calling gtl_endjob_lib()\n");
#endif
	gtl_endjob_lib();
#ifdef _DEBUG 
	printf("**** DONE\n");
#endif
#endif
}

void maxim_user_next_device(void)
{
}

void maxim_user_end_lot(void)
{
#ifdef GALAXY_DPAT
#ifdef _DEBUG 
	printf("**** DPAT: calling gtl_endlot_lib()\n");
#endif
	gtl_endlot_lib();
#ifdef _DEBUG 
	printf("**** DONE\n");
#endif
#endif
}

void maxim_user_exit(void)
{
}
/*Maxim User Executive Functions*/

#ifdef GALAXY_DPAT
int asl_get_bin(const int site_no)
{
	return u_bin_num[site_no];
}
#endif