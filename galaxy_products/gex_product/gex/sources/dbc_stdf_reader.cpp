
#include <QFile>
#include <QString>
#include <QDateTime>
#include <QFileInfo>

#include <gqtl_log.h>
#include "gate_event_manager.h"
#include "gex_constants.h"

#include "dbc_stdf_reader.h"




DbcStdfReader::DbcStdfReader(QObject *parent) :
		QObject(parent)
{
	// Init other options
    m_nFieldFilter = GQTL_STDF::CStdf_Record_V4::FieldFlag_Present;
    for(int i=0; i<GQTL_STDF::CStdf_Record_V4::Rec_COUNT; i++)
		m_nStdfRecordsCount[i] = 0;
}

DbcStdfReader::~DbcStdfReader()
{

}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
QString DbcStdfReader::normalizeTestName(const QString &strTestName)
{
	QString strName;
//	int iPos = strTestName.find("<>");
//	if(iPos>0)
//		strName = strTestName.left(iPos).simplified();
//	else
		strName = strTestName.simplified();

	return strName;
}

bool DbcStdfReader::read(const QString &strFileName, QString &strMessage)
{
	GSLOG(SYSLOG_SEV_DEBUG, QString("start %1 at %2.")
	       .arg(strFileName)
	       .arg(QTime::currentTime().toString("hh:mm:ss")));

	QMap<TestKey, Gate_ParameterDef>		mapParamInfo;// Map parameter infos
	
	m_lstPartId.clear();
	m_lstPartLocation.clear();
	m_strFileName = QFileInfo(strFileName).baseName();

	if (!QFile::exists(strFileName))
	{
		strMessage = "Error: " + strFileName + " doesn't exists!";
		return false;
	}
	if (m_clStdfParse.Open(strFileName.toLatin1().data()) == false)
	{
		strMessage = "Error: while open" + strFileName + " ";
		return false;
	}

	emit sBeginDataset();

	// Pass 1
	processPassOne(mapParamInfo);
    emit sCountedStep(m_nStdfRecordsCount[GQTL_STDF::CStdf_Record_V4::Rec_PRR]);
	emit sReadParameterInfo(mapParamInfo);

	if (m_clStdfParse.Open(strFileName.toLatin1().data()) == false)
	{
		strMessage = "Error: while open" + strFileName + " ";
		return false;
	}

	// Pass 2
	processPassTwo(mapParamInfo);
	emit sEndDataset();

	// Close input STDF file between passes.
	m_clStdfParse.Close();

	GSLOG(SYSLOG_SEV_DEBUG, QString("%1 read at %2.")
	       .arg(strFileName)
	       .arg(QTime::currentTime().toString("hh:mm:ss")));

	return true;
}

void DbcStdfReader::processPassOne(QMap<TestKey, Gate_ParameterDef> &mapParamInfo)
{
	int iRecordType;
	GSLOG(SYSLOG_SEV_DEBUG, "Pass 1");
	// Read one record from STDF file.
	int iStatus = m_clStdfParse.LoadNextRecord(&iRecordType);

    while((iStatus == GQTL_STDF::CStdfParse_V4::NoError) || (iStatus == GQTL_STDF::CStdfParse_V4::UnexpectedRecord))
	{
		// Process STDF record read.
		switch(iRecordType)
		{
            case GQTL_STDF::CStdf_Record_V4::Rec_FAR:
				break;
            case GQTL_STDF::CStdf_Record_V4::Rec_ATR:
				break;
            case GQTL_STDF::CStdf_Record_V4::Rec_MIR:
				readMIRPassOne(mapParamInfo);
				break;
            case GQTL_STDF::CStdf_Record_V4::Rec_RDR:
				break;
            case GQTL_STDF::CStdf_Record_V4::Rec_SDR:
				readSDRPassOne(mapParamInfo);
				break;
            case GQTL_STDF::CStdf_Record_V4::Rec_PMR:
				break;
            case GQTL_STDF::CStdf_Record_V4::Rec_PGR:
				break;
            case GQTL_STDF::CStdf_Record_V4::Rec_PIR:
				readPIRPassOne();
				break;
            case GQTL_STDF::CStdf_Record_V4::Rec_PRR:
				readPRRPassOne(mapParamInfo);
				break;
            case GQTL_STDF::CStdf_Record_V4::Rec_PTR:
				readPTRPassOne(mapParamInfo);
				break;
            case GQTL_STDF::CStdf_Record_V4::Rec_FTR:
				readFTRPassOne(mapParamInfo);
				break;
            case GQTL_STDF::CStdf_Record_V4::Rec_MPR:
				readMPRPassOne(mapParamInfo);
				break;
            case GQTL_STDF::CStdf_Record_V4::Rec_MRR:
				break;
            case GQTL_STDF::CStdf_Record_V4::Rec_UNKNOWN:
				break;
		}

		// Read one record from STDF file.
		iStatus = m_clStdfParse.LoadNextRecord(&iRecordType);
	};
}

