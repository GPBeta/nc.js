
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

#define BUFFER_ERROR Environment::ErrorException(NCJS_TEXT("Argument must be a Buffer"), except)
#define STRING_ERROR Environment::TypeException(NCJS_TEXT("Argument must be a string"), except)
#define  INDEX_ERROR Environment::RangeException(NCJS_TEXT("Out of range index"), except)

#define INDEX_PARAM(_VAR, _ARGS, _N, _DEF, _MAX) \
        size_t _VAR = _DEF; \
        if (NCJS_ARG_IS(UInt, _ARGS, _N)) { \
            _VAR = _ARGS[_N]->GetUIntValue(); \
            if (_VAR > _MAX) \
                return INDEX_ERROR; \
        }

/// ----------------------------------------------------------------------------
/// headers
/// ----------------------------------------------------------------------------

#include "ncjs/module/buffer.h"

#include "ncjs/module.h"
#include "ncjs/constants.h"

#include <string_search.h>
#include <include/cef_parser.h>

#include <signal.h>
#include <algorithm>

namespace ncjs {

enum Encoding { ASCII, UTF8, BASE64, UCS2, BINARY, HEX, BUFFER };

/// ----------------------------------------------------------------------------
/// variables
/// ----------------------------------------------------------------------------

static const unsigned int MAX_LENGTH =
    sizeof(int32_t) == sizeof(intptr_t) ? 0x3fffffff : 0x7fffffff;

static const int STRING_MAX_LENGTH = (1 << 28) - 16;

const CefRefPtr<Buffer> Buffer::EMPTY_BUFFER = new Buffer(0, 0);

/// ============================================================================
/// implementation
/// ============================================================================

static inline cef_char_t AsciiLower(cef_char_t c) {
    return (c >= 'A' && c <= 'Z') ? c + 32 : c;
} 

static inline Encoding ParseEncoding(const CefString& str, Encoding def)
{
    if (str.length()) {
        std_string enc(str.c_str(), str.length());
        std::transform(enc.begin(), enc.end(), enc.begin(), AsciiLower);

        if      (!enc.compare(NCJS_TEXT("utf8"))      ||
                 !enc.compare(NCJS_TEXT("utf-8")))    return UTF8;
        else if (!enc.compare(NCJS_TEXT("ascii")))    return ASCII;
        else if (!enc.compare(NCJS_TEXT("base64")))   return BASE64;
        else if (!enc.compare(NCJS_TEXT("ucs2"))      ||
                 !enc.compare(NCJS_TEXT("ucs-2"))     ||
                 !enc.compare(NCJS_TEXT("utf16le"))   ||
                 !enc.compare(NCJS_TEXT("utf-16le"))) return UCS2;
        else if (!enc.compare(NCJS_TEXT("binary"))    ||
                 !enc.compare(NCJS_TEXT("raw"))       ||
                 !enc.compare(NCJS_TEXT("raws")))     return BINARY;
        else if (!enc.compare(NCJS_TEXT("buffer")))   return BUFFER;
        else if (!enc.compare(NCJS_TEXT("hex")))      return HEX;
    }

    return def;
}

template <class T>
static void force_ascii_slow(const char* src, size_t len, T* dst) {
    for (size_t i = 0; i < len; ++i)
        dst[i] = src[i] & 0x7f;
}

#ifdef CEF_STRING_TYPE_UTF8
static void force_ascii(const char* src, size_t len, char* dst) {
    if (len < 16) {
        force_ascii_slow(src, len, dst);
        return;
    }

    const size_t bytes_per_word = sizeof(uintptr_t);
    const size_t align_mask = bytes_per_word - 1;
    const size_t src_unalign = To<uintptr_t>(src) & align_mask;
    const size_t dst_unalign = To<uintptr_t>(dst) & align_mask;

    if (src_unalign > 0) {
        if (src_unalign == dst_unalign) {
            const size_t unalign = bytes_per_word - src_unalign;
            force_ascii_slow(src, unalign, dst);
            src += unalign;
            dst += unalign;
            len -= src_unalign;
        } else {
            force_ascii_slow(src, len, dst);
            return;
        }
    }

#if defined(_WIN64) || defined(_LP64)
    const uintptr_t mask = ~0x8080808080808080ll;
#else
    const uintptr_t mask = ~0x80808080l;
#endif

    const uintptr_t* srcw = To<const uintptr_t*>(src);
          uintptr_t* dstw = To<uintptr_t*>(dst);

    for (size_t i = 0, n = len / bytes_per_word; i < n; ++i)
        dstw[i] = srcw[i] & mask;

    const size_t remainder = len & align_mask;
    if (remainder > 0) {
        const size_t offset = len - remainder;
        force_ascii_slow(src + offset, remainder, dst + offset);
    }
}
#else // UTF16 / UTF32
#define force_ascii force_ascii_slow
#endif // CEF_STRING_TYPE_UTF8

unsigned HEX2BIN(cef_char_t c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'A' && c <= 'F')
    return 10 + (c - 'A');
  if (c >= 'a' && c <= 'f')
    return 10 + (c - 'a');
  return -1;
}

