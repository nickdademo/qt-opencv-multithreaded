/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* ProcessingThead.h                                                    */
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

#ifndef PROCESSINGTHREAD_H
#define PROCESSINGTHREAD_H

// Qt header files
#include <QThread>
#include <QtGui>
// OpenCV header files
#include <opencv/highgui.h>

class ImageBuffer;

class ProcessingThread : public QThread
{
    Q_OBJECT

public:
    ProcessingThread(ImageBuffer *imageBuffer, int inputSourceWidth, int inputSourceHeight);
    ~ProcessingThread();
    void stopProcessingThread();
    volatile bool isActive;
    int avgFPS;
    int currentSizeOfBuffer;
    CvRect newROI;
    // MouseEventFlags structure definition
    struct MouseEventFlags{
        int mouseXPos;
        int mouseYPos;
        bool mouseLeftPressed;
        bool mouseRightPressed;
        bool mouseLeftReleased;
        bool mouseRightReleased;
    } frameLabel;
    // ProcessingFlags structure definition
    struct ProcessingFlags{
        bool grayscaleOn;
        bool smoothOn;
        bool dilateOn;
        bool erodeOn;
        bool flipOn;
        bool cannyOn;
    } processingFlags;
    // ProcessingSettings structure definition
    struct ProcessingSettings{
        int smoothType;
        int smoothParam1;
        int smoothParam2;
        double smoothParam3;
        double smoothParam4;
        int dilateNumberOfIterations;
        int erodeNumberOfIterations;
        int flipMode;
        double cannyThreshold1;
        double cannyThreshold2;
        int cannyApertureSize;
    } processingSettings;
private:
    void updateFPS(int);
    void drawBox(IplImage*,CvRect,int,int,int);
    void ROICalibration(bool);
    ImageBuffer *imageBuffer;
    int inputSourceWidth, inputSourceHeight;
    IplImage *currentFrameCopy, *currentFrameCopyGrayscale;
    CvRect box, originalROI;
    QImage frame;
    QTime t;
    int processingTime;
    QQueue<int> fps;
    int fpsSum;
    int sampleNo;
    // Other flags
    bool drawingBox;
    bool resetROI;
protected:
    void run();
signals:
    void newFrame(const QImage &frame);
};

#endif // PROCESSINGTHREAD_H
