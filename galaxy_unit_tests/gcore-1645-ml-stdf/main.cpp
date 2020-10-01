#include <stdlib.h>
#include <stdio.h>

#include <QFile>
#include <QTextStream>
#include <QJsonObject>
#include <QList>

#include <gqtl_atdftostdf.h>
#include <gstdl_errormgr.h>
#include "multi_limit_item.h"
#include <stdfparse.h>

QJsonObject ProcessDTR(const GQTL_STDF::Stdf_DTR_V4 & InDTR, QTextStream & OutDump,
                const unsigned int DtrCount)
{
    QString lJsonType, lJsonError;
    // Extract ML JSON object
    QJsonObject lJsonObject = InDTR.GetGsJson(lJsonType);
    if(lJsonObject.isEmpty())
    {
        if(lJsonError.isEmpty())
            OutDump << QString("%1;%2;OK: not a GS Json DTR")
                .arg(DtrCount).arg(InDTR.m_cnTEXT_DAT) << endl;
        else
            OutDump << QString("%1;%2;NOK: invalid GS Json DTR (error: %3)")
                .arg(DtrCount).arg(InDTR.m_cnTEXT_DAT).arg(lJsonError) << endl;
        return lJsonObject;
    }
    // Check if valid GS Json multi limit item
    GS::Core::MultiLimitItem lLimitItem;
    if (!lLimitItem.LoadFromJSon(lJsonObject, lJsonError) || !lLimitItem.IsValid())
    {
        OutDump << QString("Error when reading DTR record: %1 - %2")
              .arg(InDTR.m_cnTEXT_DAT)
              .arg(lJsonError) << endl;
        return lJsonObject;
    }
    if (!lLimitItem.IsValidLowLimit() && !lLimitItem.IsValidHighLimit())
    {
        OutDump << QString("No Valid limit in DTR: %1")
              .arg(InDTR.m_cnTEXT_DAT) << endl;
        return lJsonObject;
    }

    // Found a valid GS Json DTR
    // If type is "ml", dump available fields
    QString lDump;
    if(lJsonType.toLower() == "ml")
    {
        QJsonValue  lJsonValue;
        lJsonValue = lJsonObject.value("TNUM");
        if(lJsonValue.isDouble())
            lDump += QString(", TNUM=%1").arg((int)lJsonValue.toDouble());
        lJsonValue = lJsonObject.value("TNAME");
        if(lJsonValue.isString())
            lDump += QString(", TNAME=%1").arg(lJsonValue.toString());
        lJsonValue = lJsonObject.value("SITE");
        if(lJsonValue.isDouble())
            lDump += QString(", SITE=%1").arg((int)lJsonValue.toDouble());
        lJsonValue = lJsonObject.value("LL");
        if(lJsonValue.isDouble())
            lDump += QString(", LL=%1").arg(lJsonValue.toDouble());
        lJsonValue = lJsonObject.value("HL");
        if(lJsonValue.isDouble())
            lDump += QString(", HL=%1").arg(lJsonValue.toDouble());
        lJsonValue = lJsonObject.value("HBIN");
        if(lJsonValue.isDouble())
            lDump += QString(", HBIN=%1").arg((int)lJsonValue.toDouble());
        lJsonValue = lJsonObject.value("SBIN");
        if(lJsonValue.isDouble())
            lDump += QString(", SBIN=%1").arg((int)lJsonValue.toDouble());

        // Remove starting ", "
        if(lDump.startsWith(", "))
            lDump = lDump.right(lDump.size()-2);
    }

    OutDump << QString("%1;%2;OK: valid GS Json DTR found with type \"%3\"")
        .arg(DtrCount).arg(InDTR.m_cnTEXT_DAT).arg(lJsonType);
    if(!lDump.isEmpty())
        OutDump << QString(" (%1)").arg(lDump);
    OutDump << endl;

    return lJsonObject;
}

