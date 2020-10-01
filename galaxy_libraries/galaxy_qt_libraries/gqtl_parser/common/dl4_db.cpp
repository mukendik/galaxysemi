#include "dl4_db.h"

#if defined unix || __MACH__
#include <unistd.h>
#endif
#ifdef _WIN32
#include <io.h>
#endif
#include <stdlib.h>
#include <ctype.h>

#define MAX_SIZE_DB_HEADER_BYTES 2000000
#define MAX_SIZE_DB_FLD_ITEM_BYTES 100000


///////////////////////////////////////////////////////
//// START ////////////////////////////////////////////
/////////// BinaryDatabaseDimensionsVLD ///////////////
///////////////////////////////////////////////////////

BinaryDatabaseDimensionsVLD::BinaryDatabaseDimensionsVLD (void)
{
    m_reserved = 0;
    m_dimensions_n = 1;
    m_names = new STRING[1];
    m_names[0] = "Default";
    m_active_dimension = 0;
}

BinaryDatabaseDimensionsVLD::~BinaryDatabaseDimensionsVLD (void)
{
    delete [] m_names;
}

gsint32 BinaryDatabaseDimensionsVLD::GetStorageSizeBytes (void)
{
    gsint32 len=0;

    len += sizeof (m_reserved);
    len += sizeof (m_dimensions_n);
    len += sizeof (m_active_dimension);

    for (WORD i=0; i<m_dimensions_n; i++)
    {
        len += StrLen (m_names[i]) + 1;
    }

    return len;
}

gsint32 BinaryDatabaseDimensionsVLD::LoadFromBuffer (const void *buffer)
{
    gsint32 idx=0;
    const char *c = (const char *) buffer;

    idx += sizeof (m_reserved);

    idx += ReadWord(c+idx,(short*)&m_dimensions_n);
    idx += ReadWord(c+idx,(short*)&m_active_dimension);
    if (m_names)
    {
        delete [] m_names;
    }

    if (!m_dimensions_n)
    {
        m_dimensions_n = 1;
        m_names = new STRING[1];
        m_names[0] = "Default";
        return idx;
    }

    m_names = new STRING[m_dimensions_n];

    for (WORD i=0; i<m_dimensions_n; i++)
    {
        m_names[i] = (const char *) (c+idx);
        idx += StrLen (m_names[i]) + 1;
    }

    return idx;
}


const char *BinaryDatabaseDimensionsVLD::GetDimensionName (WORD i)
{
    if (i > m_dimensions_n-1) return 0;
    return m_names[i];
}


///////////////////////////////////////////////////////
//// END //////////////////////////////////////////////
/////////// BinaryDatabaseDimensionsVLD ///////////////
///////////////////////////////////////////////////////


inline gsint32 BinaryDatabase::compute_first_record_offset (void)
{
    gsint32 n =				sizeof (m_item_size_bytes) +
                            sizeof (m_items_n) +
                            sizeof (m_sizeof_custom_header) +
                            m_sizeof_custom_header +
                            sizeof (m_dimension_vld_offset) +
                            sizeof (m_dimension_vld_size) +
                            sizeof (m_database_file_name_id) +
                            sizeof (m_status) +
                            sizeof (m_reserved);

    return n;
}

BinaryDatabase::BinaryDatabase()
{
    m_stream_fixed_len_data = 0;
    m_stream_var_len_data = 0;

    m_items_n = 0;
    m_item_size_bytes = 0;
    m_first_record_offset = 0;
    m_sizeof_custom_header = 0;
    m_custom_header = 0;
    m_dimension_vld_offset = 0;
    m_dimension_vld_size = 0;

    m_packed_sizeof_vld = -1;

    m_flags = 0;  // GPZ::UMR
    m_status = 0;  // GPZ::UMR

    // Initialize mflags
    m_del_fixed_len_file = false;
    m_create_backup = true;
    m_is_modified   = false;
    m_prompt_to_save_changes = true;
    m_db_file_is_locked = 0;
    m_packed_size_change = false;

    m_database_file_name_id = 0;
    m_open_for_editing = 0;
    m_opened_stationary = false;

    m_opened_stationary_vf_pos = -1;
    m_opened_stationary_ff_pos = -1;

#if defined (i386) || defined (__i386__) || defined (_M_IX86)\
    || defined (vax) || defined (__alpha) || defined(__x86_64)\
    || defined(__x86_64__) || defined (_M_X64)
// i386, vax and alpha processors use LITTLE ENDIAN storage
    m_bSameCPUType = 1;							// all DL4 file are generated on PC, change for BigEndian only if Examinator running on SPARC
#else
// sparc processors (__sparc__) use BIG ENDIAN storage
    m_bSameCPUType = 0;							// all DL4 file are generated on PC, change for BigEndian only if Examinator running on SPARC
#endif
}

