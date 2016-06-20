
/***************************************************************
 * Name:      module.h
 * Purpose:   Defines Node-CEF Module Infrastructure
 * Author:    Joshua GPBeta (studiocghibli@gmail.com)
 * Created:   2016-05-14
 * Copyright: Studio GPBeta (www.gpbeta.com)
 * License:
 **************************************************************/
 
#ifndef NCJS_MODULE_H
#define NCJS_MODULE_H

/// ----------------------------------------------------------------------------
/// Headers
/// ----------------------------------------------------------------------------

#include "ncjs/base.h"
#include "ncjs/version.h"
#include "ncjs/string.h"
#include "ncjs/Environment.h"

/// ----------------------------------------------------------------------------
/// Macros
/// ----------------------------------------------------------------------------

#define NCJS_F_BUILTIN 0x01
#define NCJS_F_LINKED  0x02


#define NCJS_ARG_IS(_TYPE, _ARGS, _N) (_ARGS.size() > _N && _ARGS[_N]->Is##_TYPE())


#define NCJS_OBJECT_FUNCTION(_FUNCTION) \
    class _FUNCTION##Handler : public FunctionHandlerT<_object_t> { \
    public: \
        virtual bool Execute(const CefString& name, \
                             CefRefPtr<CefV8Value> object, \
                             const CefV8ValueList& arguments, \
                             CefRefPtr<CefV8Value>& retval, \
                             CefString& exception) OVERRIDE \
        { \
            Thiz()._FUNCTION(object, arguments, retval, exception); \
            return true; \
        } \
    }; \
    _FUNCTION##Handler m_##_FUNCTION; \
    inline void _FUNCTION

#define NCJS_ACCESSOR_GETTER(_FUNCTION) \
    virtual bool Get(const CefString& name, \
                     const CefRefPtr<CefV8Value> object, \
                           CefRefPtr<CefV8Value>& retval, \
                           CefString& exception) OVERRIDE \
    { \
        _FUNCTION(object, retval, exception); \
        return true; \
    } \
    inline void _FUNCTION

#define NCJS_ACCESSOR_SETTER(_FUNCTION) \
    virtual bool Set(const CefString& name, \
                     const CefRefPtr<CefV8Value> object, \
                     const CefRefPtr<CefV8Value> value, \
                           CefString& exception) OVERRIDE \
    { \
        _FUNCTION(object, value, exception); \
        return true; \
    } \
    inline void _FUNCTION


#define NCJS_BEGIN_OBJECT_FACTORY() \
    public: \
    static CefRefPtr<CefV8Value> ObjectFactory(CefRefPtr<Environment> _environment, \
                                               CefRefPtr<CefV8Context> _context) \
    { \
        CefRefPtr<_object_t> _thiz = _object_t::GetInstance(); \
        CefRefPtr<CefV8Value> _object = CefV8Value::CreateObject(NULL);

#define NCJS_END_OBJECT_FACTORY() \
        return _object; \
    }


