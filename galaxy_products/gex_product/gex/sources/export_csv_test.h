#ifndef EXPORT_CSV_TEST_H
#define EXPORT_CSV_TEST_H


#include "gex_constants.h"
#include "gstdl_type.h"
#include "multi_limit_item.h"

#include <QList>
#include <QMap>
#include <QString>

class	CFileDataInfo
{
public:

    CFileDataInfo();

    void    clear();

    time_t	m_tSetupTime;
    time_t	m_tStartTime;
    time_t	m_tEndTime;
    QString	m_strLotID;					// Dataset Lot #
    QString	m_strWaferID;				// Dataset Wafer #
    QString	m_strWaferOrientation;      // Only used to export into V1.0 format

    QChar	m_chrPosX;
    QChar	m_chrPosY;
    float	m_fWaferSize;				// Wafer diameter
    float	m_fDieWidth;				// X size
    float	m_fDieHeight;				// Y size
    QChar	m_chrWaferFlat;				// Wafer Flat orientation {U,D,L,R}
    SHORT   m_sWaferCenterX;
    SHORT   m_sWaferCenterY;
    BYTE	m_cWaferUnits;				// Units for Wafer and Die dimensions {0,1,2,3,4}
};

//////////////////////////////////////////////////////
// FTR pattern result
//////////////////////////////////////////////////////
class CFTRPatternResult
{
public:
    CFTRPatternResult();
    ~CFTRPatternResult()  {}

    bool	isValid() const				{ return m_bValid; }
    bool	result() const				{ return m_bResult; }

    void	reset()						{ m_bValid = false; m_bResult = false; }
    void	setResult(bool bSuccess);

private:

    bool	m_bValid;
    bool	m_bResult;
};

//////////////////////////////////////////////////////
// Test cell
//////////////////////////////////////////////////////
typedef QMap<BYTE, CFTRPatternResult>					tdMapSiteResult;
typedef QMap <QString, tdMapSiteResult>					tdMapPatternSiteResult;

typedef tdMapSiteResult::iterator						itMapSiteResult;
typedef QMapIterator<QString, tdMapSiteResult>			itMapPatternSiteResult;

class	CShortTest
{
public:

  CShortTest();
  ~CShortTest();
  unsigned int              lTestNumber;                // TestNumber
  int                       lPinmapIndex;               // =GEX_PTEST (-1) for standard Parametric test
                                                        // =GEX_MPTEST (-2) for parametric multi-result master test
                                                        // >=0 for MultiParametric results test, tells result pinmap index
  QMap <int,double>         lfResult;                   // Test result for a given run (and for each valid site datalogged)
  tdMapPatternSiteResult	m_qMapPatternSiteResult;	// Holds test results per site for each pattern name
  /*!
   * \var ptResultArrayIndexes
   * \brief MPR: points to the list of pinmap indexes
  */
  QList<WORD*>              mResultArrayIndexes;
  double                    lfLowLimit;                 // Test Low Limit
  double                    lfHighLimit;                // Test High Limit
  int                       res_scal;                   // Scale factor on test results
  int                       llm_scal;                   // Scale factor on LowLimit
  int                       hlm_scal;                   // Scale factor on HighLimit
  BYTE                      bLimitFlag;                 // bit0=1 (no low limit), bit1=1 (no high limit)
  BYTE                      bTestType;                  // P=Parametric,F=Functionnal,M=Multiple-result parametric.
  bool                      bTestExecuted;              // Flag telling if part was in the run flow.
  QString                   strTestName;                // Test Name
  char                      szTestUnits[GEX_UNITS];     // Test Units.
  qint32					m_nTestFlowID;				// Test flow id

  // Arrays to store displayed info...avoids to rebuild them for each report page
  char                      szTestLabel[GEX_TEST_LABEL];
  char                      szLowL[GEX_LIMIT_LABEL];
  char                      szHighL[GEX_LIMIT_LABEL];

  CShortTest *              ptNextTest;                 // Pointer to CTest structure
};

// Typdef to list of tests: used to record test flow per testing site.
typedef QList<CShortTest *> cTestList;

#endif // EXPORT_CSV_TEST_H
