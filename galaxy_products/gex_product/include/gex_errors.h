/******************************************************************************!
 * \file gex_errors.h
 ******************************************************************************/
#ifndef GEX_ERRORS
#define GEX_ERRORS

namespace GS
{
    namespace Error
    {
        // 0 = EXIT_SUCCESS
        // 1 = EXIT_FAILURE
        const int CLIENT_TRY_CONNECT_TIMEOUT  = 2;
        const int APPLICATION_ALREADY_RUNNING = 3;
        const int SERVER_CONNECTION_CLOSE     = 4;
        const int NETWORK_TIMEOUT             = 5;
        const int SOFTWARE_IDLE               = 6;
        const int ALL_LICENSES_USED           = 7;
        const int CANNOT_GET_LICENSE          = 8;
        const int USER_LICENSE_RELEASE        = 9;
        const int CANNOT_START_WEBSERVICE     = -99;
    }
}

#endif
