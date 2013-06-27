/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* Buffer.h                                                             */
/*                                                                      */
/* Nick D'Ademo <nickdademo@gmail.com>                                  */
/*                                                                      */
/* Copyright (c) 2012-2013 Nick D'Ademo                                 */
/*                                                                      */
/* Permission is hereby granted, free of charge, to any person          */
/* obtaining a copy of this software and associated documentation       */
/* files (the "Software"), to deal in the Software without restriction, */
/* including without limitation the rights to use, copy, modify, merge, */
/* publish, distribute, sublicense, and/or sell copies of the Software, */
/* and to permit persons to whom the Software is furnished to do so,    */
/* subject to the following conditions:                                 */
/*                                                                      */
/* The above copyright notice and this permission notice shall be       */
/* included in all copies or substantial portions of the Software.      */
/*                                                                      */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,      */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF   */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                */
/* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS  */
/* BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN   */
/* ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN    */
/* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE     */
/* SOFTWARE.                                                            */
/*                                                                      */
/************************************************************************/

#ifndef BUFFER_H
#define BUFFER_H

// Qt
#include <QMutex>
#include <QQueue>
#include <QSemaphore>
#include <QByteArray>
#include <QDebug>

template<class T> class Buffer
{
    public:
        Buffer(int size);
        void add(const T& data, bool dropIfFull=false);
        T get();
        int size();
        int maxSize();
        bool clear();
        bool isFull();
        bool isEmpty();

    private:
        QMutex queueProtect;
        QQueue<T> queue;
        QSemaphore *freeSlots;
        QSemaphore *usedSlots;
        QSemaphore *clearBuffer_add;
        QSemaphore *clearBuffer_get;
        int bufferSize;
};

template<class T> Buffer<T>::Buffer(int size)
{
    // Save buffer size
    bufferSize = size;
    // Create semaphores
    freeSlots = new QSemaphore(bufferSize);
    usedSlots = new QSemaphore(0);
    clearBuffer_add = new QSemaphore(1);
    clearBuffer_get = new QSemaphore(1);
}

template<class T> void Buffer<T>::add(const T& data, bool dropIfFull)
{
    // Acquire semaphore
    clearBuffer_add->acquire();
    // If dropping is enabled, do not block if buffer is full
    if(dropIfFull)
    {
        // Try and acquire semaphore to add item
        if(freeSlots->tryAcquire())
        {
            // Add item to queue
            queueProtect.lock();
            queue.enqueue(data);
            queueProtect.unlock();
            // Release semaphore
            usedSlots->release();
        }
    }
    // If buffer is full, wait on semaphore
    else
    {
        // Acquire semaphore
        freeSlots->acquire();
        // Add item to queue
        queueProtect.lock();
        queue.enqueue(data);
        queueProtect.unlock();
        // Release semaphore
        usedSlots->release();
    }
    // Release semaphore
    clearBuffer_add->release();
}

template<class T> T Buffer<T>::get()
{
    // Local variable(s)
    T data;
    // Acquire semaphores
    clearBuffer_get->acquire();
    usedSlots->acquire();
    // Take item from queue
    queueProtect.lock();
    data=queue.dequeue();
    queueProtect.unlock();
    // Release semaphores
    freeSlots->release();
    clearBuffer_get->release();
    // Return item to caller
    return data;
}

template<class T> bool Buffer<T>::clear()
{
    // Check if buffer contains items
    if(queue.size()>0)
    {
        // Stop adding items to buffer (will return false if an item is currently being added to the buffer)
        if(clearBuffer_add->tryAcquire())
        {
            // Stop taking items from buffer (will return false if an item is currently being taken from the buffer)
            if(clearBuffer_get->tryAcquire())
            {
                // Release all remaining slots in queue
                freeSlots->release(queue.size());
                // Acquire all queue slots
                freeSlots->acquire(bufferSize);
                // Reset usedSlots to zero
                usedSlots->acquire(queue.size());
                // Clear buffer
                queue.clear();
                // Release all slots
                freeSlots->release(bufferSize);
                // Allow get method to resume
                clearBuffer_get->release();
            }
            else
                return false;
            // Allow add method to resume
            clearBuffer_add->release();
            return true;
        }
        else
            return false;
    }
    else
        return false;
}

template<class T> int Buffer<T>::size()
{
    return queue.size();
}

template<class T> int Buffer<T>::maxSize()
{
    return bufferSize;
}

template<class T> bool Buffer<T>::isFull()
{
    return queue.size()==bufferSize;
}

template<class T> bool Buffer<T>::isEmpty()
{
    return queue.size()==0;
}

#endif // BUFFER_H
