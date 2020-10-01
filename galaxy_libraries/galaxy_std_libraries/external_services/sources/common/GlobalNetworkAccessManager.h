#ifndef _GLOBALNETWORKACCESSMANAGER_H_
#define _GLOBALNETWORKACCESSMANAGER_H_

#include <QNetworkAccessManager>

namespace GS
{
namespace Gex
{

/**
 * @brief Singleton designed to hold a QNetworkAccessManager instance. Necessary
 * to give this underlying network access manager enough lifetime in order to
 * process HTTP request asynchroneously
 */
class GlobalNetworkAccessManager
{
    // singleton, no copy
    Q_DISABLE_COPY( GlobalNetworkAccessManager )

public :
    /**
     * @brief used to delete the underlying network access manager instance
     */
    ~GlobalNetworkAccessManager();

    /**
     * @brief get unique instance
     * @return the unique global network access manager instance
     */
    static GlobalNetworkAccessManager * GetInstance();

    /**
     * @brief accessor on the underlying network access manager
     * @return a reference on an instance of QNetworkAccessManager
     */
    QNetworkAccessManager & GetNetworkAccessManager()
    { return *m_network_access_manager; }

    /**
     * @brief accessor on the underlying network access manager
     * @return a reference on a constant instance of QNetworkAccessManager
     */
    const QNetworkAccessManager & GetNetworkAccessManager() const
    { return *m_network_access_manager; }

    /**
     * @brief Allow to reset the underlying network access manager.
     * Preempted usefulness, not yet used
     */
    void ResetNetworkConnectionManager();

private :
    /**
     * @brief underlying network access manager, initialized at construction and
     * at reset
     */
    QNetworkAccessManager *m_network_access_manager;

    /**
     * @brief m_instance the unique instance of this class
     */
    static GlobalNetworkAccessManager *m_instance;

    /**
     * @brief private default construction, compiler can use this to initialize
     * the gloabl instance of this singleton
     */
    GlobalNetworkAccessManager();
};

}
}

#endif // _GLOBALNETWORKACCESSMANAGER_H_
