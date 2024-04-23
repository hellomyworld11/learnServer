#ifndef __LOG_H
#define __LOG_H
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>
#include "../lock/locker.h"
#include "block_queue.h"
const int NameSize = 128;
#define SYNLOG  //同步写日志
//#define ASYNLOG //异步写日志

class Log{
public:
    static Log* get_instance();
    
    static void *flush_log_thread(void *args)
    {
        Log::get_instance()->async_write_log();
    }

    bool init(const char* file_name, int log_buf_size = 8192, int splite_lines = 5000000, int max_queue_size = 0);

    void write_log(int level,const char* file, const char* fun, const int line, const char *format, ...);

    void flush(void);
private:
    Log();
    virtual ~Log();
    void* async_write_log()
    {
        std::string single_log;
        //从阻塞队列中取出一个日志string，写入文件
        while (_log_queue->pop(single_log))
        {
            _mutex.lock();
            fputs(single_log.c_str(), _fp);
            _mutex.unlock();
        }
    }
private:
    char _dir_name[NameSize];
    char _log_name[NameSize];
    int  _split_lines;
    int _log_buf_size;
    long long _count;
    int _today;
    FILE *_fp;
    char *_buf;
    block_queue<std::string> *_log_queue;
    bool _is_async;
    locker _mutex;
};

#define LOG_DEBUG(format, ...) Log::get_instance()->write_log(0, __FILE__, __FUNCTION__,__LINE__, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) Log::get_instance()->write_log(1, __FILE__, __FUNCTION__,__LINE__, format, ##__VA_ARGS__)
#define LOG_WARN(format, ...) Log::get_instance()->write_log(2, __FILE__, __FUNCTION__,__LINE__, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) Log::get_instance()->write_log(3, __FILE__, __FUNCTION__,__LINE__, format, ##__VA_ARGS__)


#endif