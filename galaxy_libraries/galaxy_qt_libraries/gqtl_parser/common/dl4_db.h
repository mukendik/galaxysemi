#ifndef DL4_INC_BL4_DB
#define DL4_INC_BL4_DB

#include "dl4_tools.h"

///////////////////////////////////////////////////////////////////////////////////
// Each fixed length record in the database is stored like this:
//
//	   Description							     Storage size
// [contents of m_fixed_length_data]		 m_sizeof_fixed_length_data bytes
// [m_sizeof_var_length_data]				 4 bytes
// [m_offset_of_var_length_data]			 4 bytes
//
// Note: m_offset_of_var_length_data will be -1 if there is no variable length data
//		 m_sizeof_var_length_data will be 0 if ther is no variable length data
//
class BINARY_DATABASE_ITEM
{
public:

    BINARY_DATABASE_ITEM (void);
    ~BINARY_DATABASE_ITEM (void);

    void *&VarLengthData (void);
    const gsint32 &VarLengthDataSize (void);
    BOOL SetSizeOfFixedLengthData (const gsint32 &);
    BOOL SetSizeOfVarLengthData (const gsint32 &);
    void *&FixedLengthData (void);
    const gsint32 &FixedLengthDataSize (void);

    BOOL SetOffsetOfVarLengthData (const gsint32 &);

    inline const gsint32 &OffsetOfVarLengthData (void) { return m_offset_var_len_data; }

    inline StoredObject *FLD (void)
    {
        m_FLD.SetStorageSize (m_sizeof_fixed_length_data);
        m_FLD.SetStaticData (m_fixed_length_data);

        return (StoredObject *) &m_FLD;
    }

    inline StoredObject *VLD (void)
    {
        m_VLD.SetStorageSize (m_sizeof_var_length_data);
        m_VLD.SetStaticData (m_var_length_data);

        return (StoredObject *) &m_VLD;
    }

private:

    struct
    {
        unsigned char del_fld : 1;
        unsigned char del_vld : 1;
    } m_bdi_status;

    void *m_fixed_length_data;
    gsint32 m_sizeof_fixed_length_data;

    void *m_var_length_data;
    gsint32 m_sizeof_var_length_data;

    gsint32 m_offset_var_len_data;

    GENERIC_STORED_OBJECT_WRAPPER m_FLD,m_VLD;
};

inline BINARY_DATABASE_ITEM::BINARY_DATABASE_ITEM (void)
{
    m_bdi_status.del_fld = true;
    m_bdi_status.del_vld = true;

    m_fixed_length_data = 0;
    m_sizeof_fixed_length_data = 0;

    m_var_length_data = 0;
    m_sizeof_var_length_data = 0;

    m_offset_var_len_data = -1;
}

inline BINARY_DATABASE_ITEM::~BINARY_DATABASE_ITEM (void)
{
    if (m_fixed_length_data && m_bdi_status.del_fld)
    {
        delete [] (unsigned char *)(m_fixed_length_data);
        m_fixed_length_data = 0;
        m_sizeof_fixed_length_data = 0;
    }

    if (m_var_length_data && m_bdi_status.del_vld)
    {
        delete [] (unsigned  char *)(m_var_length_data);
        m_var_length_data = 0;
        m_sizeof_var_length_data = 0;
    }

    m_offset_var_len_data = -1;
}

inline void *&BINARY_DATABASE_ITEM::VarLengthData (void) { return m_var_length_data; }
inline const gsint32 &BINARY_DATABASE_ITEM::VarLengthDataSize (void) { return m_sizeof_var_length_data; }
inline void *&BINARY_DATABASE_ITEM::FixedLengthData (void) { return m_fixed_length_data; }
inline const gsint32 &BINARY_DATABASE_ITEM::FixedLengthDataSize (void) { return m_sizeof_fixed_length_data; }
inline BOOL BINARY_DATABASE_ITEM::SetOffsetOfVarLengthData (const gsint32 &offset) {m_offset_var_len_data=offset; return true; }
inline BOOL BINARY_DATABASE_ITEM::SetSizeOfFixedLengthData (const gsint32 &sz)
{
    if (m_sizeof_fixed_length_data == sz) return true;

    if (m_fixed_length_data)
    {
        delete [] (unsigned char *)(m_fixed_length_data); m_fixed_length_data = 0;
    }

    m_sizeof_fixed_length_data = sz;
    m_fixed_length_data = (void *) new unsigned char [sz];
//	ASSERT(m_fixed_length_data);
    if(!m_fixed_length_data) return false;

    return true;
}

