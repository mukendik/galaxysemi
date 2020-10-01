#include <QDate>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDomDocument>
#include <QProgressBar>
#include <QApplication>
#include <QLabel>
#include <gqtl_log.h>
#include "ctest.h"
#include "cbinning.h"
#include "import_constants.h"
#include "import_kvd_xml_to_stdf.h"
#include "message.h"
#include "engine.h"

extern QProgressBar	*	GexProgressBar;		// Handle to progress bar in status bar
extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar

QString CGKVDXMLtoSTDF::m_strLastError = "";
int		CGKVDXMLtoSTDF::m_iLastError = errNoError;

CGKVDXMLtoSTDF::CGKVDXMLtoSTDF()
{
    init();
}

void CGKVDXMLtoSTDF::init()
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "KVDXML to STDF init...");
    m_bPCRExist = false;
    m_bValidCoord = true;
    setError(errNoError);
    m_eDataType = Unkown;
    m_oWaferConfiguration.clear();
    m_oWaferTestSummaryList.clear();

    m_oMIRRecord.Reset();
    m_oMIRRecord.SetCMOD_COD(' ');
    m_oMIRRecord.SetJOB_NAM("");
    m_oMIRRecord.SetJOB_REV("");
    m_oMIRRecord.SetENG_ID("");
    QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
    strUserTxt += ":MaxVisionXML/XML";
    m_oMIRRecord.SetUSER_TXT(strUserTxt);

    m_oMRRRecord.Reset();
    m_oMRRRecord.SetDISP_COD(' ');
    m_oMRRRecord.SetUSR_DESC("");
    m_oMRRRecord.SetEXC_DESC("");


    m_oMaxvisionSection.clear();
    m_oATESection.clear();
    m_oLotConfiguration.clear();

    if(!m_oTestFlow.isEmpty()){
        qDeleteAll(m_oTestFlow);
        m_oTestFlow.clear();
    }

    if(!m_oSoftBins.isEmpty()){
        qDeleteAll(m_oSoftBins);
    }
    if(!m_oHardBins.isEmpty()){
        qDeleteAll(m_oHardBins);
    }

    m_oHardBins.clear();
    m_oSoftBins.clear();
    m_oBinMapping.clear();
    m_oWaferMap.clear();
    m_oPRRecordList.clear();
    m_oPIRecordList.clear();

}

CGKVDXMLtoSTDF::~CGKVDXMLtoSTDF(){

}

QString CGKVDXMLtoSTDF::GetLastError(){
    return m_strLastError;
}

int CGKVDXMLtoSTDF::GetLastErrorCode(){
    return m_iLastError;
}

void CGKVDXMLtoSTDF::setError(int iError, const QString &strAdditionalInfo,const QDomNode &oDomNode)
{
    m_iLastError = iError;
    QString strBaseError;
    QString strLineInfo;

    if(iError == errNoError){
        strBaseError = "No Error";
    }else if(iError == errOpenFail){
        strBaseError = "Failed to open file";
    }else if(iError == errInvalidXMLFormat){
        strBaseError = "Invalid XML format";
    }else if(iError == errInvalidMaximKvdFormat){
        strBaseError = "Invalid Maxim XML format";
    }else if(iError == errWriteSTDF){
        strBaseError = "Failed creating STDF file.";
    }else if(iError == errLicenceExpired){
        strBaseError = "License has expired or Data file out of date...";
    }

    if(oDomNode.lineNumber() > 0)
        strLineInfo = QString("[line %1]").arg(oDomNode.lineNumber());
    m_strLastError = QString ("%1 \n%2 %3").arg(strBaseError).arg(strLineInfo).arg(strAdditionalInfo);
    if(iError!= errNoError){
        if(GexProgressBar)
            GexProgressBar->setValue(100);
    }
}

bool CGKVDXMLtoSTDF::IsCompatible(const QString &szFileName)
{
    CGKVDXMLtoSTDF::setError(errNoError);
    QFile oFile(szFileName);
    if(!oFile.open(QIODevice::ReadOnly))
    {
        setError(errOpenFail);
        return false;
    }
    QDomDocument oXMLDoc;
    QString strErrorMessage;
    int iErrorLine=-1, iErrorColumn=-1;
    if(!oXMLDoc.setContent(&oFile, &strErrorMessage, &iErrorLine, &iErrorColumn))
    {
        setError(errInvalidXMLFormat, QString("Error(%1) at Line(%2) and Column(%3)")
                 .arg(strErrorMessage).arg(iErrorLine).arg(iErrorColumn));
        return false;
    }
    QDomNode oRootItem(oXMLDoc);
    bool bCond1 = false;
    bool bCond2 = false;
    for(int iIdx=0; iIdx<oRootItem.childNodes().count(); iIdx++)
    {
        if(oRootItem.childNodes().at(iIdx).nodeName() == "xml")
            bCond1 = true;
        if(oRootItem.childNodes().at(iIdx).nodeName() == "insertion")
        {
            if(oRootItem.childNodes().at(iIdx).attributes().contains("xsi:noNamespaceSchemaLocation"))
                bCond2 = true;
        }
        if(bCond1 && bCond2)
            break;
    }
    bool bIsKVDMaxim = bCond1 && bCond2;

    if(!bIsKVDMaxim)
        setError(errInvalidMaximKvdFormat);

    return bIsKVDMaxim;
}

bool CGKVDXMLtoSTDF::Convert(const QString &szFileName, const QString &strFileNameSTDF)
{
    int iStepsNumber = 5;
    int iStep = (int)(100.0/(iStepsNumber+1));
    int iStepIdx = 0;

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar    bool bHideProgressAfter=true;
    bool bHideProgressAfter=true;
    bool bHideLabelAfter=false;
    if(GexProgressBar != NULL)
    {
        bHideProgressAfter = GexProgressBar->isHidden();
        GexProgressBar->setMaximum(100);
        GexProgressBar->setTextVisible(true);
        GexProgressBar->setValue(0);
        GexProgressBar->show();
    }

    if(GexScriptStatusLabel != NULL)
    {
        if(GexScriptStatusLabel->isHidden())
        {
            bHideLabelAfter = true;
            GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(szFileName).fileName()+"...");
            GexScriptStatusLabel->show();
        }
    }

    setProgress(++iStepIdx*iStep);
    init();

    QFile oFile(szFileName);
    if(!oFile.open(QIODevice::ReadOnly))
    {
        setError(errOpenFail);
        return false;
    }

    QDomDocument oXMLDoc;
    QString strErrorMessage;
    int iErrorLine=-1, iErrorColumn=-1;
    if(!oXMLDoc.setContent(&oFile, &strErrorMessage, &iErrorLine, &iErrorColumn))
    {
        setError(errInvalidXMLFormat, QString("Error(%1) at Line(%2) and Column(%3)").arg(strErrorMessage).arg(iErrorLine).arg(iErrorColumn));
        return false;
    }

    QDomNode oInsertionElem = oXMLDoc.namedItem("insertion");
    if(oInsertionElem.isNull())
    {
        setError(errInvalidMaximKvdFormat, QString("tag \"insertion\" not found"),oInsertionElem);
        return false;
    }

    setProgress(++iStepIdx*iStep);
    //    QDomNode oMaxvisionElem = oInsertionElem.namedItem("maxvision");
    //    if(oMaxvisionElem.isNull()){
    //        setError(errInvalidMaximKvdFormat, QString("tag \"maxvision\" not found"));
    //        return false;
    //    }
    //    if(!readMaxvisionSection(oMaxvisionElem)){
    //        return false;
    //    }

    setProgress(++iStepIdx*iStep);
    //chapter 7.
    QDomNode oTestOccurrence = oInsertionElem.namedItem("testOccurrence");
    if(oTestOccurrence.isNull())
    {
        setError(errInvalidMaximKvdFormat, QString("tag \"testOccurrence\" not found"),oTestOccurrence);
        return false;
    }

    if(!readTestOccurrenceSection(oTestOccurrence)){
        return false;
    }

    setProgress(++iStepIdx*iStep);


    setProgress(++iStepIdx*iStep);
    if(!WriteStdfFile(0,strFileNameSTDF))
    {
        // Convertion failed.
        QFile::remove(strFileNameSTDF);
        return false;
    }

    setProgress(100);

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    if((GexProgressBar != NULL)
            && bHideProgressAfter)
        GexProgressBar->hide();

    if((GexScriptStatusLabel != NULL)
            && bHideLabelAfter)
        GexScriptStatusLabel->hide();

    return true;
}

