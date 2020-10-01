/******************************************************************************!
 * \file drillData3DInfo.h
 * \brief Interactive drill: 3D Data Mining
 ******************************************************************************/
#ifndef DRILL_DATA_3D_INFO_H
#define DRILL_DATA_3D_INFO_H

#include <qgl.h>
#if __MACH__
#include "OpenGL/glu.h"
#else
#include <GL/glu.h>
#endif

#include "ui_drill_3d_dialog.h"
#include "drill_3d_viewer.h"

#include <QTextBrowser>
#include <QtWidgets/QTreeWidget>

#include "font_3d.h"

#ifndef DRILL_NO_DATA_MINING
# include "vec.h"
#endif

class CTest;
class CGexGroupOfFiles;
class CGexFileInGroup;

namespace Gex {

/******************************************************************************!
 * \class DrillData3DInfo
 * \brief Drill 3D Data information
 ******************************************************************************/
class DrillData3DInfo : public QObject
{
  Q_OBJECT

public:
  /*!
   * \fn DrillData3DInfo
   */
  DrillData3DInfo(int nGroupID = 0, int nFileID = 0);
  /*!
   * \fn DrillData3DInfo
   */
  DrillData3DInfo(const DrillData3DInfo& drillDataInfo);
  /*!
   * \fn ~DrillData3DInfo
   */
  ~DrillData3DInfo();
  /*!
   * \fn Reset most of the members
  */
  void Reset();
  /*!
   * \fn groupID
   */
  int groupID() const {
    return m_nGroupID;
  }
  /*!
   * \fn fileID
   */
  int fileID() const {
    return m_nFileID;
  }


  void setTestNumber(int testNumber) { m_nTestNumber = testNumber;}

  /*!
   * \fn testNumber
   */
  int testNumber() const {
    return m_nTestNumber;
  }
  /*!
   * \fn pinmap
   */
  int pinmap() const {
    return m_nPinmap;
  }
  /*!
   * \fn dieX
   */
  int dieX() const {
    return m_nDieX;
  }
  /*!
   * \fn dieY
   */
  int dieY() const {
    return m_nDieY;
  }
  /*!
   * \fn currentTestLabel
   */
  const QString& currentTestLabel() const {
    return m_strCurrentTestLabel;
  }
  /*!
   * \fn currentTestCell
   */
  CTest* currentTestCell() const {
    return m_ptCurrentTestCell;
  }
  /*!
   * \fn group
   */
  CGexGroupOfFiles* group() const {
    return m_pGroup;
  }
  /*!
   * \fn file
   */
  CGexFileInGroup* file() const {
    return m_pFile;
  }
  /*!
   * \fn hasNextTestCell
   */
  bool hasNextTestCell() const {
    return m_ptNextTestCell;
  }
  /*!
   * \fn hasPreviousTestCell
   */
  bool hasPreviousTestCell() const {
    return m_ptPreviousTestCell;
  }
  /*!
   * \fn hasValidDieCoord
   */
  bool hasValidDieCoord() const;
  /*!
   * \fn isValid
   * \brief Return if data is valid or not
   */
  bool isValid() const {
    return (m_pGroup && m_pFile);
  }
  /*!
   * \fn isFakeTest
   * \brief Return if test is fake (eg: create by gex)
   */
  bool isFakeTest() const;
  /*!
   * \fn setDieCoord
   * \brief Sets the die coord to select
   */
  void setDieCoord(int nDieX, int nDieY);
  /*!
   * \fn loadDefaultTestCell
   * brief Initialize data with a default test
   */
  void loadDefaultTestCell();
  /*!
   * \fn loadTestCell
   * \brief Initialize data with a test
   */
  void loadTestCell(int nTestNumber, int nPinmap);
  /*!
   * \fn navigateTo
   * \brief Navigate to a specific test
   */
  void navigateTo(int nTestNumber, int nPinmap, QString strTestName);
  /*!
   * \fn changeGroupID
   * \bief Change the current Group ID and load test
   */
  void changeGroupID(int nGroupID);

  /*!
   * \fn makeExportLink
   * \brief Make a link to export waferdata
   */
  QString makeExportLink() const;
  /*!
   * \fn
   * \brief Apply test filter to the current test
   */
  void applyTestFilter();

  /*!
   * \fn operator=
   */
  DrillData3DInfo& operator=(const DrillData3DInfo& drillDataInfo);

public slots:
  /*!
   * \fn changeFileID
   * \brief Change the current file ID and load test
   */
  void changeFileID(int nFileID);
  /*!
   * \fn nextTestCell
   * \brief Goes to the next test
   */
  void nextTestCell();
  /*!
   * \fn previousTestCell
   * \brief Boes to the previous test
   */
  void previousTestCell();

private:

  /*!
   * \fn findTestCell
   * \brief Find a test
   */
  bool findTestCell(int nTestNumber, int nPinmap, QString strTestName = "");
  /*!
   * \fn previousValidTestCell
   * \brief Find previous valid test
   */
  CTest* previousValidTestCell(CTest* pCurrentCell);
  /*!
   * \fn nextValidTestCell
   * \brief Find next valid test
   */
  CTest* nextValidTestCell(CTest* pCurrentCell);

private:
  /*!
   * \var m_bFirst
   * \brief Set true if this is the first loading of a test cell
   */
  bool m_bFirst;
  /*!
   * \var m_nGroupID
   * \brief ID of the group which contains the data
   */
  int m_nGroupID;
  /*!
   * \var m_nFileID
   * \brief ID of the file which contains the data
   */
  int m_nFileID;
  /*!
   * \var m_nTestNumber
   * \brief Test number of the current test
   */
  int m_nTestNumber;
  /*!
   * \var m_nPinmap
   * \brief Pinmap number of the current test
   */
  int m_nPinmap;
  /*!
   * \var m_nDieX
   * \brief X coord of a die (-32768 if no coord provided)
   */
  int m_nDieX;
  /*!
   * \var m_nDieY
   * \brief Y coord of a die (-32768 if no coord provided)
   */
  int m_nDieY;
  /*!
   * \var m_strCurrentTestLabel
   * \brief Label of current test
   */
  QString m_strCurrentTestLabel;
  /*!
   * \var m_ptCurrentTestCell
   * \brief Handle to curretn test
   */
  CTest* m_ptCurrentTestCell;
  /*!
   * \var m_ptPreviousTestCell
   * \brief Handle to Previous test
   */
  CTest* m_ptPreviousTestCell;
  /*!
   * \var m_ptNextTestCell
   * \brief Handle to Next test
   */
  CTest* m_ptNextTestCell;
  /*!
   * \var m_pGroup
   * \brief Handle to Group at position m_iGroupID
   */
  CGexGroupOfFiles* m_pGroup;
  /*!
   * \var m_pFile
   * \brief Handle to File at position m_iGroupID
   */
  CGexFileInGroup* m_pFile;

signals:
  /*!
   * \fn dataLoaded
   * \brief Emitted when new data has been loaded
   */
  void dataLoaded(bool, bool);
  /*!
   * \fn testNavigate
   * \brief Emitted when user navigates among the test
   */
  void testNavigate();
};

}  // namespace Gex

#endif
