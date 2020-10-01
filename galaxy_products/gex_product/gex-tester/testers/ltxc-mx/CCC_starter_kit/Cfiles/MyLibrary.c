#include <stdlib.h>
#include <strings.h>
#include <math.h>
#include "ltx_std.h"
#include "return_codes.h"
#include "defines.h"

#ifdef R14_3
    #include "c_library.R14.3.h"
#else
    #include "c_library.h"
#endif

#include    "dpfftw.h"
#include    "spfftw.h"
#include <string.h>
#include <syslog.h>

//If using cpp compiler, make sure these functions compiled as C 
// so their names do not get mangled and the linker can find them.

#ifdef __cplusplus
extern "C" 
{
    int MyGenSin(LTX_CADENCE_C_STRUCT * p);
    int MyInverseFFT(LTX_CADENCE_C_STRUCT * p);
    int MyForwardFFT(LTX_CADENCE_C_STRUCT * p);
    int MyMakeRamp(LTX_CADENCE_C_STRUCT * p);
    int MyQuickFoxOld(LTX_CADENCE_C_STRUCT * p);
    int MyQuickFoxNew(LTX_CADENCE_C_STRUCT * p);
    int MyOpenLogNew(LTX_CADENCE_C_STRUCT * p);
    int MyOpenLogOld(LTX_CADENCE_C_STRUCT * p);
    int MyCloseLog(LTX_CADENCE_C_STRUCT * p);
}
#endif

static FILE *MyLog=NULL;

int validate_fft_length(int length);
int ltx_itoa(int *int_values,  char *ascii_string, int str_len);
int ltx_atoi(char *ascii_string, int *ascii_values);
void writeToLog(char * buf);

int MyMakeRamp(LTX_CADENCE_C_STRUCT * p)
{
float *out_array, offset,max,delta;
unsigned long nelm;
int i,sts=STS_OK;

    if((p->param[PAR1].flag & FLOAT_SCALAR_BITS)!=IS_FLOAT_SCALAR)
        return PAR1E+NOTfloat_scalar;

    //Following is becasue of different data structure between R14.3 and 
    //everything preceding it

    #ifdef R14_3
        offset=p->param[PAR1].data.f_data;
    #else
        offset=p->param[PAR1].f_const;
    #endif

    if((p->param[PAR2].flag & FLOAT_SCALAR_BITS)!=IS_FLOAT_SCALAR)
        return PAR2E+NOTfloat_scalar;

    #ifdef R14_3
        max=p->param[PAR2].data.f_data;
    #else
        max=p->param[PAR2].f_const;
    #endif

    if((p->param[PAR3].flag & IS_FLOAT_ARRAY)!=IS_FLOAT_ARRAY)
        return PAR3E+NOTfloat_array;
    nelm=p->param[PAR3].size/sizeof(float);
    out_array=p->param[PAR3].p.fptr;

    delta=(max-offset)/nelm;
 
    for(i=0; i<nelm; i++)
        out_array[i]=offset+(i*delta);
    
    p->param[PAR3].flag|=LTX_RETURN_DATA;
        
    return sts;
}


int MyGenSin(LTX_CADENCE_C_STRUCT * p)
{
float *ramp_array,*out_array, delta;
unsigned long nelm;
int i,sts=STS_OK;

    if((p->param[PAR1].flag & IS_FLOAT_ARRAY)!=IS_FLOAT_ARRAY)
        return PAR1E+NOTfloat_array;
    nelm=p->param[PAR1].size/sizeof(float);
    ramp_array=p->param[PAR1].p.fptr;

    if((p->param[PAR2].flag & IS_FLOAT_ARRAY)!=IS_FLOAT_ARRAY)
        return PAR2E+NOTfloat_array;
    if( (p->param[PAR2].size/sizeof(float)) !=nelm)
        return ARRAY_SIZE_MISMATCH;
    out_array=p->param[PAR2].p.fptr;

    for(i=0; i<nelm; i++)
        out_array[i]=sin(ramp_array[i]);
    
    p->param[PAR2].flag|=LTX_RETURN_DATA;
        
    return sts;
}


