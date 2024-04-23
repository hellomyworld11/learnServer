#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>
#include "../lock/locker.h"
#include "../CGImysql/sql_connection_pool.h"

template <typename T>
class threadpool
{
public:
    threadpool(connection_pool *connPool, int thread_num = 8, int max_request = 10000);
    ~threadpool();

    bool append(T *request);

private:
    static void *worker(void *arg);  //类成员函数不能作为pthread_creat参数. 会被编译器优化包含了this参数
    void run();

private:
    int _thread_number;
    int _max_requests;
    pthread_t *_threads;
    std::list<T *> _workqueue;
    locker _queuelocker;  
    sem _queuestat;
    bool _stop;
    connection_pool *_connPool;
};

template <typename T>
threadpool<T>::threadpool(connection_pool *connPool, int thread_num, int max_requests)
    :_connPool(connPool),_thread_number(thread_num),_max_requests(max_requests),_stop(false),_threads(NULL)
{
    if (thread_num <= 0 || max_requests <= 0)
    {
        throw std::exception();
    }
    _threads = new pthread_t[thread_num];
    if (!_threads)
    {
        throw std::exception();
    }
    for (int i = 0; i < thread_num; i++)
    {
        if (pthread_create(_threads + i, NULL, worker, this) != 0 )
        {
            delete[] _threads;
            throw std::exception();
        }
        if(pthread_detach(_threads[i]))
        {
            delete[] _threads;
            throw std::exception();
        }
    }
}



template <typename T>
threadpool<T>::~threadpool()
{
    delete[] _threads;
    _stop = true;
}

template <typename T>
bool threadpool<T>::append(T *request)
{
    _queuelocker.lock();
    
    if (_workqueue.size() >= _max_requests)
    {
        _queuelocker.unlock();
     //   std::cout << "任务队列已满" << std::endl;
        return false;
    }
    
    _workqueue.push_back(request);

    _queuelocker.unlock();

    _queuestat.post();

    return true;
}


template <typename T>
void * threadpool<T>::worker(void *arg)
{
    threadpool *pool = static_cast<threadpool*>(arg);
    pool->run();
}

template <typename T>
void threadpool<T>::run()
{
    while(!_stop)
    {
        _queuestat.wait();
        _queuelocker.lock();
        
        if(_workqueue.empty())
        {
            _queuelocker.unlock();
            continue;
        }

        T *work =  _workqueue.front();
        _workqueue.pop_front();
        _queuelocker.unlock();    
        if (!work)
        {
            continue;
        }
        
        //这里用RAII 维护数据库的一个连接的占用与解除占用
        connectionRAII mysqlcon(&work->mysql, _connPool);
        
        work->process();     
    }
}



#endif