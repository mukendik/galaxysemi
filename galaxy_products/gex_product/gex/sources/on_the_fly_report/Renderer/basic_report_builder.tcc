/**
    \file BasicReportBuilder.tcc

    \brief This file is a special kind of header.

    It is a commonly used standard way to implement method belonging to a class
    template.
    This file is included at the end of the file BasicReportBuilder.h
 */
#include "composite.h"
#include <QJsonObject>
#include <QJsonArray>

template< class RENDERER >
    void GS::Gex::BasicReportBuilder< RENDERER >::Build( const Composite& root )
    {
        ( void )root;

        // alias to get the renderer
        renderer_reference renderer = m_Renderer_and_ReportDescription;

        // alias to get the report description that is embedded
//        const_report_description_reference report_description = m_Renderer_and_ReportDescription.m_member;

        // parse the configuration and read all that is necessary
        // depending of that is read, this builder may need lot of method
        // to deal with sections...

        // we can imagine these object initialized from the exploration of
        // report_description
        // use the renderer interface to build the report in a predefined
        // order depending of the content of the description
        // just a dummy use of some capabilities of renderers

        renderer.BeginRendering();

        // Add the page title from the root
//        QString lReportTitle = report_description["report_name"].toString();
//        renderer.AddTitle(lReportTitle.toStdString());

        // Loop on all sections
//        const QList<Component *> &lSections = root.GetElements();
//        for (int i=0; i<lSections.size(); ++i)
//        {
//            Composite* lSection = static_cast<Composite*>(lSections[i]);
//            if (!lSection)
//            {
//                GSLOG(SYSLOG_SEV_ERROR, "Empty section");
//                return;
//            }
//            renderer.NewSection(*lSection);
//        }

        renderer.EndRendering();
    }

template< class RENDERER >
    std::string GS::Gex::BasicReportBuilder< RENDERER >::GetReportFileExtension() const
    {
        // alias to get the renderer
        const_renderer_reference renderer = m_Renderer_and_ReportDescription;

        return renderer.GetReportFileExtension();
    }

template< class RENDERER >
    void GS::Gex::BasicReportBuilder< RENDERER >::SaveAs(const std::string &file_path)
    {
        // alias to get the renderer
        renderer_reference renderer = m_Renderer_and_ReportDescription;

        // delegate the save to the renderer
        renderer.SaveAs( file_path );
    }

template< class RENDERER >
    template< class DEVICE >
    void GS::Gex::BasicReportBuilder< RENDERER >::RenderReportFileIn(const std::string &file_path, DEVICE &device)
    {
        // alias to get the renderer
        renderer_reference renderer = m_Renderer_and_ReportDescription;

        // delegate the work in the renderer
        renderer.RenderReportFileIn( file_path, device );
    }
