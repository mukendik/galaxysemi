// PkInternalInfo.h: interface for the CPkInternalInfo class.
////////////////////////////////////////////////////////////////////////////////

#if !defined(_WININTERNALINFO_H__C6749101_590C_4F74_8121_B82E3BE9FA44__INCLUDED_)
#define _WININTERNALINFO_H__C6749101_590C_4F74_8121_B82E3BE9FA44__INCLUDED_

// Project headers
//#include "gqtl_zlib.h"
#include <zlib.h>

// Standard headers

// QT headers

class CPkAutoBuffer;
class CPkInternalInfo  
{
public:
	DWORD m_iBufferSize;
	z_stream m_stream;
	DWORD m_uUncomprLeft;
	DWORD m_uComprLeft;
	DWORD m_uCrc32;
	void Init();
	CPkAutoBuffer m_pBuffer;
	CPkInternalInfo();
	virtual ~CPkInternalInfo();

};

#endif // !defined(_WININTERNALINFO_H__C6749101_590C_4F74_8121_B82E3BE9FA44__INCLUDED_)
