#include <stdlib.h>
#include <math.h>

#if defined unix || __MACH__
#include <alloca.h>
#endif

#if defined(__sun__)
#include <floatingpoint.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#include "dl4_tools.h"
// For linux/solaris compilation, include gex_constants.h after dl4_tools.h !!!!
//#include "gex_constants.h"

#define FLT_MAX_4BYTES  3.402823466e+38F
#define DBL_MAX_8BYTES  1.7976931348623158e+308

#undef min
#undef max

StoredObject::StoredObject()
{
#if defined (i386) || defined (__i386__) || defined (_M_IX86)\
            || defined (vax) || defined (__alpha) || defined(__x86_64)\
            || defined(__x86_64__) || defined (_M_X64)
// i386, vax and alpha processors use LITTLE ENDIAN storage all DL4 file are generated on PC,
// change for BigEndian only if Examinator running on SPARC
  bSameCPUType = 1;
#else
// sparc processors (__sparc__) use BIG ENDIAN storage all DL4 file are generated on PC,
// change for BigEndian only if Examinator running on SPARC
  bSameCPUType = 0;
#endif
}

///////////////////////////////////////////////////////////
// Action : Reads one byte from buffer
// return : sizeof buffer
///////////////////////////////////////////////////////////
int StoredObject::ReadByte(const void *v, BYTE *ptByte)
{
  BYTE *b = (BYTE*) v;
  *ptByte = b[0];
  return 1;
}


///////////////////////////////////////////////////////////
// Action : Reads one word (2 bytes) from buffer
// return : sizeof buffer
///////////////////////////////////////////////////////////
int StoredObject::ReadWord(const void *v, short *ptWord)
{
  BYTE  *b = (BYTE*) v;
  BYTE  bByte0,bByte1;
  short sWord;
  int   iSize;

  iSize =  ReadByte(b, &bByte0);
  iSize += ReadByte(b+iSize, &bByte1);
  if(bSameCPUType)
    sWord = ((short)bByte0 & 0xff) | ((short)bByte1  << 8);
  else
    sWord = ((short)bByte1 & 0xff) | ((short)bByte0  << 8);
  *ptWord = sWord;
  return iSize;
}

///////////////////////////////////////////////////////////
// Action : Reads one dword (4 bytes) from buffer
// return : sizeof buffer
///////////////////////////////////////////////////////////
int StoredObject::ReadDword(const void *v, gsint32 *ptDword)
{
  BYTE *b = (BYTE*) v;
  short sWord0, sWord1;
  int iSize;
  gsint32 lWord;

  iSize =  ReadWord(b, &sWord0);
  iSize += ReadWord(b+iSize, &sWord1);
  if(bSameCPUType)
    lWord = ((gsint32)sWord0 & 0xffff) | ((gsint32)sWord1 << 16);
  else
    lWord = ((gsint32)sWord1 & 0xffff) | ((gsint32)sWord0 << 16);

  *ptDword = lWord;
  return iSize; // Success
}

///////////////////////////////////////////////////////////
// Action : Reads one float (4 bytes) from buffer
// return : sizeof buffer
///////////////////////////////////////////////////////////
int StoredObject::ReadFloat(const void *v, float *ptFloat,  bool *pbIsNAN)
{
  BYTE  *b = (BYTE*) v;
  float fData;
  BYTE  *bBuf2;
  BYTE  ptData[4];
  int   iIndex, iSize;

  if(pbIsNAN != NULL)
    *pbIsNAN = false;

  bBuf2 = (BYTE *)&fData;
  iSize = 0;
  for(iIndex=0;iIndex<4;iIndex++)
    iSize += ReadByte(b+iSize, (BYTE *)&ptData[iIndex]);

  if(bSameCPUType)
  {
    bBuf2[0] = ptData[0];
    bBuf2[1] = ptData[1];
    bBuf2[2] = ptData[2];
    bBuf2[3] = ptData[3];
  }
  else
  {
    bBuf2[0] = ptData[3];
    bBuf2[1] = ptData[2];
    bBuf2[2] = ptData[1];
    bBuf2[3] = ptData[0];
  }

#ifdef __sparc__
  // Check if the read value is a INF or -INF value
  // (all bits set in E, zero Fraction)
  if (((bBuf2[0] & 0x7f) == 0x7f) && (bBuf2[1] & 0x80) && // E has all bits set
      ((bBuf2[1] & 0x7f) == 0) && ((bBuf2[2] & 0xff) == 0) &&
      ((bBuf2[3] & 0xff) == 0))  // Zero Fraction
  {
    if(bBuf2[0] & 0x80)
      fData = -FLT_MAX_4BYTES;
    else
      fData = FLT_MAX_4BYTES;
  }

  // Check if read value is a NAN (all bits set in E and non-zero Fraction)
  if (((bBuf2[0] & 0x7f) == 0x7f) && (bBuf2[1] & 0x80) && // E has all bits set
      ((bBuf2[1] & 0x7f) || (bBuf2[2] & 0xff) || (bBuf2[3] & 0xff)))
    // Non-zero Fraction
  {
    if(pbIsNAN)
      *pbIsNAN = true;
  }
#else
  // Check if the read value is a INF or -INF value
  // (all bits set in E, zero Fraction)
  if (((bBuf2[3] & 0x7f) == 0x7f) && (bBuf2[2] & 0x80) && // E has all bits set
      ((bBuf2[2] & 0x7f) == 0) && ((bBuf2[1] & 0xff) == 0) &&
      ((bBuf2[0] & 0xff) == 0))  // Zero Fraction
  {
    if(bBuf2[3] & 0x80)
      fData = -FLT_MAX_4BYTES;
    else
      fData = FLT_MAX_4BYTES;
  }

  // Check if read value is a NAN (all bits set in E and non-zero Fraction)
  if (((bBuf2[3] & 0x7f) == 0x7f) && (bBuf2[2] & 0x80) && // E has all bits set
      ((bBuf2[2] & 0x7f) || (bBuf2[1] & 0xff) || (bBuf2[0] & 0xff)))
    // Non-zero Fraction
  {
    if(pbIsNAN)
      *pbIsNAN = true;
  }
#endif

  // Added to avoid '-INF value' corrupt computation...& crash under Unix!
  if(fData >= FLT_MAX_4BYTES)
    fData = FLT_MAX_4BYTES;
  else
  if(fData <= -FLT_MAX_4BYTES)
    fData = -FLT_MAX_4BYTES;

  *ptFloat = fData;
  return iSize;
}