inline BOOL BINARY_DATABASE_ITEM::SetSizeOfVarLengthData (const gsint32 &sz)
{
    if (m_sizeof_var_length_data == sz) return true;

    if (m_var_length_data)
    {
        delete [] (unsigned char *)(m_var_length_data);
        m_var_length_data = 0;
    }

    m_sizeof_var_length_data = sz;
    m_var_length_data = (void *) new unsigned char [sz];
//	ASSERT(m_var_length_data);
    if(!m_var_length_data) return false;

    return true;
}

class BinaryDatabaseDimensionsVLD : public StoredObject
{
public:

    BinaryDatabaseDimensionsVLD (void);
    ~BinaryDatabaseDimensionsVLD (void);

    gsint32 GetStorageSizeBytes (void);
    gsint32 LoadFromBuffer (const void *);

    inline WORD &GetDimensionsN (void)
    {
        return m_dimensions_n;
    }

    inline WORD &GetActiveDimension (void)
    {
        return m_active_dimension;
    }

    const char *GetDimensionName (WORD i);

protected:

    WORD m_reserved;
    WORD m_dimensions_n;
    WORD m_active_dimension;
    STRING *m_names;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class BinaryDatabase
{
public:

    BinaryDatabase ();
    ~BinaryDatabase (void);

    inline gsint32 GetCurrentPos() {	if(m_opened_stationary_ff_pos>m_opened_stationary_vf_pos)
                                 return m_opened_stationary_ff_pos;
                            else return m_opened_stationary_vf_pos;};


    bool OpenDatabaseStationaryMode (const char *file_name);

    QString GetLastError();
    void ErrorMessage (const char *e0, const char *e1=0, const char *e2=0, const char *e3=0, const char *e5=0);

    bool Append (StoredObject *fld, StoredObject *vld);
    bool Append (BINARY_DATABASE_ITEM &);
    bool Get (const gsint32 idx, BINARY_DATABASE_ITEM &);

    bool Overwrite (StoredObject &fld, StoredObject &vld, const gsint32 idx, WORD idim=0xFFFF);

    bool Read (StoredObject &fld, StoredObject &vld, const gsint32 idx);
    gsint32 ReadHDR (StoredObject &hdr);
    bool ReadFLD (StoredObject &, const gsint32 idx);
    bool ReadVLD (StoredObject &, const gsint32 idx);
    char *ReadAD (void * &);

    const char *GetDimensionName (WORD i);

    inline const WORD &DimensionsN (void)
    {
        return m_dimensions_vld.GetDimensionsN ();
    }

    inline gsint32 ItemsTotalN(bool per_dimension = true)
    {
        // Make sure that each dimension has the same number of items
        if ((m_items_n%DimensionsN ())) return 0;

        if (per_dimension == true)
            return m_items_n/DimensionsN ();

        return m_items_n;
    }

    inline WORD &GetActiveDimension (void)
    {
        return m_dimensions_vld.GetActiveDimension ();
    }

    bool IsOpen (void);
    void Close (void);

    inline const char *GetDatabaseFileName (void)
    {
        return m_file_name;
    }

    void LockDatabaseFile (void);
    void UnlockDatabaseFile (void);

    bool ReadVLD (GENERIC_STORED_OBJECT_WRAPPER &, gsint32 rec_i);
    bool ReadFLD (GENERIC_STORED_OBJECT_WRAPPER &, gsint32 rec_i);

    void Clear(void);

protected:

private:

    FILE	*m_stream_locked_db_file;

    union
    {
        BYTE m_flags;
        struct
        {
            BYTE m_del_fixed_len_file	  : 1;
            BYTE m_create_backup		  : 1;
            BYTE m_is_modified			  : 1;
            BYTE m_prompt_to_save_changes : 1;
            BYTE m_db_file_is_locked      : 1;
            BYTE m_packed_size_change	  : 1;
        };
    };

    bool m_opened_stationary;
    gsint32 m_opened_stationary_ff_pos;
    gsint32 m_opened_stationary_vf_pos;

    FILE *m_stream_fixed_len_data;
    FILE *m_stream_var_len_data;

    STRING m_var_len_file_name;
    STRING m_fixed_len_file_name;
    STRING m_file_name;

    gsint32 m_item_size_bytes;
    gsint32 m_items_n;
    gsint32 m_sizeof_custom_header;

    BinaryDatabaseDimensionsVLD m_dimensions_vld;

    DWORD m_dimension_vld_offset;
    DWORD m_dimension_vld_size;
    DWORD m_database_file_name_id;

    union
    {
        DWORD m_status;
        struct
        {
            DWORD m_open_for_editing  : 1;
        };
    };

    DWORD m_reserved[16];
    void *m_custom_header;
    gsint32 m_first_record_offset;

    gsint32 m_packed_sizeof_vld;

    inline gsint32 beginning_of_var_len_data (void);
    inline gsint32 record_offset (const gsint32 &irec, WORD idim=0xFFFF);

    inline bool vf_seek (const gsint32 &pos);
    inline bool vf_read (void *v, const gsint32 n);
    inline bool vf_read_buffer (void *v, const gsint32 n);
    inline bool vf_write (const void *v, const gsint32 n);

    inline bool ff_seek (const gsint32 &pos);
    inline bool ff_read (gsint32 &i);
    inline bool ff_read (DWORD &i);
    inline bool ff_read (void *v, const gsint32 n);
    inline bool ff_read_buffer (void *v, const gsint32 n);

    inline bool swap_order(void *passed_ptr,const gsint32 lSize);
    bool	m_bSameCPUType;

    gsint32 compute_first_record_offset (void);
};

///////////////////////////////////////// PROP_ /////////////////////////////////////////
#define PROP_TYPE_NODATA        0
#define PROP_TYPE_SWITCH        1
#define PROP_TYPE_BYTE          2
#define PROP_TYPE_WORD          3
#define PROP_TYPE_DWORD         4
#define PROP_TYPE_CHAR		    5
#define PROP_TYPE_SHORT         6
#define PROP_TYPE_LONG          7
#define PROP_TYPE_FLOAT         8
#define PROP_TYPE_DOUBLE        9
#define PROP_TYPE_STRING_8      10
#define PROP_TYPE_STRING_16     11
#define PROP_TYPE_STRING_32     12
#define PROP_TYPE_STRING_64     13
#define PROP_TYPE_STRING_80     14
#define PROP_TYPE_STRING_128    15



class PROP_SWITCH
{

public:

