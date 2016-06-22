
/***************************************************************
 * Name:      Buffer.h
 * Purpose:   Defines Node-CEF Buffer Class
 * Author:    Joshua GPBeta (studiocghibli@gmail.com)
 * Created:   2016-05-29
 * Copyright: Studio GPBeta (www.gpbeta.com)
 * License:
 **************************************************************/
 
#ifndef NCJS_BUFFER_H
#define NCJS_BUFFER_H

/// ----------------------------------------------------------------------------
/// Headers
/// ----------------------------------------------------------------------------

#include "ncjs/UserData.h"

#include <include/cef_v8.h>

namespace ncjs {

class Environment;

/// ----------------------------------------------------------------------------
/// \class Buffer
/// ----------------------------------------------------------------------------
class Buffer : public UserData {
public:

    char* Data() const { return m_buffer; }
    size_t Size() const { return m_size; }

    size_t IndexOffset(int offset)
    {
        if (offset < 0) {
            const size_t temp = -offset;
            return (temp < m_size) ? m_size - temp : 0;
        } // else start >= 0
        const size_t index = offset;
        return (index < m_size) ? index : m_size;
    }

    Buffer* SubBuffer(size_t offset, size_t size) const;
    int SubSearch(Buffer* sub, size_t offset, bool ucs2) const;

    /// Static Functions
    /// --------------------------------------------------------------

    static Buffer* Get(CefRefPtr<CefV8Value> object)
    {
        CefRefPtr<CefBase> data = object->GetUserData();
        // NOTE: User data could be NULL.
        if (IsTypeOf(data, BUFFER))
            return static_cast<Buffer*>(data.get());

        return NULL;
    }

    static void Wrap(Buffer* buffer, CefRefPtr<CefV8Value>& value);

    static bool IsWithinBounds(size_t off, size_t len, size_t max)
    {
        // Asking to seek too far into the buffer
        // check to avoid wrapping in subsequent subtraction
        if (off > max)
            return false;

        // Asking for more than is left over in the buffer
        if (max - off < len)
            return false;

        // Otherwise we're in bounds
            return true;
    }

    static Buffer* Create(CefRefPtr<Environment> env, size_t size);
    static Buffer* Create(size_t size); // despise buffer object flags
    static Buffer* Create(const CefString& str, int encoding);

private:

    Buffer(char* buffer, size_t size, const Buffer* owner = NULL) :
       UserData(BUFFER), m_owner(owner), m_buffer(buffer), m_size(size) {}
    ~Buffer() { if (NULL == m_owner.get()) free(m_buffer); }
    
    /// Declarations
    /// -----------------

    CefRefPtr<const Buffer> m_owner;

    char* m_buffer;
    size_t m_size;

    static const CefRefPtr<Buffer> EMPTY_BUFFER;

    IMPLEMENT_REFCOUNTING(Buffer);
};


} // ncjs

#endif // NCJS_BUFFER_H
