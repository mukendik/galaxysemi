#define _GALAXY_STDF_TYPE_MODULE_
#include "gstdl_stdftype.h"
#include <stdlib.h>
#include <memory.h>
#include <math.h>

#include <gstdl_macro.h>

// ----------------------------------------------------------------------------------------------------------
// CGStdfBitField class implementation



// ----------------------------------------------------------------------------------------------------------
// Description: Construct
//
// Argument(s):
//      WORD wDefSize: Default number of bits for this bit field.
// ----------------------------------------------------------------------------------------------------------
CGStdfBitField::CGStdfBitField(WORD wDefSize /*= 32*/)
{
	if(wDefSize % 8)
		m_wLengthAlloc = wDefSize/8+1;
	else
		m_wLengthAlloc = wDefSize/8;

	m_pbBitField = new BYTE[m_wLengthAlloc];
	memset(m_pbBitField,0,m_wLengthAlloc);
	m_wLength = 0; // No bit set for the moment, so length set to 0
}

// Copy constructor
CGStdfBitField::CGStdfBitField(const CGStdfBitField& source)
{
	this->operator=(source);
}

CGStdfBitField::~CGStdfBitField()
{
	ASSERT(m_wLength  <= m_wLengthAlloc * 8 + 1);

	delete [] m_pbBitField;
}

// Copy the specified bit field to this one
CGStdfBitField& CGStdfBitField::operator=(const CGStdfBitField& source)
{
	if(&source != this)
	{
		delete [] m_pbBitField;
		m_pbBitField = new BYTE[source.m_wLengthAlloc];

		memcpy(m_pbBitField,source.m_pbBitField,source.m_wLengthAlloc);
		m_wLength = source.m_wLength;
		m_wLengthAlloc = source.m_wLengthAlloc;
	}
	return *this;
}

// Compare this bitfield with the specified one. Return TRUE if equal; otherwise return FALSE.
BOOL CGStdfBitField::operator==(const CGStdfBitField& source) const
{
	if(&source != this)
	{
		if(m_wLength != source.m_wLength)
			return FALSE;

		for(WORD wLoop=0;wLoop<m_wLengthAlloc;wLoop++)
		{
			if(*(m_pbBitField+wLoop) != *(source.m_pbBitField+wLoop))
				return FALSE;
		}
	}
	return TRUE;
}

// Set the value of the specified bit (*** warning: 0 indexed bit index)
// Increase bitfield size if necessary.
void CGStdfBitField::SetBit(WORD wBitIndex,bool bValue /*= true*/)
{
	// Allocate a new bit field if current size is too small
    if (wBitIndex > (m_wLengthAlloc - 1) * sizeof(BYTE) * 8)
	{
		WORD	wOldSize = m_wLengthAlloc;
		m_wLengthAlloc = wBitIndex/8+1;

		BYTE* pNewBitField = new BYTE[m_wLengthAlloc];
		memset(pNewBitField,0,m_wLengthAlloc);
		memcpy(pNewBitField,m_pbBitField,wOldSize);
		delete [] m_pbBitField;
		m_pbBitField = pNewBitField;
	}
	if(wBitIndex >= m_wLength)
		m_wLength = wBitIndex+1; // 0 based index, so length is equal to max index + 1

	BYTE	bFlags;
	div_t	divResult;

	divResult = div( int(wBitIndex), sizeof(BYTE) * 8 );

	bFlags = *(m_pbBitField+divResult.quot);

	if(bValue == true)
		// Set the flag
		bFlags |= (1 << divResult.rem);
	else
		// Remove the flag
		bFlags &= ~(1 << divResult.rem);
	// Store the new flag value in the array
	*(m_pbBitField+divResult.quot) = bFlags;
}

// Get the value of the specified bit (*** warning: 0 indexed bit index)
BOOL CGStdfBitField::GetBit(WORD wBitIndex,bool& bValue) const
{
	if(wBitIndex > m_wLengthAlloc * sizeof(BYTE) * 8)
		return FALSE; // Out of range

	BYTE	bFlags;
	div_t	divResult;

	divResult = div( int(wBitIndex), sizeof(BYTE) * 8 );
	bFlags = *(m_pbBitField+divResult.quot);

	bValue = (bFlags & (1 << divResult.rem)) > 0 ? true : false;

	return TRUE;
}

// Convert the bitfield content to a string containing '0' and '1' characters.
// Automatically allocate memory for the string.
// So user must free itself the string.
void CGStdfBitField::ToString(PSTR *pszBitField) const
{
    bool	bValue = false;
	*pszBitField = new CHAR[m_wLength+1];

	for(WORD wBit=0;wBit<m_wLength;wBit++)
	{
		VERIFY(GetBit(wBit,bValue));
		(*pszBitField)[wBit] = bValue == true ? '1' : '0';
	}
	(*pszBitField)[m_wLength] = '\0';
}

BYTE* CGStdfBitField::GetBitField() const
{
    return m_pbBitField;
}





// ----------------------------------------------------------------------------------------------------------
// CGStdfVariableLengthByteField class implementation


