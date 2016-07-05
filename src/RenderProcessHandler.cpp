
/***************************************************************
 * Name:      RenderProcessHandler.cpp
 * Purpose:   Codes for Node-CEF Render Process Handler Class
 * Author:    Joshua GPBeta (studiocghibli@gmail.com)
 * Created:   2016-05-22
 * Copyright: Studio GPBeta (www.gpbeta.com)
 * License:
 **************************************************************/

/// ============================================================================
/// declarations
/// ============================================================================

/// ----------------------------------------------------------------------------
/// Headers
/// ----------------------------------------------------------------------------

#include "ncjs/RenderProcessHandler.h"
#include "ncjs/Core.h"
#include "ncjs/Environment.h"
#include "ncjs/constants.h"

#include <include/cef_command_line.h>

namespace ncjs {

/// ----------------------------------------------------------------------------
/// variables
/// ----------------------------------------------------------------------------

/// ============================================================================
/// implementation
/// ============================================================================

void RenderProcessHandler::OnRenderThreadCreated(CefRefPtr<CefListValue> extra_info)
{
    CefRefPtr<CefCommandLine> args = CefCommandLine::CreateCommandLine();
    
    // retrieve application path
    args->SetProgram(CefCommandLine::GetGlobalCommandLine()->GetProgram());

    OnNodeCefCreated(*args.get());

    m_core.Initialize(args);
}

void RenderProcessHandler::OnWebKitInitialized()
{
    Core::RegisterExtension();
}

void RenderProcessHandler::OnContextReleased(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefV8Context> context)
{
    // may also destroy environment object
    Environment::InvalidateContext(context);
}

void RenderProcessHandler::OnUncaughtException(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefV8Context> context,
    CefRefPtr<CefV8Exception> exception,
    CefRefPtr<CefV8StackTrace> stackTrace)
{
    // works only if CefSettings::uncaught_exception_stack_size > 0

    if (Environment* env = Environment::Get(context)) {
        CefRefPtr<CefV8Value> process = env->GetObject().process;
        CefRefPtr<CefV8Value> emit = process->GetValue(consts::str_emit);

        if (emit.get()) {
            CefV8ValueList args;
            args.push_back(CefV8Value::CreateString(consts::str_uncaught_except));
            args.push_back(CefV8Value::CreateString(exception->GetMessage()));
            emit->ExecuteFunction(process, args);
        }
    }
}

} // ncjs

