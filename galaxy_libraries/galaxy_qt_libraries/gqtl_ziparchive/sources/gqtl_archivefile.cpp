////////////////////////////////////////////////////////////////////////////////
// ArchiveFile.cpp
////////////////////////////////////////////////////////////////////////////////
// Extract files from all compressed format and tar format
// Compress only in PkZip format
////////////////////////////////////////////////////////////////////////////////

// Project headers
#include "gqtl_pkzip.h"
#include "gqtl_archivefile.h"

// Standard headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

#if !defined __linux__ && !defined __APPLE__&__MACH__
#include <sys/utime.h>
#endif

#if defined __unix__ || __APPLE__&__MACH__
#include <unistd.h>
#include <sys/param.h>
#else
#include <direct.h>
#include <utime.h>  // William L ?
#endif

// QT headers
#include <QApplication>

#ifdef    DEF_ERRNO
extern int    errno;
#endif

#undef BYTEORDER
#undef NOALLIGN
#ifdef MSDOS            /* PC/XT/AT (8088) processor */
#    define    BYTEORDER     4321
#    define    NOALLIGN    1
#else
#    define    BYTEORDER    0000
#    define    NOALLIGN    0
#endif /* DOS */

/** machine variants which require cc -Dmachine:  pdp11, z8000, DOS*/

#ifdef interdata    /* Perkin-Elmer */
#    define SIGNED_COMPARE_SLOW    /* signed compare is slower than unsigned */
#endif


#ifndef    O_BINARY
#    define    O_BINARY    0    /* System has no binary mode */
#endif



#ifdef SIGNED_COMPARE_SLOW
typedef unsigned short int   count_short;
typedef unsigned long int    cmp_code_int;    /* Cast to make compare faster */
#else
typedef long int             cmp_code_int;
#endif


#define fx_read(a,b,c)  fread( (b),1,(c),(a))
#undef    min
#define   min(a,b)    ((a>b) ? b : a)
#define MAXCODE(n)    (1L << (n))

#if BYTEORDER == 4321 && NOALLIGN == 1
#define    input(b,o,c,n,m){\
	(c) = (*(long *)(&(b)[(o)>>3])>>((o)&0x7))&(m);\
	(o) += (n);}
#else
#define    input(b,o,c,n,m){\
	register char_type  *p = &(b)[(o)>>3];\
	(c) = ((((long)(p[0]))|((long)(p[1])<<8)|((long)(p[2])<<16))>>((o)&0x7))&(m);\
	(o) += (n);}
#endif

#ifndef GZ_SUFFIX
#  define GZ_SUFFIX ".gz"
#endif
#define SUFFIX_LEN (sizeof(GZ_SUFFIX)-1)

#define BUFLEN      16384
#define MAX_NAME_LEN 1024


#ifdef MAXSEG_64K
#error removed MAXSEG_64K
#else    /* Normal machine */
#    define HSIZE (1<<16) /* (1<<15) is few, (1<<16) seems to be OK, (1<<17) is proven safe OK */
#    define    tab_prefixof(i)         codetab[i]
#    define    tab_suffixof(i)         htab[i]
#    define    de_stack                (htab+(sizeof(htab[0])*(2*HSIZE-1)))
#    define    clear_tab_prefixof()    memset(codetab, 0, 256*sizeof(codetab[0]));
#endif    /* MAXSEG_64K */

#define	isodigit(c)	( ((c) >= '0') && ((c) <= '7') )


/* tar file */
static int			xU_Open4Read(struct Readable* self, char const* filename) { return NULL==(self->f=fopen(filename,"rb")); }
static int			xU_Close(struct Readable* self) { return fclose((FILE*)self->f); }
static unsigned		xU_Read(struct Readable* self, void* buf, unsigned len) {	unsigned got=fread(buf,1,len,(FILE*)self->f);	return got>0 ? got : ferror((FILE*)self->f) ? 0U-1 : 0;}
static char const*	xU_Error(struct Readable* self, int *errnum_ret) { *errnum_ret=ferror((FILE*)self->f); return "I/O error"; }

/* lz file */
struct lz_reader_t;
static struct lz_reader_t* lz_read_new(FILE* inf);
static int			lz_read_init(struct lz_reader_t* reader, FILE* inf);
static int			lz_close(struct lz_reader_t* reader);
static unsigned		lz_read(struct lz_reader_t* reader, char*buf, unsigned len);

