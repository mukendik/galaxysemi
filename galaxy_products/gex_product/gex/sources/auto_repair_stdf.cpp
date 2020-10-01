///////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////

#include "auto_repair_stdf.h"
#include "auto_repair_dialog.h"
#include "browser_dialog.h"
#include "engine.h"
#include "stdf.h"
#include "report_build.h"
#include "temporary_files_manager.h"
#include "gex_report.h"

/*!
   \class  CGexAutoRepairStdf
   \brief The CGexAutoRepairStdf class try to auto repair stdf files
*/

///////////////////////////////////////////////////////////////////////////////////
// External objects
///////////////////////////////////////////////////////////////////////////////////
extern CGexReport*		gexReport;
extern GexMainwindow*	pGexMainWindow;

///////////////////////////////////////////////////////////////////////////////////
// External "C" functions
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CGexAutoRepairStdf::CGexAutoRepairStdf()
{

}

///////////////////////////////////////////////////////////
// Desctructor
///////////////////////////////////////////////////////////
CGexAutoRepairStdf::~CGexAutoRepairStdf()
{

}

///////////////////////////////////////////////////////////
// METHODS
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CGexAutoRepairStdf::ErrorCode repairStdf(const QString& strStdfFileName, QString& strRepairedFileName)
//
// Description	:	Check the validity of the stdf file format
//					Prompt the user to know how to handle this corrupted file then launch the repair process
//
///////////////////////////////////////////////////////////////////////////////////
CGexAutoRepairStdf::ErrorCode CGexAutoRepairStdf::repairStdf(const QString& strStdfFileName, QString& strRepairedFileName)
{
	ErrorCode	eError		= CGexAutoRepairStdf::repairUnavailable;
    int			nCpuType = 0;

	// Check if corrupted STDF V4
	if (isCorruptedStdfV4(strStdfFileName, nCpuType))
	{
		if (gexReport->autoRepair() == CGexReport::repairUndefined)
		{
			// Prompt user if we have to repair the corrupted file.
			CGexAutoRepairDialog dialogAutoRepair(pGexMainWindow, strStdfFileName);

			if (dialogAutoRepair.exec() == QDialog::Accepted)
				eError = repairStdf(strStdfFileName, strRepairedFileName, dialogAutoRepair.repairOption(), nCpuType);
			else
				eError = CGexAutoRepairStdf::repairCancelled;
		}
		else
			eError = repairStdf(strStdfFileName, strRepairedFileName, gexReport->autoRepair(), nCpuType);
	}

	return eError;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CGexAutoRepairStdf::ErrorCode repairStdf(const QString& strStdfFileName, QString& strRepairedFileName, int nOption, int nCpuType)
//
// Description	:	Repair the stdf file
//
///////////////////////////////////////////////////////////////////////////////////
CGexAutoRepairStdf::ErrorCode CGexAutoRepairStdf::repairStdf(const QString& strStdfFileName, QString& strRepairedFileName, int nOption, int nCpuType)
{
	switch (nOption)
	{
		case CGexReport::repairCreateFile	:
												{
													QFileInfo stdfInfo(strStdfFileName);
													strRepairedFileName = stdfInfo.absoluteDir().absolutePath();
													strRepairedFileName += "/gex_repaired_";
													strRepairedFileName += stdfInfo.fileName();
												}
												break;

		case CGexReport::repairReplaceFile	:	strRepairedFileName = strStdfFileName;
												break;

        case CGexReport::repairTempFile		:
            strRepairedFileName = strStdfFileName + GEX_TEMPORARY_STDF;
            GS::Gex::Engine::GetInstance().GetTempFilesManager().addFile(strRepairedFileName, TemporaryFile::BasicCheck);
        break;
	}

    GS::StdLib::Stdf stdfFile;

	if (stdfFile.RepairStdfFile(strStdfFileName.toLatin1().constData(), strRepairedFileName.toLatin1().constData(), nCpuType))
		return CGexAutoRepairStdf::repairSuccessful;
	else
		return CGexAutoRepairStdf::repairFailed;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool isCorruptedStdfV4(const QString& strStdfFileName, int& nCpuType) const
//
// Description	:	Check the file is really a V4 stdf file and it is corrupted
//
///////////////////////////////////////////////////////////////////////////////////
bool CGexAutoRepairStdf::isCorruptedStdfV4(const QString& strStdfFileName, int& nCpuType) const
{
    GS::StdLib::Stdf	stdfFile;
	bool	bValidFileStart;
	int		nStdfFormat = stdfFile.IsStdfFormat(strStdfFileName.toLatin1().constData(), &bValidFileStart);

	// Format is V4 and FAR is not found at first place.
	if(nStdfFormat == 4 && bValidFileStart == false)
	{
		// Get cpu type in order to repair file.
		nCpuType = stdfFile.GetStdfCpuType();

		// Close the Stdf File
		stdfFile.Close();

		return true;
	}

	return false;
}

