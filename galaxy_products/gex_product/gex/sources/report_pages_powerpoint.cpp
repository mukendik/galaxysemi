/////////////////////////////////////////////////////////////////////////////
// Most codes specific to the PowerPoint slides generation
/////////////////////////////////////////////////////////////////////////////

// Step1: Create a HTML home page <slides_name>.htm
// Step2: Create folder: <slides_name>.htm/<slides_name>
// Step3: Create HTML & XML files in folder <slides_name>.htm/<slides_name>
// Step4: Create one PNG image per slide ( = 1 HTML page)
// Step5: Call PowerPoint to convert this XML set of files to a PPT file: <slides_name>.ppt
#include <qapplication.h>
#include <QWebView>
#include <QWebFrame>
#include <gqtl_log.h>

#include "browser_dialog.h"
#include "report_build.h"
#include "report_options.h"
#include "gex_ppt_report.h"
#include "gex_report.h"
#include "db_engine.h"
#include "product_info.h"
#include "engine.h"
#include "message.h"

#ifdef _WIN32
 #include <QAbstractTextDocumentLayout>
 #include <QMessageBox>
#endif

// main.cpp
extern GexMainwindow *	pGexMainWindow;

// report_build.cpp
extern CReportOptions	ReportOptions;		// Holds options (report_build.h)