// Constructor / Destructor
CGStdfVariableLengthByteField::CGStdfVariableLengthByteField() : m_nByteFieldMaxLength(255)
{
	m_byPtrByteField = new BYTE[m_nByteFieldMaxLength];
	memset(m_byPtrByteField,0,m_nByteFieldMaxLength);
	m_wLength = 0; // No bit set for the moment, so length set to 0
}

CGStdfVariableLengthByteField::CGStdfVariableLengthByteField(const CGStdfVariableLengthByteField& source) : m_nByteFieldMaxLength(255)
{
	this->operator=(source);
}

CGStdfVariableLengthByteField::~CGStdfVariableLengthByteField()
{
	if(m_byPtrByteField)
	{
		delete [] m_byPtrByteField;
		m_byPtrByteField = NULL;
	}
}

// Accessor(s)

// Operation
bool	CGStdfVariableLengthByteField::SetByte(const WORD wByteIndex,const char chCharacterToSet)
{
	if (wByteIndex > m_nByteFieldMaxLength)
		return false;
	if( !( ((chCharacterToSet>='0')&&(chCharacterToSet<='9'))	||
		  ((chCharacterToSet>='a')&&(chCharacterToSet<='f'))	||
		  ((chCharacterToSet>='A')&&(chCharacterToSet<='F'))	))
		return false;		// set character has to represent a hexadecimal value !


	if(wByteIndex>m_wLength)
		m_wLength=wByteIndex;

	BYTE byCharacterToSet = chCharacterToSet;
	m_byPtrByteField[wByteIndex]=byCharacterToSet;

	// everything went well
	return true;
}

BYTE	CGStdfVariableLengthByteField::GetByte(const WORD wByteIndex,bool& bPtrIsValidByte) const
{
	bPtrIsValidByte = true;
	BYTE byRslt=0;

	if((wByteIndex > m_nByteFieldMaxLength) || (wByteIndex > m_wLength))
	{
		bPtrIsValidByte = false;
		return byRslt;
	}

	byRslt = m_byPtrByteField[wByteIndex];

	return byRslt;
}

bool	CGStdfVariableLengthByteField::ToString(PSTR pszByteField) const		// !! PSTR = char* (pointer)
{
	int nStringSize = GetLength();
	nStringSize = (nStringSize>m_nByteFieldMaxLength) ? m_nByteFieldMaxLength : nStringSize ;

	for(WORD wByteLoop=0;wByteLoop<nStringSize;wByteLoop++)
	{
		(pszByteField)[wByteLoop] = m_byPtrByteField[wByteLoop];
	}
	(pszByteField)[nStringSize] = '\0';

	// everything went well
	return true;
}

// SetString(NULL) reset
bool	CGStdfVariableLengthByteField::SetString(PSTR pszStringToCopy/*=NULL*/)							// return false if problem occured
{
	PSTR pstrPtrLoopPointer;
	int nStringSize=0;
	bool bSetStringRslt=true;

	pstrPtrLoopPointer = pszStringToCopy;
    if(pszStringToCopy)
    {
        while( (*pstrPtrLoopPointer!= '\0') && bSetStringRslt)
        {
            nStringSize++;
            pstrPtrLoopPointer++;

            bSetStringRslt = (nStringSize<m_nByteFieldMaxLength) ;
        }
    }

	if(bSetStringRslt)
	{
		memset(m_byPtrByteField, 0, m_nByteFieldMaxLength);		// reset
		m_wLength = nStringSize;

		for(int ii=0; ii<nStringSize; ii++)
		{
			bSetStringRslt &= SetByte(ii, *(pszStringToCopy+ii));
		}
	}

	return bSetStringRslt;
}


CGStdfVariableLengthByteField& CGStdfVariableLengthByteField::operator=(const CGStdfVariableLengthByteField& source)
{
	if(&source != this)
	{
		// reset memory
		memset(m_byPtrByteField, 0, m_wLength);
		m_wLength = source.m_wLength;
		memcpy(m_byPtrByteField, source.m_byPtrByteField, m_wLength);
	}
	return *this;
}

bool CGStdfVariableLengthByteField::operator==(const CGStdfVariableLengthByteField& source) const
{
	if(&source != this)
	{
		if(m_wLength != source.m_wLength)
			return false;

		for(WORD wLoop=0; wLoop<m_wLength; wLoop++)
			if( *(m_byPtrByteField+wLoop) != *(source.m_byPtrByteField+wLoop) )
				return false;
	}

	return true;
}




// ----------------------------------------------------------------------------------------------------------
// CGStdfGenDataList class implementation


CGStdfGenData::CGStdfGenData()
{
	Reset();
}

CGStdfGenData::~CGStdfGenData()
{
    if (m_szData != NULL)
        delete[] m_szData;
}

void CGStdfGenData::Reset()
{
	m_dwType = 0;
    m_bData = 0;
    m_wData = 0;
    m_dwData = 0;
    m_cData = 0;
    m_sData = 0;
    m_iData = 0;
    m_fData = 0;
    m_dfData = 0;
    if (m_szData != NULL)
        delete[] m_szData;
    m_szData = new CHAR[255];
    memset(m_szData, 0, 255);
    m_nData = 0;

    m_vlbfData.SetString(NULL); // reset
}
