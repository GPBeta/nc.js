
/***************************************************************
 * Name:      util.cpp
 * Purpose:   Code for Node-CEF Utility Module
 * Author:    Joshua GPBeta (studiocghibli@gmail.com)
 * Created:   2016-05-27
 * Copyright: Studio GPBeta (www.gpbeta.com)
 * License:
 **************************************************************/

/// ============================================================================
/// declarations
/// ============================================================================

/// ----------------------------------------------------------------------------
/// headers
/// ----------------------------------------------------------------------------

#include "ncjs/module.h"

#include <signal.h>

namespace ncjs {

/// ============================================================================
/// implementation
/// ============================================================================

/// ----------------------------------------------------------------------------
/// ModuleUtil
/// ----------------------------------------------------------------------------

class ModuleUtil : public JsObjecT<ModuleUtil> {

    // util.isDate()
    NCJS_OBJECT_FUNCTION(IsDate)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        NCJS_CHK_EQ(1, args.size());
        retval = CefV8Value::CreateBool(args[0]->IsDate());
    }

    // object factory

    NCJS_BEGIN_OBJECT_FACTORY()
        NCJS_MAP_OBJECT_FUNCTION("isDate", IsDate)
    NCJS_END_OBJECT_FACTORY()

};

/// ----------------------------------------------------------------------------
/// define module
/// ----------------------------------------------------------------------------

NCJS_DEFINE_BUILTIN_MODULE(util, ModuleUtil);

} // ncjs