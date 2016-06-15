
/***************************************************************
 * Name:      Process.cpp
 * Purpose:   Codes for Node-CEF Process Class
 * Author:    Joshua GPBeta (studiocghibli@gmail.com)
 * Created:   2016-05-25
 * Copyright: Studio GPBeta (www.gpbeta.com)
 * License:
 **************************************************************/

/// ============================================================================
/// declarations
/// ============================================================================

#define _WINSOCKAPI_    // stops windows.h including winsock.h

#define NANOS_PER_SEC 1000000000

#define UV_ERROR(_ERR, _CALL) Environment::UvException(_ERR, _CALL, NULL, except)

#define TYPE_ERROR(_MSG) Environment::TypeException(NCJS_TEXT(_MSG), except)

/// ----------------------------------------------------------------------------
/// Headers
/// ----------------------------------------------------------------------------

#include "ncjs/Process.h"
#include "ncjs/Core.h"
#include "ncjs/Environment.h"
#include "ncjs/ModuleManager.h"
#include "ncjs/module.h"
#include "ncjs/constants.h"

#include <uv.h>
#include <include/cef_v8.h>

#include <sstream>

#if defined(_MSC_VER)
#include <direct.h>
#include <io.h>
#define strcasecmp _stricmp
#define getpid GetCurrentProcessId
#define umask _umask
typedef int mode_t;
#else
#include <sys/resource.h>  // getrlimit, setrlimit
#include <unistd.h>  // setuid, getuid
#endif

namespace ncjs {

/// ----------------------------------------------------------------------------
/// variables
/// ----------------------------------------------------------------------------

/// ============================================================================
/// implementation
/// ============================================================================

static inline std_string GetLibCefVersion()
{
    std_osstream str;
    str << cef_version_info(0) << L".<branch>." << cef_version_info(1);

    return str.str();
}

static inline std_string GetChromiumVersion()
{
    std_osstream str;
    str << cef_version_info(2) << L'.' << cef_version_info(3) << L'.'
        << cef_version_info(4) << L'.' << cef_version_info(5);

    return str.str();
}

/// ----------------------------------------------------------------------------
/// accessors
/// ----------------------------------------------------------------------------

class EnvAccessor : public JsAccessorT<EnvAccessor> {
    NCJS_ACCESSOR_GETTER(Get)(const CefRefPtr<CefV8Value> object,
                                    CefRefPtr<CefV8Value>& retval, CefString& except)
    {
    }

    NCJS_ACCESSOR_SETTER(Set)(const CefRefPtr<CefV8Value> object,
                              const CefRefPtr<CefV8Value> value, CefString& except)
    {
    }
};

/// ----------------------------------------------------------------------------
/// objects
/// ----------------------------------------------------------------------------

class VersionsObject : public JsObjecT<VersionsObject> {

    // object factory

    NCJS_BEGIN_OBJECT_FACTORY()
        NCJS_MAP_OBJECT_READONLY(String, consts::str_ncjs_alias, NCJS_VERSION_STRING)
        NCJS_MAP_OBJECT_READONLY(String, consts::str_cef,        NCJS_REFTEXT(CEF_VERSION))
        NCJS_MAP_OBJECT_READONLY(String, consts::str_libcef,     GetLibCefVersion())
        NCJS_MAP_OBJECT_READONLY(String, consts::str_chromium,   GetChromiumVersion())
        NCJS_MAP_OBJECT_READONLY(String, consts::str_node,       NODE_VERSION_STRING)
        NCJS_MAP_OBJECT_READONLY(String, consts::str_uv,         uv_version_string())
        NCJS_MAP_OBJECT_READONLY(String, consts::str_modules,    NCJS_MAKESTR(NODE_MODULE_VERSION))
        //NCJS_MAP_OBJECT_READONLY(String, NCJS_REFTEXT("http_parser"), NCJS_REFTEXT(HTTP_PARSER_VERSION))
        //NCJS_MAP_OBJECT_READONLY(String, NCJS_REFTEXT("v8"), GET_V8_VERSION)
        //NCJS_MAP_OBJECT_READONLY(String, NCJS_REFTEXT("zlib"), NCJS_REFTEXT(ZLIB_VERSION))
        //NCJS_MAP_OBJECT_READONLY(String, NCJS_REFTEXT("ares"), NCJS_REFTEXT(ARES_VERSION_STR))
#if defined(NODE_HAVE_I18N_SUPPORT) && defined(U_ICU_VERSION)
        NCJS_MAP_OBJECT_READONLY(String, consts::str_icu, NCJS_REFTEXT(U_ICU_VERSION))
#endif // defined(NODE_HAVE_I18N_SUPPORT) && defined(U_ICU_VERSION)
#if HAVE_OPENSSL
        NCJS_MAP_OBJECT_READONLY(String, consts::str_openssl, GET_OPENSSL_VERSION)
#endif // HAVE_OPENSSL
    NCJS_END_OBJECT_FACTORY()
};

class ReleaseObject : public JsObjecT<ReleaseObject> {

