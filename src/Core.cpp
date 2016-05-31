
/***************************************************************
 * Name:      Core.cpp
 * Purpose:   Codes for Node-CEF Core Class
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

#include "ncjs/Core.h"
#include "ncjs/Process.h"
#include "ncjs/ModuleManager.h"
#include "ncjs/module.h"
#include "ncjs/constants.h"

#include <include/cef_command_line.h>
#include <include/cef_parser.h>

#include <sstream>

namespace ncjs {

/// ----------------------------------------------------------------------------
/// forward declaration
/// ----------------------------------------------------------------------------

NCJS_DECLARE_BUILTIN_MODULE(constants);
NCJS_DECLARE_BUILTIN_MODULE(natives);
NCJS_DECLARE_BUILTIN_MODULE(buffer);
NCJS_DECLARE_BUILTIN_MODULE(util);
NCJS_DECLARE_BUILTIN_MODULE(fs);
NCJS_DECLARE_BUILTIN_MODULE(os);
NCJS_DECLARE_BUILTIN_MODULE(uv);

// defined in natives.cpp
const CefString& GetMainSource();

/// ----------------------------------------------------------------------------
/// variables
/// ----------------------------------------------------------------------------

Core* Core::s_instance = NULL;

uv_loop_t* s_loop = NULL;

static CefRefPtr<CefCommandLine> s_argsNode;
static CefRefPtr<CefCommandLine> s_argsExec;

static CefString s_extension(L"\'use strict\'\n\
Object.defineProperty(this, \"ncjs\", {\n\
    get: function() {\n\
        native function init();\n\
        delete this.ncjs;\n\
        return this.ncjs = init();\n\
    },\n\
    configurable : true\n\
});");

static inline bool Url2Path(const CefString& url, CefString& filePath)
{
    CefURLParts parts;

    if (!CefParseURL(url, parts))
        return false;

    if (consts::str_file != parts.scheme.str)
        return false;
    
    cef_char_t* path = parts.path.str;
    size_t len = parts.path.length;
    
    // windows : /C:/A/B.html
    if (len > 3 && path[2] == NCJS_TEXT(':')) {
        // convert '/' to '\\'
        for (size_t i = 0; i < len; ++i) {
            if (path[i] == NCJS_TEXT('/'))
                path[i] = NCJS_TEXT('\\');
        }
        // remove first slash
        path += 1;
    } // else /root/A/B.html

    filePath = path;

    return true;
}

static class ExtensionHandler : public CefV8Handler {
public:
    virtual bool Execute(const CefString& name, CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except) OVERRIDE;

    NCJS_IMPL_NO_REFCOUNTING(ExtensionHandler);
} s_handler;

/// ============================================================================
/// implementation
/// ============================================================================

bool ExtensionHandler::Execute(const CefString& name, CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
{
    if (!Core::IsInitialized()) {
        except = NCJS_TEXT("Node-CEF not initialized.");
        return true;
    }

    CefRefPtr<CefV8Context> context = CefV8Context::GetCurrentContext();
    CefRefPtr<CefFrame> frame = context->GetFrame();
    CefRefPtr<CefV8Exception> exception;
    
    // Check URL, also forbid remote invoking currently.
    // TODO: Add remote modules support.
    CefString frameUrl = frame->GetURL();
    CefString execPath = s_argsNode->GetProgram();
    CefString pagePath;
    if (!Url2Path(frameUrl, pagePath)) {
        except = NCJS_TEXT("Invalid request source.");
        return true;
    }

    CefRefPtr<CefV8Value> init;
    if (!context->Eval(GetMainSource(), init, exception)) {
        except = NCJS_TEXT("Failed to evaluate nc.js.");
        return true;
    }

    // Create node environment

    CefRefPtr<Environment> env = Environment::Create(context);
    CefRefPtr<CefV8Value> process = Process::CreateObject(env, context);
    env->Setup(execPath, pagePath, frameUrl, process);
    
    CefV8ValueList arg;
    arg.push_back(process);
    retval = init->ExecuteFunction(object, arg);

    if (!(retval.get() && retval->IsObject())) {
        except = NCJS_TEXT("Failed to create nc.js object.");
        return true;
    }

    return true;
}

/// ----------------------------------------------------------------------------
/// constructor & destructor
/// ----------------------------------------------------------------------------

Core::Core() {}

Core::~Core()
{
    if (this == s_instance) {
        ModuleManager::Reset();
        Environment::Shutdown();

        s_instance = NULL;
    }
}

/// ----------------------------------------------------------------------------
/// static functions
/// ----------------------------------------------------------------------------

bool Core::RegisterExtension()
{
    if (s_instance == NULL)
        return false;

    return CefRegisterExtension(L"v8/Node-CEF", s_extension, &s_handler);
}

bool Core::Initialize(const CefRefPtr<CefCommandLine>& cmd)
{
    if (s_instance)
        return true;

    if (!cmd.get())
        return false;

    if (!Environment::Initialize())
        return false;

    // store command line arguments
    // TODO: add command line options support
    s_argsNode = cmd->Copy();
    s_argsExec = cmd->Copy();

    // To get rid of the buggy self registration when modules are linked as
    // static libraries with MSVC linkers, we have to register all built in
    // and native modules manually.
    ModuleManager::Register(NCJS_BUILTIN_MODULE(constants));
    ModuleManager::Register(NCJS_BUILTIN_MODULE(natives));
    ModuleManager::Register(NCJS_BUILTIN_MODULE(buffer));
    ModuleManager::Register(NCJS_BUILTIN_MODULE(util));
    ModuleManager::Register(NCJS_BUILTIN_MODULE(fs));
    ModuleManager::Register(NCJS_BUILTIN_MODULE(os));
    ModuleManager::Register(NCJS_BUILTIN_MODULE(uv));

    s_instance = this;

    return true;
}

} // ncjs