bool CGKVDXMLtoSTDF::WriteStdfFile(QTextStream */*hSemi_G85File*/,const QString &strFileNameSTDF){

    GS::StdLib::Stdf oStdfFile;
    GS::StdLib::StdfRecordReadInfo oRecordReadInfo;

    if(oStdfFile.Open(strFileNameSTDF.toLatin1().constData(), STDF_WRITE) != GS::StdLib::Stdf::NoError){
        // Convertion failed.
        setError(errWriteSTDF);
        return false;
    }

    //FAR
    oRecordReadInfo.iRecordType = 0;
    oRecordReadInfo.iRecordSubType = 10;
    oStdfFile.WriteHeader(&oRecordReadInfo);
    oStdfFile.WriteByte(1);
    oStdfFile.WriteByte(4);					// STDF V4
    if(oStdfFile.WriteRecord() != GS::StdLib::Stdf::NoError){
        setError(errWriteSTDF, "Issue when writing FAR Record");
        return false;
    }

    //MIR
    if(!m_oMIRRecord.Write(oStdfFile)){
        setError(errWriteSTDF, "Issue when writing MIR Record");
        return false;
    }

    //SDR
    //loop on m_oATESection.m_oSiteGroupList
    for(int iIdx=0; iIdx <m_oATESection.m_oSiteGroupList.count(); iIdx++){
        QMap<QString, QVariant> oSiteGroup = m_oATESection.m_oSiteGroupList[iIdx];
        GQTL_STDF::Stdf_SDR_V4 oSDR;
        oSDR.Reset();
        oSDR.SetHEAD_NUM( oSiteGroup["headNumber"].toInt() );
        oSDR.SetSITE_GRP( oSiteGroup["siteNumber"].toInt() );
        oSDR.SetSITE_CNT( 1 );
        oSDR.SetSITE_NUM(0,0);
        oSDR.SetHAND_TYP( oSiteGroup["handlerType"].toString() );
        oSDR.SetHAND_ID( oSiteGroup["handlerID"].toString() );
        oSDR.SetCARD_TYP( oSiteGroup["probeCardType"].toString() );
        oSDR.SetCARD_ID( oSiteGroup["probeCardID"].toString() );
        oSDR.SetLOAD_TYP( oSiteGroup["loadBoardType"].toString() );
        oSDR.SetLOAD_ID( oSiteGroup["loadBoardID"].toString() );
        oSDR.SetDIB_TYP( oSiteGroup["dibType"].toString() );
        oSDR.SetDIB_ID( oSiteGroup["dibID"].toString() );
        oSDR.SetCABL_TYP( oSiteGroup["cableType"].toString() );
        oSDR.SetCABL_ID( oSiteGroup["cableID"].toString() );
        oSDR.SetCONT_TYP( oSiteGroup["contactorType"].toString() );
        oSDR.SetCONT_ID( oSiteGroup["contactorID"].toString() );
        oSDR.SetLASR_TYP( oSiteGroup["laserType"].toString() );
        oSDR.SetLASR_ID( oSiteGroup["laserID"].toString() );
        oSDR.SetEXTR_TYP( oSiteGroup["extraType"].toString() );
        oSDR.SetEXTR_ID( oSiteGroup["extraID"].toString() );
        if(!oSDR.Write(oStdfFile)){
            setError(errWriteSTDF, "Issue when writing SDR Record");
            return false;
        }

    }

    //if(!m_oMIRRecord.m_cnPKG_TYP.isEmpty())
    {
        // dump wafer info
        //WIR
        GQTL_STDF::Stdf_WIR_V4 oWIR;
        oWIR.SetHEAD_NUM(255);
        oWIR.SetSITE_GRP(255);
        oWIR.SetSTART_T(m_oWaferMap.lWaferStartTime);
        oWIR.SetWAFER_ID(m_oWaferMap.szWaferID);
        if(!oWIR.Write(oStdfFile))
        {
            setError(errWriteSTDF, "Issue when writing WIR Record");
            return false;
        }

        //WCR
        GQTL_STDF::Stdf_WCR_V4 oWCR;
        oWCR.SetWAFR_SIZ(m_oWaferConfiguration["waferSize"].toFloat());
        oWCR.SetDIE_HT	(m_oWaferConfiguration["dieHeight"].toFloat());
        oWCR.SetDIE_WID	(m_oWaferConfiguration["dieWidth"].toFloat());
        oWCR.SetWF_UNITS(/*m_oWaferConfiguration[""]*/3);
        oWCR.SetWF_FLAT	(getWF_FLAT(m_oWaferConfiguration["orientationMarkLocation"].toString()));
        oWCR.SetCENTER_X(m_oWaferConfiguration["centerX"].toInt());
        oWCR.SetCENTER_Y(m_oWaferConfiguration["centerY"].toInt());
        oWCR.SetPOS_X	(m_oWaferConfiguration["positiveX"].toChar().toLatin1());
        oWCR.SetPOS_Y	(m_oWaferConfiguration["positiveY"].toChar().toLatin1());
        if(!oWCR.Write(oStdfFile))
        {
            setError(errWriteSTDF, "Issue when writing WCR Record");
            return false;
        }
    }

    int lPartsCount = 0;
    int lPartsFail = 0;
    for(int iIdx=0; iIdx<m_oPIRecordList.count(); iIdx++){
        //PIR
        if(!m_oPIRecordList[iIdx].Write(oStdfFile)){
            setError(errWriteSTDF, "Issue when writing PIR Record");
            return false;
        }

        //PTR
        foreach(CTest *poTest, m_oTestFlow){
            GQTL_STDF::Stdf_PTR_V4 oPTR;
            oPTR.Reset();
            if(!poTest->m_testResult.count() || !poTest->m_testResult.isValidResultAt(iIdx))
                continue;
            oPTR.SetTEST_NUM(poTest->lTestNumber);
            oPTR.SetHEAD_NUM(m_oPIRecordList[iIdx].m_u1HEAD_NUM);
            oPTR.SetSITE_NUM(m_oPIRecordList[iIdx].m_u1SITE_NUM);
            oPTR.SetTEST_FLG(poTest->m_testResult.passFailStatus(iIdx) == CTestResult::statusPass ? 0 : STDF_MASK_BIT7);
            oPTR.SetPARM_FLG(0);
            oPTR.SetRESULT  (poTest->m_testResult.resultAt(iIdx));
            oPTR.SetTEST_TXT(poTest->strTestName);
            oPTR.SetALARM_ID("");
            // case 7734
            // Check if Test limits are defined
            BYTE lFlag = 0;
            if(poTest->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL)
                lFlag |= STDF_MASK_BIT2;
            if((poTest->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) && (poTest->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLSL))
                lFlag |= STDF_MASK_BIT6;
            if(poTest->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL)
                lFlag |= STDF_MASK_BIT3;
            if((poTest->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) && (poTest->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHSL))
                lFlag |= STDF_MASK_BIT7;
            oPTR.SetOPT_FLAG(lFlag);
            oPTR.SetRES_SCAL(poTest->res_scal);
            oPTR.SetLLM_SCAL(poTest->res_scal);
            oPTR.SetHLM_SCAL(poTest->res_scal);
            oPTR.SetLO_LIMIT(poTest->GetCurrentLimitItem()->lfLowLimit);
            oPTR.SetHI_LIMIT(poTest->GetCurrentLimitItem()->lfHighLimit);
            oPTR.SetUNITS   (poTest->szTestUnits);
            oPTR.SetC_RESFMT("");
            oPTR.SetC_LLMFMT("");
            oPTR.SetC_HLMFMT("");
            oPTR.SetLO_SPEC (poTest->lfLowSpecLimit);
            oPTR.SetHI_SPEC (poTest->lfHighSpecLimit);
            if(!oPTR.Write(oStdfFile)){
                setError(errWriteSTDF, "Issue when writing oPTR Record");
                return false;
            }


        }

        //PRR
        if(!m_bValidCoord)
        {
            m_oPRRecordList[iIdx].SetX_COORD(-32768);
            m_oPRRecordList[iIdx].SetY_COORD(-32768);
        }
        if(!m_oPRRecordList[iIdx].Write(oStdfFile)){
            setError(errWriteSTDF, "Issue when writing PRR Record");
            return false;
        }
        ++lPartsCount;
        if((m_oPRRecordList[iIdx].m_b1PART_FLG & STDF_MASK_BIT4) == 0)
            if(m_oPRRecordList[iIdx].m_b1PART_FLG & STDF_MASK_BIT3)
                ++lPartsFail;
    }



    //TSR after the last PRR
    for(int iIdx=0; iIdx <m_oWaferTestSummaryList.count(); iIdx++){
        QMap<QString, QVariant> oTestSummary  = m_oWaferTestSummaryList[iIdx];
        GQTL_STDF::Stdf_TSR_V4 oTSR;
        oTSR.Reset();
        oTSR.SetHEAD_NUM(oTestSummary["HEAD_NUM"].toInt());
        oTSR.SetSITE_NUM(oTestSummary["SITE_NUM"].toInt());
        oTSR.SetTEST_TYP(oTestSummary["TEST_TYP"].toChar().toLatin1());
        oTSR.SetTEST_NUM(oTestSummary["testID"].toInt());
        oTSR.SetEXEC_CNT(oTestSummary["executions"].toInt());
        oTSR.SetFAIL_CNT(oTestSummary["fails"].toInt());
        oTSR.SetALRM_CNT(oTestSummary["alarms"].toInt());
        oTSR.SetTEST_NAM(oTestSummary["TEST_NAME"].toString());
        oTSR.SetSEQ_NAME("");
        oTSR.SetTEST_LBL("");
        oTSR.SetOPT_FLAG(oTestSummary["OPT_FLAG"].toChar().toLatin1());
        oTSR.SetTEST_TIM(oTestSummary["averageTestTime"].toFloat());
        oTSR.SetTEST_MIN(oTestSummary["min"].toFloat());
        oTSR.SetTEST_MAX(oTestSummary["max"].toFloat());
        oTSR.SetTST_SUMS(oTestSummary["sum"].toFloat());
        oTSR.SetTST_SQRS(oTestSummary["sum"].toFloat());
        if(!oTSR.Write(oStdfFile)){
            setError(errWriteSTDF, "Issue when writing TSR Record");
            return false;
        }
    }

    //WRR
    //if(!m_oMIRRecord.m_cnPKG_TYP.isEmpty())
    {
        // dump wafer info
        //WRR
        GQTL_STDF::Stdf_WRR_V4 oWRR;
        oWRR.SetHEAD_NUM(255);
        oWRR.SetSITE_GRP(255);
        oWRR.SetFINISH_T(m_oWaferMap.lWaferEndTime);
        oWRR.SetPART_CNT(lPartsCount);
        oWRR.SetRTST_CNT(4294967295UL);
        oWRR.SetABRT_CNT(4294967295UL);
        oWRR.SetGOOD_CNT(lPartsCount-lPartsFail);
        oWRR.SetFUNC_CNT(4294967295UL);
        oWRR.SetWAFER_ID(m_oWaferMap.szWaferID);
        oWRR.SetFABWF_ID(m_oWaferConfiguration["fabID"].toString());
        oWRR.SetFRAME_ID(m_oWaferConfiguration["frameID"].toString());
        oWRR.SetMASK_ID(m_oWaferConfiguration["maskSetID"].toString());
        oWRR.SetUSR_DESC(m_oWaferConfiguration["userDescription"].toString());
        oWRR.SetEXC_DESC(m_oWaferConfiguration["execDescription"].toString());
        if(!oWRR.Write(oStdfFile))
        {
            setError(errWriteSTDF, "Issue when writing WRR Record");
            return false;
        }
    }
    //HBR
    for(int iIdx=0; iIdx<m_oHardBins.count(); iIdx++){
        CBinning *poBin = m_oHardBins[iIdx];
        GQTL_STDF::Stdf_HBR_V4 oHBR;
        oHBR.Reset();
        oHBR.SetHEAD_NUM(255);
        oHBR.SetSITE_NUM(1);
        oHBR.SetHBIN_NUM(poBin->iBinValue);
        oHBR.SetHBIN_CNT(poBin->ldTotalCount);
        oHBR.SetHBIN_PF (poBin->cPassFail);
        oHBR.SetHBIN_NAM(poBin->strBinName);
        if(!oHBR.Write(oStdfFile)){
            setError(errWriteSTDF, "Issue when writing HBR Record");
            return false;
        }
    }

    //SBR
    for(int iIdx=0; iIdx<m_oSoftBins.count(); iIdx++){
        CBinning *poBin = m_oSoftBins[iIdx];
        GQTL_STDF::Stdf_SBR_V4 oSBR;
        oSBR.Reset();
        oSBR.SetHEAD_NUM(255);
        oSBR.SetSITE_NUM(1);
        oSBR.SetSBIN_NUM(poBin->iBinValue);
        oSBR.SetSBIN_CNT(poBin->ldTotalCount);
        oSBR.SetSBIN_PF (poBin->cPassFail);
        oSBR.SetSBIN_NAM(poBin->strBinName);
        if(!oSBR.Write(oStdfFile)){
            setError(errWriteSTDF, "Issue when writing SBR Record");
            return false;
        }
    }

    if(m_bPCRExist){
        if(!m_oPCRRecord.Write(oStdfFile)){
            setError(errWriteSTDF, "Issue when writing PCR Record");
            return false;
        }
    }
    //MRR
    if(!m_oMRRRecord.Write(oStdfFile)){
        setError(errWriteSTDF, "Issue when writing MRR Record");
        return false;
    }

    oStdfFile.Close();
    return true;
}


