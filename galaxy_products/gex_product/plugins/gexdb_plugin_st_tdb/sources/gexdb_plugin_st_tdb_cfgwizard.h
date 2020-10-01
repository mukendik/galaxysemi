/****************************************************************************
** Form interface generated from reading ui file 'gexdb_plugin_st_tdb_cfgwizard_base.ui'
**
** Created: Wed Mar 3 12:44:40 2004
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.2.3   edited May 19 14:22 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef GEXDB_PLUGIN_ST_TDB_CFGWIZARD_H
#define GEXDB_PLUGIN_ST_TDB_CFGWIZARD_H

// Galaxy modules includes
#include <gqtl_skin.h>

#include "ui_gexdb_plugin_st_tdb_cfg_wizard.h"
#include "gexdb_plugin_base.h"

class GexDbPlugin_StTdb_CfgWizard : public Q3Wizard, public Ui::GexDbPlugin_StTdb_CfgWizard_Base
{
	Q_OBJECT
		
public:
    GexDbPlugin_StTdb_CfgWizard( const QStringList & strlGexFields, CGexSkin * pGexSkin, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0 );
    ~GexDbPlugin_StTdb_CfgWizard();

	void Set(const GexDbPlugin_Connector & clDbConnector, const GexDbPlugin_FtpSettings & clFtpSettings, const QString & strTableName, const QMap<QString,QString> & cFieldsMapping);
	void Get(GexDbPlugin_Connector & clDbConnector, GexDbPlugin_FtpSettings & clFtpSettings, QString & strTableName, QMap<QString,QString> & cFieldsMapping);

private:
	CGexSkin *					m_pGexSkin;					// Examinator skin to use
	GexDbPlugin_Connector		*m_pclDbConnector;			// Database connector
	QString						m_strTableName;				// Name of table to use
	QMap<QString,QString>		m_cFieldsMapping;			// Holds mapping Fields from Remote DB to Galaxy DB
	
	QStringList					m_strlDrivers;				// List of QT SQL database drivers. Ie "QOCI8"
	QStringList					m_strlDrivers_FriendlyNames;// List of QT SQL database drivers (Friendly names). Ie "Oracle (QOCI8)"
	QPushButton					*m_pButtonNext;				// Ptr on Next button
	QPushButton					*m_pButtonBack;				// Ptr on Back button
	QStringList					m_strlTables;				// List of tables for connected DB
	QStringList					m_strlGexFields;			// List of Gex fields

private:
	
private slots:
	void		OnDatabaseTypeChanged();
    void		UpdateGui();
    void		OnFinish();
    void		OnCancel();
    void		OnFieldMappingMap();

protected slots:
	void		next();
	void		back();
};

#endif // GEXDB_PLUGIN_ST_TDB_CFGWIZARD_H
