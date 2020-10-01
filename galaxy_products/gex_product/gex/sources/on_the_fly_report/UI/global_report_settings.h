#ifndef GLOBAL_REPORT_SETTINGS_H
#define GLOBAL_REPORT_SETTINGS_H

#include <QWidget>

namespace Ui {
class global_report_settings;
}

class Component;

/**
 * @brief The global_report_settings class manages the GUI in charge of
 * the input regarding the global settings as the title, the comment
 */
class global_report_settings : public QWidget
{
    Q_OBJECT

public:
    explicit global_report_settings(QWidget *parent = 0);
    ~global_report_settings();

    /**
     * @brief clear the widgets of the GUI
     */
    void clear              ();

    /**
     * @brief init the GUI from a Component element
     */
    void loadReportElement  (Component* reportElement);

private slots:
    /**
     * @brief called when the text edit in charge of the comment has changed
     */
    void commentChanged ();

    /**
     * @brief called when the text label in charge of the name has changed
     */
    void nameChanged    (const QString& name);

    void keyPressEvent(QKeyEvent* event);

signals :

    /**
     * @brief signal emited when the title has been changed
     */
    void    titleReportChanged(const QString& name);

    /**
     * @brief emited whenever a change is made
     */
    void    changesHasBeenMade();

private:
    Ui::global_report_settings  *ui;
    Component                   *mReportElement;
};

#endif // GLOBAL_REPORT_SETTINGS_H
