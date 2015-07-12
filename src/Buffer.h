/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* Buffer.h                                                             */
/*                                                                      */
/* Nick D'Ademo <nickdademo@gmail.com>                                  */
/*                                                                      */
/* Copyright (c) 2012-2015 Nick D'Ademo                                 */
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

#include <QMutex>
#include <QQueue>
#include <QSemaphore>
#include <QByteArray>
#include <QDebug>

template<class T> class Buffer
{
    public:
        Buffer(int size);
        void add(const T& data, bool dropIfFull = false);
        T get();
        bool clear();
        int size() const
        {
            return m_queue.size();
        }
        int maxSize() const
        {
            return m_bufferSize;
        }
        bool isFull() const
        {
            return m_queue.size() == m_bufferSize;
        }
        bool isEmpty() const
        {
            return m_queue.size() == 0;
        }

    private:
        QMutex m_queueProtect;
        QQueue<T> m_queue;
        QSemaphore *m_freeSlots;
        QSemaphore *m_usedSlots;
        QSemaphore *m_clearBuffer_add;
        QSemaphore *m_clearBuffer_get;
        int m_bufferSize;
};

template<class T> Buffer<T>::Buffer(int size)
{
    // Save buffer size
    m_bufferSize = size;
    // Create semaphores
    m_freeSlots = new QSemaphore(m_bufferSize);
    m_usedSlots = new QSemaphore(0);
    m_clearBuffer_add = new QSemaphore(1);
    m_clearBuffer_get = new QSemaphore(1);
}

template<class T> void Buffer<T>::add(const T& data, bool dropIfFull)
{
    // Acquire semaphore
    m_clearBuffer_add->acquire();
    // If dropping is enabled, do not block if buffer is full
    if(dropIfFull)
    {
        // Try and acquire semaphore to add item
        if (m_freeSlots->tryAcquire())
        {
            // Add item to queue
            m_queueProtect.lock();
            m_queue.enqueue(data);
            m_queueProtect.unlock();
            // Release semaphore
            m_usedSlots->release();
        }
    }
    // If buffer is full, wait on semaphore
    else
    {
        // Acquire semaphore
        m_freeSlots->acquire();
        // Add item to queue
        m_queueProtect.lock();
        m_queue.enqueue(data);
        m_queueProtect.unlock();
        // Release semaphore
        m_usedSlots->release();
    }
    // Release semaphore
    m_clearBuffer_add->release();
}

template<class T> T Buffer<T>::get()
{
    // Local variable(s)
    T data;
    // Acquire semaphores
    m_clearBuffer_get->acquire();
    m_usedSlots->acquire();
    // Take item from queue
    m_queueProtect.lock();
    data = m_queue.dequeue();
    m_queueProtect.unlock();
    // Release semaphores
    m_freeSlots->release();
    m_clearBuffer_get->release();
    // Return item to caller
    return data;
}

template<class T> bool Buffer<T>::clear()
{
    // Check if buffer contains items
    if (m_queue.size() > 0)
    {
        // Stop adding items to buffer (will return false if an item is currently being added to the buffer)
        if (m_clearBuffer_add->tryAcquire())
        {
            // Stop taking items from buffer (will return false if an item is currently being taken from the buffer)
            if (m_clearBuffer_get->tryAcquire())
            {
                // Release all remaining slots in queue
                m_freeSlots->release(m_queue.size());
                // Acquire all queue slots
                m_freeSlots->acquire(m_bufferSize);
                // Reset usedSlots to zero
                m_usedSlots->acquire(m_queue.size());
                // Clear buffer
                m_queue.clear();
                // Release all slots
                m_freeSlots->release(m_bufferSize);
                // Allow get method to resume
                m_clearBuffer_get->release();
            }
            else
            {
                return false;
            }
            // Allow add method to resume
            m_clearBuffer_add->release();
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

#endif // BUFFER_H
