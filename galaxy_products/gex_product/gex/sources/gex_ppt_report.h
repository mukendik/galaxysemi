///////////////////////////////////////////////////////////
// Examinator class for generating Microsoft Powerpoint reports (Win32 only)
///////////////////////////////////////////////////////////

#ifndef GEX_PPTREPORT_H
#define GEX_PPTREPORT_H

#include <qobject.h>
#include <qstring.h>

enum PaperOrientation {ePaperOrientationPortrait, ePaperOrientationLandscape};
enum PaperFormat {ePaperFormatA4, ePaperFormatLetter};

typedef struct t_GexPptOptions
{
	int		m_nPaperOrientation;
	int		m_nPaperFormat;
	QString	m_strHeader;
	QString	m_strFooter;
	bool	m_bShowPptApplication;
	bool	m_bMinimizeApplication;
	bool	m_bShowProgressBar;

	t_GexPptOptions()
	{
		m_strHeader				= "";
		m_strFooter				= "";
		m_bShowPptApplication	= true;
		m_bMinimizeApplication	= true; 
		m_bShowProgressBar      = true;
	}
} GexPptOptions;

class ConversionProgressDialog;

class CGexPptReport: public QObject
{
    Q_OBJECT

public:
	CGexPptReport();
	~CGexPptReport();
	
	int		GeneratePptFromHtml(QWidget* parent, const GexPptOptions& stGexPptOptions, const QString& strHtmlFullFileName, QString strPptFullFileName = "");

	// Error codes returned by CGexPptReport functions
	enum ErrorCode {NoError,					// No error
					NotSupported,				// Funcition not supported on current OS
					ConversionCancelled,		// Conversion process has been cancelled by the user
					Err_RemoveDestFile,			// Couldn't remove destination file (file probably in use by another application)
					Err_LaunchScriptProcess,	// Error launching script execution process
					Err_CreateScript,			// Couldn't create script file (Error getting user home directory, error opening file...)
					Err_ScriptError,			// Error during script execution (no Word file generated)
					Err_PptApplicationInit,		// Error initializing Powerpoint Apllication
					Err_PptDocumentOpen,		// Error opening HTML document
					Err_PptDocumentSave			// Error saving Powerpoint document
	};

private:	
	bool	GenerateConversionScript(const QString& strScriptFile, const GexPptOptions& stGexPptOptions, const QString& strHtmlFullFileName, QString& strPptFullFileName);
	
signals:

private slots:
	
private:
	ConversionProgressDialog*	m_pclProgressDlg;
};

#endif // GEX_PPTREPORT_H
