///////////////////////////////////////////////////////////
// gex_ppt_report.cpp: implementation of CGexPptReport class.
///////////////////////////////////////////////////////////

#include <qfile.h>
#include <qtextstream.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qpushbutton.h>
#include <gqtl_sysutils.h>

#include "gex_ppt_report.h"
#include "process_launcher.h"
#include "conversionprogress_dialog.h"
#include "browser_dialog.h"
#include <gqtl_log.h>
#include "engine.h"
#include "command_line_options.h"

// The following enums have been built after checking their effect, because no Powerpoint IDL file (msppt9.idl) was found.
// They are used when creating the VBscipt HTML to Powerpoint conversion script.
typedef enum {
    ppWindowNormal = 1,
    ppWindowMinimized = 2,
    ppWindowMaximized = 3
} PpWindowState;

typedef enum {
    msoOrientationHorizontal = 1,
    msoOrientationVertical = 2
} MsoOrientation;

typedef enum {
    ppSlideSizeLetterPaper = 2,
    ppSlideSizeA4Paper = 3
} PpSlideSizeType ;

CGexPptReport::CGexPptReport(): m_pclProgressDlg(NULL)
{
    GSLOG(SYSLOG_SEV_DEBUG, "");
}

CGexPptReport::~CGexPptReport()
{
}

#ifdef _WIN32
int CGexPptReport::GeneratePptFromHtml(QWidget* parent, const GexPptOptions& stGexPptOptions,
                                       const QString& strHtmlFullFileName, QString strPptFullFileName/* = ""*/)
{
    // Set Powerpoint document file name if empty
    if(strPptFullFileName.isEmpty())
        strPptFullFileName = strHtmlFullFileName + ".ppt";

    // Normalize file names
    QString strRealHtmlFullFileName = strHtmlFullFileName;
    CGexSystemUtils::NormalizePath(strRealHtmlFullFileName);
    //	clSystemUtils.NormalizePath(strHtmlFullFileName);
    CGexSystemUtils::NormalizePath(strPptFullFileName);

    // Try to remove destination file
    QDir clDir;
    if((QFile::exists(strPptFullFileName) == true) && (clDir.remove(strPptFullFileName) == false))
        return Err_RemoveDestFile;

    // Get directory where to generate the script file.
    // WinNT or later: user's profile directory.
    // Unix: HOME directory
    // If no user home directory can be found, use the application's directory.
    QString			strScriptDirectory;
    if( (CGexSystemUtils::GetUserHomeDirectory(strScriptDirectory) != CGexSystemUtils::NoError) &&
        (CGexSystemUtils::GetApplicationDirectory(strScriptDirectory) != CGexSystemUtils::NoError))
        return Err_CreateScript;

    // Generate VBscript file for the conversion
    QString	strConversionScript = strScriptDirectory + "/.gex_conversion_script_h2p.vbs";
    CGexSystemUtils::NormalizePath(strConversionScript);
    if(GenerateConversionScript(strConversionScript, stGexPptOptions, strRealHtmlFullFileName, strPptFullFileName) == false)
        return Err_CreateScript;

    // Construct name of animation file: <application>/images/html2ppt.mng
    QString strAnimationFile;
    if(CGexSystemUtils::GetApplicationDirectory(strAnimationFile) == CGexSystemUtils::NoError)
    {
        strAnimationFile += "/images/html2ppt.mng";
        CGexSystemUtils::NormalizePath(strAnimationFile);
    }

    // Construct dialog tite
    QString strDlgTitle;
    strDlgTitle =	"<font color=\"#5555ff\" face=\"Arial\"><b>";
    strDlgTitle +=	"<p align=\"center\"> <i>Now finalizing Powerpoint document...!</i></b></p>";
    //strDlgTitle +=	"If you cancel this conversion, you will have to manually kill the POWERPNT process.";
    strDlgTitle +=	"</font>";

    // Run the script
    QStringList strListArg(strConversionScript);
    CProcessLauncher clProcessLauncher("cscript.exe",
                                       strListArg,
                                       GS::Gex::Engine::GetInstance().GetCommandLineOptions().IsHidden());

    // Gex is running as a batch, don't display the conversion dialog
    if (GS::Gex::Engine::GetInstance().GetCommandLineOptions().IsHidden() == true)
    {
        if(clProcessLauncher.Launch() == false)
            return Err_LaunchScriptProcess;
    }
    else
    {
        m_pclProgressDlg = new ConversionProgressDialog("Powerpoint Report", strDlgTitle, strAnimationFile, strHtmlFullFileName, strPptFullFileName, parent);

        // Hide Progress bar "Cancel" button.
        if(!stGexPptOptions.m_bShowProgressBar)
            m_pclProgressDlg->buttonCancel->hide();

        connect( &clProcessLauncher, SIGNAL( sScriptFinished() ), m_pclProgressDlg, SLOT( accept() ) );

        if(clProcessLauncher.Launch() == false)
        {
            delete m_pclProgressDlg; m_pclProgressDlg=0;
            return Err_LaunchScriptProcess;
        }

        if(m_pclProgressDlg->exec() == QDialog::Rejected)
        {
            clProcessLauncher.AbortScript();
            delete m_pclProgressDlg; m_pclProgressDlg=0;
            // Delete script file
            QFile::remove(strConversionScript);
            return ConversionCancelled;
        }

        delete m_pclProgressDlg; m_pclProgressDlg=0;
    }

    // Delete script file
    QFile::remove(strConversionScript);

    // Check if destination file exists
    if(QFile::exists(strPptFullFileName) == false)
        return Err_ScriptError;

    return NoError;
}
#else
int CGexPptReport::GeneratePptFromHtml(QWidget* /*parent*/,
                                       const GexPptOptions& /*stGexPptOptions*/,
                                       const QString& /*strHtmlFullFileName*/,
                                       QString /*strPptFullFileName = ""*/)
{
    return NotSupported;
}
#endif