    // object factory

    NCJS_BEGIN_OBJECT_FACTORY()
        NCJS_MAP_OBJECT_READONLY(String, consts::str_name, NCJS_REFTEXT(NCJS_NAME_STRING))
        //NCJS_MAP_OBJECT_READONLY(String, NCJS_REFTEXT("sourceUrl"), GET_SOURCE_URL)
        //NCJS_MAP_OBJECT_READONLY(String, NCJS_REFTEXT("headersUrl"), GET_HEADERS_URL)
        //NCJS_MAP_OBJECT_READONLY(String, NCJS_REFTEXT("libUrl"), GET_LIB_URL)
#if NCJS_VERSION_IS_LTS
        NCJS_MAP_OBJECT_READONLY(String, consts::str_lts, NCJS_REFTEXT(NCJS_VERSION_LTS_CODENAME))
#endif // NCJS_VERSION_IS_LTS
    NCJS_END_OBJECT_FACTORY()
};

class FeaturesObject : public JsObjecT<FeaturesObject> {

    // object factory

    NCJS_BEGIN_OBJECT_FACTORY()
    NCJS_END_OBJECT_FACTORY()
};

class ProcessObject : public JsObjecT<ProcessObject> {

    // process.binding()
    NCJS_OBJECT_FUNCTION(Binding)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        CefRefPtr<CefV8Context> context = CefV8Context::GetCurrentContext();
        CefRefPtr<Environment> env = Environment::Get(context);
        CefRefPtr<CefV8Value> cache = env->GetObject().binding;

        const CefString module = args[0]->GetStringValue();
        if (cache->HasValue(module)) {
            retval = cache->GetValue(module);
            return;
        }

        // Append a string to process.moduleLoadList
        std_string buf(NCJS_TEXT("Binding ")); buf += module;

        CefRefPtr<CefV8Value> modules = env->GetArray().module_load_list;
        modules->SetValue(modules->GetArrayLength(), CefV8Value::CreateString(buf));

        CefRefPtr<CefV8Value> exports;

        if (module_t* mod = ModuleManager::GetBuiltin(module)) {
            // Internal bindings don't have a "module" object, only exports.
            NCJS_CHK_EQ(mod->factory, NULL);
            NCJS_CHK_NE(mod->ctxfactory, NULL);
            exports = mod->ctxfactory(env, context, mod->priv);
        } else {
            buf = NCJS_TEXT("No such module: "); buf += module;
            except = buf;
            return;
        }

        cache->SetValue(module, exports, V8_PROPERTY_ATTRIBUTE_NONE);
        retval = exports;
    }

    // process._linkedBinding()
    NCJS_OBJECT_FUNCTION(LinkedBinding)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        CefRefPtr<CefV8Context> context = CefV8Context::GetCurrentContext();
        CefRefPtr<Environment> env = Environment::Get(context);
        CefRefPtr<CefV8Value> cache = env->GetObject().binding;

        const CefString module = args[0]->GetStringValue();
        if (cache->HasValue(module)) {
            retval = cache->GetValue(module);
            return;
        }

        CefRefPtr<CefV8Value> exports;

