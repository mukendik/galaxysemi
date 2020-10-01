#ifndef ITEM_REPORT_SETTINGS_H
#define ITEM_REPORT_SETTINGS_H

#include <QIcon>
#include <QWidget>

namespace Ui {
class item_report_settings;
}

class ReportElement;
/**
 * @brief The item_report_settings class manages the GUI to recive the input regarding
 * the item that will composed the report
 */
class item_report_settings : public QWidget
{
    Q_OBJECT

public:
    explicit item_report_settings(QWidget *parent = 0);
    ~item_report_settings();

    /**
     * @brief clear the gui
     */
    void    clear               ();

    /**
     * @brief load the GUI accprdong to the report element
     */
    void    loadReportElement   (ReportElement *reportElement);

    /**
     * @brief reLoad the GUI according the report element
     * Usefull when changes has been done in the report element
     */
    void    reLoad              ();

public slots:
    void    checkTestEmptyEntry     ();
    void    commentChanged          ();
    void    tieUntie                ();
    void    nameChanged             (const QString& name);
    void keyPressEvent(QKeyEvent* event);

signals:
    void reportElementNameChanged   (const QString& name);
    void newTest                    (ReportElement*);
    void changesHasBeenMade         ();
    void tieChanged                 (ReportElement*, bool);

private:

    /**
     * @brief fill the test label according to the Test in the report element
     */
    void    fillTest            ();
    void                        updateTieButton();

    Ui::item_report_settings*   ui;
    ReportElement*              mReportElement;
    QIcon                       mTieIcon;
    QIcon                       mUntieIcon;
    bool                        mIsTieWithSection;
};

#endif // ITEM_REPORT_SETTINGS_H
