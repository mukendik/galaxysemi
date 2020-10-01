/****************************************************************************
** Class holding list of data files (transferred, or to be transferred)
****************************************************************************/

#ifndef GEXDB_PLUGIN_DATAFILE_H
#define GEXDB_PLUGIN_DATAFILE_H

// QT includes
#include <qstring.h>
#include <QList>

// Galaxy modules includes

// Local includes

// Class holding info on a data file (remote on a ftp server, or locally created by the plugin)
class GexDbPlugin_DataFile
{
public:
    GexDbPlugin_DataFile()	{   m_bRemoteFile = true;
                                m_bTransferOK = false;
                                m_uiPort = 0;
                            }

	bool			m_bRemoteFile;			// true if file is on a remote FTP server
	QString			m_strFileName;			// File name (without path)
	QString			m_strFilePath;			// Path to file (on remote Ftp server, or on local FS)
	QString			m_strHostName;			// Ftp host name
	QString			m_strUserName;			// Ftp login name
	QString			m_strPassword;			// Ftp login password
	unsigned int	m_uiPort;				// Ftp connection port#
	bool			m_bTransferOK;			// Set to true if file successfully transferred
	QString			m_strTransferError;		// Contains transfer error message if transfer NOK
};


typedef QList<GexDbPlugin_DataFile*>				tdGexDbPluginDataFileList;
typedef QListIterator<GexDbPlugin_DataFile*>		tdGexDbPluginDataFileListIterator;
typedef QMutableListIterator<GexDbPlugin_DataFile*>	tdGexDbPluginDataFileListIteratorMutable;


#endif // GEXDB_PLUGIN_DATAFILE_H
