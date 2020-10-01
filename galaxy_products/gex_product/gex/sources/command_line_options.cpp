#include "command_line_options.h"
#include "product_info.h"
#include "gqtl_log.h"
#include "gex_constants.h"
#include "gex_oem_constants.h"

#include <QStringList>
#include <QFileInfo>
#include <QStyleFactory>

extern QString				strIniClientSection;

namespace GS
{
namespace Gex
{

CommandLineOptions::CommandLineOptions(QObject *parent)
    : QObject(parent), mHidden(false), mCloseAfterScript(false), mCustomDebugMode(false),
      mDisableErrorBox(false), mWelcomeBox(false), mProduct(GS::LPPlugin::LicenseProvider::eExaminator)
{
    setObjectName("GSCommandLineOptions");
}

CommandLineOptions::~CommandLineOptions()
{

}

bool CommandLineOptions::ParseArguments(int argc, char **argv)
{
    QStringList arguments;

    for(int lIdx = 0; lIdx < argc; ++lIdx)
    {
        if (argv[lIdx])
            arguments.append(argv[lIdx]);
    }

    bool lR = ParseArguments(arguments);

    /////////////////// FORCE OEM-LTXC  running mode.
#ifdef OEM_LTXC
    // Force running mode to be: 'OEM-Examinator for LTXC'
    mProduct = GS::LPPlugin::LicenseProvider::eLtxcOEM;
#endif
#ifdef TER_GEX
    // If Teradyne package and no Teradyne mode has been forced through "-TER" or "-TERPRO" arguments,
    // force running mode to be: 'OEM Examinator for Teradyne'
    if((mProduct != GS::LPPlugin::LicenseProvider::eTerOEM) && (mProduct != GS::LPPlugin::LicenseProvider::eTerProPlus))
        mProduct = GS::LPPlugin::LicenseProvider::eTerOEM;
#endif

     return lR;
}

bool CommandLineOptions::ParseArguments(const QStringList& arguments)
{
    // Creates the argc,argv[] list
    mArguments = arguments;

    GSLOG(SYSLOG_SEV_DEBUG,
          QString("Check Command Line options: %1")
          .arg(mArguments.join(" "))
          .toLatin1().data() );

    // Defaults
    mHidden             = false;
    mCloseAfterScript   = false;
    mCustomDebugMode    = false;
    mDisableErrorBox    = false;
    mWelcomeBox         = false;
    mProduct            = GS::LPPlugin::LicenseProvider::eExaminator;
    mRunScript.clear();
    mProfileScript.clear();
    mStartupDataFile.clear();
    mTriggerFile.clear();

    strIniClientSection = "Client";

    // No command line option specified
    if(mArguments.count() == 0)
        return true;

#ifdef GSDAEMON

    QString lExecName = QFileInfo(mArguments.first()).baseName();

    // When running from the development environment, the executable name has not been renamed.
    // Depending on the build mode (release or debug) the name daemon executable name could be either
    // gsd or gsdd.
    // In development environment, we support an extra argument, in addition of the ones used by the
    // service controller, which is used to choose the running product
    //      -GYM    Launch the Yield-Man daemon
    //      -GYME   Launch the Yield-Man-Enterprise daemon
    //      -GPM    Launch the PAT-Man daemon
    //      -GPME   Launch the PAT-Man-Enterprise daemon
    if (lExecName.compare("gsdd", Qt::CaseInsensitive) == 0 ||
        lExecName.compare("gsd", Qt::CaseInsensitive) == 0)
    {
        // Default mode is PAT-Man
        mProduct    = GS::LPPlugin::LicenseProvider::ePATMan;

        if (mArguments.count() > 1)
        {
            QString lArgument = mArguments.at(1);

            // Check that the first argument is correct
            // It should be either an argument used by the service controller or
            // an argument to select the product running.
            if(lArgument != "-i" && lArgument != "-install" &&
               lArgument != "-u" && lArgument != "-uninstall" &&
               lArgument != "-e" && lArgument != "-exec" &&
               lArgument != "-t" && lArgument != "-terminate" &&
               lArgument != "-p" && lArgument != "-pause" &&
               lArgument != "-r" && lArgument != "-resume" &&
               lArgument.compare("-GPM", Qt::CaseInsensitive) &&
               lArgument.compare("-GYM", Qt::CaseInsensitive) &&
               lArgument.compare("-GTM", Qt::CaseInsensitive) &&
               lArgument.compare("-GYME", Qt::CaseInsensitive) &&
               lArgument.compare("-GPME", Qt::CaseInsensitive)
                    )
            {
                GSLOG(SYSLOG_SEV_CRITICAL,
                      QString("Argument %1 is not supported for Quantix Daemon: %2")
                      .arg(lArgument).arg(lExecName).toLatin1().constData());
                return false;
            }

            if (mArguments.count() == 2)
            {
                // Only one argument passed in command line
                // Check it to select the product, if it doesn't match with the product arguments
                // allowed, it means that the argument is for the service controller
                if (mArguments.at(1).compare("-GPM", Qt::CaseInsensitive) == 0)
                    mProduct    = GS::LPPlugin::LicenseProvider::ePATMan;
                else if (mArguments.at(1).compare("-GYM", Qt::CaseInsensitive) == 0)
                    mProduct    = GS::LPPlugin::LicenseProvider::eYieldMan;
                else if (mArguments.at(1).compare("-GTM", Qt::CaseInsensitive) == 0)
                    mProduct    = GS::LPPlugin::LicenseProvider::eGTM;
                else if (mArguments.at(1).compare("-GYME", Qt::CaseInsensitive) == 0)
                    mProduct    = GS::LPPlugin::LicenseProvider::eYieldManEnterprise;
                else if (mArguments.at(1).compare("-GPME", Qt::CaseInsensitive) == 0)
                    mProduct    = GS::LPPlugin::LicenseProvider::ePATManEnterprise;
            }
            else if (mArguments.count() <= 4)
            {
                // Two or three  arguments passed in command line
                // The first one is for the service controller,
                // The second one is used to select the product
                // If the product arguments doesn't match with the allowed string,
                // Stops the parsing as the arguments are invalid
                if (mArguments.at(2).compare("-GPM", Qt::CaseInsensitive) == 0)
                    mProduct    = GS::LPPlugin::LicenseProvider::ePATMan;
                else if (mArguments.at(2).compare("-GYM", Qt::CaseInsensitive) == 0)
                    mProduct    = GS::LPPlugin::LicenseProvider::eYieldMan;
                else if (mArguments.at(2).compare("-GTM", Qt::CaseInsensitive) == 0)
                    mProduct = GS::LPPlugin::LicenseProvider::eGTM;
                else if (mArguments.at(2).compare("-GYME", Qt::CaseInsensitive) == 0)
                    mProduct    = GS::LPPlugin::LicenseProvider::eYieldManEnterprise;
                else if (mArguments.at(2).compare("-GPME", Qt::CaseInsensitive) == 0)
                    mProduct    = GS::LPPlugin::LicenseProvider::ePATManEnterprise;
                else
                {
                    GSLOG(SYSLOG_SEV_CRITICAL,
                          QString("%1 is not a valid argument for Quantix Semi Daemon.")
                          .arg(mArguments.at(2)).toLatin1().constData());

                    return false;
                }

                if (mArguments.count() == 4 && mArguments.at(3).startsWith("-uselp", Qt::CaseInsensitive) == false) {
                    GSLOG(SYSLOG_SEV_CRITICAL, QString("%1 is not a valid argument for Quantix Semi Daemon.").arg(mArguments.at(3)).toLatin1().constData());
                    return false;
                }

            }
            else
            {
                // The service gsd or gsdd only support up to 2 arguments
                // If there is more than two, stops parsing as the arguments passed are not
                // supported.
                GSLOG(SYSLOG_SEV_CRITICAL,
                      QString("Too many arguments for Quantix Semi Daemon: %1").arg(lExecName)
                      .toLatin1().constData());

                return false;
            }
        }
    }
    else
    {
        // When running from the production environment, the executable name is used to know
        // which product we are running:
        //      gs-ymd executable name means we are running the Yield-Man daemon
        //      gs-pmd executable name means we are running the PAT-Man daemon
        //      gs-gtmd executable name means we are running the GTM daemon
        //
        // If the executable name is different from the three above, we have to reject the parsing
        // as we don't know which product is running

        if (lExecName.compare("gs-ymd", Qt::CaseInsensitive) == 0)
        {
            mProduct    = GS::LPPlugin::LicenseProvider::eYieldMan;
        }
        else if (lExecName.compare("gs-pmd", Qt::CaseInsensitive) == 0)
        {
            mProduct    = GS::LPPlugin::LicenseProvider::ePATMan;
        }
        else if (lExecName.compare("gs-gtmd", Qt::CaseInsensitive) == 0)
        {
            mProduct    = GS::LPPlugin::LicenseProvider::eGTM;
        }
        else
        {
            GSLOG(SYSLOG_SEV_CRITICAL,
              QString("%1 is not a valid executable name.")
                  .arg(lExecName).toLatin1().constData());
            return false;
        }

        // The daemon running in production environment is started:
        // 1) EITHER with 1 service argument (-i, -u, -t...)
        // 2) OR with some startup arguments:
        //    o "-platform minimal" (2 arguments, MANDATORY)
        //    o startup mode: -GPM, -GYM, -GTM, -GPME, -GYME (OPTIONAL)
        if (mArguments.count() == 2)
        {
            QString lArgument = mArguments.at(1);

            if(lArgument != "-i" && lArgument != "-install" &&
               lArgument != "-u" && lArgument != "-uninstall" &&
               lArgument != "-e" && lArgument != "-exec" &&
               lArgument != "-t" && lArgument != "-terminate" &&
               lArgument != "-p" && lArgument != "-pause" &&
               lArgument != "-r" && lArgument != "-resume")
            {
                GSLOG(SYSLOG_SEV_CRITICAL,
                      QString("Argument %1 is not supported in Daemon mode (%2)")
                      .arg(lArgument).arg(lExecName).toLatin1().constData());
                return false;
            }
        }
        else
        {
            // Make sure "-platform minimal" is passed as an argument
            bool lPlatformArg = false;

            for(int lIdx = 1; lIdx < mArguments.count(); ++lIdx)
            {
                QString lArgument = mArguments.at(lIdx);

                GSLOG(SYSLOG_SEV_DEBUG,
                      QString("Arg %1 : '%2'").arg(lIdx).arg(lArgument).toLatin1().data());

                // Daemon product selection argument?
                if(lArgument.compare("-GYM", Qt::CaseInsensitive) == 0)
                {
                    mProduct = GS::LPPlugin::LicenseProvider::eYieldMan;
                }
                else if(lArgument.compare("-GYME", Qt::CaseInsensitive) == 0)
                {
                    mProduct = GS::LPPlugin::LicenseProvider::eYieldManEnterprise;
                }
                else if(lArgument.compare("-GPM", Qt::CaseInsensitive) == 0)
                {
                    mProduct = GS::LPPlugin::LicenseProvider::ePATMan;
                }
                else if(lArgument.compare("-GPME", Qt::CaseInsensitive) == 0)
                {
                    mProduct = GS::LPPlugin::LicenseProvider::ePATManEnterprise;
                }
                else if(lArgument.compare("-GTM", Qt::CaseInsensitive) == 0)
                {
                    mProduct = GS::LPPlugin::LicenseProvider::eGTM;
                }
                else if(lArgument.compare("-platform", Qt::CaseInsensitive) == 0)
                {
                    // Next argument must be minimal
                    if(++lIdx < mArguments.count())
                    {
                        lArgument = mArguments.at(lIdx);
                        if(lArgument.compare("minimal", Qt::CaseInsensitive) != 0)
                        {
                            GSLOG(SYSLOG_SEV_CRITICAL,
                                  QString("Daemon mode must be run with \"-platform minimal\" argument")
                                  .toLatin1().constData());
                            return false;
                        }
                        lPlatformArg = true;
                    }
                }
                else if(lArgument.startsWith("-uselp", Qt::CaseInsensitive) == false)
                {
                    GSLOG(SYSLOG_SEV_CRITICAL,
                          QString("Argument %1 not supported in Daemon mode (%2)").arg(lArgument).arg(lExecName)
                          .toLatin1().constData());
                    return false;
                }
            }
            if(!lPlatformArg)
            {
                GSLOG(SYSLOG_SEV_CRITICAL,
                      QString("Daemon mode must be run with \"-platform minimal\" argument")
                      .toLatin1().constData());
                return false;
            }
        }
    }
#else

    // No command line option specified
    if(mArguments.count() < 2)
        return true;

    QStringList lArguments;

    // First check product startup arguments
    for(int lIdx = 1; lIdx < mArguments.count(); ++lIdx)
    {
        QString lArgument = mArguments.at(lIdx);

        GSLOG(SYSLOG_SEV_DEBUG,
              QString("Arg %1 : '%2'").arg(lIdx).arg(lArgument).toLatin1().data());

        // Not a product startup argument, so keep it for next parsing
        if (CheckStartupArguments(lArgument) == false)
            lArguments.append(lArgument);
    }

    // Regular expression to check argument for Hide, Close after script and Script
    QRegExp lRegExp("^-[HSC]{1,3}", Qt::CaseInsensitive);

    for(int lIdx = 0; lIdx < lArguments.count(); ++lIdx)
    {
        QString lArgument = lArguments.at(lIdx);

        if (lArgument.startsWith("-style", Qt::CaseInsensitive))
        {
            QStringList lStyleKeys = QStyleFactory::keys();
            // check if we have another argument after -style
            // and that it exists for that OS
            if (!(((lIdx + 1) < lArguments.count()) &&
                    (lStyleKeys.contains(lArguments.at(lIdx + 1), Qt::CaseInsensitive))))
             // wrong style argument trigger a warning
            {
                GSLOG(SYSLOG_SEV_WARNING, QString("Error: invalid style argument, available styles are: %1")
                      .arg(lStyleKeys.join(" ")).toLatin1().constData());
            }
            else
            {
                ++lIdx;// ignore that argument for next check
            }
        }
        else if (lArgument.startsWith("-profile=", Qt::CaseInsensitive))
        {
            mProfileScript = lArgument.section("-profile=", 1);

            if (mProfileScript.isEmpty())
            {
                GSLOG(SYSLOG_SEV_CRITICAL, "Error : illegal empty profile argument");
                return false;
            }

            GSLOG(SYSLOG_SEV_NOTICE,
                  QString("Profile file found in arguments list: %1")
                  .arg(mProfileScript).toLatin1().data());

            if (!QFile::exists(mProfileScript))
            {
                GSLOG(SYSLOG_SEV_CRITICAL,
                      QString("Error : Profile file '%1'' does not exist").
                      arg(mProfileScript).toLatin1().constData());
                return false;
            }
        }
        else if (lArgument.startsWith("-trigger=", Qt::CaseInsensitive) )
        {
            mTriggerFile= lArgument.section("-trigger=", 1);
            if (mTriggerFile.isEmpty())
            {
                GSLOG(SYSLOG_SEV_CRITICAL, "Error : illegal empty trigger argument");
                return false;
            }

            GSLOG(SYSLOG_SEV_NOTICE,
                  QString("Trigger file found in arguments list: %1")
                  .arg(mTriggerFile).toLatin1().data());

            if (!QFile::exists(mTriggerFile))
            {
                GSLOG(SYSLOG_SEV_CRITICAL,
                      QString("Error : trigger file '%1'' does not exist").
                      arg(mTriggerFile).toLatin1().constData());
                return false;
            }
        }
        else if (lArgument.compare("-d", Qt::CaseInsensitive) == 0)
        {
            mCustomDebugMode = true;
        }
        else if (lArgument.compare("-noErrorBox", Qt::CaseInsensitive) == 0)
        {
            mDisableErrorBox = true;
        }
        else if (lRegExp.exactMatch(lArgument))
        {
            if(lArgument.contains('C', Qt::CaseInsensitive))
            {
                GSLOG(SYSLOG_SEV_DEBUG, "Check command line options: Close after script activated");
                mCloseAfterScript = true;
            }

            if(lArgument.contains('H', Qt::CaseInsensitive))
            {
                GSLOG(SYSLOG_SEV_NOTICE, "Check command line options: Running Hidden (-H option)");
                mHidden = true;
            }

            // Check for: Run Script 'argv[2]'
            if((mProduct != GS::LPPlugin::LicenseProvider::eGTM) && lArgument.contains('S', Qt::CaseInsensitive))
            {
                if(lIdx+1 < lArguments.count())
                    mRunScript = lArguments.at(++lIdx);

                if (mRunScript.isEmpty())
                {
                    GSLOG(SYSLOG_SEV_CRITICAL, "Error : illegal empty run script argument");
                    return false;
                }

                GSLOG(SYSLOG_SEV_NOTICE,
                      QString("Run script file found in arguments list: %1")
                      .arg(mRunScript).toLatin1().data());

                if (!QFile::exists(mRunScript))
                {
                    GSLOG(SYSLOG_SEV_CRITICAL,
                          QString("Error : file '%1' does not exist").
                          arg(mRunScript).toLatin1().constData());
                    return false;
                }
            }
        }
        else if( (mProduct != GS::LPPlugin::LicenseProvider::eGTM) &&
                 (lArgument.startsWith('-') == false) &&
                 (mRunScript.isEmpty()))
        {
        // Check if STDF specified in argument....and no script activated
            if(QFile::exists(lArgument))
            {
                // Save path into global variable (to be processed AFTER startup script is completed)
                mStartupDataFile = lArgument;
            }
        }
        else if (lArgument.startsWith("-uselp=", Qt::CaseInsensitive) == false)
        {
            GSLOG(SYSLOG_SEV_WARNING,
                  QString("Wrong argument used in the command line: %1").arg(lArgument)
                  .toLatin1().constData());
        }
    }
#endif

    return true;
}

bool CommandLineOptions::IsHidden() const
{
    return mHidden;
}

bool CommandLineOptions::IsCustomDebugMode() const
{
    return mCustomDebugMode;
}

bool CommandLineOptions::IsErrorBoxDisabled() const
{
    return mDisableErrorBox;
}

bool CommandLineOptions::IsWelcomeBoxEnabled() const
{
    return mWelcomeBox;
}

bool CommandLineOptions::CloseAfterRunScript() const
{
    return mCloseAfterScript;
}

GS::LPPlugin::LicenseProvider::GexProducts CommandLineOptions::GetProduct() const
{
    return mProduct;
}

const QString &CommandLineOptions::GetRunScript() const
{
    return mRunScript;
}

const QString &CommandLineOptions::GetProfileScript() const
{
    return mProfileScript;
}

const QString &CommandLineOptions::GetStartupDataFile() const
{
    return mStartupDataFile;
}

const QString &CommandLineOptions::GetTriggerFile() const
{
    return mTriggerFile;
}

const QStringList &CommandLineOptions::GetArguments() const
{
    return mArguments;
}

//Actual status for gex products
//Examinator	"-W ou empty argument"
//Examinator-Pro	"-WPRO" ,"-GEXPRO"
//Examinator-PAT	"-WPROPAT" , "-GEXPROPAT"
//Yield-Man	"-WYM", "-GEXYM" , "-GYM", "-WYME", "-GYME"
//PAT-Man	"-WPM", "-GEXPM", "-GPM", "-WPME", "-GPME"
//GTM	"-WGTM", "-GTM"
bool CommandLineOptions::CheckStartupArguments(const QString &argument)
{
    if(argument.compare("-W", Qt::CaseInsensitive) == 0)
    {
        mWelcomeBox = true;
    }
    else if(argument.compare("-WDB", Qt::CaseInsensitive) == 0 ||
            argument.compare("-WPRO", Qt::CaseInsensitive) == 0)
    {
        // Launching 'Examinator-Pro' Welcome page
        mWelcomeBox = true;
        mProduct    = GS::LPPlugin::LicenseProvider::eExaminatorPro;

        // Build .INI Client section header (allowing multiple different license manager connections)
        strIniClientSection = "Client-DB";
    }
    else if(argument.compare("-WDBPAT", Qt::CaseInsensitive) == 0 ||
            argument.compare("-WPROPAT", Qt::CaseInsensitive) == 0)
    {
        // Launching 'Examinator-PAT' Welcome page
        mWelcomeBox     = true;
        mProduct        = GS::LPPlugin::LicenseProvider::eExaminatorPAT;

        // Build .INI Client section header (allowing multiple different license manager connections)
        strIniClientSection = "Client-DBPAT";
    }
    else if(argument.compare("-GEXDB", Qt::CaseInsensitive) == 0 ||
            argument.compare("-GEXPRO", Qt::CaseInsensitive) == 0)
    {
        // Force running mode to be: 'Examinator-Pro'
        mProduct        = GS::LPPlugin::LicenseProvider::eExaminatorPro;

        // Build .INI Client section header (allowing multiple different license manager connections)
        strIniClientSection = "Client-DB";
    }
    else if(argument.compare("-GEXDBPAT", Qt::CaseInsensitive) == 0 ||
            argument.compare("-GEXPROPAT", Qt::CaseInsensitive) == 0)
    {
        // Force running mode to be: 'Examinator-Pro + PAT support'
        mProduct        = GS::LPPlugin::LicenseProvider::eExaminatorPAT;

        // Build .INI Client section header (allowing multiple different license manager connections)
        strIniClientSection = "Client-DBPAT";
    }
    else if(argument.compare("-WMO", Qt::CaseInsensitive) == 0 ||
            argument.compare("-WYM", Qt::CaseInsensitive) == 0)
    {
        // Launching 'Yield-Man' Welcome page
        mWelcomeBox     = true;
        mProduct        = GS::LPPlugin::LicenseProvider::eYieldMan;
    }
    else if(argument.compare("-WYME", Qt::CaseInsensitive) == 0)
    {
        // Launching 'Yield-Man-Enterprise' Welcome page
        mWelcomeBox     = true;
        mProduct        = GS::LPPlugin::LicenseProvider::eYieldManEnterprise;
    }
    else if(argument.compare("-WPAT", Qt::CaseInsensitive) == 0 ||
            argument.compare("-WPM", Qt::CaseInsensitive) == 0)
    {
        // Launching 'PAT-Man' Welcome page
        mWelcomeBox     = true;
        mProduct        = GS::LPPlugin::LicenseProvider::ePATMan;
    }
    else if(argument.compare("-WPME", Qt::CaseInsensitive) == 0)
    {
        // Launching 'PAT-Man-Enterprise' Welcome page
        mWelcomeBox     = true;
        mProduct        = GS::LPPlugin::LicenseProvider::ePATManEnterprise;
    }
    else if(argument.compare("-WGTM", Qt::CaseInsensitive) == 0)
    {
        // Launching 'GTM' Welcome page
        mWelcomeBox     = true;
        mProduct        = GS::LPPlugin::LicenseProvider::eGTM;
    }
    else if(argument.compare("-GEXMO", Qt::CaseInsensitive) == 0 ||
            argument.compare("-GEXYM", Qt::CaseInsensitive) == 0 ||
            argument.compare("-GYM", Qt::CaseInsensitive) == 0)
    {
        // Force running mode to be: 'Yield-Man'
        mProduct        = GS::LPPlugin::LicenseProvider::eYieldMan;
    }
    else if(argument.compare("-GYME", Qt::CaseInsensitive) == 0)
    {
        // Force running mode to be: 'Yield-Man-Enterprise'
        mProduct        = GS::LPPlugin::LicenseProvider::eYieldManEnterprise;
    }
    else if(argument.compare("-GEXMOPAT", Qt::CaseInsensitive) == 0 ||
            argument.compare("-GEXPM", Qt::CaseInsensitive) == 0 ||
            argument.compare("-GPM", Qt::CaseInsensitive) == 0)
    {
        // Force running mode to be: PAT-Man'
        mProduct        = GS::LPPlugin::LicenseProvider::ePATMan;
    }
    else if(argument.compare("-GPME", Qt::CaseInsensitive) == 0)
    {
        // Force running mode to be: PAT-Man-Enterprise'
        mProduct        = GS::LPPlugin::LicenseProvider::ePATManEnterprise;
    }
    else if(argument.compare("-GTM", Qt::CaseInsensitive) == 0)
    {
        mProduct        = GS::LPPlugin::LicenseProvider::eGTM;
    }
    else if(argument.compare("-WTB", Qt::CaseInsensitive) == 0)
    {
        // Launching 'ExaminatorToolBox' Welcome page
        mWelcomeBox     = true;
        mProduct        = GS::LPPlugin::LicenseProvider::eExaminator;

        // Build .INI Client section header (allowing multiple different license manager connections)
        strIniClientSection = "Client-TB";
    }
    else if(argument.compare("-GEXTB", Qt::CaseInsensitive) == 0)
    {
        // Force running mode to be: 'ExaminatorToolBox'
        mProduct        = GS::LPPlugin::LicenseProvider::eExaminator;

        // Build .INI Client section header (allowing multiple different license manager connections)
        strIniClientSection = "Client-TB";
    }
#ifdef TER_GEX
    else if(argument.compare("-TER", Qt::CaseInsensitive) == 0)
    {
        // Force running mode to be: 'OEM Examinator for Teradyne'
        mProduct    = GS::LPPlugin::LicenseProvider::eTerOEM;
    }
    else if(argument.compare("-TERPRO", Qt::CaseInsensitive) == 0)
    {
        // Force running mode to be: 'OEM Examinator-Pro for Teradyne'
        mProduct    = GS::LPPlugin::LicenseProvider::eTerProPlus;
    }
#endif
    else
    {
        // SPECIAL COMMAND LINE called by LTX Wrapper to force Examinator to run in LTX support mode without license check.
        QString lLtxOEMCommandLine;
        lLtxOEMCommandLine =    GEX_OEM_CMDLINE_PREFIX;
        lLtxOEMCommandLine +=   GEX_OEM_LTX_CMDLINE;
        lLtxOEMCommandLine +=   GEX_OEM_CMDLINE_SUFFIX;

/*
        QString lSzOEMCommandLine;
        lSzOEMCommandLine  =    GEX_OEM_CMDLINE_PREFIX;
        lSzOEMCommandLine  +=   GEX_OEM_SZ_CMDLINE;
        lSzOEMCommandLine  +=   GEX_OEM_CMDLINE_SUFFIX;

        QString lAslOEMCommandLine;
        lAslOEMCommandLine =    GEX_OEM_CMDLINE_PREFIX;
        lAslOEMCommandLine +=   GEX_OEM_CMDLINE_PREFIX;
        lAslOEMCommandLine +=   GEX_OEM_CMDLINE_SUFFIX;

        QString lSapphireOEMCommandLine;
        lSapphireOEMCommandLine =   GEX_OEM_CMDLINE_PREFIX;
        lSapphireOEMCommandLine +=  GEX_OEM_SAPPHIRE_CMDLINE;
        lSapphireOEMCommandLine +=  GEX_OEM_CMDLINE_SUFFIX;
*/

        // SPECIAL COMMAND LINE called by LTXC Wrapper to force Examinator to run in LTX support mode without license check.
        if(lLtxOEMCommandLine == argument)
        {
            // Force running mode to be: 'OEM-Examinator for LTX'
            mProduct    = GS::LPPlugin::LicenseProvider::eLtxcOEM;
        }
        /*
        // SPECIAL COMMAND LINE called by SZ Wrapper to force Examinator to run in SZ support mode without license check.
        else if(lSzOEMCommandLine == argument)
        {
            // Force running mode to be: 'OEM-Examinator for SZ'
            mProduct    = GS::LPPlugin::LicenseProvider::eSzOEM;
        }
        // SPECIAL COMMAND LINE called by ASL Wrapper to force Examinator to run in ASL support mode without license check.
        else if(lAslOEMCommandLine == argument)
        {
            // Force running mode to be: 'OEM-Examinator for ASL'
            mProduct    = GS::LPPlugin::LicenseProvider::eAslOEM;
        }
        // SPECIAL COMMAND LINE called by Sapphire Wrapper to force Examinator to run in Credence Sapphire / Diamond support mode without license check.
        else if(lSapphireOEMCommandLine == argument)
        {
            // Force running mode to be: 'OEM-Examinator for Credence Sapphire / Diamond'
            mProduct    = GS::LPPlugin::LicenseProvider::eSapphireOEM;
        }*/
        // Not a Product Startup Argument
        else
            return false;
    }

    return true;
}

}   // namespace Gex
}   // namespace GS
