#include <QFileDialog>
#include "report_build.h"
#include "browser_dialog.h"
#include "report_frontpage_dialog.h"
#include "gex_report.h"

// in report_build.cpp
extern CGexReport* gexReport;					// Handle to report class
extern CReportOptions	ReportOptions;			// Holds options (report_build.h)

#define	GEX_VALID_IMAGEFILES_TYPES	"Image Files (*.bmp *.gif *.png *.jpg)"

///////////////////////////////////////////////////////////
// Constructor
FrontPageDialog::FrontPageDialog( QWidget* parent, bool modal, Qt::WindowFlags f )
    : QDialog( parent, f )
{
    setupUi(this);
    setModal(modal);

    // Apply Examinator palette
    GexMainwindow::applyPalette(this);

    QObject::connect(PushButtonOk,		SIGNAL(clicked()),			this, SLOT(accept()));
    QObject::connect(PushButtonCancel,	SIGNAL(clicked()),			this, SLOT(reject()));
    QObject::connect(buttonAddLogo,		SIGNAL(clicked()),			this, SLOT(OnSelectLogo()));
    QObject::connect(textHtml,			SIGNAL(clicked(int,int)),	this, SLOT(OnEditText(int,int)));
    QObject::connect(buttonRemoveLogo,	SIGNAL(clicked()),			this, SLOT(OnRemoveLogo()));

///	pixmapLogo->setFixedSize(pixmapLogo->sizeHint());
}

///////////////////////////////////////////////////////////
// Edit HTML text
///////////////////////////////////////////////////////////
void FrontPageDialog::OnEditText(int /*iLine*/, int /*iCol*/)
{
}

///////////////////////////////////////////////////////////
// Select a new logo image
///////////////////////////////////////////////////////////
void FrontPageDialog::OnSelectLogo(void)
{
    QString strFile = QFileDialog::getOpenFileName(this, "Select Image", strImagePath, GEX_VALID_IMAGEFILES_TYPES);

    if(strFile.isEmpty())
        return;	// No selection made

    // Update screen: Repaint logo image
    SetSelection("",strFile);
}

///////////////////////////////////////////////////////////
// Unselect logo (remove from home page)
///////////////////////////////////////////////////////////
void FrontPageDialog::OnRemoveLogo(void)
{
    // Update screen: Repaint with NO logo image
    SetSelection("","");
}

///////////////////////////////////////////////////////////
// Set text + Logo to be used on report's front page
///////////////////////////////////////////////////////////
void FrontPageDialog::SetSelection(QString strFrontPageText,QString strFrontPageImage)
{
    // Update the text window
    if(strFrontPageText.isEmpty() == false)
        textHtml->setPlainText(strFrontPageText);
    else
    {
        QString strInfoText = "Modify this text to customize your Home Page Report";
        textHtml->setPlainText(strInfoText);
    }


    // Update the logo image
    strImagePath = strFrontPageImage;
    TextLabelPreviewStatus->hide();
    if(strFrontPageImage.isEmpty() == false)
    {
        QPixmap pPixmap(strFrontPageImage);
        pixmapLogo->setPixmap(pPixmap);
        QFileInfo cFileInfo(strFrontPageImage);
        TextLabelImageName->setText(cFileInfo.fileName());

        // If .GIF file, says we can't display it here, but we support it!
        if(strFrontPageImage.endsWith(".gif", Qt::CaseInsensitive))
        {
            TextLabelPreviewStatus->show();
            pixmapLogo->hide();
        }
        else
        {
            TextLabelPreviewStatus->hide();
            pixmapLogo->show();
        }
    }
    else
    {
        pixmapLogo->clear();	// No image to show: clear window
        TextLabelImageName->clear();
    }
}

///////////////////////////////////////////////////////////
// Return text + Logo to be used on report's front page
///////////////////////////////////////////////////////////
void FrontPageDialog::GetSelection(QString &strFrontPageText,QString &strFrontPageImage)
{
    // Retrieve text + path to the image
    strFrontPageText = textHtml->toPlainText();
    strFrontPageImage = strImagePath;
}
