///////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////
#include "chart_director.h"
#include <gqtl_log.h>

#define	_CHARTDIR_LICENSE_	"RDST-34UW-3FHT-4Z9K-FD32-967D" // ChartDirector commercial license ID: note Redistribution code doesn't work under Linux & Solaris.

///////////////////////////////////////////////////////////////////////////////////
// Class CChartDirector - class which 
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CChartDirector::CChartDirector() : m_pXYChart(0), m_bEnabled(true), m_nBackGroundColor(0)
{
	// Set license ID
	Chart::setLicenseCode(_CHARTDIR_LICENSE_);

#if 0
//#if defined(unix) || __MACH__ && defined(__sun__)

	// We need Solaris V6 or higher.
 	struct utsname ComputerName;
  	uname(&ComputerName);

	// Examples of release string:
	// 5.3		2.3 
	// 5.4		2.4 
	// 5.5		2.5 
	// 5.5.1	2.5.1 
	// 5.6		2.6 
	// 5.7		7 
	// 5.8		8
	// 5.9		9

	int	iMajor,iMinor;
	sscanf(ComputerName.release,"%d%*c%d",&iMajor,&iMinor);

	if(iMajor != 5 || iMinor < 6)
		m_bEnabled = false;	// Older than Solaris 6.X

#endif
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CChartDirector::~CChartDirector()
{
	clearXYChart();
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	void allocateXYChart(int nWidth, int nHeight, int nBackgroundColor)
//
// Description	:	
//
///////////////////////////////////////////////////////////////////////////////////
bool CChartDirector::allocateXYChart(int nWidth, int nHeight, int nBackgroundColor)
{
	if (isEnabled())
	{
		// Destroy the previous one if any
		clearXYChart();

		// Allocate a new one
		try
		{
			m_pXYChart = new XYChart(nWidth, nHeight, nBackgroundColor);
		}
		catch(const std::bad_alloc &e)
		{
			GSLOG(SYSLOG_SEV_ERROR, e.what() );
			return false;
		}

		m_nBackGroundColor = nBackgroundColor;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	void buildOwnerChart()
//
// Description	:	
//
///////////////////////////////////////////////////////////////////////////////////
void CChartDirector::clearXYChart()
{
	if (m_pXYChart)
	{
		delete m_pXYChart;
		m_pXYChart = NULL;

        marks.clear();
	}
}

void  CChartDirector::addLine(double x1, double y1, double x2, double y2)
{
    if (m_pXYChart)
        addLine((int)x1,(int)y1, (int)x2, (int)y2);
}

Mark * CChartDirector::addMarker(Axis * pAxis,
                                 double lfValue,
                                 int nColor,
                                 const QString& strLabel,
                                 int nWidth /*= 1*/,
                                 Chart::Alignment alignment /*= Chart::TopLeft*/,
                                 bool smartAlignement /*=true*/)
{
	Mark * pMarker = NULL;

	if (pAxis)
	{
		pMarker = pAxis->addMark(lfValue, nColor, strLabel.toLatin1().constData());
		pMarker->setLineWidth(nWidth);
		pMarker->setAlignment(alignment);
	}

    if(pMarker && smartAlignement)
    {
        MarkOrdered lMarkerOrder = { pMarker, lfValue };
        marks.push_back(lMarkerOrder);
        sortMarkersAndUpdate();
    }

	return pMarker;
}

bool SortMarker(const MarkOrdered& rh1, const MarkOrdered& rh2)
{
   return rh1.mValue < rh2.mValue;
}

void CChartDirector::sortMarkersAndUpdate()
{
    qSort(marks.begin(), marks.end(), SortMarker);

    int i =0;
    int j = 0, w = 0;

    if(marks.size() == 15)
        i = 0;

    QList<MarkOrdered>::iterator iter(marks.begin());
    QList<MarkOrdered>::iterator  iterEnd(marks.end());
    for(; iter != iterEnd; ++iter, ++i ) {
       if(iter->mMarker == 0)
           continue;

        if(i%2) {
            iter->mMarker->setFontAngle(45);
            iter->mMarker->setAlignment(Chart::TopRight);

           /* if(i <= 1) {

                mLastMarkOrdered = *iter;
                iter->mMarker->setPos(0, -iter->mMarker->getWidth());
            }
            else {

                int lLastXLeft = mLastMarkOrdered.mMarker->getLeftX();

                //int lWidth = mLastMarkOrdered.mMarker->getWidth();
                //int lLastXRight=  lWidth + lLastXLeft;

                int lCurrentXLeft = iter->mMarker->getLeftX();
                if((lCurrentXLeft - 2) <= lLastXLeft)
                {
                    if(w%3 == 0)
                        iter->mMarker->setPos(30, -iter->mMarker->getWidth());
                    else if(w%2 == 0)
                        iter->mMarker->setPos(20, -iter->mMarker->getWidth());
                    else
                        iter->mMarker->setPos(10, -iter->mMarker->getWidth());

                    ++w;
                }
                else {
                     iter->mMarker->setPos(0, -iter->mMarker->getWidth());
                     w = 0;
                }

                mLastMarkOrdered = *iter;

            }*/

            if(w%3 == 0)
                iter->mMarker->setPos(-10, -iter->mMarker->getWidth());
            else if(w%2 == 0)
                iter->mMarker->setPos(0, -iter->mMarker->getWidth());
            else
                iter->mMarker->setPos(10, -iter->mMarker->getWidth());
            ++w;

        }
        else {
            iter->mMarker->setFontAngle(0);
            iter->mMarker->setAlignment(Chart::TopLeft);

            if(j%3 == 0)
                iter->mMarker->setPos(0,0);
            else if(j%2 == 0)
                iter->mMarker->setPos(0,10);
            else
                iter->mMarker->setPos(0,20);

            ++j;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	bool drawChart(const QString& strFile, const QString& strCopyright)
//
// Description	:	
//
///////////////////////////////////////////////////////////////////////////////////
bool CChartDirector::drawChart(const QString& strFile, const QString& strCopyright)
{
	if (m_pXYChart)
	{
		if (!strCopyright.isEmpty())
		{
			int nRed	= (int)((float)((m_nBackGroundColor & 0x00ff0000) >> 4) * 0.8F);
			int nGreen  = (int)((float)((m_nBackGroundColor & 0x0000ff00) >> 2) * 0.8F);
			int nBlue   = (int)((float)((m_nBackGroundColor & 0x000000ff)) * 0.8F);
		
			int nColor	= (nRed << 4) | (nGreen << 2) | (nBlue);
			
			// Add Galaxy copyright
			m_pXYChart->getDrawArea()->text(strCopyright.toLatin1().constData(), GEXCHART_FONTNAME, 8, 10, m_pXYChart->getDrawArea()->getHeight() - 20, nColor);
		}

		// Convert ChartDirector image to Bitmap
        m_pXYChart->makeChart(strFile.toLatin1().constData());

        return true;
	}

	return false;
}
