#include "import_93ktab.h"
#include "engine.h"

#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QProgressBar>
#include <QApplication>
#include <QLabel>
#include <QtDebug>
extern QProgressBar	*	GexProgressBar;		// Handle to progress bar in status bar
extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar

#define FIELDS_COUNT_93KTAB 11

QString Parser93kTab::m_strLastError = "";
int		Parser93kTab::m_iLastError = Parser93kTab::errNoError;

Parser93kTab::Parser93kTab()
{
    init();
}

Parser93kTab::~Parser93kTab(){
    init();

}

bool Parser93kTab::Convert(const QString &szFileName, const QString &strFileNameSTDF)
{
    init();
    QFile oFile(szFileName);
    if(!oFile.open(QIODevice::ReadOnly)){
        setError(errOpenFail);
        return false;
    }
    QTextStream oTextStream(&oFile);
    double dFileSize = oFile.size();
    double dReadByte = 0;
    bool bReadNextLine = true;
    QString strLastLine;
    bool bFirstFlow = true;
    while(!oTextStream.atEnd()){
        QString strLine;
        if(bReadNextLine){
            strLine = oTextStream.readLine();
            dReadByte += strLine.count();
            setProgress((int)(dFileSize>0 ? (dReadByte/dFileSize) : 100));
        }else {
            strLine = strLastLine;
            bReadNextLine = true;
        }


        if(strLine.startsWith("*TEST PROGRAM START")){
            //MIR/MRR info
            QString strProgram = strLine.section(":",1,3);
            QStringList oFields = strProgram.split(" ");
            QString strDateAndTime="";
            if(oFields.count()>2)
                strDateAndTime = QString(oFields[oFields.count()-2]+" "+oFields[oFields.count()-1]).simplified();
            strDateAndTime.replace(" ","-");
            m_oMIRRecord.SetSETUP_T(QDateTime::fromString(strDateAndTime,"yyyy/MM/dd-hh:mm:ss").toTime_t());
        }else if(strLine.startsWith("*TEST PROGRAM END")){
            //MIR/MRR info
            QString strDateAndTime = strLine.section(":",1,3);
            strDateAndTime = strDateAndTime.simplified();
            strDateAndTime.replace(" ","-");
            m_oMRRRecord.SetFINISH_T(QDateTime::fromString(strDateAndTime.simplified(),"yyyy/MM/dd-hh:mm:ss").toTime_t());

        }else if(strLine.startsWith("*TEST MODE")){
            //Mir/MRR info
            QString strMode = strLine.section(":",1,1);
            m_oMIRRecord.SetMODE_COD(strMode[0].toLatin1());

        }else if(strLine.startsWith("*TEST INFORMATION")){
            ///Mir/MRR info
            QString strInfo = strLine.section(":",1,1);
            QString strTitle = strInfo.section("=",0,0);
            QString strData = strInfo.section("=",1,1);
            strData.clear();
            if(strTitle=="Testprogram"){
                m_oMIRRecord.SetJOB_NAM(strData);

            }else if(strTitle=="Testflow"){
                m_oMIRRecord.SetFLOW_ID(strData);

            }else if(strTitle=="TestflowSavedTime"){

            }else if(strTitle=="DeviceName"){
                m_oMIRRecord.SetPART_TYP(strData);
            }else if(strTitle=="DeviceRev"){
                m_oMIRRecord.SetDSGN_REV(strData);
            }else if(strTitle=="TestRev"){
                m_oMIRRecord.SetJOB_REV(strData);
            }else if(strTitle=="TestDesc"){
                m_oMIRRecord.SetUSER_TXT(strData);
            }else if(strTitle=="LotNumber"){
                m_oMIRRecord.SetLOT_ID(strData);

            }else if(strTitle=="LotName"){

            }else if(strTitle=="RetestCount"){
            }else if(strTitle=="ProcCode"){
                m_oMIRRecord.SetPROC_ID(strData);
            }else if(strTitle=="TestCount"){
            }else if(strTitle=="Product"){
            }else if(strTitle=="OfficialProduct"){
            }else if(strTitle=="IsWaferTest"){

            }
       }else if(strLine.startsWith("*TESTFLOW START")){
            //PIR PRR  info
            //PTR, MPR info
            QString strTemp = strLine.section(":",1,3).simplified();
            strTemp = strTemp.replace(" ","-");
            uint uiStartFlowDateTime = QDateTime::fromString(strTemp,"YYYY/MM/dd-hh:mm:ss").toTime_t();
            uint uiEndFlowDateTime = uiStartFlowDateTime;

            if(bFirstFlow){
                bFirstFlow = false;
                m_oMIRRecord.SetSTART_T(uiStartFlowDateTime);

            }
            int iSite = 1;
            QString strPartID = "";
            QStringList oBinningInfo;
            QList<GQTL_STDF::Stdf_PTR_V4*> oPTRRecords;
            QList<GQTL_STDF::Stdf_MPR_V4*> oMPRRecords;
            QMap<int, QStringList> oMPRPINList;
            do{
                if(bReadNextLine)
                    strLine = oTextStream.readLine();
                else {
                    strLine = strLastLine;
                    bReadNextLine = true;
                }

                if(strLine.startsWith("*BIN REACHED")){
                    QString strBinReached = strLine.section(":",1,1).simplified();
                    QStringList oBining = strBinReached.split(" ");
                    //*BIN REACHED:    0      1       2        3       4
                    //*BIN REACHED: PartID SiteID Fail/Pass SoftBin HardBin
                    //*BIN REACHED:     P1    1       F        15      2
                    if(oBining.count()!=5){
                        setError(errInvalidFujitsuParserFormat, strLine);
                        return false;
                    }else {//oBinningInfo
                        if(oBinningInfo.isEmpty() ){
                            oBinningInfo = oBining;
                            bool bNewBinSoft = false;
                            if(!m_oBinningMap.contains(oBinningInfo[3].toInt())){
                                m_oBinningMap[oBinningInfo[3].toInt()] = oBinningInfo[4].toInt();
                                bNewBinSoft = true;
                            }
                            if(bNewBinSoft){
                                GQTL_STDF::Stdf_SBR_V4 *poSBR = new GQTL_STDF::Stdf_SBR_V4;
                                poSBR->SetHEAD_NUM(1);
                                poSBR->SetSITE_NUM(oBinningInfo[1].toInt());
                                poSBR->SetSBIN_NUM(oBinningInfo[3].toInt());
                                poSBR->SetSBIN_CNT(1) ;
                                poSBR->SetSBIN_PF(oBinningInfo[2][0].toLatin1());
                                poSBR->SetSBIN_NAM("");
                                m_oSBRList.append(poSBR);
                            }else{
                                foreach(GQTL_STDF::Stdf_SBR_V4 *poSBR, m_oSBRList){
                                    if(poSBR->m_u2SBIN_NUM == oBinningInfo[3].toInt())
                                        poSBR->SetSBIN_CNT(poSBR->m_u4SBIN_CNT+1);
                                }
                            }

                            bool bNewBinHard = true;
                            foreach(GQTL_STDF::Stdf_HBR_V4 *poHBR, m_oHBRList){
                                if(poHBR->m_u2HBIN_NUM == oBinningInfo[4].toInt()){
                                    poHBR->SetHBIN_CNT(poHBR->m_u4HBIN_CNT+1);
                                    bNewBinHard = false;
                                    break;
                                }
                            }

                            if(bNewBinHard){
                                GQTL_STDF::Stdf_HBR_V4 *poHBR = new GQTL_STDF::Stdf_HBR_V4;
                                poHBR->SetHEAD_NUM(1);
                                poHBR->SetSITE_NUM(oBinningInfo[1].toInt());
                                poHBR->SetHBIN_NUM(oBinningInfo[4].toInt());
                                poHBR->SetHBIN_CNT(1) ;
                                poHBR->SetHBIN_PF(oBinningInfo[2][0].toLatin1());
                                poHBR->SetHBIN_NAM("");
                                m_oHBRList.append(poHBR);
                            }
                        }
                    }
                }if(strLine.startsWith("*TESTFLOW END")){
                    QString strTempDate = strLine.section(":",1,4);
                    strTempDate = strTempDate.remove(strTempDate.lastIndexOf(" "),strTempDate.count()-strTempDate.lastIndexOf(" ")+1);
                    strTempDate = strTempDate.simplified();
                    strTempDate = strTempDate.replace(" ","-");
                    uiEndFlowDateTime = QDateTime::fromString(strTempDate.simplified(),"YYYY/MM/dd-hh:mm:ss").toTime_t();
                    qint64 oElapsedTime = uiEndFlowDateTime - uiStartFlowDateTime;

                    //generate PIR/PRR
                    GQTL_STDF::Stdf_PIR_V4 *poPIR = new GQTL_STDF::Stdf_PIR_V4;
                    poPIR->SetHEAD_NUM(1);
                    poPIR->SetSITE_NUM(iSite);
                    m_oPIRList.append(poPIR);

                    GQTL_STDF::Stdf_PRR_V4 *poPRR = new GQTL_STDF::Stdf_PRR_V4;
                    poPRR->SetHEAD_NUM(1);
                    poPRR->SetSITE_NUM(iSite);
                    char cPartFlag = 0;
                    cPartFlag |= (oBinningInfo[2] == "P" ? 0 : STDF_MASK_BIT3);
                    poPRR->SetPART_FLG(cPartFlag);
                    poPRR->SetNUM_TEST(oPTRRecords.count() + oMPRRecords.count());
                    poPRR->SetHARD_BIN(oBinningInfo[4].trimmed().toInt());
                    poPRR->SetSOFT_BIN(oBinningInfo[3].trimmed().toInt());
                    poPRR->SetX_COORD (INVALID_SMALLINT);
                    poPRR->SetY_COORD (INVALID_SMALLINT);
                    poPRR->SetTEST_T  (oElapsedTime);
                    poPRR->SetPART_ID (strPartID);
                    poPRR->SetPART_TXT("");
                    m_oPRRList.append(poPRR);

                    m_oPTRList.insert(m_oPIRList.count(), oPTRRecords);
                    m_oMPRList.insert(m_oPIRList.count(), oMPRRecords);
                    m_oMPRPINList.insert(m_oPIRList.count(), oMPRPINList);

                }else if(strLine.startsWith("P")){

/*
--PTR
 => @ sign means no pin? PTR
 =>Pin name (when @ there is no pin, so it means it is a PTR
 0        1                    2                                 3                4                5         6          7           8          9        10
PartID|SiteID|Fail/Pass Status flag (when . it means Pass)|Sequencer name|?                    |Test name|Pin name |measurement|Low Limit| High Limit| Unit
P1    |1     |.                                           |Supply_Test   |Everest_...Short_Test|Idd_AVDRF|@        |  0.167774 |      -1 |       5   |mA
--MPR
0        1        2         3               4                          5         6             7              8            9        10
PartID|SiteID|Fail/Pass| Sequencer name | ?                      | Test name   | Pin name   | measurement | Low Limit| High Limit | Unit
P1    |1     |.        |Continuity_HP   |dc_tml.DcTest.Continuity|Continuity_HP|ADC_HTEST1N | -0.484468   | -0.8     |  -0.2      |V
*/

                    //Test
                    QStringList oTestProp = strLine.split("|");
                    iSite = oTestProp[1].trimmed().toInt();
                    strPartID = oTestProp[0].trimmed();

                    if(oTestProp.count() == FIELDS_COUNT_93KTAB ){
                        //Check if PTR or MPR
                        if(oTestProp[6].trimmed() == "@"){
                            //isPTR
                            processPTR(oTestProp, oPTRRecords);

                        }else {
                            //MPR
                            processMPR(oTestProp, oTextStream,oMPRRecords, oMPRPINList,bReadNextLine,strLastLine);
                        }

                    }else if(oTestProp.count() != FIELDS_COUNT_93KTAB){

                    }else{
                        setError(errInvalidFujitsuParserFormat, strLine);
                        return false;
                    }

                }
            }while(!strLine.startsWith("*TESTFLOW END") || oTextStream.atEnd());

        }
    }

    if(!WriteStdfFile(0,strFileNameSTDF))
    {
      // Convertion failed.
      QFile::remove(strFileNameSTDF);
      return false;
    }


    return true;
}

