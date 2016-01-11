/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* ProcessingThread.cpp                                                 */
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

#include "ProcessingThread.h"

#include "SharedImageBuffer.h"
#include "Buffer.h"
#include "MatToQImage.h"

#include <QDebug>

ProcessingThread::ProcessingThread(SharedImageBuffer *sharedImageBuffer, int deviceNumber) :
    QThread(),
    m_sharedImageBuffer(sharedImageBuffer),
    m_deviceNumber(deviceNumber),
    m_doStop(false),
    m_sampleCount(0)
{
    m_statistics.init();
    m_imageProcessing.init();
}

void ProcessingThread::run()
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

        // Sync
        m_sharedImageBuffer->processingSync(m_deviceNumber);
       
        // Get frame from queue, store in currentFrame, set ROI
        m_processingMutex.lock();
        m_currentFrame = cv::Mat(m_sharedImageBuffer->get(m_deviceNumber)->get().clone(), m_currentRoi);

        // Example of how to grab a frame from another stream (where Device Number=1)
        // Note: This requires stream synchronization to be ENABLED (in the Options menu of MainWindow) and frame processing for the stream you are grabbing FROM to be DISABLED.
        /*
        if (m_sharedImageBuffer->contains(1))
        {
            // Grab frame from another stream (connected to camera with Device Number=1)
            cv::Mat frameFromAnotherStream = cv::Mat(m_sharedImageBuffer->get(1)->get(), m_currentRoi);
            // Linear blend images together using OpenCV and save the result to currentFrame. Note: beta = 1 - alpha
            cv::addWeighted(frameFromAnotherStream, 0.5, m_currentFrame, 0.5, 0.0, m_currentFrame);
        }
        */

        ////////////////////////////////////
        // PERFORM IMAGE PROCESSING BELOW //
        ////////////////////////////////////
        // Grayscale conversion
        if (m_imageProcessing.grayscale.enabled && (m_currentFrame.channels() == 3 || m_currentFrame.channels() == 4))
        {
            cvtColor(
                m_currentFrame,
                m_currentFrame,
                CV_BGR2GRAY
            );
        }

        // Smooth
        if (m_imageProcessing.smooth.enabled)
        {
            switch (m_imageProcessing.smooth.type)
            {
                // Blur
                case 0:
                    blur(
                        m_currentFrame,
                        m_currentFrame,
                        cv::Size(m_imageProcessing.smooth.parameter1, m_imageProcessing.smooth.parameter2)
                    );
                    break;
                // Gaussian
                case 1:
                    GaussianBlur(
                        m_currentFrame,
                        m_currentFrame,
                        cv::Size(m_imageProcessing.smooth.parameter1, m_imageProcessing.smooth.parameter2),
                        m_imageProcessing.smooth.parameter3,
                        m_imageProcessing.smooth.parameter4
                    );
                    break;
                // Median
                case 2:
                    medianBlur(
                        m_currentFrame,
                        m_currentFrame,
                        m_imageProcessing.smooth.parameter1
                    );
                    break;
            }
        }
        // Dilate
        if (m_imageProcessing.dilate.enabled)
        {
            dilate(
                m_currentFrame,
                m_currentFrame,
                cv::Mat(),
                cv::Point(-1, -1),
                m_imageProcessing.dilate.nIterations
            );
        }
        // Erode
        if (m_imageProcessing.erode.enabled)
        {
            erode(
                m_currentFrame,
                m_currentFrame,
                cv::Mat(),
                cv::Point(-1, -1),
                m_imageProcessing.erode.nIterations
            );
        }
        // Flip
        if (m_imageProcessing.flip.enabled)
        {
            flip(
                m_currentFrame,
                m_currentFrame,
                m_imageProcessing.flip.code
            );
        }
        // Canny edge detection
        if (m_imageProcessing.canny.enabled)
        {
            Canny(
                m_currentFrame,
                m_currentFrame,
                m_imageProcessing.canny.threshold1,
                m_imageProcessing.canny.threshold2,
                m_imageProcessing.canny.apertureSize,
                m_imageProcessing.canny.l2gradient
            );
        }
        ////////////////////////////////////
        // PERFORM IMAGE PROCESSING ABOVE //
        ////////////////////////////////////

        // Convert Mat to QImage
        m_frame = MatToQImage(m_currentFrame);
        m_processingMutex.unlock();

        // Only emit valid frames
        if (!m_frame.isNull())
        {
            emit newFrame(m_frame);
        }

        // Update statistics
        m_statistics.frameCounter++;
        updateFps(m_time.elapsed());
    }

    qDebug().noquote() << QString("[%1]: Stopping processing thread...").arg(m_deviceNumber);
}

void ProcessingThread::updateFps(int timeElapsed)
{
    if (timeElapsed > 0)
    {
        // Add instantaneous value to queue
        m_statistics.frameTimes.enqueue(timeElapsed);

        // Ensure queue length is constant by popping oldest values
        if (m_statistics.frameTimes.size() > PROCESSING__FPS_STAT_QUEUE_LENGTH)
        {
            for (int i = 0; i < (m_statistics.frameTimes.size() - PROCESSING__FPS_STAT_QUEUE_LENGTH); i++);
            {
                m_statistics.frameTimes.dequeue();
            }
        }

        // Update average FPS value if queue is full
        if (m_statistics.frameTimes.size() == PROCESSING__FPS_STAT_QUEUE_LENGTH)
        {
            // Reset sum
            m_statistics.frameTimesSum = 0;

            // Store sum
            for (int i = 0; i < PROCESSING__FPS_STAT_QUEUE_LENGTH; i++)
            {
                m_statistics.frameTimesSum += m_statistics.frameTimes.at(i);
            }

            // Calculate average FPS
            m_statistics.averageFps = 1000 / (static_cast<float>(m_statistics.frameTimesSum) / PROCESSING__FPS_STAT_QUEUE_LENGTH);

            // Emit signal
            if (++m_sampleCount >= PROCESSING__EMIT_DATA_N_SAMPLES)
            {
                m_sampleCount = 0;
                emit newStatistics(m_statistics);
            }
        }
    }
}

void ProcessingThread::stop()
{
    QMutexLocker locker(&m_doStopMutex);
    m_doStop = true;
}

void ProcessingThread::updateImageProcessing(ImageProcessing imageProcessing)
{
    QMutexLocker locker(&m_processingMutex);
    m_imageProcessing = imageProcessing;
}

void ProcessingThread::setRoi(QRect roi)
{
    QMutexLocker locker(&m_processingMutex);
    m_currentRoi.x = roi.x();
    m_currentRoi.y = roi.y();
    m_currentRoi.width = roi.width();
    m_currentRoi.height = roi.height();
}

QRect ProcessingThread::currentRoi()
{
    return QRect(m_currentRoi.x, m_currentRoi.y, m_currentRoi.width, m_currentRoi.height);
}
