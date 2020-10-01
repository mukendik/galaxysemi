#ifndef PARSER_TEST_EXPECTED_RESULT_H
#define PARSER_TEST_EXPECTED_RESULT_H

#include <QString>
#include <QRegExp>
#include <QMetaType>

class ParserTestExpectedResult
{
public :
    ParserTestExpectedResult();

public :
    enum MessageMatchTypes
    {
        PlainText,
        RegularExpression
    };

    enum TestExecutionStatuses
    {
        Pass,
        Fail
    };


public :
    ParserTestExpectedResult( TestExecutionStatuses aExpectedStatus,
                              MessageMatchTypes aMessageType,
                              const QString &aMessage );

public :
    const TestExecutionStatuses mStatus;
    const MessageMatchTypes mMessageType;
    const QString mMessage;

public :
    const char * GetStatusAsString() const;
    const char * GetMessageMatchTypeAsString() const;
    bool IsConformTo( TestExecutionStatuses aStatus, const QString &aMessage ) const;

public :
    // a default constructed test result, defining a sucess and a fail without any specific message
    static const ParserTestExpectedResult sParserTestSuccess;
    static const ParserTestExpectedResult sParserTestFail;

private :
    bool IsMessageMatching( const QString &aMessage ) const;
};

Q_DECLARE_METATYPE( ParserTestExpectedResult );

#define PARSER_TEST_SUCCESS ParserTestExpectedResult::sParserTestSuccess
#define PARSER_TEST_FAIL ParserTestExpectedResult::sParserTestFail

#endif // PARSER_TEST_EXPECTED_RESULT_H
