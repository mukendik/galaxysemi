/****************************************************************************
** Derived from plugineventdump_dialog_base.h
****************************************************************************/

#ifndef PLUGINEVENTDUMP_DIALOG_H
#define PLUGINEVENTDUMP_DIALOG_H

#include "ui_plugin_eventdump_dialog.h"

class PluginEventDumpDialog : public QDialog, public Ui::PluginEventDumpDialog_base
{
	Q_OBJECT
		
public:
    PluginEventDumpDialog( const QString& strEventName, const QString& strEventDump, QWidget* parent = 0, bool modal = false, Qt::WindowFlags fl = 0 );
    ~PluginEventDumpDialog();

};

#endif // PLUGINEVENTDUMP_DIALOG_H
