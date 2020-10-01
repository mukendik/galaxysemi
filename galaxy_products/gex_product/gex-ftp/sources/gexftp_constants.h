/****************************************************************************
** Definitions for gex-ftp module
****************************************************************************/

#ifndef GEXFTPCONSTANTS_H
#define GEXFTPCONSTANTS_H

/////////////////////////////////////////////////////////////////////////////////////
// Key for encrypting password
/////////////////////////////////////////////////////////////////////////////////////
#define GEXFTP_CRYPTING_KEY			"gex@galaxysemi.com|gex_ftp|29-Sep-2004"

/////////////////////////////////////////////////////////////////////////////////////
// For log messages
/////////////////////////////////////////////////////////////////////////////////////
#define GEXFTP_MSG_INFO				0x0001
#define GEXFTP_MSG_WARNING			0x0002
#define GEXFTP_MSG_ERROR			0x0004
#define GEXFTP_MSG_INFO_XFER		0x0008
#define GEXFTP_MSG_INFO_SEQ			0x0010
#define GEXFTP_MSG_ALL				0x001F

/////////////////////////////////////////////////////////////////////////////////////
// FTP settings
/////////////////////////////////////////////////////////////////////////////////////
#define GEXFTP_NEW_SETTING                  "New settings..."
#define GEXFTP_DEFAULT_EXTENSIONS           "*.stdf; *.std; *.atdf; *.spd; *.gdf; *.gz; *.Z; *.zip"
#define GEXFTP_DEFAULT_PORT_NB              21
#define GEXFTP_FILEPOLICY_RENAME_TEXT       "Rename file"
#define GEXFTP_FILEPOLICY_REMOVE_TEXT       "Remove file"
#define GEXFTP_FILEPOLICY_LEAVE_TEXT        "Leave file"
#define GEXFTP_DOWNLOAD_OLDEST_TO_NEWEST    "Oldest to newest"
#define GEXFTP_DOWNLOAD_NEWEST_TO_OLDEST    "Newest to oldest"

/////////////////////////////////////////////////////////////////////////////////////
// FTP file status
/////////////////////////////////////////////////////////////////////////////////////
#define GEXFTP_FILESTATUS_ERROR				0
#define GEXFTP_FILESTATUS_TRANSFERRED		1
#define GEXFTP_FILESTATUS_TRANSFERRING		2

#endif // GEXFTPCONSTANTS_H
