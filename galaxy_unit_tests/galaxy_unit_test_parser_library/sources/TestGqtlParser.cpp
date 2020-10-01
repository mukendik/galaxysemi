#include <QTest>
#include "TestGqtlParser.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h> // for M_PI
#include <iomanip>
#include <gstdl_mailer.h> // to send an email
#include <gstdl_systeminfo.h>

#include <QFileInfo>
#include <QTextStream>
#include <QString>
#include <QStringList>
#include <QDate>
#include <QFile>
#include <QTemporaryDir>

#include <sstream>
#include <algorithm>
#include "stdf.h"
#include "stdfparse.h"
#include "parserFactory.h"
#include "parser_test_expected_result.h"

static QFile lAtdfFile;
static QTextStream	m_hAtdfStream;
static GQTL_STDF::StdfParse	m_clStdfParse;		// STDF V4 parser

static GQTL_STDF::Stdf_FAR_V4	m_clStdfFAR;
static GQTL_STDF::Stdf_ATR_V4	m_clStdfATR;
static GQTL_STDF::Stdf_MIR_V4	m_clStdfMIR;
static GQTL_STDF::Stdf_MRR_V4	m_clStdfMRR;
static GQTL_STDF::Stdf_PCR_V4	m_clStdfPCR;
static GQTL_STDF::Stdf_HBR_V4	m_clStdfHBR;
static GQTL_STDF::Stdf_SBR_V4	m_clStdfSBR;
static GQTL_STDF::Stdf_PMR_V4	m_clStdfPMR;
static GQTL_STDF::Stdf_PGR_V4	m_clStdfPGR;
static GQTL_STDF::Stdf_PLR_V4	m_clStdfPLR;
static GQTL_STDF::Stdf_RDR_V4	m_clStdfRDR;
static GQTL_STDF::Stdf_SDR_V4	m_clStdfSDR;
static GQTL_STDF::Stdf_WIR_V4	m_clStdfWIR;
static GQTL_STDF::Stdf_WRR_V4	m_clStdfWRR;
static GQTL_STDF::Stdf_WCR_V4	m_clStdfWCR;
static GQTL_STDF::Stdf_PIR_V4	m_clStdfPIR;
static GQTL_STDF::Stdf_PRR_V4	m_clStdfPRR;
static GQTL_STDF::Stdf_TSR_V4	m_clStdfTSR;
static GQTL_STDF::Stdf_PTR_V4	m_clStdfPTR;
static GQTL_STDF::Stdf_MPR_V4	m_clStdfMPR;
static GQTL_STDF::Stdf_FTR_V4	m_clStdfFTR;
static GQTL_STDF::Stdf_BPS_V4	m_clStdfBPS;
static GQTL_STDF::Stdf_EPS_V4	m_clStdfEPS;
static GQTL_STDF::Stdf_GDR_V4	m_clStdfGDR;
static GQTL_STDF::Stdf_DTR_V4	m_clStdfDTR;
static GQTL_STDF::Stdf_RESERVED_IMAGE_V4	m_clStdfRESERVED_IMAGE;
static GQTL_STDF::Stdf_RESERVED_IG900_V4	m_clStdfRESERVED_IG900;
static GQTL_STDF::Stdf_UNKNOWN_V4	m_clStdfUNKNOWN;
static GQTL_STDF::Stdf_VUR_V4	m_clStdfVUR;
static GQTL_STDF::Stdf_PSR_V4 	m_clStdfPSR;
static GQTL_STDF::Stdf_NMR_V4	m_clStdfNMR;
static GQTL_STDF::Stdf_CNR_V4	m_clStdfCNR;
static GQTL_STDF::Stdf_SSR_V4	m_clStdfSSR;
static GQTL_STDF::Stdf_CDR_V4	m_clStdfCDR;
static GQTL_STDF::Stdf_STR_V4	m_clStdfSTR;

Q_DECLARE_METATYPE( GS::Parser::ConverterStatus )

QString statutsToString(GS::Parser::ConverterStatus aStatus)
{
    switch(aStatus)
    {
        case GS::Parser::ConvertSuccess: return "Success";
        case GS::Parser::ConvertWarning: return "Warning";
        case GS::Parser::ConvertDelay: return "Delay";
        case GS::Parser::ConvertError: return "Error";
        default: return "Unknown";
    }
}

bool TestGqtlParser::ProcessRecordATDF(GQTL_STDF::Stdf_Record* pclRecord)
{
    QString	lAtdfString;
    if(m_clStdfParse.ReadRecord(pclRecord) == false)
    {
        // Error reading STDF file. Dump data read and add an error message.
        pclRecord->GetAtdfString(lAtdfString);
        m_hAtdfStream  << lAtdfString;
        m_hAtdfStream  <<  "** ERROR: unexpected end of record!!\n";
        return false;
    }
    else
    {
        pclRecord->GetAtdfString(lAtdfString);
        m_hAtdfStream  << lAtdfString;
        return true;
    }
}

void TestGqtlParser::ProcessTestResult( const QString &aErrorMessage,
                                        GS::Parser::ConverterStatus aStatus,
                                        const ParserTestExpectedResult &aExpectedResult) const
{
   const QString lFormatString( "Unexpected test result. "
                                "The test must `%1` and the test result message must match `%2` "
                                "as `%3` --- "
                                "Instead, the test status is `%4` with the message `%5`");

   const bool lIsTestPass = aStatus == GS::Parser::ConvertSuccess;

   if( ! aExpectedResult.IsConformTo( lIsTestPass ? ParserTestExpectedResult::Pass : ParserTestExpectedResult::Fail,
                                      aErrorMessage ) )
       QFAIL( lFormatString
                  .arg( aExpectedResult.GetStatusAsString() )
                  .arg( aExpectedResult.mMessage )
                  .arg( aExpectedResult.GetMessageMatchTypeAsString() )
                  .arg( lIsTestPass ? "pass" : "fail" )
                  .arg( aErrorMessage ).toStdString().c_str() );
}

