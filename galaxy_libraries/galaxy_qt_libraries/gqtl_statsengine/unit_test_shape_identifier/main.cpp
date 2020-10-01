
#include <QSize>
#include <QDebug>
#include <QByteArray>
#include <QDateTime>
#include <iostream>

#include <cmath> // for nan()

#include <QTextStream>
#include <QFile>
#include <QDir>
#include <QCoreApplication>

#include "r_algo.h"
#include "r_data.h"
#include "r_vector.h"
#include "r_matrix.h"

#include "stats_engine.h"
#include "stats_algo.h"
#include "stats_data.h"

#include "shape_identifier.h"

double** readCSVFile(QStringList &testsHeader, QStringList &partsHeader, const QString &inputFile)
{
    double** lDataFrame = NULL;
    QString lFile = inputFile;
    printf("Read input file %s \n", lFile.toLatin1().data());

    QFile lCsv(lFile);
    if (!lCsv.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        printf("\nUnable to open file %s \n", lFile.toLatin1().data());
        return 0;
    }
    QTextStream lStream(&lCsv);
    // get number of parts and tests
    int lNbTests = 0;
    int lNbParts = 0;
    while(!lStream.atEnd())
    {
        QString lLine = lStream.readLine();
        if (lLine.startsWith("Tests#"))
        {
            lNbTests = lLine.split(",").size();
        }
        else if (lLine.startsWith("PID-"))
        {
            lNbParts++;
        }
    }
    lDataFrame = (double**) malloc(lNbTests*sizeof(double*));
    for (int i = 0; i < lNbTests; i++ )
        lDataFrame[i] = (double *) malloc(lNbParts*sizeof(double));

    // reset position
    lStream.seek(0);
    // read header
    testsHeader = lStream.readLine().split(",");
    testsHeader.removeFirst();

    int lPartIndex = 0;
    while(!lStream.atEnd())
    {
        QString lLine = lStream.readLine();
        if (lLine.startsWith("Tests#"))
        {
            testsHeader = lLine.split(",");
            testsHeader.removeFirst();
        }
        else if (lLine.startsWith("PID-"))
        {
            QStringList lTests = lLine.split(",");
            partsHeader .append(lTests.takeFirst()); // remove parts name
            for(int lTest = 0; lTest < lTests.size(); lTest++)
            {
                bool lOk = true;
                double lValue = lTests.value(lTest).toDouble(&lOk);
                // to be more close to Gex environment
                float lFloatValue = lValue;
                lValue = lFloatValue;
                if (lOk)
                    lDataFrame[lTest][lPartIndex] = lValue;
                else
                    lDataFrame[lTest][lPartIndex] = NAN;
            }
            lPartIndex++;
        }
    }

    return lDataFrame;
}

