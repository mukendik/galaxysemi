#include <QRegularExpression>

#include "parser_test_expected_result.h"

ParserTestExpectedResult::ParserTestExpectedResult() :
    mStatus( Pass ),
    mMessageType( RegularExpression ),
    mMessage( QString( ".*" ) ) {}

ParserTestExpectedResult::ParserTestExpectedResult( TestExecutionStatuses aExpectedStatus,
                                                    MessageMatchTypes aMessageType,
                                                    const QString &aMessage ) :
    mStatus( aExpectedStatus ),
    mMessageType( aMessageType ),
    mMessage( aMessage ) {}

const char * ParserTestExpectedResult::GetStatusAsString() const
{
    if( mStatus == Pass )
        return "pass";

    return "fail";
}

const char * ParserTestExpectedResult::GetMessageMatchTypeAsString() const
{
    if( mMessageType == PlainText )
        return "plain text";

    return "regular expression";
}

bool ParserTestExpectedResult::IsConformTo(ParserTestExpectedResult::TestExecutionStatuses aStatus, const QString &aMessage) const
{
    return ( ( aStatus == mStatus ) && IsMessageMatching( aMessage ) );
}

bool ParserTestExpectedResult::IsMessageMatching(const QString &aMessage) const
{
    if( mMessageType == PlainText )
        return aMessage.compare( mMessage ) == 0;
    else if( mMessageType == RegularExpression )
        return
            QRegularExpression ( mMessage,
                                 QRegularExpression::DotMatchesEverythingOption ).match( aMessage ).hasMatch();

    return false;
}

const ParserTestExpectedResult ParserTestExpectedResult::sParserTestSuccess;
const ParserTestExpectedResult ParserTestExpectedResult::sParserTestFail( Fail, RegularExpression, QString( ".*" ) );
