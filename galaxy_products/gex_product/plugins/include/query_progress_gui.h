#ifndef QUERY_PROGRESS_GUI_H
#define QUERY_PROGRESS_GUI_H

#include <QDialog>
#include <QTextEdit>
#include "abstract_query_progress.h"
#include <gqtl_skin.h>

#include "ui_query_progress_gui.h"

namespace GS
{
namespace DbPluginBase
{

class QueryProgressGui: public QDialog, public AbstractQueryProgress, public Ui::QueryProgressGui
{
    Q_OBJECT
public:
    /// \brief Constructor
    QueryProgressGui( CGexSkin * pGexSkin,
                      QWidget* parent = 0,
                      bool modal = false,
                      Qt::WindowFlags fl = 0 );
    /// \brief Destructor
    ~QueryProgressGui();
    /// \brief add new logs
    void AddLog(const QString &log);
    /// \brief clear logs
    void ClearLogs();
    /// \brief set autoclose rule
    void SetAutoClose(bool isAutoClose);
    /// \brief set log color
    void SetLogsTextColor(const QColor &color);

private slots:
    /// \brief hide the dialog, called when abort clicked
    void OnButtonAbort();
    /// \brief hide / show details
    void OnShowDetails(bool aShowDetails);

private:
    Q_DISABLE_COPY(QueryProgressGui);
    /// \brief log start info
    void LogStart();
    /// \brief log stop info
    void LogStop();
    /// \brief log file info
    void LogFileInfo(const QString &productName,
                             const QString &lotID,
                             const QString &sublotID,
                             const QString &waferID);
    /// \brief log file start
    void LogStartFileProgress();
    /// \brief log retrieved runs
    void LogRetrievedRuns(unsigned int totalRuns);
    /// \brief log retrieved tests
    void LogRetrievedTests(unsigned int runs,
                           unsigned int totalTestResults);
    /// \brief log end of file
    void LogEndFileProgress();
};

} //END namespace DbPluginBase
} //END namespace GS


#endif // QUERY_PROGRESS_GUI_H
