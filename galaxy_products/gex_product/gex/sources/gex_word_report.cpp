///////////////////////////////////////////////////////////
// gex_word_report.cpp: implementation of CGexWordReport class.
///////////////////////////////////////////////////////////

#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDir>
#include <QPushButton>
#include <gqtl_sysutils.h>
#include <gqtl_log.h>

#include "gex_word_report.h"
#include "process_launcher.h"
#include "conversionprogress_dialog.h"
#include "engine.h"
#include "command_line_options.h"
#include "browser_dialog.h"

// The following enums have been retrieved from the Word IDL file (msword9.idl).
// They are used when creating the VBscipt HTML to Word conversion script.
typedef enum {
    wdWindowStateNormal = 0,
    wdWindowStateMaximize = 1,
    wdWindowStateMinimize = 2
} WdWindowState;

typedef enum {
    wdOrientPortrait = 0,
    wdOrientLandscape = 1
} WdOrientation;

typedef enum {
    wdPaper10x14 = 0,
    wdPaper11x17 = 1,
    wdPaperLetter = 2,
    wdPaperLetterSmall = 3,
    wdPaperLegal = 4,
    wdPaperExecutive = 5,
    wdPaperA3 = 6,
    wdPaperA4 = 7,
    wdPaperA4Small = 8,
    wdPaperA5 = 9,
    wdPaperB4 = 10,
    wdPaperB5 = 11,
    wdPaperCSheet = 12,
    wdPaperDSheet = 13,
    wdPaperESheet = 14,
    wdPaperFanfoldLegalGerman = 15,
    wdPaperFanfoldStdGerman = 16,
    wdPaperFanfoldUS = 17,
    wdPaperFolio = 18,
    wdPaperLedger = 19,
    wdPaperNote = 20,
    wdPaperQuarto = 21,
    wdPaperStatement = 22,
    wdPaperTabloid = 23,
    wdPaperEnvelope9 = 24,
    wdPaperEnvelope10 = 25,
    wdPaperEnvelope11 = 26,
    wdPaperEnvelope12 = 27,
    wdPaperEnvelope14 = 28,
    wdPaperEnvelopeB4 = 29,
    wdPaperEnvelopeB5 = 30,
    wdPaperEnvelopeB6 = 31,
    wdPaperEnvelopeC3 = 32,
    wdPaperEnvelopeC4 = 33,
    wdPaperEnvelopeC5 = 34,
    wdPaperEnvelopeC6 = 35,
    wdPaperEnvelopeC65 = 36,
    wdPaperEnvelopeDL = 37,
    wdPaperEnvelopeItaly = 38,
    wdPaperEnvelopeMonarch = 39,
    wdPaperEnvelopePersonal = 40,
    wdPaperCustom = 41
} WdPaperSize;

