#include "gtl_core.h"
#include <mongoose.h>

struct mg_callbacks callbacks;
static struct mg_context *sMgCtx=0;

// This function will be called by mongoose on every new request.
static int begin_request_handler(struct mg_connection *conn)
{
  const struct mg_request_info *request_info = mg_get_request_info(conn);
  char content[100];

  // Prepare the message we're going to send
  int content_length = snprintf(content, sizeof(content),
                                "Hello from mongoose! Remote port: %d",
                                request_info->remote_port);

  // Send HTTP reply to the client
  mg_printf(conn,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: %d\r\n"        // Always set Content-Length
            "\r\n"
            "%s",
            content_length, content);

  // Returning non-zero tells mongoose that our function has replied to
  // the client, and mongoose should not send client any more data.
  return 1;
}

int gtl_init_http_server(unsigned lPort)
{
    if (sMgCtx)
    {
        GTL_LOG(3, "MgCtx already initialized: %p", sMgCtx);
        return 0;
    }

//#ifdef GTLDEBUG
    // Todo : write the port number param in the second element of this options table
    char *options[] = {
      "listening_ports", "8080",
      "document_root", ".", //"websocket_html_root",
      NULL
    };

    char lPortStr[256]="";
    sprintf(lPortStr, "%d", lPort);
    //sprintf(options[1], options, lPortStr);

    // Prepare callbacks structure. We have only one callback, the rest are NULL.
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.begin_request = begin_request_handler;

    // struct mg_context *
    sMgCtx = mg_start(&callbacks, NULL, (const char**)options);
    if (!sMgCtx)
      return -1;
//#else
    //printf("gtl_init_http_server Port %d", lPort);
    lPort=0;
//#endif
    return 0;
}