/* Z file */
static int			xZ_Open4Read(struct Readable* self, char const* filename) {	FILE *f=fopen(filename,"rb");	if (NULL==f) return 1;	return NULL==(self->f=lz_read_new(f));}
static int			xZ_Close(struct Readable* self) {int ret=lz_close((struct lz_reader_t*)self->f);	free((struct lz_reader_t*)self->f);	return ret;}
static unsigned		xZ_Read(struct Readable* self, void* buf, unsigned len) {return lz_read((struct lz_reader_t*)self->f,(char *) buf, len);}
static char const*	xZ_Error(struct Readable* self, int *errnum_ret) {(void)self;*errnum_ret=0;	return "lzw error";}

/* gz file */
static int			xGZ_Open4Read(struct Readable* self, char const* filename) { return NULL==(self->f=gzopen(filename,"rb")); }
static int			xGZ_Close(struct Readable* self) { return gzclose((gzFile)self->f); }
static unsigned		xGZ_Read(struct Readable* self, void* buf, unsigned len) { return gzread((gzFile)self->f,buf,len); }
static char const*
xGZ_Error(struct Readable* /*self*/,
          int* /*errnum_ret*/)
{ return "gzerror((gzFile)self->f, errnum_ret)"; }

#if HAVE_BZ2LIB
#include "bzlib.h"

/* #define xFILE BZFILE* */
static int			xBZ2_Open4Read(struct Readable* self, char const* filename) { return NULL==(self->f=BZ2_bzopen(filename,"rb")); }
static int			xBZ2_Close(struct Readable* self) { BZ2_bzclose((BZFILE*)self->f); return 0; }
static unsigned		xBZ2_Read(struct Readable* self, void* buf, unsigned len) { return BZ2_bzread((BZFILE*)self->f,buf,len); }
static char const*	xBZ2_Error(struct Readable* self, int *errnum_ret) { return BZ2_bzerror((BZFILE*)self->f, errnum_ret); }
#endif

static struct lz_reader_t* lz_read_new(FILE *inf) 
{
	lz_reader_t* reader=(lz_reader_t*)malloc(sizeof(lz_reader_t));
	reader->fdin=NULL;
	if (reader!=NULL) {
		if (lz_read_init(reader, inf)!=0) {
			lz_close(reader);
			free(reader); reader=NULL;
		}
	}
	return reader;
}

static int lz_close(struct lz_reader_t* reader) {
	FILE *fin=reader->fdin;
	reader->rsize=-1;
	if (reader->inbuf!=NULL)   { free(reader->inbuf  ); reader->inbuf  =NULL; }
	if (reader->htab!=NULL)    { free(reader->htab   ); reader->htab   =NULL; }
	if (reader->codetab!=NULL) { free(reader->codetab); reader->codetab=NULL; }
	if (fin==NULL) return 0;
	reader->fdin=NULL;
	return fclose(fin);
}

//////////////////////////////////////////////////////////////////////
/** @param Zreader uninitialized
* @return 0 on success
*/
static int lz_read_init(struct lz_reader_t* reader, FILE *inf) 
{
	char_type* htab;
	unsigned short* codetab;
	code_int codex;
	reader->fdin=inf;
	if (NULL==(reader->inbuf=(unsigned char*)malloc(IBUFSIZ+67))) return 1;
	if (NULL==(htab=reader->htab=(unsigned char*)malloc(2*HSIZE))) return 1; /* Dat: HSIZE for the codes and HSIZE for de_stack */
	if (NULL==(codetab=reader->codetab=(unsigned short*)malloc(sizeof(reader->codetab[0])*HSIZE))) return 1;
	reader->insize=0;
	reader->rsize=0;
	while (reader->insize < 3 && (reader->rsize = fx_read(reader->fdin, reader->inbuf+reader->insize, IBUFSIZ)) > 0)
		reader->insize += reader->rsize;
	
	if (reader->insize < 3
		|| reader->inbuf[0] != (char_type)'\037' /* First byte of compressed file */
		|| reader->inbuf[1] != (char_type)'\235' /* Second byte of compressed file */
		) return reader->insize >= 0 ? 2 : 3;
	
	reader->maxbits = reader->inbuf[2] & BIT_MASK;
	reader->blkmode = reader->inbuf[2] & BLOCK_MODE;
	reader->maycode = MAXCODE(reader->maxbits);
	
	if (reader->maxbits > BITS) return 4;
	
	reader->maxcode = MAXCODE(reader->n_bits = INIT_BITS)-1;
	reader->bitmask = (1<<reader->n_bits)-1;
	reader->oldcode = -1;
	reader->finchar = 0;
	reader->posbits = 3<<3;
	reader->stackp  = NULL;
	reader->code    = -1;
	reader->incode  = -1; /* fake */
	reader->inbits  = 0;  /* fake */
	
	reader->freeent = ((reader->blkmode) ? FIRST : 256);
	
	clear_tab_prefixof();    /* As above, initialize the first 256 entries in the table. */
	
	for (codex = 255 ; codex >= 0 ; --codex)
		tab_suffixof(codex) = (char_type)codex;
	return 0; /* success */
}


