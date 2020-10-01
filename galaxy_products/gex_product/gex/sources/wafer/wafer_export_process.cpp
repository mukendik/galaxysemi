#include "wafer_export_process.h"
#include "wafer_export_process_private.h"

namespace GS
{
namespace Gex
{

WaferExportProcess::WaferExportProcess(QObject *lParent)
    : QObject(lParent), mPrivate(new WaferExportProcessPrivate())
{

}

WaferExportProcess::~WaferExportProcess()
{
    if (mPrivate)
    {
        delete mPrivate;
        mPrivate = NULL;
    }
}

bool WaferExportProcess::Execute(const PATProcessing& lSettings)
{
    mPrivate->mErrorMessage.clear();
    mPrivate->mExportedFiles.clear();
    mPrivate->mSettings = lSettings;

    if (mPrivate->LoadSTDFData() == false)
        return false;

    if (mPrivate->HasRequiredParts() == false)
        return false;

    if (mPrivate->GenerateOutput() == false)
    {
        for (int lIdx = 0; lIdx < mPrivate->mExportedFiles.count(); ++lIdx)
            QFile::remove(mPrivate->mExportedFiles.at(lIdx));

        return false;
    }

    return true;
}

QString WaferExportProcess::GetErrorMessage() const
{
    return mPrivate->mErrorMessage;
}

}
}