bool DbcStdfReader::readMIRPassOne(QMap<TestKey, Gate_ParameterDef> &mapParamInfo)
{
	// Check if have already load MIR record
    if(m_nStdfRecordsCount[GQTL_STDF::CStdf_Record_V4::Rec_MIR] > 1)
	{
		// ignore this record
		GSLOG(SYSLOG_SEV_WARNING, "[Record#MIR] MIR record appears multiple time. Only one MIR is allowed! This record was ignored.");
		return true;
	}

    if(m_clStdfParse.ReadRecord((GQTL_STDF::CStdf_Record_V4 *)&m_clStdfMIR) == false)
	{
		// Error reading STDF file
		// Convertion failed.
		GSLOG(SYSLOG_SEV_ERROR, "reading STDF file. Convertion failed.");
		return false;
	}

    m_nStdfRecordsCount[GQTL_STDF::CStdf_Record_V4::Rec_MIR]++;

	// Define sParam Info
	Gate_ParameterDef cParameterDef;

	// File name
	cParameterDef.m_nParameterNumber = GEX_TESTNBR_OFFSET_EXT_FILENAME;
	cParameterDef.m_bTestType = 'S';
	cParameterDef.m_strName = "file_name";
	mapParamInfo.insert(TestKey(cParameterDef.m_nParameterNumber, cParameterDef.m_strName), cParameterDef);
	
	// Lot ID
	cParameterDef.m_nParameterNumber = GEX_TESTNBR_OFFSET_EXT_LOTID;
	cParameterDef.m_bTestType = 'S';
	cParameterDef.m_strName = "lot_id";
	mapParamInfo.insert(TestKey(cParameterDef.m_nParameterNumber, cParameterDef.m_strName), cParameterDef);
	
	// Sub Lot ID
	cParameterDef.m_nParameterNumber = GEX_TESTNBR_OFFSET_EXT_SBLOTID;
	cParameterDef.m_bTestType = 'S';
	cParameterDef.m_strName = "sub_lot_id";
	mapParamInfo.insert(TestKey(cParameterDef.m_nParameterNumber, cParameterDef.m_strName), cParameterDef);

	// Job Name
	cParameterDef.m_nParameterNumber = GEX_TESTNBR_OFFSET_EXT_JOBNAME;
	cParameterDef.m_bTestType = 'S';
	cParameterDef.m_strName = "job_name";
	mapParamInfo.insert(TestKey(cParameterDef.m_nParameterNumber, cParameterDef.m_strName), cParameterDef);
	
	// Testing Site
	cParameterDef.m_nParameterNumber = GEX_TESTNBR_OFFSET_EXT_TESTING_SITE;
	cParameterDef.m_bTestType = 'P';
	cParameterDef.m_strName = "site_num";
	mapParamInfo.insert(TestKey(cParameterDef.m_nParameterNumber, cParameterDef.m_strName), cParameterDef);
	
	// Fab Location
	/*cParameterDef.m_nParameterNumber = 0;
	cParameterDef.m_bTestType = 'S';
	cParameterDef.m_strName = "fab_location";
	mapParamInfo.insert(TestKey(cParameterDef.m_nParameterNumber, cParameterDef.m_strName), cParameterDef);*/
	
	// Product
	cParameterDef.m_nParameterNumber = GEX_TESTNBR_OFFSET_EXT_PARTID;
	cParameterDef.m_bTestType = 'S';
	cParameterDef.m_strName = "product_id";
	mapParamInfo.insert(TestKey(cParameterDef.m_nParameterNumber, cParameterDef.m_strName), cParameterDef);
	
	// Process
	cParameterDef.m_nParameterNumber = GEX_TESTNBR_OFFSET_EXT_PROCID;
	cParameterDef.m_bTestType = 'S';
	cParameterDef.m_strName = "process";
	mapParamInfo.insert(TestKey(cParameterDef.m_nParameterNumber, cParameterDef.m_strName), cParameterDef);
	
	// Tester name
	cParameterDef.m_nParameterNumber = GEX_TESTNBR_OFFSET_EXT_TSTRNAM;
	cParameterDef.m_bTestType = 'S';
	cParameterDef.m_strName = "tester_name";
	mapParamInfo.insert(TestKey(cParameterDef.m_nParameterNumber, cParameterDef.m_strName), cParameterDef);
	
	// Wafer ID
	cParameterDef.m_nParameterNumber = GEX_TESTNBR_OFFSET_EXT_WAFERID;
	cParameterDef.m_bTestType = 'S';
	cParameterDef.m_strName = "wafer_id";
	mapParamInfo.insert(TestKey(cParameterDef.m_nParameterNumber, cParameterDef.m_strName), cParameterDef);
	
	// Temperature
	cParameterDef.m_nParameterNumber = GEX_TESTNBR_OFFSET_EXT_TEMPERATURE;
	cParameterDef.m_bTestType = 'S';
	cParameterDef.m_strName = "temperature";
	mapParamInfo.insert(TestKey(cParameterDef.m_nParameterNumber, cParameterDef.m_strName), cParameterDef);
	
	// SoftBin
	cParameterDef.m_nParameterNumber = GEX_TESTNBR_OFFSET_EXT_SBIN;
	cParameterDef.m_bTestType = 'P';
	cParameterDef.m_strName = "soft_bin";
	mapParamInfo.insert(TestKey(cParameterDef.m_nParameterNumber, cParameterDef.m_strName), cParameterDef);
	
	// HardBin
	cParameterDef.m_nParameterNumber = GEX_TESTNBR_OFFSET_EXT_HBIN;
	cParameterDef.m_bTestType = 'P';
	cParameterDef.m_strName = "hard_bin";
	mapParamInfo.insert(TestKey(cParameterDef.m_nParameterNumber, cParameterDef.m_strName), cParameterDef);
	
	// X-COORD
	cParameterDef.m_nParameterNumber = GEX_TESTNBR_OFFSET_EXT_DIEX;
	cParameterDef.m_bTestType = 'P';
	cParameterDef.m_strName = "x_coord";
	mapParamInfo.insert(TestKey(cParameterDef.m_nParameterNumber, cParameterDef.m_strName), cParameterDef);
	
	// Y-COORD
	cParameterDef.m_nParameterNumber = GEX_TESTNBR_OFFSET_EXT_DIEY;
	cParameterDef.m_bTestType = 'P';
	cParameterDef.m_strName = "y_coord";
	mapParamInfo.insert(TestKey(cParameterDef.m_nParameterNumber, cParameterDef.m_strName), cParameterDef);
	
	
	// [...]

	return true;
}

