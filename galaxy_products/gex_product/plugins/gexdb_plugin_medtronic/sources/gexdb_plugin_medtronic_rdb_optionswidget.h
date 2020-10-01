/****************************************************************************
** Form interface generated from reading ui file 'gexdb_plugin_galaxy_cfgwizard_base.ui'
**
** Created: Wed Mar 3 12:44:40 2004
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.2.3   edited May 19 14:22 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef GEXDB_PLUGIN_MEDTRONIC_RDB_OPTIONSWIDGET_H
#define GEXDB_PLUGIN_MEDTRONIC_RDB_OPTIONSWIDGET_H

#include "ui_gexdb_plugin_medtronic_rdb_optionswidget.h"
#include "gexdb_plugin_base.h"
#include "gexdb_plugin_medtronic_options.h"

class GexDbPlugin_Medtronic_Rdb_OptionsWidget : public QWidget, public Ui::GexDbPlugin_Medtronic_Rdb_OptionsWidget_Base
{
	Q_OBJECT

public:
    GexDbPlugin_Medtronic_Rdb_OptionsWidget( QWidget* parent = 0, Qt::WindowFlags fl = 0 );
    ~GexDbPlugin_Medtronic_Rdb_OptionsWidget();

	void GetOptionsString(QString & strOptionString);
	void SetOptionsString(const GexDbPlugin_Medtronic_Options & clOptions);
};

#endif // GEXDB_PLUGIN_MEDTRONIC_RDB_OPTIONSWIDGET_H
