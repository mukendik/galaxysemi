#ifndef BUTTON_COLLAPSE_SETTINGS_H
#define BUTTON_COLLAPSE_SETTINGS_H

#include <QLayout>
#include <QWidget>

namespace Ui {
class button_collapse_settings;
}

/**
 * @brief The button_collapse_settings class enable to associated a expand/collapse effect to a widget
 * after clicking on a button
 */
class button_collapse_settings : public QWidget
{
    Q_OBJECT

public:
    explicit button_collapse_settings(const QString& text, QWidget *parent = 0);
    ~button_collapse_settings();

    /**
     * @brief addWidget - add a widget in the frame element. This widget will be dusplay or not
     * depending on the click on the button
     * @param widget
     */
    void addWidget(QWidget* widget);

    /**
     * @brief close the frame widget above
     */
    void close();

    /**
     * @brief open and display the fram widget above
     */
    void open();

public slots :
    void updateButtonSettings();

private:
    Ui::button_collapse_settings *ui;

    QLayout* mLayout;
};

#endif // BUTTON_COLLAPSE_SETTINGS_H
