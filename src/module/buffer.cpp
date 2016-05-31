
/***************************************************************
 * Name:      buffer.cpp
 * Purpose:   Code for Node-CEF Buffer Module
 * Author:    Joshua GPBeta (studiocghibli@gmail.com)
 * Created:   2016-05-27
 * Copyright: Studio GPBeta (www.gpbeta.com)
 * License:
 **************************************************************/

/// ============================================================================
/// declarations
/// ============================================================================

#define CALLER_ERROR Environment::ErrorException(NCJS_TEXT("Caller must be a buffer"), except)

/// ----------------------------------------------------------------------------
/// headers
/// ----------------------------------------------------------------------------

#include "ncjs/module/buffer.h"

#include "ncjs/module.h"
#include "ncjs/constants.h"

#include <signal.h>

namespace ncjs {

enum Encoding { ASCII, UTF8, BASE64, UCS2, BINARY, HEX, BUFFER };

/// ----------------------------------------------------------------------------
/// variables
/// ----------------------------------------------------------------------------

static const unsigned int MAX_LENGTH =
    sizeof(int32_t) == sizeof(intptr_t) ? 0x3fffffff : 0x7fffffff;

static const int STRING_MAX_LENGTH = (1 << 28) - 16;

/// ============================================================================
/// implementation
/// ============================================================================

inline Buffer* Buffer::Create(CefRefPtr<Environment> env, size_t size)
{
    Environment::BufferObjectInfo& info = env->GetBufferObjectInfo();

    void* buffer = info.NoZeroFill() ? malloc(size) : calloc(size, 1);
    info.ResetFillFlag();

    return new Buffer(static_cast<char*>(buffer), size);
}

inline Buffer* Buffer::CreateUninitialized(size_t size)
{
    return new Buffer(static_cast<char*>(malloc(size)), size);
}

class FormatSliceParam {
public:

    FormatSliceParam(const CefRefPtr<CefV8Value>& object,
                     const CefV8ValueList& args, CefString& except) :
        buf(NULL), pos(0), len(0)
    {
        if (!Buffer::HasInstance(object)) {
            CALLER_ERROR;
            return;
        }

        buf = Buffer::Get(object);

        assert(buf != NULL);

        len = unsigned(buf->Size());

        if (len == 0)
            return;

        int end = len;
        if (NCJS_ARG_IS(Int, args, 0)) { 
            pos = args[0]->GetIntValue();
            if (pos < 0) {
                pos += len;
                if (pos < 0)
                    pos = 0;
            } else if (unsigned(pos) > len) {
                pos = len;
            }

            if (NCJS_ARG_IS(Int, args, 1)) {
                end = args[1]->GetIntValue();
                if (end < 0)
                    end = -end;
                if (unsigned(end) > len)
                    end = len;
            }
        }

        len = end - pos;

        if (len < 0)
            len = 0;
    }

    Buffer*  buf;
    int      pos;
    unsigned len;
};

template <Encoding E>
static inline void DoSliceT(const FormatSliceParam& slice, CefRefPtr<CefV8Value>& retval);

template <>
static inline void DoSliceT<UTF8>(const FormatSliceParam& slice,
                                  CefRefPtr<CefV8Value>& retval)
{
    const std::string str(slice.buf->Data() + slice.pos, slice.len);
    retval = CefV8Value::CreateString(str);
}

template <Encoding E>
static inline void SliceT(CefRefPtr<CefV8Value> object, const CefV8ValueList& args,
                          CefRefPtr<CefV8Value>& retval, CefString& except)
{
    const FormatSliceParam slice(object, args, except);

    if (!slice.buf)
        return;

    if (slice.len)
        DoSliceT<E>(slice, retval);
}

/// ----------------------------------------------------------------------------
/// accessors
/// ----------------------------------------------------------------------------

class FlagsAccessor : public JsAccessorT<FlagsAccessor> {
    NCJS_ACCESSOR_GETTER(Get)(const CefRefPtr<CefV8Value> object,
                                    CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        CefRefPtr<Environment> env = Environment::Get(CefV8Context::GetCurrentContext());
        Environment::BufferObjectInfo& info = env->GetBufferObjectInfo();

        retval = env->GetArray().array_buffer_flags;

        unsigned nField = info.FieldsCount();
        unsigned* field = info.Fields();
        
        for (unsigned i = 0; i < nField; ++i) 
            retval->SetValue(i, CefV8Value::CreateInt(field[i]));
    }

