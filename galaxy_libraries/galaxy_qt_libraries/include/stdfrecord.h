/****************************************************************************************/
/* Copyright Galaxy                                                                     */
/* This computer program is protected by copyright law                                  */
/* and international treaties. Unauthorized reproduction or                             */
/* distribution of this program, or any portion of it,may                               */
/* result in severe civil and criminal penalties, and will be                           */
/* prosecuted to the maximum extent possible under the law.                             */
/****************************************************************************************/
/*                                                                                      */
/* Notes :                                                                              */
/*                                                                                      */
/****************************************************************************************/

///////////////////////////////////////////////////////////
// StdfRecord class HEADER :
// This class contains classes to read and store STDF
// generic record
///////////////////////////////////////////////////////////

#ifndef _StdfRecords_h_
#define _StdfRecords_h_

#include <string.h>
#include <time.h>
#include <stdio.h>

 // Galaxy modules includes
#include "stdf.h"
#include <QMap>
#include <QString>

// Field manipulation macros
#define _FIELD_SET(setfunc,validationfunc,pos)\
    {\
        setfunc;\
        m_pFieldFlags[pos]	|= FieldFlag_Present;\
        if(validationfunc)	m_pFieldFlags[pos]	|= FieldFlag_Valid;\
        SetLastFieldAtdf(pos);\
    }

#define _FIELD_SET_FLAGS(validationfunc,pos)\
    {\
        m_pFieldFlags[pos]	|= FieldFlag_Present;\
        if(validationfunc)	m_pFieldFlags[pos]	|= FieldFlag_Valid;\
        SetLastFieldAtdf(pos);\
    }

#define _FIELD_CHECKREAD(readfunc,pos)\
    {\
        if(readfunc != GS::StdLib::Stdf::NoError)\
        {\
            if(pos < eposFIRST_OPTIONAL)	return false;\
            return true;\
        }\
    }

#define _FIELD_CHECKFILTER(value, field, pos)\
    {\
        if (m_pFilter)\
        {\
            if ((m_pFilter->m_pFieldFlags[pos] & FieldFlag_Present) && value != m_pFilter->field)\
                return false;\
        }\
    }

#define _LIST_ADDFIELD_ASCII(list,field,val,fieldselection,pos)\
    {\
        if((m_pFieldFlags[pos] & fieldselection) == fieldselection)\
        {\
            if(m_pFieldFlags[pos] & FieldFlag_Valid)\
            {\
                m_strTmp_macro = field;\
                m_strTmp_macro += ";";\
                m_strTmp_macro += val;\
            }\
            else if(m_pFieldFlags[pos] & FieldFlag_Present)\
            {\
                m_strTmp_macro = field;\
                m_strTmp_macro += ";<not valid> (";\
                m_strTmp_macro += val;\
                m_strTmp_macro += ")";\
            }\
            else\
            {\
                m_strTmp_macro = field;\
                m_strTmp_macro += ";<not present>";\
            }\
            list.append(m_strTmp_macro);\
        }\
    }

#define _FIELD_CHECKWRITE(writefieldfunc,pos,writerecordfunc)\
    {\
        if(m_pFieldFlags[pos] & FieldFlag_Present || pos < mLastFieldAtdf)\
        {\
            if(writefieldfunc != GS::StdLib::Stdf::NoError)	return false;\
        }\
        else\
        {\
            if(pos < eposFIRST_OPTIONAL)\
                return false;\
            writerecordfunc;\
            return true;\
        }\
    }

#define _FIELD_CHECKWRITE_KUi(writefieldfunc,array,size,pos,writerecordfunc)\
    {\
        if(m_pFieldFlags[pos] & FieldFlag_Present || pos < mLastFieldAtdf)\
        {\
            if (array == NULL)\
                return false;\
            for (int lIdx = 0; lIdx < size; ++lIdx)\
            {\
                if(writefieldfunc(array[lIdx]) != GS::StdLib::Stdf::NoError)\
                    return false;\
            }\
        }\
        else\
        {\
            if(pos < eposFIRST_OPTIONAL)\
                return false;\
            writerecordfunc;\
            return true;\
        }\
    }

#define _FIELD_CHECKWRITE_KCN(stdfObject,array,size,pos,writerecordfunc)\
    {\
        if(m_pFieldFlags[pos] & FieldFlag_Present || pos < mLastFieldAtdf)\
        {\
            if (array == NULL)\
                return false;\
            for (int lIdx = 0; lIdx < size; ++lIdx)\
            {\
                if(stdfObject.WriteString(array[lIdx].toLatin1().constData()) != GS::StdLib::Stdf::NoError)\
                    return false;\
            }\
        }\
        else\
        {\
            if(pos < eposFIRST_OPTIONAL)\
                return false;\
            writerecordfunc;\
            return true;\
        }\
    }

#define _STR_ADDFIELD_ASCII(string,field,val,fieldselection,pos)\
    {\
        if((m_pFieldFlags[pos] & fieldselection) == fieldselection)\
        {\
            if(m_pFieldFlags[pos] & FieldFlag_Valid)\
            {\
                m_strTmp2_macro = field;\
                m_strTmp_macro = m_strTmp2_macro.leftJustified(13);\
                m_strTmp_macro += " = ";\
                m_strTmp_macro += val;\
            }\
            else if(m_pFieldFlags[pos] & FieldFlag_Present)\
            {\
                m_strTmp2_macro = field;\
                m_strTmp_macro = m_strTmp2_macro.leftJustified(13);\
                m_strTmp_macro += " = <not valid> (";\
                m_strTmp_macro += val;\
                m_strTmp_macro += ")";\
            }\
            else\
            {\
                m_strTmp2_macro = field;\
                m_strTmp_macro = m_strTmp2_macro.leftJustified(13);\
                m_strTmp_macro += " = <not present>";\
            }\
            string += m_strTmp_macro;\
            string += "\n";\
        }\
    }

#define _STR_ADDFIELD_XML(string,tabs,field,val,fieldselection,pos)\
    {\
        if(((m_pFieldFlags[pos] & fieldselection) == fieldselection) && (m_pFieldFlags[pos] & FieldFlag_Present))\
        {\
            m_nTabs_macro = tabs;\
            m_strTmp_macro = "";\
            while(m_nTabs_macro-- > 0)\
                m_strTmp_macro += "\t";\
            m_strTmp_macro += "<";\
            m_strTmp_macro += field;\
            m_strTmp_macro += ">";\
            m_strTmp_macro += val;\
            m_strTmp_macro += "</";\
            m_strTmp_macro += field;\
            m_strTmp_macro += ">";\
            m_strTmp_macro += "\n";\
            string += m_strTmp_macro;\
        }\
    }

