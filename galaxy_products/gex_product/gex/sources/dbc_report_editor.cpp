#include "dbc_report_editor.h"
#include "ui_dbc_report_editor.h"


DbcReportEditor::DbcReportEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DbcReportEditor)
{
    ui->setupUi(this);
}

DbcReportEditor::~DbcReportEditor()
{
    delete ui;
}

void DbcReportEditor::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
