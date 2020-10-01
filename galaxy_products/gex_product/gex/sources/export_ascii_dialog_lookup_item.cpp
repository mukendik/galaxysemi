#include "export_ascii_dialog_lookup_item.h"

namespace GS
{
namespace Gex
{

ExportAsciiDialog_LookupItem::ExportAsciiDialog_LookupItem(QTreeWidgetItem *pclListViewItem, ExportAscii_FileInfo *pclFileInfo, ExportAscii_InputFile *pclInputFile)
{
    m_pclListViewItem = pclListViewItem;
    m_pclFileInfo = pclFileInfo;
    m_pclInputFile = pclInputFile;
}

ExportAsciiDialog_LookupItem::~ExportAsciiDialog_LookupItem()
{
}

}
}