bool CGKVDXMLtoSTDF::readMaxvisionSection(const QDomNode &oMaxvisionElem){
    readElementText(oMaxvisionElem, "type", false, m_oMaxvisionSection.m_oMaxvisionXMLProp,"");
    readElementText(oMaxvisionElem, "file_md5", false, m_oMaxvisionSection.m_oMaxvisionXMLProp,"");
    readElementText(oMaxvisionElem, "file_source",false, m_oMaxvisionSection.m_oMaxvisionXMLProp,"");
    readElementText(oMaxvisionElem, "time_stamp",false, m_oMaxvisionSection.m_oMaxvisionXMLProp,"");
    QDomElement oLibraryElement = oMaxvisionElem.firstChildElement("library");
    while (!oLibraryElement.isNull()){
        QString strLibProp, strTemp;

        readElementTextAttribute(oLibraryElement, "name", false, strTemp,"");
        strLibProp = strTemp;
        readElementTextAttribute(oLibraryElement, "version", false, strTemp,"");
        strLibProp += strTemp;
        readElementTextAttribute(oLibraryElement, "note", false, strTemp,"");
        strLibProp += strTemp;

        m_oMaxvisionSection.m_oLibraryAttributes.append(strLibProp);
        oLibraryElement = oLibraryElement.nextSiblingElement("library");
    }
    return true;

}

bool CGKVDXMLtoSTDF::readTestOccurrenceSection(const QDomNode &oTestOccurrence){

    //The 'dataType' will always be 'DieSort' for sort and final test data, 'Scribeline' for PT data, and 'matlhist' for Material History data.
    QString strDataType;
    readElementTextAttribute(oTestOccurrence.toElement(), "dataType", true, strDataType);
    if(strDataType == "DieSort")
        m_eDataType = DieSort;
    else if(strDataType == "Scribeline")
        m_eDataType = Scribeline;
    else if(strDataType == "matlhist")
        m_eDataType = Matlhist;

    if(m_eDataType == Unkown){
        setError(errInvalidMaximKvdFormat, QString("Missing \"dataType\" in \"testOccurrence\" section"),oTestOccurrence);
        return false;
    }
    //=>MIR
    QString strFiledData;
    if(!readElementText(oTestOccurrence, "step", true, strFiledData))
        return false;
    m_oMIRRecord.SetTEST_COD(strFiledData);
    if(!readElementText(oTestOccurrence, "setupTimestamp", false, strFiledData))
        return false;
    m_oMIRRecord.SetSETUP_T (getTime(strFiledData.simplified()));

    if(!readElementText(oTestOccurrence, "startTimestamp", true, strFiledData))
        return false;
    m_oMIRRecord.SetSTART_T (getTime(strFiledData.simplified()));

    if(!readElementText(oTestOccurrence, "endTimestamp", false, strFiledData))
        return false;
    // if strFiledData is empty, FINISH_T will be equal to -1 at least under win32.
    if (strFiledData.isEmpty())
        GSLOG(SYSLOG_SEV_WARNING, "Empty or unfindable test occurence endTimestamp.");

    m_oMRRRecord.SetFINISH_T(getTime(strFiledData));

    if(!readElementText(oTestOccurrence, "operatorName", false, strFiledData,""))
        return false;
    m_oMIRRecord.SetOPER_NAM (strFiledData);

    if(!readElementText(oTestOccurrence, "supervisorName", false, strFiledData, ""))
        return false;
    m_oMIRRecord.SetSUPR_NAM (strFiledData);

    if(!readElementText(oTestOccurrence, "facilityID", false, strFiledData))
        return false;
    m_oMIRRecord.SetFACIL_ID (strFiledData);

    if(!readElementText(oTestOccurrence, "floorID", false, strFiledData, ""))
        return false;
    m_oMIRRecord.SetFLOOR_ID (strFiledData);

    if(!readElementText(oTestOccurrence, "modeCode", false, strFiledData))
        return false;
    m_oMIRRecord.SetMODE_COD (getModeCodes(strFiledData));

    if(!readElementText(oTestOccurrence, "auxiliaryFile", false, strFiledData,""))
        return false;
    m_oMIRRecord.SetAUX_FILE (strFiledData);

    if(!readElementText(oTestOccurrence, "setupID", false, strFiledData))
        return false;
    m_oMIRRecord.SetSETUP_ID (strFiledData);

    if(!readElementText(oTestOccurrence, "productID", true, strFiledData,""))
        return false;
    m_oMIRRecord.SetPART_TYP (strFiledData);

    if(!readElementText(oTestOccurrence, "productFamilyID", false, strFiledData, ""))
        return false;
    m_oMIRRecord.SetFAMLY_ID (strFiledData);

    if(!readElementText(oTestOccurrence, "designRevision", false, strFiledData, ""))
        return false;
    m_oMIRRecord.SetDSGN_REV (strFiledData);

    if(!readElementText(oTestOccurrence, "maskSetID", false, strFiledData, ""))
        return false;// may be WRR
    m_oWaferConfiguration["maskSetID"] = strFiledData.simplified();

    if(!readElementText(oTestOccurrence, "processID", false, strFiledData,""))
        return false;
    m_oMIRRecord.SetPROC_ID ( strFiledData);

    if(!readElementText(oTestOccurrence, "foundry", false, strFiledData,""))
        return false;
    if(!readElementText(oTestOccurrence, "foundrySite", false, strFiledData,""))
        return false;

    if(!readElementText(oTestOccurrence, "dataCode", false, strFiledData,"")) //cdr
        return false;
    m_oMIRRecord.SetDATE_COD(strFiledData);

    QDomNode oRetestElem = oTestOccurrence.namedItem("retest");
    if(!oRetestElem.isNull()){
        readElementTextAttribute(oRetestElem.toElement(), "code", false, strFiledData, " ");
        m_oMIRRecord.SetRTST_COD (' ');
        readElementTextAttribute(oRetestElem.toElement(), "name", false, strFiledData, " ");
    }else{
        m_oMIRRecord.SetRTST_COD (strFiledData[0].toLatin1());
    }


    if(!readElementText(oTestOccurrence, "packageType", false, strFiledData))
        return false;
    m_oMIRRecord.SetPKG_TYP (strFiledData);

    if(!readElementText(oTestOccurrence, "burnTime", false, strFiledData,""))
        return false;
    m_oMIRRecord.SetBURN_TIM (strFiledData.toUShort());

    if(!readElementText(oTestOccurrence, "temperature", false, strFiledData,""))
        return false;
    m_oMIRRecord.SetTST_TEMP (strFiledData);

    //chapter 8.
    QDomNode oATEElem = oTestOccurrence.namedItem("ATE");
    if(oATEElem.isNull()){
        oATEElem = oTestOccurrence.namedItem("tool");
        if(oATEElem.isNull()){
            setError(errInvalidMaximKvdFormat, QString("nor \"ATE\" nor \"tool\" were found"),oATEElem);
            return false;
        }
    }
    if(!readATESection(oATEElem))
        return false;

    if(!readElementText(oTestOccurrence, "operationFrequency", false, strFiledData,""))
        return false;
    m_oMIRRecord.SetOPER_FRQ (strFiledData);

    if(!readElementText(oTestOccurrence, "specificationName", false, strFiledData,""))
        return false;
    m_oMIRRecord.SetSPEC_NAM (strFiledData);

    if(!readElementText(oTestOccurrence, "specificationVersion", false, strFiledData,""))
        return false;
    m_oMIRRecord.SetSPEC_VER(strFiledData);

    if(!readElementText(oTestOccurrence, "romCode", false, strFiledData,""))
        return false;
    m_oMIRRecord.SetROM_COD (strFiledData);

    if(!readElementText(oTestOccurrence, "dataProtectionCode", false, strFiledData,""))
        return false;
    m_oMIRRecord.SetPROT_COD (strFiledData[0].toLatin1());

    if(!readElementText(oTestOccurrence, "userDesc", false, strFiledData,""))
        return false;

    //chapter 9.
    if(m_eDataType == DieSort ){
        QDomNode oWaferConfigurationElem = oTestOccurrence.namedItem("waferConfiguration");
        if(oWaferConfigurationElem.isNull()){
            setError(errInvalidMaximKvdFormat, QString("tag \"waferConfiguration\" not found"),oWaferConfigurationElem);
            return false;
        }
        if(!readWaferConfigurationSection(oWaferConfigurationElem))
            return false;
    }

    //chapter 10.
    QDomNode oTestProgramElem = oTestOccurrence.namedItem("testProgram");
    if(oTestProgramElem.isNull()){
        setError(errInvalidMaximKvdFormat, QString("tag \"testProgram\" not found"),oTestProgramElem);
        return false;
    }
    if(!readTestProgramSection(oTestProgramElem))
        return false;

    QDomNode oLotUnderTestElem = oTestOccurrence.namedItem("lotUnderTest");
    if(oLotUnderTestElem.isNull()){
        setError(errInvalidMaximKvdFormat, QString("tag \"lotUnderTest\" not found"),oLotUnderTestElem);
        return false;
    }
    if(!readLotUnderTestSection(oLotUnderTestElem))
        return false;

    QDomNode oPartCountElem = oTestOccurrence.namedItem("partCount");
    if(!oPartCountElem.isNull()){
        if(!readPartCount(oPartCountElem))
            return false;
    }


    return true;

}

