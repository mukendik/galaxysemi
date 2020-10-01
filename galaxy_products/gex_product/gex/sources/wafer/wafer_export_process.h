#ifndef WAFER_EXPORT_PROCESS_H
#define WAFER_EXPORT_PROCESS_H

#include <QObject>

namespace GS
{
namespace Gex
{

class WaferExportProcessPrivate;
class PATProcessing;

class WaferExportProcess : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(WaferExportProcess)

public:

    WaferExportProcess(QObject * lParent);
    ~WaferExportProcess();

    Q_INVOKABLE bool    Execute(const PATProcessing &lSettings);
    Q_INVOKABLE QString GetErrorMessage() const;

private:

    WaferExportProcessPrivate * mPrivate;
};

}
}
#endif // WAFER_EXPORT_PROCESS_H
