
/***************************************************************
 * Name:      string.h
 * Purpose:   Defines Node-CEF Strings
 * Author:    Joshua GPBeta (studiocghibli@gmail.com)
 * Created:   2016-05-26
 * Copyright: Studio GPBeta (www.gpbeta.com)
 * License:
 **************************************************************/
 
#ifndef NCJS_STRING_H
#define NCJS_STRING_H

/// ----------------------------------------------------------------------------
/// Headers
/// ----------------------------------------------------------------------------

#include "ncjs/base.h"

#include <include/internal/cef_string.h>

#ifdef CEF_STRING_TYPE_UTF8

#define NCJS_TEXT(_STR) _STR

#define std_string ::std::string
#define std_sstream ::std::stringstream
#define std_isstream ::std::istringstream
#define std_osstream ::std::ostringstream

#else // UTF16 or UTF32

#define NCJS_TEXT(_STR) L##_STR

#define std_string ::std::wstring
#define std_sstream ::std::wstringstream
#define std_isstream ::std::wistringstream
#define std_osstream ::std::wostringstream

#endif // CEF_STRING_TYPE_UTF8

#define NCJS_DEFINE_REFTEXT(_NAME, _STR) \
    CefString _NAME(NCJS_TEXT(_STR), ArraySize(NCJS_TEXT(_STR)) - 1, false)

#define NCJS_REFTEXT(_STR) NCJS_DEFINE_REFTEXT(NCJS_STRING_H, _STR)

#endif // NCJS_STRING_H