//////////////////////////////////////////////////////////////////////
/*
* Decompress stdin to stdout.  This routine adapts to the codes in the
* file building the "string" table on-the-fly; requiring no table to
* be stored in the compressed file.  The tables used herein are shared
* with those of the compress() routine.  See the definitions above.
*/
static unsigned lz_read(struct lz_reader_t* reader, char* outbuf, unsigned obufsize) {
	register    char_type  *stackp;
	register    code_int   code;
	register    int        finchar;
	register    code_int   oldcode;
	register    code_int   incode;
	register    int        inbits;
	register    int        posbits;
	register   int         insize;
	register   int         bitmask;
	register   code_int    freeent;
	register   code_int    maxcode;
	register   code_int    maycode;
	register   int         n_bits;
	register   int         rsize;
	
	int      blkmode;
	int      maxbits;
	char_type* inbuf;
	char_type* htab;
	unsigned short* codetab;
	FILE* fdin;
	register unsigned outpos=0; /* not restored */
	
	if (reader->rsize<1) 
		return reader->rsize; /* already EOF or error */
	
	stackp =reader->stackp;
	code   =reader->code;
	finchar=reader->finchar;
	oldcode=reader->oldcode;
	incode =reader->incode;
	inbits =reader->inbits;
	posbits=reader->posbits;
	insize =reader->insize;
	bitmask=reader->bitmask;
	freeent=reader->freeent;
	maxcode=reader->maxcode;
	maycode=reader->maycode;
	n_bits =reader->n_bits;
	rsize  =reader->rsize;
	blkmode=reader->blkmode;
	maxbits=reader->maxbits;
	htab   =reader->htab;
	codetab=reader->codetab;
	fdin   =reader->fdin;
	inbuf  =reader->inbuf;
	if (oldcode!=-1) goto try_fit; /* lz_read() called again */
	
	do {
resetbuf: ;
		  {
			  register int    i;
			  int   e;
			  int   o;
			  
			  e = insize-(o = (posbits>>3));
			  
			  for (i = 0 ; i < e ; ++i)
				  inbuf[i] = inbuf[i+o];
			  
			  insize = e;
			  posbits = 0;
		  }
		  
		  if ((unsigned)insize < 64) 
		  {
			  if ((rsize = fx_read(fdin, inbuf+insize, IBUFSIZ)) < 0 || ferror(fdin)) 
				  return reader->rsize=-1;
			  insize += rsize;
			  inbuf[insize]=inbuf[insize+1]=inbuf[insize+2]=0;
		  }
		  
		  inbits = ((rsize > 0) ? (insize - insize%n_bits)<<3 :
		  (insize<<3)-(n_bits-1));
		  
		  while (inbits > posbits) 
		  {
			  if (freeent > maxcode) 
			  {
				  posbits = ((posbits-1) + ((n_bits<<3) -
					  (posbits-1+(n_bits<<3))%(n_bits<<3)));
				  ++n_bits;
				  if (n_bits == maxbits)
					  maxcode = maycode;
				  else
					  maxcode = MAXCODE(n_bits)-1;
				  
				  bitmask = (1<<n_bits)-1;
				  goto resetbuf;
			  }
			  
			  input(inbuf,posbits,code,n_bits,bitmask);
			  
			  if (oldcode == -1) 
			  {
				  outbuf[outpos++] = (char_type)(finchar = (int)(oldcode = code));
				  continue;
			  }
			  /* Dat: oldcode will never be -1 again */
			  
			  if (code == CLEAR && blkmode) 
			  {
				  clear_tab_prefixof();
				  freeent = FIRST - 1;
				  posbits = ((posbits-1) + ((n_bits<<3) -
					  (posbits-1+(n_bits<<3))%(n_bits<<3)));
				  maxcode = MAXCODE(n_bits = INIT_BITS)-1;
				  bitmask = (1<<n_bits)-1;
				  goto resetbuf;
			  }
			  
			  incode = code;
			  stackp = de_stack;
			  
			  if (code >= freeent) 
			  {    /* Special case for KwKwK string.    */
				  if (code > freeent) 
				  {
					  //register char_type         *p;
					  
					  //posbits -= n_bits;
					  //p = &inbuf[posbits>>3];
					  return reader->rsize=-1;
				  }
				  
				  *--stackp = (char_type)finchar;
				  code = oldcode;
			  }
			  
			  while ((cmp_code_int)code >= (cmp_code_int)256) 
			  {
				  /* Generate output characters in reverse order */
				  *--stackp = tab_suffixof(code);
				  code = tab_prefixof(code);
			  }
			  *--stackp =    (char_type)(finchar = tab_suffixof(code));
			  Q_ASSERT(outpos<=obufsize);
			  
			  /* And put them out in forward order */
			  {
				  register int    i;
				  
try_fit: if (outpos+(i = (de_stack-stackp)) >= obufsize) 
		 {
			 /* The entire stack doesn't fit into the outbuf */
			 i=obufsize-outpos;
			 memcpy(outbuf+outpos, stackp, i);
			 stackp+=i;
			 /* vvv Dat: blkmode, maycode, inbuf, htab, codetab, fdin need not be saved */
			 reader->stackp =stackp;
			 reader->code   =code;
			 reader->finchar=finchar;
			 reader->oldcode=oldcode;
			 reader->incode =incode;
			 reader->inbits =inbits;
			 reader->posbits=posbits;
			 reader->insize =insize;
			 reader->bitmask=bitmask;
			 reader->freeent=freeent;
			 reader->maxcode=maxcode;
			 reader->maycode=maycode;
			 reader->n_bits =n_bits;
			 reader->rsize  =rsize;
			 reader->blkmode=blkmode;
			 reader->maxbits=maxbits;
			 return obufsize;
			 /* Dat: co-routine return back to try_fit, with outpos==0, outbuf=... obufsize=... */
		 } else 
		 {
			 memcpy(outbuf+outpos, stackp, i);
			 outpos += i;
		 }
	  }
	  /* Dat: ignore stackp from now */
	  
	  if ((code = freeent) < maycode) 
	  { /* Generate the new entry. */
		  tab_prefixof(code) = (unsigned short)oldcode;
		  tab_suffixof(code) = (char_type)finchar;
		  freeent = code+1;
	  }
	  oldcode = incode;    /* Remember previous code.    */
    }
  } while (rsize > 0);
  reader->rsize=0;
  return outpos;
}


