#ifndef GEX_IMPORT_SEMI_G85_H
#define GEX_IMPORT_SEMI_G85_H

#include <qmap.h>
#include <qdatetime.h>
#include <qstringlist.h>
#include <QTextStream>

#include "importConstants.h"
#include "stdf.h"
#include "parserBase.h"


namespace GS
{
namespace Parser
{

class ParserBinningG85 : public ParserBinning
{
public:
    ParserBinningG85(QChar charBin);
};

class CGSEMI_G85toSTDF : public ParserBase
{
public:
	CGSEMI_G85toSTDF();
	~CGSEMI_G85toSTDF();

    static bool	IsCompatible(const QString &FileName);

private:
    bool ConvertoStdf(const QString &Semi_G85FileName,  QString &StdfFileName);
    bool WriteStdfFile(QTextStream &Semi_G85File, const QString &StdfFileName);

	QString m_strLastError;	// Holds last error string during SEMI_G85->STDF convertion
	QString m_strLastErrorSpecification;
	int		m_iLastError;			// Holds last error ID

	QStringList		m_lstDateTime;

	QString	m_strFabLotId;
	int		m_nWaferNb;
	QString	m_strWaferId;			//WAFER_ID = "B83541.1-01-D6"


	QString	m_strNullBin;			//NULL_BIN = "."
	int		m_nWaferRows;			//ROWS = 30
	int		m_nWaferColumns;		//COLUMNS = 47
	QString	m_strDeviceId;			//DEVICE = "ADI"
	QString	m_strPackageId;			//DEVICE = "ADI"
	QString	m_strCustomerName;		//CUSTOMER_NAME = "ADI"
	QString	m_strFormatRevision;	//FORMAT_REVISION = "ADI0811D"
	QString	m_strSupplierName;		//SUPPLIER_NAME = "AMS"
	QString	m_strLotId;				//LOT_ID = "B83541.1"
	QString m_strTesterType;		//TESTER_TYPE
	QString m_strHandlerId;
	QString m_strOperatorId;
	QString m_strHandlerType;
	QString m_strJobName;
	QString m_strJobRev;
	QString m_strTestTemp;
	QString m_strNodeName;

	int		m_nWaferSize;			//WAFER_SIZE = 200
	int		m_nFlatNotch;			//FLAT_NOTCH = 0
	char	m_cPosX;
	char	m_cPosY;
	int		m_nXSize;				//X_SIZE = 4075
	int		m_nYSize;				//Y_SIZE = 6015
	int		m_nNbRefDies;			//REF_DIES = 1
	int		m_nRefDieX;				//REF_DIE = 36 -2
	int		m_nRefDieY;				//REF_DIE = 36 -2
	int		m_nNbDies;				//DIES = 1114
	int		m_nNbBins;				//BINS = 13
    QMap<QChar, ParserBinningG85*> m_qMapBins;
};

}
}

#endif
