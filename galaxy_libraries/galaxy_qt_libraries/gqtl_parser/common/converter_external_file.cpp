//////////////////////////////////////////////////////////////////////
// converter_external_file.cpp: Convert a .CSV / Excel file to STDF V4.0
//////////////////////////////////////////////////////////////////////
#include <QString>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNodeList>
#include <QFile>
#include <QStringList>

#include "converter_external_file.h"

const QString cBinMap("binmap");
const QString cDefBinPath("bin_filepath");
const QString cDefBinName("default_bin_name");
const QString cDefBinNb("default_bin_number");

bool ConverterExternalFile::Exists(const QString &aPath)
{
    QString lExternalFileName = GetExternalFileName(aPath);

    return QFile::exists(lExternalFileName);
}

bool ConverterExternalFile::GetPromisFile(QString strPath, QString strType, QString &strFileName, QString &strFileFormat, QString &strErrorMsg, QString strCategory/*=""*/)
{
    return GetPromisFile(strPath, strType, "prod", strFileName, strFileFormat, strErrorMsg, strCategory);
}

bool ConverterExternalFile::GetPromisFile(QString strPath, QString strType, QString strMode, QString &strFileName, QString &strFileFormat, QString &strErrorMsg, QString strCategory/*=""*/)
{
    strFileName = strFileFormat = strErrorMsg = "";

    QString strExternalFileName = GetExternalFileName(strPath);

    QDomDocument doc("xml");
    if(!OpenExternalDomFile(strExternalFileName,doc,strErrorMsg))
        return false;

    if(!GetExternalFileInfo(doc.documentElement(), "promis", strType, strMode, strCategory,strFileName,strFileFormat,strErrorMsg))
        return false;

    if(strFileName.isEmpty() || strFileFormat.isEmpty())
    {
        strErrorMsg = "invalid external_files XML";
        return false;
    }

    return true;
}

bool ConverterExternalFile::GetBinmapFile(QString strPath, QString strType, QString &strFileName, QString &strFileFormat, QString &strErrorMsg)
{
    return GetBinmapFile(strPath, strType, "prod", "",strFileName, strFileFormat, strErrorMsg);
}

bool ConverterExternalFile::GetBinmapFile(QString strPath, QString strType, QString strMode, QString &strFileName, QString &strFileFormat, QString &strErrorMsg)
{
    return GetBinmapFile(strPath, strType, strMode, "",strFileName, strFileFormat, strErrorMsg);
}

bool ConverterExternalFile::GetBinmapFile(QString strPath, QString strType, QString strMode, QString strCategory, QString &strFileName, QString &strFileFormat, QString &strErrorMsg)
{
    strFileName = strFileFormat = strErrorMsg = "";

    QString strExternalFileName = GetExternalFileName(strPath);

    QDomDocument doc("xml");
    if(!OpenExternalDomFile(strExternalFileName,doc,strErrorMsg))
        return false;

    if(!GetExternalFileInfo(doc.documentElement(), cBinMap, strType,
                            strMode, strCategory, strFileName, strFileFormat, strErrorMsg))
        return false;

    if(strFileName.isEmpty() || strFileFormat.isEmpty())
    {
        strErrorMsg = "invalid converter_external_file.xml XML";
        return false;
    }

    return true;
}

QString ConverterExternalFile::GetExternalFileName(QString strFilePath)
{
    QString strXmlFileName = strFilePath.simplified();
    // Add / if needed
    if((!strXmlFileName.endsWith("\\")) && (!strXmlFileName.endsWith("/")))
        strXmlFileName += "/";

    strXmlFileName += "converter_external_file.xml";
    return strXmlFileName;
}

bool ConverterExternalFile::OpenExternalDomFile(QString strFileName, QDomDocument &doc, QString &strErrorMsg)
{
    QFile file(strFileName); // Read the text from a file
    if(!file.open(QIODevice::ReadOnly))
    {
        strErrorMsg = strFileName + " does not exist";
        return false;	// Failed opening XML file.
    }

    QString errorMsg;
    int errorLine, errorColumn=0;
    if (!doc.setContent(&file, &errorMsg, &errorLine, &errorColumn))
    {
        strErrorMsg=QString("file %1 is not xml compliant (line %2 col %3) : %4 !")
                .arg(strFileName).arg(errorLine).arg(errorColumn).arg(errorMsg);
        file.close();
        return false;
    }
    file.close();
    return true;
}

