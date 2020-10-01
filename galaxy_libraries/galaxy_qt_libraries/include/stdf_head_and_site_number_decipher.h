#ifndef STDF_HEAD_AND_SITE_NUMBER_DECIPHER
#define STDF_HEAD_AND_SITE_NUMBER_DECIPHER

#include "stdf_common.h"

// forward declarations
namespace GS
{
namespace StdLib
{
    class Stdf;
}
}

namespace GQTL_STDF
{

// forward declarations
class StdfParse;
class Stdf_GDR_V4;
class Stdf_FTR_V4;
class Stdf_HBR_V4;
class Stdf_MPR_V4;
class Stdf_PCR_V4;
class Stdf_PIR_V4;
class Stdf_PMR_V4;
class Stdf_PRR_V4;
class Stdf_PTR_V4;
class Stdf_SBR_V4;
class Stdf_SDR_V4;
class Stdf_STR_V4;
class Stdf_TSR_V4;

// This class is intended to decode a site number in the specific case the
// overflow their standard limit of 255
class HeadAndSiteNumberDecipher
{
    /** Site number **/

    // method responsible to decode the site number, according to the decipher
    // mode stored in the parser
    static unsigned short DecipherSiteNumber
        (
            const StdfParse &parser,
            unsigned char headNumber, unsigned char siteNumber
        );





    /** Head number **/

    // method responsible to decode the head number, according to the decipher
    // mode stored in the parser
    static unsigned short DecipherHeadNumber
        (
            const StdfParse &parser,
            unsigned char headNumber
        );

public :
    // method deciphering a head number for advantest
    static unsigned short DecipherHeadNumberForAdvantest
        ( unsigned char headNumber );

    // method deciphering a site number for advantest
    static unsigned short DecipherSiteNumberForAdvantest
        ( unsigned char headNumber, unsigned char siteNumber );

    // method responsible to decode the head number, according to the decipher
    // mode stored in the stdf
    static unsigned short DecipherHeadNumber
        (
            const GS::StdLib::Stdf &stdf,
            unsigned char headNumber
        );

    // method responsible to decode the site number, according to the decipher
    // mode stored in the stdf
    static unsigned short DecipherSiteNumber
        (
            const GS::StdLib::Stdf &stdf,
            unsigned char headNumber, unsigned char siteNumber
        );

    // initialize a decyphering mode in a specific parser thanks to the content
    // of a GDR record
    static void SetDecipheringModeInParser
        ( const Stdf_GDR_V4 &gdr, StdfParse &parser );

    // initialize a decyphering mode in a specific stdf thanks to the content
    // of a GDR record
    static void SetDecipheringModeInStdf
        ( const Stdf_GDR_V4 &gdr, GS::StdLib::Stdf &stdf );

    /** Site number **/

    // this is the method deciphering a site number according to a specific
    // context included in a parser instance
    static unsigned short GetSiteNumberIn
        ( const StdfParse &parser, const Stdf_FTR_V4 &ftr_record );

    // this is the method deciphering a site number according to a specific
    // context included in a parser instance
    static unsigned short GetSiteNumberIn
        ( const StdfParse &parser, const Stdf_HBR_V4 &hbr_record );

    // this is the method deciphering a site number according to a specific
    // context included in a parser instance
    static unsigned short GetSiteNumberIn
        ( const StdfParse &parser, const Stdf_MPR_V4 &mpr_record );

    // this is the method deciphering a site number according to a specific
    // context included in a parser instance
    static unsigned short GetSiteNumberIn
        ( const StdfParse &parser, const Stdf_PCR_V4 &pcr_record );

    // this is the method deciphering a site number according to a specific
    // context included in a parser instance
    static unsigned short GetSiteNumberIn
        ( const StdfParse &parser, const Stdf_PIR_V4 &pir_record );

    // this is the method deciphering a site number according to a specific
    // context included in a parser instance
    static unsigned short GetSiteNumberIn
        ( const StdfParse &parser, const Stdf_PMR_V4 &pmr_record );

    // this is the method deciphering a site number according to a specific
    // context included in a parser instance
    static unsigned short GetSiteNumberIn
        ( const StdfParse &parser, const Stdf_PRR_V4 &prr_record );

