#ifndef GTL_GETSET_H
#define GTL_GETSET_H

    #define GTL_KEY_HTTP_SERVER "http_server" // Only available in debug build.
    #define GTL_KEY_GTM_COMMUNICATION_MODE "gtm_communication_mode" // "synchronous"|"asynchronous".
    #define GTL_KEY_SOCK_CONNECT_MODE "socket_connection_mode" // "blocking"|"non-blocking"
    #define GTL_KEY_SOCK_COMM_MODE "socket_communication_mode" // "blocking"|"non-blocking"
    #define GTL_KEY_SOCK_CONNECT_TIMEOUT "socket_connection_timeout" // "<value>" in seconds
    #define GTL_KEY_SOCK_RECEIVE_TIMEOUT "socket_receive_timeout" // "<value>" in seconds
    #define GTL_KEY_SOCK_SEND_TIMEOUT "socket_send_timeout" // "<value>" in seconds

#endif // GTL_GETSET_H
