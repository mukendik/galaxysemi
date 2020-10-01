#ifndef GEXLOGWIDGET_H
#define GEXLOGWIDGET_H

#include <QLabel>
#include <QSpinBox>
#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QCheckBox>
#include <QVBoxLayout>
#include "syslog_server.h"

//extern QMap<QString, int>* s_LogLevels;

class CGexLogWidget : public QWidget
{
	Q_OBJECT

public:
	CGexLogWidget(QWidget* p);//:QWidget(p), m_currentRow(0);
    virtual ~CGexLogWidget();
    //static CGexLogWidget* s_pWidget;

public slots:
    // Set the level for the given module
    bool setLogLevel(QString mn, int l);
    void onLogLevelActionTriggered(QAction*);
    bool addLogInMenu(QString mn, int l);
    //QString AddLogLevelMenu(const QString &modulename, int currentlevel);
    //
    void onShow();
    //
	void onCellDoubleClicked(int x, int y);
    //
    QString AppendMessage(SMessage &sm);
    //
    void OnClear();

signals:
    void sLogLevelChanged(int);

private:
    QLabel mMaxLogLevelLabel;
    QSpinBox mLogLevelSpinBox;
    QTableWidget m_tw;
	QPushButton m_llpb;
    QPushButton mClearButton;
    QCheckBox mReplaceCRinMessageCB;
    QHBoxLayout mTopHBoxLayout;
    QVBoxLayout mVBoxLayout;
	int m_currentRow;
	QTableWidgetItem* GetItem(int x, int y);
    QVector<QTableWidgetItem> mTWIBuffer;
    unsigned mCurrentTWIindex;

};

#endif // GEXLOGWIDGET_H
