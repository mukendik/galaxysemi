#define R_NO_REMAP
//#include <R.h>
//#include <Rinternals.h>
//#include <Rembedded.h>
#include <cmath> // for nan()
#include <string>

//#include "gqtl_sysutils.h"
#include <QCoreApplication>
#include <QDir>
#include <QTextStream>
#include <QStringList>
#include <QFile>
#include <QDebug>
#include <QSize>
#include "r_vector.h"
#include "r_matrix.h"
#include "stats_engine.h"
#include "r_data.h"
#include "mv_groups_builder.h"
#include "mv_outliers_finder.h"
#include "stats_algo.h"
#include "r_algo.h"



double** readCSVFile(QStringList &testsHeader, QStringList &partsHeader, const QString& inputFile)
{
    double** lDataFrame = NULL;
    printf("Read %s \n", inputFile.toLatin1().data());

    QFile lCsv(inputFile);
    if (!lCsv.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        printf("nUnable to open file %s \n", inputFile.toLatin1().data());
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


bool runMultiVariateAnalysis(GS::SE::StatsEngine *engine,
          const QString& workingDir,
          double **dataFrame,
          const QStringList& testsHeader,
          const QStringList& partsHeader)
{
    bool lSuccess = false;
    GS::SE::RData* lRData = new GS::SE::RData();

    /*********************************************************************************************************/
    /*********************************************************************************************************/
    {
        printf("Build groups! \n");

        if (lRData->AllocateMatrix(GS::SE::StatsData::MVGROUP_IN_1,
                                        partsHeader.size(),
                                        testsHeader.size(),
                                        dataFrame) == false)
            return false;

        GS::SE::MVGroupsBuilder* lAlgo = static_cast<GS::SE::MVGroupsBuilder*>
                (engine->GetAlgorithm(GS::SE::StatsAlgo::MV_GROUPS_BUILDER));
        if (!lAlgo)
        {
            printf("Error: no algorithm instanciated - %s \n", engine->GetLastError().toLatin1().data());
            return false;
        }

        QMap<GS::SE::StatsAlgo::Parameter,QVariant> lParam;
        lParam.insert(GS::SE::StatsAlgo::CORR_THRESHOLD, QVariant("0.8"));
        if (lAlgo->Execute(lParam, lRData) == false)
        {
            printf("Error: unable to execute algo - %s \n", lAlgo->GetLastError().toLatin1().data());
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
                    printf("Error: while extracting result - %s \n", lVector.GetLastError().toLatin1().data());
                    return false;
                }
            }
            printf("%s \n", lDebug.toLatin1().data());
        }
    }
    /*********************************************************************************************************/
    /*********************************************************************************************************/

    {
        printf("Find outliers! \n");

        GS::SE::MVOutliersFinder* lAlgo = static_cast<GS::SE::MVOutliersFinder*>
                (engine->GetAlgorithm(GS::SE::StatsAlgo::MV_OUTLIERS_FINDER));
        if (!lAlgo)
        {
            printf("Error: no algorithm instanciated - %s \n", engine->GetLastError().toLatin1().data());
            return false;
        }

        GS::SE::RMatrix* lInputMatrix= lRData->AllocateMatrix(GS::SE::StatsData::MVOUTLIER_IN_1, partsHeader.size(), testsHeader.size() ,dataFrame);
        QMap<GS::SE::StatsAlgo::Parameter,QVariant> lParam;
        lParam.insert(GS::SE::StatsAlgo::SIGMA, "6");
        lSuccess = lAlgo->Execute(lParam, lRData);
        if (!lSuccess)
        {
            printf("Error: unable to execute algo - %s \n", lAlgo->GetLastError().toLatin1().data());
            return false;
        }

        bool lOk;

        GS::SE::RVector lVector;
        QStringList test;

        printf("outliers: \n");
        lVector = lAlgo->GetOutliers(lOk);
        if (!lOk)
        {
            printf("Error: unable to extract outliers - %s \n", lAlgo->GetLastError().toLatin1().data());
            return false;
        }
        if (!lVector.IsEmpty())
            for (int j = 0; j < lVector.GetSize(); ++j)
                test << QString("   o[%1]:%2\n").arg(QString::number(j)).arg(QString::number(lVector.GetDoubleItem(j, lOk)));
        printf("%s \n", test.join("").toLatin1().data());

        test.clear();
        lVector = lAlgo->GetZScores(lOk);
        if (!lOk)
        {
            printf("Error: unable to extract zscores - %s \n", lAlgo->GetLastError().toLatin1().data());
            return false;
        }
        printf("zscore(size:%s): \n", QString::number(lVector.GetSize()).toLatin1().data());
        if (!lVector.IsEmpty())
            for (int j = 0; j < lVector.GetSize(); ++j)
            {
                if (j > 625 && j < 640)
                    test << QString("   z[%1]:%2\n").arg(QString::number(j)).arg(QString::number(lVector.GetDoubleItem(j, lOk)));
            }
        printf("%s \n", test.join("").toLatin1().data());

        QSize lSize;
        lSize.setHeight(1200);
        lSize.setWidth(1800);
        GS::SE::RVector lLabels;
        lLabels.Build("histolabels", partsHeader.size(), GS::SE::RVector::V_STD);
        for (int i = 0; i < partsHeader.size(); ++i)
        {
            lLabels.FillStd(i, QString("PID-%1").arg(i));
        }

        if (lAlgo->BuildZScoresHisto(workingDir + QDir::separator() + "histo.png", lSize))
            printf("histo built with success! \n");
        else
        {
            printf("Error: unable to build histo - %s \n", lAlgo->GetLastError().toLatin1().data());
            return false;
        }

        if (lAlgo->BuildZScoresTrend(workingDir + QDir::separator() + "trend.png", lLabels, lSize))
            printf("trend built with success! \n");
        else
        {
            printf("Error: unable to build trend - %s \n", lAlgo->GetLastError().toLatin1().data());
            return false;
        }

        if (lAlgo->BuildZScoresQQPlot(workingDir + QDir::separator() + "qqplot.png", lSize))
            printf("qqplot built with success! \n");
        else
        {
            printf("Error: unable to build qqplot - %s \n", lAlgo->GetLastError().toLatin1().data());
            return false;
        }

        GS::SE::RMatrix lPCA = lAlgo->GetPCA(lOk);
        if (lAlgo->BuildCorrelationChart(lPCA, 0, 1, workingDir + QDir::separator() + "corrPCA.png", lLabels, lSize))
            printf("PCA corrchart built with success! \n");
        else
        {
            printf("Error: unable to build PCA corrchart - %s \n", lAlgo->GetLastError().toLatin1().data());
            return false;
        }

        if (lAlgo->BuildCorrelationChart(*lInputMatrix, 0, 1, workingDir + QDir::separator() + "corrX.png", lLabels, lSize))
            printf("X corrchart built with success! \n");
        else
        {
            printf("Error: unable to build X corrchart - %s \n", lAlgo->GetLastError().toLatin1().data());
            return false;
        }
    }

    delete lRData;

    return true;
}