template <Encoding E>
static inline CefRefPtr<CefV8Value> DoSliceT(const char* buf, size_t len);
// allocates memory if buf is NULL, caller's responsibility to free() it
template <Encoding E>
static inline size_t DoWriteT(const CefString& str, size_t len, char*& buf);

template <>
static inline CefRefPtr<CefV8Value> DoSliceT<ASCII>(const char* buf, size_t len)
{
    std::vector<cef_char_t> dst(len);
    force_ascii(buf, len, &dst[0]);

    return CefV8Value::CreateString(CefString(&dst[0], len, false));
}

template <>
static inline size_t DoWriteT<ASCII>(const CefString& str, size_t len, char*& buf)
{
    const size_t strLen = str.length();

    if (strLen < len)
        len = strLen;

    if (buf == NULL) { // auto allocation
        if (!(buf = static_cast<char*>(malloc(len))))
            return 0;
    }

    const cef_char_t* src = str.c_str();
    for (size_t i = 0; i < len; ++i)
        buf[i] = char(src[i]);

    return len;
}

template <>
static inline CefRefPtr<CefV8Value> DoSliceT<BINARY>(const char* buf, size_t len)
{
    const size_t strLen = len / sizeof(cef_char_t);

    const CefString str(To<const cef_char_t*>(buf), strLen, false);

    return CefV8Value::CreateString(str);
}

template <>
static inline size_t DoWriteT<BINARY>(const CefString& str, size_t len, char*& buf)
{
    const size_t strLen = str.length() * sizeof(cef_char_t);

    if (strLen < len)
        len = strLen;

    if (buf == NULL) { // auto allocation
        if (!(buf = static_cast<char*>(malloc(len))))
            return 0;
    }

    const cef_char_t* src = str.c_str();
    for (size_t i = 0, n = len / sizeof(cef_char_t); i < n; ++i)
        To<cef_char_t*>(buf)[i] = src[i];

    return len;
}

template <>
static inline CefRefPtr<CefV8Value> DoSliceT<BASE64>(const char* buf, size_t len)
{
    return CefV8Value::CreateString(CefBase64Encode(buf, len));
}

template <>
static inline size_t DoWriteT<BASE64>(const CefString& str, size_t len, char*& buf)
{
    CefRefPtr<CefBinaryValue> raw = CefBase64Decode(str);

    if (!raw.get())
        return 0;

    const size_t rawLen = raw->GetSize();

    if (rawLen < len)
        len = rawLen;

    if (buf == NULL) { // auto allocation
        if (!(buf = static_cast<char*>(malloc(len))))
            return 0;
    }

    return raw->GetData(buf, len, 0);
}

template <>
static inline CefRefPtr<CefV8Value> DoSliceT<HEX>(const char* buf, size_t len)
{
    const char* BIN2HEX = "0123456789abcdef";
    const size_t strLen = len * 2;

    std::vector<cef_char_t> dst(strLen);
    for (size_t i = 0, k = 0; i < len;) {
        const unsigned char val = buf[i++];
        dst[k++] = BIN2HEX[val >> 4];
        dst[k++] = BIN2HEX[val & 15];
    }

    return CefV8Value::CreateString(CefString(&dst[0], strLen, false));
}

