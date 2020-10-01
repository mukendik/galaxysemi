#ifndef EXPORT_ASCII_INPUT_FILELIST_H
#define EXPORT_ASCII_INPUT_FILELIST_H

#include <QList>
#include <QString>

#include "export_ascii_input_file.h"

namespace GS
{
namespace Gex
{


class ExportAscii_InputFileList : public QList<ExportAscii_InputFile*>
{
public:
    ExportAscii_InputFileList();
    ~ExportAscii_InputFileList();

    void					clear();
    ExportAscii_InputFile*	Append(const QString & strFullFileName, const QString & strOutputDir = "");
    ExportAscii_FileInfo*	FirstStdf(ExportAscii_FileInfo::FileTypes eFileType);
    ExportAscii_FileInfo*	NextStdf(ExportAscii_FileInfo::FileTypes eFileType);
    //! \brief Return the current input file
    ExportAscii_InputFile*	GetCurrentInputFile();
    void Repaired(ExportAscii_FileInfo *pclFileInfo);
    //! \brief Set the current input file
    void					SetCurrentInputFile(ExportAscii_InputFile *pclInputFile);

    unsigned int			m_uiNbStdfFiles_V4;
    unsigned int			m_uiNbStdfFiles_V4_Repairable;
    unsigned int			m_uiNbFiles;
    unsigned int			m_uiNbFiles_Converted;

private:
    QList<ExportAscii_InputFile*>::iterator	m_itCurrentInputFile;
    QList<ExportAscii_FileInfo*>::iterator	m_itCurrentExtractedFile;
};
}
}

#endif // EXPORT_ASCII_INPUT_FILELIST_H
