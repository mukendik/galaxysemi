#ifndef JSBINPRECEDENCE_H
#define JSBINPRECEDENCE_H

#include <QObject>

class JSBinDescription : public QObject
{
    Q_OBJECT

public:

    JSBinDescription(QObject* parent = NULL);
    JSBinDescription(const JSBinDescription& other);
    ~JSBinDescription();

    Q_INVOKABLE int GetNumber() const;
    Q_INVOKABLE QString GetName() const;
    Q_INVOKABLE QString GetCategory() const;

    Q_INVOKABLE void SetNumber(int lNumber);
    Q_INVOKABLE void SetName(const QString& lName);
    Q_INVOKABLE void SetCategory(const QString& lCategory);

    JSBinDescription& operator=(const JSBinDescription& other);

private:

    int     mNumber;
    QString mName;
    QString mCategory;
};

class JSWaferMap : public QObject
{
    Q_OBJECT

public:

    JSWaferMap(QObject* parent = NULL);
    JSWaferMap(const JSWaferMap& other);
    ~JSWaferMap();

    Q_INVOKABLE bool    Create(int lLowX, int lLowY, int lHighX, int lHighY);

    Q_INVOKABLE int     GetLowX() const;
    Q_INVOKABLE int     GetLowY() const;
    Q_INVOKABLE int     GetHighX() const;
    Q_INVOKABLE int     GetHighY() const;
    Q_INVOKABLE int     GetSizeX() const;
    Q_INVOKABLE int     GetSizeY() const;
    Q_INVOKABLE int     GetBinAt(int lX, int lY) const;
    Q_INVOKABLE void    SetBinAt(int lBin, int lX, int lY);

    bool                isValidCoord(int nDieX, int nDieY) const;
    bool                isValidXCoord(int nDieX) const;
    bool                isValidYCoord(int nDieY) const;
    bool                indexFromCoord(int &nIndex, int nDieX, int nDieY) const;

private:

    int     mLowX;
    int     mLowY;
    int     mHighX;
    int     mHighY;
    int *   mArray;
};

//class JSBinPrecedence: public QObject
//{
//    Q_OBJECT

//public:

//    JSBinPrecedence(QObject* parent = NULL);
//    ~JSBinPrecedence();

//    Q_INVOKABLE JSBinDescription ComputePrecedence(JSBinDescription lFromMap, JSBinDescription lFromSTDF);
//};

#endif // JSBINPRECEDENCE_H
