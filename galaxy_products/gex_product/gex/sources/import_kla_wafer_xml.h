#ifndef GEX_IMPORT_KlaWaferXml_WAFER_XML
#define GEX_IMPORT_KlaWaferXml_WAFER_XML

#include <qdatetime.h>
#include <qmap.h>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"

class CKlaWaferXmlBinInfo
{
public:
	CKlaWaferXmlBinInfo();
	
	QString	strName;
	int		nBinNb;
	int		nNbCnt;
	bool	bPass;
};

class CGKlaWaferXmltoSTDF
{
public:
	CGKlaWaferXmltoSTDF();
	~CGKlaWaferXmltoSTDF();
    bool	Convert(const char *KlaWaferXmlFileName, const char *strFileNameSTDF);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);

private:

    bool	ReadKlaWaferXmlFile(const char *KlaWaferXmlFileName, const char *strFileNameSTDF);
	bool	WriteStdfFile(QTextStream *hKlaWaferXmlFile,const char *strFileNameSTDF);
	bool	GotoMarker(const char *szEndMarker);
	QString	ReadNextTag();
	QString			m_strCurrentLine;
	QFile*			m_pfKlaFile;
	QTextStream*	m_phKlaWaferXmlFile;

	bool	m_bEndOfFile;



	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int	iProgressStep;
	int	iNextFilePos;
	int	iFileSize;

	QString m_strLastError;	// Holds last error string during KlaWaferXml->STDF convertion
	int		m_iLastError;			// Holds last error ID
	enum  errCodes
	{
		errNoError,			// No erro (default)
		errOpenFail,		// Failed Opening KlaWaferXml file
		errInvalidFormat,	// Invalid KlaWaferXml format
		errLicenceExpired,	// File date out of Licence window!
		errWriteSTDF		// Failed creating STDF intermediate file
	};

	unsigned int	m_lStartTime;
	
	QString	m_strLotId;				
	QString	m_strDeviceId;			
	QString m_strOperatorName;
	QString m_strJobName;
	QString m_strHandId;
	QString m_strLoadId;
	QString m_strTstrType;
	QString m_strStatNum;
	QString m_strSBLot;
	QString m_strDieSize;
	QString m_strEquipment;
	QString m_strProbId;
	QString m_strProcessId;
	QString m_strGrossDies;

	QString	m_strWaferId;			
	QString	m_strGoodBin;			
	QString m_strWaferSize;
	int		m_nWaferRows;			
	int		m_nWaferColumns;		
	char	m_cWfFlat;
	int		m_nWaferBytePerDie;
	QString m_strWaferNoValueDie;
	QString m_strWaferNoDie;
	QString	m_strUpperLeftXY;			
	QString	m_strLowerRightXY;			
	QMap<int, CKlaWaferXmlBinInfo*> m_qMapBins;
	bool	m_bBinDefInside;
	bool	m_bHaveBinSum;


	QMap<QString,QString>	m_mapTagAttributes;
	QString					m_strTagValue;
	QString					m_strTagName;

};

#endif
