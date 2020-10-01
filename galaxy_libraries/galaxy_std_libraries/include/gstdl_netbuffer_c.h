/*======================================================================*/
/* Galaxy                                                               */
/* Copyright 1996-99, Softlink                                          */
/* This computer program is protected by copyrigt law                   */
/* and international treaties. Unauthorized reproduction or             */
/* distribution of this program, or any portion of it,may               */
/* result in severe civil and criminal penalties, and will be           */
/* prosecuted to the maximum extent possible under the low.             */
/*======================================================================*/

/*======================================================================*/
/*======================= GNETBUFFER.H HEADER ==========================*/
/*======================================================================*/
#ifndef _GNET_BUFFER_H_
#define _GNET_BUFFER_H_

#include "gstdl_neterror_c.h"    /* For error codes */

// clean up, pyc, 03/11/2011
//#if defined(__unix__) || defined(__sun__)
#ifdef __sparc__
#define	_BIGENDIAN
#endif

/*======================================================================*/
/* New types definition.                                                */
/*======================================================================*/
/* Basic types and pointer on basic types */
typedef unsigned char   GNB_BOOL;
typedef char            GNB_CHAR;
typedef unsigned char   GNB_BYTE;
typedef unsigned short  GNB_WORD;
typedef unsigned int    GNB_DWORD;
typedef int	            GNB_INT4;
typedef double          GNB_DOUBLE;
typedef float           GNB_FLOAT;

typedef GNB_CHAR        *PT_GNB_CHAR;
typedef GNB_BYTE        *PT_GNB_BYTE;
typedef GNB_WORD        *PT_GNB_WORD;
typedef GNB_DWORD       *PT_GNB_DWORD;
typedef GNB_INT4        *PT_GNB_INT4;
typedef GNB_DOUBLE      *PT_GNB_DOUBLE;
typedef GNB_FLOAT       *PT_GNB_FLOAT;

/* Handler on a Galaxy Network Buffer */
struct tagGNB_HBUFFER
{
  char*         ptBuffer;
  unsigned int  uiBufferOpened;
  unsigned int  uiBufferPos;
  unsigned int  uiBufferEnd;
  unsigned int  uiAllocatedSize;
};
typedef struct tagGNB_HBUFFER GNB_HBUFFER;
typedef struct tagGNB_HBUFFER *PT_GNB_HBUFFER;

/*======================================================================*/
/* Global defines :				                                              */
/*======================================================================*/
#ifndef FALSE
#define FALSE                   0
#endif
#ifndef TRUE
#define TRUE                    1
#endif
#define GNB_BUFFER_BLOCKSIZE    4*1024
#define GNB_SEEK_BEGIN          0
#define GNB_SEEK_END            1

/*======================================================================*/
/* Error Codes            																							*/
/*======================================================================*/
/* See gneterror.h */

/*======================================================================*/
/* Exported Functions.                                                  */
/*======================================================================*/
#ifdef _GNET_BUFFER_MODULE_

  int			gnb_Init(PT_GNB_HBUFFER, char*, unsigned int);
  int			gnb_ReOpen(PT_GNB_HBUFFER, char*, unsigned int);
  int			gnb_Open(PT_GNB_HBUFFER);
  int			gnb_Close(PT_GNB_HBUFFER);
  int			gnb_Seek(PT_GNB_HBUFFER, unsigned int);
  int			gnb_AddByte(PT_GNB_HBUFFER, GNB_BYTE);
  int			gnb_AddWord(PT_GNB_HBUFFER, GNB_WORD);
  int			gnb_AddDWord(PT_GNB_HBUFFER, GNB_DWORD);
  int			gnb_AddInt4(PT_GNB_HBUFFER, GNB_INT4);
  int			gnb_AddDouble(PT_GNB_HBUFFER, GNB_DOUBLE);
  int			gnb_AddFloat(PT_GNB_HBUFFER, GNB_FLOAT);
  int			gnb_AddString(PT_GNB_HBUFFER, char*);
  int			gnb_ReadByte(PT_GNB_HBUFFER, PT_GNB_BYTE);
  int			gnb_ReadWord(PT_GNB_HBUFFER, PT_GNB_WORD);
  int			gnb_ReadDWord(PT_GNB_HBUFFER, PT_GNB_DWORD);
  int			gnb_ReadInt4(PT_GNB_HBUFFER, PT_GNB_INT4);
  int			gnb_ReadDouble(PT_GNB_HBUFFER, PT_GNB_DOUBLE);
  int			gnb_ReadFloat(PT_GNB_HBUFFER, PT_GNB_FLOAT);
  int			gnb_ReadString(PT_GNB_HBUFFER, char*);
  int			gnb_GetBufferSize(PT_GNB_HBUFFER, unsigned int*);
  int			gnb_GetBufferPointer(PT_GNB_HBUFFER, char**);

  void			gnb_SetWordToBigEndian(PT_GNB_WORD);
  void			gnb_SetBigEndianToWord(PT_GNB_WORD);
  void			gnb_SetDWordToBigEndian(PT_GNB_DWORD);
  void			gnb_SetBigEndianToDWord(PT_GNB_DWORD);
  void			gnb_SetInt4ToBigEndian(PT_GNB_INT4);
  void			gnb_SetBigEndianToInt4(PT_GNB_INT4);
  void			gnb_SetDoubleToBigEndian(PT_GNB_DOUBLE);
  void			gnb_SetBigEndianToDouble(PT_GNB_DOUBLE);
  void			gnb_SetFloatToBigEndian(PT_GNB_FLOAT);
  void			gnb_SetBigEndianToFloat(PT_GNB_FLOAT);

#else