#define _STR_ADDLIST_XML(string,count,tabs,field,val,fieldselection,pos)\
    {\
        if(((m_pFieldFlags[pos] & fieldselection) == fieldselection) && (m_pFieldFlags[pos] & FieldFlag_Present))\
        {\
            m_nTabs_macro = tabs;\
            m_strTmp_macro = "";\
            while(m_nTabs_macro-- > 0)\
                m_strTmp_macro += "\t";\
            m_strTmp_macro += "<";\
            m_strTmp_macro += field;\
            m_strTmp_macro += " count=\"";\
            m_strTmp_macro += QString::number(count);\
            m_strTmp_macro += "\"";\
            m_strTmp_macro += ">";\
            m_strTmp_macro += val;\
            m_strTmp_macro += "</";\
            m_strTmp_macro += field;\
            m_strTmp_macro += ">";\
            m_strTmp_macro += "\n";\
            string += m_strTmp_macro;\
        }\
    }

#define _STR_ADDFIELD_ATDF(string,val,pos)\
    {\
        m_strTmp_macro = "";\
        if(m_pFieldFlags[pos] & FieldFlag_Valid)\
        {\
            /* Remove spaces at start and end. */\
            m_strTmp_macro = val.trimmed();\
            /* Remove CR/LF. */\
            m_strTmp_macro.remove("\n"); \
            /* If final string should exceed 80 characters, start a new line */\
            if(((unsigned int)(string.length() +\
                m_strTmp_macro.length())) >= m_uiLineNb*ATDF_LINE_MAX_SIZE)\
            {\
                int nCharPos = ATDF_LINE_MAX_SIZE - \
                                string.length() % ATDF_LINE_MAX_SIZE;\
                while(nCharPos<=m_strTmp_macro.size())\
                {\
                    /* Insert CR character in order to cut lines in the right position */\
                    m_strTmp_macro.insert(nCharPos-1, QString("\n "));\
                    nCharPos += ATDF_LINE_MAX_SIZE;\
                }\
                m_uiLineNb++;\
            }\
        }\
        if( (string.size()%ATDF_LINE_MAX_SIZE) == 0)\
        {\
            if(m_uiFieldIndex > 0)\
            string += QString("\n ");\
        }\
        if(m_uiFieldIndex > 0)\
            m_strTmp_macro = "|" + m_strTmp_macro;\
        string += m_strTmp_macro;\
        m_uiFieldIndex++;\
    }

#define _STR_FINISHFIELD_ATDF(string)\
    {\
    string.remove(QRegExp("[\\|\\r\\n\\s]+$")); \
    string += '\n';\
    }

#define _CREATEFIELD_FROM_C1_ASCII(data)\
    {\
        m_c1Value_macro = data;\
        /* Some STDF file had 0 in this field, causing a 'end of string' in the ASCII string */\
        if(m_c1Value_macro == 0)\
            m_c1Value_macro = ' ';\
        sprintf(m_szTmp_macro, "%c", m_c1Value_macro);\
        m_strFieldValue_macro = m_szTmp_macro;\
    }

#define _CREATEFIELD_FROM_C1_XML(data)\
    {\
        m_c1Value_macro = data;\
        /* Some STDF file had 0 in this field, causing a 'end of string' in the XML string */\
        if(m_c1Value_macro == 0)\
            m_c1Value_macro = ' ';\
        sprintf(m_szTmp_macro, "%c", m_c1Value_macro);\
        m_strFieldValue_macro = m_szTmp_macro;\
        m_strFieldValue_macro.replace("<", "&lt;");\
        m_strFieldValue_macro.replace(">", "&gt;");\
    }

#define _CREATEFIELD_FROM_KUi_ASCII(count,data,pos)\
    {\
        m_strFieldValue_macro = "";\
        if(m_pFieldFlags[pos] & FieldFlag_Present)\
        {\
            if(count > 0)\
            {\
                sprintf(m_szTmp_macro, "\n        [000] = %u", data[0]);\
                m_strFieldValue_macro = m_szTmp_macro;\
            }\
            for(m_uiIndex_macro=1; m_uiIndex_macro<(unsigned int)count; m_uiIndex_macro++)\
            {\
                sprintf(m_szTmp_macro, "\n        [%03u] = %u", m_uiIndex_macro, data[m_uiIndex_macro]);\
                m_strFieldValue_macro += m_szTmp_macro;\
            }\
        }\
    }

#define _CREATEFIELD_FROM_KU8_ASCII(count,data,pos)\
    {\
        m_strFieldValue_macro = "";\
        if(m_pFieldFlags[pos] & FieldFlag_Present)\
        {\
            if(count > 0)\
            {\
                sprintf(m_szTmp_macro, "\n        [000] = %ld", (unsigned long)data[0]);\
                m_strFieldValue_macro = m_szTmp_macro;\
            }\
            for(m_uiIndex_macro=1; m_uiIndex_macro<(unsigned int)count; m_uiIndex_macro++)\
            {\
                sprintf(m_szTmp_macro, "\n        [%03u] = %ld", m_uiIndex_macro, (unsigned long)data[m_uiIndex_macro]);\
                m_strFieldValue_macro += m_szTmp_macro;\
            }\
        }\
    }


#define _CREATEFIELD_FROM_KUi_XML(count,data,pos)\
    {\
        m_strFieldValue_macro = "";\
        if(m_pFieldFlags[pos] & FieldFlag_Present)\
        {\
            if(count > 0)\
            {\
                sprintf(m_szTmp_macro, "%u", data[0]);\
                m_strFieldValue_macro = m_szTmp_macro;\
            }\
            for(m_uiIndex_macro=1; m_uiIndex_macro<(unsigned int)count; m_uiIndex_macro++)\
            {\
                sprintf(m_szTmp_macro, " %u", data[m_uiIndex_macro]);\
                m_strFieldValue_macro += m_szTmp_macro;\
            }\
        }\
    }

#define _CREATEFIELD_FROM_KU8_XML(count,data,pos)\
    {\
        m_strFieldValue_macro = "";\
        if(m_pFieldFlags[pos] & FieldFlag_Present)\
        {\
            if(count > 0)\
            {\
                sprintf(m_szTmp_macro, "%ld", (unsigned long)data[0]);\
                m_strFieldValue_macro = m_szTmp_macro;\
            }\
            for(m_uiIndex_macro=1; m_uiIndex_macro<(unsigned int)count; m_uiIndex_macro++)\
            {\
                sprintf(m_szTmp_macro, " %ld", (unsigned long)data[m_uiIndex_macro]);\
                m_strFieldValue_macro += m_szTmp_macro;\
            }\
        }\
    }

