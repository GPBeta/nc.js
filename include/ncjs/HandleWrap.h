
/***************************************************************
 * Name:      HandleWrap.h
 * Purpose:   Defines Node-CEF Handle Wrap Class
 * Author:    Joshua GPBeta (studiocghibli@gmail.com)
 * Created:   2016-07-05
 * Copyright: Studio GPBeta (www.gpbeta.com)
 * License:
 **************************************************************/
 
#ifndef NCJS_HANDLEWRAP_H
#define NCJS_HANDLEWRAP_H

/// ----------------------------------------------------------------------------
/// Headers
/// ----------------------------------------------------------------------------

#include "ncjs/UserData.h"
#include "ncjs/EventLoop.h"
#include "ncjs/Environment.h"

#include <include/base/cef_bind.h>

#include <uv.h>

namespace ncjs {

/// ----------------------------------------------------------------------------
/// \class HandleWrap
/// ----------------------------------------------------------------------------

template <class T, class B, USER_DATA::TYPE TID>
class HandleWrap : public UserData<T, TID>, public Environment::Listener, public B {
public:

    void OnStart()
    {
        if (m_active)
            return;

        m_active = true;

        // add context released listener
        Environment::Get(m_context)->AddListener(this);

        m_loop.Queue(base::Bind(&HandleWrap::AsyncInit, this));
    }

    void OnStop()
    {
        if (!m_active)
            return;

        m_active = false;

        // remove context released listener
        Environment::Get(m_context)->RemoveListener(this);

        m_loop.Queue(base::Bind(&HandleWrap::AsyncDestroy, this));
    }

    virtual void OnContextReleased(CefRefPtr<CefV8Context> context) OVERRIDE
    {
        if (m_active) {
            m_active = false;
            m_loop.Queue(base::Bind(&HandleWrap::AsyncDestroy, this));
        }
    }

protected:

    /// Asynchronous Functions
    /// --------------------------------------------------------------

    // called from the uv loop thread,
    // return true if the handle is intialized and started correctly.
    virtual bool AsyncStart(uv_loop_t* loop) = 0;

    /// Synchronous Functions
    /// --------------------------------------------------------------

    bool IsActive() const { return m_active; }

    Environment* EnterEnvironment() const
    {
        Environment* env = Environment::Get(m_context);

        if (env)
            m_context->Enter();

        return env;
    }

    bool ExitEnvironment(Environment* env) const
    {
        NCJS_ASSERT(env);

        return m_context->Exit();
    }

    const CefRefPtr<CefV8Value>& GetHandle() const { return m_handle; }

    bool EnterContext() const { return m_context->Enter(); }
    bool  ExitContext() const { return m_context->Exit(); }

    HandleWrap(const CefRefPtr<CefV8Value>& handle) :
        m_active(false),
        m_loop(Environment::GetAsyncLoop()),
        m_context(CefV8Context::GetCurrentContext()),
        m_handle(handle) { data = NULL; }

private:

    void AsyncInit()
    {
        if (AsyncStart(m_loop.ToUv())) {
            // keep alive before Delete() called 
            data = this; AddRef();
        }
    }

    void AsyncDestroy()
    {
        if (data) {
            B* base = static_cast<B*>(this);
            uv_close(reinterpret_cast<uv_handle_t*>(base), HandleWrap::Delete);
        }
    }

    static void Delete(uv_handle_t* handle)
    {
        NCJS_ASSERT(handle);

        B* base = reinterpret_cast<B*>(handle);
        HandleWrap* wrap = static_cast<HandleWrap*>(base);
        wrap->data = NULL;
        wrap->Release(); // should be deleted here
    }

    /// Declarations
    /// -----------------

    bool m_active;

    EventLoop& m_loop; // should be safe when accessing from uv loop thread
    CefRefPtr<CefV8Context> m_context;
    CefRefPtr<CefV8Value> m_handle;

    DISALLOW_COPY_AND_ASSIGN(HandleWrap);
    IMPLEMENT_REFCOUNTING(HandleWrap);
};

} // ncjs

#endif // NCJS_HANDLEWRAP_H