template <>
static inline size_t DoWriteT<HEX>(const CefString& str, size_t len, char*& buf)
{
    const size_t strLen = str.length() / 2;

    if (strLen < len)
        len = strLen;

    if (buf == NULL) { // auto allocation
        if (!(buf = static_cast<char*>(malloc(len))))
            return 0;
    }

    const cef_char_t* src = str.c_str();
    for (size_t i = 0; i < len; ++i) {
        const unsigned hi = HEX2BIN(*src++);
        const unsigned lo = HEX2BIN(*src++);
        if (!(~hi && ~hi))
            return i;
        buf[i] = (hi << 4) | lo;
    }

    return len;
}

template <>
static inline CefRefPtr<CefV8Value> DoSliceT<UCS2>(const char* buf, size_t len)
{    
    const size_t strLen = len / 2;
    const bool aligned = To<size_t>(buf) % 2 == 0;

    if (Environment::IsLE() && aligned) { 
#ifdef CEF_STRING_TYPE_UTF16
        const CefString str(To<const cef_char_t*>(buf), strLen, false);
#else
        const base::string16 str(To<const base::char16*>(buf), strLen);
#endif // CEF_STRING_TYPE_UTF16
        return CefV8Value::CreateString(str);
    }

    // BE -> LE

    base::string16 dst;
    dst.reserve(strLen);

    for (unsigned i = 0; i < len; i += 2)
      dst.push_back(buf[i] | (buf[i + 1] << 8));

    return CefV8Value::CreateString(dst);
}

template <>
static inline size_t DoWriteT<UCS2>(const CefString& str, size_t len, char*& buf)
{
#ifdef CEF_STRING_TYPE_UTF16
        const CefString& cvt = str;
#else
        const base::string16 cvt = str.ToString16();
#endif // CEF_STRING_TYPE_UTF16

    const size_t strLen = cvt.length() * 2;

    if (strLen < len)
        len = strLen;

    if (buf == NULL) { // auto allocation
        if (!(buf = static_cast<char*>(malloc(len))))
            return 0;
    }

    if (Environment::IsLE()) {
        memcpy(buf, cvt.c_str(), len);
    } else { // BE -> LE
        if (To<size_t>(buf) % 2) { // not aligned
            const char* src = To<const char*>(cvt.c_str());
            char* dst = buf;
            for (size_t i = 0, n = len / 2; i < n; ++i, dst += 2) {
                dst[1] = *src++;
                dst[0] = *src++;
            }
        } else {
            const base::char16* src = cvt.c_str();
            for (size_t i = 0, n = len / 2; i < n; ++i)
                To<base::char16*>(buf)[i] = (src[i] << 8) | (src[i] >> 8);
        }
    }

    return len;
}

template <>
static inline CefRefPtr<CefV8Value> DoSliceT<UTF8>(const char* buf, size_t len)
{
#ifdef CEF_STRING_TYPE_UTF8
    const CefString str(buf, len, false);
#else
    const std::string str(buf, len);
#endif // CEF_STRING_TYPE_UTF8
    return CefV8Value::CreateString(str);
}

template <>
static inline size_t DoWriteT<UTF8>(const CefString& str, size_t len, char*& buf)
{
#ifdef CEF_STRING_TYPE_UTF8
    const CefString& cvt = str;
#else
    const std::string cvt = str.ToString();
#endif // CEF_STRING_TYPE_UTF8

    const size_t strLen = cvt.length();

    if (strLen < len)
        len = strLen;

    if (buf == NULL) { // auto allocation
        if (!(buf = static_cast<char*>(malloc(len))))
            return 0;
    }

    memcpy(buf, cvt.c_str(), len);

    return len;
}

template <Encoding E>
static inline void SliceT(CefRefPtr<CefV8Value> object, const CefV8ValueList& args,
                          CefRefPtr<CefV8Value>& retval, CefString& except)
{
    Buffer* buf = Buffer::Get(object);

    if (buf == NULL)
        return BUFFER_ERROR;

    INDEX_PARAM(start, args, 0, 0, buf->Size());
    INDEX_PARAM(end, args, 1, buf->Size(), buf->Size());

    if (start > end)
        return INDEX_ERROR;

    if (const size_t len = end - start)
        retval = DoSliceT<E>(buf->Data() + start, len);
}

