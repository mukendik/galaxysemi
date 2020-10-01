#include "export_ascii_input_file.h"

namespace GS
{
namespace Gex
{


///////////////////////////////////////////////////////////
// Constructors / Destructors
// Class containing information about a file or group of
// files (for compressed files)
///////////////////////////////////////////////////////////
ExportAscii_InputFile::ExportAscii_InputFile(const QString & strFullFileName, const QString & strOutputDir)
{
    QStringList::Iterator it;

    // Init variables
    m_uiNbStdfFiles_V4 = 0;
    m_uiNbStdfFiles_V4_Repairable = 0;
    m_uiNbFiles = 0;
    m_pclFileInfo = new ExportAscii_FileInfo(strFullFileName, strOutputDir);

    // Check compressed files
    switch(m_pclFileInfo->m_eFileType)
    {
        case ExportAscii_FileInfo::FileType_STDF_V4:
        case ExportAscii_FileInfo::FileType_STDF_V3:
            m_uiNbStdfFiles_V4 = 1;
            m_uiNbFiles = 1;
            break;

        case ExportAscii_FileInfo::FileType_STDF_V4_FAR_NotFirst:
            m_uiNbStdfFiles_V4_Repairable = 1;
            m_uiNbFiles = 1;
            break;

        case ExportAscii_FileInfo::FileType_Compressed:
            ExportAscii_FileInfo* pclFileInfo;
            m_pclFileInfo->ExtractAll();
            for(it = m_pclFileInfo->m_strlistExtractedFiles.begin(); it != m_pclFileInfo->m_strlistExtractedFiles.end(); ++it )
            {
                pclFileInfo = new ExportAscii_FileInfo(*it, strOutputDir);
                if(pclFileInfo->m_eFileType == ExportAscii_FileInfo::FileType_STDF_V4)
                    m_uiNbStdfFiles_V4++;
                else if(pclFileInfo->m_eFileType == ExportAscii_FileInfo::FileType_STDF_V4_FAR_NotFirst)
                    m_uiNbStdfFiles_V4_Repairable++;
                m_pExtractedFiles.append(pclFileInfo);
                m_uiNbFiles++;
            }
            break;

        case ExportAscii_FileInfo::FileType_Unknown:
            m_uiNbFiles = 1;
            break;

        default:
            m_uiNbFiles = 1;
            break;
    }
}

ExportAscii_InputFile::~ExportAscii_InputFile()
{
    // Free ressources
    if(m_pclFileInfo != NULL)
        delete m_pclFileInfo;
    m_pclFileInfo=0;

    while (!m_pExtractedFiles.isEmpty())
        delete m_pExtractedFiles.takeFirst();
}

}
}
