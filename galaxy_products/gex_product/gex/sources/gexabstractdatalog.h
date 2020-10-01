#ifndef GEXABSTRACTDATALOG_H
#define GEXABSTRACTDATALOG_H

#include <QString>

class CGexFileInGroup;
class CTest;
class CPartInfo;
class CFunctionalTest;
class GexVectorFailureInfo;

class GexAbstractDatalogEngine
{
public:

	GexAbstractDatalogEngine();
	virtual ~GexAbstractDatalogEngine();

	virtual void		writeDatalog() = 0;
};

class GexAbstractDatalog
{
protected:

	GexAbstractDatalog(bool bValidSection, QString strOutputFormat);
	virtual ~GexAbstractDatalog();

public:

	static bool			createInstance(bool bValidSection);
	static bool			existInstance();
	static bool			releaseInstance();
	static bool			isDataloggable(CTest *ptTestCell, bool bFailResult, bool bIsOutlier);
	static bool			pushParametricResult(CGexFileInGroup * pFile, CTest *ptTestCell, const CPartInfo * pPartInfo, float fValue, bool bFailResult, bool bAlarmResult, int iSiteID);
	static bool			pushFunctionalResult(CGexFileInGroup * pFile, CTest *ptTestCell, const CPartInfo * pPartInfo, float fValue, bool bFailResult, const QString& strVectorName, const GexVectorFailureInfo& failureInfo, int iSiteID);

	static bool			pushPartInfo(const CPartInfo * pPartInfo);

protected:

	void				writeGenericHeader();
	void				writeGenericPartFooter(const CPartInfo *pPartInfo);

	virtual bool		open() = 0;
	virtual void		close() = 0;
	virtual void		writeBeginFileInfo() = 0;
	virtual void		writeEndFileInfo() = 0;
	virtual void		writeBeginDatalog() = 0;
	virtual void		writeParametricDatalog(CGexFileInGroup * pFile, CTest *ptTestCell, const CPartInfo * pPartInfo, float fValue, bool bFailResult, bool bAlarmResult, int iSiteID) = 0;
	virtual void		writeFunctionalDatalog(CGexFileInGroup * pFile, CTest *ptTestCell, const CPartInfo * pPartInfo, float fValue, bool bFailResult, const QString& strVectorName, const GexVectorFailureInfo& failureInfo, int iSiteID) = 0;
	virtual void		writeEndDatalog() = 0;
	virtual void		writeHeader() = 0;
	virtual void		writeFooter() = 0;
	virtual void		writePartFooter(const CPartInfo *pPartInfo) = 0;

protected:

	enum fieldName
	{
		fieldNone			= 0x0000,
		fieldTestNumber		= 0x0001,
		fieldTestName		= 0x0002,
		fieldTestLimits		= 0x0004,
		fieldTestTime		= 0x0008,
		fieldDieXY			= 0x0010,
		fieldSite			= 0x0020,
		fieldPattern		= 0x0040,
		fieldCycleCount		= 0x0080,
		fieldRelVAddr		= 0x0100,
		fieldPinsFailed		= 0x0200,
		fieldPinNumber		= 0x0400,
		fieldPinName		= 0x0800,
		fieldComments		= 0x1000
	};

	Q_DECLARE_FLAGS(fields, fieldName);

	bool							m_bValidSection;
	QString							m_strOutputFormat;
	uint							m_uiTestsInDatalog;
	fields							m_eFields;
	GexAbstractDatalogEngine *		m_pEngine;

private:

	static GexAbstractDatalog *		m_pDatalog;
};

class GexHTMLDatalog : public GexAbstractDatalog
{
public:

	GexHTMLDatalog(bool bValidSection, QString strOutputFormat);

protected:

	bool				open();
	void				close();
	void				writeBeginFileInfo();
	void				writeEndFileInfo();
	void				writeBeginDatalog();
	void				writeParametricDatalog(CGexFileInGroup * pFile, CTest *ptTestCell, const CPartInfo * pPartInfo, float fValue, bool bFailResult, bool bAlarmResult, int iSiteID);
	void				writeFunctionalDatalog(CGexFileInGroup * pFile, CTest *ptTestCell, const CPartInfo * pPartInfo, float fValue, bool bFailResult, const QString& strVectorName, const GexVectorFailureInfo& failureInfo, int iSiteID);
	void				writeEndDatalog();
	void				writeHeader();
	void				writeFooter();
	void				writePageBreak();
	void				writePartHeader(CGexFileInGroup * pFile, int iSiteID);
	void				writePartFooter(const CPartInfo *pPartInfo);

	QString				writeCommonTestInfo(CGexFileInGroup * pFile, CTest *ptTestCell, const CPartInfo * pPartInfo, bool bFailResult);
	QString				writeFunctionalTestInfo(const QString& strVectorName, const GexVectorFailureInfo& failureInfo);
	QString				writeCommonTestResult(CGexFileInGroup *pFile, CTest *ptTestCell, float fValue, bool bFailResult, bool bAlarmResult);

	void				writeTestResult(CGexFileInGroup *pFile, const QString& strTestResult, int iSiteID);
};

class GexCSVDatalog : public GexAbstractDatalog
{
public:

	enum csvVersion
	{
		csvVersionStandard,		// Standard version 1 or 2 columns to display datalog (original format)
		csvVersionAdvanced_100	// Advanced version 1.00
	};

	GexCSVDatalog(bool bValidSection, QString strOutputFormat);

protected:

	bool				open();
	void				close();
	void				writeBeginFileInfo();
	void				writeEndFileInfo();
	void				writeBeginDatalog();
	void				writeParametricDatalog(CGexFileInGroup * pFile, CTest *ptTestCell, const CPartInfo * pPartInfo, float fValue, bool bFailResult, bool bAlarmResult, int iSiteID);
	void				writeFunctionalDatalog(CGexFileInGroup * pFile, CTest *ptTestCell, const CPartInfo * pPartInfo, float fValue, bool bFailResult, const QString& strVectorName, const GexVectorFailureInfo& failureInfo, int iSiteID);
	void				writeEndDatalog();
	void				writeHeader();
	void				writeFooter();
	void				writePartFooter(const CPartInfo *pPartInfo);

	void				writeCommonFields(CGexFileInGroup * pFile, CTest *ptTestCell, const CPartInfo * pPartInfo, float fValue, bool bFailResult, bool bAlarmResult);
	void				writeFunctionalFields(const QString& strVectorName, const GexVectorFailureInfo& failureInfo, int iPinIndex);
};


#endif // GEXABSTRACTDATALOG_H
