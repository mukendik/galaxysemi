#ifndef WAFER_EXPORT_PROCESS_PRIVATE_H
#define WAFER_EXPORT_PROCESS_PRIVATE_H

#include "gex_pat_processing.h"

namespace GS
{
namespace Gex
{

class WaferExportProcessPrivate
{
public:

    WaferExportProcessPrivate();
    ~WaferExportProcessPrivate();

    QString         mErrorMessage;
    QStringList     mExportedFiles;
    PATProcessing   mSettings;

    bool            LoadSTDFData();
    bool            HasRequiredParts();
    bool            GenerateOutput();
//    void            CreateLogFile(const QString &lDestFile);
};

}
}
#endif // WAFER_EXPORT_PROCESS_PRIVATE_H
