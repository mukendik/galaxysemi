#ifndef GEX_REGEXP_VALIDATOR_H
#define GEX_REGEXP_VALIDATOR_H

#include <QDialog>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;

namespace GS
{
namespace Gex
{
/*! \class RegExpValidator
 * \brief Gui tool to evaluate Regular expressions
 */
class RegExpValidator : public QDialog
{
    Q_OBJECT

public:
    /// \brief Constructor
    /// \param inputText if not empty is load in the text field
    RegExpValidator(const QString &inputText, QWidget *parent = 0);
    virtual ~RegExpValidator();
private slots:
    /// \brief reload the gui according to what is set in text and regexp by the user
    /// called for each key typed
    void refresh();

private:
    Q_DISABLE_COPY(RegExpValidator)
    /// \brief add or remove capture fields if needed according to regexp mode
    void loadCaptureFields(const QRegExp &rx, bool isAdvancedMode);

    /// GUI labels
    QLabel      *mRegExpLabel;
    QLabel      *mErrorLabel;
    QLabel      *mTextLabel;
    QLabel      *mIndexLabel;
    QLabel      *mMatchedLengthLabel;
    QLabel      *mRegExpEngineLabel;
    QLabel      *mCaseSensitivityLabel;
    QLabel      *mMatchStatusLabel;
    QLabel      *mMatchValueLabel;
    QLabel      *mMatchCommentLabel;
    /// GUI edit boxs
    QComboBox   *mRegExpComboBox;
    QLineEdit   *mErrorEdit;
    QComboBox   *mTextComboBox;
    QComboBox   *mRegExpEngineComboBox;
    QCheckBox   *mCaseSensitivityCheckBox;
    QCheckBox   *mAdvancedCheckBox;
    QLineEdit   *mIndexEdit;
    QLineEdit   *mMatchedLengthEdit;

    QList<QLabel *> mCaptureLabelsTMP;
    QList<QLineEdit *> mCaptureEditsTMP;
};

} // END Gex
} // END GS
#endif // GEX_REGEXP_VALIDATOR_H