typedef enum {
    wdFieldEmpty = -1,
    wdFieldRef = 3,
    wdFieldIndexEntry = 4,
    wdFieldFootnoteRef = 5,
    wdFieldSet = 6,
    wdFieldIf = 7,
    wdFieldIndex = 8,
    wdFieldTOCEntry = 9,
    wdFieldStyleRef = 10,
    wdFieldRefDoc = 11,
    wdFieldSequence = 12,
    wdFieldTOC = 13,
    wdFieldInfo = 14,
    wdFieldTitle = 15,
    wdFieldSubject = 16,
    wdFieldAuthor = 17,
    wdFieldKeyWord = 18,
    wdFieldComments = 19,
    wdFieldLastSavedBy = 20,
    wdFieldCreateDate = 21,
    wdFieldSaveDate = 22,
    wdFieldPrintDate = 23,
    wdFieldRevisionNum = 24,
    wdFieldEditTime = 25,
    wdFieldNumPages = 26,
    wdFieldNumWords = 27,
    wdFieldNumChars = 28,
    wdFieldFileName = 29,
    wdFieldTemplate = 30,
    wdFieldDate = 31,
    wdFieldTime = 32,
    wdFieldPage = 33,
    wdFieldExpression = 34,
    wdFieldQuote = 35,
    wdFieldInclude = 36,
    wdFieldPageRef = 37,
    wdFieldAsk = 38,
    wdFieldFillIn = 39,
    wdFieldData = 40,
    wdFieldNext = 41,
    wdFieldNextIf = 42,
    wdFieldSkipIf = 43,
    wdFieldMergeRec = 44,
    wdFieldDDE = 45,
    wdFieldDDEAuto = 46,
    wdFieldGlossary = 47,
    wdFieldPrint = 48,
    wdFieldFormula = 49,
    wdFieldGoToButton = 50,
    wdFieldMacroButton = 51,
    wdFieldAutoNumOutline = 52,
    wdFieldAutoNumLegal = 53,
    wdFieldAutoNum = 54,
    wdFieldImport = 55,
    wdFieldLink = 56,
    wdFieldSymbol = 57,
    wdFieldEmbed = 58,
    wdFieldMergeField = 59,
    wdFieldUserName = 60,
    wdFieldUserInitials = 61,
    wdFieldUserAddress = 62,
    wdFieldBarCode = 63,
    wdFieldDocVariable = 64,
    wdFieldSection = 65,
    wdFieldSectionPages = 66,
    wdFieldIncludePicture = 67,
    wdFieldIncludeText = 68,
    wdFieldFileSize = 69,
    wdFieldFormTextInput = 70,
    wdFieldFormCheckBox = 71,
    wdFieldNoteRef = 72,
    wdFieldTOA = 73,
    wdFieldTOAEntry = 74,
    wdFieldMergeSeq = 75,
    wdFieldPrivate = 77,
    wdFieldDatabase = 78,
    wdFieldAutoText = 79,
    wdFieldCompare = 80,
    wdFieldAddin = 81,
    wdFieldSubscriber = 82,
    wdFieldFormDropDown = 83,
    wdFieldAdvance = 84,
    wdFieldDocProperty = 85,
    wdFieldOCX = 87,
    wdFieldHyperlink = 88,
    wdFieldAutoTextList = 89,
    wdFieldListNum = 90,
    wdFieldHTMLActiveX = 91
} WdFieldType;

CGexWordReport::CGexWordReport(): m_pclProgressDlg(NULL)
{
}

CGexWordReport::~CGexWordReport()
{
}

