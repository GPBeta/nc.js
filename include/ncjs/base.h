
/***************************************************************
 * Name:      base.h
 * Purpose:   Defines Node-CEF Basic Types
 * Author:    Joshua GPBeta (studiocghibli@gmail.com)
 * Created:   2016-05-14
 * Copyright: Studio GPBeta (www.gpbeta.com)
 * License:
 **************************************************************/
 
#ifndef NCJS_BASE_H
#define NCJS_BASE_H

/// ----------------------------------------------------------------------------
/// Headers
/// ----------------------------------------------------------------------------

#define NCJS_NAME_STRING "Node-CEF"
#define NCJS_ALIAS_STRING "nc.js"

#define _NCJS_MAKESTR(_STR) #_STR
#define NCJS_MAKESTR(_STR) _NCJS_MAKESTR(_STR)

#define NCJS_DEFINE_PROPERTY(_TYPE, _OBJECT, _KEY, _VALUE, _ATTRIBUTE) do \
    { \
        _OBJECT->SetValue(_KEY, CefV8Value::Create##_TYPE(_VALUE), _ATTRIBUTE); \
    } while (0)

#define NCJS_PROPERTY(_TYPE, _OBJECT, _KEY, _VALUE) \
    NCJS_DEFINE_PROPERTY(_TYPE, _OBJECT, _KEY, _VALUE, V8_PROPERTY_ATTRIBUTE_NONE)

#define NCJS_PROPERTY_CONST(_TYPE, _OBJECT, _KEY, _VALUE) \
    NCJS_DEFINE_PROPERTY(_TYPE, _OBJECT, _KEY, _VALUE, \
        CefV8Value::PropertyAttribute(V8_PROPERTY_ATTRIBUTE_READONLY | V8_PROPERTY_ATTRIBUTE_DONTDELETE))

#define NCJS_PROPERTY_CONST_AUTO(_TYPE, _OBJECT, _VALUE) \
    NCJS_PROPERTY_CONST(_TYPE, _OBJECT, NCJS_REFTEXT(#_VALUE), _VALUE)


#define NCJS_IMPL_NO_REFCOUNTING(_TYPE) \
    void    AddRef() const { } \
    bool   Release() const { return false; } \
    bool HasOneRef() const { return true; }

#ifdef _MSC_VER
#define PATH_MAX MAX_PATH
#endif // _MSC_VER

#ifdef _WIN32
#define NCJS_ABORT() raise(SIGABRT)
#else
#define NCJS_ABORT() abort()
#endif

#if defined(NDEBUG)
#define NCJS_ASSERT(_EXP)
#define NCJS_CHECK(_EXP) \
    do { \
        if (!(_EXP)) NCJS_ABORT(); \
    } while (0)
#else
#define NCJS_ASSERT(_EXP)  assert(_EXP)
#define NCJS_CHECK(_EXP)   assert(_EXP)
#endif

#define NCJS_DBG_EQ(a, b) NCJS_ASSERT((a) == (b))
#define NCJS_DBG_GE(a, b) NCJS_ASSERT((a) >= (b))
#define NCJS_DBG_GT(a, b) NCJS_ASSERT((a) > (b))
#define NCJS_DBG_LE(a, b) NCJS_ASSERT((a) <= (b))
#define NCJS_DBG_LT(a, b) NCJS_ASSERT((a) < (b))
#define NCJS_DBG_NE(a, b) NCJS_ASSERT((a) != (b))

#define NCJS_CHK_EQ(a, b) NCJS_CHECK((a) == (b))
#define NCJS_CHK_GE(a, b) NCJS_CHECK((a) >= (b))
#define NCJS_CHK_GT(a, b) NCJS_CHECK((a) > (b))
#define NCJS_CHK_LE(a, b) NCJS_CHECK((a) <= (b))
#define NCJS_CHK_LT(a, b) NCJS_CHECK((a) < (b))
#define NCJS_CHK_NE(a, b) NCJS_CHECK((a) != (b))

#define NCJS_UNREACHABLE() NCJS_ABORT()

namespace ncjs {

struct module_t;

//! Register a built-in or linked module
extern "C" bool ncjsRegisterModule(module_t* module);

// helper functions

template <class T, class V>
static inline T& As(V& v)
{
    return reinterpret_cast<T&>(v);
}

template <class T, class V>
static inline T To(const V& v)
{
    return reinterpret_cast<T>(v);
}

template <class T, unsigned N>
static inline unsigned ArraySize(const T (&src)[N])
{
    return N;
}

template <class T>
static inline void ReverseArray(T* src, size_t len)
{
    for (T* end = src + len - 1; src < end; ++src, --end) {
        const T temp = *src;
        *src = *end;
        *end = temp;
    }
}

template <class T, unsigned N>
static inline void ReverseArray(T (&src)[N])
{
    ReverseArray(src, N);
}

template<class T>
static inline const T& Max(const T& a, const T& b)
{
    return a > b ? a : b;
}

template<class T>
static inline const T& Min(const T& a, const T& b)
{
    return a < b ? a : b;
}

} // ncjs

#endif // NCJS_BASE_H
