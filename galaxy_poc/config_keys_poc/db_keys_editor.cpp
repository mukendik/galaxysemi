
#include "db_keys_editor.h"
#include "ui_db_keys_editor.h"
#include "gex_line_edit.h"
#include "regexp_validator.h"

#include <QCompleter>
#include <QLineEdit>

struct DbKeyData
{
    QString mName;
    QString mValue;
    QString mExpression;
    QString mEvaluatedValue;
};

struct StaticKeyRow
{
    QComboBox mComboName;
    QLineEdit mLineValue;
    QLineEdit mLineExpression;
    QLineEdit mLineEvaluatedValue;
};

struct DynamicKeyRow
{
    QComboBox mComboName;
    QLineEdit mLineExpression;
};


DbKeysEditor::DbKeysEditor(QWidget *parent) :
    QMainWindow(parent),
    mUi(new Ui::DbKeysEditor)
{
    mUi->setupUi(this);
    mRegExpValidator = 0;

    LoadGui();
}

DbKeysEditor::~DbKeysEditor()
{
    if (mRegExpValidator)
        delete mRegExpValidator;
    delete mUi;
}

void DbKeysEditor::LoadGui()
{
    connect(mUi->pushButtonRegExp, SIGNAL(clicked()),
            this, SLOT(OnShowRegExpValidatorRequested()));

    GexLineEdit *lineEdit = new GexLineEdit(this);
    mUi->frame->layout()->addWidget(lineEdit);

    setWindowIcon(QPixmap("galaxy.png"));
    setWindowTitle("Database Keys Editor");
}


void DbKeysEditor::OnShowRegExpValidatorRequested()
{
    QString strRegExp;
    ShowRegExpValidator(strRegExp);
}

void DbKeysEditor::ShowRegExpValidator(const QString &strRegExp)
{
    if (!mRegExpValidator)
        mRegExpValidator = new RegExpValidator(strRegExp, this);

    // Set RegExp or input Textif needed

    // Show it
    mRegExpValidator->show();
}


