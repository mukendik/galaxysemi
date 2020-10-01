////////////////////////////////////////////////////////////////////////////////
// ArchiveFile.h
////////////////////////////////////////////////////////////////////////////////
#if !defined(GQTL_ARCHIVEFILE_H)
#define GQTL_ARCHIVEFILE_H

#include <QObject>
// Project headers
#include "gqtl_ansifile.h"
//#include "gqtl_zlib.h"
#include <zlib.h>

// Standard headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

#if !defined __unix__ && !defined __APPLE__&__MACH__
#include <io.h>
#include <direct.h>
#endif

// QT headers
#include <QString>
#include <QStringList>


#define TAR_MAX_FILES 1024
#define NBUFFSIZE 8092
/* The magic field is filled with this if uname and gname are valid. */
#define	TMAGIC		"ustar  "	/* 7 chars and a null */
#define	RECORDSIZE	512
#define	NAMSIZ	100
#define	TUNMLEN	32
#define	TGNMLEN	32

#define IBUFSIZ 4096

#undef BITS
#define BITS 16 /* _must_ be able to handle this many bits, anyway */
#define FAST

#define    MAGIC_1     (char_type)'\037'	/* First byte of compressed file */
#define    MAGIC_2     (char_type)'\235'	/* Second byte of compressed file */
#define	   BIT_MASK    0x1f					/* Mask for 'number of compresssion bits' */
#define BLOCK_MODE    0x80					/* Block compresssion if table is full and */
#define FIRST    257						/* first free entry */
#define CLEAR    256						/* table clear output code */
#define INIT_BITS 9							/* initial number of bits/code */


/* Values used in typeflag field.  */

#define REGTYPE	 '0'		/* regular file */
#define AREGTYPE '\0'		/* regular file */
#define LNKTYPE  '1'		/* link */
#define SYMTYPE  '2'		/* reserved */
#define CHRTYPE  '3'		/* character special */
#define BLKTYPE  '4'		/* block special */
#define DIRTYPE  '5'		/* directory */
#define FIFOTYPE '6'		/* FIFO special */
#define CONTTYPE '7'		/* reserved */

#define BLOCKSIZE 512 //1024*1024 // 1 Mo
typedef long int code_int;
typedef    unsigned char    char_type;

struct tar_header
{						/* byte offset */
	char name[100];		/*   0 */
	char mode[8];		/* 100 */
	char uid[8];		/* 108 */
	char gid[8];		/* 116 */
	char size[12];		/* 124 */
	char mtime[12];		/* 136 */
	char chksum[8];		/* 148 */
	char typeflag;		/* 156 */
	char linkname[100];	/* 157 */
	char magic[6];		/* 257 */
	char version[2];	/* 263 */
	char uname[32];		/* 265 */
	char gname[32];		/* 297 */
	char devmajor[8];	/* 329 */
	char devminor[8];	/* 337 */
	char prefix[155];	/* 345 */
						/* 500 */
};


union tar_buffer {
	char               buffer[BLOCKSIZE];
	struct tar_header  header;
};

typedef struct lz_reader_t {
	/* constants (don't change during decompression) */
	FILE* fdin;
	char_type* inbuf;		/* [IBUFSIZ+64];  Input buffer */
	char_type* htab;		/* [2*HSIZE];  Dat: HSIZE for the codes and HSIZE for de_stack */
	unsigned short* codetab;/* [HSIZE]; */
	int blkmode;
	code_int maycode;
	
	/* variables that have to be saved */
	char_type        *stackp;
	code_int         code;
	int              finchar;
	code_int         oldcode;
	code_int         incode;
	int              inbits;
	int              posbits;
	int              insize;
	int              bitmask;
	code_int         freeent;
	code_int         maxcode;
	int              n_bits;
	int              rsize;
	int              maxbits;
} lz_reader_t;

union tar_record {
	char		charptr[RECORDSIZE];
	struct header {
		char	name[NAMSIZ];
		char	mode[8];
		char	uid[8];
		char	gid[8];
		char	size[12];
		char	mtime[12];
		char	chksum[8];
		char	linkflag;
		char	linkname[NAMSIZ];
		char	magic[8];
		char	uname[TUNMLEN];
		char	gname[TGNMLEN];
		char	devmajor[8];
		char	devminor[8];
	} header;
};

typedef struct Readable {
	void *f;
	int			(*xOpen4Read)	(struct Readable* self, char const* filename);
	int			(*xClose)		(struct Readable* self);
	unsigned	(*xRead)		(struct Readable* self, void* buf, unsigned len);
	char const* (*xError)		(struct Readable* self, int *errnum_ret);
} Readable;



class CArchiveFile : public QObject
{
	Q_OBJECT
public:	
    CArchiveFile(QString strExtractPath = "", bool bFullPath = true, bool bUniqueName = false);
	virtual ~CArchiveFile();


	void			Config(QString strExtractPath = "", bool bFullPath = true, bool bUniqueName = false);
	bool			IsCompressedFile(QString strFile);
    bool			ExtractFileList(const QString& lArchiveFilename, QStringList& lCompressedFiles);
	bool			Uncompress(QString strFile, QStringList& lstFilesExtracted);
    //! \brief Compress ?
    bool			Compress(QString strFile, QString &strZipFile);

    QString			GetLastErrorMsg() {return m_strErrorMsg;}
signals:
	void			sUncompressBegin(QString);

private:
	bool			Uncompress(QString strFile);
	bool			Error(char* msg = (char*)"");
    bool			ExtractFile (char const* TGZfile, bool lListOnly = false);
	int				xOpen4Read(char const* filename);
	bool			xClose(void);
	struct Readable m_SelfReadable;

    bool			m_bError;
	QString			m_strErrorMsg;
	QString			m_strExtractPath;		// where files in tar file have to be extract
	bool			m_bFullPath;			// false if we have to extract all files in the same directory
	bool			m_bUniqueName;			// "file" under "dir" are renamed in "dir_file"
	QStringList		m_strlFilesCreated;		// Contains list of files created by uncompress recursive function (used to delete all created files in case of error)

    QStringList	*   m_plstFilesExtracted;
	int				IsTar(char *buf, unsigned nbytes);
	static int		from_oct(int digs, char *where);

	CAnsiFile		m_cFileUtils;
};

#endif /* GQTL_ARCHIVEFILE_H */

