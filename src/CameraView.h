/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* CameraView.h                                                         */
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

#ifndef CAMERAVIEW_H
#define CAMERAVIEW_H

#include <QWidget>
#include <QThread>

#include "ImageProcessing.h"
#include "ThreadStatistics.h"
#include "Buffer.h"
#include "SharedImageBuffer.h"

#include <opencv2/opencv.hpp>

class FrameLabel;
class ProcessingThread;
class CaptureThread;
class SharedImageBuffer;

class QProgressBar;
class QPushButton;
class QLabel;

class CameraView : public QWidget
{
    Q_OBJECT

    public:
        class Settings
        {
            public:
                int deviceNumber;
                int imageBufferSize;
                SharedImageBuffer::StreamControl streamControl;
                bool dropFrameIfBufferFull;
                QThread::Priority captureThreadPriority;
                QThread::Priority processingThreadPriority;
                bool enableFrameProcessing;
                int width;
                int height;
        };
        CameraView(Settings settings, SharedImageBuffer *sharedImageBuffer, QWidget *parent = 0);
        ~CameraView();
        bool connectToCamera();

    private:
        void stopCaptureThread();
        void stopProcessingThread();

        FrameLabel *m_frameLabel;
        QLabel *m_captureRateLabel;
        QLabel *m_nFramesCapturedLabel;
        QLabel *m_processingRateLabel;
        QLabel *m_nFramesProcessedLabel;
        QLabel *m_deviceNumberLabel;
        QLabel *m_cameraResolutionLabel;
        QLabel *m_roiLabel;
        QLabel *m_mouseCursorPosLabel;
        QProgressBar *m_imageBufferStatusProgressBar;
        QPushButton *m_clearImageBufferButton;
        QLabel *m_streamControlStatusLabel;
        QPushButton *m_streamControlRunButton;
        QPushButton *m_streamControlPauseButton;

        Buffer<cv::Mat> *m_imageBuffer;
        ProcessingThread *m_processingThread;
        CaptureThread *m_captureThread;
        SharedImageBuffer *m_sharedImageBuffer;
        ImageProcessing m_imageProcessing;
        Settings m_settings;
        QMetaObject::Connection m_newSelectionConnection;
        QMetaObject::Connection m_captureStatisticsConnection;
        QMetaObject::Connection m_processingStatisticsConnection;
        QMetaObject::Connection m_newFrameConnection;

    private slots:
        void onNewSelection(QRect box);
        void updateMouseCursorPosLabel();
        void clearImageBuffer();
        void updateFrame(const QImage &frame);
        void updateProcessingThreadStatistics(ThreadStatistics statistics);
        void updateCaptureThreadStatistics(ThreadStatistics statistics);
        void onContextMenuAction(QAction *action);
        void runStream();
        void pauseStream();
        void onStreamRun(int deviceNumber);
        void onStreamPaused(int deviceNumber);

    signals:
        void updateImageProcessing(ImageProcessing imageProcessing);
};

#endif // CAMERAVIEW_H
