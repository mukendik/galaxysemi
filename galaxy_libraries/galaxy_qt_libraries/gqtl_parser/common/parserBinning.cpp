#include "parserBinning.h"

namespace GS
{
namespace Parser
{

ParserBinning::ParserBinning()
{
    mBinName = "";
    mBinCount = 0;
    mPassFail = false;
}

ParserBinning::~ParserBinning()
{
}

unsigned short ParserBinning::GetBinNumber() const
{
    return mBinNumber;
}

QString ParserBinning::GetBinName() const
{
    return mBinName;
}

bool ParserBinning::GetPassFail() const
{
    return mPassFail;
}

int ParserBinning::GetBinCount() const
{
    return mBinCount;
}


void ParserBinning::SetBinNumber(const unsigned short binNumber)
{
    mBinNumber = binNumber;
}

void ParserBinning::SetBinName(const QString & binName)
{
    mBinName = binName;
}

void ParserBinning::SetPassFail(const bool passFail)
{
    mPassFail = passFail;
}

void ParserBinning::SetBinCount(const int binCount)
{
    mBinCount = binCount;
}

void ParserBinning::IncrementBinCount(int aBinCount)
{
    mBinCount += aBinCount;
}

}
}