/////////////////////////////////////////////////////////////////////////////
// // Write HTML home page and prepare internal folders (Windows only)
/////////////////////////////////////////////////////////////////////////////
void CGexReport::WritePowerPointBegin(void)
{
#ifdef _WIN32

    // Write in <indexf.htm> file
    fprintf(hReportFile,"<html xmlns:v=\"urn:schemas-microsoft-com:vml\"\n");
    fprintf(hReportFile,"xmlns:o=\"urn:schemas-microsoft-com:office:office\"\n");
    fprintf(hReportFile,"xmlns:p=\"urn:schemas-microsoft-com:office:powerpoint\"\n");
    fprintf(hReportFile,"xmlns=\"http://www.w3.org/TR/REC-html40\">\n\n");

    fprintf(hReportFile,"<head>\n");
    fprintf(hReportFile,"<meta http-equiv=Content-Type content=\"text/html; charset=windows-1252\">\n");
    fprintf(hReportFile,"<meta name=ProgId content=PowerPoint.Slide>\n");
    fprintf(hReportFile,"<meta name=Generator content=\"Microsoft PowerPoint 9\">\n");
    fprintf(hReportFile,"<link rel=File-List href=\"./indexf_files/filelist.xml\">\n");
    fprintf(hReportFile,"<link rel=Preview href=\"./indexf_files/preview.wmf\">\n");
    fprintf(hReportFile,"<link rel=Edit-Time-Data href=\"./indexf_files/editdata.mso\">\n");
    fprintf(hReportFile,"<link rel=OLE-Object-Data href=\"./indexf_files/oledata.mso\">\n");
    fprintf(hReportFile,"<title>Page1</title>\n");
    fprintf(hReportFile,"<!--[if gte mso 9]><xml>\n");
    fprintf(hReportFile," <o:DocumentProperties>\n");
    fprintf(hReportFile,"  <o:Author>%s</o:Author>\n", GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );
    fprintf(hReportFile," </o:DocumentProperties>\n");
    fprintf(hReportFile," <o:OfficeDocumentSettings>\n");
    fprintf(hReportFile,"  <o:RelyOnVML/>\n");
    fprintf(hReportFile,"  <o:PixelsPerInch>80</o:PixelsPerInch>\n");
    fprintf(hReportFile," </o:OfficeDocumentSettings>\n");
    fprintf(hReportFile,"</xml><![endif]-->\n");
    fprintf(hReportFile,"<link rel=Presentation-XML href=\"./indexf_files/pres.xml\">\n");
    fprintf(hReportFile,"<meta name=Description content=\"22-Sep-04: Page1\">\n");
    fprintf(hReportFile,"<meta http-equiv=expires content=0>\n");
    fprintf(hReportFile,"<![if !ppt]><script>\n");
    fprintf(hReportFile,"<!--\n");
    fprintf(hReportFile,"  var appVer = navigator.appVersion;\n");
    fprintf(hReportFile,"  var msie = appVer.indexOf( \"MSIE \" );\n");
    fprintf(hReportFile,"  var msieWin31 = (appVer.indexOf( \"Windows 3.1\" ) >= 0), isMac = (appVer.indexOf(\"Macintosh\") >= 0);\n");
    fprintf(hReportFile,"  var ver = 0;\n");
    fprintf(hReportFile,"  if( msie >= 0 )\n");
    fprintf(hReportFile,"	ver = parseFloat( appVer.substring( msie+5, appVer.indexOf ( \";\", msie ) ) );\n");
    fprintf(hReportFile,"  else\n");
    fprintf(hReportFile,"	ver = parseInt( appVer );\n\n");

    fprintf(hReportFile,"	path = \"./indexf_files/error.htm\";\n\n");

    fprintf(hReportFile,"	if( msie>=0 && ( (isMac && ver>=5)||(!isMac && ver>=4) ) )\n");
    fprintf(hReportFile,"		window.location.replace( './indexf_files/frame.htm'+document.location.hash );\n");
    fprintf(hReportFile,"	else\n");
    fprintf(hReportFile,"	{\n");
    fprintf(hReportFile,"		if ( !msieWin31 && ( ( msie >= 0 && ver >= 3.02 ) || ( msie < 0 && ver >= 3 ) ) )\n");
    fprintf(hReportFile,"			window.location.replace( path );\n");
    fprintf(hReportFile,"		else\n");
    fprintf(hReportFile,"			window.location.href = path;\n");
    fprintf(hReportFile,"	}\n");
    fprintf(hReportFile,"//-->\n");
    fprintf(hReportFile,"</script><![endif]>\n");
    fprintf(hReportFile,"</head>\n\n");

    fprintf(hReportFile,"<frameset>\n");
    fprintf(hReportFile," <noframes>\n");
    fprintf(hReportFile,"  <body>\n");
    fprintf(hReportFile,"  <p>This page uses frames, but your browser doesn't support them.</p>\n");
    fprintf(hReportFile,"  </body>\n");
    fprintf(hReportFile," </noframes>\n");
    fprintf(hReportFile,"</frameset>\n\n");
    fprintf(hReportFile,"</html>\n");
    fclose(hReportFile);

        // HTML flat file is <path>/<query_folder_name>/pages/indexf.htm,
        if (reportGenerationMode() == "legacy")
            strPowerpointSubFolder = reportAbsFilePath();					// path: <path>/<query_folder_name>/pages/indexf.htm
        else
            strPowerpointSubFolder = reportFlatHtmlAbsPath();

    int iFolDerPath = strPowerpointSubFolder.lastIndexOf('.');
    strPowerpointSubFolder.truncate(iFolDerPath);			// path: <path>/<query_folder_name>/pages/indexf
    strPowerpointSubFolder += "_files";						// sub-Folder to hold all pages needed for the PPT convertion

    // Erase folder if it exists: <path>/<query_folder_name>/pages/indexf_files
    GS::Gex::Engine::GetInstance().GetDatabaseEngine().DeleteFolderContent(strPowerpointSubFolder);

    // Create folder
    QDir cDir;
    cDir.mkdir(strPowerpointSubFolder);

    // Create: <path>/<query_folder_name>/pages/indexf_files/error.htm
    QString strFile = strPowerpointSubFolder + "/error.htm";
    hReportFile = fopen(strFile.toLatin1().constData(),"w");
    if(hReportFile == NULL)
        return;

    fprintf(hReportFile,"<html>\n\n");
    fprintf(hReportFile,"<head>\n");
    fprintf(hReportFile,"<meta http-equiv=Content-Type content=\"text/html; charset=windows-1252\">\n");
    fprintf(hReportFile,"<meta name=ProgId content=PowerPoint.Slide>\n");
    fprintf(hReportFile,"<meta name=Generator content=\"Microsoft PowerPoint 9\">\n");
    fprintf(hReportFile,"<link id=Main-File rel=Main-File href=\"../indexf.htm\">\n");
    fprintf(hReportFile,"<link rel=Preview href=preview.wmf>\n");
    fprintf(hReportFile,"<meta name=Robots content=NoIndex>\n");
    fprintf(hReportFile,"</head>\n\n");
    fprintf(hReportFile,"<body>\n");
    fprintf(hReportFile,"<font face=Arial size=2><b>\n\n");
    fprintf(hReportFile,"<p>This presentation contains content that your browser may not be able to show\n");
    fprintf(hReportFile,"properly. This presentation was optimized for more recent versions of Microsoft\n");
    fprintf(hReportFile,"Internet Explorer.</p>\n\n");
    fprintf(hReportFile,"<p>If you would like to proceed anyway, click <a href=frame.htm>here</a>.</p>\n\n");
    fprintf(hReportFile,"</b></font>\n");
    fprintf(hReportFile,"</body>\n\n");
    fprintf(hReportFile,"</html>\n");
    fclose(hReportFile);

    // Create: <path>/<query_folder_name>/pages/indexf_files/frame.htm
    strFile = strPowerpointSubFolder + "/error.htm";
    hReportFile = fopen(strFile.toLatin1().constData(),"w");
    if(hReportFile == NULL)
        return;

    fprintf(hReportFile,"<html>\n\n");
    fprintf(hReportFile,"<head>\n");
    fprintf(hReportFile,"<meta http-equiv=Content-Type content=\"text/html; charset=windows-1252\">\n");
    fprintf(hReportFile,"<meta name=ProgId content=PowerPoint.Slide>\n");
    fprintf(hReportFile,"<meta name=Generator content=\"Microsoft PowerPoint 9\">\n");
    fprintf(hReportFile,"<link id=Main-File rel=Main-File href=\"../indexf.htm\">\n");
    fprintf(hReportFile,"<link rel=Preview href=preview.wmf>\n");
    fprintf(hReportFile,"<script src=script.js></script>\n");
    fprintf(hReportFile,"<script><!--\n");
    fprintf(hReportFile,"var SCREEN_MODE   = \"Kiosk\";\n");
    fprintf(hReportFile,"//-->\n");
    fprintf(hReportFile,"</script>\n");
    fprintf(hReportFile,"</head>\n\n");
    fprintf(hReportFile,"<frameset rows=\"*\" frameborder=0>\n");
    fprintf(hReportFile," <frame src=slide0001.htm name=PPTSld>\n");
    fprintf(hReportFile,"</frameset>\n\n");
    fprintf(hReportFile,"</html>\n");
    fclose(hReportFile);

    // Create: <path>/<query_folder_name>/pages/indexf_files/master03.htm
    strFile = strPowerpointSubFolder + "/master03.htm";
    hReportFile = fopen(strFile.toLatin1().constData(),"w");
    if(hReportFile == NULL)
        return;

    fprintf(hReportFile,"<html xmlns:v=\"urn:schemas-microsoft-com:vml\"\n");
    fprintf(hReportFile,"xmlns:o=\"urn:schemas-microsoft-com:office:office\"\n");
    fprintf(hReportFile,"xmlns:p=\"urn:schemas-microsoft-com:office:powerpoint\"\n");
    fprintf(hReportFile,"xmlns=\"http://www.w3.org/TR/REC-html40\">\n\n");
    fprintf(hReportFile,"<head>\n");
    fprintf(hReportFile,"<meta http-equiv=Content-Type content=\"text/html; charset=windows-1252\">\n");
    fprintf(hReportFile,"<meta name=ProgId content=FrontPage.Editor.Document>\n");
    fprintf(hReportFile,"<meta name=Generator content=\"Microsoft FrontPage 5.0\">\n");
    fprintf(hReportFile,"<link id=Main-File rel=Main-File href=\"../indexf.htm\">\n");
    fprintf(hReportFile,"<link rel=Preview href=preview.wmf>\n");
    fprintf(hReportFile,"<meta name=Robots content=NoIndex>\n");
    fprintf(hReportFile,"<link rel=Stylesheet href=\"master03_stylesheet.css\">\n");
    fprintf(hReportFile,"<![if !ppt]><script src=script.js></script>\n");
    fprintf(hReportFile,"<script>\n");
    fprintf(hReportFile,"<!--\n");
    fprintf(hReportFile,"	parent.location.href=document.all.item(\"Main-File\").href\n");
    fprintf(hReportFile,"//-->\n");
    fprintf(hReportFile,"</script>\n");
    fprintf(hReportFile,"<![endif]>\n");
    fprintf(hReportFile,"</head>\n\n");
    fprintf(hReportFile,"<body>\n\n");
    fprintf(hReportFile,"<div v:shape=\"_x0000_m1026\" class=T>Click to edit Master title style</div>\n\n");
    fprintf(hReportFile,"<div v:shape=\"_x0000_s1027\">\n\n");
    fprintf(hReportFile,"<div class=B>Click to edit Master text styles&#13;&#13;</div>\n\n");
    fprintf(hReportFile,"<div class=B1>Second level&#13;&#13;</div>\n\n");
    fprintf(hReportFile,"<div class=B2>Third level&#13;&#13;</div>\n\n");
    fprintf(hReportFile,"<div class=B3>Fourth level&#13;&#13;</div>\n\n");
    fprintf(hReportFile,"<div class=B4>Fifth level</div>\n\n");
    fprintf(hReportFile,"</div>\n\n");

    // Footer text style & location
    fprintf(hReportFile,"<div v:shape=\"_x0000_s1029\" class=O style='text-align:center'><span\n\n");
    fprintf(hReportFile,"style='font-size:42%%;mso-field-code:meta15'>%cfooter%c</span><span\n\n",0x8b,0x9b);
    fprintf(hReportFile,"style='font-size:42%%;mso-special-format:lastCR'>&#13;</span></div>\n\n");

    // Footer Slide#: text style & location
    fprintf(hReportFile,"<div v:shape=\"_x0000_s1030\" class=O style='text-align:right'><span\n\n");
    fprintf(hReportFile,"style='font-size:50%%;mso-field-code:meta16'>%c#%c</span><span style='font-size:\n",0x8b,0x9b);
    fprintf(hReportFile,"50%%;mso-special-format:lastCR'>&#13;</span></div>\n\n");
    fprintf(hReportFile,"</body>\n\n");
    fprintf(hReportFile,"</html>\n");
    fclose(hReportFile);


    // Create: <path>/<query_folder_name>/pages/indexf_files/master03.xml
    strFile = strPowerpointSubFolder + "/master03.xml";
    hReportFile = fopen(strFile.toLatin1().constData(),"w");
    if(hReportFile == NULL)
        return;

    fprintf(hReportFile,"<xml xmlns:v=\"urn:schemas-microsoft-com:vml\"\n");
    fprintf(hReportFile," xmlns:o=\"urn:schemas-microsoft-com:office:office\"\n");
    fprintf(hReportFile," xmlns:p=\"urn:schemas-microsoft-com:office:powerpoint\">\n");
    fprintf(hReportFile," <o:shapelayout v:ext=\"edit\">\n");
    fprintf(hReportFile,"  <o:idmap v:ext=\"edit\" data=\"1\"/>\n");
    fprintf(hReportFile," </o:shapelayout><p:colorscheme\n");
    fprintf(hReportFile,"  colors=\"#FFFFFF,#000000,#808080,#000000,#00CC99,#3333CC,#CCCCFF,#B2B2B2\"/>\n");
    fprintf(hReportFile," <v:background id=\"_x0000_s1025\" o:bwmode=\"white\" fillcolor=\"white [0]\">\n");
    fprintf(hReportFile,"  <v:fill color2=\"#33c [5]\"/>\n");
    fprintf(hReportFile," </v:background><p:shaperange id=\"_x0000_m1026\">\n");
    fprintf(hReportFile,"  <v:shapetype id=\"_x0000_m1026\" style='position:absolute;left:54pt;top:48pt;\n");
    fprintf(hReportFile,"   width:612pt;height:90pt;v-text-anchor:middle' coordsize=\"21600,21600\"\n");
    fprintf(hReportFile,"   o:master=\"\" o:spt=\"1\" path=\"m0,0l0,21600,21600,21600,21600,0xe\" filled=\"f\"\n");
    fprintf(hReportFile,"   fillcolor=\"#0c9 [4]\" stroked=\"f\" strokecolor=\"black [1]\">\n");
    fprintf(hReportFile,"   <v:fill color2=\"white [0]\" o:detectmouseclick=\"t\"/>\n");
    fprintf(hReportFile,"   <v:stroke joinstyle=\"miter\" o:forcedash=\"t\"/>\n");
    fprintf(hReportFile,"   <v:shadow color=\"gray [2]\"/>\n");
    fprintf(hReportFile,"   <v:path gradientshapeok=\"t\" o:connecttype=\"rect\"/>\n");
    fprintf(hReportFile,"   <o:lock v:ext=\"edit\" grouping=\"t\"/>\n");
    fprintf(hReportFile,"   <p:placeholder type=\"title\"/>\n");
    fprintf(hReportFile,"  </v:shapetype></p:shaperange>\n");
    fprintf(hReportFile," <p:shaperange id=\"_x0000_s1027\">\n");
    fprintf(hReportFile,"  <v:shapetype id=\"_x0000_s1027\" style='position:absolute;left:54pt;top:156pt;\n");
    fprintf(hReportFile,"   width:612pt;height:324pt' coordsize=\"21600,21600\" o:master=\"\" o:spt=\"1\"\n");
    fprintf(hReportFile,"   path=\"m0,0l0,21600,21600,21600,21600,0xe\" filled=\"f\" fillcolor=\"#0c9 [4]\"\n");
    fprintf(hReportFile,"   stroked=\"f\" strokecolor=\"black [1]\">\n");
    fprintf(hReportFile,"   <v:fill color2=\"white [0]\" o:detectmouseclick=\"t\"/>\n");
    fprintf(hReportFile,"   <v:stroke joinstyle=\"miter\" o:forcedash=\"t\"/>\n");
    fprintf(hReportFile,"   <v:shadow color=\"gray [2]\"/>\n");
    fprintf(hReportFile,"   <v:path gradientshapeok=\"t\" o:connecttype=\"rect\"/>\n");
    fprintf(hReportFile,"   <o:lock v:ext=\"edit\" grouping=\"t\"/>\n");
    fprintf(hReportFile,"   <p:placeholder type=\"body\" position=\"1\"/>\n");
    fprintf(hReportFile,"  </v:shapetype></p:shaperange>\n");
    fprintf(hReportFile," <p:shaperange id=\"_x0000_s1028\">\n");
    fprintf(hReportFile,"  <v:shapetype id=\"_x0000_s1028\" style='position:absolute;left:54pt;top:492pt;\n");
    fprintf(hReportFile,"   width:150pt;height:36pt' coordsize=\"21600,21600\" o:master=\"\" o:spt=\"1\"\n");
    fprintf(hReportFile,"   path=\"m0,0l0,21600,21600,21600,21600,0xe\" filled=\"f\" fillcolor=\"#0c9 [4]\"\n");
    fprintf(hReportFile,"   stroked=\"f\" strokecolor=\"black [1]\">\n");
    fprintf(hReportFile,"   <v:fill color2=\"white [0]\" o:detectmouseclick=\"t\"/>\n");
    fprintf(hReportFile,"   <v:stroke joinstyle=\"miter\" o:forcedash=\"t\"/>\n");
    fprintf(hReportFile,"   <v:shadow color=\"gray [2]\"/>\n");
    fprintf(hReportFile,"   <v:path gradientshapeok=\"t\" o:connecttype=\"rect\"/>\n");
    fprintf(hReportFile,"   <o:lock v:ext=\"edit\" grouping=\"t\"/>\n");
    fprintf(hReportFile,"   <p:placeholder type=\"dateTime\" position=\"2\" size=\"half\"/>\n");
    fprintf(hReportFile,"  </v:shapetype></p:shaperange>\n");
    fprintf(hReportFile," <p:shaperange id=\"_x0000_s1029\">\n");
    fprintf(hReportFile,"  <v:shapetype id=\"_x0000_s1029\" style='position:absolute;left:54pt;top:7in;\n");
    fprintf(hReportFile,"   width:420pt;height:24pt' coordsize=\"21600,21600\" o:master=\"\" o:spt=\"1\"\n");
    fprintf(hReportFile,"   path=\"m0,0l0,21600,21600,21600,21600,0xe\" filled=\"f\" fillcolor=\"#0c9 [4]\"\n");
    fprintf(hReportFile,"   stroked=\"f\" strokecolor=\"black [1]\">\n");
    fprintf(hReportFile,"   <v:fill color2=\"white [0]\" o:detectmouseclick=\"t\"/>\n");
    fprintf(hReportFile,"   <v:stroke joinstyle=\"miter\" o:forcedash=\"t\"/>\n");
    fprintf(hReportFile,"   <v:shadow color=\"gray [2]\"/>\n");
    fprintf(hReportFile,"   <v:path gradientshapeok=\"t\" o:connecttype=\"rect\"/>\n");
    fprintf(hReportFile,"   <o:lock v:ext=\"edit\" grouping=\"t\"/>\n");
    fprintf(hReportFile,"   <p:placeholder type=\"footer\" position=\"3\" size=\"quarter\"/>\n");
    fprintf(hReportFile,"  </v:shapetype></p:shaperange>\n");
    fprintf(hReportFile," <p:shaperange id=\"_x0000_s1030\">\n");
    fprintf(hReportFile,"  <v:shapetype id=\"_x0000_s1030\" style='position:absolute;left:516pt;top:7in;\n");
    fprintf(hReportFile,"   width:150pt;height:24pt' coordsize=\"21600,21600\" o:master=\"\" o:spt=\"1\"\n");
    fprintf(hReportFile,"   path=\"m0,0l0,21600,21600,21600,21600,0xe\" filled=\"f\" fillcolor=\"#0c9 [4]\"\n");
    fprintf(hReportFile,"   stroked=\"f\" strokecolor=\"black [1]\">\n");
    fprintf(hReportFile,"   <v:fill color2=\"white [0]\" o:detectmouseclick=\"t\"/>\n");
    fprintf(hReportFile,"   <v:stroke joinstyle=\"miter\" o:forcedash=\"t\"/>\n");
    fprintf(hReportFile,"   <v:shadow color=\"gray [2]\"/>\n");
    fprintf(hReportFile,"   <v:path gradientshapeok=\"t\" o:connecttype=\"rect\"/>\n");
    fprintf(hReportFile,"   <o:lock v:ext=\"edit\" grouping=\"t\"/>\n");
    fprintf(hReportFile,"   <p:placeholder type=\"slideNumber\" position=\"4\" size=\"quarter\"/>\n");
    fprintf(hReportFile,"  </v:shapetype></p:shaperange>\n");
    fprintf(hReportFile,"</xml>\n");
    fclose(hReportFile);


    // Create: <path>/<query_folder_name>/pages/indexf_files/master03_stylesheet.css
    strFile = strPowerpointSubFolder + "/master03_stylesheet.css";
    hReportFile = fopen(strFile.toLatin1().constData(),"w");
    if(hReportFile == NULL)
        return;

    // Defines Colors & Fonts assigned to PowerPoint Titles & Texts
    fprintf(hReportFile,"body\n");
    fprintf(hReportFile,"	{width:534px;\n");
    fprintf(hReportFile,"	height:400px;}\n");
    fprintf(hReportFile,".T\n");
    fprintf(hReportFile,"	{text-align:center;\n");
    fprintf(hReportFile,"	font-family:\"Times New Roman\";\n");
    fprintf(hReportFile,"	color:#006699;\n");
    fprintf(hReportFile,"	font-size:200%%;\n");
    fprintf(hReportFile,"	mso-char-wrap:1;\n");
    fprintf(hReportFile,"	mso-kinsoku-overflow:1;}\n");
    fprintf(hReportFile,".B\n");
    fprintf(hReportFile,"	{text-align:left;\n");
    fprintf(hReportFile,"	font-family:\"Times New Roman\";\n");
    fprintf(hReportFile,"	color:black;\n");
    fprintf(hReportFile,"	mso-color-index:1;\n");
    fprintf(hReportFile,"	font-size:152%%;\n");
    fprintf(hReportFile,"	mso-char-wrap:1;\n");
    fprintf(hReportFile,"	mso-kinsoku-overflow:1;}\n");
    fprintf(hReportFile,".B1\n");
    fprintf(hReportFile,"	{text-align:left;\n");
    fprintf(hReportFile,"	font-family:\"Times New Roman\";\n");
    fprintf(hReportFile,"	color:black;\n");
    fprintf(hReportFile,"	mso-color-index:1;\n");
    fprintf(hReportFile,"	font-size:133%%;\n");
    fprintf(hReportFile,"	mso-char-wrap:1;\n");
    fprintf(hReportFile,"	mso-kinsoku-overflow:1;}\n");
    fprintf(hReportFile,".B2\n");
    fprintf(hReportFile,"	{text-align:left;\n");
    fprintf(hReportFile,"	font-family:\"Times New Roman\";\n");
    fprintf(hReportFile,"	color:black;\n");
    fprintf(hReportFile,"	mso-color-index:1;\n");
    fprintf(hReportFile,"	font-size:114%%;\n");
    fprintf(hReportFile,"	mso-char-wrap:1;\n");
    fprintf(hReportFile,"	mso-kinsoku-overflow:1;}\n");
    fprintf(hReportFile,".B3\n");
    fprintf(hReportFile,"	{text-align:left;\n");
    fprintf(hReportFile,"	font-family:\"Times New Roman\";\n");
    fprintf(hReportFile,"	color:black;\n");
    fprintf(hReportFile,"	mso-color-index:1;\n");
    fprintf(hReportFile,"	font-size:95%%;\n");
    fprintf(hReportFile,"	mso-char-wrap:1;\n");
    fprintf(hReportFile,"	mso-kinsoku-overflow:1;}\n");
    fprintf(hReportFile,".B4\n");
    fprintf(hReportFile,"	{text-align:left;\n");
    fprintf(hReportFile,"	font-family:\"Times New Roman\";\n");
    fprintf(hReportFile,"	color:black;\n");
    fprintf(hReportFile,"	mso-color-index:1;\n");
    fprintf(hReportFile,"	font-size:95%%;\n");
    fprintf(hReportFile,"	mso-char-wrap:1;\n");
    fprintf(hReportFile,"	mso-kinsoku-overflow:1;}\n");
    fprintf(hReportFile,".N\n");
    fprintf(hReportFile,"	{mso-char-wrap:1;\n");
    fprintf(hReportFile,"	mso-kinsoku-overflow:1;}\n");
    fprintf(hReportFile,".N1\n");
    fprintf(hReportFile,"	{mso-char-wrap:1;\n");
    fprintf(hReportFile,"	mso-kinsoku-overflow:1;}\n");
    fprintf(hReportFile,".N2\n");
    fprintf(hReportFile,"	{mso-char-wrap:1;\n");
    fprintf(hReportFile,"	mso-kinsoku-overflow:1;}\n");
    fprintf(hReportFile,".N3\n");
    fprintf(hReportFile,"	{mso-char-wrap:1;\n");
    fprintf(hReportFile,"	mso-kinsoku-overflow:1;}\n");
    fprintf(hReportFile,".N4\n");
    fprintf(hReportFile,"	{mso-char-wrap:1;\n");
    fprintf(hReportFile,"	mso-kinsoku-overflow:1;}\n");
    fprintf(hReportFile,".O\n");
    fprintf(hReportFile,"	{text-align:left;\n");
    fprintf(hReportFile,"	font-family:\"Times New Roman\";\n");
    fprintf(hReportFile,"	color:black;\n");
    fprintf(hReportFile,"	mso-color-index:1;\n");
    fprintf(hReportFile,"	font-size:114%%;\n");
    fprintf(hReportFile,"	mso-char-wrap:1;\n");
    fprintf(hReportFile,"	mso-kinsoku-overflow:1;}\n");
    fprintf(hReportFile,".O1\n");
    fprintf(hReportFile,"	{mso-char-wrap:1;\n");
    fprintf(hReportFile,"	mso-kinsoku-overflow:1;}\n");
    fprintf(hReportFile,".O2\n");
    fprintf(hReportFile,"	{mso-char-wrap:1;\n");
    fprintf(hReportFile,"	mso-kinsoku-overflow:1;}\n");
    fprintf(hReportFile,".O3\n");
    fprintf(hReportFile,"	{mso-char-wrap:1;\n");
    fprintf(hReportFile,"	mso-kinsoku-overflow:1;}\n");
    fprintf(hReportFile,".O4\n");
    fprintf(hReportFile,"	{mso-char-wrap:1;\n");
    fprintf(hReportFile,"	mso-kinsoku-overflow:1;}\n");
    fprintf(hReportFile,".CB\n");
    fprintf(hReportFile,"	{mso-char-wrap:1;\n");
    fprintf(hReportFile,"	mso-kinsoku-overflow:1;}\n");
    fprintf(hReportFile,".CB1\n");
    fprintf(hReportFile,"	{mso-char-wrap:1;\n");
    fprintf(hReportFile,"	mso-kinsoku-overflow:1;}\n");
    fprintf(hReportFile,".CB2\n");
    fprintf(hReportFile,"	{mso-char-wrap:1;\n");
    fprintf(hReportFile,"	mso-kinsoku-overflow:1;}\n");
    fprintf(hReportFile,".CB3\n");
    fprintf(hReportFile,"	{mso-char-wrap:1;\n");
    fprintf(hReportFile,"	mso-kinsoku-overflow:1;}\n");
    fprintf(hReportFile,".CB4\n");
    fprintf(hReportFile,"	{mso-char-wrap:1;\n");
    fprintf(hReportFile,"	mso-kinsoku-overflow:1;}\n");
    fprintf(hReportFile,".CT\n");
    fprintf(hReportFile,"	{mso-char-wrap:1;\n");
    fprintf(hReportFile,"	mso-kinsoku-overflow:1;}\n");
    fprintf(hReportFile,".HB\n");
    fprintf(hReportFile,"	{mso-char-wrap:1;\n");
    fprintf(hReportFile,"	mso-kinsoku-overflow:1;}\n");
    fprintf(hReportFile,".HB1\n");
    fprintf(hReportFile,"	{mso-char-wrap:1;\n");
    fprintf(hReportFile,"	mso-kinsoku-overflow:1;}\n");
    fprintf(hReportFile,".HB2\n");
    fprintf(hReportFile,"	{mso-char-wrap:1;\n");
    fprintf(hReportFile,"	mso-kinsoku-overflow:1;}\n");
    fprintf(hReportFile,".HB3\n");
    fprintf(hReportFile,"	{mso-char-wrap:1;\n");
    fprintf(hReportFile,"	mso-kinsoku-overflow:1;}\n");
    fprintf(hReportFile,".HB4\n");
    fprintf(hReportFile,"	{mso-char-wrap:1;\n");
    fprintf(hReportFile,"	mso-kinsoku-overflow:1;}\n");
    fprintf(hReportFile,".QB\n");
    fprintf(hReportFile,"	{mso-char-wrap:1;\n");
    fprintf(hReportFile,"	mso-kinsoku-overflow:1;}\n");
    fprintf(hReportFile,".QB1\n");
    fprintf(hReportFile,"	{mso-char-wrap:1;\n");
    fprintf(hReportFile,"	mso-kinsoku-overflow:1;}\n");
    fprintf(hReportFile,".QB2\n");
    fprintf(hReportFile,"	{mso-char-wrap:1;\n");
    fprintf(hReportFile,"	mso-kinsoku-overflow:1;}\n");
    fprintf(hReportFile,".QB3\n");
    fprintf(hReportFile,"	{mso-char-wrap:1;\n");
    fprintf(hReportFile,"	mso-kinsoku-overflow:1;}\n");
    fprintf(hReportFile,".QB4\n");
    fprintf(hReportFile,"	{mso-char-wrap:1;\n");
    fprintf(hReportFile,"	mso-kinsoku-overflow:1;}\n");
    fprintf(hReportFile,".Tbl\n");
    fprintf(hReportFile,"	{mso-char-wrap:1;\n");
    fprintf(hReportFile,"	mso-kinsoku-overflow:1;}\n");
    fprintf(hReportFile,".Tbl1\n");
    fprintf(hReportFile,"	{mso-char-wrap:1;\n");
    fprintf(hReportFile,"	mso-kinsoku-overflow:1;}\n");
    fprintf(hReportFile,".Tbl2\n");
    fprintf(hReportFile,"	{mso-char-wrap:1;\n");
    fprintf(hReportFile,"	mso-kinsoku-overflow:1;}\n");
    fprintf(hReportFile,".Tbl3\n");
    fprintf(hReportFile,"	{mso-char-wrap:1;\n");
    fprintf(hReportFile,"	mso-kinsoku-overflow:1;}\n");
    fprintf(hReportFile,".Tbl4\n");
    fprintf(hReportFile,"	{mso-char-wrap:1;\n");
    fprintf(hReportFile,"	mso-kinsoku-overflow:1;}\n");
    fprintf(hReportFile,".defaultB\n");
    fprintf(hReportFile,"	{mso-special-format:nobullet%c;}\n",0x95);
    fprintf(hReportFile,".default\n");
    fprintf(hReportFile,"	{text-align:center;\n");
    fprintf(hReportFile,"	font-family:\"Times New Roman\";\n");
    fprintf(hReportFile,"	font-weight:normal;\n");
    fprintf(hReportFile,"	font-style:normal;\n");
    fprintf(hReportFile,"	text-decoration:none;\n");
    fprintf(hReportFile,"	text-shadow:none;\n");
    fprintf(hReportFile,"	text-effect:none;\n");
    fprintf(hReportFile,"	mso-fareast-hint:no;\n");
    fprintf(hReportFile,"	layout-flow:horizontal;\n");
    fprintf(hReportFile,"	color:black;\n");
    fprintf(hReportFile,"	mso-color-index:1;\n");
    fprintf(hReportFile,"	font-size:114%%;\n");
    fprintf(hReportFile,"	mso-text-raise:0%%;\n");
    fprintf(hReportFile,"	mso-line-spacing:\"100 0 0\";\n");
    fprintf(hReportFile,"	mso-margin-left-alt:0;\n");
    fprintf(hReportFile,"	mso-text-indent-alt:0;\n");
    fprintf(hReportFile,"	mso-char-wrap:1;\n");
    fprintf(hReportFile,"	mso-kinsoku-overflow:1;\n");
    fprintf(hReportFile,"	direction:ltr;\n");
    fprintf(hReportFile,"	mso-word-wrap:1;\n");
    fprintf(hReportFile,"	mso-vertical-align-special:baseline;\n");
    fprintf(hReportFile,"	mso-ansi-language:EN-GB;}\n");
    fprintf(hReportFile,"a:link\n");
    fprintf(hReportFile,"	{color:#CCCCFF !important;}\n");
    fprintf(hReportFile,"a:active\n");
    fprintf(hReportFile,"	{color:#3333CC !important;}\n");
    fprintf(hReportFile,"a:visited\n");
    fprintf(hReportFile,"	{color:#B2B2B2 !important;}\n");
    fclose(hReportFile);

    // Each section will be written in this ppt_slide.htm file then converted to an image.
    OpenTemporaryHtmlPowerPoint();

#endif	// Windows only
}