void BinaryDatabase::Clear (void)
{
    m_stream_fixed_len_data = 0;
    m_stream_var_len_data = 0;

    m_items_n = 0;
    m_item_size_bytes = 0;
    m_first_record_offset = 0;
    m_sizeof_custom_header = 0;
    m_custom_header = 0;
    m_del_fixed_len_file = false;

    m_dimension_vld_offset = 0;
    m_dimension_vld_size = 0;

    m_packed_sizeof_vld = -1;

    m_database_file_name_id = 0;
    m_create_backup = true;
    m_is_modified   = false;
    m_prompt_to_save_changes = true;
    m_open_for_editing = 0;

    m_db_file_is_locked = 0;
    m_opened_stationary = false;
}

void BinaryDatabase::LockDatabaseFile (void)
{
    if (m_db_file_is_locked)
        return;

    m_stream_locked_db_file = fopen(m_file_name, "rb");
    if(m_stream_locked_db_file)
        m_db_file_is_locked = true;
}

void BinaryDatabase::UnlockDatabaseFile (void)
{
    if (m_db_file_is_locked)
    {
        fclose(m_stream_locked_db_file);
        m_db_file_is_locked = false;
    }
}


BinaryDatabase::~BinaryDatabase (void)
{
        UnlockDatabaseFile ();

        if (m_custom_header)
            delete [] (unsigned char *)(m_custom_header);

        m_custom_header = 0;
        m_sizeof_custom_header = 0;

        Clear ();
        return;
}

void BinaryDatabase::Close (void)
{
        UnlockDatabaseFile ();

        if (m_custom_header)
            delete [] (unsigned char *)(m_custom_header);

        m_custom_header = 0;
        m_sizeof_custom_header = 0;

        Clear ();
        return;
}
static QString str_static_ErrorMessage;

QString BinaryDatabase::GetLastError()
{
    return str_static_ErrorMessage;
}


void BinaryDatabase::ErrorMessage (const char *e0, const char *e1, const char *e2, const char *e3,const char *e4)
{
    QString strMessage = "Import Dl4: ";
    strMessage += e0;
    if (e1)
    {
        strMessage += " ; ";
        strMessage += e1;
        if (e2)
        {
            strMessage += " ; ";
            strMessage += e2;
            if (e3)
            {
                strMessage += " ; ";
                strMessage += e3;
                if (e4)
                {
                    strMessage += " ; ";
                    strMessage += e4;
                }
            }
        }
    }

    str_static_ErrorMessage = strMessage;
}

const char *BinaryDatabase::GetDimensionName (WORD i)
{
    return m_dimensions_vld.GetDimensionName (i);
}





bool BinaryDatabase::ReadVLD (StoredObject &obj, const gsint32 idx)
{
    if (idx < 0) return false;
    if (idx >= ItemsTotalN ()) return false;

    BINARY_DATABASE_ITEM item;
    if (!Get (idx,item))
        return false;

    if(item.VarLengthData () == NULL)
        return true;

    gsint32 sz = obj.LoadFromBuffer (item.VarLengthData ());

    if (sz != item.VarLengthDataSize ())
    {
        QString strMessage;
        strMessage.sprintf("Pos in DL4 file(%d) - Error message(Offset error)", BinaryDatabase::GetCurrentPos());
        ErrorMessage("Dl4 file corrupted",strMessage.toLatin1().constData());
        return false;
    }

    return true;
}



