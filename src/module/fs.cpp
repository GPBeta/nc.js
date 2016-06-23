
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
    except = consts::str_err_notimpl; return;

#define ASYNC_CALL(_FUNCTION, _REQ, ...) \
    ASYNC_DEST_CALL(_FUNCTION, _REQ, NULL, __VA_ARGS__)

#define SYNC_DEST_CALL(_FUNCTION, _PATH, _DEST, ...) \
    SyncReqWrap req_wrap; \
    int err = uv_fs_##_FUNCTION(Environment::GetEventLoop(), \
                               &req_wrap.req, __VA_ARGS__, NULL); \
    if (err < 0) \
        return Environment::UvException(err, #_FUNCTION, NULL, _PATH, _DEST, except);

#define SYNC_CALL(_FUNCTION, _PATH, ...) \
    SYNC_DEST_CALL(_FUNCTION, _PATH, NULL, __VA_ARGS__)

#define SYNC_REQ req_wrap.req

#define SYNC_RESULT err

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
    const std::string _PATH(_ARGS[0]->GetStringValue().ToString())

#define GET_PARAM_PATH_MODE(_ARGS, _PATH, _MODE) \
    if (_ARGS.size() < 2) \
        return TYPE_ERROR("path and mode are required"); \
    if (!_ARGS[0]->IsString()) \
        return TYPE_ERROR("path must be a string"); \
    if (!_ARGS[1]->IsInt()) \
        return TYPE_ERROR("mode must be an integer"); \
    const std::string _PATH(_ARGS[0]->GetStringValue().ToString()); \
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
    const std::string _SRC(_ARGS[0]->GetStringValue().ToString()); \
    const std::string _DST(_ARGS[1]->GetStringValue().ToString())

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
/// StatWatcherPrototype
/// ----------------------------------------------------------------------------

class StatWatcher : public JsObjecT<StatWatcher> {

    // StatWatcher.start()
    NCJS_OBJECT_FUNCTION(Start)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        except = consts::str_err_notimpl;
    }

   // StatWatcher.stop()
    NCJS_OBJECT_FUNCTION(Stop)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        except = consts::str_err_notimpl;
    }

    // StatWatcher()
    NCJS_OBJECT_FUNCTION(Constructor)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        except = consts::str_err_notimpl;
    }

    // class factory

    NCJS_BEGIN_CLASS_FACTORY(Constructor)
        NCJS_MAP_OBJECT_FUNCTION("start", Start)
        NCJS_MAP_OBJECT_FUNCTION("stop", Stop)
    NCJS_END_CLASS_FACTORY()
};

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

    // fs.access()
    NCJS_OBJECT_FUNCTION(Access)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        GET_PARAM_PATH_MODE(args, path, mode);

        if (NCJS_ARG_IS(Object, args, 2)) {
            ASYNC_CALL(access, args[2], path.c_str(), mode);
        } else {
            SYNC_CALL(access, path.c_str(), path.c_str(), mode);
        }
    }

    // fs.close()
    NCJS_OBJECT_FUNCTION(Close)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        GET_PARAM_FD(args, fd);

        if (NCJS_ARG_IS(Object, args, 1)) {
            ASYNC_CALL(close, args[1], fd)
        } else {
            SYNC_CALL(close, 0, fd)
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

        const std::string path(args[0]->GetStringValue().ToString());
        const int flags = args[1]->GetIntValue();
        const int mode = args[2]->GetIntValue();

        if (NCJS_ARG_IS(Object, args, 3)) {
            ASYNC_CALL(open, args[3], path.c_str(), flags, mode)
        } else {
            SYNC_CALL(open, path.c_str(), path.c_str(), flags, mode)
            retval = CefV8Value::CreateInt(SYNC_RESULT);
        }
    }

    // fs.read()
    NCJS_OBJECT_FUNCTION(Read)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        Buffer* buffer = NULL;

        if (args.size() < 2)
            return TYPE_ERROR("fd and buffer are required");
        if (!args[0]->IsInt())
            return TYPE_ERROR("fd must be a file descriptor");
        if (!(buffer = Buffer::Get(args[1])))
            return TYPE_ERROR("Second argument needs to be a buffer");

        const int fd = args[0]->GetIntValue();

        int64_t pos = -1;
        unsigned len = 0;
        unsigned off = 0;
        char* buf = NULL;


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

    // fs.fdatasync()
    NCJS_OBJECT_FUNCTION(FDataSync)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        GET_PARAM_FD(args, fd);

        if (NCJS_ARG_IS(Object, args, 1)) {
            ASYNC_CALL(fdatasync, args[1], fd)
        } else {
            SYNC_CALL(fdatasync, 0, fd)
        }
    }

    // fs.fsync()
    NCJS_OBJECT_FUNCTION(FSync)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        GET_PARAM_FD(args, fd);

        if (NCJS_ARG_IS(Object, args, 1)) {
            ASYNC_CALL(fsync, args[1], fd)
        } else {
            SYNC_CALL(fsync, 0, fd)
        }
    }

    // fs.ftruncate()
    NCJS_OBJECT_FUNCTION(FTruncate)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        GET_PARAM_FD_INT(args, fd, length);

        if (NCJS_ARG_IS(Object, args, 2)) {
            ASYNC_CALL(ftruncate, args[2], fd, length)
        } else {
            SYNC_CALL(ftruncate, 0, fd, length)
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
            ASYNC_DEST_CALL(rename, args[2], pathNew, pathOld, pathNew)
        } else {
            SYNC_DEST_CALL(rename, pathOld, pathNew, pathOld, pathNew)
        }
    }

    // fs.rmdir()
    NCJS_OBJECT_FUNCTION(RMDir)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        GET_PARAM_PATH(args, path);

        if (NCJS_ARG_IS(Object, args, 1)) {
            ASYNC_CALL(rmdir, args[1], path.c_str())
        } else {
            SYNC_CALL(rmdir, path.c_str(), path.c_str())
        }
    }

    // fs.mkdir()
    NCJS_OBJECT_FUNCTION(MKDir)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        GET_PARAM_PATH_MODE(args, path, mode);

        if (NCJS_ARG_IS(Object, args, 2)) {
            ASYNC_CALL(mkdir, args[2], path.c_str(), mode)
        } else {
            SYNC_CALL(mkdir, path.c_str(), path.c_str(), mode)
        }
    }

    // fs.readdir()
    NCJS_OBJECT_FUNCTION(ReadDir)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        GET_PARAM_PATH(args, path);

        if (NCJS_ARG_IS(Object, args, 1)) {
            ASYNC_CALL(scandir, args[1], path.c_str(), 0)
        } else {
            SYNC_CALL(scandir, path.c_str(), path.c_str(), 0)

            NCJS_CHK_GE(SYNC_REQ.result, 0);

            CefRefPtr<CefV8Value> names = CefV8Value::CreateArray(0);

            for (unsigned i = 0; ; ++i) {
                uv_dirent_t ent;
                const int res = uv_fs_scandir_next(&SYNC_REQ, &ent);

                if (res == UV_EOF)
                    break;
                if (res)
                    return Environment::UvException(res, "readdir", NULL,
                                                    path.c_str(), NULL, except);

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

        CefRefPtr<Environment> env = Environment::Get(CefV8Context::GetCurrentContext());

        if (NCJS_ARG_IS(Object, args, 1)) {
            ASYNC_CALL(stat, args[1], path.c_str())
        } else {
            SYNC_CALL(stat, path.c_str(), path.c_str())
            retval = BuildStatsObject(env, static_cast<const uv_stat_t*>(SYNC_REQ.ptr));
        }
    }

    // fs.lstat()
    NCJS_OBJECT_FUNCTION(LStat)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        GET_PARAM_PATH(args, path);

        CefRefPtr<Environment> env = Environment::Get(CefV8Context::GetCurrentContext());

        if (NCJS_ARG_IS(Object, args, 1)) {
             ASYNC_CALL(lstat, args[1], path)
        } else {
             SYNC_CALL(lstat, path.c_str(), path.c_str())
             retval = BuildStatsObject(env, static_cast<const uv_stat_t*>(SYNC_REQ.ptr));
        }
    }

    // fs.fstat()
    NCJS_OBJECT_FUNCTION(FStat)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        GET_PARAM_FD(args, fd);

        CefRefPtr<Environment> env = Environment::Get(CefV8Context::GetCurrentContext());

        if (NCJS_ARG_IS(Object, args, 1)) {
            ASYNC_CALL(fstat, args[1], fd)
        } else {
            SYNC_CALL(fstat, 0, fd)
            retval = BuildStatsObject(env, static_cast<const uv_stat_t*>(SYNC_REQ.ptr));
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
            ASYNC_DEST_CALL(link, args[2], pathDst, pathSrc, pathDst)
        } else {
            SYNC_DEST_CALL(link, pathSrc, pathDst, pathSrc, pathDst)
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
            ASYNC_DEST_CALL(symlink, args[3], path, target, path, flags)
        } else {
            SYNC_DEST_CALL(symlink, target, path, target, path, flags)
        }
    }

    // fs.readlink()
    NCJS_OBJECT_FUNCTION(ReadLink)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        GET_PARAM_PATH(args, path);

        if (NCJS_ARG_IS(Object, args, 1)) {
            ASYNC_CALL(readlink, args[1], path.c_str())
        } else {
            SYNC_CALL(readlink, path.c_str(), path.c_str())

            retval = CefV8Value::CreateString(static_cast<const char*>(SYNC_REQ.ptr));
        }
    }

    // fs.unlink()
    NCJS_OBJECT_FUNCTION(Unlink)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        GET_PARAM_PATH(args, path);

        if (NCJS_ARG_IS(Object, args, 1)) {
            ASYNC_CALL(unlink, args[1], path.c_str())
        } else {
            SYNC_CALL(unlink, path.c_str(), path.c_str())
        }
    }

    // fs.chmod()
    NCJS_OBJECT_FUNCTION(Chmod)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        GET_PARAM_PATH_MODE(args, path, mode);

        if (NCJS_ARG_IS(Object, args, 2)) {
            ASYNC_CALL(chmod, args[2], path.c_str(), mode);
        } else {
            SYNC_CALL(chmod, path.c_str(), path.c_str(), mode);
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

        const std::string path(args[0]->GetStringValue().ToString());
        const uv_uid_t uid = args[1]->GetUIntValue();
        const uv_gid_t gid = args[2]->GetUIntValue();

        if (NCJS_ARG_IS(Object, args, 3)) {
            ASYNC_CALL(chown, args[3], path, uid, gid);
        } else {
            SYNC_CALL(chown, path.c_str(), path.c_str(), uid, gid);
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

        const std::string path(args[0]->GetStringValue().ToString());
        const double atime = args[1]->GetDoubleValue();
        const double mtime = args[2]->GetDoubleValue();

        if (NCJS_ARG_IS(Object, args, 3)) {
            ASYNC_CALL(utime, args[3], path, atime, mtime);
        } else {
            SYNC_CALL(utime, path.c_str(), path.c_str(), atime, mtime);
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

        Buffer* buf = Buffer::Get(args[1]); NCJS_CHECK(buf);

        if (off > buf->Size())
            return RANGE_ERROR("offset out of bounds");
        if (len > buf->Size())
            return RANGE_ERROR("length out of bounds");
        if (off + len < off)
            return RANGE_ERROR("off + len overflow");
        if (!Buffer::IsWithinBounds(off, len, buf->Size()))
            return RANGE_ERROR("off + len > buffer.length");

        const uv_buf_t uvbuf = uv_buf_init(buf->Data() + off, len);

        if (NCJS_ARG_IS(Object, args, 5)) {
            ASYNC_CALL(write, args[5], fd, &uvbuf, 1, pos)
        } else {
            SYNC_CALL(write, NULL, fd, &uvbuf, 1, pos)

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

        const int fd = args[0]->GetIntValue();
        const int64_t pos = size > 2 ? GET_OFFSET(args[2]) : -1;

        CefRefPtr<CefV8Value> chunks = args[1];
        const unsigned nChunk = chunks->GetArrayLength();
        std::vector<uv_buf_t> bufs(nChunk);

        for (unsigned i = 0; i < nChunk; ++i) {
            if (Buffer* buf = Buffer::Get(chunks->GetValue(i))) {
                bufs[i] = uv_buf_init(buf->Data(), unsigned(buf->Size()));
            } else { return TYPE_ERROR("array elements all need to be buffers"); }
        }

        if (NCJS_ARG_IS(Object, args, 3)) {
            ASYNC_CALL(write, args[3], fd, &bufs[0], nChunk, pos)
        } else {
            SYNC_CALL(write, NULL, fd, &bufs[0], nChunk, pos)

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

        const uv_buf_t uvbuf = uv_buf_init(buf->Data(), unsigned(buf->Size()));

        if (NCJS_ARG_IS(Object, args, 4)) {
            ASYNC_CALL(write, args[4], fd, &uvbuf, 1, pos)
        } else {
            SYNC_CALL(write, NULL, fd, &uvbuf, 1, pos)

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