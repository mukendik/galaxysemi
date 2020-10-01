#include <LogFile.h>
#include <stdio.h>
#include <stdlib.h>

bool bRtfLoaded=false;
FILE* RtfFile=NULL;
std::string sRtfFilename;

//Open log file
bool RtfInit (const std::string& sFile)
{
    //Make sure log file is not already open
	if (bRtfLoaded)
		return false;

    //Open file
	RtfFile=fopen(sFile.c_str(), "w");
	if (!RtfFile)	//(hFile == INVALID_HANDLE_VALUE)
		return false;

	atexit(RtfClose);

    //Set file loaded flag
	bRtfLoaded = true;

    //Set filename
	sRtfFilename = sFile;

    //Write RTF header
	RtfWriteString ("{\\rtf1\\ansi\\deff0{\\fonttbl{\\f0 Courier New;}}\\fs20\n");

    //Write colour table
	RtfWriteString ("{\\colortbl;\n");                     //Black
	RtfWriteString ("\\red255\\green255\\blue255;\n");     //White
	RtfWriteString ("\\red128\\green128\\blue128;\n");     //Grey
	RtfWriteString ("\\red255\\green0\\blue0;\n");         //Red
	RtfWriteString ("\\red0\\green255\\blue0;\n");         //Green
	RtfWriteString ("\\red0\\green0\\blue255;\n");         //Blue
	RtfWriteString ("\\red0\\green255\\blue255;\n");       //Cyan
	RtfWriteString ("\\red255\\green255\\blue0;\n");       //Yellow
	RtfWriteString ("\\red255\\green0\\blue255;\n");       //Magenta
	RtfWriteString ("\\red128\\green0\\blue0;\n");         //Dark Red
	RtfWriteString ("\\red0\\green128\\blue0;\n");         //Dark Green
	RtfWriteString ("\\red0\\green0\\blue128;\n");         //Dark Blue
	RtfWriteString ("\\red255\\green128\\blue128;\n");     //Light Red
	RtfWriteString ("\\red128\\green255\\blue128;\n");     //Light Green
	RtfWriteString ("\\red128\\green128\\blue255;\n");     //Light Blue
	RtfWriteString ("\\red255\\green128\\blue0;\n");       //Orange
	RtfWriteString ("}\n");

    //Write log header
	RtfLogString ("*** Logging Started");
	RtfLogString ("");

    //Success
	return true;
}

//Close log file
void RtfClose()
{
	#ifdef QT_DEBUG
	 printf("RTFClose...\n");
	#endif

    //Make sure log file is already opened
	if (!bRtfLoaded)
		return;

    //Write closing line
	RtfLogString("");
	RtfLogString("*** Logging Ended");

    //Write closing brace
	RtfWriteString ("}");

    //Close file
	if (RtfFile)
		fclose(RtfFile);

	RtfFile=NULL;

    //Clear file loaded flag
	bRtfLoaded = false;

	return;
}


//Get log file state
bool RtfGetState()
{
	if (bRtfLoaded)
		return true;
    else
		return false;
}


//Format and log a string
bool RtfLogString (std::string sText)
{
    //Make sure log file is already opened
	if (!bRtfLoaded)
		return false;

    //Format string
	RtfDoFormatting (&sText);

    //Write string to file
	RtfWriteString(sText);

    //Success
	return true;
}


//Write a specially formatted message to the log file
int RtfLogMessage(const std::string &sTime,
				  const std::string &sSource,
				  const std::string& sMessage,
				  SYSLOG_SEV messageType)
{
	std::string sString;

    switch (messageType)
    {
		case SYSLOG_SEV_ALERT:
		case SYSLOG_SEV_EMERGENCY:	//MSG_ERROR:
		   sString = "[RED]";
		break;

		case SYSLOG_SEV_CRITICAL:
		case SYSLOG_SEV_ERROR:
		  sString = "[ORANGE]";
		break;

		case SYSLOG_SEV_WARNING:	//MSG_WARN:
		case SYSLOG_SEV_NOTICE:
			//case MSG_SUCCESS:
		  sString = "[BLUE]";
        break;

		case SYSLOG_SEV_INFORMATIONAL: sString = "[BLACK]"; break;

		case SYSLOG_SEV_DEBUG:	//MSG_INFO:
		 sString = "[GREY]";
		break;
    }

	sString += "[B]<"; sString += sTime; sString += ">[/B] ";
	sString += sMessage;
	sString += "[I]<"; sString += sSource; sString += ">[/I] ";

    //Format the string and write it to the log file
	return RtfLogString (sString);
}