#if defined(__cplusplus)
extern "C"
{
  int			gnb_Init(PT_GNB_HBUFFER, char*, unsigned int);
  int			gnb_ReOpen(PT_GNB_HBUFFER, char*, unsigned int);
  int			gnb_Open(PT_GNB_HBUFFER);
  int			gnb_Close(PT_GNB_HBUFFER);
  int			gnb_Seek(PT_GNB_HBUFFER, unsigned int);
  int			gnb_AddByte(PT_GNB_HBUFFER, GNB_BYTE);
  int			gnb_AddWord(PT_GNB_HBUFFER, GNB_WORD);
  int			gnb_AddDWord(PT_GNB_HBUFFER, GNB_DWORD);
  int			gnb_AddInt4(PT_GNB_HBUFFER, GNB_INT4);
  int			gnb_AddDouble(PT_GNB_HBUFFER, GNB_DOUBLE);
  int			gnb_AddFloat(PT_GNB_HBUFFER, GNB_FLOAT);
  int			gnb_AddString(PT_GNB_HBUFFER, char*);
  int			gnb_ReadByte(PT_GNB_HBUFFER, PT_GNB_BYTE);
  int			gnb_ReadWord(PT_GNB_HBUFFER, PT_GNB_WORD);
  int			gnb_ReadDWord(PT_GNB_HBUFFER, PT_GNB_DWORD);
  int			gnb_ReadInt4(PT_GNB_HBUFFER, PT_GNB_INT4);
  int			gnb_ReadDouble(PT_GNB_HBUFFER, PT_GNB_DOUBLE);
  int			gnb_ReadFloat(PT_GNB_HBUFFER, PT_GNB_FLOAT);
  int			gnb_ReadString(PT_GNB_HBUFFER, char*);
  int			gnb_GetBufferSize(PT_GNB_HBUFFER, unsigned int*);
  int			gnb_GetBufferPointer(PT_GNB_HBUFFER, char**);

  void			gnb_SetWordToBigEndian(PT_GNB_WORD);
  void			gnb_SetBigEndianToWord(PT_GNB_WORD);
  void			gnb_SetDWordToBigEndian(PT_GNB_DWORD);
  void			gnb_SetBigEndianToDWord(PT_GNB_DWORD);
  void			gnb_SetInt4ToBigEndian(PT_GNB_INT4);
  void			gnb_SetBigEndianToInt4(PT_GNB_INT4);
  void			gnb_SetDoubleToBigEndian(PT_GNB_DOUBLE);
  void			gnb_SetBigEndianToDouble(PT_GNB_DOUBLE);
  void			gnb_SetFloatToBigEndian(PT_GNB_FLOAT);
  void			gnb_SetBigEndianToFloat(PT_GNB_FLOAT);

}
#else

  extern int	gnb_Init(PT_GNB_HBUFFER, char*, unsigned int);
  extern int	gnb_ReOpen(PT_GNB_HBUFFER, char*, unsigned int);
  extern int	gnb_Open(PT_GNB_HBUFFER);
  extern int	gnb_Close(PT_GNB_HBUFFER);
  extern int	gnb_Seek(PT_GNB_HBUFFER, unsigned int);
  extern int	gnb_AddByte(PT_GNB_HBUFFER, GNB_BYTE);
  extern int	gnb_AddWord(PT_GNB_HBUFFER, GNB_WORD);
  extern int	gnb_AddDWord(PT_GNB_HBUFFER, GNB_DWORD);
  extern int	gnb_AddInt4(PT_GNB_HBUFFER, GNB_INT4);
  extern int	gnb_AddDouble(PT_GNB_HBUFFER, GNB_DOUBLE);
  extern int	gnb_AddFloat(PT_GNB_HBUFFER, GNB_FLOAT);
  extern int	gnb_AddString(PT_GNB_HBUFFER, char*);
  extern int	gnb_ReadByte(PT_GNB_HBUFFER, PT_GNB_BYTE);
  extern int	gnb_ReadWord(PT_GNB_HBUFFER, PT_GNB_WORD);
  extern int	gnb_ReadDWord(PT_GNB_HBUFFER, PT_GNB_DWORD);
  extern int	gnb_ReadInt4(PT_GNB_HBUFFER, PT_GNB_INT4);
  extern int	gnb_ReadDouble(PT_GNB_HBUFFER, PT_GNB_DOUBLE);
  extern int	gnb_ReadFloat(PT_GNB_HBUFFER, PT_GNB_FLOAT);
  extern int	gnb_ReadString(PT_GNB_HBUFFER, char*);
  extern int	gnb_GetBufferSize(PT_GNB_HBUFFER, unsigned int*);
  extern int	gnb_GetBufferPointer(PT_GNB_HBUFFER, char**);

  extern void	gnb_SetWordToBigEndian(PT_GNB_WORD);
  extern void	gnb_SetBigEndianToWord(PT_GNB_WORD);
  extern void	gnb_SetDWordToBigEndian(PT_GNB_DWORD);
  extern void	gnb_SetBigEndianToDWord(PT_GNB_DWORD);
  extern void	gnb_SetInt4ToBigEndian(PT_GNB_INT4);
  extern void	gnb_SetBigEndianToInt4(PT_GNB_INT4);
  extern void	gnb_SetDoubleToBigEndian(PT_GNB_DOUBLE);
  extern void	gnb_SetBigEndianToDouble(PT_GNB_DOUBLE);
  extern void	gnb_SetFloatToBigEndian(PT_GNB_FLOAT);
  extern void	gnb_SetBigEndianToFloat(PT_GNB_FLOAT);

#endif /* #if defined(__cplusplus) */

#endif /* #ifdef _GSOCKET_BUFFER_MODULE_ */

#endif /* #ifndef _GSOCKET_BUFFER_H_ */

