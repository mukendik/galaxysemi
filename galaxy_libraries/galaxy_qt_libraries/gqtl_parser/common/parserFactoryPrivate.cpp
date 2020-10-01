#include "parserFactoryPrivate.h"
#include "gqtl_log.h"
#include "gqtl_archivefile.h"

namespace GS
{
namespace Parser
{

ParserFactoryPrivate::ParserFactoryPrivate()
{

}

ParserFactoryPrivate::~ParserFactoryPrivate()
{
    qDeleteAll(mParsers);
    mParsers.clear();
}

ParserAbstract *ParserFactoryPrivate::CreateParser(const QString &lFileName)
{
    return CreateParser(FindParserType(lFileName));
}

ParserAbstract *ParserFactoryPrivate::CreateParser(ParserType lType)
{
    if (mParsers.contains(lType))
    {
        ParserFactoryItemBase * lParserItem = mParsers.value(lType);

        if (lParserItem != NULL)
            return lParserItem->Create();
    }

    return NULL;
}

ParserType ParserFactoryPrivate::FindParserType(const QString &lFileName)
{
    bool            lCompressed = false;
    CArchiveFile    lArchive;

    lCompressed = lArchive.IsCompressedFile(lFileName);

    QHash<ParserType, ParserFactoryItemBase *>::iterator itParser = mParsers.begin();
    while (itParser != mParsers.end())
    {
        if (lCompressed == false || itParser.value()->IsCompressedFormat())
        {
            if (itParser.value()->IsCompatible(lFileName))
                return itParser.value()->GetType();
        }

        ++itParser;
    }

    return typeUnknown;
}

void ParserFactoryPrivate::Initialize()
{
    // Register all parsers here
}

bool ParserFactoryPrivate::RegisterParser(/*const ParserType parserType,*/
                                          ParserFactoryItemBase *property)
{
    if (property != NULL)
    {
        mParsers.insert(property->GetType(), property);

        GSLOG(SYSLOG_SEV_NOTICE,
              QString("Registered parser %1").arg(property->GetType()).toLatin1().constData());

        return true;
    }
    return false;
}

bool ParserFactoryPrivate::UnregisterParser(ParserType lType)
{
    if (mParsers.contains(lType))
    {
        delete mParsers.take(lType);
        return true;
    }

    return false;
}

}
}
