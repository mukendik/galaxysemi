#ifndef GTS_STATION_GTLWIDGET_H
#define GTS_STATION_GTLWIDGET_H

#include <QWidget>

namespace Ui {
class GtsStationGtlWidget;
}

class GtsStationGtlWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit GtsStationGtlWidget(QWidget *parent = 0);
    ~GtsStationGtlWidget();

private:
    Ui::GtsStationGtlWidget *ui;
    void    RefreshGui();

protected slots:
    void    OnComboGtlKeysChanged();
    void    OnButtonGetKey();
    void    OnButtonSetKey();
};

#endif // GTS_STATION_GTLWIDGET_H
