#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include <concurrent/task.h>
#include <concurrent/thread.h>

#include <cstddef>
#include <exception>
#include <atomic>
#include <vector>
#include <memory>
#include <thread>
#include <string>
#include <vector>

#include "ring_buffer_spsc_lockfree.hpp"

#include <boost/noncopyable.hpp>

namespace concurrent
{

using ThreadPoolQueue = concurrent::RingBufferSPSCLockFree<concurrent::Task>;
using ThreadPoolQueuePtr = std::unique_ptr<ThreadPoolQueue>;

#define DEFAULT_WORK_QUEUE_SIZE 128

struct ThreadPoolArguments
{
    bool m_pinThreadsToCores;
    bool m_hyperThreading;
    int m_workQueueSizePerThread;
    int m_threadStackSize;
    std::vector<std::string> m_threadNames;
    
    ThreadPoolArguments() 
    : m_pinThreadsToCores{true}, m_hyperThreading{false}, m_workQueueSizePerThread{ DEFAULT_WORK_QUEUE_SIZE }, m_threadStackSize{0}
    {}
};

class ThreadPool : public boost::noncopyable
{
    public :
        ThreadPool() : m_numOfThreads(0) { m_isShuttingDown.store(false); }
        ~ThreadPool() { shutdown(); }
        void initialise(const ThreadPoolArguments& args);
        void submitTask(const Task& task, std::size_t queueID) throw(std::invalid_argument);
        void shutdown();

    private:
        struct ThreadArgument
        {
            ThreadPool* m_threadPool = nullptr;
            unsigned int m_queueIndex = -1;
            ThreadArgument(ThreadPool* pool, unsigned int queueIndex) : m_threadPool(pool), m_queueIndex(queueIndex) {}
        };

        // Move ctor deletion
        ThreadPool(ThreadPool&& other) = delete;
        // Move assignment operator deletion
        ThreadPool& operator=(ThreadPool&& other) = delete;

        std::vector<ThreadPtr> m_threads;
        std::vector<ThreadArgument> m_threadArguments;
        std::vector<ThreadPoolQueuePtr>  m_threadQueues;

        unsigned int m_numOfThreads;
        std::atomic<bool> m_isShuttingDown;
        static void* workerThreadFunction(void* argument);
};
    
}//namespace
#endif