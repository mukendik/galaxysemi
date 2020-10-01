#ifndef DB_KEY_DATA_H
#define DB_KEY_DATA_H

#include <QString>
#include <QObject>

namespace GS
{
namespace Gex
{
/*! \class DbKeyData
 * \brief class to store raw data of a DB key
 */
class DbKeyData: public QObject
{
    Q_OBJECT
public:
    /// \brief Constructor
    /// \param name DB key name
    /// \param value DB key original value
    /// \param flowId Db key flow ID
    DbKeyData(QString name, QString value, int flowId);
    /// \brief Copy Constructor
    DbKeyData(const DbKeyData *source);
    /// \brief Destructor
    virtual ~DbKeyData();
    /// Getters
    QString Name()          const   {return mName;}
    QString Value()         const   {return mValue;}
    QString Expression()    const   {return mExpression;}
    QString EvaluatedValue()const   {return mEvaluatedValue;}
    bool    IsValidResult() const   {return mIsValidExpression;}
    bool    IsValidName()   const   {return mIsValidName;}
    int     FlowId()        const   {return mFlowId;}
    /// Setters
    void    SetName(QString name);
    void    SetValue(QString value);
    void    SetExpression(QString expression);
    void    SetEvaluatedValue(QString evaluatedValue, bool isValidResult);
    void    SetFlowId(int flowId);
    void    SetNameIsValid(bool isValid);
    void    SetExpressionIsValid(bool isValid);

signals:
    /// \brief Data has changed
    void    DataChanged(int rowId);

private:
    Q_DISABLE_COPY(DbKeyData)
    QString mName;              ///< Name of a key
    QString mValue;             ///< Value of a key before evaluation
    QString mExpression;        ///< Expression to evaluate
    QString mEvaluatedValue;    ///< Stores the result of the evaluation
    int     mFlowId;            ///< Id in the flow
    bool    mIsValidName;       ///< true if the name is valid
    bool    mIsValidExpression; ///< true if the expression is valid
};

} // END Gex
} // END GS

#endif // DB_KEY_DATA_H
