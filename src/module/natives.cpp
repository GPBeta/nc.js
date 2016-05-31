
/***************************************************************
 * Name:      natives.cpp
 * Purpose:   Code for Node-CEF Natives Module
 * Author:    Joshua GPBeta (studiocghibli@gmail.com)
 * Created:   2016-05-24
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
#include "ncjs/native/nc.h"
#include "ncjs/native/modules.h"

namespace ncjs {

/// ============================================================================
/// implementation
/// ============================================================================

const CefString& GetMainSource() { return natives::s_nc; }

void DefineJavaScript(CefRefPtr<Environment> env, CefRefPtr<CefV8Value> target)
{
    using namespace natives;

    for (int i = 0; s_modules[i].name; ++i) {
        const CefString name(s_modules[i].name, s_modules[i].name_len, false);
        const CefString source(s_modules[i].source, s_modules[i].source_len, false);

        target->SetValue(name, CefV8Value::CreateString(source),
            V8_PROPERTY_ATTRIBUTE_NONE);
    }
}

/// ----------------------------------------------------------------------------
/// ModuleNatives
/// ----------------------------------------------------------------------------

class ModuleNatives : public JsObjecT<ModuleNatives> {

    // object factory

    NCJS_BEGIN_OBJECT_FACTORY()
        NCJS_MAP_OBJECT_EXTRA(DefineJavaScript)
    NCJS_END_OBJECT_FACTORY()

};

/// ----------------------------------------------------------------------------
/// define module
/// ----------------------------------------------------------------------------

NCJS_DEFINE_BUILTIN_MODULE(natives, ModuleNatives);

} // ncjs