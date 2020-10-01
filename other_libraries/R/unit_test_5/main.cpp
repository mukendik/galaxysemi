//#include <QtCore/QCoreApplication>
#include <QApplication>
#include <gqtl_sysutils.h>

#include "R.h"
#include "Rmath.h"
#include "Rembedded.h"
#include "Rinternals.h"
#include "Rdefines.h"

#define GS_FLT_MAX_4BYTES 	3.402823466e+38F
#define GS_DBL_MAX_8BYTES 	1.7976931348623158e+308
#define	GEX_C_DOUBLE_NAN	(double) 1.7e308
#define STRING(x) #x
#define STRING2(x) STRING(x)

int main(int argc, char *argv[])
{
    printf("R_HOME=%s\n", getenv("R_HOME")?getenv("R_HOME"):"null");
    printf("R_LIBS=%s\n", getenv("R_LIBS")?getenv("R_LIBS"):"null");
    printf("R_PROFILE=%s\n", getenv("R_PROFILE")?getenv("R_PROFILE"):"null");

    printf("R_HOME_DEF=%s\n", STRING2(R_HOME_DEF) );
    printf("OSFOLDER=%s\n", STRING2(OSFOLDER) );

    char lDesiredRHome[255]="";
    sprintf(lDesiredRHome, "../r_home/%s", STRING2(R_HOME_DEF) );
    printf("Best R_HOME should be %s\n", lDesiredRHome);

    QString lRHomePath;
    // check if GALAXY_R_HOME is defined then use it
    lRHomePath = QString(qgetenv("GALAXY_R_HOME"));
    if (!lRHomePath.simplified().isEmpty())
        qputenv("R_HOME", lRHomePath.toLatin1().data());
    else
    {
        printf("Trying to setup R_HOME...\n");
        qputenv("R_HOME", lDesiredRHome);
    }
    printf("R_HOME=%s\n", getenv("R_HOME")?getenv("R_HOME"):"null");

    // exple: g:/galaxy_dev/other_libraries/R/win32
    char* lRLibs=getenv("R_LIBS");

    printf("main: R_LIBS=%s\n", lRLibs?lRLibs:"?");

    // needs define Win32 ?
    //char *lRDLLVersion=getDLLVersion();
    //printf("R dll version=%s\n", lRDLLVersion?lRDLLVersion:"?");

    printf("R_TempDir=%s\n", R_TempDir?R_TempDir:"?");

    QApplication a(argc, argv);
    
    // According to R official doc:
    /*
     char *argv[] = {"REmbeddedPostgres", "--gui=none", "--silent"};
     int argc = sizeof(argv)/sizeof(argv[0]);
      Rf_initEmbeddedR(argc, argv);
    */
    // on linux such sometimes appears:
    // Fatal error: you must specify '--save', '--no-save' or '--vanilla'
    // The command-line option --vanilla implies --no-site-file, --no-init-file, --no-environ and (except for R CMD) --no-restore
    // --silent ?

    printf("Preparing startup args: --gui=nano --vanilla ...\n");
    char *Rargv[] = {(char*)"REmbeddedPostgres", (char*)"--gui=none", (char*)"--vanilla", (char*)""};
    int Rargc = sizeof(Rargv)/sizeof(Rargv[0]);

    // When this is called, the environment variables such as R_HOME, R_PROFILE, R_LIBS should be appropriately set.
    printf("Rf_initEmbeddedR %d args...\n", Rargc);
    int lR=Rf_initEmbeddedR(Rargc, Rargv);
    printf("Rf_initEmbeddedR: %d\n", lR);

    printf("R_TempDir=%s\n", R_TempDir?R_TempDir:"?");

    // Checking mem used by process
    QMap<QString, QVariant> lMI=CGexSystemUtils::GetMemoryInfo(false, false);
    foreach(QString k, lMI.keys())
        printf("%s: %s \n", k.toLatin1().data(), lMI.value(k).toString().toLatin1().data() );

    // Let s try to create a huge vector without crashing
    printf("Allocating a big vector...\n");

    SEXP e=0;
    //SEXP lResult=0;
    // PROTECT or not ?
    //PROTECT(e = allocVector(INTSXP, 20000000));
    //SEXP e=NEW_CHARACTER(180000000);
    //GET_DIM(e)
    //GET_LENGTH(e)
    e = allocVector(REALSXP, 40000000); // INTSXP
    //R_tryEval(e, R_GlobalEnv, &errorOccurred);

    // if fail, R could output:
    // "Error: cannot allocate vector of length 2000000000"
    // "Error: cannot allocate vector of size 1.5Gb"
    // "Error: memory exhausted (limit reached?)" and crashs

    //R_FlushConsole();

    /*
     SEXP fun = Rf_findFun(Rf_install("allocVector"),  R_GlobalEnv);
     if (!fun)
     {
        printf("Cannot find function \n");
        return EXIT_FAILURE;
     }
     PROTECT(fun);
     e = allocVector(LANGSXP, 2);
     SETCAR(e, fun);
     SEXP arg1=0;
     SETCAR(CDR(e), arg1);
     SETCAR(e, Rf_install("allocVector")); // (REALSXP, 10000000)

    int errorOccurred=0;
    SEXP lResult=R_tryEval(e, R_GlobalEnv, &errorOccurred);
    // sometimes R outputs : "Error: bad value" and crahs

    //SEXP v=NEW_INTEGER(1000); // NEW_INTEGER is just calling allocVector...

    if (!e)
    {
        printf("allocVector returned null\n");
        return EXIT_SUCCESS;
    }
    */


    lMI=CGexSystemUtils::GetMemoryInfo(false, false);
    printf("Used mem by process: %s\n", lMI[QString("MemUsedByProcess")].toString().toLatin1().data() );

    system("pause");

    //SETCAR(e, Rf_install("foo"));
    /*
    int errorOccurred=0;
    SEXP lResult=R_tryEval(e, R_GlobalEnv, &errorOccurred);
    if (!lResult)
    {
        printf("R_tryEval returned null: errorOccurred=%d\n", errorOccurred);
        return EXIT_SUCCESS;
    }
    */
    R_FlushConsole();

    //printf("app exec...\n");
    //return a.exec();
    return EXIT_SUCCESS;
}