void TestGqtlParser::convert_data()
{
  QTest::addColumn<QString>("fileToParse");
  QTest::addColumn< ParserTestExpectedResult >( "testResult" );
  QString lFailMsg  = "";

  //fetTest LVM FT Use Cases
  QFileInfo lFile = QFileInfo("./input");
  QString lInputAbsolutPath = lFile.absolutePath();
  //UC1
  QTest::newRow("typeFetTest:LVM_FETTEST_UC1:SC4925DDY-T1-GE3_K44E006.1_FT_20190208165849.UC1.dat")
      << "FetTest_LVM_FT/16441/case1/in/SC4925DDY-T1-GE3_K44E006.1_FT_20190208165849.UC1.dat"
      << PARSER_TEST_SUCCESS;
  //UC2
  lFailMsg = QString("typeFetTest - Failed loading converter external file: file %1/input/FetTest_LVM_FT/16441/case2/in/converter_external_file.xml is not xml compliant (line 8 col 17) : tag mismatch !").arg(lInputAbsolutPath);
  QTest::newRow("typeFetTest:LVM_FETTEST_UC2:SC4925DDY-T1-GE3_K44E006.1_FT_20190208165849.UC2.dat")
      << "FetTest_LVM_FT/16441/case2/in/SC4925DDY-T1-GE3_K44E006.1_FT_20190208165849.UC2.dat"
      << ParserTestExpectedResult( ParserTestExpectedResult::Fail,
                                   ParserTestExpectedResult::PlainText,
                                   lFailMsg);
  //UC3
  lFailMsg = QString("typeFetTest - Failed reading BIN mapping file: Cannot open the bin mapping file: './input/FetTest_LVM_FT/16441/case3/externalfiles/CSVFinalTestsFile20180402.csv' specified in: '%1/input/FetTest_LVM_FT/16441/case3/in/converter_external_file.xml'").arg(lInputAbsolutPath);
  QTest::newRow("typeFetTest:LVM_FETTEST_UC3:SC4925DDY-T1-GE3_K44E006.1_FT_20190208165849.UC3.dat")
      << "FetTest_LVM_FT/16441/case3/in/SC4925DDY-T1-GE3_K44E006.1_FT_20190208165849.UC3.dat"
      << ParserTestExpectedResult( ParserTestExpectedResult::Fail,
                                   ParserTestExpectedResult::PlainText,
                                   lFailMsg);
  //UC4
  lFailMsg = QString("typeFetTest - Failed reading BIN mapping file: Incorrect BinMap format [LVM FT] for the bin mapping file ./input/FetTest_LVM_FT/16441/case4/externalfiles/CSVFinalTestsFile20180402.csv specified in converter external file %1/input/FetTest_LVM_FT/16441/case4/in/converter_external_file.xml.\nSupported format: Enabled,Family,STD_Tests#,Tests#,PowerTech,STATEC/SPEKTA,Mapped Test,Bin#,Bin Name,Condition\nEx: 1,Igss,1/101,1,1 Igss,1_Igss,1,10,IGSS @ + 100%VGS,IGSS @ + 100%VGS\nLine was :\n1,Igss,1/101,1,1 Igss,1_Igss").arg(lInputAbsolutPath);
  QTest::newRow("typeFetTest:LVM_FETTEST_UC4:SC4925DDY-T1-GE3_K44E006.1_FT_20190208165849.UC4.dat")
      << "FetTest_LVM_FT/16441/case4/in/SC4925DDY-T1-GE3_K44E006.1_FT_20190208165849.UC4.dat"
      << ParserTestExpectedResult( ParserTestExpectedResult::Fail,
                                   ParserTestExpectedResult::PlainText,
                                   lFailMsg);
  //UC5
  lFailMsg = QString("typeFetTest - Failed reading PROMIS data file: Cannot open the promis file %1/input/FetTest_LVM_FT/16441/case5/externalfiles/PROMIS_DATA_FT.TXT in read mode specified in %1/input/FetTest_LVM_FT/16441/case5/in/converter_external_file.xml").arg(lInputAbsolutPath);
  QTest::newRow("typeFetTest:LVM_FETTEST_UC5:SC4925DDY-T1-GE3_K44E006.1_FT_20190208165849.UC5.dat")
      << "FetTest_LVM_FT/16441/case5/in/SC4925DDY-T1-GE3_K44E006.1_FT_20190208165849.UC5.dat"
      << ParserTestExpectedResult( ParserTestExpectedResult::Fail,
                                   ParserTestExpectedResult::PlainText,
                                   lFailMsg);
  //UC6
  lFailMsg = QString("typeFetTest - Failed reading PROMIS data file: ./input/FetTest_LVM_FT/16441/case6/externalfiles/PROMIS_DATA_FT.TXT specified in converter external file %1/input/FetTest_LVM_FT/16441/case6/in/converter_external_file.xml.\nMissing columns. At least 8 columns are expected.").arg(lInputAbsolutPath);
  QTest::newRow("typeFetTest:LVM_FETTEST_UC6:SC4925DDY-T1-GE3_K44E006.1_FT_20190208165849.UC6.dat")
      << "FetTest_LVM_FT/16441/case6/in/SC4925DDY-T1-GE3_K44E006.1_FT_20190208165849.UC6.dat"
      << ParserTestExpectedResult( ParserTestExpectedResult::Fail,
                                   ParserTestExpectedResult::PlainText,
                                   lFailMsg);
  //UC7
  lFailMsg = QString("typeFetTest - Failed reading BIN mapping file: Invalid default Bin Number -1 in the external file: %1/input/FetTest_LVM_FT/16441/case7/in/converter_external_file.xml").arg(lInputAbsolutPath);
  QTest::newRow("typeFetTest:LVM_FETTEST_UC7:SC4925DDY-T1-GE3_K44E006.1_FT_20190208165849.UC7.dat")
      << "FetTest_LVM_FT/16441/case7/in/SC4925DDY-T1-GE3_K44E006.1_FT_20190208165849.UC7.dat"
      << ParserTestExpectedResult( ParserTestExpectedResult::Fail,
                                   ParserTestExpectedResult::PlainText,
                                   lFailMsg);
  //UC8
  lFailMsg = QString("typeFetTest - Failed reading BIN mapping file: Invalid default Bin Number 32768 in the external file: %1/input/FetTest_LVM_FT/16441/case8/in/converter_external_file.xml").arg(lInputAbsolutPath);
  QTest::newRow("typeFetTest:LVM_FETTEST_UC8:SC4925DDY-T1-GE3_K44E006.1_FT_20190208165849.UC8.dat")
      << "FetTest_LVM_FT/16441/case8/in/SC4925DDY-T1-GE3_K44E006.1_FT_20190208165849.UC8.dat"
      << ParserTestExpectedResult( ParserTestExpectedResult::Fail,
                                   ParserTestExpectedResult::PlainText,
                                   lFailMsg);
  //UC9
  lFailMsg = QString("typeFetTest - Failed reading BIN mapping file: The [LVM FT] bin mapping file: './input/FetTest_LVM_FT/16441/case9/externalfiles/CSVFinalTestsFile20180402.csv' specified in the file: '%1/input/FetTest_LVM_FT/16441/case9/in/converter_external_file.xml' contains a duplicated test number: '10'").arg(lInputAbsolutPath);
  QTest::newRow("typeFetTest:LVM_FETTEST_UC9:SC4925DDY-T1-GE3_K44E006.1_FT_20190208165849.UC9.dat")
      << "FetTest_LVM_FT/16441/case9/in/SC4925DDY-T1-GE3_K44E006.1_FT_20190208165849.UC9.dat"
      << ParserTestExpectedResult( ParserTestExpectedResult::Fail,
                                   ParserTestExpectedResult::PlainText,
                                   lFailMsg);
  //UC10
  lFailMsg = QString("typeFetTest - Failed reading PROMIS data file: Cannot find the promis item with key : K44E006.1 in promis file :  %1/input/FetTest_LVM_FT/16441/case10/externalfiles/PROMIS_DATA_FT.TXT specified in file : %1/input/FetTest_LVM_FT/16441/case10/in/converter_external_file.xml").arg(lInputAbsolutPath);
  QTest::newRow("typeFetTest:LVM_FETTEST_UC10:SC4925DDY-T1-GE3_K44E006.1_FT_20190208165849.UC10.dat")
      << "FetTest_LVM_FT/16441/case10/in/SC4925DDY-T1-GE3_K44E006.1_FT_20190208165849.UC10.dat"
      << ParserTestExpectedResult( ParserTestExpectedResult::Fail,
                                   ParserTestExpectedResult::PlainText,
                                   lFailMsg);
 //UC11
  QTest::newRow("typeFetTest:LVM_FETTEST_UC11:SC4925DDY-T1-GE3_K44E006.1_FT_20190208165849.UC11.dat")
      << "FetTest_LVM_FT/16441/case11/in/SC4925DDY-T1-GE3_K44E006.1_FT_20190208165849.UC11.dat"
      << PARSER_TEST_SUCCESS;
  //UC12
  QTest::newRow("typeFetTest:LVM_FETTEST_UC12:SC4925DDY-T1-GE3_K44E006.1_FT_20190208165849.UC12.dat")
      << "FetTest_LVM_FT/16441/case12/in/SC4925DDY-T1-GE3_K44E006.1_FT_20190208165849.UC12.dat"
      << PARSER_TEST_SUCCESS;
  //UC13
  QTest::newRow("typeFetTest:LVM_FETTEST_UC13:SC4925DDY-T1-GE3_K44E006.1_FT_20190208165849.UC13.dat")
      << "FetTest_LVM_FT/16441/case13/in/SC4925DDY-T1-GE3_K44E006.1_FT_20190208165849.UC13.dat"
      << PARSER_TEST_SUCCESS;
  //UC14
  lFailMsg = QString("typeFetTest - Failed loading converter external file: Failed mapping binning for part 1. The part is FAIL with all enabled tests being PASS, and there is no default binning defined in file %1/input/FetTest_LVM_FT/16441/case14/in/converter_external_file.xml.\n").arg(lInputAbsolutPath);
  QTest::newRow("typeFetTest:LVM_FETTEST_UC14:SC4925DDY-T1-GE3_K44E006.1_FT_20190208165849.UC14.dat")
      << "FetTest_LVM_FT/16441/case14/in/SC4925DDY-T1-GE3_K44E006.1_FT_20190208165849.UC14.dat"
      << ParserTestExpectedResult( ParserTestExpectedResult::Fail,
                                   ParserTestExpectedResult::PlainText,
                                   lFailMsg);
  //UC15
  QTest::newRow("typeFetTest:LVM_FETTEST_UC15:SC4925DDY-T1-GE3_K44E006.1_FT_20190208165849.UC15.dat")
      << "FetTest_LVM_FT/16441/case15/in/SC4925DDY-T1-GE3_K44E006.1_FT_20190208165849.UC15.dat"
      << PARSER_TEST_SUCCESS;
  //UC16
  QTest::newRow("typeFetTest:LVM_FETTEST_UC16:SC4925DDY-T1-GE3_K44E006.1_FT_20190208165849.UC16.dat")
      << "FetTest_LVM_FT/16441/case16/in/SC4925DDY-T1-GE3_K44E006.1_FT_20190208165849.UC16.dat"
      << PARSER_TEST_SUCCESS;
  //UC17
  lFailMsg = QString("typeFetTest - Failed loading converter external file: Failed mapping binning for part 1. The part is FAIL with all enabled tests being PASS, and there is no default binning defined in file %1/input/FetTest_LVM_FT/16441/case17/in/converter_external_file.xml.\n").arg(lInputAbsolutPath);
  QTest::newRow("typeFetTest:LVM_FETTEST_UC17:SC4925DDY-T1-GE3_K44E006.1_FT_20190208165849.UC17.dat")
      << "FetTest_LVM_FT/16441/case17/in/SC4925DDY-T1-GE3_K44E006.1_FT_20190208165849.UC17.dat"
      << ParserTestExpectedResult( ParserTestExpectedResult::Fail,
                                   ParserTestExpectedResult::PlainText,
                                   lFailMsg);
  //////////////END OF fetTest lvmFt Use Cases

  // Atdf converter
  QTest::newRow("typdeAtdfToStdf:C33D713.1_FT_0.atd") << "Atdf/C33D713.1_FT_0.atd" << PARSER_TEST_SUCCESS;
  QTest::newRow("typdeAtdfToStdf:EJ15D03.8_FT_0.atd") << "Atdf/EJ15D03.8_FT_0.atd" << PARSER_TEST_SUCCESS;
  QTest::newRow("typdeAtdfToStdf:G21D088.17_FT_0.atd") << "Atdf/G21D088.17_FT_0.atd" << PARSER_TEST_SUCCESS;
  QTest::newRow("typdeAtdfToStdf:G35K282.8_FT_0.atd") << "Atdf/G35K282.8_FT_0.atd" << PARSER_TEST_SUCCESS;

  QTest::newRow("typdeAtdfToStdf:C33D713.1_FT_0b.atd")
      << "Atdf/external_file/C33D713.1_FT_0b.atd"
      << ParserTestExpectedResult( ParserTestExpectedResult::Fail,
                                   ParserTestExpectedResult::RegularExpression,
                                   "AtdfToStdf - Invalid file format: ATDF bin mapping check rejected for file .+/C33D713.1_FT_0b\\.atd - Name for Bin \\[398\\]\\[UIS\\] does not comply with the one defined in the mapping file \\[398\\]\\[UIS_Leak\\] .+/input/Atdf/external_file/CSVFinalTestsFile20180402\\.csv specified in .+/input/Atdf/external_file/converter_external_file\\.xml");

  QTest::newRow("typdeAtdfToStdf:G35K282.8_FT_0b.atd") << "Atdf/external_file/G35K282.8_FT_0b.atd" << PARSER_TEST_SUCCESS;//testing test# 0 simply ignored not fail

  QTest::newRow("typdeAtdfToStdf:ED47D75.2_FT_0.atd")
      << "Atdf/external_file/ED47D75.2_FT_0.atd"
      << ParserTestExpectedResult( ParserTestExpectedResult::Fail,
                                   ParserTestExpectedResult::RegularExpression,
                                   "AtdfToStdf - Invalid file format: ATDF bin mapping check rejected for file .+/ED47D75\\.2_FT_0\\.atd - Name for Bin \\[6\\]\\[IGSS @ \\+ 120% VGS\\] does not comply with the one defined in the mapping file \\[6\\]\\[IGSS @ \\+ High Voltage\\] .+/input/Atdf/external_file/CSVFinalTestsFile20180402\\.csv specified in .+/input/Atdf/external_file/converter_external_file\\.xml");
  //ATDF UC1
  QTest::newRow("typdeAtdfToStdf:C33D713.1_FT_0_1.atd") << "Atdf/case1/C33D713.1_FT_0_1.atd"
                                                        << PARSER_TEST_SUCCESS;
  //ATDF UC2
  QTest::newRow("typdeAtdfToStdf:C33D713.1_FT_0_2.atd")
      << "Atdf/case2/C33D713.1_FT_0_2.atd"
      << ParserTestExpectedResult( ParserTestExpectedResult::Fail,
                                   ParserTestExpectedResult::RegularExpression,
                                   "AtdfToStdf - Failed loading converter external file: file .+/converter_external_file\\.xml is not xml compliant \\(line 4 col 17\\) : tag mismatch !" );
  //ATDF UC3
  QTest::newRow("typdeAtdfToStdf:C33D713.1_FT_0_3.atd")
      << "Atdf/case3/C33D713.1_FT_0_3.atd"
      << ParserTestExpectedResult( ParserTestExpectedResult::Fail,
                                   ParserTestExpectedResult::RegularExpression,
                                   "AtdfToStdf - Failed loading converter external file: Cannot open the bin mapping file: '.+/input/Atdf/case3/externalfiles/CSVFinalTestsFile20180402\\.csv' specified in: '.+/input/Atdf/case3/converter_external_file.xml'");
  //ATDF UC4
  lFile = QFileInfo("./input/Atdf/case4/C33D713.1_FT_0_4.atd");
  lInputAbsolutPath = lFile.absolutePath();
  lFailMsg = QString("AtdfToStdf - Failed loading converter external file: Incorrect BinMap format [LVM FT] for the bin mapping file "
                  "./input/Atdf/case4/externalfiles/CSVFinalTestsFile20180402.csv specified in converter external file %1/converter_external_file.xml.\n"
                  "Supported format: Enabled,Family,STD_Tests#,Tests#,PowerTech,STATEC/SPEKTA,Mapped Test,Bin#,Bin Name,Condition\n"
                  "Ex: 1,Igss,1/101,1,1 Igss,1_Igss,1,10,IGSS @ + 100%VGS,IGSS @ + 100%VGS\n"
                  "Line was :\n"
              "1,Igss,1/101,1,1 Igss,1_Igss").arg(lInputAbsolutPath);
  QTest::newRow("typdeAtdfToStdf:C33D713.1_FT_0_4.atd")
      << "Atdf/case4/C33D713.1_FT_0_4.atd"
      << ParserTestExpectedResult( ParserTestExpectedResult::Fail,
                                   ParserTestExpectedResult::PlainText,
                                   lFailMsg);
  //ATDF UC5
  QTest::newRow("typdeAtdfToStdf:C33D713.1_FT_0_5.atd")
      << "Atdf/case5/C33D713.1_FT_0_5.atd"
      << ParserTestExpectedResult( ParserTestExpectedResult::Fail,
                                   ParserTestExpectedResult::RegularExpression,
                                   "AtdfToStdf - Failed loading converter external file: Cannot open the bin mapping file: '.+/input/Atdf/case5/externalfiles/CSVSortEntriesFile20180402\\.csv' specified in: '.+/input/Atdf/case5/converter_external_file\\.xml'");
  //ATDF UC6
  QTest::newRow("typdeAtdfToStdf:C33D713.1_FT_0_6.atd")
      << "Atdf/case6/C33D713.1_FT_0_6.atd"
      << ParserTestExpectedResult( ParserTestExpectedResult::Fail,
                                   ParserTestExpectedResult::RegularExpression,
                                   "AtdfToStdf - Failed loading converter external file: Incorrect BinMap format \\[LVM FT\\] for the bin mapping file '.+/input/Atdf/case6/externalfiles/CSVSortEntriesFile20180402\\.csv' specified in the file '.+/input/Atdf/case6/converter_external_file\\.xml'\nSupported format: Enabled,Family,Tests name,Test#,ASE,AMKOR,BIN #\nEx: 1,G-D-S \\(Short\\),GDS sh,1000,1000_GDS sh,1000 GDS sh,1000\nLine was: 1,G-D-S \\(Short\\),GDS sh");
  //ATDF UC7
  QTest::newRow("typdeAtdfToStdf:C33D713.1_FT_0_7.atd") << "Atdf/case7/C33D713.1_FT_0_7.atd"
                                                        << PARSER_TEST_SUCCESS;
  //ATDF UC8
  QTest::newRow("typdeAtdfToStdf:C33D713.1_FT_0_8.atd")
      << "Atdf/case8/C33D713.1_FT_0_8.atd"
      << ParserTestExpectedResult( ParserTestExpectedResult::Fail,
                                   ParserTestExpectedResult::RegularExpression,
                                   "AtdfToStdf - Invalid file format: ATDF bin mapping check rejected for file .+/C33D713\\.1_FT_0_8\\.atd - Name for Bin \\[10\\]\\[WRONG_BIN_NAME\\] does not comply with the one defined in the mapping file \\[10\\]\\[IGSS @ \\+ 100%VGS\\] .+/input/Atdf/case8/externalfiles/CSVFinalTestsFile20180402\\.csv specified in .+/input/Atdf/case8/converter_external_file\\.xml");
  //ATDF UC9
  QTest::newRow("typdeAtdfToStdf:C33D713.1_FT_0_9.atd")
      << "Atdf/case9/C33D713.1_FT_0_9.atd"
      << ParserTestExpectedResult( ParserTestExpectedResult::Fail,
                                   ParserTestExpectedResult::RegularExpression,
                                   "AtdfToStdf - Invalid file format: ATDF bin mapping check rejected for file .+/C33D713\\.1_FT_0_9\\.atd - Bin \\[10999\\]\\[IGSS @ \\+ 100%VGS\\] was not found in the mapping files specified in .+/input/Atdf/case9/converter_external_file\\.xml");
  //ATDF UC10
  QTest::newRow("typdeAtdfToStdf:C33D713.1_FT_0_10.atd")
      << "Atdf/case10/C33D713.1_FT_0_10.atd"
      << ParserTestExpectedResult( ParserTestExpectedResult::Fail,
                                   ParserTestExpectedResult::RegularExpression,
                                   "AtdfToStdf - Invalid file format: ATDF bin mapping check rejected for file .+/C33D713\\.1_FT_0_10\\.atd - Name for Bin \\[1010\\]\\[WRONG_BIN_NAME\\] does not comply with the one defined in the mapping file \\[1010\\]\\[UIS\\] .+/input/Atdf/case10/externalfiles/CSVSortEntriesFile20180402\\.csv specified in .+/input/Atdf/case10/converter_external_file\\.xml");
  //ATDF UC11
  QTest::newRow("typdeAtdfToStdf:C33D713.1_FT_0_11.atd")
      << "Atdf/case11/C33D713.1_FT_0_11.atd"
      << ParserTestExpectedResult( ParserTestExpectedResult::Fail,
                                   ParserTestExpectedResult::RegularExpression,
                                   "AtdfToStdf - Invalid file format: ATDF bin mapping check rejected for file .+/C33D713\\.1_FT_0_11\\.atd - Bin \\[1099\\]\\[UIS\\] was not found in the mapping files specified in .+/input/Atdf/case11/converter_external_file\\.xml");
  //ATDF UC12
  QTest::newRow("typdeAtdfToStdf:C33D713.1_FT_0_12.atd")
      << "Atdf/case12/C33D713.1_FT_0_12.atd"
      << ParserTestExpectedResult( ParserTestExpectedResult::Fail,
                                   ParserTestExpectedResult::RegularExpression,
                                   "AtdfToStdf - Failed loading converter external file: The \\[LVM FT\\] bin mapping file: '.+/input/Atdf/case12/externalfiles/CSVFinalTestsFile20180402\\.csv' specified in the file: '.+/input/Atdf/case12/converter_external_file\\.xml' contains a duplicated test number: '10'");
  //ATDF UC13
  QTest::newRow("typdeAtdfToStdf:C33D713.1_FT_0_13.atd")
      << "Atdf/case13/C33D713.1_FT_0_13.atd"
      << ParserTestExpectedResult( ParserTestExpectedResult::Fail,
                                   ParserTestExpectedResult::RegularExpression,
                                   "AtdfToStdf - Failed loading converter external file: The \\[LVM FT SE\\] bin mapping file: '.+/input/Atdf/case13/externalfiles/CSVSortEntriesFile20180402\\.csv' specified in the file: '.+/input/Atdf/case13/converter_external_file\\.xml' contains a duplicated test number: '1010'");
  //////////////END OF ATDF TESTS

  QTest::newRow("typeVishayASE:VISHAY_SUD50N04-8M8P-4GE3_K10D565.16_DC1_1.csv")
      << "ASE/use_case_1/VISHAY_SUD50N04-8M8P-4GE3_K10D565.16_DC1_1.csv"
      << PARSER_TEST_SUCCESS;
  QTest::newRow("typeVishayASE:VISHAY_SUD50N04-8M8P-4GE3_K10D565.16_DC1_2.csv")
      << "ASE/use_case_2/VISHAY_SUD50N04-8M8P-4GE3_K10D565.16_DC1_2.csv"
      << PARSER_TEST_SUCCESS;
  QTest::newRow("typeVishayASE:VISHAY_SUD50N04-8M8P-4GE3_K10D565.16_DC1_3.csv")
      << "ASE/use_case_3/VISHAY_SUD50N04-8M8P-4GE3_K10D565.16_DC1_3.csv"
      << PARSER_TEST_SUCCESS;
  QTest::newRow("typeVishayASE:VISHAY_SUD50N04-8M8P-4GE3_K10D565.16_DC1_4.csv")
      << "ASE/use_case_4/VISHAY_SUD50N04-8M8P-4GE3_K10D565.16_DC1_4.csv"
      << PARSER_TEST_SUCCESS;
  QTest::newRow("typeVishayASE:VISHAY_SUD50N04-8M8P-4GE3_K10D565.16_DC1_5.csv")
      << "ASE/use_case_5/VISHAY_SUD50N04-8M8P-4GE3_K10D565.16_DC1_5.csv"
      << ParserTestExpectedResult( ParserTestExpectedResult::Fail,
                                   ParserTestExpectedResult::RegularExpression,
                                   "typeVishayASE - Failed reading BIN mapping file: Cannot find the bin map item key: '56' in the bin map file: '.+/input/ASE/use_case_5/CSVFinalTestsFile20181112\\.csv' specified in: '.+/input/ASE/use_case_5/converter_external_file\\.xml" );
  QTest::newRow("typeVishayASE:VISHAY_SUD50N04-8M8P-4GE3_K10D565.16_DC1_6.csv")
      << "ASE/use_case_6/VISHAY_SUD50N04-8M8P-4GE3_K10D565.16_DC1_6.csv"
      << ParserTestExpectedResult( ParserTestExpectedResult::Fail,
                                   ParserTestExpectedResult::RegularExpression,
                                   "typeVishayASE - Failed reading BIN mapping file: Cannot find the bin map item key: '1003' in the bin map file: '.+/input/ASE/use_case_6/CSVSortEntriesFile20181102\\.csv' specified in: '.+/input/ASE/use_case_6/converter_external_file\\.xml" );
  QTest::newRow("typeVishayASE:VISHAY_SUD50N04-8M8P-4GE3_K10D565.16_DC1_7.csv")
      << "ASE/use_case_7/VISHAY_SUD50N04-8M8P-4GE3_K10D565.16_DC1_7.csv"
      << PARSER_TEST_SUCCESS;
  QTest::newRow("typeVishayASE:VISHAY_SUD50N04-8M8P-4GE3_K10D565.16_DC1_8.csv")
      << "ASE/use_case_8/VISHAY_SUD50N04-8M8P-4GE3_K10D565.16_DC1_8.csv"
      << PARSER_TEST_SUCCESS;
  QTest::newRow("typeVishayASE:VISHAY_SUD50N04-8M8P-4GE3_K10D565.16_DC1_17179.csv")
      << "ASE/17179/VISHAY_SUD50N04-8M8P-4GE3_K10D565.16_DC1_17179.csv"
      << PARSER_TEST_SUCCESS;


  QTest::newRow("WatTsmXml:watTsmcXml.xml") << "watTsmcXml.xml" << PARSER_TEST_SUCCESS;
  QTest::newRow("typeSkyworksIFF:4178.pcme") << "4178.pcme" << PARSER_TEST_SUCCESS;
  QTest::newRow("typeSkyworksIFF:4179.pcmd") << "4179.pcmd" << PARSER_TEST_SUCCESS;
  QTest::newRow("InphiPresto:Inphi_FT...20150923.csv") << "Inphi_FT_datalog_C6671_3214SZ_FD_TEST_FLOW_R410_20150923.csv" << PARSER_TEST_SUCCESS;
  QTest::newRow("PCM Dongbu:newDongbu.csv") << "newDongbu.csv" << PARSER_TEST_SUCCESS;
  QTest::newRow("MexicaliMap:Maxicali.txt") << "Maxicali.txt" << PARSER_TEST_SUCCESS;
  QTest::newRow("SiTimeEtest:C09177_CMS150M.csv") << "C09177_CMS150M.csv" << PARSER_TEST_SUCCESS;
  QTest::newRow("WoburnSECSIIMap:272151S190.txt") << "93007B1NS-6929559-272151S190.txt" << PARSER_TEST_SUCCESS;
  QTest::newRow("WatAsmc:PCM_6E2640...PRT") << "PCM_6E2640_asmc.PRT" << PARSER_TEST_SUCCESS;
  QTest::newRow("WatAsmc:PCM_6E2642...PRT") << "PCM_6E2642_asmc.PRT" << PARSER_TEST_SUCCESS;
  QTest::newRow("Wat:F6201101.WAT") << "F6201101.WAT" << PARSER_TEST_SUCCESS;
  QTest::newRow("WatSmic:WatSmic.dat") << "WatSmic.dat" << PARSER_TEST_SUCCESS;
  QTest::newRow("InphiPresto:PrestoFT.csv") << "PrestoFT.csv" << PARSER_TEST_SUCCESS;
  QTest::newRow("typeMicronSum:Micron.sum") << "Micron.sum" << PARSER_TEST_SUCCESS;
  QTest::newRow("WatUmc:WatUmc.csv") << "WatUmc.csv" << PARSER_TEST_SUCCESS;
  QTest::newRow("typeSkyworksIFF:J91675...20160121083846.prbd") << "J91675.1_J91675-14_WP_ASX_PK1059_F_PK1059-A1-JAZ_20160121083846.prbd" << PARSER_TEST_SUCCESS;
  QTest::newRow("AquantiaPCM:FAB1.wacdata.dis") << "FAB1.wacdata.dis" << PARSER_TEST_SUCCESS;
  QTest::newRow("typeSkyFasterCSV:FastTestCsv.csv") << "FastTestCsv.csv" << PARSER_TEST_SUCCESS;
  QTest::newRow("InphiPresto:InphiCsv.csv") << "InphiCsv.csv" << PARSER_TEST_SUCCESS;
  QTest::newRow("typeTriQuintDC:TriQuintDC.txt") << "TriQuintDC.txt" << PARSER_TEST_SUCCESS;
  QTest::newRow("typeSpinstand:Spindstand.025") << "Spindstand.025" << PARSER_TEST_SUCCESS;
  QTest::newRow("typePcmHjtc:PCM heijan technology") << "heijan_technology_hjtc.CSV" << PARSER_TEST_SUCCESS;
  QTest::newRow("typePcmHjtc:PCM heijan technology CRITICAL Tests") << "heijan_technology_hjtc_critical_tests.CSV" << PARSER_TEST_SUCCESS;
  QTest::newRow("typeSipex:POLAR E-Test") << "SIPEX_POLAR.CSV" << PARSER_TEST_SUCCESS;
  QTest::newRow("typeSpektraLotSummary:A04K041.1_Summary-1.CSV") << "SpektraLotSummary/converter_external_file/A04K041.1_Summary-1.CSV" << PARSER_TEST_SUCCESS;
  QTest::newRow("typeSpektraLotSummary:A04K041.1_Summary-2.CSV") << "SpektraLotSummary/no_converter_external_file/A04K041.1_Summary-2.CSV" << PARSER_TEST_SUCCESS;
  QTest::newRow("typeSpektraDatalog:K20K180.1_W01.Dta-1.CSV") << "SpektraDatalog/converter_external_file/K20K180.1_W01.Dta-1.CSV" << PARSER_TEST_SUCCESS;
  QTest::newRow("typeSpektraDatalog:K20K180.1_W01.Dta-2.CSV") << "SpektraDatalog/no_converter_external_file/K20K180.1_W01.Dta-2.CSV" << PARSER_TEST_SUCCESS;
  QTest::newRow("typeFetTest:LVM_FETTEST_QA:SC2302CDS-T1-GE3_K13E005.8_FT_20180618205552.dat")
      << "FetTest_LVM_FT/converter_external_file/SC2302CDS-T1-GE3_K13E005.8_FT_20180618205552.dat"
      << PARSER_TEST_SUCCESS;
  QTest::newRow("typeFetTest:LVM_FETTEST_QA:SC2302CDS-T1-GE3_K13E005.18A_FT_20180621033530.dat")
      << "FetTest_LVM_FT/converter_external_file/SC2302CDS-T1-GE3_K13E005.18A_FT_20180621033530.dat"
      << PARSER_TEST_SUCCESS;
  QTest::newRow("typeFetTest:LVM_FETTEST_QA:SC2302CDS-T1-GE3_K13E005.28_FT_20180618205552_17179.dat")
      << "FetTest_LVM_FT/17179/SC2302CDS-T1-GE3_K13E005.28_FT_20180618205552.dat"
      << PARSER_TEST_SUCCESS;
  QTest::newRow("typeFetTestSummary:J05Y069.9.csv") << "FetTestSummary_LVM_FT/converter_external_file/J05Y069.9.csv" << PARSER_TEST_SUCCESS;
  QTest::newRow("typeFetTestSummary:J05Y069.9_17179.csv") << "FetTestSummary_LVM_FT/17179/J05Y069.9_17179.csv" << PARSER_TEST_SUCCESS;
  QTest::newRow("typeFetTestSummary:reject-J05Y069.9.csv")
      << "FetTestSummary_LVM_FT/reject_bad_test_number/reject-J05Y069.9.csv"
      << ParserTestExpectedResult( ParserTestExpectedResult::Fail,
                                   ParserTestExpectedResult::PlainText,
                                   "typeFetTestSummary - Invalid file format: Final Test Entry 36/146 map to different bin numbers and names. Please make sure that each entry maps to a single bin number and name" );
  QTest::newRow("typeVishayATM:EJ23V002.3_Summary_20181107.103003_ptech.txt")
      << "ATM/external_file/EJ23V002.3_Summary_20181107.103003_ptech.txt"
      << PARSER_TEST_SUCCESS;
  QTest::newRow("typeVishayATM:GCORE-16747")
      << "ATM/GCORE-16747/3B_EK15Y131.1_Summary_20180920.184418_ptech.txt"
      << PARSER_TEST_SUCCESS;
  QTest::newRow("typeVishayATM:GCORE-17179")
      << "ATM/GCORE-17179/EK15Y131.2_Summary_20180920.185122_ptech_17179.txt"
      << PARSER_TEST_SUCCESS;
  QTest::newRow("typeStdf:EJ04D27.5_Data_20180209.172644_ptech.stdf") << "FTAMKOR_FT_LVM/EJ04D27.5_Data_20180209.172644_ptech.stdf" << PARSER_TEST_SUCCESS;
  QTest::newRow("typeStdf:H45Y103_1_01_P_ETS074654_01272017_patman.std")
          << "EAGLE_WT_LVM/H45Y103_1_01_P_ETS074654_01272017_patman.std"
          << PARSER_TEST_SUCCESS;

  QTest::newRow("typeStdf:GCORE-17225")
          << "B_WT_LVM/N09D212_1_12_P_ETS060702_04222019.std"
          << PARSER_TEST_SUCCESS;

  // Vishay ASE sum datalog parser test cases
  QTest::newRow("typeVishay_ASE_SUM_case_1")
      << "ASE/sum/case1/VIY_SUD50P06-15-GE3_K15K261.1_20180719074025-case1.sum"
      << PARSER_TEST_SUCCESS;
  QTest::newRow("typeVishay_ASE_SUM_case_2") << "ASE/sum/case2/VIY_SUD50P06-15-GE3_K15K261.1_20180719074025-case2.sum" << PARSER_TEST_SUCCESS;
  QTest::newRow("typeVishay_ASE_SUM_case_3") << "ASE/sum/case3/VIY_SUD50P06-15-GE3_K15K261.1_20180719074025-case3.sum" << PARSER_TEST_SUCCESS;
  QTest::newRow("typeVishay_ASE_SUM_case_4") << "ASE/sum/case4/VIY_SUD50P06-15-GE3_K15K261.1_20180719074025-case4.sum" << PARSER_TEST_SUCCESS;
  QTest::newRow("typeVishay_ASE_SUM_case_5") << "ASE/sum/case5/VIY_SUD50P06-15-GE3_K15K261.1_20180719074025-case5.sum"
      << ParserTestExpectedResult( ParserTestExpectedResult::Fail,
                                   ParserTestExpectedResult::RegularExpression,
                                   "typeVishayASESummary - Invalid file format: Cannot find the bin map item key: '419' in the bin map file: '.+/input/ASE/sum/case5/CSVFinalTestsFile20181112\\.csv' specified in: '.+/input/ASE/sum/case5/converter_external_file\\.xml" );
  QTest::newRow("typeVishay_ASE_SUM_case_6") << "ASE/sum/case6/VIY_SUD50P06-15-GE3_K15K261.1_20180719074025-case6.sum"
      << ParserTestExpectedResult( ParserTestExpectedResult::Fail,
                                   ParserTestExpectedResult::RegularExpression,
                                   "typeVishayASESummary - Invalid file format: Cannot find the bin map item key: '1059' in the bin map file: '.+/input/ASE/sum/case6/CSVFinalTestsFile20181112\\.csv' specified in: '.+/input/ASE/sum/case6/converter_external_file\\.xml" );
  QTest::newRow("typeVishay_ASE_SUM_GCORE-17179")
      << "ASE/sum/17179/VIY_SUD50P06-15-GE3_K15K261.1_20180719074025-17179.sum"
      << PARSER_TEST_SUCCESS;
  QTest::newRow("typeVishay_ASE_SUM_hvm_flow")
      << "ASE/sum/hvm_flow/VISHAY_IRF9630PBF_K21K207.1_20180718233151.sum"
      << PARSER_TEST_SUCCESS;
  QTest::newRow("typeVishay_ASE_SUM_case_8") << "ASE/sum/case8/Integrated_summary_format0807.sum" << PARSER_TEST_SUCCESS;


  //GCORE-16654 Vishay Stdf to Stdf algo verification cases:
  QTest::newRow("typeStdf:EJ04D27.5_Data_20180209.172644_ptech1.std") << "VishayStdf/case1/EJ04D27.5_Data_20180209.172644_ptech1.std" << PARSER_TEST_SUCCESS;
  QTest::newRow("typeStdf:EJ04D27.5_Data_20180209.172644_ptech1b.std") << "VishayStdf/case1b/EJ04D27.5_Data_20180209.172644_ptech1b.std" << PARSER_TEST_FAIL;//lFailMsg;
  QTest::newRow("typeStdf:EJ04D27.5_Data_20180209.172644_ptech2.std") << "VishayStdf/case2/EJ04D27.5_Data_20180209.172644_ptech2.std" << PARSER_TEST_SUCCESS;
  QTest::newRow("typeStdf:EJ04D27.5_Data_20180209.172644_ptech3.std") << "VishayStdf/case3/EJ04D27.5_Data_20180209.172644_ptech3.std" << PARSER_TEST_SUCCESS;
  QTest::newRow("typeStdf:EJ04D27.5_Data_20180209.172644_ptech4.std") << "VishayStdf/case4/EJ04D27.5_Data_20180209.172644_ptech4.std" << PARSER_TEST_SUCCESS;
  QTest::newRow("typeStdf:EJ04D27.5_Data_20180209.172644_ptech5.std") << "VishayStdf/case5/EJ04D27.5_Data_20180209.172644_ptech5.std" << PARSER_TEST_FAIL;
  QTest::newRow("typeStdf:EJ04D27.5_Data_20180209.172644_ptech6.std") << "VishayStdf/case6/EJ04D27.5_Data_20180209.172644_ptech6.std" << PARSER_TEST_SUCCESS;
  QTest::newRow("typeStdf:EJ04D27.5_Data_20180209.172644_ptech7.std") << "VishayStdf/case7/EJ04D27.5_Data_20180209.172644_ptech7.std" << PARSER_TEST_SUCCESS;
  QTest::newRow("typeStdf:EJ04D27.5_Data_20180209.172644_ptech8.std") << "VishayStdf/case8/EJ04D27.5_Data_20180209.172644_ptech8.std" << PARSER_TEST_SUCCESS;
  QTest::newRow("typeStdf:EJ04D27.5_Data_20180209.172644_ptech9.std") << "VishayStdf/case9/EJ04D27.5_Data_20180209.172644_ptech9.std" << PARSER_TEST_SUCCESS;
  QTest::newRow("typeStdf:EJ04D27.5_Data_20180209.172644_ptech_17179.std") << "VishayStdf/17179/EJ04D27.5_Data_20180209.172644_ptech_17179.std" << PARSER_TEST_SUCCESS;


  // Big ones
  QTest::newRow("SkyNPCSV:SkyNpCsv.csv") << "SkyNpCsv.csv" << PARSER_TEST_SUCCESS;
  QTest::newRow("typeSkyworksIFF:1906313.1_FT_DRG_77356_18_N_20160221200723.ftpi") << "1906313.1_FT_DRG_77356_18_N_20160221200723.ftpi" << PARSER_TEST_SUCCESS;
  QTest::newRow("typeTriQuintRF:TriQuintRF") << "TriQuintRF" << PARSER_TEST_SUCCESS;

  QTest::newRow("typeWoburnCsv:Woburn.CSV") << "Woburn.CSV" << PARSER_TEST_SUCCESS;
  QTest::newRow("typeWoburnCsv:WoburnUnorderedNoData.csv") << "WoburnUnorderedNoData.csv"
                                                           << ParserTestExpectedResult( ParserTestExpectedResult::Fail,
                                                                                        ParserTestExpectedResult::RegularExpression,
                                                                                        "typeWoburnCsv - Missing mandatory info: There is no data in the input file : .+" );

  QTest::newRow("typeSkyworksIFF:4177.ftpi") << "4177.ftpi" << PARSER_TEST_SUCCESS;
  QTest::newRow("typeSkyworksIFF:4195.prbd") << "4195.prbd" << PARSER_TEST_SUCCESS;
  QTest::newRow("typeFetTest:k17g0j1.dat") << "FetTest_HVM_WS/k17g0j1.dat" << PARSER_TEST_SUCCESS;

  QTest::newRow("typeFetTest:x32gu2.dat") << "FetTest_LVM_WS/converter_external_file/x32gu2.dat" << PARSER_TEST_SUCCESS;
  QTest::newRow("typeFetTest:eh22y092.dat") << "FetTest_LVM_WS/no_converter_external_file/eh22y092.dat" << PARSER_TEST_SUCCESS;
  QTest::newRow("typeVanguardPcm:VanguardPCM.csv") << "VanguardPCM.csv" << PARSER_TEST_SUCCESS;
  QTest::newRow("typeInphiBBA:DP042_P005_2.finished.galaxy.csv") << "DP042_P005_2.finished.galaxy.csv" << PARSER_TEST_SUCCESS;

  // IFF files from GCORE-14661
  QTest::newRow("typeSkyworksIFF:16701_23_I_48333461P1_2018.06.06_06.30.52.std.txt_11.ftpi") << "16701_23_I_48333461P1_2018.06.06_06.30.52.std.txt_11.ftpi" << PARSER_TEST_SUCCESS;
  QTest::newRow("typeSkyworksIFF:16701_23_I_48333461P1_2018.06.06_06.30.52.std.txt_11_2.ftpi") << "16701_23_I_48333461P1_2018.06.06_06.30.52.std.txt_11_2.ftpi" << PARSER_TEST_SUCCESS;
  QTest::newRow("typeSkyworksIFF:M7ASLX52_PK5139_A_4842168.1wp_QMTLJIG_2018_06_06_06_17_37.std.txt_11.prbd") << "M7ASLX52_PK5139_A_4842168.1wp_QMTLJIG_2018_06_06_06_17_37.std.txt_11.prbd" << PARSER_TEST_SUCCESS;
  QTest::newRow("typeSkyworksIFF:EQD_20180605_a.csv_36.pcme") << "EQD_20180605_a.csv_36.pcme" << PARSER_TEST_SUCCESS;
  QTest::newRow("typeSkyworksIFF:DC_ZR017_4757260_3.csv_26.pcmd") << "DC_ZR017_4757260_3.csv_26.pcmd" << PARSER_TEST_SUCCESS;

  QTest::newRow("typeHvmSummary:hvmSummaryWithExt.CSV") << "HvmSummary/hvmSummaryWithExt.CSV" << PARSER_TEST_SUCCESS;
  QTest::newRow("typeHvmSummary:hvmSummaryNoExt.CSV") << "hvmSummaryNoExt.CSV" << PARSER_TEST_SUCCESS;

  QTest::newRow("typeKLA:K16D615.08.sinf") << "K16D615.08.sinf" << PARSER_TEST_SUCCESS;
  QTest::newRow("typeVishayASEQA:VISHAY_SUD50P06-15-GE3_K15K261.case1.log2")
      << "VishayASE_QA/case1/VISHAY_SUD50P06-15-GE3_K15K261.case1.log2"
      << PARSER_TEST_SUCCESS;
  QTest::newRow("typeVishayASEQA:VISHAY_SUD50P06-15-GE3_K15K261.case2.log2")
      << "VishayASE_QA/case2/VISHAY_SUD50P06-15-GE3_K15K261.case2.log2"
      << PARSER_TEST_SUCCESS;
  QTest::newRow("typeVishayASEQA:VISHAY_SUD50P06-15-GE3_K15K261.case3.log2")
      << "VishayASE_QA/case3/VISHAY_SUD50P06-15-GE3_K15K261.case3.log2"
      << PARSER_TEST_SUCCESS;
  QTest::newRow("typeVishayASEQA:VISHAY_SUD50P06-15-GE3_K15K261.case4.log2")
      << "VishayASE_QA/case4/VISHAY_SUD50P06-15-GE3_K15K261.case4.log2"
      << ParserTestExpectedResult( ParserTestExpectedResult::Fail,
                                   ParserTestExpectedResult::RegularExpression,
                                   "typeVishayASEQA - typeVishayASEQA - Invalid file format: - Failed processing input LOG2 \\[.+/input/VishayASE_QA/case4/VISHAY_SUD50P06-15-GE3_K15K261\\.case4\\.log2\\] - Unknown test number 379 in the bin map file .+/input/VishayASE_QA/external_file/CSVFinalTestsFile20181112\\.csv specified in .+/input/VishayASE_QA/case4/converter_external_file\\.xml" );
  QTest::newRow("typeVishayASEQA:VISHAY_SUD50P06-15-GE3_K15K261.case5.log2")
      << "VishayASE_QA/case5/VISHAY_SUD50P06-15-GE3_K15K261.case5.log2"
      << PARSER_TEST_SUCCESS;
  QTest::newRow("typeVishayASEQA:VISHAY_SUD50P06-15-GE3_K15K261.case6.log2")
      << "VishayASE_QA/case6/VISHAY_SUD50P06-15-GE3_K15K261.case6.log2"
      << PARSER_TEST_SUCCESS;
  QTest::newRow("typeVishayASEQA:VISHAY_SUD50P06-15-GE3_K15K261.case7.log2")
      << "VishayASE_QA/case7/VISHAY_SUD50P06-15-GE3_K15K261.case7.log2"
      << PARSER_TEST_SUCCESS;
  QTest::newRow("typeVishayASEQA:VISHAY_SUD50P06-15-GE3_K15K261.17179.log2")
      << "VishayASE_QA/17179/VISHAY_SUD50P06-15-GE3_K15K261.17179.log2"
      << PARSER_TEST_SUCCESS;
  QTest::newRow("typeVishayASEQA:hvm_flow")
      << "VishayASE_QA/hvm_flow/VISHAY_IRF9630PBF_K21K207.1_20180718233151.log2"
      << PARSER_TEST_SUCCESS;

  QTest::newRow("typeFetTest:new .dat format (16 mb input file)")
      << "FetTest/new_dat_format/SCA445EDJ-T1-GE3_K44D924.1_FT_20190115050416.dat"
      << PARSER_TEST_SUCCESS;

  QTest::newRow("typeWatGlobalFoundry:WatGlobalFoundry")
      << "WATGlobalFoundry/small_file.csv"
      << PARSER_TEST_SUCCESS;
}