bool DbcStdfReader::readMRRPassOne(QMap<TestKey, Gate_ParameterDef>& /*mapParamInfo*/)
{
    if(m_clStdfParse.ReadRecord((GQTL_STDF::CStdf_Record_V4 *)&m_clStdfMRR) == FALSE)
	{
		// Error reading STDF file
		// Convertion failed.
		return false;
	}
    m_nStdfRecordsCount[GQTL_STDF::CStdf_Record_V4::Rec_MRR]++;

	return true;
}

bool DbcStdfReader::readSDRPassOne(QMap<TestKey, Gate_ParameterDef>& /*mapParamInfo*/)
{
    if(m_clStdfParse.ReadRecord((GQTL_STDF::CStdf_Record_V4 *)&m_clStdfSDR) == FALSE)
	{
		// Error reading STDF file
		// Convertion failed.
		return false;
	}
	
    m_nStdfRecordsCount[GQTL_STDF::CStdf_Record_V4::Rec_SDR]++;
	return true;
}

bool DbcStdfReader::readPIRPassOne()
{
    if(m_clStdfParse.ReadRecord((GQTL_STDF::CStdf_Record_V4 *)&m_clStdfPIR) == false)
	{
		// Error reading STDF file
		return false;
	}

    m_nStdfRecordsCount[GQTL_STDF::CStdf_Record_V4::Rec_PIR]++;

	return true;
}

Gate_ParameterDef DbcStdfReader::readPTRInfo(const GQTL_STDF::CStdf_PTR_V4 &stdfParser_V4)
{
	Gate_ParameterDef cParameterDef;
	
	cParameterDef.ResetData();
	cParameterDef.m_bTestType = 'P';
	cParameterDef.m_nParameterNumber = stdfParser_V4.m_u4TEST_NUM;
	cParameterDef.m_strName = normalizeTestName(stdfParser_V4.m_cnTEST_TXT);
	cParameterDef.m_strUnits = stdfParser_V4.m_cnUNITS;
	//		cParameterDef.m_nParameterIndex = datasetDef.m_lTotalParameters;

	bool bSaveLL, bSaveHL;
	bSaveLL = bSaveHL = false;
	if(stdfParser_V4.m_b1OPT_FLAG & STDF_MASK_BIT1)	// valid flag
	{
		// Verify if have valid limits
		bSaveLL = (((stdfParser_V4.m_b1OPT_FLAG & STDF_MASK_BIT4) == 0)
					&& ((stdfParser_V4.m_b1OPT_FLAG & STDF_MASK_BIT6) == 0));
		
		bSaveHL = (((stdfParser_V4.m_b1OPT_FLAG & STDF_MASK_BIT5) == 0)
					&& ((stdfParser_V4.m_b1OPT_FLAG & STDF_MASK_BIT7) == 0));
		
		if (bSaveLL)
		{
			cParameterDef.m_bHasLowL = true;
			cParameterDef.m_lfLowL = stdfParser_V4.m_r4LO_LIMIT;
		}
		if (bSaveHL)
		{
			cParameterDef.m_bHasHighL = true;
			cParameterDef.m_lfHighL = stdfParser_V4.m_r4HI_LIMIT;
		}
		if(cParameterDef.m_strUnits.isEmpty())
			cParameterDef.m_strUnits = stdfParser_V4.m_cnUNITS;

		// Update Strict flag
		// Only for the first saved
		if (bSaveLL && bSaveHL)
		{
			// If LowLimit == HighLimit then no strict limits
			if((cParameterDef.m_bHasLowL)
				&& (cParameterDef.m_bHasHighL)
				&& (cParameterDef.m_lfLowL==cParameterDef.m_lfHighL))
			{
				cParameterDef.m_uiFlags &= ~FLAG_TESTINFO_LL_STRICT;
				cParameterDef.m_uiFlags &= ~FLAG_TESTINFO_HL_STRICT;
			}
			else
			{
				if((stdfParser_V4.m_b1PARM_FLG & STDF_MASK_BIT6) == 0)
					cParameterDef.m_uiFlags |= FLAG_TESTINFO_LL_STRICT;
				if((stdfParser_V4.m_b1PARM_FLG & STDF_MASK_BIT7) == 0)
					cParameterDef.m_uiFlags |= FLAG_TESTINFO_HL_STRICT;
			}
		}
	}
	
	return cParameterDef;
}

