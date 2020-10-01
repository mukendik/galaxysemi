#ifdef GCORE15334
#ifndef PAT_PROCESS_FT_H
#define PAT_PROCESS_FT_H

namespace GS
{
namespace Gex
{

class PATProcessFTPrivate;

class PATProcessFT
{
public:

    PATProcessFT();
    virtual ~PATProcessFT();

private:

    PATProcessFTPrivate * mPrivate;
};

}
}

#endif // PAT_PROCESS_FT_H
#endif
