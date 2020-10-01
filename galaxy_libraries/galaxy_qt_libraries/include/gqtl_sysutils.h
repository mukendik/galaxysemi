///////////////////////////////////////////////////////////
// GEX utilities classes: header file
///////////////////////////////////////////////////////////

#ifndef GQTL_SYSTEM_UTILS_H
#define GQTL_SYSTEM_UTILS_H

// QT headers
#include <QString>
#include <QMap>
#include <QVariant>

#ifdef WIN32
    #undef CopyFile
#endif

class CGexSystemUtils : public QObject
{
    Q_OBJECT

public:
    // Error codes returned by CGexSystemUtils functions
    enum ErrorCode {NoError,		// No error
                    NotSupported	// Function is not supported on current OS platform
    };
    // Constructor / destructor functions
    CGexSystemUtils();
    ~CGexSystemUtils();

public slots:
    // should return : win32, win64, linux32, linux64, mac32, mac64, solaris32 or solaris64
    static QString GetPlatform();

    static long GetWordSize() {return QSysInfo::WordSize;}

    // Return the list of QObject classes with the current count of instance in the CoreApp if any.
    // Recursive or not : apply this function to all children is recursive true.
    static QString GetQObjectsCount(QObject &root, QMap<QString, QVariant> &lCounts, const bool lRecursive);

    // Return the current platform dynamic lib ext : dll, so, dylib,...
    static QString GetPlatformDynLibExtension();

    // Utility static functions
    static bool	CheckGexEnvironment(QString & strUserHome, QString & strApplicationDir, QString & strError);
    static int	GetUserHomeDirectory(QString& strDirectory);
    // return an CGexSystemUtils ErrorCode (enum)
    static int	GetApplicationDirectory(QString& strDirectory);

    /*
        Copy source to destination and return true or false
        On Linux, chmod target to delete it and write.
    */
    static bool CopyFile(const QString& strFrom, const QString& strTo);

    /* Get mem info (in Mo, % or Ko) for this process and OS
        Raw test will perform a raw allocation test untill no more mem available, use with precautions.
        The cross platforms keys are:
        - MemUsedByProcess : Mem in Mo used by this process
        - TotalPhysMem : total physical memory in Mo on the machine
        - AvailContinuousMem : Available continuous mem for this process
        - AvailFragmentedMem : Available fragmented mem for this process (+ or - 10Mo)
    */
    static QMap<QString, QVariant> GetMemoryInfo(const bool lRawTest, const bool detailed);

    /*
     * Get list of processes currently running on the system: key is ProcessID, value is exec name
    */
    static QMap<unsigned, QString> GetProcesses();

    // Action : Normalize the specified path.
    // return :	nothing
    // Use correct '\' or '/' separators (depending on OS)
    static void	NormalizePath(QString& strPath);
    // Use correct '\' or '/' separators (depending on OS)
    static void	NormalizePath(const QString& strPathSrc, QString& strPathDest);
    static void	SetGuiStyle();
    // Clean the specified string for use in a path (replace special characters by '_')
    static void	NormalizeString(QString& strString);
    // Clean the specified string for use in a path (replace special characters by '_')
    static void	NormalizeString(const QString& strStringSrc, QString& strStringDest);
    // Get Local time for specified time_t value. Format is: ddd MMM dd hh:mm:ss yyyy\n (ie "Wed Jan 02 02:03:55 1980\n")
    static QString	GetLocalTime(time_t tTime);
    // Get UTC time for specified time_t value. Format is: ddd MMM dd hh:mm:ss yyyy\n (ie "Wed Jan 02 02:03:55 1980\n")
    static QString	GetUtcTime(time_t tTime);
    // Get UTC time for specified time_t value. Format is given as the second argument.
    static QString	GetUtcTime(const time_t tTime, const QString & tFormat);
    // Clean the specified string for use in a file name (replace special characters by '_')
    static void	NormalizeFileName(const QString& strStringSrc, QString& strStringDest);
    // Clean the specified string for use in a file name (replace special characters by '_')
    static void	NormalizeFileName(QString& strString);

};

#endif // !defined(GQTL_SYSTEM_UTILS_H)