///////////////////////////////////////////////////////////
// Action : Reads one double (8 bytes) from buffer
// return : sizeof buffer
///////////////////////////////////////////////////////////
int StoredObject::ReadDouble(const void *v, double *ptDouble)
{
  BYTE  *b = (BYTE*) v;
  double  lfData;
  BYTE  *bBuf8;
  BYTE  ptData[8];
  int   iIndex, iSize;

  bBuf8 = (BYTE *)&lfData;
  iSize = 0;
  for(iIndex=0;iIndex<8;iIndex++)
    iSize += ReadByte(b+iSize, (BYTE *)&ptData[iIndex]);

  if(bSameCPUType)
  {
    bBuf8[0] = ptData[0];
    bBuf8[1] = ptData[1];
    bBuf8[2] = ptData[2];
    bBuf8[3] = ptData[3];
    bBuf8[4] = ptData[4];
    bBuf8[5] = ptData[5];
    bBuf8[6] = ptData[6];
    bBuf8[7] = ptData[7];
  }
  else
  {
    bBuf8[0] = ptData[7];
    bBuf8[1] = ptData[6];
    bBuf8[2] = ptData[5];
    bBuf8[3] = ptData[4];
    bBuf8[4] = ptData[3];
    bBuf8[5] = ptData[2];
    bBuf8[6] = ptData[1];
    bBuf8[7] = ptData[0];
  }

  // Added to avoid '-INF value' corrupt computation...& crash under Unix!
  if(lfData >= DBL_MAX_8BYTES)
    lfData = DBL_MAX_8BYTES;
  else
  if(lfData <= -DBL_MAX_8BYTES)
    lfData = -DBL_MAX_8BYTES;

  *ptDouble = lfData;
  return iSize;
}

///////////////////////////////////////////////////////////
// Action : Reads one String from buffer
// return : sizeof buffer
///////////////////////////////////////////////////////////
int StoredObject::ReadString(const void *v, char *szString, short *len)
{
  BYTE  *b = (BYTE*) v;
  BYTE  iIndex;
  int   iSize;

  *szString = 0;

  iSize = ReadByte(b, (BYTE*)len);
  *len &= 0xff;
  for(iIndex=0;iIndex<*len;iIndex++)
    iSize += ReadByte(b+iSize, (BYTE *)&szString[iIndex]);

  szString[iIndex] = 0;
  return iSize;
}

///////////////////////////////////////////////////////////
// Copy in  buffer a x bytes string (x on 1 byte).
///////////////////////////////////////////////////////////
int StoredObject::WriteString(void *v, char *szString, short len)
{
  BYTE  *b = (BYTE*) v;
  LPSTR ptChar;
  int iIndex, iSize = 0;

  b[iSize++] = len; // string length.
  ptChar = szString;

  for(iIndex=0; iIndex<len; iIndex++)
  {
    b[iSize++] = *ptChar;
    ptChar++;
  }
  return iSize;
}

///////////////////////////////////////////////////////////
// Copy in buffer a 4 bytes number.
///////////////////////////////////////////////////////////
int StoredObject::WriteDword(void *v, gsint32 dwData)
{
  BYTE  *b = (BYTE*) v;
  int   iSize = 0;
  LPSTR ptData;

  ptData = (LPSTR)&dwData;
  if(bSameCPUType)
  {
    b[iSize++] = ptData[0];
    b[iSize++] = ptData[1];
    b[iSize++] = ptData[2];
    b[iSize++] = ptData[3];
  }
  else
  {
    b[iSize++] = ptData[3];
    b[iSize++] = ptData[2];
    b[iSize++] = ptData[1];
    b[iSize++] = ptData[0];
  }
  return iSize;
}