bool Parser93kTab::IsCompatible(const QString &szFileName){
    QFile oFile(szFileName);
    if(!oFile.open(QIODevice::ReadOnly)){
        setError(errOpenFail);
        return false;
    }
    QTextStream oTextStream(&oFile);
    QString strLine = oTextStream.readLine();
    if(!strLine.isNull()){
        if(strLine.startsWith("*TEST PROGRAM START:"))
            return true;
    }

    return false;
}

QString Parser93kTab::GetLastError(){
    return m_strLastError;
}

int Parser93kTab::GetLastErrorCode(){
    return m_iLastError;
}

void Parser93kTab::setError(int iError, const QString &strAdditionalInfo){
    m_iLastError = iError;
    QString strBaseError;

    if(iError == errNoError){
        strBaseError = "No Error";
    }else if(iError == errOpenFail){
        strBaseError = "Failed to open file";
    }else if(iError == errInvalidFujitsuParserFormat){
        strBaseError = "Invalid 93kTab format";
    }else if(iError == errWriteSTDF){
        strBaseError = "Failed creating STDF file.";
    }else if(iError == errLicenceExpired){
        strBaseError = "License has expired or Data file out of date...";
    }
    m_strLastError = QString ("%1 \n %2").arg(strBaseError).arg(strAdditionalInfo);
    if(iError!= errNoError){
        if(GexProgressBar)
            GexProgressBar->setValue(100);
    }
}

