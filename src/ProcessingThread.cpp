/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* ProcessingThread.cpp                                                 */
/*                                                                      */
/* Nick D'Ademo <nickdademo@gmail.com>                                  */
/*                                                                      */
/* Copyright (c) 2012 Nick D'Ademo                                      */
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

#include "ImageBuffer.h"
#include "ProcessingThread.h"
#include "MatToQImage.h"

// Qt header files
#include <QDebug>
// OpenCV header files
#include <opencv/cv.h>
#include <opencv/highgui.h>
// Configuration header file
#include "Config.h"

ProcessingThread::ProcessingThread(ImageBuffer *imageBuffer, int inputSourceWidth, int inputSourceHeight) : QThread(),
                                   imageBuffer(imageBuffer),
                                   inputSourceWidth(inputSourceWidth),
                                   inputSourceHeight(inputSourceHeight)
{
    // Create images
    currentFrameCopy.create(Size(inputSourceWidth,inputSourceHeight),CV_8UC3);
    currentFrameCopyGrayscale.create(Size(inputSourceWidth,inputSourceHeight),CV_8UC1);
    // Initialize variables
    stopped=false;
    sampleNo=0;
    fpsSum=0;
    avgFPS=0;
    fps.clear();
    // Initialize currentROI variable
    currentROI=Rect(0,0,inputSourceWidth,inputSourceHeight);
    // Store original ROI
    originalROI=currentROI;
} // ProcessingThread constructor

ProcessingThread::~ProcessingThread()
{
} // ProcessingThread destructor

void ProcessingThread::run()
{
    while(1)
    {
        /////////////////////////////////
        // Stop thread if stopped=TRUE //
        /////////////////////////////////
        stoppedMutex.lock();
        if (stopped)
        {
            stopped=false;
            stoppedMutex.unlock();
            break;
        }
        stoppedMutex.unlock();
        /////////////////////////////////
        /////////////////////////////////

        // Save processing time
        processingTime=t.elapsed();
        // Start timer (used to calculate processing rate)
        t.start();
        // Get frame from queue
        Mat currentFrame=imageBuffer->getFrame();
        // Make copy of current frame (processing will be performed on this copy)
        currentFrame.copyTo(currentFrameCopy);
        // Set ROI of currentFrameCopy
        currentFrameCopy.locateROI(frameSize,framePoint);
        currentFrameCopy.adjustROI(-currentROI.y,-(frameSize.height-currentROI.height-currentROI.y),
                                   -currentROI.x,-(frameSize.width-currentROI.width-currentROI.x));

        updateMembersMutex.lock();
        ///////////////////
        // PERFORM TASKS //
        ///////////////////
        if(resetROIFlag)
            resetROI();
        else if(setROIFlag)
            setROI();
        ////////////////////////////////////
        // PERFORM IMAGE PROCESSING BELOW //
        ////////////////////////////////////
        else
        {
            // Grayscale conversion
            if(grayscaleOn)
                cvtColor(currentFrameCopy,currentFrameCopyGrayscale,CV_BGR2GRAY);
            // Smooth (in-place operations)
            if(smoothOn)
            {
                if(grayscaleOn)
                {
                    switch(smoothType)
                    {
                        // BLUR
                        case 0:
                            blur(currentFrameCopyGrayscale,currentFrameCopyGrayscale,Size(smoothParam1,smoothParam2));
                            break;
                        // GAUSSIAN
                        case 1:
                            GaussianBlur(currentFrameCopyGrayscale,currentFrameCopyGrayscale,Size(smoothParam1,smoothParam2),smoothParam3,smoothParam4);
                            break;
                        // MEDIAN
                        case 2:
                            medianBlur(currentFrameCopyGrayscale,currentFrameCopyGrayscale,smoothParam1);
                            break;
                    }
                }
                else
                {
                    switch(smoothType)
                    {
                        // BLUR
                        case 0:
                            blur(currentFrameCopy,currentFrameCopy,Size(smoothParam1,smoothParam2));
                            break;
                        // GAUSSIAN
                        case 1:
                            GaussianBlur(currentFrameCopy,currentFrameCopy,Size(smoothParam1,smoothParam2),smoothParam3,smoothParam4);
                            break;
                        // MEDIAN
                        case 2:
                            medianBlur(currentFrameCopy,currentFrameCopy,smoothParam1);
                            break;
                    }
                }
            }
            // Dilate
            if(dilateOn)
            {
                if(grayscaleOn)
                    dilate(currentFrameCopyGrayscale,currentFrameCopyGrayscale,Mat(),Point(-1,-1),dilateNumberOfIterations);
                else
                    dilate(currentFrameCopy,currentFrameCopy,Mat(),Point(-1,-1),dilateNumberOfIterations);
            }
            // Erode
            if(erodeOn)
            {
                if(grayscaleOn)
                    erode(currentFrameCopyGrayscale,currentFrameCopyGrayscale,Mat(),Point(-1,-1),erodeNumberOfIterations);
                else
                    erode(currentFrameCopy,currentFrameCopy,Mat(),Point(-1,-1),erodeNumberOfIterations);
            }
            // Flip
            if(flipOn)
            {
                if(grayscaleOn)
                    flip(currentFrameCopyGrayscale,currentFrameCopyGrayscale,flipCode);
                else
                    flip(currentFrameCopy,currentFrameCopy,flipCode);
            }
            // Canny edge detection
            if(cannyOn)
            {
                // Frame must be converted to grayscale first if grayscale conversion is OFF
                if(!grayscaleOn)
                    cvtColor(currentFrameCopy,currentFrameCopyGrayscale,CV_BGR2GRAY);

                Canny(currentFrameCopyGrayscale,currentFrameCopyGrayscale,
                      cannyThreshold1,cannyThreshold2,
                      cannyApertureSize,cannyL2gradient);
            }
        }
        ////////////////////////////////////
        // PERFORM IMAGE PROCESSING ABOVE //
        ////////////////////////////////////

        // Convert Mat to QImage: Show grayscale frame [if either Grayscale or Canny processing modes are ON]
        if(grayscaleOn||cannyOn)
            frame=MatToQImage(currentFrameCopyGrayscale);
        // Convert Mat to QImage: Show BGR frame
        else
            frame=MatToQImage(currentFrameCopy);
        updateMembersMutex.unlock();

        // Update statistics
        updateFPS(processingTime);
        currentSizeOfBuffer=imageBuffer->getSizeOfImageBuffer();
        // Inform GUI thread of new frame (QImage)
        emit newFrame(frame);
    }
    qDebug() << "Stopping processing thread...";
}

