#include "query_progress_gui.h"
#include <gqtl_log.h>

namespace GS
{
namespace DbPluginBase
{

QueryProgressGui::QueryProgressGui( CGexSkin * pGexSkin,
                                    QWidget* parent,
                                    bool modal,
                                    Qt::WindowFlags fl )
    : QDialog( parent, fl )
{
    setupUi(this);
    setModal(modal);

    textEditLog->setAcceptRichText(true);
    textEditLog->show();

    // Set Examinator skin
    pGexSkin->applyPalette(this);
    mIsAbortRequested = false;

    connect(buttonAbort,        &QPushButton::clicked           , this, &QueryProgressGui::OnButtonAbort);
    connect(checkBoxDetails,    &QCheckBox::clicked /* bool */  , this, &QueryProgressGui::OnShowDetails /* bool */);

    OnShowDetails(checkBoxDetails->isChecked());
}

QueryProgressGui::~QueryProgressGui()
{
}

void QueryProgressGui::SetAutoClose(bool isAutoClose)
{
    checkBoxAutoClose->setChecked(isAutoClose);
}

void QueryProgressGui::LogStart()
{
    editProductName->setText("");
    editLotID->setText("");
    editWaferID->setText("");
    editNbRuns->setText("0");
    editNbTestResults->setText("0");
    editTimer->setText("00:00:00");
    progressBarOverall->reset();
    progressBarOverall->setMaximum(mTotalFiles*100);
    progressBarCurrent->reset();
    show();
}

void QueryProgressGui::LogStop()
{
    if(checkBoxAutoClose->isChecked())
        hide();
}

void QueryProgressGui::LogFileInfo(const QString &productName,
                                   const QString &lotID,
                                   const QString &sublotID,
                                   const QString &waferID)
{
    // Update edit fields
    editProductName->setText(productName);
    editLotID->setText(lotID);
    editWaferID->setText(waferID);
    editSublotID->setText(sublotID);

    // UPdate timer fields
    QTime clCurrent(0,0,0,0);
    editTimer->setText(clCurrent.addMSecs(mQueryTime.elapsed()).toString("hh:mm:ss"));
}

void QueryProgressGui::LogStartFileProgress()
{
    progressBarCurrent->reset();
    progressBarCurrent->setMaximum(mRunsInFile);
}

void QueryProgressGui::LogRetrievedRuns(unsigned int totalRuns)
{

    // Update fields
    editNbRuns->setText(QString::number(totalRuns));

    // Update timer
    QTime clCurrent(0,0,0,0);
    editTimer->setText(clCurrent.addMSecs(mQueryTime.elapsed()).toString("hh:mm:ss"));
}

void QueryProgressGui::LogRetrievedTests(unsigned int runs,
                                         unsigned int totalTestResults)
{
    // Update edit fields
    editNbTestResults->setText(QString::number(totalTestResults));

    // Update progress bars
    progressBarCurrent->setValue(runs);
    if(mRunsInFile)
        progressBarOverall->setValue(mRetrievedFiles*100+(runs*100)/mRunsInFile);

    // Update timer fields
    QTime clCurrent(0,0,0,0);
    editTimer->setText(clCurrent.addMSecs(mQueryTime.elapsed()).toString("hh:mm:ss"));
}

void QueryProgressGui::LogEndFileProgress()
{
    // update progress bar
    progressBarCurrent->setMaximum(progressBarCurrent->maximum());
    progressBarOverall->setValue((mRetrievedFiles)*100);
}

void QueryProgressGui::AddLog(const QString &log)
{
    textEditLog->append(log);
}

void QueryProgressGui::SetLogsTextColor(const QColor &color)
{
    textEditLog->setTextColor(color);
}

void QueryProgressGui::ClearLogs()
{
    textEditLog->clear();
}

void QueryProgressGui::OnButtonAbort()
{
    mIsAbortRequested = true;
    if (textEditLog->textColor() == Qt::red) // an error occured
        hide();
}

void QueryProgressGui::OnShowDetails(bool aShowDetails)
{
    if (aShowDetails) progressBarOverall->setMinimumWidth(500);
    else progressBarOverall->setMinimumWidth(300);
    frameDetails->setVisible(aShowDetails);
    adjustSize();
}

} //END namespace DbPluginBase
} //END namespace GS

