#include "export_ascii_file_info.h"

#include <QDir>
#include <QFileInfo>
#include <QApplication>

#include "gqtl_sysutils.h"
#include "gqtl_archivefile.h"

namespace GS
{
namespace Gex
{
///////////////////////////////////////////////////////////
// Constructors / Destructors
// Class containing information about a single file
///////////////////////////////////////////////////////////
ExportAscii_FileInfo::ExportAscii_FileInfo(const QString & strFullFileName, const QString & strOutputDir)
{
    QDir			clDirectory;
    QFileInfo		clFileInfo;

    // Set variables
    m_strFullFileName = strFullFileName;
  CGexSystemUtils::NormalizePath(m_strFullFileName);

    clFileInfo.setFile(m_strFullFileName);
    m_strFileName = clFileInfo.fileName();
    m_strFileType = "Unknown";
    m_uiFileSize = clFileInfo.size();
    m_eFileType = FileType_Unknown;
    m_nStdf_Cpu = 1;
    m_nStdf_Version = 4;
    m_pclMIR = NULL;
    m_strOutputDir = strOutputDir;
    m_bFileDumped = false;

    // First try to open file in STDF V4/V3 parser, to check if it is a STDF V4/V3 file
    GQTL_STDF::StdfParse	stdfParser;
    if(stdfParser.Open(strFullFileName.toLatin1().constData()) == true)
    {
        if(stdfParser.LoadNextRecord(GQTL_STDF::Stdf_Record::Rec_MIR, true) == GQTL_STDF::StdfParse::NoError)
        {
            // File is a STDF V4 file
            stdfParser.GetVersion(&m_nStdf_Version);
            if(m_nStdf_Version == STDF_V_4)
            {
                m_strFileType = "STDF V4";
                m_eFileType = FileType_STDF_V4;
                m_pclMIR = new GQTL_STDF::Stdf_MIR_V4;
            }
            else
            {
                m_strFileType = "STDF V3";
                m_eFileType = FileType_STDF_V3;
                m_pclMIR = new GQTL_STDF::Stdf_MIR_V3;
            }

            stdfParser.ReadRecord(m_pclMIR);
            stdfParser.GetCpuType(&m_nStdf_Cpu);
            stdfParser.GetVersion(&m_nStdf_Version);

        }
        stdfParser.Close();
    }

    // Construct name of ascii dump
    if(m_strOutputDir.isEmpty())
    {
        m_strFileName_Ascii = m_strFileName;
        m_strFullFileName_Ascii = m_strFullFileName;
    }
    else
    {
        m_strFileName_Ascii = m_strFileName;
        m_strFullFileName_Ascii = m_strOutputDir + "/";
        m_strFullFileName_Ascii += m_strFileName_Ascii;
        CGexSystemUtils::NormalizePath(m_strFullFileName_Ascii);
    }

    // Construct name of repaired file (if possible)
    if(m_strOutputDir.isEmpty())
    {
        m_strFileName_Repaired = "gex_repaired_" + m_strFileName;
        //m_strFullFileName_Repaired = clFileInfo.dirPath(); // Qt3
        m_strFullFileName_Repaired = clFileInfo.path();
        m_strFullFileName_Repaired += "/" + m_strFileName_Repaired;
    }
    else
    {
        m_strFileName_Repaired = "gex_repaired_" + m_strFileName;
        m_strFullFileName_Repaired = m_strOutputDir + "/gex_repaired_";
        m_strFullFileName_Repaired += m_strFileName_Repaired;
    }
  CGexSystemUtils::NormalizePath(m_strFullFileName_Repaired);

    // Check if file detected as STDF V4
    if(m_eFileType != FileType_Unknown)
        return;

    // Check if STDF V3 file or corrupted STDF V4
    GS::StdLib::Stdf	clStdf;
    bool	bValidFileStart;
    int nStdfFormat = clStdf.IsStdfFormat(strFullFileName.toLatin1().constData(), &bValidFileStart);
    if(nStdfFormat == 3)
    {
        m_strFileType = "STDF V3 (not supported)";
        m_eFileType = FileType_STDF_V3;
        m_nStdf_Version = 3;
        m_nStdf_Cpu = clStdf.GetStdfCpuType();
        clStdf.Close();
        return;
    }
    else if(nStdfFormat == 4)
    {
        if(bValidFileStart)
        {
            m_strFileType = "STDF V4 (no MIR)";
            m_eFileType = FileType_STDF_V4;
            m_nStdf_Version = 4;
            m_nStdf_Cpu = clStdf.GetStdfCpuType();
            clStdf.Close();
        }
        else
        {
            m_strFileType = "STDF V4 (corrupted: try repair)";
            m_eFileType = FileType_STDF_V4_FAR_NotFirst;
            m_nStdf_Version = 4;
            m_nStdf_Cpu = clStdf.GetStdfCpuType();
            clStdf.Close();
        }
        return;
    }
    else if(nStdfFormat == 1)
    {
        m_strFileType = "STDF VX (no FAR: not supported)";
        m_eFileType = FileType_STDF_NoFAR;
        clStdf.Close();
        return;
    }

    // Create extraction directory
    if(strOutputDir.isEmpty())
        m_strExtractionDir = m_strFullFileName + "_gex_stdf_dump";
    else
        m_strExtractionDir = strOutputDir;
    if(!clDirectory.exists(m_strExtractionDir) && !clDirectory.mkdir(m_strExtractionDir))
        return;

    // Try to uncompress file, to check if it is a compressed file
    CArchiveFile	clZip(m_strExtractionDir);
    QStringList		strlistUncompressedFiles;

    // Uncompress file
    if(clZip.Uncompress(m_strFullFileName, strlistUncompressedFiles) && (strlistUncompressedFiles.count() > 0))
    {
        // File is a compressed file
        m_strFileType = "Compressed";
        m_eFileType = FileType_Compressed;
        // Add all extracted files to list of extracted files using full path
        QString strExtractedFile;
        for(QStringList::Iterator it = strlistUncompressedFiles.begin(); it != strlistUncompressedFiles.end(); ++it )
        {
            strExtractedFile = m_strExtractionDir + "/" + (*it);
            CGexSystemUtils::NormalizePath(strExtractedFile);
            m_strlistExtractedFiles.append(strExtractedFile);
        }
    }

    // Check if file detected as compressed file
    if(m_eFileType != FileType_Unknown)
        return;

    // File type not detected.
    // remove extraction directory
    clDirectory.rmdir(m_strExtractionDir);
}

ExportAscii_FileInfo::~ExportAscii_FileInfo()
{
    QDir clExtractionbDir;

    // Delete extraction directory
    if(m_eFileType == FileType_Compressed)
    {
        for(QStringList::Iterator it = m_strlistExtractedFiles.begin(); it != m_strlistExtractedFiles.end(); ++it )
            QFile::remove(*it);
        clExtractionbDir.rmdir(m_strExtractionDir);
    }

    // Free ressources
    if(m_pclMIR != NULL)
        delete m_pclMIR;
    m_pclMIR=0;
}

///////////////////////////////////////////////////////////
// File has been repaired, update variables
///////////////////////////////////////////////////////////
void ExportAscii_FileInfo::Repaired()
{
    QDir			clDirectory;
    QFileInfo		clFileInfo;

    // Set variables
    m_strFullFileName = m_strFullFileName_Repaired;
  CGexSystemUtils::NormalizePath(m_strFullFileName);

    clFileInfo.setFile(m_strFullFileName);
    m_strFileName = clFileInfo.fileName();
    m_strFileType = "Unknown";
    m_uiFileSize = clFileInfo.size();
    m_eFileType = FileType_Unknown;

    // First try to open file in STDF V4/V3 parser, to check if it is a STDF V4/V3 file
    GQTL_STDF::StdfParse	stdfParser;
    if(stdfParser.Open(m_strFullFileName.toLatin1().constData()) == true)
    {
        if(stdfParser.LoadNextRecord(GQTL_STDF::Stdf_Record::Rec_MIR, true) == GQTL_STDF::StdfParse::NoError)
        {
            // File is a STDF V4/3 file
            stdfParser.GetVersion(&m_nStdf_Version);
            if(m_nStdf_Version == STDF_V_4)
            {
                m_strFileType = "STDF V4";
                m_eFileType = FileType_STDF_V4;
                m_pclMIR = new GQTL_STDF::Stdf_MIR_V4;
            }
            else
            {
                m_strFileType = "STDF V3";
                m_eFileType = FileType_STDF_V3;
                m_pclMIR = new GQTL_STDF::Stdf_MIR_V3;
            }
            stdfParser.ReadRecord((GQTL_STDF::Stdf_Record *)m_pclMIR);
            stdfParser.GetCpuType(&m_nStdf_Cpu);
            stdfParser.GetVersion(&m_nStdf_Version);
        }
        stdfParser.Close();
    }

    // Check if file detected as STDF V4
    if(m_eFileType != FileType_Unknown)
        return;

    // Check if STDF V3 file or
    GS::StdLib::Stdf	clStdf;
    bool	bValidFileStart;
    int nStdfFormat = clStdf.IsStdfFormat(m_strFullFileName.toLatin1().constData(), &bValidFileStart);
    if(nStdfFormat == 3)
    {
        m_strFileType = "STDF V3 (not supported)";
        m_eFileType = FileType_STDF_V3;
        m_nStdf_Version = 3;
        m_nStdf_Cpu = clStdf.GetStdfCpuType();
        clStdf.Close();
        return;
    }
    else if(nStdfFormat == 4)
    {
        if(bValidFileStart)
        {
            m_strFileType = "STDF V4 (no MIR)";
            m_eFileType = FileType_STDF_V4;
            m_nStdf_Version = 4;
            m_nStdf_Cpu = clStdf.GetStdfCpuType();
            clStdf.Close();
        }
        else
        {
            m_strFileType = "STDF V4 (corrupted: try repair)";
            m_eFileType = FileType_STDF_V4_FAR_NotFirst;
            m_nStdf_Version = 4;
            m_nStdf_Cpu = clStdf.GetStdfCpuType();
            clStdf.Close();
        }
        return;
    }
    else if(nStdfFormat == 1)
    {
        m_strFileType = "STDF VX (no FAR: not supported)";
        m_eFileType = FileType_STDF_NoFAR;
        clStdf.Close();
        return;
    }
}

///////////////////////////////////////////////////////////
// Recursively extract all files, if the current file is
// a compressed file
///////////////////////////////////////////////////////////
void ExportAscii_FileInfo::ExtractAll()
{
    // Make sure file is a compressed file
    if(m_eFileType != FileType_Compressed)
        return;

    // Recursively extract all files
    bool					bFilesExtracted;
    QStringList::Iterator	it1, it2;
    CArchiveFile			clZip(m_strExtractionDir);
    QStringList				strlistUncompressedFiles;
    QString					strExtractedFile;
    do
    {
        bFilesExtracted = false;
        for(it1 = m_strlistExtractedFiles.begin(); it1 != m_strlistExtractedFiles.end(); ++it1 )
        {
            if(clZip.Uncompress(*it1, strlistUncompressedFiles) && (strlistUncompressedFiles.count() > 0))
            {
                // File is a compressed file, remove file, and entry in the list + add extracted files to the list
                QFile::remove(*it1);
                for(it2 = strlistUncompressedFiles.begin(); it2 != strlistUncompressedFiles.end(); ++it2 )
                {
                    strExtractedFile = m_strExtractionDir + "/" + (*it2);
                    CGexSystemUtils::NormalizePath(strExtractedFile);
                    m_strlistExtractedFiles.append(strExtractedFile);
                }
                //it1 = m_strlistExtractedFiles.remove(it1); // Qt3
                it1 = m_strlistExtractedFiles.erase(it1); // Qt4 ? Check me.

                bFilesExtracted = true;
            }
        }
    }
    while(bFilesExtracted);
}

///////////////////////////////////////////////////////////
// Dump STDF file to Ascii dump
///////////////////////////////////////////////////////////
bool ExportAscii_FileInfo::Convert(CSTDFtoASCII & clStdfToAscii, const QString & strAsciiFileName/* = ""*/,
                                   QProgressBar* pProgressBar /*= NULL*/)
{
    QFileInfo clFileInfo;

    // Check if custom ascii dump output filename
    if(!strAsciiFileName.isEmpty())
        m_strFullFileName_Ascii = strAsciiFileName;
    else
    {
          m_strFullFileName_Ascii = m_strFullFileName;
        // Add extension (if not already added)
        if(!m_strFullFileName_Ascii.endsWith(".txt"))
            m_strFullFileName_Ascii += ".txt";
    }
    clFileInfo.setFile(m_strFullFileName_Ascii);
    m_strFileName_Ascii = clFileInfo.fileName();

    // Convert file
    if(clStdfToAscii.Convert(m_strFullFileName,m_strFullFileName_Ascii,pProgressBar) == true)
    {
        m_bFileDumped = true;
        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////
// Dump STDF file to Atdf dump
///////////////////////////////////////////////////////////
bool ExportAscii_FileInfo::Convert(CSTDFtoATDF & clStdfToAtdf, const QString & strAsciiFileName/* = ""*/,
                                   QProgressBar* pProgressBar /*= NULL*/)
{
    QFileInfo clFileInfo;

    // Check if custom ascii dump output filename
    if(!strAsciiFileName.isEmpty())
        m_strFullFileName_Ascii = strAsciiFileName;
    else
    {
          m_strFullFileName_Ascii = m_strFullFileName;
        // Add extension (if not already added)
        if(!m_strFullFileName_Ascii.endsWith(".atd"))
            m_strFullFileName_Ascii += ".atd";
    }
    clFileInfo.setFile(m_strFullFileName_Ascii);
    m_strFileName_Ascii = clFileInfo.fileName();

    // Convert file
    if(clStdfToAtdf.Convert(m_strFullFileName,m_strFullFileName_Ascii, pProgressBar) == true)
    {
        m_bFileDumped = true;
        return true;
    }

    return false;
}

}
}
