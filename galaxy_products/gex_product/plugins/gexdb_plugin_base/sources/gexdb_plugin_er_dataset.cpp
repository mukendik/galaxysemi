///////////////////////////////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////////////////////////////
#include "gexdb_plugin_er_dataset.h"
#include "gex_shared.h"
#include "gexdb_plugin_base.h"

///////////////////////////////////////////////////////////////////////////////////
// C Function
///////////////////////////////////////////////////////////////////////////////////
int gexCompareString(const QString& strLeft, const QString& strRight)
{
	bool	bOkLeft			= false;
	bool	bOkRight		= false;
	int		nLeftValue		= strLeft.toInt(&bOkLeft);
	int		nRightValue		= strRight.toInt(&bOkRight);

	if (bOkLeft && bOkRight)
		return (nLeftValue - nRightValue);

	return strLeft.compare(strRight);
}

uint qHash(const GexDbPluginERDatasetField& erField)
{
	return qHash(erField.name());
}

static QStringList sKeyOrder = QStringList() << "$PAggregate|asc" << "$PSerie|asc";

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexDbPluginERPostProcessing
//
// Description	:	Class which defines a post processing action
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexDbPluginERPostProcessing::GexDbPluginERPostProcessing() : m_uiTopNCount(0)
{

}

GexDbPluginERPostProcessing::GexDbPluginERPostProcessing(const GexDbPluginERPostProcessing& other)
{
	*this = other;
}