bool BinaryDatabase::ReadFLD (StoredObject &fld, const gsint32 idx)
{
    if((idx < 0)
    || (idx >= ItemsTotalN ()))
    {
        ErrorMessage("ReadFLD");
        return false;
    }


    const gsint32 seek_pos = m_first_record_offset + idx*(m_item_size_bytes+8);

    if(!ff_seek (seek_pos))
    {
        ErrorMessage("ReadFLD");
        return false;
    }

    BYTE *b = new BYTE [m_item_size_bytes];
    if (!b)
    {
        ErrorMessage("Can't allocate enough memory to perform database operation.");
        return false;
    }

    if(!ff_read_buffer (b,m_item_size_bytes))
    {
        delete [] b; b=0;
        return false;
    }

    if(fld.LoadFromBuffer (b) != m_item_size_bytes)
    {
        QString strMessage;
        strMessage.sprintf("Pos in DL4 file(%d) - Error message(Offset error)", BinaryDatabase::GetCurrentPos());
        ErrorMessage("Dl4 file corrupted",strMessage.toLatin1().constData());
        delete [] b; b=0;
        return false;
    }

    delete [] b; b=0;

    return true;
}

char *
BinaryDatabase::ReadAD (void * &buffer)
{
    char		c, sizebuf[128];
    int			i, size;
    static char	tag[128];

    tag[0] = '\0';
    if (vf_read(&c, 1L) && c == '<') {
        i = 0;
        while (vf_read(&c, 1L)) {
            if (isspace(c)) {
                break;
            }
            tag[i++] = c;
        }
        tag[i] = '\0';
        i = 0;
        while (vf_read(&c, 1L)) {
            if (isspace(c)) {
                continue;
            }
            sizebuf[i++] = c;
            if (c == '>') {
                break;
            }
        }
        sizebuf[i] = '\0';
        if(!vf_read(&c, 1L))
        {
            ErrorMessage("ReadAD");
            return NULL;
        }

        if ((size = atoi(sizebuf)) == 0) {
            return NULL;
        }
        buffer = (void *)(new BYTE[size]);
        if (!buffer)
        {
            ErrorMessage("Can't allocate enough memory to perform database operation.");
            return NULL;
        }
        if(!vf_read_buffer(buffer, size))
        {
            ErrorMessage("ReadAD");
            return NULL;
        }
    }

    return (tag[0]) ? &tag[0] : NULL;
}

gsint32 BinaryDatabase::ReadHDR (StoredObject &hdr)
{
    gsint32 sz = hdr.LoadFromBuffer (m_custom_header);

    if(sz != m_sizeof_custom_header)
    {
        QString strMessage;
        strMessage.sprintf("Pos in DL4 file(%d) - Error message(Offset error)", BinaryDatabase::GetCurrentPos());
        ErrorMessage("Dl4 file corrupted",strMessage.toLatin1().constData());
        return 0;
    }


    return sz;
}

bool BinaryDatabase::Read (StoredObject &fld, StoredObject &vld, const gsint32 idx)
{
    if((idx < 0)
    || (idx >= ItemsTotalN ()))
    {
        ErrorMessage("Read");
        return false;
    }

    BINARY_DATABASE_ITEM item;
    if(!Get (idx,item))
        return false;

    gsint32 sz = fld.LoadFromBuffer (item.FixedLengthData ());
    if(sz != item.FixedLengthDataSize ())
    {
        QString strMessage;
        strMessage.sprintf("Pos in DL4 file(%d) - Error message(Offset error)", BinaryDatabase::GetCurrentPos());
        ErrorMessage("Dl4 file corrupted",strMessage.toLatin1().constData());
        return false;
    }

    sz = vld.LoadFromBuffer (item.VarLengthData ());

    if(sz != item.VarLengthDataSize ())
    {
        QString strMessage;
        strMessage.sprintf("Pos in DL4 file(%d) - Error message(Offset error)", BinaryDatabase::GetCurrentPos());
        ErrorMessage("Dl4 file corrupted",strMessage.toLatin1().constData());
        return false;
    }

    return true;
}

