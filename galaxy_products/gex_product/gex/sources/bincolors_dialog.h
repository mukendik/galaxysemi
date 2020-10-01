#ifndef GEX_BINCOLORS_H
#define GEX_BINCOLORS_H

#include "ui_bincolors_dialog.h"
#include "ui_edit_bin_entry_dialog.h"
#include "report_build.h"

/////////////////////////////////////////////////////////////////////////////
// Edit a specific Bin entry color (Hard or Soft bin)
class EditBinEntryDialog : public QDialog, public Ui::EditBinEntryDialogBase
{
	Q_OBJECT

public:
	
	EditBinEntryDialog(QString strBinType="", QString strBinList="", QColor cColor=Qt::red, QWidget* parent = 0, bool modal = false, Qt::WindowFlags f = 0 );
	
	QString	GetBinList(void);	// Return Bin list entered
	QColor	GetColor(void);		// Return Bin color selected

public slots:
	
	void	OnOk();
};

/////////////////////////////////////////////////////////////////////////////
// Color management for the list of Bins (Soft & Hard) 
class BinColorsDialog : public QDialog, public Ui::BinColorsDialogBase
{
	Q_OBJECT
		
public:
	BinColorsDialog( QWidget* parent = 0, bool modal = false, Qt::WindowFlags f = 0 );

private:

    void	FillCustomBinList   (CBinning * ptBinCell);
    void	FillBinningList     (void);
    void	AddBinToList        (QString strBinList, QColor cColor,QListWidgetItem *cSelection=NULL);
    void	SaveColorList         (QListWidget * pListWidget,QList <CBinColor> &pBinList);
    void    AddToList           (QListWidget *listWidget, QList<CBinColor> &listColor, CGexGroupOfFiles *group, bool displayColorPerSite);


	QListWidget			*	m_ListWidget;
	QString					m_strBinType;	// Set to hold current Bin type edited "Soft Bin" or "Hard Bin"

public slots:
	void	OnUseDefaultColors();
    void	OnTabChange(int lIndex);
	void	OnAddBin();
	void	OnDeleteBin();
	void	OnEditBin();
	void	OnOk();
	void	OnFillWithDefaultColors();


};

#endif
