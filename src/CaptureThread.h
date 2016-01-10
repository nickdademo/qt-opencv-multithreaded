/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* CaptureThread.h                                                      */
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

#ifndef CAPTURETHREAD_H
#define CAPTURETHREAD_H

#include <QThread>
#include <QTime>
#include <QQueue>

#include <opencv2/opencv.hpp>

#include "ThreadStatistics.h"

class SharedImageBuffer;

class CaptureThread : public QThread
{
    Q_OBJECT

    public:
        CaptureThread(SharedImageBuffer *sharedImageBuffer, int deviceNumber, bool dropFrameIfBufferFull, int width, int height);
        void stop();
        bool connectToCamera();
        void disconnectCamera();
        bool isCameraConnected();
        cv::VideoCapture& videoCapture()
        {
            return m_cap;
        }

    private:
        void updateFps(int timeElapsed);
        SharedImageBuffer *m_sharedImageBuffer;
        cv::VideoCapture m_cap;
        cv::Mat m_grabbedFrame;
        QTime m_time;
        QMutex m_doStopMutex;
        QQueue<int> m_fps;
        ThreadStatistics m_statistics;
        volatile bool m_doStop;
        int m_captureTime;
        int m_sampleNumber;
        int m_fpsSum;
        bool m_dropFrameIfBufferFull;
        int m_deviceNumber;
        int m_width;
        int m_height;

    protected:
        void run();

    signals:
        void newStatistics(ThreadStatistics statistics);
};

#endif // CAPTURETHREAD_H
