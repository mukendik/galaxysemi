/****************************************************************************************/
/* Copyright Galaxy                                                                     */
/* This computer program is protected by copyright law                                  */
/* and international treaties. Unauthorized reproduction or                             */
/* distribution of this program, or any portion of it,may                               */
/* result in severe civil and criminal penalties, and will be                           */
/* prosecuted to the maximum extent possible under the law.                             */
/****************************************************************************************/
/*                                                                                      */
/* Module: gtl_core_image.h                                                             */
/*                                                                                      */
/* Exports of the gtl_core library                                                      */
/*                                                                                      */
/****************************************************************************************/
/*                                                                                      */
/* Notes:                                                                               */
/*                                                                                      */
/* This header file contains the interface to the GTL (Galaxy Tester Library) core      */
/* library:                                                                             */
/*                                                                                      */
/* Windows        : gtl_core.lib or gtl_core.a                                          */
/* Solaris/Linux  : gtl_core.a                                                          */
/*                                                                                      */
/****************************************************************************************/

#ifndef _GTL_CORE_IMAGE_H_
#define _GTL_CORE_IMAGE_H_

/*======================================================================================*/
/* MACROS                                                                               */
/*======================================================================================*/

#define GTL_TEST(x)			{	if(gtl_nDisabled)	test x;\
								else\
								{\
									gtl_test_lib(read_current_site(), tl_read_datnum(), "", x);\
									test x;\
								}\
							}

#define GTL_TEST_NAME(x, y)	{	if(gtl_nDisabled)	test x;\
								else\
								{\
									gtl_test_lib(read_current_site(), tl_read_datnum(), y, x);\
									test x;\
								}\
							}

#define GTL_BEGINJOB(x, y)	gtl_beginjob_lib(x, y)

#define GTL_ENDJOB()		gtl_endjob_lib()

#define GTL_ENDLOT()		gtl_endlot_lib()

#define GTL_CLOSE()			gtl_close_lib()

#define GTL_RESET()			gtl_reset_lib()

#define GTL_SORT()			{	if(gtl_nDisabled)	sort bin;\
								else\
								{\
									set site_enable_all;\
									serial\
									{\
										sort bin = gtl_binning_lib(read_current_site(), tl_get_seq_bin());\
									}\
								}\
							}

#define GTL_SORT_NB(x)		{	if(gtl_nDisabled)	sort bin = x;\
								else\
								{\
									sort bin = gtl_binning_lib(read_current_site(), x);\
								}\
							}

#include "gtl_core.h"

#endif /* #ifndef _GTL_CORE_IMAGE_H_ */

