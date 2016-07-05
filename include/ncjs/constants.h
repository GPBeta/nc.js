
/***************************************************************
 * Name:      constants.h
 * Purpose:   Defines Node-CEF Constants
 * Author:    Joshua GPBeta (studiocghibli@gmail.com)
 * Created:   2016-05-25
 * Copyright: Studio GPBeta (www.gpbeta.com)
 * License:
 **************************************************************/
 
#ifndef NCJS_CONSTANTS_H
#define NCJS_CONSTANTS_H

/// ----------------------------------------------------------------------------
/// Headers
/// ----------------------------------------------------------------------------

#include "ncjs/base.h"
#include "ncjs/string.h"

#define _NCJS_CONST_VAR_CEFSTR(_NAME) extern const CefString _NAME
#define _NCJS_CONST_CEFSTR_X(_NAME, _STR) \
    _NCJS_CONST_VAR_CEFSTR(_NAME)(_STR, ArraySize(_STR) - 1, false)

#ifdef _NCJS_DEFINE_CONSTANTS

#define _NCJS_CONST_DECLARE_CEFSTR(_NAME, _STR) _NCJS_CONST_CEFSTR_X(_NAME, NCJS_TEXT(_STR))

#else

#define _NCJS_CONST_DECLARE_CEFSTR(_NAME, _STR) _NCJS_CONST_VAR_CEFSTR(_NAME)

#endif // _NCJS_DEFINE_CONSTANTS

namespace ncjs {

namespace consts {

// strings

_NCJS_CONST_VAR_CEFSTR(str_NULL);

_NCJS_CONST_DECLARE_CEFSTR(str_address, "address");
_NCJS_CONST_DECLARE_CEFSTR(str_args, "args");
_NCJS_CONST_DECLARE_CEFSTR(str_argv, "argv");
_NCJS_CONST_DECLARE_CEFSTR(str_binding, "binding");
_NCJS_CONST_DECLARE_CEFSTR(str_cache, "_cache");
_NCJS_CONST_DECLARE_CEFSTR(str_change, "change");
_NCJS_CONST_DECLARE_CEFSTR(str_dir, "dir");
_NCJS_CONST_DECLARE_CEFSTR(str_emit, "emit");
_NCJS_CONST_DECLARE_CEFSTR(str_exec_argv, "execArgv");
_NCJS_CONST_DECLARE_CEFSTR(str_exec_path, "execPath");
_NCJS_CONST_DECLARE_CEFSTR(str_family, "family");
_NCJS_CONST_DECLARE_CEFSTR(str_file, "file");
_NCJS_CONST_DECLARE_CEFSTR(str_idle, "idle");
_NCJS_CONST_DECLARE_CEFSTR(str_internal, "internal");
_NCJS_CONST_DECLARE_CEFSTR(str_IPv4, "IPv4");
_NCJS_CONST_DECLARE_CEFSTR(str_IPv6, "IPv6");
_NCJS_CONST_DECLARE_CEFSTR(str_irq, "irq");
_NCJS_CONST_DECLARE_CEFSTR(str_junction, "junction");
_NCJS_CONST_DECLARE_CEFSTR(str_length, "length");
_NCJS_CONST_DECLARE_CEFSTR(str_mac, "mac");
_NCJS_CONST_DECLARE_CEFSTR(str_model, "model");
_NCJS_CONST_DECLARE_CEFSTR(str_modules, "modules");
_NCJS_CONST_DECLARE_CEFSTR(str_module_load_list, "moduleLoadList");
_NCJS_CONST_DECLARE_CEFSTR(str_netmask, "netmask");
_NCJS_CONST_DECLARE_CEFSTR(str_nice, "nice");
_NCJS_CONST_DECLARE_CEFSTR(str_onchange, "onchange");
_NCJS_CONST_DECLARE_CEFSTR(str_oncomplete, "oncomplete");
_NCJS_CONST_DECLARE_CEFSTR(str_onstop, "onstop");
_NCJS_CONST_DECLARE_CEFSTR(str_prototype, "prototype");
_NCJS_CONST_DECLARE_CEFSTR(str_rename, "rename");
_NCJS_CONST_DECLARE_CEFSTR(str_rss, "rss");
_NCJS_CONST_DECLARE_CEFSTR(str_scopeid, "scopeid");
_NCJS_CONST_DECLARE_CEFSTR(str_speed, "speed");
_NCJS_CONST_DECLARE_CEFSTR(str_sys, "sys");
_NCJS_CONST_DECLARE_CEFSTR(str_times, "times");
_NCJS_CONST_DECLARE_CEFSTR(str_uncaught_except, "uncaughtException");
_NCJS_CONST_DECLARE_CEFSTR(str_unknown, "<unknown>");
_NCJS_CONST_DECLARE_CEFSTR(str_user, "user");
_NCJS_CONST_DECLARE_CEFSTR(str_wrap, "_wrap");

_NCJS_CONST_DECLARE_CEFSTR(str_err_notimpl, "Not Implemented.");

_NCJS_CONST_DECLARE_CEFSTR(str_ncjs_alias, NCJS_ALIAS_STRING);
_NCJS_CONST_DECLARE_CEFSTR(str_ncjs_name, NCJS_NAME_STRING);


} // consts

} // ncjs

#endif // NCJS_CONSTANTS_H