bool BinaryDatabase::Get (const gsint32 idx, BINARY_DATABASE_ITEM &item)
{
    if((idx < 0)
    || (idx >= ItemsTotalN ()))
    {
        ErrorMessage("Get");
        return false;
    }

    if (!item.FixedLengthData ()) // Fixed length data storage not allocated yet, allocate it now
    {
        item.SetSizeOfFixedLengthData (m_item_size_bytes);
    }

    if (item.FixedLengthDataSize () != m_item_size_bytes)
    {
        QString strMessage;
        strMessage.sprintf("Pos in DL4 file(%d) - Error message(Offset error)", BinaryDatabase::GetCurrentPos());
        ErrorMessage("Dl4 file corrupted",strMessage.toLatin1().constData());
        return false;
    }

    if (!ff_seek (record_offset (idx)))
        return false;

    if (!ff_read_buffer (item.FixedLengthData (),m_item_size_bytes))
        return false;

    gsint32 var_length_data_size;
    gsint32 offset_of_var_length_data;

    if (!ff_read (var_length_data_size))
        return false;

    if (!ff_read (offset_of_var_length_data))
        return false;

    if (var_length_data_size)
    {
        if (!item.SetSizeOfVarLengthData (var_length_data_size))
        {
            ErrorMessage("Get");
            return false;
        }
        if (!item.SetOffsetOfVarLengthData (offset_of_var_length_data))
        {
            ErrorMessage("Get");
            return false;
        }

        if (!vf_seek (offset_of_var_length_data))
            return false;

        if (!vf_read_buffer (item.VarLengthData (),var_length_data_size))
            return false;

    }

    return true;
}

inline gsint32 BinaryDatabase::record_offset (const gsint32 &irec, WORD idim/*=0xFFFF*/)
{
    if (idim == 0xFFFF)
        idim = GetActiveDimension ();

    gsint32 offset = m_first_record_offset;
    offset += irec * (m_item_size_bytes + 8);
    if (idim) offset += ItemsTotalN () * idim * (m_item_size_bytes + 8);
    return offset;
}



bool BinaryDatabase::OpenDatabaseStationaryMode (const char *file_name)
{
    str_static_ErrorMessage="";
    if (IsOpen ())
    {
        UnlockDatabaseFile ();
        Clear ();
    }



    /////////////////////////////////////////////////////////////////////////////////////////
    // Check access
    if (access (file_name,00))
    {
        ErrorMessage ("File: ",file_name," does not exist.");
        return false;
    }

    if (access (file_name,04))
    {
        ErrorMessage ("File: ",file_name," no read access.");
        return false;
    }
    ////////////////////////////////////////////////////////////////////////////////////////

    m_file_name = file_name;
    m_opened_stationary = true;
    LockDatabaseFile ();

    bool iret = false;
    while (1)
    {
        if (ff_seek (0) != true) break;
        if (ff_read (m_item_size_bytes) != true) break;
        if (ff_read (m_items_n) != true) break;
        if (ff_read (m_sizeof_custom_header) != true) break;

        if (m_item_size_bytes <= 0 || m_item_size_bytes > MAX_SIZE_DB_FLD_ITEM_BYTES ||
            m_items_n < 0 ||
            m_sizeof_custom_header < 0)
        {
            QString strMessage;
            strMessage.sprintf("Pos in DL4 file(%d) - ", BinaryDatabase::GetCurrentPos());

            if (m_item_size_bytes > MAX_SIZE_DB_FLD_ITEM_BYTES)
            {
                strMessage += " sizeof of fixed length item is greater than ";
                strMessage += QString::number((gsint32)MAX_SIZE_DB_FLD_ITEM_BYTES);
                strMessage += "bytes";
            }
            else
            if(m_item_size_bytes == 0)
                strMessage += " sizeof of fixed length item is null";
            else
                strMessage += " sizeof of fixed length item is negative";

            ErrorMessage ("Dl4 file corrupted",strMessage.toLatin1().constData());
            break;
        }

        m_custom_header = (void *) new unsigned char [m_sizeof_custom_header];
        if (!m_custom_header)
        {
            ErrorMessage ("Problem allocating memory for custom header.");
            break;
        }

        if (ff_read (m_custom_header,m_sizeof_custom_header) != true) break;

        if (ff_read (m_dimension_vld_offset) != true) break;
        if (ff_read (m_dimension_vld_size) != true) break;
        if (ff_read (m_database_file_name_id) != true) break;
        if (ff_read (m_status) != true) break;

        m_first_record_offset = compute_first_record_offset ();

        iret = true;
        break;
    }

    if (iret == false)
    {
        UnlockDatabaseFile ();
        Clear ();
        return false;
    }

    return true;
}


