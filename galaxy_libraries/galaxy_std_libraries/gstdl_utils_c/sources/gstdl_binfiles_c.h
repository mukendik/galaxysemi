/*======================================================================*/
/* Galaxy                                                               */
/* Copyright 1996-98, Softlink                                          */
/* This computer program is protected by copyrigt law                   */
/* and international treaties. Unauthorized reproduction or             */
/* distribution of this program, or any portion of it,may               */
/* result in severe civil and criminal penalties, and will be           */
/* prosecuted to the maximum extent possible under the low.             */
/*======================================================================*/

/*======================================================================*/
/*========================== GBINFILES.H HEADER ========================*/
/*======================================================================*/
#ifndef _GBinFiles_h_

#define _GBinFiles_h_

#define GBF_MAX_ERRORMSG_LENGTH	256

/*======================================================================*/
/* Types of GBF Records :                                               */
/*======================================================================*/
#define RECTYPE_ALL    0
#define RECTYPE_FAR   10

/*======================================================================*/
/* gbf_XXX commands error codes returned                                */
/*======================================================================*/
#define GBF_ERR_OKAY		0		/* No Error : OKAY						*/
#define GBF_ERR_COF			1		/* Can't Open File                      */
#define GBF_ERR_FTS			2		/* File too small                       */
#define GBF_ERR_NFR			3		/* First record Not FAR Record          */
#define GBF_ERR_UEF			4		/* Unexpected End Of file.              */
#define GBF_ERR_MAF			5		/* Memory Allocation Failure            */
#define GBF_ERR_IRT			6		/* Invalid Record Type                  */
#define GBF_ERR_AOM			7		/* Access out of memory.                */
#define GBF_ERR_RNF			8		/* Record not found.                    */
#define GBF_ERR_FNF			9		/* File not found.                      */
#define GBF_ERR_NFL			10		/* No File Loaded.                      */
#define GBF_ERR_CWF			11		/* Can't create file                    */
#define GBF_ERR_NSQ			12		/* No sequencer found                   */
#define GBF_ERR_MAV			13		/* Memory access violation              */
#define GBF_ERR_CRF			14		/* Can't read file						*/
#define GBF_ERR_EBS			15		/* Exceed buffer size					*/
#define GBF_ERR_RLE			16		/* Record List empty 					*/
#define GBF_ERR_FAE			17		/* File Already exists					*/
#define GBF_ERR_REN			18		/* Error renaming file					*/
#define GBF_ERR_CNF			19		/* Cell not found in cell map			*/
#define GBF_ERR_EMR			20		/* Empty record							*/
#define GBF_ERR_LOGGMFNF	21		/* Log Gmf not found for specific Grf Gmf*/
#define GBF_ERR_BADHANDLE	22		/* Invalid handle passed */
#define GBF_ERR_CORRUPTEDREC	23	/* Corrupted record in file */
#define GBF_ERR_COPY		24		/* Error copying file					*/

/*======================================================================*/
/* gbf_XXX commands error messages		                                */
/*======================================================================*/
#define GBF_STR_ERR_OKAY			"No error"					
#define GBF_STR_ERR_MAF				"Memory allocation failure!"
#define GBF_STR_ERR_COF				"Couldn't open file:\n%s!"
#define GBF_STR_ERR_FTS				"Corrupted file (size):\n%s!"
#define GBF_STR_ERR_NFR				"Corrupted file (FAR):\n%s!"
#define GBF_STR_ERR_UEF				"Unexpected end of file:\n%s!"
#define GBF_STR_ERR_IRT				"Invalid record type %s!"
#define GBF_STR_ERR_AOM				"Access out of memory"
#define GBF_STR_ERR_RNF				"Record not found %s!"
#define GBF_STR_ERR_FNF				"File not found:\n%s!"
#define GBF_STR_ERR_NFL				"No file loaded!"
#define GBF_STR_ERR_CWF				"Couldn't write file:\n%s!"
#define GBF_STR_ERR_NSQ				"No sequencer found!"
#define GBF_STR_ERR_MAV				"Memory access violation!"
#define GBF_STR_ERR_CRF				"Can't read file:\n%s!"
#define GBF_STR_ERR_EBS				"Exceed buffer size!"
#define GBF_STR_ERR_FAE				"File already exists:\n%s!"
#define GBF_STR_ERR_RLE				"Record list empty!"
#define GBF_STR_ERR_REN				"Couldn't rename file:\n%s!"
#define GBF_STR_ERR_CNF				"Couldn't find cell in cell map!"
#define GBF_STR_ERR_UNKNOWN			"Unknown error code: %d!"
#define GBF_STR_ERR_EMR				"Empty record!"
#define GBF_STR_ERR_LOGGMFNF		"Couldn't find LOG GMF for:\n%s!"
#define GBF_STR_ERR_BADHANDLE		"Invalid handle!"
#define GBF_STR_ERR_CORRUPTEDREC	"Corrupted record in file: \n%s!"
#define GBF_STR_ERR_COPY			"Couldn't copy file:\n%s!"

