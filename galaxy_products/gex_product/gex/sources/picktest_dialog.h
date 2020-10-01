#ifndef PICKTEST_DIALOG_H
#define PICKTEST_DIALOG_H

#include <QMap>
#include <QRegExp>
#include "ui_picktest_dialog.h"

class PickTestItem;
class CGexGroupOfFiles;
class CGexReport;
class CTest;

/////////////////////////////////////////////////////////////////////////////
class PickTestDialog : public QDialog, public Ui::PickTestDialogBase
{
    Q_OBJECT

public:

    enum TestTypeFlag
    {
        TestAll				= 0xff,
        TestParametric		= 0x01,
        TestMultiParametric	= 0x02,
        TestFunctional		= 0x04,
        TestGenericGalaxy	= 0x08
    };

    Q_DECLARE_FLAGS(TestType, TestTypeFlag)

    PickTestDialog(QWidget* parent = 0, bool modal = false, Qt::WindowFlags f = 0 );
    ~PickTestDialog();
    int			getGroupID(void);

    bool		isEmpty(void);

    bool		fillParameterList(int iGroup = -1, bool bForceBuildList = false);	// Test list built from file
    bool		fillParameterList(QStringList& strParameterList, const QStringList &pinNumber = QStringList());					// Test list given by an external object (Remote database, ...)

    void		setUseInteractiveFilters(bool bUseFilters);
    void		setMultipleSelection(bool bAllowMultiSelections);
    void		setMultipleGroups(bool bAllowMultiGroups, bool bAllowAllGroups);
    void		setAllowedTestType(PickTestDialog::TestType eTestType);				//bool bParametric, bool bMultiParametric, bool bFunctional);
    void		setBlockCompressionEnabled(bool enable);

    QString		testList(bool bCheckCustomList=true);
    QString		testItemizedList(bool bCheckCustomList=true);
    QStringList testItemizedListNames(void);

    QList<PickTestItem*> testSelected();


    /// !brief return the original name of the formated name
    QString     GetOriginalTestName(const QString) const;

public slots:

    void		OnSelectGroup(int iSelection);
    void		OnParamFilter(void);
    void		OnTestSelection(void);
    void        updatePickerFromFilter(const QString &);
    /*!
     * \fn OnChangeSyntax
     */
    void OnChangeSyntax(int lSyntax);
private:

    bool		buildParameterList(bool bForceBuildList);
    void		updateTreeWidget(int nGroupID);
    void		EnumerateGroupNames(void);
    bool		SetSelectionToCustomList(void);
    void        clearPickTestItemsList();
    void        fillPickTestItemsList(QStringList &parameterList, const QStringList &pinNumber = QStringList());
    void        fillPickTestItemsList(CGexReport *ptGexReport);
//    PickTestItem * pickTestItemFromList(const QString &testNumber, const QString &testName,
//                                        const QString &pinName, char testType); // Returns pointer to picktestItem if found, NULL otherwise

    QString		m_strFilterExpr;
    TestType	m_eTestTypeAllowed;
    bool		bAllowMultipleSelections;
    bool		bAllowMultipleGroups;
    bool		bAllowAllGroupsSelection;           // 'true' if user can select "All datasets"
    bool		m_bUseInteractiveFilters;
    bool		m_bItemizeList;                     // 'true' if must always return itemized list (eg: list visible is a filtered list)
    bool        mCompressBlocks;
    QList<PickTestItem*> m_pickTestItemsList;       // Initial List of PickTestItems
    QMap<QString, QString> mNameAndFormatedName;    /// !param map of formated name, original name

    /*!
     * \var mPatternSyntax
     */
    enum QRegExp::PatternSyntax mPatternSyntax;
};

class PickTestTreeWidgetItem : public QTreeWidgetItem
{

public:

    PickTestTreeWidgetItem(QTreeWidget * pTreeWidget);
    virtual ~PickTestTreeWidgetItem();

    bool operator< ( const QTreeWidgetItem & other ) const;

    PickTestItem*   mItem;
};

#endif // PICKTEST_DIALOG_H
