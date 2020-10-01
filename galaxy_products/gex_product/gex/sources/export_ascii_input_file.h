#ifndef EXPORT_ASCII_INPUT_FILE_H
#define EXPORT_ASCII_INPUT_FILE_H

#include <QString>
#include <QList>

#include "export_ascii_file_info.h"

namespace GS
{
namespace Gex
{

class ExportAscii_InputFile
{
public:
    ExportAscii_InputFile(const QString & strFullFileName, const QString & strOutputDir);
    ~ExportAscii_InputFile();

    ExportAscii_FileInfo				*m_pclFileInfo;
    QList<ExportAscii_FileInfo*>		m_pExtractedFiles;
    unsigned int						m_uiNbStdfFiles_V4;
    unsigned int						m_uiNbStdfFiles_V4_Repairable;
    unsigned int						m_uiNbFiles;
};

}
}
#endif // EXPORT_ASCII_INPUT_FILE_H
