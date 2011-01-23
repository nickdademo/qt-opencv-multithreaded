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
                                   : QThread(), imageBuffer(imageBuffer)
{
    // Create IplImages
    currentFrameCopy=cvCreateImage(cvSize(inputSourceWidth,inputSourceHeight),IPL_DEPTH_8U,3);
    currentFrameCopyGrayscale=cvCreateImage(cvSize(inputSourceWidth,inputSourceHeight),IPL_DEPTH_8U,1);
    // Initialize variables
    isActive=true;
    sampleNo=0;
    fpsSum=0;
    avgFPS=0;
    fps.clear();
    // Initialize MouseEventFlags structure
    frameLabel.mouseXPos=0;
    frameLabel.mouseYPos=0;
    frameLabel.mouseLeftPressed=false;
    frameLabel.mouseRightPressed=false;
    frameLabel.mouseLeftReleased=true;
    frameLabel.mouseRightReleased=true;
    // Initialize ProcessingFlags structure
    processingFlags.grayscaleOn=false;
    processingFlags.smoothOn=false;
    processingFlags.dilateOn=false;
    processingFlags.erodeOn=false;
    processingFlags.flipOn=false;
    processingFlags.cannyOn=false;
    // Initialize other flags
    drawingBox=false;
    resetROI=false;
    // Initialize newROI variable
    newROI=cvRect(0,0,inputSourceWidth,inputSourceHeight);
    // Store original ROI
    originalROI=newROI;
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
    while(isActive)
    {
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
            cvSetImageROI(currentFrame,newROI);
            // Make copy of current frame (processing will be performed on this copy)
            cvCopy(currentFrame,currentFrameCopy);

            /////////////////////////
            // HANDLE MOUSE EVENTS //
            /////////////////////////
            //  Start drawing box once left mouse button is pressed
            if(frameLabel.mouseLeftPressed&&!drawingBox)
            {
                // Reset ROI before setting a new ROI
                ROICalibration(true);
                // Set drawingBox flag
                drawingBox=true;
                box=cvRect(frameLabel.mouseXPos,frameLabel.mouseYPos,0,0);
            } // if
            // Set ROI reset flag
            else if(frameLabel.mouseRightPressed&&!resetROI)
                resetROI=true;
            // Continually resize selection box as mouse cursor moves with left mouse button held
            else if(frameLabel.mouseLeftPressed&&drawingBox)
            {
                box.width=frameLabel.mouseXPos-box.x;
                box.height=frameLabel.mouseYPos-box.y;
                // Draw selection box on currentFrameCopy
                drawBox(currentFrameCopy, box, 0xFF, 0x00, 0x00); // Colour = BLUE
            } // else if
            // Set new ROI on left mouse button release
            else if(frameLabel.mouseLeftReleased&&drawingBox)
                ROICalibration(false);
            // Reset to original ROI on right mouse button release
            else if(frameLabel.mouseRightReleased&&resetROI)
                ROICalibration(true);

            ////////////////////////////////////
            // PERFORM IMAGE PROCESSING BELOW //
            ////////////////////////////////////
            else
            {
                // Grayscale conversion
                if(processingFlags.grayscaleOn)
                    cvCvtColor(currentFrameCopy,currentFrameCopyGrayscale,CV_BGR2GRAY);
                // Smooth
                if(processingFlags.smoothOn)
                {
                    if(processingFlags.grayscaleOn)
                        cvSmooth(currentFrameCopyGrayscale,currentFrameCopyGrayscale,SMOOTH_TYPE,SMOOTH_PARAM_1,SMOOTH_PARAM_2,
                                 SMOOTH_PARAM_3,SMOOTH_PARAM_4);
                    else
                        cvSmooth(currentFrameCopy,currentFrameCopy,SMOOTH_TYPE,SMOOTH_PARAM_1,SMOOTH_PARAM_2,
                                 SMOOTH_PARAM_3,SMOOTH_PARAM_4);
                } // if
                // Dilate
                if(processingFlags.dilateOn)
                {
                    if(processingFlags.grayscaleOn)
                        cvDilate(currentFrameCopyGrayscale,currentFrameCopyGrayscale,NULL,DILATE_ITERATIONS);
                    else
                        cvDilate(currentFrameCopy,currentFrameCopy,NULL,DILATE_ITERATIONS);
                } // if
                // Erode
                if(processingFlags.erodeOn)
                {
                    if(processingFlags.grayscaleOn)
                        cvErode(currentFrameCopyGrayscale,currentFrameCopyGrayscale,NULL,ERODE_ITERATIONS);
                    else
                        cvErode(currentFrameCopy,currentFrameCopy,NULL,ERODE_ITERATIONS);
                } // if
                // Flip
                if(processingFlags.flipOn)
                {
                    if(processingFlags.grayscaleOn)
                        cvFlip(currentFrameCopyGrayscale,NULL,FLIPMODE);
                    else
                        cvFlip(currentFrameCopy,NULL,FLIPMODE);
                } // if
                // Canny edge detection
                if(processingFlags.cannyOn)
                {
                    if(processingFlags.grayscaleOn)
                        cvCanny(currentFrameCopyGrayscale,currentFrameCopyGrayscale,CANNY_THRESHOLD_1,CANNY_THRESHOLD_2);
                    else
                    {
                        // Frame must be converted to grayscale first
                        cvCvtColor(currentFrameCopy,currentFrameCopyGrayscale,CV_BGR2GRAY);
                        cvCanny(currentFrameCopyGrayscale,currentFrameCopyGrayscale,CANNY_THRESHOLD_1,CANNY_THRESHOLD_2);
                    } // else
                } // if
            } // else
            ////////////////////////////////////
            // PERFORM IMAGE PROCESSING ABOVE //
            ////////////////////////////////////

            //// Convert IplImage to QImage: Show grayscale frame
            //// (if either Grayscale or Canny processing modes are on AND a box is not being drawn).
            if((processingFlags.grayscaleOn&&!drawingBox)||(processingFlags.cannyOn&&!drawingBox))
                frame=IplImageToQImage(currentFrameCopyGrayscale);
            //// Convert IplImage to QImage: Show BGR frame
            else
                frame=IplImageToQImage(currentFrameCopy);
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
    isActive=false;
} // stopProcessingThread()

void ProcessingThread::drawBox(IplImage* img, CvRect box, int B, int G, int R)
{
    cvRectangle(img,cvPoint(box.x,box.y),cvPoint(box.x+box.width,box.y+box.height),cvScalar(B,G,R));
} // drawBox()

void ProcessingThread::ROICalibration(bool reset)
{
    // Reset to original ROI
    if(reset)
    {
        // Check if current ROI is the same as original ROI
        if((newROI.x==originalROI.x)&&(newROI.y==originalROI.y)&&(newROI.width==originalROI.width)&&(newROI.height==originalROI.height))
        {
            // Reset resetROI flag
            resetROI=false;
            qDebug() << "WARNING: Cannot reset ROI - already set to original ROI.";
        } // if
        else
        {
            // Reset ROI
            cvResetImageROI(currentFrameCopy);
            cvResetImageROI(currentFrameCopyGrayscale);
            // Set ROI back to original ROI
            newROI=originalROI;
            // Reset resetROI flag
            resetROI=false;
            qDebug() << "ROI successfully reset.";
        } // else
    } // if

    // Set new ROI
    else
    {
        // Selection box can also be drawn from bottom-right to top-left corner
        if(box.width<0)
        {
                 box.x+=box.width;
                 box.width *=-1;
        } // if
        if(box.height<0)
        {
                 box.y+=box.height;
                 box.height*=-1;
        } // if
        // Reset drawingBox flag
        drawingBox=false;
        // Check if selection box has positive dimensions
        if((box.width>0)&&((box.height)>0))
        {
            // Check if selection box is not outside window
            if((box.x<0)||(box.y<0)||((box.x+(box.width))>newROI.width)||((box.y+(box.height))>newROI.height))
                qDebug() << "ERROR: Selection box outside range. Please try again.";
            else
            {
                // Set area outside ROI in currentFrameCopy to blue
                cvSet(currentFrameCopy, cvScalar(127,0,0));
                // Set area outside ROI in currentFrameCopyGrayscale to gray
                cvSet(currentFrameCopyGrayscale, cvScalar(127,0,0));
                // Store new ROI in newROI variable
                newROI=box;
                // Set new ROI
                cvSetImageROI(currentFrameCopy, newROI);
                cvSetImageROI(currentFrameCopyGrayscale, newROI);
            } // else
         } // if
    } // else
} // ROICalibration()
