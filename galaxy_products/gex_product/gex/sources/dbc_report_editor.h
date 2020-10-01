#ifndef DBC_REPORT_EDITOR_H
#define DBC_REPORT_EDITOR_H

#include <QDialog>

namespace Ui {
    class DbcReportEditor;
}

class DbcReportEditor : public QDialog
{
    Q_OBJECT

public:
    explicit DbcReportEditor(QWidget *parent = 0);
    ~DbcReportEditor();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::DbcReportEditor *ui;
};

#endif // DBC_REPORT_EDITOR_H
