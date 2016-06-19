
/***************************************************************
 * Name:      uv.cpp
 * Purpose:   Code for Node-CEF UV Module
 * Author:    Joshua GPBeta (studiocghibli@gmail.com)
 * Created:   2016-05-27
 * Copyright: Studio GPBeta (www.gpbeta.com)
 * License:
 **************************************************************/

/// ============================================================================
/// declarations
/// ============================================================================

#define _WINSOCKAPI_    // stops windows.h including winsock.h

/// ----------------------------------------------------------------------------
/// headers
/// ----------------------------------------------------------------------------

#include "ncjs/module.h"
#include "ncjs/constants.h"

#include <uv.h>

namespace ncjs {

/// ============================================================================
/// implementation
/// ============================================================================

void DefineUvConstants(CefRefPtr<Environment> env, CefRefPtr<CefV8Value> target)
{
#define V(_NAME, _) NCJS_PROPERTY_CONST_AUTO(Int, target, UV_##_NAME);
    UV_ERRNO_MAP(V)
#undef V
}

/// ----------------------------------------------------------------------------
/// ModuleUV
/// ----------------------------------------------------------------------------

class ModuleUV : public JsObjecT<ModuleUV> {

    // uv.errname()
    NCJS_OBJECT_FUNCTION(ErrName)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        int err = args[0]->GetIntValue();
        if (err >= 0) {
            except = NCJS_TEXT("err >= 0");
            return;
        }
        retval = CefV8Value::CreateString(uv_err_name(err));
    }

    // object factory

    NCJS_BEGIN_OBJECT_FACTORY()
        NCJS_MAP_OBJECT_FUNCTION("errname", ErrName)

        NCJS_MAP_OBJECT_EXTRA(DefineUvConstants)
    NCJS_END_OBJECT_FACTORY()

};

/// ----------------------------------------------------------------------------
/// define module
/// ----------------------------------------------------------------------------

NCJS_DEFINE_BUILTIN_MODULE(uv, ModuleUV);

} // ncjs