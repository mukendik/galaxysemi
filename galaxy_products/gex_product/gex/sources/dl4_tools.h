
#ifndef DL4_INC_MISC
#define DL4_INC_MISC

#define MAXSTRLEN 127
#define SPACES_PER_TAB 4

#include <stdio.h>
#if defined __APPLE__&__MACH__
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif
#include <qstring.h>

#include "gstdl_type.h"
#include "gs_types.h"
//typedef unsigned char	BYTEFLAG;
//#if !defined(_WINDOWS_H) && !defined( _WINDOWS_)
//typedef void			VOID;
//typedef void*			PVOID;
//typedef void*			LPVOID;
//typedef char			CHAR;
//typedef unsigned char	BYTE;

//typedef unsigned short	WORD;
//typedef unsigned long	DWORD;
//typedef unsigned int	UINT;
//typedef short			SHORT;
//typedef int				INT;
//typedef int				INT32;
//#if defined(WIN32)
//typedef __int64			INT64;
//#endif
//typedef int				BOOL;
//typedef long			LONG;
//typedef float			FLOAT;
//typedef char*			PSTR;
//typedef char*			LPSTR;
//typedef const char*		PCSTR;

///*#define	true			1
//#define false			0*/

//#if !defined(NULL)
//#define NULL    0
//#endif
//#endif // _WINDOWS_H

//typedef double			DOUBLE;

class STRING;

#define SIZEOF_INT				4
#define MAX_SITE_IN_ASLHANDLER	128 //255
#define MAX_SITES_FOR_DATALOG	128 //255
#define LIMITS_ARRAY_SIZE		4
#define MAX_PASS_BINS			4

BOOL DoubleToString (double d, char *s, short max_len);
BOOL FloatToString (float d, char *s, short max_len, short sig_digits = 0, BOOL fixed_decimal = false);

#define ASSERT_LOCATION(f)		if(!f) throw "Allocation error";
#define ASSERT_TAB_INDEX(f)		if(f<0) throw "Tab index error";


inline char upper_case (char c)
{
    if (c >= 'a' && c <= 'z') return (c - 0x20);
    return c;
}


inline BOOL IsLessThan (const char *c0, const char *c1, BOOL case_sensitive=false)
{
    int i=0;
    while (1)
    {
        if (!c0[i])
        {
            if (c1[i]) return true;
            return false;
        }

        if (!c1[i]) return false;

        if (case_sensitive==false)
        {
            if (upper_case (c0[i]) > upper_case(c1[i])) return false;
            if (upper_case (c0[i]) < upper_case(c1[i])) return true;
        }
        else
        {
            if (c0[i] > c1[i]) return false;
            if (c0[i] < c1[i]) return true;
        }

        i++;
    }
}

inline BOOL IsGreaterThan (const char *c0, const char *c1, BOOL case_sensitive=false)
{
    int i=0;
    while (1)
    {
        if (!c1[i])
        {
            if (!c0[i]) return false;
            return true;
        }

        if (!c0[i]) return false;

        if (case_sensitive==false)
        {
            if (upper_case (c0[i]) < upper_case (c1[i])) return false;
            if (upper_case (c0[i]) > upper_case (c1[i])) return true;
        }
        else
        {
            if (c0[i] < c1[i]) return false;
            if (c0[i] > c1[i]) return true;
        }

        i++;
    }
}

inline int StrLen (const char *s)
{
    int n=0;
    while (s[n]) n++;
    return n;
}

inline short StrMCpy (char *dest, const char *src, short n)
{
    short i;
    for (i=0; i<n-1; i++)
    {
        dest[i] = src[i];
        if(!dest[i]) return i;
    }

    dest[i] = 0;
    return i;
}

#ifndef StrCmp
inline int StrCmp (const char *s0, const char *s1)
{
    gsint32 i=0;
    while (s0[i])
    {
        if (s0[i] < s1[i]) return -1;
        if (s0[i] > s1[i]) return 1;
        i++;
    }

    return(s1[i])? -1:0;
}
#endif

inline void MemSet (void *buffer, const unsigned char byte, const int bytes)
{
    unsigned int i;

    // Assign bytes in sets of sizeof(int)
    union
    {
        unsigned char c[sizeof(int)];
        unsigned int  i;
    } u;

    for (i=0;i<sizeof(int);i++)	u.c[i] = byte;

    unsigned int n = bytes/sizeof(int);

    unsigned int *ibuffer = (unsigned int *) buffer;

    for (i=0;i<n;i++) ibuffer[i] = u.i;

    // Assign the remaining bytes individually
    unsigned char *cbuffer = (unsigned char *) &ibuffer[n];
    n = bytes%sizeof(int);
    for (i=0;i<n;i++) cbuffer[i] = byte;
}

