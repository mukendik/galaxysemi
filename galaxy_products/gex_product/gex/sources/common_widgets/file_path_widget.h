#ifndef FILEPATHWIDGET_H
#define FILEPATHWIDGET_H

#include <QGroupBox>

class QLineEdit;
class QPushButton;

class FilePathWidget: public QGroupBox
{
    Q_OBJECT
public:
    FilePathWidget(const QString& title,
                    const QString& caption,
                   const QString& filter,
                   const QString& defaultPath,
                   bool isChecked = true,
                   QWidget* parent = 0);
    QString GetFilePath() const;
    void SetFilePath(const QString& path);

private slots:
    void GetOpenFilePath();

private:
    void Init();

    bool            mIsChecked;
    QString         mTitle;
    QString         mFilePath;
    QString         mCaption;
    QString         mFilter;
    QLineEdit*      mFilePathEdit;
    QPushButton*    mGetPathButton;
};

#endif // FILEPATHWIDGET_H
