#ifndef GEX_IMPORT_FET_TEST_H
#define GEX_IMPORT_FET_TEST_H

#include "parserBase.h"
#include "parameterDictionary.h"
#include "stdf.h"
#include "queryable_by_test_number_store.h"

#include <qmap.h>
#include <qdatetime.h>
#include <qstringlist.h>
#include <QScopedPointer>
#include <QFile>

#define FET_TEST_BLOCK_SIZE	1536

namespace GS
{
namespace Parser
{

class CGFET_TESTtoSTDF: public ParserBase
{
public:
    CGFET_TESTtoSTDF();
    ~CGFET_TESTtoSTDF();

    static bool IsCompatible(const QString& szFileName);

private:
    bool            ConvertoStdf(const QString& FetTestFileName,  QString& strFileNameSTDF);
    ConverterStatus GetConvertStatus() const;

    int     GetLookupTestNumber(int aTestNumber) const;
    bool	GetFetTestName(int aTestNumber, QString & aTestName);
    bool	openFile(QFile &fFile, QIODevice::OpenModeFlag openMode,QString &strFileName, int iTimeoutSec=2);
    bool	ReadBinMapFile(QString strBinmapFileName, QString strType, QString strFormat);
    bool	ReadBinMapFile_HVM_WS(QString strBinmapFileName);
    bool	ReadBinMapFile_LVM_WS(QString strBinmapFileName);
    bool	ReadBinMapFile_HVM_FT(QString strBinmapFileName);
    bool	ReadBinMapFile_LVM_FT(QString strBinmapFileName);
    ConverterStatus ReadFetTestFile(const QString& FetTestFileName, QString &strFileNameSTDF);
    ConverterStatus WriteStdfFile(QFile *hFile, QString &strFileNameSTDF);

    int		ReadBlock(QFile* pFile, char *data, qint64 len);

    bool ReadTesFile(const char *TestFileName);
    bool ReadCprFile(const char *CprFileName);

    bool RetrieveTestName(int aTestNumber, QString &aOutputTestName );
    ConverterStatus IssueErrorForInexistingBinMapItem(int alookupTestNumber);
    bool RetrieveBinInfo(int aTestNumber, int &aBinSoft, QString& aBinName ) const;
    void UpdateSoftBinMap(int aBinNum, const QString &aBinName , bool lPassStatus);

    ConverterStatus     mConverterStatus;

    QString		m_strDataFilePath;
    long		m_lStartTime;				// Startup time
    char		m_szBlock[FET_TEST_BLOCK_SIZE];
    int			m_iSSN;
    int			m_iESN;
    int			m_iSNNUM;
    int			m_iSNSIZE;
    int			m_iTestNumber;
    int			m_iSegNumber;
    int			m_iTestN[32];
    int			m_iFcnN[32];
    QString		m_strDataFileRem;
    QString		m_strDataFileDate;
    int			m_iStartSegment;
    int			m_iEndSegment;
    bool		m_bWaferMap;
    QString		m_strWaferId;
    int			m_iStartingX;
    int			m_iStartingY;
    int			m_iEndingX;
    int			m_iEndingY;
    QString		m_strFetTestFileName;
    QString		m_strRunFile;
    QString		m_strTestFile;
    QString		m_strOperator;
    QString		m_strLotId;
    QString		m_strSubLotId;
    QString		m_strProber;
    int			m_nFailedTest;

    QMap<int, ParserBinning*> m_qMapBins;
    QMap<int, ParserBinning*> m_qMapBins_Soft;
    QMap<int, ParserParameter*> m_qMapParameterList;	// List of Parameters

    bool		m_bPromisFtFile;	// Set to true if GEX_PROMIS_FT_DATA_PATH is defined, and an entry found in the specified PROMIS file
    bool        mIsNewFormat;
    int         mStartLargeSN;
    int         mEndLargeSN;

    QScopedPointer< Qx::BinMapping::QueryableByTestNumber > mBinMapStore;

    enum FET_FLOW_E
    {
       FET_FLOW_UNDEF,
       FET_FLOW_HVM_WS,
       FET_FLOW_HVM_FT,
       FET_FLOW_LVM_WS,
       FET_FLOW_LVM_FT
    };
    FET_FLOW_E m_specificFlowType;
    ConverterStatus partFailedUpdateMapping(bool aPartIsPass, bool aAllTestsPass, int aBin_Soft, int aiBin);
};
}
}

#endif