inline void MemMove (void *buffer, const void *src, const int bytes)
{
    if(bytes > 0)
        memmove(buffer, src, bytes);

/*	if (!(bytes%SIZEOF_INT))
    {
        unsigned int *ib = (unsigned int *) buffer;
        unsigned int *is = (unsigned int *) src;
        int n = bytes/SIZEOF_INT;
        for (int i=0; i<n; i++) { ib[i] = is[i]; }
        return;
    }

    unsigned char *c = (unsigned char *) buffer;
    unsigned char *cs = (unsigned char *) src;
    for (int i=0; i<bytes; i++) { c[i] = cs[i]; }
*/
}

BOOL EngineeringNotation(double input_variable,
                        char *output_var_str,
                        short max_output_str_len,
                        short sig_digits);

void trim (char *s, BOOL leading=true, BOOL extra=true, BOOL trailing=true);

const char *GetString (unsigned short);
const char *GetString (gsuint32);
const char *GetString (short);
const char *GetString (gsint32);
const char *GetString (UINT);



/////////////////////////////////////////////////////////////////////////////////////////////////

class StoredObject
{
public:
    StoredObject(void);
    virtual ~StoredObject (void) { return; }

    virtual gsint32 GetStorageSizeBytes (void) = 0;
    virtual gsint32 LoadFromBuffer (const void *v) = 0;

    BOOL Assign (StoredObject &);

    BOOL bSameCPUType;									// all DL4 file are generated on PC, change for BigEndian only if Examinator running on SPARC

    // Function for read basic type
    int	ReadByte(const void *v, BYTE *bData);			// read 1 char. (U*1)
    int	ReadWord(const void *v, short *ptWord);			// read word	(U*2)
    int	ReadWord(const void *v, gsint32* ptWord) {
    return this->ReadDword(v, ptWord); }
    int	ReadDword(const void *v, gsint32 *ptDword);		// read Dword	(U*4)
    int	ReadFloat(const void *v,  float *ptFloat, bool *pbIsNAN=NULL);		// read float	(R*4)
    int	ReadDouble(const void *v, double *ptDouble);	// read double  (R*8)
    int	ReadString(const void *v, char *szString, short *len);

    // Function for write basic type
    int	WriteByte(void *v, BYTE bData);					// write 1 char. (U*1)
    int	WriteWord(void *v, WORD wData);					// write word	(U*2)
    int	WriteDword(void *v, gsint32 dwData);				// write Dword	(U*4)
    int	WriteFloat(void *v, float fData);				// write float	(R*4)
    int	WriteDouble(void *v, double dDouble);			// write double  (R*8)
    int	WriteString(void *v, char *szString, short len);
};

// Defined in the MISC.CPP module
class STRING : public StoredObject
{
public:

    STRING (void);
    STRING (const char *);
    STRING (const STRING&);
    ~STRING (void);

    inline BOOL Error () { return err; }

    gsint32 GetStorageSizeBytes (void);
    gsint32 LoadFromBuffer (const void *v);


private:
    char err;

public:
    char *GetStringPtr();
    operator const char* ();
    operator short ();
    operator gsint32 ();
    operator unsigned short ();
    operator gsuint32 ();
    operator double ();

    STRING &operator = (STRING &);
    STRING &operator += (STRING &);

    STRING &operator = (const char*);
    STRING &operator += (const char*);

    STRING &operator =  (short);
    STRING &operator += (short);

    STRING &operator = (unsigned short);
    STRING &operator += (unsigned short);

    STRING &operator =  (gsint32);
    STRING &operator += (gsint32);

    STRING &operator = (gsuint32);
    STRING &operator += (gsuint32);

//    STRING &operator =  (UINT);
//    STRING &operator += (UINT);

    STRING &operator = (double v);
    STRING &operator += (double v);

    STRING &operator = (float v);
    STRING &operator += (float);

        inline BOOL operator < (const char *s0)
    {
        if (StrCmp (s,s0) < 0) return true;
        return false;
    }

        inline BOOL operator > (const char *s0)
    {
        if (StrCmp (s,s0) > 0) return true;
        return false;
    }

