#ifndef GEX_IMPORT_PCM_HJTC_H
#define GEX_IMPORT_PCM_HJTC_H

#include <qmap.h>
#include <qdatetime.h>
#include <qstringlist.h>

#include "stdf.h"

#include "parserBase.h"

namespace GS
{
namespace Parser
{

class ParserParameter;
class ParserWafer;

class CGPcmHjtctoSTDF : public ParserBase
{
public:
    CGPcmHjtctoSTDF();
    ~CGPcmHjtctoSTDF();

    bool ConvertoStdf(const QString& aPcmHjtcFileName, QString& aFileNameSTDF);

    static bool	IsCompatible(const QString& szFileName);

private:
    ParserParameter *FindParameterEntry(int iWaferID,int iSiteID,QString strParamName,QString strUnits="");
    void SaveParameterResult(int iWaferID,int iSiteID,QString strName,QString strUnits,QString strValue);
    void SaveParameterLimit(QString strName,QString strValue,int iLimit);
    //void NormalizeLimits(ParserParameter *pParameter);
    bool ReadPcmHjtcFile(const QString& PcmHjtcFileName);
    bool WriteStdfFile(const QString &strFileNameSTDF);

    enum eLimitType
    {
        eLowLimit,							// Flag to specify to save the PcmHjtc Parameter LOW limit
        eHighLimit							// Flag to specify to save the PcmHjtc Parameter HIGH limit
    };

    QStringList mCriticalTests;         /// \param List of critical tests
    QString		mProductID;				/// \param ProductID
    QString		mProcessID;				/// \param ProcessID
    QString		mSpecName;				/// \param Test specification name
    QString		mLotID;					/// \param LotID
    QString		mOperatorID;			/// \param ProcessID
    QString		mNodeName;				/// \param ProcessID
    long		mStartTime;				/// \param Startup time
    QList<ParserWafer*> mWaferList;		/// \param List of Wafers in PcmHjtc file

    int         mTotalParameters;       /// \param Number of parameters
};

}
}

#endif // GEX_IMPORT_PCM_HJTC_H
