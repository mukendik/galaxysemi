#include "QJsonDocument"
#include "QFile"
#include "component.h"

float Component::mActualVersion = 1.0;
int   Component::mImageIncrement = 1;


T_Component GetConnectedType(T_Component type)
{
    switch(type)
    {
        case T_HISTO:               return T_HISTO_CONNECTED;
        case T_TREND:               return T_TREND_CONNECTED;
        case T_BOXPLOT:             return T_BOXPLOT_CONNECTED;
        case T_PROBA :              return T_PROBA_CONNECTED;
        case T_WAFER:               return T_WAFER_CONNECTED;
        case T_TABLE:    return T_CAPABILITY_TABLE_CONNECTED;
        default: return type;
    }
}

T_Component GetUnConnectedType(T_Component type)
{

    switch(type)
    {
        case T_HISTO_CONNECTED:             return T_HISTO;
        case T_TREND_CONNECTED:             return T_TREND;
        case T_BOXPLOT_CONNECTED:           return T_BOXPLOT;
        case T_PROBA_CONNECTED :            return T_PROBA;
        case T_WAFER_CONNECTED:             return T_WAFER;
        case T_CAPABILITY_TABLE_CONNECTED:  return T_TABLE;
        default: return type;
    }
}

QJsonObject Test::toJson  ()
{
    QJsonObject lJsonTest;
    lJsonTest.insert("Number",  mNumber);
    lJsonTest.insert("Name",    mName);
    lJsonTest.insert("PinIndex", mPinIndex);
    lJsonTest.insert("GroupID", mGroupId);
    lJsonTest.insert("FileIndex", mFileIndex);

    return lJsonTest;
}

void  Test::fromJson(const QJsonObject& jsonDescription)
{
    mNumber     = jsonDescription["Number"].toString();
    mName       = jsonDescription["Name"].toString();
    mPinIndex   = jsonDescription["PinIndex"].toString();
    mGroupId    = jsonDescription["GroupID"].toInt();
    mFileIndex  = jsonDescription["FileIndex"].toInt();
}

QJsonObject Group::toJson()
{
    QJsonObject lJsonGroup;
    lJsonGroup.insert("Number",  mNumber);
    lJsonGroup.insert("Name",    mName);

    return lJsonGroup;
}

void Group::fromJson(const QJsonObject &jsonDescription)
{
    mNumber     = jsonDescription["Number"].toString();
    mName       = jsonDescription["Name"].toString();
}

Component::Component(const QString &name, const QJsonObject &jsonDescription,Component* parent, T_Component type): mType(type)
{
    mName               = name;
    mJsonDescription    = jsonDescription;
    mComponentParent   = parent;
}

Component::~Component()
{
}

const QList<Component *> &Component::GetElements() const
{
    return mEmptyList;
}

const QString& Component::GetName() const
{
    return mName;
}

void Component::SetName(const QString& name)
{
    mName = name;
}

QJsonObject Component::GetJsonDescription() const
{
    return mJsonDescription;
}

const QString &Component::GetComment() const
{
    return mComment;
}

void Component::SetComment(const QString &comment)
{
    mComment = comment;
}

T_Component Component::GetType() const
{
    return mType;
}

Component *Component::Parent() const
{
    return mComponentParent;
}

// -- Use for Debug content only
void Component::Debug(const QJsonObject& object)
{
    QFile lFile("/Users/arnaudtouchard/Developpement/temp/jsonContent.txt");
    if (lFile.open(QIODevice::WriteOnly) == false)
        return;
    // convert the json object to a document and write it
    QJsonDocument lDocument;
    lDocument.setObject(object);
    QByteArray lObjectSerializition = lDocument.toJson();
    lFile.write(lObjectSerializition);
    lFile.close();
}

