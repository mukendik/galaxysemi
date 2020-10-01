//////////////////////////////////////////////////////////
// File: scripting_io.h
// Includes mecanism so Script functions I/O messages
// are sent to the right console window
//////////////////////////////////////////////////////////

#ifndef _GEX_SCRIPTING_IO_H
#define _GEX_SCRIPTING_IO_H

#include <QString>
#include "csl/zcsl.hpp"

class GexScripting;

// Wait until script returns exit code.
//void WaitUntilScriptCompleted(void);
//bool WaitUntilScriptCompleted_Timeout(int iTimeout);
// Saves handle to GUI window
void Scripting_IO_Init(GexScripting *pt);
// Sends string to GUI Scripting console window
void WriteScriptMessage(ZString strZtext, bool bEOL);
void WriteScriptMessage(QString strQText, bool bEOL);
// Erases the list of 'favorite scripts'
void EraseFavoriteList(void);
// Insert script to the favorite list
void InsertFavoriteList(const char *szPath, const char *szTitle);
// Get a message from the scripting console
QString GetScriptConsoleMessage(QString strField);
// Tells GEX to show the report page once script is completed
void ShowReportOnCompletion(const char *szShowPage);
// Tell GEX to set an environment variable
void SetConfigurationField(char *szSection,char *szField,char *szValue);
// Scripting: Show/Hide HTML console window
void ShowHtmlConsole(bool bShow);
// Scripting: Load page into HTML console window
void ShowHtmlPage(char *szPage);
#endif
