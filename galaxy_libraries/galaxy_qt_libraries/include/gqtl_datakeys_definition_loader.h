#ifndef DATAKEYS_DEFINITION_LOADER_H
#define DATAKEYS_DEFINITION_LOADER_H

#include <QString>
#include <gqtl_datakeys_data.h>

namespace GS
{
namespace QtLib
{
class DataKeysDefinitionLoaderPrivate;

/// \brief The DataKeysDefinitionLoader singleton class is responsible of the load of the keys supported by the gexbdkeys
class GQTL_DATAKEYSSHARED_EXPORT DataKeysDefinitionLoader
{
public:

    /// \brief GetInstance instanciate et return the singleton
    /// \return the singleton
    static DataKeysDefinitionLoader &GetInstance();
    ////// \brief DestroyInstance destroy the singleton
    static void DestroyInstance();
    ///
    /// \brief LoadingPass gives the status of the XML loading
    /// \param errorMessage : contains the root cause of the loading error
    /// \return true if the loading is Ok else false
    bool LoadingPass(QString &errorMessage);

    QStringList GetStaticKeys();
    DataKeysData GetDataKeysData(const QString &key);

    /// \brief initialize the datakeys list
    void initialize();
    /// \brief clear the datakeys list
    void clear();
private:
    DataKeysDefinitionLoader();
    ~DataKeysDefinitionLoader();

    static DataKeysDefinitionLoader *mInstance;
    DataKeysDefinitionLoaderPrivate *mPrivate;
};

}
}
#endif // DATAKEYS_DEFINITION_LOADER_H