/////////////////////////////////////////////////////////////////////////////
// Open Temporary HTML file that will hold HTML codes for ONE slide only.
/////////////////////////////////////////////////////////////////////////////
void CGexReport::OpenTemporaryHtmlPowerPoint(bool bOpenHtml)
{
    #ifdef _WIN32
        // Each section will be written in this ppt_slide.htm file then converted to an image.
        QString strFile = m_pReportOptions->strReportDirectory + "/pages/ppt_slide.htm";
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Open temporary PowerPoint html %1").arg( strFile).toLatin1().constData());
        hAdvancedReport = hReportFile = fopen(strFile.toLatin1().constData(),"w");
        if(hReportFile == NULL)
        {
            GSLOG(SYSLOG_SEV_WARNING, QString("cant open temp html pres file %1").arg( strFile).toLatin1().constData());
            return;
        }

        if(!bOpenHtml)
        {
            // Wrap ALL the HTML content into a table of one cell with a forced width:
            // prevents QT from rendering HTML page with smaller than expected tables width.
            fprintf(hReportFile,"<html>\n");
            fprintf(hReportFile,"<body>\n");
            fprintf(hReportFile,"<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\" "\
              "style=\"border-collapse: collapse\" bordercolor=\"#111111\" width=\"900\">\n");
            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,"<td width=\"100%%\">\n");
        }
    #endif	// Windows only
    Q_UNUSED(bOpenHtml);
}