///////////////////////////////////////////////////////////
// Copy in buffer a 2 bytes number.
///////////////////////////////////////////////////////////
int StoredObject::WriteWord(void *v, WORD wData)
{
  BYTE  *b = (BYTE*) v;
  int   iSize = 0;
  LPSTR ptData;

  ptData = (LPSTR)&wData;
  if(bSameCPUType)
  {
    b[iSize++] = ptData[0];
    b[iSize++] = ptData[1];
  }
  else
  {
    b[iSize++] = ptData[1];
    b[iSize++] = ptData[0];
  }
  return iSize;
}

///////////////////////////////////////////////////////////
// Copy in buffer a 1 byte number.
///////////////////////////////////////////////////////////
int StoredObject::WriteByte(void *v, BYTE bData)
{
  BYTE  *b = (BYTE*) v;
  int   iSize = 0;

  b[iSize++] = bData;
  return iSize;
}

///////////////////////////////////////////////////////////
// Copy in buffer a 4 bytes float number.
///////////////////////////////////////////////////////////
int StoredObject::WriteFloat(void *v, float fData)
{
  BYTE  *b = (BYTE*) v;
  int   iSize = 0;
  BYTE  *ptData;

  ptData = (BYTE *)&fData;
  if(bSameCPUType)
  {
    b[iSize++] = ptData[0];
    b[iSize++] = ptData[1];
    b[iSize++] = ptData[2];
    b[iSize++] = ptData[3];
  }
  else
  {
    b[iSize++] = ptData[3];
    b[iSize++] = ptData[2];
    b[iSize++] = ptData[1];
    b[iSize++] = ptData[0];
  }
  return iSize;
}

///////////////////////////////////////////////////////////
// Copy in buffer a 8 bytes double number.
///////////////////////////////////////////////////////////
int StoredObject::WriteDouble(void *v, double dData)
{
  BYTE  *b = (BYTE*) v;
  int   iSize = 0;
  BYTE  *ptData;

  ptData = (BYTE *)&dData;
  if(bSameCPUType)
  {
    b[iSize++] = ptData[0];
    b[iSize++] = ptData[1];
    b[iSize++] = ptData[2];
    b[iSize++] = ptData[3];
    b[iSize++] = ptData[4];
    b[iSize++] = ptData[5];
    b[iSize++] = ptData[6];
    b[iSize++] = ptData[7];
  }
  else
  {
    b[iSize++] = ptData[7];
    b[iSize++] = ptData[6];
    b[iSize++] = ptData[5];
    b[iSize++] = ptData[4];
    b[iSize++] = ptData[3];
    b[iSize++] = ptData[2];
    b[iSize++] = ptData[1];
    b[iSize++] = ptData[0];
  }
  return iSize;
}


////////////////////////////////////////////////////////////////
// Concatenate until dst is up to maxlen char long
//
inline BOOL decimal_point_exists (const char *s_num)
{
  int i=0;

  while (s_num[i])
  {
    if (s_num[i] == '.')
      return true;

    i++;
  }

  return false;
}


inline void eliminate_redundant_characters_from_number (char *s_num)
{
  BOOL exponent_found = false;

  // if last character is a decimal point, add 1 trailing zero
  QString temp = s_num;
  short i = temp.indexOf ('.');
  short j = temp.length ();
  if (i == j - 1) temp += "0";
  j = temp.length ();
  StrMCpy (s_num, temp.toLatin1().constData(), j + 1) ;

  // Eliminate leading zeros in the exponent
  i=1;
  while (1)
  {
    if (s_num[i] == 'e' || s_num[i] == 'E')
    {
      exponent_found = true;
      i++;
      if (s_num[i] == '-') i++; else
      if (s_num[i] == '+') i++;
      short i0=i;
      while (s_num[i] == '0') i++;
      while (s_num[i])
      {
        s_num[i0++] = s_num[i++];
      }
      s_num[i0] = 0;

      break;
    }
    i++;
    if (!s_num[i]) break;
  }

  // Eliminate decimal point if possible
  i=1;
  while (s_num[i])
  {
    if (s_num[i] == 'e' || s_num[i] == 'E')
    {
      if (s_num[i-1] == '.')
      {
        while (s_num[i])
        {
          s_num[i-1] = s_num[i];
          i++;
        }
        s_num[i-1] = 0;
        break;
      }
    }
    i++;
  }

  // Eliminate trailing zeros
  if (!exponent_found && decimal_point_exists (s_num))
  {
    short len = StrLen (s_num);
    while (s_num[len-1] == '0')
    {
      if (!len) break;
      len--;
      if (s_num[len-1] != '.')
        s_num[len] = 0;
    }
  }
}

inline BOOL IsWhiteSpaceChar (const char &c)
{
    if (c == ' ') return true;   // SPACE
    if (c == 0x09 ) return true;  // TAB
    return false;
}

