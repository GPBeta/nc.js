
/***************************************************************
 * Name:      Core.h
 * Purpose:   Defines Node-CEF Core Class
 * Author:    Joshua GPBeta (studiocghibli@gmail.com)
 * Created:   2016-05-24
 * Copyright: Studio GPBeta (www.gpbeta.com)
 * License:
 **************************************************************/
 
#ifndef NCJS_CORE_H
#define NCJS_CORE_H

class CefV8Handler;
class CefCommandLine;

template <class T>
class CefRefPtr;

/// ----------------------------------------------------------------------------
/// Headers
/// ----------------------------------------------------------------------------

namespace ncjs {

/// ----------------------------------------------------------------------------
/// \class Core
/// ----------------------------------------------------------------------------
class Core {

    friend class RenderProcessHandler;

public:

    /// Static Functions
    /// --------------------------------------------------------------

    static bool IsInitialized() { return s_instance != 0; }

private:

    static bool RegisterExtension();

    bool Initialize(const CefRefPtr<CefCommandLine>& cmd);

    /// Constructors & Destructor
    /// --------------------------------------------------------------
    
    Core();
    ~Core();

    /// Declarations
    /// -----------------

    static Core* s_instance;
};


} // ncjs

#endif // NCJS_CORE_H
