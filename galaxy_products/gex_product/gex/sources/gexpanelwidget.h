#ifndef _GEX_PANEL_WIDGET_H
#define _GEX_PANEL_WIDGET_H

#include <QtWidgets>

class GexPanelWidget : public QWidget
{
    Q_OBJECT

public:

    enum area
    {
        leftArea,
        rightArea
    };

    GexPanelWidget(QWidget * pParent, area areaZone = leftArea);
    ~GexPanelWidget();

    void				addPanelWidget(const QString& strName, QWidget * pPanelWidget);
    QWidget*            currentTabWidget() { return m_pTabWidget->currentWidget(); }

protected:
    void				showPanel();
    void				hidePanel();

private:

    enum state
    {
        stateOpen,
        stateClose
    };

    enum mask
    {
        automatic,
        manual
    };

    QGridLayout *		m_pGridLayout;
    QVBoxLayout *		m_pVBoxLayout;

    state				m_eState;
    area				m_eArea;

    QTabWidget*         m_pTabWidget;

    QSize				m_maximumSize;

public slots:

    void				onOpenClose(bool);
};

#endif // _GEX_PANEL_WIDGET_H
