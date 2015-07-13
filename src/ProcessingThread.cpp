/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* ProcessingThread.cpp                                                 */
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

#include "ProcessingThread.h"

#include "SharedImageBuffer.h"
#include "Buffer.h"
#include "MatToQImage.h"
#include "Config.h"

#include <QDebug>

ProcessingThread::ProcessingThread(SharedImageBuffer *sharedImageBuffer, int deviceNumber) :
    QThread(),
    m_sharedImageBuffer(sharedImageBuffer)
{
    m_deviceNumber = deviceNumber;
    m_doStop = false;
    m_sampleNumber = 0;
    m_fpsSum = 0;
    m_fps.clear();
    m_statsData.averageFPS = 0;
    m_statsData.nFramesProcessed = 0;
}

void ProcessingThread::run()
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

        // Save processing time
        m_processingTime = m_t.elapsed();
        // Start timer (used to calculate processing rate)
        m_t.start();

        m_processingMutex.lock();
        // Get frame from queue, store in currentFrame, set ROI
        m_currentFrame = cv::Mat(m_sharedImageBuffer->getByDeviceNumber(m_deviceNumber)->get().clone(), m_currentROI);

        // Example of how to grab a frame from another stream (where Device Number=1)
        // Note: This requires stream synchronization to be ENABLED (in the Options menu of MainWindow) and frame processing for the stream you are grabbing FROM to be DISABLED.
        /*
        if(sharedImageBuffer->containsImageBufferForDeviceNumber(1))
        {
            // Grab frame from another stream (connected to camera with Device Number=1)
            cv::Mat frameFromAnotherStream = cv::Mat(sharedImageBuffer->getByDeviceNumber(1)->getFrame(), currentROI);
            // Linear blend images together using OpenCV and save the result to currentFrame. Note: beta = 1 - alpha
            cv::addWeighted(frameFromAnotherStream, 0.5, currentFrame, 0.5, 0.0, currentFrame);
        }
        */

        ////////////////////////////////////
        // PERFORM IMAGE PROCESSING BELOW //
        ////////////////////////////////////
        // Grayscale conversion
        if (m_imgProcFlags.grayscaleOn && (m_currentFrame.channels() == 3 || m_currentFrame.channels() == 4))
        {
            cvtColor(m_currentFrame,
                m_currentFrame,
                CV_BGR2GRAY);
        }

        // Smooth
        if (m_imgProcFlags.smoothOn)
        {
            switch (m_imgProcSettings.smoothType)
            {
                // Blur
                case 0:
                    blur(m_currentFrame,
                        m_currentFrame,
                        cv::Size(m_imgProcSettings.smoothParam1, m_imgProcSettings.smoothParam2));
                    break;
                // Gaussian
                case 1:
                    GaussianBlur(m_currentFrame,
                        m_currentFrame,
                        cv::Size(m_imgProcSettings.smoothParam1, m_imgProcSettings.smoothParam2),
                        m_imgProcSettings.smoothParam3,
                        m_imgProcSettings.smoothParam4);
                    break;
                // Median
                case 2:
                    medianBlur(m_currentFrame,
                        m_currentFrame,
                        m_imgProcSettings.smoothParam1);
                    break;
            }
        }
        // Dilate
        if (m_imgProcFlags.dilateOn)
        {
            dilate(m_currentFrame,
                m_currentFrame,
                cv::Mat(),
                cv::Point(-1, -1),
                m_imgProcSettings.dilateNumberOfIterations);
        }
        // Erode
        if (m_imgProcFlags.erodeOn)
        {
            erode(m_currentFrame,
                m_currentFrame,
                cv::Mat(),
                cv::Point(-1, -1),
                m_imgProcSettings.erodeNumberOfIterations);
        }
        // Flip
        if (m_imgProcFlags.flipOn)
        {
            flip(m_currentFrame,
                m_currentFrame,
                m_imgProcSettings.flipCode);
        }
        // Canny edge detection
        if (m_imgProcFlags.cannyOn)
        {
            Canny(m_currentFrame,
                m_currentFrame,
                m_imgProcSettings.cannyThreshold1,
                m_imgProcSettings.cannyThreshold2,
                m_imgProcSettings.cannyApertureSize,
                m_imgProcSettings.cannyL2gradient);
        }
        ////////////////////////////////////
        // PERFORM IMAGE PROCESSING ABOVE //
        ////////////////////////////////////

        // Convert Mat to QImage
        m_frame = MatToQImage(m_currentFrame);
        m_processingMutex.unlock();

        // Inform GUI thread of new frame (QImage)
        emit newFrame(m_frame);

        // Update statistics
        updateFPS(m_processingTime);
        m_statsData.nFramesProcessed++;
        // Inform GUI of updated statistics
        emit updateStatisticsInGUI(m_statsData);
    }

    qDebug() << "Stopping processing thread...";
}

void ProcessingThread::updateFPS(int timeElapsed)
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
        m_statsData.averageFPS = m_fpsSum / PROCESSING_FPS_STAT_QUEUE_LENGTH;
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

void ProcessingThread::updateImageProcessingFlags(ImageProcessingFlags imgProcFlags)
{
    QMutexLocker locker(&m_processingMutex);
    m_imgProcFlags.grayscaleOn = imgProcFlags.grayscaleOn;
    m_imgProcFlags.smoothOn = imgProcFlags.smoothOn;
    m_imgProcFlags.dilateOn=imgProcFlags.dilateOn;
    m_imgProcFlags.erodeOn=imgProcFlags.erodeOn;
    m_imgProcFlags.flipOn=imgProcFlags.flipOn;
    m_imgProcFlags.cannyOn=imgProcFlags.cannyOn;
}

void ProcessingThread::updateImageProcessingSettings(ImageProcessingSettings imgProcSettings)
{
    QMutexLocker locker(&m_processingMutex);
    m_imgProcSettings.smoothType=imgProcSettings.smoothType;
    m_imgProcSettings.smoothParam1=imgProcSettings.smoothParam1;
    m_imgProcSettings.smoothParam2=imgProcSettings.smoothParam2;
    m_imgProcSettings.smoothParam3=imgProcSettings.smoothParam3;
    m_imgProcSettings.smoothParam4=imgProcSettings.smoothParam4;
    m_imgProcSettings.dilateNumberOfIterations=imgProcSettings.dilateNumberOfIterations;
    m_imgProcSettings.erodeNumberOfIterations=imgProcSettings.erodeNumberOfIterations;
    m_imgProcSettings.flipCode=imgProcSettings.flipCode;
    m_imgProcSettings.cannyThreshold1=imgProcSettings.cannyThreshold1;
    m_imgProcSettings.cannyThreshold2=imgProcSettings.cannyThreshold2;
    m_imgProcSettings.cannyApertureSize=imgProcSettings.cannyApertureSize;
    m_imgProcSettings.cannyL2gradient=imgProcSettings.cannyL2gradient;
}

void ProcessingThread::setROI(QRect roi)
{
    QMutexLocker locker(&m_processingMutex);
    m_currentROI.x = roi.x();
    m_currentROI.y = roi.y();
    m_currentROI.width = roi.width();
    m_currentROI.height = roi.height();
}

QRect ProcessingThread::getCurrentROI()
{
    return QRect(m_currentROI.x, m_currentROI.y, m_currentROI.width, m_currentROI.height);
}