int identifyShapes(GS::SE::StatsEngine * engine,
                   QStringList testsHeader,
                   QStringList partsHeader,
                   double **dataFrame,
                   QString refFile,
                   QString /*outDir*/)
{
    if (!engine)
    {
        printf("Error: no engine instanciated!\n");
        return 0;
    }

    printf("Read ref file %s \n", refFile.toLatin1().data());

    // check if ref file exists
    QFile lRefFile(refFile);
    if (!lRefFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        printf("Error: unable to open file - %s \n", refFile.toLatin1().data());
        return 0;
    }
    QTextStream lStream(&lRefFile);

    GS::SE::ShapeIdentifier* lAlgo = static_cast<GS::SE::ShapeIdentifier*>
            (engine->GetAlgorithm(GS::SE::StatsAlgo::SHAPE_IDENTIFIER));
    if (!lAlgo)
    {
        printf("Error: no algorithm instanciated - %s \n", engine->GetLastError().toLatin1().data());
        return 0;
    }

    int lStatus = 1;

    printf("Identify shapes... \n");

    // For each test compute shape analysis
    for (int lTestIndex = 0; lTestIndex < testsHeader.size(); ++lTestIndex)
    {
        if (testsHeader.at(lTestIndex).trimmed().isEmpty())
            continue;
        GS::SE::RData* lStatsData = new GS::SE::RData();
        GS::SE::RVector* lInputVector = lStatsData->AllocateVector(GS::SE::StatsData::SHAPEIDENTIFIER_IN,
                                                                   partsHeader.size(),
                                                                   dataFrame[lTestIndex]);
        if (!lInputVector)
        {
            printf("Error: unable to allocate vector - %s \n", lStatsData->GetLastError().toLatin1().data());
            continue;
        }

        QMap<GS::SE::StatsAlgo::Parameter,QVariant> lParam;

        if (lAlgo->Execute(lParam, lStatsData) == false)
        {
            printf("Error: unable to execute algo - %s \n", lAlgo->GetLastError().toLatin1().data());
            return false;
        }

        bool lSuccess = false;
        GS::SE::RVector lShape = lAlgo->GetShapeName(lSuccess);
        if (!lSuccess)
        {
            printf("Error: Unable to get shape name!\n");
            continue;
        }
        GS::SE::RVector lConfidence = lAlgo->GetConfidenceLevel(lSuccess);
        if (!lSuccess)
        {
            printf("Error: Unable to get confidence level!\n");
            continue;
        }

        QString lOutput = "Test " + testsHeader.at(lTestIndex)
                 + " => "
                 + QString::fromStdString(lShape.GetStringItem(0, lSuccess))
                 + "(confidence:"
                 + QString::fromStdString(lConfidence.GetStringItem(0, lSuccess))
                 + ")" ;

        QString lRefLine = lStream.readLine();
        // if diff between ref and output
        if (lRefLine.trimmed() != lOutput.trimmed())
        {
            lStatus = 0;
            printf("Error: diff between ref and output!\n"
                   "             Ref: %s\n"
                   "             Output: %s\n",
                   lRefLine.toLatin1().data(),
                   lOutput.toLatin1().data()
                   );
        }

/*
        // build graph
        QSize lSize(600, 400);
        lAlgo->BuildHistogram(*lInputVector,
                              outDir + "/shape" + QString::number(lTestIndex) +".png",
                              lSize,
                              "T" + testsHeader.at(lTestIndex) + " - " +
                              QString((char*)lShape.GetStringItem(0, lSuccess)) +
                              " (Confidence: "+
                              QString((char*)lConfidence.GetStringItem(0, lSuccess)) + ")");
*/
        // Clear R data
        delete lStatsData;
        lStatsData = 0;
    }

    return lStatus;
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    printf("Starting unit test...\n");
    QString lWorkDir(QDir::cleanPath(QString(getenv("DEVDIR")) + "/galaxy_libraries/galaxy_qt_libraries/gqtl_statsengine/unit_test_shape_identifier/data"));
    bool lStatus = false;

    QDir lDir;
    if (!lDir.mkpath(lWorkDir))
    {
        printf("Error: unable to create working dir: %s\n", lWorkDir.toLatin1().data());
        return EXIT_FAILURE;
    }

    QString lInputFile(lWorkDir + QDir::separator() + "data_samples.std.gextb.csv");
    QString lRefFile(lWorkDir + QDir::separator() + "data_samples_results.txt");

    QStringList lTestsHeader;
    QStringList lPartsHeader;

    double **lDataFrame = NULL;

    QString lOutDir = QDir::cleanPath(lWorkDir + QDir::separator() + QDateTime::currentDateTime().toString("yyMMdd-hhmmss"));
    /*
        QDir lDir;
        if (!lDir.mkdir(lOutDir))
        {
            qDebug() << "Unable to create dir: " << lOutDir;
            return EXIT_FAILURE;
        }
        */

    lDataFrame = readCSVFile(lTestsHeader,lPartsHeader, lInputFile);

    if (lDataFrame == NULL)
    {
        printf("Error: no data frame created!\n");
        return EXIT_FAILURE;
    }

    /*        char type;
        do
        {
            std::cout << "Press y to continue" << endl;
            std::cin >> type;
        }
        while( !std::cin.fail() && type!='y' && type!='n' );
        if (type=='n')
            exit(0);
        else
            std::cout << "Identifying Shape..." << std::flush  ;
*/
    printf("GALAXY_R_HOME: %s\n", QString(qgetenv("GALAXY_R_HOME")).toLatin1().data());

    QString lError;
    for (int i = 0; i < 1 ; ++i)
    {
        GS::SE::StatsEngine * lEngine = GS::SE::StatsEngine::GetInstance(QCoreApplication::applicationDirPath(), lError);
        if (lEngine)
        {
            lStatus = identifyShapes(lEngine, lTestsHeader, lPartsHeader, lDataFrame, lRefFile, lOutDir);
            GS::SE::StatsEngine::ReleaseInstance();
        }
    }

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
