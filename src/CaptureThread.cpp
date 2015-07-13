/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* CaptureThread.cpp                                                    */
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

#include "CaptureThread.h"

#include "SharedImageBuffer.h"
#include "Config.h"

#include <QDebug>

CaptureThread::CaptureThread(SharedImageBuffer *sharedImageBuffer, int deviceNumber, bool dropFrameIfBufferFull, int width, int height) :
    QThread(),
    m_sharedImageBuffer(sharedImageBuffer)
{
    m_dropFrameIfBufferFull = dropFrameIfBufferFull;
    m_deviceNumber = deviceNumber;
    m_width = width;
    m_height = height;
    m_doStop = false;
    m_sampleNumber = 0;
    m_fpsSum = 0;
    m_fps.clear();
    m_statsData.averageFPS = 0;
    m_statsData.nFramesProcessed = 0;
}

void CaptureThread::run()
{
    while(1)
    {
        ////////////////////////////////
        // Stop thread if doStop=TRUE //
        ////////////////////////////////
        m_doStopMutex.lock();
        if (m_doStop)
        {
            m_doStop = false;
            m_doStopMutex.unlock();
            break;
        }
        m_doStopMutex.unlock();
        /////////////////////////////////
        /////////////////////////////////

        // Save capture time
        m_captureTime = m_t.elapsed();
        // Start timer (used to calculate capture rate)
        m_t.start();

        // Synchronize with other streams (if enabled for this stream)
        m_sharedImageBuffer->sync(m_deviceNumber);

        // Capture frame (if available)
        if (!m_cap.grab())
        {
            continue;
        }

        // Retrieve frame
        m_cap.retrieve(m_grabbedFrame);
        // Add frame to buffer
        m_sharedImageBuffer->getByDeviceNumber(m_deviceNumber)->add(m_grabbedFrame, m_dropFrameIfBufferFull);

        // Update statistics
        updateFPS(m_captureTime);
        m_statsData.nFramesProcessed++;
        // Inform GUI of updated statistics
        emit updateStatisticsInGUI(m_statsData);
    }

    qDebug() << "Stopping capture thread...";
}

bool CaptureThread::connectToCamera()
{
    // Open camera
    bool camOpenResult = m_cap.open(m_deviceNumber);
    // Set resolution
    if (m_width != -1)
    {
        m_cap.set(CV_CAP_PROP_FRAME_WIDTH, m_width);
    }
    if (m_height != -1)
    {
        m_cap.set(CV_CAP_PROP_FRAME_HEIGHT, m_height);
    }
    // Return result
    return camOpenResult;
}

bool CaptureThread::disconnectCamera()
{
    // Camera is connected
    if (m_cap.isOpened())
    {
        // Disconnect camera
        m_cap.release();
        return true;
    }
    // Camera is NOT connected
    else
    {
        return false;
    }
}

void CaptureThread::updateFPS(int timeElapsed)
{
    // Add instantaneous FPS value to queue
    if(timeElapsed > 0)
    {
        m_fps.enqueue((int)1000 / timeElapsed);
        // Increment sample number
        m_sampleNumber++;
    }
    // Maximum size of queue is DEFAULT_CAPTURE_FPS_STAT_QUEUE_LENGTH
    if (m_fps.size()>CAPTURE_FPS_STAT_QUEUE_LENGTH)
    {
        m_fps.dequeue();
    }
    // Update FPS value every DEFAULT_CAPTURE_FPS_STAT_QUEUE_LENGTH samples
    if ((m_fps.size() == CAPTURE_FPS_STAT_QUEUE_LENGTH) && (m_sampleNumber == CAPTURE_FPS_STAT_QUEUE_LENGTH))
    {
        // Empty queue and store sum
        while (!m_fps.empty())
        {
            m_fpsSum += m_fps.dequeue();
        }
        // Calculate average FPS
        m_statsData.averageFPS = m_fpsSum / CAPTURE_FPS_STAT_QUEUE_LENGTH;
        // Reset sum
        m_fpsSum = 0;
        // Reset sample number
        m_sampleNumber = 0;
    }
}

void CaptureThread::stop()
{
    QMutexLocker locker(&m_doStopMutex);
    m_doStop = true;
}

bool CaptureThread::isCameraConnected()
{
    return m_cap.isOpened();
}

int CaptureThread::getInputSourceWidth()
{
    return m_cap.get(CV_CAP_PROP_FRAME_WIDTH);
}

int CaptureThread::getInputSourceHeight()
{
    return m_cap.get(CV_CAP_PROP_FRAME_HEIGHT);
}
