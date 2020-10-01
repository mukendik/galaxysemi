// -------------------------------------------------------------------------- //
// testfpu sur linux, cygwin, mingw, et sunos
// -------------------------------------------------------------------------- //
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// -------------------------------------------------------------------------- //
// byteSwap
// -------------------------------------------------------------------------- //
#include <algorithm>
#define byteSwap(x) ByteSwap((unsigned char*) &x, sizeof(x))
void ByteSwap(unsigned char* b, int n)
{
   int i = 0;
   int j = n - 1;
   while (i < j) {
      std::swap(b[i], b[j]);
      i++;
      j--;
   }
}

// -------------------------------------------------------------------------- //
// isBigEndian
// -------------------------------------------------------------------------- //
bool isBigEndian()
{
   short word = 0x4321;
   return (*(char *) &word != 0x21);
}

// -------------------------------------------------------------------------- //
// hexdump
// -------------------------------------------------------------------------- //
void hexdump(const double val)
{
    unsigned char* ptr = (unsigned char*) &val;
    unsigned int i;

    for (i = 0; i < sizeof(double); ++i) {
        printf("%02X ", *ptr);
        ++ptr;
    }
    printf(" ");
}

// -------------------------------------------------------------------------- //
// hexdump
// -------------------------------------------------------------------------- //
void hexdump(const unsigned int val)
{
    unsigned char* ptr = (unsigned char*) &val;
    unsigned int i;

    for (i = 0; i < sizeof(unsigned int); ++i) {
        printf("%02X ", *ptr);
        ++ptr;
    }
    printf(" ");
}

// -------------------------------------------------------------------------- //
// setFpu
// -------------------------------------------------------------------------- //
#ifndef SPARC
void setFpu(unsigned int mode)
{
    asm("fldcw %0" : : "m" (*&mode));
}
#else
void setFpu(unsigned int mode)
{
    uint64_t lmode = mode & 0xFF00u;

    lmode = lmode << 52;

    asm("ld %0, %%fsr" : : "m" (*&lmode));
}
#endif

// -------------------------------------------------------------------------- //
// getFpu
// -------------------------------------------------------------------------- //
#ifndef SPARC
void getFpu(unsigned int& mode)
{
    asm("fnstcw %0;" : "=m" (*&mode));
}
#else
void getFpu(unsigned int& mode)
{
    uint64_t lmode = 0;
    asm("st %%fsr,%0" : "=m" (*&lmode));

    mode = lmode >> 32;
}
#endif

// -------------------------------------------------------------------------- //
// charArrayToDouble
// -------------------------------------------------------------------------- //
double charArrayToDouble(unsigned char src0,
                         unsigned char src1,
                         unsigned char src2,
                         unsigned char src3,
                         unsigned char src4,
                         unsigned char src5,
                         unsigned char src6,
                         unsigned char src7)
{
    double val = 0;
    unsigned char* dst = (unsigned char*) &val;

    dst[0] = src0;
    dst[1] = src1;
    dst[2] = src2;
    dst[3] = src3;
    dst[4] = src4;
    dst[5] = src5;
    dst[6] = src6;
    dst[7] = src7;

    if (isBigEndian()) {
        byteSwap(val);
    }
    return val;
}

// -------------------------------------------------------------------------- //
// compute
// -------------------------------------------------------------------------- //
void compute(int n)
{
    double lfSkewSum;
    double lfSigma;
    double lfMean;

    switch (n) {
    case 1:
        lfSkewSum = charArrayToDouble(0x2C,0x41,0xF1,0x82,0x43,0xB0,0x24,0xC0);
        lfSigma   = charArrayToDouble(0xE2,0x5E,0xC7,0x54,0x26,0x38,0x7D,0x40);
        lfMean    = charArrayToDouble(0x00,0x00,0x00,0x00,0x00,0x50,0x89,0x40);
        lfSkewSum += pow((3 - lfMean) / lfSigma, 3.0);
        break;
    case 2:
        lfSkewSum = charArrayToDouble(0x8D,0x4B,0x7B,0xD8,0x67,0x42,0x8C,0xC0);
        lfSigma   = charArrayToDouble(0xE3,0x5E,0xC7,0x54,0x26,0x38,0x7D,0x40);
        lfMean    = charArrayToDouble(0x00,0x00,0x00,0x00,0x00,0x50,0x89,0x40);
        lfSkewSum += pow((1305 - lfMean) / lfSigma, 3.0);
        break;
    case 3:
        lfSkewSum = charArrayToDouble(0x00,0x00,0x00,0xE0,0xAF,0xAF,0x86,0xC0);
        lfSigma   = charArrayToDouble(0x00,0x00,0x00,0x20,0x71,0x38,0x7D,0x40);
        lfMean    = charArrayToDouble(0x00,0x00,0x00,0x00,0x00,0x50,0x89,0x40);
        lfSkewSum += pow((207 - lfMean) / lfSigma, 3.0);
        break;
    case 4:
        lfSkewSum = charArrayToDouble(0xF1,0xFC,0x0E,0xFD,0x16,0xBA,0x14,0xC0);
        lfSigma   = charArrayToDouble(0xE3,0x5E,0xC7,0x54,0x26,0x38,0x7D,0x40);
        lfMean    = charArrayToDouble(0x00,0x00,0x00,0x00,0x00,0x50,0x89,0x40);
        lfSkewSum += pow((1619 - lfMean) / lfSigma, 3.0);
        break;
    case 5:
        lfSkewSum = charArrayToDouble(0x24,0x1B,0x13,0xFD,0x16,0xBA,0x14,0xC0);
        lfSigma   = charArrayToDouble(0xE2,0x5E,0xC7,0x54,0x26,0x38,0x7D,0x40);
        lfMean    = charArrayToDouble(0x00,0x00,0x00,0x00,0x00,0x50,0x89,0x40);
        lfSkewSum += pow((1619 - lfMean) / lfSigma, 3.0);
        break;
    case 6:
        lfSkewSum = charArrayToDouble(0x63,0xB9,0x9D,0x68,0x42,0x78,0x50,0xC0);
        lfSigma   = charArrayToDouble(0xE2,0x5E,0xC7,0x54,0x26,0x38,0x7D,0x40);
        lfMean    = charArrayToDouble(0x00,0x00,0x00,0x00,0x00,0x50,0x89,0x40);
        lfSkewSum += pow((1607 - lfMean) / lfSigma, 3.0);
        break;
    default:
        return;
    }

    if (isBigEndian()) {
        byteSwap(lfSkewSum);
    }
    hexdump(lfSkewSum);
    if (isBigEndian()) {
        byteSwap(lfSkewSum);
    }
    printf("%.15g", lfSkewSum);
}


// -------------------------------------------------------------------------- //
// main
// -------------------------------------------------------------------------- //
int main(int argc, char** argv)
{
    unsigned int fpu;
    unsigned int n;
    unsigned int i;

    if (argc != 2) {
        printf("usage: %s <1-6>\n", argv[0]);
        return 1;
    }
    n = atoi(argv[1]);

    getFpu(fpu);
    hexdump(fpu);
    printf("\n");

    for (i = 0; i < 16; ++i) {
        if ((i & 3) == 1) {
            continue;
        }

        setFpu((i << 8) + 0x7F);

        getFpu(fpu);
        hexdump(fpu);

        compute(n);
        printf("\n");
    }

    return 0;
}
