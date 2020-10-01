#ifndef COLLAPSIBLE_BUTTON_H
#define COLLAPSIBLE_BUTTON_H

#include <QVBoxLayout>
#include <QWidget>
#include <QDialog>

namespace Ui {
class CollapsibleButton;
}


enum T_STYLE {T_Header, T_Informative };

/**
 * @brief The CollapsibleButton class enable to associated a expand/collapse effect to a widget
 * after clicking on a button
 */
class CollapsibleButton : public QWidget
{
    Q_OBJECT

public:
    explicit CollapsibleButton(const QString& text,T_STYLE style = T_Header, int sizePolicy = 0, QWidget *parent = 0);
    //CollapsibleButton(QWidget *parent = 0);

    ~CollapsibleButton();

    /**
     * @brief addWidget - add a widget in the frame element. This widget will be dusplay or not
     * depending on the click on the button
     * @param widget
     */
    void addWidget(QWidget* widget);

    void setTitle(const QString& title);

    /**
     * @brief close the frame widget above
     */
    void close();

    /**
     * @brief open and display the fram widget above
     */
    void open();

    /**
     * @brief Init the UI according to the settings or with teh default value
     */
    void InitUI();

    /**
     * @brief calling this function will enable the widget to be pin/unpin
     * The layout containing the widget must be adress in order to manage the
     * remove (unpin) and add (pin) action
     */
    void SetDetachable  (QVBoxLayout*  dialogParent);

public slots :
    void updateButtonSettings   ();
    void pinUnpin               ();

private:

    T_STYLE                 mStyle;
    Ui::CollapsibleButton * mUi;
    QVBoxLayout*            mLayout;
    QVBoxLayout*            mLayoutParent;   ///Hold the layout parent
    bool                    mAttached;
    int                     mIndexInLayout;
    int                     mSizePolicy;
    QString                 mTitle;

    /**
     * @brief update the icon pin/unpin
     */
    void UpdateIconPinUnPin(bool isPin);
};

#endif // COLLAPSIBLE_BUTTON_H