    // this is the method deciphering a site number according to a specific
    // context included in a parser instance
    static unsigned short GetSiteNumberIn
        ( const StdfParse &parser, const Stdf_PTR_V4 &ptr_record );

    // this is the method deciphering a site number according to a specific
    // context included in a parser instance
    static unsigned short GetSiteNumberIn
        ( const StdfParse &parser, const Stdf_SBR_V4 &sbr_record );

    // this is the method deciphering a site number according to a specific
    // context included in a parser instance
    static unsigned short GetSiteNumberIn
        ( const StdfParse &parser, const Stdf_STR_V4 &str_record );

    // this is the method deciphering a site number according to a specific
    // context included in a parser instance
    static unsigned short GetSiteNumberIn
        ( const StdfParse &parser, const Stdf_TSR_V4 &tsr_record );

    // this is the method deciphering a site number according to a specific
    // context included in a stdf instance
    static unsigned short GetSiteNumberIn
        ( const GS::StdLib::Stdf &stdf, const Stdf_FTR_V4 &ftr_record );

    // this is the method deciphering a site number according to a specific
    // context included in a stdf instance
    static unsigned short GetSiteNumberIn
        ( const GS::StdLib::Stdf &stdf, const Stdf_HBR_V4 &hbr_record );

    // this is the method deciphering a site number according to a specific
    // context included in a stdf instance
    static unsigned short GetSiteNumberIn
        ( const GS::StdLib::Stdf &stdf, const Stdf_MPR_V4 &mpr_record );

    // this is the method deciphering a site number according to a specific
    // context included in a stdf instance
    static unsigned short GetSiteNumberIn
        ( const GS::StdLib::Stdf &stdf, const Stdf_PCR_V4 &pcr_record );

    // this is the method deciphering a site number according to a specific
    // context included in a stdf instance
    static unsigned short GetSiteNumberIn
        ( const GS::StdLib::Stdf &stdf, const Stdf_PIR_V4 &pir_record );

    // this is the method deciphering a site number according to a specific
    // context included in a stdf instance
    static unsigned short GetSiteNumberIn
        ( const GS::StdLib::Stdf &stdf, const Stdf_PMR_V4 &pmr_record );

    // this is the method deciphering a site number according to a specific
    // context included in a stdf instance
    static unsigned short GetSiteNumberIn
        ( const GS::StdLib::Stdf &stdf, const Stdf_PRR_V4 &prr_record );

    // this is the method deciphering a site number according to a specific
    // context included in a stdf instance
    static unsigned short GetSiteNumberIn
        ( const GS::StdLib::Stdf &stdf, const Stdf_PTR_V4 &ptr_record );

    // this is the method deciphering a site number according to a specific
    // context included in a stdf instance
    static unsigned short GetSiteNumberIn
        ( const GS::StdLib::Stdf &stdf, const Stdf_SBR_V4 &sbr_record );

    // this is the method deciphering a site number according to a specific
    // context included in a stdf instance
    static unsigned short GetSiteNumberIn
        ( const GS::StdLib::Stdf &stdf, const Stdf_STR_V4 &str_record );

    // this is the method deciphering a site number according to a specific
    // context included in a stdf instance
    static unsigned short GetSiteNumberIn
        ( const GS::StdLib::Stdf &stdf, const Stdf_TSR_V4 &tsr_record );

    /** Head number **/

    // this is the method deciphering a head number according to a specific
    // context included in a parser instance
    static unsigned short GetHeadNumberIn
        ( const StdfParse &parser, const Stdf_FTR_V4 &ftr_record );

    // this is the method deciphering a head number according to a specific
    // context included in a parser instance
    static unsigned short GetHeadNumberIn
        ( const StdfParse &parser, const Stdf_HBR_V4 &hbr_record );

    // this is the method deciphering a head number according to a specific
    // context included in a parser instance
    static unsigned short GetHeadNumberIn
        ( const StdfParse &parser, const Stdf_MPR_V4 &mpr_record );

