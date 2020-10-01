#include "export_csv.h"
#include "csv_converter_v1x.h"
#include "csv_converter_v2x.h"
#include <gqtl_log.h>

CSTDFtoCSV::CSTDFtoCSV() : mCsvVersion(""), mPrivateCSVConverter(NULL)
{
    QVariant varVersion = ReportOptions.GetOption("toolbox", "csv_version");

    if (varVersion.isValid())
    {
        int     majorVersion = -1;
        int     minorVersion = -1;
        bool    okMajor      = true;
        bool    okMinor      = true;

        mCsvVersion     = varVersion.toString();
        majorVersion    = mCsvVersion.section(".", 0, 0).toInt(&okMajor);
        minorVersion    = mCsvVersion.section(".", 1, 1).toInt(&okMinor);

        if (okMajor && okMinor)
            mPrivateCSVConverter = AbstractCsvConverter::CreateConverter(majorVersion, minorVersion);
    }
    else
        GSLOG(SYSLOG_SEV_WARNING, "Invalid CSV version option");
}

CSTDFtoCSV::~CSTDFtoCSV()
{
    if (mPrivateCSVConverter)
    {
        delete mPrivateCSVConverter;
        mPrivateCSVConverter = NULL;
    }
}

bool CSTDFtoCSV::Convert(const QString& stdfFileName, const QString& csvFileName)
{
    if (mPrivateCSVConverter)
        return mPrivateCSVConverter->Convert(stdfFileName, csvFileName);

    return false;
}

QString CSTDFtoCSV::GetLastError() const
{
    if (mPrivateCSVConverter)
        return mPrivateCSVConverter->GetLastError();
    else
        return QString("Unknown CSV output version requested %1").arg(mCsvVersion);
}

void CSTDFtoCSV::setProgressDialog(QProgressDialog* poProgDialog, int iFileNumber, int iFileCount)
{
    if(mPrivateCSVConverter)
	mPrivateCSVConverter->setProgressDialog(poProgDialog,iFileNumber,iFileCount);
}
