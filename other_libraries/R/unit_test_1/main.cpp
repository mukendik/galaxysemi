//#include <QtCore/QCoreApplication>

#include "R.h"
#include "Rmath.h"
#include "Rembedded.h"
#include "Rinternals.h"

#define GS_FLT_MAX_4BYTES 3.402823466e+38F
#define GS_DBL_MAX_8BYTES 1.7976931348623158e+308
#define GEX_C_DOUBLE_NAN (double) 1.7e308
#define STRING(x) #x
#define STRING2(x) STRING(x)

int main(int /*argc*/, char* argv[])
{
    fprintf(stderr, "argv=%s\n", argv[0]?argv[0]:"?");
    fprintf(stderr, "R_HOME=%s\n",
            getenv("R_HOME") ? getenv("R_HOME") : "null");
    fprintf(stderr, "R_LIBS=%s\n",
            getenv("R_LIBS") ? getenv("R_LIBS") : "null");
    fprintf(stderr, "R_PROFILE=%s\n",
            getenv("R_PROFILE") ? getenv("R_PROFILE") : "null");

    fprintf(stderr, "R_HOME_DEF=%s\n", STRING2(R_HOME_DEF));
    fprintf(stderr, "OSFOLDER=%s\n", STRING2(OSFOLDER));

    char lDesiredRHome[255] = "";
    sprintf(lDesiredRHome, "../r_home/%s", STRING2(R_HOME_DEF));
    fprintf(stderr, "Best R_HOME should be %s\n", lDesiredRHome);

    //QString lRHomePath;
    char* lRHomePath=0;
    // check if GALAXY_R_HOME is defined then use it
    //lRHomePath = QString(qgetenv("GALAXY_R_HOME"));
    lRHomePath=getenv("GALAXY_R_HOME");
    //if (! lRHomePath.simplified().isEmpty())
    char lTmp[1024]="";
    if (lRHomePath==0)
    {
        //qputenv("R_HOME", lRHomePath.toLatin1().data());
        sprintf(lTmp, "R_HOME=%s", lRHomePath);
        putenv(lTmp);
    }
    else
    {
        fprintf(stderr, "Trying to setup R_HOME...\n");
        //qputenv("R_HOME", lDesiredRHome);
        sprintf(lTmp, "R_HOME=%s", lDesiredRHome);
        putenv(lTmp);
    }
    fprintf(stderr, "R_HOME=%s\n",
            getenv("R_HOME") ? getenv("R_HOME") : "null");

    // exple: g:/galaxy_dev/other_libraries/R/win32
    char* lRLibs = getenv("R_LIBS");

    fprintf(stderr, "main: R_LIBS=%s\n", lRLibs ? lRLibs : "?");

    // needs define Win32 ?
    //char *lRDLLVersion=getDLLVersion();
    //fprintf(stderr, "R dll version=%s\n", lRDLLVersion?lRDLLVersion:"?");

    fprintf(stderr, "R_TempDir=%s\n", R_TempDir ? R_TempDir : "?");

    //QCoreApplication a(argc, argv);

    // According to R official doc:
    /*
       char *argv[] = {"REmbeddedPostgres", "--gui=none", "--silent"};
       int argc = sizeof(argv)/sizeof(argv[0]);
       Rf_initEmbeddedR(argc, argv);
     */
    // on linux such sometimes appears:
    // Fatal error: you must specify '--save', '--no-save' or '--vanilla'
    // The command-line option --vanilla implies --no-site-file,
    // --no-init-file, --no-environ and (except for R CMD) --no-restore
    // --silent ?

    fprintf(stderr, "Preparing startup args: --gui=nano --vanilla ...\n");
    char* Rargv[] = {
        (char*) "REmbeddedPostgres", (char*) "--gui=none", (char*) "--vanilla",
        (char*) ""
    };
    int Rargc = sizeof(Rargv) / sizeof(Rargv[0]);

    // When this is called, the environment variables such as R_HOME,
    // R_PROFILE, R_LIBS should be appropriately set.
    fprintf(stderr, "Rf_initEmbeddedR %d args...\n", Rargc);
    int lR = Rf_initEmbeddedR(Rargc, Rargv);
    fprintf(stderr, "Rf_initEmbeddedR: %d\n", lR);
    // I have read that Rf_initEmbeddedR always return 1 but anyway...
    if (lR!=1)
        return EXIT_FAILURE;

    fprintf(stderr, "R_TempDir=%s\n", R_TempDir ? R_TempDir : "?");

    SEXP e;
    PROTECT(e = allocVector(LANGSXP, 1));
    SETCAR(e, Rf_install("foo"));
    int errorOccurred;
    R_tryEval(e, R_GlobalEnv, &errorOccurred);

    R_FlushConsole();

    double max_4bytes = GS_FLT_MAX_4BYTES;
    double max_8bytes = GEX_C_DOUBLE_NAN;

    //R_ProcessEvents(); // crash ?
    fprintf(stderr, "NA tests\n");
    fprintf(stderr, "Is %lf NA : %d\n", 1.5, R_IsNA(1.5));
    fprintf(stderr, "Is FLT_MAX_4BYTES NA : %d\n", R_IsNA(max_4bytes));
    fprintf(stderr, "Is DBL_MAX_8BYTES NA : %d\n", R_IsNA(max_8bytes));
    fprintf(stderr, "Is %lf NA : %d\n", R_NaN, R_IsNA(R_NaN));
    fprintf(stderr, "Is %lf NA : %d\n", R_NaN, R_IsNA(R_NaReal));
    fprintf(stderr, "Is %lf NA : %d\n", R_NaN, R_IsNA(R_NaInt));
    fprintf(stderr, "NaN tests\n");
    fprintf(stderr, "Is %lf NaN : %d\n", 1.5, R_IsNaN(1.5));
    fprintf(stderr, "Is FLT_MAX_4BYTES NaN : %d\n", R_IsNaN(max_4bytes));
    fprintf(stderr, "Is DBL_MAX_8BYTES NaN : %d\n", R_IsNaN(max_8bytes));
    fprintf(stderr, "Is %lf NaN : %d\n", R_NaN, R_IsNaN(R_NaN));
    fprintf(stderr, "Is %lf NaN : %d\n", R_NaReal, R_IsNaN(R_NaReal));
    fprintf(stderr, "Is %d NaN : %d\n", R_NaInt, R_IsNaN(R_NaInt));
    fprintf(stderr, "Finite tests\n");
    fprintf(stderr, "Is %lf Finite : %d\n", 1.5, R_finite(1.5));
    fprintf(stderr, "Is FLT_MAX_4BYTES Finite : %d\n", R_finite(max_4bytes));
    fprintf(stderr, "Is DBL_MAX_8BYTES Finite : %d\n", R_finite(max_8bytes));
    fprintf(stderr, "Is %lf Finite : %d\n", R_NaN, R_finite(R_NaN));
    fprintf(stderr, "Is %lf Finite : %d\n", R_NaReal, R_finite(R_NaReal));
    fprintf(stderr, "Is %d Finite : %d\n", R_NaInt, R_finite(R_NaInt));

    fprintf(stderr, "R_PosInf= %lf\n", R_PosInf);

    //Rfprintf(stderr, "Rprintf..."); // crash

    fprintf(stderr, "unif_rand=%lf\n", unif_rand());

    fprintf(stderr, "2^2=%lf\n", R_pow(2, 2));
    fprintf(stderr, "norm_rand()=%lf\n", norm_rand());

    //Rf_endEmbeddedR(0); // todo: end R

    //fprintf(stderr, "app exec...\n");
    //return a.exec();
    fprintf(stderr, "Success\n");
    return EXIT_SUCCESS;
}
