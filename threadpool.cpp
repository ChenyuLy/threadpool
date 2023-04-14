#include "threadpool.h"


void ThreadPool::init()
{
    for(int i = 0; i<m_threads.size();i++){
        m_threads.at(i) = std::thread(Worker(i,this));
        m_threads.at(i).detach();
    }
}

void ThreadPool::shutdown()
{
    is_shutdown = true;
    m_cond.notify_all();

    // for(int i = 0; i< m_threads.size();i++){
    //     if(m_threads.at(i).joinable())
    //     {
    //         m_threads.at(i).join();
    //     }
    // }
}
