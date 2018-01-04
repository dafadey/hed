#include <pthread.h>
#include <vector>
#include <iostream>

#define VOLATILE

struct context
{
  //context(int);
  context() : active(false), id(-1), f_ptr(NULL), arg_ptr(NULL), mtx(), busy_mtx() {}
  ~context();
  
  void init(int) VOLATILE;
  
  inline void lock() VOLATILE
  {
    pthread_mutex_lock(&mtx);
  }
  inline void unlock() VOLATILE
  {
    pthread_mutex_unlock(&mtx);
  }

  inline void busy_lock() VOLATILE
  {
    pthread_mutex_lock(&busy_mtx);
  }
  inline void busy_unlock() VOLATILE
  {
    pthread_mutex_unlock(&busy_mtx);
  }

  void start_thread() VOLATILE;

  bool active;
  int id;
  void (*f_ptr)(void*);
  void* arg_ptr;
  pthread_mutex_t mtx;
  pthread_mutex_t busy_mtx;
  pthread_t pth;
  void print() VOLATILE
  {
    std::cout << "context # " << id << (active ? ", active" : ", non-active") << " func_ptr=" << f_ptr << ", args_ptr=" << arg_ptr << "\n";
  }
};

void *th_worker(void* _ctx_ptr)
{
  context* ctx_ptr = (context*) _ctx_ptr;
  ctx_ptr->busy_unlock();
  ctx_ptr->active = true;
  while(ctx_ptr->active)
  {
    ctx_ptr->lock();
    if(ctx_ptr->f_ptr)
      ctx_ptr->f_ptr(ctx_ptr->arg_ptr);
    ctx_ptr->busy_unlock();
  }
  return NULL;
}

void context::init(int _id) VOLATILE
{
  id = _id;
  pthread_mutex_init(&mtx, NULL);
  pthread_mutex_lock(&mtx);
  pthread_mutex_init(&busy_mtx, NULL);
  pthread_mutex_lock(&busy_mtx);
}

context::~context()
{
  pthread_mutex_destroy(&mtx);
  pthread_mutex_destroy(&busy_mtx);
}

void context::start_thread() VOLATILE
{
  pthread_create(&pth, NULL, th_worker, this);
}

struct executor
{
  executor(int n) : sz(n), threads(NULL)
  {
    threads = new VOLATILE context[n];
    for(int i=0;i!=n;i++)
      threads[i].start_thread();
    for(int i=0;i!=n;i++)
      threads[i].busy_lock();
    for(int i=0;i!=n;i++)
      threads[i].busy_unlock();
  }
  int sz;
  VOLATILE context* threads;
  
  void exec(void (*f)(void*), void* argptr, int thid)
  {
    threads[thid].busy_lock();
    threads[thid].f_ptr = f;
    threads[thid].arg_ptr = argptr;
    threads[thid].unlock();
  }

  void sync()
  {
    for(int thid(0); thid!=sz; thid++)
    {
      threads[thid].busy_lock();
      threads[thid].f_ptr = NULL;
      threads[thid].arg_ptr = NULL;
      threads[thid].unlock();
    }
  }
  
  void finish()
  {
    for(int thid(0); thid!=sz; thid++)
    {
      threads[thid].busy_lock();
      threads[thid].f_ptr = NULL;
      threads[thid].arg_ptr = NULL;
      threads[thid].active = false;
      threads[thid].unlock();
    }
  }
 };