BOOL DoubleToString (double d, char *s_ret, short max_len)
{
    if (max_len < 10) return false;

    if (!d)
  {
    s_ret[0] = '0';
    s_ret[1] = 0;
    return true;
  }

  char *s = (char *) alloca (max_len+10);
  if (!s) return false;

    if (!d)
  {
    s_ret[0] = '0';
    s_ret[1] = 0;
    return true;
  }

    while (1)
    {
        if (d < 0.0F)
        {
            if (d < -1.0e6)
                break;

            if (d > -1.0e-6)
                break;

            if (d == (gsint32) d) // Integer valued
            {
                sprintf (s,"%d",(gsint32)d);
        StrMCpy (s_ret,s,max_len+1);
                return true;
            }

      short sig_dig = max_len-1;

      s = gcvt((double) d, sig_dig, s);
      eliminate_redundant_characters_from_number (s);
      if (StrLen (s) > max_len)
      {
        sig_dig -= StrLen (s)-max_len;
        if (sig_dig < 1)
        {
          StrMCpy (s_ret,s,max_len+1);
          return false;
        }

        while (1)
        {
          s = gcvt((double) d, sig_dig, s);
          eliminate_redundant_characters_from_number (s);

          if (StrLen (s) > max_len)
          {
            sig_dig--;
            continue;
          }

          break;
        }

        if (StrLen (s) > max_len) return false;
      }

      StrMCpy (s_ret,s,max_len+1);
      return true;
        }

        if (d > 1.0e6)
            break;

        if (d < 1.0e-6)
            break;

        if (d == (gsint32) d) // Integer valued
        {
            sprintf (s,"%d",(gsint32)d);
      StrMCpy (s_ret,s,max_len+1);
            return true;
        }

    short sig_dig = max_len-1;

    s = gcvt((double) d, sig_dig, s);
    eliminate_redundant_characters_from_number (s);
    if (StrLen (s) > max_len)
    {
      sig_dig -= StrLen (s)-max_len;
      if (sig_dig < 1)
      {
        StrMCpy (s_ret,s,max_len+1);
        return false;
      }

      while (1)
      {
        s = gcvt((double) d, sig_dig, s);
        eliminate_redundant_characters_from_number (s);

        if (StrLen (s) > max_len)
        {
          sig_dig--;
          continue;
        }

        break;
      }

      if (StrLen (s) > max_len) return false;
    }

    StrMCpy (s_ret,s,max_len+1);
    return true;
   }

    BOOL iret = EngineeringNotation ( d, s, max_len, std::min((int)max_len-8, 12)  );

  StrMCpy (s_ret,s,max_len+1);

    return iret;
}

BOOL FloatToString (float d, char *s_ret, short max_len, short sig_digits /* = 0 */, BOOL fixed_decimal /* = false */)
{
    if (max_len < 7) return false;

  char *s = (char *) alloca (2*max_len+1);
  if (!s) return false;

  if (fixed_decimal)
  {
    double max_abs_value = pow (10,max_len);

    if (fabs (d) >= max_abs_value)
      return false;

    if (fabs (d) <= 1.0/max_abs_value)
    {
      s_ret[0] = '0';
      s_ret[1] = '.';
      MemSet (&s_ret[2],'0',sig_digits);
      s_ret[2+sig_digits] = 0;
      return true;
    }

    STRING fmt = "%-";
    fmt += max_len;
    fmt += ".";
    fmt += sig_digits;
    fmt += "f";
    sprintf (s_ret,fmt,d);

    s_ret[max_len] = 0;

    if (s_ret[max_len-1] == '.')
      s_ret[max_len-1] =0;


    return true;
  }

    if (!d)
  {
    s_ret[0] = '0';
    s_ret[1] = 0;
    return true;
  }

    while (1)
    {
        if (d < 0.0F)
        {
            if (d < -1.0e3F)
                break;

            if (d > -1.0e-3F)
                break;

            if (d == (gsint32) d) // Integer valued
            {
                sprintf (s,"%d",(gsint32)d);
        StrMCpy (s_ret,s,max_len+1);
                return true;
            }

      short sig_dig;
      if (sig_digits) sig_dig = sig_digits;
      else sig_dig = max_len-1;

      s = gcvt((double) d, sig_dig, s);
      eliminate_redundant_characters_from_number (s);
      if (StrLen (s) > max_len)
      {
        sig_dig -= StrLen (s)-max_len;
        if (sig_dig < 1)
        {
          StrMCpy (s_ret,s,max_len+1);
          return false;
        }

        while (1)
        {
          s = gcvt((double) d, sig_dig, s);
          eliminate_redundant_characters_from_number (s);

          if (StrLen (s) > max_len)
          {
            sig_dig--;
            continue;
          }

          break;
        }

        if (StrLen (s) > max_len) return false;
      }

      StrMCpy (s_ret,s,max_len+1);
      return true;
        }

        if (d > 1.0e3F)
            break;

        if (d < 1.0e-3F)
            break;

        if (d == (gsint32) d) // Integer valued
        {
            sprintf (s,"%d",(gsint32)d);
      StrMCpy (s_ret,s,max_len+1);
            return true;
        }

    short sig_dig;
    if (sig_digits) sig_dig = sig_digits;
    else sig_dig = max_len-1;

    s = gcvt((double) d, sig_dig, s);
    eliminate_redundant_characters_from_number (s);
    if (StrLen (s) > max_len)
    {
      sig_dig -= StrLen (s)-max_len;
      if (sig_dig < 1)
      {
        StrMCpy (s_ret,s,max_len+1);
        return false;
      }

      while (1)
      {
        s = gcvt((double) d, sig_dig, s);
        eliminate_redundant_characters_from_number (s);

        if (StrLen (s) > max_len)
        {
          sig_dig--;
          continue;
        }

        break;
      }

      if (StrLen (s) > max_len) return false;
    }

    StrMCpy (s_ret,s,max_len+1);
    return true;
   }

    BOOL iret = EngineeringNotation ((double) d, s, max_len, std::min((int)max_len-5, 7)  );

  StrMCpy (s_ret,s,max_len+1);

  return iret;
}



