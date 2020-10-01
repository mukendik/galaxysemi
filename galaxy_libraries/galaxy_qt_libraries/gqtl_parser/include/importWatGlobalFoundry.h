#ifndef GEX_IMPORT_WATGF_ETEST_H
#define GEX_IMPORT_WATGF_ETEST_H

#include <qdatetime.h>
#include <qstringlist.h>
#include <QTextStream>

//#include "gex_constants.h"
#include "parserParameter.h"
#include "stdfparse.h"
#include "parserBase.h"

namespace GS
{
namespace Parser
{

class WatGlobalFoundryEtesttoSTDF : public ParserBase
{
public:
    WatGlobalFoundryEtesttoSTDF();
    ~WatGlobalFoundryEtesttoSTDF();
    bool ConvertoStdf(const QString &aInputFileName, QString &aStdfFileName);
    static bool	IsCompatible(const QString &aFileName);

private:

    static bool ReadHeader(QMap<QString, QString> &aHeaders, QTextStream& aTxtSTream, QString &aErrorMessage);
    static void InitCompatibleKeys();

    void SaveParameter(int iIndex,QString strName);
    bool ReadWatGlobalFoundryFile(QTextStream &aTxtStream, const QString &aOutputSTDF);
    bool WriteStdfFile(QTextStream *aWatGlobalFoundryFilestream, const QString &aOutputSTDF);

    bool ProcessHeader(QTextStream &aTxtStream);

    int                     mTotalColumns;      /// \param Number of columns
    int                     mTotalParameters;   /// \param Number of parameters
    ParserParameter*        mTestsList;     /// \param List of Parameters in Wafer
    GQTL_STDF::Stdf_MIR_V4  mMIRRecord;
    GQTL_STDF::Stdf_ATR_V4  mATRRecord;
    GQTL_STDF::Stdf_WIR_V4  mWIRRecord;
    GQTL_STDF::Stdf_MRR_V4  mMRRRecord;
    GQTL_STDF::Stdf_WRR_V4  mWRRRecord;
    GQTL_STDF::Stdf_WCR_V4  mWCRRecord;
    static QVector<QString> mCompatibleKeys;
    QVector< QPair<int, QString> > mListTestPrograms;
    QString                 mTestPrograms;
    QStringList             mFlatOrientations;
    int                     mParameterOffset;       /// \param the offset of the first test. It depends on the presence of the column SlotId
    bool mExistSlotID;
    bool WriteTestProgram(int aPNPIndex, QTextStream &aTxtStream, const QString &aOutputSTDF);
    void RemoveCommaAtTheEnd(QString &aString);
};

}
}

#endif // IMPORTWATGFETEST