int MyInverseFFT(LTX_CADENCE_C_STRUCT * p)
{
float *in_array,*out_array, delta;
unsigned long nelm;
int i,sts=STS_OK, ret_val;

    if((p->param[PAR1].flag & IS_FLOAT_ARRAY)!=IS_FLOAT_ARRAY)
        return PAR1E+NOTfloat_array;
    nelm=p->param[PAR1].size/sizeof(float);
    ret_val=validate_fft_length(nelm);
    if(ret_val !=STS_OK)
        return BAD_FFT_LEN;
    in_array=p->param[PAR1].p.fptr;

    if((p->param[PAR2].flag & IS_FLOAT_ARRAY)!=IS_FLOAT_ARRAY)
        return PAR2E+NOTfloat_array;
    if( (p->param[PAR2].size/sizeof(float)) !=nelm)
        return ARRAY_SIZE_MISMATCH;
    out_array=p->param[PAR2].p.fptr;


    ret_val=spFFTW::RealFFT.inverseFFT(nelm,in_array,out_array);
    if(ret_val !=STS_OK)
        return FFT_ERR;

    p->param[PAR2].flag|=LTX_RETURN_DATA;

    return sts;
}


int MyForwardFFT(LTX_CADENCE_C_STRUCT * p)
{
float *in_array,*out_array, delta;
unsigned long nelm;
int i,sts=STS_OK, ret_val;

    if((p->param[PAR1].flag & IS_FLOAT_ARRAY)!=IS_FLOAT_ARRAY)
        return PAR1E+NOTfloat_array;
    nelm=p->param[PAR1].size/sizeof(float);
    ret_val=validate_fft_length(nelm);
    if(ret_val !=STS_OK)
        return BAD_FFT_LEN;
    in_array=p->param[PAR1].p.fptr;

    if((p->param[PAR2].flag & IS_FLOAT_ARRAY)!=IS_FLOAT_ARRAY)
        return PAR2E+NOTfloat_array;
    if( (p->param[PAR2].size/sizeof(float)) !=nelm)
        return ARRAY_SIZE_MISMATCH;
    out_array=p->param[PAR2].p.fptr;

    ret_val=spFFTW::RealFFT.forwardFFT(nelm,in_array,out_array);
    if(ret_val !=STS_OK)
        return FFT_ERR;

    p->param[PAR2].flag|=LTX_RETURN_DATA;
        
    return sts;
}


