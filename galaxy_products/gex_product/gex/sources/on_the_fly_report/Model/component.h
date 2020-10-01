#ifndef COMPONENT_H
#define COMPONENT_H
#include <QJsonObject>
//#include "composite.h"
#include <QList>

#define MIN_OFR_JSON_FORMAT 1.00f
#define MAX_OFR_JSON_FORMAT 1.00f

class CTest;

    /**
    \brief This class is used to save the CTest in minimum format and use it more easelly
    */
class Test
{

public:
    Test(): mGroupId(0), mFileIndex(0), mPinIndex("-1") {}

    Test(const QString& number, const QString& name, const QString& pinIndex, int groupId, int fileIndex):
        mNumber (number),
        mName   (name),
        mPinIndex(pinIndex),
        mGroupId(groupId),
        mFileIndex(fileIndex)
    {}

    /**
     * \fn QJsonObject toJson  ()
     * \brief this function dumps the current object in a Qjson Object and retun it.
     * \return the dumped QJson object.
     */
    QJsonObject toJson  ();

    /**
        \brief serialize the state of the object into the json format
     */
    void        fromJson(const QJsonObject& jsonDescription);


    /**
     * @brief number of the test associated with the component
     */
    QString mNumber;

    /**
     * @brief name of the test associated with the component
     */
    QString mName;

    /**
     * @brief pin index of the test associated with the component
     */
    QString mPinIndex;

    /**
     * @brief group index of the test associated with the component
     */
    int     mGroupId;

    /**
     * @brief file index of the test associated with the component
     */
    int     mFileIndex;
};

class Group
{
public:
    Group() {}
    Group(const QString& number, const QString& name):
        mNumber(number),
        mName(name)
    {}

    /**
     * \fn QJsonObject toJson  ()
     * \brief this function dumps the current object in a Qjson Object and retun it.
     * \return the dumped QJson object.
     */
    QJsonObject toJson  ();

    /**
        \brief serialize the state of the object into the json format
     */
    void fromJson(const QJsonObject& jsonDescription);


    /**
     * @brief number of the group associated with the component
     */
    QString mNumber;

    /**
     * @brief name of the group associated with the component
     */
    QString mName;
};


enum T_Component { T_ROOT= 0,
                   T_SECTION,
                   T_HISTO ,
                   T_TREND,
                   T_BOXPLOT,
                   T_PROBA,
                   T_WAFER,
                   T_TABLE,
                   T_HISTO_CONNECTED ,
                   T_TREND_CONNECTED,
                   T_BOXPLOT_CONNECTED,
                   T_PROBA_CONNECTED,
                   T_WAFER_CONNECTED,
                   T_CAPABILITY_TABLE_CONNECTED,
                   T_NONE};


T_Component GetConnectedType(T_Component type);

T_Component GetUnConnectedType(T_Component type);


/**
    \brief Base class of the model that will keep in memory the report
 */
class Component
{
public:
    static float          mActualVersion;
    static int            mImageIncrement;
    Component(const QString &name, const QJsonObject& jsonDescription, Component* parent, T_Component type);

    virtual ~Component();

    /**
        \brief clear the state of the object. Nothing is done in the base class.
        To be implement if needed
     */
    virtual void Clear() {}

    /**
     * \fn bool DrawSection(QString &imagePath, CTest* sectionTest = NULL);
     * \brief this function draws the chart image in the image path given in parameter.
     * \param imagePath the path of the created image
     * \param sectionTest the test comming from the section if exists
     * \return true if the draw has been done with success. Otherwise return false.
     */
    virtual bool            DrawSection( QString &imagePath, int imageSize, CTest* sectionTest = NULL) = 0;

    /**
     * @brief EraseElement - erase child if any
     */
    virtual void            EraseChild(Component *){}

    /**
     * @brief ToJson -  serialized the object to a json object
     * @param jsonObj
     * @return
     */
    virtual QJsonObject      ToJson() =0;

    virtual const QList<Component*> & GetElements() const;


    ///\brief getter/setter
    const QString &         GetName             () const;
    void                    SetName             (const QString& name);

    QJsonObject             GetJsonDescription  () const;

    const QString&          GetComment          () const;
    void                    SetComment          (const QString& comment);

    T_Component             GetType             () const;

    Component*              Parent              () const;
    /**
     * @brief UpdateJson - Will be called during the ToJson function process.
     * The default implementation do nothing. To be reimplemented in the derivate Component object
     * that can be updated through the GUI
     *
     */
    virtual void            UpdateJson         () {}

    /**
     * \brief Debug - This function dumps the object into a file
     */
    void Debug(const QJsonObject& object);

    /**
     * @brief index of the component in the tree representation
     */
    int                 mIndexPosition;
protected:

    /**
        \brief the type T_Component of the component
     */
    T_Component         mType;

    /**
     * @brief name of the component that will be displayed
     */
    QString             mName;

    /**
     * @brief the comment associated with the component
     */
    QString             mComment;

    /**
     * @brief pointeur to the component parent if any
     */
    Component           *mComponentParent;

    /**
     * @brief the json format of the component
     */
    QJsonObject         mJsonDescription;

    /**
     * @brief enable the function reference return of Componenet list when empty
     */
    QList<Component*>   mEmptyList;
};


/**
 * @brief functor used to sort the component by the index position in the tree
 * representation
 */
struct SortComponent
{
    bool operator()(Component* compo1, Component* compo2)
    {
        return compo1->mIndexPosition < compo2->mIndexPosition;
    }
};
#endif // COMPONENT_H