bool WriteAtdf(QList<QJsonObject> JsonObjects,
               const QString OutAtdf1_FN, const QString OutAtdf2_FN)
{
    // Open output ATDF files
    QFile lAtdfOut1(OutAtdf1_FN);
    if(!lAtdfOut1.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    QFile lAtdfOut2(OutAtdf2_FN);
    if(!lAtdfOut2.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        lAtdfOut1.close();
        return false;
    }
    QTextStream lAtdf1(&lAtdfOut1);
    QTextStream lAtdf2(&lAtdfOut2);

    // Loop through Json objects
    QJsonObject lJsonObject;
    QJsonValue  lJsonValue;
    QString     lAtdfString;
    for(int i=0; i<JsonObjects.size(); ++i)
    {
        lJsonObject = JsonObjects.at(i);

        // Write Json object into first output STDF (DTR)
        GQTL_STDF::Stdf_DTR_V4 lDTR;
        lDTR.SetTEXT_DAT(lJsonObject);
        lDTR.GetAtdfString(lAtdfString);
        lAtdf1 << lAtdfString;

        // Manually create DTR and write it to STDF
        QString lDtrText="{\"TYPE\":\"ML\"";
        lJsonValue = lJsonObject.value("TNAME");
        if(lJsonValue.isString())
            lDtrText += QString(",\"TNAME\":\"%1\"").arg(lJsonValue.toString());
        lJsonValue = lJsonObject.value("TNUM");
        if(lJsonValue.isDouble())
            lDtrText += QString(",\"TNUM\":\"%1\"").arg((int)lJsonValue.toDouble());
        lJsonValue = lJsonObject.value("SITE");
        if(lJsonValue.isDouble())
            lDtrText += QString(",\"SITE\":\"%1\"").arg((int)lJsonValue.toDouble());
        lJsonValue = lJsonObject.value("LL");
        if(lJsonValue.isDouble())
            lDtrText += QString(",\"LL\":\"%1\"")
                .arg(QString::number(lJsonValue.toDouble(),'e',3));
        lJsonValue = lJsonObject.value("HL");
        if(lJsonValue.isDouble())
            lDtrText += QString(",\"HL\":\"%1\"")
                .arg(QString::number(lJsonValue.toDouble(),'e',3));
        lJsonValue = lJsonObject.value("HBIN");
        if(lJsonValue.isDouble())
            lDtrText += QString(",\"HBIN\":\"%1\"").arg((int)lJsonValue.toDouble());
        lJsonValue = lJsonObject.value("SBIN");
        if(lJsonValue.isDouble())
            lDtrText += QString(",\"SBIN\":\"%1\"").arg((int)lJsonValue.toDouble());
        lDtrText += "}";
        lDTR.Reset();
        lDTR.SetTEXT_DAT(lDtrText);
        lDTR.GetAtdfString(lAtdfString);
        lAtdf2 << lAtdfString;
    }

    // CLose
    lAtdfOut1.close();
    lAtdfOut2.close();

    return true;
}

bool CompareToRef(const QString & Out_FN)
{
    printf("comparing output file %s to ref... ", Out_FN.toLatin1().constData());

    // Open ref file
    QString lRefFN = Out_FN + ".ref";
    QFile lFileRef(lRefFN);
    if(!lFileRef.open(QIODevice::ReadOnly))
    {
        printf("NOK. Error opening file %s.\n", lRefFN.toLatin1().constData());
        return false;
    }
    // Read Ref file
    QString lStringRef = lFileRef.readAll();
    lFileRef.close();

    // Open output file
    QFile lFileOut(Out_FN);
    if(!lFileOut.open(QIODevice::ReadOnly))
    {
        printf("NOK. Error opening file %s.\n", Out_FN.toLatin1().constData());
        return false;
    }
    // Read Output file
    QString lStringOut = lFileOut.readAll();
    lFileOut.close();

    // Compare strings
    if (lStringRef.compare(lStringOut) != 0)
    {
        QString lDiff_FN = Out_FN + ".diff";
        printf("NOK. Ref('%s', %d chars) != Output ('%s', %d chars). See %s.\n)",
           lRefFN.toLatin1().constData(), lStringRef.size(),
           Out_FN.toLatin1().constData(), lStringOut.size(),
           lDiff_FN.toLatin1().constData());
        system(QString("diff -y --suppress-common-lines %1 %2>%3")
           .arg(lRefFN).arg(Out_FN).arg(lDiff_FN).toLatin1().data() );
        return false;
    }

    printf("OK\n");
    return true;
}

int main(int argc, char *argv[])
{
    Q_UNUSED(argc);
    Q_UNUSED(argv);

    printf("gcore-1645-ml-stdf unit test: testing multi-limits storage in STDF using JSON objects in DTR records\n");

    QString lInAtdf_FN = QString("./gcore-1645-in.atd");
    QString lInStdf_FN = QString("./gcore-1645-in.std");
    QString lOutDump_FN = QString("./gcore-1645-out.csv");
    QString lOutAtdf1_FN = QString("./gcore-1645-out1.atd");
    QString lOutAtdf2_FN = QString("./gcore-1645-out2.atd");

    // Convert ATDF to STDF
    printf("converting ATDF input file (%s) to STDF (%s)... ",
           lInAtdf_FN.toLatin1().constData(), lInStdf_FN.toLatin1().constData());
    GS::ATDFtoSTDF lAtdf2Stdf;
    if(lAtdf2Stdf.Convert(lInAtdf_FN.toLatin1().constData(),
                          lInStdf_FN.toLatin1().constData()) == false)
    {
        printf("NOK\n");
        return EXIT_FAILURE;
    }
    printf("OK\n");

    // Open input STDF
    printf("opening input STDF (%s)... ", lInStdf_FN.toLatin1().constData());
    GQTL_STDF::StdfParse lStdfIn;
    if(lStdfIn.Open(lInStdf_FN.toLatin1().constData()) == false)
    {
        printf("NOK\n");
        return EXIT_FAILURE;
    }
    printf("OK\n");

    // Open output dump file
    printf("opening output dump file (%s)... ", lOutDump_FN.toLatin1().constData());
    QFile lOutputDump(lOutDump_FN);
    if(!lOutputDump.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        lStdfIn.Close();
        printf("NOK\n");
        return EXIT_FAILURE;
    }
    printf("OK\n");

    // Write some header to output dump
    QTextStream lOut(&lOutputDump);
    lOut << "# gcore-1645 unit test output file. this file should contain a dump of all DTR records with a status" << endl;
    lOut << "DTR#;DTR.TEXT_DAT;Status" << endl;

    // Parse through all STDF records, process any DTR, and keep list of
    // valid "ML" Json objects
    printf("parsing through all STDF records looking for DTRs...\n");
    QList<QJsonObject>  lJsonObjects;
    QJsonObject         lJsonML;
    int lRecType=0;
    unsigned int lRecordCount=0, lDtrCount=0;

    int lError=lStdfIn.LoadNextRecord(&lRecType);
    GQTL_STDF::Stdf_DTR_V4 lDTR;
    while(lError == GQTL_STDF::StdfParse::NoError)
    {
        ++lRecordCount;
        if(lRecType == GQTL_STDF::Stdf_Record::Rec_DTR)
        {
            ++lDtrCount;
            printf("reading DTR#%d... ", lDtrCount);
            if(lStdfIn.ReadRecord(&lDTR) == false)
            {
                QString strError = GGET_LASTERRORMSG(StdfParse, &lStdfIn);
                printf("NOK\n");
                printf("ERROR reading DTR#%d (%s)\n", lDtrCount,
                       strError.toLatin1().constData());
                lOutputDump.close();
                lStdfIn.Close();
                return EXIT_FAILURE;
            }
            printf("OK\n");

            // Processing DTR record
            printf("processing DTR#%d... OK\n", lDtrCount);
            lJsonML = ProcessDTR(lDTR, lOut, lDtrCount);
            if(!lJsonML.isEmpty())
                lJsonObjects.append(lJsonML);
        }

        lError = lStdfIn.LoadNextRecord(&lRecType);
    }

    // Check if loop exited on error
    if(lError != GQTL_STDF::StdfParse::EndOfFile)
    {
        printf("ERROR loading STDF record (error code is %d)\n", lError);
        lOutputDump.close();
        lStdfIn.Close();
        return EXIT_FAILURE;
    }

    // Write ATDF files
    printf("writing ATDF output files (%s, %s)... ",
           lOutAtdf1_FN.toLatin1().constData(),
           lOutAtdf2_FN.toLatin1().constData());
    if(!WriteAtdf(lJsonObjects, lOutAtdf1_FN, lOutAtdf2_FN))
    {
        printf("NOK\n");
        lOutputDump.close();
        lStdfIn.Close();
        return EXIT_FAILURE;
    }
    printf("OK\n");

    lOut << QString("# %1 records read, %2 DTR processed").arg(lRecordCount).arg(lDtrCount) << endl;

    // Close all files
    lOutputDump.close();
    lStdfIn.Close();

    // Compare results with REF files
    bool bStatus = true;
    if(!CompareToRef(lOutDump_FN))
        bStatus = false;
    if(!CompareToRef(lOutAtdf1_FN))
        bStatus = false;
    if(!CompareToRef(lOutAtdf2_FN))
        bStatus = false;
    if(!bStatus)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}
