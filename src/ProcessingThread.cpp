/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* ProcessingThread.cpp                                                 */
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

#include "ImageBuffer.h"
#include "ProcessingThread.h"
#include "ShowIplImage.h"

// Qt header files
#include <QDebug>
// OpenCV header files
#include <opencv/cv.h>
#include <opencv/highgui.h>
// Header file containing default values
#include "DefaultValues.h"

ProcessingThread::ProcessingThread(ImageBuffer *imageBuffer, int inputSourceWidth, int inputSourceHeight)
                                   : QThread(), imageBuffer(imageBuffer), inputSourceWidth(inputSourceWidth),
                                   inputSourceHeight(inputSourceHeight)
{
    // Create IplImages
    currentFrameCopy=cvCreateImage(cvSize(inputSourceWidth,inputSourceHeight),IPL_DEPTH_8U,3);
    currentFrameCopyGrayscale=cvCreateImage(cvSize(inputSourceWidth,inputSourceHeight),IPL_DEPTH_8U,1);
    // Initialize variables
    stopped=false;
    sampleNo=0;
    fpsSum=0;
    avgFPS=0;
    fps.clear();
    // Initialize processing flags
    grayscaleOn=false;
    smoothOn=false;
    dilateOn=false;
    erodeOn=false;
    flipOn=false;
    cannyOn=false;
    // Initialize task flags
    setROIOn=false;
    resetROIOn=false;
    // Initialize processing settings
    smoothType=DEFAULT_SMOOTH_TYPE;
    smoothParam1=DEFAULT_SMOOTH_PARAM_1;
    smoothParam2=DEFAULT_SMOOTH_PARAM_2;
    smoothParam3=DEFAULT_SMOOTH_PARAM_3;
    smoothParam4=DEFAULT_SMOOTH_PARAM_4;
    dilateNumberOfIterations=DEFAULT_DILATE_ITERATIONS;
    erodeNumberOfIterations=DEFAULT_ERODE_ITERATIONS;
    flipMode=DEFAULT_FLIP_MODE;
    cannyThreshold1=DEFAULT_CANNY_THRESHOLD_1;
    cannyThreshold2=DEFAULT_CANNY_THRESHOLD_2;
    cannyApertureSize=DEFAULT_CANNY_APERTURE_SIZE;
    // Initialize currentROI variable
    currentROI=cvRect(0,0,inputSourceWidth,inputSourceHeight);
    // Store original ROI
    originalROI=currentROI;
} // ProcessingThread constructor

ProcessingThread::~ProcessingThread()
{
    // Free IplImages (if they exist)
    if(currentFrameCopy!=NULL)
        cvReleaseImage(&currentFrameCopy);
    if(currentFrameCopyGrayscale!=NULL)
        cvReleaseImage(&currentFrameCopyGrayscale);
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
        IplImage* currentFrame = imageBuffer->getFrame();
        // Check that grabbed frame is not a NULL image
        if(currentFrame!=NULL)
        {
            // Set ROI of grabbed frame
            cvSetImageROI(currentFrame,currentROI);
            // Make copy of current frame (processing will be performed on this copy)
            cvCopy(currentFrame,currentFrameCopy);
            ///////////////////
            // PERFORM TASKS //
            ///////////////////
            updateMembersMutex.lock();
            if(resetROIOn)
                resetROI();
            else if(setROIOn)
                setROI();
            ////////////////////////////////////
            // PERFORM IMAGE PROCESSING BELOW //
            ////////////////////////////////////
            else
            {
                // Grayscale conversion
                if(grayscaleOn)
                    cvCvtColor(currentFrameCopy,currentFrameCopyGrayscale,CV_BGR2GRAY);
                // Smooth
                if(smoothOn)
                {
                    if(grayscaleOn)
                        cvSmooth(currentFrameCopyGrayscale,currentFrameCopyGrayscale,
                                 smoothType,smoothParam1,smoothParam2,smoothParam3,smoothParam4);
                    else
                        cvSmooth(currentFrameCopy,currentFrameCopy,
                                 smoothType,smoothParam1,smoothParam2,smoothParam3,smoothParam4);
                } // if
                // Dilate
                if(dilateOn)
                {
                    if(grayscaleOn)
                        cvDilate(currentFrameCopyGrayscale,currentFrameCopyGrayscale,NULL,
                                 dilateNumberOfIterations);
                    else
                        cvDilate(currentFrameCopy,currentFrameCopy,NULL,
                                 dilateNumberOfIterations);
                } // if
                // Erode
                if(erodeOn)
                {
                    if(grayscaleOn)
                        cvErode(currentFrameCopyGrayscale,currentFrameCopyGrayscale,NULL,
                                erodeNumberOfIterations);
                    else
                        cvErode(currentFrameCopy,currentFrameCopy,NULL,
                                erodeNumberOfIterations);
                } // if
                // Flip
                if(flipOn)
                {
                    if(grayscaleOn)
                        cvFlip(currentFrameCopyGrayscale,NULL,flipMode);
                    else
                        cvFlip(currentFrameCopy,NULL,flipMode);
                } // if
                // Canny edge detection
                if(cannyOn)
                {
                    // Frame must be converted to grayscale first if grayscale conversion is OFF
                    if(!grayscaleOn)
                        cvCvtColor(currentFrameCopy,currentFrameCopyGrayscale,CV_BGR2GRAY);

                    cvCanny(currentFrameCopyGrayscale,currentFrameCopyGrayscale,
                            cannyThreshold1,cannyThreshold2,
                            cannyApertureSize);
                } // if
            } // else
            ////////////////////////////////////
            // PERFORM IMAGE PROCESSING ABOVE //
            ////////////////////////////////////

            //// Convert IplImage to QImage: Show grayscale frame
            //// (if either Grayscale or Canny processing modes are ON)
            if(grayscaleOn||cannyOn)
                frame=IplImageToQImage(currentFrameCopyGrayscale);
            //// Convert IplImage to QImage: Show BGR frame
            else
                frame=IplImageToQImage(currentFrameCopy);
            updateMembersMutex.unlock();
            // Update statistics
            updateFPS(processingTime);
            currentSizeOfBuffer=imageBuffer->getSizeOfImageBuffer();
            // Inform controller of new frame (QImage)
            emit newFrame(frame);
            // Release IplImage
            if(currentFrame!=NULL)
                cvReleaseImage(&currentFrame);
        } // if
        else
            qDebug() << "ERROR: Processing thread received a NULL image.";
    } // while
    qDebug() << "Stopping processing thread...";
} // run()