bool CGKVDXMLtoSTDF::readATESection(const QDomNode &oATEElem){
    //chapter 8.

    if(!readElementTextAttribute(oATEElem.toElement(), "name", true, m_oATESection.m_oATEXMLProp,""))
        return false;
    m_oMIRRecord.SetNODE_NAM(m_oATESection.m_oATEXMLProp["name"].toString());

    readElementTextAttribute(oATEElem.toElement(), "type", false, m_oATESection.m_oATEXMLProp,"");
    m_oMIRRecord.SetTSTR_TYP(m_oATESection.m_oATEXMLProp["type"].toString());

    readElementText(oATEElem, "stationNumber", false, m_oATESection.m_oATEXMLProp,"0");
    m_oMIRRecord.SetSTAT_NUM( m_oATESection.m_oATEXMLProp["stationNumber"].toInt());

    readElementText(oATEElem, "serialNumber", false, m_oATESection.m_oATEXMLProp,"");
    m_oMIRRecord.SetSERL_NUM(m_oATESection.m_oATEXMLProp["serialNumber"].toString());

    readElementText(oATEElem, "testerExecutiveType", false, m_oATESection.m_oATEXMLProp,"");
    m_oMIRRecord.SetEXEC_TYP(m_oATESection.m_oATEXMLProp["testerExecutiveType"].toString());

    readElementText(oATEElem, "testerExecutiveVersion", false, m_oATESection.m_oATEXMLProp,"");
    m_oMIRRecord.SetEXEC_VER(m_oATESection.m_oATEXMLProp["testerExecutiveVersion"].toString());

    //SiteGroup section // =>SDR
    QDomElement oSiteGroupElement = oATEElem.firstChildElement("siteGroup");
    QMap<QString, QVariant> oTempMap;
    while (!oSiteGroupElement.isNull()){
        oTempMap.clear();
        readElementTextAttribute(oSiteGroupElement,"number",false,oTempMap,"0");
        readElementTextAttribute(oSiteGroupElement,"headNumber",false,oTempMap,"0");

        if(!readElementText(oSiteGroupElement,"siteNumber",false,oTempMap))
            return false;
        if(!readElementText(oSiteGroupElement,"handlerType",false,oTempMap))
            return false;
        if(!readElementText(oSiteGroupElement,"handlerID",false,oTempMap))
            return false;
        if(!readElementText(oSiteGroupElement,"probeCardType",false,oTempMap))
            return false;
        if(!readElementText(oSiteGroupElement,"probeCardID",false,oTempMap))
            return false;
        if(!readElementText(oSiteGroupElement,"loadBoardType",false,oTempMap))
            return false;
        if(!readElementText(oSiteGroupElement,"loadBoardID",false,oTempMap))
            return false;
        if(!readElementText(oSiteGroupElement,"dibType",false,oTempMap))
            return false;
        if(!readElementText(oSiteGroupElement,"dibID",false,oTempMap))
            return false;
        if(!readElementText(oSiteGroupElement,"cableType",false,oTempMap))
            return false;
        if(!readElementText(oSiteGroupElement,"cableID",false,oTempMap))
            return false;
        if(!readElementText(oSiteGroupElement,"contactorType",false,oTempMap))
            return false;
        if(!readElementText(oSiteGroupElement,"contactorID",false,oTempMap))
            return false;
        if(!readElementText(oSiteGroupElement,"laserType",false,oTempMap))
            return false;
        if(!readElementText(oSiteGroupElement,"laserID",false,oTempMap))
            return false;
        if(!readElementText(oSiteGroupElement,"extraType",false,oTempMap))
            return false;
        if(!readElementText(oSiteGroupElement,"extraID",false,oTempMap))
            return false;
        if(!readElementText(oSiteGroupElement,"toolSlot",false,oTempMap,""))
            return false;

        m_oATESection.m_oSiteGroupList.append(oTempMap);
        oSiteGroupElement = oSiteGroupElement.nextSiblingElement("siteGroup");
    }
    if(m_oATESection.m_oSiteGroupList.isEmpty()){
        setError(errInvalidMaximKvdFormat, QString("Missing tag \"%1\" in \"%2\" section").arg("siteGroup").arg(oATEElem.nodeName()),oATEElem);
        return false;
    }
    return true;
}

bool CGKVDXMLtoSTDF::readWaferConfigurationSection(const QDomNode &oWaferConfigElem){

    //WCR
    if(!readElementText(oWaferConfigElem,"waferSize",false,m_oWaferConfiguration))
        return false;
    if(!readElementText(oWaferConfigElem,"dieHeight",false,m_oWaferConfiguration))
        return false;
    if(!readElementText(oWaferConfigElem,"dieWidth",false,m_oWaferConfiguration))
        return false;
    if(!readElementText(oWaferConfigElem,"units",false,m_oWaferConfiguration))
        return false;
    if(!readElementText(oWaferConfigElem,"orientationMarkType",false,m_oWaferConfiguration))
        return false;
    if(!readElementText(oWaferConfigElem,"orientationMarkLocation",false,m_oWaferConfiguration))
        return false;
    if(!readElementText(oWaferConfigElem,"centerX",false,m_oWaferConfiguration))
        return false;
    if(!readElementText(oWaferConfigElem,"centerY",false,m_oWaferConfiguration))
        return false;
    if(!readElementText(oWaferConfigElem,"positiveX",false,m_oWaferConfiguration))
        return false;
    if(!readElementText(oWaferConfigElem,"positiveY",false,m_oWaferConfiguration))
        return false;

    return true;
}

bool CGKVDXMLtoSTDF::readTestProgramSection(const QDomNode &oTestProgramElem){

    QString strField;
    if(!readElementTextAttribute(oTestProgramElem.toElement(), "name", false,strField,""))
        return false;
    if(!readElementTextAttribute(oTestProgramElem.toElement(), "version", false,strField,""))
        return false;

    //testFlow
    QDomNode oTestFlowElem = oTestProgramElem.namedItem("testFlow");
    if(!oTestFlowElem.isNull()){
        if(!readElementTextAttribute(oTestProgramElem.toElement(),"flowID",false,strField,""))
            return false;
        m_oMIRRecord.SetFLOW_ID(strField);
        if(!readTestFlowSection(oTestFlowElem))
            return false;
    }else
        m_oMIRRecord.SetFLOW_ID("");

    //binSet
    QDomNode oBinSetElem = oTestProgramElem.firstChildElement("binSet");
    if(oBinSetElem.isNull()){
        setError(errInvalidMaximKvdFormat, QString("tag \"binSet\" not found"),oBinSetElem);
        return false;
    }
    if(!readBinSet(oTestProgramElem))
        return false;

    return true;
}