BOOL EngineeringNotation( double input_variable,
              char *output_var_str,
              short max_output_str_len,
              short sig_digits)
{
  char sign=0;
  short i,j;

  for (i=0;i<max_output_str_len;i++)
    output_var_str[i] = 0;

  // Determine the sign of the input variable.
  if (input_variable < 0.0)
  {
    input_variable = -input_variable;
    sign = '-';
  }
  else
    sign = '+';

  double mantessa,exponent;
  short iexponent;

  char s_tmp[128];

  if (input_variable == 0.0)
  {
    mantessa = 0.0;
    iexponent = 0;
  }
  else
  {
//    char* dummy;
    // BG: First check some conditions,
    // so we don't have to check the maths functions return value (log10, ...)
    if(input_variable < 0)
    {
      if (gcvt(input_variable, sig_digits, s_tmp))
      {
      }

      eliminate_redundant_characters_from_number (s_tmp);

      if (sign == '-')
      {
        STRING s0 = "-";
        s0 += s_tmp;
        strncpy (s_tmp,s0,127);
      }

      BOOL iret = false;
      if (StrLen (s_tmp) <= max_output_str_len) iret = true;

      strncpy(output_var_str,
              s_tmp,std::min(StrLen(s_tmp) + 1, (int) max_output_str_len));
      output_var_str[max_output_str_len] = 0;

      return iret;
    }

    double lg = log10 (input_variable);
    double tmp = modf (lg,&exponent);
    mantessa = pow (10.0,tmp);

    iexponent = (short) exponent;

    while (mantessa < 1.0)
    {
      if (iexponent < -100 || iexponent > 100)
      {
        if (gcvt(input_variable, sig_digits, s_tmp))
        {
        }

        eliminate_redundant_characters_from_number (s_tmp);

        if (sign == '-')
        {
          STRING s0 = "-";
          s0 += s_tmp;
          strncpy (s_tmp,s0,127);
        }

        strncpy(output_var_str,
                s_tmp, std::min(StrLen(s_tmp) + 1, (int) max_output_str_len));
        return false;
      }

      mantessa *= 10;
      iexponent--;
    }
  }

  short decimal_location=1;

  while ( static_cast< short >( abs( iexponent ) ) % 3 )
  {
    iexponent--;
    decimal_location++;
  }

  int junk0=0,junk1=0;
  char *s_number = fcvt (mantessa,sig_digits,&junk0,&junk1);

  i=0;
  if (i<max_output_str_len)
    output_var_str[i++] = sign;

  j=0;
  while (i<max_output_str_len && j < decimal_location)
  {
    output_var_str[i++] = s_number[j++];
  }

  if (i<max_output_str_len)
    output_var_str[i++] = '.';

  while (i<max_output_str_len && j < sig_digits)
    output_var_str[i++] = s_number[j++];

  char s_exponent[10];
  sprintf (s_exponent,"%+03d",iexponent);

  if (i<max_output_str_len)
    output_var_str[i++] = 'E';

  j=0;
  while (i<max_output_str_len && s_exponent[j])
    output_var_str[i++] = s_exponent[j++];

  eliminate_redundant_characters_from_number (output_var_str);

  if (i==max_output_str_len)
    return (false); // Error

  return (true);
}

////////////////////////////////////////////////////////////////////////////
// Eliminate leading/trailing/extra whitespace characters from a string
//
void trim (char *s, BOOL leading/*=true*/, BOOL extra/*=true*/, BOOL trailing/*=true*/)
{
    short i,j,k,n=StrLen (s);

    // Terminate the string at the first CR or LF character
    for (i=0;i<n;i++)
    {
        if (s[i] == 0x0d || s[i] == 0x0a)
        {
            s[i] = 0;
            n = i+1;
            break;
        }
    }

    // Eliminate leading spaces
  if (leading)
  {
    for (i=0;i<n;i++)
    {
      if (!IsWhiteSpaceChar (s[i]))
        break;
    }

    k=i;
    for (j=0;j<n-k;j++)
      s[j] = s[i++];

    s[j] = 0;

    n -= k;  // new length of the string
  }

    // Eliminate trailing spaces
  if (trailing)
  {
    for (i=n-1;i>0;i--)
    {
      if (!IsWhiteSpaceChar (s[i]))
        break;
    }

    s[i+1] = 0;

    n = i+1;   // new length of the string
  }

    // Eliminate extra spaces
  if (extra)
  {
    if (n>1)
    {
      i=1;
      for (j=1;j<n;j++)
      {
        if (IsWhiteSpaceChar (s[j]) && IsWhiteSpaceChar (s[j-1]))
          continue;

        s[i++] = s[j];
      }
      s[i] = 0;
    }
  }
}