        if (module_t* mod = ModuleManager::GetLinked(module)) {
            if (mod->ctxfactory) {
                exports = mod->ctxfactory(env, context, mod->priv);
            } else {
                except = NCJS_TEXT("Linked module has no declared entry point.");
                return;
            }
        } else {
            std_string buf(NCJS_TEXT("No such module was linked: ")); buf += module;
            except = buf;
            return;
        }

        cache->SetValue(module, exports, V8_PROPERTY_ATTRIBUTE_NONE);
        retval = exports;
    }

    // process._startProfilerIdleNotifier()
    NCJS_OBJECT_FUNCTION(StartProfilerIdleNotifier)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        except = consts::str_err_notimpl;
    }

    // process._stopProfilerIdleNotifier()
    NCJS_OBJECT_FUNCTION(StopProfilerIdleNotifier)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        except = consts::str_err_notimpl;
    }

    // process._getActiveRequests()
    NCJS_OBJECT_FUNCTION(GetActiveRequests)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        except = consts::str_err_notimpl;
    }

    // process._getActiveHandles()
    NCJS_OBJECT_FUNCTION(GetActiveHandles)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        except = consts::str_err_notimpl;
    }

    // process.reallyExit()
    NCJS_OBJECT_FUNCTION(Exit)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        except = consts::str_err_notimpl;
    }

    // process.abort()
    NCJS_OBJECT_FUNCTION(Abort)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        except = consts::str_err_notimpl;
    }

    // process.chdir()
    NCJS_OBJECT_FUNCTION(Chdir)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        if (args.size() != 1 || !args[0]->IsString())
            return TYPE_ERROR("Bad argument.");

        std::string path = args[0]->GetStringValue().ToString();
        
        if (int err = uv_chdir(path.c_str()))
            return UV_ERROR(err, "uv_chdir");
    }

    // process.cwd()
    NCJS_OBJECT_FUNCTION(Cwd)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
#ifdef _WIN32
        char buf[MAX_PATH * 4];
#else
        char buf[PATH_MAX];
