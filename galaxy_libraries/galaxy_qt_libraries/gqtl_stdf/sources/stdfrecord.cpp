/****************************************************************************************/
/* Copyright Galaxy                                                                     */
/* This computer program is protected by copyright law                                  */
/* and international treaties. Unauthorized reproduction or                             */
/* distribution of this program, or any portion of it,may                               */
/* result in severe civil and criminal penalties, and will be                           */
/* prosecuted to the maximum extent possible under the law.                             */
/****************************************************************************************/
/*                                                                                      */
/* Notes :                                                                              */
/*                                                                                      */
/****************************************************************************************/

///////////////////////////////////////////////////////////
// StdfParse class IMPLEMENTATION :
// This class contains routines to read records from an
// stdf file
///////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qdatetime.h>
#include <gqtl_log.h>

#include "stdfrecord.h"
#include "stdfrecords_v4.h"

namespace GQTL_STDF
{
///////////////////////////////////////////////////////////
// GENERIC RECORD
///////////////////////////////////////////////////////////
Stdf_Record::Stdf_Record()
{
    mLastFieldAtdf = 0;
    // 24 digits seems to be the max precision for double
    // we can lower that number but then the ascii output could lost precision
    mPrecision=24;
    mFormat='e';
}

Stdf_Record::Stdf_Record(const Stdf_Record& other)
{
    *this = other;
}

Stdf_Record::~Stdf_Record()
{
    // This destructor is virtual and must be implemented in derived classes
}

Stdf_Record &Stdf_Record::operator=(const Stdf_Record &other)
{
    if (this != &other)
    {
        mPrecision      = other.mPrecision;
        mFormat         = other.mFormat;
        m_stRecordInfo  = other.m_stRecordInfo;
        m_uiFieldIndex  = other.m_uiFieldIndex;
        m_uiLineNb      = other.m_uiLineNb;
        mLastFieldAtdf  = other.mLastFieldAtdf;
    }

    return *this;
}

void Stdf_Record::Reset(void)
{
    // This function is virtual and must be implemented in derived classes
    mLastFieldAtdf = 0;
}

void Stdf_Record::SetPrecision(const int &lPrecision)
{
    mPrecision=lPrecision;
}

void Stdf_Record::SetFormat(char lFormat)
{
    mFormat=lFormat;
}

QString Stdf_Record::GetRecordShortName(void)
{
    // This function is virtual and must be implemented in derived classes
    GSLOG(3, "Virtual method called")
    return "";
}

QString Stdf_Record::GetRecordLongName(void)
{
    // This function is virtual and must be implemented in derived classes
    GSLOG(4, "Virtual method called")
    return "";
}

int Stdf_Record::GetRecordType(void)
{
    // This function is virtual and must be implemented in derived classes
    GSLOG(3, "Virtual function called");
    return Rec_UNKNOWN;
}

bool Stdf_Record::Read(GS::StdLib::Stdf& /*clStdf*/)
{
    // This function is virtual and must be implemented in derived classes
    GSLOG(3, "Virtual function called");
    return true;
}

bool Stdf_Record::Write(GS::StdLib::Stdf& /*clStdf*/)
{
    // This function is virtual and must be implemented in derived classes
    GSLOG(3, "Virtual function called");
    return true;
}

void Stdf_Record::GetAsciiFieldList(QStringList& /*listFields*/,
                                        int /*nFieldSelection = 0*/)
{
    // This function is virtual and must be implemented in derived classes
    GSLOG(3, "Virtual function called");
}

void Stdf_Record::GetAsciiString(QString& /*strAsciiString*/,
                                     int /*nFieldSelection = 0*/
                                     )
{
    // This function is virtual and must be implemented in derived classes
    GSLOG(3, "Virtual function called");
}

void Stdf_Record::GetXMLString(QString& /*strXmlString*/,
                                   const int /*nIndentationLevel*/,
                                   int /*nFieldSelection = 0*/)
{
    // This function is virtual and must be implemented in derived classes
    GSLOG(3, "Virtual function called");
}

void Stdf_Record::GetAtdfString(QString& /*strAtdfString*/)
{
    // This function is virtual and must be implemented in derived classes
    GSLOG(3, "Virtual function called");
}

void Stdf_Record::GetAsciiRecord(QString & strRecord, int nFieldSelection /*= 0*/ /*char format, int lPrecision*/ )
{
    //Q_UNUSED(format)

    QString	strAsciiString, strField, strTmp;

    // Write record name
    strRecord.sprintf("** %s (%s) **\n", GetRecordLongName().toLatin1().constData(), GetRecordShortName().toLatin1().constData());

    if((nFieldSelection & FieldFlag_None) == 0)
    {
        // Get fields
        GetAsciiString(strAsciiString, nFieldSelection); // , format

        // Write record header
        strTmp.sprintf("%s.rec_len", GetRecordShortName().toLatin1().constData());
        strField = strTmp.leftJustified(13);
        strTmp.sprintf("%s = %d\n", strField.toLatin1().constData(), m_stRecordInfo.iRecordSize);
        strRecord += strTmp;

        strTmp.sprintf("%s.rec_typ", GetRecordShortName().toLatin1().constData());
        strField = strTmp.leftJustified(13);
        strTmp.sprintf("%s = %d\n", strField.toLatin1().constData(), m_stRecordInfo.iRecordType);
        strRecord += strTmp;

        strTmp.sprintf("%s.rec_sub", GetRecordShortName().toLatin1().constData());
        strField = strTmp.leftJustified(13);
        strTmp.sprintf("%s = %d\n", strField.toLatin1().constData(), m_stRecordInfo.iRecordSubType);
        strRecord += strTmp;

        // Write all fields
        strRecord += strAsciiString;
    }
}

void Stdf_Record::SetLastFieldAtdf(const int lLastField)
{
    // This test is mandatory because field position  is not the same between stdf and atdf
    if (mLastFieldAtdf < lLastField)
        mLastFieldAtdf = lLastField;
}

QMap<int, Stdf_Record*> CStdfRecordFactory::m_oRecordMap;

void CStdfRecordFactory::clear(){
    qDeleteAll(m_oRecordMap.values());
    m_oRecordMap.clear();
}

Stdf_Record *CStdfRecordFactory::recordInstanciate(int nRecordType){

    if(nRecordType == Stdf_Record::Rec_FAR)
        return  new Stdf_FAR_V4;
    if(nRecordType == Stdf_Record::Rec_ATR)
        return  new Stdf_ATR_V4;
    if(nRecordType == Stdf_Record::Rec_MIR)
        return  new Stdf_MIR_V4;
    if(nRecordType == Stdf_Record::Rec_MRR)
        return  new Stdf_MRR_V4;
    if(nRecordType == Stdf_Record::Rec_PCR)
        return  new Stdf_PCR_V4;
    if(nRecordType == Stdf_Record::Rec_HBR)
        return  new Stdf_HBR_V4;
    if(nRecordType == Stdf_Record::Rec_SBR)
        return  new Stdf_SBR_V4;
    if(nRecordType == Stdf_Record::Rec_PMR)
        return  new Stdf_PMR_V4;
    if(nRecordType == Stdf_Record::Rec_PGR)
        return  new Stdf_PGR_V4;
    if(nRecordType == Stdf_Record::Rec_PLR)
        return  new Stdf_PLR_V4;
    if(nRecordType == Stdf_Record::Rec_RDR)
        return  new Stdf_RDR_V4;
    if(nRecordType == Stdf_Record::Rec_SDR)
        return  new Stdf_SDR_V4;
    if(nRecordType == Stdf_Record::Rec_WIR)
        return  new Stdf_WIR_V4;
    if(nRecordType == Stdf_Record::Rec_WRR)
        return  new Stdf_WRR_V4;
    if(nRecordType == Stdf_Record::Rec_WCR)
        return  new Stdf_WCR_V4;
    if(nRecordType == Stdf_Record::Rec_PIR)
        return  new Stdf_PIR_V4;
    if(nRecordType == Stdf_Record::Rec_PRR)
        return  new Stdf_PRR_V4;
    if(nRecordType == Stdf_Record::Rec_TSR)
        return  new Stdf_TSR_V4;
    if(nRecordType == Stdf_Record::Rec_PTR)
        return  new Stdf_PTR_V4;
    if(nRecordType == Stdf_Record::Rec_MPR)
        return  new Stdf_MPR_V4;
    if(nRecordType == Stdf_Record::Rec_FTR)
        return  new Stdf_FTR_V4;
    if(nRecordType == Stdf_Record::Rec_BPS)
        return  new Stdf_BPS_V4;
    if(nRecordType == Stdf_Record::Rec_EPS)
        return  new Stdf_EPS_V4;
    if(nRecordType == Stdf_Record::Rec_GDR)
        return  new Stdf_GDR_V4;
    if(nRecordType == Stdf_Record::Rec_DTR)
        return  new Stdf_DTR_V4;
    if(nRecordType == Stdf_Record::Rec_RESERVED_IMAGE)
        return  new Stdf_RESERVED_IMAGE_V4;
    if(nRecordType == Stdf_Record::Rec_RESERVED_IG900)
        return  new Stdf_RESERVED_IG900_V4;
    if(nRecordType == Stdf_Record::Rec_UNKNOWN)
        return  new Stdf_UNKNOWN_V4;
    if(nRecordType == Stdf_Record::Rec_VUR)
        return  new Stdf_VUR_V4;
    if(nRecordType == Stdf_Record::Rec_PSR)
        return  new Stdf_PSR_V4;
    if(nRecordType == Stdf_Record::Rec_NMR)
        return  new Stdf_NMR_V4;
    if(nRecordType == Stdf_Record::Rec_CNR)
        return  new Stdf_CNR_V4;
    if(nRecordType == Stdf_Record::Rec_SSR)
        return  new Stdf_SSR_V4;
    if(nRecordType == Stdf_Record::Rec_CDR)
        return  new Stdf_CDR_V4;
    if(nRecordType == Stdf_Record::Rec_STR)
        return  new Stdf_STR_V4;

    return 0;


}

Stdf_Record *CStdfRecordFactory::recordInstance(int nRecordType){
    if(m_oRecordMap.contains(nRecordType))
        return m_oRecordMap[nRecordType];

    Stdf_Record *poObj = recordInstanciate(nRecordType);
    if(poObj)
        m_oRecordMap.insert(nRecordType, poObj);
    return poObj;

}
}