template <Encoding E>
static inline void WriteT(CefRefPtr<CefV8Value> object, const CefV8ValueList& args,
                          CefRefPtr<CefV8Value>& retval, CefString& except)
{
    Buffer* buf = Buffer::Get(object);

    if (buf == NULL)
        return BUFFER_ERROR;

    if (!NCJS_ARG_IS(String, args, 0))
        return STRING_ERROR;

    const CefString str = args[0]->GetStringValue();

    if (E == HEX && str.length() % 2 != 0)
        return Environment::TypeException(NCJS_TEXT("Invalid hex string"), except);

    INDEX_PARAM(offset, args, 1, 0, buf->Size());

    size_t len = buf->Size() - offset;
    if (NCJS_ARG_IS(UInt, args, 2)) {
        const unsigned value = args[2]->GetUIntValue();
        if (value < len)
            len = value;
    }

    if (len && str.length()) {
        char* buffer = buf->Data() + offset;
        len = DoWriteT<E>(str, len, buffer);
    }

    retval = CefV8Value::CreateUInt(unsigned(len));
}


template <class T, Environment::Endianness E>
static inline void ReadNumberT(const CefV8ValueList& args,
                               CefRefPtr<CefV8Value>& retval, CefString& except)
{
    NCJS_CHK_GE(args.size(), 2);

    Buffer* buf = Buffer::Get(args[0]);

    if (buf == NULL)
        return BUFFER_ERROR;

    const unsigned offset = args[1]->GetUIntValue();

    NCJS_CHK_LE(offset + sizeof(T), buf->Size());

    T value = *To<T*>(buf->Data() + offset);
    if (E != Environment::GetEndianness())
        ReverseArray(To<char*>(&value), sizeof(T));

    retval = CefV8Value::CreateDouble(value);
}

template <class T, Environment::Endianness E>
static inline void WriteNumberT(const CefV8ValueList& args,
                                CefRefPtr<CefV8Value>& retval, CefString& except)
{
    NCJS_CHK_GE(args.size(), 3);

    Buffer* buf = Buffer::Get(args[0]);

    if (buf == NULL)
        return BUFFER_ERROR;

    const unsigned offset = args[2]->GetUIntValue();

    NCJS_CHK_LE(offset + sizeof(T), buf->Size());

    char* value = buf->Data() + offset;
    *To<T*>(value) = T(args[1]->GetDoubleValue());
    if (E != Environment::GetEndianness())
        ReverseArray(value, sizeof(T));

    retval = CefV8Value::CreateUInt(sizeof(T));
}

inline Buffer* Buffer::SubBuffer(size_t offset, size_t size) const
{
    NCJS_ASSERT(offset + size < m_size);

    return new Buffer(m_buffer + offset, size, this);
}

inline int Buffer::SubSearch(Buffer* sub, size_t offset, bool ucs2) const
{
    NCJS_ASSERT(sub);

    const char* subBuf = sub->Data();
    const size_t subLen = sub->Size();

    if (subLen == 0 || m_size == 0 ||
        m_size < offset || subLen + offset > m_size)
        return -1;

    size_t result = m_size;

    if (ucs2) {
        if (m_size - offset < 2 || subLen < 2)
            return -1;

        result = node::SearchString(
                To<const uint16_t*>(m_buffer), m_size / 2,
                To<const uint16_t*>(subBuf), subLen / 2, offset / 2) * 2;
        // fix result for not aligned buffer
        if (result + 1 == m_size)
            result = m_size;
    } else {
        result = node::SearchString(
                To<const uint8_t*>(m_buffer), m_size,
                To<const uint8_t*>(subBuf), subLen, offset);
    }

    return (result == m_size) ? -1 : int(result);
}

inline void Buffer::Wrap(Buffer* buffer, CefRefPtr<CefV8Value>& value)
{
    if (buffer) {
        value = CefV8Value::CreateObject(NULL);
        value->SetValue(consts::str_length,
                        CefV8Value::CreateUInt(unsigned(buffer->Size())),
                        V8_PROPERTY_ATTRIBUTE_READONLY);
        value->SetUserData(buffer);
    }
}

inline Buffer* Buffer::Create(CefRefPtr<Environment> env, size_t size)
{
    if (size == 0)
        return EMPTY_BUFFER;

    Environment::BufferObjectInfo& info = env->GetBufferObjectInfo();

    void* buffer = info.NoZeroFill() ? malloc(size) : calloc(size, 1);
    info.ResetFillFlag();

    return buffer ? new Buffer(static_cast<char*>(buffer), size) : NULL;
}

