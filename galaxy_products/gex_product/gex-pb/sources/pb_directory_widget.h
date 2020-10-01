#ifndef PB_DIRECTORY_WIDGET_H
#define PB_DIRECTORY_WIDGET_H

#include <QWidget>

class QLineEdit;
class QToolButton;

class PbDirectoryWidget : public QWidget
{
    Q_OBJECT

public:

    PbDirectoryWidget(QWidget *parent);

    bool eventFilter(QObject *obj, QEvent *ev);
    void setFilters(const QStringList& lFilters);
    void setDefaultValue(const QString& lDefaultValue);

public Q_SLOTS:

    void setValue(const QString &value);

private Q_SLOTS:
    void buttonBrowseClicked();
    void buttonDefaultClicked();

Q_SIGNALS:

    void valueChanged(const QString &value);

private:
    QStringList     mFilters;
    QString         mValue;
    QString         mDefaultValue;
    QLineEdit *     mLineEdit;
    QToolButton *   mBrowseButton;
    QToolButton *   mDefaultButton;
};

#endif // PB_DIRECTORY_WIDGET_H