#define NCJS_BEGIN_OBJECT_BUILDER() \
    public: \
    static void ObjectBuilder(CefRefPtr<Environment> _environment, \
                              CefRefPtr<CefV8Context> _context, \
                              CefRefPtr<CefV8Value> _object) \
    { \
        CefRefPtr<_object_t> _thiz = _object_t::GetInstance();

#define NCJS_END_OBJECT_BUILDER() }


#define NCJS_MAP_OBJECT_X(_TYPE, _NAME, _VALUE, _ATTRIBUTE) \
    _object->SetValue(_NAME, CefV8Value::Create##_TYPE(_VALUE), _ATTRIBUTE);

#define NCJS_MAP_OBJECT(_TYPE, _NAME, _VALUE) \
    NCJS_MAP_OBJECT_X(_TYPE, _NAME, _VALUE, V8_PROPERTY_ATTRIBUTE_NONE)

#define NCJS_MAP_OBJECT_READONLY(_TYPE, _NAME, _VALUE) \
    NCJS_MAP_OBJECT_X(_TYPE, _NAME, _VALUE, V8_PROPERTY_ATTRIBUTE_READONLY)

#define NCJS_MAP_OBJECT_CONSTANT(_TYPE, _NAME, _VALUE) \
    NCJS_MAP_OBJECT_X(_TYPE, _NAME, _VALUE, \
        CefV8Value::PropertyAttribute(V8_PROPERTY_ATTRIBUTE_READONLY | V8_PROPERTY_ATTRIBUTE_DONTDELETE))


#define NCJS_MAP_OBJECT_FUNCTION_X(_NAME, _FUNCTION, _ATTRIBUTE) \
    _thiz->m_##_FUNCTION.Initial(_thiz); \
    _object->SetValue(_NAME, CefV8Value::CreateFunction(_NAME, &_thiz->m_##_FUNCTION), _ATTRIBUTE);

#define NCJS_MAP_OBJECT_FUNCTION(_NAME, _FUNCTION) \
    NCJS_MAP_OBJECT_FUNCTION_X(_NAME, _FUNCTION, V8_PROPERTY_ATTRIBUTE_NONE)

#define NCJS_MAP_OBJECT_FUNCTION_READONLY(_NAME, _FUNCTION) \
    NCJS_MAP_OBJECT_FUNCTION_X(_NAME, _FUNCTION, V8_PROPERTY_ATTRIBUTE_READONLY)

#define NCJS_MAP_OBJECT_FUNCTION_CONSTANT(_NAME, _FUNCTION) \
    NCJS_MAP_OBJECT_FUNCTION_X(_NAME, _FUNCTION, \
        CefV8Value::PropertyAttribute(V8_PROPERTY_ATTRIBUTE_READONLY | V8_PROPERTY_ATTRIBUTE_DONTDELETE))


#define NCJS_MAP_OBJECT_FACTORY_X(_NAME, _OBJECT, _ATTRIBUTE) \
    _object->SetValue(_NAME, _OBJECT::ObjectFactory(_environment, _context), _ATTRIBUTE);

#define NCJS_MAP_OBJECT_FACTORY(_NAME, _OBJECT) \
    NCJS_MAP_OBJECT_FACTORY_X(_NAME, _OBJECT, V8_PROPERTY_ATTRIBUTE_NONE)

#define NCJS_MAP_OBJECT_FACTORY_READONLY(_NAME, _OBJECT) \
    NCJS_MAP_OBJECT_FACTORY_X(_NAME, _OBJECT, V8_PROPERTY_ATTRIBUTE_READONLY)

#define NCJS_MAP_OBJECT_FACTORY_CONSTANT(_NAME, _OBJECT) \
    NCJS_MAP_OBJECT_FACTORY_X(_NAME, _OBJECT, \
        CefV8Value::PropertyAttribute(V8_PROPERTY_ATTRIBUTE_READONLY | V8_PROPERTY_ATTRIBUTE_DONTDELETE))


#define NCJS_MAP_OBJECT_ACCESSOR_X(_NAME, _ACCESSOR, _ATTRIBUTE) \
    CefRefPtr<CefV8Value> _obj##_ACCESSOR = CefV8Value::CreateObject(_ACCESSOR::GetInstance()); \
    _object->SetValue(_NAME, _obj##_ACCESSOR, _ATTRIBUTE);

#define NCJS_MAP_OBJECT_ACCESSOR(_NAME, _ACCESSOR) \
    NCJS_MAP_OBJECT_ACCESSOR_X(_NAME, _ACCESSOR, V8_PROPERTY_ATTRIBUTE_NONE)

#define NCJS_MAP_OBJECT_ACCESSOR_READONLY(_NAME, _ACCESSOR) \
    NCJS_MAP_OBJECT_ACCESSOR_X(_NAME, _ACCESSOR, V8_PROPERTY_ATTRIBUTE_READONLY)

#define NCJS_MAP_OBJECT_ACCESSOR_CONSTANT(_NAME, _ACCESSOR) \
    NCJS_MAP_OBJECT_ACCESSOR_X(_NAME, _ACCESSOR, \
        CefV8Value::PropertyAttribute(V8_PROPERTY_ATTRIBUTE_READONLY | V8_PROPERTY_ATTRIBUTE_DONTDELETE))

#define NCJS_MAP_OBJECT_EXTRA(_FUNCTION) \
    _FUNCTION(_environment, _object);


#define NCJS_DEFINE_MODULE(_MODULE, _FACTORY) \
	static ::ncjs::module_t s_module = \
    { \
      NODE_MODULE_VERSION, \
      _FLAGS, \
      NULL, \
      __FILE__, \
      NULL, \
      (::ncjs::addon_context_factory_func) (_FACTORY::ObjectFactory), \
      NCJS_MAKESTR(_MODULE), \
      NULL, \
      NULL, \
    }; \
	static void* s_module_register = ::ncjs::ncjsRegisterModule(&s_module)

#define NCJS_BUILTIN_MODULE(_MODULE) g_builtin_module_##_MODULE

#define NCJS_DECLARE_BUILTIN_MODULE(_MODULE) extern ::ncjs::module_t NCJS_BUILTIN_MODULE(_MODULE)

#define NCJS_DEFINE_BUILTIN_MODULE(_MODULE, _FACTORY) \
	NCJS_DECLARE_BUILTIN_MODULE(_MODULE) = \
    { \
      NCJS_MODULE_VERSION, \
      NCJS_F_BUILTIN, \
      NULL, \
      __FILE__, \
      NULL, \
      (::ncjs::addon_context_factory_func) (_FACTORY::ObjectFactory), \
      NCJS_MAKESTR(_MODULE), \
      NULL, \
      NULL, \
    } \


namespace ncjs {

typedef void* addon_factory_func;

typedef CefRefPtr<CefV8Value> (*addon_context_factory_func)(
    CefRefPtr<Environment> enviroment,
    CefRefPtr<CefV8Context> context,
    void* priv);

struct module_t {
    int version;
    unsigned int flags;
    void* handle;
    const char* filename;
    addon_factory_func factory;
    addon_context_factory_func ctxfactory;
    const char* modname;
    void* priv;
    module_t* link;
};

template <class T, bool SHARE = true>
class JsObjecT;

template <class T, bool SHARE = true>
class JsAccessorT;

// share a singleton instance between objects
template <class T>
class JsObjecT<T, true> {
public:
    typedef T _object_t;

    static T* GetInstance() { static T shared; return &shared; }

    NCJS_IMPL_NO_REFCOUNTING(T);
};

template <class T>
class JsAccessorT<T, true> : public CefV8Accessor {
public:
    static T* GetInstance() { static T shared; return &shared; }

    NCJS_IMPL_NO_REFCOUNTING(T);
};

// create an instance for each object
template <class T>
class JsObjecT<T, false> : public CefBase {
public:
    typedef T _object_t;

    static T* GetInstance() { return new T; }

    IMPLEMENT_REFCOUNTING(T);
};

template <class T>
class JsAccessorT<T, false> : public CefV8Accessor {
public:
    static T* GetInstance() { return new T; }

    IMPLEMENT_REFCOUNTING(T);
};


template <class T>
class FunctionHandlerT : public CefV8Handler {
public:

    void    AddRef() const OVERRIDE { _thiz->AddRef(); }
    bool   Release() const OVERRIDE { return _thiz->Release(); }
    bool HasOneRef() const OVERRIDE { return _thiz->HasOneRef(); }

          T& Thiz()       { return *_thiz; }
    const T& Thiz() const { return *_thiz; }

    void Initial(T* thiz) { _thiz = thiz; }

    FunctionHandlerT() : _thiz(NULL) {}

private:

    T* _thiz;
};

} // ncjs

#endif // NCJS_MODULE_H
