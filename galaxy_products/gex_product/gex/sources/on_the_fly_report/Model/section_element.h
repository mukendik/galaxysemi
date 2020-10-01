#ifndef SECTION_ELEMENT
#define SECTION_ELEMENT

#include <QStringList>
#include "composite.h"

enum T_TestListFilter { T_TESTS_LIST = 0, T_TESTS_TOPN, T_TESTS_NOFILTER};
enum T_GroupListFilter { T_GROUPS_LIST, T_GROUPS_NOFILTER};

/**
 * @brief The SectionElement class is the representation in memory of a section from a report point of view
 * this object will contain the information for the build of the json and the initialization of the GUI
 */
class SectionElement: public Composite
{

public :
    SectionElement(const QString &name, Component *parent, const QJsonObject& jsonDescription=QJsonObject());
    ~SectionElement();


    /**
     * @brief initialized the instance from a json objetc
     */
    bool                    LoadJson             (const QJsonObject &description);

    /**
     * @brief clear the list of tests filter if any
     */
    void                    ClearTestsFilter    ();

    /**
     * @brief clear the list of groups filter if any
     */
    void                    ClearGroupsFilter    ();

    /**
     * @brief AddTestFilter
     */
    void                    AddTestFilter       (const QString& testNumber,
                                                 const QString& testName,
                                                 const QString& pinName,
                                                 int groupId,
                                                 int fileIndex);

    /**
     * @brief AddGroupFilter
     */
    void                    AddGroupFilter       (const QString& groupNumber,
                                                 const QString& groupName);


    ///\brief Getter/Setter
    void                    SetTestListFilterType(T_TestListFilter type);
    void                    SetGroupListFilterType(T_GroupListFilter type);
    T_TestListFilter        GetTestListFilterType() const;
    T_GroupListFilter       GetGroupListFilterType() const;
    QList<Test*>            GetTestsListFilter   () const;
    QList<Group*>           GetGroupsListFilter   () const;
    QList<Group*>           GetGroupsList   () const;

    /**
     * @brief Set a Top N failed value
     */
    void                    SetTopNValue         (int topNValue);

    /**
     * @brief Retrieve the Top N failed value
     */
    int                     GetTopNValue         ()  const;

    /**
     * @brief Set a split rule
     */
    void                    SetSplitByGroup         (bool value);

    /**
     * @brief Retrieve the split rule
     */
    bool                    IsSplitByGroup         ()  const;

    bool HasTiedChildElements() const;
    bool ContainsTable() const;
    void TiedAllChild();


protected:

    void                    UpdateElementsBeforeAdded(Component* element);

   /**
    * @brief UpdateJson - reimplementation of the Component::UpdateJson()
   */
   void                     UpdateJson();

    T_TestListFilter        mTestFilterType;
    T_GroupListFilter       mGroupFilterType;
    QList<Test*>            mTestsFilter;
    QList<Group*>           mGroupsFilter;
    int                     mTopNValue;
    bool                    mSplitByGroup;


};

#endif // SECTION_ELEMENT