bool DbcStdfReader::readPTRPassOne(QMap<TestKey, Gate_ParameterDef> &mapParamInfo)
{
    if(m_clStdfParse.ReadRecord((GQTL_STDF::CStdf_Record_V4 *)&m_clStdfPTR) == false)
	{
		// Error reading STDF file
		return false;
	}

    m_nStdfRecordsCount[GQTL_STDF::CStdf_Record_V4::Rec_PTR]++;

//	QString strParamIndex = QString::number(m_clStdfPTR.m_u4TEST_NUM) + normalizeTestName(m_clStdfPTR.m_cnTEST_TXT);
	
	Gate_ParameterDef cParameterDef = readPTRInfo(m_clStdfPTR);
	
	bool bKnownParam = false;
	if (!cParameterDef.m_bHasHighL && !cParameterDef.m_bHasLowL)
	{
		bKnownParam = lastPTRInfo(mapParamInfo, cParameterDef);
	}
	
	if (!bKnownParam)
	{
		TestKey clTestKey(cParameterDef);
		// If new parameter
		if (!mapParamInfo.contains(clTestKey))
		{
			mapParamInfo.insert(clTestKey, cParameterDef);
		}
	}
	return true;
}

bool DbcStdfReader::readFTRPassOne(QMap<TestKey, Gate_ParameterDef>& /*mapParamInfo*/)
{

    if(m_clStdfParse.ReadRecord((GQTL_STDF::CStdf_Record_V4 *)&m_clStdfFTR) == false)
	{
		// Error reading STDF file
		return false;
	}

    m_nStdfRecordsCount[GQTL_STDF::CStdf_Record_V4::Rec_FTR]++;

	return true;
}

bool DbcStdfReader::readMPRPassOne(QMap<TestKey, Gate_ParameterDef>& /*mapParamInfo*/)
{
    if(m_clStdfParse.ReadRecord((GQTL_STDF::CStdf_Record_V4 *)&m_clStdfMPR) == false)
	{
		// Error reading STDF file
		return false;
	}

    m_nStdfRecordsCount[GQTL_STDF::CStdf_Record_V4::Rec_MPR]++;

	return true;
}

bool DbcStdfReader::readPRRPassOne(QMap<TestKey, Gate_ParameterDef>& /*mapParamInfo*/)
{
	QString strBuffer;
	QString strNull = "null";
    if(m_clStdfParse.ReadRecord((GQTL_STDF::CStdf_Record_V4 *)&m_clStdfPRR) == false)
	{
		// Error reading STDF file
		return false;
	}

    m_nStdfRecordsCount[GQTL_STDF::CStdf_Record_V4::Rec_PRR]++;

	// Check if it is a valid PRR
	if((m_clStdfPRR.m_u2HARD_BIN > 32767) && (m_clStdfPRR.m_u2SOFT_BIN > 32767))
		return true;

  //FIXME: not used ?
	/*bool	bPass = true;

	// Check if Pass/Fail flag valid
	if((m_clStdfPRR.m_b1PART_FLG & STDF_MASK_BIT4) == 0)
	{
		if(m_clStdfPRR.m_b1PART_FLG & STDF_MASK_BIT3)
			bPass = false;
	}*/


	QString strPartId = (m_clStdfPRR.m_cnPART_ID.length()!=0) ? m_clStdfPRR.m_cnPART_ID : strNull;
	QString strXCoord = (m_clStdfPRR.m_i2X_COORD != INVALID_SMALLINT) ? QString::number(m_clStdfPRR.m_i2X_COORD) : strNull;
	QString strYCoord = (m_clStdfPRR.m_i2Y_COORD != INVALID_SMALLINT) ? QString::number(m_clStdfPRR.m_i2Y_COORD) : strNull;

	// Prior ID -> coord XY
	if((strXCoord != strNull) && (strYCoord != strNull))
	{
		strBuffer = strXCoord + ":" + strYCoord;
		if (!m_lstPartLocation.contains(strBuffer))
			m_lstPartLocation.push_front(strBuffer);
	}
	// If no coord XY, ID is Part_ID
	else if(strPartId != strNull)
	{
		if (!m_lstPartId.contains(strPartId))
			m_lstPartId.push_front(strPartId);
	}

	return true;
}

