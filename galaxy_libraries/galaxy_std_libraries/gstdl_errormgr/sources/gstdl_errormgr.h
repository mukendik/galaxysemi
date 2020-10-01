// ----------------------------------------------------------------------------------------------------------
// gerrormgr.h
// ----------------------------------------------------------------------------------------------------------
//
// Purpose:
//		Define a standard mechanism to handle error code and message within all Galaxy libraries and 
//		software. This mechanism does not use standard C++ exception to be used under environments that
//		don't support C++ exception.
//
//		For more information, see the Microsoft Word related document.
// ----------------------------------------------------------------------------------------------------------
//
// Notes: 
//
// ----------------------------------------------------------------------------------------------------------

#ifndef _GALAXY_ERROR_MANAGER_HEADER_
#define _GALAXY_ERROR_MANAGER_HEADER_

#if defined(_WIN32)
#ifdef _MSC_VER
	#pragma warning(disable : 4251)
	#pragma warning(disable : 4786)
#endif
#endif

// Include STL string header, and...
#include <string>

#include <vector>
#include <stdio.h>
#include <stdarg.h>
#include <gstdl_type.h>

#if 0
#if defined(_WIN32)
#	if defined(_GALAXY_ERRORMGR_EXPORTS_)
#		define GERRORMGR_API __declspec(dllexport)
#	elif defined _GALAXY_ERROR_MANAGER_DLL_MODULE
#		define GERRORMGR_API
#	else	// _GALAXY_ERRORMGR_EXPORTS_
#		define GERRORMGR_API __declspec(dllimport)
#	endif // _GALAXY_ERRORMGR_EXPORTS_
#else // defined(_WIN32)
#	define GERRORMGR_API
#endif // defined(_WIN32)
#endif

#define GERRORMGR_API

#define GEM_MAX_ERRORSTRING		1024*100		// Size used for temporary string buffer; should be enough in any case!

// ----------------------------------------------------------------------------------------------------------
// DECLARATION MECHANISM MACROS
// ----------------------------------------------------------------------------------------------------------
// Due to Sun Sparc 5.0 Compiler limitation, we need to provide class name as parameter
// for these 2 macros (With Microsoft compiler it would be possible to have a parameter only for the first one)

#define GDECLARE_ERROR_MAP(className)\
	public:\
	enum ErrorCode##className

#define GDECLARE_END_ERROR_MAP(className)\
	;\
	void XGFormatError##className(ErrorCode##className eError,PCSTR lpcszHeader = NULL, const CGErrorMgr* pclError = NULL,...) const;\
	mutable CGErrorMgr		m_clLastError##className;\
	mutable ErrorCode##className	m_eLastErrorCode##className;\
	const CGErrorMgr*	GetLastError##className() const { return &m_clLastError##className; }\
	ErrorCode##className	GetLastErrorCode##className() const { return m_eLastErrorCode##className; }

// ----------------------------------------------------------------------------------------------------------
// IMPLEMENTATION MECHANISM MACROS
// ----------------------------------------------------------------------------------------------------------

