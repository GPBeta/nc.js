
/***************************************************************
 * Name:      Environment.cpp
 * Purpose:   Codes for Node-CEF Environment Class
 * Author:    Joshua GPBeta (studiocghibli@gmail.com)
 * Created:   2016-05-26
 * Copyright: Studio GPBeta (www.gpbeta.com)
 * License:
 **************************************************************/

/// ============================================================================
/// declarations
/// ============================================================================

#define _WINSOCKAPI_    // stops windows.h including winsock.h

/// ----------------------------------------------------------------------------
/// Headers
/// ----------------------------------------------------------------------------

#include "ncjs/Environment.h"
#include "ncjs/string.h"
#include "ncjs/constants.h"
#include "ncjs/EventLoop.h"

#include <uv.h>

#include <sstream>

namespace ncjs {

/// ----------------------------------------------------------------------------
/// variables
/// ----------------------------------------------------------------------------

uv_loop_t* Environment::s_loopSync = NULL;
EventLoop Environment::s_loopAsync;

const double Environment::s_startTime = double(uv_now(uv_default_loop()));

Environment::EnvMap Environment::s_map;


static const NCJS_DEFINE_REFTEXT(s_scriptNew, "(function() {\n\
    var obj = Object.create(this.prototype);\n\
    var res = this.apply(obj, arguments);\n\
    return (typeof res === 'object' && res) || obj;\n\
});");

static const NCJS_DEFINE_REFTEXT(s_scriptError, "(function(str) {\n\
    return new Error(str);\n\
});");

static const NCJS_DEFINE_REFTEXT(s_scriptThrow, "(function(str) {\n\
    throw new Error(str);\n\
});");

/// ============================================================================
/// implementation
/// ============================================================================

void Environment::Setup(const CefString& execPath, const CefString& pagePath,
                        const CefString& frameUrl, CefRefPtr<CefV8Value> process)
{
    m_pathExec = execPath;
    m_pathPage = pagePath;
    m_urlFrame = frameUrl;

    m_object.process = process;

    // argv[0] represents application path
    // argv[1] represents page path
    CefRefPtr<CefV8Value> vExecPath = CefV8Value::CreateString(execPath);
    CefRefPtr<CefV8Value> vPagePath = CefV8Value::CreateString(pagePath);

    // process.argv
    CefRefPtr<CefV8Value> argsNode = CefV8Value::CreateArray(2);
    argsNode->SetValue(0, vExecPath);
    argsNode->SetValue(1, vPagePath);
    process->SetValue(consts::str_argv, argsNode, V8_PROPERTY_ATTRIBUTE_NONE);

    // process.execArgv
    CefRefPtr<CefV8Value> argsExec = CefV8Value::CreateArray(2);
    argsExec->SetValue(0, vExecPath);
    argsExec->SetValue(1, vPagePath);
    process->SetValue(consts::str_exec_argv, argsExec, V8_PROPERTY_ATTRIBUTE_NONE);

    // process.execPath
    process->SetValue(consts::str_exec_path, vExecPath, V8_PROPERTY_ATTRIBUTE_NONE);

    // process.moduleLoadList
    process->SetValue(consts::str_module_load_list,
        GetArray().module_load_list, V8_PROPERTY_ATTRIBUTE_READONLY);
    

    // variables not defined in Node.js
    
    // process.binding._cache
    CefRefPtr<CefV8Value> binding = process->GetValue(consts::str_binding);
    binding->SetValue(consts::str_cache, m_object.binding_cache, V8_PROPERTY_ATTRIBUTE_NONE);
}

/// ----------------------------------------------------------------------------
/// constructor & destructor
/// ----------------------------------------------------------------------------

Environment::BufferObjectInfo::BufferObjectInfo()
{
    for (int i = 0; i < FIELDS_COUNT; ++i)
        m_fields[i] = 0;
}

Environment::Environment()
{
}

Environment::~Environment()
{
}

/// ----------------------------------------------------------------------------
/// utilities functions
/// ----------------------------------------------------------------------------

static inline const char* GetErrorString(int errorno) {
#define ERRNO_CASE(_ERR)  case _ERR: return #_ERR;
    switch (errorno) {
#ifdef EACCES
    ERRNO_CASE(EACCES);
#endif

#ifdef EADDRINUSE
    ERRNO_CASE(EADDRINUSE);
#endif

#ifdef EADDRNOTAVAIL
    ERRNO_CASE(EADDRNOTAVAIL);
#endif

#ifdef EAFNOSUPPORT
    ERRNO_CASE(EAFNOSUPPORT);
#endif

#ifdef EAGAIN
    ERRNO_CASE(EAGAIN);
#endif

#ifdef EWOULDBLOCK
# if EAGAIN != EWOULDBLOCK
    ERRNO_CASE(EWOULDBLOCK);
# endif
#endif

#ifdef EALREADY
    ERRNO_CASE(EALREADY);
#endif

#ifdef EBADF
    ERRNO_CASE(EBADF);
#endif

#ifdef EBADMSG
    ERRNO_CASE(EBADMSG);
#endif

#ifdef EBUSY
    ERRNO_CASE(EBUSY);
#endif

#ifdef ECANCELED
    ERRNO_CASE(ECANCELED);
#endif

#ifdef ECHILD
    ERRNO_CASE(ECHILD);
#endif

#ifdef ECONNABORTED
    ERRNO_CASE(ECONNABORTED);
#endif

#ifdef ECONNREFUSED
    ERRNO_CASE(ECONNREFUSED);
#endif

#ifdef ECONNRESET
    ERRNO_CASE(ECONNRESET);
#endif

#ifdef EDEADLK
    ERRNO_CASE(EDEADLK);
#endif

#ifdef EDESTADDRREQ
    ERRNO_CASE(EDESTADDRREQ);
#endif

#ifdef EDOM
    ERRNO_CASE(EDOM);
#endif

#ifdef EDQUOT
    ERRNO_CASE(EDQUOT);
#endif

#ifdef EEXIST
    ERRNO_CASE(EEXIST);
#endif

#ifdef EFAULT
    ERRNO_CASE(EFAULT);
#endif

#ifdef EFBIG
    ERRNO_CASE(EFBIG);
#endif

#ifdef EHOSTUNREACH
    ERRNO_CASE(EHOSTUNREACH);
#endif

#ifdef EIDRM
    ERRNO_CASE(EIDRM);
#endif

#ifdef EILSEQ
    ERRNO_CASE(EILSEQ);
#endif

#ifdef EINPROGRESS
    ERRNO_CASE(EINPROGRESS);
#endif

#ifdef EINTR
    ERRNO_CASE(EINTR);
#endif

#ifdef EINVAL
    ERRNO_CASE(EINVAL);
#endif

#ifdef EIO
    ERRNO_CASE(EIO);
#endif

#ifdef EISCONN
    ERRNO_CASE(EISCONN);
#endif

#ifdef EISDIR
    ERRNO_CASE(EISDIR);
#endif

#ifdef ELOOP
    ERRNO_CASE(ELOOP);
#endif

#ifdef EMFILE
    ERRNO_CASE(EMFILE);
#endif

#ifdef EMLINK
    ERRNO_CASE(EMLINK);
#endif

#ifdef EMSGSIZE
    ERRNO_CASE(EMSGSIZE);
#endif

#ifdef EMULTIHOP
    ERRNO_CASE(EMULTIHOP);
#endif

#ifdef ENAMETOOLONG
    ERRNO_CASE(ENAMETOOLONG);
#endif

#ifdef ENETDOWN
    ERRNO_CASE(ENETDOWN);
#endif

#ifdef ENETRESET
    ERRNO_CASE(ENETRESET);
#endif

#ifdef ENETUNREACH
    ERRNO_CASE(ENETUNREACH);
#endif

#ifdef ENFILE
    ERRNO_CASE(ENFILE);
#endif

#ifdef ENOBUFS
    ERRNO_CASE(ENOBUFS);
#endif

#ifdef ENODATA
    ERRNO_CASE(ENODATA);
#endif

#ifdef ENODEV
    ERRNO_CASE(ENODEV);
#endif

#ifdef ENOENT
    ERRNO_CASE(ENOENT);
#endif

#ifdef ENOEXEC
    ERRNO_CASE(ENOEXEC);
#endif

#ifdef ENOLINK
    ERRNO_CASE(ENOLINK);
#endif

#ifdef ENOLCK
# if ENOLINK != ENOLCK
    ERRNO_CASE(ENOLCK);
# endif
#endif

#ifdef ENOMEM
    ERRNO_CASE(ENOMEM);
#endif

#ifdef ENOMSG
    ERRNO_CASE(ENOMSG);
#endif

#ifdef ENOPROTOOPT
    ERRNO_CASE(ENOPROTOOPT);
#endif

#ifdef ENOSPC
    ERRNO_CASE(ENOSPC);
#endif

#ifdef ENOSR
    ERRNO_CASE(ENOSR);
#endif

#ifdef ENOSTR
    ERRNO_CASE(ENOSTR);
#endif

#ifdef ENOSYS
    ERRNO_CASE(ENOSYS);
#endif

#ifdef ENOTCONN
    ERRNO_CASE(ENOTCONN);
#endif

#ifdef ENOTDIR
    ERRNO_CASE(ENOTDIR);
#endif

#ifdef ENOTEMPTY
# if ENOTEMPTY != EEXIST
    ERRNO_CASE(ENOTEMPTY);
# endif
#endif

#ifdef ENOTSOCK
    ERRNO_CASE(ENOTSOCK);
#endif

#ifdef ENOTSUP
    ERRNO_CASE(ENOTSUP);
#else
# ifdef EOPNOTSUPP
    ERRNO_CASE(EOPNOTSUPP);
# endif
#endif

#ifdef ENOTTY
    ERRNO_CASE(ENOTTY);
#endif

#ifdef ENXIO
    ERRNO_CASE(ENXIO);
#endif


#ifdef EOVERFLOW
    ERRNO_CASE(EOVERFLOW);
#endif

#ifdef EPERM
    ERRNO_CASE(EPERM);
#endif

#ifdef EPIPE
    ERRNO_CASE(EPIPE);
#endif

#ifdef EPROTO
    ERRNO_CASE(EPROTO);
#endif

#ifdef EPROTONOSUPPORT
    ERRNO_CASE(EPROTONOSUPPORT);
#endif

#ifdef EPROTOTYPE
    ERRNO_CASE(EPROTOTYPE);
#endif

#ifdef ERANGE
    ERRNO_CASE(ERANGE);
#endif

#ifdef EROFS
    ERRNO_CASE(EROFS);
#endif

#ifdef ESPIPE
    ERRNO_CASE(ESPIPE);
#endif

#ifdef ESRCH
    ERRNO_CASE(ESRCH);
#endif

#ifdef ESTALE
    ERRNO_CASE(ESTALE);
#endif

#ifdef ETIME
    ERRNO_CASE(ETIME);
#endif

#ifdef ETIMEDOUT
    ERRNO_CASE(ETIMEDOUT);
#endif

#ifdef ETXTBSY
    ERRNO_CASE(ETXTBSY);
#endif

#ifdef EXDEV
    ERRNO_CASE(EXDEV);
#endif
    }

    return "";
}

inline bool Environment::FindEnvironment(const CefRefPtr<CefV8Context>& context, EnvMap::iterator& it)
{
    // really need a map?
    for (it = s_map.begin(); it != s_map.end(); ++it) {
        if (it->first->IsSame(context))
            return true;
    }
    return false;
}

/// ----------------------------------------------------------------------------
/// static functions
/// ----------------------------------------------------------------------------

void Environment::ErrorException(int err, const char* syscall, const char* msg, CefString& except)
{
    std::ostringstream format(GetErrorString(err));

    format << ", ";

    if (msg && *msg)
        format << msg;
    else
        format << strerror(err);

    except = format.str();
}

void Environment::UvException(int err, const char* syscall, const char* msg,
                              const char* path, const char* dest, CefString& except)
{
    std::ostringstream format;

    format << uv_err_name(err) << ": ";

    if (msg && *msg)
        format << msg;
    else
        format << uv_strerror(err);

    format << ", " << syscall;

    if (path)
        format << " '" << path << '\'';

    if (dest)
        format << " -> '" << dest << '\'';

    except = format.str();
}

Environment* Environment::Get(const CefRefPtr<CefV8Context>& context)
{
    EnvMap::iterator it;
    return FindEnvironment(context, it) ? it->second : NULL;
}

Environment* Environment::Create(CefRefPtr<CefV8Context> context)
{
    Environment* env = new Environment;
    CefRefPtr<CefV8Value> global = context->GetGlobal();

    // register context and store environment object
    s_map[context] = env;

    // comile functions
    CefRefPtr<CefV8Exception> except;
    // new
    context->Eval(s_scriptNew, env->GetFunction().op_new, except);
    // new Error(str)
    context->Eval(s_scriptError, env->GetFunction().new_error, except);
    // throw
    context->Eval(s_scriptThrow, env->GetFunction().op_throw, except);

    return env;
}

void Environment::InvalidateContext(const CefRefPtr<CefV8Context>& context)
{
    EnvMap::iterator itMap;

    if (FindEnvironment(context, itMap)) {
        ListenerList list;
        list.swap(itMap->second->m_listener);

        for (ListenerList::const_iterator it = list.begin(); it != list.end(); ++it)
            (*it)->OnContextReleased(context);

        s_map.erase(itMap);
    }
}

bool Environment::Initialize()
{
    if (s_loopAsync.IsRunning())
        return true;

    // initialize synchronous uv event loop
    s_loopSync = uv_default_loop();

    NCJS_CHECK(s_loopSync);

    return s_loopAsync.Start();
}

void Environment::Shutdown()
{
    if (!s_loopAsync.IsRunning())
        return;

    s_loopAsync.Stop();
    s_loopSync = NULL;
}

} // ncjs