#define _CREATEFIELD_FROM_KU8_ATDF(count,data,pos)\
    {\
        m_strFieldValue_macro = "";\
        if((m_pFieldFlags[pos] & FieldFlag_Present) && (count > 0))\
        {\
            sprintf(m_szTmp_macro, "%ld", (unsigned long)data[0]);\
            m_strFieldValue_macro = m_szTmp_macro;\
            for(m_uiIndex_macro=1; m_uiIndex_macro<(unsigned int)count; m_uiIndex_macro++)\
            {\
                sprintf(m_szTmp_macro, ",%ld", (unsigned long)data[m_uiIndex_macro]);\
                m_strFieldValue_macro += m_szTmp_macro;\
            }\
        }\
    }

#define _CREATEFIELD_FROM_KUi_ATDF(count,data,pos)\
    {\
        m_strFieldValue_macro = "";\
        if((m_pFieldFlags[pos] & FieldFlag_Present) && (count > 0))\
        {\
            sprintf(m_szTmp_macro, "%u", data[0]);\
            m_strFieldValue_macro = m_szTmp_macro;\
            for(m_uiIndex_macro=1; m_uiIndex_macro<(unsigned int)count; m_uiIndex_macro++)\
            {\
                sprintf(m_szTmp_macro, ",%u", data[m_uiIndex_macro]);\
                m_strFieldValue_macro += m_szTmp_macro;\
            }\
        }\
    }

#define _CONVERT_STDF_TO_ATDF_PLR_GRP_RADX(radix)\
            switch(radix)\
            {\
                case 2:\
                    m_c1Value_macro = 'B';\
                    break;\
                case 8:\
                    m_c1Value_macro = 'O';\
                    break;\
                case 10:\
                    m_c1Value_macro = 'D';\
                    break;\
                case 16:\
                    m_c1Value_macro = 'H';\
                    break;\
                case 20:\
                    m_c1Value_macro = 'S';\
                    break;\
                case 0:\
                default:\
                    m_c1Value_macro = ' ';\
                    break;\
            }\

#define _CREATEFIELD_FROM_KUi_ATDF_PLR_GRP_RADX(count,data,pos)\
    {\
        m_strFieldValue_macro = "";\
        if((m_pFieldFlags[pos] & FieldFlag_Present) && (count > 0))\
        {\
            m_uiIndex_macro = (unsigned int)count-1;\
            _CONVERT_STDF_TO_ATDF_PLR_GRP_RADX(data[m_uiIndex_macro])\
            sprintf(m_szTmp_macro, "%c", m_c1Value_macro);\
            m_strFieldValue_macro = m_szTmp_macro;\
            while(m_uiIndex_macro > 0)\
            {\
                m_uiIndex_macro--;\
                _CONVERT_STDF_TO_ATDF_PLR_GRP_RADX(data[m_uiIndex_macro])\
                if (m_c1Value_macro == ' ')\
                    sprintf(m_szTmp_macro, ",");\
                else\
                    sprintf(m_szTmp_macro, "%c,", m_c1Value_macro);\
                m_strFieldValue_macro = QString(m_szTmp_macro) + m_strFieldValue_macro;\
            }\
        }\
    }

#define _CREATEFIELD_FROM_KIi_ASCII(count,data,pos)\
    {\
        m_strFieldValue_macro = "";\
        if(m_pFieldFlags[pos] & FieldFlag_Present)\
        {\
            if(count > 0)\
            {\
                sprintf(m_szTmp_macro, "\n        [000] = %d", data[0]);\
                m_strFieldValue_macro = m_szTmp_macro;\
            }\
            for(m_uiIndex_macro=1; m_uiIndex_macro<(unsigned int)count; m_uiIndex_macro++)\
            {\
                sprintf(m_szTmp_macro, "\n        [%03u] = %d", m_uiIndex_macro, data[m_uiIndex_macro]);\
                m_strFieldValue_macro += m_szTmp_macro;\
            }\
        }\
    }

#define _CREATEFIELD_FROM_KIi_XML(count,data,pos)\
    {\
        m_strFieldValue_macro = "";\
        if(m_pFieldFlags[pos] & FieldFlag_Present)\
        {\
            if(count > 0)\
            {\
                sprintf(m_szTmp_macro, "%d", data[0]);\
                m_strFieldValue_macro = m_szTmp_macro;\
            }\
            for(m_uiIndex_macro=1; m_uiIndex_macro<(unsigned int)count; m_uiIndex_macro++)\
            {\
                sprintf(m_szTmp_macro, " %d", data[m_uiIndex_macro]);\
                m_strFieldValue_macro += m_szTmp_macro;\
            }\
        }\
    }

#define _CREATEFIELD_FROM_KRi_ASCII(count,data,pos)\
    {\
        m_strFieldValue_macro = "";\
        if(m_pFieldFlags[pos] & FieldFlag_Present)\
        {\
            if(count > 0)\
            {\
                sprintf(m_szTmp_macro, "\n        [000] = %g", data[0]);\
                m_strFieldValue_macro = m_szTmp_macro;\
            }\
            for(m_uiIndex_macro=1; m_uiIndex_macro<(unsigned int)count; m_uiIndex_macro++)\
            {\
                sprintf(m_szTmp_macro, "\n        [%03u] = %g", m_uiIndex_macro, data[m_uiIndex_macro]);\
                m_strFieldValue_macro += m_szTmp_macro;\
            }\
        }\
    }

#ifdef WIN32
#define _CREATEFIELD_FROM_KRi_ATDF(count,data,pos)\
    {\
        unsigned int lOldFormat = _set_output_format(_TWO_DIGIT_EXPONENT);\
        m_strFieldValue_macro = "";\
        if( (m_pFieldFlags[pos] & FieldFlag_Present) && count>0 )\
        {\
            m_uiIndex_macro = (unsigned int)0; \
            sprintf(m_szTmp_macro, "%g", data[m_uiIndex_macro]);\
            m_strFieldValue_macro = m_szTmp_macro;\
            while(m_uiIndex_macro < (unsigned int)count - 1) \
            {\
                m_uiIndex_macro++;\
                sprintf(m_szTmp_macro, ",%g", data[m_uiIndex_macro]);\
                m_strFieldValue_macro += m_szTmp_macro;\
            }\
        }\
        _set_output_format(lOldFormat);\
    }
#else
#define _CREATEFIELD_FROM_KRi_ATDF(count,data,pos)\
    {\
        m_strFieldValue_macro = "";\
        if( (m_pFieldFlags[pos] & FieldFlag_Present) && count>0 )\
        {\
            m_uiIndex_macro = (unsigned int)0; \
            sprintf(m_szTmp_macro, "%g", data[m_uiIndex_macro]);\
            m_strFieldValue_macro = m_szTmp_macro;\
            while(m_uiIndex_macro < (unsigned int)count - 1) \
            {\
                m_uiIndex_macro++;\
                sprintf(m_szTmp_macro, ",%g", data[m_uiIndex_macro]);\
                m_strFieldValue_macro += m_szTmp_macro;\
            }\
        }\
    }

