#include "testable_bin_map_store.h"
#include <sstream>

SerializableToFile::~SerializableToFile() {}

template <typename T>
static void SerializeField(std::ofstream &aOutputFileStream, const BinMapItemBase & aBinMapItem,
                   const std::string& aHeader, T Qx::BinMapping::BinMapItemBase::* aBinMapItemMethod)
{
    try {
        std::stringstream   lOutputStringStream;
        lOutputStringStream << aHeader << (aBinMapItem.*aBinMapItemMethod)() << "|";

        aOutputFileStream << lOutputStringStream.str();
    }
    catch( const Qx::BinMapping::CannotAccessUnhandledField& )
    {
        // voluntarily ignore exception here. It is a generic test case dealing only with BinMapItemBase types which
        // cannot know which method is really implemented or not. Therefore, if the method called is not implemented
        // and throws an exception, nothing is serialized into the output file.
    }
}

std::ofstream & operator <<( std::ofstream &aOutputFileStream, const BinMapItemBase &aBinMapItem )
{
    SerializeField(aOutputFileStream, aBinMapItem, "test enabled:", &BinMapItemBase::GetEnabled);
    SerializeField(aOutputFileStream, aBinMapItem, "test name:", &BinMapItemBase::GetTestName);
    SerializeField(aOutputFileStream, aBinMapItem, "test number:", &BinMapItemBase::GetTestNumber);
    SerializeField(aOutputFileStream, aBinMapItem, "test number alias:", &BinMapItemBase::GetTestNumberAlias);
    SerializeField(aOutputFileStream, aBinMapItem, "binning name:", &BinMapItemBase::GetBinName);
    SerializeField(aOutputFileStream, aBinMapItem, "binning name alias:", &BinMapItemBase::GetBinNameAlias);
    SerializeField(aOutputFileStream, aBinMapItem, "hard bin number:", &BinMapItemBase::GetHardBinNumber);
    SerializeField(aOutputFileStream, aBinMapItem, "soft bin number:", &BinMapItemBase::GetSoftBinNumber);
    SerializeField(aOutputFileStream, aBinMapItem, "binning category:", &BinMapItemBase::GetBinCategory);

    return aOutputFileStream;
}
