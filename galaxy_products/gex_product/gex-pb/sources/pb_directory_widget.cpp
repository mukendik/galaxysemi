#include "pb_directory_widget.h"

#include <QLineEdit>
#include <QToolButton>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QApplication>
#include <QFileDialog>

PbDirectoryWidget::PbDirectoryWidget(QWidget *parent)
    : QWidget(parent),
      mLineEdit(new QLineEdit),
      mBrowseButton(new QToolButton),
      mDefaultButton(new QToolButton)
{
  QHBoxLayout *lt = new QHBoxLayout(this);

  lt->setContentsMargins(0, 0, 0, 0);
  lt->setSpacing(0);
  lt->addWidget(mLineEdit);

  mBrowseButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);
  mBrowseButton->setFixedWidth(20);
  setFocusProxy(mBrowseButton);
  setFocusPolicy(mBrowseButton->focusPolicy());
  mBrowseButton->setText(tr("..."));
  mBrowseButton->installEventFilter(this);
  lt->addWidget(mBrowseButton);

  mDefaultButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);
  mDefaultButton->setFixedWidth(20);
  setFocusProxy(mDefaultButton);
  setFocusPolicy(mDefaultButton->focusPolicy());
  mDefaultButton->setIcon(QIcon(":/gex/icons/resetproperty.png"));
  mDefaultButton->setIconSize(QSize(16, 16));
  mDefaultButton->installEventFilter(this);
  lt->addWidget(mDefaultButton);

  mLineEdit->setReadOnly(true);
  mLineEdit->setText(mValue);

  connect(mDefaultButton,   SIGNAL(clicked()),
          this,             SLOT(buttonDefaultClicked()));
  connect(mBrowseButton,    SIGNAL(clicked()),
          this,             SLOT(buttonBrowseClicked()));
  connect(mLineEdit,        SIGNAL(textChanged(QString)),
          this,             SIGNAL(valueChanged(QString)));
}

bool PbDirectoryWidget::eventFilter(QObject *obj, QEvent *ev)
{
    if (obj == mBrowseButton || obj == mDefaultButton)
    {
        switch (ev->type())
        {
            case QEvent::KeyPress:
            case QEvent::KeyRelease:
                { // Prevent the QToolButton from handling Enter/Escape meant control the delegate
                    switch (static_cast<const QKeyEvent*>(ev)->key())
                    {
                        case Qt::Key_Escape:
                        case Qt::Key_Enter:
                        case Qt::Key_Return:
                            ev->ignore();
                            return true;
                        default:
                            break;
                    }
            }
            break;
            default:
                break;
        }
    }

    return QWidget::eventFilter(obj, ev);
}

void PbDirectoryWidget::setFilters(const QStringList &lFilters)
{
    mFilters = lFilters;
}

void PbDirectoryWidget::setDefaultValue(const QString &lDefaultValue)
{
    mDefaultValue = lDefaultValue;
}

void PbDirectoryWidget::setValue(const QString &value)
{
    if (mValue != value)
    {
        mValue = value;
        mLineEdit->setText(value);
    }

    mDefaultButton->setEnabled(mValue != mDefaultValue);
}

void PbDirectoryWidget::buttonBrowseClicked()
{
    QString lPathValue;

    if (mFilters.isEmpty())
    {
        lPathValue = QFileDialog::getExistingDirectory((QWidget*)this->parent(),
                                                   "Please select a directory",
                                                      mValue,
                                                      QFileDialog::ShowDirsOnly);
    }
    else
    {
        QFileDialog qfdFileDialog((QWidget*)this->parent(), QString("Please select a file"));

        qfdFileDialog.setOption(QFileDialog::DontUseNativeDialog, false);
        qfdFileDialog.setNameFilters(mFilters);

        if (qfdFileDialog.exec() == QDialog::Accepted)
            lPathValue = qfdFileDialog.selectedFiles().first();
    }

    if (lPathValue.isEmpty() == false)
        setValue(lPathValue);
}

void PbDirectoryWidget::buttonDefaultClicked()
{
    setValue(mDefaultValue);
}
