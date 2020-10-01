///////////////////////////////////////////////////////////
// Examinator class for generating Microsoft Word reports (Win32 only)
///////////////////////////////////////////////////////////

#ifndef GEX_WORDREPORT_H
#define GEX_WORDREPORT_H

#include <qobject.h>
#include <qstring.h>

typedef struct t_GexWordOptions
{
	enum PaperOrientation {ePaperOrientationPortrait, ePaperOrientationLandscape};
	enum PaperFormat {ePaperFormatA4, ePaperFormatLetter};

	int		m_nPaperOrientation;
	int		m_nPaperFormat;
	QString	m_strFooter;
	QString	m_strHeader;
	double	m_lfMargin_Left;
	double	m_lfMargin_Right;
	double	m_lfMargin_Top;
	double	m_lfMargin_Bottom;
	bool	m_bShowWordApplication;
	bool	m_bMinimizeApplication;
	bool	m_bShowProgressBar;

	t_GexWordOptions()
	{
		m_nPaperOrientation		= ePaperOrientationLandscape;
		m_nPaperFormat			= ePaperFormatA4;
		m_strFooter				= "";
		m_strHeader				= "";
		m_lfMargin_Left			= 1.25;
		m_lfMargin_Right		= 1.25;
		m_lfMargin_Top			= 1.0;
		m_lfMargin_Bottom		= 1.0;
		m_bShowWordApplication	= false;
		m_bMinimizeApplication	= true; 
		m_bShowProgressBar      = true;

	}
} GexWordOptions;

class ConversionProgressDialog;

class CGexWordReport: public QObject
{
    Q_OBJECT

public:
	CGexWordReport();
	~CGexWordReport();
	
	int		GenerateDocFromHtml(QWidget* parent, const GexWordOptions& stGexWordOptions, const QString& strHtmlFullFileName, QString strDocFullFileName = "");

	// Error codes returned by CGexWordReport functions
	enum ErrorCode {NoError,					// No error
					NotSupported,				// Funcition not supported on current OS
					ConversionCancelled,		// Conversion process has been cancelled by the user
					Err_RemoveDestFile,			// Couldn't remove destination file (file probably in use by another application)
					Err_LaunchScriptProcess,	// Error launching script execution process
					Err_CreateScript,			// Couldn't create script file (Error getting user home directory, error opening file...)
					Err_ScriptError,			// Error during script execution (no Word file generated)
					Err_WordApplicationInit,	// Error initializing Word Apllication
					Err_WordDocumentOpen,		// Error opening HTML document
					Err_WordDocumentSave		// Error saving Word document
	};

private:	
	bool	GenerateConversionScript(const QString& strScriptFile, const GexWordOptions& stGexWordOptions, const QString& strHtmlFullFileName, QString& strDocFullFileName);
	
signals:

private slots:
	
private:
	ConversionProgressDialog*	m_pclProgressDlg;
};

#endif // GEX_WORDREPORT_H
