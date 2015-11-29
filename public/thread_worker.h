#ifndef _THREAD_WORKER_
#define _THREAD_WORKER_

#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <queue>

class ThreadWorker
{
public:

    //put the job into thread.
    void SetJob(std::function &func)
    {
        //set the proc function.
        m_func = func;

        //notify the condition variable.
        m_mutex.lock();
        m_cond_var.notify_one();
        m_mutex.unlock();
    }

    //do the process.
    void Run() {
        //wait for job.
        m_mutex.lock();
        m_cond_var.wait(m_mutex);
        func();
        m_mutex.unlock();

        //put itself into idle queue.
    }

private:
    std::function m_func;
    std::queue<ThreadWorker> &m_idle_queue;
    std::vector<std::thread> &m_thread_pool;
    int thread_index;

    std::mutex m_mutex;
    std::condition_variable m_cond_var;
};

#endif
