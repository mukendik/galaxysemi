#ifndef _HTTP_CHANNEL_H_
#define _HTTP_CHANNEL_H_

#include <string>
#include <QObject>
#include <QNetworkReply>
#include <QTcpSocket>

class QNetworkAccessManager;

namespace GS
{
namespace Gex
{

template< class >
    class DumpableAs;

/**
 * @brief The HttpChannel class expose a basic feature allowing the user to post
 * data in a stringified JSON format. Coupled with Qt capabilities to allow the
 * user to connect its own slots with exposed signals
 */
class HttpChannel :
    public QObject
{
    Q_OBJECT

public :
    /**
     * @brief HttpChannel construction
     * @param address address of the listening service
     * @param port port used by the listening service
     * @param route path to the service API
     */
    HttpChannel( const std::string &address,
                 unsigned short port,
                 const std::string &route,
                 QObject *parent = NULL );

    /**
     * @brief free some resources when relevant
     */
    virtual ~HttpChannel();

    /**
     * @brief GetAddress basic accessor
     * @return the address
     */
    const std::string &GetAddress() const { return m_address; }

    /**
     * @brief SetAddress basic mutator
     * @param address value to modify the inner state of this instance
     */
    void SetAddress( const std::string &address ) { m_address = address; fix_address_and_host(); }

    /**
     * @brief GetPort basic accessor
     * @return the port set
     */
    unsigned short GetPort() const { return m_port; }

    /**
     * @brief SetPort basic mutator
     * @param port the value to modify the related inner state with
     */
    void SetPort( unsigned short port ) { m_port = port; }

    /**
     * @brief GetRoute basic accessor
     * @return the route set
     */
    const std::string &GetRoute() const { return m_route; }

    /**
     * @brief SetRoute basic mutator
     * @param route the value to modify the related inner state with
     */
    void SetRoute( const std::string &route ) { m_route = route; fix_route(); }

    /**
     * @brief GetHost basic accessor
     * @return the host set
     */
    const std::string & GetHost() const { return m_host; }

    /**
     * @brief PostData transfers a dumpable object that can be represented by a
     * string throughout the network until it reaches a listening service
     * @param data sumpable as string object
     */
    void PostData( const DumpableAs< const std::string > &data );

    void GetData();

signals :
    /**
     * @brief postFinished is a signal indicating data has been successfully
     * posted
     */
    void postFinished(QByteArray response);

    /**
     * @brief postError is a dignal emitted if an error has occured during a post
     *
     * @param error the network error that occured
     */
    void postError( QNetworkReply::NetworkError error );

    /**
     * @brief getFinished is a signal indicating data has been successfully
     * gotten
     */
    void getFinished(QByteArray response);

    /**
     * @brief getError is a dignal emitted if an error has occured during a get
     *
     * @param error the network error that occured
     */
    void getError( QNetworkReply::NetworkError error );

    /**
     * @brief connected emitted when successfully connected to the host
     */
    void connected();

    /**
     * @brief connectionError emitted when a connection error occurs
     * @param error the type of the error
     */
    void connectionError( QTcpSocket::SocketError error );

private :
    /**
     * @brief m_socket underlying socket, establishing a connection to test host and port reliability
     */
    QTcpSocket *m_socket;

    /**
     * @brief m_output_reply is a reply to use when dealing with posting some data
     */
    QNetworkReply *m_output_reply;

    /**
     * @brief m_host is the determined host to check the network connection
     */
    std::string m_host;

    /**
     * @brief address of the service
     */
    std::string m_address;

    /**
     * @brief port the sevice uses to listen
     */
    unsigned short m_port;

    /**
     * @brief virtual path to access job API of the listening service
     */
    std::string m_route;

    /**
     * @brief data to post through this http channel.
     */
    DumpableAs< const std::string > *m_data;

    /**
     * @brief set_data is an internal setter setting data to be posted, freeing previous data used
     * @param data the data to psot and store here
     */
    void set_data(const DumpableAs<const std::string> &data );

    /**
     * @brief internal method used to fix the address at construction, inserting
     * "http://" at the beginning of the address if needed
     */
    void fix_address_and_host();

    /**
     * @brief internal method used to fix the specified route, inserting '/' at
     * the beginning if needed
     */
    void fix_route();

private slots :
    /**
     * @brief private slot on finished post, forwarding to exposed signal
     */
    void onReplyPostFinished();

    /**
     * @brief onPostError private slot forwarding a signal indicating the post was a fail
     *
     * @param error the network error that occured
     */
    void onReplyPostError( QNetworkReply::NetworkError error );

    /**
     * @brief onPostData reacts on successfully connected socket
     */
    void onPostData();

    void onReplyGetFinished();

    void onReplyGetError(QNetworkReply::NetworkError error);

    void onGetData();

    /**
     * @brief onConnectionError reacts on a socket connection fail
     * @param error the error that occured
     */
    void onConnectionError( QAbstractSocket::SocketError error );
};

} // namespace Gex
} // namespace GS

#endif // _HTTP_CHANNEL_H_
