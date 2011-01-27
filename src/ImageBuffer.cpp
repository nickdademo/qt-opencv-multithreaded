/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* ImageBuffer.cpp                                                      */
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

// Qt header files
#include <QDebug>

ImageBuffer::ImageBuffer(int bufferSize) : bufferSize(bufferSize)
{
    // Semaphore initializations
    freeSlots = new QSemaphore(bufferSize);
    usedSlots = new QSemaphore(0);
    clearBuffer1 = new QSemaphore(1);
    clearBuffer2 = new QSemaphore(1);
} // ImageBuffer constructor

void ImageBuffer::addFrame(const IplImage* image)
{
    clearBuffer1->acquire();
    freeSlots->acquire();
    // Copy the input IplImage
    IplImage* temp = cvCloneImage(image);
    // Add image to queue
    mutex.lock();
    imageQueue.enqueue(temp);
    mutex.unlock();
    clearBuffer1->release();
    usedSlots->release();
} // addFrame()

IplImage* ImageBuffer::getFrame()
{
    clearBuffer2->acquire();
    usedSlots->acquire();
    IplImage* temp=0;
    // Take image from queue
    mutex.lock();
    temp=imageQueue.dequeue();
    mutex.unlock();
    freeSlots->release();
    clearBuffer2->release();
    // Return image to caller
    return temp;
} // getFrame()

void ImageBuffer::clearBuffer()
{
    // Check if buffer is not empty
    if(imageQueue.size()!=0)
    {
        // Stop adding frames to buffer
        clearBuffer1->acquire();
        // Stop taking frames from buffer
        clearBuffer2->acquire();
        // Release all remaining slots in queue
        freeSlots->release(imageQueue.size());
        // Acquire all queue slots
        freeSlots->acquire(bufferSize);
        // Reset usedSlots to zero
        usedSlots->acquire(imageQueue.size());
        // Clear buffer by dequeuing items
        while(imageQueue.size()!=0)
        {
            // Local temporary storage
            IplImage* temp;
            // Dequeue IplImage
            temp=imageQueue.dequeue();
            // Release IplImage
            cvReleaseImage(&temp);
        }
        // Release all slots
        freeSlots->release(bufferSize);
        // Allow getFrame() to resume
        clearBuffer2->release();
        // Allow addFrame() to resume
        clearBuffer1->release();
        qDebug() << "Image buffer successfully cleared.";
    }
    else
        qDebug() << "WARNING: Could not clear image buffer: already empty.";
} // clearBuffer()

int ImageBuffer::getSizeOfImageBuffer()
{
    return imageQueue.size();
} // getSizeOfImageBuffer()
