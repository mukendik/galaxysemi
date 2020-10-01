#ifndef EXPORT_ASCII_FILE_INFO_H
#define EXPORT_ASCII_FILE_INFO_H

#include "export_ascii.h"
#include "export_atdf.h"
namespace GS
{
namespace Gex
{


class ExportAscii_FileInfo
{
public:
    enum FileTypes {
            FileType_STDF_NoFAR				= 0,
            FileType_STDF_V3				= 1,
            FileType_STDF_V4				= 2,
            FileType_STDF_V4_FAR_NotFirst	= 3,
            FileType_Compressed				= 4,
            FileType_Unknown				= 5
    };

    ExportAscii_FileInfo(const QString & strFullFileName, const QString & strOutputDir);
    ~ExportAscii_FileInfo();
    void			ExtractAll();
    bool			Convert(CSTDFtoASCII & clStdfToAscii, const QString & strAsciiFileName = "", QProgressBar* pProgressBar = NULL);
    bool			Convert(CSTDFtoATDF & clStdfToAtdf, const QString & strAsciiFileName = "", QProgressBar* pProgressBar = NULL);
    void			Repaired();

    QString			m_strFullFileName;
    QString			m_strFullFileName_Ascii;
    QString			m_strFullFileName_Repaired;
    QString			m_strFileName;
    QString			m_strFileName_Ascii;
    QString			m_strFileName_Repaired;
    unsigned int	m_uiFileSize;
    QString			m_strFileType;
    FileTypes		m_eFileType;
    int				m_nStdf_Cpu;
    int				m_nStdf_Version;
    GQTL_STDF::Stdf_Record *m_pclMIR;
    QString			m_strOutputDir;
    QString			m_strExtractionDir;
    QStringList		m_strlistExtractedFiles;
    bool			m_bFileDumped;
};

}
}
#endif // EXPORT_ASCII_FILE_INFO_H