void DbcStdfReader::processPassTwo(QMap<TestKey, Gate_ParameterDef> &mapParamInfo)
{
	/// If there are several times the same test for the same run and the same site, it keeps the last value
	QMap<int, QMap<TestKey, Gate_DataResult> > mapParamResultsBySite;

	int iRecordType, iRunId, iFlowId;
	Gate_LotDef cLotDef;

	GSLOG(SYSLOG_SEV_DEBUG, "Pass 2");
	// Read one record from STDF file.
	int iStatus = m_clStdfParse.LoadNextRecord(&iRecordType);
	iRunId = 1;
	iFlowId = 1;
    while((iStatus == GQTL_STDF::CStdfParse_V4::NoError) || (iStatus == GQTL_STDF::CStdfParse_V4::UnexpectedRecord))
	{
		// Process STDF record read.
		switch(iRecordType)
		{
            case GQTL_STDF::CStdf_Record_V4::Rec_MIR:
				readMIRPassTwo(cLotDef);
				break;
            case GQTL_STDF::CStdf_Record_V4::Rec_WIR:
				readWIRPassTwo(cLotDef);
				break;
            case GQTL_STDF::CStdf_Record_V4::Rec_PTR:
				readPTRPassTwo(mapParamInfo, mapParamResultsBySite, iRunId, iFlowId);
				break;
            case GQTL_STDF::CStdf_Record_V4::Rec_FTR:
				readFTRPassTwo(mapParamInfo, mapParamResultsBySite, iRunId, iFlowId);
				break;
            case GQTL_STDF::CStdf_Record_V4::Rec_PRR:
				readPRRPassTwo(mapParamInfo, mapParamResultsBySite, iRunId,iFlowId, cLotDef);
				break;
            case GQTL_STDF::CStdf_Record_V4::Rec_UNKNOWN:
				break;
		}
		// Read one record from STDF file.
		iStatus = m_clStdfParse.LoadNextRecord(&iRecordType);
	};
}


void DbcStdfReader::readMIRPassTwo(Gate_LotDef &cLotDef)
{
    if(m_clStdfParse.ReadRecord((GQTL_STDF::CStdf_Record_V4 *)&m_clStdfMIR) == false)
	{
		// Error reading STDF file
		// Convertion failed.
		GSLOG(SYSLOG_SEV_ERROR, "reading STDF file. Convertion failed.");
		return;
	}

	// Lot ID
	cLotDef.m_strLotID = m_clStdfMIR.m_cnLOT_ID;
	// Sub Lot ID
	cLotDef.m_strSubLotID = m_clStdfMIR.m_cnSBLOT_ID;
	// Job Name
	cLotDef.m_strJobName = m_clStdfMIR.m_cnJOB_NAM;
	// Fab Location
	/// TODO
	// Product
	cLotDef.m_strProductID = m_clStdfMIR.m_cnPART_TYP;
	// Process
	cLotDef.m_strProcId = m_clStdfMIR.m_cnPROC_ID;
	// Tester name
	cLotDef.m_strTesterName = m_clStdfMIR.m_cnNODE_NAM;
	// Temperature
	cLotDef.m_strTestTemperature = m_clStdfMIR.m_cnTST_TEMP;
}

