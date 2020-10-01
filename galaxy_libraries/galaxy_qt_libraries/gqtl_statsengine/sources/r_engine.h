#ifndef R_ENGINE_H
#define R_ENGINE_H

/*! \class REngine
 * \brief
 *
 */

#include <QStringList>

namespace GS
{
namespace SE
{

class REngine
{
public:
    /// \brief Constructor
    REngine();
    /// \brief Destructor
    ~REngine();
    /// \brief Init R things
    bool Init(const QString &appDir);
    /// \brief Source function included in the scripts
    bool SourceScripts(const QStringList &scripts);
    /// \brief return last error
    QString GetLastError();
    /// \brief return errors list
    QStringList GetErrors();
    /// \brief clean error stack
    void ClearErrors();

private:
    /// \brief return app directory
    QString GetApplicationDirectory();
    QStringList mErrors; ///< Holds errors
};

} // namespace SE
} // namespace GS

#endif // R_ENGINE_H
