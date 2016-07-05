
/***************************************************************
 * Name:      fs_stat_watcher.cpp
 * Purpose:   Code for Node-CEF FS StatWatcher Module
 * Author:    Joshua GPBeta (studiocghibli@gmail.com)
 * Created:   2016-07-02
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
#include "ncjs/HandleWrap.h"
#include "ncjs/module/fs.h"

#include <include/wrapper/cef_closure_task.h>
#include <uv.h>

namespace ncjs {

/// ----------------------------------------------------------------------------
/// variables
/// ----------------------------------------------------------------------------

/// ============================================================================
/// implementation
/// ============================================================================

/// ----------------------------------------------------------------------------
/// StatWatcher
/// ----------------------------------------------------------------------------

class StatWatcher : public JsObjecT<StatWatcher> {

    class Handle : public HandleWrap<Handle, uv_fs_poll_t, USER_DATA::STAT_WATCHER_WRAP> {
    public:

        Handle(const CefRefPtr<CefV8Value>& handle, const CefString& path, unsigned interval) :
            HandleWrap(handle), m_path(path.ToString()), m_interval(interval) {}

    private:

        void OnChange(int status, const uv_stat_t& prev, const uv_stat_t& curr)
        {
            if (!IsActive())
                return;

            if (Environment* env = EnterEnvironment()) {
                CefV8ValueList args;
                args.push_back(BuildStatsObject(*env, &curr));
                args.push_back(BuildStatsObject(*env, &prev));
                args.push_back(CefV8Value::CreateInt(status));
                const CefRefPtr<CefV8Value>& handle = GetHandle();
                const CefRefPtr<CefV8Value> callback = handle->GetValue(consts::str_onchange);
                callback->ExecuteFunction(handle, args);

                ExitEnvironment(env);
            }
        }

        virtual bool AsyncStart(uv_loop_t* loop) OVERRIDE
        {
            int err = uv_fs_poll_init(loop, this);
            if (err >= 0) {
                err = uv_fs_poll_start(this, Handle::Callback, m_path.c_str(), m_interval);
                return err >= 0;
            }

            return false;
        }

        static void Callback(uv_fs_poll_t* handle, int status,
                             const uv_stat_t* prev, const uv_stat_t* curr)
        {
            NCJS_ASSERT(handle);

            Handle* wrap = static_cast<Handle*>(handle);
            CefPostTask(TID_RENDERER,
                        base::Bind(&Handle::OnChange, wrap, status, *prev, *curr));
        }

        /// Declarations
        /// -----------------

        const std::string m_path;
        const unsigned m_interval;
    };

    // StatWatcher.start()
    NCJS_OBJECT_FUNCTION(Start)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        NCJS_CHK_EQ(args.size(), 3);

        CefRefPtr<CefV8Value> objWrap = object->GetValue(consts::str_wrap);

        if (Handle::Unwrap(objWrap))
            return; // already started

        EventLoop& loop = Environment::GetAsyncLoop();
        CefRefPtr<CefV8Context> context = CefV8Context::GetCurrentContext();
        const CefString path = args[0]->GetStringValue();
        // const bool persistent = args[1]->GetBoolValue(); // always persistent
        const unsigned interval = args[2]->GetUIntValue();

        CefRefPtr<Handle> handle(new Handle(object, path, interval));
        handle->Wrap(objWrap);
        handle->OnStart();
    }

   // StatWatcher.stop()
    NCJS_OBJECT_FUNCTION(Stop)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        CefRefPtr<CefV8Value> objWrap = object->GetValue(consts::str_wrap);
        CefRefPtr<Handle> handle = Handle::Unwrap(objWrap);

        if (!handle)
            return; // already stopped

        CefRefPtr<CefV8Value> callback = object->GetValue(consts::str_onstop);
        callback->ExecuteFunction(object, args);

        objWrap->SetUserData(NULL);
        handle->OnStop();
    }

    // StatWatcher()
    NCJS_OBJECT_FUNCTION(Constructor)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        NCJS_PROPERTY_CONST(Object, object, consts::str_wrap, NULL);
    }

    // class factory

    NCJS_BEGIN_CLASS_FACTORY(Constructor)
        NCJS_MAP_OBJECT_FUNCTION("start", Start)
        NCJS_MAP_OBJECT_FUNCTION("stop", Stop)
    NCJS_END_CLASS_FACTORY()
};

/// ----------------------------------------------------------------------------
/// define module
/// ----------------------------------------------------------------------------

NCJS_DEFINE_BUILTIN_MODULE(fs_stat_watcher, StatWatcher);

} // ncjs