
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

} // ncjs

