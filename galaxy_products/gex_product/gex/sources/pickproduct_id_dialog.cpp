#include "gex_shared.h"
#include "db_engine.h"
#include "pickproduct_id_dialog.h"
#include "db_transactions.h"
#include "gex_database_entry.h"
#include "browser_dialog.h"
#include "engine.h"

///////////////////////////////////////////////////////////
// Constructor
PickProductIdDialog::PickProductIdDialog(QWidget* parent, GexDatabaseEntry* pDatabaseEntry, bool modal, Qt::WindowFlags f )
    : QDialog( parent, f )
{
    setupUi(this);
    setModal(modal);

    // Apply Examinator palette
    GexMainwindow::applyPalette(this);

    QObject::connect(PushButtonOk,		SIGNAL(clicked()),								this, SLOT(accept()));
    QObject::connect(PushButtonCancel,	SIGNAL(clicked()),								this, SLOT(reject()));
    QObject::connect(listWidget,		SIGNAL(itemDoubleClicked(QListWidgetItem*)),	this, SLOT(accept()));

    // Empty test list
    listWidget->clear();

    QList<GexDatabaseEntry*> pDatabaseEntries;
    if(pDatabaseEntry == NULL)
    {
        // Read list of Products in each database
        GS::Gex::Engine::GetInstance().GetDatabaseEngine().LoadDatabasesListIfEmpty();

        pDatabaseEntries = GS::Gex::Engine::GetInstance().GetDatabaseEngine().mDatabaseEntries;
    }
    else
        pDatabaseEntries.append(pDatabaseEntry);

    QList<GexDatabaseEntry*>::iterator itBegin	= pDatabaseEntries.begin();
    QList<GexDatabaseEntry*>::iterator itEnd	= pDatabaseEntries.end();

    while(itBegin != itEnd)
    {

        // Build path to the relevant Index file that holds all valid Filter values
        // Path= <Database name>/filters/<filter_name_ID>
        QString strIndexFile = (*itBegin)->PhysicalPath() + GEX_DATABASE_FILTER_FOLDER;
        strIndexFile += gexFilterChoices[GEX_QUERY_FILTER_PRODUCT];

        // Read Filter table file, so to fill list.
        QFile f(strIndexFile);
        if(f.open(QIODevice::ReadOnly))
        {
            // Filter table file exists...read it to see if matching entry found.
            QString strString;
            QTextStream hFilterTableFile(&f);	// Assign file handle to data stream

            // Read all lines, fill list box.
            do
            {
                strString = hFilterTableFile.readLine();
                if((strString.isEmpty() == false) && (listWidget->findItems(strString, Qt::MatchExactly).count() == 0))
                    listWidget->addItem(strString);
            }
            while(hFilterTableFile.atEnd() == false);

            // Close file
            f.close();
        }

        // Move to next entry.
        itBegin++;
    };

    // Sort the list
    listWidget->sortItems();
}

///////////////////////////////////////////////////////////
// Returns the list of products selected
///////////////////////////////////////////////////////////
QString PickProductIdDialog::getSelection(void)
{
    QListWidgetItem *	pListWidgetItem	= 0;
    QString				strTestList		= "";
    int					iSelectionCount	= 0;

    for(int iIndex=0; iIndex < listWidget->count();iIndex++)
    {
        pListWidgetItem = listWidget->item(iIndex);

        if(pListWidgetItem && pListWidgetItem->isSelected())
        {
            // Separator between multiple selections
            if(iSelectionCount)
                strTestList += ",";

            // Save string selected
            strTestList += pListWidgetItem->text();

            // Update counter index
            iSelectionCount++;
        }
    }

    return strTestList;
}
