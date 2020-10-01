#include "xychart_data.h"

GexDbPlugin_XYChart_Data::GexDbPlugin_XYChart_Data()
{
    m_pcXAxisLabels  = NULL;
    m_plfYAxisData   = NULL;
    m_plfYAxisData_2 = NULL;
    m_plfData        = NULL;
    m_plfData_1      = NULL;
    m_plfData_2      = NULL;
    m_plfData_3      = NULL;
    m_plfData_4      = NULL;
    m_nNbDataPoints  = 0;
    m_bDoubleYAxis   = false;
    m_nMarkerPrecision   = 0;
    m_nMarkerPrecision_2 = 0;
}

GexDbPlugin_XYChart_Data::~GexDbPlugin_XYChart_Data()
{
    if(m_pcXAxisLabels != NULL)
    {
        int lSize = sizeof(m_pcXAxisLabels);
        for (int lIter = 0; lIter < lSize; ++lIter)
            delete m_pcXAxisLabels[lIter];
        delete [] m_pcXAxisLabels;
    }
    if(m_plfData != NULL)
        delete [] m_plfData;
    if(m_plfData_1 != NULL)
        delete [] m_plfData_1;
    if(m_plfData_2 != NULL)
        delete [] m_plfData_2;
    if(m_plfData_3 != NULL)
        delete [] m_plfData_3;
    if(m_plfData_4 != NULL)
        delete [] m_plfData_4;
    m_strlStrings.clear();
    m_lflist_Data.clear();
    m_lflist_Data_1.clear();
    m_lflist_Data_2.clear();
}

void GexDbPlugin_XYChart_Data::Add(const double lfData, const QString &strLabel)
{
    m_strlStrings.append(strLabel);
    m_lflist_Data.append(lfData);
    m_nNbDataPoints++;
}

void GexDbPlugin_XYChart_Data::Add(const double lfData, const QString &strLabel, const double lfData_1)
{
    m_strlStrings.append(strLabel);
    m_lflist_Data.append(lfData);
    m_lflist_Data_1.append(lfData_1);
    m_nNbDataPoints++;
}

void GexDbPlugin_XYChart_Data::Add(const double lfData, const QString &strLabel, const double lfData_1, const double lfData_2)
{
    m_strlStrings.append(strLabel);
    m_lflist_Data.append(lfData);
    m_lflist_Data_1.append(lfData_1);
    m_lflist_Data_2.append(lfData_2);
    m_nNbDataPoints++;
}

void GexDbPlugin_XYChart_Data::Add(const double lfData, const QString &strLabel, const double lfData_1, const double lfData_2, const double lfData_3)
{
    m_strlStrings.append(strLabel);
    m_lflist_Data.append(lfData);
    m_lflist_Data_1.append(lfData_1);
    m_lflist_Data_2.append(lfData_2);
    m_lflist_Data_3.append(lfData_3);
    m_nNbDataPoints++;
}

void GexDbPlugin_XYChart_Data::Add(const double lfData, const QString &strLabel, const double lfData_1, const double lfData_2, const double lfData_3, const double lfData_4)
{
    m_strlStrings.append(strLabel);
    m_lflist_Data.append(lfData);
    m_lflist_Data_1.append(lfData_1);
    m_lflist_Data_2.append(lfData_2);
    m_lflist_Data_3.append(lfData_3);
    m_lflist_Data_4.append(lfData_4);
    m_nNbDataPoints++;
}

void GexDbPlugin_XYChart_Data::Convert()
{
    if(m_nNbDataPoints > 0)
    {
        if(m_pcXAxisLabels != NULL)
        {
            int lSize = sizeof(m_pcXAxisLabels);
            for (int lIter = 0; lIter < lSize; ++lIter)
                delete m_pcXAxisLabels[lIter];
            delete [] m_pcXAxisLabels;
        }
        if(m_plfData != NULL)
            delete [] m_plfData;
        if(m_plfData_1 != NULL)
            delete [] m_plfData_1;
        if(m_plfData_2 != NULL)
            delete [] m_plfData_2;
        if(m_plfData_3 != NULL)
            delete [] m_plfData_3;
        if(m_plfData_4 != NULL)
            delete [] m_plfData_4;
        m_pcXAxisLabels = new const char*[m_nNbDataPoints];
        m_plfData = new double[m_nNbDataPoints];
        if(m_lflist_Data_1.count() > 0)
            m_plfData_1 = new double[m_nNbDataPoints];
        if(m_lflist_Data_2.count() > 0)
            m_plfData_2 = new double[m_nNbDataPoints];
        if(m_lflist_Data_3.count() > 0)
            m_plfData_3 = new double[m_nNbDataPoints];
        if(m_lflist_Data_4.count() > 0)
            m_plfData_4 = new double[m_nNbDataPoints];
        for(int i=0; i<m_nNbDataPoints; i++)
        {
            QByteArray lString = m_strlStrings[i].toLatin1().constData();
            char *lData = new char[lString.size() + 1];
            strcpy(lData, lString.data());
            m_pcXAxisLabels[i] = lData;

            m_plfData[i] = m_lflist_Data[i];
            if(m_lflist_Data_1.count() > 0)
                m_plfData_1[i] = m_lflist_Data_1[i];
            if(m_lflist_Data_2.count() > 0)
                m_plfData_2[i] = m_lflist_Data_2[i];
            if(m_lflist_Data_3.count() > 0)
                m_plfData_3[i] = m_lflist_Data_3[i];
            if(m_lflist_Data_4.count() > 0)
                m_plfData_4[i] = m_lflist_Data_4[i];
        }
    }
}
