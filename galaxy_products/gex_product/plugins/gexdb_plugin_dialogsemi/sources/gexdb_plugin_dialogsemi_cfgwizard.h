/****************************************************************************
** Form interface generated from reading ui file 'gexdb_plugin_galaxy_cfgwizard_base.ui'
**
** Created: Wed Mar 3 12:44:40 2004
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.2.3   edited May 19 14:22 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef GEXDB_PLUGIN_DIALOGSEMI_CFGWIZARD_H
#define GEXDB_PLUGIN_DIALOGSEMI_CFGWIZARD_H

// Galaxy modules includes
#include <gqtl_sysutils.h>
#include <QPushButton>

#include "ui_gexdb_plugin_dialogsemi_cfg_wizard.h"
#include "gexdb_plugin_base.h"

class GexDbPlugin_Dialogsemi_CfgWizard : public Q3Wizard, public Ui::GexDbPlugin_Dialogsemi_CfgWizard_Base
{
	Q_OBJECT

public:
    GexDbPlugin_Dialogsemi_CfgWizard( const QStringList & strlGexFields, CGexSkin * pGexSkin, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0 );
    ~GexDbPlugin_Dialogsemi_CfgWizard();

	void Set(const GexDbPlugin_Connector & clDbConnector);
	void Get(GexDbPlugin_Connector & clDbConnector);
	
private:
	GexDbPlugin_Connector	*m_pclDbConnector;			// Database connector
	
	QStringList				m_strlDrivers;				// List of QT SQL database drivers. Ie "QOCI8"
	QStringList				m_strlDrivers_FriendlyNames;// List of QT SQL database drivers (Friendly names). Ie "Oracle (QOCI8)"
	QPushButton				*m_pButtonNext;				// Ptr on Next button
	QPushButton				*m_pButtonBack;				// Ptr on Back button
	QStringList				m_strlGexFields;			// List of Gex fields

private slots:
	void		OnDatabaseTypeChanged();
    void		UpdateGui();
    void		OnFinish();
    void		OnCancel();

protected slots:
	void		next();
	void		back();
};

#endif // GEXDB_PLUGIN_DIALOGSEMI_CFGWIZARD_H
