
#include <assert.h>

#include "uv.h"
#include "internal.h"
#include "atomicops-inl.h"
#include "handle-inl.h"
#include "req-inl.h"

int uv_async_send(uv_async_t* handle) {
  uv_loop_t* loop = handle->loop;

  if (handle->type != UV_ASYNC) {
    /* Can't set errno because that's not thread-safe. */
    return -1;
  }

  /* The user should make sure never to call uv_async_send to a closing */
  /* or closed handle. */
  assert(!(handle->flags & UV__HANDLE_CLOSING));

  if (!uv__atomic_exchange_set(&handle->async_sent)) {
    POST_COMPLETION_FOR_REQ(loop, &handle->async_req);
  }

  return 0;
}
