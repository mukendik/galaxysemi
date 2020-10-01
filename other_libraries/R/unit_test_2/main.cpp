#define R_NO_REMAP 
#include <R.h>
#include <Rinternals.h>
#include <Rembedded.h>
#include <cmath> // for nan()
#include <string>

#define	GEX_C_DOUBLE_NAN (double) 1.7e308	// NaN value for Galaxy software


class RApplication {
public:
    RApplication(){
        const char *R_argv[] = {
            "--gui=none", 
            "--no-save", 
            "--no-readline", 
            "--silent"
        };
        int argc = 4 ;
        
        Rf_initEmbeddedR(argc, (char**)R_argv);
        R_ReplDLLinit() ;
    }
    
    
    void generate_dataset_1()
    {
        printf("generate_dataset_1...\n");
        // creating some data
        SEXP d1 = PROTECT( Rf_allocVector( REALSXP, 10 ) ) ;
        for( int i=0; i<10; i++)
        {
            REAL(d1)[i] = sqrt(i);
            //REAL(d1)[i] = GEX_C_DOUBLE_NAN;
            //REAL(d1)[i] = R_NaN;
            //REAL(d1)[i] = nan(""); // same results than R_NaN
        }
        REAL(d1)[0] = nan(""); // just to test if R can still compute stats with nan values

        printf("assign this data in the global environment...\n");
        SEXP symbol = Rf_install( "dataset1" );
        if (!symbol)
            return;
        printf("define var...\n");
        Rf_defineVar( symbol, d1, R_GlobalEnv );
        printf("UNPROTECT...\n");
        UNPROTECT(1) ; // d1
    }

    bool run_script(const char* filename )
    {
        printf("run_script %s\n", filename);
        SEXP call = PROTECT( Rf_lang2( 
            Rf_install( "try" ), 
            Rf_lang2( Rf_install( "source" ), Rf_mkString(filename) )  
            ) ) ;
        printf("eval...\n");
        SEXP res = PROTECT( Rf_eval( call, R_GlobalEnv ) ) ;
        if (!res)
            printf("null eval result...\n");
        UNPROTECT(2) ;
        
        return ! Rf_inherits( res, "try-error") ; 
    }
    
    double get_result_1()
    {
        printf("get_result_1...\n");
        SEXP res = Rf_findVar( Rf_install("result1"), R_GlobalEnv );
        if (!res)
            return 0.0;
        return REAL(res)[0] ;
    }
    
    ~RApplication(){
        Rf_endEmbeddedR(0);    
    }
private:

};

int main(int, char** )
{

    RApplication R ;
    
    // generate some data
    R.generate_dataset_1() ;

    printf("R_NaN=%g %f\n", R_NaN, R_NaN);

    // run a script
    int success = R.run_script( "scripts/mean.R" ) ;
    
    // get results
    if( success ){
        double result = R.get_result_1() ;
        Rprintf( "success, result is: %5.3f\n", result ) ;
    }
    
    return !success ;
}
