
#include <QPushButton>
#include <QFileDialog>
#include <QLineEdit>
#include <QLayout>
#include <QFrame>

#include "file_path_widget.h"

FilePathWidget::FilePathWidget(
        const QString &title,
        const QString& caption,
        const QString& filter,
        const QString& defaultPath,
        bool isChecked /* = true*/,
        QWidget *parent):
    QGroupBox(parent),
    mTitle(title),
    mFilePathEdit(0),
    mGetPathButton(0),
    mFilePath(defaultPath),
    mCaption(caption),
    mIsChecked(isChecked),
    mFilter(filter)
{
    Init();
}

QString FilePathWidget::GetFilePath() const
{
    return mFilePath;
}

void FilePathWidget::SetFilePath(const QString& path)
{
    mFilePath = path;
    mFilePathEdit->setText(mFilePath);
}

void FilePathWidget::GetOpenFilePath()
{
    QString lFilePath =  QFileDialog::getOpenFileName(
                this, mCaption, mFilePath, mFilter);
    if (!lFilePath.isEmpty())
    {
        mFilePath = lFilePath.remove("\r").remove("\n");
        mFilePathEdit->setText(mFilePath);
    }
}

void FilePathWidget::Init()
{
    // Init group box
    setCheckable(true);
    setLayout(new QHBoxLayout());
    layout()->setContentsMargins(4, 4, 4, 4);
    setChecked(mIsChecked);
    setTitle(mTitle);
    // LineEdit
    mFilePathEdit = new QLineEdit(mFilePath, this);
    layout()->addWidget(mFilePathEdit);
    // Button
    mGetPathButton = new QPushButton();
    mGetPathButton->setIcon(QIcon(":/gex/icons/file_open.png"));
    mGetPathButton->setMaximumSize(QSize(32,32));
    layout()->addWidget(mGetPathButton);

    // Connect objects
    connect(mGetPathButton, SIGNAL(clicked(bool)),
            this, SLOT(GetOpenFilePath()));
}