////////////////////////////////////////////////////////////////////////////////
// from_oct
////////////////////////////////////////////////////////////////////////////////
// Quick and dirty octal conversion.
//
// Result is -1 if the field is invalid (all blank, or nonoctal).
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::from_oct(int digs, char *where)
{
	int	value;
	
	while (*where==' ' || *where=='\f' || *where=='\n' || *where=='\r' || *where=='\t' || *where=='\v') {
		where++;
		if (--digs <= 0)
			return -1;		/* All blank field */
	}
	value = 0;
	while (digs > 0 && isodigit(*where)) {	/* Scan til nonoctal */
		value = (value << 3) | (*where++ - '0');
		--digs;
	}
	
	if (digs > 0 && *where!='\0' && *where!=' ' && *where!='\f' && *where!='\n' && *where!='\r' && *where!='\t' && *where!='\v')
		return -1;			/* Ended on non-space/nul */
	
	return value;
}


////////////////////////////////////////////////////////////////////////////////
// IsTar
////////////////////////////////////////////////////////////////////////////////
// Return
//	0 if the checksum is bad (i.e., probably not a tar archive),
//	1 for old UNIX tar file,
//	2 for Unix Std (POSIX) tar file.
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::IsTar(char *buf, unsigned nbytes)
{
	union tar_record *header = (union tar_record *)buf;
	int	i;
	int	sum, recsum;
	char	*p;
	
	if (nbytes < sizeof(union tar_record))
		return 0;
	
	recsum = from_oct(8,  header->header.chksum);
	
	sum = 0;
	p = header->charptr;
	for (i = sizeof(union tar_record); --i >= 0;) 
		sum += 0xFF & *p++;
	
	/* Adjust checksum to count the "chksum" field as blanks. */
	for (i = sizeof(header->header.chksum); --i >= 0;)
		sum -= 0xFF & header->header.chksum[i];
	sum += ' '* sizeof header->header.chksum;
	
	if (sum != recsum)
		return 0;	/* Not a tar archive */
	
	if (0==strcmp(header->header.magic, TMAGIC))
		return 2;		/* Unix Standard tar archive */
	
	return 1;			/* Old fashioned tar archive */
}