inline Buffer* Buffer::Create(size_t size)
{
    if (size == 0)
        return EMPTY_BUFFER;

    void* buffer = static_cast<char*>(malloc(size));
    return buffer ? new Buffer(static_cast<char*>(buffer), size) : NULL;
}

inline Buffer* Buffer::Create(const CefString& str, int encoding)
{
    char* buffer = NULL;
    size_t size = 0;

    switch (encoding) {
        case ASCII:  size = DoWriteT<ASCII> (str, -1, buffer); break;
        case UTF8:   size = DoWriteT<UTF8>  (str, -1, buffer); break;
        case BASE64: size = DoWriteT<BASE64>(str, -1, buffer); break;
        case UCS2:   size = DoWriteT<UCS2>  (str, -1, buffer); break;
        case BINARY: size = DoWriteT<BINARY>(str, -1, buffer); break;
        case HEX:    size = DoWriteT<HEX>   (str, -1, buffer); break;
        default: break;
    }

    return buffer ? new Buffer(buffer, size) : EMPTY_BUFFER;
}

Buffer* Buffer::Create(const CefString& str, const CefString& encoding)
{
    return Create(str, ParseEncoding(encoding, UTF8));
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
        SliceT<ASCII>(object, args, retval, except);
    }
    // buffer.base64Slice()
    NCJS_OBJECT_FUNCTION(Base64Slice)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        SliceT<BASE64>(object, args, retval, except);
    }
    // buffer.binarySlice()
    NCJS_OBJECT_FUNCTION(BinarySlice)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        SliceT<BINARY>(object, args, retval, except);
    }
    // buffer.hexSlice()
    NCJS_OBJECT_FUNCTION(HexSlice)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        SliceT<HEX>(object, args, retval, except);
    }
    // buffer.ucs2Slice()
    NCJS_OBJECT_FUNCTION(Ucs2Slice)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        SliceT<UCS2>(object, args, retval, except);
    }
    // buffer.utf8Slice()
    NCJS_OBJECT_FUNCTION(Utf8Slice)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        SliceT<UTF8>(object, args, retval, except);
    }
    // buffer.asciiWrite()
    NCJS_OBJECT_FUNCTION(AsciiWrite)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        WriteT<ASCII>(object, args, retval, except);
    }
    // buffer.base64Write()
    NCJS_OBJECT_FUNCTION(Base64Write)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        WriteT<BASE64>(object, args, retval, except);
    }
    // buffer.binaryWrite()
    NCJS_OBJECT_FUNCTION(BinaryWrite)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        WriteT<BINARY>(object, args, retval, except);
    }
    // buffer.hexWrite()
    NCJS_OBJECT_FUNCTION(HexWrite)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        WriteT<HEX>(object, args, retval, except);
    }
    // buffer.ucs2Write()
    NCJS_OBJECT_FUNCTION(Ucs2Write)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        WriteT<UCS2>(object, args, retval, except);
    }
    // buffer.utf8Write()
    NCJS_OBJECT_FUNCTION(Utf8Write)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        WriteT<UTF8>(object, args, retval, except);
    }
    // buffer.copy()
    NCJS_OBJECT_FUNCTION(Copy)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        Buffer* src = NULL;
        Buffer* dst = NULL;

        if (!args.size() ||
            !(src = Buffer::Get(object)) || !(dst = Buffer::Get(args[0])))
            return BUFFER_ERROR;

        INDEX_PARAM(dstStart, args, 1, 0, dst->Size());
        INDEX_PARAM(srcStart, args, 2, 0, src->Size());
        INDEX_PARAM(srcEnd,   args, 3, src->Size(), src->Size());

        if (srcStart > srcEnd)
            return INDEX_ERROR;

        const size_t toCopy = Min(Min(srcEnd - srcStart, dst->Size() - dstStart),
                                  src->Size() - srcStart);

        if (toCopy)
            memmove(dst->Data() + dstStart, src->Data() + srcStart, toCopy);

        retval = CefV8Value::CreateUInt(unsigned(toCopy));
    }

    // object builder

    NCJS_BEGIN_OBJECT_BUILDER()
        NCJS_MAP_OBJECT_FUNCTION("asciiSlice",  AsciiSlice)
        NCJS_MAP_OBJECT_FUNCTION("base64Slice", Base64Slice)
        NCJS_MAP_OBJECT_FUNCTION("binarySlice", BinarySlice)
        NCJS_MAP_OBJECT_FUNCTION("hexSlice",    HexSlice)
        NCJS_MAP_OBJECT_FUNCTION("ucs2Slice",   Ucs2Slice)
        NCJS_MAP_OBJECT_FUNCTION("utf8Slice",   Utf8Slice)

        NCJS_MAP_OBJECT_FUNCTION("asciiWrite",  AsciiWrite)
        NCJS_MAP_OBJECT_FUNCTION("base64Write", Base64Write)
        NCJS_MAP_OBJECT_FUNCTION("binaryWrite", BinaryWrite)
        NCJS_MAP_OBJECT_FUNCTION("hexWrite",    HexWrite)
        NCJS_MAP_OBJECT_FUNCTION("ucs2Write",   Ucs2Write)
        NCJS_MAP_OBJECT_FUNCTION("utf8Write",   Utf8Write)

        NCJS_MAP_OBJECT_FUNCTION("copy",        Copy)
    NCJS_END_OBJECT_BUILDER()
};

