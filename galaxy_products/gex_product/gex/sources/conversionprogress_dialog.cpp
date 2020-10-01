// Progress dialog box for file format conversion with an animation

#include <QFileInfo>
#include <QTimer>
#include <QMovie>

#include "conversionprogress_dialog.h"
#include "browser_dialog.h"

ConversionProgressDialog::
        ConversionProgressDialog(const QString& strCaption,
                                 const QString& strTitle,
                                 const QString& strAnimationFile,
                                 const QString& /*strInputFile*/,
                                 const QString& /*strOutputFile*/,
                                 QWidget* parent,
                                 bool modal,
                                 Qt::WindowFlags fl)
    : QDialog( parent, fl )
{
    setupUi(this);
    setModal(modal);

    // Apply Examinator palette
    GexMainwindow::applyPalette(this);

    QObject::connect(buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));

    // Set dialog caption
    setWindowTitle(strCaption);

    // Set title inside dialog box
    QString strRichEditTitle = "<font color=\"#5555ff\" face=\"Arial\">";
    strRichEditTitle += strTitle;
    strRichEditTitle += "</font>";
    editTitle->setText(strRichEditTitle);

    // Set animation file
    if(strAnimationFile != "")
    {
        QFileInfo clFileInfo(strAnimationFile);
        if(clFileInfo.exists() && clFileInfo.isReadable())
        {
            labelAnimation->setMovie(new QMovie(strAnimationFile, QByteArray(), this));
            labelAnimation->movie()->start();
        }
    }

    // Hide dialog if gex run in hidden mode
    //if(pGexMainWindow->bArgHide)
    //	hide();

    // Start timer
    m_nSeconds = 0;
    QTimer* pTimer = new QTimer(this);
    connect( pTimer, SIGNAL(timeout()), this, SLOT(OnSecTimer()));
    pTimer->start(1000);
}

/*
 *  Destroys the object and frees any allocated resources
 */
ConversionProgressDialog::~ConversionProgressDialog()
{
}

void ConversionProgressDialog::OnSecTimer(void)
{
    labelElapsedSec->setText(QString::number(++m_nSeconds));
}
