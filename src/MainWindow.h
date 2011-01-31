/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* MainWindow.h                                                         */
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_MainWindow.h"
#include "Structures.h"

#define QUOTE_(x) #x
#define QUOTE(x) QUOTE_(x)

class CameraConnectDialog;
class ProcessingSettingsDialog;
class Controller;

class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
private:
    CameraConnectDialog *cameraConnectDialog;
    ProcessingSettingsDialog *processingSettingsDialog;
    Controller *controller;
    ProcessingFlags processingFlags;
    QString appVersion;
    int sourceWidth;
    int sourceHeight;
    int deviceNumber;
    int imageBufferSize;
public slots:
    void connectToCamera();
    void disconnectCamera();
    void about();
    void clearImageBuffer();
    void setGrayscale(bool);
    void setSmooth(bool);
    void setDilate(bool);
    void setErode(bool);
    void setFlip(bool);
    void setCanny(bool);
    void setProcessingSettings();
    void updateMouseCursorPosLabel();
private slots:
    void updateFrame(const QImage &frame);
signals:
    void newProcessingFlags(struct ProcessingFlags p_flags);
};

#endif // MAINWINDOW_H
