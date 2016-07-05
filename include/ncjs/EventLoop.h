
/***************************************************************
 * Name:      EventLoop.h
 * Purpose:   Defines Node-CEF Event Loop Class
 * Author:    Joshua GPBeta (studiocghibli@gmail.com)
 * Created:   2016-6-29
 * Copyright: Studio GPBeta (www.gpbeta.com)
 * License:
 **************************************************************/
 
#ifndef NCJS_EVENTLOOP_H
#define NCJS_EVENTLOOP_H

typedef struct uv_loop_s uv_loop_t;

/// ----------------------------------------------------------------------------
/// Headers
/// ----------------------------------------------------------------------------

#include <include/base/cef_callback_forward.h>
#include <include/base/cef_macros.h>

namespace ncjs {

class EventLoopImpl;

/// ----------------------------------------------------------------------------
/// \class EventLoop
/// ----------------------------------------------------------------------------
class EventLoop {
public:

    bool IsRunning() const { return m_impl ? true : false; }

    bool Start();
    bool Stop();

    bool Queue(const base::Closure& work);

    uv_loop_t* ToUv() const { return reinterpret_cast<uv_loop_t*>(m_impl); }

    /// Constructors & Destructor
    /// --------------------------------------------------------------
    
    EventLoop();
    ~EventLoop();

private:

    /// Declarations
    /// -----------------

    EventLoopImpl* m_impl;

    DISALLOW_COPY_AND_ASSIGN(EventLoop);
};


} // ncjs

#endif // NCJS_EVENTLOOP_H
