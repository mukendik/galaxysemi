#ifndef GEXDB_GLOBAL_FILES_H
#define GEXDB_GLOBAL_FILES_H

#include <QString>

#include "gexdb_plugin_base.h"

class GexdbGlobalFiles
{
public:

    // Constructor / Destructor
    GexdbGlobalFiles(GexDbPlugin_Base *pPluginBase);
    ~GexdbGlobalFiles();

    //! \brief Return last error message
    const QString& lastError() const;
    // Return true if the local file (update) date time is equal or more recent
    // than the one of the database
    bool isLocalFileUpToDate(const QDateTime &dateLocal,
                            const QString &strName,
                            const QString &strType,
                            const QString &strFormat);
    // Return true if file already exists in database
    bool exists(const QString &strName,
                const QString &strType,
                const QString &strFormat);
    // Save file to globel_files table, with a checksum on the file + date
    // Return true if everything is OK
    bool saveFile(const QString &strName,
                                                  const QString &strType,
                                                  const QString &strFormat,
                                                  const QDateTime &dateLastUpdate,
                                                  const QString &strContent);
    // Query the DB to get the requested file, check if the file hasn't been
    // updated with another application by comparing the checksum
    bool loadFile(const QString &strName,
                                                  const QString &strType,
                                                  const QString &strFormat,
                                                  QDateTime &dateLastUpdate,
                                                  QString &strContent);

private:
    QString escapeXMLChar(const QString& strXmlToEscape);
    QString restoreXMLChar(const QString& strEscapedXml);

    QString					m_strLastErrorMsg;
    GexDbPlugin_Base		*m_pPluginBase;
};


#endif // GEXDB_GLOBAL_FILES_H
