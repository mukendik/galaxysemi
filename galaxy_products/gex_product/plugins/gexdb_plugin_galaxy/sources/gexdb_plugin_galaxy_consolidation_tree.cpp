#include "gexdb_plugin_galaxy.h"
#include "consolidation_tree_replies.h"
#include "consolidation_tree.h"
#include <gqtl_log.h>

bool GexDbPlugin_Galaxy::GetConsolidationTree(QString &s, CTReplies &replies)
{
  s.clear();

  if (m_pConsolidationTree)
  {
    if (!m_pConsolidationTree->isUpToDate())
      m_pConsolidationTree->loadFromDB();

    s       = m_pConsolidationTree->xmlContent();
    replies = m_pConsolidationTree->replies();

    return m_pConsolidationTree->isValid();
  }

  return false;
 }

bool GexDbPlugin_Galaxy::ValidateConsolidationTree(const QString &xml_string, CTReplies &replies)
{
  GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("XML size =%1").arg( xml_string.size()).toLatin1().constData());

  ConsolidationTreeData CTData(this);

  return CTData.setContent(xml_string, replies);
}

bool GexDbPlugin_Galaxy::SendConsolidationTree(const QString &xml_string, CTReplies &replies)
{
  bool success = false;

  if (m_pConsolidationTree)
  {
    success = m_pConsolidationTree->update(xml_string);
    replies = m_pConsolidationTree->replies();
  }

  return success;
}
