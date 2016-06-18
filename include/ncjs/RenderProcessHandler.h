
/***************************************************************
 * Name:      RenderProcessHandler.h
 * Purpose:   Defines Node-CEF Render Process Handler Class
 * Author:    Joshua GPBeta (studiocghibli@gmail.com)
 * Created:   2016-05-22
 * Copyright: Studio GPBeta (www.gpbeta.com)
 * License:
 **************************************************************/
 
#ifndef NCJS_RENDERPROCESSHANDLER_H
#define NCJS_RENDERPROCESSHANDLER_H

class CefCommandLine;
/// ----------------------------------------------------------------------------
/// Headers
/// ----------------------------------------------------------------------------

#include "ncjs/Core.h"

#include <include/cef_render_process_handler.h>

namespace ncjs {

/// ----------------------------------------------------------------------------
/// \class RenderProcessHandler
/// ----------------------------------------------------------------------------
class RenderProcessHandler : public CefRenderProcessHandler {
public:

    /// Callback Functions
    /// --------------------------------------------------------------

    virtual void OnNodeCefCreated(CefCommandLine& args) {}

    /// Overriden Functions
    /// --------------------------------------------------------------

    // NOTE: Please call these functions in your implementaions
    // if you have overriden these methods.

    virtual void OnRenderThreadCreated(
        CefRefPtr<CefListValue> extra_info) OVERRIDE;

    virtual void OnWebKitInitialized() OVERRIDE;

    virtual void OnContextCreated(CefRefPtr<CefBrowser> browser,
                                  CefRefPtr<CefFrame> frame,
                                  CefRefPtr<CefV8Context> context) OVERRIDE {}

    virtual void OnContextReleased(CefRefPtr<CefBrowser> browser,
                                   CefRefPtr<CefFrame> frame,
                                   CefRefPtr<CefV8Context> context) OVERRIDE {}

    virtual void OnUncaughtException(CefRefPtr<CefBrowser> browser,
                                     CefRefPtr<CefFrame> frame,
                                     CefRefPtr<CefV8Context> context,
                                     CefRefPtr<CefV8Exception> exception,
                                     CefRefPtr<CefV8StackTrace> stackTrace) OVERRIDE;
private:

    Core m_core;
};


} // ncjs

#endif // NCJS_RENDERPROCESSHANDLER_H
