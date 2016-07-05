
/***************************************************************
 * Name:      fs.h
 * Purpose:   Defines Node-CEF File System Classes
 * Author:    Joshua GPBeta (studiocghibli@gmail.com)
 * Created:   2016-07-02
 * Copyright: Studio GPBeta (www.gpbeta.com)
 * License:
 **************************************************************/
 
#ifndef NCJS_MODULE_FS_H
#define NCJS_MODULE_FS_H

template <class T> class CefRefPtr;
class CefV8Value;
class CefV8Context;

/// ----------------------------------------------------------------------------
/// Headers
/// ----------------------------------------------------------------------------

namespace ncjs {

class Environment;

typedef void UvState;

CefRefPtr<CefV8Value> BuildStatsObject(Environment& env, const UvState* stat);

} // ncjs

#endif // NCJS_MODULE_FS_H
