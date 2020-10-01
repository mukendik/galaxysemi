#include "simple_stdf_to_atdf_converter.h"
#include "stdfparse.h"

#include <QFile>
#include <QTextStream>

SimpleStdfToAtdfConverter::SimpleStdfToAtdfConverter(const QString &aStdfFilePath, const QString &aAtdfFilePath) :
    mStdfFilePath{ aStdfFilePath },
    mAtdfFilePath{ aAtdfFilePath } {}

void SimpleStdfToAtdfConverter::RunConversion()
{
    OpenFilesAndAcquireStream();
    Convert();
}

void SimpleStdfToAtdfConverter::OpenFilesAndAcquireStream()
{
    InitializeStdfParser();
    AcquireAtdfFileStream();
}

void SimpleStdfToAtdfConverter::AcquireAtdfFileStream()
{
    mAtdfFile.reset( new QFile{ mAtdfFilePath } );

    if( ! mAtdfFile->open( QFile::WriteOnly | QFile::Text ) )
        throw "Cannot create ATDF file for conversion";

    mAtdfFileStream.reset( new QTextStream{ mAtdfFile.data() } );
}

void SimpleStdfToAtdfConverter::InitializeStdfParser()
{
    if( ! mStdfParser.Open( mStdfFilePath.toStdString().c_str() ) )
        throw "Cannot open STDF file for conversion";
}

void SimpleStdfToAtdfConverter::Convert()
{
    for( int lRecordTypeRead; mStdfParser.LoadNextRecord( &lRecordTypeRead ) == GQTL_STDF::StdfParse::NoError; )
        PushAtdfRecordInAtdfFileStream( lRecordTypeRead );
}

