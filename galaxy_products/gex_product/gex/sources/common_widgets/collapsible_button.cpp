#include <QVBoxLayout>
#include "browser_dialog.h"
#include "common_widgets/collapsible_button.h"
#include "ui_collapsible_button.h"

const int DEFAULT_HEADER_SIZE = 12;
const int DEFAULT_INFORMATIVE_SIZE = 10;

extern GexMainwindow *	pGexMainWindow;

CollapsibleButton::CollapsibleButton(const QString &text, T_STYLE style /*= T_Header*/,  int sizePolicy, QWidget *parent) :
    QWidget(parent),
    mStyle(style),
    mUi(new Ui::CollapsibleButton),
    mLayout(0),
    mLayoutParent(0),
    mAttached(true),
    mIndexInLayout(-1),
    mSizePolicy(sizePolicy)
{
    mUi->setupUi(this);
    mTitle = text;
    mUi->pushButtonSettings->setText(text);
    mUi->pushButtonPin->setIcon(QIcon(""));
    InitUI();
}

CollapsibleButton::~CollapsibleButton()
{
    if(mAttached == false)
        hide();
    delete mUi;
}

void CollapsibleButton::SetDetachable(QVBoxLayout*  dialogParent)
{
   mLayoutParent = dialogParent;
   if(mLayoutParent)
   {
      UpdateIconPinUnPin(true);
   }
}


void CollapsibleButton::UpdateIconPinUnPin(bool isPin)
{
    if(isPin)
    {
        mUi->pushButtonPin->setIcon(QIcon(":/gex/icons/unpinBlack.png"));
        mUi->pushButtonPin->setIconSize(QSize(13,13));
        setStyleSheet( "QPushButton#pushButtonPin:pressed{"
                       "background-color:transparent;"
                       "image: url(:/gex/icons/pinBlack.png);"
                       "}");
        mUi->pushButtonPin->setToolTip("Unpin the table");
        mUi->pushButtonSettings->setIcon(QIcon(":/gex/icons/DownTriangle.png"));
        mUi->pushButtonSettings->setEnabled(true);
    }
    else
    {
        mUi->pushButtonPin->setIcon(QIcon(":/gex/icons/pinBlack.png"));
        mUi->pushButtonPin->setIconSize(QSize(13,13));
        setStyleSheet( "QPushButton#pushButtonPin:pressed{"
                       "background-color:transparent;"
                       "image: url(:/gex/icons/unpinBlack.png);"
                       "}");

        mUi->pushButtonPin->setToolTip("Pin the table");
        //-- when unpin, the fold/Unfold functionality has no sense
        mUi->pushButtonSettings->setIcon(QIcon(""));
        mUi->pushButtonSettings->setEnabled(false);
    }
}

void CollapsibleButton::InitUI()
{

    mUi->pushButtonSettings->setIcon(QIcon(":/gex/icons/DownTriangle.png"));
    connect(mUi->pushButtonSettings, SIGNAL(clicked(bool)), this, SLOT(updateButtonSettings()));
    connect(mUi->pushButtonPin, SIGNAL(clicked(bool)), this, SLOT(pinUnpin()));

    int lSizePolicy = mSizePolicy;
    switch (mStyle) {
    case T_Header:
    {
        if(lSizePolicy == 0)
            lSizePolicy = DEFAULT_HEADER_SIZE;
        mUi->pushButtonSettings->setStyleSheet( QString("text-align: left;"\
                                              "padding-left:5px;"\
                                              "background-color:rgba(31, 132, 202, 255);"\
                                              "font-size:%1px;"\
                                              "border:1px solid black;"\
                                              "border-radius: 6px;"\
                                              "color:white").arg(lSizePolicy).toLatin1().constData());
        break;
    }
    case T_Informative:
    {
        if(lSizePolicy == 0)
            lSizePolicy = DEFAULT_INFORMATIVE_SIZE;
        mUi->pushButtonSettings->setStyleSheet(  QString("text-align: left;"\
                                                "padding-left:5px;"\
                                                "background-color:#C0C0C0;"\
                                                "font-size:%1px;"\
                                                "border:1px solid black;" \
                                                "border-radius: 3px;").arg(lSizePolicy).toLatin1().constData());
        mUi->pushButtonPin->hide();
        static_cast<QGridLayout*>(layout())->setHorizontalSpacing(0);
    }
                break;
    default:
        break;
    }

}

void CollapsibleButton::addWidget(QWidget* widget)
{
    if(mLayout == 0)
    {
        mLayout = new QVBoxLayout();
        mLayout->setContentsMargins(0,0,0,0);
        mUi->frame->setLayout(mLayout);
    }

    if(mStyle == T_Informative)
        widget->setStyleSheet("font-size:10px");

    if(mSizePolicy != 0)
        widget->setStyleSheet(QString("font-size:%1px").arg(mSizePolicy).toLatin1().constData());

    mLayout->addWidget(widget);
}

void CollapsibleButton::setTitle(const QString &title)
{
    mUi->pushButtonSettings->setText(title);
}

void CollapsibleButton::close()
{
    mUi->pushButtonSettings->setIcon(QIcon(":/gex/icons/RightTriangle.png"));
    mUi->frame->hide();
}

void CollapsibleButton::open()
{
    mUi->pushButtonSettings->setIcon(QIcon(":/gex/icons/DownTriangle.png"));
    mUi->frame->show();
}

void CollapsibleButton::updateButtonSettings()
{
    if(mUi->frame->isHidden() == false)
    {
        close();
    }
    else
    {
        open();
    }
}

void CollapsibleButton::pinUnpin()
{
    if(mAttached)
    {
        if(mLayoutParent == 0)
            return;

        mIndexInLayout = mLayoutParent->indexOf(this);
        mLayoutParent->removeWidget(this);

        setParent(0, Qt::Dialog);
        setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowMinMaxButtonsHint);
        setWindowTitle(mTitle);
        //-- force the unfold
        open();
        show();
        move(QPoint(100, 100));
        raise();
        mAttached = false;
        UpdateIconPinUnPin(false);

    }
    else
    {
        hide();
        setParent(0);
        mLayoutParent->insertWidget(mIndexInLayout, this);
        mAttached = true;
        show();
        UpdateIconPinUnPin(true);

        //-- force the mainwindow to stay in foreground (in some case comes in top level)
        pGexMainWindow->lower();
    }
}