///////////////////////////////////////////////////////////
// Operators
///////////////////////////////////////////////////////////
GexDbPluginERPostProcessing& GexDbPluginERPostProcessing::operator=(const GexDbPluginERPostProcessing& other)
{
	if (this != &other)
	{
		m_uiTopNCount	= other.m_uiTopNCount;
		m_strOtherName	= other.m_strOtherName;
		m_strlstFields	= other.m_strlstFields;
		m_lstCriteria	= other.m_lstCriteria;
	}

	return *this;
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool GexDbPluginERPostProcessing::execute(GexDbPluginERDatasetGroup &datasetGroup) const
//
// Description	:	Execute a post processing action
//
///////////////////////////////////////////////////////////////////////////////////
bool GexDbPluginERPostProcessing::execute(GexDbPluginERDatasetGroup &datasetGroup) const
{
	datasetGroup.clean();

	QList<GexDbPluginERDatasetRow>::iterator itRow = datasetGroup.m_lstRow.begin();

	while (itRow != datasetGroup.m_lstRow.end())
	{
		datasetGroup.addAggregate(*itRow);

		++itRow;
	}

    sKeyOrder = m_strlstFields;

	qSort(datasetGroup.m_lstSerie);

	uint uiTopNSerie = 0;

	for(int nSerie = 0; nSerie < datasetGroup.m_lstSerie.count(); nSerie++)
	{
		bool bMatchCriteria = true;

		for (int nCriteria = 0; nCriteria < m_lstCriteria.count(); ++nCriteria)
		{
			bMatchCriteria &= datasetGroup.m_lstSerie.at(nSerie).field(m_lstCriteria.at(nCriteria).m_strField).match(m_lstCriteria.at(nCriteria).m_strOperator,
																										m_lstCriteria.at(nCriteria).m_strValue);
		}

		if (bMatchCriteria)
		{
			if (uiTopNSerie >= m_uiTopNCount)
			{
				GexDbPluginERDatasetField field = datasetGroup.m_lstSerie.at(nSerie).field("SerieName");
				for (int nRow = 0; nRow < datasetGroup.m_lstRow.count(); nRow++)
				{
					if (datasetGroup.m_lstRow.at(nRow).field("SerieName").value() == field.value())
					{
						datasetGroup.m_lstRow[nRow].setSerie(m_strOtherName);

						if (datasetGroup.m_lstRow[nRow].hasBinGroupBy())
						{
							datasetGroup.m_lstRow[nRow].setField("bin_no",		QVariant(QString("100000")));
							datasetGroup.m_lstRow[nRow].setField("bin_name",	QVariant(QString("")));
						}
					}
				}
			}
			else
				++uiTopNSerie;
		}
	}

	datasetGroup.groupRowBy(QStringList() << "SerieName" << "AggregateName");

	// Clean up older data
	datasetGroup.clean();

	return true;
}

void GexDbPluginERPostProcessing::addField(const QString& strField, const QString& strOrder)
{
	if (strField.isEmpty() == false)
	{
		QString strTemp = strField;

		if (strOrder.isEmpty() == false)
			strTemp += "|" + strOrder;

		m_strlstFields.append(strTemp);
	}
}

void GexDbPluginERPostProcessing::addCriteria(const GexDbPluginERPostProcessingCriteria& criteria)
{
	m_lstCriteria.append(criteria);
}



///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexDbPluginERDatasetField
//
// Description	:	Class which defines a field
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexDbPluginERDatasetField::GexDbPluginERDatasetField(const QString& strName) : m_strName(strName), m_varValue("")
{
}

///////////////////////////////////////////////////////////
// Copy constructor
///////////////////////////////////////////////////////////
GexDbPluginERDatasetField::GexDbPluginERDatasetField(const GexDbPluginERDatasetField& erField)
{
	*this = erField;
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexDbPluginERDatasetField::~GexDbPluginERDatasetField()
{

}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	const GexDbPluginERDatasetField& GexDbPluginERDatasetField::operator=(const GexDbPluginERDatasetField& erField)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
GexDbPluginERDatasetField&	GexDbPluginERDatasetField::operator=(const GexDbPluginERDatasetField& erField)
{
	if (this != &erField)
	{
		m_strName	= erField.name();
		m_varValue	= erField.value();
	}

	return *this;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool GexDbPluginERDatasetField::operator==(const GexDbPluginERDatasetField& erField) const
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
bool GexDbPluginERDatasetField::operator==(const GexDbPluginERDatasetField& erField) const
{
	return (m_strName.compare(erField.name(), Qt::CaseInsensitive) == 0);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool GexDbPluginERDatasetField::match(const QString& strOperator, const QVariant& varValue)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
bool GexDbPluginERDatasetField::match(const QString& strOperator, const QString& strValue) const
{
	QStringList strlstValue = strValue.split("|");
	QVariant	varValue;
	bool		bMatch		= false;

	for (int nValue = 0; nValue < strlstValue.count(); ++nValue)
	{
		varValue = strlstValue.at(nValue);

		if (strOperator == "=")
			bMatch |= (varValue == m_varValue);

		if (strOperator == "!=")
			bMatch |= (varValue != m_varValue);
	}

	return bMatch;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexDbPluginERDataset
//
// Description	:	Class which defines a dataset
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexDbPluginERDataset::GexDbPluginERDataset()
{
	m_nCurrentGroup = 0;
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexDbPluginERDataset::~GexDbPluginERDataset()
{

}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

void GexDbPluginERDataset::applyPostProcessing(const GexDbPluginERDatasetSettings& settings)
{
	for (int nGroup = 0; nGroup < m_lstGroup.count(); ++nGroup)
	{
		for(int nAlgo = 0; nAlgo < settings.postProcesses().count(); ++nAlgo)
			settings.postProcesses().at(nAlgo).execute(m_lstGroup[nGroup]);
	}
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexDbPluginERDataset::addRow(const GexDbPluginERDatasetRow &resultRow)
//
// Description	:	Add a row to the dataset
//
///////////////////////////////////////////////////////////////////////////////////
void GexDbPluginERDataset::addRow(const GexDbPluginERDatasetRow &resultRow)
{
	GexDbPluginERDatasetGroup resultGroup;
	resultGroup.setGroup(resultRow.group());

	int nIndex = m_lstGroup.indexOf(resultGroup);

	if (nIndex == -1)
	{
		resultGroup.addRow(resultRow);
		resultGroup.setGroupId(m_lstGroup.count());
		m_lstGroup.append(resultGroup);
	}
	else
		m_lstGroup[nIndex].addRow(resultRow);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexDbPluginERDataset::addRow(const QString& strEmptyAggregate)
//
// Description	:	Add a row to the dataset
//
///////////////////////////////////////////////////////////////////////////////////
void GexDbPluginERDataset::addRow(const QString& strEmptyAggregate)
{
	for (int nGroup = 0; nGroup < m_lstGroup.count(); ++nGroup)
		m_lstGroup[nGroup].addRow(strEmptyAggregate);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexDbPluginERDataset::sortGroups()
//
// Description	:	Ordering groups
//
//////////////////////////////////////////	/////////////////////////////////////////
void GexDbPluginERDataset::sortGroups()
{
	for (int nGroup = 0; nGroup < m_lstGroup.count(); ++nGroup)
		m_lstGroup[nGroup].sort(m_lstOrderBy);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexDbPluginERDataset::setCurrentGroup(const QString& strGroup)
//
// Description	:	Define the current group to process
//
//////////////////////////////////////////	/////////////////////////////////////////
void GexDbPluginERDataset::setCurrentGroup(const QString& strGroup)
{
	for (int nGroup = 0; nGroup < m_lstGroup.count(); ++nGroup)
	{
		if (m_lstGroup.at(nGroup).group() == strGroup)
		{
			m_nCurrentGroup = nGroup;
			return;
		}
	}

	m_nCurrentGroup = -1;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexDbPluginERDatasetGroup
//
// Description	:	Class which defines a group into a dataset
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexDbPluginERDatasetGroup::GexDbPluginERDatasetGroup() : GexDbPluginERDatasetSerie()
{
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexDbPluginERDatasetGroup::~GexDbPluginERDatasetGroup()
{

}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	int GexDbPluginERDatasetGroup::indexOfSerie(const QString& strSerie)
//
// Description	:	Retrieve the index of the given serie
//
///////////////////////////////////////////////////////////////////////////////////
int GexDbPluginERDatasetGroup::indexOfSerie(const QString& strSerie) const
{
	GexDbPluginERDatasetSerie resultSerie;

	resultSerie.setSerie(strSerie);

	return m_lstSerie.indexOf(resultSerie);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	const GexDbPluginERDatasetRow& GexDbPluginERDatasetGroup::serieAt(int nIndex)
//
// Description	:	Retrieve the serie of the given index
//
///////////////////////////////////////////////////////////////////////////////////
const GexDbPluginERDatasetSerie& GexDbPluginERDatasetGroup::serieAt(int nIndex) const
{
	return m_lstSerie.at(nIndex);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexDbPluginERDatasetGroup::addRow(const GexDbPluginERDatasetRow& rowResult)
//
// Description	:	Add a row to the group
//
///////////////////////////////////////////////////////////////////////////////////
void GexDbPluginERDatasetGroup::addRow(const GexDbPluginERDatasetRow& rowResult)
{
	QString strAggregateSerie = rowResult.aggregate() + ";" + rowResult.serie();

	if (m_hashAggregateSerie.contains(strAggregateSerie))
	{
		int nIndex = m_hashAggregateSerie.value(strAggregateSerie);

		m_lstRow[nIndex].setBinVolume(m_lstRow[nIndex].binVolume() + rowResult.binVolume());
	}
	else
	{
		m_hashAggregateSerie.insert(strAggregateSerie, m_lstRow.count());
		m_lstRow.append(rowResult);
	}

	if (m_lstSerieValue.indexOf(rowResult.serie()) == -1)
		m_lstSerieValue.append(rowResult.serie());
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexDbPluginERDatasetGroup::addRow(const QString& strEmptyAggregate)
//
// Description	:	Add a empty aggregate to the group
//
///////////////////////////////////////////////////////////////////////////////////
void GexDbPluginERDatasetGroup::addRow(const QString& strEmptyAggregate)
{
	GexDbPluginERDatasetRow datasetRow(GexDbPluginERDatasetRow::RowEmpty);

	datasetRow.setGroup(group());
	datasetRow.setAggregate(strEmptyAggregate);

	for (int nSerie = 0; nSerie < m_lstSerieValue.count(); ++nSerie)
	{
		datasetRow.setSerie(m_lstSerieValue.at(nSerie));

		addRow(datasetRow);
	}
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexDbPluginERDatasetGroup::pushAggregateValue(const QString &strAggregateValue)
//
// Description	:	Add an aggregate value to the group
//
///////////////////////////////////////////////////////////////////////////////////
void GexDbPluginERDatasetGroup::pushAggregateValue(const QString &strAggregateValue)
{
	if (m_lstAggregateValue.indexOf(strAggregateValue) == -1)
		m_lstAggregateValue.append(strAggregateValue);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexDbPluginERDatasetGroup::sort(const QStringList & lstKeyOrder)
//
// Description	:	Sort data
//
///////////////////////////////////////////////////////////////////////////////////
void GexDbPluginERDatasetGroup::sort(const QStringList& lstKeyOrder)
{
	if (lstKeyOrder.count() > 0)
        sKeyOrder = lstKeyOrder;
	else
        sKeyOrder = QStringList("$PAggregate|asc") << "$PSerie|asc";

	// Clean up older data
	clean();

	// Sort rows
	qSort(m_lstRow);

	QList<GexDbPluginERDatasetRow>::iterator itRow = m_lstRow.begin();

	while (itRow != m_lstRow.end())
	{
		addAggregate(*itRow);

		++itRow;
	}

	// Find the Overall Serie and move it at the last item
	GexDbPluginERDatasetSerie resultSerie;

	resultSerie.setGroup(group());
	resultSerie.setSerie("Overall");

	int		nSerieIndex		= m_lstSerie.indexOf(resultSerie);

	if (nSerieIndex != -1)
		m_lstSerie.append(m_lstSerie.takeAt(nSerieIndex));

}

void GexDbPluginERDatasetGroup::executeTopNFailBin(int nTopN)
{
	// Clean up older data
	clean();

	QList<GexDbPluginERDatasetRow>::iterator itRow = m_lstRow.begin();

	while (itRow != m_lstRow.end())
	{
		addAggregate(*itRow);

		++itRow;
	}

    sKeyOrder = QStringList("bin_volume|desc");

	qSort(m_lstSerie);

	int nTopNSerie = 0;

	for(int nSerie = 0; nSerie < m_lstSerie.count(); nSerie++)
	{
		if (m_lstSerie.at(nSerie).field("bin_cat").value().toString() == "F")
		{
			if (nTopNSerie >= nTopN)
			{
				GexDbPluginERDatasetField field = m_lstSerie.at(nSerie).field("SerieName");
				for (int nRow = 0; nRow < m_lstRow.count(); nRow++)
				{
					if (m_lstRow.at(nRow).field("SerieName").value() == field.value())
					{
						m_lstRow[nRow].setSerie("Others");
						m_lstRow[nRow].setField("SerieName",	QVariant(QString("Others")));

						if (m_lstRow[nRow].hasBinGroupBy())
						{
							m_lstRow[nRow].setField("bin_no",		QVariant(QString("-1")));
							m_lstRow[nRow].setField("bin_name",		QVariant(QString("")));
						}
					}
				}
			}
			else
				++nTopNSerie;
		}
	}

	groupRowBy(QStringList() << "SerieName" << "AggregateName");

	// Clean up older data
	clean();
}

void GexDbPluginERDatasetGroup::groupRowBy(const QStringList &strlstField)
{
    sKeyOrder = strlstField;

	qSort(m_lstRow);

	GexDbPluginERDatasetRow			rowMerged;
	QList<GexDbPluginERDatasetRow>	lstRowMerged;

	for (int nRow = 0; nRow < m_lstRow.count(); nRow++)
	{
		if (nRow == 0)
			rowMerged = m_lstRow[nRow];
		else if (rowMerged < m_lstRow.at(nRow))
		{
			lstRowMerged.append(rowMerged);

			rowMerged = m_lstRow[nRow];
		}
		else
		{
			if (m_lstRow[nRow].hasBinGroupBy() == false)
			{
				rowMerged.setVolume(rowMerged.volume() + m_lstRow.at(nRow).volume());
				rowMerged.setGoodVolume(rowMerged.goodVolume() + m_lstRow.at(nRow).goodVolume());
				rowMerged.setGrossVolume(rowMerged.grossVolume() + m_lstRow.at(nRow).grossVolume());
			}

			rowMerged.setBinVolume(rowMerged.binVolume() + m_lstRow.at(nRow).binVolume());
		}
	}
	lstRowMerged.append(rowMerged);

	m_lstRow.clear();
	m_lstRow = lstRowMerged;

}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool GexDbPluginERDatasetGroup::addAggregate(const GexDbPluginERDatasetRow &resultAggregate)
//
// Description	:	Add an aggregate to the serie
//
///////////////////////////////////////////////////////////////////////////////////
bool GexDbPluginERDatasetGroup::addAggregate(const GexDbPluginERDatasetRow &resultAggregate)
{
	GexDbPluginERDatasetSerie resultSerie;

	resultSerie.setGroup(resultAggregate.group());
	resultSerie.setSerie(resultAggregate.serie());

	int		nSerieIndex			= m_lstSerie.indexOf(resultSerie);
	int		nAggregateIndex		= indexOfAggregate(resultAggregate.aggregate());

	if (nSerieIndex == -1)
	{
		// Add aggregate to the serie
		resultSerie.addAggregate(resultAggregate);

		m_lstSerie.append(resultSerie);
	}
	else
		m_lstSerie[nSerieIndex].addAggregate(resultAggregate);

	if (nAggregateIndex == -1)
	{
		m_hashAggregatePosition.insert(resultAggregate.aggregate(), m_lstAggregate.count());
		m_lstAggregate.append(resultAggregate);

		if (resultAggregate.binGroupBy() == GexDbPluginERDatasetRow::BinGroupByAggregate && nSerieIndex != -1)
		{
			// Do nothing
		}
		else
		{
			setVolume(volume() + resultAggregate.volume());
			setGoodVolume(goodVolume() + resultAggregate.goodVolume());
			setGrossVolume(grossVolume() + resultAggregate.grossVolume());
			setWaferCount(waferCount() + resultAggregate.waferCount());
		}

		setBinVolume(binVolume() + resultAggregate.binVolume());
	}
	else
	{
		if (resultAggregate.binGroupBy() == GexDbPluginERDatasetRow::BinGroupByAggregate)
		{
			m_lstAggregate[nAggregateIndex].setVolume(m_lstAggregate.at(nAggregateIndex).volume() + resultAggregate.volume());
			m_lstAggregate[nAggregateIndex].setGoodVolume(m_lstAggregate.at(nAggregateIndex).goodVolume() + resultAggregate.goodVolume());
			m_lstAggregate[nAggregateIndex].setGrossVolume(m_lstAggregate.at(nAggregateIndex).grossVolume() + resultAggregate.grossVolume());
			m_lstAggregate[nAggregateIndex].setWaferCount(m_lstAggregate.at(nAggregateIndex).waferCount() + resultAggregate.waferCount());

			if (nSerieIndex == -1)
			{
				setVolume(volume() + resultAggregate.volume());
				setGoodVolume(goodVolume() + resultAggregate.goodVolume());
				setGrossVolume(grossVolume() + resultAggregate.grossVolume());
				setWaferCount(waferCount() + resultAggregate.waferCount());
			}
		}
		else if (resultAggregate.binGroupBy() != GexDbPluginERDatasetRow::BinGroupBySerie)
		{
			m_lstAggregate[nAggregateIndex].setVolume(m_lstAggregate.at(nAggregateIndex).volume() + resultAggregate.volume());
			m_lstAggregate[nAggregateIndex].setGoodVolume(m_lstAggregate.at(nAggregateIndex).goodVolume() + resultAggregate.goodVolume());
			m_lstAggregate[nAggregateIndex].setGrossVolume(m_lstAggregate.at(nAggregateIndex).grossVolume() + resultAggregate.grossVolume());
			m_lstAggregate[nAggregateIndex].setWaferCount(m_lstAggregate.at(nAggregateIndex).waferCount() + resultAggregate.waferCount());

			setVolume(volume() + resultAggregate.volume());
			setGoodVolume(goodVolume() + resultAggregate.goodVolume());
			setGrossVolume(grossVolume() + resultAggregate.grossVolume());
			setWaferCount(waferCount() + resultAggregate.waferCount());
		}

		m_lstAggregate[nAggregateIndex].setBinVolume(m_lstAggregate.at(nAggregateIndex).binVolume() + resultAggregate.binVolume());
		setBinVolume(binVolume() + resultAggregate.binVolume());
	}

	setStartTime(qMax(startTime(), resultAggregate.startTime()));

	// If serie is a binning field, get bin information
	if (resultSerie.hasBinGroupBy() && hasBinGroupBy() == false)
		setBinGroupBy(resultSerie.binGroupBy());

	// Add aggregate value to the list
	pushAggregateValue(resultAggregate.aggregate());

	return true;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexDbPluginERDatasetGroup::clean()
//
// Description	:	Clean up group
//
///////////////////////////////////////////////////////////////////////////////////
void GexDbPluginERDatasetGroup::clean()
{
	GexDbPluginERDatasetSerie::clean();

	m_lstSerieValue.clear();
	m_lstAggregateValue.clear();
	m_lstSerie.clear();
	m_hashAggregateSerie.clear();
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	const GexDbPluginERDatasetSerie& GexDbPluginERDatasetGroup::worstSerie() const
//
// Description	:	Retrieve the worst serie based on the yield
//
///////////////////////////////////////////////////////////////////////////////////
const GexDbPluginERDatasetSerie&  GexDbPluginERDatasetGroup::worstSerie() const
{
	QList<GexDbPluginERDatasetSerie>::const_iterator	itWorstSerie	= m_lstSerie.begin();
	QList<GexDbPluginERDatasetSerie>::const_iterator	itSerie			= m_lstSerie.begin();
	QString												strWorstBinCat;
	QString												strCurrentBinCat;

	while (++itSerie != m_lstSerie.end())
	{
		if ((*itSerie).serie() != "Overall")
		{
			strWorstBinCat		= (*itWorstSerie).field("bin_cat").value().toString();
			strCurrentBinCat	= (*itSerie).field("bin_cat").value().toString();

			if ((*itWorstSerie).serie() == "Overall")
				itWorstSerie = itSerie;
			else if ((*itSerie).binGroupBy() == GexDbPluginERDatasetRow::BinGroupBySerie)
			{
				if (strCurrentBinCat == "F" && strWorstBinCat == "F")
				{
					if ((*itSerie).binYield() > (*itWorstSerie).binYield())
						itWorstSerie = itSerie;
				}
				else if (strCurrentBinCat == "P" && strWorstBinCat == "P")
				{
					if ((*itSerie).binYield() < (*itWorstSerie).binYield())
						itWorstSerie = itSerie;
				}
				else if (strCurrentBinCat == "F" && strWorstBinCat == "P")
					itWorstSerie = itSerie;
			}
			else
			{
				if ((*itSerie).yield() < (*itWorstSerie).yield())
                    itWorstSerie = itSerie;
			}
		}
	}

	return (*itWorstSerie);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	const GexDbPluginERDatasetSerie& GexDbPluginERDatasetGroup::bestSerie() const
//
// Description	:	Retrieve the best serie based on the yield
//
///////////////////////////////////////////////////////////////////////////////////
const GexDbPluginERDatasetSerie& GexDbPluginERDatasetGroup::bestSerie() const
{
	QList<GexDbPluginERDatasetSerie>::const_iterator	itBestSerie		= m_lstSerie.begin();
	QList<GexDbPluginERDatasetSerie>::const_iterator	itSerie			= m_lstSerie.begin();
	QString												strBestBinCat;
	QString												strCurrentBinCat;

	while (++itSerie != m_lstSerie.end())
	{
		if ((*itSerie).serie() != "Overall")
		{
			strBestBinCat		= (*itBestSerie).field("bin_cat").value().toString();
			strCurrentBinCat	= (*itSerie).field("bin_cat").value().toString();

			if ((*itBestSerie).serie() == "Overall")
				itBestSerie = itSerie;
			if ((*itSerie).binGroupBy() == GexDbPluginERDatasetRow::BinGroupBySerie)
			{
				if (strCurrentBinCat == "F" && strBestBinCat == "F")
				{
					if ((*itSerie).binYield() > (*itBestSerie).binYield())
						itBestSerie = itSerie;
				}
				else if (strCurrentBinCat == "P" && strBestBinCat == "P")
				{
					if ((*itSerie).binYield() < (*itBestSerie).binYield())
						itBestSerie = itSerie;
				}
				else if (strCurrentBinCat == "F" && strBestBinCat == "P")
					itBestSerie = itSerie;
			}
			else
			{
				if ((*itSerie).yield() < (*itBestSerie).yield())
                    itBestSerie = itSerie;
			}
		}
	}

	return (*itBestSerie);
}

bool GexDbPluginERDatasetGroup::operator==(const GexDbPluginERDatasetGroup& resultGroup) const
{
	return group() == resultGroup.group();
}

bool GexDbPluginERDatasetGroup::operator!=(const GexDbPluginERDatasetGroup& resultGroup) const
{
	return group() != resultGroup.group();
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexDbPluginERDatasetSerie
//
// Description	:	Class which defines a serie into a group
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexDbPluginERDatasetSerie::GexDbPluginERDatasetSerie() : GexDbPluginERDatasetRow()
{

}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexDbPluginERDatasetSerie::~GexDbPluginERDatasetSerie()
{

}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	int GexDbPluginERDatasetSerie::indexOfAggregate(const QString& strAggregate)
//
// Description	:	Retrieve the index of the given aggregate
//
///////////////////////////////////////////////////////////////////////////////////
int GexDbPluginERDatasetSerie::indexOfAggregate(const QString& strAggregate) const
{
	if (m_hashAggregatePosition.contains(strAggregate))
		return m_hashAggregatePosition.value(strAggregate);

	return -1;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	const GexDbPluginERDatasetRow& GexDbPluginERDatasetSerie::aggregateAt(int nIndex)
//
// Description	:	Retrieve the aggregate of the given index
//
///////////////////////////////////////////////////////////////////////////////////
const GexDbPluginERDatasetRow& GexDbPluginERDatasetSerie::aggregateAt(int nIndex) const
{
	return m_lstAggregate.at(nIndex);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool GexDbPluginERDatasetSerie::addAggregate(const GexDbPluginERDatasetRow &resultAggregate)
//
// Description	:	Add an aggregate to the serie. Return true if aggregate already exists, otherwise false.
//
///////////////////////////////////////////////////////////////////////////////////
bool GexDbPluginERDatasetSerie::addAggregate(const GexDbPluginERDatasetRow &resultAggregate)
{
	int nIndex = indexOfAggregate(resultAggregate.aggregate());

	// If serie is a binning field, get bin information
	if (resultAggregate.hasBinGroupBy() && hasBinGroupBy() == false)
	{
		setBinGroupBy(resultAggregate.binGroupBy());

		if (binGroupBy() == GexDbPluginERDatasetRow::BinGroupBySerie)
		{
			setField(resultAggregate.field("bin_no"));
			setField(resultAggregate.field("bin_cat"));
			setField(resultAggregate.field("bin_name"));
			setField(resultAggregate.field("bin_type"));
		}
	}

	if (nIndex == -1)
	{
		m_hashAggregatePosition.insert(resultAggregate.aggregate(), m_lstAggregate.count());
		m_lstAggregate.append(resultAggregate);

		// sets volume
		if (resultAggregate.binGroupBy() != GexDbPluginERDatasetRow::BinGroupByAggregate)
		{
			setVolume(volume() + resultAggregate.volume());
			setGoodVolume(goodVolume() + resultAggregate.goodVolume());
			setGrossVolume(grossVolume() + resultAggregate.grossVolume());
			setWaferCount(waferCount() + resultAggregate.waferCount());
		}
		else
		{
			setVolume(resultAggregate.volume());
			setGoodVolume(resultAggregate.goodVolume());
			setGrossVolume(resultAggregate.grossVolume());
			setWaferCount(resultAggregate.waferCount());
		}

		setBinVolume(binVolume() + resultAggregate.binVolume());
		setStartTime(qMax(startTime(), resultAggregate.startTime()));

		return false;
	}

	m_lstAggregate[nIndex].setBinVolume(m_lstAggregate[nIndex].binVolume() + resultAggregate.binVolume());
	return true;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexDbPluginERDatasetSerie::clean()
//
// Description	:	Clean up serie
//
///////////////////////////////////////////////////////////////////////////////////
void GexDbPluginERDatasetSerie::clean()
{
	GexDbPluginERDatasetRow::clean();

	m_lstAggregate.clear();
	m_hashAggregatePosition.clear();
}

bool GexDbPluginERDatasetSerie::operator==(const GexDbPluginERDatasetSerie& resultSerie) const
{
	return serie() == resultSerie.serie();
}

bool GexDbPluginERDatasetSerie::operator!=(const GexDbPluginERDatasetSerie& resultSerie) const
{
	return serie() != resultSerie.serie();
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexDbPluginERDatasetRow
//
// Description	:	Class which defines a row in the dataset
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexDbPluginERDatasetRow::GexDbPluginERDatasetRow(Type eType /*= RowValid*/) : m_eType(eType)
{
	clean();
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexDbPluginERDatasetRow::~GexDbPluginERDatasetRow()
{
}

//////////////////////////////////////////////////////	/////
// Methods
///////////////////////////////////////////////////////////
void GexDbPluginERDatasetRow::clean()
{
	m_eBinGroupBy		= BinGroupByNone;
	m_uiStartTime		= 0;
	m_uiVolume			= 0;
	m_uiBinVolume		= 0;
	m_uiGoodVolume		= 0;
	m_uiGrossVolume		= 0;
	m_uiWaferCount		= 0;
}

QString GexDbPluginERDatasetRow::group() const
{
	return field("GroupName").value().toString();
//	return m_strGroup;
}

QString GexDbPluginERDatasetRow::serie() const
{
	return field("SerieName").value().toString();
//	return m_strSerie;
}

QString GexDbPluginERDatasetRow::aggregate() const
{
	return field("AggregateName").value().toString();
//	return m_strAggregate;
}

void GexDbPluginERDatasetRow::setGroup(const QString& strGroup)
{
//	m_strGroup	= strGroup;
	setField("GroupName", QVariant(strGroup));
}

void GexDbPluginERDatasetRow::setSerie(const QString& strSerie)
{
//	m_strSerie	= strSerie;
	setField("SerieName", QVariant(strSerie));
}

void GexDbPluginERDatasetRow::setAggregate(const QString& strAggregate)
{
//	m_strAggregate	= strAggregate;
	setField("AggregateName", QVariant(strAggregate));
}

const GexDbPluginERDatasetField GexDbPluginERDatasetRow::field(const QString& strFieldName) const
{
	GexDbPluginERDatasetField						field		= GexDbPluginERDatasetField(strFieldName);
	QSet<GexDbPluginERDatasetField>::const_iterator	itField		= m_setField.find(field);

	if (itField == m_setField.end())
		return field;
	else
		return (*itField);
}

void GexDbPluginERDatasetRow::setField(const GexDbPluginERDatasetField& field)
{
	QSet<GexDbPluginERDatasetField>::iterator	itField		= m_setField.find(field);

	if (itField != m_setField.end())
		m_setField.erase(itField);

//	if (m_setField.contains(field))
//		m_setField.remove(field);

	m_setField.insert(field);
}

void GexDbPluginERDatasetRow::setField(const QString& strName, const QVariant& varValue)
{
	GexDbPluginERDatasetField field = GexDbPluginERDatasetField(strName);
	field.setValue(varValue);

	setField(field);
}

bool GexDbPluginERDatasetRow::operator<(const GexDbPluginERDatasetRow& datasetRow) const
{
	QString strField;
	QString strClause;

    for (int nKey = 0; nKey < sKeyOrder.count(); ++nKey)
	{
        strField	= sKeyOrder.at(nKey).section("|", 0, 0);
        strClause	= sKeyOrder.at(nKey).section("|", 1, 1).toLower();

		if (strField == "$PAggregate" && aggregate() != datasetRow.aggregate())
		{
			if (strClause == "desc")
				return gexCompareString(aggregate(), datasetRow.aggregate()) > 0;
			else
				return gexCompareString(aggregate(), datasetRow.aggregate()) < 0;
		}
		else if (strField == "$PSerie" && serie() != datasetRow.serie())
		{
			if (strClause == "desc")
			{
				if (serie() == "Overall")
					return true;
				else if (datasetRow.serie() == "Overall")
					return false;
				else
					return gexCompareString(serie(), datasetRow.serie()) > 0;
			}
			else
			{
				if (serie() == "Overall")
					return false;
				else if (datasetRow.serie() == "Overall")
					return true;
				else
					return gexCompareString(serie(), datasetRow.serie()) < 0;
			}
		}
		else if ((strField == "$PVolume" || strField == "volume") && m_uiVolume != datasetRow.volume())
		{
			if (strClause == "desc")
				return m_uiVolume > datasetRow.volume();
			else
				return m_uiVolume < datasetRow.volume();
		}
		else if ((strField == "$PGoodVolume" || strField == "good_volume") && m_uiGoodVolume != datasetRow.goodVolume())
		{
			if (strClause == "desc")
				return m_uiGoodVolume > datasetRow.goodVolume();
			else
				return m_uiGoodVolume < datasetRow.goodVolume();
		}
		else if ((strField == "$PBinVolume" || strField == "bin_volume") && m_uiBinVolume != datasetRow.binVolume())
		{
			if (strClause == "desc")
				return m_uiBinVolume > datasetRow.binVolume();
			else
				return m_uiBinVolume < datasetRow.binVolume();
		}
		else if ((strField == "$PGrossVolume" || strField == "gross_volume") && m_uiGrossVolume!= datasetRow.grossVolume())
		{
			if (strClause == "desc")
				return m_uiGrossVolume > datasetRow.grossVolume();
			else
				return m_uiGrossVolume < datasetRow.grossVolume();
		}
		else if ((strField == "$PYield" || strField.toLower() == "yield") && yield() != datasetRow.yield())
		{
			if (strClause == "desc")
				return yield() > datasetRow.yield();
			else
				return yield() < datasetRow.yield();
		}
		else if ((strField == "$PBinYield" || strField.toLower() == "bin_yield") && binYield() != datasetRow.binYield())
		{
			if (strClause == "desc")
				return binYield() > datasetRow.binYield();
			else
				return binYield() < datasetRow.binYield();
		}
		else if ((strField == "$PGrossYield" || strField.toLower() == "gross_yield") && grossYield() != datasetRow.grossYield())
		{
			if (strClause == "desc")
				return grossYield() > datasetRow.grossYield();
			else
				return grossYield() < datasetRow.grossYield();
		}
		else if ((strField == "$PGrossBinYield" || strField.toLower() == "gross_bin_yield") && grossBinYield() != datasetRow.grossBinYield())
		{
			if (strClause == "desc")
				return grossBinYield() > datasetRow.grossBinYield();
			else
				return grossBinYield() < datasetRow.grossBinYield();
		}
		else if (strField == "$PBinNo" && field("bin_no").value().toString() != datasetRow.field("bin_no").value().toString())
		{
			if (strClause == "desc")
				return gexCompareString(field("bin_no").value().toString(), datasetRow.field("bin_no").value().toString()) > 0;
			else
				return gexCompareString(field("bin_no").value().toString(), datasetRow.field("bin_no").value().toString()) < 0;
		}
		else if ((strField == "$PTime" || strField == "start_time") && startTime() != datasetRow.startTime())
		{
			if (strClause == "desc")
				return startTime() > datasetRow.startTime();
			else
				return startTime() < datasetRow.startTime();
		}
		else
		{
			GexDbPluginERDatasetField erField(strField);

			if (m_setField.contains(erField) && datasetRow.m_setField.contains(erField))
			{
				int nCompare = gexCompareString((*m_setField.find(erField)).value().toString(), (*datasetRow.m_setField.find(erField)).value().toString());

				if (nCompare != 0)
				{
					if (strClause == "desc")
					{
						if ((*m_setField.find(erField)).value().toString() == "Overall")
							return true;
						else if ((*datasetRow.m_setField.find(erField)).value().toString() == "Overall")
							return false;
						else
							return nCompare > 0;
					}
					else
					{
						if ((*m_setField.find(erField)).value().toString() == "Overall")
							return false;
						else if ((*datasetRow.m_setField.find(erField)).value().toString() == "Overall")
							return true;
						else
							return nCompare < 0;
					}
				}
			}
		}
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexDbPluginERDatasetSettings
//
// Description	:	Class which defines the parameters for a dataset
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexDbPluginERDatasetSettings::GexDbPluginERDatasetSettings()
{
	m_bFullTimeLine		= false;
	m_bOverall			= false;
	m_bExportRawData	= false;
	m_nTimePeriod		= GEX_QUERY_TIMEPERIOD_ALLDATES;
	m_eBinType			= GexDbPluginERDatasetSettings::NoBin;
	m_lTimeNFactor		= 0;
}

GexDbPluginERDatasetSettings::GexDbPluginERDatasetSettings(const GexDbPluginERDatasetSettings& datasetSettings)
{
	m_strReport			= datasetSettings.m_strReport;
	m_strName			= datasetSettings.m_strName;
	m_strDatabase		= datasetSettings.m_strDatabase;
	m_strTestingStage	= datasetSettings.m_strTestingStage;
	m_eBinType			= datasetSettings.m_eBinType;
	m_nTimePeriod		= datasetSettings.m_nTimePeriod;
	m_strTimeStep		= datasetSettings.m_strTimeStep;
	m_lTimeNFactor		= datasetSettings.m_lTimeNFactor;
	m_dtCalendarFrom	= datasetSettings.m_dtCalendarFrom;
	m_dtCalendarTo		= datasetSettings.m_dtCalendarTo;
	m_lstGroups			= datasetSettings.m_lstGroups;
	m_lstSeries			= datasetSettings.m_lstSeries;
	m_lstAggregates		= datasetSettings.m_lstAggregates;
	m_lstFilters		= datasetSettings.m_lstFilters;
	m_lstOrders			= datasetSettings.m_lstOrders;
	m_bOverall			= datasetSettings.m_bOverall;
	m_bFullTimeLine		= datasetSettings.m_bFullTimeLine;
	m_bExportRawData	= datasetSettings.m_bExportRawData;
	m_lstPostProcessing	= datasetSettings.m_lstPostProcessing;
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexDbPluginERDatasetSettings::~GexDbPluginERDatasetSettings()
{

}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	uint GexDbPluginERDatasetSettings::timestampFrom() const
//
// Description	:	Returns a timestamp from the "date from"
//
///////////////////////////////////////////////////////////////////////////////////
uint GexDbPluginERDatasetSettings::timestampFrom() const
{
	QDateTime dtFrom;

	switch (m_nTimePeriod)
	{
		case GEX_QUERY_TIMEPERIOD_TODAY			:	dtFrom.setDate(QDate::currentDate());
													dtFrom.setTime(QTime());
													break;

		case GEX_QUERY_TIMEPERIOD_LAST2DAYS		:	dtFrom.setDate(QDate::currentDate().addDays(-1));
													dtFrom.setTime(QTime());
													break;

		case GEX_QUERY_TIMEPERIOD_LAST3DAYS		:	dtFrom.setDate(QDate::currentDate().addDays(-2));
													dtFrom.setTime(QTime());
													break;

		case GEX_QUERY_TIMEPERIOD_LAST7DAYS		:	dtFrom.setDate(QDate::currentDate().addDays(-6));
													dtFrom.setTime(QTime());
													break;

		case GEX_QUERY_TIMEPERIOD_LAST14DAYS	:	dtFrom.setDate(QDate::currentDate().addDays(-13));
													dtFrom.setTime(QTime());
													break;

		case GEX_QUERY_TIMEPERIOD_LAST31DAYS	:	dtFrom.setDate(QDate::currentDate().addMonths(-1));
													dtFrom.setTime(QTime());
													break;

		case GEX_QUERY_TIMEPERIOD_THISWEEK		:	dtFrom.setDate(QDate::currentDate().addDays(1-QDate::currentDate().dayOfWeek()));
													dtFrom.setTime(QTime());
													break;

		case GEX_QUERY_TIMEPERIOD_THISMONTH		:	dtFrom.setDate(QDate::currentDate().addDays(1-QDate::currentDate().day()));
													dtFrom.setTime(QTime());
													break;

		case GEX_QUERY_TIMEPERIOD_LAST_N_X		:
		case GEX_QUERY_TIMEPERIOD_CALENDAR		:	dtFrom.setDate(m_dtCalendarFrom);
													dtFrom.setTime(QTime());
													break;

		case GEX_QUERY_TIMEPERIOD_ALLDATES		:	dtFrom.setTime_t(0);
		default									:	break;
	}

	return dtFrom.toTime_t();
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	uint GexDbPluginERDatasetSettings::timestampTo() const
//
// Description	:	Returns a timestamp from the "date to"
//
///////////////////////////////////////////////////////////////////////////////////
uint GexDbPluginERDatasetSettings::timestampTo() const
{
	QDateTime dtTo;

	switch (m_nTimePeriod)
	{
		case GEX_QUERY_TIMEPERIOD_CALENDAR		:	dtTo.setDate(m_dtCalendarTo);
													dtTo.setTime(QTime(23, 59, 59, 999));
													break;

		case GEX_QUERY_TIMEPERIOD_ALLDATES		:
		case GEX_QUERY_TIMEPERIOD_TODAY			:
		case GEX_QUERY_TIMEPERIOD_LAST2DAYS		:
		case GEX_QUERY_TIMEPERIOD_LAST3DAYS		:
		case GEX_QUERY_TIMEPERIOD_LAST7DAYS		:
		case GEX_QUERY_TIMEPERIOD_LAST14DAYS	:
		case GEX_QUERY_TIMEPERIOD_LAST31DAYS	:
		case GEX_QUERY_TIMEPERIOD_THISWEEK		:
		case GEX_QUERY_TIMEPERIOD_THISMONTH		:
		case GEX_QUERY_TIMEPERIOD_LAST_N_X		:
		default									:	dtTo.setDate(QDate::currentDate());
													dtTo.setTime(QTime(23, 59, 59, 999));
													break;
	}

	return dtTo.toTime_t();
}

void GexDbPluginERDatasetSettings::setDates(const QString& strTimeSteps, long lTimeFactor)
{
    m_dtCalendarTo = QDate::currentDate();

	if (strTimeSteps == "years")
	{
        m_dtCalendarFrom = m_dtCalendarTo.addYears(-lTimeFactor);
	}
	else if (strTimeSteps == "quarters")
	{
        m_dtCalendarFrom = m_dtCalendarTo.addMonths(-lTimeFactor * 3);
	}
	else if (strTimeSteps == "months")
	{
        m_dtCalendarFrom = m_dtCalendarTo.addMonths(-lTimeFactor);
	}
	else if (strTimeSteps == "weeks")
        m_dtCalendarFrom = m_dtCalendarTo.addDays(-lTimeFactor * 7);
	else if (strTimeSteps == "days")
        m_dtCalendarFrom = m_dtCalendarTo.addDays(-lTimeFactor);

	m_strTimeStep	= strTimeSteps;
	m_lTimeNFactor	= lTimeFactor;
}

void GexDbPluginERDatasetSettings::addFilter(const QString& strField, const QString& strValue,
                                             GexDbPluginERDatasetSettings::FilterOperator eOperator /*= GexDbPluginERDatasetSettings::OpEqual*/,
                                             QString strAdditionalData)
{
    m_lstFilters.append(GexDbPluginERDatasetSettings::GexDbPluginERDatasetPrivateFilter(strField, strValue, eOperator,strAdditionalData));
}

bool GexDbPluginERDatasetSettings::isSerieField(const QString& strField) const
{
	bool bIsSerie = false;

	for (int nSerie = 0; nSerie < m_lstSeries.count() && bIsSerie == false; ++nSerie)
	{
		if (m_lstSeries.at(nSerie) == strField)
			bIsSerie = true;
	}

	return bIsSerie;
}

bool GexDbPluginERDatasetSettings::isGroupField(const QString& strField) const
{
    bool bIsGroup = false;

    for (int nGroup = 0; nGroup < m_lstGroups.count() && bIsGroup == false; ++nGroup)
    {
        if (m_lstGroups.at(nGroup) == strField)
            bIsGroup = true;
    }

    return bIsGroup;
}

bool GexDbPluginERDatasetSettings::isFilteredField(const QString& strField, QString& strToString) const
{
	bool bIsFiltered = false;

	for (int nFilter = 0; nFilter < m_lstFilters.count() && bIsFiltered == false; ++nFilter)
	{
		if (m_lstFilters.at(nFilter).field() == strField)
		{
			bIsFiltered = true;
			strToString = m_lstFilters.at(nFilter).toString();
		}
	}

    return bIsFiltered;
}

bool GexDbPluginERDatasetSettings::isOrderedField(const QString& strField, QString& orderDirection) const
{
    QStringList lOrderDetail;

    for (int nOrder = 0; nOrder < m_lstOrders.count(); ++nOrder)
    {
        lOrderDetail = m_lstOrders.at(nOrder).split('|');

        // Check if the field matches
        if (lOrderDetail[0] == strField)
        {
            if (lOrderDetail.count() > 1)
            {
                orderDirection = lOrderDetail[1];
            }
            else
            {
                orderDirection.clear();
            }

            return true;
        }
    }

    return false;
}

bool GexDbPluginERDatasetSettings::useIntermediateConsolidatedData() const
{
    bool use = false;

    QString lField;

    for(int idx = 0; idx < m_lstFilters.count(); ++idx)
    {
        lField = m_lstFilters.at(idx).field();

        if (lField.compare(GEXDB_PLUGIN_DBFIELD_CONSOLIDATION_NAME, Qt::CaseInsensitive) == 0)
            use = true;
    }

    for(int idx = 0; idx < m_lstAggregates.count() && !use; ++idx)
    {
        lField = m_lstAggregates.at(idx);

        if (lField.compare(GEXDB_PLUGIN_DBFIELD_CONSOLIDATION_NAME, Qt::CaseInsensitive) == 0)
            use = true;
    }

    for(int idx = 0; idx < m_lstSeries.count() && !use; ++idx)
    {
        lField = m_lstSeries.at(idx);

        if (lField.compare(GEXDB_PLUGIN_DBFIELD_CONSOLIDATION_NAME, Qt::CaseInsensitive) == 0)
            use = true;
    }

    for(int idx = 0; idx < m_lstGroups.count() && !use; ++idx)
    {
        lField = m_lstGroups.at(idx);

        if (lField.compare(GEXDB_PLUGIN_DBFIELD_CONSOLIDATION_NAME, Qt::CaseInsensitive) == 0)
            use = true;
    }

    return use;
}

bool GexDbPluginERDatasetSettings::isEqual(const GexDbPluginERDatasetSettings& datasetSettings) const
{
	if (m_eBinType != datasetSettings.m_eBinType)
		return false;

	if (m_bFullTimeLine != datasetSettings.m_bFullTimeLine)
		return false;

	if (m_bOverall != datasetSettings.m_bOverall)
		return false;

	if (m_lstGroups != datasetSettings.m_lstGroups)
		return false;

	if (m_lstSeries != datasetSettings.m_lstSeries)
		return false;

	if (m_lstAggregates != datasetSettings.m_lstAggregates)
		return false;

	if (m_nTimePeriod != datasetSettings.m_nTimePeriod)
		return false;

	if (m_nTimePeriod == GEX_QUERY_TIMEPERIOD_CALENDAR)
	{
		if (m_dtCalendarFrom != datasetSettings.m_dtCalendarFrom)
			return false;

		if (m_dtCalendarTo != datasetSettings.m_dtCalendarTo)
			return false;
	}

	if (m_strDatabase != datasetSettings.m_strDatabase)
		return false;

	if (m_strTestingStage != datasetSettings.m_strTestingStage)
		return false;

	if (m_lstFilters != datasetSettings.m_lstFilters)
		return false;

    return true;
}

void GexDbPluginERDatasetSettings::checkForIntermediateConsolidationSplit()
{
    if (m_strTestingStage == "Wafer Sort" || m_strTestingStage == "Final Test")
    {
        bool    bHasSplit       = false;
        bool    lMustBeSplitted = false;
        QString strField;
        QString strConsolidationName = GEXDB_PLUGIN_DBFIELD_CONSOLIDATION_NAME;

        for(int idx = 0; idx < m_lstFilters.count(); ++idx)
        {
            strField = m_lstFilters.at(idx).field();

            if (strField.compare(strConsolidationName, Qt::CaseInsensitive) == 0)
                lMustBeSplitted = true;
        }

        for(int idx = 0; idx < m_lstAggregates.count(); ++idx)
        {
            strField = m_lstAggregates.at(idx);

            if (strField.compare(strConsolidationName, Qt::CaseInsensitive) == 0)
                lMustBeSplitted = true;
        }

        for(int idx = 0; idx < m_lstSeries.count() && !bHasSplit; ++idx)
        {
            strField = m_lstSeries.at(idx);

            if (strField.compare(strConsolidationName, Qt::CaseInsensitive) == 0)
                lMustBeSplitted = true;
        }

        for(int idx = 0; idx < m_lstGroups.count() && !bHasSplit; ++idx)
        {
            strField = m_lstGroups.at(idx);

            if (strField.compare(strConsolidationName, Qt::CaseInsensitive) == 0)
                bHasSplit = true;
        }

        if (bHasSplit == false && lMustBeSplitted)
            m_lstGroups.prepend(GEXDB_PLUGIN_DBFIELD_CONSOLIDATION_NAME);
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexDbPluginERDatasetSettings
//
// Description	:	Class which defines the parameters for a dataset
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexDbPluginERDatasetSettings::GexDbPluginERDatasetPrivateFilter::GexDbPluginERDatasetPrivateFilter(const QString& strField, const QString& strValue, FilterOperator eOperator /*= OpEqual*/, QString strAdditional_data)
{
	m_strField	= strField;
	m_strValue	= strValue;
	m_eOperator	= eOperator;
    m_strAdditional_data = strAdditional_data;
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexDbPluginERDatasetSettings::GexDbPluginERDatasetPrivateFilter::~GexDbPluginERDatasetPrivateFilter()
{

}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////
QString GexDbPluginERDatasetSettings::GexDbPluginERDatasetPrivateFilter::toString() const
{
	QString		strString	= field();
	QStringList lstValue	= value().split("|");
    QStringList lstAddationalData ;
    if(!AdditionalData().isEmpty())
        lstAddationalData = AdditionalData().split("|");


	if (filterOperator() == GexDbPluginERDatasetSettings::OpNotEqual)
	{
		strString += " &ne; ";

		for (int nValue = 0; nValue < lstValue.count(); ++nValue)
		{
            strString += lstValue.at(nValue);
            if(!lstAddationalData.isEmpty())
                strString += QString(" (%1) ").arg(lstAddationalData.at(nValue));

			if (nValue < lstValue.count() - 1)
				strString += " and ";
		}
	}
	else if (filterOperator() == GexDbPluginERDatasetSettings::OpEqual)
	{
		strString += " = ";

		for (int nValue = 0; nValue < lstValue.count(); ++nValue)
		{
			strString += lstValue.at(nValue);
            if(!lstAddationalData.isEmpty())
                strString += QString(" (%1) ").arg(lstAddationalData.at(nValue));

			if (nValue < lstValue.count() - 1)
				strString += " or ";
		}
	}

	return strString;
}

bool GexDbPluginERDatasetSettings::GexDbPluginERDatasetPrivateFilter::operator==(const GexDbPluginERDatasetPrivateFilter& privateFilter) const
{
	if (m_eOperator != privateFilter.m_eOperator)
		return false;

	if (m_strField != privateFilter.m_strField)
		return false;

	if (m_strValue != privateFilter.m_strValue)
		return false;

	return true;
}
