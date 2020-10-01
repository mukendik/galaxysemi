#ifndef PICKPART_DIALOG_H
#define PICKPART_DIALOG_H

#include "ui_pickpart_dialog.h"

/////////////////////////////////////////////////////////////////////////////
class PartInfo
{
public:
	// PartInfo(long ldRunID, QString strPartID, QString strCustomPartID, QString strDieXY, int nSoftBin, int nHardBin, int nSiteNb);		case 3935
	PartInfo(long ldRunID, QString strPartID, QString strCustomPartID, QString strDieXY, int nSoftBin, int nHardBin, int nSiteNb, QString strWaferId);
	long		m_ldRunID;
	QString		m_strPartID;
	QString		m_strCustomPartID;
	QString		m_strDieXY;
	int			m_nSoftBin;
	int			m_nHardBin;
	int			m_nSiteNb;
	QString		m_strWaferId;		// case 3935
};

/////////////////////////////////////////////////////////////////////////////
class PickPartDialog : public QDialog, public Ui::PickPartDialogBase
{
	Q_OBJECT

public:

	PickPartDialog( QWidget* parent = 0, bool modal = false, Qt::WindowFlags f = 0 );
	~PickPartDialog();

	bool				setFile(QString strFile);
	bool				setGroupId(int iGroupID);
	bool				setGroupAndFileID(int nGroupID, int nFileID);

	QString				getPartsList(int iRawId=0,char cSeparator=',');
	QString				GetCurrentWaferId() const;

	bool				m_bFoundCustomPartIDs;		// Set to true if custom partIDs were found in the file

protected:

	enum findPart
	{
		usePartID = 0,
		useCustomPartID,
		useXY
	};

private:

	static findPart		m_eFindPartMode;

public slots:

	void				onFindPart();
	void				OnWaferIdSelectionChanged(QString strSelectedWaferId);//int nWaferIndexInCombo);

protected slots:

	void				onChangeFindPartMode(int nIndex);
};

class PickPartTreeWidgetItem : public QTreeWidgetItem
{

public:

	PickPartTreeWidgetItem(QTreeWidget * pTreeWidget);
	virtual ~PickPartTreeWidgetItem();

	bool operator< ( const QTreeWidgetItem & other ) const;

protected:

	bool	sortInteger(const QString& strLeft, const QString& strRight) const;
	bool	sortPartID(const QString& strLeft, const QString& strRight) const;
	bool	sortCoordinates(const QString& strLeft, const QString& strRight) const;
};

#endif // PICKPART_DIALOG_H
