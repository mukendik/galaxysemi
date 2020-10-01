#ifndef FUJITSU_PARSER_H
#define FUJITSU_PARSER_H
#include<QString>
#include <QDate>
#include <stdfrecords_v4.h>
class QTextStream;

class Parser93kTab
{
public:
    enum  CGKVDXMLtoSTDF_errCodes
    {
        errNoError,			// No erro (default)
        errOpenFail,		// Failed Opening  file
        errInvalidFujitsuParserFormat,	// Invalid format
        errFormatNotSupported,	// Format not supported
        errLicenceExpired,	// File date out of Licence window!
        errWriteSTDF		// Failed creating STDF intermediate file
    };

    Parser93kTab();
    ~Parser93kTab();
    bool        Convert(const QString &szKVDFileName, const QString &strFileNameSTDF);
    static bool	IsCompatible(const QString &szFileName);
public:
    QString     GetLastError();
    int         GetLastErrorCode();



private:
    void init();
    static void setError(int iError, const QString &strAdditionalInfo = QString());
    static QString m_strLastError;	// Holds last error string during convertion
    static int		m_iLastError;			// Holds last error ID
private:

    bool WriteStdfFile(QTextStream *hSemi_G85File,const QString &strFileNameSTDF);

private:
    GQTL_STDF::Stdf_MIR_V4  m_oMIRRecord;
    GQTL_STDF::Stdf_MRR_V4  m_oMRRRecord;

    QMap<QString, int> m_oTestNumbers;
    int m_iTestNumberCount;

    QMap<int, QList<GQTL_STDF::Stdf_PTR_V4*> > m_oPTRList;
    QMap<int, QList<GQTL_STDF::Stdf_MPR_V4*> > m_oMPRList;
    QMap<int, QMap<int, QStringList> > m_oMPRPINList;

    QList<GQTL_STDF::Stdf_PIR_V4*> m_oPIRList;
    QList<GQTL_STDF::Stdf_PRR_V4*> m_oPRRList;
    QMap<int, int> m_oBinningMap;
    QList<GQTL_STDF::Stdf_HBR_V4*> m_oHBRList;
    QList<GQTL_STDF::Stdf_SBR_V4*> m_oSBRList;

private:
    bool processPTR(const QStringList &oTestProp, QList<GQTL_STDF::Stdf_PTR_V4*> &oPTRRecords);
    bool processMPR(const QStringList &oFirstTestProp,
                    QTextStream &oTextStream,
                    QList<GQTL_STDF::Stdf_MPR_V4*> &oMPRRecords,
                    QMap<int, QStringList> &oMPRPINList,
                    bool &bReadNextLine, QString &strNextLine);
    void writePinList(const QStringList &oPintList, stdf_type_u1 u1Head, stdf_type_u1 u1Site ,GS::StdLib::Stdf &oStdfFile);

private:
    void setProgress(double dVal, const QString &strMessage= QString());
    int getProgress();
};

#endif // FUJITSU_PARSER_H
