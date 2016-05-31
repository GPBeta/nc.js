
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

#define ASYNC_CALL(_FUNCTION, _REQ, ...) \
    except = consts::str_err_notimpl; return;

#define SYNC_DEST_CALL(_FUNCTION, _PATH, _DEST, ...) \
    SyncReqWrap req_wrap; \
    int err = uv_fs_##_FUNCTION(Environment::GetEventLoop(), \
                               &req_wrap.req, __VA_ARGS__, NULL); \
    if (err < 0) \
        return Environment::UvException(err, #_FUNCTION, NULL, except);

#define SYNC_CALL(_FUNCTION, _PATH, ...) \
    SYNC_DEST_CALL(_FUNCTION, _PATH, NULL, __VA_ARGS__)

#define SYNC_REQ req_wrap.req

#define SYNC_RESULT err

#define TYPE_ERROR(_MSG) Environment::TypeException(NCJS_TEXT(_MSG), except)
#define RANGE_ERROR(_MSG) TYPE_ERROR(_MSG)

/// ----------------------------------------------------------------------------
/// headers
/// ----------------------------------------------------------------------------

#include "ncjs/module.h"
#include "ncjs/constants.h"
#include "ncjs/module/buffer.h"

#include <uv.h>

#include <fcntl.h>

namespace ncjs {

/// ----------------------------------------------------------------------------
/// variables
/// ----------------------------------------------------------------------------

struct SyncReqWrap {
    uv_fs_t req;

    SyncReqWrap() {}
    ~SyncReqWrap() { uv_fs_req_cleanup(&req); }

    DISALLOW_COPY_AND_ASSIGN(SyncReqWrap);
};

/// ============================================================================
/// implementation
/// ============================================================================

CefRefPtr<CefV8Value> BuildStatsObject(CefRefPtr<Environment> env, const uv_stat_t* s) {
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
    return env->New(env->GetFunction().ctor_fs_stats, argv);
}

/// ----------------------------------------------------------------------------
/// ModuleFS
/// ----------------------------------------------------------------------------

class ModuleFS : public JsObjecT<ModuleFS> {

    // fs.internalModuleReadFile()
    NCJS_OBJECT_FUNCTION(InternalModuleReadFile)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        uv_loop_t* loop = Environment::GetEventLoop();

        NCJS_CHECK(NCJS_ARG_IS(String, args, 0));
        const std::string path(args[0]->GetStringValue().ToString());

        uv_fs_t reqOpen;
        const int fd = uv_fs_open(loop, &reqOpen, path.c_str(), O_RDONLY, 0, NULL);
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
        uv_loop_t* loop = Environment::GetEventLoop();

        NCJS_CHECK(NCJS_ARG_IS(String, args, 0));

        const std::string path(args[0]->GetStringValue().ToString());

        uv_fs_t req;
        int rc = uv_fs_stat(loop, &req, path.c_str(), NULL);
        if (rc == 0) {
            const uv_stat_t* const s = static_cast<const uv_stat_t*>(req.ptr);
            rc = !!(s->st_mode & S_IFDIR);
        }
        uv_fs_req_cleanup(&req);

        retval = CefV8Value::CreateInt(rc);
    }

    // fs.stat()
    NCJS_OBJECT_FUNCTION(Stat)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except) {
        CefRefPtr<Environment> env = Environment::Get(CefV8Context::GetCurrentContext());

        if (args.size() < 1)
            return TYPE_ERROR("path required");
        if (!args[0]->IsString())
            return TYPE_ERROR("path must be a string");

        const std::string path(args[0]->GetStringValue().ToString());

        if (NCJS_ARG_IS(Object, args, 1)) {
            ASYNC_CALL(stat, args[1], path.c_str())
        } else {
            SYNC_CALL(stat, path.c_str(), path.c_str())
            retval = BuildStatsObject(env, static_cast<const uv_stat_t*>(SYNC_REQ.ptr));
        }
    }

    // fs.lstat()
    NCJS_OBJECT_FUNCTION(LStat)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except) {
        CefRefPtr<Environment> env = Environment::Get(CefV8Context::GetCurrentContext());

        if (args.size() < 1)
            return TYPE_ERROR("path required");
        if (!args[0]->IsString())
            return TYPE_ERROR("path must be a string");

        const std::string path(args[0]->GetStringValue().ToString());

        if (NCJS_ARG_IS(Object, args, 1)) {
             ASYNC_CALL(lstat, args[1], path)
        } else {
             SYNC_CALL(lstat, path.c_str(), path.c_str())
             retval = BuildStatsObject(env, static_cast<const uv_stat_t*>(SYNC_REQ.ptr));
        }
    }

    // fs.fstat()
    NCJS_OBJECT_FUNCTION(FStat)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except) {
        CefRefPtr<Environment> env = Environment::Get(CefV8Context::GetCurrentContext());

        if (args.size() < 1)
            return TYPE_ERROR("fd is required");
        if (!args[0]->IsInt())
            return TYPE_ERROR("fd must be a file descriptor");

        int fd = args[0]->GetIntValue();

        if (NCJS_ARG_IS(Object, args, 1)) {
            ASYNC_CALL(fstat, args[1], fd)
        } else {
            SYNC_CALL(fstat, 0, fd)
            retval = BuildStatsObject(env, static_cast<const uv_stat_t*>(SYNC_REQ.ptr));
        }
    }


    /*
     * Wrapper for read(2).
     *
     * bytesRead = fs.read(fd, buffer, offset, length, position)
     *
     * 0 fd        integer. file descriptor
     * 1 buffer    instance of Buffer
     * 2 offset    integer. offset to start reading into inside buffer
     * 3 length    integer. length to read
     * 4 position  file position - null for current position
     *
     */
    NCJS_OBJECT_FUNCTION(Read)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except) {
        if (args.size() < 2)
            return TYPE_ERROR("fd and buffer are required");
        if (!args[0]->IsInt())
            return TYPE_ERROR("fd must be a file descriptor");
        if (!Buffer::HasInstance(args[1]))
            return TYPE_ERROR("Second argument needs to be a buffer");

        int fd = args[0]->GetIntValue();

        int64_t pos = -1;
        unsigned len = 0;
        unsigned off = 0;
        char* buf = NULL;

        CefRefPtr<Buffer> buffer = Buffer::Get(args[1]);

        if (NCJS_ARG_IS(Int, args, 2))
            off = args[2]->GetUIntValue();
        if (off >= buffer->Size())
            return RANGE_ERROR("Offset is out of bounds");

        if (NCJS_ARG_IS(Int, args, 3))
            len = args[3]->GetUIntValue();
        if (!Buffer::IsWithinBounds(off, len, buffer->Size()))
            return RANGE_ERROR("Length extends beyond buffer");

        if (NCJS_ARG_IS(Int, args, 4))
            pos = args[4]->GetUIntValue();

        buf = buffer->Data() + off;

        uv_buf_t uvbuf = uv_buf_init(const_cast<char*>(buf), len);

        if (NCJS_ARG_IS(Object, args, 5)) {
            ASYNC_CALL(read, args[5], fd, &uvbuf, 1, pos);
        } else {
            SYNC_CALL(read, 0, fd, &uvbuf, 1, pos)
            retval = CefV8Value::CreateInt(SYNC_RESULT);
        }
    }

    // fs.open()
    NCJS_OBJECT_FUNCTION(Open)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except) {

        size_t len = args.size();
        if (len < 1)
            return TYPE_ERROR("path required");
        if (len < 2)
            return TYPE_ERROR("flags required");
        if (len < 3)
            return TYPE_ERROR("mode required");
        if (!args[0]->IsString())
            return TYPE_ERROR("path must be a string");
        if (!args[1]->IsInt())
            return TYPE_ERROR("flags must be an int");
        if (!args[2]->IsInt())
            return TYPE_ERROR("mode must be an int");

        const std::string path(args[0]->GetStringValue().ToString());
        int flags = args[1]->GetIntValue();
        int mode = args[2]->GetIntValue();

        if (NCJS_ARG_IS(Object, args, 3)) {
            ASYNC_CALL(open, args[3], path.c_str(), flags, mode)
        } else {
            SYNC_CALL(open, path.c_str(), path.c_str(), flags, mode)
            retval = CefV8Value::CreateInt(SYNC_RESULT);
        }
    }

    // fs.close()
    NCJS_OBJECT_FUNCTION(Close)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except) {

        if (args.size() < 1)
            return TYPE_ERROR("fd is required");
        if (!args[0]->IsInt())
            return TYPE_ERROR("fd must be a file descriptor");

        int fd = args[0]->GetIntValue();

        if (NCJS_ARG_IS(Object, args, 1)) {
            ASYNC_CALL(close, args[1], fd)
        } else {
            SYNC_CALL(close, 0, fd)
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
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("FSInitialize"), FSInitialize)

        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("internalModuleReadFile"), InternalModuleReadFile)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("internalModuleStat"), InternalModuleStat)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("stat"), Stat)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("lstat"), LStat)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("fstat"), FStat)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("read"), Read)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("open"), Open)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("close"), Close)
    NCJS_END_OBJECT_FACTORY()

};

/// ----------------------------------------------------------------------------
/// define module
/// ----------------------------------------------------------------------------

NCJS_DEFINE_BUILTIN_MODULE(fs, ModuleFS);

} // ncjs