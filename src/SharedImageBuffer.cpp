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
    m_doCaptureSync = false;
    m_nArrivedCapture = 0;
    m_doProcessingSync = false;
    m_nArrivedProcessing = 0;
}

void SharedImageBuffer::add(int deviceNumber, Buffer<cv::Mat>* imageBuffer, bool run, bool captureThreadSync, bool processingThreadSync)
{
    m_imageBufferMutex.lock();
    m_imageBufferMap[deviceNumber] = imageBuffer;
    m_imageBufferMutex.unlock();

    if (captureThreadSync)
    {
        m_captureSyncMutex.lock();
        m_captureSyncSet.insert(deviceNumber);
        m_captureSyncMutex.unlock();
    }
    if (processingThreadSync)
    {
        m_processingSyncMutex.lock();
        m_processingSyncSet.insert(deviceNumber);
        m_processingSyncMutex.unlock();
    }

    m_streamControlMutex.lock();
    if (!m_streamControlSemaphoreMap.contains(deviceNumber))
    {
        QSemaphore *semaphore = new QSemaphore(0);
        m_streamControlSemaphoreMap[deviceNumber] = semaphore;
        _setStreamRunning(deviceNumber, run);
    }
    m_streamControlMutex.unlock();
}

Buffer<cv::Mat>* SharedImageBuffer::get(int deviceNumber)
{
    QMutexLocker locker(&m_imageBufferMutex);
    return m_imageBufferMap[deviceNumber];
}

void SharedImageBuffer::remove(int deviceNumber)
{
    m_imageBufferMutex.lock();
    m_imageBufferMap.remove(deviceNumber);
    m_imageBufferMutex.unlock();

    m_captureSyncMutex.lock();
    m_captureSyncSet.remove(deviceNumber);
    m_captureSyncMutex.unlock();

    m_processingSyncMutex.lock();
    m_processingSyncSet.remove(deviceNumber);
    m_processingSyncMutex.unlock();

    m_streamControlMutex.lock();
    m_streamControlMap.remove(deviceNumber);
    m_streamControlSemaphoreMap.remove(deviceNumber);
    m_streamControlMutex.unlock();
}

void SharedImageBuffer::captureSync(int deviceNumber)
{
    QMutexLocker locker(&m_captureSyncMutex);

    // Perform sync if enabled
    if (m_captureSyncSet.contains(deviceNumber))
    {
        // Increment arrived count
        m_nArrivedCapture++;
        // We are the last to arrive: wake all waiting threads
        if (m_doCaptureSync && (m_nArrivedCapture == m_captureSyncSet.size()))
        {
            m_captureSyncWaitCondition.wakeAll();
        }
        // Still waiting for other streams to arrive: wait
        else
        {
            m_captureSyncWaitCondition.wait(&m_captureSyncMutex);
        }
        // Decrement arrived count
        m_nArrivedCapture--;
    }
}

bool SharedImageBuffer::isCaptureSyncEnabled(int deviceNumber)
{
    QMutexLocker locker(&m_captureSyncMutex);
    return m_captureSyncSet.contains(deviceNumber);
}

void SharedImageBuffer::startCaptureSync()
{
    QMutexLocker locker(&m_captureSyncMutex);
    m_doCaptureSync = true;
    m_captureSyncWaitCondition.wakeAll();
    emit captureSyncStarted();
}

void SharedImageBuffer::stopCaptureSync()
{
    QMutexLocker locker(&m_captureSyncMutex);
    m_doCaptureSync = false;
    emit captureSyncStopped();
}

void SharedImageBuffer::processingSync(int deviceNumber)
{
    QMutexLocker locker(&m_processingSyncMutex);

    // Perform sync if enabled
    if (m_processingSyncSet.contains(deviceNumber))
    {
        // Increment arrived count
        m_nArrivedProcessing++;
        // We are the last to arrive: wake all waiting threads
        if (m_doProcessingSync && (m_nArrivedProcessing == m_processingSyncSet.size()))
        {
            m_processingSyncWaitCondition.wakeAll();
        }
        // Still waiting for other streams to arrive: wait
        else
        {
            m_processingSyncWaitCondition.wait(&m_processingSyncMutex);
        }
        // Decrement arrived count
        m_nArrivedProcessing--;
    }
}

void SharedImageBuffer::startProcessingSync()
{
    QMutexLocker locker(&m_processingSyncMutex);
    m_doProcessingSync = true;
    m_processingSyncWaitCondition.wakeAll();
    emit processingSyncStarted();
}

void SharedImageBuffer::stopProcessingSync()
{
    QMutexLocker locker(&m_processingSyncMutex);
    m_doProcessingSync = false;
    emit processingSyncStopped();
}

bool SharedImageBuffer::isProcessingSyncEnabled(int deviceNumber)
{
    QMutexLocker locker(&m_processingSyncMutex);
    return m_processingSyncSet.contains(deviceNumber);
}

void SharedImageBuffer::streamControl(int deviceNumber)
{
    m_streamControlMutex.lock();
    QSemaphore *semaphore = m_streamControlSemaphoreMap[deviceNumber];
    m_streamControlMutex.unlock();

    // Wait if paused (i.e. if no semaphore is available)
    if (semaphore->available() == 0)
    {
        semaphore->acquire();
        semaphore->release();
    }
}

bool SharedImageBuffer::isStreamRunning(int deviceNumber)
{
    QMutexLocker locker(&m_streamControlMutex);
    return m_streamControlMap[deviceNumber];
}

void SharedImageBuffer::setStreamRunning(int deviceNumber, bool run)
{
    QMutexLocker locker(&m_streamControlMutex);
    _setStreamRunning(deviceNumber, run);
}

void SharedImageBuffer::_setStreamRunning(int deviceNumber, bool run)
{
    if (run)
    {
        // Run stream if no semaphore is available
        QSemaphore *semaphore = m_streamControlSemaphoreMap[deviceNumber];
        if (semaphore->available() == 0)
        {
            semaphore->release();
        }

        emit streamRun(deviceNumber);
    }
    else
    {
        // Pause stream if semaphore is available
        QSemaphore *semaphore = m_streamControlSemaphoreMap[deviceNumber];
        if (semaphore->available() > 0)
        {
            semaphore->acquire();
        }

        emit streamPaused(deviceNumber);
    }

    m_streamControlMap[deviceNumber] = run;
}

bool SharedImageBuffer::contains(int deviceNumber)
{
    return m_imageBufferMap.contains(deviceNumber);
}