#endif

// gcore 178 : dont use %g as it can create non X platform e+/-000 notation. Let s use %f: no scientific notation.
// Let's also use given precision : max seems to be 24 digits for doubles ?
// previous implementation was:
// sprintf(m_szTmp_macro, "\n        [000] = %s",
//  QString::number( (float)((float)data[0]),'f',mPrecision).toLatin1().data());
// sprintf(m_szTmp_macro, "\n        [%03u] = %s", m_uiIndex_macro,
//  QString::number((float)data[m_uiIndex_macro],'f',mPrecision).toLatin1().data() );
// Let do a QString::number implementation in order to control precision
#define _CREATEFIELD_FROM_KRi_ASCII_NANCHECK(count,data,nan,pos)\
    {\
        m_strFieldValue_macro = "";\
        if(m_pFieldFlags[pos] & FieldFlag_Present)\
        {\
            if(count > 0)\
            {\
                if(nan[0])\
                    sprintf(m_szTmp_macro, "\n        [000] = NaN");\
                else\
                    sprintf(m_szTmp_macro, "\n        [000] = %.24f", data[0]);\
                    m_strFieldValue_macro = m_szTmp_macro;\
            }\
            for(m_uiIndex_macro=1; m_uiIndex_macro<(unsigned int)count; m_uiIndex_macro++)\
            {\
                if(nan[m_uiIndex_macro])\
                    sprintf(m_szTmp_macro, "\n        [%03u] = NaN", m_uiIndex_macro);\
                else\
                    sprintf(m_szTmp_macro, "\n        [%03u] = %.24f", m_uiIndex_macro, data[m_uiIndex_macro]);\
                    m_strFieldValue_macro += m_szTmp_macro;\
            }\
        }\
    }

#define _CREATEFIELD_FROM_KRi_XML(count,data,pos)\
    {\
        m_strFieldValue_macro = "";\
        if(m_pFieldFlags[pos] & FieldFlag_Present)\
        {\
            if(count > 0)\
            {\
                sprintf(m_szTmp_macro, "%g", data[0]);\
                m_strFieldValue_macro = m_szTmp_macro;\
            }\
            for(m_uiIndex_macro=1; m_uiIndex_macro<(unsigned int)count; m_uiIndex_macro++)\
            {\
                sprintf(m_szTmp_macro, " %g", data[m_uiIndex_macro]);\
                m_strFieldValue_macro += m_szTmp_macro;\
            }\
        }\
    }

#define _CREATEFIELD_FROM_KRi_XML_NANCHECK(count,data,nan,pos)\
    {\
        m_strFieldValue_macro = "";\
        if(m_pFieldFlags[pos] & FieldFlag_Present)\
        {\
            if(count > 0)\
            {\
                if(nan[0])\
                    sprintf(m_szTmp_macro, "NaN");\
                else\
                    sprintf(m_szTmp_macro, "%g", data[0]);\
                m_strFieldValue_macro = m_szTmp_macro;\
            }\
            for(m_uiIndex_macro=1; m_uiIndex_macro<(unsigned int)count; m_uiIndex_macro++)\
            {\
                if(nan[m_uiIndex_macro])\
                    sprintf(m_szTmp_macro, " NaN");\
                else\
                    sprintf(m_szTmp_macro, " %g", data[m_uiIndex_macro]);\
                m_strFieldValue_macro += m_szTmp_macro;\
            }\
        }\
    }

#define _CREATEFIELD_FROM_CN_XML(data)\
    {\
        m_strFieldValue_macro = data;\
        m_strFieldValue_macro.replace("<", "&lt;");\
        m_strFieldValue_macro.replace(">", "&gt;");\
    }

#define _CREATEFIELD_FROM_KCN_ASCII(count,data,pos)\
    {\
        m_strFieldValue_macro = "";\
        if(m_pFieldFlags[pos] & FieldFlag_Present)\
        {\
            if(count > 0)\
            {\
                sprintf(m_szTmp_macro, "\n        [000] = %s", data[0].toLatin1().constData());\
                m_strFieldValue_macro = m_szTmp_macro;\
            }\
            for(m_uiIndex_macro=1; m_uiIndex_macro<(unsigned int)count; m_uiIndex_macro++)\
            {\
                sprintf(m_szTmp_macro, "\n        [%03u] = %s", m_uiIndex_macro, data[m_uiIndex_macro].toLatin1().constData());\
                m_strFieldValue_macro += m_szTmp_macro;\
            }\
        }\
    }

#define _CREATEFIELD_FROM_KCN_ATDF(count,data,pos)\
    {\
        m_strFieldValue_macro = "";\
        if((m_pFieldFlags[pos] & FieldFlag_Present) && count > 0)\
        {\
            m_uiIndex_macro = (unsigned int)0; \
            sprintf(m_szTmp_macro, "%s", data[0].toLatin1().constData());\
            m_strFieldValue_macro = m_szTmp_macro;\
            while(m_uiIndex_macro < (unsigned int)count - 1) \
            {\
                m_uiIndex_macro++;\
                sprintf(m_szTmp_macro, ",%s", data[m_uiIndex_macro].toLatin1().constData());\
                m_strFieldValue_macro += m_szTmp_macro;\
            }\
        }\
    }

/////////////////////////////////////////////////////////////////
// convert a cn data in a string
// char*        => QString
// '3abc'       => 'abc'
/////////////////////////////////////////////////////////////////
#define _CONVERT_CN_TO_STRING(cnData, strField)\
{\
    int nDataCount = *cnData; \
    strField.clear(); \
    for(int ii=1; ii<=nDataCount; ii++) \
        strField += *(cnData+ii); \
}

////////////////////////////////////////////////////////////////////////////////
// convert a kcn field in a k-list of string (if the field is
// not present, only clear the list.
// QString*                        => QStringList
// 'abc' '' 'a' '' '' 'ab' 'abc'   => 'abc', '', 'a', '', '', 'ab', 'abc'
////////////////////////////////////////////////////////////////////////////////
#define _CONVERT_kCN_TO_STRINGLIST(nCount, strKCnData, qslStringList, pos)\
{\
    qslStringList.clear(); \
    if((m_pFieldFlags[pos]&FieldFlag_Present) && (nCount>0) && (strKCnData!=NULL))\
    {\
        for(int ii=0; ii<nCount; ii++) \
            qslStringList.append(strKCnData[ii]); \
    } \
}