void ProcessingThread::updateFPS(int timeElapsed)
{
    // Add instantaneous FPS value to queue
    if(timeElapsed>0)
    {
        fps.enqueue((int)1000/timeElapsed);
        // Increment sample number
        sampleNo++;
    }
    // Maximum size of queue is DEFAULT_PROCESSING_FPS_STAT_QUEUE_LENGTH
    if(fps.size()>DEFAULT_PROCESSING_FPS_STAT_QUEUE_LENGTH)
        fps.dequeue();
    // Update FPS value every DEFAULT_PROCESSING_FPS_STAT_QUEUE_LENGTH samples
    if((fps.size()==DEFAULT_PROCESSING_FPS_STAT_QUEUE_LENGTH)&&(sampleNo==DEFAULT_PROCESSING_FPS_STAT_QUEUE_LENGTH))
    {
        // Empty queue and store sum
        while(!fps.empty())
            fpsSum+=fps.dequeue();
        // Calculate average FPS
        avgFPS=fpsSum/DEFAULT_PROCESSING_FPS_STAT_QUEUE_LENGTH;
        // Reset sum
        fpsSum=0;
        // Reset sample number
        sampleNo=0;
    }
} // updateFPS()

void ProcessingThread::stopProcessingThread()
{
    stoppedMutex.lock();
    stopped=true;
    stoppedMutex.unlock();
} // stopProcessingThread()

void ProcessingThread::setROI()
{
    // Save selection as new (current) ROI
    currentROI=selectionBox;
    qDebug() << "ROI successfully SET.";
    // Reset flag to FALSE
    setROIFlag=false;
} // setROI()

void ProcessingThread::resetROI()
{
    // Reset ROI to original
    currentROI=originalROI;
    qDebug() << "ROI successfully RESET.";
    // Reset flag to FALSE
    resetROIFlag=false;
} // resetROI()

void ProcessingThread::updateImageProcessingFlags(struct ImageProcessingFlags imageProcessingFlags)
{
    QMutexLocker locker(&updateMembersMutex);
    this->grayscaleOn=imageProcessingFlags.grayscaleOn;
    this->smoothOn=imageProcessingFlags.smoothOn;
    this->dilateOn=imageProcessingFlags.dilateOn;
    this->erodeOn=imageProcessingFlags.erodeOn;
    this->flipOn=imageProcessingFlags.flipOn;
    this->cannyOn=imageProcessingFlags.cannyOn;
} // updateImageProcessingFlags()

void ProcessingThread::updateImageProcessingSettings(struct ImageProcessingSettings imageProcessingSettings)
{
    QMutexLocker locker(&updateMembersMutex);
    this->smoothType=imageProcessingSettings.smoothType;
    this->smoothParam1=imageProcessingSettings.smoothParam1;
    this->smoothParam2=imageProcessingSettings.smoothParam2;
    this->smoothParam3=imageProcessingSettings.smoothParam3;
    this->smoothParam4=imageProcessingSettings.smoothParam4;
    this->dilateNumberOfIterations=imageProcessingSettings.dilateNumberOfIterations;
    this->erodeNumberOfIterations=imageProcessingSettings.erodeNumberOfIterations;
    this->flipCode=imageProcessingSettings.flipCode;
    this->cannyThreshold1=imageProcessingSettings.cannyThreshold1;
    this->cannyThreshold2=imageProcessingSettings.cannyThreshold2;
    this->cannyApertureSize=imageProcessingSettings.cannyApertureSize;
    this->cannyL2gradient=imageProcessingSettings.cannyL2gradient;
} // updateImageProcessingSettings()

void ProcessingThread::updateTaskData(struct TaskData taskData)
{
    QMutexLocker locker(&updateMembersMutex);
    this->setROIFlag=taskData.setROIFlag;
    this->resetROIFlag=taskData.resetROIFlag;
    this->selectionBox.x=taskData.selectionBox.left();
    this->selectionBox.y=taskData.selectionBox.top();
    this->selectionBox.width=taskData.selectionBox.width();
    this->selectionBox.height=taskData.selectionBox.height();
} // updateTaskData()

int ProcessingThread::getAvgFPS()
{
    return avgFPS;
} // getAvgFPS()

int ProcessingThread::getCurrentSizeOfBuffer()
{
    return currentSizeOfBuffer;
} // getCurrentSizeOfBuffer()

Rect ProcessingThread::getCurrentROI()
{
    return currentROI;
} // getCurrentROI();
