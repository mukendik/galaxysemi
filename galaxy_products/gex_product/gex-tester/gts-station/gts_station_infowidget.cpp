#include "gts_station_infowidget.h"


/////////////////////////////////////////////////////////////////////////////////////
// Constructs a GtsStationInfowidget as a child of 'parent', with the
// name 'name' and widget flags set to 'f'.
/////////////////////////////////////////////////////////////////////////////////////
GtsStationInfowidget::GtsStationInfowidget(QWidget* parent, Qt::WindowFlags fl)
    : QWidget(parent, fl)
{
  // Setup UI
  setupUi(this);
}

GtsStationInfowidget::~GtsStationInfowidget()
{
}

void GtsStationInfowidget::Reset()
{
    treeStdfFiles->clear();
    //editLoadedStdfFile->setText("");
	editProgramName->setText("");
	editLotID->setText("");
	editSublotID->setText("");
	editWaferID->setText("");
}

/////////////////////////////////////////////////////////////////////////////////////
// Update widget data
/////////////////////////////////////////////////////////////////////////////////////
void GtsStationInfowidget::Update(const QString & StdfFileName,
                                  const GS::QtLib::DatakeysContent & KeysContent)
{
    QList<QTreeWidgetItem *> lItems = treeStdfFiles->findItems(StdfFileName, Qt::MatchFixedString);
    if(lItems.size() > 0)
        treeStdfFiles->setCurrentItem(lItems.first());
    editProgramName->setText(KeysContent.Get("ProgramName").toString());
    editLotID->setText(KeysContent.Get("Lot").toString());
    editSublotID->setText(KeysContent.Get("SubLot").toString());
}

/////////////////////////////////////////////////////////////////////////////////////
// Update widget data
/////////////////////////////////////////////////////////////////////////////////////
void GtsStationInfowidget::Update(const GtsStdfFiles & StdfFiles)
{
    GtsStdfFiles::const_iterator i = StdfFiles.constBegin();
    while (i != StdfFiles.constEnd())
    {
        QTreeWidgetItem* item = new QTreeWidgetItem(treeStdfFiles);
        item->setText(0, i.value().first);
        item->setText(1, QString("%1").arg(i.value().second.Get("RetestIndex").toInt()));
        item->setText(2, i.value().second.Get("RetestBinList").toString());
        item->setText(3, QString("%1").arg(i.value().second.Get("StartTime").toInt()));
        ++i;
    }
}