/////////////////////////////////////////////////////////////////////////////
// Finish to write HTML & XML files before converting to PPT files (Windows only)
/////////////////////////////////////////////////////////////////////////////
void CGexReport::WritePowerPointFinish(void)
{
#ifdef _WIN32
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Write PowerPoint finish code (filelist.xml & pres.xml)...");
    long	lIndex;

    // Create: <path>/<query_folder_name>/pages/indexf_files/filelist.xml
    QString strFile = strPowerpointSubFolder + "/filelist.xml";
    hReportFile = fopen(strFile.toLatin1().constData(),"w");
    if(hReportFile == NULL)
        return;

    fprintf(hReportFile,"<xml xmlns:o=\"urn:schemas-microsoft-com:office:office\">\n");
    fprintf(hReportFile," <o:File HRef=\"master03.htm\"/>\n");
    fprintf(hReportFile," <o:File HRef=\"master03.xml\"/>\n");
    fprintf(hReportFile,"  <o:File HRef=\"pres.xml\"/>\n");
    for(lIndex=1;lIndex <= lSlideIndex; lIndex++)
    {
        fprintf(hReportFile, " <o:File HRef=\"slide%04ld.htm\"/>\n", lIndex);
        fprintf(hReportFile, " <o:File HRef=\"slide%04ld_image001.png\"/>\n",
                lIndex);
    }

    fprintf(hReportFile," <o:File HRef=\"master03_stylesheet.css\"/>\n");
    fprintf(hReportFile," <o:MainFile HRef=\"../indexf.htm\"/>\n");
    fprintf(hReportFile," <o:File HRef=\"error.htm\"/>\n");
    fprintf(hReportFile,"  <o:File HRef=\"filelist.xml\"/>\n");
    fprintf(hReportFile,"</xml>\n");
    fclose(hReportFile);

    // Create: <path>/<query_folder_name>/pages/indexf_files/pres.xml
    strFile = strPowerpointSubFolder + "/pres.xml";
    hReportFile = fopen(strFile.toLatin1().constData(),"w");
    if(hReportFile == NULL)
        return;

    fprintf(hReportFile,"<xml xmlns:v=\"urn:schemas-microsoft-com:vml\"\n");
    fprintf(hReportFile," xmlns:o=\"urn:schemas-microsoft-com:office:office\"\n");
    fprintf(hReportFile," xmlns:p=\"urn:schemas-microsoft-com:office:powerpoint\">\n");
    fprintf(hReportFile," <p:presentation>\n");
    fprintf(hReportFile,"  <p:master id=\"3\" type=\"main\" href=\"master03.htm\" xmlhref=\"master03.xml\"\n");
    fprintf(hReportFile,"   template=\"Default Design\" layout=\"title_body\"\n");
    fprintf(hReportFile,"   slots=\"title,body,dateTime,footer,slideNumber\">\n");
    fprintf(hReportFile,"   <p:schemes>\n");
    fprintf(hReportFile,"    <p:colorscheme\n");
    fprintf(hReportFile,"     colors=\"#0000FF,#FFFFFF,#000000,#FFFF00,#FF9900,#00FFFF,#FF0000,#969696\"/>\n");
    fprintf(hReportFile,"    <p:colorscheme\n");
    fprintf(hReportFile,"     colors=\"#FFFFFF,#000000,#808080,#000000,#00CC99,#3333CC,#CCCCFF,#B2B2B2\"/>\n");
    fprintf(hReportFile,"    <p:colorscheme\n");
    fprintf(hReportFile,"     colors=\"#FFFFFF,#000000,#333333,#000000,#DDDDDD,#808080,#4D4D4D,#EAEAEA\"/>\n");
    fprintf(hReportFile,"    <p:colorscheme\n");
    fprintf(hReportFile,"     colors=\"#F8F8F8,#000000,#666633,#808000,#339933,#800000,#0033CC,#FFCC66\"/>\n");
    fprintf(hReportFile,"    <p:colorscheme\n");
    fprintf(hReportFile,"     colors=\"#FFFFFF,#000000,#808080,#000000,#FFCC66,#0000FF,#CC00CC,#C0C0C0\"/>\n");
    fprintf(hReportFile,"    <p:colorscheme\n");
    fprintf(hReportFile,"     colors=\"#FFFFFF,#000000,#808080,#000000,#C0C0C0,#0066FF,#FF0000,#009900\"/>\n");
    fprintf(hReportFile,"    <p:colorscheme\n");
    fprintf(hReportFile,"     colors=\"#FFFFFF,#000000,#808080,#000000,#3399FF,#99FFCC,#CC00CC,#B2B2B2\"/>\n");
    fprintf(hReportFile,"   </p:schemes>\n");
    fprintf(hReportFile,"  </p:master>\n");

    for(lIndex=1;lIndex <= lSlideIndex; lIndex++)
        fprintf(hReportFile,
                "  <p:slide id=\"%ld\" href=\"slide%04ld.htm\"/>\n",
                lIndex, lIndex);

    fprintf(hReportFile,"  <p:viewstate type=\"outline\"/>\n");
    fprintf(hReportFile,"  <p:font name=\"Times New Roman\" charset=\"0\" type=\"4\"/>\n");
    fprintf(hReportFile,"  <p:pptdocumentsettings framecolors=\"WhiteTextOnBlack\" hidenavigation=\"t\"\n");
    fprintf(hReportFile,"   donotresizegraphics=\"t\" hideslideanimation=\"t\"/>\n");
    fprintf(hReportFile," </p:presentation>\n");
    fprintf(hReportFile," <o:shapedefaults v:ext=\"edit\" spidmax=\"7170\"/>\n");
    fprintf(hReportFile,"</xml>\n");
    fclose(hReportFile);

#endif	// Windows only
}