/////////////////////////////////////////////////////////////////////////
//////// BEGIN //////////////////////////////////////////////////////////
///////////////////// STRING ////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

char *STRING::GetStringPtr()
{
  return s;
}


short STRING::len (void)
{
  return (StrLen (s));
}

void STRING::Trim (BOOL leading/*=true*/, BOOL extra/*=true*/, BOOL trailing/*=true*/)
{
  trim (s,leading,extra,trailing);
}

BOOL STRING::LoadFileName (const char *file_name, BOOL include_path, BOOL include_name, BOOL include_ext)
{
  short ipath=0, iname=-1, iext=-1;

  while (IsWhiteSpaceChar (file_name[ipath])) ipath++;

  if (!file_name[ipath]) return false;

  short i=ipath;
  short icolon = -1;
  short islash = -1;
  while (1)
  {
    if (file_name[i] == ':')  { icolon = i; i++;  continue; }
    if (file_name[i] == '\\') { islash = i; i++;  continue; }
    if (file_name[i] == '.')  { iext   = i; i++;  continue; }
    if (!file_name[i])
    {
      if (icolon == -1 && islash == -1)
      {
        // There was not path spec
        iname = ipath;
        ipath = -1;
        break;
      }
      else
      {
        if (islash >= 0) { iname = islash+1; break;}
        if (icolon >= 0) { iname = icolon+1; break; }
      }

//      ASSERT (false);
    }

    i++;
  }

  if (iext >= 0 && islash >= 0 && islash > iext) iext = -1;

  short iend_of_name = i;

  while (file_name[i]) i++;

  if (include_path && include_name && include_ext)
  {
    return (Assign (&file_name[ipath],i-ipath));
  }

  if (include_path && include_name && ! include_ext)
  {
    if (iext == -1) iext = i;
    return (Assign (&file_name[ipath],iext-ipath));
  }

  if (include_path && ! include_name && ! include_ext)
  {
    return (Assign (&file_name[ipath],iname-ipath));
  }

  if (! include_path && include_name && include_ext)
  {
    return (Assign (&file_name[iname],i-iname));
  }

  if (! include_path && include_name && ! include_ext)
  {
    if (iext >= 0)
      return (Assign (&file_name[iname],iext-iname));
    else
      return (Assign (&file_name[iname],iend_of_name-iname));
  }

  if (! include_path && ! include_name && include_ext)
  {
    if (iext < 0)
      iext = i;
    return (Assign (&file_name[iext],i-iext));
  }

  return false;
}

BOOL STRING::SetValidFileName (const char *path, const char *file_name)
{
  if (StrLen (path))
  {
    *this = path;
    Trim (true,false,true);

    short n = len ();

    if (n)
    {
      if (s[n-1] != '\\')
        *this += "\\";

      *this += file_name;

      return true;
    }
  }

  *this = file_name;
  return true;
}


STRING& STRING::operator = (STRING &s0)
{
  *this = (const char *)s0;
  return (*this);
}

STRING& STRING::operator += (STRING &s0)
{
  *this += (const char *)s0;
  return (*this);
}

STRING::operator short ()
{
  return (atoi (s));
}

STRING::operator unsigned short ()
{
  unsigned short n;
  if (sscanf (s,"%hu",&n) != 1) return (0);
  return (n);
}

STRING::operator gsuint32 ()
{
  gsuint32 n;
  if (sscanf (s,"%u",&n) != 1) return (0);
  return n;
}

STRING::operator gsint32 ()
{
  return (atol (s));
}

STRING::operator double ()
{
  return (atof (s));
}


STRING& STRING::operator += (short i)
{
  if (err) return (*this);

  char s_i[20];
  sprintf (s_i,"%d",i);

  *this += s_i;

  return (*this);
}

STRING& STRING::operator += (unsigned short i)
{
    if (err) return (*this);

    char s_i[20];
    sprintf (s_i,"%u",i);

    *this += s_i;

    return (*this);
}

STRING& STRING::operator = (unsigned short i)
{
    if (err) return (*this);

    char s_i[20];
    sprintf (s_i,"%u",i);

    *this = s_i;

    return (*this);
}

STRING& STRING::operator += (float v)
{
  if (err) return (*this);

  char s_v[17];

  FloatToString (v,s_v,8);

  *this += s_v;

  return (*this);
}

