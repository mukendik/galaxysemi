///////////////////////////////////////////////////////////////////////////////////////
// Deriven from gts_mainwindow_base.h
///////////////////////////////////////////////////////////////////////////////////////

#ifndef _GTS_MAINWINDOW_H
#define _GTS_MAINWINDOW_H

#include <qptrlist.h>

#include "gts_mainwindow_base.h"

class QProcess;
class QListViewItem;

class Gts_StationInfo
{
// CONSTRUCTOR/DESTRUCTOR
public:
	Gts_StationInfo(unsigned int uiStationNb, QProcess *pStationProcess, QListViewItem *pListViewItem);
	~Gts_StationInfo();

// MEMBERS
public:
	unsigned int	m_uiStationNb;
	QProcess		*m_pStationProcess;
	QListViewItem	*m_pListViewItem;
};

class Gts_StationList: public QPtrList<Gts_StationInfo>
{
// CONSTRUCTOR/DESTRUCTOR
public:
	Gts_StationList() { setAutoDelete(TRUE); }

// PUBLIC METHODS
public:
	unsigned int	GetFirstAvailableStationNb();

// PROTECTED METHODS
protected:
	int compareItems(QPtrCollection::Item item1, QPtrCollection::Item item2);

// PROTECTED DATA
protected:
};

class Gts_MainWindow : public Gts_MainWindow_base
{
public:
    Gts_MainWindow( QWidget* parent = 0, const char* name = 0, WFlags fl = WType_TopLevel );
    ~Gts_MainWindow();

private:
	QString			m_strApplicationName;	// Full application name
	Gts_StationList	m_listStations;			// list of opened stations

protected slots:
    void OnNewStation();
    void OnExit();
    void OnAbout();
    void OnCloseStation();
};

#endif // _GTS_MAINWINDOW_H