/////////////////////////////////////////////////////////////////////////////
// Set section name. can use "<br>" in the string to have multi-line title!
/////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
void CGexReport::SetPowerPointSlideName(QString strSlideName,bool bAppend)
{

    QString of=m_pReportOptions->GetOption("output", "format").toString();

    // If function called while not creating a PPT file...quietly return
    if (of!="PPT") //if(pReportOptions->iOutputFormat != GEX_OPTION_OUTPUT_PPT)
        return;
    if(bAppend)
        strSlideTitle += strSlideName;	// Appending string to current slide name (allows to dynamically build slide name)
    else
        strSlideTitle = strSlideName;	// Set slide name to given string

}
#else
void CGexReport::SetPowerPointSlideName(QString /*strSlideName*/,
                                        bool /*bAppend*/)
{
}
#endif	// Windows only

#ifdef _WIN32
void CGexReport::RefreshPowerPointSlideName(QString strSection,int iChartInPage/*=1*/,
                                            int iMaxChartsPerPage/*=1*/,
                                            CTest *ptTestCellX/*=NULL*/,CTest *ptTestCellY/*=NULL*/)
{
    QString of=m_pReportOptions->GetOption("output", "format").toString();

    // If function called while not creating a PPT file...quietly return
    if (of!="PPT") //if(pReportOptions->iOutputFormat != GEX_OPTION_OUTPUT_PPT)
        return;

    // build string "<section> : <Test name>"
    QString strLabel = strSection;
    if(strSection.isEmpty() == false)
        strLabel += " : ";
    if(ptTestCellX != NULL)
        strLabel += ptTestCellX->strTestName;
    if(ptTestCellY != NULL)
        strLabel += " vs " + ptTestCellY->strTestName;

    bool bAppend=true;
    if(iChartInPage == 1)
        bAppend = false;
    else
    if(((iChartInPage-1) % iMaxChartsPerPage) == 0)
        bAppend = false;

    if(bAppend)
        strLabel = "<br>" + strLabel;	// Insert 'line-feed' HTML code to have this line appended to the others.

    // Update Slide name
    SetPowerPointSlideName(strLabel,bAppend);
}
#else
void CGexReport::RefreshPowerPointSlideName(QString /*strSection*/,
                                            int /*iChartInPage = 1*/,
                                            int /*iMaxChartsPerPage = 1*/,
                                            CTest* /*ptTestCellX = NULL*/,
                                            CTest* /*ptTestCellY = NULL*/)
{
}
#endif