    NCJS_ACCESSOR_SETTER(Set)(const CefRefPtr<CefV8Value> object,
                              const CefRefPtr<CefV8Value> value, CefString& except)
    {
        CefRefPtr<Environment> env = Environment::Get(CefV8Context::GetCurrentContext());
        Environment::BufferObjectInfo& info = env->GetBufferObjectInfo();

        unsigned nField = info.FieldsCount();
        unsigned* field = info.Fields();
        
        for (unsigned i = 0; i < nField; ++i)
            field[i] = value->GetValue(i)->GetIntValue();
    }
};

/// ----------------------------------------------------------------------------
/// objects
/// ----------------------------------------------------------------------------

class BufferPrototype : public JsObjecT<BufferPrototype> {

    // buffer.asciiSlice()
    NCJS_OBJECT_FUNCTION(AsciiSlice)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        except = consts::str_err_notimpl;
    }
    // buffer.base64Slice()
    NCJS_OBJECT_FUNCTION(Base64Slice)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        except = consts::str_err_notimpl;
    }
    // buffer.binarySlice()
    NCJS_OBJECT_FUNCTION(BinarySlice)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        except = consts::str_err_notimpl;
    }
    // buffer.hexSlice()
    NCJS_OBJECT_FUNCTION(HexSlice)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        except = consts::str_err_notimpl;
    }
    // buffer.ucs2Slice()
    NCJS_OBJECT_FUNCTION(Ucs2Slice)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        except = consts::str_err_notimpl;
    }
    // buffer.utf8Slice()
    NCJS_OBJECT_FUNCTION(Utf8Slice)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        // TODO: handling encoding errors
        SliceT<UTF8>(object, args, retval, except);
    }
    // buffer.asciiWrite()
    NCJS_OBJECT_FUNCTION(AsciiWrite)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        except = consts::str_err_notimpl;
    }
    // buffer.base64Write()
    NCJS_OBJECT_FUNCTION(Base64Write)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        except = consts::str_err_notimpl;
    }
    // buffer.binaryWrite()
    NCJS_OBJECT_FUNCTION(BinaryWrite)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        except = consts::str_err_notimpl;
    }
    // buffer.hexWrite()
    NCJS_OBJECT_FUNCTION(HexWrite)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        except = consts::str_err_notimpl;
    }
    // buffer.ucs2Write()
    NCJS_OBJECT_FUNCTION(Ucs2Write)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        except = consts::str_err_notimpl;
    }
    // buffer.utf8Write()
    NCJS_OBJECT_FUNCTION(Utf8Write)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        except = consts::str_err_notimpl;
    }
    // buffer.copy()
    NCJS_OBJECT_FUNCTION(Copy)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        except = consts::str_err_notimpl;
    }

    // object builder

    NCJS_BEGIN_OBJECT_BUILDER()
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("asciiSlice"),  AsciiSlice)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("base64Slice"), Base64Slice)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("binarySlice"), BinarySlice)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("hexSlice"),    HexSlice)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("ucs2Slice"),   Ucs2Slice)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("utf8Slice"),   Utf8Slice)

        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("asciiWrite"),  AsciiWrite)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("base64Write"), Base64Write)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("binaryWrite"), BinaryWrite)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("hexWrite"),    HexWrite)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("ucs2Write"),   Ucs2Write)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("utf8Write"),   Utf8Write)

        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("copy"),        Copy)
    NCJS_END_OBJECT_BUILDER()
};

class BufferObject : public JsObjecT<BufferObject> {

    // createBuffer()
    NCJS_OBJECT_FUNCTION(CreateBuffer)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        CefRefPtr<Environment> env = Environment::Get(CefV8Context::GetCurrentContext());

        NCJS_CHECK(args.size() == 1);
        
        retval = CefV8Value::CreateObject(NULL);
        retval->SetUserData(Buffer::Create(env, args[0]->GetUIntValue()));
    }

    // subArray()
    NCJS_OBJECT_FUNCTION(SubArray)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        const FormatSliceParam slice(object, args, except);

        if (slice.buf == NULL)
            return;

        Buffer* buffer = Buffer::CreateUninitialized(slice.len);
        memcpy(buffer->Data(), slice.buf->Data() + slice.pos, slice.len);

        retval = CefV8Value::CreateObject(NULL);
        retval->SetUserData(buffer);
    }

    // object builder

    NCJS_BEGIN_OBJECT_BUILDER()
        // functions
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("createBuffer"), CreateBuffer)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("subarray"), SubArray)
        // accessors
        NCJS_MAP_OBJECT_ACCESSOR(consts::str_flags, FlagsAccessor)
    NCJS_END_OBJECT_BUILDER()
};

/// ----------------------------------------------------------------------------
/// ModuleBuffer
/// ----------------------------------------------------------------------------

class ModuleBuffer : public JsObjecT<ModuleBuffer> {