bool CGKVDXMLtoSTDF::readTestFlowSection(const QDomNode &oTestFlowElem){
    //m_oTestFlow
    QDomElement oTestElement = oTestFlowElem.firstChildElement("test");
    CTest *poTest = 0;
    QString strTemp;
    bool bError = true;
    while (!oTestElement.isNull()){
        bError = true;
        poTest = new CTest;
        strTemp.clear();
        //ID & Name
        if(!readElementTextAttribute(oTestElement, "ID", true, strTemp)){
            break;
        }
        poTest->lTestNumber =  strTemp.toInt();
        if(!readElementTextAttribute(oTestElement, "name", true, strTemp)){
            break;
        }
        poTest->strTestName = strTemp;

        //FailBin
        QDomNode oFailBinElement = oTestElement.namedItem("failBin");
        if(oFailBinElement.isNull()){
            setError(errInvalidMaximKvdFormat, QString("tag \"failBin\" not found"),oFailBinElement);
            break;
        }
        if(!readElementTextAttribute(oFailBinElement.toElement(), "binSetName", true, strTemp))
            break;
        //TODO : poTest->
        if(!readElementTextAttribute(oFailBinElement.toElement(), "ID", true, strTemp))
            break;
        poTest->iFailBin = strTemp.toInt();

        //Limits
        QDomNode oLimitElement = oTestElement.namedItem("limits");
        // case 7734
        // Check if Test limits are defined
        poTest->GetCurrentLimitItem()->bLimitFlag = 0;
        if(!readElementText(oLimitElement, "testLow", true, strTemp,QString::number(-C_INFINITE)))
            poTest->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOLTL;
        else
            poTest->GetCurrentLimitItem()->lfLowLimit = strTemp.toDouble();
        if(!readElementText(oLimitElement, "testHigh", true, strTemp,QString::number(C_INFINITE)))
            poTest->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOHTL;
        else
            poTest->GetCurrentLimitItem()->lfHighLimit = strTemp.toDouble();
        if(!readElementText(oLimitElement, "specLow", true, strTemp,QString::number(-C_INFINITE)))
            poTest->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOLSL;
        else
            poTest->lfLowSpecLimit = strTemp.toDouble();
        if(!readElementText(oLimitElement, "specHigh", true, strTemp,QString::number(C_INFINITE)))
            poTest->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOHSL;
        else
            poTest->lfHighSpecLimit = strTemp.toDouble();
        if(!readElementText(oLimitElement, "statLow", false, strTemp,QString::number(-C_INFINITE)))
            break;
        if(!readElementText(oLimitElement, "statHigh", false, strTemp,QString::number(C_INFINITE)))
            break;
        if(!readElementText(oLimitElement, "targetLow", false, strTemp,QString::number(-C_INFINITE)))
            break;
        if(!readElementText(oLimitElement, "targetHigh", false, strTemp,QString::number(C_INFINITE)))
            break;
        if(!readElementText(oLimitElement, "controlLow", false, strTemp,QString::number(-C_INFINITE)))
            break;
        if(!readElementText(oLimitElement, "controlHigh", false, strTemp,QString::number(C_INFINITE)))
            break;

        //Units
        if(!readElementText(oTestElement, "units", false, strTemp))
            break;
        strcpy(poTest->szTestUnits, strTemp.toLatin1().constData()) ;

        //scale
        if(!readElementText(oTestElement, "scale", false, strTemp, "0"))
            break;
        poTest->res_scal = strTemp.toInt();

        //precision
        if(!readElementText(oTestElement, "precision", false, strTemp))
            break;
        //poTest->res_scal = strTemp.toInt();

        //precision
        if(!readElementText(oTestElement, "notation", false, strTemp))
            break;

        bError = false;
        m_oTestFlow.append(poTest);
        oTestElement = oTestElement.nextSiblingElement("test");
    }

    if(bError){
        if(poTest)
            delete poTest;
        if(!m_oTestFlow.isEmpty())
            qDeleteAll(m_oTestFlow);
        return false;
    }
    if(m_oTestFlow.isEmpty()){
        setError(errInvalidMaximKvdFormat, QString("Missing tag \"%1\" in \"%2\" section").arg("test").arg(oTestFlowElem.nodeName()),oTestElement);
        return false;
    }

    return true;
}

bool CGKVDXMLtoSTDF::readBinSet(const QDomNode &oBinSetElem){

    QDomElement oBinElement = oBinSetElem.firstChildElement("binSet");
    QString strField;
    bool bError = true;
    CBinning *poBin = 0;
    while (!oBinElement.isNull()){
        bError = true;
        poBin = 0;
        if(!readElementTextAttribute(oBinElement, "name", true, strField))
            break;
        QString strBinName = strField;
        //Check if Hardware or software or PARAMETRIC.
        if(strBinName.endsWith("HARDWARE",Qt::CaseInsensitive)){
            if(oBinElement.childNodes().count()!=2){
                setError(errInvalidMaximKvdFormat, QString("section \"%1\" must contains two \"%2\" ").arg(oBinElement.nodeName()).arg("Bin"),oBinSetElem);
                break;
            }

            QDomElement oBin1 = oBinElement.firstChildElement("bin");
            poBin = new CBinning;
            poBin->strBinName = strBinName;
            if(!readElementTextAttribute(oBin1, "desc", true, strField))
                break;
            poBin->strBinName = strField;
            if(!readElementTextAttribute(oBin1, "ID", true, strField))
                break;
            poBin->iBinValue = strField.toInt();
            if(!readElementTextAttribute(oBin1, "pass", true, strField))
                break;
            poBin->cPassFail = (strField.toLower() == "true") ? 'P' : (strField.toLower() == "false" ? 'F' : ' ');
            insertNewBinning(m_oHardBins, &poBin);

            QDomElement oBin2 = oBin1.nextSiblingElement("bin");
            poBin = new CBinning;
            poBin->strBinName = strBinName;
            if(!readElementTextAttribute(oBin2, "desc", true, strField))
                break;
            poBin->strBinName = strField;
            if(!readElementTextAttribute(oBin2, "ID", true, strField))
                break;
            poBin->iBinValue = strField.toInt();
            if(!readElementTextAttribute(oBin2, "pass", true, strField))
                break;
            poBin->cPassFail = (strField.toLower() == "true") ? 'P' : (strField.toLower() == "false" ? 'F' : ' ');
            insertNewBinning(m_oHardBins, &poBin);

        }else if(strBinName.endsWith("SOFTWARE",Qt::CaseInsensitive)){

            QDomElement oBin = oBinElement.firstChildElement("bin");
            while(!oBin.isNull()){
                poBin = new CBinning;
                poBin->strBinName = strBinName;
                if(!readElementTextAttribute(oBin, "desc", true, strField))
                    break;
                poBin->strBinName = strField;
                if(!readElementTextAttribute(oBin, "ID", true, strField))
                    break;
                poBin->iBinValue = strField.toInt();
                if(!readElementTextAttribute(oBin, "pass", true, strField))
                    break;
                poBin->cPassFail = (strField.toLower() == "true") ? 'P' : (strField.toLower() == "false" ? 'F' : ' ');

                QDomElement oBinMap = oBin.namedItem("binMap").toElement();
                if(oBinMap.isNull()){
                    setError(errInvalidMaximKvdFormat, QString("Missing tag \"%1\" in \"%2\" section").arg("binMap").arg(oBin.nodeName()),oBinSetElem);
                    break;
                }

                if(!readElementTextAttribute(oBinMap, "ID", true, strField))
                    break;
                m_oBinMapping[poBin->iBinValue] = strField.toInt();

                insertNewBinning(m_oSoftBins, &poBin);
                oBin = oBin.nextSiblingElement("bin");
            }


        }else if(strBinName.endsWith("PARAMETRIC")){

            if(oBinElement.childNodes().count()!=2){
                setError(errInvalidMaximKvdFormat, QString("section \"%1\" must contains tow \"%2\" ").arg(oBinElement.nodeName()).arg("Bin"),oBinSetElem);
                break;
            }

            QDomElement oBin1 = oBinElement.firstChildElement("bin");
            poBin = new CBinning;
            poBin->strBinName = strBinName;
            if(!readElementTextAttribute(oBin1, "desc", true, strField))
                break;
            poBin->strBinName = strField;
            if(!readElementTextAttribute(oBin1, "ID", true, strField))
                break;
            poBin->iBinValue = strField.toInt();
            if(!readElementTextAttribute(oBin1, "pass", true, strField))
                break;
            poBin->cPassFail = (strField.toLower() == "true") ? 'P' : (strField.toLower() == "false" ? 'F' : ' ');
            insertNewBinning(m_oHardBins, &poBin);

            QDomElement oBin2 = oBin1.nextSiblingElement("bin");
            poBin = new CBinning;
            poBin->strBinName = strBinName;
            if(!readElementTextAttribute(oBin2, "desc", true, strField))
                break;
            poBin->strBinName = strField;
            if(!readElementTextAttribute(oBin2, "ID", true, strField))
                break;
            poBin->iBinValue = strField.toInt();
            if(!readElementTextAttribute(oBin2, "pass", true, strField))
                break;
            poBin->cPassFail = (strField.toLower() == "true") ? 'P' : (strField.toLower() == "false" ? 'F' : ' ');
            insertNewBinning(m_oHardBins, &poBin);
        }
        bError = false;
        oBinElement = oBinElement.nextSiblingElement("binSet");
    }

    if(bError){
        if(poBin)
            delete poBin;
        if(!m_oSoftBins.isEmpty()){
            qDeleteAll(m_oSoftBins);
        }
        if(!m_oHardBins.isEmpty()){
            qDeleteAll(m_oHardBins);
        }

        m_oHardBins.clear();
        m_oHardBins.clear();
        setError(errInvalidMaximKvdFormat, QString("Error when parsing tag \"binSet\""),oBinSetElem);
        return false;
    }
    if(m_oSoftBins.isEmpty()&&m_oHardBins.isEmpty()){
        setError(errInvalidMaximKvdFormat, QString("Missing tag \"%1\" in \"%2\" section").arg("binSet").arg(oBinSetElem.nodeName()),oBinSetElem);
        return false;
    }
    return true;
}

