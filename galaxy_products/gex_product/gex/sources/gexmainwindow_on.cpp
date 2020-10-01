#include "browser_dialog.h"
#include "engine.h"
#include "pickuser_dialog.h"
#include <gqtl_log.h>
#include "report_options.h"
#include "temporary_files_manager.h"
#include "csl/csl_engine.h"
#include "command_line_options.h"

#include "message.h"

extern CReportOptions	ReportOptions;			// Holds options (report_build.h)

///////////////////////////////////////////////
///			SLOTS
///////////////////////////////////////////////


///////////////////////////////////////////////////////////
// Closing GEX application...save environment
///////////////////////////////////////////////////////////
void GexMainwindow::OnClose(void)
{
    GSLOG(SYSLOG_SEV_DEBUG, "On close...");

    mOnClose = true;
    // Save environment info into startup script file
    if (!GS::Gex::Engine::GetInstance().GetCommandLineOptions().IsHidden() && mUnsavedOptions)
    {
        bool lOk;
        GS::Gex::Message::request("", "Save profile " +
                                  GetProfileLabel(GS::Gex::Engine::GetInstance().GetStartupScript()) + "?", lOk);
        if (lOk)
        {
            OnSaveEnvironment();
        }
    }

    QString strErrorMsg = "";
    if (!saveAdditionalModules(&strErrorMsg))
        GSLOG(SYSLOG_SEV_WARNING, QString("Error while writting additional modules: '%1'")
               .arg(strErrorMsg).toLatin1().data() );
}

void GexMainwindow::OnLoadUserOptions(void)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "");

    PickUserDialog cPickUser(this);

    // Set GUI in 'Load' mode
    cPickUser.setLoadMode(true);

    if(cPickUser.exec() != 1)
        return;

    // Get username selected
    QString lUserName;
    QString lCurrentUserScript = GS::Gex::Engine::GetInstance().GetStartupScript();
    QString lSelectedUserScript;
    cPickUser.getSelectedUserName(lUserName, lSelectedUserScript);

    if (mUnsavedOptions)
    {
        bool lOk;
        GS::Gex::Message::request("", "Save profile " +
                                  GetProfileLabel(lCurrentUserScript) +
                                  " before loading " +
                                  GetProfileLabel(lSelectedUserScript) + "?", lOk);
        if (lOk)
        {
            OnSaveEnvironment();
        }
    }

    GS::Gex::Engine::GetInstance().SetStartupScript(lSelectedUserScript);

    mUnsavedOptions = false;

    // Load options: run startup script : $HOME/.<profile>.csl
    GS::Gex::CSLEngine::GetInstance().RunStartupScript(lSelectedUserScript);
}

void GexMainwindow::SaveCurrentProfile()
{
    QString lCurrentUserScript = GS::Gex::Engine::GetInstance().GetStartupScript();
    if (mUnsavedOptions)
    {
        bool lOk;
        GS::Gex::Message::request("", "Save profile " +
                                  GetProfileLabel(lCurrentUserScript) +
                                  " before loading csl profile?", lOk);
        if (lOk)
        {
            OnSaveEnvironment();
        }
    }
    mUnsavedOptions = false;
}

void GexMainwindow::LoadProfileFromScriptingCenter()
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "");

    // Get username selected
    QString lSelectedUserScript(GS::Gex::Engine::GetInstance().Get("UserFolder").toString()+ GEX_SCRIPTING_CENTER_NAME);

    GS::Gex::Engine::GetInstance().SetStartupScript(lSelectedUserScript);
    mUnsavedOptions = false;

    // Load options: run startup script : $HOME/.<profile>.csl
    GS::Gex::CSLEngine::GetInstance().RunStartupScript(lSelectedUserScript);
}

void GexMainwindow::OnSaveUserOptions(void)
{
    GSLOG(SYSLOG_SEV_WARNING, "on save user options...");
    mUnsavedOptions = false;
    PickUserDialog cPickUser(this);
    // Set GUI in 'Save as...' mode
    cPickUser.setLoadMode(false);
    // Display popup
    if(cPickUser.exec() != 1)
        return;

    // Get username selected
    QString lUserName;
    QString lUserScript = GS::Gex::Engine::GetInstance().GetStartupScript();
    cPickUser.getSelectedUserName(lUserName, lUserScript);

    GS::Gex::Engine::GetInstance().SetStartupScript(lUserScript);

    // Save options
    OnSaveEnvironment();

    // Force reload so all changes are effective
    GS::Gex::CSLEngine::GetInstance().RunStartupScript(GS::Gex::Engine::GetInstance().GetStartupScript());
}

void GexMainwindow::OnDefaultOptions(void)
{
    // Confirm reset of all options to 'default' settings...
    bool lOk;
    GS::Gex::Message::request("", "Confirm to reset all options to default?", lOk);
    if (! lOk)
    {
        return;
    }

    mUnsavedOptions = true;
    // Reset options to default
    ReportOptions.Reset(true);

    GS::Gex::OptionsHandler optionsHandler(GS::Gex::Engine::GetInstance().GetOptionsHandler());
    optionsHandler.Reset(true);
    GS::Gex::Engine::GetInstance().SetOptionsMap(optionsHandler.GetOptionsMap());

    // Update GUI based on reset options
    //RefreshOptionsEdits();
    if(!RefreshOptionsCenter())
        GEX_ASSERT(false);
}

GexMainwindow::analyseMode GexMainwindow::GetAnalyseMode() const
{
    return mAnalyseMode;
}

void GexMainwindow::SetAnalyseMode(const GexMainwindow::analyseMode &analyseMode)
{
    mAnalyseMode = analyseMode;
}
