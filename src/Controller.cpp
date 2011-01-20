/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* Controller.cpp                                                       */
/*                                                                      */
/* Nick D'Ademo <nickdademo@gmail.com>                                  */
/*                                                                      */
/* Copyright (c) 2011 Nick D'Ademo                                      */
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

#include "Controller.h"
#include "ImageBuffer.h"

// Qt header files
#include <QtGui>

Controller::Controller(int deviceNumber, int imageBufferSize) : imageBufferSize(imageBufferSize)
{
    /*
    QThread::IdlePriority               0	scheduled only when no other threads are running.
    QThread::LowestPriority             1	scheduled less often than LowPriority.
    QThread::LowPriority                2	scheduled less often than NormalPriority.
    QThread::NormalPriority             3	the default priority of the operating system.
    QThread::HighPriority               4	scheduled more often than NormalPriority.
    QThread::HighestPriority            5	scheduled more often than HighPriority.
    QThread::TimeCriticalPriority	6	scheduled as often as possible.
    QThread::InheritPriority            7	use the same priority as the creating thread. This is the default.
    */

    // Create image buffer with user-defined size
    imageBuffer = new ImageBuffer(imageBufferSize);
    // Create capture thread with user-defined device number
    captureThread = new CaptureThread(imageBuffer, deviceNumber);
    // Only start threads if camera connection was successful
    if(captureThread->capture!=NULL)
    {
        // Start capturing frames from camera
        captureThread->start(QThread::IdlePriority);
        // Create processing thread
        processingThread = new ProcessingThread(imageBuffer,getInputSourceWidth(),getInputSourceHeight());
        // Start processing captured frames
        processingThread->start();
    }
    else
        qDebug() << "ERROR: Could not connect to camera.";
} // Controller constructor

Controller::~Controller()
{
    // Delete image buffer
    delete imageBuffer;
} // Controller deconstructor

void Controller::disconnectCamera()
{
    captureThread->disconnectCamera();
} // disconnectCamera()

void Controller::stopCaptureThread()
{
    qDebug() << "About to stop capture thread...";
    captureThread->stopCaptureThread();
    // Take one frame off a FULL queue to allow the capture thread to finish
    if(imageBuffer->getSizeOfImageBuffer()==imageBufferSize)
    {
        IplImage* temp;
        temp=imageBuffer->getFrame();
        cvReleaseImage(&temp);
    }
    captureThread->wait();
    qDebug() << "Capture thread successfully stopped.";
} // stopCaptureThread()

void Controller::stopProcessingThread()
{
    qDebug() << "About to stop processing thread...";
    processingThread->stopProcessingThread();
    processingThread->wait();
    qDebug() << "Processing thread successfully stopped.";
} // stopProcessingThread()

void Controller::deleteCaptureThread()
{
    // Delete thread
    delete captureThread;
} // deleteCaptureThread()

void Controller::deleteProcessingThread()
{
    // Delete thread
    delete processingThread;
} // deleteProcessingThread()

void Controller::clearImageBuffer()
{
    imageBuffer->clearBuffer();
} // clearImageBuffer()

int Controller::getInputSourceWidth()
{
    return cvGetCaptureProperty(captureThread->capture, CV_CAP_PROP_FRAME_WIDTH);
} // getInputSourceWidth()

int Controller::getInputSourceHeight()
{
    return cvGetCaptureProperty(captureThread->capture, CV_CAP_PROP_FRAME_HEIGHT);
} // getInputSourceHeight()