bool CGKVDXMLtoSTDF::readLotUnderTestSection(const QDomNode &oLotUnderTestElem){


    if(!readElementTextAttribute(oLotUnderTestElem.toElement(),"lotID",true,m_oLotConfiguration))
        return false;
    m_oMIRRecord.SetLOT_ID(m_oLotConfiguration["lotID"].toString());

    if(!readElementTextAttribute(oLotUnderTestElem.toElement(),"sublotID",false,m_oLotConfiguration))
        return false;
    m_oMIRRecord.SetSBLOT_ID(m_oLotConfiguration["sublotID"].toString());

    if(!readElementTextAttribute(oLotUnderTestElem.toElement(),"parentLotID",false,m_oLotConfiguration,""))
        return false;
    if(!readElementTextAttribute(oLotUnderTestElem.toElement(),"datalogLotID",false,m_oLotConfiguration,""))
        return false;

    if(m_eDataType == DieSort){
        if(!readElementTextAttribute(oLotUnderTestElem.toElement(),"alternateLotID",false,m_oLotConfiguration,""))
            return false;
    }

    if(!readElementText(oLotUnderTestElem,"move",false,m_oLotConfiguration,""))
        return false;
    if(!readElementText(oLotUnderTestElem,"yield",false,m_oLotConfiguration,""))
        return false;

    QDomNode oWaferUnderTestElem = oLotUnderTestElem.namedItem("waferUnderTest");
    if(oWaferUnderTestElem.isNull()){
        setError(errInvalidMaximKvdFormat, QString("tag \"waferUnderTest\" not found"),oLotUnderTestElem);
        return false;
    }
    if(!readWaferUnderTestSection(oWaferUnderTestElem))
        return false;

    // case 7734
    // Missing endTimestamp from MIR and WIR
    if((m_oMRRRecord.m_u4FINISH_T == 0) && (m_oWaferMap.lWaferEndTime == 0))
    {
        setError(errInvalidMaximKvdFormat, QString("Missing tag \"endTimestamp\" in \"testOccurrence\" or \"waferUnderTest\"section"));
        return false;
    }

    QString strID, strName;
    int iCount = 0;
    CBinning *poBin = 0;
    //lotBinSummary
    QString strTag = "lotBinSummary";
    QDomNode oBinSummary = oLotUnderTestElem.firstChildElement(strTag);
    while(!oBinSummary.isNull()){
        if(!readElementTextAttribute(oBinSummary.toElement(),"name",true,strName))
            return false;
        QDomElement oBinCount = oBinSummary.firstChildElement("binCount");
        while(!oBinCount.isNull()){
            if(!readElementTextAttribute(oBinCount.toElement(),"ID",true,strID))
                return false;
            iCount = oBinCount.text().toInt();
            if(strName.endsWith("HARDWARE")){


            }else if(strName.endsWith("SOFTWARE")){

            }
            oBinCount = oBinCount.nextSiblingElement("binCount");
        }
        oBinSummary = oBinSummary.nextSiblingElement(strTag);
    }

    //waferBinSummary
    strTag = "waferBinSummary";
    //HBR/SBR
    QString strWaferID;
    oBinSummary = oLotUnderTestElem.firstChildElement(strTag);
    while(!oBinSummary.isNull()){
        if(!readElementTextAttribute(oBinSummary.toElement(),"name",true,strName))
            return false;
        if(!readElementTextAttribute(oBinSummary.toElement(),"waferID",false,strWaferID))
            return false;
        QDomElement oBinCount = oBinSummary.firstChildElement("binCount");
        while(!oBinCount.isNull()){
            if(!readElementTextAttribute(oBinCount.toElement(),"ID",true,strID))
                return false;
            iCount = oBinCount.text().toInt();
            // Message::information(0,"Fill waferBinSummary",
            //                      "Fill waferBinSummary");
            if(strName.endsWith("HARDWARE")){
                poBin = getBinning(m_oHardBins, strID.toInt());
                poBin->ldTotalCount = iCount;

            }else if(strName.endsWith("SOFTWARE")){
                poBin = getBinning(m_oSoftBins, strID.toInt());
                poBin->ldTotalCount = iCount;
            }
            oBinCount = oBinCount.nextSiblingElement("binCount");
        }
        oBinSummary = oBinSummary.nextSiblingElement(strTag);
    }

    //waferTestSummary => TSR
    QDomNode oWaferTestSummary = oLotUnderTestElem.firstChildElement("waferTestSummary");
    QString strTestId;
    while(!oWaferTestSummary.isNull()){
        QMap <QString, QVariant> oWaferTestSummaryData;

        if(!readElementTextAttribute(oWaferTestSummary.toElement(),"testID",true,strTestId))
            return false;
        if(!readElementTextAttribute(oWaferTestSummary.toElement(),"waferID",false,strWaferID))
            return false;
        CTest *poTest = getTest(m_oTestFlow, strTestId.toUInt());
        if(!poTest){
            setError(errInvalidMaximKvdFormat, QString("Missing test number %1").arg(strTestId),oLotUnderTestElem);
            return false;
        }
        oWaferTestSummaryData["HEAD_NUM"] = 255;
        oWaferTestSummaryData["SITE_NUM"] = -1;

        oWaferTestSummaryData["TEST_TYP"] = poTest->bTestType;
        oWaferTestSummaryData["testID"] = strTestId.toInt();
        oWaferTestSummaryData["TEST_NAME"] = poTest->strTestName;

        if(!readElementText(oWaferTestSummary,"executions",false,oWaferTestSummaryData))
            return false;
        //poTest->ldSamplesExecs =  poTest->ldExecs = strField.toInt();
        if(!readElementText(oWaferTestSummary,"fails",false,oWaferTestSummaryData))
            return false;
        //poTest->ldSampleFails = poTest->ldFailCount = strField.toInt();
        if(!readElementText(oWaferTestSummary,"alarms",false,oWaferTestSummaryData))
            return false;
        //poTest->iAlarm = strField.toInt();
        char cOPT_FLAG = 0;
        if(!readElementText(oWaferTestSummary,"averageTestTime",false,oWaferTestSummaryData))
            return false;
        cOPT_FLAG |= STDF_MASK_BIT2;
        ///poTest->
        if(!readElementText(oWaferTestSummary,"min",false,oWaferTestSummaryData))
            return false;
        cOPT_FLAG |= STDF_MASK_BIT0;
        //poTest->lfMin = poTest->lfSamplesMin = strField.toDouble();
        if(!readElementText(oWaferTestSummary,"max",false,oWaferTestSummaryData))
            return false;
        cOPT_FLAG |= STDF_MASK_BIT1;
        //poTest->lfMax = poTest->lfSamplesMax = strField.toDouble();
        if(!readElementText(oWaferTestSummary,"sum",false,oWaferTestSummaryData))
            return false;
        cOPT_FLAG |= STDF_MASK_BIT4;
        //poTest->lfSamplesTotal = poTest->lfTotal = strField.toDouble();

        if(!readElementText(oWaferTestSummary,"sumOfSqrs",false,oWaferTestSummaryData))
            return false;
        cOPT_FLAG |= STDF_MASK_BIT5;
        //poTest->lfSamplesTotalSquare = poTest->lfTotalSquare = strField.toDouble();
        oWaferTestSummaryData["OPT_FLAG"] = cOPT_FLAG;
        oWaferTestSummary = oWaferTestSummary.nextSiblingElement("waferTestSummary");

        m_oWaferTestSummaryList.append(oWaferTestSummaryData);
    }

    QString strField;
    QDomNode oWaferYieldSet = oLotUnderTestElem.firstChildElement("waferYieldSet");
    if(!oWaferYieldSet.isNull()){
        if(!readElementTextAttribute(oWaferYieldSet.toElement(),"waferID",false,strField))
            return false;
        if(!readElementTextAttribute(oWaferYieldSet.toElement(),"yield",false,strField))
            return false;
        if(!readElementTextAttribute(oWaferYieldSet.toElement(),"move",false,strField))
            return false;
    }

    return true;
}

