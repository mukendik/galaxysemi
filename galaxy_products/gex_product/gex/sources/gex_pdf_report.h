///////////////////////////////////////////////////////////
// Examinator class for generating PDF reports
///////////////////////////////////////////////////////////

#ifndef GEX_PDFREPORT_H
#define GEX_PDFREPORT_H

#include <qobject.h>
#include <qstring.h>
#ifndef GSDAEMON
    #include <QWebView>
#endif

typedef struct t_GexPdfOptions
{
	enum PaperOrientation {ePaperOrientationPortrait, ePaperOrientationLandscape};
	enum PaperFormat {ePaperFormatA4, ePaperFormatLetter};
	enum MarginUnits {eMarginUnitsInches, eMarginUmitsCentimeters};
    //! \brief GCORE-118 : let's offer the choice of the printer
    enum PrinterType {eHtmlDoc, eQPrinter} mPrinterType;

	int		m_nPaperOrientation;
	int		m_nPaperFormat;
	double	m_lfMargin_Left;
	double	m_lfMargin_Right;
	double	m_lfMargin_Top;
	double	m_lfMargin_Bottom;
	int		m_nMarginUnits;
	bool	m_bShowProgressBar;
	bool	m_bEmbedFonts;		// note : some exporters do not support this option
	QString	m_strHeader;
	QString	m_strFooter;
    QString m_imageFooter;
    QString m_imageFooterFile;

	t_GexPdfOptions()
	{
        mPrinterType = eHtmlDoc;
        m_nPaperOrientation	= ePaperOrientationPortrait;
        m_nPaperFormat = ePaperFormatA4;
        m_lfMargin_Left = 0.5;
        m_lfMargin_Right = 0.5;
        m_lfMargin_Top = 0.5;
        m_lfMargin_Bottom = 0.5;
        m_nMarginUnits = eMarginUnitsInches;
        m_bShowProgressBar = true;
        m_bEmbedFonts = false;
	}
} GexPdfOptions;

class ConversionProgressDialog;

class CGexPdfReport: public QObject
{
    Q_OBJECT

public:
	CGexPdfReport();
	~CGexPdfReport();
	
    //! \brief Generate a pdf from a html using given options.
    //! \param strPdfFullFileName empty, the pdf file will be HtmlSource+".pdf"
    //! \return an ErrorCode
    Q_INVOKABLE int	GeneratePdfFromHtml(QWidget* parent, const GexPdfOptions& stGexPdfOptions,
                            const QString& strHtmlFullFileName, QString strPdfFullFileName = "");

	// Error codes returned by CGexPdfReport functions
	enum ErrorCode {NoError,					// No error
                    NotSupported,				// Function not supported on current OS
					ConversionCancelled,		// Conversion process has been cancelled by the user
					Err_MissingExe,				// The htmldoc executable is missing
					Err_MissingDir_App,			// A directory is missing (application dir)
					Err_MissingDir_Data,		// A directory is missing (htmldoc data dir...)
					Err_MissingDir_Fonts,		// A directory is missing (htmldoc fonts dir...)
					Err_RemoveDestFile,			// Couldn't remove destination file (file probably in use by another application)
					Err_ProcessError,			// Error during htmldoc process execution (no PDF file generated)
                    Err_LaunchHtmlDocProcess,	// Error launching htmldoc process
                    Err_UnknownPrinterType,     // Printer type not supported
                    Err_TimeOut                 // 11: Time out: pdf generation is too long, abnormal
	};

public slots:
    //! \brief called when webview load finish
    void OnLoadFinished(bool);
    //! \brief Called when webview is porgressing
    void OnLoadProgress(int);
    //! \brief Called when webview starts to load
    void OnLoadStarted();
private:	
    void GenerateHtmldocArgument(const QString& strApplicationDir,
                                 QStringList& strArguments, const GexPdfOptions& stGexPdfOptions,
                                 const QString& strHtmlFullFileName, QString& strPdfFullFileName);
    //! \brief Generate a pdf from a html using given options.
    //! \param strPdfFullFileName empty, the pdf file will be HtmlSource+".pdf"
    //! \return an ErrorCode
    int	GeneratePdfFromHtmlUsingQPrinter(QWidget* parent, const GexPdfOptions& stGexPdfOptions,
                            const QString& strHtmlFullFileName, QString strPdfFullFileName = "");

    //! \brief Generate a pdf from a html using given options.
    //! \param strPdfFullFileName empty, the pdf file will be HtmlSource+".pdf"
    //! \return an ErrorCode
    int	GeneratePdfFromHtmlUsingHtmlDoc(QWidget* parent, const GexPdfOptions& stGexPdfOptions,
                            const QString& strHtmlFullFileName, QString strPdfFullFileName = "");

private:
    ConversionProgressDialog* m_pclProgressDlg;

    #ifndef GSDAEMON
        QWebView mWebView;
    #endif
};

#endif // GEX_PDFREPORT_H
