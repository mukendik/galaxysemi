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
    printf("\nRead %s \n", inputFile.toLatin1().data());

    QFile lCsv(inputFile);
    if (!lCsv.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        printf("\nUnable to open file %s \n", inputFile.toLatin1().data());
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


/*int test1()
{

    RApplication R ;
    double **lDataFrame = NULL;
    QStringList lTestsHeader;
    QStringList lPartsHeader;

    lDataFrame = readCSVFile(lTestsHeader,lPartsHeader);

    // generate some data
    R.generate_dataset_2(lDataFrame,lTestsHeader.size(),lPartsHeader.size()) ;

//    printf("R_NaN=%g %f\n", R_NaN, R_NaN);

//    // run a script
//    int success = R.run_script( "scripts/mean.R" ) ;
//    int success = R.run_script( "E:/galaxy_repositories/galaxy_dev_v72/galaxy_poc/mv_pat_poc/R/mv_group_builder.R" );
    int success = R.run_script( "E:/galaxy_repositories/galaxy_dev_v72/galaxy_poc/mv_pat_poc/R/mv_outliers_finder.R");

    // get results
    if( success ){
        int result = R.get_result_2() ;
        Rprintf( "outliers: %d\n", result) ;
    }
    else
        printf("error\n");

    return !success ;
}*/


int test2(GS::SE::StatsEngine *engine,
          const QString& workingDir,
          double **dataFrame,
          const QStringList& testsHeader,
          const QStringList& partsHeader)
{
    if (!engine)
        return false;

    if (dataFrame == NULL)
        return false;

    bool lSuccess = false;

    GS::SE::RData* lStatsData2 = new GS::SE::RData();


    /*********************************************************************************************************/
    /*********************************************************************************************************/
    {
        qDebug() << "Build groups!";

        if (lStatsData2->AllocateMatrix(GS::SE::StatsData::MVGROUP_IN_1,
                                        partsHeader.size(),
                                        testsHeader.size(),
                                        dataFrame) == false)
            return false;

        QMap<GS::SE::StatsAlgo::Parameter,QVariant> lParam;
        lParam.insert(GS::SE::StatsAlgo::CORR_THRESHOLD, QVariant("0.8"));
        if (engine->GetAlgorithm(GS::SE::StatsAlgo::MV_GROUPS_BUILDER)->Execute(lParam, lStatsData2) == false)
            return false;

        bool ok;
        GS::SE::MVGroupsBuilder* lAlgo = static_cast<GS::SE::MVGroupsBuilder*>(engine->GetAlgorithm(GS::SE::StatsAlgo::MV_GROUPS_BUILDER));

        int lGgroupsSize = lAlgo->GetGroupsSize();
        for (int i = 0; i < lGgroupsSize; ++i)
        {
            QString lDebug;
            lDebug.append("groups ").append(QString::number(i)).append(":");
            GS::SE::RVector lVector = lAlgo->GetGroup(i);
            if (lVector.IsEmpty())
                continue;
            for (int j = 0; j < lVector.GetSize(); ++j)
                lDebug.append(" ").append(testsHeader.at(lVector.GetIntegerItem(j, ok)));
            qDebug() << lDebug;
        }
    }
    /*********************************************************************************************************/
    /*********************************************************************************************************/

    {
        qDebug() << "Find outliers!";

        if (dataFrame == NULL)
            return false;

        GS::SE::RMatrix* lInputMatrix= lStatsData2->AllocateMatrix(GS::SE::StatsData::MVOUTLIER_IN_1, partsHeader.size(), testsHeader.size() ,dataFrame);
        QMap<GS::SE::StatsAlgo::Parameter,QVariant> lParam;
        lParam.insert(GS::SE::StatsAlgo::SIGMA, "6");
        lSuccess = engine->GetAlgorithm(GS::SE::StatsAlgo::MV_OUTLIERS_FINDER)->Execute(lParam, lStatsData2);
        if (!lSuccess)
            return false;
        bool ok;
        GS::SE::StatsAlgo* statalgo = engine->GetAlgorithm(GS::SE::StatsAlgo::MV_OUTLIERS_FINDER);
        GS::SE::RAlgo* ralgo = static_cast<GS::SE::RAlgo*>(statalgo);
        GS::SE::MVOutliersFinder* algo = static_cast<GS::SE::MVOutliersFinder*>(ralgo);

        GS::SE::RVector lVector;
        QStringList test;

        qDebug() << "outliers: ";
        lVector = algo->GetOutliers(ok);
        if (!lVector.IsEmpty())
            for (int j = 0; j < lVector.GetSize(); ++j)
                test << QString("o[%1]:%2\n").arg(QString::number(j)).arg(QString::number(lVector.GetDoubleItem(j, ok)));
        qDebug() << test.join(",");

        test.clear();
        lVector = algo->GetZScores(ok);
        qDebug() << "zscore:";
        qDebug() << lVector.GetSize();
        if (!lVector.IsEmpty())
            for (int j = 0; j < lVector.GetSize(); ++j)
            {
                if (j > 625 && j < 640)
                    test << QString("z[%1]:%2\n").arg(QString::number(j)).arg(QString::number(lVector.GetDoubleItem(j, ok)));
            }
        qDebug() << test.join(",");

        QSize lSize;
        lSize.setHeight(1200);
        lSize.setWidth(1800);
        GS::SE::RVector lLabels;
        lLabels.Build("histolabels", partsHeader.size(), GS::SE::RVector::V_STD);
        for (int i = 0; i < partsHeader.size(); ++i)
        {
            lLabels.FillStd(i, QString("PID-%1").arg(i));
        }
        if (algo->BuildZScoresHisto(workingDir + QDir::separator() + "histo.png", lSize))
            qDebug() << "histo built with success!";
        if (algo->BuildZScoresTrend(workingDir + QDir::separator() + "trend.png", lLabels, lSize))
            qDebug() << "trend built with success!";
        if (algo->BuildZScoresQQPlot(workingDir + QDir::separator() + "qqplot.png", lSize))
            qDebug() << "qqplot built with success!";

        GS::SE::RMatrix lPCA = algo->GetPCA(ok);
        if (algo->BuildCorrelationChart(lPCA, 0, 1, workingDir + QDir::separator() + "corrPCA.png", lLabels, lSize))
            qDebug() << "PCA corrchart built with success!";
        if (algo->BuildCorrelationChart(*lInputMatrix, 0, 1, workingDir + QDir::separator() + "corrX.png", lLabels, lSize))
            qDebug() << "X corrchart built with success!";

    }

    delete lStatsData2;

//        if (i%100 == 0)
//        {
//            qDebug() << "test" << i;
//            QMap<QString, QVariant> lMemInfo=CGexSystemUtils::GetMemoryInfo(false, true);
//            qDebug() << lMemInfo["MemUsedByProcess"].toString();
            // output gc

//            SEXP call = Rf_lang2(Rf_install("rm"), Rf_lang1(Rf_install("ls")));
//            SET_TAG(CDR(call), Rf_install("list"));
//            Rf_eval(call, R_GlobalEnv );

//            Rf_eval( Rf_lang2( Rf_install("print"), Rf_lang1(Rf_install("gc"))) , R_GlobalEnv );
//            QMap<QString, QVariant> lMemInfo=CGexSystemUtils::GetMemoryInfo(false, true);
//            qDebug() << lMemInfo["MemUsedByProcess"].toString();
//        }
//    }

    system("pause");

    return !lSuccess ;
}


int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    QString lWorkDir(QDir::cleanPath(QString(getenv("DEVDIR")) + "/other_libraries/R/unit_test_4/data"));
    QString lInputFile(lWorkDir + QDir::separator() + "ds1_outliers.csv");
    QStringList lTestsHeader;
    QStringList lPartsHeader;
    double **lDataFrame = NULL;

    lDataFrame = readCSVFile(lTestsHeader,lPartsHeader, lInputFile);

    if (lDataFrame == NULL)
    {
        printf("Error: no data frame created!\n");
        return EXIT_FAILURE;
    }

    printf("GALAXY_R_HOME: %s\n", QString(qgetenv("GALAXY_R_HOME")).toLatin1().data());


    QString lError;

    GS::SE::StatsEngine* lEngine = GS::SE::StatsEngine::GetInstance(QCoreApplication::applicationDirPath(), lError);
//    test1();
    test2(lEngine, lWorkDir, lDataFrame, lTestsHeader,lPartsHeader);
    GS::SE::StatsEngine::ReleaseInstance();

}

