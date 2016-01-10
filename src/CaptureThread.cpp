/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* CaptureThread.cpp                                                    */
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

#include "CaptureThread.h"

#include "SharedImageBuffer.h"
#include "Config.h"

#include <QDebug>

CaptureThread::CaptureThread(SharedImageBuffer *sharedImageBuffer, int deviceNumber, bool dropFrameIfBufferFull, int width, int height) :
    QThread(),
    m_sharedImageBuffer(sharedImageBuffer),
    m_deviceNumber(deviceNumber),
    m_dropFrameIfBufferFull(dropFrameIfBufferFull),
    m_width(width),
    m_height(height),
    m_doStop(false),
    m_sampleCount(0)
{
    m_statistics.init();
}

void CaptureThread::run()
{
    while(true)
    {
        // Start timer (used to calculate capture rate)
        m_time.start();

        ///////////////////////
        // Thread stop logic //
        ///////////////////////
        m_doStopMutex.lock();
        if (m_doStop)
        {
            m_doStop = false;
            m_doStopMutex.unlock();
            break;
        }
        m_doStopMutex.unlock();
        ///////////////////////
        ///////////////////////

        // Stream control
        m_sharedImageBuffer->streamControl(m_deviceNumber);

        // Capture frame (if available)
        if (!m_cap.grab())
        {
            continue;
        }
        m_statistics.frameCounter++;

        // Retrieve frame
        m_cap.retrieve(m_grabbedFrame);
        // Add frame to buffer (if valid)
        if (!m_grabbedFrame.empty())
        {
            m_sharedImageBuffer->get(m_deviceNumber)->add(m_grabbedFrame, m_dropFrameIfBufferFull);
        }

        // Update statistics
        updateFps(m_time.elapsed());
    }

    qDebug().noquote() << QString("[%1]: Stopping capture thread...").arg(m_deviceNumber);
}

bool CaptureThread::connectToCamera()
{
    // Open camera
    bool result = m_cap.open(m_deviceNumber);
    if (result)
    {
        // Set resolution
        if (m_width != -1)
        {
            if (!m_cap.set(CV_CAP_PROP_FRAME_WIDTH, m_width))
            {
                result = false;
            }
        }
        if (m_height != -1)
        {
            if (!m_cap.set(CV_CAP_PROP_FRAME_HEIGHT, m_height))
            {
                result = false;
            }
        }
    }

    return result;
}

void CaptureThread::disconnectCamera()
{
    m_cap.release();
}

bool CaptureThread::isCameraConnected()
{
    return m_cap.isOpened();
}

void CaptureThread::updateFps(int timeElapsed)
{
    if (timeElapsed > 0)
    {
        // Add instantaneous value to queue
        m_statistics.frameTimes.enqueue(timeElapsed);

        // Ensure queue length is constant by popping oldest values
        if (m_statistics.frameTimes.size() > CAPTURE__FPS_STAT_QUEUE_LENGTH)
        {
            for (int i = 0; i < (m_statistics.frameTimes.size() - CAPTURE__FPS_STAT_QUEUE_LENGTH); i++);
            {
                m_statistics.frameTimes.dequeue();
            }
        }

        // Update average FPS value if queue is full
        if (m_statistics.frameTimes.size() == CAPTURE__FPS_STAT_QUEUE_LENGTH)
        {
            // Reset sum
            m_statistics.frameTimesSum = 0;

            // Store sum
            for (int i = 0; i < CAPTURE__FPS_STAT_QUEUE_LENGTH; i++)
            {
                m_statistics.frameTimesSum += m_statistics.frameTimes.at(i);
            }

            // Calculate average FPS
            m_statistics.averageFps = 1000 / (static_cast<float>(m_statistics.frameTimesSum) / CAPTURE__FPS_STAT_QUEUE_LENGTH);

            // Emit signal
            if (++m_sampleCount >= CAPTURE__EMIT_DATA_N_SAMPLES)
            {
                m_sampleCount = 0;
                emit newStatistics(m_statistics);
            }
        }
    }
}

void CaptureThread::stop()
{
    QMutexLocker locker(&m_doStopMutex);
    m_doStop = true;
}