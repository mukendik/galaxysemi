#include <QtWidgets>

#include <gqtl_log.h>

#include "regexp_validator.h"
#include "product_info.h"
#include "gex_shared.h"
#include "message.h"

namespace GS
{
namespace Gex
{
RegExpValidator::RegExpValidator(const QString &inputText, QWidget *parent)
     : QDialog(parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
 {

    //check this page
    QPalette lPalette = palette();
    lPalette.setColor(backgroundRole(), Qt::white);
    setPalette(lPalette);
    mRegExpComboBox = new QComboBox;
    mRegExpComboBox->setEditable(true);
    mRegExpComboBox->setSizePolicy(QSizePolicy::Expanding,
                                   QSizePolicy::Preferred);

    mRegExpLabel = new QLabel("Regular Expression");
    mRegExpLabel->setBuddy(mRegExpComboBox);

    mTextComboBox = new QComboBox;
    mTextComboBox->setEditable(true);
    mTextComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    mTextLabel = new QLabel("Text");
    mTextLabel->setBuddy(mTextComboBox);

    mErrorEdit = new QLineEdit("");
    mErrorEdit->setReadOnly(true);
    lPalette = mErrorEdit->palette();
    lPalette.setColor(QPalette::Text, Qt::red);
    lPalette.setBrush(QPalette::Base,
                     lPalette.brush(QPalette::Disabled, QPalette::Base));
    mErrorEdit->setPalette(lPalette);
    QFont lFont = mErrorEdit->font();
    lFont.setStyle(QFont::StyleItalic);
    mErrorEdit->setFont(lFont);

    mMatchStatusLabel = new QLabel("Status");
    mMatchValueLabel = new QLabel("");
    mMatchCommentLabel = new QLabel("(RegExps in GexDbKeys require a complete match)");
    mMatchCommentLabel->setStyleSheet(" qproperty-alignment: AlignRight;");
    mMatchCommentLabel->setEnabled(false);

    mErrorLabel = new QLabel("Syntax Error");
    mErrorLabel->setBuddy(mErrorEdit);

    mAdvancedCheckBox = new QCheckBox("Advanced");
    mAdvancedCheckBox->setChecked(false);

    mCaseSensitivityCheckBox = new QCheckBox("Case Sensitive");
    mCaseSensitivityCheckBox->setChecked(false);

    mCaseSensitivityLabel = new QLabel("(by default RegExps in Quantix products are not case sensitive)");
    mCaseSensitivityLabel->setStyleSheet(" qproperty-alignment: AlignRight;");
    mCaseSensitivityLabel->setEnabled(false);

    mRegExpEngineComboBox = new QComboBox;
    mRegExpEngineComboBox->insertItem(0, "Standard RegExp "
                                      "(Used in versions >= V7.0, Perl, JS, ...)",
                                QVariant("RegExp2"));
    mRegExpEngineComboBox->insertItem(1, "Legacy RegExp "
                                      "(Used in versions < V7.0)",
                                QVariant("RegExp"));
    mRegExpEngineComboBox->setCurrentIndex(0);
    mRegExpEngineLabel= new QLabel("RegExp Engine");

    mIndexLabel = new QLabel("Index of Match");
    mIndexEdit = new QLineEdit;
    mIndexEdit->setReadOnly(true);

    mMatchedLengthLabel = new QLabel("Matched Length");
    mMatchedLengthEdit = new QLineEdit;
    mMatchedLengthEdit->setReadOnly(true);

    mCaptureLabelsTMP.append(new QLabel(QString("Match")));
    mCaptureEditsTMP.append(new QLineEdit);
    mCaptureEditsTMP[0]->setReadOnly(true);

    QHBoxLayout *lMatchLayout = new QHBoxLayout;
    lMatchLayout->addWidget(mMatchValueLabel);
    lMatchLayout->addWidget(mMatchCommentLabel);

    QHBoxLayout *lCheckBoxLayout = new QHBoxLayout;
    lCheckBoxLayout->addWidget(mAdvancedCheckBox);
    lCheckBoxLayout->addStretch(1);

    QGridLayout *lMainLayout = new QGridLayout;
    lMainLayout->addWidget(mTextLabel, 0, 0);
    lMainLayout->addWidget(mTextComboBox, 0, 1);
    lMainLayout->addWidget(mRegExpLabel, 1, 0);
    lMainLayout->addWidget(mRegExpComboBox, 1, 1);
    lMainLayout->addWidget(mMatchStatusLabel, 2, 0);
    lMainLayout->addLayout(lMatchLayout, 2, 1);
    lMainLayout->addWidget(mCaptureLabelsTMP[0], 3, 0);
    lMainLayout->addWidget(mCaptureEditsTMP[0], 3, 1);
    lMainLayout->addWidget(mErrorLabel, 4, 0);
    lMainLayout->addWidget(mErrorEdit, 4, 1);
    lMainLayout->addWidget(mCaseSensitivityCheckBox, 5, 0);
    lMainLayout->addWidget(mCaseSensitivityLabel, 5, 1);
    lMainLayout->addLayout(lCheckBoxLayout, 6, 0, 1, 2);
    lMainLayout->addWidget(mRegExpEngineLabel, 7, 0);
    lMainLayout->addWidget(mRegExpEngineComboBox, 7, 1);
    lMainLayout->addWidget(mIndexLabel, 8, 0);
    lMainLayout->addWidget(mIndexEdit, 8, 1);
    lMainLayout->addWidget(mMatchedLengthLabel, 9, 0);
    lMainLayout->addWidget(mMatchedLengthEdit, 9, 1);

    setLayout(lMainLayout);

    connect(mRegExpComboBox, SIGNAL(editTextChanged(QString)),
            this, SLOT(refresh()));
    connect(mRegExpEngineComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(refresh()));
    connect(mTextComboBox, SIGNAL(editTextChanged(QString)),
            this, SLOT(refresh()));
    connect(mCaseSensitivityCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(refresh()));
    connect(mAdvancedCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(refresh()));

    if (!inputText.isEmpty())
        mTextComboBox->addItem(inputText);

    setWindowTitle("Regular Expression Checker");
    setMinimumWidth(500);
    refresh();
}

RegExpValidator::~RegExpValidator()
{
    delete mRegExpLabel;
    delete mErrorEdit;
    delete mErrorLabel;
    delete mTextLabel;
    delete mRegExpComboBox;
    delete mTextComboBox;
    delete mAdvancedCheckBox;
    delete mIndexLabel;
    delete mMatchedLengthLabel;
    delete mIndexEdit;
    delete mMatchedLengthEdit;
    delete mCaseSensitivityCheckBox;
    delete mCaseSensitivityLabel;
    delete mMatchStatusLabel;
    delete mMatchValueLabel;
    delete mMatchCommentLabel;

    while (!mCaptureLabelsTMP.isEmpty())
        delete mCaptureLabelsTMP.takeLast();

    while (!mCaptureEditsTMP.isEmpty())
        delete mCaptureEditsTMP.takeLast();
}

void RegExpValidator::refresh()
{
    setUpdatesEnabled(false);

    QString lPattern = mRegExpComboBox->currentText();
    QString lText = mTextComboBox->currentText();

    QRegExp lRx(lPattern);
    QString lSelectedRegExpEngine = mRegExpEngineComboBox->
            itemData(mRegExpEngineComboBox->currentIndex()).toString();
    bool lIsCaseSensitive = mCaseSensitivityCheckBox->isChecked();

    // Set default pattern syntax (will be RegExp2 defaulty in Qt5 )
    if (lSelectedRegExpEngine == "RegExp")
        lRx.setPatternSyntax(QRegExp::RegExp);
    else if (lSelectedRegExpEngine == "RegExp2")
        lRx.setPatternSyntax(QRegExp::RegExp2);
    else
    {
        lRx.setPatternSyntax(QRegExp::RegExp2);
        GSLOG(SYSLOG_SEV_ERROR, "Unable to get RegExp Engine");
    }
    lRx.setCaseSensitivity(lIsCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);

    bool lIsAdv = mAdvancedCheckBox->isChecked();

    mRegExpEngineLabel->setVisible(lIsAdv);
    mRegExpEngineComboBox->setVisible(lIsAdv);
    mIndexLabel->setVisible(lIsAdv);
    mIndexEdit->setVisible(lIsAdv);
    mMatchedLengthLabel->setVisible(lIsAdv);
    mMatchedLengthEdit->setVisible(lIsAdv);

    // Set right color to regexp box
    QPalette lPalette = mRegExpComboBox->palette();
    if (lRx.isValid())
    {
        mErrorEdit->setText("");
        lPalette.setColor(QPalette::Text,
                         mTextComboBox->palette().color(QPalette::Text));
    }
    else
    {
        mErrorEdit->setText(lRx.errorString());
        lPalette.setColor(QPalette::Text, Qt::red);
    }
    mRegExpComboBox->setPalette(lPalette);

    mIndexEdit->setText(QString::number(lRx.indexIn(lText)));
    mMatchedLengthEdit->setText(QString::number(lRx.matchedLength()));

    // set match status
    if (lRx.cap(0).isEmpty())
    {
        mMatchValueLabel->setText("no match");
        mMatchValueLabel->setStyleSheet("QLabel { color : red; font-weight : bold; }");
    }
    else
    {
        bool lCompleteMatch = lRx.exactMatch(lText);
        if (lCompleteMatch)
        {
            mMatchValueLabel->setText("complete match");
            mMatchValueLabel->setStyleSheet("QLabel { color : green; font-weight : bold; }");
        }
        else
        {
            mMatchValueLabel->setText("partial match");
            mMatchValueLabel->setStyleSheet("QLabel { color : black; font-weight : bold; }");
        }
    }

    loadCaptureFields(lRx, lIsAdv);

    setUpdatesEnabled(true);
    setFixedHeight(sizeHint().height());
}

void RegExpValidator::loadCaptureFields(const QRegExp &rx,
                                        bool isAdvancedMode)
{
    if( GS::LPPlugin::ProductInfo::getInstance()->isNotSupportedCapability(GS::LPPlugin::ProductInfo::toolbox))
    {
      // Evaluation mode or OEM mode, refuse to run this function!
      GS::Gex::Message::information(
          "", "This function is disabled in teradyne mode\n\nContact " +
          QString(GEX_EMAIL_SALES) + " for more information!");
        return;
    }

    // If not enough capture field -> add
    while (mCaptureEditsTMP.count() <= rx.captureCount())
    {
        int lCaptureId = mCaptureEditsTMP.count();
        mCaptureLabelsTMP.append(new QLabel(QString("Capture %1").arg(lCaptureId)));
        mCaptureEditsTMP.append(new QLineEdit);
        ((QGridLayout*)layout())->addWidget(mCaptureLabelsTMP[lCaptureId], 10 + lCaptureId, 0);
        ((QGridLayout*)layout())->addWidget(mCaptureEditsTMP[lCaptureId], 10 + lCaptureId, 1);
    }
    // If too much capture field -> delete
    while ((mCaptureEditsTMP.count() > 1) &&
           (mCaptureEditsTMP.count() > (rx.captureCount() + 1)))
    {
        delete mCaptureLabelsTMP.takeLast();
        delete mCaptureEditsTMP.takeLast();
    }
    // Set Text (do not hide first record)
    mCaptureEditsTMP[0]->setText(rx.cap(0));
    for (int i = 1; i < mCaptureEditsTMP.count(); ++i)
    {
        mCaptureEditsTMP[i]->setText(rx.cap(i));
        mCaptureLabelsTMP[i]->setVisible(isAdvancedMode);
        mCaptureEditsTMP[i]->setVisible(isAdvancedMode);
    }

    adjustSize();
}

} // END Gex
} // END GS
