#include "report_log.h"

namespace GS
{
namespace Gex
{

ReportLog::ReportLog(const QString &content, const ReportLogType &type) : mContent(content), mType(type)
{
}

ReportLog::ReportLog(const ReportLog &other):mContent(other.mContent),mType(other.mType)
{
}

ReportLog &ReportLog::operator=(const ReportLog &other)
{
    mContent = other.mContent;
    mType = other.mType;
    return *this;
}

ReportLog::~ReportLog()
{

}

void ReportLog::SetContent(const QString &content)
{
    mContent = content;
}

void ReportLog::SetType(const ReportLogType &type)
{
    mType = type;
}

QString ReportLog::GetContent() const
{
    return mContent;
}

ReportLog::ReportLogType ReportLog::GetType() const
{
    return mType;
}

}
}
