#ifndef __BLOCK_QUEUE_H
#define __BLOCK_QUEUE_H
#include <iostream>
#include <stdlib.h>

//循环队列

template<typename T>
class block_queue
{
public:
    block_queue(int max_size = 1000)
    {
        if (max_size <= 0)
        {
            exit(-1);
        }

        _max_size = max_size;
        _array = new T[max_size];
        _size = 0;
        _front = -1;
        _back = -1;
        
    }

    void clear()
    {
        _size = 0;
        _front = -1;
        _back = -1;

    }

    ~block_queue()
    {
        if (_array != NULL)
        {
            delete[] _array;
            _array = NULL;
        }             
    }

    bool full()
    {
            return _size >= _max_size;
    }

    bool empty()
    {
        if (_size <= 0)
        {
            return true;
        }
        return false;
    }

    bool front(T &value)
    {
        if (_front >= 0 && _front < _size)
        {
            value = _array[_front];
            return true;
        }
        return false;
    }

    bool back(T &value)
    {
          if (_back >= 0 && _back < _size)
        {
            value = _array[_back];
            return true;
        }
        return false;
    }

    int size()
    {
        return _size;
    }

    int max_size()
    {
        return _max_size;
    }

    bool push(const T &item)
    {
        if (_size >= _max_size)
        {
            return false;
        }
        
       _back = (_back + 1) % _max_size;
        _array[_back] = item;

        _size++;

        return true;
    }

    bool pop(T &item)
    {
        if (_size == 0)
        {
            return false;
        }
        _front = (_front + 1) % _max_size;
        item = _array[_front];
        _size--;
        return true;
        
    }



private:

    T *_array;
    int _size;
    int _max_size;
    int _front;
    int _back;
};

















#endif