#include "stdf_head_and_site_number_decipher.h"
#include "stdfrecords_v4.h"
#include "stdfparse.h"
#include "stdf.h"
#include "gqtl_log.h"

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::DecipherSiteNumberForAdvantest
    ( unsigned char headNumber, unsigned char siteNumber )
{
    // get the MSB of the head number
    unsigned char head_msb = headNumber & 0xF0;

    // transform head number msb to build deciphered site number
    unsigned short deciphered_site_number_msb = head_msb << 4;

    // return the deciphered site number
    return deciphered_site_number_msb | siteNumber;
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::DecipherSiteNumber
    (
        const StdfParse &parser,
        unsigned char headNumber, unsigned char siteNumber
    )
{
    // specific case for head number in merge mode (255)
    if( headNumber == 255 ) return siteNumber;

    // get the deciphering mode
    GS::StdLib::DecipheringModes mode =
        parser.GetHeadAndSiteNumberDecipheringMode();

    switch( mode )
    {
    // no decipher needed
    case GS::StdLib::no_deciphering:
        return siteNumber;

    // specific advantest deciphering
    case GS::StdLib::advantest_deciphering:
        return DecipherSiteNumberForAdvantest( headNumber, siteNumber );
    }

    return 0;
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::DecipherSiteNumber
    (
        const GS::StdLib::Stdf &stdf,
        unsigned char headNumber, unsigned char siteNumber
    )
{
    // get the deciphering mode
    GS::StdLib::DecipheringModes mode =
        stdf.GetHeadAndSiteNumberDecipheringMode();

    switch( mode )
    {
    // no decipher needed
    case GS::StdLib::no_deciphering:
        return siteNumber;

    // specific advantest deciphering
    case GS::StdLib::advantest_deciphering:
        return DecipherSiteNumberForAdvantest( headNumber, siteNumber );
    }

    return 0;
}

void GQTL_STDF::HeadAndSiteNumberDecipher::SetDecipheringModeInParser
    ( const Stdf_GDR_V4 &gdr, StdfParse &parser )
{
    // this GDR contains fields
    if( gdr.m_u2FLD_CNT > 0 )
    {
        if( ! gdr.m_vnGEN_DATA )
        {
            GSLOG(SYSLOG_SEV_ERROR,
                  "Error while reading GDR record. Generic field is left "
                  "uninitialized.");
            return;
        }

        const GQTL_STDF::stdf_type_vn &firstField = *gdr.m_vnGEN_DATA;

        // works only if the first field is a cn type
        if( firstField.uiDataTypeCode == GQTL_STDF::stdf_type_vn::eTypeCN )
        {
            // look for a specific string
            if( ! firstField.m_cnData.isEmpty() )
            {
                if
                    (
                        firstField.m_cnData.compare
                            (
                              GS::StdLib::ADVANTEST_DECIPHER_NAME_IN_GDR,
                              Qt::CaseInsensitive
                            ) == 0
                    )
                    parser.SetHeadAndSiteNumberDecipheringMode
                        ( GS::StdLib::advantest_deciphering );
                /*else if...*/
                    /*parser.SetDecipheringMode( ... );*/
                /*else if...*/
                    /*parser.SetDecipheringMode( ... );*/
                /*else if...*/
                    /*parser.SetDecipheringMode( ... );*/
            }
        }
    }
}

void GQTL_STDF::HeadAndSiteNumberDecipher::SetDecipheringModeInStdf
    ( const Stdf_GDR_V4 &gdr, GS::StdLib::Stdf &stdf )
{
    // this GDR contains fields
    if( gdr.m_u2FLD_CNT > 0 )
    {
        if( ! gdr.m_vnGEN_DATA )
        {
            GSLOG(SYSLOG_SEV_ERROR,
                  "Error while reading GDR record. Generic field is left "
                  "uninitialized.");
            return;
        }

        const GQTL_STDF::stdf_type_vn &firstField = *gdr.m_vnGEN_DATA;

        // works only if the first field is a cn type
        if( firstField.uiDataTypeCode == GQTL_STDF::stdf_type_vn::eTypeCN )
        {
            // look for a specific string
            if( ! firstField.m_cnData.isEmpty() )
            {
                if
                    (
                        firstField.m_cnData.compare
                            (
                              GS::StdLib::ADVANTEST_DECIPHER_NAME_IN_GDR,
                              Qt::CaseInsensitive
                            ) == 0
                    )
                    stdf.SetHeadAndSiteNumberDecipheringMode
                        ( GS::StdLib::advantest_deciphering );
                /*else if...*/
                    /*stdf.SetDecipheringMode( ... );*/
                /*else if...*/
                    /*stdf.SetDecipheringMode( ... );*/
                /*else if...*/
                    /*stdf.SetDecipheringMode( ... );*/
            }
        }
    }
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
    ( const StdfParse &parser, const Stdf_FTR_V4 &ftr_record )
{
    return
        DecipherSiteNumber
        ( parser, ftr_record.m_u1HEAD_NUM, ftr_record.m_u1SITE_NUM );
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
    ( const StdfParse &parser, const Stdf_HBR_V4 &hbr_record )
{
    return
        DecipherSiteNumber
        ( parser, hbr_record.m_u1HEAD_NUM, hbr_record.m_u1SITE_NUM );
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
    ( const StdfParse &parser, const Stdf_MPR_V4 &mpr_record )
{
    return
        DecipherSiteNumber
        ( parser, mpr_record.m_u1HEAD_NUM, mpr_record.m_u1SITE_NUM );
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
    ( const StdfParse &parser, const Stdf_PCR_V4 &pcr_record )
{
    return
        DecipherSiteNumber
        ( parser, pcr_record.m_u1HEAD_NUM, pcr_record.m_u1SITE_NUM );
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
    ( const StdfParse &parser, const Stdf_PIR_V4 &pir_record )
{
    return
        DecipherSiteNumber
        ( parser, pir_record.m_u1HEAD_NUM, pir_record.m_u1SITE_NUM );
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
    ( const StdfParse &parser, const Stdf_PMR_V4 &pmr_record )
{
    return
        DecipherSiteNumber
        ( parser, pmr_record.m_u1HEAD_NUM, pmr_record.m_u1SITE_NUM );
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
    ( const StdfParse &parser, const Stdf_PRR_V4 &prr_record )
{
    return
        DecipherSiteNumber
        ( parser, prr_record.m_u1HEAD_NUM, prr_record.m_u1SITE_NUM );
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
    ( const StdfParse &parser, const Stdf_PTR_V4 &ptr_record )
{
    return
        DecipherSiteNumber
        ( parser, ptr_record.m_u1HEAD_NUM, ptr_record.m_u1SITE_NUM );
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
    ( const StdfParse &parser, const Stdf_SBR_V4 &sbr_record )
{
    return
        DecipherSiteNumber
        ( parser, sbr_record.m_u1HEAD_NUM, sbr_record.m_u1SITE_NUM );
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
    ( const StdfParse &parser, const Stdf_STR_V4 &str_record )
{
    return
        DecipherSiteNumber
        ( parser, str_record.m_u1HEAD_NUM, str_record.m_u1SITE_NUM );
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
    ( const StdfParse &parser, const Stdf_TSR_V4 &tsr_record )
{
    return
        DecipherSiteNumber
        ( parser, tsr_record.m_u1HEAD_NUM, tsr_record.m_u1SITE_NUM );
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
    ( const GS::StdLib::Stdf &stdf, const Stdf_FTR_V4 &ftr_record )
{
    return
        DecipherSiteNumber
        ( stdf, ftr_record.m_u1HEAD_NUM, ftr_record.m_u1SITE_NUM );
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
    ( const GS::StdLib::Stdf &stdf, const Stdf_HBR_V4 &hbr_record )
{
    return
        DecipherSiteNumber
        ( stdf, hbr_record.m_u1HEAD_NUM, hbr_record.m_u1SITE_NUM );
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
    ( const GS::StdLib::Stdf &stdf, const Stdf_MPR_V4 &mpr_record )
{
    return
        DecipherSiteNumber
        ( stdf, mpr_record.m_u1HEAD_NUM, mpr_record.m_u1SITE_NUM );
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
    ( const GS::StdLib::Stdf &stdf, const Stdf_PCR_V4 &pcr_record )
{
    return
        DecipherSiteNumber
        ( stdf, pcr_record.m_u1HEAD_NUM, pcr_record.m_u1SITE_NUM );
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
    ( const GS::StdLib::Stdf &stdf, const Stdf_PIR_V4 &pir_record )
{
    return
        DecipherSiteNumber
        ( stdf, pir_record.m_u1HEAD_NUM, pir_record.m_u1SITE_NUM );
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
    ( const GS::StdLib::Stdf &stdf, const Stdf_PMR_V4 &pmr_record )
{
    return
        DecipherSiteNumber
        ( stdf, pmr_record.m_u1HEAD_NUM, pmr_record.m_u1SITE_NUM );
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
    ( const GS::StdLib::Stdf &stdf, const Stdf_PRR_V4 &prr_record )
{
    return
        DecipherSiteNumber
        ( stdf, prr_record.m_u1HEAD_NUM, prr_record.m_u1SITE_NUM );
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
    ( const GS::StdLib::Stdf &stdf, const Stdf_PTR_V4 &ptr_record )
{
    return
        DecipherSiteNumber
        ( stdf, ptr_record.m_u1HEAD_NUM, ptr_record.m_u1SITE_NUM );
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
    ( const GS::StdLib::Stdf &stdf, const Stdf_SBR_V4 &sbr_record )
{
    return
        DecipherSiteNumber
        ( stdf, sbr_record.m_u1HEAD_NUM, sbr_record.m_u1SITE_NUM );
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
    ( const GS::StdLib::Stdf &stdf, const Stdf_STR_V4 &str_record )
{
    return
        DecipherSiteNumber
        ( stdf, str_record.m_u1HEAD_NUM, str_record.m_u1SITE_NUM );
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
    ( const GS::StdLib::Stdf &stdf, const Stdf_TSR_V4 &tsr_record )
{
    return
        DecipherSiteNumber
        ( stdf, tsr_record.m_u1HEAD_NUM, tsr_record.m_u1SITE_NUM );
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::DecipherHeadNumberForAdvantest
    ( unsigned char headNumber )
{
    // return the MSB of the head number
    return ( headNumber & 0x0F );
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::DecipherHeadNumber
    (
        const StdfParse &parser,
        unsigned char headNumber
    )
{
    // specific case for head number in merge mode (255)
    if( headNumber == 255 ) return headNumber;

    // get the deciphering mode
    GS::StdLib::DecipheringModes mode =
        parser.GetHeadAndSiteNumberDecipheringMode();

    switch( mode )
    {
    // no decipher needed
    case GS::StdLib::no_deciphering:
        return headNumber;

    // specific advantest deciphering
    case GS::StdLib::advantest_deciphering:
        return DecipherHeadNumberForAdvantest( headNumber );
    }

    return 0;
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::DecipherHeadNumber
    (
        const GS::StdLib::Stdf &stdf,
        unsigned char headNumber
    )
{
    // get the deciphering mode
    GS::StdLib::DecipheringModes mode =
        stdf.GetHeadAndSiteNumberDecipheringMode();

    switch( mode )
    {
    // no decipher needed
    case GS::StdLib::no_deciphering:
        return headNumber;

    // specific advantest deciphering
    case GS::StdLib::advantest_deciphering:
        return DecipherHeadNumberForAdvantest( headNumber );
    }

    return 0;
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetHeadNumberIn
    ( const StdfParse &parser, const Stdf_FTR_V4 &ftr_record )
{
    return
        DecipherHeadNumber
        ( parser, ftr_record.m_u1HEAD_NUM );
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetHeadNumberIn
    ( const StdfParse &parser, const Stdf_HBR_V4 &hbr_record )
{
    return
        DecipherHeadNumber
        ( parser, hbr_record.m_u1HEAD_NUM);
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetHeadNumberIn
    ( const StdfParse &parser, const Stdf_MPR_V4 &mpr_record )
{
    return
        DecipherHeadNumber
        ( parser, mpr_record.m_u1HEAD_NUM );
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetHeadNumberIn
    ( const StdfParse &parser, const Stdf_PCR_V4 &pcr_record )
{
    return
        DecipherHeadNumber
        ( parser, pcr_record.m_u1HEAD_NUM );
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetHeadNumberIn
    ( const StdfParse &parser, const Stdf_PIR_V4 &pir_record )
{
    return
        DecipherHeadNumber
        ( parser, pir_record.m_u1HEAD_NUM );
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetHeadNumberIn
    ( const StdfParse &parser, const Stdf_PMR_V4 &pmr_record )
{
    return
        DecipherHeadNumber
        ( parser, pmr_record.m_u1HEAD_NUM );
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetHeadNumberIn
    ( const StdfParse &parser, const Stdf_PRR_V4 &prr_record )
{
    return
        DecipherHeadNumber
        ( parser, prr_record.m_u1HEAD_NUM );
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetHeadNumberIn
    ( const StdfParse &parser, const Stdf_PTR_V4 &ptr_record )
{
    return
        DecipherHeadNumber
        ( parser, ptr_record.m_u1HEAD_NUM );
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetHeadNumberIn
    ( const StdfParse &parser, const Stdf_SBR_V4 &sbr_record )
{
    return
        DecipherHeadNumber
        ( parser, sbr_record.m_u1HEAD_NUM );
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetHeadNumberIn
    ( const StdfParse &parser, const Stdf_STR_V4 &str_record )
{
    return
        DecipherHeadNumber
        ( parser, str_record.m_u1HEAD_NUM );
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetHeadNumberIn
    ( const StdfParse &parser, const Stdf_TSR_V4 &tsr_record )
{
    return
        DecipherHeadNumber
        ( parser, tsr_record.m_u1HEAD_NUM );
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetHeadNumberIn
    ( const GS::StdLib::Stdf &stdf, const Stdf_FTR_V4 &ftr_record )
{
    return
        DecipherHeadNumber
        ( stdf, ftr_record.m_u1HEAD_NUM );
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetHeadNumberIn
    ( const GS::StdLib::Stdf &stdf, const Stdf_HBR_V4 &hbr_record )
{
    return
        DecipherHeadNumber
        ( stdf, hbr_record.m_u1HEAD_NUM );
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetHeadNumberIn
    ( const GS::StdLib::Stdf &stdf, const Stdf_MPR_V4 &mpr_record )
{
    return
        DecipherHeadNumber
        ( stdf, mpr_record.m_u1HEAD_NUM );
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetHeadNumberIn
    ( const GS::StdLib::Stdf &stdf, const Stdf_PCR_V4 &pcr_record )
{
    return
        DecipherHeadNumber
        ( stdf, pcr_record.m_u1HEAD_NUM );
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetHeadNumberIn
    ( const GS::StdLib::Stdf &stdf, const Stdf_PIR_V4 &pir_record )
{
    return
        DecipherHeadNumber
        ( stdf, pir_record.m_u1HEAD_NUM );
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetHeadNumberIn
    ( const GS::StdLib::Stdf &stdf, const Stdf_PMR_V4 &pmr_record )
{
    return
        DecipherHeadNumber
        ( stdf, pmr_record.m_u1HEAD_NUM );
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetHeadNumberIn
    ( const GS::StdLib::Stdf &stdf, const Stdf_PRR_V4 &prr_record )
{
    return
        DecipherHeadNumber
        ( stdf, prr_record.m_u1HEAD_NUM );
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetHeadNumberIn
    ( const GS::StdLib::Stdf &stdf, const Stdf_PTR_V4 &ptr_record )
{
    return
        DecipherHeadNumber
        ( stdf, ptr_record.m_u1HEAD_NUM );
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetHeadNumberIn
    ( const GS::StdLib::Stdf &stdf, const Stdf_SBR_V4 &sbr_record )
{
    return
        DecipherHeadNumber
        ( stdf, sbr_record.m_u1HEAD_NUM );
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetHeadNumberIn
    ( const GS::StdLib::Stdf &stdf, const Stdf_STR_V4 &str_record )
{
    return
        DecipherHeadNumber
        ( stdf, str_record.m_u1HEAD_NUM );
}

unsigned short GQTL_STDF::HeadAndSiteNumberDecipher::GetHeadNumberIn
    ( const GS::StdLib::Stdf &stdf, const Stdf_TSR_V4 &tsr_record )
{
    return
        DecipherHeadNumber
        ( stdf, tsr_record.m_u1HEAD_NUM );
}
