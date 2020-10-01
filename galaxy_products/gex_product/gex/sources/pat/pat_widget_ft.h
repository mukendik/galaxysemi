#ifdef GCORE15334
#ifndef PAT_WIDGET_FT_H
#define PAT_WIDGET_FT_H

#include <QWidget>

namespace Ui {
class PATWidgetFT;
}

namespace GS
{
namespace Gex
{

class TesterServer;

class PATWidgetFT : public QWidget
{
    Q_OBJECT

public:

    explicit PATWidgetFT(QWidget *parent = 0);
    ~PATWidgetFT();

    enum ReportFormat
    {
        ReportDisabled    = 0,
        ReportHTML,
        ReportWord,
        ReportCsv,
        ReportPpt,
        ReportPdf,
        ReportInteractive
    };

    enum OutputDataFormat
    {
        OutputDisabled = 0,
        OutputSTDF,
        OutputATDF
    };

    QString GetRecipeFile() const;
    QString GetTestDataFile() const;
    void    SetRecipeFile(const QString& lRecipe);
    void    SetTestDataFile(const QString& lTestData);

    void    ForceAttachWindow(bool lAttach = true, bool lToFront = true);

public slots:
    void    RunSimulator();

protected slots:

    void    UpdateUI();
    void    OnDetachWindow();
    void    OnEditRecipeFile();
    void    OnSelectTestDataFile();
    void    OnSelectRecipeFile();
    void    OnPATProcessingDone(const QString& lOutputDataFile, const QString &lTraceabilityFile);
    void    OnPATProcessingFailed(const QString& lErrorMessage);
    void    OnStatusProgress(const QString& lMessage, qint64 lDone, qint64 lTotal);

protected:

    void    dragEnterEvent(QDragEnterEvent * lEvent);
    void    dropEvent(QDropEvent * lEvent);

    bool    CreateGtlTesterConf(const QString& lTesterConf, int lSocket);

private:

    Ui::PATWidgetFT *   mUi;
    TesterServer *      mTesterServer;
    bool                mChildWindow;
};

}
}

#endif // PAT_WIDGET_FT_H
#endif