class BindingObject : public JsObjecT<BindingObject> {

    // createBuffer()
    NCJS_OBJECT_FUNCTION(CreateBuffer)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        CefRefPtr<Environment> env = Environment::Get(CefV8Context::GetCurrentContext());

        NCJS_CHECK(args.size() == 1);

        Buffer::Wrap(Buffer::Create(env, args[0]->GetUIntValue()), retval);
    }

    // subArray()
    NCJS_OBJECT_FUNCTION(SubArray)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        Buffer* buf = Buffer::Get(object);

        if (buf == NULL)
            return BUFFER_ERROR;

        size_t len = buf->Size();
        size_t start = 0;

        if (size_t end = len) {
            if (NCJS_ARG_IS(Int, args, 0))
                start = buf->IndexOffset(args[0]->GetIntValue());
            if (NCJS_ARG_IS(Int, args, 1))
                end = buf->IndexOffset(args[1]->GetIntValue());
            len = end > start ? end - start : 0;
        }

        Buffer::Wrap(buf->SubBuffer(start, len), retval);
    }

    // setAt() no throw
    NCJS_OBJECT_FUNCTION(SetAt)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        if (args.size() < 1)
            return;

        unsigned value = 0;

        if (args.size() >= 2) {
            retval = args[1];
            value = retval->GetUIntValue();
        }

        if (Buffer* buf = Buffer::Get(object)) {
            const unsigned offset = args[0]->GetUIntValue();
            if (offset < buf->Size())
                buf->Data()[offset] = As<char>(value);
        }
    }

    // getAt() no throw
    NCJS_OBJECT_FUNCTION(GetAt)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        if (args.size() < 1)
            return;

        if (Buffer* buf = Buffer::Get(object)) {
            const unsigned offset = args[0]->GetUIntValue();
            if (offset < buf->Size())
                retval = CefV8Value::CreateUInt(As<unsigned char>(buf->Data()[offset]));
        }
    }

    // object builder

    NCJS_BEGIN_OBJECT_BUILDER()
        // functions
        NCJS_MAP_OBJECT_FUNCTION("createBuffer", CreateBuffer)
        NCJS_MAP_OBJECT_FUNCTION("subarray", SubArray)
        NCJS_MAP_OBJECT_FUNCTION("setAt", SetAt)
        NCJS_MAP_OBJECT_FUNCTION("getAt", GetAt)
        // accessors
        NCJS_MAP_OBJECT_ACCESSOR("flags", FlagsAccessor)
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
        NCJS_CHECK(args.size() >= 2);
        NCJS_CHECK(args[0]->IsString());
        NCJS_CHECK(args[1]->IsString());

        const CefString str = args[0]->GetStringValue();
        const CefString enc = args[1]->GetStringValue();

        Buffer::Wrap(Buffer::Create(str, enc), retval);
    }

    // buffer.byteLengthUtf8()
    NCJS_OBJECT_FUNCTION(ByteLengthUtf8)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        NCJS_CHECK(NCJS_ARG_IS(String, args, 0));

#ifdef CEF_STRING_TYPE_UTF8
        const size_t len = args[0]->GetStringValue().length();
#else
        const size_t len = args[0]->GetStringValue().ToString().length();
