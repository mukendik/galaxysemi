#include "testable_promis_interpreter.h"
#include <sstream>

SerializableToFile::~SerializableToFile() {}

template <typename T>
static void SerializeField(std::ofstream &aOutputFileStream, const PromisItemBase & aPromisItem,
                   const std::string& aHeader, T Qx::BinMapping::PromisItemBase::* aPromisItemMethod)
{
    try {
        std::stringstream   lOutputStringStream;
        lOutputStringStream << aHeader << (aPromisItem.*aPromisItemMethod)() << "|";

        aOutputFileStream << lOutputStringStream.str();
    }
    catch( const Qx::BinMapping::PromisItemBase::CannotAccessUnhandledField& )
    {
        // voluntarily ignore exception here. It is a generic test case dealing only with BinMapItemBase types which
        // cannot know which method is really implemented or not. Therefore, if the method called is not implemented
        // and throws an exception, nothing is serialized into the output file.
    }
}

std::ofstream & operator <<( std::ofstream &aOutputFileStream, const PromisItemBase &aPromisItem )
{
    SerializeField(aOutputFileStream, aPromisItem, "part number:", &PromisItemBase::GetPartNumber);
    SerializeField(aOutputFileStream, aPromisItem, "package type:", &PromisItemBase::GetPackageType);
    SerializeField(aOutputFileStream, aPromisItem, "geometry name:", &PromisItemBase::GetGeometryName);
    SerializeField(aOutputFileStream, aPromisItem, "date code:", &PromisItemBase::GetDateCode);
    SerializeField(aOutputFileStream, aPromisItem, "site location:", &PromisItemBase::GetSiteLocation);
    SerializeField(aOutputFileStream, aPromisItem, "wafer count:", &PromisItemBase::GetNbWafers);
    SerializeField(aOutputFileStream, aPromisItem, "fab site:", &PromisItemBase::GetFabSite);
    SerializeField(aOutputFileStream, aPromisItem, "equipment id:", &PromisItemBase::GetEquipmentID);
    SerializeField(aOutputFileStream, aPromisItem, "part id:", &PromisItemBase::GetPartID);
    SerializeField(aOutputFileStream, aPromisItem, "gross die per wafer:", &PromisItemBase::GetGrossDiePerWafer);
    SerializeField(aOutputFileStream, aPromisItem, "die width:", &PromisItemBase::GetDieWidth);
    SerializeField(aOutputFileStream, aPromisItem, "die height:", &PromisItemBase::GetDieHeight);
    SerializeField(aOutputFileStream, aPromisItem, "flat orientation:", &PromisItemBase::GetFlatOrientation);
    SerializeField(aOutputFileStream, aPromisItem, "test site:", &PromisItemBase::GetTestSite);
    SerializeField(aOutputFileStream, aPromisItem, "fab location:", &PromisItemBase::GetFabLocation);
    SerializeField(aOutputFileStream, aPromisItem, "die part:", &PromisItemBase::GetDiePart);
    SerializeField(aOutputFileStream, aPromisItem, "product id:", &PromisItemBase::GetProductId);
    SerializeField(aOutputFileStream, aPromisItem, "site id:", &PromisItemBase::GetSiteId);
    SerializeField(aOutputFileStream, aPromisItem, "package:", &PromisItemBase::GetPackage);
    SerializeField(aOutputFileStream, aPromisItem, "lot id product 2:", &PromisItemBase::GetLotIdP2);
    SerializeField(aOutputFileStream, aPromisItem, "geometry name product 2:", &PromisItemBase::GetGeometryNameP2);
    SerializeField(aOutputFileStream, aPromisItem, "lot id product 3:", &PromisItemBase::GetLotIdP3);
    SerializeField(aOutputFileStream, aPromisItem, "geometry name product 3:", &PromisItemBase::GetGeometryNameP3);
    SerializeField(aOutputFileStream, aPromisItem, "lot id product 4:", &PromisItemBase::GetLotIdP4);
    SerializeField(aOutputFileStream, aPromisItem, "geometry name product 4:", &PromisItemBase::GetGeometryNameP4);
    SerializeField(aOutputFileStream, aPromisItem, "division:", &PromisItemBase::GetDivision);

    return aOutputFileStream;
}

