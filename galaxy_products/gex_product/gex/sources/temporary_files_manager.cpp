#include <QFile>
#include <QList>


#include "temporary_files_manager.h"

///////////////////////////////////////////TEMPORARY FILE MANAGER CLASS DECLARATIONS/////////////////////////////////////////////////
///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
TemporaryFilesManager::TemporaryFilesManager()
{
	m_bIsLocked = false;
}

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
TemporaryFilesManager::TemporaryFilesManager(QList<TemporaryFile*> pTemporaryFilesList)
{
	m_bIsLocked = false;
	m_temporaryFilesList << pTemporaryFilesList;
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
TemporaryFilesManager::~TemporaryFilesManager()
{
	removeAll();
}

///////////////////////////////////////////////////////////
// Add file to tmp file list
///////////////////////////////////////////////////////////
void TemporaryFilesManager::addFile(TemporaryFile *pTemporaryFile)
{
	m_temporaryFilesList << pTemporaryFile;
}

///////////////////////////////////////////////////////////
// Add file to tmp file list
///////////////////////////////////////////////////////////
void TemporaryFilesManager::addFile(QString strAbsFileName, TemporaryFile::removalMode eMode)
{
	m_temporaryFilesList << new TemporaryFile(strAbsFileName, eMode);
}

///////////////////////////////////////////////////////////
// Add a list of file to tmp file list
///////////////////////////////////////////////////////////
void TemporaryFilesManager::addList(QList<TemporaryFile*> pTemporaryFilesList)
{
	m_temporaryFilesList << pTemporaryFilesList;
}

///////////////////////////////////////////////////////////
// Remove all files from tmp file list and delete all these
// files
///////////////////////////////////////////////////////////
void TemporaryFilesManager::removeAll()
{
	if (!isLocked())
	{
		while (!m_temporaryFilesList.isEmpty())
		{
            TemporaryFile* fileName = (m_temporaryFilesList.isEmpty())? NULL : m_temporaryFilesList.first();
            QFile(fileName->name()).remove();
			delete m_temporaryFilesList.takeFirst();
		}
	}
}

///////////////////////////////////////////////////////////
//	Count all files corresponding to the eMode
///////////////////////////////////////////////////////////
int	TemporaryFilesManager::count(TemporaryFile::removalMode eMode)
{
	int iNbFile = 0;
	QList<TemporaryFile*>::iterator it = m_temporaryFilesList.begin();
	while (it != m_temporaryFilesList.end())
	{
		if ((*it)->mode() == eMode)
			iNbFile++;
		++it;
	}
	return iNbFile;
}

///////////////////////////////////////////////////////////
// Remove all temporary files corresponding to the eMode
// if conditions are ok
///////////////////////////////////////////////////////////
void TemporaryFilesManager::removeAll(TemporaryFile::removalMode eMode, bool bFullErase/*=false*/)
{
	if (!isLocked())
	{
		QList<TemporaryFile*>::iterator it = m_temporaryFilesList.begin();
		int nbBasic = count(TemporaryFile::BasicCheck);
		bool bFileToRemove = false;
		while(it != m_temporaryFilesList.end())
		{
			if ((*it)->mode() == eMode)
			{
				bFileToRemove = true;
				switch (eMode)
				{
				case TemporaryFile::BasicCheck :
					if ((nbBasic <= NB_MAX_TMP_BASIC_FILES) && !bFullErase)
						bFileToRemove = false;
					break;
				case TemporaryFile::EditedCheck :
					break;
				case TemporaryFile::CloseCheck :
					break;
				}
				if (bFileToRemove)
				{
					QFile((*it)->name()).remove();
					delete *it;
					it = m_temporaryFilesList.erase(it);
				}
			}
			if (bFileToRemove == false)
				++it;
		}
	}
}



///////////////////////////////////////////TEMPORARY FILE CLASS DECLARATIONS/////////////////////////////////////////////////

int TemporaryFile::s_NumInstances=0;


///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
TemporaryFile::TemporaryFile(QString strAbsFileName, TemporaryFile::removalMode eMode)
{
	m_eRemovalMode	= eMode;
	m_strAbsName	= strAbsFileName;
	s_NumInstances++;
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
TemporaryFile::~TemporaryFile()
{
    s_NumInstances--;
}

///////////////////////////////////////////////////////////
// Set removal mode
///////////////////////////////////////////////////////////
void TemporaryFile::setRemovalMode(removalMode eMode)
{
	m_eRemovalMode = eMode;
}

///////////////////////////////////////////////////////////
// Set file name
///////////////////////////////////////////////////////////
void TemporaryFile::setName(QString strAbsFileName)
{
	m_strAbsName = strAbsFileName;
}