inline gsint32 BinaryDatabase::beginning_of_var_len_data (void)
{
    return m_first_record_offset + m_items_n*(m_item_size_bytes+8);
}

inline bool BinaryDatabase::ff_seek (const gsint32 &pos)
{
    if (m_opened_stationary)
    {
        m_opened_stationary_ff_pos = pos;
        return true;
    }
    return false;
}

inline bool BinaryDatabase::vf_seek (const gsint32 &pos)
{
    if (m_opened_stationary)
    {
        m_opened_stationary_vf_pos = pos + beginning_of_var_len_data ();
        return true;
    }
    return false;

}

bool BinaryDatabase::IsOpen (void)
{
    if (m_opened_stationary)
        return true;

    if (m_stream_fixed_len_data && m_stream_var_len_data)
        return true;

    return false;
}

inline bool BinaryDatabase::swap_order(void *passed_ptr,const gsint32 lSize)
{
    if(m_bSameCPUType)
        return true;

    unsigned char *hold,*ptr,last,count,i;

    ptr = (unsigned char *)passed_ptr;

    switch(lSize)
    {
        case 1:
            return true;
        case 2:
        {
            hold = new unsigned char[2];
//			ASSERT (hold);
            last = 1;
            count = 2;
            break;
        }
        case 4:
        {
            hold = new unsigned char[4];
//			ASSERT (hold);
            last = 3;
            count = 4;
            break;
        }
        case 8:
        {
            hold = new unsigned char[4];
//			ASSERT (hold);
            last = 7;
            count = 8;
            break;
        }
        default : return true;
    }

    for(i=0;i<count;i++)
        hold[last - i] = ptr[i];

    for(i=0;i<count;i++)
        ptr[i] = hold[i];

    delete [] hold;
    return true;
}


inline bool BinaryDatabase::ff_read (gsint32 &i)
{
    if (m_opened_stationary_ff_pos < 0)
    {
        ErrorMessage("ff_read");
        return false;
    }

    fseek(m_stream_locked_db_file,m_opened_stationary_ff_pos, SEEK_SET);
  if (fread(&i, sizeof(gsint32), 1, m_stream_locked_db_file) != 1) {
    ErrorMessage("ff_read");
    return false;
  }

    swap_order(&i,sizeof(gsint32));

    m_opened_stationary_ff_pos += sizeof(gsint32);
    return true;
}

inline bool BinaryDatabase::ff_read (DWORD &i)
{
    if (m_opened_stationary_ff_pos < 0)
    {
        ErrorMessage("ff_read");
        return false;
    }
    fseek(m_stream_locked_db_file,m_opened_stationary_ff_pos, SEEK_SET);
  if (fread(&i, sizeof(DWORD), 1, m_stream_locked_db_file) != 1) {
    ErrorMessage("ff_read");
    return false;
  }

    swap_order(&i,sizeof(DWORD));

    m_opened_stationary_ff_pos += sizeof(DWORD);
    return true;
}

inline bool BinaryDatabase::vf_read (void *v, const gsint32 n)
{
    if (!n) return true;

    if (m_opened_stationary_vf_pos < 0)
    {
        ErrorMessage("vf_read");
        return false;
    }

    fseek(m_stream_locked_db_file,m_opened_stationary_vf_pos, SEEK_SET);
  if (fread(v, n, 1, m_stream_locked_db_file) != 1) {
    ErrorMessage("vf_read");
    return false;
  }

    swap_order(v,n);

    m_opened_stationary_vf_pos += n;
    return true;

}

