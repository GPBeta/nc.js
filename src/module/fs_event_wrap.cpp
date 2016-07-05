
/***************************************************************
 * Name:      fs_event_wrap.cpp
 * Purpose:   Code for Node-CEF FS Event Wrap Module
 * Author:    Joshua GPBeta (studiocghibli@gmail.com)
 * Created:   2016-07-04
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

namespace ncjs {

/// ----------------------------------------------------------------------------
/// variables
/// ----------------------------------------------------------------------------

/// ============================================================================
/// implementation
/// ============================================================================

/// ----------------------------------------------------------------------------
/// FSEvent
/// ----------------------------------------------------------------------------

class FSEvent : public JsObjecT<FSEvent> {

    class Handle : public HandleWrap<Handle, uv_fs_event_t, USER_DATA::FS_EVENT_WRAP> {
    public:

        Handle(const CefRefPtr<CefV8Value>& handle, const CefString& path, unsigned flags) :
            HandleWrap(handle), m_path(path), m_flags(flags) {}

    private:

        void OnChange(const CefString& filename, int events, int status)
        {
            if (!IsActive())
                return;

            if (Environment* env = EnterEnvironment()) {
                const CefString* evt = &consts::str_NULL;
                if (status == 0) {
                    if (events & UV_RENAME)
                        evt = &consts::str_rename;
                    else if (events & UV_CHANGE)
                        evt = &consts::str_change;
                    else
                        NCJS_UNREACHABLE();
                }

                CefV8ValueList args;
                args.push_back(CefV8Value::CreateInt(status));
                args.push_back(CefV8Value::CreateString(*evt));
                if (filename.length())
                    args.push_back(CefV8Value::CreateString(filename));
                
                const CefRefPtr<CefV8Value>& handle = GetHandle();
                const CefRefPtr<CefV8Value> callback = handle->GetValue(consts::str_onchange);
                callback->ExecuteFunction(handle, args);

                ExitEnvironment(env);
            }
        }

        virtual bool AsyncStart(uv_loop_t* loop) OVERRIDE
        {
            int err = uv_fs_event_init(loop, this);
            if (err >= 0) {
                err = uv_fs_event_start(this, Handle::Callback, m_path.c_str(), m_flags);
                if (err >= 0)
                    return true;
            }
            // failed
            Callback(this, m_path.c_str(), 0, err);
            return false;
        }

        static void Callback(uv_fs_event_t* handle,
                             const char* filename, int events, int status)
        {
            NCJS_ASSERT(handle);
            
            const CefString path(filename);
            Handle* wrap = static_cast<Handle*>(handle);
            CefPostTask(TID_RENDERER,
                        base::Bind(&Handle::OnChange, wrap, path, events, status));
        }

        /// Declarations
        /// -----------------

        const std::string m_path;
        const unsigned m_flags;
    };

    // FSEvent.start()
    NCJS_OBJECT_FUNCTION(Start)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        NCJS_CHK_EQ(args.size(), 3);

        if (!args[0]->IsString())
            return Environment::TypeException(NCJS_TEXT("filename must be a valid string"), except);

        // we cannot return an error code immediately like Node.js does,
        // so if there are errors at startup stage, it will be reported in onchange(err) later
        retval = CefV8Value::CreateInt(0);

        CefRefPtr<CefV8Value> objWrap = object->GetValue(consts::str_wrap);

        if (Handle::Unwrap(objWrap))
            return; // already started

        const CefString path = args[0]->GetStringValue();
        // const bool persistent = args[1]->GetBoolValue(); // always persistent
        const unsigned flags = args[2]->GetBoolValue() ? UV_FS_EVENT_RECURSIVE : 0;

        CefRefPtr<Handle> handle(new Handle(object, path, flags));
        handle->Wrap(objWrap);
        handle->OnStart();
    }

   // FSEvent.close()
    NCJS_OBJECT_FUNCTION(Close)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        CefRefPtr<CefV8Value> objWrap = object->GetValue(consts::str_wrap);
        CefRefPtr<Handle> handle = Handle::Unwrap(objWrap);

        if (!handle)
            return; // already stopped

        objWrap->SetUserData(NULL);

        handle->OnStop();
    }

    // FSEvent()
    NCJS_OBJECT_FUNCTION(Constructor)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        NCJS_PROPERTY_CONST(Object, object, consts::str_wrap, NULL);
    }

    // class factory

    NCJS_BEGIN_CLASS_FACTORY(Constructor)
        NCJS_MAP_OBJECT_FUNCTION("start", Start)
        NCJS_MAP_OBJECT_FUNCTION("close", Close)
    NCJS_END_CLASS_FACTORY()
};

/// ----------------------------------------------------------------------------
/// ModuleEventWrap
/// ----------------------------------------------------------------------------

class ModuleEventWrap : public JsObjecT<ModuleEventWrap> {

    // object factory

    NCJS_BEGIN_OBJECT_FACTORY()
        // objects
        NCJS_MAP_OBJECT_FACTORY("FSEvent", FSEvent)
    NCJS_END_OBJECT_FACTORY()

};

/// ----------------------------------------------------------------------------
/// define module
/// ----------------------------------------------------------------------------

NCJS_DEFINE_BUILTIN_MODULE(fs_event_wrap, ModuleEventWrap);

} // ncjs