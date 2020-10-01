#ifndef BININFOH
#define BININFOH

#include <QString>

class BinInfo
{
public:
    BinInfo()
    {
        m_strBinName    = "?";
        m_cBinCat       = '?';
        m_nBinCount     = 0;
    }
    BinInfo(const BinInfo & source)
    {
        m_strBinName    = source.m_strBinName;
        m_cBinCat       = source.m_cBinCat;
        m_nBinCount     = source.m_nBinCount;
    }
    BinInfo & operator=(const BinInfo & source)
    {
        m_strBinName    = source.m_strBinName;
        m_cBinCat       = source.m_cBinCat;
        m_nBinCount     = source.m_nBinCount;

        return *this;
    }

    QString m_strBinName;		// BIN name
    QChar   m_cBinCat;			// BIN category ('P' or 'F')
    int     m_nBinCount;		// Nb of parts with this binning
};


#endif