        inline BOOL operator >= (const char *s0)
    {
        if (StrCmp (s,s0) >= 0) return true;
        return false;
    }

        inline BOOL operator <= (const char *s0)
    {
        if (StrCmp (s,s0) <= 0) return true;
        return false;
    }


        inline BOOL operator == (const char *s0)
    {
        if (!StrCmp (s0,s))
            return true;

        return false;
    }

        inline BOOL operator != (const char *s0)
    {
        if (StrCmp (s0,s))
            return true;

        return false;
    }

        inline BOOL operator < (STRING &s0)
    {
        return (*this < (const char *) s0);
    }

        inline BOOL operator > (STRING &s0)
    {
        return (*this > (const char *) s0);
    }

        inline BOOL operator >= (STRING &s0)
    {
        return (*this >= (const char *) s0);
    }

        inline BOOL operator <= (STRING &s0)
    {
        return (*this <= (const char *) s0);
    }

        inline BOOL operator == (STRING &s0)
    {
        return (*this == (const char *) s0);
    }

        inline BOOL operator != (STRING &s0)
    {
        return (*this == (const char *) s0);
    }

    void AppendHexValue (gsint32 u4);

    short len (void);
    bool HasLen (void);
    void Clear (void);
    void Trim (BOOL leading=true, BOOL extra=true, BOOL trailing=true);

    BOOL LoadFileName (const char *file_name, BOOL include_path, BOOL include_name, BOOL include_ext);
    BOOL SetValidFileName (const char *path, const char *file_name);

    BOOL Assign (const char *s, short n);
    BOOL Assign (const char *s0);
    BOOL Assign (const char *s0, const char *s1);
    BOOL Assign (const char *s0, const char *s1, const char *s2);
    BOOL Assign (const char *s0, const char *s1, const char *s2, const char *s3);

    BOOL Overwrite (short i, const char *c, short width);
    BOOL Insert    (short i, const char *c, short width);
    BOOL Remove    (short i, short width);

protected:
    char *s;
};

inline bool STRING::HasLen (void)
{
    if (s[0]) return true;
    return false;
}

inline void STRING::Clear (void)
{
    s[0] = 0;
}

inline STRING::operator const char* ()
{
    return s;
}



inline BOOL Copy (FILE *&stream_out, FILE *&stream_in, void* /*p*/=0, const gsint32 max_bytes=-1, gsint32 *bytes_written=0)
{
    int buffer_size = 0x7FFF;

    if (max_bytes >= 0)
    {
        if(buffer_size > max_bytes)
            buffer_size = max_bytes;
    }

    if (!buffer_size)
        return true;

    unsigned char *buffer = new unsigned char[buffer_size];
//	ASSERT (buffer);
    if (!buffer) return false;

    int nread,nwritten;

    gsint32 total_bytes_read=0;

    while (1)
    {
        if (max_bytes >= 0)
        {
            if (total_bytes_read + buffer_size > max_bytes)
                buffer_size = max_bytes - total_bytes_read;
        }

        nread = (int)fread (buffer,1,buffer_size,stream_in);
        if (!nread)
        {
            delete [] buffer;
            if (bytes_written) *bytes_written = total_bytes_read;
            return true;
        }

        total_bytes_read += nread;

        nwritten = (int)fwrite (buffer,1,nread,stream_out);

        if (nwritten != nread)  // Write error
        {
//			ASSERT (false);
            delete [] buffer;
            return false;
        }

        if (nread < buffer_size) // EOF
        {
            delete [] buffer;
            if (bytes_written) *bytes_written = total_bytes_read;
            return true;
        }

        if (max_bytes >= 0 && total_bytes_read == max_bytes) // Finished writing
        {
            delete [] buffer;
            if (bytes_written) *bytes_written = total_bytes_read;
            return true;
        }
    }
}

class SO_DWORD : public StoredObject
{
public:
    inline SO_DWORD (void)
    {
        m_dword = 0;
        return;
    }

    inline ~SO_DWORD (void)
    {
        return;
    }

    gsint32 GetStorageSizeBytes (void)
    {
        return sizeof (gsint32);
    }

    gsint32 LoadFromBuffer (const void *v)
    {
        return ReadDword(v,(gsint32*)&m_dword);
    }

    inline BOOL operator < (gsint32 dword)
    {
        return (m_dword < dword);
    }

