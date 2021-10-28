#ifndef THREADPOOL_H
#define THREADPOOL_H

// STL
#include <thread>
#include <vector>


class ThreadPool
{
  public:
    constexpr static void (*ThreadDeleter)(std::thread *t) = [] (std::thread *t) {t->join(); delete t;};
    typedef std::unique_ptr<std::thread, decltype(ThreadDeleter)> ThreadPtr;
    typedef std::vector<ThreadPtr> ThreadPoolPrivate;
    ThreadPoolPrivate threads;
};

#endif // THREADPOOL_H
