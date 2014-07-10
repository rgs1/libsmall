/*-*- Mode: C; c-basic-offset: 2; indent-tabs-mode: nil -*-*/

/* simple abstraction to dispatch work onto a thread */

#include <assert.h>
#include <pthread.h>

#include <small/util.h>
#include <small/worker.h>

#include "util-internal.h"


struct worker {
  void *(*loop)(void *data);
  void *data;
  pthread_t tid;
};

SMALL_EXPORT
worker *worker_new(void *(*loop)(void *), void *data)
{
  worker *w = (worker *)safe_alloc(sizeof(worker));
  w->loop = loop;
  w->data = data;
  return w;
}

SMALL_EXPORT
void worker_run(worker *w)
{
  pthread_create(&w->tid, NULL, w->loop, w);
  set_thread_name(w->tid, "worker");
}

SMALL_EXPORT
worker *worker_create(void *(*loop)(void *), void *data)
{
  worker *w = worker_new(loop, data);
  worker_run(w);
  return w;
}

SMALL_EXPORT
void *worker_get_data(worker *w)
{
  assert(w);
  return w->data;
}

SMALL_EXPORT
void worker_destroy(worker *w)
{
  /* TODO:
   *
   *   loop should be wrapped with a func that does:
   *   a) worker_running(True)
   *   b) loop()
   *   c) worker_running(False)
   *
   *   then, to actually destroy a worker:
   *   0) call worker_wants_stop()
   *   1) wait N secs
   *   2) if worker_running(), pthread_kill(thread, SIGUSR1)
   *   3) from SIGUSR1 handle, pthread_exit()
   *   4) wait N secs
   *   5) if !worker_running() free(w) else fail()
   */
}
