#ifndef GEX_REGEXP_VALIDATOR_H
#define GEX_REGEXP_VALIDATOR_H

#include <QDialog>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;

class RegExpValidator : public QDialog
{
    Q_OBJECT

public:
    RegExpValidator(const QString &inputText, QWidget *parent = 0);
    ~RegExpValidator();
private slots:
    void refresh();

private:
    QLabel *patternLabel;
    QLabel *errorLabel;
    QLabel *escapedPatternLabel;
    QLabel *textLabel;
    QComboBox *patternComboBox;
    QLineEdit *errorEdit;
    QLineEdit *escapedPatternLineEdit;
    QComboBox *textComboBox;
    QCheckBox *advancedCheckBox;

    QLabel *indexLabel;
    QLabel *matchedLengthLabel;
    QLineEdit *indexEdit;
    QLineEdit *matchedLengthEdit;

    enum { MaxCaptures = 6 };
    QLabel *captureLabels[MaxCaptures];
    QLineEdit *captureEdits[MaxCaptures];
};

#endif // GEX_REGEXP_VALIDATOR_H
