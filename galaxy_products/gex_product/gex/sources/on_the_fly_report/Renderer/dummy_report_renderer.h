#ifndef DUMMY_REPORT_RENDERER_H
#define DUMMY_REPORT_RENDERER_H

#include <string>

class QJsonObject;
class QDialog;

namespace GS
{
namespace Gex
{
/**
    \brief this is an example of a report renderer. This one has no state and
    is only limited to demonstrate the flexibility of the basic_report_builder
    class.

    This class define all that is mandatory in term of interface, constant and
    type exposure to be used with the basic_report_builder type.
    If you define a custome report renderer that miss one or several element,
    a compile time error will raise
 */
class DummyReportRenderer
{
public :
    /**
        \brief begin the rendering process
     */
    void BeginRendering();

    /**
        \brief end the rendering process
     */
    void EndRendering();

    /**
        \brief create a new page in a report

        \param description the json object describing a report page
     */
    void NewPage( const QJsonObject &description );

    /**
        \brief create a new section in the current page

        \param description the json object describing a report section
     */
    void NewSection( const QJsonObject &description );

    /**
        \brief add text in the current section

        \param description the json object describing a report text
     */
    void AddText( const QJsonObject &description );

    /**
        \brief add image in the current section

        \param description the json object describing a report image
     */
    void AddImage( const QJsonObject &description );

    /**
        \brief Give the extension of the file that will contain the rendered
        report data

        \return the file extension, here it will be a text file
     */
    std::string GetReportFileExtension() const;

    /**
        \brief save the built report in a file

        \param file_path the file to save path
     */
    void SaveAs( const std::string &file_path );

    /**
        \brief render a report file in a QDialog *

        \param file_path the report file path
        \param device the device in which render the report
     */
    void RenderReportFileIn
        ( const std::string &file_path, QDialog *device );
};

// forward declare the basic_report_builder template, see below
template< class > class BasicReportBuilder;

/**
    \brief Exposes an alias of the basic_report_builder parameterized with the
    dummy_report_renderer type to ease the use of the facility in client side
    code
 */
typedef BasicReportBuilder< DummyReportRenderer > DummyReportBuilder;

}
}

#endif // DUMMY_REPORT_RENDERER_H