    inline BOOL operator > (gsint32 dword)
    {
        return (m_dword > dword);
    }

    inline BOOL operator == (gsint32 dword)
    {
        return (m_dword == dword);
    }

    inline BOOL operator <= (gsint32 dword)
    {
        return (m_dword <= dword);
    }

    inline BOOL operator >= (gsint32 dword)
    {
        return (m_dword >= dword);
    }

    inline operator gsint32 ()
    {
        return m_dword;
    }

    inline void operator = (gsint32 dword)
    {
        m_dword = dword;
    }

    inline void operator = (SO_DWORD &dword)
    {
        m_dword = (gsint32) dword;
    }

private:

    gsint32 m_dword;
};



//BOOL TheseStoredObjectsContainExactlyTheSameData (StoredObject *S0, StoredObject *S1);

class GENERIC_STORED_OBJECT_WRAPPER : public StoredObject
{
public:
    GENERIC_STORED_OBJECT_WRAPPER (void);
    ~GENERIC_STORED_OBJECT_WRAPPER (void);
    BOOL DynamicallyAllocateData (gsint32 size);
    gsint32 GetStorageSizeBytes (void);
    gsint32 LoadFromBuffer (const void *buffer);
    void SetStorageSize (gsint32 data_size);
    void SetStaticData (const void *data);

private:

    void *m_data;
    gsint32 m_data_size;
    BYTE m_data_dymamically_allocated;
};



/////////////////////////////////////////////// MISC ///////////////////////////////////////////////

///////////////////////// MCTRL //////////////////////////////////

///////////////////////// MCTRL //////////////////////////////////

/////////////////////////////////// LSTTMPL ///////////////////////////////////

