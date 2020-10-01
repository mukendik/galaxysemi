#ifndef RBORROW_DIALOG_H
#define RBORROW_DIALOG_H

#include <QDialog>

namespace Ui {
class rborrow_dialog;
}

class RBorrow_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit RBorrow_dialog(QWidget *parent = 0);
    ~RBorrow_dialog();

    QString     GetServer() const;
    int         GetPort() const;

    void        SetServer(const QString& server);
    void        SetPort(int port);

protected slots:

    void        UpdateUI();

private:

    Ui::rborrow_dialog *ui;
};

#endif // RBORROW_DIALOG_H
