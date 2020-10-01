#ifndef KVD_XML_TO_STDF_H
#define KVD_XML_TO_STDF_H

#include <QMap>
#include <QList>
#include <QString>
#include <QVariant>
#include <QDomNode>
#include <QStringList>
#include <stdfrecords_v4.h>

#include <wafermap.h>

class CTest;
class QDate;
class QTextStream;
class CBinning;


class CGKVDXMLtoSTDF
{
public:
    enum DataType{
        Unkown,
        DieSort,// WS and FT
        Scribeline,// PT DATA
        Matlhist//Material History data.
    };
    enum  CGKVDXMLtoSTDF_errCodes
    {
        errNoError,			// No erro (default)
        errOpenFail,		// Failed Opening  file
        errInvalidXMLFormat,	// Invalid format
        errInvalidMaximKvdFormat,	// Invalid format
        errFormatNotSupported,	// Format not supported
        errLicenceExpired,	// File date out of Licence window!
        errWriteSTDF		// Failed creating STDF intermediate file
    };

public:
    CGKVDXMLtoSTDF();
    ~CGKVDXMLtoSTDF();
    //! \brief ?
    bool Convert(const QString &szKVDFileName, const QString &strFileNameSTDF);
    static bool	IsCompatible(const QString &szFileName);

public:
    QString GetLastError();
    int     GetLastErrorCode();
private:
    void init();
    static void setError(int iError, const QString &strAdditionalInfo = QString(), const QDomNode &oDomNode = QDomNode());
    static QString m_strLastError;	// Holds last error string during convertion
    static int		m_iLastError;			// Holds last error ID

protected:

    bool readMaxvisionSection(const QDomNode & oMaxvisionElem);

    bool readTestOccurrenceSection(const QDomNode & oTestOccurrence);

    bool readATESection(const QDomNode &oATEElem);

    bool readWaferConfigurationSection(const QDomNode &oWaferConfigElem);

    bool readTestProgramSection(const QDomNode & oTestProgramElem);
    bool readBinSet(const QDomNode &oBinSetElem);
    bool readTestFlowSection(const QDomNode &oTestFlowElem);

    bool readLotUnderTestSection(const QDomNode &);
    bool readWaferUnderTestSection(const QDomNode & oWaferUnderTestElem);

    bool readPartCount(const QDomNode & oPartCountElem);

    bool readElementText(const QDomNode &oDomNode,
                        const QString &strItem,
                        bool bRequired,
                        QMap <QString, QVariant> &oMap, const QString &strDefault = QString());
    bool readElementText(const QDomNode &oDomNode,
                        const QString &strItem,
                        bool bRequired,
                        QString &strData, const QString &strDefault = QString());
    bool readElementTextAttribute(const QDomElement &oDomElement,
                                        const QString &strAttribute,
                                        bool bRequired,
                                        QString &strData, const QString &strDefault= QString());
    bool readElementTextAttribute(const QDomElement &oDomElement,
                                        const QString &strAttribute,
                                        bool bRequired,
                                        QMap<QString, QVariant> &oMap, const QString &strDefault= QString());
    //! \brief parse "dd-MMM-yyyy" ?
    //! \return 0 if string empty or the time_t value
    time_t getTime(const QString &strDateTime);
    char getModeCodes(const QString &strFiledData);
    char getWF_FLAT(const QString &strFiledData);
    bool insertNewBinning(QList <CBinning *> &oBinsList, CBinning **poNewBin);
    CBinning *getBinning(QList <CBinning *> &oBinsList, int iBin);
    CTest *getTest(QList <CTest *> &oTestList, unsigned int iTestID);
    char getTestFlag(const QString &strResult);

    void setProgress(double dVal, const QString &strMessage= QString());
    int getProgress();

protected:
    struct CGKVDXMLMaxvisionSection{
        QMap <QString, QVariant> m_oMaxvisionXMLProp;
        QStringList m_oLibraryAttributes;
        void clear(){
            m_oMaxvisionXMLProp.clear();
            m_oLibraryAttributes.clear();
        }

    };

    struct CGKVDXMLATESection{
        QMap <QString, QVariant> m_oATEXMLProp;
        QList<QMap <QString, QVariant> > m_oSiteGroupList;
        void clear(){
            m_oATEXMLProp.clear();
            m_oSiteGroupList.clear();
        }

    };

protected:

    QList<QMap <QString, QVariant> > m_oWaferTestSummaryList;
    CWaferMap m_oWaferMap;
    QMap <QString, QVariant> m_oLotConfiguration;
    QList <CTest *> m_oTestFlow;
    QList <CBinning *> m_oSoftBins;
    QList <CBinning *> m_oHardBins;
    QMap<int, int> m_oBinMapping;
    CGKVDXMLATESection m_oATESection;
    CGKVDXMLMaxvisionSection m_oMaxvisionSection;
    QMap <QString, QVariant> m_oWaferConfiguration;
    GQTL_STDF::Stdf_MIR_V4  m_oMIRRecord;
    GQTL_STDF::Stdf_MRR_V4  m_oMRRRecord;
    GQTL_STDF::Stdf_PCR_V4  m_oPCRRecord;
    bool m_bPCRExist ;
    bool m_bValidCoord ;

    QList<GQTL_STDF::Stdf_PRR_V4>  m_oPRRecordList;
    QList<GQTL_STDF::Stdf_PIR_V4>  m_oPIRecordList;

    DataType m_eDataType;


private:

    bool WriteStdfFile(QTextStream *hSemi_G85File,const QString &strFileNameSTDF);


};

#endif // KVD_XML_TO_STDF_H