void Parser93kTab::init(){
    setError(errNoError);
    m_oMIRRecord.Reset();
    m_oMIRRecord.SetSETUP_T (0);
    m_oMIRRecord.SetSTART_T (0);
    m_oMIRRecord.SetSTAT_NUM(0);
    m_oMIRRecord.SetMODE_COD(' ');
    m_oMIRRecord.SetRTST_COD(' ');
    m_oMIRRecord.SetPROT_COD(' ');
    m_oMIRRecord.SetBURN_TIM(0);
    m_oMIRRecord.SetCMOD_COD(' ');
    m_oMIRRecord.SetLOT_ID  ("NA");
    m_oMIRRecord.SetPART_TYP("NA");
    m_oMIRRecord.SetNODE_NAM("NA");
    m_oMIRRecord.SetTSTR_TYP("NA");
    m_oMIRRecord.SetJOB_NAM ("NA");
    m_oMIRRecord.SetJOB_REV ("NA");
    m_oMIRRecord.SetSBLOT_ID("NA");
    m_oMIRRecord.SetOPER_NAM("NA");
    m_oMIRRecord.SetEXEC_TYP("NA");
    m_oMIRRecord.SetEXEC_VER("NA");
    m_oMIRRecord.SetTEST_COD("NA");
    m_oMIRRecord.SetTST_TEMP("NA");
    m_oMIRRecord.SetUSER_TXT("NA");
    m_oMIRRecord.SetAUX_FILE("NA");
    m_oMIRRecord.SetPKG_TYP ("NA");
    m_oMIRRecord.SetFAMLY_ID("NA");
    m_oMIRRecord.SetDATE_COD("NA");
    m_oMIRRecord.SetFACIL_ID("NA");
    m_oMIRRecord.SetFLOOR_ID("NA");
    m_oMIRRecord.SetPROC_ID ("NA");
    m_oMIRRecord.SetOPER_FRQ("NA");
    m_oMIRRecord.SetSPEC_NAM("NA");
    m_oMIRRecord.SetSPEC_VER("NA");
    m_oMIRRecord.SetFLOW_ID ("NA");
    m_oMIRRecord.SetSETUP_ID("NA");
    m_oMIRRecord.SetDSGN_REV("NA");
    m_oMIRRecord.SetENG_ID  ("NA");
    m_oMIRRecord.SetROM_COD ("NA");
    m_oMIRRecord.SetSERL_NUM("NA");
    m_oMIRRecord.SetSUPR_NAM("NA");

    m_oMRRRecord.Reset();
    m_oTestNumbers.clear();
    m_iTestNumberCount=0;


    m_oMPRPINList.clear();
    if(m_oMPRList.count()){
        foreach(int iKey,m_oMPRList.keys()){
            if(m_oMPRList[iKey].count()){
                qDeleteAll(m_oMPRList[iKey]);
                m_oMPRList[iKey].clear();
            }
        }
        m_oMPRList.clear();
    }
    if(m_oPTRList.count()){
        foreach(int iKey,m_oPTRList.keys()){
            if(m_oPTRList[iKey].count()){
                qDeleteAll(m_oPTRList[iKey]);
                m_oPTRList[iKey].clear();
            }
        }
        m_oPTRList.clear();
    }
    if(m_oPIRList.count()){
        qDeleteAll(m_oPIRList);
        m_oPIRList.clear();
    }
    if(m_oPRRList.count()){
        qDeleteAll(m_oPRRList);
        m_oPRRList.clear();
    }
    if(m_oHBRList.count()){
        qDeleteAll(m_oHBRList);
        m_oHBRList.clear();
    }
    if(m_oHBRList.count()){
        qDeleteAll(m_oHBRList);
        m_oHBRList.clear();
    }
    m_oBinningMap.clear();

}