void TestGqtlParser::convert()
{
    /////////////////////
    // CONVERT TO STDF //
    /////////////////////
    QFETCH(QString, fileToParse);
    QFETCH( ParserTestExpectedResult, testResult );

    std::string lTempParam("./param");

    QFileInfo lFile("./input/" + fileToParse);
    std::string lParserType;

    GS::Parser::ParserFactory *lFactory = GS::Parser::ParserFactory::GetInstance();
    if(lFactory == 0)
    {
        QFAIL("lFactory is null.");
        return;
    }

    QByteArray baInputFile = lFile.absoluteFilePath().toLatin1();
    const char* inputFile = baInputFile.constData();
    GS::Parser::ParserAbstract *lParser = lFactory->CreateParser(inputFile);
    if(lParser == 0 )
    {
        QWARN(inputFile);
        QFAIL("No parser found for this file.");
        return;
    }

    QDir().mkdir("./out");
    lParserType = lParser->GetName();
    // QWARN(lParserType.c_str());
    lParser->SetProgressHandler(0);

    // Create a temporary directory to store parameters definition
    lParser->SetParameterFolder(lTempParam);
    std::string lOutStdfFilePath("./out/" + QFileInfo(lFile).fileName().toStdString() + ".stdf");

    // clean if exists
    QFile lOutStdfFile( QString::fromStdString(lOutStdfFilePath));
    if (lOutStdfFile.exists())
    {
        lOutStdfFile.remove();
    }

    // QWARN(inputFile);
    // QWARN(lOutDir.c_str());
    std::string lErrorMessage;
    GS::Parser::ConverterStatus lStatus = lParser->Convert(inputFile, lOutStdfFilePath);
    lParser->GetLastError(lErrorMessage);

    ProcessTestResult( QString::fromStdString( lErrorMessage ), lStatus, testResult );

    if (lStatus != GS::Parser::ConvertSuccess)
        return;

    ////////////////
    // PARSE ATDF //
    ////////////////
    QString lStdfFileName(QDir::currentPath());
    lStdfFileName.append("/out/" + fileToParse + ".stdf");

    // Get the file name from the parser
    if (lParser->GetListStdfFiles().size() >= 1)
    {
        // use the stdf list
        std::list<std::string> lFiles = lParser->GetListStdfFiles();
        std::list<std::string>::iterator lFirst(lFiles.begin());
        // Get only the first file
        lStdfFileName = QString::fromStdString(*lFirst);
    }
    else
    {
        // use the std string updated by the converter
        lStdfFileName = QString::fromStdString(lOutStdfFilePath);
    }

    QFileInfo lFileInfo(lStdfFileName);
    QString	lOutputAtdfFileName("./out/" + lFileInfo.completeBaseName() + ".atdf");

    // clean if exists
    QFile lOutAtdFile( lOutputAtdfFileName);
    if (lOutAtdFile.exists())
    {
        lOutAtdFile.remove();
    }

    QString strFileNameSTDF(lStdfFileName);
    QString strAtdfFileName(lOutputAtdfFileName);

    int nRecordType;

    // Open Atdf files
    lAtdfFile.setFileName(strAtdfFileName);
    if(lAtdfFile.open(QIODevice::WriteOnly|QIODevice::Text) == false) {
        QFAIL("Error, can not open ATDF file:");
        QFAIL(strAtdfFileName.toLatin1().data());
        return;
    }
    // Assign file I/O stream
    m_hAtdfStream.setDevice(&lAtdfFile);

    // Open STDF file to read...
    QByteArray baFileNameSTDFe = strFileNameSTDF.toLatin1();
    const char* fileNameSTDFe = baFileNameSTDFe.constData();
    int iStatus = m_clStdfParse.Open(fileNameSTDFe);

    // QWARN(fileNameSTDFe);
    if(iStatus == false)
    {
        QFAIL("Error, can not open STDF file.");
        return;
    }
    iStatus = m_clStdfParse.LoadNextRecord(&nRecordType);

    if (iStatus != GQTL_STDF::StdfParse::NoError) {
        QFAIL("Cannot open STDF");
        return;
    }

    while((iStatus == GQTL_STDF::StdfParse::NoError) || (iStatus == GQTL_STDF::StdfParse::UnexpectedRecord))
    {
        std::stringstream nRecordTypeConvert;
        nRecordTypeConvert << nRecordType;
        // QWARN(nRecordTypeConvert.str().c_str());

        // Process STDF record read.
        switch(nRecordType)
        {
        case GQTL_STDF::Stdf_Record::Rec_FAR:
            ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfFAR);
            break;
        case GQTL_STDF::Stdf_Record::Rec_ATR:
            ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfATR);
            break;
        case GQTL_STDF::Stdf_Record::Rec_MIR:
            ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfMIR);
            break;
        case GQTL_STDF::Stdf_Record::Rec_MRR:
            ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfMRR);
            break;
        case GQTL_STDF::Stdf_Record::Rec_PCR:
            ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfPCR);
            break;
        case GQTL_STDF::Stdf_Record::Rec_HBR:
            ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfHBR);
            break;
        case GQTL_STDF::Stdf_Record::Rec_SBR:
            ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfSBR);
            break;
        case GQTL_STDF::Stdf_Record::Rec_PMR:
            ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfPMR);
            break;
        case GQTL_STDF::Stdf_Record::Rec_PGR:
            ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfPGR);
            break;
        case GQTL_STDF::Stdf_Record::Rec_PLR:
            ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfPLR);
            break;
        case GQTL_STDF::Stdf_Record::Rec_RDR:
            ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfRDR);
            break;
        case GQTL_STDF::Stdf_Record::Rec_SDR:
            ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfSDR);
            break;
        case GQTL_STDF::Stdf_Record::Rec_WIR:
            if(ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfWIR) != true)
            {
                // Close STDF file
                m_clStdfParse.Close();
                // Close Atdf files
                lAtdfFile.close();
            }
            break;
        case GQTL_STDF::Stdf_Record::Rec_WRR:
            if(ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfWRR) != true)
            {
                // Close STDF file
                m_clStdfParse.Close();
                // Close Atdf files
                lAtdfFile.close();
            }
            break;
        case GQTL_STDF::Stdf_Record::Rec_WCR:
            ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfWCR);
            break;
        case GQTL_STDF::Stdf_Record::Rec_PIR:
            ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfPIR);
            break;
        case GQTL_STDF::Stdf_Record::Rec_PRR:
            ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfPRR);
            break;
        case GQTL_STDF::Stdf_Record::Rec_TSR:
            ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfTSR);
            break;
        case GQTL_STDF::Stdf_Record::Rec_PTR:
            ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfPTR);
            break;
        case GQTL_STDF::Stdf_Record::Rec_MPR:
            ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfMPR);
            break;
        case GQTL_STDF::Stdf_Record::Rec_FTR:
            ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfFTR);
            break;
        case GQTL_STDF::Stdf_Record::Rec_BPS:
            ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfBPS);
            break;
        case GQTL_STDF::Stdf_Record::Rec_EPS:
            ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfEPS);
            break;
        case GQTL_STDF::Stdf_Record::Rec_GDR:
            ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfGDR);
            break;
        case GQTL_STDF::Stdf_Record::Rec_DTR:
            ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfDTR);
            break;
        case GQTL_STDF::Stdf_Record::Rec_RESERVED_IMAGE:
            ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfRESERVED_IMAGE);
            break;
        case GQTL_STDF::Stdf_Record::Rec_RESERVED_IG900:
            ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfRESERVED_IG900);
            break;
        case GQTL_STDF::Stdf_Record::Rec_UNKNOWN:
            ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfUNKNOWN);
            break;
        case GQTL_STDF::Stdf_Record::Rec_VUR :
            ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfVUR);
            break;
        case GQTL_STDF::Stdf_Record::Rec_PSR :
            ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfPSR);
            break;
        case GQTL_STDF::Stdf_Record::Rec_NMR :
            ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfNMR);
            break;
        case GQTL_STDF::Stdf_Record::Rec_CNR :
            ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfCNR);
            break;
        case GQTL_STDF::Stdf_Record::Rec_SSR :
            ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfSSR);
            break;
        case GQTL_STDF::Stdf_Record::Rec_CDR :
            ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfCDR);
            break;
        case GQTL_STDF::Stdf_Record::Rec_STR :
            ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfSTR);
            break;
        }
            // Read one record from STDF file.
        iStatus = m_clStdfParse.LoadNextRecord(&nRecordType);
    };
    lAtdfFile.close();

    if( lParserType == "typeFetTest" )
    {
        if(!lAtdfFile.fileName().contains(m_clStdfMIR.m_cnSBLOT_ID, Qt::CaseInsensitive))
        {
            std::stringstream lMsg;
            lMsg << " Incorrect naming convention for the generated file name <" << lAtdfFile.fileName().toLatin1().constData() ;
            lMsg << "> Missing SubLotId: <" << QString(m_clStdfMIR.m_cnSBLOT_ID).toLatin1().constData() <<"> .";
            QFAIL(lMsg.str().c_str());
            return;
        }
    }

    ///////////////////
    // Compare files //
    ///////////////////

    // compare Atdf files...
    QString lRefAtdfFileName("./ref/");
    lRefAtdfFileName.append(fileToParse);
    lRefAtdfFileName.append(".atd");

    QFile   lFileRef(lRefAtdfFileName);
    if( lFileRef.open(QIODevice::ReadOnly) == false)
    {
        QFAIL("Cannot open atdf reference file.");
        return;
    }
    // oneRefMultiSiteFound = true;

    QString lStringRef = lFileRef.readAll().simplified();
    lFileRef.close();

    QFile   lFileOut(lOutputAtdfFileName);
    lFileOut.open(QIODevice::ReadOnly);
    QString lStringOut = lFileOut.readAll().simplified();
    lFileOut.close();

    //-- For skywork IFF we have to apply a Qtimezone. However this is not apply on Mac because it chrashes otherwise.
    // -- thus in order to not fail on test due to the date are removed.
    //-- remove all white space in the file, because some are inserted insiede a date if it has been truncate by a \n
    lStringRef.replace(" ","");
    lStringOut.replace(" ","");
    //-- remove hour and date
    lStringRef = lStringRef.remove(QRegExp("[0-9]{2}:[0-9]{2}:[0-9]{2}[0-9]{2}-[a-zA-Z]{3}-[0-9]{4}" ));
    lStringOut = lStringOut.remove(QRegExp("[0-9]{2}:[0-9]{2}:[0-9]{2}[0-9]{2}-[a-zA-Z]{3}-[0-9]{4}" ));

    if (QString::compare(lStringRef, lStringOut) != 0)
    {
        std::stringstream lMsg;
        lMsg << " Generated ATDF file: <" << lOutputAtdfFileName.toLatin1().constData() ;
        lMsg << "> differs from the ref file: <" <<lRefAtdfFileName.toLatin1().constData() <<"> .";
        QFAIL(lMsg.str().c_str());
    }
}

QTEST_MAIN(TestGqtlParser)
