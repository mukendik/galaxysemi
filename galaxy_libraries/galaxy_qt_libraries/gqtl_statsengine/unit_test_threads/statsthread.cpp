#include "statsthread.h"

#include <cmath> // for nan()
#include <string>


#include <QTextStream>
#include <QStringList>
#include <QFile>
#include <QDebug>
#include <QSize>
#include <QSharedPointer>
#include "r_vector.h"
#include "r_matrix.h"
#include "stats_engine.h"
#include "r_data.h"
#include "mv_groups_builder.h"
#include "mv_outliers_finder.h"
#include "stats_algo.h"
#include "r_algo.h"


StatsThread::StatsThread(const QString &appDir, int threadId):
    mAppDir(appDir), mThreadId(threadId), mIsThreadOK(true)
{

}

double** StatsThread::readCSVFile(QStringList &testsHeader, QStringList &partsHeader, const QString& inputFile)
{
    double** lDataFrame = NULL;
    printf("Thread#%d: Read %s \n", mThreadId, inputFile.toLatin1().data());

    QFile lCsv(inputFile);
    if (!lCsv.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        printf("Thread#%d: Unable to open file %s \n", mThreadId, inputFile.toLatin1().data());
        return 0;
    }
    QTextStream lStream(&lCsv);
    // get number of column
    int lNbTests = lStream.readLine().split(",").size();
    // get number of parts
    int lNbParts = 0;
    while(!lStream.atEnd())
    {
        lNbParts++;
        lStream.readLine();
    }
    lDataFrame = (double**) malloc(lNbParts*sizeof(double*));
    for (int i = 0; i < lNbParts; i++ )
        lDataFrame[i] = (double *) malloc(lNbTests*sizeof(double));

    // reset position
    lStream.seek(0);
    // read header
    testsHeader = lStream.readLine().split(",");
    testsHeader.removeFirst();

    int lPartIndex = 0;
    while(!lStream.atEnd())
    {
        QStringList lTests = lStream.readLine().split(",");
        partsHeader .append(lTests.takeFirst()); // remove parts name
        for(int lTest = 0; lTest < lTests.size(); lTest++)
        {
            bool lOk = true;
            double lValue = lTests.value(lTest).toDouble(&lOk);
            // to be more close to Gex environment
            float lFloatValue = lValue;
            lValue = lFloatValue;
            if (lOk)
                lDataFrame[lPartIndex][lTest] = lValue;
            else
                lDataFrame[lPartIndex][lTest] = NAN;
        }
        lPartIndex++;
    }

    return lDataFrame;
}


bool StatsThread::runAnalysis(GS::SE::StatsEngine *engine,
          double **dataFrame,
          const QStringList& testsHeader,
          const QStringList& partsHeader)
{
    QSharedPointer<GS::SE::RData> lRData(new GS::SE::RData());
    /*********************************************************************************************************/
    /*********************************************************************************************************/
    {
        printf("Thread#%d: Build groups! \n", mThreadId);

        if (!lRData->AllocateMatrix(GS::SE::StatsData::MVGROUP_IN_1,
                                        partsHeader.size(),
                                        testsHeader.size(),
                                        dataFrame))
        {
            printf("Thread#%d: Error: unable to allocate data matrix - %s \n", mThreadId, lRData->GetLastError().toLatin1().data());
            return false;
        }

        GS::SE::MVGroupsBuilder* lAlgo = static_cast<GS::SE::MVGroupsBuilder*>
                (engine->GetAlgorithm(GS::SE::StatsAlgo::MV_GROUPS_BUILDER));
        if (!lAlgo)
        {
            printf("Thread#%d: Error: no algorithm instanciated - %s \n", mThreadId, engine->GetLastError().toLatin1().data());
            return false;
        }

        QMap<GS::SE::StatsAlgo::Parameter,QVariant> lParam;
        lParam.insert(GS::SE::StatsAlgo::CORR_THRESHOLD, QVariant("0.8"));
        if (lAlgo->Execute(lParam, lRData.data()) == false)
        {
            printf("Thread#%d: Error: unable to execute algo - %s \n", mThreadId, lAlgo->GetLastError().toLatin1().data());
            return false;
        }

        bool lOk;

        int lGgroupsSize = lAlgo->GetGroupsSize();
        for (int i = 0; i < lGgroupsSize; ++i)
        {
            QString lDebug;
            lDebug.append("groups ").append(QString::number(i)).append(":");
            GS::SE::RVector lVector = lAlgo->GetGroup(i);
            if (lVector.IsEmpty())
                continue;
            for (int j = 0; j < lVector.GetSize(); ++j)
            {
                lDebug.append(" ").append(testsHeader.at(lVector.GetIntegerItem(j, lOk)));
                if (!lOk)
                {
                    printf("Thread#%d: Error: while extracting result - %s \n", mThreadId, lVector.GetLastError().toLatin1().data());
                    return false;
                }
            }
            printf("Thread#%d: %s \n", mThreadId, lDebug.toLatin1().data());
        }
    }

    return true;
}

bool StatsThread::isThreadOk() const
{
    return mIsThreadOK;
}


void StatsThread::run()
{
    QString lWorkDir(QDir::cleanPath(QString(getenv("DEVDIR")) + "/galaxy_libraries/galaxy_qt_libraries/gqtl_statsengine/unit_test_threads/data"));
    QString lInputFile(lWorkDir + QDir::separator() + "ds1_outliers.csv");
    QStringList lTestsHeader;
    QStringList lPartsHeader;
    double **lDataFrame = NULL;
    bool lStatus = false;


    QString lError;
    printf("Thread#%d: GALAXY_R_HOME: %s\n", mThreadId, QString(qgetenv("GALAXY_R_HOME")).toLatin1().data());
    GS::SE::StatsEngine* lEngine = GS::SE::StatsEngine::GetInstance(mAppDir, lError);
    if (!lEngine)
    {
        printf("Thread#%d: Error unable to get StatsEngine instance: %s\n", mThreadId, QString(lError).toLatin1().data());
        mIsThreadOK = false;
        return;
    }

    lDataFrame = readCSVFile(lTestsHeader,lPartsHeader, lInputFile);

    if (lDataFrame == NULL)
    {
        printf("Thread#%d: Error: no data frame created!\n", mThreadId);
        mIsThreadOK = false;
        return;
    }

    lStatus = runAnalysis(lEngine, lDataFrame, lTestsHeader,lPartsHeader);

    if (!lStatus)
    {
        printf("Thread#%d: Error while running analysis: %s\n", mThreadId, lEngine->GetLastError().toLatin1().data());
        mIsThreadOK = false;
        return;
    }

    GS::SE::StatsEngine::ReleaseInstance();
}

