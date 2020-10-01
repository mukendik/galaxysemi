// HtmlPng.h : header file for the HtmlPng.cpp file
//

#if !defined(HTMLPNG_INCLUDED_)
#define HTMLPNG_INCLUDED_

//////////////////////////////////////////////////////
// Examinator Settings	
//////////////////////////////////////////////////////
#include "stdf.h"
class	CHtmlPng
{
public:
	CHtmlPng();
	~CHtmlPng();

	bool CreateWebReportImages(const char *szReportDirectory);
	bool CreateWebReportPages(const char *szReportDirectory);
	bool CreateBinImages(const char *szReportDirectory);
	bool CreateWebMessagesPages(const char *strFrom, const QString& strUserHomeFolder);
	bool CreateWebMessagesImages(const char *szFrom, const QString& strUserHomeFolder);
	bool CopyGivenFile(const QString& strFrom, const QString& strTo);

private:
	bool TryCreateFile(BYTE *ptFrom, const char *szTo, long lCount);
};

#endif // !defined(HTMLPNG_INCLUDED_)
