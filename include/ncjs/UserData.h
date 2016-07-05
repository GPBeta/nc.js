
/***************************************************************
 * Name:      UserData.h
 * Purpose:   Defines Node-CEF User Data Class
 * Author:    Joshua GPBeta (studiocghibli@gmail.com)
 * Created:   2016-05-30
 * Copyright: Studio GPBeta (www.gpbeta.com)
 * License:
 **************************************************************/
 
#ifndef NCJS_USERDATA_H
#define NCJS_USERDATA_H

/// ----------------------------------------------------------------------------
/// Headers
/// ----------------------------------------------------------------------------

#include <include/cef_v8.h>

namespace ncjs {

namespace USER_DATA {

enum { UNKNOWN, BUFFER, FS_EVENT_WRAP, STAT_WATCHER_WRAP, CUSTOM = 0x1000 };

typedef int TYPE;

} // USER_DATA

/// ----------------------------------------------------------------------------
/// \class UserData
/// Please derive your user data class to ncjs::UserData and specify a
/// number larger than USER_DATA::CUSTOM for the constructor's parameter to
/// prevent conflicts of user data set by Node-CEF users.
/// ----------------------------------------------------------------------------
template <class T, USER_DATA::TYPE TID>
class UserData : public CefBase {
public:

    USER_DATA::TYPE GetDataType() const { return m_type; }
    bool IsTypeOf(USER_DATA::TYPE type) const { return m_type == type; }


    bool Wrap(const CefRefPtr<CefV8Value>& wrap)
    {
        return wrap->SetUserData(static_cast<UserData<T, TID>*>(this));
    }

    CefRefPtr<CefV8Value> Wrap()
    {
        const CefRefPtr<CefV8Value> wrap = CefV8Value::CreateObject(NULL);
        Wrap(wrap);
        return wrap;
    }

    /// Static Functions
    /// --------------------------------------------------------------

    static T* Unwrap(const CefRefPtr<CefV8Value>& wrap)
    {
        if (CefBase* base = wrap->GetUserData().get()) {
            UserData<T, TID>* data = static_cast<UserData<T, TID>*>(base);
            if (data->IsTypeOf(TID))
                return static_cast<T*>(data);
        }

        return NULL;
    }

    /// Constructors & Destructor
    /// --------------------------------------------------------------

    UserData() : m_type(TID) {}

private:

    /// Declarations
    /// -----------------

    const USER_DATA::TYPE m_type;
};

} // ncjs

#endif // NCJS_USERDATA_H
