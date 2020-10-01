#ifndef INFOWIDGET_H
#define INFOWIDGET_H

#include <QWidget>

namespace Ui {
class Info;
}

class InfoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit InfoWidget(QWidget *parent = 0);
    ~InfoWidget();

     /// \brief clear the content every label
     void Clear();

    /// \brief : hide or show the icon indicated that this is the production version
    void ProductionVersion      (bool state);

    void UpdateIdLabel          (const QString& label);
    void UpdateCreationLabel    (const QString& label);
    void UpdateExpirationLabel  (const QString& label);
    void UpdateComputationLabel (const QString& label);

private:

    Ui::Info* mUI;
};

#endif // SPMINFOWIDGET_H