int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    QString lWorkDir(QDir::cleanPath(QString(getenv("DEVDIR")) + "/galaxy_libraries/galaxy_qt_libraries/gqtl_statsengine/unit_test_multi_variate/data"));
    QString lInputFile(lWorkDir + QDir::separator() + "ds1_outliers.csv");
    QStringList lTestsHeader;
    QStringList lPartsHeader;
    double **lDataFrame = NULL;
    bool lStatus = false;

    printf("Working Dir: %s\n", lWorkDir.toLatin1().data());
    printf("Input File: %s\n", lInputFile.toLatin1().data());

    lDataFrame = readCSVFile(lTestsHeader,lPartsHeader, lInputFile);

    if (lDataFrame == NULL)
    {
        printf("Error: no data frame created!\n");
        return EXIT_FAILURE;
    }

    printf("GALAXY_R_HOME: %s\n", QString(qgetenv("GALAXY_R_HOME")).toLatin1().data());

    QString lError;
    GS::SE::StatsEngine* lEngine = GS::SE::StatsEngine::GetInstance(QCoreApplication::applicationDirPath(), lError);
    if (!lEngine)
    {
        printf("Error unable to get StatsEngine instance: %s\n", QString(lError).toLatin1().data());
        return EXIT_FAILURE;
    }
    lStatus = runMultiVariateAnalysis(lEngine, lWorkDir, lDataFrame, lTestsHeader,lPartsHeader);
    GS::SE::StatsEngine::ReleaseInstance();

    delete lEngine;

    if (lStatus)
    {
        printf("Test succesfully done!\n");
        return EXIT_SUCCESS;
    }
    else
    {
        printf("Error in unit test...\n");
        return EXIT_FAILURE;
    }
}

