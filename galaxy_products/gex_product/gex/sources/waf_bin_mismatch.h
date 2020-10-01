#ifndef WAF_BIN_MISMATCH_H
#define WAF_BIN_MISMATCH_H

#include <QObject>

//////////////////////////////////////////////////////
// Classes used for binning mismatch count
// (Wafer compare feature)
//////////////////////////////////////////////////////
class CWafBinMismatch
{
public:
    CWafBinMismatch(CBinning *pBinGroup1, CBinning *pBinGroup2)
    {
        m_pBinGroup1	= pBinGroup1;
        m_pBinGroup2	= pBinGroup2;
        m_uiCount		= 1;
    }

    bool operator<(const CWafBinMismatch& other) const
    {
        return m_uiCount < other.m_uiCount;
    }

    CBinning		*m_pBinGroup1;
    CBinning		*m_pBinGroup2;
    unsigned int	m_uiCount;

    static bool lessThan(const CWafBinMismatch * pItem1, const CWafBinMismatch * pItem2)
    {
        return (*pItem1) < (*pItem2);
    }

private:
    Q_DISABLE_COPY(CWafBinMismatch)     // please develop copy constructor and = operator before remove this macro
};

class CWafBinMismatchPareto
{
public:
    CWafBinMismatchPareto();
    ~CWafBinMismatchPareto();

    unsigned int					totalCount() const										{ return m_uiTotalCount; }
    const QList<CWafBinMismatch*>&	listWaferBinMismatch() const							{ return m_lstWafBinMismatch; }

    void							insert(CBinning* pBinGroup1, CBinning* pBinGroup2);
    void							sort(void)												{ qSort(m_lstWafBinMismatch.begin(), m_lstWafBinMismatch.end(), CWafBinMismatch::lessThan); }


private:

    unsigned int						m_uiTotalCount;				// Sum of all mismatch dies
    QHash<QString, CWafBinMismatch*>	m_hashWafBinMismatch;
    QList<CWafBinMismatch*>				m_lstWafBinMismatch;
};



#endif // WAF_BIN_MISMATCH_H
