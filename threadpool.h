#include <iostream>
#include <thread>
#include <mutex>
#include <memory>
#include <condition_variable>
#include <functional>
#include <queue>
#include <vector>
#include <future>

template <typename T>
class SafeQueue
{
private:
    std::queue<T> m_queue;
    std::mutex m_mutex;

public:
    SafeQueue(){};
    ~SafeQueue(){};
    SafeQueue(SafeQueue &&){};
    bool empty()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    int size()
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        return m_queue.size();
    }

    void enqueue(T &t)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.push(t);
    }

    bool dequeue(T &t)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_queue.empty())
            return false;

        t = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }
};


class ThreadPool
{
private:
    int is_shutdown;
    SafeQueue<std::function<void()>> m_q;
    std::vector<std::thread> m_threads;

    std::condition_variable m_cond;
    std::mutex m_mutex;
    class Worker
    {
    private:
        ThreadPool * m_pool;
        int m_id;
    public:
        Worker(int id,ThreadPool *pool):m_id(id),m_pool(pool){};
        ~Worker(){};

        void operator()(){
            std::function<void()> func;
            bool dequeue;

            while (!m_pool->is_shutdown)
            {
                std::unique_lock<std::mutex> lock(m_pool->m_mutex);
                while (m_pool->m_q.empty())
                {
                    m_pool->m_cond.wait(lock);
                }

                dequeue = m_pool->m_q.dequeue(func);
                if(dequeue){
                    func();
                }
            }
            
        }
    };

public:
    ThreadPool(int n_threads):m_threads(std::vector<std::thread>(n_threads)),is_shutdown(false){};
    ~ThreadPool(){};
    
    ThreadPool(const ThreadPool &) = delete; //删除拷贝函数
    ThreadPool(ThreadPool &&) = delete;//删除移动沟杂函数
    ThreadPool &operator=(const ThreadPool &) = delete; //默认赋值函数
    ThreadPool &operator=(ThreadPool &&) = delete;  //移动拷贝函数

    void init();
    void shutdown();
    
    template<typename F,typename ...Args>
    auto submit(F &&f,Args && ...args)->std::future<decltype(f(args...))>{
        std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f),std::forward<Args> (args)...);
        auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>> (func);

        std::function<void()> warpper_func = [task_ptr](){ (*task_ptr)(); };

        m_q.enqueue(warpper_func);
        m_cond.notify_one();

        return task_ptr->get_future();
    }


    
    
    // template<typename F,typename ...Args>
    // auto submit(F &&f,Args && ...args) -> std::future<decltype(f(args...))> {
    //     std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f),std::forward<Args>(args)...);
    //     auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>> (func);
        
    //     std::function<void()> warpper_func = [task_ptr](){
    //         (*task_ptr)();
    //     };

    //     m_q.enqueue(warpper_func);
    //     m_cond.notify_one();
    //     return task_ptr->get_future();
    // }
};