/*======================================================================*/
/* definitions.                                                         */
/*======================================================================*/
typedef unsigned char   GBF_BYTE;
typedef unsigned short  GBF_WORD;
typedef unsigned int    GBF_DWORD;
typedef int	            GBF_INT4;

/* Record Structure */
struct TagGbfRec
{
  GBF_BYTE          *ptBuffer;    /* Pointer to record buffer           */
  unsigned int      nBuildOffset; /* Offset used during Buffer building */
  unsigned int      nAllocCount;  /* Nb of memory block allocations     */
  struct TagGbfRec  *ptNext;      /* Pointer to next GBF record         */
};
typedef struct TagGbfRec GBF_REC;      /* Gbf Record STRUCTURE          */
typedef struct TagGbfRec *PT_GBF_REC;  /* Pointer to GBF_REC            */

/* Record List Structure */
struct TagGbfRecList
{
  PT_GBF_REC  ptFirstRecord;  /* Pointer on first record                            */
  PT_GBF_REC  ptCurRecord;    /* Pointer on current record (fdor search functions)  */
  PT_GBF_REC  ptLastRecord;   /* Pointer on last record                             */
};
typedef struct TagGbfRecList GBF_RECLIST; /* Gbf Record list STRUCTURE  */
typedef struct TagGbfRecList *PT_GBF_RECLIST; /* Pointer to GBF_RECLIST */

/*======================================================================*/
/* Macros to support both PC and UNIX plateforms                        */
/*======================================================================*/
#if defined __unix__ || __APPLE__&__MACH__
/* UNIX macros */
#define	GBF_strcmp(a,b)		strcmp(a,b)
#define	GBF_stricmp(a,b)	strcasecmp(a,b)
#else
/* PC macros   */
#define	GBF_strcmp(a,b)		strcmp(a,b)
#define	GBF_stricmp(a,b)	stricmp(a,b)
#endif

/* Define ANSI under SOLARIS */
#if (defined(__STDC__) && !defined(ANSI)) || defined(_WIN32)
#define ANSI
#endif

/* Need to export functions? */
#if defined(_GBinFilesModule_)
#define _GBINFILE_EXPORT
#else
#define _GBINFILE_EXPORT	extern
#endif

/*======================================================================*/
/* Exported Functions.                                                  */
/*======================================================================*/
#if !defined(__cplusplus) && !defined(ANSI)

	_GBINFILE_EXPORT	int             gbf_LoadFromFile();
	_GBINFILE_EXPORT	int             gbf_LoadHeaderFromFile();
	_GBINFILE_EXPORT	int             gbf_SaveToFile();
	_GBINFILE_EXPORT	int             gbf_AppendToFile();
	_GBINFILE_EXPORT	int             gbf_RenameFile();
	_GBINFILE_EXPORT	PT_GBF_RECLIST  gbf_NewRecList();
	_GBINFILE_EXPORT	int             gbf_DeleteRecList();
	_GBINFILE_EXPORT	PT_GBF_REC      gbf_BeginRecord();
	_GBINFILE_EXPORT	int             gbf_EndRecord();
	_GBINFILE_EXPORT	int             gbf_FreeRecord();
	_GBINFILE_EXPORT	int             gbf_AddRecord();
	_GBINFILE_EXPORT	int             gbf_LoadNextRecordFromFile();
	_GBINFILE_EXPORT	int             gbf_FindFirstRecord();
	_GBINFILE_EXPORT	int             gbf_FindNextRecord();
	_GBINFILE_EXPORT	int             gbf_DeleteRecords();
	_GBINFILE_EXPORT	int             gbf_REC_AddStruct();
	_GBINFILE_EXPORT	int             gbf_REC_AddByte();
	_GBINFILE_EXPORT	int             gbf_REC_AddWord();
	_GBINFILE_EXPORT	int             gbf_REC_AddDWord();
	_GBINFILE_EXPORT	int             gbf_REC_AddInt4();
	_GBINFILE_EXPORT	int             gbf_REC_AddFloat();
	_GBINFILE_EXPORT	int             gbf_REC_AddString();
	_GBINFILE_EXPORT	int             gbf_REC_AddBuffer();
	_GBINFILE_EXPORT	int             gbf_REC_ChangeByte();
	_GBINFILE_EXPORT	int             gbf_REC_ChangeWord();
	_GBINFILE_EXPORT	int             gbf_REC_ChangeDWord();
	_GBINFILE_EXPORT	int             gbf_REC_ChangeInt4();
	_GBINFILE_EXPORT	int             gbf_REC_ChangeFloat();
	_GBINFILE_EXPORT	int             gbf_REC_ReadStruct();
	_GBINFILE_EXPORT	int             gbf_REC_ReadByte();
	_GBINFILE_EXPORT	int             gbf_REC_ReadWord();
	_GBINFILE_EXPORT	int             gbf_REC_ReadDWord();
	_GBINFILE_EXPORT	int             gbf_REC_ReadInt4();
	_GBINFILE_EXPORT	int             gbf_REC_ReadFloat();
	_GBINFILE_EXPORT	int             gbf_REC_ReadString();
	_GBINFILE_EXPORT	int			    gbf_REC_ReadBuffer();
	_GBINFILE_EXPORT	int			    gbf_GetErrorMessage();
	_GBINFILE_EXPORT	int 			gbf_ReallocBufferBySize();


