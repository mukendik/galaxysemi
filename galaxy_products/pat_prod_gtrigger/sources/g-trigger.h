#ifndef G_TRIGGER_H
#define G_TRIGGER_H

#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QTimer>
#include <QList>
#include <QPair>
#include <QTextStream>

class QFile;

namespace GS
{
namespace GTrigger
{

enum WaferType
{
    eUnknownWafer	= 0,
    ePatWafer		= 1,
    eNoPatWafer		= 2
};

class CPromisTextFile
{
public:
    CPromisTextFile();			// constructor
    void clear();

    QString	strPromisLotID;		// Column1
    int		iWafersInSplit;		// Col2 - Total wafers in spplit-lot
    QString strFacilityID;		// Col3 - FabSite (eg: SCFAB)
    QString	strEquipmentID;		// Col4 - Equipment ID (eg: 3301400)
    QString	strDsPartNumber;	// Col5 - DS Part Number (eg: DSANBADT40BCAA6A.04)
    QString	strGeometryName;	// Col6 - Geometry (eg: ANBAD40BCAA)
    int		iGrossDie;			// Col7 - Gross die count
    float	lfDieW;				// Col8 - DieX size
    float	lfDieH;				// Col9 - DieY size
    int		iFlat;				// Col10 - Flat orientation
    QString	strSiteLocation;	// Col11 - Site Location (ie KS, SM)
};

class CInputFile
{
public:
    CInputFile(const QString & strFile);
    bool	Open();
    void	Close();
    bool	NextLine(QString & strLine);

    QString			m_strFileName;
    QFile			*m_pclFile;
    QTextStream		m_clStream;
    unsigned int	m_uiLineNb;
};

class WaferFiles
{
public:
    WaferFiles(int WaferNb, const GS::GTrigger::WaferType Type = GS::GTrigger::eUnknownWafer);
    ~WaferFiles(void);
    void            AddDataFile(const QString & FileName, const QDateTime & TimeStamp = QDateTime::currentDateTime());
    void            SetWaferType(const GS::GTrigger::WaferType Type) { mWaferType=Type; }
    void            Sort() { qSort(mDataFiles.begin(), mDataFiles.end()); }
    int             WaferNb() const { return mWaferNb; }
    QString         WaferID() const { return mWaferID; }
    GS::GTrigger::WaferType GetWaferType() const { return mWaferType; }
    void            DataFiles(QStringList & DataFiles) const;
    unsigned int    Count() const { return mDataFiles.count(); }

protected:
    // WaferNb
    int                                 mWaferNb;
    // WaferID
    QString                             mWaferID;
    // Wafer type (pat/nopat)
    WaferType                           mWaferType;
    // Data files found for this wafer
    QList< QPair<QDateTime, QString> >  mDataFiles;
};

class WaferFilesList: public QMap<int, WaferFiles>
{
public:
    void            SetProductID(QString ProductID) { mProductID=ProductID; }
    void            SetLotID(QString LotID) { mLotID=LotID; }
    void            SetPromisLotID(QString PromisLotID) { mPromisLotID=PromisLotID; }
    void            SetSublotID(QString SublotID) { mSublotID=SublotID; }
    QString         ProductID() const { return mProductID; }
    QString         LotID() const { return mLotID; }
    QString         PromisLotID() const { return mPromisLotID; }
    QString         SublotID() const { return mSublotID; }
    unsigned int    DistinctWafers(const GS::GTrigger::WaferType Type, bool IgnoreWafersWithNoFiles);
    unsigned int    DistinctWafers(bool IgnoreWafersWithNoFiles);
    unsigned int    DistinctFiles(const GS::GTrigger::WaferType Type);
    unsigned int    DistinctFiles();
protected:
    // ProductID
    QString         mProductID;
    // LotID
    QString         mLotID;
    // Lot splitID
    QString         mPromisLotID;
    // SublotID
    QString         mSublotID;
};

class WaferInfo
{
public:
    WaferInfo(void);

    void		Clear(void);

    QDateTime	cDateTime;
    long		lWaferID;
    long		iGrossDie;
    long		lTotalDies;
    long		lTotalGood_PrePAT;
    long		lTotalGood_PostPAT;

    float		lfYield_PrePAT;
    float		lfYield_PostPAT;
    float		lfDeltaYield;

    WaferType	eWaferType;
    bool		bValidWafer;

    QString		strPassFail;

    // Binnings
    QStringList	strlBinNames;
    QStringList	strlBinNumbers;
    QStringList	strlBinCounts;
};

class SummaryInfo
{
public:
    SummaryInfo(const QString & strOutFolderLog, const QString & strProductionFolder, const QString & strSplitLotID, const QString & strLotID);
    SummaryInfo(const QString & strOutFolderLog, const QString & strProductionFolder);
    ~SummaryInfo(void);

    void		Clear();
    bool		Contains(long lWaferID);
    bool		Add(WaferInfo *pWafer);
    WaferInfo	*At(long lWaferID);
    void		Remove(long lWaferID);
    bool		WriteSummaryLogFile(QString &SummaryLogFileName, const QString &AppName);
    bool        WriteWaferLogFile(const QString & AppName, long WaferID, QString & WaferLogFileName,
                                  float ValidWaferThreshold_GrossDieRatio, bool WithExt=true);
    void CreateWaferLogFileDirectory( const QString &aWaferLogFilePath ) const;

    QString		m_strOutFolderLog;
    QString		m_strProductionFolder;
    QString		m_strLotID;
    QString		m_strSplitLotID;
    QString		m_strProduct;
    QString		m_strProgram;
    QString		m_strRecipe;
    QString		m_strAlarmInfo;

    QMap<long, WaferInfo*> m_cWaferList;
};

class SummaryList: public QStringList
{
public:
    bool        GetNextSummaryFile(QString & strSummaryFile);
    QString		m_strSummaryFolder;

private:

};

} // namespace GS
} // namespace GTrigger

#endif // G_TRIGGERDIALOG_H
