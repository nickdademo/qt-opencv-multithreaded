/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* SharedImageBuffer.h                                                  */
/*                                                                      */
/* Nick D'Ademo <nickdademo@gmail.com>                                  */
/*                                                                      */
/* Copyright (c) 2012-2016 Nick D'Ademo                                 */
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

#ifndef SHAREDIMAGEBUFFER_H
#define SHAREDIMAGEBUFFER_H

#include <QHash>
#include <QSet>
#include <QWaitCondition>
#include <QMutex>
#include <QObject>
#include <QSemaphore>

#include <opencv2/opencv.hpp>

#include "Buffer.h"

class SharedImageBuffer : public QObject
{
    Q_OBJECT

    public:
        SharedImageBuffer();
        void add(int deviceNumber, Buffer<cv::Mat> *imageBuffer, bool run, bool captureThreadSync, bool processingThreadSync);
        Buffer<cv::Mat>* get(int deviceNumber);
        void remove(int deviceNumber);
        bool contains(int deviceNumber);

        void setStreamRunning(int deviceNumber, bool run);
        bool isStreamRunning(int deviceNumber);
        void streamControl(int deviceNumber);

        void captureSync(int deviceNumber);
        bool isCaptureSyncEnabled(int deviceNumber);
        void startCaptureSync();
        void stopCaptureSync();
        bool isCaptureSyncInProgress()
        {
            return m_doCaptureSync;
        }

        void processingSync(int deviceNumber);
        bool isProcessingSyncEnabled(int deviceNumber);
        void startProcessingSync();
        void stopProcessingSync();
        bool isProcessingSyncInProgress()
        {
            return m_doProcessingSync;
        }

    private:
        void _setStreamRunning(int deviceNumber, bool run);

        QHash<int, Buffer<cv::Mat>*> m_imageBufferMap;
        QMutex m_imageBufferMutex;

        QHash<int, bool> m_streamControlMap;
        QHash<int, QSemaphore*> m_streamControlSemaphoreMap;
        QMutex m_streamControlMutex;

        QWaitCondition m_captureSyncWaitCondition;
        QSet<int> m_captureSyncSet;
        QMutex m_captureSyncMutex;
        bool m_doCaptureSync;
        int m_nArrivedCapture;

        QWaitCondition m_processingSyncWaitCondition;
        QSet<int> m_processingSyncSet;
        QMutex m_processingSyncMutex;
        bool m_doProcessingSync;
        int m_nArrivedProcessing;

    signals:
        void streamRun(int deviceNumber);
        void streamPaused(int deviceNumber);
        void captureSyncStarted();
        void captureSyncStopped();
        void processingSyncStarted();
        void processingSyncStopped();
};

#endif // SHAREDIMAGEBUFFER_H
