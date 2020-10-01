#ifndef DB_GEXDATABASEQUERY_H
#define DB_GEXDATABASEQUERY_H

#include <QString>
#include <QStringList>
#include <QDate>
#include <QMap>

// Query fields
class GexDatabaseQuery
{
  static int sInstances;

public:
    static int GetNumberOfInstances() { return sInstances; }
  GexDatabaseQuery();
    GexDatabaseQuery(const GexDatabaseQuery&);

    GexDatabaseQuery& operator= (const GexDatabaseQuery&);

  ~GexDatabaseQuery();

    bool setQueryFilter(const QString &field, const QString &value);

  void			clear(void);				// Reset fields.
  bool			bLocalDatabase;				// true if Database is Local, false if on server
  bool			bHoldFileCopy;				// true if Database holds a file copy, 'false' if only holds a link.
  bool			bCompressed;				// true if insert data in .gz format.
  bool			bSummaryOnly;				// true if Database only holds the summary file
  bool			bBlackHole;					// true if NO storage to do in the Database
  bool			bExternal;					// true if External / Corporate database
  bool			bOfflineQuery;				// true if query to be executed over local cache database, not remote database!	QString	strDatabaseLogicalName;		// Database logical name
  bool			bConsolidatedExtraction;	// true if query to be executed over consolidated data or raw data

  QString			strDatabaseLogicalName;		// Database logical name
  QString			strDataTypeQuery;			// Type of data to query on (wafer sort, final test,...)
  QDate			calendarFrom;				// Filter: From date
  QDate			calendarTo;					// Filter: To date
  QTime			calendarFrom_Time;			// Filter: From time
  QTime			calendarTo_Time;			// Filter: To time
  int				iTimePeriod;				// GEX_QUERY_TIMEPERIOD_TODAY, etc...
  int				iTimeNFactor;				// use for example in Last N * X
  enum eTimeStep { DAYS, WEEKS, MONTHS, QUARTERS, YEARS  } m_eTimeStep;	// Step used in Last N * X
  long			lMinimumPartsInFile;		// Ignore any file with few samples than specified here.
  double			lfQueryFileSize;			// Amount of disk space occupied by all files matching the query.

  int				iBurninTime;
  QString			strTitle;
  QString			strDataOrigin;
  QString			strDibName;
  QString			strDibType;
  QString			strFacilityID;
  QString			strFamilyID;
  QString			strFloorID;
  QString			strFrequencyStep;
  QString			strLoadBoardName;
  QString			strLoadBoardType;
  QString			strLotID;					// Filter: LotID
  QString			strOperator;
  QString			strPackageType;
  QString			strProberType;
  QString			strProberName;
  QString			strProductID;
  QString			strJobName;
  QString			strJobRev;
  QString			strSubLotID;				// Filter: SubLotID
  QString			strTemperature;
  QString			strNodeName;
  QString			strTesterType;
  QString			strTestingCode;
  QString			strProcessID;
  QString			strWaferID;
  QString			strUser1;
  QString			strUser2;
  QString			strUser3;
  QString			strUser4;
  QString			strUser5;
  QString			strRetestNbr;				// Retest# filter: 0=first test, 1=first retest, etc. 0|1 for test + retest
  int				iProcessData;				// Data type to filter GEX_PROCESSPART_ALL, etc..
  QString			strProcessData;				// List of parts/bins to process/exclude
  int				iSite;						// = -1 for all sites, or a specific site#
  QString			strMapFile;					// Mapping file.
  quint64			uFilterFlag;				// Bits set for filter flags (eg:GEX_QUERY_FLAG_FLOOR | GEX_QUERY_FLAG_PROBERNAME)
  QString			strTestList;				// Remote DB only: define list or parameters to retrieve (allows reducing dataset size)
  QStringList		strlSqlFilters;				// Remote DB only: list of meta-data filters, with syntax <meta-data field name>=<filter value> (ie "BusinessUnit=Consumer")
  QStringList     mSplitFields;				// Fields to be used to split the dataset
    QMap<QString, QString>     mTestConditions; // Test conditions list
  QString			strOptionsString;			// Remote DB only: plugin options string
  // All others non hardcoded gexQuery(....) will be pushed in this var
  // you can push as many args as you want : "section","field","value1","value2",...
    // Examples :
    // - db_bintype : hard, soft
    // - db_extraction_mode   : last_test_instance_only, 1_out_of_N_samples N
    // - db_notch, db_waferzonesfile, db_et_notch, db_exclude_wafers_not_available_at, db_granularity...
    QList< QList<QString> > m_gexQueries;

};


#endif // DB_GEXDATABASEQUERY_H
