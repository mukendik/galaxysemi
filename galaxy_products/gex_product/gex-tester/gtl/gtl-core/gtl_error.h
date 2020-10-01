/****************************************************************************************/
/* Copyright Galaxy                                                                     */
/* This computer program is protected by copyright law                                  */
/* and international treaties. Unauthorized reproduction or                             */
/* distribution of this program, or any portion of it,may                               */
/* result in severe civil and criminal penalties, and will be                           */
/* prosecuted to the maximum extent possible under the law.                             */
/****************************************************************************************/

// This module checks all error codes and dipslays corresponding error message

#ifndef _GTL_ERROR_H_
#define _GTL_ERROR_H_

/* Module identificators (for gtl_err_Display() function) */
#define GTL_MODULE_MAIN			0	/* gtl_main.c */
#define GTL_MODULE_SOCKET		1	/* gtl_socket.c */
#define GTL_MODULE_GNET			2	/* gnetbuffer.c gnetmessage.c */

/*======================================================================================*/
/* PUBLIC Functions                                                                     */
/*======================================================================================*/
#ifdef _GTL_ERROR_MODULE_

void			gtl_err_Display();
void			gtl_err_Retrieve();

#else

extern void		gtl_err_Display();
extern void		gtl_err_Retrieve();

#endif /* #ifdef _GTL_ERROR_MODULE_ */

#endif /* #ifndef _GTL_ERROR_H_ */