void DbcStdfReader::readWIRPassTwo(Gate_LotDef &cLotDef)
{
    if(m_clStdfParse.ReadRecord((GQTL_STDF::CStdf_Record_V4 *)&m_clStdfWIR) == false)
	{
		// Error reading STDF file
		// Convertion failed.
		GSLOG(SYSLOG_SEV_ERROR, "reading STDF file. Convertion failed.");
		return;
	}
	
	// Wafer ID
	cLotDef.m_strWaferID = m_clStdfWIR.m_cnWAFER_ID;
}
bool DbcStdfReader::readPTRPassTwo(QMap<TestKey, Gate_ParameterDef> &mapParamInfo, QMap<int, QMap<TestKey, Gate_DataResult> > &mapParamResultsBySite, const int &iRunId, int &iFlowId)
{
    if(m_clStdfParse.ReadRecord((GQTL_STDF::CStdf_Record_V4 *)&m_clStdfPTR) == false)
	{
		// Error reading STDF file
		return false;
	}
	
	Gate_ParameterDef cParameterDef = readPTRInfo(m_clStdfPTR);
	
	bool bKnownParam = false;
	if (!cParameterDef.m_bHasHighL && !cParameterDef.m_bHasLowL)
	{
		bKnownParam = lastPTRInfo(mapParamInfo, cParameterDef);
	}
	
	TestKey clTestKey(cParameterDef);
	if (!bKnownParam)
	{
		if (!mapParamInfo.contains(clTestKey))
		{
			// Error no parameter def
			GSLOG(SYSLOG_SEV_ERROR, QString("no test for number:%1, name: %2.")
			                  .arg(QString::number(clTestKey.number()))
			                  .arg(clTestKey.name()));
			return false;
		}
	}

	Gate_DataResult clDataResult;

	clDataResult.m_nParameterIndex = mapParamInfo[clTestKey].m_nParameterIndex;
	clDataResult.m_uiProgramRunIndex = iRunId;
	clDataResult.m_nFlowId = iFlowId;
	clDataResult.m_nSiteNum = m_clStdfPTR.m_u1SITE_NUM;
	clDataResult.m_bTestFailed = isTestFail(m_clStdfPTR, mapParamInfo[clTestKey]);

	bool bExecuted = (m_clStdfPTR.m_b1TEST_FLG & STDF_MASK_BIT4) == 0;
	bool bValidResult = (m_clStdfPTR.m_b1TEST_FLG & STDF_MASK_BIT1) == 0;
	clDataResult.m_bValidValue = (bExecuted && bValidResult);

	if (clDataResult.m_bValidValue)
		clDataResult.m_lfValue = m_clStdfPTR.m_r4RESULT;

	mapParamResultsBySite[clDataResult.m_nSiteNum].insert(clTestKey, clDataResult);

	iFlowId++;
	return true;
}

bool
DbcStdfReader::
readFTRPassTwo(QMap<TestKey, Gate_ParameterDef>& /*mapParamInfo*/,
			   QMap<int, QMap<TestKey, Gate_DataResult> >&
			   /*mapParamResultsBySite*/,
			   const int& /*iRunId*/,
			   int& iFlowId)
{
    if(m_clStdfParse.ReadRecord((GQTL_STDF::CStdf_Record_V4 *)&m_clStdfFTR) == false)
	{
		// Error reading STDF file
		return false;
	}

	iFlowId++;
	return true;
}

bool
DbcStdfReader::
readMPRPassTwo(QMap<TestKey, Gate_ParameterDef>& /*mapParamInfo*/,
			   QMap<int, QMap<TestKey, Gate_DataResult> >&
			   /*mapParamResultsBySite*/,
			   const int& /*iRunId*/,
			   int& iFlowId)
{
    if(m_clStdfParse.ReadRecord((GQTL_STDF::CStdf_Record_V4 *)&m_clStdfMPR) == false)
	{
		// Error reading STDF file
		return false;
	}

	iFlowId++;
	return true;
}