bool CGKVDXMLtoSTDF::readWaferUnderTestSection(const QDomNode &oWaferUnderTestElem)
{

    QString strField;
    if(!readElementTextAttribute(oWaferUnderTestElem.toElement(),"ID",false,strField ))
        return false;
    qstrcpy(m_oWaferMap.szWaferID, strField.toLatin1().constData());

    if(!readElementTextAttribute(oWaferUnderTestElem.toElement(),"fabID",false,strField ))
        return false;
    m_oWaferConfiguration["fabID"] = strField.simplified();

    if(!readElementText(oWaferUnderTestElem,"siteGroup",false,strField))
        return false;
    m_oWaferConfiguration["siteGroup"] = strField.simplified();

    if(!readElementText(oWaferUnderTestElem,"startTimestamp",true,strField))
        return false;
    m_oWaferMap.lWaferStartTime = getTime(strField.simplified());

    if(!readElementText(oWaferUnderTestElem,"endTimestamp",false,strField))
        return false;
    m_oWaferMap.lWaferEndTime = getTime(strField.simplified());

    // GCORE-102 : if no testoccurence end time, use the one from the wafer
    if (m_oMRRRecord.m_u4FINISH_T == -1 || m_oMRRRecord.m_u4FINISH_T == 0)
    {
        GSLOG(SYSLOG_SEV_NOTICE, "No test occurence end time found. Using the one from the wafer");
        m_oMRRRecord.m_u4FINISH_T = getTime(strField);
    }

    // case 7734
    // Missing endTimestamp from MIR and WIR
    if((m_oMRRRecord.m_u4FINISH_T == 0) && (m_oWaferMap.lWaferEndTime == 0))
    {
        setError(errInvalidMaximKvdFormat,
                 QString("Missing tag \"endTimestamp\" in \"testOccurrence\" or \"waferUnderTest\"section"));
        return false;
    }

    if(!readElementText(oWaferUnderTestElem,"frameID",false,strField,""))
        return false;
    m_oWaferConfiguration["frameID"] = strField.simplified();

    if(!readElementText(oWaferUnderTestElem,"userDescription",false,strField,""))
        return false;
    m_oWaferConfiguration["userDescription"] = strField.simplified();
    if(!readElementText(oWaferUnderTestElem,"execDescription",false,strField,""))
        return false;
    m_oWaferConfiguration["execDescription"] = strField.simplified();

    if(!readElementText(oWaferUnderTestElem,"areaPerTest",false,strField,""))
        return false;
    int iDieUnderTestCount = oWaferUnderTestElem.toElement().elementsByTagName("dieUnderTest").count();
    QDomElement oDieUnderTest = oWaferUnderTestElem.firstChildElement("dieUnderTest");
    QMap<int,bool> lMapHBinCat;
    for(int iIdx=0; iIdx<m_oHardBins.count(); iIdx++)
    {
        CBinning *poBin = m_oHardBins[iIdx];
        lMapHBinCat[poBin->iBinValue] = (poBin->cPassFail == 'P');
    }
    QMap<QString, int> lXYCoordsCount;
    while(!oDieUnderTest.isNull()){
        GQTL_STDF::Stdf_PRR_V4 oPRR;oPRR.Reset();
        GQTL_STDF::Stdf_PIR_V4 oPIR;oPIR.Reset();

        oPIR.SetHEAD_NUM(1);
        oPRR.SetHEAD_NUM(1);

        if(!readElementTextAttribute(oDieUnderTest,"ID",true,strField))
            return false;
        oPRR.SetPART_ID(strField);

        if(!readElementTextAttribute(oDieUnderTest,"x",true,strField))
            return false;
        oPRR.SetX_COORD(strField.toInt());

        if(!readElementTextAttribute(oDieUnderTest,"y",true,strField))
            return false;
        oPRR.SetY_COORD(strField.toInt());

        if(lXYCoordsCount.contains(QString("%1 - %2").arg(oPRR.m_i2X_COORD).arg(oPRR.m_i2Y_COORD)))
            lXYCoordsCount[QString("%1 - %2").arg(oPRR.m_i2X_COORD).arg(oPRR.m_i2Y_COORD)]++;
        else
            lXYCoordsCount[QString("%1 - %2").arg(oPRR.m_i2X_COORD).arg(oPRR.m_i2Y_COORD)] = 1;

        if(!readElementText(oDieUnderTest,"siteNum",false,strField,"1"))
            return false;
        oPIR.SetSITE_NUM(strField.toInt());
        oPRR.SetSITE_NUM(strField.toInt());

        if(!readElementText(oDieUnderTest,"elapsedTime",false,strField,"0"))
            return false;
        oPRR.SetTEST_T(strField.toULong());
        //conditionSet
        QDomElement oConditionSet = oDieUnderTest.namedItem("conditionSet").toElement();
        if(!oConditionSet.isNull()){
            oPRR.SetNUM_TEST(oConditionSet.elementsByTagName("m").count());

        }else
            oPRR.SetNUM_TEST(0);

        //Loop on test Result
        QDomElement oTestResult = oConditionSet.firstChildElement("m");
        while(!oTestResult.isNull())
        {
            QString strTestId, strResult, strContent;
            if(!readElementTextAttribute(oTestResult,"testID",true,strTestId))
                return false;
            CTest *poTest = getTest(m_oTestFlow, strTestId.toInt());
            if(!poTest){
                setError(errInvalidMaximKvdFormat, QString("Missing test number %1").arg(strTestId),oWaferUnderTestElem);
                return false;
            }

            // case 7734
            // Check if Test limits are defined
            CTestResult::PassFailStatus ePassFailStatus = CTestResult::statusUndefined;
            if(!readElementTextAttribute(oTestResult,"result",true,strResult))
                ePassFailStatus = CTestResult::statusUndefined;
            else
                ePassFailStatus = (strResult == "p" ? CTestResult::statusPass : CTestResult::statusFail);
            strContent = oTestResult.text();

            if(ePassFailStatus == CTestResult::statusUndefined)
            {
                // Pass flag is not defined
                // check if have limits defined
                if((poTest->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL)
                        || (poTest->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL))
                {
                    // error
                    m_strLastError += QString(" for testID[%1] - no limits defined").arg(strTestId);
                    return false;
                }
                if((strContent.toDouble() >= poTest->GetCurrentLimitItem()->lfLowLimit)
                        && (strContent.toDouble() <= poTest->GetCurrentLimitItem()->lfHighLimit))
                    ePassFailStatus = CTestResult::statusPass;
                else
                    ePassFailStatus = CTestResult::statusFail;
            }

            if(!poTest->m_testResult.count())
                poTest->m_testResult.createResultTable(iDieUnderTestCount, false);

            poTest->m_testResult.pushResultAt(m_oPRRecordList.count(), strContent.toDouble());
            poTest->m_testResult.setPassFailStatusAt(m_oPRRecordList.count(), ePassFailStatus);
            oTestResult = oTestResult.nextSiblingElement("m");
        }

        QDomNode oBinAssignment = oDieUnderTest.namedItem("binAssignment");
        if(!readElementTextAttribute(oBinAssignment.toElement(),"ID",true,strField))
            return false;
        oPRR.SetSOFT_BIN(strField.toInt());
        oPRR.SetHARD_BIN(m_oBinMapping[strField.toInt()]);

        char cPART_FLG = 0;
        if(lMapHBinCat.contains(m_oBinMapping[strField.toInt()]))
        {
            if(!lMapHBinCat[m_oBinMapping[strField.toInt()]])
                cPART_FLG |= STDF_MASK_BIT3;
        }
        else
            cPART_FLG |= STDF_MASK_BIT4;

        oPRR.SetPART_FLG(cPART_FLG);
        m_oPRRecordList.append(oPRR);
        m_oPIRecordList.append(oPIR);
        oDieUnderTest = oDieUnderTest.nextSiblingElement("dieUnderTest");
    }

    if(lXYCoordsCount.count() <= 1)
        m_bValidCoord = false;

    /*
    int iReticleUnderTestCount = oWaferUnderTestElem.toElement().elementsByTagName("reticleUnderTest").count();
    QDomElement oReticleUnderTest = oWaferUnderTestElem.firstChildElement("reticleUnderTest");
    while(!oReticleUnderTest.isNull()){
        GQTL_STDF::Stdf_PRR_V4 oPRR;oPRR.Reset();
        GQTL_STDF::Stdf_PIR_V4 oPIR;oPIR.Reset();

        oPIR.SetHEAD_NUM(1);
        oPRR.SetHEAD_NUM(1);

        if(!readElementTextAttribute(oReticleUnderTest,"ID",true,strField))
            return false;
        oPRR.SetPART_ID(strField);

        if(!readElementTextAttribute(oReticleUnderTest,"x",true,strField))
            return false;
        oPRR.SetX_COORD(strField.toInt());

        if(!readElementTextAttribute(oReticleUnderTest,"y",true,strField))
            return false;
        oPRR.SetY_COORD(strField.toInt());

        if(!readElementText(oReticleUnderTest,"siteNum",true,strField))
            return false;
        oPIR.SetSITE_NUM(strField.toInt());
        oPRR.SetSITE_NUM(strField.toInt());

        if(!readElementText(oReticleUnderTest,"elapsedTime",true,strField))
            return false;
        oPRR.SetTEST_T(strField.toFloat());
        //conditionSet
        QDomElement oConditionSet = oReticleUnderTest.namedItem("conditionSet").toElement();
        if(oConditionSet.isNull()){
            setError(errInvalidMaximKvdFormat, QString("Missing tag \"%1\" ").arg(oConditionSet.nodeName()));
            return false;

        }
        oPRR.SetNUM_TEST(oConditionSet.elementsByTagName("m").count());

        //Loop on test Result
        QDomElement oTestResult = oConditionSet.firstChildElement("m");
        while(!oTestResult.isNull()){
            QString strTestId, strResult, strContent;
            if(!readElementTextAttribute(oTestResult,"testID",true,strTestId))
                return false;
            CTest *poTest = getTest(m_oTestFlow, strTestId.toInt());

            if(!readElementTextAttribute(oTestResult,"result",true,strResult))
                return false;
            //int iTestFlag = getTestFlag(strResult);
            CTestResult::PassFailStatus ePassFailStatus = (strResult == "p" ? CTestResult::statusPass : CTestResult::statusFail);
            strContent = oTestResult.text();

            if(!poTest->m_testResult.count())
                poTest->m_testResult.createResultTable(iReticleUnderTestCount, false);

            poTest->m_testResult.pushResultAt(m_oPRRecordList.count(), strContent.toDouble());
            poTest->m_testResult.setPassFailStatusAt(m_oPRRecordList.count(), ePassFailStatus);
            oTestResult = oTestResult.nextSiblingElement("m");
        }

        QDomNode oBinAssignment = oReticleUnderTest.namedItem("binAssignment");
        if(!readElementTextAttribute(oBinAssignment.toElement(),"ID",true,strField))
            return false;
        oPRR.SetSOFT_BIN(strField.toInt());
        oPRR.SetHARD_BIN(m_oBinMapping[strField.toInt()]);



        char cPART_FLG = 0;
        oPRR.SetPART_FLG(cPART_FLG);
        m_oPRRecordList.append(oPRR);
        m_oPIRecordList.append(oPIR);
        oReticleUnderTest = oReticleUnderTest.nextSiblingElement("reticleUnderTest");
    }*/
    return true;
}