#ifdef _WIN32
QString CGexReport::WritePowerPointSlide(FILE *hFile, QString strSlideName, bool bOpenHtml)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("Write PowerPoint Slide %1").arg(strSlideName).toLatin1().constData());
    // If function called while not creating a pres file...quietly return
    QString of=m_pReportOptions->GetOption("output", "format").toString();
    if (of!="PPT" && of!="ODP") //if (pReportOptions->iOutputFormat != GEX_OPTION_OUTPUT_PPT)
        return "ok: not a presenation type report (no PPT nor ODP)";

    // Close HTML page to capture as an image
    FILE  *hFileHandle=0;
    if(hFile != NULL)
        hFileHandle = hFile;
    else
        hFileHandle = hReportFile;

    // If nothink to flush, quietly return.
    if(hFileHandle == NULL)
        return "ok: nothing to flush, report file null";

    // End the HTML slide file, closing the HTML TABLE used to force the content to have a given width (eg: 900 pixels)
    fprintf(hFileHandle,"\n    </td>\n");
    fprintf(hFileHandle,"   </tr>\n");
    fprintf(hFileHandle,"  </table>\n");
    fprintf(hFileHandle," </body>\n");
    fprintf(hFileHandle,"</html>\n");

    // Close file so we can convert it to a PNG image.
    fclose(hFileHandle);

    // Makes sure the file closed has its handle cleared!
    hFile = NULL;

    // Get slide name to use: either a custom name is specified by caller, or use Section name specified on first slide in this section
    QString	strLabel = strSlideName;
    if(strLabel.isEmpty())
        strLabel = strSlideTitle;
    if(strLabel.isEmpty())
        strLabel = "";

    // Track Slide#
    lSlideIndex++;

    // Load HTML slide section into Pixmap
    QString strFile = m_pReportOptions->strReportDirectory + "/pages/ppt_slide.htm";
    #ifdef QT_DEBUG
        QFile::copy(strFile, GS::Gex::Engine::GetInstance().Get("TempFolder").toString()+"/ppt_slide.htm");
    #endif

    // 7131 : QTextBrowser does not auto resize images, QWebView does. Let's try to use QWebView.

    // QTextBrowser : wafermaps wont be resized to enter into the page and are sometimes cut
      //  QTextBrowser cTextBrowser;
      //  cTextBrowser.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      //  cTextBrowser.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      //  cTextBrowser.setSource(QUrl::fromLocalFile(strFile));

    // 7131: bug fix using a WebView ?
    QWebView cTextBrowser;
    QWebSettings::clearMemoryCaches(); // must have: if not, the cache will keep previous images from previous report
    QWebSettings::clearIconDatabase();
    cTextBrowser.load(QUrl::fromLocalFile(strFile));
    QFile lSourceFile(strFile);
    if (!lSourceFile.open(QIODevice::ReadOnly))
        return "error: cannot open html source file: "+strFile;
    QTextStream lSourceTextStream(&lSourceFile);
    cTextBrowser.setHtml( lSourceTextStream.readAll(), QUrl::fromLocalFile(QFileInfo(strFile).absolutePath()+"/") );
    lSourceFile.close();

    int iWidth = cTextBrowser.size().width();
    int iHeight = cTextBrowser.size().height();
    // Default image size is 950x650, see if resize required
    double	lfResizeX=1,lfResizeY=1;
    if(iWidth > 950)
        lfResizeX = (double)iWidth/950.0;
    if(iHeight > 650)
        lfResizeY = (double)iHeight/650.0;
    // Resize (add 10 extra pixels in Y to avoid the display of a scroll bar!)
    cTextBrowser.resize(950*lfResizeX, 10+650*lfResizeY);

    if (cTextBrowser.page())
        if (cTextBrowser.page()->mainFrame())
            cTextBrowser.page()->mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);

    // Mandatory: makes sure QT processes its resize message (even if object is hidden)
    QCoreApplication::processEvents();

    // Grab updated HTML image and save it to disk!
    // todo: QPixmap::grabWidget is deprecated, use QWidget::grab() instead
    //QPixmap	pPixmap = QPixmap::grabWidget(&cTextBrowser);
    QPixmap	pPixmap=cTextBrowser.grab();
    if (pPixmap.isNull())
    {
        GSLOG(SYSLOG_SEV_ERROR, "Failed to grab WebView");
        return "error: failed to grab webview";
    }

    strFile.sprintf("%s/slide%04ld_image001.png", strPowerpointSubFolder.toLatin1().constData(), lSlideIndex);
    if (!pPixmap.save(strFile,"PNG"))
        return "error: cannot save pixmap to "+strFile;
    #ifdef QT_DEBUG
        pPixmap.save( GS::Gex::Engine::GetInstance().Get("TempFolder").toString()
                      +QString("/slide%1.png").arg(lSlideIndex),"PNG");
    #endif

    // Create: <path>/<query_folder_name>/pages/indexf_files/slide0001.htm
    strFile.sprintf("%s/slide%04ld.htm", strPowerpointSubFolder.toLatin1().constData(), lSlideIndex);
    hFileHandle = fopen(strFile.toLatin1().constData(),"w");
    if(hFileHandle == NULL)
        return QString("error: cannot open report file '%1'").arg(strFile);

    // Compute font size to force, based on the number of lines in the title.
    // Size of 100% if title is only one line
    // Size of 50% if title is 2 ines, etc...
    float fFontSize = 100.0/ (2 + strSlideTitle.count("<br>",Qt::CaseInsensitive));

    fprintf(hFileHandle,"<html xmlns:v=\"urn:schemas-microsoft-com:vml\"\n");
    fprintf(hFileHandle,"xmlns:o=\"urn:schemas-microsoft-com:office:office\"\n");
    fprintf(hFileHandle,"xmlns:p=\"urn:schemas-microsoft-com:office:powerpoint\"\n");
    fprintf(hFileHandle,"xmlns=\"http://www.w3.org/TR/REC-html40\">\n");

    fprintf(hFileHandle,"<head>\n");
    fprintf(hFileHandle,"<meta http-equiv=Content-Type content=\"text/html; charset=windows-1252\">\n");
    fprintf(hFileHandle,"<meta name=ProgId content=PowerPoint.Slide>\n");
    fprintf(hFileHandle,"<meta name=Generator content=\"Microsoft PowerPoint 9\">\n");
    fprintf(hFileHandle,"<link id=Main-File rel=Main-File href=\"../indexf.htm\">\n");
    fprintf(hFileHandle,"<link rel=Preview href=preview.wmf>\n");
    fprintf(hFileHandle,"<!--[if !mso]>\n");
    fprintf(hFileHandle,"<style>\n");
    fprintf(hFileHandle,"v\\:* {behavior:url(#default#VML);}\n");
    fprintf(hFileHandle,"o\\:* {behavior:url(#default#VML);}\n");
    fprintf(hFileHandle,"p\\:* {behavior:url(#default#VML);}\n");
    fprintf(hFileHandle,".shape {behavior:url(#default#VML);}\n");
    fprintf(hFileHandle,"v\\:textbox {display:none;}\n");
    fprintf(hFileHandle,"</style>\n");
    fprintf(hFileHandle,"<![endif]-->\n");
    fprintf(hFileHandle,"<title>%s</title>\n", GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );
    fprintf(hFileHandle,"<meta name=Description content=\"22-Sep-04: Page1\">\n");
    fprintf(hFileHandle,"<link rel=Stylesheet href=\"master03_stylesheet.css\">\n");
    fprintf(hFileHandle,"<![if !ppt]>\n");
    fprintf(hFileHandle,"<style media=print>\n");
    fprintf(hFileHandle,"<!--.sld\n");
    fprintf(hFileHandle,"	{left:0px !important;\n");
    fprintf(hFileHandle,"	width:6.0in !important;\n");
    fprintf(hFileHandle,"	height:4.5in !important;\n");
    fprintf(hFileHandle,"	font-size:107%% !important;}\n");
    fprintf(hFileHandle,"-->\n");
    fprintf(hFileHandle,"</style>\n");
    fprintf(hFileHandle,"<script src=script.js></script><script><!--\n");
    fprintf(hFileHandle, "gId=\"slide%04ld.htm\"\n", lSlideIndex);
    fprintf(hFileHandle,"if( !IsNts() ) Redirect( \"PPTSld\", gId );\n");
    fprintf(hFileHandle,"//-->\n");
    fprintf(hFileHandle,"</script><!--[if vml]><script>g_vml = 1;\n");
    fprintf(hFileHandle,"</script><![endif]--><script for=window event=onload><!--\n");
    fprintf(hFileHandle,"if( !IsSldOrNts() ) return;\n");
    fprintf(hFileHandle,"if( MakeNotesVis() ) return;\n");
    fprintf(hFileHandle,"LoadSld( gId );\n");
    fprintf(hFileHandle,"MakeSldVis(0);\n");
    fprintf(hFileHandle,"//-->\n");
    fprintf(hFileHandle,"</script><![endif]><o:shapelayout v:ext=\"edit\">\n");
    fprintf(hFileHandle,"<o:idmap v:ext=\"edit\" data=\"2\"/>\n");
    fprintf(hFileHandle,"</o:shapelayout>\n");
    fprintf(hFileHandle,"</head>\n");

    fprintf(hFileHandle,"<body lang=EN-GB style='margin:0px;background-color:black'\n");
    fprintf(hFileHandle,"onclick=\"DocumentOnClick()\">\n");

    fprintf(hFileHandle,"<div id=SlideObj class=sld style='position:absolute;top:0px;left:0px;\n");
    fprintf(hFileHandle,"width:575px;height:430px;font-size:16px;background-color:white;clip:rect(0%%, 101%%, 101%%, 0%%);\n");
    fprintf(hFileHandle,"visibility:hidden'><p:slide coordsize=\"720,540\"\n");
    fprintf(hFileHandle," colors=\"#FFFFFF,#000000,#808080,#000000,#00CC99,#3333CC,#CCCCFF,#B2B2B2\"\n");
    fprintf(hFileHandle," masterhref=\"master03.xml\">\n");
    fprintf(hFileHandle," <p:shaperange href=\"master03.xml#_x0000_s1025\"/><![if !ppt]><p:shaperange\n");
    fprintf(hFileHandle,"  href=\"master03.xml#_x0000_s1028\"/><p:shaperange\n");
    fprintf(hFileHandle,"  href=\"master03.xml#_x0000_s1029\"/><![endif]><p:shaperange\n");
    fprintf(hFileHandle,"  href=\"master03.xml#_x0000_m1026\"/><v:shape id=\"_x0000_s2051\" type=\"#_x0000_m1026\"\n");
    fprintf(hFileHandle,"  style='position:absolute;left:0;top:0;width:672pt;height:66pt'>\n");
    fprintf(hFileHandle,"  <v:fill o:detectmouseclick=\"f\"/>\n");
    fprintf(hFileHandle,"  <v:stroke o:forcedash=\"f\"/>\n");
    fprintf(hFileHandle,"  <o:lock v:ext=\"edit\" text=\"f\"/>\n");
    fprintf(hFileHandle,"  <p:placeholder type=\"title\" position=\"-1\"/></v:shape>\n");
    fprintf(hFileHandle," <div v:shape=\"_x0000_s2051\" class=T style='position:absolute;top:1.75%%;\n");
    fprintf(hFileHandle," left:.93%%;width:91%%;height:9.25%%' style='font-size:%d%%'><span lang=EN-US>%s</span><span\n",(int)fFontSize,strSlideTitle.toLatin1().constData());
    fprintf(hFileHandle," style='mso-special-format:lastCR;display:none'>&#13;</span></div>\n");
    fprintf(hFileHandle," <v:shapetype id=\"_x0000_t75\" coordsize=\"21600,21600\" o:spt=\"75\"\n");
    fprintf(hFileHandle,"  o:preferrelative=\"t\" path=\"m@4@5l@4@11@9@11@9@5xe\" filled=\"f\" stroked=\"f\">\n");
    fprintf(hFileHandle,"  <v:stroke joinstyle=\"miter\"/>\n");
    fprintf(hFileHandle,"  <v:formulas>\n");
    fprintf(hFileHandle,"   <v:f eqn=\"if lineDrawn pixelLineWidth 0\"/>\n");
    fprintf(hFileHandle,"   <v:f eqn=\"sum @0 1 0\"/>\n");
    fprintf(hFileHandle,"   <v:f eqn=\"sum 0 0 @1\"/>\n");
    fprintf(hFileHandle,"   <v:f eqn=\"prod @2 1 2\"/>\n");
    fprintf(hFileHandle,"   <v:f eqn=\"prod @3 21600 pixelWidth\"/>\n");
    fprintf(hFileHandle,"   <v:f eqn=\"prod @3 21600 pixelHeight\"/>\n");
    fprintf(hFileHandle,"   <v:f eqn=\"sum @0 0 1\"/>\n");
    fprintf(hFileHandle,"   <v:f eqn=\"prod @6 1 2\"/>\n");
    fprintf(hFileHandle,"   <v:f eqn=\"prod @7 21600 pixelWidth\"/>\n");
    fprintf(hFileHandle,"   <v:f eqn=\"sum @8 21600 0\"/>\n");
    fprintf(hFileHandle,"   <v:f eqn=\"prod @7 21600 pixelHeight\"/>\n");
    fprintf(hFileHandle,"   <v:f eqn=\"sum @10 21600 0\"/>\n");
    fprintf(hFileHandle,"  </v:formulas>\n");
    fprintf(hFileHandle,"  <v:path o:extrusionok=\"f\" gradientshapeok=\"t\" o:connecttype=\"rect\"/>\n");
    fprintf(hFileHandle,"  <o:lock v:ext=\"edit\" aspectratio=\"t\"/>\n");
    fprintf(hFileHandle," </v:shapetype><v:shape id=\"_x0000_s2052\" type=\"#_x0000_t75\" style='position:absolute;\n");
    fprintf(hFileHandle,"  left:48pt;top:70pt;width:575pt;height:430pt' fillcolor=\"#0c9 [4]\"\n");
    fprintf(hFileHandle,"  strokecolor=\"black [1]\">\n");
    fprintf(hFileHandle,"  <v:fill color2=\"white [0]\"/>\n");
    fprintf(hFileHandle,"  <v:imagedata src=\"slide%04ld_image001.png\" o:title=\"\"/>\n", lSlideIndex);
    fprintf(hFileHandle,"  <v:shadow color=\"gray [2]\"/>\n");
    fprintf(hFileHandle," </v:shape></p:slide></div>\n");
    fprintf(hFileHandle,"</body>\n");
    fprintf(hFileHandle,"</html>\n");

    fclose(hFileHandle);

    // Each section will be written in this ppt_slide.htm file then converted to an image.
    OpenTemporaryHtmlPowerPoint(bOpenHtml);

    return "ok";
}
#else
QString CGexReport::WritePowerPointSlide(FILE* /*hFile*/, QString /*strSlideName*/,bool /*bOpenHtml*/)
{
    return "ok";
}
#endif