bool DbcStdfReader::readPRRPassTwo(QMap<TestKey, Gate_ParameterDef> &mapParamInfo, QMap<int, QMap<TestKey, Gate_DataResult> > &mapParamResultsBySite, int &iRunId, int &iFlowId, const Gate_LotDef &cLotDef)
{
    if(m_clStdfParse.ReadRecord((GQTL_STDF::CStdf_Record_V4 *)&m_clStdfPRR) == false)
	{
		// Error reading STDF file
		return false;
	}

	// If we have test to record on this site
//	if (mapParamResultsBySite.contains(m_clStdfPRR.m_u1SITE_NUM))
//	{
		// Add sParams Results
		Gate_DataResult clDataResult;

		// File name
		TestKey clTestKey(GEX_TESTNBR_OFFSET_EXT_FILENAME, "file_name");
		clDataResult.m_nParameterIndex = mapParamInfo[clTestKey].m_nParameterIndex;
		clDataResult.m_strValue = m_strFileName;
		clDataResult.m_nFlowId = -1;
		mapParamResultsBySite[m_clStdfPRR.m_u1SITE_NUM].insert(clTestKey, clDataResult);
		
		// Lot ID
		clTestKey = TestKey(GEX_TESTNBR_OFFSET_EXT_LOTID, "lot_id");
		clDataResult.m_nParameterIndex = mapParamInfo[clTestKey].m_nParameterIndex;
		clDataResult.m_strValue = cLotDef.m_strLotID;
		clDataResult.m_nFlowId = -1;
		mapParamResultsBySite[m_clStdfPRR.m_u1SITE_NUM].insert(clTestKey, clDataResult);
		
		// Sub Lot ID
		clTestKey = TestKey(GEX_TESTNBR_OFFSET_EXT_SBLOTID, "sub_lot_id");
		clDataResult.m_nParameterIndex = mapParamInfo[clTestKey].m_nParameterIndex;
		clDataResult.m_strValue = cLotDef.m_strSubLotID;
		clDataResult.m_nFlowId = -1;
		mapParamResultsBySite[m_clStdfPRR.m_u1SITE_NUM].insert(clTestKey, clDataResult);
	
		// Job Name
		clTestKey = TestKey(GEX_TESTNBR_OFFSET_EXT_JOBNAME, "job_name");
		clDataResult.m_nParameterIndex = mapParamInfo[clTestKey].m_nParameterIndex;
		clDataResult.m_strValue = cLotDef.m_strJobName;
		clDataResult.m_nFlowId = -1;
		mapParamResultsBySite[m_clStdfPRR.m_u1SITE_NUM].insert(clTestKey, clDataResult);
		
		// Testing Site
		clTestKey = TestKey(GEX_TESTNBR_OFFSET_EXT_TESTING_SITE, "site_num");
		clDataResult.m_nParameterIndex = mapParamInfo[clTestKey].m_nParameterIndex;
		clDataResult.m_lfValue = m_clStdfPRR.m_u1SITE_NUM;
		clDataResult.m_nFlowId = -1;
		mapParamResultsBySite[m_clStdfPRR.m_u1SITE_NUM].insert(clTestKey, clDataResult);
		
		// Fab Location
/*		clTestKey(0, "fab_location");
		clDataResult.m_nParameterIndex = mapParamInfo[clTestKey].m_nParameterIndex;
		clDataResult.m_strValue = cLotDef.m_strLotID;
		clDataResult.m_nFlowId = -1;
		mapParamResultsBySite[m_clStdfPRR.m_u1SITE_NUM].insert(clTestKey, clDataResult);*/
		
		// Product
		clTestKey = TestKey(GEX_TESTNBR_OFFSET_EXT_PARTID, "product_id");
		clDataResult.m_nParameterIndex = mapParamInfo[clTestKey].m_nParameterIndex;
		clDataResult.m_strValue = cLotDef.m_strProductID;
		clDataResult.m_nFlowId = -1;
		mapParamResultsBySite[m_clStdfPRR.m_u1SITE_NUM].insert(clTestKey, clDataResult);
		
		// Process
		clTestKey = TestKey(GEX_TESTNBR_OFFSET_EXT_PROCID, "process");
		clDataResult.m_nParameterIndex = mapParamInfo[clTestKey].m_nParameterIndex;
		clDataResult.m_strValue = cLotDef.m_strProcId;
		clDataResult.m_nFlowId = -1;
		mapParamResultsBySite[m_clStdfPRR.m_u1SITE_NUM].insert(clTestKey, clDataResult);
		
		// Tester name
		clTestKey = TestKey(GEX_TESTNBR_OFFSET_EXT_TSTRNAM, "tester_name");
		clDataResult.m_nParameterIndex = mapParamInfo[clTestKey].m_nParameterIndex;
		clDataResult.m_strValue = cLotDef.m_strTesterName;
		clDataResult.m_nFlowId = -1;
		mapParamResultsBySite[m_clStdfPRR.m_u1SITE_NUM].insert(clTestKey, clDataResult);
		
		// Wafer ID
		clTestKey = TestKey(GEX_TESTNBR_OFFSET_EXT_WAFERID, "wafer_id");
		clDataResult.m_nParameterIndex = mapParamInfo[clTestKey].m_nParameterIndex;
		clDataResult.m_strValue = cLotDef.m_strWaferID;
		clDataResult.m_nFlowId = -1;
		mapParamResultsBySite[m_clStdfPRR.m_u1SITE_NUM].insert(clTestKey, clDataResult);
		
		// Temperature
		clTestKey = TestKey(GEX_TESTNBR_OFFSET_EXT_TEMPERATURE, "temperature");
		clDataResult.m_nParameterIndex = mapParamInfo[clTestKey].m_nParameterIndex;
		clDataResult.m_strValue = cLotDef.m_strTestTemperature;
		clDataResult.m_nFlowId = -1;
		mapParamResultsBySite[m_clStdfPRR.m_u1SITE_NUM].insert(clTestKey, clDataResult);
		
		// SoftBin
		clTestKey = TestKey(GEX_TESTNBR_OFFSET_EXT_SBIN, "soft_bin");
		clDataResult.m_nParameterIndex = mapParamInfo[clTestKey].m_nParameterIndex;
		clDataResult.m_lfValue = m_clStdfPRR.m_u2SOFT_BIN;
		clDataResult.m_nFlowId = -1;
		mapParamResultsBySite[m_clStdfPRR.m_u1SITE_NUM].insert(clTestKey, clDataResult);
		
		// HardBin
		clTestKey = TestKey(GEX_TESTNBR_OFFSET_EXT_HBIN, "hard_bin");
		clDataResult.m_nParameterIndex = mapParamInfo[clTestKey].m_nParameterIndex;
		clDataResult.m_lfValue = m_clStdfPRR.m_u2HARD_BIN;
		clDataResult.m_nFlowId = -1;
		mapParamResultsBySite[m_clStdfPRR.m_u1SITE_NUM].insert(clTestKey, clDataResult);
		
		// X_COORD
		clTestKey = TestKey(GEX_TESTNBR_OFFSET_EXT_DIEX, "x_coord");
		clDataResult.m_nParameterIndex = mapParamInfo[clTestKey].m_nParameterIndex;
		clDataResult.m_lfValue = m_clStdfPRR.m_i2X_COORD;
		clDataResult.m_nFlowId = -1;
		mapParamResultsBySite[m_clStdfPRR.m_u1SITE_NUM].insert(clTestKey, clDataResult);
		
		// Y_COORD
		clTestKey = TestKey(GEX_TESTNBR_OFFSET_EXT_DIEY, "y_coord");
		clDataResult.m_nParameterIndex = mapParamInfo[clTestKey].m_nParameterIndex;
		clDataResult.m_lfValue = m_clStdfPRR.m_i2Y_COORD;
		clDataResult.m_nFlowId = -1;
		mapParamResultsBySite[m_clStdfPRR.m_u1SITE_NUM].insert(clTestKey, clDataResult);
		// [...]

		QMap <TestKey, Gate_DataResult> mapParamResultByParamNumber = mapParamResultsBySite.take(m_clStdfPRR.m_u1SITE_NUM);
		QMapIterator <TestKey, Gate_DataResult> itParamResult(mapParamResultByParamNumber);
		while (itParamResult.hasNext())
		{
			itParamResult.next();
			mapParamResultByParamNumber[itParamResult.key()].m_nStepNum = iRunId;
		}
		emit sReadStep(mapParamResultByParamNumber);
//	}
//	else
//	{
//		GSLOG(SYSLOG_SEV_WARNING, "no param for site: %d ", m_clStdfPRR.m_u1SITE_NUM);
//		return false;
//	}

	iRunId++;
	iFlowId = 1;
	return true;
}

