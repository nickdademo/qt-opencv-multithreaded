/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* MainWindow.cpp                                                       */
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

#include "CameraConnectDialog.h"
#include "Controller.h"
#include "MainWindow.h"

// Qt header files
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{  
    // Setup user interface
    setupUi(this);
    // Initially set controller to NULL
    controller=NULL;
    // Initialize processing flags
    grayscaleOn=false;
    smoothOn=false;
    dilateOn=false;
    erodeOn=false;
    flipOn=false;
    cannyOn=false;
    // Save application version in QString variable
    appVersion=QUOTE(APP_VERSION);
    // Connect GUI signals to slots
    connect(actionConnectToCamera, SIGNAL(triggered()), this, SLOT(connectToCamera()));
    connect(actionDisconnectCamera, SIGNAL(triggered()), this, SLOT(disconnectCamera()));
    connect(actionExit, SIGNAL(triggered()), this, SLOT(close()));
    connect(actionGrayscale, SIGNAL(toggled(bool)), this, SLOT(setGrayscale(bool)));
    connect(actionSmooth, SIGNAL(toggled(bool)), this, SLOT(setSmooth(bool)));
    connect(actionDilate, SIGNAL(toggled(bool)), this, SLOT(setDilate(bool)));
    connect(actionErode, SIGNAL(toggled(bool)), this, SLOT(setErode(bool)));
    connect(actionFlip, SIGNAL(toggled(bool)), this, SLOT(setFlip(bool)));
    connect(actionCanny, SIGNAL(toggled(bool)), this, SLOT(setCanny(bool)));
    connect(actionAbout, SIGNAL(triggered()), this, SLOT(about()));
    connect(pushButtonClearImageBuffer, SIGNAL(released()), this, SLOT(clearImageBuffer()));
    // Enable/disable appropriate menu items
    actionConnectToCamera->setDisabled(false);
    actionDisconnectCamera->setDisabled(true);
    menuProcessing->setDisabled(true);
    // Set GUI in main window
    actionGrayscale->setChecked(false);
    actionSmooth->setChecked(false);
    actionDilate->setChecked(false);
    actionErode->setChecked(false);
    actionFlip->setChecked(false);
    actionCanny->setChecked(false);
    qLabelFrame->setText("No camera connected.");
    progressBarBuffer->setValue(0);
    qLabelBuffer->setText("[000/000]");
    qLabelCaptureRate->setText("");
    qLabelProcessingRate->setText("");
    qLabelDeviceNumber->setText("");
    qLabelCameraResolution->setText("");
    qLabelROI->setText("");
    qLabelMouseCursorPos->setText("");
    pushButtonClearImageBuffer->setDisabled(true);
} // MainWindow constructor

MainWindow::~MainWindow()
{
    // Check if controller exists
    if(controller!=NULL)
    {
        // Disconnect queued connection
        disconnect(controller->processingThread,SIGNAL(newFrame(QImage)),this,SLOT(updateFrame(QImage)));
        // Stop processing thread
        if(controller->processingThread->isActive)
            controller->stopProcessingThread();
        // Stop capture thread
        if(controller->captureThread->isActive)
            controller->stopCaptureThread();
        // Clear image buffer
        controller->clearImageBuffer();
        // Check if threads have stopped
        if((controller->captureThread->isFinished())&&(controller->processingThread->isFinished()))
        {
            // Disconnect camera if connected
            if(controller->captureThread->capture!=NULL)
                controller->disconnectCamera();
            // Delete processing and capturethreads
            controller->deleteProcessingThread();
            controller->deleteCaptureThread();
        }
        // Delete controller
        delete controller;
        controller=NULL;
    }
} // MainWindow deconstructor