    // 5 bytes storage.  This must not change!!!
    BYTE setting;
    DWORD properties;

    inline PROP_SWITCH (void)
    {
        setting = false;
        properties = 0;
    }

    inline PROP_SWITCH & operator = (BOOL status) { setting = status; return (*this); }
    inline operator BOOL () { return setting; }
};

class PROP_BYTE
{

public:

    // 5 bytes storage. This must not change!!!!
    BYTE value;
    DWORD properties;

    inline PROP_BYTE (void)
    {
        value = 0;
        properties = 0;
    }

    inline PROP_BYTE & operator = (BYTE i) { value = i; return (*this); }
    inline operator BYTE () { return value; }
};

class PROP_CHAR
{

public:

    // 5 bytes storage. This must not change!!!!
    char value;
    DWORD properties;

    inline PROP_CHAR (void)
    {
        value = 0;
        properties = 0;
    }

    inline PROP_CHAR & operator = (char i) { value = i; return (*this); }
    inline operator char () { return value; }
};

class PROP_SHORT
{

public:

    // 6 bytes storage. This must not change!!!!
    short value;
    DWORD  properties;

    inline PROP_SHORT (void)
    {
        value = 0;
        properties = 0;
    }

    inline PROP_SHORT & operator = (short i) { value = i; return (*this); }
    inline operator short () { return value; }
};

class PROP_LONG
{

public:

    // 8 bytes storage. This must not change!!!!
    gsint32 value;
    DWORD  properties;

