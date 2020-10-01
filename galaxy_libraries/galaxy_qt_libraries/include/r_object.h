#ifndef R_OBJECT_H
#define R_OBJECT_H

/*! \class RObject
 * \brief
 *
 */

#include <QString>
#include <QStringList>

namespace GS
{
namespace SE
{

class GSexp;
class RObjectPrivate;

class RObject
{
public:
    /// \brief Destructor
    virtual ~RObject();
    /// \brief return name of object in R mem
    QString GetName() const ;
    /// \brief return SEXP ptr to R object in memory
    GSexp* GetSexp() const ;
    /// \brief check if is linked to a R object
    bool IsNull() const ;
    /// \brief protect the R object from the R garbage collector
    bool Protect(const QString &name);
    /// \brief return last error
    QString GetLastError() const;
    /// \brief return errors list
    QStringList GetErrors() const;
    /// \brief clean error stack
    void ClearErrors();
    /// \brief true if object is protected
    bool IsProtected() const;

protected:
    RObject();
    /// \brief Constructor
    RObject(RObjectPrivate & lPrivateData);
    /// \brief Constructor
    RObject(RObjectPrivate & lPrivateData, GSexp* sexp);
    /// \brief clean R memory
    void CleanRMemory();
    /// \brief store ref in R mem
    bool Install(const QString& name);

    RObjectPrivate* mPrivate; ///< ptr to private members

private:
    Q_DECLARE_PRIVATE_D(mPrivate, RObject)
};

} // namespace SE
} // namespace GS

#endif // R_OBJECT_H
