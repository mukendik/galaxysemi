#include "export_ascii_input_filelist.h"

namespace GS
{
namespace Gex
{

///////////////////////////////////////////////////////////
// Append an element to the list
///////////////////////////////////////////////////////////
ExportAscii_InputFile* ExportAscii_InputFileList::Append(const QString & strFullFileName, const QString & strOutputDir/* = ""*/)
{
    ExportAscii_InputFile* pclInputFile = new ExportAscii_InputFile(strFullFileName, strOutputDir);
    m_uiNbStdfFiles_V4 += pclInputFile->m_uiNbStdfFiles_V4;
    m_uiNbStdfFiles_V4_Repairable += pclInputFile->m_uiNbStdfFiles_V4_Repairable;
    m_uiNbFiles += pclInputFile->m_uiNbFiles;
    append(pclInputFile);
    return pclInputFile;
}

///////////////////////////////////////////////////////////
// Update an element in the list that has been repaired
///////////////////////////////////////////////////////////
void ExportAscii_InputFileList::Repaired(ExportAscii_FileInfo *pclFileInfo)
{
    // Make sure ptr on current item is set
    if(m_itCurrentInputFile == end())
        return;

    // Decrease Repairable counters
    m_uiNbStdfFiles_V4_Repairable--;
    (*m_itCurrentInputFile)->m_uiNbStdfFiles_V4_Repairable--;

    // Update file info (sets new detected file type)
    pclFileInfo->Repaired();

    // Update counters
    switch(pclFileInfo->m_eFileType)
    {
        case ExportAscii_FileInfo::FileType_STDF_V4:
        case ExportAscii_FileInfo::FileType_STDF_V3:
            (*m_itCurrentInputFile)->m_uiNbStdfFiles_V4++;
            m_uiNbStdfFiles_V4++;
            break;

        case ExportAscii_FileInfo::FileType_STDF_V4_FAR_NotFirst:
            (*m_itCurrentInputFile)->m_uiNbStdfFiles_V4_Repairable++;
            m_uiNbStdfFiles_V4_Repairable++;
            break;

        case ExportAscii_FileInfo::FileType_STDF_NoFAR:
        case ExportAscii_FileInfo::FileType_Compressed:
        case ExportAscii_FileInfo::FileType_Unknown:
            //TODO ?
            break;
    }
}
///////////////////////////////////////////////////////////
// Return FileInfo ptr on first file of specified type
///////////////////////////////////////////////////////////
ExportAscii_FileInfo* ExportAscii_InputFileList::FirstStdf(ExportAscii_FileInfo::FileTypes eFileType)
{
    // Find first STDF file in the list
    m_itCurrentInputFile	= begin();

    while(m_itCurrentInputFile != end())
    {
        if((*m_itCurrentInputFile)->m_pclFileInfo->m_eFileType == eFileType)
            return (*m_itCurrentInputFile)->m_pclFileInfo;
        else if((*m_itCurrentInputFile)->m_pclFileInfo->m_eFileType == ExportAscii_FileInfo::FileType_Compressed)
        {
            QList<ExportAscii_FileInfo*>::iterator itEnd	= (*m_itCurrentInputFile)->m_pExtractedFiles.end();
            m_itCurrentExtractedFile = (*m_itCurrentInputFile)->m_pExtractedFiles.begin();

            while(m_itCurrentExtractedFile != itEnd)
            {
                if((*m_itCurrentExtractedFile)->m_eFileType == ExportAscii_FileInfo::FileType_STDF_V4)
                    return (*m_itCurrentExtractedFile);

                m_itCurrentExtractedFile++;
            }
        }

        m_itCurrentInputFile++;
    }

    return NULL;
}

///////////////////////////////////////////////////////////
// Return FileInfo ptr on next file of specified type
///////////////////////////////////////////////////////////
ExportAscii_FileInfo* ExportAscii_InputFileList::NextStdf(ExportAscii_FileInfo::FileTypes eFileType)
{
    // Make sure current ptr is set
    if(m_itCurrentInputFile == end())
        return NULL;

    // Check if we need to move to next input file
    if((*m_itCurrentInputFile)->m_pclFileInfo->m_eFileType == ExportAscii_FileInfo::FileType_Compressed)
    {
        QList<ExportAscii_FileInfo*>::iterator itEnd	= (*m_itCurrentInputFile)->m_pExtractedFiles.end();

        m_itCurrentExtractedFile++;

        while(m_itCurrentExtractedFile != itEnd)
        {
            if((*m_itCurrentExtractedFile)->m_eFileType == ExportAscii_FileInfo::FileType_STDF_V4)
                return (*m_itCurrentExtractedFile);

            m_itCurrentExtractedFile++;
        }
    }

    m_itCurrentInputFile++;

    // Find next STDF file
    while(m_itCurrentInputFile != end())
    {
        if((*m_itCurrentInputFile)->m_pclFileInfo->m_eFileType == eFileType)
            return (*m_itCurrentInputFile)->m_pclFileInfo;
        else if((*m_itCurrentInputFile)->m_pclFileInfo->m_eFileType == ExportAscii_FileInfo::FileType_Compressed)
        {
            QList<ExportAscii_FileInfo*>::iterator itEnd	= (*m_itCurrentInputFile)->m_pExtractedFiles.end();
            m_itCurrentExtractedFile = (*m_itCurrentInputFile)->m_pExtractedFiles.begin();

            while(m_itCurrentExtractedFile != itEnd)
            {
                if((*m_itCurrentExtractedFile)->m_eFileType == ExportAscii_FileInfo::FileType_STDF_V4)
                    return (*m_itCurrentExtractedFile);

                m_itCurrentExtractedFile++;
            }
        }

        m_itCurrentInputFile++;
    }

    return NULL;
}

ExportAscii_InputFile*	ExportAscii_InputFileList::GetCurrentInputFile()
{
    if (m_itCurrentInputFile == end())
        return NULL;
    else
        return (*m_itCurrentInputFile);
}

void ExportAscii_InputFileList::SetCurrentInputFile(ExportAscii_InputFile *pclInputFile)
{
    //m_itCurrentInputFile = find(pclInputFile); // Qt3
    // Qt4
    int i = indexOf(pclInputFile);
    m_itCurrentInputFile = (i == -1 ? end() : (begin()+i));
}

///////////////////////////////////////////////////////////
// Constructors / Destructors
// Custom Ptr List of ExportAscii_InputFile
///////////////////////////////////////////////////////////
ExportAscii_InputFileList::ExportAscii_InputFileList(): QList<ExportAscii_InputFile*>()
{
    m_itCurrentInputFile = end();
}

ExportAscii_InputFileList::~ExportAscii_InputFileList()
{
    clear();
}

void ExportAscii_InputFileList::clear()
{
    while(!isEmpty())
        delete takeFirst();

    m_itCurrentInputFile = end();

    m_uiNbStdfFiles_V4 = 0;
    m_uiNbStdfFiles_V4_Repairable = 0;
    m_uiNbFiles = 0;
    m_uiNbFiles_Converted = 0;
}


}
}