bool ConverterExternalFile::GetExternalFileInfo(QDomElement docElem, QString strData, QString strType, QString strMode, QString strCategory, QString &strFile, QString &strFormat, QString &strErrorMsg)
{
    strFile = strFormat = strErrorMsg = "";

    // Check we support this version of .xml
    if(docElem.nodeName().toLower() != "external_files")
    {
        strErrorMsg = "not a external_files XML";
        return false;
    }
    if(docElem.elementsByTagName(strData).isEmpty())
    {
        strErrorMsg = "invalid external_files XML";
        return false;
    }

    bool bFound = false;
    int iIndex;
    QDomNode docNode;
    QDomNodeList lstNode = docElem.elementsByTagName(strData);
    for(iIndex=0; iIndex < lstNode.count(); iIndex++)
    {
        docNode = lstNode.at(iIndex);
        if(docNode.toElement().attribute("type").toLower() == strType.toLower())
        {
            if(docNode.toElement().attribute("mode","prod").toLower() == strMode.toLower())
            {
                QString lCategory = docNode.toElement().attribute("category");
                if(strCategory.isEmpty() ||
                                (lCategory.toLower() == strCategory.toLower()))
                {
                    bFound = true;
                    break;
                }
            }
        }
    }

    if(!bFound)
    {
        strErrorMsg = "invalid external_files XML";
        return false;
    }

    //grab format
    strFormat = docNode.toElement().attribute("format");

    //let's take file path
    strFile = docNode.toElement().text().simplified();

    //let's check if file path has to be taken using the new format
    if(strData == cBinMap)
    {
        //let's try to grab it using the new format
        //here is a reminder of the formats:
        /* old format example:
       <binmap format="vishay-lvm" type="final" category="final_tests">...\externalfiles\CSVFinalTestsFile20180402.csv</binmap>
       */
        /*  new format example:
       <binmap format="vishay-lvm" type="final" category="final_tests">
         <bin_filepath>...\externalfiles\CSVFinalTestsFile20180402.csv</bin_filepath>
         <default_bin_name>test name</default_bin_name>
         <default_bin_number>9999</default_bin_number>
       </binmap>
       */
        QDomNodeList lNodeList = docElem.elementsByTagName(cBinMap);//lDoc.documentElement().elementsByTagName(strData);
        if(!lNodeList.isEmpty()){
            QDomNode ltmpNode = lNodeList.at(0);
            QDomNodeList lChildNodes = ltmpNode.childNodes();
            for(int lIndex=0; lIndex < lChildNodes.count(); lIndex++)
            {
                QDomNode lDocNode = lChildNodes.at(lIndex);
                if (lDocNode.isElement()) {
                    QDomElement lElement = lDocNode.toElement();
                    if(lElement.tagName() == cDefBinPath)
                    {
                        strFile = lElement.text().simplified();
                        return true;
                    }
                }
                lDocNode = lDocNode.nextSibling();
            }
        }
    }

    if(strFile.isEmpty())
    {
        strErrorMsg = "invalid external_files XML";
        return false;
    }

    return true;
}

QStringList ConverterExternalFile::LoadAttributs(const QString& aExternalFilPath, const QString& aAttribut, QString& lErrorMsg)
{

    if(aAttribut.isEmpty()) return QStringList();

    QStringList lTypeList;
    QString lExternalFileName = GetExternalFileName(aExternalFilPath);
    QDomDocument lDoc("xml");
    if(!OpenExternalDomFile(lExternalFileName, lDoc, lErrorMsg))
        return lTypeList;

    // Check we support this version of .xml
    if(lDoc.documentElement().nodeName().toLower() != "external_files")
    {
        lErrorMsg = "not a external_files XML";
        return lTypeList;
    }

    QDomNode lDocNode;
    QDomNodeList lNodes = lDoc.documentElement().childNodes();
    for(int lIndex=0; lIndex < lNodes.count(); lIndex++)
    {
        lDocNode = lNodes.at(lIndex);


        QString lType = lDocNode.toElement().attribute(aAttribut).toLower();
        if(lType.isEmpty() == false && lTypeList.contains(lType) == false)
        {
            lTypeList << lType;
        }
    }
    return lTypeList;

}

QStringList ConverterExternalFile::LoadOptionnalDefaultVals(const QString& aExternalFilPath, QString& aStrErrorMsg)
{
    QStringList lValuesList;

    QString lExternalFileName = GetExternalFileName(aExternalFilPath);
    QDomDocument lDoc("xml");
    if(!OpenExternalDomFile(lExternalFileName, lDoc, aStrErrorMsg))
        return lValuesList;
    if(lDoc.documentElement().nodeName().toLower() != "external_files")
        return lValuesList;

    //by default, return a list that contains 2 empty elements
    //(information is optionnal but size must be 2)
    lValuesList.append("");
    lValuesList.append("");

    //check if it has default bin number and bin name information
    QDomNodeList lNodeList = lDoc.documentElement().elementsByTagName(cBinMap);//lDoc.documentElement().elementsByTagName(strData);

    if(lNodeList.isEmpty())
        return lValuesList;
    QDomNode ltmpNode = lNodeList.at(0);
    QDomNodeList lChildNodes = ltmpNode.childNodes();

    for(int lIndex=0; lIndex < lChildNodes.count(); lIndex++)
    {
        QDomNode lDocNode = lChildNodes.at(lIndex);
        if (lDocNode.isElement()) {
            QDomElement lElement = lDocNode.toElement();

            if(lElement.tagName() == cDefBinName)
            {
                lValuesList[0] = lElement.text().simplified();
            }
            else if(lElement.tagName() ==  cDefBinNb)
            {
                lValuesList[1] = lElement.text().simplified();
            }
        }
        lDocNode = lDocNode.nextSibling();
    }

    Q_ASSERT(lValuesList.size() == 2);

    return lValuesList;
}

