/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* SharedImageBuffer.cpp                                                */
/*                                                                      */
/* Nick D'Ademo <nickdademo@gmail.com>                                  */
/*                                                                      */
/* Copyright (c) 2012-2016 Nick D'Ademo                                 */
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

#include <QDebug>

SharedImageBuffer::SharedImageBuffer()
{
    m_doSync = false;
    m_nArrived = 0;
}

void SharedImageBuffer::add(int deviceNumber, Buffer<cv::Mat>* imageBuffer, StreamControl streamControl, bool sync)
{
    QMutexLocker locker(&m_mutex);

    // Add to maps
    m_imageBufferMap[deviceNumber] = imageBuffer;
    if (sync)
    {
        m_syncSet.insert(deviceNumber);
    }
    if (!m_runPauseStreamMap.contains(deviceNumber))
    {
        QSemaphore *semaphore = new QSemaphore(0);
        m_runPauseStreamMap[deviceNumber] = semaphore;
        _setStreamControl(deviceNumber, streamControl);
    }
}

Buffer<cv::Mat>* SharedImageBuffer::get(int deviceNumber)
{
    return m_imageBufferMap[deviceNumber];
}

void SharedImageBuffer::remove(int deviceNumber)
{
    QMutexLocker locker(&m_mutex);

    // Remove from maps
    m_imageBufferMap.remove(deviceNumber);
    m_syncSet.remove(deviceNumber);
    m_streamControlMap.remove(deviceNumber);
    m_runPauseStreamMap.remove(deviceNumber);
}

void SharedImageBuffer::streamControl(int deviceNumber)
{
    m_mutex.lock();
    QSemaphore *semaphore = m_runPauseStreamMap[deviceNumber];
    m_mutex.unlock();

    // Wait if paused (i.e. if no semaphore is available)
    if (semaphore->available() == 0)
    {
        semaphore->acquire();
        semaphore->release();
    }

    // Perform sync if enabled
    m_mutex.lock();
    if (m_syncSet.contains(deviceNumber))
    {
        // Increment arrived count
        m_nArrived++;
        // We are the last to arrive: wake all waiting threads
        if (m_doSync && (m_nArrived == m_syncSet.size()))
        {
            m_syncedStreams.wakeAll();
        }
        // Still waiting for other streams to arrive: wait
        else
        {
            m_syncedStreams.wait(&m_mutex);
        }
        // Decrement arrived count
        m_nArrived--;
    }
    m_mutex.unlock();
}

SharedImageBuffer::StreamControl SharedImageBuffer::getStreamControl(int deviceNumber)
{
    QMutexLocker locker(&m_mutex);
    return m_streamControlMap[deviceNumber];
}

void SharedImageBuffer::setStreamControl(int deviceNumber, StreamControl streamControl)
{
    QMutexLocker locker(&m_mutex);
    _setStreamControl(deviceNumber, streamControl);
}

void SharedImageBuffer::_setStreamControl(int deviceNumber, StreamControl streamControl)
{
    if (streamControl == StreamControl::Run)
    {
        // Run stream if no semaphore is available
        QSemaphore *semaphore = m_runPauseStreamMap[deviceNumber];
        if (semaphore->available() == 0)
        {
            semaphore->release();
        }

        emit streamRun(deviceNumber);
    }
    else if (streamControl == StreamControl::Pause)
    {
        // Pause stream if semaphore is available
        QSemaphore *semaphore = m_runPauseStreamMap[deviceNumber];
        if (semaphore->available() > 0)
        {
            semaphore->acquire();
        }

        emit streamPaused(deviceNumber);
    }

    m_streamControlMap[deviceNumber] = streamControl;
}

bool SharedImageBuffer::contains(int deviceNumber)
{
    return m_imageBufferMap.contains(deviceNumber);
}

void SharedImageBuffer::startSync()
{
    QMutexLocker locker(&m_mutex);
    m_doSync = true;
    m_syncedStreams.wakeAll();
    emit syncStarted();
}

void SharedImageBuffer::stopSync()
{
    QMutexLocker locker(&m_mutex);
    m_doSync = false;
    emit syncStopped();
}

bool SharedImageBuffer::isSyncEnabled(int deviceNumber)
{
    QMutexLocker locker(&m_mutex);
    return m_syncSet.contains(deviceNumber);
}