bool CGexPptReport::GenerateConversionScript(const QString& strScriptFile,
                                             const GexPptOptions& stGexPptOptions,
                                             const QString& strHtmlFullFileName,
                                             QString& strPptFullFileName)
{
    // Get shor name of in and out files
    QFileInfo	clFileInfo(strHtmlFullFileName);
    QString		strHtmlShortFileName = clFileInfo.fileName();
    clFileInfo.setFile(strPptFullFileName);
    QString		strPptShortFileName = clFileInfo.fileName();

    // First open the file
    QFile clFile(strScriptFile);
    if(clFile.open(QIODevice::WriteOnly) == false)
        return false;

    // Create text stream
    QTextStream	clStream(&clFile);

    // Write script
    // Header
    clStream << "'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''" << endl;
    clStream << "'  Copyright Quantix                                                                    '" << endl;
    clStream << "'  This computer program is protected by copyright law                                  '" << endl;
    clStream << "'  and international treaties. Unauthorized reproduction or                             '" << endl;
    clStream << "'  distribution of this program, or any portion of it,may                               '" << endl;
    clStream << "'  result in severe civil and criminal penalties, and will be                           '" << endl;
    clStream << "'  prosecuted to the maximum extent possible under the law.                             '" << endl;
    clStream << "'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''" << endl;
    clStream << "'                                                                                       '" << endl;
    clStream << "'  Description:                                                                         '" << endl;
    clStream << "'                                                                                       '" << endl;
    clStream << "'  This VBScript converts a Html file (.htm, .html) to a Powerpoint document (.ppt)     '" << endl;
    clStream << "'  file. It also does the following tasks:                                              '" << endl;
    clStream << "'                                                                                       '" << endl;
    clStream << "'  o add a footer containing a string given as parameter and the page number            '" << endl;
    clStream << "'                                                                                       '" << endl;
    clStream << "'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''" << endl;
    clStream << "'                                                                                       '" << endl;
    clStream << "'  Notes :                                                                              '" << endl;
    clStream << "'                                                                                       '" << endl;
    clStream << "'  o Requires MS-Powerpoint to be installed.                                            '" << endl;
    clStream << "'                                                                                       '" << endl;
    clStream << "'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''" << endl;
    clStream << endl;

    // Explicit option
    clStream << "option explicit" << endl;
    clStream << endl;

    // Call conversion function
    clStream << "ConvertHtml2Ppt \"" << strHtmlFullFileName << "\", \"" << strHtmlShortFileName << "\", \"" << strPptFullFileName << "\", \"" << strPptShortFileName << "\", \"" << stGexPptOptions.m_strHeader << "\", \"" << stGexPptOptions.m_strFooter << "\"" << endl;
    clStream << endl;

    // Conversion function definition
    clStream << "Function ConvertHtml2Ppt(ByRef strFileIn_HTML, ByRef strFileIn_HTML_short, ByRef strFileOut_PPT, ByRef strFileOut_PPT_short, ByRef strHeaderString, ByRef strFooterString)" << endl;
    clStream << endl;
    clStream << "	' Variables." << endl;
    clStream << "	Dim Powerpoint" << endl;
    clStream << "	Dim Presentation" << endl;
    clStream << "	Dim Result" << endl;
    clStream << endl;
    clStream << "	' Silently ignore errors to avoid VBScript message boxes, but handle errors in the script..." << endl;
    clStream << "	'On Error Resume Next" << endl;
    clStream << endl;

    // Open Powerpoint
    clStream << "	' Open Powerpoint" << endl;
    clStream << "	Set Powerpoint = CreateObject(\"Powerpoint.Application\")" << endl;
    clStream << "	If IsObject(Powerpoint) = False Then" << endl;
    clStream << "		ConvertHtml2Ppt = " << QString::number(Err_PptApplicationInit) << endl;
    clStream << "		MsgBox \"Function Exit code = \" & " << QString::number(Err_PptApplicationInit) << ", 0, \"VBScript\"" << endl;
    clStream << "		Exit Function" << endl;
    clStream << "	End If" << endl;
    clStream << endl;

    // Set Powerpoint Visible/Minimized
    // Powerpoint 2000 refuses to be not visible, so don't use the Visible property!!!
    // Even to set it to True, because it overrides the WindowState property (Window always maximized).
    if(stGexPptOptions.m_bMinimizeApplication)
    {
        clStream << "	' Minimize Powerpoint application window" << endl;
        clStream << "	Powerpoint.WindowState = " << QString::number(ppWindowMinimized) << endl;
        clStream << endl;
    }
    else
    {
        clStream << "	' Set Powerpoint application window to normal size" << endl;
        clStream << "	Powerpoint.WindowState = " << QString::number(ppWindowNormal) << endl;
        clStream << endl;
    }

    // Open HTML file
    clStream << "	' Open HTML file under Powerpoint" << endl;
    clStream << "	Set Presentation = Powerpoint.Presentations.Open(strFileIn_HTML, False, False, False)" << endl;
    clStream << "	If IsObject(Presentation) = False Then" << endl;
    clStream << "		ConvertHtml2Ppt = " << QString::number(Err_PptDocumentOpen) << endl;
    clStream << "		MsgBox \"Function Exit code = \" & " << QString::number(Err_PptDocumentOpen) << ", 0, \"VBScript\"" << endl;
    clStream << "		Powerpoint.Quit" << endl;
    clStream << "		Set Presentation = Nothing" << endl;
    clStream << "		Set Powerpoint = Nothing" << endl;
    clStream << "		Exit Function" << endl;
    clStream << "	End If" << endl;
    clStream << endl;

    // Save as Powerpoint document
    clStream << "	' Save as .PPT file. Arguments: '1' = Save to PPT, '12' = save to HTML, etc..." << endl;
    clStream << "	Result = Presentation.SaveAs(strFileOut_PPT, 1, False)" << endl;
    clStream << "	If Presentation.Name = strFileIn_HTML_short Then" << endl;
    clStream << "		ConvertHtml2Ppt = " << QString::number(Err_PptDocumentSave) << endl;
    clStream << "		MsgBox \"Function Exit code = \" & " << QString::number(Err_PptDocumentSave) << ", 0, \"VBScript\"" << endl;
    clStream << "		Powerpoint.Close" << endl;
    clStream << "		Powerpoint.Quit" << endl;
    clStream << "		Set Presentation = Nothing" << endl;
    clStream << "		Set Powerpoint = Nothing" << endl;
    clStream << "		Exit Function" << endl;
    clStream << "	End If" << endl;
    clStream << endl;

    // Set page properties (paper size, paper orientation)
    clStream << "	' Set page properties" << endl;
    clStream << "	With Presentation.PageSetup" << endl;
    if(stGexPptOptions.m_nPaperFormat == ePaperFormatA4)
        clStream << "		.SlideSize = " << QString::number(ppSlideSizeA4Paper) << endl;
    else
        clStream << "		.SlideSize = " << QString::number(ppSlideSizeLetterPaper) << endl;
    if(stGexPptOptions.m_nPaperOrientation == ePaperOrientationPortrait)
        clStream << "		.SlideOrientation = " << QString::number(msoOrientationVertical) << endl;
    else
        clStream << "		.SlideOrientation = " << QString::number(msoOrientationHorizontal) << endl;
    clStream << "	End With" << endl;
    clStream << endl;

    // Add Footer
    clStream << "	' Add Footer" << endl;
    clStream << "	If Presentation.HasTitleMaster Then" << endl;
    clStream << "		With Presentation.TitleMaster.HeadersFooters" << endl;
    clStream << "			With .Footer" << endl;
    clStream << "				.Text = strFooterString" << endl;
    clStream << "				.Visible = True" << endl;
    clStream << "			End With" << endl;
    clStream << "			.SlideNumber.Visible = True" << endl;
    clStream << "		End With" << endl;
    clStream << "	End If" << endl;
    clStream << "	With Presentation.SlideMaster.HeadersFooters" << endl;
    clStream << "		With .Footer" << endl;
    clStream << "			.Text = strFooterString" << endl;
    clStream << "			.Visible = True" << endl;
    clStream << "		End With" << endl;
    clStream << "		.SlideNumber.Visible = True" << endl;
    clStream << "	End With" << endl;
    clStream << "	With Presentation.Slides(1).HeadersFooters" << endl;
    clStream << "		With .Footer" << endl;
    clStream << "			.Text = strFooterString" << endl;
    clStream << "			.Visible = True" << endl;
    clStream << "		End With" << endl;
    clStream << "		.SlideNumber.Visible = True" << endl;
    clStream << "	End With" << endl;
    clStream << endl;

    // Save document
    clStream << "	' Save document" << endl;
    clStream << "	Presentation.Save()" << endl;
    clStream << endl;

    // Close Presentation
    clStream << "	' Close presentation" << endl;
    clStream << "	Presentation.Close" << endl;

    // Close Powerpoint if no presentation is open
    clStream << "	' Close session"							<< endl;
    clStream << "	If Powerpoint.Presentations.count = 0 Then"	<< endl;
    clStream << "		Powerpoint.Quit"						<< endl;
    clStream << "	End If"										<< endl;

    clStream << "	Set Presentation = Nothing"					<< endl;
    clStream << "	Set Powerpoint = Nothing"					<< endl;
    clStream << endl;
    clStream << "	ConvertHtml2Ppt = " << QString::number(NoError) << endl;
    clStream << "	MsgBox \"Function Exit code = \" & " << QString::number(NoError) << ", 0, \"VBScript\"" << endl;
    clStream << endl;
    clStream << "End Function" << endl;

    clFile.close();

    return true;
}