////////////////////////////////////////////////////////////////////////////////
// convert char/chal qstringlists to an atdf compliant qstring
// 'abc', 'def', 'ghi'; '123', '', '456'    => 'a1,b2,c3/d,e,f/g4,h5,i6'
////////////////////////////////////////////////////////////////////////////////
#define _CONVERT_CHAR_CHAL_STRINGLISTS_TO_ATDF(qslCharStringList, qslChalStringList, strAtdfField)\
{\
    int nListsSize = qslCharStringList.count(); \
    strAtdfField.clear(); \
    if( ((nListsSize == qslChalStringList.count()) || (qslChalStringList.count()==0)) && (nListsSize != 0) ) \
    {\
        QString strCharList, strChalList; \
        for(int ii=0; ii<nListsSize; ii++) \
        {\
            strCharList = qslCharStringList.at(ii); \
            if(!qslChalStringList.isEmpty()) \
                strChalList = qslChalStringList.at(ii); \
            else \
                strChalList.clear(); \
            bool bIsUsedChal = !(strChalList.isEmpty()); \
            int nCharListSize = strCharList.size(); \
            if(bIsUsedChal) \
                if (nCharListSize != strChalList.size()) \
                    GSLOG(3, "nCharListSize != strChalList.size()"); \
            for(int jj=0; jj<nCharListSize; jj++) \
            {\
                if(bIsUsedChal) \
                { \
                    strAtdfField.append(strChalList.at(jj)); \
                    strAtdfField.append(strCharList.at(jj)); \
                } \
                else \
                    strAtdfField.append(strCharList.at(jj)); \
                strAtdfField.append(QString(",")); \
            }\
            strAtdfField.remove(QRegExp(",$")); \
            strAtdfField.append(QString("/")); \
        }\
        strAtdfField.remove(QRegExp("/$")); \
        /* each qslCharStringList element was empty */\
    if(strAtdfField.size() == qslCharStringList.count() -1)\
            strAtdfField.clear();\
    }\
}

#define _CREATEFIELD_FROM_KCN_XML(count,data,pos)\
    {\
        m_strFieldValue_macro = "";\
        if(m_pFieldFlags[pos] & FieldFlag_Present)\
        {\
            if(count > 0)\
            {\
                m_strFieldValue_macro = data[0];\
                m_strFieldValue_macro.replace("<", "&lt;");\
                m_strFieldValue_macro.replace(">", "&gt;");\
            }\
            for(m_uiIndex_macro=1; m_uiIndex_macro<(unsigned int)count; m_uiIndex_macro++)\
            {\
                m_strTmp_macro = data[m_uiIndex_macro];\
                m_strTmp_macro.replace("<", "&lt;");\
                m_strTmp_macro.replace(">", "&gt;");\
                m_strFieldValue_macro += " ";\
                m_strFieldValue_macro += m_strTmp_macro;\
            }\
        }\
    }

#define _CREATEFIELD_FROM_N1_ASCII(data)\
    {\
        sprintf(m_szTmp_macro, "0x%02x", (int)data & 0x0ff);\
        m_strFieldValue_macro = m_szTmp_macro;\
    }

#define _CREATEFIELD_FROM_N1_ATDF(data)\
    {\
        sprintf(m_szTmp_macro, "%02X", (int)data & 0x0ff);\
        m_strFieldValue_macro = m_szTmp_macro;\
    }

#define _CREATEFIELD_FROM_N1_XML(data)\
    {\
        sprintf(m_szTmp_macro, "%02x", (int)data & 0x0ff);\
        m_strFieldValue_macro = m_szTmp_macro;\
    }

#define _CREATEFIELD_FROM_B1_ASCII(data)\
    {\
        sprintf(m_szTmp_macro, "0x%02x", (int)data & 0x0ff);\
        m_strFieldValue_macro = m_szTmp_macro;\
    }

#define _CREATEFIELD_FROM_B1_ATDF(data)\
    {\
        sprintf(m_szTmp_macro, "%02x", (int)data & 0x0ff);\
        m_strFieldValue_macro = m_szTmp_macro;\
    }

#define _CREATEFIELD_FROM_B1_XML(data)\
    {\
        sprintf(m_szTmp_macro, "%02x", (int)data & 0x0ff);\
        m_strFieldValue_macro = m_szTmp_macro;\
    }

#define _CREATEFIELD_FROM_BN_ASCII(data)\
    {\
        m_strFieldValue_macro = "";\
        if(data.m_bLength > 0)\
        {\
            m_strFieldValue_macro = "0x";\
            m_ptChar_macro = data.m_pBitField;\
            for(m_iIndex_macro=(data.m_bLength-1); m_iIndex_macro>=0; --m_iIndex_macro)\
            {\
                sprintf(m_szTmp_macro, "%02x", (int)m_ptChar_macro[m_iIndex_macro] & 0x0ff);\
                m_strFieldValue_macro += m_szTmp_macro;\
            }\
        }\
    }

#define _CREATEFIELD_FROM_BN_ATDF(data)\
    {\
        m_strFieldValue_macro = "";\
        if(data.m_bLength > 0)\
        {\
            m_ptChar_macro = data.m_pBitField;\
            for(m_iIndex_macro=(data.m_bLength-1); m_iIndex_macro>=0; --m_iIndex_macro)\
            {\
                sprintf(m_szTmp_macro, "%02X", (int)m_ptChar_macro[m_iIndex_macro] & 0x0ff);\
                m_strFieldValue_macro += m_szTmp_macro;\
            }\
        }\
    }

#define _CREATEFIELD_FROM_BN_XML(data)\
    {\
        m_strFieldValue_macro = "";\
        if(data.m_bLength > 0)\
        {\
            m_ptChar_macro = data.m_pBitField;\
            for(m_iIndex_macro=(data.m_bLength-1); m_iIndex_macro>=0; --m_iIndex_macro)\
            {\
                sprintf(m_szTmp_macro, "%02x", (int)m_ptChar_macro[m_iIndex_macro] & 0x0ff);\
                m_strFieldValue_macro += m_szTmp_macro;\
            }\
        }\
    }

#define _CREATEFIELD_FROM_DN_ASCII(data)\
    {\
        m_strFieldValue_macro = "";\
        if(data.m_uiLength > 0)\
        {\
            m_strFieldValue_macro = "0x";\
            m_ptChar_macro = data.m_pBitField;\
            for(m_uiIndex_macro=data.m_uiLength-1; m_uiIndex_macro>0; m_uiIndex_macro--)\
            {\
                sprintf(m_szTmp_macro, "%02x", (int)m_ptChar_macro[m_uiIndex_macro] & 0x0ff);\
                m_strFieldValue_macro += m_szTmp_macro;\
            }\
            sprintf(m_szTmp_macro, "%02x", (int)m_ptChar_macro[0] & 0x0ff);\
            m_strFieldValue_macro += m_szTmp_macro;\
        }\
    }

