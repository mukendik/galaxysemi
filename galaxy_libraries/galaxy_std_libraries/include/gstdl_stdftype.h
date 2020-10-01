// gstdftype.h
// ----------------------------------------------------------------------------------------------------------
//
// Notes: * To be able to export STL symbols from this DLL, use technic describe in 
//			Microsoft Knowledge Base Article - 168958 
//			http://support.microsoft.com/default.aspx?scid=KB;en-us;q168958
//
// ----------------------------------------------------------------------------------------------------------

#ifndef _GALAXY_STDF_TYPE_HEADER_
#define _GALAXY_STDF_TYPE_HEADER_

/*
#if defined(_WIN32)
#	if defined(_GALAXY_STDF_TYPE_MODULE_)
#		define GSTDFTYPE_API __declspec(dllexport)
#	elif defined(_GALAXY_STDF_DLL_MODULE)
#		define GSTDFTYPE_API
#	else	// _GALAXY_STDF_TYPE_MODULE_
#		define GSTDFTYPE_API __declspec(dllimport)
#	endif // _GALAXY_STDF_TYPE_MODULE_
#else
#	define GSTDFTYPE_API
#endif
  */

#define GSTDFTYPE_API

#include <gstdl_type.h>

// ----------------------------------------------------------------------------------------------------------
// class CGStdfBitField

class GSTDFTYPE_API CGStdfBitField
{
// Constructor / Destructor
public:
	CGStdfBitField(WORD wDefSize = 32);
	CGStdfBitField(const CGStdfBitField& source);
	~CGStdfBitField();

// Attributes
public:
	WORD		GetLength() const { return m_wLength; } // Return the number of bits available in this bit field

// Operation
public:
	void		SetBit(WORD wBitIndex,bool bValue = true);
	BOOL		GetBit(WORD wBitIndex,bool& bValue) const;
	void		ToString(PSTR *pszBitField) const;
    BYTE*       GetBitField() const;

	CGStdfBitField& operator=(const CGStdfBitField& source);
	BOOL operator==(const CGStdfBitField& source) const;

protected:
	BYTE	*m_pbBitField;
	WORD	m_wLength;			// Number of valid bit represented by the objects
	WORD	m_wLengthAlloc;		// Number of bytes allocated to store the bits

	friend class CGStdfFile;
};

typedef CGStdfBitField	BITFIELD;


// ----------------------------------------------------------------------------------------------------------
// class CGStdfBitField
// B*n data type
// pyc, 29/04/2011, case 4717

class GSTDFTYPE_API CGStdfVariableLengthByteField
{
// Constructor / Destructor
public:
	CGStdfVariableLengthByteField();
	CGStdfVariableLengthByteField(const CGStdfVariableLengthByteField& source);
	~CGStdfVariableLengthByteField();

// Accessor(s)
public:
	WORD		GetLength() const { return m_wLength+1; } // Return the number of bytes available in this byte field (count starts at 0)
    int         GetMaxLenght() const { return m_nByteFieldMaxLength; }
    BYTE*       GetByteField() const  {return m_byPtrByteField;}

// Operation
public:
	bool		SetByte(const WORD wByteIndex,const char chCharacterToSet);		// return false if problem occured
	BYTE		GetByte(const WORD wByteIndex,bool& bPtrIsValidByte) const;		// *bPtrIsValidByte is false if problem occured
	bool		ToString(PSTR pszByteField) const;								// return false if problem occured
    bool		SetString(PSTR pszStringToCopy=NULL);							// return false if problem occured

	CGStdfVariableLengthByteField& operator=(const CGStdfVariableLengthByteField& source);
	bool operator==(const CGStdfVariableLengthByteField& source) const;

protected:
	const int		m_nByteFieldMaxLength;
	BYTE*			m_byPtrByteField;
	WORD			m_wLength;			// index of the last byte (length of the field -1, starts at 0
};

typedef CGStdfVariableLengthByteField	VARIABLELENGTHBYTEFIELD;


// ----------------------------------------------------------------------------------------------------------
// class CGStdfGenData

class GSTDFTYPE_API CGStdfGenData
{
public:
	CGStdfGenData();
	~CGStdfGenData();
public:
	void	Reset();

	enum ETYPE	{	eTypeByte		= 1,
					eTypeWord		= 2,
					eTypeDWord		= 3,
					eTypeChar		= 4,
					eTypeShort		= 5,
					eTypeInt		= 6,
					eTypeFloat		= 7,
					eTypeDouble		= 8,
					eTypeString		= 10,
					eTypeBinString	= 11,
					eTypeBitField	= 12,
					eTypeNibble		= 13
	};

	DWORD		m_dwType;

	union
	{
		BYTE		m_bData;
		WORD		m_wData;
		DWORD		m_dwData;
		CHAR		m_cData;
		SHORT		m_sData;
		INT32		m_iData;
		FLOAT		m_fData;
		DOUBLE		m_dfData;
        CHAR*		m_szData;
		BYTE		m_nData; // Nibble
	};
    VARIABLELENGTHBYTEFIELD	m_vlbfData;
	BITFIELD	m_dnData;			
};

typedef CGStdfGenData	GENDATA;

#endif // _GALAXY_STDF_TYPE_HEADER_

