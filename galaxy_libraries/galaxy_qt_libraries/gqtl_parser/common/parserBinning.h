#ifndef PARSERBINNING_H
#define PARSERBINNING_H

#include <QString>

namespace GS
{
namespace Parser
{

class ParserBinning
{
public:
    /// \brief Constructor
    ParserBinning();

    /// \brief Destructor
    ~ParserBinning();

    /// \brief Getters
    unsigned short  GetBinNumber() const;
    QString         GetBinName() const;
    bool            GetPassFail() const;
    int             GetBinCount() const;

    /// \brief Setters
    void    SetBinNumber(const unsigned short binNumber);
    void    SetBinName(const QString &binName);
    void    SetPassFail(const bool passFail);
    void    SetBinCount(const int binCount);
    void    IncrementBinCount(int aBinCount);

protected:
    unsigned short	mBinNumber;     /// \param Bin number
    QString         mBinName;       /// \param Bin name
    bool            mPassFail;		/// \param Bin cat
    int             mBinCount;		/// \param Bin count
};

}
}

#endif // PARAMETERDICTIONARY_H