////////////////////////////////////////////////////////////////////////////////
// xClose
// return true on success, false else
////////////////////////////////////////////////////////////////////////////////
bool CArchiveFile::xClose(void) 
{
	if (m_SelfReadable.xClose(&m_SelfReadable))
		return false;

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// xOpen4Read
// return true on success, positive for various failure reasons
////////////////////////////////////////////////////////////////////////////////
int CArchiveFile::xOpen4Read(char const* filename) 
{
	unsigned i;
	char buf[RECORDSIZE];

	FILE *f=fopen(filename, "rb");
	if (f==NULL) 
		return 4;
	i=sizeof(buf); 
	while (i--!=0) 
		buf[i]='\0';
	if (5>fread(buf, 1, sizeof(buf), f)) 
		return 2;
	
	if(buf[0]==0037 && (buf[1]&255)==0213 && (buf[2]&255)<=8) 
	{
		fclose(f);
		m_SelfReadable.xOpen4Read=xGZ_Open4Read;
		m_SelfReadable.xClose=xGZ_Close;
		m_SelfReadable.xRead=xGZ_Read;
		m_SelfReadable.xError=xGZ_Error;
		if (xGZ_Open4Read(&m_SelfReadable, filename)!=0) return 1;
	} 
#if HAVE_BZ2LIB
	else if(buf[0]==0102 && buf[1]==0132 && buf[2]==0150) 
	{
		fclose(f);
		m_SelfReadable.xOpen4Read=xBZ2_Open4Read;
		m_SelfReadable.xClose=xBZ2_Close;
		m_SelfReadable.xRead=xBZ2_Read;
		m_SelfReadable.xError=xBZ2_Error;
		if (xBZ2_Open4Read(&m_SelfReadable, filename)!=0) return 1;
	} 
#endif
	else if(buf[0]==0037 && (buf[1]&255)==0235) 
	{
		fclose(f);
		m_SelfReadable.xOpen4Read=xZ_Open4Read;
		m_SelfReadable.xClose=xZ_Close;
		m_SelfReadable.xRead=xZ_Read;
		m_SelfReadable.xError=xZ_Error;
		if (xZ_Open4Read(&m_SelfReadable, filename)!=0) return 1;
	} 
	else if((buf[257]==0165 && buf[258]==0163 && buf[259]==0164	&& buf[260]==0141 && buf[261]==0162
			&& (buf[262]==0000 || (buf[262]==0040 && buf[263]==0040 && buf[264]==0))) 
		|| IsTar(buf, sizeof(buf))) 
	{
		//rewind(f);	// Unexplained crash on a .tar file containing .gz files. Replaced by fclose() + fopen()
		fclose(f);
		f=fopen(filename, "rb");
		m_SelfReadable.f=f; 
		m_SelfReadable.xOpen4Read=xU_Open4Read;
		m_SelfReadable.xClose=xU_Close;
		m_SelfReadable.xRead=xU_Read;
		m_SelfReadable.xError=xU_Error;
	} 
	else return 3;
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// IsCompressedFile
////////////////////////////////////////////////////////////////////////////////
// return true if this file is in a known compressed format
////////////////////////////////////////////////////////////////////////////////
bool CArchiveFile::IsCompressedFile(QString strFileName) 
{
	char		buf[RECORDSIZE];
	unsigned	i;
	CAnsiFile	cFile;
	if(!cFile.Open(strFileName, CAnsiFile::modeNoTruncate | CAnsiFile::modeRead))
		return Error(cFile.GetLastErrorMsg().toLatin1().data());

	i = cFile.Read(buf,RECORDSIZE);
	cFile.Close();
	if (5>i) // too short
		return Error((char*)"not a compressed file");

	if(buf[0]==0037 && (buf[1]&255)==0213 && (buf[2]&255)<=8) 
		return true;
#if HAVE_BZ2LIB
	else if(buf[0]==0102 && buf[1]==0132 && buf[2]==0150) 
		return true;
#endif
	else if(buf[0]==0037 && (buf[1]&255)==0235) 
		return true;
	else if((buf[257]==0165 && buf[258]==0163 && buf[259]==0164	&& buf[260]==0141 && buf[261]==0162
			&& (buf[262]==0000 || (buf[262]==0040 && buf[263]==0040 && buf[264]==0))) 
		|| IsTar(buf, sizeof(buf))) 
		return true;

	if(CPkZip::IsPkZipFile(strFileName))
		return true;

    return Error((char*)"not a compressed file");
}

bool CArchiveFile::ExtractFileList(const QString &lArchiveFilename, QStringList &lCompressedFiles)
{
    QString	lAbsFileName = m_cFileUtils.GetAbsFileName(lArchiveFilename.toLatin1().constData());

    m_bError        = false;
    m_strErrorMsg.clear();

    if(!m_cFileUtils.FileExists(lAbsFileName.toLatin1().constData()))
        return Error(m_cFileUtils.GetLastErrorMsg().toLatin1().data());

    if(!IsCompressedFile(lAbsFileName))
        return false;

    if(CPkZip::IsPkZipFile(lAbsFileName))
    {
        CPkZip      lPkZip;

        if(!lPkZip.ExtractFiles(lAbsFileName, lCompressedFiles, true))
        {
            return Error(lPkZip.GetLastErrorMsg().toLatin1().data());
        }
    }
    else
    {
        m_strErrorMsg = "";
        switch (xOpen4Read(lAbsFileName.toLatin1().constData()))
        {
            case 0: break;
            case 1: return Error((char*)"cannot decode compressed file");
            case 2: return Error((char*)"file too short");
            case 3: return Error((char*)"not a compressed file");
            case 4: return Error((char*)"cannot open compressed file");
        }

        ExtractFile(lAbsFileName.toLatin1().constData(), true);

        xClose();
    }

    return !m_bError;
}

////////////////////////////////////////////////////////////////////////////////
// Error
////////////////////////////////////////////////////////////////////////////////
bool CArchiveFile::Error(char* msg) 
{
	m_strErrorMsg = "Zip file error";
	if((msg != NULL) && (strlen(msg) != 0))
	{
		m_strErrorMsg += ": ";
		m_strErrorMsg += msg;
	}

	m_bError = true;
	return false;
}


static int getoct(char *p,int width)
{
	int result = 0;
	char c;
	
	while (width --)
    {
		c = *p++;
		if (c == ' ')
			continue;
		if (c == 0)
			break;
		result = result * 8 + (c - '0');
    }
	return result;
}



////////////////////////////////////////////////////////////////////////////////
// ExtractFile
////////////////////////////////////////////////////////////////////////////////
// Compressed file extraction
////////////////////////////////////////////////////////////////////////////////
bool CArchiveFile::ExtractFile(char const* TGZfile, bool lListOnly /* = false */)
{
	union  tar_buffer buffer;
	int    is_tar_ok=0;
    unsigned int len;
	int    err;
	int    getheader = 1;
	int    remaining = 0;
	FILE   *outfile = NULL;
	QString strFileName;
	QString strAbsFileName;
	time_t tartime = 0;
	
	m_bError = false;
	m_strErrorMsg = "";

	while (1) 
	{
        if(m_bError)
            return false;

        len = m_SelfReadable.xRead(&m_SelfReadable, &buffer, BLOCKSIZE);
        if (len+1 == 0)
            return Error((char*)m_SelfReadable.xError(&m_SelfReadable, &err));
        if (!is_tar_ok && !(is_tar_ok=IsTar(buffer.buffer, len)))
        {
            // comressed file not tared
            FILE *of=0;
            QString strExt = m_cFileUtils.GetFileExt(TGZfile);
            strFileName = m_cFileUtils.GetFileName(TGZfile);
            if(strExt == "")
                strFileName += ".unc";
            else
                strFileName = strFileName.left(strFileName.length() - strExt.length() - 1);

            if (lListOnly == false)
            {
                strAbsFileName = m_cFileUtils.PathCat(m_strExtractPath.toLatin1().constData(), strFileName.toLatin1().constData());

                if (NULL==(of=fopen(strAbsFileName.toLatin1().constData(), "wb")))
                    return Error(strerror(errno));
                // Add to list of created files
                m_strlFilesCreated.append(strAbsFileName);
            }

            do
            {
                if (lListOnly == false)
                {
                    if (fwrite(buffer.buffer, 1, len, of) != len)
                    {
                        fclose(of);
                        return Error(strerror(errno));
                    }
                    // Process application's events to prevent freezing GUI
                    QCoreApplication::processEvents();
                }
            }
            while (0<(len=m_SelfReadable.xRead(&m_SelfReadable, &buffer, BLOCKSIZE))); /* Imp: larger blocks */

            if (lListOnly == false && of != NULL)
            {
                len=ferror(of);
                if (0!=(len|fclose(of)))
                    return Error(strerror(errno));
            }

            if(m_plstFilesExtracted)
                m_plstFilesExtracted->append(strFileName);

            return true;
        }
        /*
        * Always expect complete blocks to process
        * the tar information.
        */
        if (len != BLOCKSIZE)
            return Error();

		/*
		* If we have to get a tar header
		*/
		if (getheader == 1)
		{
			/*
			* if we met the end of the tar
			* or the end-of-tar block,
			* we are done
			*/
			if ((len == 0)  || (buffer.header.name[0]== 0)) break;
			
			tartime = (time_t)getoct(buffer.header.mtime,12);
			strFileName = m_cFileUtils.NormalizePath(buffer.header.name);
			strAbsFileName = m_cFileUtils.PathCat(m_strExtractPath.toLatin1().constData(),strFileName.toLatin1().constData());
			emit sUncompressBegin(strAbsFileName);
            switch (buffer.header.typeflag)
            {
                case DIRTYPE:
                    if(m_bFullPath && lListOnly == false)
                    {
                        if(!m_cFileUtils.CreateDirectory(strAbsFileName.toLatin1().constData()))
                            return Error(m_cFileUtils.GetLastErrorMsg().toLatin1().data());
                    }
                    break;
                case REGTYPE:
                case AREGTYPE:
                    remaining = getoct(buffer.header.size,12);

                    if (lListOnly == false)
                    {
                        if (remaining)
                        {
                            if(!m_bFullPath && m_bUniqueName)
                                strFileName.replace(CHAR_SEP,'_');

                            strAbsFileName = m_cFileUtils.PathCat(m_strExtractPath.toLatin1().constData(),strFileName.toLatin1().constData());
                            if(!m_cFileUtils.CreateDirectory(m_cFileUtils.GetFilePath(strAbsFileName.toLatin1().constData()).toLatin1().constData()))
                                return Error(m_cFileUtils.GetLastErrorMsg().toLatin1().data());

                            outfile = fopen(strAbsFileName.toLatin1().constData(),"wb");
                            if(outfile != NULL)
                            {
                                // Add to list of created files, and to list of extracted files
                                m_strlFilesCreated.append(strAbsFileName);
                                m_plstFilesExtracted->append(strFileName);
                            }
                        }
                        else
                            outfile = NULL;
                        /*
                        * could have no contents
                        */
                    }

                    getheader = (remaining) ? 0 : 1;
                    break;
                default:
                    break;
            }
		}
		else
		{
			unsigned int bytes = (remaining > BLOCKSIZE) ? BLOCKSIZE : remaining;
			
            if (lListOnly == false && outfile != NULL)
            {
                if (fwrite(&buffer,sizeof(char),bytes,outfile) != bytes)
                {
                    fclose(outfile);
                    outfile = NULL;
                    UnLink(strAbsFileName.toLatin1().constData());
                }
            }
			remaining -= bytes;
			if (remaining == 0)
			{
				getheader = 1;
                if (lListOnly == false && outfile != NULL)
				{
					struct utimbuf settime;
					
					settime.actime = settime.modtime = tartime;
					
					fclose(outfile);
					outfile = NULL;
					utime(strAbsFileName.toLatin1().constData(),&settime);
				}
			}
		}

		// Process application's events, to prvent freezing the GUI
        QCoreApplication::processEvents();
    }
	
	return true;
}


////////////////////////////////////////////////////////////////////////////////
// CArchiveFile
////////////////////////////////////////////////////////////////////////////////
CArchiveFile::CArchiveFile(QString strExtractPath, bool bFullPath, bool bUniqueName)
{
	QString strPath = m_cFileUtils.GetAbsFileName(strExtractPath.toLatin1().constData());
	m_strExtractPath = m_cFileUtils.NormalizePath(strPath.toLatin1().constData());
	m_bFullPath = bFullPath;
	m_bUniqueName = !m_bFullPath && bUniqueName;
	m_plstFilesExtracted = NULL;
	m_bError = false;
}

void CArchiveFile::Config(QString strExtractPath, bool bFullPath, bool bUniqueName)
{
	QString strPath = m_cFileUtils.GetAbsFileName(strExtractPath.toLatin1().constData());
	m_strExtractPath = m_cFileUtils.NormalizePath(strPath.toLatin1().constData());
	m_bFullPath = bFullPath;
	m_bUniqueName = !m_bFullPath && bUniqueName;
}

////////////////////////////////////////////////////////////////////////////////
// CArchiveFile
////////////////////////////////////////////////////////////////////////////////
CArchiveFile::~CArchiveFile()
{

}

////////////////////////////////////////////////////////////////////////////////
// Uncompress (public)
////////////////////////////////////////////////////////////////////////////////
bool CArchiveFile::Uncompress(QString strFile, QStringList &lstFilesExtracted)
{
	bool	bStatus;
	int		uiIndex;
	
	m_plstFilesExtracted = &lstFilesExtracted;
	m_strlFilesCreated.clear();

	// Call recursive uncompress function
	bStatus = Uncompress(strFile);
	if(!bStatus)
	{
		// If some files were created, remove them
		for(uiIndex = 0; uiIndex < m_strlFilesCreated.count(); uiIndex++)
			remove(m_strlFilesCreated[uiIndex].toLatin1().constData());

		// Clear list of extracted files
		lstFilesExtracted.clear();
	}

	return bStatus;
}

////////////////////////////////////////////////////////////////////////////////
// Uncompress (private, recursive)
////////////////////////////////////////////////////////////////////////////////
bool CArchiveFile::Uncompress(QString strFile)
{
	emit sUncompressBegin(strFile) ;
	QString strExtractPath = m_strExtractPath;
	int		iIndex;

	QString	strAbsFileName = m_cFileUtils.GetAbsFileName(strFile.toLatin1().constData());

	m_bError = false;
	m_strErrorMsg = "";

	if(!m_cFileUtils.FileExists(strAbsFileName.toLatin1().constData()))
		return Error(m_cFileUtils.GetLastErrorMsg().toLatin1().data());

	if(!IsCompressedFile(strAbsFileName))
		return false;
	
	if(m_strExtractPath.isEmpty())
	{
		// Extract in the same dir than the original file
		strExtractPath = m_cFileUtils.GetFilePath(strAbsFileName.toLatin1().constData());
	}
	else 
	if(!m_cFileUtils.CreateDirectory((char*)m_strExtractPath.toLatin1().constData()))
		return Error(m_cFileUtils.GetLastErrorMsg().toLatin1().data());//"cannot create directory"


	if(CPkZip::IsPkZipFile(strAbsFileName))
	{
        CPkZip cPkZip(strExtractPath, m_bFullPath, m_bUniqueName);
		QStringList lstFiles;
        if(!cPkZip.ExtractFiles(strAbsFileName, lstFiles))
		{
			// Check if some files were extracted
			if(lstFiles.count() > 0)
				m_strlFilesCreated += lstFiles;
			return Error(cPkZip.GetLastErrorMsg().toLatin1().data());
		}

		for(iIndex = 0; iIndex < lstFiles.count(); iIndex++)
		{
			QString strZipFile = m_cFileUtils.PathCat(strExtractPath.toLatin1().constData(), lstFiles[iIndex].toLatin1().constData());
			if(IsCompressedFile(strZipFile))
			{
				// Uncompress file
				if(!Uncompress(strZipFile))
					return false;

				// Success: remove compressed file, and remove entry in list of files created
				remove(strZipFile.toLatin1().constData());
				m_strlFilesCreated.removeOne(strZipFile);
			}
			else
				m_plstFilesExtracted->append(lstFiles[iIndex]);
		}
		
		lstFiles.clear();
		return true;
	}

#if DOSISH
    setmode(0, O_BINARY);
#endif

	
	m_strErrorMsg = "";
	switch (xOpen4Read(strAbsFileName.toLatin1().constData())) 
	{
		case 0: break;
		case 1: return Error((char*)"cannot decode compressed file");
		case 2: return Error((char*)"file too short");
		case 3: return Error((char*)"not a compressed file");
		case 4: return Error((char*)"cannot open compressed file");
	}
	
	if(m_strExtractPath.isEmpty())
	{
		m_strExtractPath = strExtractPath;
		strExtractPath = "";
	}

	ExtractFile(strFile.toLatin1().constData());
	xClose();
	m_strExtractPath = strExtractPath;
	return !m_bError;
}

bool CArchiveFile::Compress(QString strFile, QString &strZipFile)
{
	QString	strAbsFileName = m_cFileUtils.GetAbsFileName(strFile.toLatin1().constData());
	QString	strAbsZipFile;

	m_bError = false;
	m_strErrorMsg = "";
	
	if(!m_cFileUtils.FileExists(strAbsFileName.toLatin1().constData()))
		return Error(m_cFileUtils.GetLastErrorMsg().toLatin1().data());

	if(IsCompressedFile(strAbsFileName))
		return Error((char*)"File already compressed");
	
    FILE  *in;
    gzFile out;
	char outmode[20];

    char buf[BUFLEN];
    int len;
	
	strcpy(outmode, "wb9f");
	if(m_strExtractPath.isEmpty())
		strAbsZipFile = strZipFile = m_cFileUtils.NormalizePath(strAbsFileName.toLatin1().constData()) + GZ_SUFFIX;
	else
	{
		// compress the file in a new folder
		if(!m_cFileUtils.CreateDirectory(m_strExtractPath.toLatin1().constData()))
			return Error(m_cFileUtils.GetLastErrorMsg().toLatin1().data());

		strZipFile = m_cFileUtils.GetFileName(strAbsFileName.toLatin1().constData()) + GZ_SUFFIX;
		strAbsZipFile = m_cFileUtils.PathCat(m_strExtractPath.toLatin1().constData(),strZipFile.toLatin1().constData());
	}

    in = fopen(strAbsFileName.toLatin1().constData(), "rb");
    if (in == NULL)
		return Error(strerror(errno));

    out = gzopen(strAbsZipFile.toLatin1().constData(), outmode);
    if (out == NULL) 
		return false;

	

    for (;;) {
        len = fread(buf, 1, sizeof(buf), in);
        if (ferror(in))
			return Error(strerror(errno));

        if (len == 0) break;

        if (gzwrite(out, buf, (unsigned)len) != len) 
			return false;

		// Process application's events to prevent freezing the GUI
        QCoreApplication::processEvents();
    }
    fclose(in);
    if (gzclose(out) != Z_OK) 
		return false;
	return true;
}