STRING& STRING::operator = (float v)
{
  if (err) return (*this);

  char s_v[17];
    FloatToString (v,s_v,8);

  *this = s_v;

  return (*this);
}

STRING& STRING::operator += (double v)
{
  if (err) return (*this);

  char s_v[19];
    DoubleToString (v,s_v,17);

  *this += s_v;

  return (*this);
}

STRING& STRING::operator = (double v)
{
  if (err) return (*this);

  char s_v[19];
    DoubleToString (v,s_v,17);

  *this = s_v;

  return (*this);
}

STRING& STRING::operator += (UINT i)
{
  if (err) return (*this);

  char s_i[20];

  sprintf (s_i,"%u",i);

  *this += s_i;

  return (*this);
}

STRING& STRING::operator = (UINT i)
{
  if (err) return (*this);

  char s_i[20];

  sprintf (s_i,"%u",i);

  *this = s_i;

  return (*this);
}

//STRING& STRING::operator += (gsuint32 l)
//{
//  if (err) return (*this);

//  char s0[20];
//  sprintf (s0,"%lu",l);

//  *this += s0;

//  return (*this);
//}

//STRING& STRING::operator = (gsuint32 l)
//{
//  if (err) return (*this);

//  char s0[20];
//  sprintf (s0,"%lu",l);

//  *this = s0;

//  return (*this);
//}

STRING& STRING::operator += (gsint32 l)
{
  if (err) return (*this);

  char s0[20];
  sprintf (s0,"%d",l);

  *this += s0;

  return (*this);
}

STRING& STRING::operator = (gsint32 l)
{
  if (err) return (*this);

  char s0[20];
  sprintf (s0,"%d",l);

  *this = s0;

  return (*this);
}


STRING& STRING::operator += (const char *s0)
{
    if (!s0) return (*this);
  if (err) return (*this);

  short newlen = StrLen (s0) + StrLen (s);
  char *s_old = s;

  s = new char[newlen+1];
//  ASSERT(s);

  strcpy (s,s_old);
  strcat (s,s0);

  delete s_old;

  return (*this);
}

BOOL STRING::Assign (const char *s0)
{
  *this = s0;
  return true;
}

BOOL STRING::Assign (const char *s0, const char *s1)
{
  *this = s0;
  if (s1) *this += s1;
  return true;
}

BOOL STRING::Assign (const char *s0, const char *s1, const char *s2)
{
  *this = s0;
  if (s1) *this += s1;
  if (s2) *this += s2;
  return true;
}

BOOL STRING::Assign (const char *s0, const char *s1, const char *s2, const char *s3)
{
  *this = s0;
  if (s1) *this += s1;
  if (s2) *this += s2;
  if (s3) *this += s3;
  return true;
}

BOOL STRING::Assign (const char *s0, short n)
{
    if (n <= 0)
        return false;

  delete s;

  s = new char[n+1];
  if (!s) return false;

  strncpy (s,s0,n);
  s[n] = 0;

  return true;
}


STRING &STRING::operator = (short i)
{
  if (err) return (*this);

  char s_i[20];
  sprintf (s_i,"%d",i);

  *this = s_i;

  return (*this);
}

STRING& STRING::operator = (const char *s0)
{
    if (!s0) return (*this);
    if (err) return (*this);

  short len = StrLen (s0);

  delete s;

    s = new char[len+1];
    if (!s)
    {
        err=1;
        return *this;
    }
    strcpy (s,s0);
    return *this;
}

STRING::STRING (void)
{
  s = new char[1];
  s[0] = 0;
    err=0;
}

STRING::STRING (const char *s0)
{
    s=0;
    err=0;

  if (!s0)
  {
    s = new char[1];
    s[0]=0;
    return;
  }

    short len = StrLen (s0);

    s = new char[len+1];
    if (!s)
    {
        err=1;
        return;
    }

    strcpy (s,s0);
}

STRING::STRING (const STRING& src) : StoredObject()
{
  err = 0;
  short len = StrLen(src.s);
  s = new char[len+1];
  if (!s)
  {
    err = 1;
    return;
  }
  strcpy (s,src.s);
}

STRING::~STRING (void)
{
  if (s != NULL) {
    delete[] s;
  }

  s = NULL;
}

BOOL STRING::Overwrite (short i, const char *c, short width)
{
    short org_len = len ();
  short j;

  if (org_len < i + width)
  {
    if (!c) return true;

    short idx_of_last_non_space=0;
    j=0;

    while (c[j])
    {
      if (c[j] != ' ')
        idx_of_last_non_space = j;

      j++;
    }

    if (idx_of_last_non_space+i < org_len)
    {   // No need to reallocate
      width = org_len - i;
    }
    else
    {
      short alloc_size = idx_of_last_non_space+2+i;
      char *tmp = new char[alloc_size];
      if (!tmp) return false;

      strcpy (tmp,s);

      delete s; s = tmp;

      // Pad with spaces
      for (j=org_len; j<alloc_size; j++)
        s[j] = ' ';

      // Make sure that s[] is null terminated
      s[alloc_size-1] = 0;
      width = std::min((int)width, (int)idx_of_last_non_space+1);
    }
  }

  short n=0;
  char c0;
  j=0;

  while (n < width)
  {
    if (c)
      c0 = c[j];
    else
      c0 = ' ';

    if (c0)
      j++;
    else
      c0 = ' ';

    s[i] = c0;

    i++; n++;
  }

  return true;
}

