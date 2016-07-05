
/***************************************************************
 * Name:      fs.cpp
 * Purpose:   Code for Node-CEF FS Module
 * Author:    Joshua GPBeta (studiocghibli@gmail.com)
 * Created:   2016-05-27
 * Copyright: Studio GPBeta (www.gpbeta.com)
 * License:
 **************************************************************/

/// ============================================================================
/// declarations
/// ============================================================================

#define _WINSOCKAPI_    // stops windows.h including winsock.h

#define ASYNC_DEST_CALL(_FUNCTION, _REQ, _DEST, ...) \
    EventLoop& _loop = Environment::GetAsyncLoop(); \
    CefRefPtr<AsyncReqWrap> _wrap(new AsyncReqWrap(_loop, #_FUNCTION, _DEST, _REQ)); \
    AsyncCall<&uv_fs_##_FUNCTION>(_loop, _wrap, &uv_fs_##_FUNCTION, __VA_ARGS__); \
    retval = _REQ

#define ASYNC_CALL(_FUNCTION, _REQ, ...) \
    ASYNC_DEST_CALL(_FUNCTION, _REQ, NULL, __VA_ARGS__)

#define ASYNC_HOLD_DATA(_DATA) _wrap->HoldData(_DATA)

#define SYNC_DEST_CALL(_FUNCTION, _PATH, _DEST, ...) \
    SyncReqWrap _req; \
    int _err = uv_fs_##_FUNCTION(Environment::GetSyncLoop(), \
                               &_req.req, __VA_ARGS__, NULL); \
    if (_err < 0) \
        return Environment::UvException(_err, #_FUNCTION, NULL, _PATH, _DEST, except)

#define SYNC_CALL(_FUNCTION, _PATH, ...) \
    SYNC_DEST_CALL(_FUNCTION, _PATH, NULL, __VA_ARGS__)

#define SYNC_REQ _req.req
#define SYNC_RESULT _err

#define    TYPE_ERROR(_MSG) Environment::TypeException(NCJS_TEXT(_MSG), except)
#define   RANGE_ERROR(_MSG) Environment::RangeException(NCJS_TEXT(_MSG), except)
#define UNKNOWN_ERROR(_MSG) Environment::ErrorException(NCJS_TEXT(_MSG), except)

#define GET_OFFSET(_VAL) ((_VAL)->IsInt() ? (_VAL)->GetIntValue() : -1)

#define GET_PARAM_FD(_ARGS, _FD) \
    if (_ARGS.size() < 1) \
        return TYPE_ERROR("fd is required"); \
    if (!_ARGS[0]->IsInt()) \
        return TYPE_ERROR("fd must be a file descriptor"); \
    const int _FD = _ARGS[0]->GetIntValue()

#define GET_PARAM_FD_INT(_ARGS, _FD, _VAR) \
    if (_ARGS.size() < 2) \
        return TYPE_ERROR("fd and " NCJS_TEXT(#_VAR) NCJS_TEXT(" are required")); \
    if (!_ARGS[0]->IsInt()) \
        return TYPE_ERROR("fd must be a file descriptor"); \
    if (!_ARGS[1]->IsInt()) \
        return TYPE_ERROR(#_VAR NCJS_TEXT(" must be an integer")); \
    const int _FD = args[0]->GetIntValue(); \
    const int _VAR = args[1]->GetIntValue()

#define GET_PARAM_PATH(_ARGS, _PATH) \
    if (_ARGS.size() < 1) \
        return TYPE_ERROR("path required"); \
    if (!_ARGS[0]->IsString()) \
        return TYPE_ERROR("path must be a string"); \
    const AutoString _PATH(_ARGS[0]->GetStringValue().ToString())

#define GET_PARAM_PATH_MODE(_ARGS, _PATH, _MODE) \
    if (_ARGS.size() < 2) \
        return TYPE_ERROR("path and mode are required"); \
    if (!_ARGS[0]->IsString()) \
        return TYPE_ERROR("path must be a string"); \
    if (!_ARGS[1]->IsInt()) \
        return TYPE_ERROR("mode must be an integer"); \
    const AutoString _PATH(_ARGS[0]->GetStringValue().ToString()); \
    const int _MODE = _ARGS[1]->GetIntValue()

#define GET_PARAM_SRC_DST(_ARGS, _SRC, _DST) \
    if (_ARGS.size() < 1) \
        return TYPE_ERROR("dst path required"); \
    if (_ARGS.size() < 2) \
        return TYPE_ERROR("src path required"); \
    if (!_ARGS[0]->IsString()) \
        return TYPE_ERROR("dst path must be a string"); \
    if (!_ARGS[1]->IsString()) \
        return TYPE_ERROR("src path must be a string"); \
    const AutoString _SRC(_ARGS[0]->GetStringValue().ToString()); \
    const AutoString _DST(_ARGS[1]->GetStringValue().ToString())

/// ----------------------------------------------------------------------------
/// headers
/// ----------------------------------------------------------------------------

#include "ncjs/module.h"
#include "ncjs/constants.h"
#include "ncjs/EventLoop.h"
#include "ncjs/module/fs.h"
#include "ncjs/module/buffer.h"

#include <include/base/cef_bind.h>
#include <include/wrapper/cef_closure_task.h>
#include <uv.h>

#include <fcntl.h>

namespace ncjs {

/// ----------------------------------------------------------------------------
/// variables
/// ----------------------------------------------------------------------------

class AutoString : public std::string {
public:
    AutoString(const char* str) { if (str) assign(str); }
    AutoString(const std::string& str) : std::string(str) {}
    operator const char*() const { return length() ? c_str() : NULL; }
};

class AutoUvBuffer : public CefBase, public uv_buf_t {
public:
    AutoUvBuffer(const CefRefPtr<Buffer>& buf, unsigned pos, unsigned sz) :
        holder(buf) { base = buf->Data() + pos; len = sz; }

private:
    CefRefPtr<Buffer> holder;

    IMPLEMENT_REFCOUNTING(AutoUvBuffer);
};

struct SyncReqWrap {
    uv_fs_t req;

    SyncReqWrap() {}
    ~SyncReqWrap() { uv_fs_req_cleanup(&req); }

    DISALLOW_COPY_AND_ASSIGN(SyncReqWrap);
};

class AsyncReqWrap : public CefBase {

    template <void* T> struct After;

public:
    template <void* T, class F, class P1>
    void Run(const P1& p1)
    {
        Dispatch(static_cast<F>(T)(loop, &req, p1, &After<T>::Entry));
    }

    template <void* T, class F, class P1, class P2>
    void Run(const P1& p1, const P2& p2)
    {
        Dispatch(static_cast<F>(T)(loop, &req, p1, p2, &After<T>::Entry));
    }

    template <void* T, class F, class P1, class P2, class P3>
    void Run(const P1& p1, const P2& p2, const P3& p3)
    {
        Dispatch(static_cast<F>(T)(loop, &req, p1, p2, p3, &After<T>::Entry));
    }

    template <void* T, class F, class P1, class P2, class P3, class P4>
    void Run(const P1& p1, const P2& p2, const P3& p3, const P4& p4)
    {
        Dispatch(static_cast<F>(T)(loop, &req, p1, p2, p3, p4, &After<T>::Entry));
    }

    void HoldData(const CefRefPtr<CefBase>& lifeSpanData)
    {
        data = lifeSpanData;
    }

    AsyncReqWrap(EventLoop& eventLoop, const char* syscall, const AutoString& destPath,
        const CefRefPtr<CefV8Value>& reqWrap) :
        loop(eventLoop.ToUv()), call(syscall), dest(destPath),
        context(CefV8Context::GetCurrentContext()), wrap(reqWrap) {}
    ~AsyncReqWrap() { uv_fs_req_cleanup(&req); }

private:

    void Dispatch(int result)
    {
        // keep alive, must call Release manually
        req.data = this; AddRef();

        if (result < 0) {
            req.result = result;
            CefPostTask(TID_RENDERER, base::Bind(&AsyncReqWrap::OnError, this));
        }
    }

    template <void* T> struct After {
        static void Entry(uv_fs_t* req)
        {
            AsyncReqWrap* wrap = static_cast<AsyncReqWrap*>(req->data);

            NCJS_ASSERT(wrap);

            if (req->result < 0) {
                CefPostTask(TID_RENDERER, base::Bind(&AsyncReqWrap::OnError, wrap));
            } else {
                CefPostTask(TID_RENDERER, base::Bind(&AsyncReqWrap::OnSuccess<T>, wrap));
            }
        }
    };

    template <void* T>
    void Success(Environment& env, CefV8ValueList& args) {}

    // specialized Success<T>()

    template <> void Success<&uv_fs_utime>(Environment& env, CefV8ValueList& args) 
    {
        args.clear();
    }
    template <> void Success<&uv_fs_futime>(Environment& env, CefV8ValueList& args)
    {
        args.clear();
    }
    template <> void Success<&uv_fs_open>(Environment& env, CefV8ValueList& args)
    {
        args.push_back(CefV8Value::CreateInt(int(req.result)));
    }
    template <> void Success<&uv_fs_read>(Environment& env, CefV8ValueList& args)
    {
        args.push_back(CefV8Value::CreateInt(int(req.result)));
    }
    template <> void Success<&uv_fs_write>(Environment& env, CefV8ValueList& args)
    {
        args.push_back(CefV8Value::CreateInt(int(req.result)));
    }
    template <> void Success<&uv_fs_stat>(Environment& env, CefV8ValueList& args)
    {
        args.push_back(BuildStatsObject(env, static_cast<uv_stat_t*>(req.ptr)));
    }
    template <> void Success<&uv_fs_lstat>(Environment& env, CefV8ValueList& args)
    {
        args.push_back(BuildStatsObject(env, static_cast<uv_stat_t*>(req.ptr)));
    }
    template <> void Success<&uv_fs_fstat>(Environment& env, CefV8ValueList& args)
    {
        args.push_back(BuildStatsObject(env, static_cast<uv_stat_t*>(req.ptr)));
    }
    template <> void Success<&uv_fs_readlink>(Environment& env, CefV8ValueList& args)
    {
        args.push_back(CefV8Value::CreateString(static_cast<const char*>(req.ptr)));
    }
    template <> void Success<&uv_fs_scandir>(Environment& env, CefV8ValueList& args)
    {
        CefRefPtr<CefV8Value> names = CefV8Value::CreateArray(0);

        for (unsigned i = 0; ; ++i) {
            uv_dirent_t ent;
            const int res = uv_fs_scandir_next(&req, &ent);

            if (res == UV_EOF)
                break;
            if (res) {
                CefString except;
                CefV8ValueList str;
                Environment::UvException(res, call, NULL,
                    static_cast<const char*>(req.path), NULL, except);
                str.push_back(CefV8Value::CreateString(except));
                args.clear();
                args.push_back(env.GetFunction().new_error->ExecuteFunction(NULL, str));
                break;
            }

            names->SetValue(i, CefV8Value::CreateString(ent.name));               
        }

        args.push_back(names);
    }

    template <void* T>
    void OnSuccess()
    {
        if (Environment* env = Environment::Get(context)) {
            context->Enter();

            CefV8ValueList args; // should have at least 1 argument
            args.push_back(CefV8Value::CreateNull());
            Success<T>(*env, args);
            CefRefPtr<CefV8Value> callback = wrap->GetValue(consts::str_oncomplete);
            callback->ExecuteFunction(wrap, args);

            context->Exit();
        }
        // really delete here
        req.data = NULL; Release();
    }

    void OnError()
    {
        if (Environment* env = Environment::Get(context)) {
            context->Enter();

            CefV8ValueList args;
            CefString except;
            Environment::UvException(int(req.result), call, NULL, req.path, dest, except);
            args.push_back(CefV8Value::CreateString(except));
            CefRefPtr<CefV8Value> callback = wrap->GetValue(consts::str_oncomplete);
            callback->ExecuteFunction(wrap, args);

            context->Exit();
        } // else context already released

        // really delete here
        req.data = NULL; Release();
    }

    /// Declarations
    /// -----------------

    uv_loop_t* loop;
    uv_fs_t req;

    const char* call;
    AutoString dest;

    CefRefPtr<CefV8Context> context;
    CefRefPtr<CefV8Value> wrap;

    CefRefPtr<CefBase> data;

    DISALLOW_COPY_AND_ASSIGN(AsyncReqWrap);
    IMPLEMENT_REFCOUNTING(AsyncReqWrap);
};

template <void* T, class F, class P1>
void AsyncCall(EventLoop& loop, const CefRefPtr<AsyncReqWrap>& wrap,
    const F&, const P1& p1)
{
    loop.Queue(base::Bind(&AsyncReqWrap::Run<T, F, P1>, wrap, p1));
}

template <void* T, class F, class P1, class P2>
void AsyncCall(EventLoop& loop, const CefRefPtr<AsyncReqWrap>& wrap,
    const F&, const P1& p1, const P2& p2)
{
    loop.Queue(base::Bind(&(AsyncReqWrap::Run<T, F, P1, P2>), wrap, p1, p2));
}

template <void* T, class F, class P1, class P2, class P3>
void AsyncCall(EventLoop& loop, const CefRefPtr<AsyncReqWrap>& wrap,
    const F&, const P1& p1, const P2& p2, const P3& p3)
{
    loop.Queue(base::Bind(&AsyncReqWrap::Run<T, F, P1, P2, P3>, wrap, p1, p2, p3));
}

template <void* T, class F, class P1, class P2, class P3, class P4>
void AsyncCall(EventLoop& loop, const CefRefPtr<AsyncReqWrap>& wrap,
    const F&, const P1& p1, const P2& p2, const P3& p3, const P4& p4)
{
    loop.Queue(base::Bind(&AsyncReqWrap::Run<T, F, P1, P2, P3, P4>, wrap, p1, p2, p3, p4));
}

/// ============================================================================
/// implementation
/// ============================================================================

CefRefPtr<CefV8Value> BuildStatsObject(Environment& env, const UvState* stat)
{
    const uv_stat_t* s = static_cast<const uv_stat_t*>(stat);
    // see node_file.cc #343
    CefV8ValueList argv;
    argv.reserve(14);

    // Integers.
#define X(_NAME) \
    CefRefPtr<CefV8Value> _NAME = CefV8Value::CreateUInt(unsigned(s->st_##_NAME)); \
    if (!_NAME.get()) \
        return _NAME; \
    argv.push_back(_NAME);

    X(dev)
    X(mode)
    X(nlink)
    X(uid)
    X(gid)
    X(rdev)
# if defined(__POSIX__)
    X(blksize)
# else
    argv.push_back(CefV8Value::CreateUndefined());
# endif
#undef X

    // Numbers.
#define X(_NAME) \
    CefRefPtr<CefV8Value> _NAME = CefV8Value::CreateDouble(double(s->st_##_NAME)); \
    if (!_NAME.get()) \
        return _NAME; \
    argv.push_back(_NAME);

    X(ino)
    X(size)
# if defined(__POSIX__)
    X(blocks)
# else
    argv.push_back(CefV8Value::CreateUndefined());
# endif
#undef X

    // Dates.
#define X(_NAME) \
    CefRefPtr<CefV8Value> _NAME##_msec = CefV8Value::CreateDouble( \
                (double(s->st_##_NAME.tv_sec) * 1000) + \
                (double(s->st_##_NAME.tv_nsec / 1000000)));\
    if (!_NAME##_msec.get()) \
        return _NAME##_msec; \
    argv.push_back(_NAME##_msec);

    X(atim)
    X(mtim)
    X(ctim)
    X(birthtim)
#undef X

    // Call out to JavaScript to create the stats object
    return env.New(env.GetFunction().ctor_fs_stats, argv);
}

/// ----------------------------------------------------------------------------
/// FSReqWrap
/// ----------------------------------------------------------------------------

class FSReqWrap : public JsObjecT<FSReqWrap> {
    // FSReqWrap()
    NCJS_OBJECT_FUNCTION(Constructor)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        // TODO: check if constructor call
    }

    // class factory

    NCJS_BEGIN_CLASS_FACTORY(Constructor)
    NCJS_END_CLASS_FACTORY()
};

class StatWatcher {
public: // we should really separate StatWatcher from fs module
    static CefRefPtr<CefV8Value> ObjectFactory(CefRefPtr<Environment>, CefRefPtr<CefV8Context>);
};

/// ----------------------------------------------------------------------------
/// ModuleFS
/// ----------------------------------------------------------------------------

class ModuleFS : public JsObjecT<ModuleFS> {

    // fs.internalModuleReadFile()
    NCJS_OBJECT_FUNCTION(InternalModuleReadFile)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        uv_loop_t* loop = Environment::GetSyncLoop();

        NCJS_CHECK(NCJS_ARG_IS(String, args, 0));
        const AutoString path(args[0]->GetStringValue().ToString());

        uv_fs_t reqOpen;
        const int fd = uv_fs_open(loop, &reqOpen, path, O_RDONLY, 0, NULL);
        uv_fs_req_cleanup(&reqOpen);

        if (fd < 0)
            return;

        std::vector<char> chars;
        int64_t offset = 0;
        for (;;) {
            const size_t BLOCK_SIZE = 32 << 10;
            const size_t start = chars.size();
            chars.resize(start + BLOCK_SIZE);

            uv_buf_t buf;
            buf.base = &chars[start];
            buf.len = BLOCK_SIZE;

            uv_fs_t reqRead;
            const ssize_t nChar = uv_fs_read(loop, &reqRead, fd, &buf, 1, offset, NULL);
            uv_fs_req_cleanup(&reqRead);

            NCJS_CHK_GE(nChar, 0);
            if (static_cast<size_t>(nChar) < BLOCK_SIZE)
                chars.resize(start + nChar);

            if (nChar == 0)
                break;

            offset += nChar;
        }

        uv_fs_t reqClose;
        NCJS_CHK_EQ(0, uv_fs_close(loop, &reqClose, fd, NULL));
        uv_fs_req_cleanup(&reqClose);

        size_t start = 0;
        // Skip UTF-8 BOM.
        if (chars.size() >= 3 && 0 == memcmp(&chars[0], "\xEF\xBB\xBF", 3))
            start = 3;  

        const CefString text(&chars[start]);
        retval = CefV8Value::CreateString(text);
    }

    // fs.internalModuleStat()
    NCJS_OBJECT_FUNCTION(InternalModuleStat)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        uv_loop_t* loop = Environment::GetSyncLoop();

        NCJS_CHECK(NCJS_ARG_IS(String, args, 0));

        const AutoString path(args[0]->GetStringValue().ToString());

        uv_fs_t req;
        int rc = uv_fs_stat(loop, &req, path, NULL);
        if (rc == 0) {
            const uv_stat_t* const s = static_cast<const uv_stat_t*>(req.ptr);
            rc = !!(s->st_mode & S_IFDIR);
        }
        uv_fs_req_cleanup(&req);

        retval = CefV8Value::CreateInt(rc);
    }

    // fs.access()
    NCJS_OBJECT_FUNCTION(Access)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        GET_PARAM_PATH_MODE(args, path, mode);

        if (NCJS_ARG_IS(Object, args, 2)) {
            ASYNC_CALL(access, args[2], path, mode);
        } else {
            SYNC_CALL(access, path, path, mode);
        }
    }

    // fs.close()
    NCJS_OBJECT_FUNCTION(Close)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        GET_PARAM_FD(args, fd);

        if (NCJS_ARG_IS(Object, args, 1)) {
            ASYNC_CALL(close, args[1], fd);
        } else {
            SYNC_CALL(close, 0, fd);
        }
    }

    // fs.open()
    NCJS_OBJECT_FUNCTION(Open)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        const size_t size = args.size();
        if (size < 1)
            return TYPE_ERROR("path required");
        if (size < 2)
            return TYPE_ERROR("flags required");
        if (size < 3)
            return TYPE_ERROR("mode required");
        if (!args[0]->IsString())
            return TYPE_ERROR("path must be a string");
        if (!args[1]->IsInt())
            return TYPE_ERROR("flags must be an int");
        if (!args[2]->IsInt())
            return TYPE_ERROR("mode must be an int");

        const AutoString path(args[0]->GetStringValue().ToString());
        const int flags = args[1]->GetIntValue();
        const int mode = args[2]->GetIntValue();

        if (NCJS_ARG_IS(Object, args, 3)) {
            ASYNC_CALL(open, args[3], path, flags, mode);
        } else {
            SYNC_CALL(open, path, path, flags, mode);
            retval = CefV8Value::CreateInt(SYNC_RESULT);
        }
    }

    // fs.read()
    NCJS_OBJECT_FUNCTION(Read)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        Buffer* buf = NULL;

        if (args.size() < 2)
            return TYPE_ERROR("fd and buffer are required");
        if (!args[0]->IsInt())
            return TYPE_ERROR("fd must be a file descriptor");
        if (!(buf = Buffer::Unwrap(args[1])))
            return TYPE_ERROR("Second argument needs to be a buffer");

        const int fd = args[0]->GetIntValue();

        int64_t pos = -1;
        unsigned len = 0;
        unsigned off = 0;

        if (NCJS_ARG_IS(Int, args, 2))
            off = args[2]->GetUIntValue();
        if (off >= buf->Size())
            return RANGE_ERROR("Offset is out of bounds");

        if (NCJS_ARG_IS(Int, args, 3))
            len = args[3]->GetUIntValue();
        if (!Buffer::IsWithinBounds(off, len, buf->Size()))
            return RANGE_ERROR("Length extends beyond buffer");

        if (NCJS_ARG_IS(Int, args, 4))
            pos = args[4]->GetIntValue(); // be aware -1

        CefRefPtr<AutoUvBuffer> uvbuf(new AutoUvBuffer(buf, off, len));

        if (NCJS_ARG_IS(Object, args, 5)) {
            ASYNC_CALL(read, args[5], fd, uvbuf, 1, pos);
        } else {
            SYNC_CALL(read, 0, fd, uvbuf, 1, pos);
            retval = CefV8Value::CreateInt(SYNC_RESULT);
        }
    }

    // fs.fdatasync()
    NCJS_OBJECT_FUNCTION(FDataSync)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        GET_PARAM_FD(args, fd);

        if (NCJS_ARG_IS(Object, args, 1)) {
            ASYNC_CALL(fdatasync, args[1], fd);
        } else {
            SYNC_CALL(fdatasync, 0, fd);
        }
    }

    // fs.fsync()
    NCJS_OBJECT_FUNCTION(FSync)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        GET_PARAM_FD(args, fd);

        if (NCJS_ARG_IS(Object, args, 1)) {
            ASYNC_CALL(fsync, args[1], fd);
        } else {
            SYNC_CALL(fsync, 0, fd);
        }
    }

    // fs.ftruncate()
    NCJS_OBJECT_FUNCTION(FTruncate)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        GET_PARAM_FD_INT(args, fd, length);

        if (NCJS_ARG_IS(Object, args, 2)) {
            ASYNC_CALL(ftruncate, args[2], fd, length);
        } else {
            SYNC_CALL(ftruncate, 0, fd, length);
        }
    }

    // fs.rename()
    NCJS_OBJECT_FUNCTION(Rename)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        GET_PARAM_SRC_DST(args, strOld, strNew);

        const char* pathOld = strOld.c_str();
        const char* pathNew = strNew.c_str();

        if (NCJS_ARG_IS(Object, args, 2)) {
            ASYNC_DEST_CALL(rename, args[2], pathNew, pathOld, pathNew);
        } else {
            SYNC_DEST_CALL(rename, pathOld, pathNew, pathOld, pathNew);
        }
    }

    // fs.rmdir()
    NCJS_OBJECT_FUNCTION(RMDir)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        GET_PARAM_PATH(args, path);

        if (NCJS_ARG_IS(Object, args, 1)) {
            ASYNC_CALL(rmdir, args[1], path);
        } else {
            SYNC_CALL(rmdir, path, path);
        }
    }

    // fs.mkdir()
    NCJS_OBJECT_FUNCTION(MKDir)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        GET_PARAM_PATH_MODE(args, path, mode);

        if (NCJS_ARG_IS(Object, args, 2)) {
            ASYNC_CALL(mkdir, args[2], path, mode);
        } else {
            SYNC_CALL(mkdir, path, path, mode);
        }
    }

    // fs.readdir()
    NCJS_OBJECT_FUNCTION(ReadDir)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        GET_PARAM_PATH(args, path);

        if (NCJS_ARG_IS(Object, args, 1)) {
            ASYNC_CALL(scandir, args[1], path, 0);
        } else {
            SYNC_CALL(scandir, path, path, 0);

            NCJS_CHK_GE(SYNC_REQ.result, 0);

            CefRefPtr<CefV8Value> names = CefV8Value::CreateArray(0);

            for (unsigned i = 0; ; ++i) {
                uv_dirent_t ent;
                const int res = uv_fs_scandir_next(&SYNC_REQ, &ent);

                if (res == UV_EOF)
                    break;
                if (res)
                    return Environment::UvException(res, "readdir", NULL,
                                                    path, NULL, except);

                names->SetValue(i, CefV8Value::CreateString(ent.name));               
            }

            retval = names;
        }
    }

    // fs.stat()
    NCJS_OBJECT_FUNCTION(Stat)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        GET_PARAM_PATH(args, path);

        Environment* env = Environment::Get(CefV8Context::GetCurrentContext());

        if (NCJS_ARG_IS(Object, args, 1)) {
            ASYNC_CALL(stat, args[1], path);
        } else {
            SYNC_CALL(stat, path, path);
            retval = BuildStatsObject(*env, static_cast<uv_stat_t*>(SYNC_REQ.ptr));
        }
    }

    // fs.lstat()
    NCJS_OBJECT_FUNCTION(LStat)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        GET_PARAM_PATH(args, path);

        Environment* env = Environment::Get(CefV8Context::GetCurrentContext());

        if (NCJS_ARG_IS(Object, args, 1)) {
             ASYNC_CALL(lstat, args[1], path);
        } else {
             SYNC_CALL(lstat, path, path);
             retval = BuildStatsObject(*env, static_cast<uv_stat_t*>(SYNC_REQ.ptr));
        }
    }

    // fs.fstat()
    NCJS_OBJECT_FUNCTION(FStat)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        GET_PARAM_FD(args, fd);

        Environment* env = Environment::Get(CefV8Context::GetCurrentContext());

        if (NCJS_ARG_IS(Object, args, 1)) {
            ASYNC_CALL(fstat, args[1], fd);
        } else {
            SYNC_CALL(fstat, 0, fd);
            retval = BuildStatsObject(*env, static_cast<uv_stat_t*>(SYNC_REQ.ptr));
        }
    }

    // fs.link()
    NCJS_OBJECT_FUNCTION(Link)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        GET_PARAM_SRC_DST(args, strSrc, strDst);

        const char* pathSrc = strSrc.c_str();
        const char* pathDst = strDst.c_str();

        if (NCJS_ARG_IS(Object, args, 2)) {
            ASYNC_DEST_CALL(link, args[2], pathDst, pathSrc, pathDst);
        } else {
            SYNC_DEST_CALL(link, pathSrc, pathDst, pathSrc, pathDst);
        }
    }

    // fs.symlink()
    NCJS_OBJECT_FUNCTION(SymLink)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        GET_PARAM_SRC_DST(args, strSrc, strDst);

        int flags = 0;
        if (NCJS_ARG_IS(Object, args, 2)) {
            const CefString mode = args[2]->GetStringValue();
            if (mode == consts::str_dir)
                flags |= UV_FS_SYMLINK_DIR;
            else if (mode == consts::str_junction)
                flags |= UV_FS_SYMLINK_JUNCTION;
            else if (mode != consts::str_file)
                return UNKNOWN_ERROR("Unknown symlink type");
        }

        const char* target = strSrc.c_str();
        const char* path = strDst.c_str();

        if (NCJS_ARG_IS(Object, args, 3)) {
            ASYNC_DEST_CALL(symlink, args[3], path, target, path, flags);
        } else {
            SYNC_DEST_CALL(symlink, target, path, target, path, flags);
        }
    }

    // fs.readlink()
    NCJS_OBJECT_FUNCTION(ReadLink)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        GET_PARAM_PATH(args, path);

        if (NCJS_ARG_IS(Object, args, 1)) {
            ASYNC_CALL(readlink, args[1], path);
        } else {
            SYNC_CALL(readlink, path, path);

            retval = CefV8Value::CreateString(static_cast<const char*>(SYNC_REQ.ptr));
        }
    }

    // fs.unlink()
    NCJS_OBJECT_FUNCTION(Unlink)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        GET_PARAM_PATH(args, path);

        if (NCJS_ARG_IS(Object, args, 1)) {
            ASYNC_CALL(unlink, args[1], path);
        } else {
            SYNC_CALL(unlink, path, path);
        }
    }

    // fs.chmod()
    NCJS_OBJECT_FUNCTION(Chmod)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        GET_PARAM_PATH_MODE(args, path, mode);

        if (NCJS_ARG_IS(Object, args, 2)) {
            ASYNC_CALL(chmod, args[2], path, mode);
        } else {
            SYNC_CALL(chmod, path, path, mode);
        }
    }

    // fs.fchmod()
    NCJS_OBJECT_FUNCTION(FChmod)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        GET_PARAM_FD_INT(args, fd, mode);

        if (NCJS_ARG_IS(Object, args, 2)) {
            ASYNC_CALL(fchmod, args[2], fd, mode);
        } else {
            SYNC_CALL(fchmod, 0, fd, mode);
        }
    }

    // fs.chown()
    NCJS_OBJECT_FUNCTION(Chown)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        const size_t size = args.size();
        if (size < 1)
            return TYPE_ERROR("path required");
        if (size < 2)
            return TYPE_ERROR("uid required");
        if (size < 3)
            return TYPE_ERROR("gid required");
        if (!args[0]->IsString())
            return TYPE_ERROR("path must be a string");
        if (!args[1]->IsUInt())
            return TYPE_ERROR("uid must be an unsigned int");
        if (!args[2]->IsUInt())
            return TYPE_ERROR("gid must be an unsigned int");

        const AutoString path(args[0]->GetStringValue().ToString());
        const uv_uid_t uid = args[1]->GetUIntValue();
        const uv_gid_t gid = args[2]->GetUIntValue();

        if (NCJS_ARG_IS(Object, args, 3)) {
            ASYNC_CALL(chown, args[3], path, uid, gid);
        } else {
            SYNC_CALL(chown, path, path, uid, gid);
        }
    }

    // fs.fchown()
    NCJS_OBJECT_FUNCTION(FChown)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        const size_t size = args.size();
        if (size < 1)
            return TYPE_ERROR("fd required");
        if (size < 2)
            return TYPE_ERROR("uid required");
        if (size < 3)
            return TYPE_ERROR("gid required");
        if (!args[0]->IsInt())
            return TYPE_ERROR("fd must be a file descriptor");
        if (!args[1]->IsUInt())
            return TYPE_ERROR("uid must be an unsigned int");
        if (!args[2]->IsUInt())
            return TYPE_ERROR("gid must be an unsigned int");

        const int fd = args[0]->GetIntValue();
        const uv_uid_t uid = args[1]->GetUIntValue();
        const uv_gid_t gid = args[2]->GetUIntValue();

        if (NCJS_ARG_IS(Object, args, 3)) {
            ASYNC_CALL(fchown, args[3], fd, uid, gid);
        } else {
            SYNC_CALL(fchown, 0, fd, uid, gid);
        }
    }

    // fs.utimes()
    NCJS_OBJECT_FUNCTION(UTimes)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        const size_t size = args.size();
        if (size < 1)
            return TYPE_ERROR("path required");
        if (size < 2)
            return TYPE_ERROR("atime required");
        if (size < 3)
            return TYPE_ERROR("mtime required");
        if (!args[0]->IsString())
            return TYPE_ERROR("path must be a string");
        if (!args[1]->IsDouble())
            return TYPE_ERROR("atime must be a number");
        if (!args[2]->IsDouble())
            return TYPE_ERROR("mtime must be a number");

        const AutoString path(args[0]->GetStringValue().ToString());
        const double atime = args[1]->GetDoubleValue();
        const double mtime = args[2]->GetDoubleValue();

        if (NCJS_ARG_IS(Object, args, 3)) {
            ASYNC_CALL(utime, args[3], path, atime, mtime);
        } else {
            SYNC_CALL(utime, path, path, atime, mtime);
        }
    }

    // fs.futimes()
    NCJS_OBJECT_FUNCTION(FUTimes)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        const size_t size = args.size();
        if (size < 1)
            return TYPE_ERROR("fd required");
        if (size < 2)
            return TYPE_ERROR("atime required");
        if (size < 3)
            return TYPE_ERROR("mtime required");
        if (!args[0]->IsString())
            return TYPE_ERROR("fd must be a file descriptor");
        if (!args[1]->IsDouble())
            return TYPE_ERROR("atime must be a number");
        if (!args[2]->IsDouble())
            return TYPE_ERROR("mtime must be a number");

        const int fd = args[0]->GetIntValue();
        const double atime = args[1]->GetDoubleValue();
        const double mtime = args[2]->GetDoubleValue();

        if (NCJS_ARG_IS(Object, args, 3)) {
            ASYNC_CALL(futime, args[3], fd, atime, mtime);
        } else {
            SYNC_CALL(futime, 0, fd, atime, mtime);
        }
    }

    // fs.writeBuffer()
    NCJS_OBJECT_FUNCTION(WriteBuffer)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        const size_t size = args.size(); NCJS_CHK_GE(size, 2);

        if (!args[0]->IsInt())
            return TYPE_ERROR("first argument must be file descriptor");

        const int fd = args[0]->GetIntValue();
        const unsigned off = size > 2 ? args[2]->GetUIntValue() : 0;
        const unsigned len = size > 3 ? args[3]->GetUIntValue() : 0;
        const int64_t pos = size > 4 ? GET_OFFSET(args[4]) : -1;

        Buffer* buf = Buffer::Unwrap(args[1]); NCJS_CHECK(buf);

        if (off > buf->Size())
            return RANGE_ERROR("offset out of bounds");
        if (len > buf->Size())
            return RANGE_ERROR("length out of bounds");
        if (off + len < off)
            return RANGE_ERROR("off + len overflow");
        if (!Buffer::IsWithinBounds(off, len, buf->Size()))
            return RANGE_ERROR("off + len > buffer.length");

        CefRefPtr<AutoUvBuffer> uvbuf(new AutoUvBuffer(buf, off, len));

        if (NCJS_ARG_IS(Object, args, 5)) {
            ASYNC_CALL(write, args[5], fd, uvbuf, 1, pos);
            ASYNC_HOLD_DATA(buf);
        } else {
            SYNC_CALL(write, NULL, fd, uvbuf, 1, pos);

            retval = CefV8Value::CreateInt(SYNC_RESULT);
        }
    }

    // fs.writeBuffers()
    NCJS_OBJECT_FUNCTION(WriteBuffers)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        const size_t size = args.size();

        NCJS_CHK_GE(size, 2);
        NCJS_CHECK(args[0]->IsInt());
        NCJS_CHECK(args[1]->IsArray());

        class BufferHolder : public CefBase {
        public:
            std::vector<uv_buf_t> bufs;
            std::vector< CefRefPtr<Buffer> > data;

            BufferHolder(size_t size) : bufs(size), data(size) {}

            IMPLEMENT_REFCOUNTING(BufferHolder);
        };

        const int fd = args[0]->GetIntValue();
        const int64_t pos = size > 2 ? GET_OFFSET(args[2]) : -1;

        CefRefPtr<CefV8Value> chunks = args[1];
        const unsigned nChunk = chunks->GetArrayLength();
        CefRefPtr<BufferHolder> hold(new BufferHolder(nChunk));

        for (unsigned i = 0; i < nChunk; ++i) {
            if (Buffer* buf = Buffer::Unwrap(chunks->GetValue(i))) {
                hold->data[i] = buf;
                hold->bufs[i] = uv_buf_init(buf->Data(), unsigned(buf->Size()));
            } else { return TYPE_ERROR("array elements all need to be buffers"); }
        }

        if (NCJS_ARG_IS(Object, args, 3)) {
            ASYNC_CALL(write, args[3], fd, &hold->bufs[0], nChunk, pos);
            ASYNC_HOLD_DATA(hold);
        } else {
            SYNC_CALL(write, NULL, fd, &hold->bufs[0], nChunk, pos);

            retval = CefV8Value::CreateInt(SYNC_RESULT);
        }
    }

    // fs.writeString()
    NCJS_OBJECT_FUNCTION(WriteString)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        const size_t size = args.size(); NCJS_CHK_GE(size, 2);

        if (!args[0]->IsInt())
            return TYPE_ERROR("first argument must be file descriptor");

        const int fd = args[0]->GetIntValue();
        const CefString str = args[1]->GetStringValue();
        const int64_t   pos = size > 2 ? GET_OFFSET(args[2]) : -1;
        const CefString enc = size > 3 ? args[3]->GetStringValue() : CefString();

        CefRefPtr<Buffer> buf = Buffer::Create(str, enc); NCJS_CHECK(buf);

        CefRefPtr<AutoUvBuffer> uvbuf(new AutoUvBuffer(buf, 0, unsigned(buf->Size())));

        if (NCJS_ARG_IS(Object, args, 4)) {
            ASYNC_CALL(write, args[4], fd, uvbuf, 1, pos);
        } else {
            SYNC_CALL(write, NULL, fd, uvbuf, 1, pos);

            retval = CefV8Value::CreateInt(SYNC_RESULT);
        }
    }

    // fs.FSInitialize()
    NCJS_OBJECT_FUNCTION(FSInitialize)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        NCJS_CHECK(NCJS_ARG_IS(Function, args, 0));

        CefRefPtr<Environment> env = Environment::Get(CefV8Context::GetCurrentContext());

        env->GetFunction().ctor_fs_stats = args[0];
    }

    // object factory

    NCJS_BEGIN_OBJECT_FACTORY()
        // objects
        NCJS_MAP_OBJECT_FACTORY("StatWatcher", StatWatcher)
        NCJS_MAP_OBJECT_FACTORY("FSReqWrap",   FSReqWrap)

        // functions
        NCJS_MAP_OBJECT_FUNCTION("FSInitialize", FSInitialize)

        NCJS_MAP_OBJECT_FUNCTION("internalModuleReadFile", InternalModuleReadFile)
        NCJS_MAP_OBJECT_FUNCTION("internalModuleStat",     InternalModuleStat)
        NCJS_MAP_OBJECT_FUNCTION("access",       Access)
        NCJS_MAP_OBJECT_FUNCTION("close",        Close)
        NCJS_MAP_OBJECT_FUNCTION("open",         Open)
        NCJS_MAP_OBJECT_FUNCTION("read",         Read)
        NCJS_MAP_OBJECT_FUNCTION("fdatasync",    FDataSync)
        NCJS_MAP_OBJECT_FUNCTION("fsync",        FSync)
        NCJS_MAP_OBJECT_FUNCTION("ftruncate",    FTruncate)
        NCJS_MAP_OBJECT_FUNCTION("rename",       Rename)
        NCJS_MAP_OBJECT_FUNCTION("rmdir",        RMDir)
        NCJS_MAP_OBJECT_FUNCTION("mkdir",        MKDir)
        NCJS_MAP_OBJECT_FUNCTION("readdir",      ReadDir)
        NCJS_MAP_OBJECT_FUNCTION("stat",         Stat)
        NCJS_MAP_OBJECT_FUNCTION("lstat",        LStat)
        NCJS_MAP_OBJECT_FUNCTION("fstat",        FStat)
        NCJS_MAP_OBJECT_FUNCTION("link",         Link)
        NCJS_MAP_OBJECT_FUNCTION("symlink",      SymLink)
        NCJS_MAP_OBJECT_FUNCTION("readlink",     ReadLink)
        NCJS_MAP_OBJECT_FUNCTION("unlink",       Unlink)
        NCJS_MAP_OBJECT_FUNCTION("chmod",        Chmod)
        NCJS_MAP_OBJECT_FUNCTION("fchmod",       FChmod)
        NCJS_MAP_OBJECT_FUNCTION("chown",        Chown)
        NCJS_MAP_OBJECT_FUNCTION("fchown",       FChown)
        NCJS_MAP_OBJECT_FUNCTION("utimes",       UTimes)
        NCJS_MAP_OBJECT_FUNCTION("futimes",      FUTimes)
        NCJS_MAP_OBJECT_FUNCTION("writeBuffer",  WriteBuffer)
        NCJS_MAP_OBJECT_FUNCTION("writeBuffers", WriteBuffers)
        NCJS_MAP_OBJECT_FUNCTION("writeString",  WriteString)
    NCJS_END_OBJECT_FACTORY()

};

/// ----------------------------------------------------------------------------
/// define module
/// ----------------------------------------------------------------------------

NCJS_DEFINE_BUILTIN_MODULE(fs, ModuleFS);

} // ncjs