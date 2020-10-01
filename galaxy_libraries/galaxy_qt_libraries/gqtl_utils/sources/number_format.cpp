#include <stdio.h>
#include <gqtl_utils.h>

namespace GS
{
namespace QtLib
{

#define DEFAULT_FLOAT_PRECISION 7

///////////////////////////////////////////////////////////
// Class that formats numbers into strings
///////////////////////////////////////////////////////////
NumberFormat::NumberFormat()
{
    setOptions();
}

NumberFormat::~NumberFormat()
{

}

QString NumberFormat::formatNumericValue(int iValue, bool bFormat)
{
    if((mFormat=='e') || (mFormat=='E'))
        return formatNumericValue((double)iValue, bFormat);
    else
        return QString::number(iValue);
}

QString NumberFormat::formatNumericValue(uint uiValue, bool bFormat)
{
    if((mFormat=='e') || (mFormat=='E'))
        return formatNumericValue((double)uiValue, bFormat);
    else
        return QString::number(uiValue);
}

QString NumberFormat::formatNumericValue(long lValue, bool bFormat)
{
    if((mFormat=='e') || (mFormat=='E'))
        return formatNumericValue((double)lValue, bFormat);
    else
        return QString::number(lValue);
}

QString NumberFormat::formatNumericValue(ulong ulValue, bool bFormat)
{
    if((mFormat=='e') || (mFormat=='E'))
        return formatNumericValue((double)ulValue, bFormat);
    else
        return QString::number(ulValue);
}

QString NumberFormat::formatNumericValue(qlonglong llValue, bool bFormat)
{
    if((mFormat=='e') || (mFormat=='E'))
        return formatNumericValue((double)llValue, bFormat);
    else
        return QString::number(llValue);

}

QString NumberFormat::formatNumericValue(qulonglong ullVal, bool bFormat)
{
    if((mFormat=='e') || (mFormat=='E'))
        return formatNumericValue((double)ullVal, bFormat);
    else
        return QString::number(ullVal);
}

QString NumberFormat::formatNumericValue(double dValue, bool bFormat, const QString &strDefault)
{
    if(bFormat)
    {
        if(mComplexFormat)
        {
            if(mPrecision!=-1)
            {
                return displaySigFigs(QString::number(dValue,'f',20), mPrecision, -999, ((mFormat=='e') || (mFormat=='E')));
            }
            else
            {
                if((mFormat=='e') || (mFormat=='E'))
                {
                    return QString::number(dValue, 'e', DEFAULT_FLOAT_PRECISION);
                }
                else if((mFormat=='f') || (mFormat=='F'))
                {
                    return QString::number(dValue, 'f', DEFAULT_FLOAT_PRECISION);
                }
                else if((mFormat=='g') || (mFormat=='G'))
                {
                    return QString::number(dValue, 'g', DEFAULT_FLOAT_PRECISION);
                }
                else if(strDefault.isEmpty())
                {
                    return QString::number(dValue);
                }
                else
                {
                    return strDefault;
                }
            }
        }
        else
        {
            if((mFormat=='e') || (mFormat=='E'))
            {
                return QString::number(dValue, 'e', (mPrecision==-1)?DEFAULT_FLOAT_PRECISION:mPrecision);
            }
            else if((mFormat=='f') || (mFormat=='F'))
            {
                return QString::number(dValue, 'f', (mPrecision==-1)?DEFAULT_FLOAT_PRECISION:mPrecision);
            }
            else if((mFormat=='g') || (mFormat=='G'))
            {
                return QString::number(dValue, 'g', (mPrecision==-1)?DEFAULT_FLOAT_PRECISION:mPrecision);
            }
            else if(strDefault.isEmpty())
            {
                return QString::number(dValue);
            }
            else
            {
                return strDefault;
            }
        }
    }
    else if(strDefault.isEmpty())
    {
        return QString::number(dValue);
    }
    else
    {
        return strDefault;
    }

    //The Code before modification
    //    if(m_bScientificFormat)
    //       return QString::number(dValue,'e',NumberFormat::m_iPrecision);
    //    else if(strDefault.isEmpty())
    //       return QString::number(dValue);
    //    else
    //       return strDefault;
}

void NumberFormat::setOptions(char Format/*='g'*/, int Precision/*=-1*/, bool ComplexFormat/*=true*/)
{
    mComplexFormat = ComplexFormat;
    mFormat = Format;
    mPrecision = Precision;
}


int NumberFormat::parseOrder(const QString &strString){
    bool bBeginning = true;
    bool bSeenDot = false;
    bool bSeenSomething = false;
    QString strZeros = "";
    QString strLeadZeros = "";
    QString strAll = "";
    int iDecPlaces = 0;
    int iTotalDecs = 0;
    bool bPos = true;

    for (int i=0; i<strString.count(); i++){
        char cChar = strString[i].toLatin1();
        if (cChar>='1' && cChar<='9'){
            strAll += strZeros + cChar;
            strZeros = "";
            bSeenSomething = true;
            if (!bSeenDot){
                iTotalDecs++;
                iDecPlaces++;
            }
            bBeginning = false;
        } else if (cChar=='0'){
            if (bSeenDot){
                if (bSeenSomething){
                    strAll += strZeros + cChar;
                    strZeros = "";
                } else {
                    strLeadZeros += cChar;
                    iDecPlaces--;
                }
            } else {
                iTotalDecs++;
                if (bSeenSomething){
                    strLeadZeros += cChar;
                    iDecPlaces++;
                    strZeros += cChar;
                } else {
                    strLeadZeros += cChar;
                }
            }
            bBeginning = false;
        } else if (!bSeenDot && cChar=='.'){
            strAll += strZeros;
            strZeros = "";
            bSeenDot=true;
            bBeginning = false;
        } else if ((cChar=='e' || cChar=='E') && i+1<strString.count()){
            int iRaised = strString.mid(i+1, strString.count()).toInt();
            iDecPlaces += iRaised;
            iTotalDecs += iRaised;
            i = strString.count();
        } else if (bBeginning && (cChar=='+' || cChar=='-')){
            if (cChar=='-'){
                bPos = !bPos;
            }
        }
    }
    if (strAll == ""){
        return iTotalDecs;
    } else {
        return iDecPlaces;
    }
}

QString NumberFormat::parseMantissa(const QString &strString){

    bool bBeginning = true;
    bool bSeenDot = false;
    bool bSeenSomething = false;
    QString strZeros = "";
    QString strLeadZeros = "";
    QString strAll = "";
    int iDecPlaces = 0;
    int iTotalDecs = 0;
    bool bPos = true;

    for (int i=0; i<strString.count(); i++){
        char cChar = strString[i].toLatin1();
        if (cChar>='1' && cChar<='9'){
            strAll += strZeros + cChar;
            strZeros = "";
            bSeenSomething = true;
            if (!bSeenDot){
                iTotalDecs++;
                iDecPlaces++;
            }
            bBeginning = false;
        } else if (cChar=='0'){
            if (bSeenDot){
                if (bSeenSomething){
                    strAll += strZeros + cChar;
                    strZeros = "";
                } else {
                    strLeadZeros += cChar;
                    iDecPlaces--;
                }
            } else {
                iTotalDecs++;
                if (bSeenSomething){
                    strLeadZeros += cChar;
                    iDecPlaces++;
                    strZeros += cChar;
                } else {
                    strLeadZeros += cChar;
                }
            }
            bBeginning = false;
        } else if (!bSeenDot && cChar=='.'){
            strAll += strZeros;
            strZeros = "";
            bSeenDot=true;
            bBeginning = false;
        } else if ((cChar=='e' || cChar=='E') && i+1<strString.count()){
            int iRaised = strString.mid(i+1, strString.count()).toInt();
            iDecPlaces += iRaised;
            iTotalDecs += iRaised;
            i = strString.count();
        } else if (bBeginning && (cChar=='+' || cChar=='-')){
            if (cChar=='-'){
                bPos = !bPos;
            }
        }
    }
    if (strAll == ""){
        return strLeadZeros;
    } else {
        return strAll;
    }
}

bool NumberFormat::parseSign(const QString &strString){
    bool bBeginning = true;
    bool bSeenDot = false;
    bool bSeenSomething = false;
    QString strZeros = "";
    QString strLeadZeros = "";
    QString strAll = "";
    int iDecPlaces = 0;
    int iTotalDecs = 0;
    bool bPos = true;

    for (int i=0; i<strString.count(); i++){
        char cChar = strString[i].toLatin1();
        if (cChar>='1' && cChar<='9'){
            strAll += strZeros + cChar;
            strZeros = "";
            bSeenSomething = true;
            if (!bSeenDot){
                iTotalDecs++;
                iDecPlaces++;
            }
            bBeginning = false;
        } else if (cChar=='0'){
            if (bSeenDot){
                if (bSeenSomething){
                    strAll += strZeros + cChar;
                    strZeros = "";
                } else {
                    strLeadZeros += cChar;
                    iDecPlaces--;
                }
            } else {
                iTotalDecs++;
                if (bSeenSomething){
                    strLeadZeros += cChar;
                    iDecPlaces++;
                    strZeros += cChar;
                } else {
                    strLeadZeros += cChar;
                }
            }
            bBeginning = false;
        } else if (!bSeenDot && cChar=='.'){
            strAll += strZeros;
            strZeros = "";
            bSeenDot=true;
            bBeginning = false;
        } else if ((cChar=='e' || cChar=='E') && i+1<strString.count()){
            int iRaised = strString.mid(i+1, strString.count()).toInt();
            iDecPlaces += iRaised;
            iTotalDecs += iRaised;
            i = strString.count();
        } else if (bBeginning && (cChar=='+' || cChar=='-')){
            if (cChar=='-'){
                bPos = !bPos;
            }
        }
    }
    if (strAll == ""){
        return true ;
    } else {
        return bPos;
    }
}

QString NumberFormat::round(const QString &strMantissa, int iDigits){

    int iLast = strMantissa.count() - iDigits - 1;
    if (iLast < 0){
        return "";
    } else if (iLast >= strMantissa.count() -1){
        return strMantissa;
    } else {
        char cNextToLast = strMantissa[iLast+1].toLatin1();
        char cLastChar = strMantissa[iLast].toLatin1();
        bool bRoundUp = false;
        if (cNextToLast > '5') {
            bRoundUp = true;
        } else if (cNextToLast == '5') {
            for (int j=iLast+2; j<strMantissa.count(); j++){
                if(strMantissa[j] != '0'){
                    bRoundUp = true;
                }
            }
            if (cLastChar % 2 == 1){
                bRoundUp = true;
            }
        }
        QString strResult = "";
        for (int i=iLast; i>=0; i--){
            char cChar = strMantissa[i].toLatin1();
            if (bRoundUp){
                char cNextChar = '0';
                if (cChar == '9'){
                    cNextChar = '0';
                } else {
                    switch (cChar){
                        case '0': cNextChar='1'; break;
                        case '1': cNextChar='2'; break;
                        case '2': cNextChar='3'; break;
                        case '3': cNextChar='4'; break;
                        case '4': cNextChar='5'; break;
                        case '5': cNextChar='6'; break;
                        case '6': cNextChar='7'; break;
                        case '7': cNextChar='8'; break;
                        case '8': cNextChar='9'; break;
                    }
                    bRoundUp = false;
                }
                strResult = cNextChar + strResult;
            } else {
                strResult = cChar + strResult;
            }
        }
        if (bRoundUp){
            strResult = '1' + strResult;
        }
        return strResult;
    }
}

int NumberFormat::trailingZeros(const QString &strMantissa){

    int iZeros = 0;
    for (int i=strMantissa.count()-1; i>=0; i--){
        char cChar = strMantissa[i].toLatin1();
        if (cChar=='0'){
            iZeros++;
        } else {
            return iZeros;
        }
    }
    return iZeros;
}

QString NumberFormat::displaySigFigs(const QString &strFloat, int iSigFigs, int iSigDecs, bool bScientific){

    QString strTemp = QString("") + strFloat;
    int iOrder = parseOrder(strTemp);
    QString strMantissa = parseMantissa(strTemp);
    bool bPositive = parseSign(strTemp);

    int iAdd;
    int iDecAdd;
    int iSigAdd;
    bool bZeroScientific=false;

    if (strFloat.toDouble() == 0 || strMantissa=="" || strMantissa=="0"){
        strMantissa = "";
        for (int i=0; i<iSigFigs; i++){
            strMantissa += "0";
        }
        iOrder = iSigFigs + iSigDecs;
        if (iSigDecs < 0 && -iSigDecs >= iSigFigs){
            bZeroScientific = true;
        }
    } else {
        iDecAdd = ((iOrder - strMantissa.count()) - iSigDecs);
        iSigAdd = iSigFigs - strMantissa.count();
        iAdd = qMin(iSigAdd, iDecAdd);
        if (iAdd < 0){
            QString strRounded = round(strMantissa, -iAdd);
            if (strRounded.count() > strMantissa.count() + iAdd){
                iOrder++;
                if (iDecAdd > iSigAdd){
                    strRounded = round(strRounded, 1);
                }
            }
            strMantissa=strRounded;
        } else if (iAdd > 0){
            for (int i=0; i<iAdd; i++){
                strMantissa += '0';
            }
        }
        if (strMantissa=="" || strMantissa=="0"){
            strMantissa = "0";
            bPositive = true;
            iOrder = 1 + iSigDecs;
            if (iOrder != 0){
                bZeroScientific = true;
            }
        }
    }
    bool bUseScientific = (bScientific || strMantissa.count() > 20 || iOrder > 21 || iOrder < -5 ||
            (iOrder - strMantissa.count() > 0 && trailingZeros(strMantissa) > 0) || bZeroScientific);
    QString strReturnVal = "";
    if (!bPositive){
        strReturnVal += "-";
    }
    if (bUseScientific) {
        strReturnVal += strMantissa[0];
        if (strMantissa.count() > 1){
            strReturnVal += '.' + strMantissa.mid(1, strMantissa.count());
        }
        if (iOrder-1!=0){
            strReturnVal += "e" + QString::number(iOrder-1);
        }
    } else {
        QString strWholePart = "";
        QString strFractPart = "";
        bool bNeedDot = true;
        if (iOrder > 0){
            if (strMantissa.count() > iOrder){
                strWholePart = strMantissa.mid(0, iOrder);
                strFractPart = strMantissa.mid(iOrder, strMantissa.count());
            } else {
                strWholePart = strMantissa;
                bNeedDot = (trailingZeros(strMantissa) != 0);
                for(int i=0; i<iOrder-strMantissa.count(); i++){
                    strWholePart += "0";
                }
            }
        } else {
            for(int i=0; i<-iOrder; i++){
                strFractPart += "0";
            }
            strFractPart += strMantissa;
        }
        strReturnVal += (
            (strWholePart==""?"0":strWholePart) + (bNeedDot?".":"") + strFractPart

        );
    }
    return (strReturnVal);
}

} // gqtlib
} // GS
