#ifndef BASIC_REPORT_BUILDER_H
#define BASIC_REPORT_BUILDER_H

#include "utility.h"

#include <string>

class QJsonObject;

namespace GS
{
namespace Gex
{
/**
    \brief This class is responsible for the creation of the report, delegating
    the implementation detail to the type specified in template parameter list

    \tparam RENDERER The renderer type exposing the interface responsible of
    rendering some report parts
 */
template< class RENDERER >
    class BasicReportBuilder
    {
        /**
            \brief Alias on the renderer type
         */
        typedef RENDERER renderer_type;

        /**
            \brief Alias on the reference on the renderer type
         */
        typedef RENDERER & renderer_reference;

        /**
            \brief Alias on the reference on the constant renderer type
         */
        typedef const RENDERER & const_renderer_reference;

        /**
            \brief Alias on the type used to represent the description of a
            report
         */
        typedef QJsonObject report_description_type;

        /**
            \brief Alias on reference on the type used to represent the
            description of a report
         */
        typedef const QJsonObject & const_report_description_reference;

        /**
            \brief EBCO application for renderer_type that may be stateless
         */
        typedef base_opt< RENDERER, QJsonObject > embedded_description_type;

        /**
            \brief This is the full report serialized in JSON document. This is
            embedded in the base_opt instance to allow a full optimization in
            case of RENDERER is stateless
         */
         embedded_description_type m_Renderer_and_ReportDescription;

    public :
         /**
            \brief explicit construction of this report builder,
            default-initialized with a renderer instance that is default
            constructed

            \param report_description the JSON report description to render
            \param renderer an instance of the renderer
          */
        explicit BasicReportBuilder
            (
                const_report_description_reference report_description,
                const_renderer_reference renderer = renderer_type()
            ) :
            m_Renderer_and_ReportDescription( renderer, report_description ) {}

         /**
            \brief Build the report from its description using the specified
            renderer
          */
        void Build(const Composite& root);

        /**
            \brief obtains the report file extension, picked from the renderer
            type specified in the template parameter list

            \return the report file extension
         */
        std::string GetReportFileExtension() const;

        /**
            \brief save the built report in a file

            \param file_path the file path
         */
        void SaveAs( const std::string &file_path );

        /**
            \brief render the built report in a device whose type is forward  as
            reference to the specified renderer

            \tparam DEVICE the device type in which render the report

            \param file_path the path to the saved report file
            \param device a reference to a device in which render the built
            report
         */
        template< class DEVICE >
            void RenderReportFileIn
            ( const std::string &file_path, DEVICE &device );
    };
}
}

// include the implmentation from a separate file
#include "basic_report_builder.tcc"

#endif // BASIC_REPORT_BUILDER_H