#else

#if defined(__cplusplus) /* If __cplusplus defined, it means this module is exported! */
extern "C"
{
#endif /* defined(__cplusplus) */

  int             gbf_LoadFromFile(char *szFileName, PT_GBF_RECLIST *ptptRecList);
  int             gbf_LoadHeaderFromFile(char *szFileName, PT_GBF_RECLIST *ptptRecList, int *ptnPosInFile);
  int             gbf_SaveToFile(char *szFileName, PT_GBF_RECLIST ptRecList, int bOverWrite);
  int             gbf_AppendToFile(char *szFileName, PT_GBF_RECLIST ptRecList);
  int             gbf_RenameFile(char *szOldFileName, char *szNewFileName);
  PT_GBF_RECLIST  gbf_NewRecList(GBF_WORD wVersion);
  int             gbf_LoadNextRecordFromFile(char *szFileName, PT_GBF_RECLIST *ptptRecList, int *ptnPosInFile, int nNbRecordToLoad);
  int             gbf_FindFirstRecord(PT_GBF_RECLIST ptRecList, PT_GBF_REC *ptptRecord, GBF_WORD wRecordType);
  int             gbf_FindNextRecord(PT_GBF_RECLIST ptRecList, PT_GBF_REC *ptptRecord, GBF_WORD wRecordType);
  int             gbf_DeleteRecList(PT_GBF_RECLIST *ptptRecList);
  PT_GBF_REC      gbf_BeginRecord(GBF_WORD wRecordType);
  int             gbf_EndRecord(PT_GBF_REC ptRecord);
  int             gbf_FreeRecord(PT_GBF_REC *ptptRecord);
  int             gbf_AddRecord(PT_GBF_RECLIST ptRecList, PT_GBF_REC ptRecord);
  int             gbf_DeleteRecords(PT_GBF_RECLIST ptRecList, GBF_WORD wRecordType);
  int             gbf_REC_AddStruct(PT_GBF_REC ptRecord, GBF_BYTE *ptBuffer, int nBufferSize);
  int             gbf_REC_AddByte(PT_GBF_REC ptRecord, GBF_BYTE bData);
  int             gbf_REC_AddWord(PT_GBF_REC ptRecord, GBF_WORD wData);
  int             gbf_REC_AddDWord(PT_GBF_REC ptRecord, GBF_DWORD dwData);
  int             gbf_REC_AddInt4(PT_GBF_REC ptRecord, GBF_INT4 nData);
  int             gbf_REC_AddFloat(PT_GBF_REC ptRecord, float fData);
  int             gbf_REC_AddString(PT_GBF_REC ptRecord, char *szString);
  int			  gbf_REC_AddBuffer(PT_GBF_REC ptRecord, char *szBuffer, int nBufferSize);
  int             gbf_REC_ChangeByte(PT_GBF_REC ptRecord, int nOffset, GBF_BYTE bData);
  int             gbf_REC_ChangeWord(PT_GBF_REC ptRecord, int nOffset, GBF_WORD wData);
  int             gbf_REC_ChangeDWord(PT_GBF_REC ptRecord, int nOffset, GBF_DWORD dwData);
  int             gbf_REC_ChangeInt4(PT_GBF_REC ptRecord, int nOffset, GBF_INT4 nData);
  int             gbf_REC_ChangeFloat(PT_GBF_REC ptRecord, int nOffset, float fData);
  int             gbf_REC_ReadStruct(PT_GBF_REC ptRecord, int nOffset, GBF_BYTE *ptBuffer, int nBufferSize);
  int             gbf_REC_ReadByte(PT_GBF_REC ptRecord, int nOffset, GBF_BYTE *bData);
  int             gbf_REC_ReadWord(PT_GBF_REC ptRecord, int nOffset, GBF_WORD *wData);
  int             gbf_REC_ReadDWord(PT_GBF_REC ptRecord, int nOffset, GBF_DWORD *dwData);
  int             gbf_REC_ReadInt4(PT_GBF_REC ptRecord, int nOffset, GBF_INT4 *nData);
  int             gbf_REC_ReadFloat(PT_GBF_REC ptRecord, int nOffset, float *fData);
  int             gbf_REC_ReadString(PT_GBF_REC ptRecord, int nOffset, char *szString);
  int			  gbf_REC_ReadBuffer(PT_GBF_REC ptRecord, int nOffset, char **ppcBuffer, int *pnBufferSize);
  int			  gbf_GetErrorMessage(int nErrCode, char *szText,char *szErrorMessage);
  int			  gbf_ReallocBufferBySize(PT_GBF_REC ptRecord,int nNewSize);

#if defined(__cplusplus)
}
#endif /* defined(__cplusplus) */

#endif /* !defined(__cplusplus) && !defined(ANSI) */

#endif /* #ifndef _GBinFiles_h_ */

