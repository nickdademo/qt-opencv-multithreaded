/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* CameraView.h                                                         */
/*                                                                      */
/* Nick D'Ademo <nickdademo@gmail.com>                                  */
/*                                                                      */
/* Copyright (c) 2012-2015 Nick D'Ademo                                 */
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

#include "Structures.h"

namespace Ui {
    class CameraView;
}

class ProcessingThread;
class CaptureThread;
class SharedImageBuffer;
class ImageProcessingSettingsDialog;

class CameraView : public QWidget
{
    Q_OBJECT

    public:
        explicit CameraView(int deviceNumber, SharedImageBuffer *sharedImageBuffer, QWidget *parent = 0);
        ~CameraView();
        bool connectToCamera(bool dropFrame, int capThreadPrio, int procThreadPrio, bool createProcThread, int width, int height);

    private:
        void stopCaptureThread();
        void stopProcessingThread();
        Ui::CameraView *ui;
        int m_deviceNumber;
        bool m_isCameraConnected;
        ProcessingThread *m_processingThread;
        CaptureThread *m_captureThread;
        SharedImageBuffer *m_sharedImageBuffer;
        ImageProcessingSettingsDialog *m_imageProcessingSettingsDialog;
        ImageProcessingFlags m_imageProcessingFlags;

    public slots:
        void setImageProcessingSettings();
        void newMouseData(MouseData mouseData);
        void updateMouseCursorPosLabel();
        void clearImageBuffer();

    private slots:
        void updateFrame(const QImage &frame);
        void updateProcessingThreadStats(ThreadStatisticsData statData);
        void updateCaptureThreadStats(ThreadStatisticsData statData);
        void handleContextMenuAction(QAction *action);

    signals:
        void newImageProcessingFlags(ImageProcessingFlags imageProcessingFlags);
        void setROI(QRect roi);
};

#endif // CAMERAVIEW_H