#define _CREATEFIELD_FROM_DN_ATDF(data)\
    {\
        m_strFieldValue_macro = "";\
        if(data.m_uiLength > 0)\
        {\
            m_ptChar_macro = data.m_pBitField;\
            for(m_uiIndex_macro=data.m_uiLength-1; m_uiIndex_macro>0; m_uiIndex_macro--)\
            {\
                sprintf(m_szTmp_macro, "%02X", (int)m_ptChar_macro[m_uiIndex_macro] & 0x0ff);\
                m_strFieldValue_macro += m_szTmp_macro;\
            }\
            sprintf(m_szTmp_macro, "%02X", (int)m_ptChar_macro[0] & 0x0ff);\
            m_strFieldValue_macro += m_szTmp_macro;\
        }\
    }

#define _CREATEFIELD_FROM_DN_XML(data)\
    {\
        m_strFieldValue_macro = "";\
        if(data.m_uiLength > 0)\
        {\
            m_ptChar_macro = data.m_pBitField;\
            for(m_uiIndex_macro=data.m_uiLength-1; m_uiIndex_macro>0; m_uiIndex_macro--)\
            {\
                sprintf(m_szTmp_macro, "%02x", (int)m_ptChar_macro[m_uiIndex_macro] & 0x0ff);\
                m_strFieldValue_macro += m_szTmp_macro;\
            }\
            sprintf(m_szTmp_macro, "%02x", (int)m_ptChar_macro[0] & 0x0ff);\
            m_strFieldValue_macro += m_szTmp_macro;\
        }\
    }

#define _CREATEFIELD_FROM_DN_ATDF_FTR_PINS(data)\
    {\
        unsigned int uiIndexInByte, uiPmrIndex=0; \
        char cByte;\
        m_strFieldValue_macro = "";\
        if(data.m_uiLength > 0)\
        {\
            m_ptChar_macro = data.m_pBitField;\
            for(m_uiIndex_macro=0; m_uiIndex_macro<data.m_uiLength; m_uiIndex_macro++)\
            {\
                cByte = m_ptChar_macro[m_uiIndex_macro];\
                for(uiIndexInByte=0; uiIndexInByte<8; uiIndexInByte++)\
                {\
                    if((cByte >> uiIndexInByte) & 0x01)\
                    {\
                        if(m_strFieldValue_macro.isEmpty())\
                            sprintf(m_szTmp_macro, "%d", uiPmrIndex);\
                        else\
                            sprintf(m_szTmp_macro, ",%d", uiPmrIndex);\
                        m_strFieldValue_macro += m_szTmp_macro;\
                    }\
                    uiPmrIndex++;\
                }\
            }\
        }\
    }

#define _CREATEFIELD_FROM_KN1_ASCII(count,data,pos)\
    {\
        m_strFieldValue_macro = "";\
        if(m_pFieldFlags[pos] & FieldFlag_Present)\
        {\
            for(m_uiIndex_macro=0; m_uiIndex_macro<(unsigned int)count; m_uiIndex_macro++)\
            {\
                sprintf(m_szTmp_macro, "\n        [%03u] = 0x%01x", m_uiIndex_macro, (int)data[m_uiIndex_macro]);\
                m_strFieldValue_macro += m_szTmp_macro;\
            }\
        }\
    }

#define _CREATEFIELD_FROM_KN1_ATDF(count,data,pos)\
    {\
        m_strFieldValue_macro = "";\
        if((m_pFieldFlags[pos] & FieldFlag_Present) && (count > 0))\
        {\
            m_uiIndex_macro = (unsigned int)0; \
            sprintf(m_szTmp_macro, "%X", (int)data[m_uiIndex_macro]);\
            m_strFieldValue_macro = m_szTmp_macro;\
            while(m_uiIndex_macro < (unsigned int)count - 1) \
            {\
                m_uiIndex_macro++;\
                sprintf(m_szTmp_macro, ",%X", (int)data[m_uiIndex_macro]);\
                m_strFieldValue_macro += m_szTmp_macro;\
            }\
        }\
    }

#define _CREATEFIELD_FROM_KN1_XML(count,data,pos)\
    {\
        m_strFieldValue_macro = "";\
        if(m_pFieldFlags[pos] & FieldFlag_Present)\
        {\
            for(m_uiIndex_macro=0; m_uiIndex_macro<(unsigned int)count; m_uiIndex_macro++)\
            {\
                sprintf(m_szTmp_macro, " %01x", (int)data[m_uiIndex_macro]);\
                m_strFieldValue_macro += m_szTmp_macro;\
            }\
        }\
    }

// Some constant definition
#define ATDF_LINE_MAX_SIZE      79
#define INVALID_SMALLINT		-32768
#define INVALID_INT				4294967295lu
// Some default values
#define STDF_MAX_U1				255
#define STDF_MAX_U2				65535
// Position in byte
#define STDF_MASK_BIT0			0x01
#define STDF_MASK_BIT1			0x02
#define STDF_MASK_BIT2			0x04
#define STDF_MASK_BIT3			0x08
#define STDF_MASK_BIT4			0x10
#define STDF_MASK_BIT5			0x20
#define STDF_MASK_BIT6			0x40
#define STDF_MASK_BIT7			0x80
// STDF record types/subtypes
#define STDF_FAR_TYPE   0
#define STDF_FAR_STYPE  10
#define STDF_ATR_TYPE   0
#define STDF_ATR_STYPE  20
#define STDF_MIR_TYPE   1
#define STDF_MIR_STYPE  10
#define STDF_MRR_TYPE   1
#define STDF_MRR_STYPE  20
#define STDF_PCR_TYPE   1
#define STDF_PCR_STYPE  30
#define STDF_HBR_TYPE   1
#define STDF_HBR_STYPE  40
#define STDF_SBR_TYPE   1
#define STDF_SBR_STYPE  50
#define STDF_PMR_TYPE   1
#define STDF_PMR_STYPE  60
#define STDF_PGR_TYPE   1
#define STDF_PGR_STYPE  62
#define STDF_PLR_TYPE   1
#define STDF_PLR_STYPE  63
#define STDF_RDR_TYPE   1
#define STDF_RDR_STYPE  70
#define STDF_SDR_TYPE   1
#define STDF_SDR_STYPE  80
#define STDF_WIR_TYPE   2
#define STDF_WIR_STYPE  10
#define STDF_WRR_TYPE   2
#define STDF_WRR_STYPE  20
#define STDF_WCR_TYPE   2
#define STDF_WCR_STYPE  30
#define STDF_PIR_TYPE   5
#define STDF_PIR_STYPE  10
#define STDF_PRR_TYPE   5
#define STDF_PRR_STYPE  20
#define STDF_TSR_TYPE   10
#define STDF_TSR_STYPE  30
#define STDF_PTR_TYPE   15
#define STDF_PTR_STYPE  10
#define STDF_MPR_TYPE   15
#define STDF_MPR_STYPE  15
#define STDF_FTR_TYPE   15
#define STDF_FTR_STYPE  20
#define STDF_BPS_TYPE   20
#define STDF_BPS_STYPE  10
#define STDF_EPS_TYPE   20
#define STDF_EPS_STYPE  20
#define STDF_GDR_TYPE   50
#define STDF_GDR_STYPE  10
#define STDF_DTR_TYPE   50
#define STDF_DTR_STYPE  30
#define STDF_RESERVED_IMAGE_TYPE	180
#define STDF_RESERVED_IG900_TYPE	181

