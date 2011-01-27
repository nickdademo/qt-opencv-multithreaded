/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* MainWindow.cpp                                                       */
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

#include "CaptureThread.h"
#include "ImageBuffer.h"

// Qt header files
#include <QDebug>

CaptureThread::CaptureThread(ImageBuffer *buffer, int deviceNumber):QThread(), imageBuffer(buffer)
{
    // Open camera
    capture=cvCaptureFromCAM(deviceNumber);
    // Initialize variables
    stopped=false;
    sampleNo=0;
    fpsSum=0;
    avgFPS=0;
    fps.clear();
} // CaptureThread constructor

void CaptureThread::run()
{
    while(1)
    {
        /////////////////////////////////
        // Stop thread if stopped=TRUE //
        /////////////////////////////////
        mutex1.lock();
        if (stopped)
        {
            stopped=false;
            mutex1.unlock();
            break;
        }
        mutex1.unlock();
        /////////////////////////////////
        /////////////////////////////////
        // Save capture time
        captureTime=t.elapsed();
        // Start timer (used to calculate capture rate)
        t.start();
        // Capture and add frame to buffer
        imageBuffer->addFrame(cvQueryFrame(capture));
        // Update statistics
        mutex2.lock();
        updateFPS(captureTime);
        mutex2.unlock();
    }
    qDebug() << "Stopping capture thread...";
} // run()

void CaptureThread::disconnectCamera()
{
    // Disconnect camera if connected
    if(capture!=NULL)
    {
        cvReleaseCapture(&capture);
        if(capture==NULL)
            qDebug() << "Camera successfully disconnected.";
        else
            qDebug() << "ERROR: Camera could not be disconnected.";
    }
} // disconnectCamera()

void CaptureThread::updateFPS(int timeElapsed)
{
    // Add instantaneous FPS value to queue
    if(timeElapsed>0)
    {
        fps.enqueue((int)1000/timeElapsed);
        // Increment sample number
        sampleNo++;
    }
    // Maximum size of queue is 16
    if(fps.size() > 16)
        fps.dequeue();
    // Update FPS value every 16 samples
    if((fps.size()==16)&&(sampleNo==16))
    {
        // Empty queue and store sum
        while(!fps.empty())
            fpsSum+=fps.dequeue();
        avgFPS=fpsSum/16; // Calculate average FPS
        fpsSum=0; // Reset sum
        sampleNo=0; // Reset sample number
    }
} // updateFPS()

void CaptureThread::stopCaptureThread()
{
    mutex1.lock();
    stopped=true;
    mutex1.unlock();
} // stopCaptureThread()

int CaptureThread::getAvgFPS()
{
    QMutexLocker lock(&mutex2);
    return avgFPS;
} // getAvgFPS()

bool CaptureThread::isCameraConnected()
{
    if(capture!=NULL)
        return true;
    else
        return false;
} // isCameraConnected()

int CaptureThread::getInputSourceWidth()
{
    return cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH);
} // getInputSourceWidth()

int CaptureThread::getInputSourceHeight()
{
    return cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT);
} // getInputSourceHeight()
