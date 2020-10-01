#ifndef SATISTICAL_MONITORING_VERSION_WIDGET_H
#define SATISTICAL_MONITORING_VERSION_WIDGET_H

#include "mo_task.h"
#include <QWidget>
#include "ui_split_widget.h"


class CollapsibleButton;
class SPMVersionWidget;
class QTableView;
class SPMInfoWidget;

namespace GS
{
namespace Gex
{

class SplittedWindowWidget: public QWidget
{
    Q_OBJECT

    public:

        explicit SplittedWindowWidget(QWidget* parent = 0);
        ~SplittedWindowWidget();

        /// \brief add a widget in the right banner
        CollapsibleButton *CreateUnfoldWidgetInRightBanner(const QString &title, QWidget* widget);

        /// \brief add a widget in the left banner
        CollapsibleButton *CreateUnfoldWidgetInLeftBanner(const QString &title, QWidget* widget);

        /// \brief Init overall UI
        void InitUI();

    protected:
        /// \brief reimplement this function to add widgets in the right banner
        virtual void CustomizeRightBannerUI(QLayout*){}

        /// \brief reimplement this function to add widgets in the left banner
        virtual void  CustomizeLeftBannerUI(QLayout*) {}


    private:
        Ui::SplitWidget*        mUI;                        ///< Holds ptr to UI
        QList<CollapsibleButton*>    mRightBannerUnfoldButtons;

        /// \brief Init the right banner content
        void            InitRightBannerUI();

};


}
}

#endif
