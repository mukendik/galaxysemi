#ifndef GQTL_DATAKEYS_LOADER_H
#define GQTL_DATAKEYS_LOADER_H

#include <QObject>

#include "gqtl_datakeys_global.h"
#include "gqtl_datakeys_content.h"

struct GsData;

namespace GS
{
namespace QtLib
{

class Range;
/*! \class DatakeysLoader
 * \brief class used to load content from a data file into a DatakeysContent container.
 *
 */
class GQTL_DATAKEYSSHARED_EXPORT DatakeysLoader: public QObject
{
    Q_OBJECT

public:
    /// \brief Default Constructor
    DatakeysLoader(QObject *parent = NULL);
    /// \brief Destructor
    ~DatakeysLoader();
    /// \brief Load data into container (only STDF V4 supported for now)
    static bool Load(const QString & dataFilePath, DatakeysContent & keysContent, QString & errorString,
                     bool *checkPassBinNotInList=NULL, GS::QtLib::Range *range_PassBinlistForRejectTest=NULL,
                     GS::QtLib::Range *globalYieldBinCheck=NULL, bool fromInputFile=false);
    /*!
     * \fn Load 2
     */
    static bool Load(struct GsData* lGsData,
                     int lSqliteSplitlotId,
                     const QString& dataFilePath,
                     DatakeysContent& keysContent,
                     QString& errorString,
                     bool fromInputFile = false);
    // \brief The following function overload the keys content values for the targeted STDF Record
    static bool LoadMIR(int stdfVersion, GQTL_STDF::StdfParse &stdfReader, DatakeysContent & keysContent,
                        QString & errorString, bool fromInputFile=false);
    static bool LoadMRR(int stdfVersion, GQTL_STDF::StdfParse &stdfReader, DatakeysContent & keysContent,
                        QString & errorString, bool fromInputFile=false);
    static bool LoadHBR(int stdfVersion, GQTL_STDF::StdfParse &stdfReader, DatakeysContent & keysContent,
                        QString & errorString, bool *checkPassBinNotInList,
                        GS::QtLib::Range *range_PassBinlistForRejectTest, GS::QtLib::Range *globalYieldBinCheck);
    static bool LoadSBR(int stdfVersion, GQTL_STDF::StdfParse &stdfReader, DatakeysContent & keysContent,
                        QString & errorString, GS::QtLib::Range *globalYieldBinCheck);
    static bool LoadSDR(int stdfVersion, GQTL_STDF::StdfParse &stdfReader, DatakeysContent & keysContent,
                        QString & errorString, bool fromInputFile=false);
    static bool LoadWIR(int stdfVersion, GQTL_STDF::StdfParse &stdfReader, DatakeysContent & keysContent,
                        QString & errorString, bool fromInputFile=false);
    static bool LoadWRR(int stdfVersion, GQTL_STDF::StdfParse &stdfReader, DatakeysContent & keysContent,
                        QString & errorString, bool fromInputFile=false);
    static bool LoadWCR(int stdfVersion, GQTL_STDF::StdfParse &stdfReader, DatakeysContent & keysContent,
                        QString & errorString, bool fromInputFile=false);
    static bool LoadPRR(int stdfVersion, GQTL_STDF::StdfParse &stdfReader, DatakeysContent & keysContent,
                        QString & errorString,bool *checkPassBinNotInList=NULL,
                        GS::QtLib::Range *range_PassBinlistForRejectTest=NULL, GS::QtLib::Range *globalYieldBinCheck=NULL);
    static bool LoadDTR(int stdfVersion, GQTL_STDF::StdfParse &stdfReader, DatakeysContent & keysContent,
                        QString & errorString );
private:
    /*!
     * \fn LoadSqlite
     */
    static bool LoadSqlite(struct GsData* d,
                           int splitlotId,
                           const QString& dataFilePath,
                           DatakeysContent& keysContent,
                           QString & errorString,
                           bool fromInputFile);
};

} //END namespace QtLib
} //END namespace GS

#endif // GQTL_DATAKEYS_LOADER_H
