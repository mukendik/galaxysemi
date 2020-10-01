/******************************************************************************!
 * \file report_template_gui.h
 ******************************************************************************/
#ifndef REPORT_TEMPLATE_GUI_H
#define REPORT_TEMPLATE_GUI_H

#include "ui_wizard_custom_report_admin_dialog.h"
#include "ui_wizard_custom_report_edit_dialog.h"
#include "report_template.h"  // FIXME: duplicate icMarkerWidth ?

namespace GS {
namespace Gex {

class ReportTemplate;
class ReportTemplateIO;
class ReportTemplateSection;

/******************************************************************************!
 * \class ReportTemplateWizard_EditSection
 * \brief Dialog box: Edit section
 ******************************************************************************/
class ReportTemplateGui_EditSection : public QDialog,
    public Ui::EditReportSectionDialogBase
{
    Q_OBJECT

public:
    /*!
     * \fn ReportTemplateWizard_EditSection
     * \brief Constructor
     */
    ReportTemplateGui_EditSection(QWidget*   parent = 0,
                                  bool       modal = false,
                                  Qt::WindowFlags fl = 0);
    /*!
     * \fn getSectionDetails
     * \brief Retrieve section details from GUI fields
     */
    void getSectionDetails(ReportTemplateSection* pSection,
                           bool                   bCopy = false);
    /*!
     * \fn fillFields
     * \brief Preload fields from existing section
     *        (used for editing existing section)
     */
    void fillFields(ReportTemplateSection* pSection);

private:
    /*!
     * \fn clear
     */
    void clear();
    /*!
     * \var cColor
     * \brief Line color
     */
    QColor cColor;
    /*!
     * \var icMarkerWidth
     * \brief Markers: Line width (0=hide)
     */
    int icMarkerWidth[GEX_CRPT_MARKER_TOTALSIZE];
    /*!
     * \var cMarkerColor
     * \brief Markers: color
     */
    QColor cMarkerColor[GEX_CRPT_MARKER_TOTALSIZE];

public slots:
    /*!
     * \fn OnOk
     */
    void OnOk();
    /*!
     * \fn OnSectionType
     */
    void OnSectionType();
    /*!
     * \fn OnSectionAggregate
     */
    void OnSectionAggregate();
    /*!
     * \fn OnPickAggregateTest
     */
    void OnPickAggregateTest();
    /*!
     * \fn OnPickAggregateTestY
     */
    void OnPickAggregateTestY();
    /*!
     * \fn OnAggregateLineColor
     * \brief Define pen color
     */
    void OnAggregateLineColor(const QColor&);
    /*!
     * \fn OnAggregateMarkerSelection
     * \brief Customizing a marker
     */
    void OnAggregateMarkerSelection();
    /*!
     * \fn OnAggregateMarkerColor
     * \brief Define Marker color
     */
    void OnAggregateMarkerColor(const QColor&);
    /*!
     * \fn OnAggregateMarkerSize
     * \brief Define Marker pen size
     */
    void OnAggregateMarkerSize();
    /*!
     * \fn OnSectionWafmap
     */
    void OnSectionWafmap();
    /*!
     * \fn OnPickTestWafermap
     */
    void OnPickTestWafermap();
    /*!
     * \fn OnCorrelationGbSelectFile
     */
    void OnCorrelationGbSelectFile();
    /*!
     * \fn OnCreateGuardBandFile
     */
    void OnCreateGuardBandFile();
    /*!
     * \fn OnSectionDataAudit
     */
    void OnSectionDataAudit();
    /*!
     * \fn OnAddFilter
     */
    void OnAddFilter();
    /*!
     * \fn OnEditFilter
     */
    void OnEditFilter();
    /*!
     * \fn OnDuplicateFilter
     */
    void OnDuplicateFilter();
    /*!
     * \fn OnRemoveFilter
     */
    void OnRemoveFilter();
    /*!
     * \fn OnRemoveAllFilters
     */
    void OnRemoveAllFilters();
    /*!
     * \fn onPickTestDatalog
     */
    void onPickTestDatalog();
    /*!
     * \fn onTestTypeDatalog
     */
    void onTestTypeDatalog();
};

/******************************************************************************!
 * \class ReportTemplateGui
 * \brief Dialog box: Custom report Wizard
 ******************************************************************************/
class ReportTemplateGui : public QDialog,
    public Ui::ReportTemplateWizardDialogBase
{
    Q_OBJECT

public:
    /*!
     * \fn ReportTemplateGui
     * \brief Constructor
     */
    ReportTemplateGui(QWidget*   parent = 0,
                      bool       modal = false,
                      Qt::WindowFlags fl = 0);
    /*!
     * \fn ~ReportTemplateGui
     * \brief Destructor
     */
    ~ReportTemplateGui();
    /*!
     * \fn dragEnterEvent
     */
    void dragEnterEvent(QDragEnterEvent*);
    /*!
     * \fn dropEvent
     */
    void dropEvent(QDropEvent*);
    /*!
     * \fn OnLoadReportTemplate
     */
    bool OnLoadReportTemplate(QString strTemplateFile, bool bQuietLoad = false);
    /*!
     * \fn getReportTemplate
     */
    ReportTemplate* getReportTemplate();
    /*!
     * \fn getReportTemplateIO
     */
    ReportTemplateIO* getReportTemplateIO();

    /*!
     * \var m_bTemplateModified
     * \brief true if template modified and requires to be saved
     */
    bool m_bTemplateModified;
    /*!
     * \var m_strTemplateFile
     * \brief Full path to Template file to create or edit
     */
    QString m_strTemplateFile;

private:
    /*!
     * \var mReportTemplate
     */
    ReportTemplate* mReportTemplate;
    /*!
     * \var mReportTemplateIO
     */
    ReportTemplateIO* mReportTemplateIO;
    /*!
     * \fn OnSaveReportTemplate
     */
    void OnSaveReportTemplate(bool bAskConfirm);
    /*!
     * \fn SetSelection
     */
    void SetSelection(QString strFrontPageText, QString strFrontPageImage);
    /*!
     * \fn UpdateGUI
     */
    void UpdateGUI();
    /*!
     * \fn UpdateGuiSectionsList
     */
    void UpdateGuiSectionsList();

public slots:
    /*!
     * \fn reject
     * \brief Called if user hits the ESCAPE key
     */
    void reject();
    /*!
     * \fn OnOk
     */
    void OnOk();
    /*!
     * \fn OnLoadReportTemplate
     */
    void OnLoadReportTemplate();
    /*!
     * \fn OnSaveReportTemplate
     */
    void OnSaveReportTemplate();
    /*!
     * \fn OnTemplateModified
     */
    void OnTemplateModified();
    /*!
     * \fn OnSelectLogo
     */
    void OnSelectLogo();
    /*!
     * \fn OnRemoveLogo
     */
    void OnRemoveLogo();
    /*!
     * \fn OnNewSection
     */
    void OnNewSection();
    /*!
     * \fn OnAutoFill
     */
    void OnAutoFill();
    /*!
     * \fn OnEditSection
     */
    void OnEditSection();
    /*!
     * \fn OnDuplicateSection
     */
    void OnDuplicateSection();
    /*!
     * \fn OnDeleteSection
     */
    void OnDeleteSection();
    /*!
     * \fn OnDeleteAllSections
     */
    void OnDeleteAllSections();
    /*!
     * \fn OnMoveSectionUp
     */
    void OnMoveSectionUp();
    /*!
     * \fn OnMoveSectionDown
     */
    void OnMoveSectionDown();

private slots:
    /*!
     * \fn OnSaveAsReportTemplate
     */
    void OnSaveAsReportTemplate();
};

}
}

#endif
