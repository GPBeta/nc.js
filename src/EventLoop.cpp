
/***************************************************************
 * Name:      EventLoop.cpp
 * Purpose:   Codes for Node-CEF Event Loop Class
 * Author:    Joshua GPBeta (studiocghibli@gmail.com)
 * Created:   2016-06-29
 * Copyright: Studio GPBeta (www.gpbeta.com)
 * License:
 **************************************************************/

/// ============================================================================
/// declarations
/// ============================================================================

#define _WINSOCKAPI_    // stops windows.h including winsock.h

#define OFFSET_OF(type, member) (size_t)&(((type*)0)->member)

#define CONTAINER_OF(ptr, type, member) \
        ((type *)( (char *)(ptr) - OFFSET_OF(type, member) ))

/// ----------------------------------------------------------------------------
/// Headers
/// ----------------------------------------------------------------------------

#include "ncjs/EventLoop.h"

#include "ncjs/base.h"

#include <include/base/cef_bind.h>
#include <uv.h>

#include <vector>

namespace ncjs {

/// ----------------------------------------------------------------------------
/// variables
/// ----------------------------------------------------------------------------

class EventLoopImpl : public uv_loop_t {

    typedef std::vector<base::Closure> TaskQueue;

public:

    bool Queue(const base::Closure& work);

    /// Constructors & Destructor
    /// --------------------------------------------------------------

    EventLoopImpl();
    ~EventLoopImpl();

private:

    /// Static Functions
    /// --------------------------------------------------------------

    static void Run(void* arg);

    static void AsyncStop(uv_async_t* async);
    static void AsyncQueue(uv_async_t* async);

    /// Declarations
    /// -----------------

    uv_mutex_t m_mutex;
    uv_thread_t m_thread;
    uv_async_t m_asyncStop;
    uv_async_t m_asyncQueue;

    TaskQueue m_queue;
};

/// ============================================================================
/// implementation
/// ============================================================================

bool EventLoopImpl::Queue(const base::Closure& work)
{
    bool idling;
    {
        uv_mutex_lock(&m_mutex);

        idling = m_queue.empty();
        m_queue.push_back(work);

        uv_mutex_unlock(&m_mutex);
    }

    if (idling) // wake up work thread
        uv_async_send(&m_asyncQueue);

    return true;
}

bool EventLoop::Start()
{
    if (m_impl)
        return true;

    m_impl = new EventLoopImpl;

    return true;
}

bool EventLoop::Stop()
{
    if (m_impl == NULL)
        return true;

    delete m_impl;
    m_impl = NULL;

    return true;
}

bool EventLoop::Queue(const base::Closure& work)
{
    if (m_impl == NULL)
        return false;

    return m_impl->Queue(work);
}

/// ----------------------------------------------------------------------------
/// constructor & destructor
/// ----------------------------------------------------------------------------

EventLoopImpl::EventLoopImpl()
{
    NCJS_CHK_EQ(uv_loop_init(this), 0);
    NCJS_CHK_EQ(uv_mutex_init(&m_mutex), 0);
    NCJS_CHK_EQ(uv_async_init(this, &m_asyncStop, &AsyncStop), 0);
    NCJS_CHK_EQ(uv_async_init(this, &m_asyncQueue, &AsyncQueue), 0);
    NCJS_CHK_EQ(uv_thread_create(&m_thread, &Run, this), 0);
}

EventLoopImpl::~EventLoopImpl()
{
    uv_async_send(&m_asyncStop);
    uv_thread_join(&m_thread);

    uv_close(reinterpret_cast<uv_handle_t*>(&m_asyncStop), NULL);
    uv_close(reinterpret_cast<uv_handle_t*>(&m_asyncQueue), NULL);

    // UV_RUN_DEFAULT so that libuv has a chance to clean up.
    uv_run(this, UV_RUN_DEFAULT);

    uv_loop_close(this);
    uv_mutex_destroy(&m_mutex);
}

EventLoop::EventLoop() : m_impl(NULL) {}

EventLoop::~EventLoop() { Stop(); }

/// ----------------------------------------------------------------------------
/// static functions
/// ----------------------------------------------------------------------------

void EventLoopImpl::Run(void* arg)
{
    NCJS_ASSERT(arg);

    EventLoopImpl* impl = static_cast<EventLoopImpl*>(arg);

    uv_run(impl, UV_RUN_DEFAULT);
}

void EventLoopImpl::AsyncStop(uv_async_t* async)
{
    NCJS_ASSERT(async);

    EventLoopImpl* impl = CONTAINER_OF(async, EventLoopImpl, m_asyncStop);

    uv_stop(impl);
}

void EventLoopImpl::AsyncQueue(uv_async_t* async)
{
    NCJS_ASSERT(async);

    EventLoopImpl* impl = CONTAINER_OF(async, EventLoopImpl, m_asyncQueue);

    TaskQueue queue;

    {
        uv_mutex_lock(&impl->m_mutex);

        queue.swap(impl->m_queue);

        uv_mutex_unlock(&impl->m_mutex);
    }

    for (TaskQueue::const_iterator it = queue.begin(); it != queue.end(); ++it)
        it->Run();
}

} // ncjs

