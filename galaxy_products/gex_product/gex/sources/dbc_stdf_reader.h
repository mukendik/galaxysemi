#ifndef DBC_STDF_READER_H
#define DBC_STDF_READER_H

#include <QObject>
#include <QMap>

#include "cstdfparse_v4.h"
#include "ctest.h"

#define GEX_TESTNBR_OFFSET_EXT_WAFERID	(GEX_TESTNBR_OFFSET_EXT+9)
#define GEX_TESTNBR_OFFSET_EXT_LOTID	(GEX_TESTNBR_OFFSET_EXT+10)
#define GEX_TESTNBR_OFFSET_EXT_PROCID	(GEX_TESTNBR_OFFSET_EXT+11)
#define GEX_TESTNBR_OFFSET_EXT_JOBNAME	(GEX_TESTNBR_OFFSET_EXT+12)
#define GEX_TESTNBR_OFFSET_EXT_SBLOTID	(GEX_TESTNBR_OFFSET_EXT+13)
#define GEX_TESTNBR_OFFSET_EXT_TSTRNAM	(GEX_TESTNBR_OFFSET_EXT+14)
#define GEX_TESTNBR_OFFSET_EXT_LOADID	(GEX_TESTNBR_OFFSET_EXT+15)
#define GEX_TESTNBR_OFFSET_EXT_FILENAME	(GEX_TESTNBR_OFFSET_EXT+16)

#define INVALID_SMALLINT					-32768

#define FLAG_TESTINFO_LL_STRICT				0x02
#define FLAG_TESTINFO_HL_STRICT				0x04

class CGate_EventManager;
class Gate_DatasetDef;
class Gate_SiteDescriptionMap;
class Gate_LotDef;
class Gate_ParameterDef;
class Gate_DataResult;
class TestKey;

class DbcStdfReader : public QObject
{
	Q_OBJECT
public:
	explicit DbcStdfReader(QObject *parent = 0);
	virtual ~DbcStdfReader();
	bool	read(const QString& strFileName, QString &strMessage);

public slots:

signals:
	void sBeginDataset();
	void sEndDataset();
	void sReadParameterInfo(const QMap<TestKey, Gate_ParameterDef>& mapParamInfo);
	void sReadStep(const QMap<TestKey, Gate_DataResult>& mapParamResultsBySite);
	void sCountedStep(int);

private:
	QString normalizeTestName(const QString &strTestName);
    bool isTestFail(const GQTL_STDF::CStdf_PTR_V4 &clRecord, const Gate_ParameterDef &cParameterDef);
	bool lastPTRInfo(QMap<TestKey, Gate_ParameterDef> &mapParamInfo, Gate_ParameterDef &cParameterDef);

	void	processPassOne(QMap<TestKey, Gate_ParameterDef> &mapParamInfo);
	bool	readMIRPassOne(QMap<TestKey, Gate_ParameterDef> &mapParamInfo);
	bool	readMRRPassOne(QMap<TestKey, Gate_ParameterDef> &mapParamInfo);
	bool	readSDRPassOne(QMap<TestKey, Gate_ParameterDef> &mapParamInfo);
	bool	readPIRPassOne();
	bool	readPRRPassOne(QMap<TestKey, Gate_ParameterDef> &mapParamInfo);
    Gate_ParameterDef readPTRInfo(const GQTL_STDF::CStdf_PTR_V4 &stdfParser_V4);
	bool	readPTRPassOne(QMap<TestKey, Gate_ParameterDef> &mapParamInfo);
	bool	readFTRPassOne(QMap<TestKey, Gate_ParameterDef> &mapParamInfo);
	bool	readMPRPassOne(QMap<TestKey, Gate_ParameterDef> &mapParamInfo);

	void	processPassTwo(QMap<TestKey, Gate_ParameterDef> &mapParamInfo);
	void	readMIRPassTwo(Gate_LotDef &cLotDef);
	void	readWIRPassTwo(Gate_LotDef &cLotDef);
	bool	readPTRPassTwo(QMap<TestKey, Gate_ParameterDef> &mapParamInfo, QMap<int, QMap<TestKey, Gate_DataResult> > &mapParamResultsBySite, const int &iRunId, int &iFlowId);
	bool	readFTRPassTwo(QMap<TestKey, Gate_ParameterDef> &mapParamInfo, QMap<int, QMap<TestKey, Gate_DataResult> > &mapParamResultsBySite, const int &iRunId, int &iFlowId);
	bool	readMPRPassTwo(QMap<TestKey, Gate_ParameterDef> &mapParamInfo, QMap<int, QMap<TestKey, Gate_DataResult> > &mapParamResultsBySite, const int &iRunId, int &iFlowId);
	bool	readPRRPassTwo(QMap<TestKey, Gate_ParameterDef> &mapParamInfo, QMap<int, QMap<TestKey, Gate_DataResult> > &mapParamResultsBySite, int &iRunId, int &iFlowId, const Gate_LotDef &cLotDef);

	QString			m_strFileName;
	int				m_nFieldFilter;					// Field filter (no fields, present fields...)
    GQTL_STDF::CStdfParse_V4	m_clStdfParse;					// STDF V4 parser
    GQTL_STDF::CStdf_MIR_V4	m_clStdfMIR;
    GQTL_STDF::CStdf_WIR_V4	m_clStdfWIR;
    GQTL_STDF::CStdf_FTR_V4	m_clStdfMRR;
    GQTL_STDF::CStdf_PTR_V4	m_clStdfPTR;
    GQTL_STDF::CStdf_FTR_V4	m_clStdfFTR;
    GQTL_STDF::CStdf_MPR_V4	m_clStdfMPR;
    GQTL_STDF::CStdf_PRR_V4	m_clStdfPRR;
    GQTL_STDF::CStdf_PIR_V4	m_clStdfPIR;
    GQTL_STDF::CStdf_SDR_V4	m_clStdfSDR;
	CTest			m_currentTest;
	int				m_lPass;
    int				m_nStdfRecordsCount[GQTL_STDF::CStdf_Record_V4::Rec_COUNT];
	CGate_EventManager *m_pEventManager;

	QList<QString>	m_lstPartId;
	QList<QString>	m_lstPartLocation;
};


#endif // DBC_STDF_READER_H
