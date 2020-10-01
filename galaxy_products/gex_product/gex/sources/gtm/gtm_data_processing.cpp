#ifdef GCORE15334


#include <QFileDialog>
#include <QInputDialog>
#include <QPainter>

#include <math.h>
#include <time.h>
#if !defined unix && !defined __MACH__
  #include <io.h>
#endif
#include <sys/stat.h>

#include <gqtl_log.h>
#include <stdf.h>
#include <gex_pat_constants_extern.h>
#include <gex_constants.h>

#include "station.h"
#include "classes.h"
#include "ctest.h"
#include "patman_lib.h"
#include "pat_info.h"
#include "cstats.h"
#include "message.h"
#include "gtm_testerwindow.h"
#include "engine.h"
#include "product_info.h"
#include "clientnode.h"

//#include "DebugMemory.h" // must be the last include


unsigned COutlierSummary::sNumOfInstances=0;

COutlierSummary::COutlierSummary()
{
    iBaselineFails = 0;	// Holds total failures during baseline
    iProdFails = 0;		// Holds total production failures
}

/*
int Gtm_TesterWindow::GetTimeoutValue(int iTimeoutID)
{
    switch(iTimeoutID)
    {
        case GEX_TPAT_ALRM_TIME_INFINITE:	// Timeout: Infinite (production stopped)
            return -1;

        case GEX_TPAT_ALRM_TIME_NONE:		// Timeout: none (production continues)
        default:
            return 0;

        case GEX_TPAT_ALRM_TIME_30S:		// Timeout:  30 sec
            return 30;

        case GEX_TPAT_ALRM_TIME_1MIN:		// Timeout:  1 minute
            return 60;

        case GEX_TPAT_ALRM_TIME_2MIN:		// Timeout:  2 minutes
            return 2*60;

        case GEX_TPAT_ALRM_TIME_3MIN:		// Timeout:  3 minutes
            return 3*60;

        case GEX_TPAT_ALRM_TIME_4MIN:		// Timeout:  4 minutes
            return 4*60;

        case GEX_TPAT_ALRM_TIME_5MIN:		// Timeout:  5 minutes
            return 5*60;

        case GEX_TPAT_ALRM_TIME_10MIN:		// Timeout:  10 minutes
            return 10*60;

        case GEX_TPAT_ALRM_TIME_15MIN:		// Timeout:  15 minutes
            return 15*60;

        case GEX_TPAT_ALRM_TIME_30MIN:		// Timeout:  30 minutes
            return 30*60;

        case GEX_TPAT_ALRM_TIME_1H:			// Timeout:  1 hour
            return 3600;

        case GEX_TPAT_ALRM_TIME_2H:			// Timeout:  2 hours
            return 2*3600;

        case GEX_TPAT_ALRM_TIME_3H:			// Timeout:  3 hours
            return 3*3600;

        case GEX_TPAT_ALRM_TIME_4H:			// Timeout:  4 hours
            return 4*3600;

        case GEX_TPAT_ALRM_TIME_5H:			// Timeout:  5 hours
            return 5*3600;

        case GEX_TPAT_ALRM_TIME_6H:			// Timeout:  6 hours
            return 6*3600;
    }
}
*/

///////////////////////////////////////////////////////////
// Copy a file from one folder to another
bool Gtm_TesterWindow::CopyGivenFile(QString strFrom,QString strTo,bool bEraseAfterCopy/*=false*/)
{
#if defined unix || __MACH__
    size_t	lReadSize, lWriteSize;
    char 	*ptBuffer;
    QFile 	hTo(strTo);
    FILE	*hFromFile, *hToFile;

    // Erase destination if it exists.
    if(hTo.exists() == true)
    {
        // Make it writable...so we can erase it!
        chmod(strTo.toLatin1().constData(), 0777);
        if(hTo.remove(strTo)!= true)
            return false;	// Failed to erase destination file.
    }

    // Allocate buffer to perform the file copy.
    ptBuffer = (char *) malloc(1000000);
    if(ptBuffer == NULL)
        return false;	// Memory allocation failure.

    hFromFile = fopen(strFrom.toLatin1().constData(), "rb");
    if(hFromFile == NULL)
    {
        delete ptBuffer;
        return false;
    }

    hToFile = fopen(strTo.toLatin1().constData(), "wb");
    if(hToFile == NULL)
    {
        delete ptBuffer;
        fclose(hFromFile);
        return false;
    }

    while(!feof(hFromFile))
    {
        lReadSize = fread(ptBuffer, sizeof(char), 1000000, hFromFile);
        if(lReadSize > 0)
        {
            lWriteSize = fwrite(ptBuffer, sizeof(char), lReadSize, hToFile);
            if(lWriteSize != lReadSize)
            {
                delete ptBuffer;
                fclose(hFromFile);
                fclose(hToFile);
                return false;
            }
        }
        else if(ferror(hFromFile))
        {
            delete ptBuffer;
            fclose(hFromFile);
            fclose(hToFile);
            return false;
        }
    }
    delete ptBuffer;
    fclose(hFromFile);
    fclose(hToFile);
#else
    // Windows Copy (make destination writable first!).
    //_chmod(strTo.latin1(),_S_IREAD | _S_IWRITE); // Qt3
    _chmod(strTo.toLatin1().constData(),_S_IREAD | _S_IWRITE);
    // Do not fail if file exist: overwrite it!
    if(CopyFileA(strFrom.toLatin1().constData(), strTo.toLatin1().constData(), false) != true)
        return false;
#endif

    if(bEraseAfterCopy)
    {
        // Erase source
        QDir cDir;
        return cDir.remove(strFrom);
    }

    return true;
}

#if 0
///////////////////////////////////////////////////////////
// Disable PAT because of fatal alarm condition
///////////////////////////////////////////////////////////
void Gtm_TesterWindow::DisableDynamicPat(void)
{
    GSLOG(4, "Disabling Dynamic PAT...");
    cStation.SetStationStatus(CStation::STATION_DISABLED);

    // If tester requested to no longer receive alram notification, return now! (eg: if flushing last few runs when issuing end-of-lot)
    if(m_uiSilentMode)
        return;

    // Resend new test limits to tester (will be infinite as PAT is disabled!)
    clientPatInit_Dynamic();
}
#endif

#endif

