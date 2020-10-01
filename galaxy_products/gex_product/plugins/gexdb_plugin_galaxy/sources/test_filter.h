#ifndef TEST_FILTER_H
#define TEST_FILTER_H

///////////////////////////////////////////////////////////
// GexDbPlugin_Galaxy_TestFilter class:
// Handle test list for filtering on tests at extraction
///////////////////////////////////////////////////////////
class GexDbPlugin_Galaxy_TestFilter
{
public:
  GexDbPlugin_Galaxy_TestFilter(const QString & strFullTestlist);
  QString getFullTestNblist()    const   { return m_strTestNbList_Full; }
  QString getCleanTestNblist() const     { return m_strTestNbList_Clean; }
  bool    extractPatTests() const        { return m_bExtractPatTests; }
  bool    extractAllTests() const        { return (m_strTestNbList_Full=="*"); }
  bool    isEmpty() const                { return m_strTestNbList_Full.isEmpty(); }

private:
  QString    m_strTestNbList_Full;      // Full test list specified by user, including keywords, ie "100,101,110-115" "100,pat,500-505"
  QString    m_strTestNbList_Clean;     // Cleaned test# list, having keywords removed nad range replaced, ie "100,101,110,111,112,113,114,115" "100,500,501,502,503,504,505"
  bool       m_bExtractPatTests;        // True if PAT tests should be extracted
};

#endif // TEST_FILTER_H
