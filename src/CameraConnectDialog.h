/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* CameraConnectDialog.h                                                */
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

#ifndef CAMERACONNECTDIALOG_H
#define CAMERACONNECTDIALOG_H

// Qt
#include <QtWidgets>
// Local
#include "Config.h"

namespace Ui {
class CameraConnectDialog;
}

class CameraConnectDialog : public QDialog
{
    Q_OBJECT
    
    public:
        explicit CameraConnectDialog(QWidget *parent = 0);
        ~CameraConnectDialog();
        void setDeviceNumber();
        void setImageBufferSize();
        int getDeviceNumber();
        int getImageBufferSize();
        bool getDropFrameCheckBoxState();
        int getCaptureThreadPrio();
        int getProcessingThreadPrio();
        QString getTabLabel();

    private:
        Ui::CameraConnectDialog *ui;

    public slots:
        void resetToDefaults();
};

#endif // CAMERACONNECTDIALOG_H
