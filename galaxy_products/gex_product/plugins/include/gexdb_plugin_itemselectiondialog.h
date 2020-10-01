/****************************************************************************
** Form interface generated from reading ui file 'export_xml_dialog_base.ui'
**
** Created: Wed Mar 3 12:44:40 2004
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.2.3   edited May 19 14:22 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef GEXDB_PLUGIN_ITEMSELECTION_DIALOG_H
#define GEXDB_PLUGIN_ITEMSELECTION_DIALOG_H

// Galaxy modules includes
#include <gqtl_skin.h>

#include "ui_gexdb_plugin_itemselection_dialog.h"

class GexDbPlugin_ItemSelectionDialog : public QDialog, public Ui::GexDbPlugin_ItemSelectionDialog_Base
{
	Q_OBJECT

public:
    GexDbPlugin_ItemSelectionDialog( const QString & strDescription, const QStringList & strlTables, CGexSkin * pGexSkin, QWidget* parent = 0, bool modal = false, Qt::WindowFlags fl = 0 );
    ~GexDbPlugin_ItemSelectionDialog();
};

#endif // GEXDB_PLUGIN_ITEMSELECTION_DIALOG_H
