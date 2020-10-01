#ifndef GEX_IMPORT_SEMIE142XML_XML
#define GEX_IMPORT_SEMIE142XML_XML

#include <qdatetime.h>
#include <qmap.h>
#include <QTextStream>

#include "xml_parser.h"
#include "gex_constants.h"
#include "stdf.h"

class CSemiE142XmlLayoutInfo
{
public:
	CSemiE142XmlLayoutInfo()
	{
		nLowerLeftX = nLowerLeftY = 0;
		fDeviceSizeX = fDeviceSizeY = 0.0;
	};
	
	QString	strLayoutName;
	QString strProductId;
	QString strUnit;
	int		nLowerLeftX;
	int		nLowerLeftY;
	float	fDeviceSizeX;
	float	fDeviceSizeY;

};

class CSemiE142XmlBinInfo
{
public:
	CSemiE142XmlBinInfo();
	
	QString	strName;
	int		nBinNb;
	int		nNbCnt;
	int		nNbCntSummary;
	bool	bPass;
};

class CSemiE142XmlDevice
{
public:
	CSemiE142XmlDevice(int iPosX, int iPosY, int iBinResult, int iFlagResult)
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


class CGSemiE142XmltoSTDF: public XML::Observer
{
public:
	CGSemiE142XmltoSTDF();
	~CGSemiE142XmltoSTDF();
    bool	Convert(const char *SemiE142XmlFileName, const char *strFileNameSTDF);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);
    void Notify();


private:

    bool	ReadSemiE142XmlFile(const char *semiE142XmlFileName, const char *strFileNameSTDF);
    bool    WriteStdfFile(const char *strFileNameSTDF);
	bool	GotoMarker(const char *szEndMarker);
	QString	ReadNextTag();
	QString	ReadLine();
	QString			m_strCurrentLine;
	QTextStream*	m_phSemiE142XmlFile;
    XML::XMLParser  m_XMLManager;
    bool            m_bEndOfFile;



	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int	iProgressStep;
	int	iNextFilePos;
	int	iFileSize;

	QString m_strLastError;	// Holds last error string during SemiE142Xml->STDF convertion
	QString m_strLastErrorSpecification;
	int		m_iLastError;			// Holds last error ID
	enum  errCodes
	{
		errNoError,			// No erro (default)
		errOpenFail,		// Failed Opening SemiE142Xml file
		errInvalidFormat,	// Invalid SemiE142Xml format
		errFormatNotSupported,	// SemiE142Xml Format not supported (not a 2DWafer Map)
		errLicenceExpired,	// File date out of Licence window!
		errWriteSTDF		// Failed creating STDF intermediate file
	};

	// Save layouts information
	QString m_strCurrentLayout;
	QMap<QString,CSemiE142XmlLayoutInfo> m_mapChildLayouts;


	unsigned int	m_lStartTime;
	unsigned int	m_lEndTime;
	
	QString	m_strLotId;				
	QString	m_strProductId;			
	QString m_strOperatorName;
	QString m_strNodeName;
	QString m_strAuxFile;
	QString m_strProgramName;
	QString m_strProgramVer;
	QString m_strHandlerId;
	QString m_strProbeId;
	QString m_strLoadBoardId;

	int		m_nBinType;
	QString m_strBinNull;
	QString m_strStatus;
	bool	m_bHaveGoodBins;
	bool	m_bSaveBinCntSummary;

	QString	m_strWaferId;			
	float	m_fWaferSizeX;
	float	m_fWaferSizeY;
	int		m_nWaferUnit;			
	int		m_nWaferRows;			
	int		m_nWaferColumns;		
	int		m_nNbRefDies;
	int		m_nOriginPosX;
	int		m_nOriginPosY;
	int		m_nFirstRefPosX;
	int		m_nFirstRefPosY;
	char	m_cWfFlat;
	char	m_cPosX;
	char	m_cPosY;
	char	m_cOrgPosX;
	char	m_cOrgPosY;
	int		m_nWaferBytePerDie;
	QStringList m_strInvalidBinList;	// Used to store list of Bins not in PAss or Fail categories (eg: NullBin, UglyBin, etc)
	QMap<int, CSemiE142XmlBinInfo*> m_qMapBins;


	QMap<QString,QString>	m_mapTagAttributes;
	QString					m_strTagValue;
	QString					m_strTagName;

    uint BuildDate(QString strString);
};

#endif
