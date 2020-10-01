#ifndef GEX_IMPORT_CSMC_SPDM_H
#define GEX_IMPORT_CSMC_SPDM_H

#include <qdatetime.h>
#include <qmap.h>
#include <qstringlist.h>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"

typedef QMap<int, float> fValueOfSiteMap;

class CGPcmSPDMParameter
{
public:
    QString strName;	// Parameter name. E.g: "Idd_Total_Shutdown"
    QString strUnits;	// Parameter units,E.g: "A"
    float	fLowLimit;	// Parameter Low Spec limit, E.g: 0.00004
    float	fHighLimit;	// Parameter High Spec limit, E.g: -0.00004
    bool	bValidLowLimit;		// Low limit defined
    bool	bValidHighLimit;	// High limit defined
    float	fValue;		// Parameter result
    bool	bStaticHeaderWritten;	// 'true' after first STDF PTR static header data written.
};


class CGCSMC_SPDMtoSTDF
{
public:
    CGCSMC_SPDMtoSTDF();
    ~CGCSMC_SPDMtoSTDF();

    ///\brief This function is a public function that is called from the extern code (other classes)
    /// and call the reader and the writer function.
    bool	Convert(const char *CsmcSpdmFileName, const char *strFileNameSTDF);
    QString GetLastError();

    ///\brief This function returns true if a file matchs with the format of the current parser. The spec must contain all mandatory fields.
    static bool	IsCompatible(const char *szFileName);

private:
    ///\brief Check if the date in the input file is validate with the license date

    void LoadParameterIndexTable(void);
    void DumpParameterIndexTable(void);
    void UpdateParameterIndexTable(QString strParamName);

    ///\brief This function reads the content of the input file according to the specification of the parser
    bool ReadCsmcSpdmFile(const char *CsmcSpdmFileName, const char *strFileNameSTDF);

    ///\brief This function writes the required records in the stdf format according to the readed fields.
    /// The specification must contains required records to be written in the stdf output file.
    bool WriteStdfFile(QTextStream *hCsmcSpdmFile,const char *strFileNameSTDF);

    QString ReadLine(QTextStream& hFile);

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    int	iProgressStep;
    int	iNextFilePos;
    int	iFileSize;

    QString strLastError;	// Holds last error string during CSMC_SPDM->STDF convertion
    int	iLastError;			// Holds last error ID
    enum  errCodes
    {
        errNoError,			// No erro (default)
        errOpenFail,		// Failed Opening CSMC_SPDM file
        errInvalidFormat,	// Invalid CSMC_SPDM format
        errInvalidFormatLowInRows,			// Didn't find parameter rows
        errNoLimitsFound,	// Missing limits...no a valid CSMC_SPDM file
        errLicenceExpired,	// File date out of Licence window!
        errWriteSTDF		// Failed creating STDF intermediate file
    };

    bool					m_bNewCsmcPmdParameterFound;	// set to true if CSMC_SPDM file has a Parameter name not in our reference table=> makes it to be updated
    CGPcmSPDMParameter *	m_pCGPcmSPDMParameter;		// List of Parameters tables.
    int						m_iTotalParameters;				// Holds the total number of parameters / tests in each part tested
    QStringList				m_pFullCsmcSpdmParametersList;	// Complete list of ALL CSMC_SPDM parameters known.

    long	m_lStartTime;				// Startup time
    QString	m_strLotID;					// LotID string
    QString	m_strProductID;				// Product / Device name
    QString	m_strProgramID;				// Program name
    QString mCsmcType;                  // The type of the parsed file (SPDM or DPDM)

};


#endif
