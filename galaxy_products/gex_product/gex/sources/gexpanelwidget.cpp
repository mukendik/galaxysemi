#include "gexpanelwidget.h"

GexPanelWidget::GexPanelWidget(QWidget * pParent, GexPanelWidget::area areaZone /*= leftArea*/) : QWidget(pParent)
{
    m_eArea			= areaZone;
    m_eState		= stateOpen;

    m_pGridLayout	= new QGridLayout(this);
    m_pGridLayout->setSpacing(0);
    m_pGridLayout->setContentsMargins(0, 0, 0, 0);

    m_pVBoxLayout	= new QVBoxLayout();
    m_pVBoxLayout->setContentsMargins(0, 0, 0, 0);

    m_pVBoxLayout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Expanding));

    m_pGridLayout->addLayout(m_pVBoxLayout, 0, 0, 1, 1);

    m_pTabWidget	= new QTabWidget(this);
    m_pTabWidget->setTabPosition(QTabWidget::South);
    m_pGridLayout->addWidget(m_pTabWidget, 0, 1, 1, 1);

    m_maximumSize = maximumSize();
}

GexPanelWidget::~GexPanelWidget()
{
}

void GexPanelWidget::addPanelWidget(const QString& strName, QWidget * pPanelWidget)
{
    m_pTabWidget->addTab(pPanelWidget, strName);
}

void GexPanelWidget::showPanel()
{
    m_pTabWidget->show();
    m_pGridLayout->addWidget(m_pTabWidget, 0, 1, 1, 1);
}

void GexPanelWidget::hidePanel()
{
    m_pGridLayout->removeWidget(m_pTabWidget);
    m_pTabWidget->hide();
}

void GexPanelWidget::onOpenClose(bool /*bChecked*/)
{
    if (m_eState == stateOpen)
    {
        hidePanel();
        m_eState = stateClose;

        setMaximumSize(m_pVBoxLayout->maximumSize());
    }
    else
    {
        showPanel();
        m_eState = stateOpen;

        setMaximumSize(m_maximumSize);
    }
}
