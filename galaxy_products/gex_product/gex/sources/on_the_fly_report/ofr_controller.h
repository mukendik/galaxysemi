#ifndef OFRCONTROLLER_H
#define OFRCONTROLLER_H

#include <QString>
#include <QJsonObject>
#include <QObject>

#include "ofr_manager.h"
class Component;

namespace GS
{
namespace Gex
{

class OFR_Controller :  public QObject
{
    Q_OBJECT
public:
    /**
     * \brief GetInstance.
     */
    static OFR_Controller* GetInstance();

    /**
     * \fn bool OpenJsonFile
     * \brief this function opens the specified JSON report definition file and check is validity.
     * \param jsonConfigFile: the JSON report definition file.
     * \return true if the JSON report definition file has been successfully opened. Otherwise return false.
     */
    bool            OpenJsonFile(const QString &jsonConfigFile);

    /**
     * \fn bool LoadJsonFile
     * \brief this function loads the specified JSON report definition file and initializes some internal structures.
     * \param jsonConfigFile: the JSON report definition file.
     * \return true if the JSON report definition file has been successfully loaded. Otherwise return false.
     */
    bool            LoadJsonFile(const QString &jsonConfigFile);


    /**
      * @brief Refresh the view
      */
    void RefreshView();

    /**
     * \brief Reset.
     */
    void            Reset();
    /**
     * @brief ResetLastSectionSelected
     */
    void            ResetLastSectionSelected();

    /**
     * \fn bool SaveJsonObject
     * \brief this function saves the specified JSON report definition file. If it is for a Csl, we have to not save
     *        the parameter in the mJsonFileName
     * \param jsonConfigFile: the JSON report definition file.
     * \return true if the JSON report definition file has been successfully loaded. Otherwise return false.
     */
    bool            SaveJsonObject(const QString &jsonOFRConfigFile, bool forCsl=false);

    /**
     * \brief GetSettings.
     */
    QJsonObject& GetSettings();

    /**
     * \brief AddElementSettings.
     */
    void AddElementSettings(const QJsonObject &eltSettings);

    /**
     * \brief AddElementToSection.
     */
    void AddElementToSection(int indexSection, const QJsonObject &eltSettings);

    /**
     * \brief GetRoot.
     */
    Component * GetRoot() const;

    /**
     * \brief Hide.
     */
    void Hide();

    /**
     * \fn bool GenerateReport
     * \brief this function create a report from input file.
     * \param jsonFileName: the input file.
     * \param reportName: the output file.
     * \param param:a map (key, value), it contains for the moment the format of the output file("report_format"="pdf").
     * \return true if the generation has been done with success. Otherwise return false.
     */
    bool GenerateReport(const QString &jsonFileName,
                        const QString &reportName,
                        const QMap<QString, QString> &params);

    /**
     * \brief GenerateReport.
     */
    bool GenerateReport(const QString &pdfReportName);

    /**
     * \brief GetLastIndexSectionUpdated.
     */
    int GetLastIndexSectionUpdated() const;

    /**
     * \brief BuildListSection - Loop over the section component and build a list of there name
     * \return
     */
    QList<QString>      BuildListSection                    ();

    /**
     * \brief CanAddNewItem.
     */
    bool                CanAddNewItem                       (const QString& sectionName);

    /**
     * \brief UpdateLastIndexUsedAfterADeletion.
     */
    void                UpdateLastIndexUsedAfterADeletion   (int indexDeleted);

    /**
     * \brief IsReportEmpty.
     */
    bool                IsReportEmpty                       ();

    /**
     * \brief BuildContextualMenu.
     */
    QMenu* BuildContextualMenu(QMenu* parent, QAction** lastUpdate, QAction** newSection, QList<QAction*>& sections);

    /**
     * \brief SaveReportScript.
     */
    void SaveReportScript();
    /**
     * \brief Destroy.
     */
    static void Destroy();

    bool        IsEmptyReportBuilder() const;
public slots:
    /**
     * \brief OFR_Admin.
     */
    void            OFR_Admin();

private:
    static OFR_Controller* mInstance;        /// \param The OFR controler instance (static)

    /**
     * \brief Destructor.
     */
    ~OFR_Controller();

    /**
     * \brief default Constructor.
     */
    OFR_Controller();

    /**
     * \brief unimplemented copy Constructor.
     */
    OFR_Controller( const OFR_Controller & );

    /**
     * \brief unimplemented assignation operator.
     */
    OFR_Controller & operator = ( const OFR_Controller & );

    Component *mReportElements;             /// \param The report elements list
    Composite *mRoot;                       /// \param The OFR root element
    int       mIndex;                       /// \param The index

    int       mLastIndexSectionUpdated;     /// \param The last index section updated

    QString   mReportName;                  /// \param The OFR report name

    QJsonObject     mSettings;              /// \param The setting comming from the json file
    OFRManager*     mManager;               /// \param The OFR manager
    QList<QString>  mSectionsWithTable;     /// \param The list of sections those contain capability table
    QString         mJsonFileName;          /// \param The json file name
};

}
}

#endif // OFRCONTROLLER_H
