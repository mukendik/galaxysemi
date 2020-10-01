#ifndef REPORT_LOG_H
#define REPORT_LOG_H

#include <QString>

namespace GS
{
namespace Gex
{
/*! \class Report log
 * \brief data class used to save Warning, Error, Information log when building the report
 *
 */
class ReportLog
{
public:
    /// enum log type can be error, warning, information
    enum ReportLogType
    {
        ReportError,
        ReportWarning,
        ReportInformation

    };

    /// \brief Constructor
    ReportLog(const QString &content, const ReportLogType &type);
    /// \brief Copy Constructor
    ReportLog(const ReportLog &);
    /// \brief operator=
    ReportLog &operator=(const ReportLog &);

    /// \brief Destructor
    virtual ~ReportLog();
    /// \brief Set log content
    void SetContent(const QString &text);
    /// \brief Set log type
    void SetType(const ReportLogType &type);
    /// \brief Get log content
    QString GetContent() const;
    /// \brief Get log type
    ReportLogType GetType() const;

private:
    QString mContent;
    ReportLogType mType;


};

}
}

#endif // REPORT_LOG_H
