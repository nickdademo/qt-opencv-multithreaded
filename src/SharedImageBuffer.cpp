/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* SharedImageBuffer.cpp                                                */
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

#include "SharedImageBuffer.h"

SharedImageBuffer::SharedImageBuffer()
{
    m_nArrived = 0;
    m_doSync = false;
}

void SharedImageBuffer::add(int deviceNumber, Buffer<cv::Mat>* imageBuffer, bool sync)
{
    // Device stream is to be synchronized
    if(sync)
    {
        m_mutex.lock();
        m_syncSet.insert(deviceNumber);
        m_mutex.unlock();
    }
    // Add image buffer to map
    m_imageBufferMap[deviceNumber] = imageBuffer;
}

Buffer<cv::Mat>* SharedImageBuffer::getByDeviceNumber(int deviceNumber)
{
    return m_imageBufferMap[deviceNumber];
}

void SharedImageBuffer::removeByDeviceNumber(int deviceNumber)
{
    // Remove buffer for device from imageBufferMap
    m_imageBufferMap.remove(deviceNumber);

    // Also remove from syncSet (if present)
    m_mutex.lock();
    if (m_syncSet.contains(deviceNumber))
    {
        m_syncSet.remove(deviceNumber);
        m_wc.wakeAll();
    }
    m_mutex.unlock();
}

void SharedImageBuffer::sync(int deviceNumber)
{
    // Only perform sync if enabled for specified device/stream
    m_mutex.lock();
    if (m_syncSet.contains(deviceNumber))
    {
        // Increment arrived count
        m_nArrived++;
        // We are the last to arrive: wake all waiting threads
        if (m_doSync && (m_nArrived == m_syncSet.size()))
        {
            m_wc.wakeAll();
        }
        // Still waiting for other streams to arrive: wait
        else
        {
            m_wc.wait(&m_mutex);
        }
        // Decrement arrived count
        m_nArrived--;
    }
    m_mutex.unlock();
}

void SharedImageBuffer::wakeAll()
{
    QMutexLocker locker(&m_mutex);
    m_wc.wakeAll();
}

void SharedImageBuffer::setSyncEnabled(bool enable)
{
    m_doSync = enable;
}

bool SharedImageBuffer::isSyncEnabledForDeviceNumber(int deviceNumber)
{
    return m_syncSet.contains(deviceNumber);
}

bool SharedImageBuffer::getSyncEnabled()
{
    return m_doSync;
}

bool SharedImageBuffer::containsImageBufferForDeviceNumber(int deviceNumber)
{
    return m_imageBufferMap.contains(deviceNumber);
}
