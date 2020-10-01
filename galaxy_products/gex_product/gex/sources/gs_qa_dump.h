#ifndef GTM_QA_DUMP_H
#define GTM_QA_DUMP_H

#include <QObject>
#include <QFile>

namespace GS
{
namespace Gex
{

class GsQaDump : public QObject
{
public:

    GsQaDump(QObject * parent = NULL);
    virtual ~GsQaDump();

    // Open dump file
    bool Open(const QString & lShortFileName, QIODevice::OpenMode lMode);
    // Write line to dump file
    qint64 WriteString(const QString & lLine, bool lAddCr=false);
    // Close dump file
    void Close();
    // Remove all dump files in QA output folder
    static void RemoveAllFiles();

private:

    Q_DISABLE_COPY(GsQaDump)

    QFile   mFile;  // Dump file
};

}
}

#endif // GTM_QA_DUMP_H
