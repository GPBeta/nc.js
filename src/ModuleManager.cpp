
/***************************************************************
 * Name:      ModuleManager.cpp
 * Purpose:   Codes for Node-CEF Module Manager Class
 * Author:    Joshua GPBeta (studiocghibli@gmail.com)
 * Created:   2016-05-24
 * Copyright: Studio GPBeta (www.gpbeta.com)
 * License:
 **************************************************************/

/// ============================================================================
/// declarations
/// ============================================================================

/// ----------------------------------------------------------------------------
/// Headers
/// ----------------------------------------------------------------------------

#include "ncjs/ModuleManager.h"
#include "ncjs/Core.h"
#include "ncjs/module.h"

namespace ncjs {

/// ----------------------------------------------------------------------------
/// variables
/// ----------------------------------------------------------------------------

module_t* ModuleManager::s_linked = NULL;
module_t* ModuleManager::s_builtin = NULL;
module_t* ModuleManager::s_pending = NULL;

/// ============================================================================
/// implementation
/// ============================================================================

static inline void ResetModuleList(module_t* root)
{
    for (module_t* module = root; module;) {
        module_t* temp = module;
        module = module->link;
        temp->link = NULL;
    }
}

static inline module_t* FindModue(module_t* root, const CefString& id)
{
    for (module_t* module = root; module; module = module->link) {
        if (id == module->modname)
            return module;
    }

    return NULL;
}

extern "C" bool ncjsRegisterModule(module_t* module)
{
    if (module)
        return ModuleManager::Register(*module);

    return false;
}

/// ----------------------------------------------------------------------------
/// static functions
/// ----------------------------------------------------------------------------

module_t*  ModuleManager::GetLinked(const CefString& id)
{
    return FindModue(s_linked, id);
}

module_t* ModuleManager::GetBuiltin(const CefString& id)
{
    return FindModue(s_builtin, id);
}

bool ModuleManager::Register(module_t& module)
{
    if (module.flags & NCJS_F_BUILTIN) {
        module.link = s_builtin;
        s_builtin = &module;
    } else if (!Core::IsInitialized()) {
        module.flags = NCJS_F_LINKED;
        module.link = s_linked;
        s_linked = &module;
    } else {
        module.link = s_pending;
        s_pending = &module;
    }

    return true;
}

void ModuleManager::Reset()
{
    // break module lists  
    ResetModuleList(s_linked);
    ResetModuleList(s_builtin);
    ResetModuleList(s_pending);
    
    s_linked = s_builtin = s_pending = NULL;
}

} // ncjs