#endif

        size_t len = sizeof(buf);
        if (int err = uv_cwd(buf, &len))
            return UV_ERROR(err, "uv_cwd");

        retval = CefV8Value::CreateString(std::string(buf, len));
    }

    // process.umask()
    NCJS_OBJECT_FUNCTION(Umask)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        uint32_t old;

        if (args.size() < 1 || args[0]->IsUndefined()) {
            old = umask(0);
            umask(static_cast<mode_t>(old));
        } else if (!args[0]->IsInt() && !args[0]->IsString()) {
            return TYPE_ERROR("argument must be an integer or octal string.");
        } else {
            int oct;
            if (args[0]->IsInt()) {
                oct = args[0]->GetUIntValue();
            } else {
                oct = 0;
                CefString str = args[0]->GetStringValue();

                // Parse the octal string
                const cef_char_t* buf = str.c_str();
                for (size_t i = 0; i < str.length(); i++) {
                    cef_char_t c = buf[i];
                    if (c > NCJS_TEXT('7') || c < NCJS_TEXT('0')) {
                        return TYPE_ERROR("invalid octal string");
                    }
                    oct *= 8;
                    oct += c - NCJS_TEXT('0');
                }
            }
            old = umask(static_cast<mode_t>(oct));
        }

        retval = CefV8Value::CreateUInt(old);
    }

    // process._kill()
    NCJS_OBJECT_FUNCTION(Kill)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        if (args.size() != 2)
            return TYPE_ERROR("Bad argument.");

        int pid = args[0]->GetIntValue();
        int sig = args[1]->GetIntValue();
        int err = uv_kill(pid, sig);

        retval = CefV8Value::CreateInt(err);
    }

    // process.hrtime()
    NCJS_OBJECT_FUNCTION(Hrtime)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        uint64_t t = uv_hrtime();

        if (args.size() > 0) {
            // return a time diff tuple
            if (!(args[0]->IsArray() && args[0]->GetArrayLength() == 2))
                return TYPE_ERROR("process.hrtime() only accepts an Array tuple.");

            CefRefPtr<CefV8Value> inArray = args[0];
            uint64_t seconds = inArray->GetValue(0)->GetUIntValue();
            uint64_t   nanos = inArray->GetValue(1)->GetUIntValue();
            t -= (seconds * NANOS_PER_SEC) + nanos;
        }

        retval = CefV8Value::CreateArray(2);
        retval->SetValue(0, CefV8Value::CreateUInt(unsigned(t / NANOS_PER_SEC)));
        retval->SetValue(1, CefV8Value::CreateUInt(unsigned(t % NANOS_PER_SEC)));
    }

    // process.dlopen()
    NCJS_OBJECT_FUNCTION(DLOpen)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        except = consts::str_err_notimpl;
    }

    // process.uptime()
    NCJS_OBJECT_FUNCTION(Uptime)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        uv_loop_t* loop = Environment::GetEventLoop();
        uv_update_time(loop);
        double uptime = uv_now(loop) - Environment::GetProcessStartTime();

        retval = CefV8Value::CreateDouble(uptime / 1000);
    }

    // process.memoryUsage()
    NCJS_OBJECT_FUNCTION(MemoryUsage)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        size_t rss;

        if (int err = uv_resident_set_memory(&rss))
            return UV_ERROR(err, "uv_resident_set_memory");

        // TODO: add missing 'heapTotal' and 'heapUsed'

        retval = CefV8Value::CreateObject(NULL);
        retval->SetValue(consts::str_rss, CefV8Value::CreateDouble(double(rss)),
                         V8_PROPERTY_ATTRIBUTE_NONE);
    }

    // process._setupPromises()
    NCJS_OBJECT_FUNCTION(SetupPromises)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        except = consts::str_err_notimpl;
    }

    // process._setupDomainUse()
    NCJS_OBJECT_FUNCTION(SetupDomainUse)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        except = consts::str_err_notimpl;
    }

    // object factory

    NCJS_BEGIN_OBJECT_FACTORY()
        // constants
        NCJS_MAP_OBJECT_READONLY(String, consts::str_version,  NODE_VERSION)
        NCJS_MAP_OBJECT_READONLY(String, consts::str_arch,     NCJS_REFTEXT(NCJS_ARCH))
        NCJS_MAP_OBJECT_READONLY(String, consts::str_platform, NCJS_REFTEXT(NCJS_PLATFORM))
        NCJS_MAP_OBJECT_READONLY(Int, consts::str_pid, getpid())
        // objects
        NCJS_MAP_OBJECT(Object, consts::str_events, NULL)
        NCJS_MAP_OBJECT_FACTORY(consts::str_versions, VersionsObject)
        NCJS_MAP_OBJECT_FACTORY_READONLY(consts::str_features, FeaturesObject)
        // functions
        NCJS_MAP_OBJECT_FUNCTION(consts::str_binding, Binding)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("_linkedBinding"), LinkedBinding)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("_startProfilerIdleNotifier"), StartProfilerIdleNotifier)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("_stopProfilerIdleNotifier"), StopProfilerIdleNotifier)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("_getActiveRequests"), GetActiveRequests)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("_getActiveHandles"), GetActiveHandles)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("reallyExit"), Exit)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("abort"), Abort)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("chdir"), Chdir)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("cwd"), Cwd)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("umask"), Umask)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("_kill"), Kill)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("hrtime"), Hrtime)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("dlopen"), DLOpen)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("uptime"), Uptime)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("memoryUsage"), MemoryUsage)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("_setupPromises"), SetupPromises)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("_setupDomainUse"), SetupDomainUse)
        // accessors
        NCJS_MAP_OBJECT_ACCESSOR(consts::str_env, EnvAccessor)
    NCJS_END_OBJECT_FACTORY()
};

/// ----------------------------------------------------------------------------
/// static functions
/// ----------------------------------------------------------------------------

CefRefPtr<CefV8Value> Process::CreateObject(CefRefPtr<Environment> enviroment,
    CefRefPtr<CefV8Context> context)
{
    return ProcessObject::ObjectFactory(enviroment, context);
}

} // ncjs

