#ifdef GCORE15334


#ifndef STATION_H
#define STATION_H

#include "test_data.h"
#include <QDateTime>
#include <QHash>
#include <QMutex>

#define	GTM_STAGE_DISABLED		0		// Station is disabled because of fatal exception (eg: too many outliers, etc...).
#define	GTM_STAGE_BASELINE		1		// Station building its base line
#define	GTM_STAGE_PRODUCTION	2		// Station in production (base line already computed)

class COptionsPat;
class CPatInfo;
class CPatDefinition;
class ClientNode;

// Holds Details about the station & Lot under test
/*! \class  CStationInfo
    \brief
*/
class CStationInfo : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(CStationInfo)
    //QString			strJobName;			// Program name ? moved to QObject prop
    mutable QMutex mMutex;

public:
    static const char* sProductPropName;
    static const char* sLotPropName;
    static const char* sSublotPropName;
    static const char* sSplitlotIDPropName;
    static const char* sJobNamePropName;
    static const char* sJobRevPropName;
    static const char* sTesterNamePropName;
    static const char* sTesterTypePropName;
    static const char* sRetestIndexPropName;
    static const char* sLotStartedPropName;
    static const char* sOperatorPropName;

    CStationInfo();
    virtual ~CStationInfo() { }

    void clear(bool bAllVariables=true);	// Clears variables.

    //QString			strTesterName;		// Client Tester name. Moved to TesterNameProp
    //QString			strTesterType;		// Client Tester type
    QString			strOperator;		// Operator
    int				iStation;			// client Station#
    //QString			strJobRev;			// Program Rev.
    //QString			strProduct;			// Product name
    //QString			strLot;				// Lot name / LotID
    //QString			strSubLot;			// SubLot
    //QDateTime		tLotStarted;		// Date when lot started
};

///////////////////////////////////////////////////////////
// Holds a tests statistics info about final test outliers
/*! \class Holds outlier summary info for a given test
    \brief
*/
class COutlierSummary : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(COutlierSummary)
    static unsigned sNumOfInstances;
public:
    static unsigned GetNumOfInstances() { return sNumOfInstances; }
    COutlierSummary();	// Constructor
    virtual ~COutlierSummary() { }

    CTest *ptTestCell;	// Points to test definition entry.
    int	iBaselineFails;	// Holds total failures during baseline
    int	iProdFails;		// Holds total production failures
};

class CStation : public QObject
{
    Q_OBJECT

    // Holds Details about the station & Lot under test. Move me to private.
    CStationInfo        mStationInfo;

public:
    enum stationStatus
    {
        STATION_ENABLED,
        STATION_DISABLED
    };

    CStation(QObject* parent=0);		// Constructor
    virtual ~CStation();	// Destructor

    CPatInfo *          GetPatInfo();
    COptionsPat &       GetPatOptions();
    unsigned int        GetPatDefinitionsCount();
    QHash<QString, CPatDefinition *>& GetPatDefinitions();

    // Get PAT definition
    CPatDefinition *    GetPatDefinition(long lTestNumber, long lPinmapIndex,
                                     const QString& lTestName);
    // Session date (socket connection)
    QDateTime           SessionDate;
    // Keeps track of socket last packet exchanged.
    QDateTime           LastHandshake;

    //! \brief Thread safe get from StationInfo property names
    QVariant Get(const char*);
    // Get station nb.
    int                 GetStationNb() {return mStationInfo.iStation;}

    // Holds all test results & statistics
    GS::Gex::CTestData  cTestData;

public slots:
    stationStatus   GetStationStatus(); // 6935: get station status
    void            SetStationStatus(stationStatus lStatus);
    //! \brief Thread safe set the station info (product, lot,...) from the network message.
    // Returns 'ok' or 'error'
    QString         SetInfo(PT_GNM_Q_INIT pMsg_Q_INIT);
    QString         SetProdInfo(PT_GNM_PRODINFO);

    // Clears variables & init pointers (of PatInfo)
    void Clear();
    //! \brief Clear Station infos (product, lot,...)
    void ClearInfo(bool AllVariables);

    // Read PAT config file
    bool ReadPatRecipe(const QString &RecipeFile, QString &ErrorMessage);
    bool ReadPatRecipe(const char * RecipeBuffer, QString &ErrorMessage);

    // Compute test stats
    bool    ComputeTestStats(GS::Gex::SiteTestResults* Site);
    // Compute dynamic PAT limits
    bool    ComputeDynamicPatLimits(QList<int> & SitesToCompute, QList<int> & SitesToUse);
    bool    ComputeDynamicPatLimits(GS::Gex::SiteTestResults* Site);

private:
    Q_DISABLE_COPY(CStation)

    //! \brief Mutex in order to handle concurrent threads access
    mutable QMutex  mMutex;
    // Holds all global pat info required to create & build PAT reports (loads recipe, limits, etc...)
    CPatInfo        *mPatInfo;
    // 6935: replace iStage with iCurrentStatus (stage stored per site in site objects)
    stationStatus   mStationStatus;
};

#endif // STATION_H
#endif