void SimpleStdfToAtdfConverter::PushAtdfRecordInAtdfFileStream(int aRecordType)
{
    // MIR and MRR are voluntarily ignored as it contains current date and time informations not suitable for test
    // purposes
    GQTL_STDF::Stdf_FAR_V4 lFar;
    GQTL_STDF::Stdf_ATR_V4 lAtr;
    GQTL_STDF::Stdf_PCR_V4 lPcr;
    GQTL_STDF::Stdf_HBR_V4 lHbr;
    GQTL_STDF::Stdf_SBR_V4 lSbr;
    GQTL_STDF::Stdf_PMR_V4 lPmr;
    GQTL_STDF::Stdf_PGR_V4 lPgr;
    GQTL_STDF::Stdf_PLR_V4 lPlr;
    GQTL_STDF::Stdf_RDR_V4 lRdr;
    GQTL_STDF::Stdf_SDR_V4 lSdr;
    GQTL_STDF::Stdf_WIR_V4 lWir;
    GQTL_STDF::Stdf_WRR_V4 lWrr;
    GQTL_STDF::Stdf_WCR_V4 lWcr;
    GQTL_STDF::Stdf_PIR_V4 lPir;
    GQTL_STDF::Stdf_PRR_V4 lPrr;
    GQTL_STDF::Stdf_TSR_V4 lTsr;
    GQTL_STDF::Stdf_PTR_V4 lPtr;
    GQTL_STDF::Stdf_MPR_V4 lMpr;
    GQTL_STDF::Stdf_FTR_V4 lFtr;
    GQTL_STDF::Stdf_BPS_V4 lBps;
    GQTL_STDF::Stdf_EPS_V4 lEps;
    GQTL_STDF::Stdf_GDR_V4 lGdr;
    GQTL_STDF::Stdf_DTR_V4 lDtr;
    GQTL_STDF::Stdf_RESERVED_IMAGE_V4 lReservedImageV4;
    GQTL_STDF::Stdf_RESERVED_IG900_V4 lReservedIG900V4;
    GQTL_STDF::Stdf_UNKNOWN_V4 lUnknownV4;
    GQTL_STDF::Stdf_VUR_V4 lVur;
    GQTL_STDF::Stdf_PSR_V4 lPsr;
    GQTL_STDF::Stdf_NMR_V4 lNmr;
    GQTL_STDF::Stdf_CNR_V4 lCnr;
    GQTL_STDF::Stdf_SSR_V4 lSsr;
    GQTL_STDF::Stdf_STR_V4 lStr;
    GQTL_STDF::Stdf_CDR_V4 lCdr;

    switch( aRecordType )
    {
    default :
        throw "unexpected record found while converting STDF file to ATDF";

    case GQTL_STDF::Stdf_Record::Rec_FAR :
        WriteAtdfRecordInStream( &lFar );
        break;
    case GQTL_STDF::Stdf_Record::Rec_ATR :
        WriteAtdfRecordInStream( &lAtr );
        break;
    case GQTL_STDF::Stdf_Record::Rec_MIR :
        break;
    case GQTL_STDF::Stdf_Record::Rec_MRR :
        break;
    case GQTL_STDF::Stdf_Record::Rec_PCR :
        WriteAtdfRecordInStream( &lPcr );
        break;
    case GQTL_STDF::Stdf_Record::Rec_HBR :
        WriteAtdfRecordInStream( &lHbr );
        break;
    case GQTL_STDF::Stdf_Record::Rec_SBR :
        WriteAtdfRecordInStream( &lSbr );
        break;
    case GQTL_STDF::Stdf_Record::Rec_PMR :
        WriteAtdfRecordInStream( &lPmr );
        break;
    case GQTL_STDF::Stdf_Record::Rec_PGR :
        WriteAtdfRecordInStream( &lPgr );
        break;
    case GQTL_STDF::Stdf_Record::Rec_PLR :
        WriteAtdfRecordInStream( &lPlr );
        break;
    case GQTL_STDF::Stdf_Record::Rec_RDR :
        WriteAtdfRecordInStream( &lRdr );
        break;
    case GQTL_STDF::Stdf_Record::Rec_SDR :
        WriteAtdfRecordInStream( &lSdr );
        break;
    case GQTL_STDF::Stdf_Record::Rec_WIR :
        WriteAtdfRecordInStream( &lWir );
        break;
    case GQTL_STDF::Stdf_Record::Rec_WRR :
        WriteAtdfRecordInStream( &lWrr );
        break;
    case GQTL_STDF::Stdf_Record::Rec_WCR :
        WriteAtdfRecordInStream( &lWcr );
        break;
    case GQTL_STDF::Stdf_Record::Rec_PIR :
        WriteAtdfRecordInStream( &lPir );
        break;
    case GQTL_STDF::Stdf_Record::Rec_PRR :
        WriteAtdfRecordInStream( &lPrr );
        break;
    case GQTL_STDF::Stdf_Record::Rec_TSR :
        WriteAtdfRecordInStream( &lTsr );
        break;
    case GQTL_STDF::Stdf_Record::Rec_PTR :
        WriteAtdfRecordInStream( &lPtr );
        break;
    case GQTL_STDF::Stdf_Record::Rec_MPR :
        WriteAtdfRecordInStream( &lMpr );
        break;
    case GQTL_STDF::Stdf_Record::Rec_FTR :
        WriteAtdfRecordInStream( &lFtr );
        break;
    case GQTL_STDF::Stdf_Record::Rec_BPS :
        WriteAtdfRecordInStream( &lBps );
        break;
    case GQTL_STDF::Stdf_Record::Rec_EPS :
        WriteAtdfRecordInStream( &lEps );
        break;
    case GQTL_STDF::Stdf_Record::Rec_GDR :
        WriteAtdfRecordInStream( &lGdr );
        break;
    case GQTL_STDF::Stdf_Record::Rec_DTR :
        WriteAtdfRecordInStream( &lDtr );
        break;
    case GQTL_STDF::Stdf_Record::Rec_RESERVED_IMAGE :
        WriteAtdfRecordInStream( &lReservedImageV4 );
        break;
    case GQTL_STDF::Stdf_Record::Rec_RESERVED_IG900 :
        WriteAtdfRecordInStream( &lReservedIG900V4 );
        break;
    case GQTL_STDF::Stdf_Record::Rec_UNKNOWN :
        WriteAtdfRecordInStream( &lUnknownV4 );
        break;
    case GQTL_STDF::Stdf_Record::Rec_VUR :
        WriteAtdfRecordInStream( &lVur );
        break;
    case GQTL_STDF::Stdf_Record::Rec_PSR :
        WriteAtdfRecordInStream( &lPsr );
        break;
    case GQTL_STDF::Stdf_Record::Rec_NMR :
        WriteAtdfRecordInStream( &lNmr );
        break;
    case GQTL_STDF::Stdf_Record::Rec_CNR :
        WriteAtdfRecordInStream( &lCnr );
        break;
    case GQTL_STDF::Stdf_Record::Rec_SSR :
        WriteAtdfRecordInStream( &lSsr );
        break;
    case GQTL_STDF::Stdf_Record::Rec_STR :
        WriteAtdfRecordInStream( &lStr );
        break;
    case GQTL_STDF::Stdf_Record::Rec_CDR :
        WriteAtdfRecordInStream( &lCdr );
        break;
    }
}

void SimpleStdfToAtdfConverter::WriteAtdfRecordInStream(GQTL_STDF::Stdf_Record *aRecord)
{
    if( ! mStdfParser.ReadRecord( aRecord ) )
        throw "Could not read stdf record for conversion";

    QString	lAtdfString;
    aRecord->GetAtdfString(lAtdfString);
    *mAtdfFileStream << lAtdfString;
}