#define GBEGIN_ERROR_MAP(className)	void className::XGFormatError##className(ErrorCode##className eError,\
														PCSTR lpcszHeader, const CGErrorMgr* pclError,...) const\
									{\
										va_list	arglist;\
										std::string		strError;\
										char			szBuffer[GEM_MAX_ERRORSTRING];\
										if(pclError)\
											m_clLastError##className = *pclError;\
										else\
											m_clLastError##className.Reset();\
										m_eLastErrorCode##className = eError;\
										switch(eError)\
										{\
										default:\
												strError = "Unknow error";\
												break;

#define	GMAP_ERROR(e,string)			case e:\
											{\
												va_start(arglist, pclError);\
												vsprintf(szBuffer,string,arglist);\
												if(lpcszHeader)\
													strError = lpcszHeader;\
												strError += szBuffer;\
												va_end(arglist);\
											}\
											break;


#define GEND_ERROR_MAP(className)		} /* switch */\
										m_clLastError##className += strError;\
									}

// ----------------------------------------------------------------------------------------------------------
// MACROS TO USE ERROR MANAGEMENT MECHANISM
// ----------------------------------------------------------------------------------------------------------

// Set error
#define GSET_ERROR(className,errorCode)\
					XGFormatError##className(errorCode,NULL,NULL)
#define GSET_ERROR0(className,errorCode,error_pointer)\
					XGFormatError##className(errorCode,NULL,error_pointer)
#define GSET_ERROR1(className,errorCode,error_pointer,a1)\
					XGFormatError##className(errorCode,NULL,error_pointer,a1)
#define GSET_ERROR2(className,errorCode,error_pointer,a1,a2)\
					XGFormatError##className(errorCode,NULL,error_pointer,a1,a2)
#define GSET_ERROR3(className,errorCode,error_pointer,a1,a2,a3)\
					XGFormatError##className(errorCode,NULL,error_pointer,a1,a2,a3)
#define GSET_ERROR4(className,errorCode,error_pointer,a1,a2,a3,a4)\
					XGFormatError##className(errorCode,NULL,error_pointer,a1,a2,a3,a4)
#define GSET_ERROR5(className,errorCode,error_pointer,a1,a2,a3,a4,a5)\
					XGFormatError##className(errorCode,NULL,error_pointer,a1,a2,a3,a4,a5)

#define GSET_ERRORH(className,errorCode,header)\
					XGFormatError##className(errorCode,header,NULL)
#define GSET_ERRORH0(className,errorCode,error_pointer,header)\
					XGFormatError##className(errorCode,header,error_pointer)
#define GSET_ERRORH1(className,errorCode,error_pointer,a1,header)\
					XGFormatError##className(errorCode,header,error_pointer,a1)
#define GSET_ERRORH2(className,errorCode,error_pointer,a1,a2,header)\
					XGFormatError##className(errorCode,header,error_pointer,a1,a2)
#define GSET_ERRORH3(className,errorCode,error_pointer,a1,a2,a3,header)\
					XGFormatError##className(errorCode,header,error_pointer,a1,a2,a3)
#define GSET_ERRORH4(className,errorCode,error_pointer,a1,a2,a3,a4,header)\
					XGFormatError##className(errorCode,header,error_pointer,a1,a2,a3,a4)
#define GSET_ERRORH5(className,errorCode,error_pointer,a1,a2,a3,a4,a5,header)\
					XGFormatError##className(errorCode,header,error_pointer,a1,a2,a3,a4,a5)

// Retrieve object error status
#define GGET_LASTERROR(className,object_pointer)						(&((object_pointer)->m_clLastError##className))
#define GGET_LASTERRORCODE(className,object_pointer)					((object_pointer)->m_eLastErrorCode##className)				

#define GSET_MSGLEVEL(className,object_pointer,level)					(object_point)->m_clLastError##className.SetMessageLevel(level)
#define GGET_LASTERRORMSG(className,object_pointer)						(object_pointer)->m_clLastError##className.GetErrorMessage().c_str()
#define GGET_LASTERRORMSG2(className,object_pointer,level)				(object_pointer)->m_clLastError##className.GetErrorMessage(level).c_str()
#define GGET_LASTERRORMSG3(className,object_pointer,strErrorMsg)		(object_pointer)->m_clLastError##className.GetErrorMessage(strErrorMsg)
#define GGET_LASTERRORMSG4(className,object_pointer,strErrorMsg,level)	(object_pointer)->m_clLastError##className.GetErrorMessage(strErrorMsg,level)

#define GGET_ERRORSTRINGLIST(className,object_pointer) (object_pointer)->m_clLastError##className.GetStringErrorList();

// Retrieve error code type name
#define GECODE_TYPENAME(className)		ErrorCode##className 	


// ----------------------------------------------------------------------------------------------------------
// Galaxy Warning management allows you to have a list of warnings for an object.
// Unlike Galaxy Error management, the list of messages belong to the object that
// use this feature.
// ----------------------------------------------------------------------------------------------------------

// Declare these macros in your class definition (.h) to enable
// Galaxy Warning management feature
//
// e.g. :	class CMyClassName
//			{
//				GDECLARE_WARNING_MAP(CMyClassName)
//				{
//					eOkay,
//					eFileOpen,
//					eCloseFile
//				}
//				GDECLARE_END_WARNING_MAP(CMyClassName)
//			};
//	
// Notes :
//			All members implemented are 'mutable', and XGFormatWarning has got the 'const' attribute.
//			So you can format warning messages in your functions with 'const' attributes. 
//

#define GDECLARE_WARNING_MAP(className)\
	public:\
	enum WarningCode##className

// Because of Sun Sparc Compiler limitation, we cannot declare a enum like this :
//			enum EEnumName;			like a class.
// We need to provide entire enum declaration, so we cannot have simply one macro
// to declare the error map.
#define GDECLARE_END_WARNING_MAP(className)\
	;\
	void XGFormatWarning##className(WarningCode##className eWarning,PCSTR lpcszHeader = NULL,...) const;\
	mutable CGErrorMgr		m_clLastWarning##className;\
	mutable WarningCode##className	m_eLastWarningCode##className;\
	const CGErrorMgr*	GetLastWarning##className() const { return &m_clLastWarning##className; }\
	WarningCode##className	GetLastWarningCode##className() const { return m_eLastWarningCode##className; }

// Declare these macros in your class implementation (.cpp) to enable
// Galaxy Error management feature
//
// e.g. :	GBEGIN_ERROR_MAP(CMyClassName)
//				GMAP_ERROR(eFileOpen,_T("Cannot open file : %s"))
//				GMAP_ERROR(eCloseFile,_T("Cannot close file : %s"))
//			GEND_ERROR_MAP(CMyClassName)

#define GBEGIN_WARNING_MAP(className)	void className::XGFormatWarning##className(WarningCode##className eWarning,\
														PCSTR lpcszHeader,...) const\
									{\
										va_list	arglist;\
										std::string	strWarning;\
										char		szBuffer[GEM_MAX_ERRORSTRING];\
										m_eLastWarningCode##className = eWarning;\
										switch(eWarning)\
										{\
										default:\
												strWarning = _T("Unknow warning");\
												break;

#define	GMAP_WARNING(e,string)			case e:\
											{\
												va_start(arglist, lpcszHeader);\
												if(lpcszHeader)\
													strWarning = lpcszHeader;\
												vsprintf(szBuffer,string,arglist);\
												strWarning += szBuffer;\
												va_end(arglist);\
											}\
											break;

#define GEND_WARNING_MAP(className)		} /* switch */\
										m_clLastWarning##className += strWarning;\
									}

// Use this macro when you want to set an error message
//
// e.g. :		CString	strFileName;
//				...
//				GSET_ERROR(eFileOpen,NULL,(LPCTSTR)strFileName);
//
#define GADD_WARNING(className,warningCode,header)\
					XGFormatWarning##className(warningCode,header)
#define GADD_WARNING1(className,warningCode,a1,header)\
					XGFormatWarning##className(warningCode,header,a1)
#define GADD_WARNING2(className,warningCode,a1,a2,header)\
					XGFormatWarning##className(warningCode,header,a1,a2)
#define GADD_WARNING3(className,warningCode,a1,a2,a3,header)\
					XGFormatWarning##className(warningCode,header,a1,a2,a3)
#define GADD_WARNING4(className,warningCode,a1,a2,a3,a4,header)\
					XGFormatWarning##className(warningCode,header,a1,a2,a3,a4)
#define GADD_WARNING5(className,warningCode,a1,a2,a3,a4,a5,header)\
					XGFormatWarning##className(warningCode,header,a1,a2,a3,a4,a5)

// Use this macro to retrieve the error object associated with the specified object
//
// e.g :		CMyClassName	object1;
//				COtherClassName	object2;
//				...
//				GSET_ERROR(eError,GGET_LASTERROR(&object1));
//

#define GGET_WARNINGSTRINGLIST(className,object_pointer) (object_pointer)->m_clLastWarning##className.GetStringErrorList();

#define GSET_MAXWARNING(className,object_pointer,nLevel) (object_pointer)->m_clLastWarning##className.SetMessageLevel(nLevel);
#define GRESET_WARNING(className,object_pointer)	(object_pointer)->m_clLastWarning##className.Reset();



// ----------------------------------------------------------------------------------------------------------
// class CGErrorMgr
// ----------------------------------------------------------------------------------------------------------
// Usually, you never have to use directly this class. Use macros provided above instead.

class GERRORMGR_API CGErrorMgr
{
// Constructor / Destructor
public:
	CGErrorMgr();
	CGErrorMgr(const CGErrorMgr& error);
	virtual ~CGErrorMgr();

// Attributes
public:
	std::string						GetErrorMessage(int nMsgLevel = -1) const;
	void							GetErrorMessage(std::string & strErrorMsg, int nMsgLevel = -1);
	void							SetMessageLevel(int nLevel) { m_nMessageLevel = nLevel; }
	const std::vector<std::string>&	GetStringErrorList() { return m_lstErrorString; }

// Operations
public:
	void		Reset();
	CGErrorMgr&	operator =(const CGErrorMgr& error);
	CGErrorMgr&	operator +=(PCSTR lpcszErrorMessage);
	CGErrorMgr&	operator +=(const std::string& strErrorMessage);
	
// Implementation
protected:
	std::vector<std::string>	m_lstErrorString;
	int							m_nMessageLevel;
	void						CopyContent(const CGErrorMgr& error);
};

#endif // _GALAXY_ERROR_MANAGER_HEADER_
