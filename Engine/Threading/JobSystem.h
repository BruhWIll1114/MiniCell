#pragma once

#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include "Types.h"

namespace minicell
{
    class JobSystem : NonCopyable 
    {
        private:
            std::vector<std::thread> m_workers;
            std::condition_variable m_jobReady; // wakes workers when work or shutdown is available
            std::condition_variable m_idle;     // wakes waitIdle() when all work has finished
            
            std::mutex m_mutex;
            std::queue<std::function<void()>> m_jobs;       // shared FIFO queue of pending work
            usize m_inFlight = 0;                           // jobs removed from the queue but not yet finished
            bool m_shutdown = false;
            
        public:
            explicit JobSystem(usize workerCount);
            ~JobSystem();

            void submit(std::function<void()> job);
            void waitIdle();
    };
}