#include "regexp_validator.h"

#include <QtGui>


RegExpValidator::RegExpValidator(const QString &inputText, QWidget *parent)
     : QDialog(parent)
 {
     patternComboBox = new QComboBox;
     patternComboBox->setEditable(true);
     patternComboBox->setSizePolicy(QSizePolicy::Expanding,
                                    QSizePolicy::Preferred);

     patternLabel = new QLabel("Regular expression:");
     patternLabel->setBuddy(patternComboBox);

     escapedPatternLineEdit = new QLineEdit;
     escapedPatternLineEdit->setReadOnly(true);
     QPalette palette = escapedPatternLineEdit->palette();
     palette.setBrush(QPalette::Base,
                      palette.brush(QPalette::Disabled, QPalette::Base));
     escapedPatternLineEdit->setPalette(palette);

     escapedPatternLabel = new QLabel("Escaped Pattern:");
     escapedPatternLabel->setBuddy(escapedPatternLineEdit);

     textComboBox = new QComboBox;
     textComboBox->setEditable(true);
     textComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

     textLabel = new QLabel("Text:");
     textLabel->setBuddy(textComboBox);

     errorEdit = new QLineEdit("");
     errorEdit->setReadOnly(true);
     palette = errorEdit->palette();
     palette.setColor(QPalette::Text, Qt::red);
     palette.setBrush(QPalette::Base,
                      palette.brush(QPalette::Disabled, QPalette::Base));
     errorEdit->setPalette(palette);
     QFont font = errorEdit->font();
     font.setStyle(QFont::StyleItalic);
     errorEdit->setFont(font);

     errorLabel = new QLabel("Syntax Error");
     errorLabel->setBuddy(errorEdit);


     advancedCheckBox = new QCheckBox("Advanced");
     advancedCheckBox->setChecked(false);

     indexLabel = new QLabel("Index of Match:");
     indexEdit = new QLineEdit;
     indexEdit->setReadOnly(true);

     matchedLengthLabel = new QLabel("Matched Length:");
     matchedLengthEdit = new QLineEdit;
     matchedLengthEdit->setReadOnly(true);

     for (int i = 0; i < MaxCaptures; ++i) {
         captureLabels[i] = new QLabel(QString("Capture %1:").arg(i));
         captureEdits[i] = new QLineEdit;
         captureEdits[i]->setReadOnly(true);
     }
     captureLabels[0]->setText("Match:");

     QHBoxLayout *checkBoxLayout = new QHBoxLayout;
     checkBoxLayout->addWidget(advancedCheckBox);
     checkBoxLayout->addStretch(1);

     QGridLayout *mainLayout = new QGridLayout;
     mainLayout->addWidget(textLabel, 0, 0);
     mainLayout->addWidget(textComboBox, 0, 1);
     mainLayout->addWidget(patternLabel, 1, 0);
     mainLayout->addWidget(patternComboBox, 1, 1);
     mainLayout->addWidget(captureLabels[0], 2, 0);
     mainLayout->addWidget(captureEdits[0], 2, 1);
     mainLayout->addWidget(errorLabel, 3, 0);
     mainLayout->addWidget(errorEdit, 3, 1);
     mainLayout->addLayout(checkBoxLayout, 4, 0, 1, 2);
     mainLayout->addWidget(indexLabel, 5, 0);
     mainLayout->addWidget(indexEdit, 5, 1);
     mainLayout->addWidget(matchedLengthLabel, 6, 0);
     mainLayout->addWidget(matchedLengthEdit, 6, 1);

     for (int j = 1; j < MaxCaptures; ++j) {
         mainLayout->addWidget(captureLabels[j], 7 + j, 0);
         mainLayout->addWidget(captureEdits[j], 7 + j, 1);
     }

     mainLayout->addWidget(escapedPatternLabel, 13, 0);
     mainLayout->addWidget(escapedPatternLineEdit, 13, 1);
     setLayout(mainLayout);

     connect(patternComboBox, SIGNAL(editTextChanged(QString)),
             this, SLOT(refresh()));
     connect(textComboBox, SIGNAL(editTextChanged(QString)),
             this, SLOT(refresh()));
     connect(advancedCheckBox, SIGNAL(toggled(bool)),
             this, SLOT(refresh()));

     if (!inputText.isEmpty())
         textComboBox->addItem(inputText);

     setWindowTitle("Gex Regular Expression Validator");
     setMinimumWidth(500);
     refresh();
 }

RegExpValidator::~RegExpValidator()
{
    delete patternLabel;
    delete errorEdit;
    delete errorLabel;
    delete escapedPatternLabel;
    delete textLabel;
    delete patternComboBox;
    delete escapedPatternLineEdit;
    delete textComboBox;
    delete advancedCheckBox;

    delete indexLabel;
    delete matchedLengthLabel;
    delete indexEdit;
    delete matchedLengthEdit;

    for (int i = 0; i < MaxCaptures; ++i)
    {
        delete captureLabels[i];
        delete captureEdits[i];
    }
}

 void RegExpValidator::refresh()
 {
     setUpdatesEnabled(false);

     QString pattern = patternComboBox->currentText();
     QString text = textComboBox->currentText();

     QString escaped = pattern;
     escaped.replace("\\", "\\\\");
     escaped.replace("\"", "\\\"");
     escaped.prepend("\"");
     escaped.append("\"");
     escapedPatternLineEdit->setText(escaped);

     QRegExp rx(pattern);

     bool isAdv = advancedCheckBox->isChecked();
     indexLabel->setVisible(isAdv);
     indexEdit->setVisible(isAdv);
     matchedLengthLabel->setVisible(isAdv);
     matchedLengthEdit->setVisible(isAdv);

     for (int j = 1; j < MaxCaptures; ++j) {
         captureLabels[j]->setVisible(isAdv);
         captureEdits[j]->setVisible(isAdv);
     }
     escapedPatternLabel->setVisible(isAdv);
     escapedPatternLineEdit->setVisible(isAdv);

     QPalette palette = patternComboBox->palette();
     if (rx.isValid())
     {
         errorEdit->setText("");
         palette.setColor(QPalette::Text,
                          textComboBox->palette().color(QPalette::Text));
     }
     else
     {
         errorEdit->setText(rx.errorString());
         palette.setColor(QPalette::Text, Qt::red);
     }
     patternComboBox->setPalette(palette);

     indexEdit->setText(QString::number(rx.indexIn(text)));
     matchedLengthEdit->setText(QString::number(rx.matchedLength()));
     for (int i = 0; i < MaxCaptures; ++i) {
         captureLabels[i]->setEnabled(i <= rx.captureCount());
         captureEdits[i]->setEnabled(i <= rx.captureCount());
         captureEdits[i]->setText(rx.cap(i));
     }

     setUpdatesEnabled(true);
     setFixedHeight(sizeHint().height());
 }

