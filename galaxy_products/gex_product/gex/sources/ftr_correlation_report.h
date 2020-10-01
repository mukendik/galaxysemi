#ifndef FTR_CORRELATION_REPORT_H
#define FTR_CORRELATION_REPORT_H

#include "gex_report_unit.h"

#include <QList>
#include <QPair>
#include <QMap>
#include <QString>


namespace GS
{
namespace Gex
{
    class FTRCorrelationReport :  public ReportUnit
    {
        public:
            class TestCorrelationData;
        protected:
            QList<int>m_oTestNumberList;
            QList <TestCorrelationData *> m_oCommonFailList;
        public:
            FTRCorrelationReport(CGexReport*, const QString& cslkey);
            ~FTRCorrelationReport();

            QList<int> &getTestNumberList () {
                return m_oTestNumberList;
            }

            QList <TestCorrelationData *> &getCommonFailList(){
                return m_oCommonFailList;
            }

            void clear();
            QString CreatePages();
            QString CloseSection();
            QString PrepareSection(bool bValidSection);
            void    exportToCSV(const QString &strFileName);


    protected:
            void buildFTRCorrelation (QList <TestCorrelationData *> &oCommonFailList, QList<int> &oTestNumberList, CTest *poTestList, CGexTestRange *poRange, const QList<CTest *> &oSelectedTestList);
            static TestCorrelationData *getCorrObj(QList <TestCorrelationData *> &oCommonFailList, TestCorrelationData *poCorr);


    public:
            class TestCorrelationData
            {
                    QPair <int, int> m_oTestCouple;
                    QList <int> m_oFaillingPartList;
                    QStringList m_oVectT1IdxList;
                    QStringList m_oVectT2IdxList;
                public:
                    TestCorrelationData(int iT1, int iT2);
                    void setPair(int iT1, int iT2);
                    ~TestCorrelationData();
                    void addFaillingPart(int iPart, const QString &strT1VectIdx, const QString &strT2VectIdx);
                    int count();
                    bool operator==(const TestCorrelationData& poObj) const;
                    bool operator==(const TestCorrelationData* &poObj) const;
                    QString generateBookmark();
                    QString generateHref();
                    QString generateToolTip();
                    QString generateHtmlSection(CGexReport *poGexReport);
                    QString generateCSVSection(CGexReport *poGexReport);



                    QPair <int, int> &getTestCouple();
                    QList <int> &getFallingPart();
                    QStringList &getVect1IdxList();
                    QStringList &getVect2IdxList();
                    static TestCorrelationData *buildFaillingPart(CTest *poTest1, CTest *poTest2);
                    static void getTestFaillingPart (CTest *poTest, QList <int> &oTestFaillingPart, QList <QString> &oTestVectorIdxFaillingPart);

            };
    };
} // namespace Gex
} // namespace GS

#endif // FTR_CORRELATION_REPORT_H