    // this is the method deciphering a head number according to a specific
    // context included in a parser instance
    static unsigned short GetHeadNumberIn
        ( const StdfParse &parser, const Stdf_PCR_V4 &pcr_record );

    // this is the method deciphering a head number according to a specific
    // context included in a parser instance
    static unsigned short GetHeadNumberIn
        ( const StdfParse &parser, const Stdf_PIR_V4 &pir_record );

    // this is the method deciphering a head number according to a specific
    // context included in a parser instance
    static unsigned short GetHeadNumberIn
        ( const StdfParse &parser, const Stdf_PMR_V4 &pmr_record );

    // this is the method deciphering a head number according to a specific
    // context included in a parser instance
    static unsigned short GetHeadNumberIn
        ( const StdfParse &parser, const Stdf_PRR_V4 &prr_record );

    // this is the method deciphering a head number according to a specific
    // context included in a parser instance
    static unsigned short GetHeadNumberIn
        ( const StdfParse &parser, const Stdf_PTR_V4 &ptr_record );

    // this is the method deciphering a head number according to a specific
    // context included in a parser instance
    static unsigned short GetHeadNumberIn
        ( const StdfParse &parser, const Stdf_SBR_V4 &sbr_record );

    // this is the method deciphering a head number according to a specific
    // context included in a parser instance
    static unsigned short GetHeadNumberIn
        ( const StdfParse &parser, const Stdf_STR_V4 &str_record );

    // this is the method deciphering a head number according to a specific
    // context included in a parser instance
    static unsigned short GetHeadNumberIn
        ( const StdfParse &parser, const Stdf_TSR_V4 &tsr_record );

    // this is the method deciphering a head number according to a specific
    // context included in a stdf instance
    static unsigned short GetHeadNumberIn
        ( const GS::StdLib::Stdf &stdf, const Stdf_FTR_V4 &ftr_record );

    // this is the method deciphering a head number according to a specific
    // context included in a stdf instance
    static unsigned short GetHeadNumberIn
        ( const GS::StdLib::Stdf &stdf, const Stdf_HBR_V4 &hbr_record );

    // this is the method deciphering a head number according to a specific
    // context included in a stdf instance
    static unsigned short GetHeadNumberIn
        ( const GS::StdLib::Stdf &stdf, const Stdf_MPR_V4 &mpr_record );

    // this is the method deciphering a head number according to a specific
    // context included in a stdf instance
    static unsigned short GetHeadNumberIn
        ( const GS::StdLib::Stdf &stdf, const Stdf_PCR_V4 &pcr_record );

    // this is the method deciphering a head number according to a specific
    // context included in a stdf instance
    static unsigned short GetHeadNumberIn
        ( const GS::StdLib::Stdf &stdf, const Stdf_PIR_V4 &pir_record );

    // this is the method deciphering a head number according to a specific
    // context included in a stdf instance
    static unsigned short GetHeadNumberIn
        ( const GS::StdLib::Stdf &stdf, const Stdf_PMR_V4 &pmr_record );

    // this is the method deciphering a head number according to a specific
    // context included in a stdf instance
    static unsigned short GetHeadNumberIn
        ( const GS::StdLib::Stdf &stdf, const Stdf_PRR_V4 &prr_record );

    // this is the method deciphering a head number according to a specific
    // context included in a stdf instance
    static unsigned short GetHeadNumberIn
        ( const GS::StdLib::Stdf &stdf, const Stdf_PTR_V4 &ptr_record );

    // this is the method deciphering a head number according to a specific
    // context included in a stdf instance
    static unsigned short GetHeadNumberIn
        ( const GS::StdLib::Stdf &stdf, const Stdf_SBR_V4 &sbr_record );

    // this is the method deciphering a head number according to a specific
    // context included in a stdf instance
    static unsigned short GetHeadNumberIn
        ( const GS::StdLib::Stdf &stdf, const Stdf_STR_V4 &str_record );

    // this is the method deciphering a head number according to a specific
    // context included in a stdf instance
    static unsigned short GetHeadNumberIn
        ( const GS::StdLib::Stdf &stdf, const Stdf_TSR_V4 &tsr_record );
};

}

#endif // STDF_HEAD_AND_SITE_NUMBER_DECIPHER
