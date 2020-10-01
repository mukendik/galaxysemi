#ifndef TEST_OVERALL_PROMIS_INTERPRETER_H
#define TEST_OVERALL_PROMIS_INTERPRETER_H

#include <QtTest>
#include <QScopedPointer>
#include "promis_interpreter_factory.h"
#include "test_doubles/testable_promis_interpreter.h"

using namespace Qx::BinMapping;

class TestOverallPromisInterpreter : public QObject
{
    Q_OBJECT

private :
    void InstantiatePromisInterpreter(Qx::BinMapping::PromisIntepreterTypes aPromisType);
    void ResetTestCase();
    void SetupPromisTest(PromisIntepreterTypes aPromisInterpreterType,
                         const QString &aPromisInputDirectoryName,
                         const QString &aPromisInputFileName,
                         const QString &aPromisKey );
    void SetupPromisTestPaths(const QString &aPromisInputDirectoryName, const QString &aPromisInputFileName);
    void SetupPromisInterpreter(PromisIntepreterTypes aPromisInterpreterType);
    void TestPromisInterpreter();
    void FailTest(PromisIntepreterTypes aPromisType ) const;
    void SetupPromisTestDirectoryPaths( const QString &aPromisType );
    void SetupPromisTestFilePaths(const QString &aPromisInputFile );
    void CompareOutputAndReferenceFileContents();

private slots:
    void initTestCase();
    void cleanupTestCase();

    void run_all_tests_data();
    void run_all_tests();

private :
    QScopedPointer< SerializableToFile > mTestablePromisInterpreter;
    QString mPromisInputDirectory;
    QString mPromisOutputDirectory;
    QString mPromisReferenceDirectory;
    QString mPromisInputFilePath;
    QString mPromisOutputFilePath;
    QString mPromisReferenceFilePath;
    QString mPromisKey;
    QString mConvertExternalFilePath;
};

#endif // TEST_OVERALL_PROMIS_INTERPRETER_H
