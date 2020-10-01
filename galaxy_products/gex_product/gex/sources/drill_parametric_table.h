#ifndef DRILL_PARAMETRIC_TABLE_H
#define DRILL_PARAMETRIC_TABLE_H

#include "loading.h"
#include "drill_table.h"
#include "renderer_keys.h"
#include "multi_limit_item.h"


#define	DRILL_PTEST_TOTAL_STATIC_COLS	60      // Total number of cols!


class DrillParametricTable : public DrillTable
{
    Q_OBJECT

public:
    /// \brief Constructor
    DrillParametricTable(GexWizardTable *parentWizard, QWidget * parent = 0, Loading* lLoading = 0);
    ~DrillParametricTable();
    /// \brief Reset table: Empties table, then loads it with all statistics info
    void	ResetTable();
    /// \brief Init column positions according to options
    void    InitColumnPositions();
    void InitShiftColumnLabels();
    /// \brief Set show/hide mode for functional tests
    void	SetHideFunctionalTests(bool hideFunctionalTests);
    /// \brief A column has to be sorted...
    void	sortColumn( int col, bool ascending, bool wholeRows);
    /// \brief return column position for a given column id
    int     ColPosition(const QString& column) const;
    /// \brief return the column identifier according to its position
    QString ColIdentifier(int columnPos) const;


    void toJson(QJsonObject &element, bool capabilityTable);
public slots:
    /// \brief Hide/Show Functional Tests has changed, Update the test table
    void	onHideFunctionalTests(bool);
    //GCORE-17548 overloading DrillTable for signal focusOut signal
    void focusOutEvent(QFocusEvent *) {/* do nothing*/}
private:
    bool	mHideFunctionalTests;           ///< True if functional tests are hidden
    static QMap<QString, QString> mSortCol; ///< Store corrsponding column name to the sorting name

    ///< return the value of the cell at the position (line, column) if it exists
    QString GetItemValue(int line, int column);

    ///< serialize the tests list to the json format
    void TestsListToJson(QJsonObject& lElt);
    ///< serialize the groups list to the json format
    void GroupsListToJson(QJsonObject& lElt);

    ///< serialize the fields list to the json format
    void FieldsListToJson(QJsonObject& lElt);

    void InitShiftHorizontalHeader(const QString & columnName);
    void InsertShiftPosition(const QString & columnName, int& lColumnIndex);
    void GetmultiLimitShift(CTest *testCell, const QString& groupName, GS::Core::MLShift &multiLimitShifts);
    void InitShiftHeaderSize(const QString &columnName);
};
#endif //DRILL_PARAMETRIC_TABLE_H
