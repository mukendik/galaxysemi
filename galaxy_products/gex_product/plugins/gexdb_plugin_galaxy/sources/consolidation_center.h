#ifndef CONSOLIDATION_CENTER_H
#define CONSOLIDATION_CENTER_H

#include <QWidget>
#include <QPushButton>
#include <QTextEdit>
#include <QTableWidget>

class GexDbPlugin_Galaxy;

class ConsolidationCenter : public QWidget
{
    Q_OBJECT

public:
    ConsolidationCenter(QWidget* pw, GexDbPlugin_Galaxy* p);
    ~ConsolidationCenter();
    // Reload the conslidation tree
    // Will overwrite current text edit
    QString Reload();
    // Validate (XSD + custom validations)
    QString Validate();
    // Upload tree in DB
    QString Send();

private:

    void    showBottomWidget(bool show, int );

public slots:
    void OnSendButtonReleased();
    void OnValidateButtonReleased();
    void OnRevertButtonReleased();
    void OnTextChanged() { mSendButton->setEnabled(false); };
private slots:
    // Update TextEdit with the message
    void UpdateLogMessage(const QString &message, bool isPlainText);
private:
    GexDbPlugin_Galaxy* mPlugin;
    QPushButton* mValidateButton;
    QPushButton* mSendButton;
    QPushButton* mRevertButton;
    QTextEdit* mXMLTextEdit;
    QTextEdit* mLogTextEdit;
    QTableWidget* mTableWidget;
    QList<QTableWidgetItem*> mListTableWidgetItem;
    class XmlSyntaxHighlighter *mXmlSyntaxHighlighter;

};

#endif // CONSOLIDATION_CENTER_H
