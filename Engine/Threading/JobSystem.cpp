
#include <utility>
#include "Logger.h"
#include "JobSystem.h"

namespace minicell
{
    JobSystem::JobSystem(usize workerCount)
    {
        if (workerCount == 0) 
        {
            minicell::logInfo("Minimum of one worker should be assigned");
            return;
        }
        
        for (usize i=0; i<workerCount; ++i)
        {
            m_workers.emplace_back([this]
            {
                while (true) 
                {
                    std::function<void()> job;
                    
                    {
                        std::unique_lock<std::mutex> lock(m_mutex);     // lock shared queue and state
                        m_jobReady.wait(lock, [this]                    
                        { 
                            return m_shutdown || !m_jobs.empty();
                        });                                             // sleep until shutdown is requested or a queued job is available

                        if (m_shutdown && m_jobs.empty())               
                        {
                            return;
                        }                                               // exit this worker after shutdown is requested and the queue is drained

                        job = std::move(m_jobs.front());                // move the oldest queued job into this worker's local job
                        m_jobs.pop();                                   // then remove the front-most job
                        ++m_inFlight;                                   // the job is no longer queued but not yet finished
                    }   // mutex automatically unlocks

                    job();

                    {
                        std::unique_lock<std::mutex> lock(m_mutex);     // gain the lock
                        --m_inFlight;                                   // mark the job as finished
                        if (m_jobs.empty() && m_inFlight == 0)          // the system has become idle; wake every thread waiting in waitIdle()
                        {
                            m_idle.notify_all();
                        }
                    }
                }

            });
        }
    }

    JobSystem::~JobSystem()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_shutdown = true;
        lock.unlock();

        m_jobReady.notify_all();

        for (auto& w : m_workers) {
            if (w.joinable()) {
                w.join();
            }
        }
    }

    // add one job to the shared queue and wake a worker
    void JobSystem::submit(std::function<void()> job)
    {
        std::unique_lock<std::mutex> lock(m_mutex);  // acquire and lock
        m_jobs.push(std::move(job));
        lock.unlock();
        m_jobReady.notify_one();
    }

    // block until no jobs are queued or executing
    void JobSystem::waitIdle()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_idle.wait(lock, [this]{ 
            return m_jobs.empty() && m_inFlight == 0;
        });
    }
}