void MainWindow::connectToCamera()
{
    // Create dialog
    cameraConnectDialog = new CameraConnectDialog(this);
    // Prompt user
    // If user presses OK button on dialog, tell controller to connect to camera; else do nothing
    if(cameraConnectDialog->exec()==1)
    {
        // Store image buffer size in local variable
        imageBufferSize=cameraConnectDialog->getImageBufferSize();
        // Create controller
        controller = new Controller(cameraConnectDialog->getDeviceNumber(),imageBufferSize);
        // If camera was successfully connected
        if(controller->captureThread->capture!=NULL)
        {
            // Create queued connection between processing thread (emitter) and GUI thread (receiver)
            connect(controller->processingThread,SIGNAL(newFrame(QImage)),this,SLOT(updateFrame(QImage)),Qt::QueuedConnection);
            // Setup progressBarBuffer in main window with minimum and maximum values
            progressBarBuffer->setMinimum(0);
            progressBarBuffer->setMaximum(imageBufferSize);
            // Enable/disable appropriate menu items
            actionConnectToCamera->setDisabled(true);
            actionDisconnectCamera->setDisabled(false);
            menuProcessing->setDisabled(false);
            // Enable "Clear Image Buffer" push button in main window
            pushButtonClearImageBuffer->setDisabled(false);
            // Get input stream properties
            sourceWidth=controller->getInputSourceWidth();
            sourceHeight=controller->getInputSourceHeight();
            // Set text in qLabels in main window
            qLabelDeviceNumber->setNum(cameraConnectDialog->getDeviceNumber());
            qLabelCameraResolution->setText(QString::number(sourceWidth)+QString("x")+QString::number(sourceHeight));
        }
        // Display error dialog if camera connection is unsuccessful
        else
            QMessageBox::warning(this,"ERROR:","Could not connect to camera.");
    }
} // connectToCamera()

void MainWindow::disconnectCamera()
{
    // Check if controller exists
    if(controller!=NULL)
    {
        // Disconnect queued connection
        disconnect(controller->processingThread,SIGNAL(newFrame(QImage)),this,SLOT(updateFrame(QImage)));
        // Stop processing thread
        if(controller->processingThread->isActive)
            controller->stopProcessingThread();
        // Stop capture thread
        if(controller->captureThread->isActive)
            controller->stopCaptureThread();
        // Clear image buffer
        controller->clearImageBuffer();
        // Check if threads have stopped
        if((controller->captureThread->isFinished())&&(controller->processingThread->isFinished()))
        {
            // Disconnect camera if connected
            if(controller->captureThread->capture!=NULL)
                controller->disconnectCamera();
            // Delete processing and capture threads
            controller->deleteProcessingThread();
            controller->deleteCaptureThread();
        }
        // Delete controller
        delete controller;
        controller=NULL;
        // Enable/Disable appropriate menu items
        actionConnectToCamera->setDisabled(false);
        actionDisconnectCamera->setDisabled(true);
        menuProcessing->setDisabled(true);
        // Set GUI in main window
        actionGrayscale->setChecked(false);
        actionSmooth->setChecked(false);
        actionDilate->setChecked(false);
        actionErode->setChecked(false);
        actionFlip->setChecked(false);
        actionCanny->setChecked(false);
        qLabelFrame->setText("No camera connected.");
        progressBarBuffer->setValue(0);
        qLabelBuffer->setText("[000/000]");
        qLabelCaptureRate->setText("");
        qLabelProcessingRate->setText("");
        qLabelDeviceNumber->setText("");
        qLabelCameraResolution->setText("");
        qLabelROI->setText("");
        qLabelMouseCursorPos->setText("");
        pushButtonClearImageBuffer->setDisabled(true);
    }
    // Display error dialog if camera could not be disconnected
    else
        QMessageBox::warning(this,"ERROR:","Could not disconnect camera.");
} // disconnectCamera()

void MainWindow::about()
{
    QMessageBox::information(this,"About",QString("Written by Nick D'Ademo\n\nContact: nickdademo@gmail.com\nWebsite: www.nickdademo.com\n\nVersion: ")+appVersion);
} // about()

void MainWindow::clearImageBuffer()
{
    controller->clearImageBuffer();
} // clearImageBuffer()

void MainWindow::setGrayscale(bool input)
{
    // Not checked
    if(!input)
        grayscaleOn=false;
    // Checked
    else if(input)
        grayscaleOn=true;
} // setGrayscale()

void MainWindow::setSmooth(bool input)
{
    // Not checked
    if(!input)
        smoothOn=false;
    // Checked
    else if(input)
        smoothOn=true;
} // setSmooth()

