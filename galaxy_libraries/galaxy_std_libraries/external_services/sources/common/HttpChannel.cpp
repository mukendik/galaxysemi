#include "HttpChannel.h"
#include "DumpableAs.h"
#include "GlobalNetworkAccessManager.h"

#include <algorithm>
#include <QUrl>
#include <QtNetwork>
#include <QJsonDocument>
#include <QTcpSocket>

GS::Gex::HttpChannel::HttpChannel( const std::string &address,
                                   unsigned short port,
                                   const std::string &route,
                                   QObject *parent ) :
    QObject( parent ),
    m_socket( NULL ),
    m_output_reply( NULL ),
    m_host( address ),
    m_address( address ),
    m_port( port ),
    m_route( route ),
    m_data( NULL )
{
    m_socket = new QTcpSocket( this );

    fix_address_and_host();
    fix_route();
}

GS::Gex::HttpChannel::~HttpChannel()
{
    if( m_output_reply )
        m_output_reply->deleteLater();

    if( m_socket )
    {
        m_socket->disconnect();
        m_socket->abort();
        m_socket->deleteLater();
    }
}

void GS::Gex::HttpChannel::fix_address_and_host()
{
    m_host = m_address;
    const std::string prefix( "http://" );

    std::size_t prefix_length =
        std::distance( prefix.begin(), prefix.end() );

    std::size_t address_length =
        std::distance( m_address.begin(), m_address.end() );

    // prefix bigger than address, add the prefix
    if( address_length < prefix_length )
    {
        m_address.insert( 0, "http://" );
    }
    else
    {
        typedef std::string::const_iterator const_iterator;
        typedef std::pair< const_iterator, const_iterator > mismatch_info;

        // look for mismatch between prefix and starting of m_address
        mismatch_info mismatch =
            std::mismatch( prefix.begin(), prefix.end(), m_address.begin() );

        // mismatch found before end of prefix, add it to the address
        if( mismatch.first != prefix.end() )
        {
            m_address.insert( 0, "http://" );
        }
        else
        {
            const_iterator host_begin = mismatch.second;
            const_iterator host_end = m_address.end();
            m_host = std::string( host_begin, host_end );
        }
    }
}

void GS::Gex::HttpChannel::fix_route()
{
    // a route must begin with /
    if( ( *m_route.begin() ) != '/' )
        m_route.insert( 0, "/" );
}

void GS::Gex::HttpChannel::onPostData()
{
    // create a valid url using address, port and route
    QUrl url( QString::fromStdString( m_address ) );
    url.setPort( m_port );
    url.setPath( QString::fromStdString( m_route ) );

    // access the global network access manager singleton's underlying network
    // access manager to post data
    QNetworkAccessManager &network_access_manager =
        GlobalNetworkAccessManager::GetInstance()->GetNetworkAccessManager();

    // the post request set to send json data
    QNetworkRequest request( url );

    // transform data for transmission
    const std::string dumped_data( m_data->Dump() );

    // parse the dumped data, ensuring the JSON is valid
    QJsonDocument json =
        QJsonDocument::fromJson
        ( QString::fromStdString( dumped_data ).toUtf8() );

    QByteArray post_data = json.toJson();

    request.setHeader( QNetworkRequest::ContentTypeHeader, "application/json" );
    request.setHeader( QNetworkRequest::ContentLengthHeader, post_data.size() );

    m_output_reply = network_access_manager.post( request, post_data );

    connect( m_output_reply, SIGNAL(error(QNetworkReply::NetworkError)),
             this, SLOT(onReplyPostError()) );

    connect( m_output_reply, SIGNAL(finished()),
             this, SLOT(onReplyPostFinished()) );
}

void GS::Gex::HttpChannel::onGetData()
{
    // create a valid url using address, port and route
    QUrl url( QString::fromStdString( m_address ) );
    url.setPort( m_port );
    url.setPath( QString::fromStdString( m_route ) );

    // access the global network access manager singleton's underlying network
    // access manager to post data
    QNetworkAccessManager &network_access_manager =
        GlobalNetworkAccessManager::GetInstance()->GetNetworkAccessManager();

    // the post request set to send json data
    QNetworkRequest request( url );

    m_output_reply = network_access_manager.get(request);

    connect( m_output_reply, SIGNAL(error(QNetworkReply::NetworkError)),
             this, SLOT(onReplyGetError()) );

    connect( m_output_reply, SIGNAL(readyRead()),
             this, SLOT(onReplyGetFinished()) );
}

void GS::Gex::HttpChannel::set_data( const DumpableAs<const std::string> &data )
{
    // setting data to post
    if( m_data != NULL )
    {
        delete m_data;
        m_data = NULL;
    }

    m_data = data.Clone();
}

void GS::Gex::HttpChannel::PostData( const DumpableAs<const std::string> &data )
{
    // setting internal data to clone
    set_data( data );

    // first, attempt to make a connection to host:port
    if( m_socket->state() != QAbstractSocket::ConnectedState )
    {
        // reset socket status and signal handling
        m_socket->disconnect();
        m_socket->abort();

        // connecting
        connect( m_socket, SIGNAL(connected()),
                 this, SLOT(onPostData()) );
        connect( m_socket, SIGNAL(error(QAbstractSocket::SocketError)),
                 this, SLOT(onConnectionError(QAbstractSocket::SocketError)) );
        m_socket->connectToHost( QString::fromStdString( m_host ), m_port );
    }
    else
    {
        onPostData();
    }
}

void GS::Gex::HttpChannel::onReplyPostFinished()
{
    emit postFinished(m_output_reply->readAll());
    m_output_reply->deleteLater();
}

void GS::Gex::HttpChannel::onReplyPostError(QNetworkReply::NetworkError error)
{
    emit postError( error );
    m_output_reply->deleteLater();
}

void GS::Gex::HttpChannel::GetData()
{
    // first, attempt to make a connection to host:port
    if( m_socket->state() != QAbstractSocket::ConnectedState )
    {
        // reset socket status and signal handling
        m_socket->disconnect();
        m_socket->abort();

        // connecting
        connect( m_socket, SIGNAL(connected()),
                 this, SLOT(onGetData()) );
        connect( m_socket, SIGNAL(error(QAbstractSocket::SocketError)),
                 this, SLOT(onConnectionError(QAbstractSocket::SocketError)) );
        m_socket->connectToHost( QString::fromStdString( m_host ), m_port );
    }
    else
    {
        onGetData();
    }
}

void GS::Gex::HttpChannel::onReplyGetFinished()
{
    emit getFinished(m_output_reply->readAll());
    m_output_reply->deleteLater();
}

void GS::Gex::HttpChannel::onReplyGetError(QNetworkReply::NetworkError error)
{
    emit getError(error);
    m_output_reply->deleteLater();
}

void GS::Gex::HttpChannel::onConnectionError(QAbstractSocket::SocketError error)
{
    emit connectionError( error );
}
