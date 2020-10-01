#ifndef STDF_COMMON
#define STDF_COMMON

namespace GS
{
namespace StdLib
{

    // enum grouping all mode for deciphering test numbers
    enum DecipheringModes
    {
        // no specific deciphering
        no_deciphering,

        // this is the advantest decipher mode. The advanced deciphering
        // consists to use the 4 MSB of the head number to extend the range of
        // the site number.
        // Hence, site number are encoded on 12 bits instead of 8.
        advantest_deciphering
    };

    // the decypher name for advantest retrieved in a GDR record
    const char ADVANTEST_DECIPHER_NAME_IN_GDR[] = "SDR_HC";
}

}

#endif // STDF_COMMON
