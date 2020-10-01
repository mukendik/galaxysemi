#ifndef DB_TABLE_LINK_H
#define DB_TABLE_LINK_H

#include <QObject>
#include <QMap>
#include <QDomElement>

namespace GS
{
namespace DbPluginBase
{

class DbTableLink : public QObject
{
    Q_OBJECT
public:
    /// \struct store key value and type
    struct Key
    {
        enum KeyType
        {
            FIELD,
            STRING
        };
        inline Key() {mType = Key::FIELD;}
        inline Key(const QString &keyValue, const KeyType &keyType = Key::FIELD)
            : mValue(keyValue), mType(keyType){}
        bool operator==(const Key &source) const
        {
            if (mValue != source.mValue)
                return false;
            if (mType != source.mType)
                return false;
            return true;
        }
        bool operator!=(const Key &source) const
        {
            return !((*this) == source);
        }
        QString mValue;///< holds key value
        KeyType mType; ///< holds key type
    };

    /// \struct store keys pair between table 1 and table 2
    struct KeysLink
    {
        inline KeysLink() {}
        inline KeysLink(const Key &key1, const Key &key2)
            : mTable1Key(key1), mTable2Key(key2){}
        static bool IsEqual(const KeysLink &link1, const KeysLink &link2, bool isSymetric)
        {
            if (isSymetric)
                return ((link1.mTable1Key == link2.mTable1Key) && (link1.mTable2Key == link2.mTable2Key));
            else
                return ((link1.mTable1Key == link2.mTable2Key) && (link1.mTable2Key == link2.mTable1Key));
        }

        Key mTable1Key; ///< holds key from table 1
        Key mTable2Key; ///< holds key from table 2
    };
    /// \brief Constructor
    DbTableLink(QObject *parent = 0);
    DbTableLink(const QString &id, QObject *parent = 0);
    DbTableLink(const QString &table1,
                const QString &table2,
                const QList<KeysLink> &keysLinks,
                QObject *parent = 0);
    /// \brief Destructor
    virtual ~DbTableLink();
    /// \brief copy constructor
    DbTableLink(const DbTableLink &source);
    /// \brief define = operator
    DbTableLink & operator=(const DbTableLink &source);
    /// \brief define = operator
    bool operator==(const DbTableLink &source) const;
    bool operator!=(const DbTableLink &source) const;

    /// \brief load table link from Dom element
    bool                            LoadFromDom(const QDomElement &node);
    /// \brief load keylink from Dom element
    bool                            LoadKeyLinksFromDom(const QDomElement &node);
    /// \brief set table link id
    void                            SetId(const QString &id);
    /// \brief link id
    const QString&                  Id() const;
    /// \brief name of table 1
    const QString&                  Table1() const;
    /// \brief name of table 2
    const QString&                  Table2() const;
    /// \brief list of normalized table links ex: "table1.fieldx|table2.fieldx"
    QStringList                     NormalizedTablesLinks() const;
    /// \brief return the clause on fields
    QString                         Conditions() const;
    
private:
    /// \brief clear key link list
    void            ClearTableLinkElt();
    /// \brief check if node exist and log otherwise
    bool            NodeIsValid(const QDomElement &node) const;

    QString         mId;            ///< holds the link id
    QString         mTable1;        ///< holds name of table 1
    QString         mTable2;        ///< holds name of table 2
    QList<KeysLink>  mKeysLinks;     ///< holds list of keys pair between table 1 and table 2
};

} //END namespace DbPluginBase
} //END namespace GS

#endif // DB_TABLE_LINK_H