// STDF V4-2007 specific
#define STDF_VUR_TYPE   0
#define STDF_VUR_STYPE  30
#define STDF_PSR_TYPE   1
#define STDF_PSR_STYPE  90
#define STDF_NMR_TYPE   1
#define STDF_NMR_STYPE  91
#define STDF_CNR_TYPE   1
#define STDF_CNR_STYPE  92
#define STDF_SSR_TYPE   1
#define STDF_SSR_STYPE  93
#define STDF_CDR_TYPE   1
#define STDF_CDR_STYPE  94
#define STDF_STR_TYPE   15
#define STDF_STR_STYPE  30

// STDF V3 specific
#define STDF_SCR_TYPE   25
#define STDF_SCR_STYPE  40
#define STDF_PDR_TYPE   10
#define STDF_PDR_STYPE  10
#define STDF_SHB_TYPE   25
#define STDF_SHB_STYPE  10
#define STDF_SSB_TYPE   25
#define STDF_SSB_STYPE  20
#define STDF_PDR_TYPE   10
#define STDF_PDR_STYPE  10
#define STDF_FDR_TYPE   10
#define STDF_FDR_STYPE  20
#define STDF_STS_TYPE   25
#define STDF_STS_STYPE  30
#define GSIZE_DEFAULT	256

// STDF type mapping
typedef char                stdf_type_c1;
typedef QString             stdf_type_cn;
typedef char                stdf_type_b1;
typedef unsigned char       stdf_type_u1;
typedef unsigned short      stdf_type_u2;
typedef unsigned long       stdf_type_u4;
typedef unsigned long long	stdf_type_u8;
typedef char                stdf_type_i1;
typedef qint16              stdf_type_i2;
typedef qint32              stdf_type_i4;
typedef float               stdf_type_r4;
typedef double              stdf_type_r8;
typedef char                stdf_type_n1;

namespace GQTL_STDF
{

/*
B*n	Variable length bit-encoded field:	char[]
First byte = unsigned count of bytes to follow (maximum of 255 bytes).
First data item in least significant bit of the second byte of the array (first byte is count.)
*/
struct stdf_type_bn
{
    stdf_type_bn()
    {
        Clear();
    }

    void Clear()
    {
        m_bLength = 0;
        memset(m_pBitField, 0, STDF_MAX_U1+1);
    }

    // count in nb. of bytes stored in m_pBitField
    BYTE	m_bLength;
    // data
    char	m_pBitField[STDF_MAX_U1+1];

    stdf_type_bn& operator=(const stdf_type_bn& source)
    {
        m_bLength = source.m_bLength;
        if(m_bLength > 0)
            memcpy((char *)m_pBitField, (char *)(source.m_pBitField), m_bLength);
        return *this;
    }
};
/*
D*n	Variable length bit-encoded field:	char[]
First two bytes = unsigned COUNT OF BITS to follow (maximum of 65,535 bits).
First data item in least significant bit of the third byte of the array (first two bytes are count).
Unused bits at the high order end of the last byte must be zero.
*/
struct stdf_type_dn
{
    stdf_type_dn()
    {
        Clear();
    }

    // !!!! count in NB. OF BYTES stored in m_pBitField !!!!
    unsigned int	m_uiLength;
    // data
    char			m_pBitField[(STDF_MAX_U2+1)/8];

    stdf_type_dn& operator=(const stdf_type_dn& source)
    {
        m_uiLength = source.m_uiLength;
        if(m_uiLength > 0)
            memcpy((char *)m_pBitField, (char *)(source.m_pBitField), m_uiLength);
        return *this;
    }

    void Clear()
    {
        m_uiLength = 0;
        memset(m_pBitField, 0, (STDF_MAX_U2+1)/8);
    }
};
struct stdf_type_vn
{
    stdf_type_vn()
    {
        Reset();
    }

    void Reset()
    {
        uiDataTypeCode = eTypePad;
        m_bnData.Clear();
        m_cnData.clear();
        m_dnData.Clear();
    }

    unsigned int	uiDataTypeCode;
    union
    {
        stdf_type_u1	m_u1Data;
        stdf_type_u2	m_u2Data;
        stdf_type_u4	m_u4Data;
        stdf_type_i1	m_i1Data;
        stdf_type_i2	m_i2Data;
        stdf_type_i4	m_i4Data;
        stdf_type_r4	m_r4Data;
        stdf_type_r8	m_r8Data;
        stdf_type_n1	m_n1Data;
    };
    stdf_type_bn		m_bnData;
    stdf_type_dn		m_dnData;
    stdf_type_cn		m_cnData;
    enum DataType {
            eTypePad				= 0,
            eTypeU1					= 1,
            eTypeU2					= 2,
            eTypeU4					= 3,
            eTypeI1					= 4,
            eTypeI2					= 5,
            eTypeI4					= 6,
            eTypeR4					= 7,
            eTypeR8					= 8,
            eTypeCN					= 10,
            eTypeBN					= 11,
            eTypeDN					= 12,
            eTypeN1					= 13
    };

    stdf_type_vn& operator=(const stdf_type_vn& source)
    {
        uiDataTypeCode = source.uiDataTypeCode;
        switch(uiDataTypeCode)
        {
        case eTypeU1:
            m_u1Data = source.m_u1Data;
            break;
        case eTypeU2:
            m_u2Data = source.m_u2Data;
            break;
        case eTypeU4:
            m_u4Data = source.m_u4Data;
            break;
        case eTypeI1:
            m_i1Data = source.m_i1Data;
            break;
        case eTypeI2:
            m_i2Data = source.m_i2Data;
            break;
        case eTypeI4:
            m_i4Data = source.m_i4Data;
            break;
        case eTypeR4:
            m_r4Data = source.m_r4Data;
            break;
        case eTypeR8:
            m_r8Data = source.m_r8Data;
            break;
        case eTypeN1:
            m_n1Data = source.m_n1Data;
            break;
        case eTypeCN:
            m_cnData = source.m_cnData;
            break;
        case eTypeBN:
            m_bnData = source.m_bnData;
            break;
        case eTypeDN:
            m_dnData = source.m_dnData;
            break;
        default:
            break;
        }
        return *this;
    }
};


///////////////////////////////////////////////////////////
// GENERIC RECORD
///////////////////////////////////////////////////////////
class Stdf_Record
{
protected:
    //! \brief Desired precision to use for float/double value for some records (PTR, MPR, ...)
    int mPrecision;
    //! \brief Format for outputting float/double records (PTR, MPR): 'e', 'f', 'a'
    char mFormat;

public:
    // Constructor / destructor functions
    Stdf_Record();
    Stdf_Record(const Stdf_Record& other);
	// Use a virtual destructor to avoid warning under gcc (Class has virtual functions, but destructor is not virtual).
    virtual ~Stdf_Record();