template <class PTR_CLASS, class INDEX_TYPE=gsint32> class PtrSet : public StoredObject
{
private:

    INDEX_TYPE m_alloc_increment_size;
    INDEX_TYPE m_alloc_n;
    INDEX_TYPE m_ptrs_n;
    PTR_CLASS **m_ptrs;

public:

    inline PtrSet (INDEX_TYPE alloc_increment_size)
    {
        m_alloc_increment_size = alloc_increment_size;
        m_alloc_n = 0;
        m_ptrs_n = 0;
        m_ptrs = 0;
        m_flags = 0;  // GPZ::UMR
        m_delete_objects = false;
    }

    inline PtrSet (void)
    {
        m_alloc_increment_size = 1;
        m_alloc_n = 0;
        m_ptrs_n = 0;
        m_ptrs = 0;
        m_flags = 0;  // GPZ::UMR
        m_delete_objects = true;
    }

    inline ~PtrSet (void)
    {
        delete_pointers ();
    }

    inline void SetStatusDeleteObjects (BOOL status)
    {
        m_delete_objects = status;
    }

    inline BOOL increase_allocation_if_necessary (INDEX_TYPE n)
    {
        if (m_alloc_n <= n)
        {
            INDEX_TYPE incr = m_alloc_increment_size;
            if (!incr) incr = m_alloc_n;
            if (!incr) incr++;
            if (m_alloc_n + incr < n) incr = n - m_alloc_n;

            PTR_CLASS **tmp = new PTR_CLASS*[incr+m_alloc_n];
            if (!tmp) return false;

            MemMove (tmp,m_ptrs,sizeof(PTR_CLASS*)*m_ptrs_n);
            MemSet (&tmp[m_alloc_n],0,sizeof(PTR_CLASS*)*incr);

            delete m_ptrs;

            m_ptrs = tmp;
            m_alloc_n += incr;
        }

        return true;
    }

    inline BOOL Append (PTR_CLASS *tr)
    {
        if (increase_allocation_if_necessary (m_ptrs_n+1) == false)
            return false;

        m_ptrs[m_ptrs_n++] = tr;

        return true;
    }

    inline BOOL Insert (INDEX_TYPE i, PTR_CLASS *tr)
    {
        if (i > m_ptrs_n)
            return false;

        if (increase_allocation_if_necessary (m_ptrs_n+1) == false)
            return false;

        for (INDEX_TYPE j=m_ptrs_n; j>i; j--)
            m_ptrs[j] = m_ptrs[j-1];

        m_ptrs[i] = tr;

        m_ptrs_n++;

        return true;
    }

    inline BOOL Remove (INDEX_TYPE i)
    {
        if (i >= m_ptrs_n)
            return false;

        if (m_delete_objects == true)
        {
            delete m_ptrs[i];
            m_ptrs[i] = 0;
        }

        for (INDEX_TYPE j=i; j<m_ptrs_n-1; j++)
            m_ptrs[j] = m_ptrs[j+1];

        m_ptrs_n--;
        m_ptrs[m_ptrs_n] = 0;

        return true;
    }

    inline BOOL RemoveAll (void)
    {
        delete_pointers ();
        return true;
    }

    inline const INDEX_TYPE &GetAllocCount (void)
    {
        return m_alloc_n;
    }

    inline PTR_CLASS *GetPtr (INDEX_TYPE i)
    {
        if (i >= m_ptrs_n) return 0;
        if (!m_ptrs) return 0;
        return m_ptrs[i];
    }

    inline gsint32 LoadFromBuffer (const void *v)
    {
        delete_pointers ();

        const BYTE *b = (const BYTE *) v;

        gsint32 i=0;

        // Extract the number of objects from the buffer
        INDEX_TYPE ptrs_n;
    i += ReadWord(b+i,(INDEX_TYPE*) &ptrs_n);

        increase_allocation_if_necessary (ptrs_n);

        m_ptrs_n = ptrs_n;

        for (gsint32 j=0; j<m_ptrs_n; j++)
        {
            // Allocate each object
            m_ptrs[j] = new PTR_CLASS;
            if (!m_ptrs[j]) return 0;

            // Initialize each object from the buffer
            i += m_ptrs[j]->LoadFromBuffer ((const char *) (b+i));
        }

        return i;
    }

    inline gsint32 GetStorageSizeBytes (void)
    {
        gsint32 i=0;

        i += sizeof (m_ptrs_n);

        for (gsint32 j=0; j<(gsint32)m_ptrs_n; j++)
            i += m_ptrs[j]->GetStorageSizeBytes ();

        return i;
    }

    inline BOOL SetCount (const INDEX_TYPE count)
    {
        delete_pointers ();
        increase_allocation_if_necessary (count);
        for (INDEX_TYPE i=0; i<count; i++)
        {
            m_ptrs[i] = new PTR_CLASS;
            if (!m_ptrs[i]) return false;
        }
        m_ptrs_n = count;
        return true;
    }

    inline INDEX_TYPE &GetCount (void)
    {
        return m_ptrs_n;
    }

    void SetIsQa (BOOL status)
    {
        m_is_qa = status;
    }

    inline BOOL GetIsQa (void)
    {
        return m_is_qa;
    }

    inline PTR_CLASS *GetObjectPtr (const INDEX_TYPE j)
    {
        if (j<0 || j>m_ptrs_n-1) return 0;

        return m_ptrs[j];
    }

    inline BOOL Assign (const INDEX_TYPE j, const void *buffer)
    {
        if (j<0 || j>m_ptrs_n-1) return false;

        m_ptrs[j]->LoadFromBuffer (buffer);

        return true;
    }

private:

    inline void delete_pointers (void)
    {
        if (m_delete_objects == true)
        {
            for (INDEX_TYPE i=0; i<m_ptrs_n; i++)
            {
                if (m_ptrs[i] != NULL) //[QA RETEST] delete_pointers
                {
                    delete m_ptrs[i];	//[QA RETEST] delete_pointers
                    m_ptrs[i] = NULL;	//[QA RETEST] delete_pointers
                }
            }
        }

            delete m_ptrs;

        m_ptrs = 0;
        m_ptrs_n = 0;
        m_alloc_n = 0;
    }

    union {
        BYTE m_flags;
        struct
        {
            BYTE m_delete_objects : 1;
            BYTE m_is_qa		  : 1;  //[QA RETEST] PtrSet
            BYTE m_reserved		  : 6;
        };
    };

};

template <class ARRAY_TYPE, class CAST_AS_TYPE, class DATA_TYPE> class BinSearch
{
public:
    inline gsint32 GetIndex (ARRAY_TYPE *array, const gsint32 array_size_n,
                   const DATA_TYPE &target)
    {
        gsint32 i;
        for (i=0; i<array_size_n; i++)
        {
            if ((DATA_TYPE)((CAST_AS_TYPE &)array[i]) >= target)
            {
                if ((DATA_TYPE)((CAST_AS_TYPE &)array[i]) == target)
                    return i;

                return (i-1);
            }
        }

        return i;
    }
};

/////////////////////////////////// LSTTMPL ///////////////////////////////////


#endif //DL4_INC_MISC

