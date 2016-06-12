
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

#include <include/cef_base.h>

namespace ncjs {

/// ----------------------------------------------------------------------------
/// \class UserData
/// Please derive your user data class to ncjs::UserData and specify a
/// number larger than UserData::CUSTOM for the constructor's parameter to
/// prevent conflicts of user data set by Node-CEF users.
/// ----------------------------------------------------------------------------
class UserData : public CefBase {
public:

    enum Type { UNKNOWN, ENVIRONMENT, BUFFER, CUSTOM = 0x1000 };

    Type GetType() const { return m_type; }
    bool IsTypeOf(Type type) const { return m_type == type; }

    /// Static Functions
    /// --------------------------------------------------------------

    static Type MakeType(int type) { return static_cast<Type>(type); }
    
    static bool IsTypeOf(const CefRefPtr<CefBase>& data, Type type)
    {
        if (CefBase* p = data.get())
            return static_cast<UserData*>(p)->IsTypeOf(type);

        return false;
    }

    /// Constructors & Destructor
    /// --------------------------------------------------------------

    UserData(Type type = CUSTOM) : m_type(type) {}

private:

    /// Declarations
    /// -----------------

    const Type m_type;
};

} // ncjs

#endif // NCJS_USERDATA_H
