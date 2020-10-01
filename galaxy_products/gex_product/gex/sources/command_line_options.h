#ifndef COMMAND_LINE_OPTIONS_H
#define COMMAND_LINE_OPTIONS_H

#include <QObject>
#include <QStringList>
#include "license_provider.h"

namespace GS
{
namespace Gex
{

class CommandLineOptions : public QObject
{
    Q_OBJECT

public:

    CommandLineOptions(QObject * parent = NULL);
    ~CommandLineOptions();

    bool                ParseArguments(int argc, char ** argv);
    // Parse args ?
    bool                ParseArguments(const QStringList& arguments);

    bool                IsHidden() const;
    bool                IsCustomDebugMode() const;
    bool                IsErrorBoxDisabled() const;
    bool                IsWelcomeBoxEnabled() const;
    bool                CloseAfterRunScript() const;
    GS::LPPlugin::LicenseProvider::GexProducts GetProduct() const;
    const QString&      GetRunScript() const;
    const QString&      GetProfileScript() const;
    const QString&      GetStartupDataFile() const;
    const QString&      GetTriggerFile() const;
    const QStringList&  GetArguments()const;

    void                SetWelcomeBoxEnabled(bool Value){ mWelcomeBox=Value;}
protected:

    bool                CheckStartupArguments(const QString& argument);

private:

    bool                mHidden;                // Hide GEX while running
    bool                mCloseAfterScript;      // Close GEX once script 'argv[2]' is finished.
    bool                mCustomDebugMode;       // Used to activate certain debug features without re-compiling
    bool                mDisableErrorBox;       //
    bool                mWelcomeBox;            // Show the welcome dialog to select the license type
    GS::LPPlugin::LicenseProvider::GexProducts  mProduct;               // Product to run from the command line
    QString             mRunScript;             // Script to run at launch time
    QString             mProfileScript;         // Profile to open given by exec arguments
    QString             mStartupDataFile;       // Data file to process as given by argument
    QString             mTriggerFile;           // trigger to handle/insert as given by argument
    QStringList         mArguments;             // List of arguments parsed
};

}   // namespace Gex
}   // namespace GS

#endif // COMMAND_LINE_OPTIONS_H
