#ifndef THREADPOOL_H
#define THREADPOOL_H

// STL
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>


class ThreadPool
{
  public:
    ThreadPool(unsigned int threadsNum, unsigned int maxThreads = std::thread::hardware_concurrency())
      : m_cancelled(false)
      , m_maxThreads(maxThreads)
    {
      for (unsigned int i = 0; i < threadsNum; ++i)
        m_threads.push_back(std::thread(&ThreadPool::runJob, this));
    }

    ThreadPool() = delete;
    ThreadPool(const ThreadPool&) = delete;

    ~ThreadPool()
    {
      m_cancelled = true;
      std::unique_lock<std::mutex> lck(m_mtx);
      m_cv.notify_all();

      for (std::thread& th : m_threads)
        th.join();
    }

    bool addJob(void (*fcn)(void*), void* args)
    {
      Job job {fcn, args};
      std::unique_lock<std::mutex> lck(m_mtx);
      if (m_jobs.size() >= m_maxThreads)
      {
        //m_cv.wait(lck);
        return false;
      }
      m_jobs.push_back(job);
      m_cv.notify_one();
      return true;
    }

    void wait()
    {
      std::unique_lock<std::mutex> lck(m_mtx);
      while (!m_cancelled && !m_jobs.empty())
        m_cv.wait(lck);
    }

  private:

    void runJob()
    {
      // std::defer_lock means m_mtx not locked after creation
      std::unique_lock<std::mutex> lck(m_mtx, std::defer_lock);
      while (!m_cancelled)
      {
        lck.lock();
        while (!m_cancelled && m_jobs.empty())
          m_cv.wait(lck);

        if (m_cancelled)
        {
          lck.unlock();
          return;
        }

        Job job = m_jobs.front();
        m_jobs.erase(m_jobs.begin());
        lck.unlock();

        job.function(job.arg);
        lck.lock();
        m_cv.notify_all();
        lck.unlock();
      }
    }

  private:
    struct Job
    {
      //! function template
      void (*function)(void* arg);
      //! args
      void* arg;
    };

    bool m_cancelled;
    unsigned int m_maxThreads;
    std::vector<Job> m_jobs;
    std::mutex m_mtx;
    std::condition_variable m_cv;
    std::vector<std::thread> m_threads;
};

#endif // THREADPOOL_H