    inline PROP_LONG (void)
    {
        value = 0;
        properties = 0;
    }

    inline PROP_LONG & operator = (gsint32 i) { value = i; return (*this); }
    inline operator gsint32 () { return value; }
};

class PROP_DWORD
{

public:

    // 8 bytes storage. This must not change!!!!
    DWORD value;
    DWORD properties;

    inline PROP_DWORD (void)
    {
        value = 0;
        properties = 0;
    }

    inline PROP_DWORD & operator = (DWORD i) { value = i; return (*this); }
    inline operator DWORD () { return value; }
};

class PROP_WORD
{

public:

    // 6 bytes storage. This must not change!!!!
    WORD value;
    DWORD properties;

    inline PROP_WORD (void)
    {
        value = 0;
        properties = 0;
    }

    inline PROP_WORD & operator = (WORD i) { value = i; return (*this); }
    inline operator WORD () { return value; }
};

class PROP_FLOAT
{

public:

    // 8 bytes storage. This must not change!!!!
    float f;
    DWORD properties;

    inline PROP_FLOAT (void)
    {
        f = 0.0F;
        properties = 0;
    }

    inline PROP_FLOAT & operator = (float v) { f = v; return (*this); }
    inline operator float () { return f; }
};

class PROP_DOUBLE
{

public:

    // 12 bytes storage. This must not change!!!!
    double d;
    DWORD properties;

    inline PROP_DOUBLE (void)
    {
        d = 0.0;
        properties = 0;
    }

    inline PROP_DOUBLE & operator = (double v) { d = v; return (*this); }
    inline operator double () { return d; }
};

class PROP_STRING_8
{

public:

    // 12 bytes storage. This must not change!!!!
    char  s[8];
    DWORD properties;

    inline PROP_STRING_8 (void)
    {
        MemSet (s,0,8);
        properties = 0;
    }

    inline PROP_STRING_8 & operator << (const char *src) { StrMCpy (s,src,8); return (*this); }
    inline operator const char * () { return s; }
};


class PROP_STRING_16
{

public:

    // 20 bytes storage. This must not change!!!!
    char  s[16];
    DWORD  properties;

    inline PROP_STRING_16 (void)
    {
        MemSet (s,0,16);
        properties = 0;
    }

    inline PROP_STRING_16 & operator << (const char *src) { StrMCpy (s,src,16); return (*this); }
    inline operator const char * () { return s; }
};


class PROP_STRING_32
{

public:

    // 36 bytes storage. This must not change!!!!
    char  s[32];
    DWORD  properties;

    inline PROP_STRING_32 (void)
    {
        MemSet (s,0,32);
        properties = 0;
    }

    inline PROP_STRING_32 & operator << (const char *src) { StrMCpy (s,src,32); return (*this); }
    inline operator const char * () { return s; }
};

class PROP_STRING_64
{

public:

    // 68 bytes storage. This must not change!!!!
    char  s[64];
    DWORD properties;

    inline PROP_STRING_64 (void)
    {
        MemSet (s,0,64);
        properties = 0;
    }

    inline PROP_STRING_64 & operator << (const char *src) { StrMCpy (s,src,64); return (*this); }
    inline operator const char * () { return s; }
};

class PROP_STRING_80
{

public:

    // 84 bytes storage. This must not change!!!!
    char  s[80];
    DWORD  properties;

    inline PROP_STRING_80 (void)
    {
        MemSet (s,0,80);
        properties = 0;
    }

    inline PROP_STRING_80 & operator << (const char *src) { StrMCpy (s,src,80); return (*this); }
    inline operator const char * () { return s; }
};

class PROP_STRING_128
{

public:

    // 132 bytes storage. This must not change!!!!
    char  s[128];
    DWORD  properties;

    inline PROP_STRING_128 (void)
    {
        MemSet (s,0,128);
        properties = 0;
    }

    inline PROP_STRING_128 & operator << (const char *src) { StrMCpy (s,src,128); return (*this); }
    inline operator const char * () { return s; }
};


///////////////////////////////////////// PROP_ /////////////////////////////////////////
#endif //DL4_INC_BL4_DB
