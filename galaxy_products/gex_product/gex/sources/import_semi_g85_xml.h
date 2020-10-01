#ifndef GEX_IMPORT_SEMIG85XML_XML
#define GEX_IMPORT_SEMIG85XML_XML

#include <qdatetime.h>
#include <qmap.h>
#include <QTextStream>
#include "xml_parser.h"
#include "gex_constants.h"
#include "stdf.h"

class CSemiG85XmlBinInfo
{
public:
	CSemiG85XmlBinInfo();
	
	QString	strName;
	int		nBinNb;
	int		nNbCnt;
	int		nNbCntSummary;
	bool	bPass;
};

class CSemiG85XmlDevice
{
public:
	CSemiG85XmlDevice(int iPosX, int iPosY, int iBinResult, int iFlagResult)
	{
		m_iPosX			= iPosX;
		m_iPosY			= iPosY;
		m_iBinResult	= iBinResult;
		m_iFlagResult	= iFlagResult;
	}
	
	int		m_iPosX;
	int		m_iPosY;
	int		m_iBinResult;
	int		m_iFlagResult;
};


class CGSemiG85XmltoSTDF: public XML::Observer
{
public:
	CGSemiG85XmltoSTDF();
	~CGSemiG85XmltoSTDF();
    bool	Convert(const char *SemiG85XmlFileName, const char *strFileNameSTDF);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);

    bool WriteStdfFile(const char *strFileNameSTDF);
    void Notify();

private:

    bool	ReadSemiG85XmlFile(const char *SemiG85XmlFileName, const char *strFileNameSTDF);
     bool	WriteStdfFile(QTextStream *hSemiG85XmlFile,const char *strFileNameSTDF);

    QString			m_strCurrentLine;
	QFile*			m_pfSemiG85XmlFile;
	QTextStream*	m_phSemiG85XmlFile;
    XML::XMLParser  m_XMLManager;
    bool            m_bEndOfFile;


	QString m_strLastError;	// Holds last error string during SemiG85Xml->STDF convertion
	QString m_strLastErrorSpecification;	// Holds last error string during SemiG85Xml->STDF convertion
	int		m_iLastError;	// Holds last error ID
	enum  errCodes
	{
		errNoError,			// No erro (default)
		errOpenFail,		// Failed Opening SemiG85Xml file
		errInvalidFormat,	// Invalid SemiG85Xml format
		errLicenceExpired,	// File date out of Licence window!
		errWriteSTDF		// Failed creating STDF intermediate file
	};

	unsigned int	m_lStartTime;
	
	QString	m_strLotId;				
	QString	m_strDeviceId;			
	QString m_strOperatorName;
	QString m_strNodeName;
	int		m_nBinType;
	QString m_strBinNull;
	QString m_strStatus;
	bool	m_bHaveGoodBins;
	bool	m_bSaveBinCntSummary;

	QString	m_strWaferId;			
	float	m_fWaferSize;
	float	m_fWaferSizeX;
	float	m_fWaferSizeY;
	int		m_nWaferRows;			
	int		m_nWaferColumns;
	bool	m_bWaferCenter;
	int		m_nNbRefDies;
	int		m_nFirstRefPosX;
	int		m_nFirstRefPosY;
	char	m_cWfFlat;
	char	m_cPosX;
	char	m_cPosY;
	int		m_nWaferBytePerDie;
	QMap<int, CSemiG85XmlBinInfo*> m_qMapBins;


	QMap<QString,QString>	m_mapTagAttributes;
	QString					m_strTagValue;
	QString					m_strTagName;

};

#endif
