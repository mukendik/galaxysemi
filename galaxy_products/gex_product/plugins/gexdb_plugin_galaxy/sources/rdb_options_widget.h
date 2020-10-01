#ifndef RDB_OPTIONS_WIDGET_H
#define RDB_OPTIONS_WIDGET_H

#include "ui_rdb_options_widget.h"
#include "gexdb_plugin_base.h"
#include "rdb_options.h"
#include "libgexpb.h"
#include "gex_scriptengine.h"

namespace GS
{
namespace DbPluginGalaxy
{

class RdbOptionsWidget : public QWidget, public Ui::RdbOptionsWidget
{
    Q_OBJECT

    CGexPB* mPropBrowser;
    const GexScriptEngine& mGSE;
    //int mExtractRawDataPropID;
    //int mExtractPartsIfNoResultsPropID;
    //int mExtractionGroupByPropID;
    // Map associating : the key of the option as defined in Option class and the PropId as given by the PropBrowser
    QMap<QString, int> mOptionKeyToPropID;

public:
    RdbOptionsWidget(const GexScriptEngine& se, QWidget* parent = 0, Qt::WindowFlags fl = 0 );
    //  Destroys the object and frees any allocated resources
    ~RdbOptionsWidget();

    // separated by ;
    void GetOptionsString(QString & strOptionString);
    void SetOptionsString(const RdbOptions & clOptions);
};

} // END DbPluginGalaxy
} // END GS

#endif // RDB_OPTIONS_WIDGET_H
