#ifndef AUTO_REPAIR_STDF_H
#define AUTO_REPAIR_STDF_H

///////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////
#include <QString>

///////////////////////////////////////////////////////////////////////////////////
// Class CGexAutoRepairStdf - class which try to auto repair stdf files
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
class CGexAutoRepairStdf
{
public:

	// Define the return code when auto repairing a stdf file
	enum ErrorCode
	{
		repairCancelled = 0,			// User doesn't want to repair the file
		repairFailed,					// Repair operation failed
		repairSuccessful,				// Success to repair the file
		repairUnavailable				// Unable to repair this kind of corruption
	};

	CGexAutoRepairStdf();
	~CGexAutoRepairStdf();

	ErrorCode	repairStdf(const QString& strStdfFileName, QString& strRepairedFileName);								// Do some check then repair the file

protected:

	bool		isCorruptedStdfV4(const QString& strStdfFileName, int& nCpuType) const;									// Check stdf format

	ErrorCode	repairStdf(const QString& strStdfFileName, QString& strRepairedFileName, int nOption, int nCpuType);	// Repair stdf file

};

#endif // AUTO_REPAIR_STDF_H
