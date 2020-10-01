#ifndef GQTL_DATAKEYS_FILE_H
#define GQTL_DATAKEYS_FILE_H

#include <QObject>

#include "gqtl_datakeys_global.h"

namespace GS
{
namespace QtLib
{

struct DatakeysFilePrivate;

struct DatakeysError
{
    int     mLine;
    QString mMessage;

    inline DatakeysError()
        : mLine(-1) {}

    inline DatakeysError(int lLine, const QString& lMessage)
        : mLine(lLine), mMessage(lMessage)  {}
};

struct DatakeysInfo
{
    int     mLine;          /*!< holds line id*/
    QString mKeyName;       /*!< holds key name*/
    QString mKeyExpression; /*!< holds key expression*/

    inline DatakeysInfo()
        : mLine(-1) {}

    inline DatakeysInfo(int line, const QString& keyName, const QString& keyExp)
        : mLine(line), mKeyName(keyName), mKeyExpression(keyExp) {}
};

/*! \class DatakeysFile
 * \brief class used to read and write *.gexdbkeys files
 *
 */
class GQTL_DATAKEYSSHARED_EXPORT DatakeysFile: public QObject
{
    Q_OBJECT

public:
    /// \brief Default Constructor
    DatakeysFile(QObject *parent = NULL);
    /// \brief Constructor
    /// \param fileName : working file
    DatakeysFile(const QString &fileName);
    /// \brief Copy Constructor
    DatakeysFile(const DatakeysFile& other);
    /// \brief Destructor
    ~DatakeysFile();
    /// \brief Overload operator=
    DatakeysFile& operator=(const DatakeysFile& other);
    /// \brief Read the file and load data members
    bool            Read(QList<DatakeysError> &lErrors);
    /// \brief Write the file by using data members
    bool            Write(DatakeysError &lError);
    /// \return the numbers of static keys
    int             CountStaticKeys() const;
    /// \return the numbers of dyn keys
    int             CountDynamicKeys() const;
    /// \brief set the static key linked to the flow_id
    bool            GetStaticKeysAt(DatakeysInfo &key, int flowId) const;
    /// \brief set the dyn key linked to the flow_id
    bool            GetDynamicKeysAt(DatakeysInfo &key, int flowId) const;
    /// \brief Returns the db keys file name
    QString         GetFileName() const;
    /// \brief insert the key at the flow id
    /// if not in range at the end
    void            InsertStaticKey(const DatakeysInfo &key, int flowId = -1);
    /// \brief insert the key at the flow id
    /// if not in range at the end
    void            InsertStaticKey(const QString &name,
                                    const QString &expression,
                                    int flowId = -1);
    void            InsertDynamicKey(const DatakeysInfo &key, int flowId = -1);
    /// \brief insert the key at the flow id
    /// if not in range at the end
    void            InsertDynamicKey(const QString &name,
                                    const QString &expression,
                                    int flowId = -1);
    /// \brief remove the key at the flow id
    bool            RemoveStaticKeyAt(int flowId);
    /// \brief remove the key at the flow id
    bool            RemoveDynamicKeyAt(int flowId);
private:
    DatakeysFilePrivate *mPrivate; /*!< pivate members*/

};

} //END namespace QtLib
} //END namespace GS

#endif // GQTL_DATAKEYS_FILE_H