    Stdf_Record& operator=(const Stdf_Record& other);

    enum FieldFlags {
            FieldFlag_Empty			= 0x00,
            FieldFlag_None			= 0x01,
            FieldFlag_Present		= 0x02,
            FieldFlag_Valid			= 0x04,
            FieldFlag_ReducedList	= 0x08
    };

    enum EOPT_FLAG	{	eNOTVALID_TEST_MIN	=0x1,	// See complete flags definition in STDF specification
                        eNOTVALID_TEST_MAX	=0x2,	// version 4, page 37
                        eNOTVALID_TEST_TIM	=0x4,
                        eRESERVED_BIT3		=0x8,
                        eNOTVALID_TST_SUMS	=0x10,
                        eNOTVALID_TST_SQRS	=0x20,
                        eRESERVED_BIT6		=0x40,
                        eRESERVED_BIT7		=0x80,
                        eOPT_FLAG_ALL		=0xFF
    };
	// Reset record data
    virtual void		Reset(void) = 0;
	// Return short name of the record
	virtual QString		GetRecordShortName(void);
	// Return long name of the record
	virtual QString		GetRecordLongName(void);
	// Return record type
	virtual int			GetRecordType(void);
	// Read record
    virtual bool		Read(GS::StdLib::Stdf & clStdf);
	// Write record
    virtual bool		Write(GS::StdLib::Stdf & clStdf);
	// Construct a stringlist with all fields to display
	// Each string in the list has following format: <Field name>;<Field value>
	virtual void		GetAsciiFieldList(QStringList & listFields, int nFieldSelection = 0);
    //! \brief Construct a ASCII string of the record
    virtual void		GetAsciiString(QString & strAsciiString, int nFieldSelection = 0); // char format='e' ?
	// Construct a XML string of the record
	virtual void		GetXMLString(QString & strXmlString, const int nIndentationLevel, int nFieldSelection = 0);
	// Construct a ATDF string of the record
	virtual void		GetAtdfString(QString & strAtdfString);
	
    //! \brief Construct a ASCII string of the record
    //! \arg todo ? If floating point record type, use the given format : 'e' (scientific notation) or 'g' (regular with 6 precison)
    //! \arg todo ? precision : number of digits after decimal for float/double records
    void GetAsciiRecord(QString &strRecord, int nFieldSelection = 0 /* char format='e', int lPrecision=6*/);
    //! \brief Set precision for float/double records type
    void SetPrecision(const int &lPrecision);
    //! \brief Set output format for float/double records: 'e', 'f', 'a'.
    void SetFormat(char);
    // setter for the mLastFieldAtdf
    void SetLastFieldAtdf(const int lLastField);

// DATA
public:
	// STDF record types
    enum RecordTypes {
			Rec_FAR					= 0,		// FAR record
			Rec_ATR					= 1,		// ATR record
			Rec_MIR					= 2,		// MIR record
            Rec_MRR					= 3,		// MRR record
            Rec_PCR					= 4,		// PCR record
            Rec_HBR					= 5,		// HBR record
            Rec_SBR					= 6,		// SBR record
            Rec_PMR					= 7,		// PMR record
            Rec_PGR					= 8,		// PGR record
            Rec_PLR					= 9,		// PLR record
            Rec_RDR					= 10,		// RDR record
            Rec_SDR					= 11,		// SDR record
            Rec_WIR					= 12,		// WIR record
            Rec_WRR					= 13,		// WRR record
            Rec_WCR					= 14,		// WCR record
            Rec_PIR					= 15,		// PIR record
            Rec_PRR					= 16,		// PRR record
            Rec_TSR					= 17,		// TSR record
            Rec_PTR					= 18,		// PTR record
            Rec_MPR					= 19,		// MPR record
            Rec_FTR					= 20,		// FTR record
            Rec_BPS					= 21,		// BPS record
            Rec_EPS					= 22,		// EPS record
            Rec_GDR					= 23,		// GDR record
            Rec_DTR					= 24,		// DTR record
			Rec_RESERVED_IMAGE		= 25,		// Reserved for use by Image software
			Rec_RESERVED_IG900		= 26,		// Reserved for use by IG900 software
            Rec_UNKNOWN				= 27,		// Unknown record type.
	    	Rec_VUR = 29,
	    	Rec_PSR = 30,
	    	Rec_NMR = 31,
	    	Rec_CNR = 32,
	    	Rec_SSR = 33,
	    	Rec_SCR = 34,
	    	Rec_STR = 35,
	    	Rec_CDR = 36,
            Rec_SHB = 37,
            Rec_SSB = 38,
            Rec_PDR = 39,
            Rec_FDR = 40,
            Rec_STS = 41,
            Rec_COUNT				= 42 		// !!!! Marker used for array sizes. !!!! Must be (last+1). !!!! Always leave as last in the list !!!!!
	};

	// Record information
    GS::StdLib::StdfRecordReadInfo	m_stRecordInfo;

protected:
	// Some protected variables used by the macros
    char				m_szTmp_macro[1024];
    unsigned int        m_uiIndex_macro;
    int                 m_iIndex_macro;
	int					m_nTabs_macro;
	const char*			m_ptChar_macro;
	QString				m_strFieldValue_macro;
	QString				m_strTmp_macro;
	QString				m_strTmp2_macro;
	stdf_type_c1		m_c1Value_macro;
	unsigned int		m_uiFieldIndex;
	// unsigned int		m_uiOptionalFieldIndex;
    unsigned int		m_uiLineNb;
    int                 mLastFieldAtdf;
};


class CStdfRecordFactory{

public:
    static Stdf_Record *recordInstanciate(int nRecord);
    static Stdf_Record *recordInstance(int nRecord);
    static void clear();
protected:
    static QMap<int, Stdf_Record*> m_oRecordMap;

};
}

#endif // #ifndef _CStdfRecords_V4_h_