int MyQuickFoxOld(LTX_CADENCE_C_STRUCT * p)
{
int *in_array,*out_array;
unsigned long nelm;
int max_len=MAXSTRLEN;     //Maximum string size in cadence
char in_string[MAXSTRLEN];
int sts=STS_OK;
char log[2000];

    if((p->param[PAR1].flag & IS_INTEGER_ARRAY)!=IS_INTEGER_ARRAY)
        return PAR1E+NOTinteger_array;
    in_array=p->param[PAR1].p.iptr;
    nelm=p->param[PAR1].size/sizeof(int);
    if(nelm<MAXSTRLEN)
        max_len=nelm;

    /* Convert integer array into char array */
    if((sts=ltx_itoa(in_array,in_string,max_len))!=STS_OK)
        return sts;

    /* Make sure input string equals "The quick brown fox " */
    if((strcmp(in_string,"The quick brown fox "))!=0)
        return INVALID_STRING;

    /* Finish the sentence */
    strcat(in_string,"jumped over the lazy dog.");

    /*Make sure it will all fit in integer array */
    if(strlen(in_string)>nelm)
        return ARRAY_SIZE_NOT_VALID;

    sprintf(log,"Size of in_string=%d",strlen(in_string));
    writeToLog(log);

    /* Convert char array back into integer array */
    ltx_atoi(in_string,in_array);

    /* Tell Cadence we have added data to the integer array */
    p->param[PAR1].flag|=LTX_RETURN_DATA;

    return sts;

}
int MyQuickFoxNew(LTX_CADENCE_C_STRUCT * p)
{
int sts=FUNCTION_UNAVAILABLE;
#ifdef R14_3
STRING *in_cadence_string;  /*See definition of STRING in c_library.R14.3.h */
unsigned char maxlen,len, temp_len;
char temp_string[MAXSTRLEN], temp[1000];
sts=STS_OK;

    if((p->param[PAR1].flag & LTX_IS_STRING)!=LTX_IS_STRING)
        return PAR1E+NOTstring;
    in_cadence_string=p->param[PAR1].p.strptr;

    /* Get maxlen (size) of Cadence string */
    maxlen=in_cadence_string->maxlen;   

    /* Get current len of Cadence string (number of valid characters in the string)  */
    len=in_cadence_string->len;   

    /* Make sure input Cadence string contains "The quick brown fox " */
    if( (strncmp(in_cadence_string->txt,"The quick brown fox ",len)) !=0)
        return INVALID_STRING;

    /*Make sure it will all fit in the Cadence string */
    strcpy(temp_string,"jumped over the lazy dog.");
    temp_len=strlen(temp_string);
    if( (len+temp_len) >maxlen)
        return STRING_SIZE_ERROR;

    /* Finish the sentence */
    strcat(in_cadence_string->txt,temp_string);

    /* Update the current string size */
    in_cadence_string->len=len+temp_len;

    sprintf(temp,"From MyQuickFoxNew: maxlen=%d, original len=%d, new len=%d text=%s",maxlen,len,len+temp_len,in_cadence_string->txt);
    writeToLog(temp);

    /* Tell Cadence we have added data to the Cadence string */
    p->param[PAR1].flag|=LTX_RETURN_DATA;
#endif
    return sts;

}
int MyOpenLogNew(LTX_CADENCE_C_STRUCT * p)
{
int sts=FUNCTION_UNAVAILABLE;
#ifdef R14_3
STRING *in_cadence_string;  /*See definition of STRING in c_library.R14.3.h */

    sts=STS_OK;
    if(MyLog)
        return LOG_FILE_ALREADY_OPEN;
    if((p->param[PAR1].flag & LTX_IS_STRING)!=LTX_IS_STRING)
        return PAR1E+NOTstring;
    in_cadence_string=p->param[PAR1].p.strptr;

    MyLog=fopen(in_cadence_string->txt,"w");
    if(!MyLog)
        return LOG_OPEN_ERR;
#endif 
    return sts;
}  
int MyOpenLogOld(LTX_CADENCE_C_STRUCT * p)
{
int sts=STS_OK;
int *in_array;
unsigned long nelm,len=MAXSTRLEN;
char in_string[MAXSTRLEN];

    if(MyLog)
        return LOG_FILE_ALREADY_OPEN;

    if((p->param[PAR1].flag & IS_INTEGER_ARRAY)!=IS_INTEGER_ARRAY)
        return PAR1E+NOTinteger_array;
    in_array=p->param[PAR1].p.iptr;
    nelm=p->param[PAR1].size/sizeof(int);
    if(nelm<MAXSTRLEN)
        len=nelm;

    /* Convert integer array into char array */
    if((sts=ltx_itoa(in_array,in_string,len))!=STS_OK)
        return sts;

    MyLog=fopen(in_string,"w");
    if(!MyLog)
        return LOG_OPEN_ERR;

    return sts;
}  
int MyCloseLog(LTX_CADENCE_C_STRUCT * p)
{
int sts;

    if(!MyLog)     //File was never opened
        return STS_OK;

    sts=fclose(MyLog);
    if(sts)
        return LOG_CLOSE_ERR;
  
    return STS_OK;
}  

/*********************************************************************
 * converts an array of ascii integer values into an ascii string 
 *******************************************************************/
int ltx_itoa(int *int_values,  char *ascii_string, int str_len)
{
    int i = 0;

    if (!str_len){
        return INVALID_STRING;
    }
    for(i=0; i<str_len; i++)
    {
        if(int_values[i]==0){
            ascii_string[i]='\0';
            return STS_OK;
        }
        else if(int_values[i]==10){
            if( (i+1)==str_len){
                return INVALID_STRING;  //No room for '\n' & '\0'
	    }
  	    else{
                ascii_string[i]='\n';
                ascii_string[i+1]='\0';
                return STS_OK;
	    }
	}
        else {
	    ascii_string[i] = (char)int_values[i];
	}
    }
    //If we get here, we went through int array and never found a
    // 0 or 10. No room left in string so error
    return INVALID_STRING;  
}

/* converts an ascii string to an array of ascii integers */
int ltx_atoi(char *ascii_string, int *ascii_values)
{
	int i = 0;
	
	do{
		ascii_values[i] = (int)ascii_string[i];
		i++;
	}while(ascii_string[i] != '\n' && ascii_string[i] != '\0');

	if(ascii_string[i] == '\n')
		ascii_values[i] = 10;
	else
		ascii_values[i] = 0;

	return 0;

}

int validate_fft_length(int length)
{
    int kk;

    for ( kk = 1; kk <= MAX_FFT_LEN; kk *= 2 )
    {
        if ( length == kk )
            return 0;
    }

    return BAD_FFT_LEN;
}
void writeToLog(char * buf)
{
char temp[2000];

    if(!MyLog)
        return;

    sprintf(temp,"%s\n",buf);  //Add CR to message
    
    /* Could add error checking if you wanted to */
    fwrite(temp,strlen(temp),1,MyLog);
    fflush(MyLog);        //Forces all messages to be written to the file immediately
}
