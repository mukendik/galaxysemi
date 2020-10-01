#include "gexdb_plugin_galaxy.h"
#include "test_filter.h"

/////////////////////////////////////////////////////////////////////////////////////////
// GexDbPlugin_Galaxy_TestFilter OBJECT
// Handle test list for filtering on tests at extraction
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// Description : Constructor
//
// Argument(s) :
//		strFullTestlist: Full test list specified by user, including keywords, ie "1,2,10-20" "1,pat,100-105"
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
GexDbPlugin_Galaxy_TestFilter::GexDbPlugin_Galaxy_TestFilter(const QString & strFullTestlist)
{
	QStringList::Iterator	itTestlist;
	QString					strToken;
    QStringList				strlInterval, strlFullTestlist = strFullTestlist.split(QRegExp("[,;]"));
	unsigned int			uiBeginValue, uiEndValue;
	bool					bOk;

	m_strTestNbList_Full = strFullTestlist.simplified();
	m_bExtractPatTests = false;

	// Check if testlist not "*" or ""
	if((m_strTestNbList_Full == "*") || m_strTestNbList_Full.isEmpty())
		return;

	// Parse the test list to extract keywords and create a clean testlist
	for(itTestlist = strlFullTestlist.begin(); itTestlist != strlFullTestlist.end(); itTestlist++)
	{
		strToken = (*itTestlist);
		if(strToken.toLower() == "pat")
			m_bExtractPatTests = true;
		else if(strToken == "*")
		{
			// If one token is "*", same as if testlist is "*"
			m_strTestNbList_Full = "*";
			m_strTestNbList_Clean = "";
			return;
		}
		else
		{
			switch(strToken.count('-'))
			{
				case 0:
					uiBeginValue = strToken.toUInt(&bOk);
					if(bOk)
					{
						if(!m_strTestNbList_Clean.isEmpty())
							m_strTestNbList_Clean += ",";
						m_strTestNbList_Clean += strToken;
					}
					break;

				case 1:
                    strlInterval = strToken.split('-');
					if(strlInterval.size() != 2)
						break;
					uiBeginValue = strlInterval[0].toUInt(&bOk);
					if(!bOk)
						break;
					uiEndValue = strlInterval[1].toUInt(&bOk);
					if(!bOk)
						break;
					// Make sure End > Begin, otherwise swap
					if(uiBeginValue > uiEndValue)
						strToken = QString::number(uiEndValue) + "-" + QString::number(uiBeginValue);
					// Add token
					if(!m_strTestNbList_Clean.isEmpty())
						m_strTestNbList_Clean += ",";
					m_strTestNbList_Clean += strToken;

					break;

				default:
					break;
			}
		}
	}
}