BOOL STRING::Insert (short i, const char *c, short width)
{
    if (!c)
    {
        return true;
    }

    short org_len = len ();

  short idx_of_last_non_space=-1;
  short j=0;

  short nalloc;

  if (i >= org_len)
  {
    while (c[j])
    {
      if (c[j] != ' ')
        idx_of_last_non_space = j;

      j++;
    }

    if (idx_of_last_non_space < 0)
      return true;

    width = std::min((int)width, (int)idx_of_last_non_space+1);

    nalloc = org_len + (i-org_len) + width + 1;
  }
  else
    nalloc = org_len + width + 1;

  if (!width)
    return true;

  char *tmp = new char[nalloc];
  if (!tmp) return false;

  strcpy (tmp,s);
  delete s; s = tmp;

  if (i >= org_len)
  {
    // Fill with spaces from the end of the old line
    // to the beginning of the insert section
    for (j=org_len; j<i; j++)
      s[j] = ' ';
  }
  else
  {
    // Shift the contents of the original string forward
    // to make room for the insert
    for (j=org_len+width-1; ((j >= i) && (j >= width)); j--)
      s[j] = s[j-width];
  }

    j=0;
    short n=0;
    char c0;
  while (n < width)
  {
    c0 = c[j];

    if (c0)
      j++;
    else
      c0 = ' ';

    s[i++] = c0;
    n++;
  }

  s[nalloc-1] = 0;

  return true;
}

BOOL STRING::Remove (short i, short width)
{
  short org_len = len ();

  if (i<0) return false;
  if (i >= org_len) return true;

  if (i+width >= org_len)
  {
    s[i] = 0;

    char *tmp = new char[i+1];
    if (!tmp) return false;

    strcpy (tmp,s);
    delete s; s=tmp;

    return true;
  }

  short j=i+width;

  while (1)
  {
    s[i] = s[j];

    if (!s[j])
      break;

    i++; j++;
  }

  return true;
}


gsint32 STRING::GetStorageSizeBytes (void)
{
  return StrLen (s)+1;
}

gsint32 STRING::LoadFromBuffer (const void *buffer)
{
  *this = (const char *) buffer;
  return StrLen (s) + 1;
}

void STRING::AppendHexValue (gsint32 u4)
{
  char buffer[64];
    sprintf (buffer,"%04X",(gsuint32)u4);
  (*this) += buffer;
}


/////////////////////////////////////////////////////////////////////////
//////// END ////////////////////////////////////////////////////////////
///////////////////// STRING ////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////
//////// BEGIN //////////////////////////////////////////////////////////
///////////////////// GENERIC_STORED_OBJECT_WRAPPER /////////////////////
/////////////////////////////////////////////////////////////////////////

GENERIC_STORED_OBJECT_WRAPPER::GENERIC_STORED_OBJECT_WRAPPER (void)
{
  m_data = 0;
  m_data_size = 0;
  m_data_dymamically_allocated = false;

}

GENERIC_STORED_OBJECT_WRAPPER::~GENERIC_STORED_OBJECT_WRAPPER (void)
{
  if (m_data_dymamically_allocated)
    delete [] (BYTE*) m_data;
}

gsint32 GENERIC_STORED_OBJECT_WRAPPER::GetStorageSizeBytes (void)
{
  return m_data_size;
}

BOOL GENERIC_STORED_OBJECT_WRAPPER::DynamicallyAllocateData (gsint32 size)
{
  m_data_size = size;
  m_data = (void *) new BYTE[size];
  if (m_data == NULL) { m_data_size = 0; return false; }
  MemSet (m_data,0,size);
  m_data_dymamically_allocated = true;
  return true;
}

gsint32 GENERIC_STORED_OBJECT_WRAPPER::LoadFromBuffer (const void *buffer)
{
//  ASSERT (m_data);
  MemMove (m_data,buffer,m_data_size);
  return m_data_size;
}

void GENERIC_STORED_OBJECT_WRAPPER::SetStorageSize (gsint32 data_size)
{
  m_data_size = data_size;
}

void GENERIC_STORED_OBJECT_WRAPPER::SetStaticData (const void *data)
{
  if (m_data_dymamically_allocated)
    delete [] (BYTE*) m_data;

  m_data_dymamically_allocated = false;

  union
  {
    void *v;
    const void *v_const;
  };

  // Way to get around const/non const assignment
  v_const = data;
  m_data = v;
}

/////////////////////////////////////////////////////////////////////////
//////// END ////////////////////////////////////////////////////////////
///////////////////// GENERIC_STORED_OBJECT_WRAPPER /////////////////////
/////////////////////////////////////////////////////////////////////////