#ifdef _WIN32
int CGexWordReport::GenerateDocFromHtml(QWidget* parent,
                                        const GexWordOptions& stGexWordOptions,
                                        const QString& strHtmlFullFileName,
                                        QString strDocFullFileName/* = ""*/)
{
    QString strHtmlFlatFileName = strHtmlFullFileName;
    // Set Word document file name if empty
    if(strDocFullFileName.isEmpty())
        strDocFullFileName = strHtmlFullFileName + ".doc";

    // Normalize file names
    CGexSystemUtils::NormalizePath(strHtmlFlatFileName);
    CGexSystemUtils::NormalizePath(strDocFullFileName);

    // Try to remove destination file
    QDir clDir;
    if((QFile::exists(strDocFullFileName) == true) && (clDir.remove(strDocFullFileName) == false))
        return Err_RemoveDestFile;

    // Get directory where to generate the script file.
    // WinNT or later: user's profile directory.
    // Unix: HOME directory
    // If no user home directory can be found, use the application's directory.
    QString	strScriptDirectory;
    if( (CGexSystemUtils::GetUserHomeDirectory(strScriptDirectory) != CGexSystemUtils::NoError) &&
      (CGexSystemUtils::GetApplicationDirectory(strScriptDirectory) != CGexSystemUtils::NoError))
        return Err_CreateScript;

    // Generate VBscript file for the conversion
    QString	strConversionScript = strScriptDirectory + "/.gex_conversion_script_h2w.vbs";
    CGexSystemUtils::NormalizePath(strConversionScript);
    if(GenerateConversionScript(strConversionScript, stGexWordOptions, strHtmlFlatFileName, strDocFullFileName) == false)
        return Err_CreateScript;

    // Construct name of animation file: <application>/images/html2word.mng
    QString strAnimationFile;
    if(CGexSystemUtils::GetApplicationDirectory(strAnimationFile) == CGexSystemUtils::NoError)
    {
        strAnimationFile += "/images/html2word.mng";
        CGexSystemUtils::NormalizePath(strAnimationFile);
    }

    // Construct dialog tite
    QString strDlgTitle;
    strDlgTitle =	"<font color=\"#5555ff\" face=\"Arial\"><b>";
    strDlgTitle +=	"<p align=\"center\"> <i>Now finalizing Word document...!</i></b></p>";
    //strDlgTitle +=	"If you cancel this conversion, you will have to manually kill the WINWORD process.";
    strDlgTitle +=	"</font>";

    #ifdef GSDAEMON
        Q_UNUSED(parent)
    #endif

    // Run the script
    QStringList strListArg(strConversionScript);
    CProcessLauncher clProcessLauncher("cscript.exe",
                                       strListArg,
                                   #ifdef GSDAEMON
                                       true
                                   #else
                                       GS::Gex::Engine::GetInstance().GetCommandLineOptions().IsHidden()
                                   #endif
                                       );

    GSLOG(6, QString("Launching cscript with args : %1").arg(strListArg.join(" ")).toLatin1().data() );

#ifndef GSDAEMON
    // Gex is running as a batch, don't display the conversion dialog
    if (GS::Gex::Engine::GetInstance().GetCommandLineOptions().IsHidden() == true)
    {
#endif
        if(clProcessLauncher.Launch() == false)
            return Err_LaunchScriptProcess;
#ifndef GSDAEMON
    }
    else
    {
        m_pclProgressDlg = new ConversionProgressDialog(
          "Word Report", strDlgTitle, strAnimationFile, strHtmlFlatFileName, strDocFullFileName, parent);

        // Hide Progress bar "Cancel" button.
        if(!stGexWordOptions.m_bShowProgressBar)
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
#endif

    // Delete script file
    QFile::remove(strConversionScript);

    // Check if destination file exists
    if(QFile::exists(strDocFullFileName) == false)
        return Err_ScriptError;

    return NoError;
}
#else
    int
    CGexWordReport::GenerateDocFromHtml(QWidget* /*parent*/,
                                        const GexWordOptions& /*stGexWordOptions*/,
                                        const QString& /*strHtmlFullFileName*/,
                                        QString /*strDocFullFileName = ""*/)
    {
        return NotSupported;
    }
#endif

bool CGexWordReport::GenerateConversionScript(const QString& strScriptFile,
                                              const GexWordOptions& stGexWordOptions,
                                              const QString& strHtmlFullFileName,
                                              QString& strDocFullFileName)
{
    // Get short name of in and out files
    QFileInfo clFileInfo(strHtmlFullFileName);
    QString	strHtmlShortFileName = clFileInfo.fileName();
    clFileInfo.setFile(strDocFullFileName);
    QString	strDocShortFileName = clFileInfo.fileName();

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
    clStream << "'  This VBScript converts a Html file (.htm, .html) to a Word document (.doc) file. It  '" << endl;
    clStream << "'  also does the following tasks:                                                       '" << endl;
    clStream << "'                                                                                       '" << endl;
    clStream << "'  o add a footer containing a string given as parameter and the page number            '" << endl;
    clStream << "'  o break all links and include images into the Word document                          '" << endl;
    clStream << "'                                                                                       '" << endl;
    clStream << "'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''" << endl;
    clStream << "'                                                                                       '" << endl;
    clStream << "'  Notes :                                                                              '" << endl;
    clStream << "'                                                                                       '" << endl;
    clStream << "'  o Requires MS-Word to be installed.                                                  '" << endl;
    clStream << "'                                                                                       '" << endl;
    clStream << "'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''" << endl;
    clStream << endl;

    // Explicit option
    clStream << "option explicit" << endl;
    clStream << endl;

    // Call conversion function
    clStream << "ConvertHtml2Doc \"" << strHtmlFullFileName << "\", \"" << strHtmlShortFileName << "\", \"" << strDocFullFileName << "\", \"" << strDocShortFileName << "\", \"" << stGexWordOptions.m_strHeader << "\", \"" << stGexWordOptions.m_strFooter << "\"" << endl;
    clStream << endl;

    // Conversion function definition
    clStream << "Function ConvertHtml2Doc(ByRef strFileIn_HTML, ByRef strFileIn_HTML_short, ByRef strFileOut_DOC, ByRef strFileOut_DOC_short, ByRef strHeaderString, ByRef strFooterString)" << endl;
    clStream << endl;
    clStream << "	' Variables." << endl;
    clStream << "	Dim Word" << endl;
    clStream << "	Dim Document" << endl;
    clStream << "	Dim Result" << endl;
    clStream << endl;
    clStream << "	' Silently ignore errors to avoid VBScript message boxes, but handle errors in the script..." << endl;
    clStream << "	On Error Resume Next" << endl;
    clStream << endl;

    // Open Word
    clStream << "	' Open Word" << endl;
    clStream << "	Set Word = CreateObject(\"Word.Application\")" << endl;
    clStream << "	If IsObject(Word) = False Then" << endl;
    clStream << "		ConvertHtml2Doc = " << QString::number(Err_WordApplicationInit) << endl;
    clStream << "		MsgBox \"Function Exit code = \" & " << QString::number(Err_WordApplicationInit) << ", 0, \"VBScript\"" << endl;
    clStream << "		Exit Function" << endl;
    clStream << "	End If" << endl;
    clStream << endl;

    // Open HTML file
    clStream << "	' Open HTML file under Word, Quiet mode (No pop-up to confirm it's a HTML format)" << endl;
    clStream << "	Set Document = Word.Documents.Open(strFileIn_HTML, False)" << endl;
    clStream << "	If IsObject(Document) = False Then" << endl;
    clStream << "		ConvertHtml2Doc = " << QString::number(Err_WordDocumentOpen) << endl;
    clStream << "		MsgBox \"Function Exit code = \" & " << QString::number(Err_WordDocumentOpen) << ", 0, \"VBScript\"" << endl;
    clStream << "		Word.Quit" << endl;
    clStream << "		Set Document = Nothing" << endl;
    clStream << "		Set Word = Nothing" << endl;
    clStream << "		Exit Function" << endl;
    clStream << "	End If" << endl;
    clStream << endl;

    // Save as Word document
    clStream << "	' Save as .DOC file" << endl;
    clStream << "	Result = Document.SaveAs(strFileOut_DOC, 0)" << endl;
    clStream << "	If Document.Name = strFileIn_HTML_short Then" << endl;
    clStream << "		ConvertHtml2Doc = " << QString::number(Err_WordDocumentSave) << endl;
    clStream << "		MsgBox \"Function Exit code = \" & " << QString::number(Err_WordDocumentSave) << ", 0, \"VBScript\"" << endl;
    clStream << "		Document.Close" << endl;
    clStream << "		Word.Quit" << endl;
    clStream << "		Set Document = Nothing" << endl;
    clStream << "		Set Word = Nothing" << endl;
    clStream << "		Exit Function" << endl;
    clStream << "	End If" << endl;
    clStream << endl;

    // Set Word Visible/Minimized
    if(stGexWordOptions.m_bShowWordApplication == false)
    {
        clStream << "	' Make sure Word application is not visible" << endl;
        clStream << "	Word.Visible = False" << endl;
        clStream << endl;
    }
    else
    {
        if(stGexWordOptions.m_bMinimizeApplication)
        {
            clStream << "	' Minimize Word application window" << endl;
            clStream << "	Word.WindowState = " << QString::number(wdWindowStateMinimize) << endl;
            clStream << endl;
        }
        else
        {
            clStream << "	' Set Word application window to normal size" << endl;
            clStream << "	Word.WindowState = " << QString::number(wdWindowStateNormal) << endl;
            clStream << endl;
        }
        clStream << "	' Make Word application visible" << endl;
        clStream << "	Word.Visible = True" << endl;
        clStream << endl;
    }

    // Update all fields
    clStream << "	' If Word < Word 2000: Update all fields" << endl;
    clStream << "	If Word.Version < 10 Then" << endl;
    clStream << "		Document.Fields.Update" << endl;
    clStream << "	End If" << endl;
    clStream << endl;

    // Unlink embedded images
    clStream << "	' Unlink only embedded images fields" << endl;
    clStream << "	Dim CurShape" << endl;
    clStream << "	For Each CurShape in Document.InlineShapes" << endl;
    clStream << "		If CurShape.Type = 4 Then" << endl;
    clStream << "			CurShape.Field.Unlink" << endl;
    clStream << "		End If" << endl;
    clStream << "	Next" << endl;
    clStream << endl;

    // Set page properties (margins, paper size, paper orientation)
    clStream << "	' Set page properties" << endl;
    clStream << "	With Document.PageSetup" << endl;
    clStream << "		.LeftMargin = Word.InchesToPoints(" << QString::number(stGexWordOptions.m_lfMargin_Left) << ")" << endl;
    clStream << "		.RightMargin = Word.InchesToPoints(" << QString::number(stGexWordOptions.m_lfMargin_Right) << ")" << endl;
    clStream << "		.TopMargin = Word.InchesToPoints(" << QString::number(stGexWordOptions.m_lfMargin_Top) << ")" << endl;
    clStream << "		.BottomMargin = Word.InchesToPoints(" << QString::number(stGexWordOptions.m_lfMargin_Bottom) << ")" << endl;
    if(stGexWordOptions.m_nPaperFormat == GexWordOptions::ePaperFormatA4)
        clStream << "		.PaperSize = " << QString::number(wdPaperA4) << endl;
    else
        clStream << "		.PaperSize = " << QString::number(wdPaperLetter) << endl;
    if(stGexWordOptions.m_nPaperOrientation == GexWordOptions::ePaperOrientationPortrait)
        clStream << "		.Orientation = " << QString::number(wdOrientPortrait) << endl;
    else
        clStream << "		.Orientation = " << QString::number(wdOrientLandscape) << endl;
    clStream << "	End With" << endl;
    clStream << endl;

    // Add Footer
    clStream << "	' Add Footer" << endl;
    clStream << "	With Document.Sections(1)" << endl;
    clStream << "		.Footers(1).PageNumbers.Add(1)" << endl;
    clStream << "	End With" << endl;
    clStream << "	Document.Sections(1).Footers(1).Range.Select()" << endl;
    clStream << "	Word.Selection.Collapse(1)" << endl;
    clStream << "	With Word.Selection.ParagraphFormat" << endl;
    clStream << "		.Alignment = 1" << endl;
    clStream << "	End With" << endl;
    clStream << "	Word.Selection.TypeText(strFooterString)" << endl;
    clStream << "	Word.Selection.TypeParagraph()" << endl;
    // Header string under word didn't look nice...so we've added it in the footer!
    clStream << "	Word.Selection.TypeText(strHeaderString)" << endl;
    clStream << endl;

    // Save document
    clStream << "	' Save document" << endl;
    clStream << "	Document.Save()" << endl;
    clStream << endl;

    // Close Word
    clStream << "	' Close session" << endl;
    clStream << "	Document.Close" << endl;
    clStream << "	Word.Quit" << endl;
    clStream << "	Set Document = Nothing" << endl;
    clStream << "	Set Word = Nothing" << endl;
    clStream << endl;
    clStream << "	ConvertHtml2Doc = " << QString::number(NoError) << endl;
    clStream << "	MsgBox \"Function Exit code = \" & " << QString::number(NoError) << ", 0, \"VBScript\"" << endl;
    clStream << endl;
    clStream << "End Function" << endl;

    clFile.close();

    return true;
}