#endif // CEF_STRING_TYPE_UTF8

        retval = CefV8Value::CreateUInt(unsigned(len));
    }

    // buffer.compare()
    NCJS_OBJECT_FUNCTION(Compare)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        Buffer* aBuf = NULL;
        Buffer* bBuf = NULL;

        if (args.size() < 2 ||
            !(aBuf = Buffer::Get(args[0])) || !(bBuf = Buffer::Get(args[1])))
            return BUFFER_ERROR;

        const size_t len = Min(aBuf->Size(), bBuf->Size());
        int res = len > 0 ? memcmp(aBuf->Data(), bBuf->Data(), len) : 0;

        if (res == 0) {
            if (aBuf->Size() > bBuf->Size())
                res = 1;
            else if (aBuf->Size() < bBuf->Size())
                res = -1;
        } else {
            res = res > 0 ? 1 : -1;
        }

        retval = CefV8Value::CreateInt(res);
    }

    // buffer.fill()
    NCJS_OBJECT_FUNCTION(Fill)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        NCJS_CHECK(args.size() >= 4);

        Buffer* buf = Buffer::Get(args[0]);

        if (buf == NULL)
            return BUFFER_ERROR;

        const size_t start = args[2]->GetUIntValue();
        const size_t end = args[3]->GetUIntValue();
        const size_t len = end - start;

        NCJS_CHK_LE(len + start, buf->Size());

        if (args[1]->IsInt()) {
            const int value = args[1]->GetUIntValue() & 0xFF;
            memset(buf->Data() + start, value, len);
            return;
        } // else string

#ifdef CEF_STRING_TYPE_UTF8
        const CefString str = args[1]->GetStringValue();
#else
        const std::string str = args[1]->GetStringValue().ToString();