//////////////////////////////////////////////////////////////////////
// Check if the PTR test is fail
//////////////////////////////////////////////////////////////////////
bool DbcStdfReader::isTestFail(const GQTL_STDF::CStdf_PTR_V4 &clRecord, const Gate_ParameterDef &cParameterDef)
{
	// If test is flagged as NOT EXECUTED
	if((clRecord.m_b1TEST_FLG & STDF_MASK_BIT4))
		return false;

	// If test is flagged as PASS/FAIL VALID && FAILED
	if(((clRecord.m_b1TEST_FLG & STDF_MASK_BIT6) == 0) &&
		((clRecord.m_b1TEST_FLG & STDF_MASK_BIT7)))
		return true;

	// If test is flagged as PASS/FAIL VALID && NOT FAILED
	if(((clRecord.m_b1TEST_FLG & STDF_MASK_BIT6) == 0) &&
		((clRecord.m_b1TEST_FLG & STDF_MASK_BIT7) == 0))
		return false;

	// If test is flagged as RESULT NOT VALID
	if((clRecord.m_b1TEST_FLG & STDF_MASK_BIT1))
		return false;

	// Check LIMITS
	bool bFail = false;
	if(cParameterDef.m_bHasLowL)
	{
		// Low limit exists (manages Strict and not-strict limits)
		if(cParameterDef.m_uiFlags & FLAG_TESTINFO_LL_STRICT)
			bFail = (cParameterDef.m_lfLowL < clRecord.m_r4RESULT) ? false : true;
		else
			bFail = (cParameterDef.m_lfLowL <= clRecord.m_r4RESULT) ? false : true;
	}

	if(!bFail && (cParameterDef.m_bHasHighL))
	{
		// High limit exists (manages Strict and not-strict limits)
		if(cParameterDef.m_uiFlags & FLAG_TESTINFO_HL_STRICT)
			bFail = (cParameterDef.m_lfHighL > clRecord.m_r4RESULT) ? false : true;
		else
			bFail = (cParameterDef.m_lfHighL >= clRecord.m_r4RESULT) ? false : true;
	}

	// If test is flagged as EXECUTED and is bFail
	if (((clRecord.m_b1TEST_FLG & STDF_MASK_BIT4) == 0) && bFail)
		return true;
	else
		return false;
}

bool DbcStdfReader::lastPTRInfo(QMap<TestKey, Gate_ParameterDef> &mapParamInfo, Gate_ParameterDef &cParameterDef)
{
	QMapIterator<TestKey, Gate_ParameterDef> it(mapParamInfo);
	it.toBack();
	while (it.hasPrevious())
	{
		it.previous();
		if ((it.value().m_nParameterNumber == cParameterDef.m_nParameterNumber) && (it.value().m_strName == cParameterDef.m_strName))
		{
			cParameterDef = it.value();
			return true;
		}
	}

	return false;
}