extern QString GenerateODP(QString reportFlatHtmlAbsPath, QString strDestination);

QString	CGexReport::ConvertHtmlToPowerpoint(void)
{
#ifdef GSDAEMON
    GSLOG(4, "Convert Html To Powerpoint NOT supported in daemon mode");
    return "error: PPT output NOT supported in daemon mode";
#endif

    QString r("");

#ifdef _WIN32
    CGexPptReport	clPptReport;
    QString			strDestination;
    QString			strHtmlReportFolder;
    QDir			cDir;
    int				nStatus=0;

    // Finish writing the XML list files now that all HTML slides are created
    WritePowerPointFinish();

    // HTML flat file is <path>/<query_folder_name>/pages/indexf.htm,
    // PPT output must be: <path>/<query_folder_name>.ppt
    strDestination	= BuildTargetReportName(strHtmlReportFolder,".ppt");

    cDir.remove(strDestination);	// Make sure destination doesn't exist.

    // Retrieve Powerpoint Options
    GexPptOptions	stGexPptOptions;

    // Show progress bar & messages?
    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        stGexPptOptions.m_bShowProgressBar = false;

    // Show Powerpoint?
    stGexPptOptions.m_bShowPptApplication = true;
    stGexPptOptions.m_bMinimizeApplication = true;

    // Paper size
    /*
    switch(pReportOptions->iPaperSize)
    {
        case GEX_OPTION_PAPER_SIZE_A4:			// Page size is European A4
            stGexPptOptions.m_nPaperFormat = ePaperFormatA4;
            break;
        case GEX_OPTION_PAPER_SIZE_LETTER:		// Page size is US Letter
            stGexPptOptions.m_nPaperFormat = ePaperFormatLetter;
            break;
    }
    */
    QString ps=m_pReportOptions->GetOption("output", "paper_size").toString();
    if (ps=="A4")
        stGexPptOptions.m_nPaperFormat = ePaperFormatA4;
    if (ps=="letter")
        stGexPptOptions.m_nPaperFormat = ePaperFormatLetter;

    // Paper orientation
    stGexPptOptions.m_nPaperOrientation = ePaperOrientationLandscape;	// ePaperOrientationPortrait or ePaperOrientationLandscape

    // Header & Footer: Examinator version + URL & Lot/Product info if known.
    BuildHeaderFooterText(stGexPptOptions.m_strHeader,stGexPptOptions.m_strFooter);

    // Message box to be used if we fail to create the PowerPoint file (file locked)
    QMessageBox mb( GS::Gex::Engine::GetInstance().Get("AppFullName").toString(),
        "Failed to create Slides document...maybe it's already in use?\nIf so, close PowerPoint first then try again...",
        QMessageBox::Question,
        QMessageBox::Yes | QMessageBox::Default,
        QMessageBox::No  | QMessageBox::Escape,
        0,
        0 );
    mb.setWindowIcon(QPixmap(":/gex/icons/gex_application_48x48.png"));
    mb.setButtonText( QMessageBox::Yes, "&Try again" );
    mb.setButtonText( QMessageBox::NoAll, "&Cancel" );

build_ppt_file:
    // Generate Powerpoint document
    nStatus = clPptReport.GeneratePptFromHtml(pGexMainWindow, stGexPptOptions, reportFlatHtmlAbsPath(), strDestination);

    switch(nStatus)
    {
        case CGexPptReport::Err_RemoveDestFile:
            if(stGexPptOptions.m_bShowProgressBar == false)
            {
                // Running Monitoring: then do not show dialog box, add message to log file instead...
                GS::Gex::Message::information(
                    "", "PowerPoint already in use by another process.");
                r=QString("error: destination unwritable: PowerPoint perhaps already in used by another process");
                break;
            }

            if(mb.exec() == QMessageBox::Yes)
                goto build_ppt_file;	// Try again
            break;

        case CGexPptReport::ConversionCancelled:	// Conversion process has been cancelled by the user
            r=QString("error: conversion cancelled");
            GS::Gex::Message::information(
                "", "Sides generation cancelled.\n\n"
                "A PowerPoint instance may still be "
                "active and needs to be closed.");
            break;

        case CGexPptReport::Err_ScriptError:		// Error during script execution (no PPT file generated)
            r=QString("error: script error");
            GS::Gex::Message::information(
                "", "Failed to create slides (script error).\n"
                "Check you have 'Microsoft Office' and "
                "'PowerPoint' installed!");
            #ifdef QT_DEBUG
                strDestination.chop(3); strDestination.append("odp");
                r=GenerateODP(reportFlatHtmlAbsPath(), strDestination);
                GS::Gex::Message::information("", QString("Trying to generate ODP : %1").arg(r));
                GSLOG(SYSLOG_SEV_INFORMATIONAL, r.toLatin1().data());
            #endif

            break;

        default:
            GS::Gex::Message::information("", "Error creating slides.");
            r="Error creating slides.";
            break;

        case CGexPptReport::NoError:	// No error
            r=QString("ok");
            break;
        }

    // Cleanup: erase the HTML folder created for the flat HTML file.
    GS::Gex::Engine::GetInstance().GetDatabaseEngine().DeleteFolderContent(strHtmlReportFolder);

    // Update the report file name created (from HTML file name to '.doc' file just created
    setLegacyReportName(strDestination);

#endif	// Windows only

    return r;
}

