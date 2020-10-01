// Htmlpng.cpp : Includes hard-coded .HTML and .png files !
//

///////////////////////////////////////////////////////////
// Class used to create Web resources in target folders.
///////////////////////////////////////////////////////////
#ifdef _WIN32
  #include <windows.h>
#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#if !defined unix && !defined __MACH__
#include <direct.h>
#include <io.h>
#endif
#include <gqtl_skin.h>
#include <gqtl_sysutils.h>
#include <QFileInfo>

#include "engine.h"
#include "browser_dialog.h"
#include "report_build.h"
#include "report_options.h"
#include "chtmlpng.h"
#include "gex_constants.h"
#include "stdf.h"
#include "product_info.h"
#include "gqtl_log.h"

// report_build.cpp
extern CReportOptions	ReportOptions;		// Holds options (report_build.h)
extern GexMainwindow *	pGexMainWindow;
extern CGexSkin*        pGexSkin;			// holds skin settings

///////////////////////////////////////////////////////////
// Constructor : reset all private variables.
///////////////////////////////////////////////////////////
CHtmlPng::CHtmlPng()
{
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CHtmlPng::~CHtmlPng()
{
}

///////////////////////////////////////////////////////////
// Create .png images in the given folder.
///////////////////////////////////////////////////////////
bool CHtmlPng::TryCreateFile(BYTE * ptFrom, const char * szTo,long lCount)
{
	FILE	*hOutputFile;
	int		iCode;
	long	lIndex=0;

	hOutputFile = fopen(szTo,"wb");
	if(hOutputFile == NULL)
		return false;	// Error

	lCount--;	// This to prevent the very last byte in structure to be
				// copied to the file: the last byte is a dummy byte.
	while(lCount--)
	{
		iCode = ptFrom[lIndex++];
		fputc(iCode,hOutputFile);
	};
	// .png or .htm, etc.. file created. Close it.
	fclose(hOutputFile);
	return true;
}

/*

//  MOVED TO GQTL

///////////////////////////////////////////////////////////
// Copy a file from one folder to another
///////////////////////////////////////////////////////////
bool toolCopyGivenFile(const QString& strFrom, const QString& strTo)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("Copy %1 to %2...").arg( strFrom).toLatin1().constData(), strTo).toLatin1().constData());.arg(    GSLOG(SYSLOG_SEV_DEBUG, "Copy %1 to %2...").arg( strFrom).toLatin1().constData().arg( strTo).toLatin1().constData());
#if defined unix || __MACH__
	size_t	lReadSize, lWriteSize;
	char 	*ptBuffer;
	QFile 	hTo(strTo);
    FILE	*hFromFile=0, *hToFile=0;

	// Erase destination if it exists.
	if(hTo.exists() == true)
	{
		// Make it writable...so we can erase it!
		chmod(strTo.toLatin1().constData(),0777);
		if(hTo.remove(strTo)!= true)
			return false;	// Failed to erase destination file.
	}

	// Allocate buffer to perform the file copy.
	ptBuffer = (char *) malloc(1000000);
	if(ptBuffer == NULL)
		return false;	// Memory allocation failure.

	hFromFile = fopen(strFrom.toLatin1().constData(), "rb");
	if(hFromFile == NULL)
	{
		delete ptBuffer;
		return false;
	}

	hToFile = fopen(strTo.toLatin1().constData(), "wb");
	if(hToFile == NULL)
	{
		delete ptBuffer;
		fclose(hFromFile);
		return false;
	}

	while(!feof(hFromFile))
	{
		lReadSize = fread(ptBuffer, sizeof(char), 1000000, hFromFile);
		if(lReadSize > 0)
		{
			lWriteSize = fwrite(ptBuffer, sizeof(char), lReadSize, hToFile);
			if(lWriteSize != lReadSize)
			{
				delete ptBuffer;
				fclose(hFromFile);
				fclose(hToFile);
				return false;
			}
		}
		else if(ferror(hFromFile))
		{
			delete ptBuffer;
			fclose(hFromFile);
			fclose(hToFile);
			return false;
		}
	}
	delete ptBuffer;
	fclose(hFromFile);
	fclose(hToFile);
	return true;
#else
	// Windows Copy (make destination writable first!).
	bool bResult;
	QDir d;
	QFile cFile;
	cFile.setPermissions(strTo,QFile::ReadOwner | QFile::WriteOwner | QFile::ReadUser | QFile::WriteUser);
	// Erase file if exists
	d.remove(strTo);
	// Copy file
	bResult = cFile.copy(strFrom,strTo);
	return bResult;
#endif
}
*/

///////////////////////////////////////////////////////////
// Create .png images in the given folder.
///////////////////////////////////////////////////////////
bool CHtmlPng::CopyGivenFile(const QString& strFrom, const QString& strTo)
{
    return CGexSystemUtils::CopyFile(strFrom,strTo);
}

///////////////////////////////////////////////////////////
// Copy contents of a given folder
///////////////////////////////////////////////////////////
bool toolCopyFolderContent(QString strFromFolder,QString strToFolder)
{
	QStringList strDataFiles;
	QStringList::Iterator it;
	QString strSrcPageFile,strDestPageFile;
	QDir cDir;

	cDir.setPath(strFromFolder);
	cDir.setFilter(QDir::Files);
	strDataFiles = cDir.entryList(QStringList("*"));	// Look for all files
	for(it = strDataFiles.begin(); it != strDataFiles.end(); ++it )
	{
		// Add any valid file to the list...
		if((*it != ".")  && (*it != ".."))
		{
			strSrcPageFile = strFromFolder + *it;
			strDestPageFile = strToFolder + *it;
            if(CGexSystemUtils::CopyFile(strSrcPageFile,strDestPageFile) == false)
				return false;	// failed copying file...
		}
	}

	return true;	// Success
}

///////////////////////////////////////////////////////////
// Create .png images in the given folder.
///////////////////////////////////////////////////////////
bool CHtmlPng::CreateWebReportImages(const char *szReportDirectory)
{
	QStringList strDataFiles;
	QStringList::Iterator it;
	QString strSource,strDestination,strSrcImageFile,strDestImageFile;
	QDir	cDir;

	// Source (<gex_application>/images/)
    strSource = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
	strSource += "/images/";

	// Create report folder directory in case it doesn't exit yet !
	strDestination = szReportDirectory;
    if (!cDir.mkdir(strDestination))
    {
        // Failing to create the folder
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Problem to create the %1...").arg(strDestination).toLatin1().constData());
    }

    QFileInfo lDirectory(strDestination);
    if (!lDirectory.exists())
        return false;

	// Create directory <stdf-file-path>/<report>/images folder
	strDestination += "/images";
	cDir.mkdir(strDestination);
	strDestination += "/";
	cDir.setPath(strSource);
	cDir.setFilter(QDir::Files);
	strDataFiles = cDir.entryList(QStringList("*.png"));	// Files extensions to look for...: *.png
	for(it = strDataFiles.begin(); it != strDataFiles.end(); ++it )
	{
		// Add any valid file to the list...
		if((*it != ".")  && (*it != ".."))
		{
			// Destination (<intranet_path>/examinator_monitoring/.images)
			strSrcImageFile = strSource + *it;
			strDestImageFile = strDestination + *it;
            if(CGexSystemUtils::CopyFile(strSrcImageFile,strDestImageFile) == false)
				return false;	// failed copiing image file...
		}
	}

	return true;	// Success
}

///////////////////////////////////////////////////////////
// Create .png images in the given folder for BIN colors
///////////////////////////////////////////////////////////
bool CHtmlPng::CreateBinImages(const char *szReportDirectory)
{
	// Do not create images unless we create HTML pages (for either HTML report, or DOC, PDF, PPT reports).
	//if((ReportOptions.iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED) == 0)
	QString of=ReportOptions.GetOption("output", "format").toString();
    if (of!="HTML"&&of!="DOC"&&of!="PDF"&&of!="PPT"&&of!="ODT")
		return true;

	// make sure destination path exists.
	QDir cDir;
	QString	strImageFile;
	QString strDestination = szReportDirectory;
	strDestination += "/images";
	cDir.mkdir(strDestination);

	// Create custom RGB images to disk.
	QPixmap	*pPixmap;
	pPixmap = new QPixmap(10,10);

	// Check all RGB images for the SOFT bin
	QList<CBinColor>::iterator itBegin = ReportOptions.softBinColorList.begin();
	QList<CBinColor>::iterator itEnd	= ReportOptions.softBinColorList.end();

	while(itBegin != itEnd)
	{
		// Fill pixmap with relevant color
		pPixmap->fill((*itBegin).cBinColor);

		// Build Image file to create
		strImageFile = strImageFile.sprintf("%s/rgb_%02x%02x%02x.png",strDestination.toLatin1().constData(),
												(*itBegin).cBinColor.red(),
												(*itBegin).cBinColor.green(),
												(*itBegin).cBinColor.blue());

		// Save pixmap image to disk
		pPixmap->save(strImageFile,"PNG");


		// Move to next Bin Color entry
		itBegin++;
	};

	// Check all RGB images for the HARD bin
	itBegin = ReportOptions.hardBinColorList.begin();
	itEnd	= ReportOptions.hardBinColorList.end();

	while(itBegin != itEnd)
	{
		// Fill pixmap with relevant color
		pPixmap->fill((*itBegin).cBinColor);

		// Build Image file to create
		strImageFile = strImageFile.sprintf("%s/rgb_%02x%02x%02x.png",strDestination.toLatin1().constData(),
			(*itBegin).cBinColor.red(),
			(*itBegin).cBinColor.green(),
			(*itBegin).cBinColor.blue());

		// Save pixmap image to disk
		pPixmap->save(strImageFile,"PNG");

		// Move to next BinColor object
		itBegin++;
	};

	// create black pixmap, used to paint all binis not assigned to any color !
	pPixmap->fill(Qt::black);

	// Build Black Image file to create
	strImageFile = strImageFile.sprintf("%s/rgb_%02x%02x%02x.png",strDestination.toLatin1().constData(),0,0,0);

	// Save pixmap image to disk
	pPixmap->save(strImageFile,"PNG");

	// Delete temporary buffer created
	delete pPixmap; pPixmap=NULL;

	return true;
}

///////////////////////////////////////////////////////////
// Create .HTML pages in the given folder.
///////////////////////////////////////////////////////////
bool CHtmlPng::CreateWebReportPages(const char *szReportDirectory)
{
	QStringList strDataFiles;
	QStringList::Iterator it;
	QString	strString,strSource,strDestination;
	QString strSrcPageFile,strDestPageFile;
	QDir cDir;

	// Source (<gex_application>/pages/)
    //strSource = pGexMainWindow->navigationSkinDir();
    strSource = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString() + pGexSkin->path();
	if(strSource.endsWith("/") == false && strSource.endsWith("\\") == false)
		strSource += "/";
	strSource += "report/pages/";

	// Create report folder directory in case it doesn't exit yet
	strString = szReportDirectory;
	cDir.mkdir(strString);

	// Create directory <stdf-file-path>/<report>/pages folder
	strDestination = QString(szReportDirectory) + QString("/pages");
	cDir.mkdir(strDestination);

	// Create directory <stdf-file-path>/<report>/drill folder
	strString = QString(szReportDirectory) + QString("/drill");
	cDir.mkdir(strString);

	QString strSrcImageFile,strDestImageFile;

	// Create directory <stdf-file-path>/<report>/images folder
	strDestination += "/";
	cDir.setPath(strSource);
	cDir.setFilter(QDir::Files);
	strDataFiles = cDir.entryList(QStringList("*.htm"));	// Files extensions to look for...: *.htm
	for(it = strDataFiles.begin(); it != strDataFiles.end(); ++it )
	{
		// Add any valid file to the list...
		if((*it != ".")  && (*it != ".."))
		{
			// Destination <stdf-file-path>/<report>/pages folder
			strSrcPageFile = strSource + *it;
			strDestPageFile = strDestination + *it;
            if(CGexSystemUtils::CopyFile(strSrcPageFile,strDestPageFile) == false)
				return false;	// failed copying HTML page file...
		}
	}

    // Copy index page depending on GEX version
    if(GS::LPPlugin::ProductInfo::getInstance()->isExaminatorTer())
    {
        // TER OEM version has a different report index page (no interactive wafermap...)
        strSrcPageFile = strSource + "index_ter.htm";
    }
    else if(GS::LPPlugin::ProductInfo::getInstance()->isExaminator())
    {
        // GEX standard version has a different report index page (no "All" link...)
        strSrcPageFile = strSource + "index.htm";

    }
    else
    {
        // GEX-PRO, GEX-PAT...
        strSrcPageFile = strSource + "index_pro.htm";
    }

    strDestPageFile = szReportDirectory;
    strDestPageFile += "/index.htm";
    if(CGexSystemUtils::CopyFile(strSrcPageFile,strDestPageFile) == false)
		return false;	// failed copying HTML page file...

	return true;	// Success
}

///////////////////////////////////////////////////////////
// Copy ExaminatorWEB application HTML folder
// used to display application messages.
///////////////////////////////////////////////////////////
bool CHtmlPng::CreateWebMessagesPages(const char * szFrom, const QString& strUserHomeFolder)
{
	QDir	cDir;
	QString	strString, strExaminatorWebApp;
	QString strFrom,strTo;

	// Get ExaminatorWeb application path.
    strExaminatorWebApp = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString() + QString(szFrom);

	// Create "$home/help" folder
	strString = strUserHomeFolder + "/help";
	cDir.mkdir(strString);

	// Create "$home/help/pages" folder
	strString = strUserHomeFolder + "/help/pages";
	cDir.mkdir(strString);

	// Copy files: <ExaminatorWebApp>/help/pages/xxxxx.htm
	strFrom = strExaminatorWebApp + GEX_HTMLPAGE_WEB_NOREPORT;
	strTo = strString + "/" + GEX_HTMLPAGE_WEB_NOREPORT;
	if(CopyGivenFile(strFrom,strTo) != true) return false;

	strFrom = strExaminatorWebApp + GEX_HTMLPAGE_WEB_EMPTYQUERY;
	strTo = strString + "/" + GEX_HTMLPAGE_WEB_EMPTYQUERY;
	if(CopyGivenFile(strFrom,strTo) != true) return false;

	strFrom = strExaminatorWebApp + GEX_HTMLPAGE_WEB_NODATABASE;
	strTo = strString + "/" + GEX_HTMLPAGE_WEB_NODATABASE;
	if(CopyGivenFile(strFrom,strTo) != true) return false;

	strFrom = strExaminatorWebApp + GEX_HTMLPAGE_WEB_NOSETTINGS;
	strTo = strString + "/" + GEX_HTMLPAGE_WEB_NOSETTINGS;
	if(CopyGivenFile(strFrom,strTo) != true) return false;

	strFrom = strExaminatorWebApp + GEX_HTMLPAGE_WEB_IMPORT;
	strTo = strString + "/" + GEX_HTMLPAGE_WEB_IMPORT;
	if(CopyGivenFile(strFrom,strTo) != true) return false;

	// Load "$home/user" folder with preset pages.
	strString = strUserHomeFolder + GEX_DATABASE_WEB_EXCHANGE;

	strFrom = strExaminatorWebApp + GEX_HTMLPAGE_WEB_HOME;
	strTo = strString + "/" + GEX_HTMLPAGE_WEB_HOME;
	if(CopyGivenFile(strFrom,strTo) != true) return false;

	strFrom = strExaminatorWebApp + GEX_HTMLPAGE_WEB_REPORTS;
	strTo = strString + "/" + GEX_HTMLPAGE_WEB_REPORTS;
	if(CopyGivenFile(strFrom,strTo) != true) return false;

	strFrom = strExaminatorWebApp + GEX_HTMLPAGE_WEB_DATABASES;
	strTo = strString + "/" + GEX_HTMLPAGE_WEB_DATABASES;
	if(CopyGivenFile(strFrom,strTo) != true) return false;

	return true;	// Success
}

///////////////////////////////////////////////////////////
// Copy ExaminatorWEB application images folder
// used to display application messages.
///////////////////////////////////////////////////////////
bool CHtmlPng::CreateWebMessagesImages(const char * szFrom, const QString& strUserHomeFolder)
{
	QDir	cDir;
	QString	strString, strExaminatorWebApp;
	QString strFrom,strTo;

	// Get ExaminatorWeb application path.
    strExaminatorWebApp = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString() + QString(szFrom);

	// Create "$home/help" folder
	strString = strUserHomeFolder + "/help";
	cDir.mkdir(strString);

	// Create "$home/import" folder
	strString = strUserHomeFolder + GEX_DATABASE_WEB_IMPORT;
	cDir.mkdir(strString);

	// Create "$home/help/images" folder
	strString = strUserHomeFolder + "/help/images";
	cDir.mkdir(strString);

	// Copy files: <ExaminatorWebApp>/help/images/xxxxx.png
	strFrom = strExaminatorWebApp + "ruler.png";
	strTo = strString + "/ruler.png";
	if(CopyGivenFile(strFrom,strTo) != true) return false;

	strFrom = strExaminatorWebApp + "rarrow.png";
	strTo = strString + "/rarrow.png";
	if(CopyGivenFile(strFrom,strTo) != true) return false;

	strFrom = strExaminatorWebApp + "assistant.png";
	strTo = strString + "/assistant.png";
	if(CopyGivenFile(strFrom,strTo) != true) return false;

	strFrom = strExaminatorWebApp + "sad_face.png";
	strTo = strString + "/sad_face.png";
	if(CopyGivenFile(strFrom,strTo) != true) return false;

	strFrom = strExaminatorWebApp + "smile_face.png";
	strTo = strString + "/smile_face.png";
	if(CopyGivenFile(strFrom,strTo) != true) return false;

	// Images used for HOME page & Database or Report staus pages.
	strFrom = strExaminatorWebApp + "onefile.png";
	strTo = strString + "/onefile.png";
	if(CopyGivenFile(strFrom,strTo) != true) return false;

	strFrom = strExaminatorWebApp + "twofiles.png";
	strTo = strString + "/twofiles.png";
	if(CopyGivenFile(strFrom,strTo) != true) return false;

	strFrom = strExaminatorWebApp + "resources.png";
	strTo = strString + "/resources.png";
	if(CopyGivenFile(strFrom,strTo) != true) return false;

	strFrom = strExaminatorWebApp + "eg_mixed.png";
	strTo = strString + "/eg_mixed.png";
	if(CopyGivenFile(strFrom,strTo) != true) return false;

	strFrom = strExaminatorWebApp + "file_remove.png";
	strTo = strString + "/file_remove.png";
	if(CopyGivenFile(strFrom,strTo) != true) return false;

	strFrom = strExaminatorWebApp + "file_open.png";
	strTo = strString + "/file_open.png";
	if(CopyGivenFile(strFrom,strTo) != true) return false;

	strFrom = strExaminatorWebApp + "zip.png";
	strTo = strString + "/zip.png";
	if(CopyGivenFile(strFrom,strTo) != true) return false;

	return true;	// Success
}
