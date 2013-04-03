/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* CameraView.h                                                         */
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

#ifndef CAMERAVIEW_H
#define CAMERAVIEW_H

// Qt
#include <QMainWindow>
// Local
#include "CaptureThread.h"
#include "ProcessingThread.h"
#include "ImageBuffer.h"
#include "ImageProcessingSettingsDialog.h"
#include "Structures.h"

namespace Ui {
    class CameraView;
}

class CameraView : public QMainWindow
{
    Q_OBJECT
    
    public:
        explicit CameraView(QWidget *parent, int id);
        ~CameraView();
        bool connectToCamera(int deviceNumber, int imageBufferSize, bool dropFrame, int capThreadPrio, int procThreadPrio);

    private:
        Ui::CameraView *ui;
        ImageBuffer *imageBuffer;
        ProcessingThread *processingThread;
        CaptureThread *captureThread;
        ImageProcessingSettingsDialog *imageProcessingSettingsDialog;
        ImageProcessingFlags imageProcessingFlags;
        void stopCaptureThread();
        void stopProcessingThread();
        int id;
        bool isCameraConnected;

    public slots:
        void setImageProcessingSettings();
        void newMouseData(struct MouseData mouseData);
        void updateMouseCursorPosLabel();
        void clearImageBuffer();
        void setGrayscale(bool);
        void setSmooth(bool);
        void setDilate(bool);
        void setErode(bool);
        void setFlip(bool);
        void setCanny(bool);

    private slots:
        void updateFrame(const QImage &frame);

    signals:
        void newImageProcessingFlags(struct ImageProcessingFlags imageProcessingFlags);
        void setROI(QRect roi);
};

#endif // CAMERAVIEW_H