    // buffer.createFromString()
    NCJS_OBJECT_FUNCTION(CreateFromString)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        except = consts::str_err_notimpl;
    }

    // buffer.createFromArrayBuffer()
    NCJS_OBJECT_FUNCTION(CreateFromArrayBuffer)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        except = consts::str_err_notimpl;
    }

    // buffer.byteLengthUtf8()
    NCJS_OBJECT_FUNCTION(ByteLengthUtf8)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        except = consts::str_err_notimpl;
    }

    // buffer.compare()
    NCJS_OBJECT_FUNCTION(Compare)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        except = consts::str_err_notimpl;
    }

    // buffer.fill()
    NCJS_OBJECT_FUNCTION(Fill)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        except = consts::str_err_notimpl;
    }

    // buffer.indexOfBuffer()
    NCJS_OBJECT_FUNCTION(IndexOfBuffer)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        except = consts::str_err_notimpl;
    }

    // buffer.indexOfNumber()
    NCJS_OBJECT_FUNCTION(IndexOfNumber)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        except = consts::str_err_notimpl;
    }

    // buffer.indexOfString()
    NCJS_OBJECT_FUNCTION(IndexOfString)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        except = consts::str_err_notimpl;
    }

    // buffer.readDoubleBE()
    NCJS_OBJECT_FUNCTION(ReadDoubleBE)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        except = consts::str_err_notimpl;
    }

    // buffer.readDoubleLE()
    NCJS_OBJECT_FUNCTION(ReadDoubleLE)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        except = consts::str_err_notimpl;
    }

    // buffer.readFloatBE()
    NCJS_OBJECT_FUNCTION(ReadFloatBE)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        except = consts::str_err_notimpl;
    }

    // buffer.readFloatLE()
    NCJS_OBJECT_FUNCTION(ReadFloatLE)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        except = consts::str_err_notimpl;
    }

    // buffer.writeDoubleBE()
    NCJS_OBJECT_FUNCTION(WriteDoubleBE)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        except = consts::str_err_notimpl;
    }

    // buffer.writeDoubleLE()
    NCJS_OBJECT_FUNCTION(WriteDoubleLE)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        except = consts::str_err_notimpl;
    }

    // buffer.writeFloatBE()
    NCJS_OBJECT_FUNCTION(WriteFloatBE)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        except = consts::str_err_notimpl;
    }

    // buffer.writeFloatLE()
    NCJS_OBJECT_FUNCTION(WriteFloatLE)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        except = consts::str_err_notimpl;
    }

    // buffer.setupBufferJS()
    NCJS_OBJECT_FUNCTION(SetupBufferJS)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        CefRefPtr<CefV8Context> context = CefV8Context::GetCurrentContext();
        CefRefPtr<Environment> env = Environment::Get(context);

        NCJS_CHECK(NCJS_ARG_IS(Object, args, 0));

        env->GetObject().ptype_buffer = args[0];
        BufferPrototype::ObjectBuilder(env, context, args[0]);
        
        NCJS_CHECK(NCJS_ARG_IS(Object, args, 1));
        BufferObject::ObjectBuilder(env, context, args[1]);
    }

    // object factory

    NCJS_BEGIN_OBJECT_FACTORY()
        // constants
        NCJS_MAP_OBJECT_READONLY(Int, NCJS_REFTEXT("kMaxLength"),               MAX_LENGTH)
        NCJS_MAP_OBJECT_READONLY(Int, NCJS_REFTEXT("kStringMaxLength"),  STRING_MAX_LENGTH)

        // functions
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("setupBufferJS"), SetupBufferJS)

        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("createFromString"),      CreateFromString)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("createFromArrayBuffer"), CreateFromArrayBuffer)

        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("byteLengthUtf8"), ByteLengthUtf8)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("compare"),        Compare)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("fill"),           Fill)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("indexOfBuffer"),  IndexOfBuffer)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("indexOfNumber"),  IndexOfNumber)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("indexOfString"),  IndexOfString)

        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("readDoubleBE"), ReadDoubleBE)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("readDoubleLE"), ReadDoubleLE)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("readFloatBE"),  ReadFloatBE)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("readFloatLE"),  ReadFloatLE)

        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("writeDoubleBE"), WriteDoubleBE)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("writeDoubleLE"), WriteDoubleLE)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("writeFloatBE"),  WriteFloatBE)
        NCJS_MAP_OBJECT_FUNCTION(NCJS_REFTEXT("writeFloatLE"),  WriteFloatLE)
    NCJS_END_OBJECT_FACTORY()

};

/// ----------------------------------------------------------------------------
/// define module
/// ----------------------------------------------------------------------------

NCJS_DEFINE_BUILTIN_MODULE(buffer, ModuleBuffer);

} // ncjs