// Extract Lot, Wafer from Base36 encrypted string
// returns 'false' on error.
// Input: x16oo1.PRN
bool ConverterExternalFile::DecryptLot(const QString& aCryptedLot, QString& aLot, int& aWaferNumber, QString& aErrorMsg)
{
    // Wafer# is last character (base 36 encoded)
    bool	lOk;
    QString lValue = aCryptedLot.right(1);

    aWaferNumber = lValue.toInt(&lOk,36);
    if(!lOk)
    {
        aErrorMsg = "Invalid wafer# (in FET_TEST file) extracted from lot id: " + aCryptedLot;
        return false;
    }

    // Ensure we ignore the Wafer# in case part of the encoded string.
    QString lLocalCryptedLot = aCryptedLot.left(aCryptedLot.length()-1);
    lLocalCryptedLot = lLocalCryptedLot.toUpper();	// Upper case.

    int     lSerialStart;

    // If engineering lot, drop the prefix E
    bool    lEngLot=false;
    if(lLocalCryptedLot[0].toUpper() == 'E')
    {
        lLocalCryptedLot = lLocalCryptedLot.mid(1);
        lEngLot = true;
    }

    // A=year code, F=fab char code,
    // NN is week# (0-53), XX is serial# base-36, W is base-36 wafer#
    int             lPartialLotID;
    QString         lSuffix;
    QString         lStringFormat;
    QString         lFab = "";	// To hold fab character name (eg: M or D or K or I, etc...)

    switch(lLocalCryptedLot.length())
    {
    case 5:	// Format: ANNXX (W)
        lSerialStart = 3;	// Offset to 'XX' string
        lSuffix = "";
        break;

    case 6:	// Format: ANNFXX (W)
        lSerialStart = 4;	// Offset to 'XX' string
        lSuffix = "";
        // Extract fab name character A
        lFab = lLocalCryptedLot[3];
        break;

    case 7:	// Format: ANNFXXA (W)
        lSerialStart = 4;	// Offset to 'XX' string
        lSuffix = lLocalCryptedLot.right(1);	// Extract 'A' suffix
        // Extract fab name character A
        lFab = lLocalCryptedLot[3];
        break;

    default: // Unsupported format.
        aErrorMsg = "Unsupported format: " + lLocalCryptedLot;
        return false;
    }

    // Decode encoded (base 36) middle lot data: XX.
    lValue = lLocalCryptedLot.mid(lSerialStart,2);
    lPartialLotID = lValue.toInt(&lOk,36);
    if(!lOk)
    {
        aErrorMsg = "Invalid partial lot id (in FET_TEST file) extracted from: " + lValue;
        return false;
    }

    // Build first half of decoded LotID (2 or 3 digits string generated)
    QString	lPartialLot;
    /// For Engineering lot
    // New Lot_ID ENG convention
    //D: VSIG
    //  Serial Number assigned for Production Control (01‐999)
    //  2 digits or 3 if needed
    //K: TOWER
    //V: Vanguard
    //E: Tower Texas
    //'': SantaClar FAB
    // and all other Engineerie FAB Lot
    //  Serial Number assigned for Production Control (001‐999)
    //  always 3 digits or 4 if needed
    if(lEngLot)
    {
        // D:VSIG
        if(lFab.startsWith("D",Qt::CaseInsensitive))
        {
            // 2 digits or 3 if needed.
            if(lPartialLotID <= 99)
                lStringFormat = "%02d";	// Write lotID as 2 digits
            else
                lStringFormat = "%03d";	// Write lotID as 3 digits

        }
        //'': SantaClar FAB: always 3 digits
        // K: TOWER
        // V: Vanguard
        // E: Tower Texas
        else
        {
            // 3 digits...or 4 if needed
            if(lPartialLotID <= 999)
                lStringFormat = "%03d";	// Write lotID as 3 digits
            else
                lStringFormat = "%04d";	// Write lotID as 4 digits
        }
    }
    else
    {
        lStringFormat = "%03d";	// Write lotID as 3 digits
    }
    lPartialLot.sprintf(lStringFormat.toLatin1().data(),lPartialLotID);


    ///////////////////////////////////////////////////////
    // Build complete Lot string.
    ///////////////////////////////////////////////////////
    aLot = "";
    if(lEngLot)
        aLot = "E";

    // Add prefix string
    aLot += lLocalCryptedLot.left(lSerialStart);

    // Add decoded lot sub-string
    aLot += lPartialLot;

    // Add suffix if any
    aLot += lSuffix;

    aLot = aLot.toUpper();

    return true;
}