void MainWindow::setDilate(bool input)
{
    // Not checked
    if(!input)
        dilateOn=false;
    // Checked
    else if(input)
        dilateOn=true;
} // setDilate()

void MainWindow::setErode(bool input)
{
    // Not checked
    if(!input)
        erodeOn=false;
    // Checked
    else if(input)
        erodeOn=true;
} // setErode()

void MainWindow::setFlip(bool input)
{
    // Not checked
    if(!input)
        flipOn=false;
    // Checked
    else if(input)
        flipOn=true;
} // setFlip()

void MainWindow::setCanny(bool input)
{
    // Not checked
    if(!input)
        cannyOn=false;
    // Checked
    else if(input)
        cannyOn=true;
} // setCanny()

void MainWindow::updateFrame(const QImage &frame)
{
    // Reset mouse event flags
    // The following checks ensure that every click/release pair, no matter how fast, is picked up by the processing thread
    if(controller->processingThread->qLabelFrame_mouseRightPressed&&qLabelFrame->getRightMouseButtonRelease())
        qLabelFrame->setRightMouseButtonPress(false);
    if(controller->processingThread->qLabelFrame_mouseLeftPressed&&qLabelFrame->getLeftMouseButtonRelease())
        qLabelFrame->setLeftMouseButtonPress(false);
    // Update mouse event flags
    controller->processingThread->qLabelFrame_mouseXPos=qLabelFrame->getMouseXPos();
    controller->processingThread->qLabelFrame_mouseYPos=qLabelFrame->getMouseYPos();
    controller->processingThread->qLabelFrame_mouseLeftPressed=qLabelFrame->getLeftMouseButtonPress();
    controller->processingThread->qLabelFrame_mouseRightPressed=qLabelFrame->getRightMouseButtonPress();
    controller->processingThread->qLabelFrame_mouseLeftReleased=qLabelFrame->getLeftMouseButtonRelease();
    controller->processingThread->qLabelFrame_mouseRightReleased=qLabelFrame->getRightMouseButtonRelease();
    // Update processing flags
    controller->processingThread->grayscaleOn=grayscaleOn;
    controller->processingThread->smoothOn=smoothOn;
    controller->processingThread->dilateOn=dilateOn;
    controller->processingThread->erodeOn=erodeOn;
    controller->processingThread->flipOn=flipOn;
    controller->processingThread->cannyOn=cannyOn;
    // Show statistics
    //// Show [number of images in buffer / image buffer size] in qLabelBuffer in main window
    qLabelBuffer->setText(QString("[")+QString::number(controller->processingThread->currentSizeOfBuffer)+
                          QString("/")+QString::number(imageBufferSize)+QString("]"));
    //// Show percentage of image bufffer full in progressBarBuffer in main window
    progressBarBuffer->setValue(controller->processingThread->currentSizeOfBuffer);
    //// Show processing rate in qLabelProcessingRate in main window
    qLabelCaptureRate->setNum(controller->captureThread->avgFPS);
    qLabelCaptureRate->setText(qLabelCaptureRate->text()+" fps");
    //// Show processing rate in qLabelProcessingRate in main window
    qLabelProcessingRate->setNum(controller->processingThread->avgFPS);
    qLabelProcessingRate->setText(qLabelProcessingRate->text()+" fps");
    //// Show ROI information in qLabelROI in main window
    qLabelROI->setText(QString("(")+QString::number(controller->processingThread->newROI.x)+QString(",")+
                       QString::number(controller->processingThread->newROI.y)+QString(") ")+
                       QString::number(controller->processingThread->newROI.width)+
                       QString("x")+QString::number(controller->processingThread->newROI.height));
    //// Show mouse cursor position in qLabelMouseCursorPos in main window
    qLabelMouseCursorPos->setText(QString("(")+QString::number(qLabelFrame->getMouseXPos())+
                                  QString(",")+QString::number(qLabelFrame->getMouseYPos())+
                                  QString(")"));
    // Display frame in main window
    qLabelFrame->setPixmap(QPixmap::fromImage(frame));
} // updateFrame()
