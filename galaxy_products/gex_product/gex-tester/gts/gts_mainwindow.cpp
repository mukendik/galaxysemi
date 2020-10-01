///////////////////////////////////////////////////////////////////////////////////////
// Implementation of Gts_MainWindow
///////////////////////////////////////////////////////////////////////////////////////

#include <qlistview.h>
#include <qprocess.h>
#include <qtimer.h>

#include "gts_mainwindow.h"


///////////////////////////////////////////////////////////
// Constructor / Destructor
///////////////////////////////////////////////////////////
Gts_StationInfo::Gts_StationInfo(unsigned int uiStationNb, QProcess *pStationProcess, QListViewItem *pListViewItem)
{
	m_uiStationNb = uiStationNb;
	m_pStationProcess = pStationProcess;
	m_pListViewItem = pListViewItem;
}

Gts_StationInfo::~Gts_StationInfo()
{
}

///////////////////////////////////////////////////////////
// Constructor / Destructor
///////////////////////////////////////////////////////////
Gts_MainWindow::Gts_MainWindow( QWidget* parent, const char* name, WFlags fl )
    : Gts_MainWindow_base( parent, name, fl )
{
	// Set Caption: application full name
	m_strApplicationName = "GTS - Galaxy Tester Simulator (V1.0 B1)";
	setCaption(m_strApplicationName);
}

Gts_MainWindow::~Gts_MainWindow()
{
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : New station requested
//
// Argument(s) :
//
// Return type : none
/////////////////////////////////////////////////////////////////////////////////////////
void Gts_MainWindow::OnNewStation()
{
	unsigned int uiStationNb = m_listStations.GetFirstAvailableStationNb();

	// Launch a station process
	QStringList	listCommand;
	listCommand.append("S:\\gex_product\\gex-tester\\gts-station\\release\\gts-station.exe");
	listCommand.append(QString::number(uiStationNb));
	QProcess *pStationProcess = new QProcess(listCommand, this);
	if(pStationProcess->start() == FALSE)
	{
		delete pStationProcess;
		return;
	}

	// Create new item in list view
    QListViewItem *pItem = new QListViewItem(listViewStations, 0);
    pItem->setText(0, QString::number(uiStationNb));
    pItem->setText(2, "0");
    pItem->setText(3, "0%");

	// Create new station object
	Gts_StationInfo *pStation = new Gts_StationInfo(uiStationNb, pStationProcess, pItem);

	// Add new station to the list
	m_listStations.append(pStation);

	// Connect signals
	connect(pStationProcess, SIGNAL(processExited()), this, SLOT(OnCloseStation()));
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Called when application is about to exit
//
// Argument(s) :
//
// Return type : none
/////////////////////////////////////////////////////////////////////////////////////////
void Gts_MainWindow::OnExit()
{
	Gts_StationInfo		*pStation;
	QListViewItem		*pItem;
	QProcess			*pStationProcess;

	// Remove all stations from list view
	pStation = m_listStations.first();
	while(pStation)
	{
		// Remove item from list view
		pItem = pStation->m_pListViewItem;
		listViewStations->takeItem(pItem);
		delete pItem;

		// Kill station process
		pStationProcess = pStation->m_pStationProcess;
		pStationProcess->tryTerminate();
		QTimer::singleShot(5000, pStationProcess, SLOT(kill()));
		
		pStation = m_listStations.next();
	}

	// Clear station list (will delete each individual station as auto-delete is set)
	m_listStations.clear();

	// Close Window
	close();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : About button clicked
//
// Argument(s) :
//
// Return type : none
/////////////////////////////////////////////////////////////////////////////////////////
void Gts_MainWindow::OnAbout()
{
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Called when a station has closed
//
// Argument(s) :
//
// Return type : none
/////////////////////////////////////////////////////////////////////////////////////////
void Gts_MainWindow::OnCloseStation()
{
	Gts_StationInfo		*pStation;
	QListViewItem		*pItem;
	QProcess			*pStationProcess;

	// Check wich process has terminated, and update list view
	pStation = m_listStations.first();
	while(pStation)
	{
		// Check station process
		pStationProcess = pStation->m_pStationProcess;
		if(pStationProcess->isRunning() == FALSE)
		{
			// Remove item from list view
			pItem = pStation->m_pListViewItem;
			listViewStations->takeItem(pItem);
			delete pItem;

			// Delete process object
			delete pStationProcess;

			// Remove station from list of stations
			m_listStations.remove(pStation);
			pStation = m_listStations.current();
		}
		else
			pStation = m_listStations.next();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Compares 2 items of the station list
//
// Argument(s) :
//
//	QCollection::Item Item1
//		First item
//
//	QCollection::Item Item2
//		Second item
//
// Return type : 0 if item1==item2, >0 if item1>item2, <0 if item1<item2 
/////////////////////////////////////////////////////////////////////////////////////////
int Gts_StationList::compareItems(QCollection::Item Item1, QCollection::Item Item2)
{
	unsigned int uiSationNb_1 = ((Gts_StationInfo *)Item1)->m_uiStationNb;
	unsigned int uiSationNb_2 = ((Gts_StationInfo *)Item2)->m_uiStationNb;

	if(uiSationNb_1 > uiSationNb_2)
		return 1;
	if(uiSationNb_1 < uiSationNb_2)
		return -1;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Returns first available station nb
//
// Argument(s) :
//
// Return type : first available station nb
/////////////////////////////////////////////////////////////////////////////////////////
unsigned int Gts_StationList::GetFirstAvailableStationNb()
{
	unsigned int uiStationIndex;

	// Check if list is empty
	if(isEmpty())
		return 1;

	// Make sure station list is sorted
	sort();

	// Check if we have a hole in the station list
	for(uiStationIndex=0; uiStationIndex<count(); uiStationIndex++)
	{
		if(at(uiStationIndex)->m_uiStationNb != (uiStationIndex+1))
			return (uiStationIndex+1);
	}

	// Return next number
	return (uiStationIndex+1);
	
}

