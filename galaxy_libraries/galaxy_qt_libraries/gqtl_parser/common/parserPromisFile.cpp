
#include <QTime>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>

#include "parserPromisFile.h"
#include "converter_external_file.h"
#include "promis_interpreter_factory.h"
#include "promis_item_base.h"
#include "promis_exceptions.h"

namespace GS
{
namespace Parser
{


//////////////////////////////////////////////////////////////////////
// Call the good ReadCustomDataFile
//////////////////////////////////////////////////////////////////////
ParserPromisFile::ParserPromisFile()
{
    // CLear/Set variables to default
    m_iGrossDieCount = -1;
    m_strSublotId = m_strLotId= m_strOperator = m_strProber = "" ;
    m_cFlat = ' ';
    m_lfDieH=0.0;
    m_lfDieW=0.0;
    mLastErrorCode = errPrNoError;
}

bool ParserPromisFile::ReadPromisDataFile(const QString& strPromisFileName, const QString& strType,
                                          const QString& strFormat, const QString& aPromisKey,
                                          const QString &aConverterExternalFilePath )
{
    if(strType == "wafer")
    {
        if(strFormat == "vishay-hvm")
            return ReadPromisDataFile_HVM_WS(strPromisFileName, aPromisKey, aConverterExternalFilePath);
        else
            return ReadPromisDataFile_LVM_WS(strPromisFileName, aPromisKey, aConverterExternalFilePath );
    }
    else
        if(strType == "final")
        {
            if(strFormat == "vishay-hvm")
                return ReadPromisDataFile_HVM_FT(strPromisFileName, aPromisKey, aConverterExternalFilePath );
            else
                return ReadPromisDataFile_LVM_FT(strPromisFileName, aPromisKey, aConverterExternalFilePath );
        }

    return false;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse custom PROMIS file
// return 'false' if data could not be read from specified PROMIS file.
//////////////////////////////////////////////////////////////////////
bool ParserPromisFile::ReadPromisDataFile_LVM_WS(const QString &strPromisFileName,
                                                 const QString& aPromisKey,
                                                 const QString &aConverterExternalFilePath )
{
    // CLear/Set variables to default
    m_strFacilityID = "";
    m_strSiteLocation = "";
    m_strEquipmentID = "";
    m_strTesterType = "";
    m_strDateCode = "";
    mRomCode = "";
    m_strProcID = "";
    mPackageType = "";
    m_strPromisLotId_D2 = "";
    m_strGeometryName_D2 = "";
    m_strPromisLotId_D3 = "";
    m_strGeometryName_D3 = "";
    m_strPromisLotId_D4 = "";
    m_strGeometryName_D4 = "";
    m_strSublotId = m_strLotId;
    m_iGrossDieCount = -1;

    try
    {
        mPromisInterpreter.reset( Qx::BinMapping::PromisInterpreterFactory< Qx::BinMapping::promis_lvm_wt >
                                  ::MakePromisInterpreter( aPromisKey.toStdString(),
                                                           strPromisFileName.toStdString(),
                                                           aConverterExternalFilePath.toStdString() ) );

        const Qx::BinMapping::PromisItemBase &lPromisItem = mPromisInterpreter->GetPromisItem();

        m_strSublotId       = lPromisItem.GetSublotID().c_str();
        m_strFacilityID     = lPromisItem.GetFabSite().c_str();
        m_strEquipmentID    = lPromisItem.GetEquipmentID().c_str();
        m_strExtraID        = lPromisItem.GetEquipmentID().c_str();
        m_strProcID         = lPromisItem.GetPartID().c_str();
        mProductId          = lPromisItem.GetGeometryName().c_str();
        m_iGrossDieCount    = QString::fromStdString(lPromisItem.GetGrossDiePerWafer()).toInt();
        m_lfDieW            = QString::fromStdString(lPromisItem.GetDieWidth()).toFloat();
        m_lfDieH            = QString::fromStdString(lPromisItem.GetDieHeight()).toFloat();
        mTestCode           = lPromisItem.GetTestSite().c_str();

        switch(QString::fromStdString(lPromisItem.GetFlatOrientation()).toInt())
        {
            case 0: m_cFlat = 'D';
                break;
            case 90: m_cFlat = 'L';
                break;
            case 180: m_cFlat = 'U';
                break;
            case 270: m_cFlat = 'R';
                break;
            default:  m_cFlat = ' ';
                break;
        }
    }
    catch( const std::exception &lException )
    {
        mLastErrorMessage = lException.what();
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse custom PROMIS file
// return 'false' if data could not be read from specified PROMIS file.
//////////////////////////////////////////////////////////////////////
bool ParserPromisFile::ReadPromisDataFile_HVM_WS(const QString &strPromisFileName,
                                                 const QString& aPromisKey,
                                                 const QString &aConverterExternalFilePath)
{
    InitParameters();

    try
    {
        mPromisInterpreter.reset( Qx::BinMapping::PromisInterpreterFactory< Qx::BinMapping::promis_hvm_wt >
                                  ::MakePromisInterpreter( aPromisKey.toStdString(),
                                                           strPromisFileName.toStdString(),
                                                           aConverterExternalFilePath.toStdString() ) );

        const Qx::BinMapping::PromisItemBase &lPromisItem = mPromisInterpreter->GetPromisItem();

        m_strSublotId       = lPromisItem.GetSublotID().c_str();
        m_strFacilityID     = lPromisItem.GetFabLocation().c_str();
        m_strProcID         = lPromisItem.GetDiePart().c_str();
        mProductId          = lPromisItem.GetGeometryName().c_str();
        m_iGrossDieCount    = QString::fromStdString(lPromisItem.GetGrossDiePerWafer()).toInt();
        m_lfDieW            = QString::fromStdString(lPromisItem.GetDieWidth()).toFloat();
        m_lfDieH            = QString::fromStdString(lPromisItem.GetDieHeight()).toFloat();
        mTestCode           = lPromisItem.GetSiteLocation().c_str();

        switch(QString::fromStdString(lPromisItem.GetFlatOrientation()).toInt())
        {
            case 0: m_cFlat = 'D';
                break;
            case 90: m_cFlat = 'L';
                break;
            case 180: m_cFlat = 'U';
                break;
            case 270: m_cFlat = 'R';
                break;
            default:  m_cFlat = ' ';
                break;
        }
    }
    catch( const std::exception &lException )
    {
        mLastErrorMessage = lException.what();
        return false;
    }

    return true;
}

void ParserPromisFile::InitParameters()
{
    // CLear/Set variables to default
    m_strFacilityID = "";
    m_strExtraID = "";
    m_strSiteLocation = "";
    m_strProber = "";			// To avoid writing a SDR
    m_strDateCode = "";
    mRomCode = "";
    m_strProcID = "";
    mPackageType = "";
    m_strPromisLotId_D2 = "";
    m_strGeometryName_D2 = "";
    m_strPromisLotId_D3 = "";
    m_strGeometryName_D3 = "";
    m_strPromisLotId_D4 = "";
    m_strGeometryName_D4 = "";
    m_strSublotId = m_strLotId;
    m_iGrossDieCount = -1;
}
//////////////////////////////////////////////////////////////////////
// Read and Parse custom FT PROMIS file
// return 'false' if data could not be read from specified PROMIS file.
//////////////////////////////////////////////////////////////////////
bool ParserPromisFile::ReadPromisDataFile_LVM_FT(const QString& strPromisFileName,
                                                 const QString& aPromisKey,
                                                 const QString &aConverterExternalFilePath )
{
    InitParameters();
    m_strSublotId = aPromisKey;

    try
    {
        QString lPromisFilePath = QFileInfo( strPromisFileName ).absoluteFilePath();

        mPromisInterpreter.reset( Qx::BinMapping::PromisInterpreterFactory< Qx::BinMapping::promis_lvm_ft >
                                  ::MakePromisInterpreter( aPromisKey.toStdString(),
                                                           lPromisFilePath.toStdString(),
                                                           aConverterExternalFilePath.toStdString() ) );

        const Qx::BinMapping::PromisItemBase &lPromisItem = mPromisInterpreter->GetPromisItem();

        m_strDateCode = lPromisItem.GetDateCode().c_str();
        mRomCode = lPromisItem.GetPackage().c_str();
        m_strEquipmentID = lPromisItem.GetEquipmentID().c_str();
        m_strTesterType = lPromisItem.GetEquipmentID().c_str();
        m_strProcID = lPromisItem.GetProductId().c_str();
        mTestCode = lPromisItem.GetSiteId().c_str();
        m_strFacilityID = lPromisItem.GetSiteId().c_str();
        mProductId = lPromisItem.GetGeometryName().c_str();
        mPackageType = lPromisItem.GetPackageType().c_str();
        m_strPromisLotId_D2 = lPromisItem.GetLotIdP2().c_str();
        m_strGeometryName_D2 = lPromisItem.GetGeometryNameP2().c_str();
        m_strPromisLotId_D3 = lPromisItem.GetLotIdP3().c_str();
        m_strGeometryName_D3 = lPromisItem.GetGeometryNameP3().c_str();
        m_strPromisLotId_D4 = lPromisItem.GetLotIdP4().c_str();
        m_strGeometryName_D4 = lPromisItem.GetGeometryNameP4().c_str();
    }
    catch( const Qx::BinMapping::InvalidLvmFtFieldNbPromisFileFormat &lException)
    {
        mLastErrorMessage = lException.what();
        mLastErrorCode = errPrFieldNb;
        return false;
    }
    catch( const std::exception &lException )
    {
        mLastErrorMessage = lException.what();
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse custom FT PROMIS file
// return 'false' if data could not be read from specified PROMIS file.
//////////////////////////////////////////////////////////////////////
bool ParserPromisFile::ReadPromisDataFile_HVM_FT(const QString &strPromisFileName,
                                                 const QString& aPromisKey,
                                                 const QString &aConverterExternalFilePath )
{
    InitParameters();
    m_strSublotId = aPromisKey;

    try
    {
        mPromisInterpreter.reset( Qx::BinMapping::PromisInterpreterFactory< Qx::BinMapping::promis_hvm_ft >
                                  ::MakePromisInterpreter( aPromisKey.toStdString(),
                                                           strPromisFileName.toStdString(),
                                                           aConverterExternalFilePath.toStdString() ) );

        const Qx::BinMapping::PromisItemBase &lPromisItem = mPromisInterpreter->GetPromisItem();

        m_strDateCode = lPromisItem.GetDateCode().c_str();
        mPackageType = lPromisItem.GetPackageType().c_str();
        m_strEquipmentID = lPromisItem.GetEquipmentID().c_str();
        m_strProcID = lPromisItem.GetPartNumber().c_str();
        m_strFacilityID = lPromisItem.GetSiteLocation().c_str();
        mProductId = lPromisItem.GetGeometryName().c_str();
        m_strExtraID = lPromisItem.GetDiePart().c_str();
        m_strSiteLocation = lPromisItem.GetDivision().c_str();
    }
    catch( const std::exception &lException )
    {
        mLastErrorMessage = lException.what();
        return false;
    }

    return true;
}


//////////////////////////////////////////////////////////////////////
// Read and Parse custom FT PROMIS file
// return 'false' if data could not be read from specified PROMIS file.
//////////////////////////////////////////////////////////////////////
bool ParserPromisFile::ReadPromisSubcon_LVM_FT(const QString& aPromisFileNamePath,
                                               const QString& aPromisKey,
                                               const QString &aConverterExternalFilePath )
{
    if(aPromisKey.isEmpty())
    {
        // Failed Opening Promis file
        mLastErrorMessage = "No Promis Lot Id defined";

        // Convertion failed.
        return false;
    }

    try
    {
        QString lPromisFilePath = QFileInfo( aPromisFileNamePath ).absoluteFilePath();

        mPromisInterpreter.reset( Qx::BinMapping::PromisInterpreterFactory< Qx::BinMapping::promis_lvm_ft_subcon_data >
                                  ::MakePromisInterpreter( aPromisKey.toStdString(),
                                                           lPromisFilePath.toStdString(),
                                                           aConverterExternalFilePath.toStdString() ) );

        const Qx::BinMapping::PromisItemBase &lPromisItem = mPromisInterpreter->GetPromisItem();

        m_strProcID = lPromisItem.GetPartNumber().c_str();
        mRomCode = lPromisItem.GetPackageType().c_str();
        mProductId = lPromisItem.GetGeometryName().c_str();
    }
    catch( const std::exception &lException )
    {
        mLastErrorMessage = lException.what();
        return false;
    }

    return true;
}

QString ParserPromisFile::GetFacilityID() const
{
    return m_strFacilityID;
}

QString ParserPromisFile::GetSiteLocation() const
{
    return m_strSiteLocation;
}

QString ParserPromisFile::GetEquipmentID() const
{
    return m_strEquipmentID;
}

QString ParserPromisFile::GetTesterType() const
{
    return m_strTesterType;
}

QString ParserPromisFile::GetDsPartNumber() const
{
    return m_strDsPartNumber;
}

QString ParserPromisFile::GetOperator() const
{
    return m_strOperator;
}

QString ParserPromisFile::GetLotId() const
{
    return m_strLotId;
}

QString ParserPromisFile::GetSublotId() const
{
    return m_strSublotId;
}

QString ParserPromisFile::GetProber() const
{
    return m_strProber;
}

QString ParserPromisFile::GetDateCode() const
{
    return m_strDateCode;
}

QString ParserPromisFile::GetRomCode() const
{
    return mRomCode;
}

QString ParserPromisFile::GetProcID() const
{
    return m_strProcID;
}

QString ParserPromisFile::GetPromisLotId_D2() const
{
    return m_strPromisLotId_D2;
}

QString ParserPromisFile::GetGeometryName_D2() const
{
    return m_strGeometryName_D2;
}

QString ParserPromisFile::GetPromisLotId_D3() const
{
    return m_strPromisLotId_D3;
}

QString ParserPromisFile::GetGeometryName_D3() const
{
    return m_strGeometryName_D3;
}

QString ParserPromisFile::GetPromisLotId_D4() const
{
    return m_strPromisLotId_D4;
}

QString ParserPromisFile::GetGeometryName_D4() const
{
    return m_strGeometryName_D4;
}

int ParserPromisFile::GetGrossDieCount() const
{
    return m_iGrossDieCount;
}

BYTE ParserPromisFile::GetFlat() const
{
    return m_cFlat;
}

float ParserPromisFile::GetDieH() const
{
    return m_lfDieH;
}

float ParserPromisFile::GetDieW() const
{
    return m_lfDieW;
}

QString ParserPromisFile::GetLastErrorMessage() const
{
    return mLastErrorMessage;
}

ParserPromisFile::errPromisCode ParserPromisFile::GetLastErrorCode() const
{
    return mLastErrorCode;
}

void ParserPromisFile::SetLotId(const QString &strLotId)
{
    m_strLotId = strLotId;
}

void ParserPromisFile::SetSublotId(const QString &aSublotId)
{
    m_strSublotId = aSublotId;
}

void ParserPromisFile::SetProductId(const QString &strProductId)
{
    mProductId = strProductId;
}

}
}
