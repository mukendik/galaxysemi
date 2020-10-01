///////////////////////////////////////////////////////////
// Examinator Monitoring: Admin widget
///////////////////////////////////////////////////////////

#ifndef GEXMO_ADMIN_H
#define GEXMO_ADMIN_H
#include "ui_mo_admin_dialog.h"

class GexYMEventLogViewManager;

class MonitoringWidget: public QWidget, public Ui::monitor_admin_basedialog
{
    Q_OBJECT

public:
    MonitoringWidget(QWidget* parent = 0, Qt::WindowFlags fl = 0 );
    ~MonitoringWidget();
protected:
    QStringList	m_strlLogFiles_History;
    QStringList	m_strlLogFiles_GexFtp;
    QStringList	m_strlLogFiles_GexEmail;
    QStringList	m_strlLogFiles_Report;
    QPixmap		m_pxInfo;						// Pixmap for information messages
    QPixmap		m_pxError;						// Pixmap for error messages
    QPixmap		m_pxWarning;					// Pixmap for warning messages
    QPixmap		m_pxSuccess;					// Pixmap for success messages

    void			UpdateListOfFiles();
    void			LoadFile_History(const QString & strMoHistoryLogFile);
    void			LoadFile_GexFtp(const QString & strGexFtpLogFile);
    void			LoadFile_Report(const QString & strReportLogFile);
    QTreeWidgetItem *LoadLine_Report(const QString & strLine, bool bEnsureItemVisible);
    void			LoadFile_GexEmail(const QString & strGexEmailLogFile);

public slots:
    void	OnMainSelectionChanged();
    void	OnMoHistoryLogUpdated(const QString &,const QString &);
    void	OnFileSelected_History();
    void	OnButtonSearch_Next_History();
    void	OnButtonSearch_Prev_History();
    void	OnMoReportLogUpdated(const QString &,const QString &);
    void	OnFileSelected_Report();
    void	OnButtonSearch_Next_Report();
    void	OnButtonSearch_Prev_Report();
    void	OnButtonRefresh_GexFtp();
    void	OnFileSelected_GexFtp();
    void	OnButtonSearch_Next_GexFtp();
    void	OnButtonSearch_Prev_GexFtp();
    void	OnButtonRefresh_GexEmail();
    void	OnFileSelected_GexEmail();
    void	OnButtonSearch_Next_GexEmail();
    void	OnButtonSearch_Prev_GexEmail();


    //YM EVENT LOG Part:
private:
    GexYMEventLogViewManager *m_poYMEventLogViewManager;
    bool    isYMEvenLogsEnabled();
    void    initLogManager(QTreeWidgetItem *poItem, QWidget *poContainer);


/*    void    updateFilterView(QTreeWidgetItem *poItem);
private:
    GexYMEventLogLoader *m_poGexYMEventLog;
    GexYMEventLogView *m_poGexYMEventLogView;
public:
    GexYMEventLogLoader *getGexYMEventLogLoader(){
        return m_poGexYMEventLog;
    }
    GexYMEventLogView *getGexYMEventLogView(){
        return m_poGexYMEventLogView;
    }

private:
    bool    isYMEvenLogsEnabled();
    void    initYMEvenLogsEnabled(QWidget *poFilterDialog);
public slots:
    void	addFilter();
    void	removeFilter();
    void    buildOperatorField(int iIdx);
    void    buildFieldChoice(int iIdx);
    void    defineNewFilter();
*/
};
#endif
