#ifndef PARSERPROMISFILE_H
#define PARSERPROMISFILE_H

#include <QString>
#include "parserBase.h"
#include "promis_interpreter_base.h"

namespace GS
{
namespace Parser
{

class ParserPromisFile
{

public:
    enum  errPromisCode
    {
        errPrNoError,                        /// \param No erro (default)
        errPrFieldNb = 1			   /// \param Use this one if promis file has at least one line with not enough (or too mmuch) fields
    };


public:
    ParserPromisFile();
    bool ReadPromisDataFile(const QString& strPromisFileName, const QString &strType,
                            const QString &strFormat, const QString &aPromisKey, const QString &aConverterExternalFilePath);
    bool ReadPromisDataFile_LVM_WS(const QString& strPromisFileName, const QString &aPromisKey, const QString &aConverterExternalFilePath);
    bool ReadPromisDataFile_HVM_WS(const QString& strPromisFileName, const QString &aPromisKey, const QString &aConverterExternalFilePath);
    bool ReadPromisDataFile_LVM_FT(const QString& strPromisFileName, const QString& aPromisKey, const QString &aConverterExternalFilePath);
    bool ReadPromisDataFile_HVM_FT(const QString& strPromisFileName, const QString& aPromisKey, const QString &aConverterExternalFilePath);

    /// !brief read the promis file for the subcon in the format LVM-FT
    bool ReadPromisSubcon_LVM_FT(const QString& aPromisFileNamePath, const QString &aPromisKey, const QString &aConverterExternalFilePath);

    const QString& GetTestCode() const { return mTestCode; }
    const QString& GetExtraID() const  { return m_strExtraID; }
    const QString& GetProductId() const { return mProductId; }
    const QString& GetPackageType() const { return mPackageType; }
    QString GetFacilityID() const;
    QString GetSiteLocation() const;
    QString GetEquipmentID() const;
    QString GetTesterType() const;
    QString GetDsPartNumber() const;
    QString GetOperator() const;
    QString GetLotId() const;
    QString GetSublotId() const;
    QString GetProber() const;
    QString GetDateCode() const;
    QString GetRomCode() const;
    QString GetProcID() const;
    QString GetPromisLotId_D2() const;
    QString GetGeometryName_D2() const;
    QString GetPromisLotId_D3() const;
    QString GetGeometryName_D3() const;
    QString GetPromisLotId_D4() const;
    QString GetGeometryName_D4() const;
    int GetGrossDieCount() const;
    BYTE GetFlat() const;
    float GetDieH() const;
    float GetDieW() const;
    QString GetLastErrorMessage() const;
    errPromisCode GetLastErrorCode() const;

    void SetLotId(const QString &strLotId);
    void SetSublotId(const QString &aSublotId);
    void SetProductId(const QString &strProductId);
    ///!brief init common parameters
    void InitParameters();

private:

    QString		m_strFacilityID;	// Fab location
    QString		m_strSiteLocation;	// Test site location
    QString		m_strExtraID;
    QString		m_strEquipmentID;
    QString		m_strTesterType;
    QString		m_strDsPartNumber;
    QString		m_strOperator;
    QString		mProductId;
    QString		m_strLotId;
    QString		m_strSublotId;
    QString		m_strProber;
    QString		m_strDateCode;
    QString		mRomCode;
    QString     mTestCode;
    QString		m_strProcID;
    QString		mPackageType;
    QString		m_strPromisLotId_D2;
    QString		m_strGeometryName_D2;
    QString		m_strPromisLotId_D3;
    QString		m_strGeometryName_D3;
    QString		m_strPromisLotId_D4;
    QString		m_strGeometryName_D4;
    int			m_iGrossDieCount;
    BYTE		m_cFlat;
    float		m_lfDieH;
    float		m_lfDieW;
    QString     	mLastErrorMessage;
    errPromisCode	mLastErrorCode;

    QScopedPointer< Qx::BinMapping::PromisInterpreterBase > mPromisInterpreter;
};

}
}

#endif // PARSERPROMISFILE_H
