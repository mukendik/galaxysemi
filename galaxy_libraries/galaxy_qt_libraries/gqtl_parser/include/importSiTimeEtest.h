#ifndef GEX_IMPORT_SITIME_ETEST_H
#define GEX_IMPORT_SITIME_ETEST_H

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

class SiTimeEtesttoSTDF : public ParserBase
{
public:
    SiTimeEtesttoSTDF();
    ~SiTimeEtesttoSTDF();
    bool ConvertoStdf(const QString &inputFileName, QString &StdfFileName);
    static bool	IsCompatible(const QString &fileName);
//    std::list<std::string> GetListStdfFiles() const;

private:
    void SaveParameter(int iIndex,QString strName);
    bool ReadSiTimeFile(const QString& SiTimeFileName, const QString &outputSTDF);
    bool WriteStdfFile(QTextStream *SiTimeFilestream, const QString &fileNameSTDF);

//    bool RemoveOutputFiles();
    bool WriteStdfFile(QStringList& lAllLines);



    QString                 mLotId;            /// \param LotID
    QString                 mProductId;        /// \param ProductID

    int                     mTotalColumns;      /// \param Number of columns
    int                     mTotalParameters;   /// \param Number of parameters
    QList<ParserParameter>  mParameterList;     /// \param List of Parameters in Wafer
    QStringList             mOutputFiles;       /// \param The list of stdf output files
    GQTL_STDF::StdfParse    mStdfParse;
    ParameterDictionary     mParameterDirectory;
};

}
}

#endif // IMPORTSITIMEETEST

