#ifndef PAT_REPORT_FT_GUI_H
#define PAT_REPORT_FT_GUI_H

#include <QWidget>

namespace Ui {
class pat_report_ft_gui;
}

namespace GS
{
namespace Gex
{

class PATReportFTGui : public QWidget
{
    Q_OBJECT

public:
    explicit PATReportFTGui(QWidget *parent = 0);
    ~PATReportFTGui();

    bool            IsDataReady() const;
    const QString&  GetWorkingPath() const;
    QStringList     GetDataFiles() const;
    QString         GetTraceabilityFile() const;

    void            SetWorkingPath(const QString& lWorkingPath);

    void            WriteProcessFilesSection(FILE *hFile);

protected slots:

    void    UpdateUI();
    void    OnRemoveDataFile();
    void    OnSelectTestDataFile();
    void    OnSelectTraceabilityFile();

protected:

    void    dragEnterEvent(QDragEnterEvent * lEvent);
    void    dropEvent(QDropEvent * lEvent);
    bool    eventFilter(QObject * lObj, QEvent * lEvent);

private:

    QString                 mWorkingPath;
    Ui::pat_report_ft_gui * mUi;
};

}
}

#endif // PAT_REPORT_FT_GUI_H
