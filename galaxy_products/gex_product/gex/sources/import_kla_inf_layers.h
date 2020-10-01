#ifndef GEX_IMPORT_KLA_INF_LAYERS
#define GEX_IMPORT_KLA_INF_LAYERS

#include <qdatetime.h>
#include <qmap.h>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"

class CKlaInfLayersBinInfo
{
public:
	CKlaInfLayersBinInfo();

	QString	strName;
	int		nBinNb;
	int		nNbCnt;
	bool	bPass;
};

// Case 6531
class CGKlaInfLayersBinmap: public QMap<int, CKlaInfLayersBinInfo>
{
public:
    bool isGoodBin(int KlaBin)
    {
        if(contains(KlaBin)) return value(KlaBin).bPass;
        return (KlaBin==1);
    }
};

class CGKlaInfLayerstoSTDF
{
public:
	CGKlaInfLayerstoSTDF();
	~CGKlaInfLayerstoSTDF();
    bool	Convert(const char *KlaInfLayersFileName, const char *strFileNameSTDF);
	QString GetLastError();

	static bool	IsCompatible(const char *szFileName);

private:

    bool	ReadKlaInfLayersFile(const char *KlaInfLayersFileName,const char *strFileNameSTDF);
	bool	WriteStdfFile(const char *szFileNameSTDF);
	QString ReadLine(QTextStream& hFile);
	bool	GoToNextTag(QTextStream& hFile);
	bool	ReadAttributes(QTextStream& hFile);
	long	GetDateTimeFromString(QString strDateTime);

	QString					m_strCurrentLine;
	QString					m_strCurrentTag;
	QStringList				m_lstTagAttributes;
	QStringList				m_lstRowsData;

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	int	iProgressStep;
	int	iNextFilePos;
	int	iFileSize;

	QString m_strLastError;	// Holds last error string during KlaInfLayers->STDF convertion
	int		m_iLastError;			// Holds last error ID
	enum  errCodes
	{
		errNoError,			// No erro (default)
		errOpenFail,		// Failed Opening KlaInfLayers file
		errInvalidFormat,	// Invalid KlaInfLayers format
		errLicenceExpired,	// File date out of Licence window!
		errWriteSTDF		// Failed creating STDF intermediate file
	};

	float	m_fDieWidth;
	float	m_fDieHeight;
	float	m_fWaferDiameter;

	int		m_nFieldPacked;
	int		m_nFieldSize;
	int		m_nFieldBase;
	int		m_nWaferNotch;
	int		m_nWaferStepX;
	int		m_nWaferStepY;

	long	m_lStartTime;
	long	m_lEndTime;

	QString	m_strFieldOff;
	QString	m_strFieldOn;


	QString	m_strLotId;
	QString	m_strSubLot;
	QString	m_strDeviceId;
	QString m_strOperatorName;
	QString m_strNodeName;
	QString m_strJobName;
	QString m_strJobRev;
	QString m_strExecName;
	QString m_strExecRev;
	QString m_strHandId;
	QString m_strLoadId;
	QString m_strTstrType;
	QString m_strStatNum;
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
	QString	m_strUpperLeftXY;
	QString	m_strLowerRightXY;
    CGKlaInfLayersBinmap m_qMapBins;
};

#endif
