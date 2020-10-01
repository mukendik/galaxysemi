#ifndef JAVASCRIPTCENTER_H
#define JAVASCRIPTCENTER_H

#include <QCheckBox>
#include <QWidget>
#include <QLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QShortcut>
#include <QSpinBox>
#include <QPushButton>
#include "gex_scriptengine.h"
#include "qscriptsyntaxhighlighter_p.h"

extern GexScriptEngine* pGexScriptEngine;
extern bool UpdateGexScriptEngine(class CReportOptions* pReportOptions);

class JavaScriptCenter : public QWidget
{
  Q_OBJECT

    QTextEdit mJSTextEdit;
    //QTextEdit mConsoleTextEdit;
    QTextEdit mStatus;
    QPushButton mEvaluateButton;
    QPushButton mAbortButton;
    QSpinBox mProcessEventsInterval;
    QScriptSyntaxHighlighter mHighlighter;
    QShortcut mEvaluateShortcut;
    QVBoxLayout mVBLayout;
    QHBoxLayout mHBLayout;
    QLabel mProcessEventsLabel;
    QCheckBox mAttachDebuggerCB;
    bool mDebuggerPasswordOk;
public:
    JavaScriptCenter(QWidget *p);
    ~JavaScriptCenter();
public slots:
    void OnEvaluate();
    void OnAbort();
    bool OnSave();
    bool OnAttachDebuggerRequested();
};

#endif // JAVASCRIPTCENTER_H
