#ifndef EXPORT_ASCII_DIALOG_LOOKUP_ITEM_H
#define EXPORT_ASCII_DIALOG_LOOKUP_ITEM_H

#include <QTreeWidgetItem>
#include "export_ascii_file_info.h"
#include "export_ascii_input_file.h"

namespace GS
{
namespace Gex
{


class ExportAsciiDialog_LookupItem
{
public:
    ExportAsciiDialog_LookupItem(QTreeWidgetItem *pclListViewItem, ExportAscii_FileInfo *pclFileInfo, ExportAscii_InputFile *pclInputFile);
    ~ExportAsciiDialog_LookupItem();
    QTreeWidgetItem			*m_pclListViewItem;
    ExportAscii_FileInfo	*m_pclFileInfo;
    ExportAscii_InputFile	*m_pclInputFile;
};
}
}
#endif // EXPORT_ASCII_DIALOG_LOOKUP_ITEM_H
