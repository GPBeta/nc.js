
/***************************************************************
 * Name:      Process.h
 * Purpose:   Defines Node-CEF Process Class
 * Author:    Joshua GPBeta (studiocghibli@gmail.com)
 * Created:   2016-05-24
 * Copyright: Studio GPBeta (www.gpbeta.com)
 * License:
 **************************************************************/
 
#ifndef NCJS_PROCESS_H
#define NCJS_PROCESS_H


template <class T>
class CefRefPtr;
class CefV8Value;
class CefBrowser;
class CefFrame;
class CefV8Context;

/// ----------------------------------------------------------------------------
/// Headers
/// ----------------------------------------------------------------------------

namespace ncjs {

class Environment;

/// ----------------------------------------------------------------------------
/// \class Process
/// ----------------------------------------------------------------------------
class Process {
public:

    /// Static Functions
    /// --------------------------------------------------------------

    static CefRefPtr<CefV8Value> CreateObject(CefRefPtr<Environment> enviroment,
                                              CefRefPtr<CefV8Context> context);

};


} // ncjs

#endif // NCJS_PROCESS_H