void ProcessingThread::updateFPS(int timeElapsed)
{
    // Add instantaneous FPS value to queue
    if(timeElapsed>0)
    {
        fps.enqueue((int)1000/timeElapsed);
        // Increment sample number
        sampleNo++;
    } // if
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
    } // if
} // updateFPS()

void ProcessingThread::stopProcessingThread()
{
    stoppedMutex.lock();
    stopped=true;
    stoppedMutex.unlock();
} // stopProcessingThread()

void ProcessingThread::setROI()
{
    // Selection box can also be drawn from bottom-right to top-left corner
    if(selectionBox.width<0)
    {
        selectionBox.x+=selectionBox.width;
        selectionBox.width*=-1;
    } // if
    if(selectionBox.height<0)
    {
        selectionBox.y+=selectionBox.height;
        selectionBox.height*=-1;
    } // if

    // Check if selection box has positive dimensions
    if((selectionBox.width>0)&&((selectionBox.height)>0))
    {
        // Check if selection box is not outside window
        if((selectionBox.x<0)||(selectionBox.y<0)||((selectionBox.x+selectionBox.width)>inputSourceWidth)||((selectionBox.y+selectionBox.height)>inputSourceHeight))
            qDebug() << "ERROR: Selection box outside range. Please try again.";
        else
        {
            // Set area outside ROI in currentFrameCopy to blue
            cvSet(currentFrameCopy, cvScalar(127,0,0));
            // Set area outside ROI in currentFrameCopyGrayscale to gray
            cvSet(currentFrameCopyGrayscale, cvScalar(127,0,0));
            // Store new ROI in currentROI variable
            currentROI=selectionBox;
            // Set new ROI
            cvSetImageROI(currentFrameCopy, currentROI);
            cvSetImageROI(currentFrameCopyGrayscale, currentROI);
            qDebug() << "ROI successfully SET.";
        } // else
     } // if
    // Reset setROIOn flag to FALSE
    setROIOn=false;
} // setROI()

void ProcessingThread::resetROI()
{
    // Check if current ROI is the same as original ROI
    if((currentROI.x==originalROI.x)&&(currentROI.y==originalROI.y)&&
       (currentROI.width==originalROI.width)&&(currentROI.height==originalROI.height))
        qDebug() << "WARNING: Cannot reset ROI: already set to original ROI.";
    else
    {
        // Reset ROI
        cvResetImageROI(currentFrameCopy);
        cvResetImageROI(currentFrameCopyGrayscale);
        // Set ROI back to original ROI
        currentROI=originalROI;
        qDebug() << "ROI successfully RESET.";
    } // else
    // Reset resetROIOn flag to FALSE
    resetROIOn=false;
} // resetROI()

void ProcessingThread::updateProcessingFlags(struct ProcessingFlags processingFlags)
{
    QMutexLocker locker(&updateMembersMutex);
    this->grayscaleOn=processingFlags.grayscaleOn;
    this->smoothOn=processingFlags.smoothOn;
    this->dilateOn=processingFlags.dilateOn;
    this->erodeOn=processingFlags.erodeOn;
    this->flipOn=processingFlags.flipOn;
    this->cannyOn=processingFlags.cannyOn;
} // updateProcessingFlags()

void ProcessingThread::updateProcessingSettings(struct ProcessingSettings processingSettings)
{
    QMutexLocker locker(&updateMembersMutex);
    this->smoothType=processingSettings.smoothType;
    this->smoothParam1=processingSettings.smoothParam1;
    this->smoothParam2=processingSettings.smoothParam2;
    this->smoothParam3=processingSettings.smoothParam3;
    this->smoothParam4=processingSettings.smoothParam4;
    this->dilateNumberOfIterations=processingSettings.dilateNumberOfIterations;
    this->erodeNumberOfIterations=processingSettings.erodeNumberOfIterations;
    this->flipMode=processingSettings.flipMode;
    this->cannyThreshold1=processingSettings.cannyThreshold1;
    this->cannyThreshold2=processingSettings.cannyThreshold2;
    this->cannyApertureSize=processingSettings.cannyApertureSize;
} // updateProcessingSettings()

void ProcessingThread::updateTaskData(struct TaskData taskData)
{
    QMutexLocker locker(&updateMembersMutex);
    this->setROIOn=taskData.setROIOn;
    this->resetROIOn=taskData.resetROIOn;
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

CvRect ProcessingThread::getCurrentROI()
{
    return currentROI;
} // getCurrentROI();
