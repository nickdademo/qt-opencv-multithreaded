/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* ImageBuffer.cpp                                                      */
/*                                                                      */
/* Nick D'Ademo <nickdademo@gmail.com>                                  */
/*                                                                      */
/* Copyright (c) 2012-2013 Nick D'Ademo                                 */
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

ImageBuffer::ImageBuffer(int bufferSize) : bufferSize(bufferSize)
{
    // Create semaphores
    freeSlots = new QSemaphore(bufferSize);
    usedSlots = new QSemaphore(0);
    clearBuffer1 = new QSemaphore(1);
    clearBuffer2 = new QSemaphore(1);
}

void ImageBuffer::addFrame(const Mat& frame, bool dropFrameIfFull)
{
    // Acquire semaphore
    clearBuffer1->acquire();
    // If frame dropping is enabled, do not block if buffer is full
    if(dropFrameIfFull)
    {
        // Try and acquire semaphore to add frame
        if(freeSlots->tryAcquire())
        {
            // Add frame to queue
            imageQueueProtect.lock();
            imageQueue.enqueue(frame);
            imageQueueProtect.unlock();
            // Release semaphore
            usedSlots->release();
        }
    }
    // If buffer is full, wait on semaphore
    else
    {
        // Acquire semaphore
        freeSlots->acquire();
        // Add frame to queue
        imageQueueProtect.lock();
        imageQueue.enqueue(frame);
        imageQueueProtect.unlock();
        // Release semaphore
        usedSlots->release();
    }
    // Release semaphore
    clearBuffer1->release();
}

Mat ImageBuffer::getFrame()
{
    // Local variable(s)
    Mat frame;
    // Acquire semaphores
    clearBuffer2->acquire();
    usedSlots->acquire();
    // Take frame from queue
    imageQueueProtect.lock();
    frame=imageQueue.dequeue();
    imageQueueProtect.unlock();
    // Release semaphores
    freeSlots->release();
    clearBuffer2->release();
    // Return frame to caller
    return frame.clone();
}

bool ImageBuffer::clear()
{
    // Check if buffer contains frames
    if(imageQueue.size()>0)
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
        // Clear buffer
        imageQueue.clear();
        // Release all slots
        freeSlots->release(bufferSize);
        // Allow getFrame() to resume
        clearBuffer2->release();
        // Allow addFrame() to resume
        clearBuffer1->release();
        return true;
    }
    else
        return false;
}

int ImageBuffer::getSize()
{
    return imageQueue.size();
}

int ImageBuffer::getMaxSize()
{
    return bufferSize;
}
