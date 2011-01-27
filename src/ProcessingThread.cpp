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
} // ProcessingThread deconstructor

void ProcessingThread::run()
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
            mutex2.lock();
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
            mutex2.unlock();
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
            // Update statistics
            updateFPS(processingTime);
            mutex4.lock();
            currentSizeOfBuffer=imageBuffer->getSizeOfImageBuffer();
            mutex4.unlock();
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
        mutex3.lock();
        avgFPS=fpsSum/16; // Calculate average FPS
        mutex3.unlock();
        fpsSum=0; // Reset sum
        sampleNo=0; // Reset sample number
    } // if
} // updateFPS()

void ProcessingThread::stopProcessingThread()
{
    mutex1.lock();
    stopped=true;
    mutex1.unlock();
} // stopProcessingThread()

void ProcessingThread::setROI()
{
    // Selection box can also be drawn from bottom-right to top-left corner
    if(box.width<0)
    {
        box.x+=box.width;
        box.width*=-1;
    } // if
    if(box.height<0)
    {
        box.y+=box.height;
        box.height*=-1;
    } // if

    // Check if selection box has positive dimensions
    if((box.width>0)&&((box.height)>0))
    {
        // Check if selection box is not outside window
        if((box.x<0)||(box.y<0)||((box.x+box.width)>inputSourceWidth)||((box.y+box.height)>inputSourceHeight))
            qDebug() << "ERROR: Selection box outside range. Please try again.";
        else
        {
            // Set area outside ROI in currentFrameCopy to blue
            cvSet(currentFrameCopy, cvScalar(127,0,0));
            // Set area outside ROI in currentFrameCopyGrayscale to gray
            cvSet(currentFrameCopyGrayscale, cvScalar(127,0,0));
            // Store new ROI in currentROI variable
            currentROI=box;
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

void ProcessingThread::updateProcessingFlags(struct ProcessingFlags p_flags)
{
    QMutexLocker locker(&mutex2);
    this->grayscaleOn=p_flags.grayscaleOn;
    this->smoothOn=p_flags.smoothOn;
    this->dilateOn=p_flags.dilateOn;
    this->erodeOn=p_flags.erodeOn;
    this->flipOn=p_flags.flipOn;
    this->cannyOn=p_flags.cannyOn;
} // updateProcessingFlags()

void ProcessingThread::updateProcessingSettings(struct ProcessingSettings p_settings)
{
    QMutexLocker locker(&mutex2);
    this->smoothType=p_settings.smoothType;
    this->smoothParam1=p_settings.smoothParam1;
    this->smoothParam2=p_settings.smoothParam2;
    this->smoothParam3=p_settings.smoothParam3;
    this->smoothParam4=p_settings.smoothParam4;
    this->dilateNumberOfIterations=p_settings.dilateNumberOfIterations;
    this->erodeNumberOfIterations=p_settings.erodeNumberOfIterations;
    this->flipMode=p_settings.flipMode;
    this->cannyThreshold1=p_settings.cannyThreshold1;
    this->cannyThreshold2=p_settings.cannyThreshold2;
    this->cannyApertureSize=p_settings.cannyApertureSize;
} // updateProcessingSettings()

void ProcessingThread::updateTaskData(struct TaskData taskData)
{
    QMutexLocker locker(&mutex2);
    this->setROIOn=taskData.setROIOn;
    this->resetROIOn=taskData.resetROIOn;
    this->box.x=taskData.selectionBox.left();
    this->box.y=taskData.selectionBox.top();
    this->box.width=taskData.selectionBox.width();
    this->box.height=taskData.selectionBox.height();
} // updateTaskData()

int ProcessingThread::getAvgFPS()
{
    QMutexLocker lock(&mutex3);
    return avgFPS;
} // getAvgFPS()

int ProcessingThread::getCurrentSizeOfBuffer()
{
    QMutexLocker lock(&mutex4);
    return currentSizeOfBuffer;
} // getCurrentSizeOfBuffer()

CvRect ProcessingThread::getCurrentROI()
{
    return currentROI;
} // getCurrentROI();