#endif // CEF_STRING_TYPE_UTF8

        if (str.length() == 0)
            return;

        memcpy(buf->Data() + start, str.c_str(), Min(str.length(), len));

        if (str.length() >= len)
            return;

        size_t filled = str.length();
        char* src = buf->Data() + start;
        char* dst = src + filled;
        while (filled < len - filled) {
            memcpy(dst, src, filled);
            dst += filled;
            filled *= 2;
        }

        if (filled < len) {
            memcpy(dst, src, len - filled);
            filled = len;
        }
    }

    // buffer.indexOfBuffer()
    NCJS_OBJECT_FUNCTION(IndexOfBuffer)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        NCJS_ASSERT(args.size() >= 3);
        NCJS_ASSERT(args[2]->IsInt());

        Encoding enc = UTF8;
        if (args.size() > 3)
            enc = ParseEncoding(args[3]->GetStringValue(), UTF8);

        Buffer* obj = Buffer::Get(args[0]);
        Buffer* buf = Buffer::Get(args[1]);

        if (obj == NULL || buf == NULL)
            return BUFFER_ERROR;

        const size_t offset = obj->IndexOffset(args[2]->GetIntValue());

        retval = CefV8Value::CreateInt(obj->SubSearch(buf, offset, enc == UCS2));
    }

    // buffer.indexOfNumber()
    NCJS_OBJECT_FUNCTION(IndexOfNumber)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        NCJS_ASSERT(args.size() >= 3);
        NCJS_ASSERT(args[1]->IsUInt());
        NCJS_ASSERT(args[2]->IsInt());

        Buffer* buf = Buffer::Get(args[0]);

        if (buf == NULL)
            return BUFFER_ERROR;

        const unsigned number = args[1]->GetUIntValue();
        const size_t start = buf->IndexOffset(args[2]->GetIntValue());
        const size_t len = buf->Size();

        size_t res = -1;

        if (len && start < len) {
            if (void* ptr = memchr(buf->Data() + start, number, len - start))
                res = static_cast<char*>(ptr) - buf->Data();
        }

        retval = CefV8Value::CreateUInt(int(res));
    }

    // buffer.indexOfString()
    NCJS_OBJECT_FUNCTION(IndexOfString)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        NCJS_ASSERT(args.size() >= 3);
        NCJS_ASSERT(args[1]->IsString());
        NCJS_ASSERT(args[2]->IsInt());

        Encoding enc = UTF8;
        if (args.size() > 3)
            enc = ParseEncoding(args[3]->GetStringValue(), UTF8);

        Buffer* obj = Buffer::Get(args[0]);

        if (obj == NULL)
            return BUFFER_ERROR;

        CefRefPtr<Buffer> buf = Buffer::Create(args[1]->GetStringValue(), enc);

        int res = -1;

        if (buf.get()) {
            const size_t offset = obj->IndexOffset(args[2]->GetIntValue());
            res = obj->SubSearch(buf, offset, enc == UCS2);
        }

        retval = CefV8Value::CreateInt(res);
    }

    // buffer.readDoubleBE()
    NCJS_OBJECT_FUNCTION(ReadDoubleBE)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        ReadNumberT<double, Environment::BIG_ENDIAN>(args, retval, except);
    }

    // buffer.readDoubleLE()
    NCJS_OBJECT_FUNCTION(ReadDoubleLE)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        ReadNumberT<double, Environment::LITTLE_ENDIAN>(args, retval, except);
    }

    // buffer.readFloatBE()
    NCJS_OBJECT_FUNCTION(ReadFloatBE)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        ReadNumberT<float, Environment::BIG_ENDIAN>(args, retval, except);
    }

    // buffer.readFloatLE()
    NCJS_OBJECT_FUNCTION(ReadFloatLE)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        ReadNumberT<float, Environment::LITTLE_ENDIAN>(args, retval, except);
    }

    // buffer.writeDoubleBE()
    NCJS_OBJECT_FUNCTION(WriteDoubleBE)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        WriteNumberT<double, Environment::BIG_ENDIAN>(args, retval, except);
    }

    // buffer.writeDoubleLE()
    NCJS_OBJECT_FUNCTION(WriteDoubleLE)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        WriteNumberT<double, Environment::LITTLE_ENDIAN>(args, retval, except);
    }

    // buffer.writeFloatBE()
    NCJS_OBJECT_FUNCTION(WriteFloatBE)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        WriteNumberT<float, Environment::BIG_ENDIAN>(args, retval, except);
    }

    // buffer.writeFloatLE()
    NCJS_OBJECT_FUNCTION(WriteFloatLE)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        WriteNumberT<float, Environment::LITTLE_ENDIAN>(args, retval, except);
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
        BindingObject::ObjectBuilder(env, context, args[1]);
    }

    // object factory

    NCJS_BEGIN_OBJECT_FACTORY()
        // constants
        NCJS_MAP_OBJECT_READONLY(Int, "kMaxLength",               MAX_LENGTH)
        NCJS_MAP_OBJECT_READONLY(Int, "kStringMaxLength",  STRING_MAX_LENGTH)

        // functions
        NCJS_MAP_OBJECT_FUNCTION("setupBufferJS", SetupBufferJS)

        NCJS_MAP_OBJECT_FUNCTION("createFromString",      CreateFromString)

        NCJS_MAP_OBJECT_FUNCTION("byteLengthUtf8", ByteLengthUtf8)
        NCJS_MAP_OBJECT_FUNCTION("compare",        Compare)
        NCJS_MAP_OBJECT_FUNCTION("fill",           Fill)
        NCJS_MAP_OBJECT_FUNCTION("indexOfBuffer",  IndexOfBuffer)
        NCJS_MAP_OBJECT_FUNCTION("indexOfNumber",  IndexOfNumber)
        NCJS_MAP_OBJECT_FUNCTION("indexOfString",  IndexOfString)

        NCJS_MAP_OBJECT_FUNCTION("readDoubleBE", ReadDoubleBE)
        NCJS_MAP_OBJECT_FUNCTION("readDoubleLE", ReadDoubleLE)
        NCJS_MAP_OBJECT_FUNCTION("readFloatBE",  ReadFloatBE)
        NCJS_MAP_OBJECT_FUNCTION("readFloatLE",  ReadFloatLE)

        NCJS_MAP_OBJECT_FUNCTION("writeDoubleBE", WriteDoubleBE)
        NCJS_MAP_OBJECT_FUNCTION("writeDoubleLE", WriteDoubleLE)
        NCJS_MAP_OBJECT_FUNCTION("writeFloatBE",  WriteFloatBE)
        NCJS_MAP_OBJECT_FUNCTION("writeFloatLE",  WriteFloatLE)
    NCJS_END_OBJECT_FACTORY()

};

/// ----------------------------------------------------------------------------
/// define module
/// ----------------------------------------------------------------------------

NCJS_DEFINE_BUILTIN_MODULE(buffer, ModuleBuffer);

} // ncjs