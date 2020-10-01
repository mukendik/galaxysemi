#ifndef DRILL_DEVICE_TABLE
#define DRILL_DEVICE_TABLE

#include <drill_table.h>

/// \class 'Device Result' table
class DrillDeviceTable : public DrillTable
{
    Q_OBJECT

public:
    /// \brief Constructor
    DrillDeviceTable(GexWizardTable *parentWizard ,QWidget* parent = 0, Loading *loading = 0 );
    /// \brief Indicates Test with no results must be hidden
    void	setHideTestsWithNoResults(bool lHideTestsWithNoResults);
    /// \brief A column has to be sorted...
    Q_INVOKABLE void sortColumn(int col, bool ascending, bool wholeRows);
    /// \brief Reset table: Empties table, then loads it with all info
    /// \param lSubLotIndex the list of elements to add. If = -1, add all parts
    Q_INVOKABLE void ResetTable(const gsint8 lSubLotIndex=-1);
    /// \brief Load Device Results table with relevant results
    Q_INVOKABLE QString LoadDeviceResults(long lRunOffset);
    /// \brief Update/fill the combobox with all PartIDs
    void updatePartResultComboBox(CGexGroupOfFiles *pGroup,
                                  CGexFileInGroup *pFile,
                                  const gsint8 lSubLotIndex=-1);

    void SetSubLotIndex(const gsint8 subLotIndex);

    gsint8 GetSubLotIndex() const;

    void UpdateMultiLimit(int multiLimit);
public slots:

    void	OnHideTestsWithNoResults(bool);
signals:

    void    onUpdatePartLabel(QString);

private:

    bool            mHideTestsWithNoResults;	///< True if tests with no results are hidden
    gsint8          mSubLotIndex;
};

class DrillDeviceWidget : public QWidget
{
public:
    DrillDeviceWidget(QWidget * parent = NULL);
    ~DrillDeviceWidget();

    DrillDeviceTable * GetDeviceTable();

private:

    QLabel *            mDeviceLabel;
    DrillDeviceTable *  mDeviceTable;
};

#endif // DRILL_DEVICE_TABLE