inline bool BinaryDatabase::vf_read_buffer (void *v, const gsint32 n)
{
    if (!n) return true;

    if (m_opened_stationary_vf_pos < 0)
    {
    ErrorMessage("vf_read_buffer");
        return false;
    }
    fseek(m_stream_locked_db_file,m_opened_stationary_vf_pos, SEEK_SET);
  if (fread(v, n, 1, m_stream_locked_db_file) != 1) {
    ErrorMessage("vf_read_buffer");
    return false;
  }
    m_opened_stationary_vf_pos += n;
    return true;

}

inline bool BinaryDatabase::ff_read (void *v, const gsint32 n)
{
    if (!n) return true;

    if (m_opened_stationary_ff_pos < 0)
    {
        ErrorMessage("ff_read");
        return false;
    }
    fseek(m_stream_locked_db_file,m_opened_stationary_ff_pos, SEEK_SET);
  if (fread(v, n, 1, m_stream_locked_db_file) != 1) {
    ErrorMessage("ff_read");
    return false;
  }

    swap_order(v,n);

    m_opened_stationary_ff_pos += n;
    return true;
}

inline bool BinaryDatabase::ff_read_buffer (void *v, const gsint32 n)
{
    if (!n) return true;

    if (m_opened_stationary_ff_pos < 0)
    {
        ErrorMessage("ff_read_buffer");
        return false;
    }
    fseek(m_stream_locked_db_file,m_opened_stationary_ff_pos, SEEK_SET);
  if (fread(v, n, 1, m_stream_locked_db_file) != 1) {
    ErrorMessage("ff_read_buffer");
    return false;
  }
    m_opened_stationary_ff_pos += n;
    return true;
}

bool BinaryDatabase::ReadFLD (GENERIC_STORED_OBJECT_WRAPPER &FLD, gsint32 rec_i)
{
    if ((rec_i < 0)
    || (rec_i >= ItemsTotalN ()))
    {
        ErrorMessage("ReadFLD");
        return false;
    }

    const gsint32 seek_pos = m_first_record_offset + rec_i*(m_item_size_bytes+8);

    if(!ff_seek (seek_pos))
        return false;

    if(!FLD.DynamicallyAllocateData (m_item_size_bytes))
    {
        ErrorMessage("ReadFLD");
        return false;
    }

    BYTE *b = new BYTE [m_item_size_bytes];
    if(!b)
    {
        ErrorMessage("Can't allocate enough memory to perform database operation.");
        return false;
    }

    if(!ff_read_buffer (b,m_item_size_bytes))
    {
        delete [] b;
        return false;
    }

    gsint32 sz = FLD.LoadFromBuffer (b);
    if (sz != m_item_size_bytes)
    {
        QString strMessage;
        strMessage.sprintf("Pos in DL4 file(%d) - Error message(Offset error)", BinaryDatabase::GetCurrentPos());
        ErrorMessage("Dl4 file corrupted",strMessage.toLatin1().constData());
        delete [] b;
        return false;
    }

    delete [] b;

    return true;
}

bool BinaryDatabase::ReadVLD (GENERIC_STORED_OBJECT_WRAPPER &VLD, gsint32 rec_i)
{
    if ((rec_i < 0)
    || (rec_i >= ItemsTotalN ()))
    {
        ErrorMessage("ReadVLD");
        return false;
    }

    BINARY_DATABASE_ITEM item;
    if(!Get (rec_i,item))
        return false;

    if(!VLD.DynamicallyAllocateData (item.VarLengthDataSize ()))
    {
        ErrorMessage("ReadVLD");
        return false;
    }

    gsint32 sz = VLD.LoadFromBuffer (item.VarLengthData ());

    if (sz != item.VarLengthDataSize ())
    {
        QString strMessage;
        strMessage.sprintf("Pos in DL4 file(%d) - Error message(Offset error)", BinaryDatabase::GetCurrentPos());
        ErrorMessage("Dl4 file corrupted",strMessage.toLatin1().constData());
        return false;
    }

    return true;
}

