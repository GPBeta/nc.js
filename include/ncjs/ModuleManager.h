
/***************************************************************
 * Name:      ModuleManager.h
 * Purpose:   Defines Node-CEF Render Process Handler Class
 * Author:    Joshua GPBeta (studiocghibli@gmail.com)
 * Created:   2016-05-24
 * Copyright: Studio GPBeta (www.gpbeta.com)
 * License:
 **************************************************************/
 
#ifndef NCJS_MODULEMANAGER_H
#define NCJS_MODULEMANAGER_H

/// ----------------------------------------------------------------------------
/// Headers
/// ----------------------------------------------------------------------------

#include "ncjs/string.h"

namespace ncjs {

struct module_t;

/// ----------------------------------------------------------------------------
/// \class ModuleManager
/// ----------------------------------------------------------------------------
class ModuleManager {
public:

    /// Static Functions
    /// --------------------------------------------------------------

    static module_t*  GetLinked() { return s_linked; }
    static module_t* GetBuiltin() { return s_builtin; }
    static module_t* GetPending() { return s_pending; }

    static module_t*  GetLinked(const CefString& id);
    static module_t* GetBuiltin(const CefString& id);

    static bool Register(module_t& module);

    static void Reset();

private:

    /// Declarations
    /// -----------------

    static module_t* s_linked;
    static module_t* s_builtin;
    static module_t* s_pending;
};


} // ncjs

#endif // NCJS_MODULEMANAGER_H
