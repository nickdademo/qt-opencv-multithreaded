/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* SharedImageBuffer.cpp                                                */
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

#include "SharedImageBuffer.h"

SharedImageBuffer::SharedImageBuffer()
{
    // Initialize variables(s)
    nArrived=0;
    doSync=false;
}

void SharedImageBuffer::add(int deviceNumber, Buffer<Mat>* imageBuffer, bool sync)
{
    // Device stream is to be synchronized
    if(sync)
    {
        mutex.lock();
        syncSet.insert(deviceNumber);
        mutex.unlock();
    }
    // Add image buffer to map
    imageBufferMap[deviceNumber]=imageBuffer;
}

Buffer<Mat>* SharedImageBuffer::getByDeviceNumber(int deviceNumber)
{
    return imageBufferMap[deviceNumber];
}

void SharedImageBuffer::removeByDeviceNumber(int deviceNumber)
{
    // Remove buffer for device from imageBufferMap
    imageBufferMap.remove(deviceNumber);

    // Also remove from syncSet (if present)
    mutex.lock();
    if(syncSet.contains(deviceNumber))
    {
        syncSet.remove(deviceNumber);
        wc.wakeAll();
    }
    mutex.unlock();
}

void SharedImageBuffer::sync(int deviceNumber)
{
    // Only perform sync if enabled for specified device/stream
    mutex.lock();
    if(syncSet.contains(deviceNumber))
    {
        // Increment arrived count
        nArrived++;
        // We are the last to arrive: wake all waiting threads
        if(doSync && (nArrived==syncSet.size()))
            wc.wakeAll();
        // Still waiting for other streams to arrive: wait
        else
            wc.wait(&mutex);
        // Decrement arrived count
        nArrived--;
    }
    mutex.unlock();
}

void SharedImageBuffer::wakeAll()
{
    QMutexLocker locker(&mutex);
    wc.wakeAll();
}

void SharedImageBuffer::setSyncEnabled(bool enable)
{
    doSync=enable;
}

bool SharedImageBuffer::isSyncEnabledForDeviceNumber(int deviceNumber)
{
    return syncSet.contains(deviceNumber);
}

bool SharedImageBuffer::getSyncEnabled()
{
    return doSync;
}

bool SharedImageBuffer::containsImageBufferForDeviceNumber(int deviceNumber)
{
    return imageBufferMap.contains(deviceNumber);
}
