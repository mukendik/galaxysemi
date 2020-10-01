/****************************************************************************************/
/* Copyright Galaxy                                                                     */
/* This computer program is protected by copyright law                                  */
/* and international treaties. Unauthorized reproduction or                             */
/* distribution of this program, or any portion of it,may                               */
/* result in severe civil and criminal penalties, and will be                           */
/* prosecuted to the maximum extent possible under the law.                             */
/****************************************************************************************/
/*                                                                                      */
/* Module: gtc_constants.h                                                              */
/*                                                                                      */
/* This file has global definitions used throughout all GTL, GTM, GTS modules.          */
/*                                                                                      */
/****************************************************************************************/
/*                                                                                      */
/* Notes:                                                                               */
/*                                                                                      */
/****************************************************************************************/
/*                                                                                      */
/* Revision history:                                                                    */
/*                                                                                      */
/* o [BG] 01 June 2005: Created.                                                         */
/*                                                                                      */
/****************************************************************************************/

#ifndef _GTC_CONSTANTS_H_
#define _GTC_CONSTANTS_H_

/*======================================================================================*/
/* Constants                                                                            */
/*======================================================================================*/
/* In case it is not already defined */
#ifndef MAXHOSTNAMELEN
    #define MAXHOSTNAMELEN					256
#endif

/* Status codes */
#define	GTC_STATUS_OK					0x0000	/* OK */
#define	GTC_STATUS_CRCF					0x0001	/* Couldn't read config file */
#define	GTC_STATUS_LICEXPIRED			0x0002	/* License expired */
#define	GTC_STATUS_MAXCONNECTIONS		0x0003	/* Max number of connections reached */
#define	GTC_STATUS_SOCKET       		0x0004	/* Socket error */
#define	GTC_STATUS_VERSIONMISMATCH      0x0005	/* Version mismatch between client and server */
#define	GTC_STATUS_EPM                  0x0006	/* Error peocessing message */
#define	GTC_STATUS_NO_GTM_LIC  			0x0007	/* Couldn't checkout a valid license */
#define	GTC_STATUS_NO_GTL_LIC           0x0008	/* Couldn't checkout a valid license */


/* Running modes */
#define	GTC_GTLMODULE_PAT				0x0001	/* PAT mode */
#define	GTC_GTLMODULE_SKIPTEST			0x0002	/* SKIPTEST mode */

/* GTM->GTL commands */
#define GTC_GTLCOMMAND_NONE				0		/* No command */
#define GTC_GTLCOMMAND_ASCII			1		/* Command in ascii string (string must be interpreted) */
#define GTC_GTLCOMMAND_RESTARTBASELINE	2		/* Restart baseline */
#define GTC_GTLCOMMAND_RESET			3		/* Reset GTL (into a state as if test program just loaded...) */

/* Option specifying the key to use to find a test in the TestList
    When server returns init, in the recipe, there is a key (TestKey) in order to select how to identify a test.
*/
#define GTC_FINDTEST_TESTNUMBER			0x01
#define GTC_FINDTEST_TESTNAME			0x02
#define GTC_FINDTEST_TESTNUMBERANDNAME	0x03

/* STRING LENGTHS */
#define GTC_MAX_STRING					256
#define GTC_MAX_VERSION_LEN 			100
#define GTC_MAX_LOTID_LEN				256
#define GTC_MAX_SLOTID_LEN				256
#define	GTC_MAX_PRODUCTID_LEN			256
#define GTC_MAX_SRVNAME_LEN 			64 // the domain name could be even more than that... exple: galaxysemi-licensemanager-mpoc.maximintegrated.com
#define GTC_MAX_IPADDR_LEN				20
#define GTC_MAX_PORT_LEN				10
#define GTC_MAX_USERNAME_LEN			64
#define GTC_MAX_OPERATORNAME_LEN		64
#define GTC_MAX_PASSWD_LEN				64
#define GTC_MAX_TESTNAME_LEN			256
#define GTC_MAX_TESTUNITS_LEN			10
#define GTC_MAX_PROJECTNAME_LEN			256
#define GTC_MAX_JOBNAME_LEN				256
#define GTC_MAX_JOBREV_LEN				20
#define GTC_MAX_NODENAME_LEN			256
#define GTC_MAX_HOSTID_LEN				20
#define GTC_MAX_LIBVER_LEN				20
#define GTC_MAX_TESTERTYPE_LEN			256
#define GTC_MAX_TESTEREXEC_LEN			256
#define GTC_MAX_ROOTCAUSE_LEN			128
#define GTC_MAX_TESTERMESSAGE			512
#define GTC_MAX_TESTERCOMMAND			512
#define GTC_MAX_STRINGTOSTDF			1024
#define GTC_MAX_PARTID_LEN				128
#define GTC_MAX_DISTRIBUTIONSHAPE_LEN   50
#define GTC_MAX_EVENT_TYPE              50
#define GTC_MAX_EVENT_SUBTYPE           50
#define GTC_MAX_MESSAGE_LEN             1024
#define GTC_MAX_QUERY_LEN               102400

#define GTC_MAX_PATH					512     /* max. length of full pathname */
#define GTC_MAX_DRIVE					3       /* max. length of drive component */
#define GTC_MAX_DIR						256     /* max. length of path component */
#define GTC_MAX_FNAME					256     /* max. length of file name component */
#define GTC_MAX_EXT						256     /* max. length of extension component */

/* Test result flags
    Default value is 0 (invalid). Flag is updated in gtl_tl_CheckTestResults(...).
*/
#define GTC_TRFLAG_VALID				0x01	/* Test result is valid  */
#define GTC_TRFLAG_SPAT_OUTLIER			0x02	/* Test result is a SPAT outlier  */
#define GTC_TRFLAG_DPAT_OUTLIER			0x04	/* Test result is a DPAT outlier  */

/* Part result flags */
#define GTC_PRFLAG_FAIL 				0x01	/* Part failed  */

#endif /* #ifndef _GTC_CONSTANTS_H_ */

