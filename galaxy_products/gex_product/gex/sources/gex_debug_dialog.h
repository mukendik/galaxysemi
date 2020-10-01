///////////////////////////////////////////////////////////
// GEX generic debug dialog - can be used for testing code
///////////////////////////////////////////////////////////

#ifndef GEX_DEBUG_DIALOG_H
#define GEX_DEBUG_DIALOG_H

#include "ui_gex_debug_dialog.h"
#include "db_transactions.h"

class GexDatabaseEntry;

class GexDebugDialog : public QDialog, public Ui::Gex_Debug_basedialog
{
	Q_OBJECT

public:

    GexDebugDialog( QWidget* parent = 0, bool modal = false, Qt::WindowFlags fl = 0 );
    ~GexDebugDialog();

protected:
	void Reset();
	GexDatabaseEntry	*m_pDatabaseEntry;
	bool				m_bFinalTest;
	QStringList			m_strlSplitlots;
	QString				m_strTestingStage;

protected slots:
	void OnDatabaseChanged(const QString & strDatabase);
	void OnTestingStageChanged(const QString & strTestingStage);
	void OnGetSplitlots();
	void OnPurgeSplitlots();
	void OnConsolidateWafers();
	//void OnGetProducts(void);
	//void OnGetLots(void);
    // Update textEdit_Log with log messages
    void UpdateLogMessage(const QString &message, bool isPlainText);
};

#endif // GEX_DEBUG_DIALOG_H
