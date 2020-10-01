#ifndef TEMPORARY_FILES_MANAGER_H
#define TEMPORARY_FILES_MANAGER_H

#include <QString>
#include <QList>


#define NB_MAX_TMP_BASIC_FILES 10


///////////////////////////////////////////TEMPORARY FILE CLASS DECLARATIONS/////////////////////////////////////////////////
class TemporaryFile
{
public:
	enum removalMode
	{
		BasicCheck,		// To be remove if more than NB_MAX_TMP_BASIC_FILES in the manager
		EditedCheck,	// To be remove if file has been edited
		CloseCheck		// To be remove on close
	};

	TemporaryFile(QString strAbsName, removalMode eMode);
    ~TemporaryFile();
	void			setRemovalMode(removalMode eMode);
	void			setName(QString strAbsFileName);
	removalMode		mode()const			{return m_eRemovalMode;}
	QString			name()const			{return m_strAbsName;}
	static int	GetNumberOfInstances()	{return s_NumInstances; }
private:
	QString			m_strAbsName;		// File name with absolute path
	removalMode		m_eRemovalMode;			// Flag to know when remove this file
	static int s_NumInstances;
};


///////////////////////////////////////////TEMPORARY FILE MANAGER CLASS DECLARATIONS/////////////////////////////////////////////////
class TemporaryFilesManager
{
public:
	TemporaryFilesManager();
	TemporaryFilesManager(QList<TemporaryFile*> pTemporaryFilesList);
	~TemporaryFilesManager();
	void	addFile(TemporaryFile *pTemporaryFile);
	void	addFile(QString strAbsFileName, TemporaryFile::removalMode eMode);
	void	addList(QList<TemporaryFile*> pTemporaryFilesList);
	void	removeAll();
	void	removeAll(TemporaryFile::removalMode eMode, bool bFullErase = false);
	int		count()							{return m_temporaryFilesList.count();}
	int		count(TemporaryFile::removalMode eMode);
	bool	isLocked()		{return m_bIsLocked;}
	void	lock()			{m_bIsLocked = true;}
	void	unlock()		{m_bIsLocked = false;}

private:
	bool					m_bIsLocked;
	QList<TemporaryFile*>	m_temporaryFilesList;	// List of all temporary Files
};


#endif // TEMPORARY_FILES_MANAGER_H
