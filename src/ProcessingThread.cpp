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
#include "Config.h"

#include <QDebug>

ProcessingThread::ProcessingThread(SharedImageBuffer *sharedImageBuffer, int deviceNumber) :
    QThread(),
    m_sharedImageBuffer(sharedImageBuffer),
    m_deviceNumber(deviceNumber),
    m_doStop(false),
    m_sampleNumber(0),
    m_fpsSum(0)
{
    m_fps.clear();
    m_statistics.init();
    m_imageProcessing.init();
}

void ProcessingThread::run()
{
    while(true)
    {
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

        // Save processing time
        m_processingTime = m_time.elapsed();
        // Start timer (used to calculate processing rate)
        m_time.start();

        m_processingMutex.lock();

        // Get frame from queue, store in currentFrame, set ROI
        m_currentFrame = cv::Mat(m_sharedImageBuffer->get(m_deviceNumber)->get().clone(), m_currentRoi);

        // Example of how to grab a frame from another stream (where Device Number=1)
        // Note: This requires stream synchronization to be ENABLED (in the Options menu of MainWindow) and frame processing for the stream you are grabbing FROM to be DISABLED.
        /*
        if(sharedImageBuffer->contains(1))
        {
            // Grab frame from another stream (connected to camera with Device Number=1)
            cv::Mat frameFromAnotherStream = cv::Mat(sharedImageBuffer->get(1)->getFrame(), currentRoi);
            // Linear blend images together using OpenCV and save the result to currentFrame. Note: beta = 1 - alpha
            cv::addWeighted(frameFromAnotherStream, 0.5, currentFrame, 0.5, 0.0, currentFrame);
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
        updateFps(m_processingTime);
        m_statistics.nFramesProcessed++;
        emit newStatistics(m_statistics);
    }

    qDebug().noquote() << QString("[%1]: Stopping processing thread...").arg(m_deviceNumber);
}

void ProcessingThread::updateFps(int timeElapsed)
{
    // Add instantaneous FPS value to queue
    if(timeElapsed > 0)
    {
        m_fps.enqueue((int)1000 / timeElapsed);
        // Increment sample number
        m_sampleNumber++;
    }

    // Maximum size of queue is DEFAULT_PROCESSING_FPS_STAT_QUEUE_LENGTH
    if (m_fps.size() > PROCESSING_FPS_STAT_QUEUE_LENGTH)
    {
        m_fps.dequeue();
    }

    // Update FPS value every DEFAULT_PROCESSING_FPS_STAT_QUEUE_LENGTH samples
    if ((m_fps.size() == PROCESSING_FPS_STAT_QUEUE_LENGTH) && (m_sampleNumber == PROCESSING_FPS_STAT_QUEUE_LENGTH))
    {
        // Empty queue and store sum
        while (!m_fps.empty())
        {
            m_fpsSum += m_fps.dequeue();
        }
        // Calculate average FPS
        m_statistics.averageFps = m_fpsSum / PROCESSING_FPS_STAT_QUEUE_LENGTH;
        // Reset sum
        m_fpsSum = 0;
        // Reset sample number
        m_sampleNumber = 0;
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
