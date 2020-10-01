
#include <qxchartnamesplitsextractor.h>
#include <QStringList>

QList<QString> QxChartNameSplitsExtractor::ExtractSplitsDataset(const QString& aChartName)
{
    int lEndPos = 0;
    QString lWorkingName = aChartName;
    QList<QString> lFilterNames;
    while((lEndPos = lWorkingName.indexOf(QStringLiteral("]"))) != -1)
    {
        int lStartPos = lWorkingName.indexOf(QStringLiteral("["));
        lFilterNames << lWorkingName.mid(lStartPos + 1, lEndPos - lStartPos - 1);

        lWorkingName =  lWorkingName.remove(0, lEndPos + 1) ;
    }

    return lFilterNames;
}

bool QxChartNameSplitsExtractor::SeparateSplitsNameValue(const QString& aChartName,
                                                             QList<QString>& aOutSplitsName,
                                                             QList<QString>& aOutSplitsValue)
{
    QList<QString> lListOfSplits = ExtractSplitsDataset(aChartName);

    if(lListOfSplits.isEmpty())
    {
        return false;
    }

    QList<QString>::iterator lIter(lListOfSplits.begin());
    QList<QString>::iterator lIterEnd(lListOfSplits.end());
    for(; lIter != lIterEnd; ++lIter)
    {
        QStringList lNameValue = (*lIter).split(QStringLiteral("="));
        if(lNameValue.size() != 2) continue;
        aOutSplitsName << lNameValue[0];
        aOutSplitsValue << lNameValue[1];
    }
    return true;
}


QString QxChartNameSplitsExtractor::ExtractRequestTitle(const QString& aChartName)
{
    int lIndex = aChartName.indexOf(QStringLiteral("["));
    if(lIndex == -1)
    {
        return QString();
    }
    return aChartName.mid(0, lIndex);
}

QString QxChartNameSplitsExtractor::ExtractTestName(const QString& aChartName)
{
    int lIndex = aChartName.indexOf(QStringLiteral("-"));
    if(lIndex == -1)
    {
        return QString();
    }

    return aChartName.mid(lIndex);
}