bool Parser93kTab::WriteStdfFile(QTextStream *,const QString &strFileNameSTDF){
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


    for(int iIdx=0; iIdx<m_oPIRList.count(); iIdx++){
        //PIR

        if(!m_oPIRList[iIdx]->Write(oStdfFile)){
            setError(errWriteSTDF, "Issue when writing PIR Record");
            return false;
        }

        //PTR & MPR
        for(int iRecord=0; iRecord<m_oPTRList[iIdx+1].count(); iRecord++){
            if(!(m_oPTRList[iIdx+1])[iRecord]->Write(oStdfFile)){
                setError(errWriteSTDF, "Issue when writing PTR Record");
                return false;
            }
        }
        for(int iRecord=0; iRecord<m_oMPRList[iIdx+1].count(); iRecord++){
//            m_oMPRList.insert(m_oPIRList.count(), oMPRRecords);
//            m_oMPRPINList.insert(m_oPIRList.count(), oMPRPINList);
            if(m_oMPRPINList.contains(iIdx+1) && m_oMPRPINList[iIdx+1].contains(iRecord))
                writePinList((m_oMPRPINList[iIdx+1])[iRecord],
                             (m_oMPRList[iIdx+1])[iRecord]->m_u1HEAD_NUM,
                             (m_oMPRList[iIdx+1])[iRecord]->m_u1SITE_NUM ,
                             oStdfFile);

            if(!(m_oMPRList[iIdx+1])[iRecord]->Write(oStdfFile)){
                setError(errWriteSTDF, "Issue when writing MPR Record");
                return false;
            }
        }

        //PRR
        if(!m_oPRRList[iIdx]->Write(oStdfFile)){
            setError(errWriteSTDF, "Issue when writing PRR Record");
            return false;
        }
    }

    for(int iIdx=0; iIdx<m_oHBRList.count(); iIdx++){
        //HBR
        if(!m_oHBRList[iIdx]->Write(oStdfFile)){
            setError(errWriteSTDF, "Issue when writing HBR Record");
            return false;
        }
    }

    for(int iIdx=0; iIdx<m_oSBRList.count(); iIdx++){
        //SBR
        if(!m_oSBRList[iIdx]->Write(oStdfFile)){
            setError(errWriteSTDF, "Issue when writing PIR Record");
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

void Parser93kTab::writePinList(const QStringList &oPintList, stdf_type_u1 u1Head, stdf_type_u1 u1Site, GS::StdLib::Stdf &oStdfFile){

    for(int iIdx=0; iIdx<oPintList.count(); iIdx++){
        QString strPinName = oPintList[iIdx];
        GQTL_STDF::Stdf_PMR_V4 oPMR;
        oPMR.Reset();
        if(strPinName.isEmpty())
            continue;
        oPMR.SetPMR_INDX(iIdx);
        oPMR.SetCHAN_TYP(0);
        oPMR.SetCHAN_NAM(strPinName);
        oPMR.SetPHY_NAM("");
        oPMR.SetLOG_NAM("");
        oPMR.SetHEAD_NUM(u1Head);
        oPMR.SetSITE_NUM(u1Site);
        oPMR.Write(oStdfFile);
    }

}

bool Parser93kTab::processPTR(const QStringList &oTestProp,
                              QList<GQTL_STDF::Stdf_PTR_V4*> &oPTRRecords){
/*
--PTR
 => @ sign means no pin? PTR
 =>Pin name (when @ there is no pin, so it means it is a PTR
 0        1                    2                                 3                4                5         6          7           8          9        10
PartID|SiteID|Fail/Pass Status flag (when . it means Pass)|Sequencer name|?                    |Test name|Pin name |measurement|Low Limit| High Limit| Unit
P1    |1     |.                                           |Supply_Test   |Everest_...Short_Test|Idd_AVDRF|@        |  0.167774 |      -1 |       5   |mA
*/
    QString strTestName = oTestProp[3].trimmed() + " " + oTestProp[5].trimmed();
    if(!m_oTestNumbers.contains(strTestName)){
        m_oTestNumbers[strTestName] = ++m_iTestNumberCount;
    }
    GQTL_STDF::Stdf_PTR_V4 *poPTR = new GQTL_STDF::Stdf_PTR_V4;
    poPTR->SetTEST_NUM( m_oTestNumbers[strTestName]);
    poPTR->SetHEAD_NUM(1);
    poPTR->SetSITE_NUM(oTestProp[1].trimmed().toInt());
    poPTR->SetTEST_FLG(oTestProp[2].trimmed() == "." ? 0 : STDF_MASK_BIT7);
    poPTR->SetPARM_FLG(0);
    poPTR->SetRESULT(oTestProp[7].trimmed().toDouble());
    poPTR->SetTEST_TXT(strTestName);
    poPTR->SetALARM_ID("");
    poPTR->SetOPT_FLAG(0);
    poPTR->SetRES_SCAL(0);
    poPTR->SetLLM_SCAL(0);
    poPTR->SetHLM_SCAL(0);
    poPTR->SetLO_LIMIT(oTestProp[8].trimmed().toDouble());
    poPTR->SetHI_LIMIT(oTestProp[9].trimmed().toDouble());
    poPTR->SetUNITS(oTestProp[10].trimmed());
    poPTR->SetC_RESFMT("");
    poPTR->SetC_LLMFMT("");
    poPTR->SetC_HLMFMT("");
    poPTR->SetLO_SPEC(0);
    poPTR->SetHI_SPEC(0);
    oPTRRecords.append(poPTR);
    return true;
}





bool Parser93kTab::processMPR(const QStringList &oFirstTestProp,
                              QTextStream &oTextStream,
                              QList<GQTL_STDF::Stdf_MPR_V4*> &oMPRRecords,QMap<int, QStringList> &oMPRPINList,
                              bool &bReadNextLine,
                              QString &strNextLine){
/*
--MPR
0        1        2         3               4                          5         6             7              8            9        10
PartID|SiteID|Fail/Pass| Sequencer name | ?                      | Test name   | Pin name   | measurement | Low Limit| High Limit | Unit
P1    |1     |.        |Continuity_HP   |dc_tml.DcTest.Continuity|Continuity_HP|ADC_HTEST1N | -0.484468   | -0.8     |  -0.2      |V
*/

    //Loop until all MPR result read
    QString strLine;
    QStringList oTestProp = oFirstTestProp;
    QString strTestName = oTestProp[3].trimmed() + " " + oTestProp[5].trimmed();
    if(!m_oTestNumbers.contains(strTestName)){
        m_oTestNumbers[strTestName] = ++m_iTestNumberCount;
    }
    GQTL_STDF::Stdf_MPR_V4 *poMPR = new GQTL_STDF::Stdf_MPR_V4;

    poMPR->SetTEST_NUM( m_oTestNumbers[strTestName]);
    poMPR->SetHEAD_NUM(1);
    poMPR->SetSITE_NUM(oTestProp[1].trimmed().toInt());
    poMPR->SetTEST_FLG(oTestProp[2].trimmed() == "." ? 0 : STDF_MASK_BIT7);
    poMPR->SetPARM_FLG(0);

    int iTestResultIdx = 0;
    double dResult =0;//7
    double dHigh=0;//9
    double dLow=0;//8
    QString strUnit;//10
    QString strPinName;//6
    QList<double> oResultsList;
    QString strCurrentTestName = strTestName;
    QStringList oPinLists;

    while(oTestProp.count() == FIELDS_COUNT_93KTAB
          && oTestProp[6].trimmed() != "@"
          && strCurrentTestName == strTestName
          && !oTextStream.atEnd()){

        //Process each test Result
        strPinName  = oTestProp[6].trimmed();
        if(!oPinLists.contains(strPinName))
            oPinLists.append(strPinName);

        dResult     = oTestProp[7].trimmed().toDouble();
        dLow        = oTestProp[8].trimmed().toDouble();
        dHigh       = oTestProp[9].trimmed().toDouble();
        strUnit     = oTestProp[10].trimmed();

        oResultsList.append(dResult);
        poMPR->SetLO_LIMIT(dLow);
        poMPR->SetHI_LIMIT(dHigh);
        poMPR->SetUNITS
                (strUnit);
        strLine = oTextStream.readLine();
        bReadNextLine = false;
        strNextLine = strLine;
        oTestProp = strLine.split("|");
        strCurrentTestName = oTestProp[3].trimmed() + " " + oTestProp[5].trimmed();;
        iTestResultIdx++;
    }


    poMPR->SetRSLT_CNT(oResultsList.count());
    for(int iIdx=0; iIdx <oResultsList.count(); iIdx++){
        poMPR->SetRTN_RSLT(iIdx,oResultsList[iIdx]);
    }
    poMPR->SetTEST_TXT(strTestName);
    poMPR->SetALARM_ID("");

    poMPR->SetOPT_FLAG(STDF_MASK_BIT1|STDF_MASK_BIT2|STDF_MASK_BIT3);
    poMPR->SetRES_SCAL(0);
    poMPR->SetLLM_SCAL(0);
    poMPR->SetHLM_SCAL(0);

    poMPR->SetRTN_ICNT(0);
    poMPR->SetRTN_STAT(0,0);
    poMPR->SetSTART_IN(0);
    poMPR->SetINCR_IN (0);
    poMPR->SetRTN_INDX(0,0);
    poMPR->SetUNITS_IN("");

    poMPR->SetC_RESFMT("");
    poMPR->SetC_LLMFMT("");
    poMPR->SetC_HLMFMT("");
    poMPR->SetLO_SPEC(0);
    poMPR->SetHI_SPEC(0);
    oMPRRecords.append(poMPR);
    oMPRPINList.insert(oMPRRecords.count()-1, oPinLists);

    return true;

}

void Parser93kTab::setProgress(double dVal, const QString &strMessage){

    if(GexProgressBar)
    {
        GexProgressBar->setValue((int)dVal);
        if(GexScriptStatusLabel && !strMessage.isEmpty())
            GS::Gex::Engine::GetInstance().UpdateLabelStatus(strMessage);
        QCoreApplication::processEvents();
    }
}

int Parser93kTab::getProgress(){

    if(GexProgressBar)
        return GexProgressBar->value();
    return 0;
}
