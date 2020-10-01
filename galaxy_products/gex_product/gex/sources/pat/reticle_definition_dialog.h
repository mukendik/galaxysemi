#ifndef RETICLE_DEFINITION_DIALOG_H
#define RETICLE_DEFINITION_DIALOG_H

#include <QDialog>
#include <QFlags>
#include <QLabel>
#include <QMouseEvent>
#include <QValidator>

#include "pat_option_reticle.h"

class COptionsPat;

namespace Ui {
class ReticleDefinitionDialog;
}

class ReticleDefinitionDialog : public QDialog
{
    Q_OBJECT

public:
    /// \brief Constructor
    explicit ReticleDefinitionDialog(QWidget *parent = 0);
    /// \brief Destructor
    ~ReticleDefinitionDialog();

    /// \brief Init UI with default values and Connect SIGNALS/SLOTS
    void InitGui(const COptionsPat &patOptions);
    /// \brief Load rule into UI
    void LoadGui(const PATOptionReticle &reticleRule);
    /// \brief extract data from UI and set it to rule
    void ReadGUI(PATOptionReticle& reticleRule);

public slots:
    /// \brief change die to bad or good die according to previous status
    void OnDieClicked(int row, int col);

private slots:
    /// \brief Draw demonstration of which area will be impacted
    void OnUpdateInkingArea();
    /// \brief Draw demonstration
    void OnUpdateInkingParameters();
    /// \brief Enable / Disable frames according to rules
    void OnUpdateEnabledFrames();
    /// \brief Check / Uncheck all corners depending on checkbox "All"
    void OnCheckAllCornerUpdated();
    /// \brief Check / Uncheck "All" check box based on all corners checkbox
    void OnCheckAnyCornerUpdated();
    /// \brief Show context menu to reset bad dies
    void OnCustomContextMenuInking(QPoint);
    /// \brief reset bad dies and re-paint
    void OnResetBadDiesRequested();
    /// \brief clear all bad dies and re-paint
    void OnClearBadDiesRequested();
    /// \brief Enable/Disable Ok button depending on certain fileds value
    void OnUpdateOkButton();
    /// \brief Hide/show controls based on selection
    void OnUpdateDefectivityCheckSection();

private:
    /// \brief Fill reticle mask with list from PAT options
    void FillReticleMaskList(const COptionsPat &patOptions);
    /// \brief Call all UI update slots
    void RefreshUI();
    /// \brief retrieve all checked edges/corners
    PATOptionReticle::ActivatedCorners GetCheckedCornerEdges();
    /// \brief init bad dies matrix
    void ResetBadDies();
    /// \brief Clear bad dies
    void ClearBadDies();

    QStringList                 mRules;         ///< holds existing rules name
    int                         mMaxRow;        ///< holds max rows
    int                         mMaxCol;        ///< holds max columns
    int**                       mBadDiesMatrix; ///< holds matrix of bad due to die
    Ui::ReticleDefinitionDialog *mui;           ///< UI ptr
};


class ClickableGridItemLabel : public QLabel
{
   Q_OBJECT

public:
   ClickableGridItemLabel(int row, int col, QWidget * parent = 0, Qt::WindowFlags f = 0)
       :QLabel(parent, f), mRow(row), mCol(col)
   {
   }
   ~ClickableGridItemLabel(){;}

signals:
   /// \brief signal sent when label is clicked
   void clicked(int, int);
protected:
   /// \brief implement mouse press event
   void mousePressEvent(QMouseEvent* e)
   {
       if (e->button() == Qt::LeftButton)
           emit clicked(mRow, mCol);
   }

   int mRow;  ///< holds row position in the Grid
   int mCol;  ///< holds col position in the Grid
};

#endif // RETICLE_DEFINITION_DIALOG_H
