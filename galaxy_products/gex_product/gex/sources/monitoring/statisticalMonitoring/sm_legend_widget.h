#ifndef SMLEGENDWIDGET_H
#define SMLEGENDWIDGET_H

#include <QWidget>


namespace Ui
{
class SMLegend;
}

namespace GS
{
namespace Gex
{
/**
 *  Class that provides a legend UI for statistical monitoring widgets
 */
class SMLegendWidget : public QWidget
{
    Q_OBJECT
public:
    /// \brief Constructor
    explicit SMLegendWidget(QWidget *parent = 0);
    /// \brief Destructor
    ~SMLegendWidget();
    /// \brief Init legend colors
    void InitUI();

private:
    Ui::SMLegend* mUI;
};


} // namespace Gex
} // namespace GS

#endif // SMLEGENDWIDGET_H
