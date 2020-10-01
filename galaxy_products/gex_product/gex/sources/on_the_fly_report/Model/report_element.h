#ifndef REPORTELEMENT_H
#define REPORTELEMENT_H

#include "component.h"


class CTest;
class CGexFileInGroup;
class CGexGroupOfFiles;

/**
 * @brief The ReportElement class is the base class of all unique element.
 */
class ReportElement : public Component
{

public:
    ReportElement(const QString& name, const QJsonObject &description, Component* parent, T_Component type);

    virtual ~ReportElement();

    /**
     * \brief this function export the report element to Json.
     */
    QJsonObject         ToJson();

    /**
     * @brief this function will read the json object desctription in order to initialized
     */
    virtual void        LoadJson();


    /**
     * @brief inform is a test is associated with this element and if it's activated
     */
    bool                IsTestActivated   ();

    /**
     * @brief activate or desactivate the test associated with the element if any
     */
    void                SetTestActivated  (bool activated);


    /**
     * @brief the test associated or an empty structure
     */
    const Test&         GetTestFilter();

    /**
     * @brief Inform if this element is bonded to the section
     * It means that this element will be drawn according to the section's test filter if any.
     */
    bool                IsTieWithSection      ()   const;

    /**
     * @brief Set/Unset the bond with the section
     * Be aware that the bond must be set, but not necesseraly activated
     * The activation will depend on the section's tests filter
     */
    void                SetTieWithSection     (bool status)   ;

    /**
     * @brief Inform if this element is under the control of the section
     * It means that this element will be drawnn according to the list of tests
     * associated with the section rather than the test associated with it.
     */
    bool                IsSectionSettingsControlActivated () const;
    void                SetSectionSettingsControlActivated(bool status);

protected:

     /**
      * @brief UpdateTestInChartOverlays -  update the json single chart test reference
      *  with the current one
      */
     virtual void       UpdateTestInChartOverlays(const Test& /*test*/, QJsonObject& /*chartOverlays*/) {}

    /**
     * @brief return if exists the corresponding CTest pointeur
     */
    CTest*              FindTestCell    (int testNumber, int pinIndex, const QString &testName, int dataSet, int fileIndex);

    /**
        \brief update the json object with update with the current element state
     */
    void                UpdateJson      ();

    /**
     * @brief Create a png image
     * @param testReferenceCell
     * @param prefix. Prefix of the png file that will be added
     * @param dataSet. Index of the data that must be used
     * @return the fullname of the png file created or an empty string if failled
     */
    QString             CreateImageName (CTest* testReferenceCell, const QString& prefix, int dataSet);

    /**
     * @brief Update the json object regarding the update that may have been done
     * through the GUI
     */
    void                UpdateTestJson  (QJsonObject& toUpdate);

    QString             mJsonRefName;    /// \distinguish the type of chart to look for in the json

    bool                mTestActivated;
    Test                mTest;
    bool                mTieWithSection;        ///\ if yes, top N and test list on section will prevail on the mTest
    bool                mSectionSettingsControlActivated; ///\ if yes the test associated will be ignored unless mTieWithSection = false
};

#endif // REPORTELEMENT_H