bool CGKVDXMLtoSTDF::readPartCount(const QDomNode & oPartCountElem){
    m_oPCRRecord.Reset();
    m_bPCRExist = true;
    QString strField;
    m_oPCRRecord.SetHEAD_NUM(255);
    m_oPCRRecord.SetSITE_NUM(255);
    if(!readElementText(oPartCountElem,"partCount",false,strField))
        return false;
    m_oPCRRecord.SetPART_CNT(strField.toInt());

    if(!readElementText(oPartCountElem,"retestCount",false,strField))
        return false;
    m_oPCRRecord.SetRTST_CNT(strField.toInt());

    if(!readElementText(oPartCountElem,"abortCount",false,strField))
        return false;
    m_oPCRRecord.SetABRT_CNT(strField.toInt());

    if(!readElementText(oPartCountElem,"goodCount",false,strField))
        return false;
    m_oPCRRecord.SetGOOD_CNT(strField.toInt());

    if(!readElementText(oPartCountElem,"functionalCount",false,strField))
        return false;
    m_oPCRRecord.SetFUNC_CNT(strField.toInt());
    return true;
}

time_t CGKVDXMLtoSTDF::getTime(const QString &strDateTime)
{
    if (strDateTime.isEmpty()) // no need to continue
        return 0;
    //"dd-MMM-yyyy"
    QString strDate = strDateTime.section(" ",0,0);
    int iDay = strDate.section("-",0,0).toInt();
    QString strMonth = strDate.section("-",1,1);
    int iMonth = 1;
    if(strMonth.toLower() == "jan") iMonth = 1;
    if(strMonth.toLower() == "feb") iMonth = 2;
    if(strMonth.toLower() == "mar") iMonth = 3;
    if(strMonth.toLower() == "apr") iMonth = 4;
    if(strMonth.toLower() == "may") iMonth = 5;
    if(strMonth.toLower() == "jun") iMonth = 6;
    if(strMonth.toLower() == "jul") iMonth = 7;
    if(strMonth.toLower() == "aug") iMonth = 8;
    if(strMonth.toLower() == "sep") iMonth = 9;
    if(strMonth.toLower() == "oct") iMonth = 10;
    if(strMonth.toLower() == "nov") iMonth = 11;
    if(strMonth.toLower() == "dec") iMonth = 12;
    int iYear = strDate.section("-",2,2).toInt();
    if (iYear < 70)
        iYear += 2000;
    else
        if (iYear <= 99)
            iYear += 1900;
    QDate oDate(iYear, iMonth, iDay);
    QTime oTime = QTime::fromString(strDateTime.section(" ",1,1),"hh:mm:ss");
    return QDateTime(oDate, oTime,Qt::UTC).toTime_t();
}

char CGKVDXMLtoSTDF::getModeCodes(const QString &strFiledData){

    if(strFiledData == "PROD")// Production
        return 'P';
    if(strFiledData == "DEV")//  Development
        return 'D';
    if(strFiledData == "ENG")//  Engineering
        return 'E';
    if(strFiledData == "COR")//  Correlation
        return 'C';
    if(strFiledData == "REF")//  Reference
        return 'R';

    return char(0);
}

char CGKVDXMLtoSTDF::getWF_FLAT(const QString &strFiledData){
    if(strFiledData ==  "UP") return 'U';
    if(strFiledData ==  "DOWN") return 'D';
    if(strFiledData ==  "LEFT") return 'L';
    if(strFiledData ==  "RIGHT") return 'R';
    return ' ';
}

bool CGKVDXMLtoSTDF::insertNewBinning(QList <CBinning *> &oBinsList, CBinning **poNewBin){

    if(!oBinsList.isEmpty()){
        foreach (CBinning *poBin, oBinsList) {
            if(poBin->iBinValue == (*poNewBin)->iBinValue){
                delete (*poNewBin);
                (*poNewBin) = poBin;
                return false;
            }
        }
    }
    oBinsList.append(*poNewBin);
    return true;
}

CBinning *CGKVDXMLtoSTDF::getBinning(QList <CBinning *> &oBinsList, int iBin){

    if(!oBinsList.isEmpty()){
        foreach (CBinning *poBin, oBinsList) {
            if(poBin->iBinValue == iBin){
                return poBin;
            }
        }
    }
    return 0;
}

char CGKVDXMLtoSTDF::getTestFlag(const QString &strResult){
    if(strResult == "p") // Pass (testLow/testHigh)
        return 0;
    else
        return STDF_MASK_BIT7;


}

CTest *CGKVDXMLtoSTDF::getTest(QList <CTest *> &oTestList, unsigned int iTestID){

    if(!oTestList.isEmpty()){
        foreach (CTest *poTest, oTestList) {
            if(poTest->lTestNumber == iTestID){
                return poTest;
            }
        }
    }
    return 0;
}

bool CGKVDXMLtoSTDF::readElementText(const QDomNode &oDomNode,
                                     const QString &strItem,
                                     bool bRequired,
                                     QMap<QString, QVariant> &oMap, const QString &strDefault){

    if(!bRequired){
        //        if(strDefault.isEmpty())
        //            GS::Gex::Message::information("Empty Field",
        //                                          QString("tag(%1) Item(%2)").
        //                                          arg(oDomNode.nodeName()).
        //                                          arg(strItem));
    }

    QString strValue;
    if(!oDomNode.namedItem(strItem).isNull())
        strValue = oDomNode.namedItem(strItem).toElement().text();
    if(strValue.simplified().toUpper() == "NA")
        strValue = "";

    if(!strValue.isEmpty()){
        oMap[strItem] = oDomNode.namedItem(strItem).toElement().text();
        return true;
    }else{
        oMap[strItem] = "";
        if(!bRequired)
            oMap[strItem] = strDefault;
        if(bRequired)
            setError(errInvalidMaximKvdFormat, QString("Missing tag \"%1\" in \"%2\" section").arg(strItem).arg(oDomNode.nodeName()),oDomNode);
        return !bRequired;
    }
}

bool CGKVDXMLtoSTDF::readElementText(const QDomNode &oDomNode,
                                     const QString &strItem,
                                     bool bRequired,
                                     QString &strData, const QString &strDefault){
    if(!bRequired){
        //        if(strDefault.isEmpty())
        //            GS::Gex::Message::information("Empty Field",
        //                                          QString("tag(%1) Item(%2)").
        //                                          arg(oDomNode.nodeName()).
        //                                          arg(strItem));
    }
    QString strValue;
    if(!oDomNode.namedItem(strItem).isNull())
        strValue = oDomNode.namedItem(strItem).toElement().text();
    if(strValue.simplified().toUpper() == "NA")
        strValue = "";

    if(!strValue.isEmpty()){
        strData =  oDomNode.namedItem(strItem).toElement().text();;
        return true;
    }else{
        strData = "";
        if(!bRequired)
            strData = strDefault;
        if(bRequired)
            setError(errInvalidMaximKvdFormat, QString("Missing tag \"%1\" in \"%2\" section").arg(strItem).arg(oDomNode.nodeName()),oDomNode);
        return !bRequired;
    }
}

bool CGKVDXMLtoSTDF::readElementTextAttribute(const QDomElement &oDomElement,
                                              const QString &strAttribute,
                                              bool bRequired,
                                              QMap<QString, QVariant> &oMap, const QString &strDefault){
    if(!bRequired){
        //        if(strDefault.isEmpty())
        //            GS::Gex::Message::information("Empty Field",
        //                                          QString("tag(%1) Item(%2)").
        //                                          arg(oDomElement.nodeName()).
        //                                          arg(strAttribute));
    }

    QString strValue;
    if(!oDomElement.attribute(strAttribute).isEmpty())
        strValue = oDomElement.attribute(strAttribute);
    if(strValue.simplified().toUpper() == "NA")
        strValue = "";

    if(!strValue.isEmpty()){
        oMap[strAttribute] = oDomElement.attribute(strAttribute);
        return true;
    }else{
        oMap[strAttribute] = "";
        if(!bRequired)
            oMap[strAttribute] = strDefault;
        if(bRequired)
            setError(errInvalidMaximKvdFormat, QString("Missing attribute\"%1\" in \"%2\" section").arg(strAttribute).arg(oDomElement.nodeName()),oDomElement.parentNode());
        return !bRequired;
    }

}

bool CGKVDXMLtoSTDF::readElementTextAttribute(const QDomElement &oDomElement,
                                              const QString &strAttribute,
                                              bool bRequired,
                                              QString &strData, const QString &strDefault){
    if(!bRequired){
        //        if(strDefault.isEmpty())
        //            GS::Gex::Message::information("Empty Field",
        //                                          QString("tag(%1) Item(%2)").
        //                                          arg(oDomElement.nodeName()).
        //                                          arg(strAttribute));
    }

    QString strValue;
    if(!oDomElement.attribute(strAttribute).isEmpty())
        strValue = oDomElement.attribute(strAttribute);
    if(strValue.simplified().toUpper() == "NA")
        strValue = "";

    if(!strValue.isEmpty()){
        strData =   oDomElement.attribute(strAttribute);
        return true;
    }else{
        strData = "";
        if(!bRequired)
            strData = strDefault;
        if(bRequired)
            setError(errInvalidMaximKvdFormat, QString("Missing attribute \"%1\" in \"%2\" section").arg(strAttribute).arg(oDomElement.nodeName()),oDomElement.parentNode());
        return !bRequired;
    }


}

void CGKVDXMLtoSTDF::setProgress(double dVal, const QString &strMessage)
{
    if(GexProgressBar)
    {
        GexProgressBar->setValue((int)dVal);
        if(GexScriptStatusLabel && !strMessage.isEmpty())
            GS::Gex::Engine::GetInstance().UpdateLabelStatus(strMessage);
        QCoreApplication::processEvents();
    }
}

int CGKVDXMLtoSTDF::getProgress(){

    if(GexProgressBar)
        return GexProgressBar->value();
    return 0;
}

