#ifndef GEX_IMPORT_PCM_DONGBU_H
#define GEX_IMPORT_PCM_DONGBU_H

#include <qdatetime.h>
#include <qmap.h>
#include <qstringlist.h>
#include <QTextStream>
#include "parserParameter.h"
#include "parserBase.h"

#include "stdf.h"

namespace GS
{
namespace Parser
{

class PCM_DONGBUtoSTDF: public ParserBase
{
public:
    PCM_DONGBUtoSTDF();
    ~PCM_DONGBUtoSTDF();
    bool ConvertoStdf(const QString &PcmDongbuFileName, QString &strFileNameSTDF);

    static bool	IsCompatible(const QString& fileName);

private:
    bool ReadPcmDongbuFile(const QString &PcmDongbuFileName, const QString &strFileNameSTDF);
    bool WriteStdfFile(QTextStream *pcmDongbuFile, const QString &strFileNameSTDF);

    enum  errCodes
	{
		errNoError,			// No erro (default)
		errOpenFail,		// Failed Opening PCM_DONGBU file
		errInvalidFormat,	// Invalid PCM_DONGBU format
        errInvalidFormatLowInRows,			// Didn't find parameter rows
		errNoLimitsFound,	// Missing limits...no a valid PCM_DONGBU file
		errLicenceExpired,	// File date out of Licence window!
		errWriteSTDF		// Failed creating STDF intermediate file
    };

    QList<ParserParameter>  mParameter;                     /// \param The list of parameters
    int						mTotalParameters;				// Holds the total number of parameters / tests in each part tested
    QStringList				mFullPcmDongbuParametersList;	// Complete list of ALL PCM_DONGBU parameters known.

    QString	mLotID;					// LotID string
    QString	mProductID;				// Product / Device name
    QString	mProgramID;				// Program name
    QString mOperatorName;              // operator name

};

}
}

#endif