//Replace formatting tags in a string with RTF formatting strings
void RtfDoFormatting(std::string* sText)
{
    //Fix special symbols {, }, and backslash
	RtfReplaceTag (sText, "\\", "\\\\");
	RtfReplaceTag (sText, "{", "\\{");
	RtfReplaceTag (sText, "}", "\\}");

    //Colours
	RtfReplaceTag (sText, "[BLACK]", "\\cf0 ");
	RtfReplaceTag (sText, "[WHITE]", "\\cf1 ");
	RtfReplaceTag (sText, "[GREY]", "\\cf2 ");
	RtfReplaceTag (sText, "[RED]", "\\cf3 ");
	RtfReplaceTag (sText, "[GREEN]", "\\cf4 ");
	RtfReplaceTag (sText, "[BLUE]", "\\cf5 ");
	RtfReplaceTag (sText, "[CYAN]", "\\cf6 ");
	RtfReplaceTag (sText, "[YELLOW]", "\\cf7 ");
	RtfReplaceTag (sText, "[MAGENTA]", "\\cf8 ");
	RtfReplaceTag (sText, "[DARK_RED]", "\\cf9 ");
	RtfReplaceTag (sText, "[DARK_GREEN]", "\\cf10 ");
	RtfReplaceTag (sText, "[DARK_BLUE]", "\\cf11 ");
	RtfReplaceTag (sText, "[LIGHT_RED]", "\\cf12 ");
	RtfReplaceTag (sText, "[LIGHT_GREEN]", "\\cf13 ");
	RtfReplaceTag (sText, "[LIGHT_BLUE]", "\\cf14 ");
	RtfReplaceTag (sText, "[ORANGE]", "\\cf15 ");

    //Text style
	RtfReplaceTag (sText, "[PLAIN]", "\\plain ");
	RtfReplaceTag (sText, "[B]", "\\b ");
	RtfReplaceTag (sText, "[/B]", "\\b0 ");
	RtfReplaceTag (sText, "[I]", "\\i ");
	RtfReplaceTag (sText, "[/I]", "\\i0 ");
	RtfReplaceTag (sText, "[U]", "\\ul ");
	RtfReplaceTag (sText, "[/U]", "\\ul0 ");
	RtfReplaceTag (sText, "[S]", "\\strike ");
	RtfReplaceTag (sText, "[/S]", "\\strike0 ");
	RtfReplaceTag (sText, "[SUB]", "\\sub ");
	RtfReplaceTag (sText, "[/SUB]", "\\sub0 ");
	RtfReplaceTag (sText, "[SUPER]", "\\super ");
	RtfReplaceTag (sText, "[/SUPER]", "\\super0 ");
	RtfReplaceTag (sText, "[LEFT]", "\\ql ");
	RtfReplaceTag (sText, "[RIGHT]", "\\qr ");
	RtfReplaceTag (sText, "[CENTER]", "\\qc ");
	RtfReplaceTag (sText, "[FULL]", "\\qj ");
	RtfReplaceTag (sText, "[TITLE]", "\\fs40 ");
	RtfReplaceTag (sText, "[/TITLE]", "\\fs20 ");
	RtfReplaceTag (sText, "[H]", "\\fs24 ");
	RtfReplaceTag (sText, "[/H]", "\\fs20 ");

    //Add brackets and line formatting tags
    sText->insert (0, "{\\pard ");
    sText->insert (sText->length (), "\\par}\n");
}


//Replace all instances of a formatting tag with the RTF equivalent
void RtfReplaceTag(std::string* sText, const std::string& sTag, const std::string& sReplacement)
{
    int start = 0;

	while (sText->find(sTag, start) != std::string::npos)
    {
        start = (int) sText->find(sTag, start);
        sText->replace (start, sTag.length (), sReplacement);
        start += (int) sTag.length () + 1;
    }
}

//Write a string directly to the log file
void RtfWriteString(const std::string& sText)
{
	//DWORD bytesWritten;
	//WriteFile (hFile, sText.c_str (), (int) sText.length (), &bytesWritten, NULL);
	if (RtfFile)
	 fprintf(RtfFile,"%s" ,sText.c_str());
}
