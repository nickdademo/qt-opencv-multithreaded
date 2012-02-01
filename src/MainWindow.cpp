/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* MainWindow.cpp                                                       */
/*                                                                      */
/* Nick D'Ademo <nickdademo@gmail.com>                                  */
/*                                                                      */
/* Copyright (c) 2012 Nick D'Ademo                                      */
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
#include "ImageProcessingSettingsDialog.h"
#include "Controller.h"
#include "MainWindow.h"

// Qt header files
#include <QDebug>
// Configuration header file
#include "Config.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{  
    // Setup user interface
    setupUi(this);
    // Create controller
    controller = new Controller;
    // Save application version in QString variable
    appVersion=QUOTE(APP_VERSION);
    // Initialize data structures
    initializeImageProcessingFlagsStructure();
    initializeTaskDataStructure();
    // Initialize GUI
    setInitialGUIState();
    // Connect signals to slots
    signalSlotsInit();
    // Initialize flag
    isCameraConnected=false;
} // MainWindow constructor

MainWindow::~MainWindow()
{
    // Disconnect camera if connected
    if(isCameraConnected)
        controller->disconnectCamera();
} // MainWindow destructor

void MainWindow::connectToCamera()
{
    // Create CameraConnectDialog instance
    cameraConnectDialog = new CameraConnectDialog(this);
    // Create ProcessingSettingsDialog instance
    imageProcessingSettingsDialog = new ImageProcessingSettingsDialog(this);
    // PROMPT USER:
    // If user presses OK button on dialog, connect to camera; else do nothing
    if(cameraConnectDialog->exec()==1)
    {
        // Set private member variables in cameraConnectDialog to values in dialog
        cameraConnectDialog->setDeviceNumber();
        cameraConnectDialog->setImageBufferSize();
        // Store image buffer size in local variable
        imageBufferSize=cameraConnectDialog->getImageBufferSize();
        // Store device number in local variable
        deviceNumber=cameraConnectDialog->getDeviceNumber();
        // Connect to camera
        if((isCameraConnected=controller->connectToCamera(deviceNumber,imageBufferSize,cameraConnectDialog->getDropFrameCheckBoxState())))
        {
            // Create connection between frameLabel (emitter) and GUI thread (receiver/listener)
            connect(this->frameLabel,SIGNAL(newMouseData(struct MouseData)),this,SLOT(newMouseData(struct MouseData)));
            // Create connection between processing thread (emitter) and GUI thread (receiver/listener)
            connect(controller->processingThread,SIGNAL(newFrame(QImage)),this,SLOT(updateFrame(QImage)));
            // Create connections (3) between GUI thread (emitter) and processing thread (receiver/listener)
            connect(this->imageProcessingSettingsDialog,SIGNAL(newImageProcessingSettings(struct ImageProcessingSettings)),controller->processingThread,SLOT(updateImageProcessingSettings(struct ImageProcessingSettings)));
            connect(this,SIGNAL(newImageProcessingFlags(struct ImageProcessingFlags)),controller->processingThread,SLOT(updateImageProcessingFlags(struct ImageProcessingFlags)));
            connect(this,SIGNAL(newTaskData(struct TaskData)),controller->processingThread,SLOT(updateTaskData(struct TaskData)));
            // Initialize data structures
            initializeImageProcessingFlagsStructure();
            initializeTaskDataStructure();
            // Set data to defaults in processingThread
            emit newImageProcessingFlags(imageProcessingFlags);
            emit newTaskData(taskData);
            // Setup imageBufferBar in main window with minimum and maximum values
            imageBufferBar->setMinimum(0);
            imageBufferBar->setMaximum(imageBufferSize);
            // Enable/Disable appropriate GUI items
            connectToCameraAction->setEnabled(false);
            disconnectCameraAction->setEnabled(true);
            imageProcessingSettingsAction->setEnabled(true);
            imageProcessingMenu->setEnabled(true);
            // Enable "Clear Image Buffer" push button in main window
            clearImageBufferButton->setEnabled(true);
            // Get input stream properties
            sourceWidth=controller->captureThread->getInputSourceWidth();
            sourceHeight=controller->captureThread->getInputSourceHeight();
            // Set text in labels in main window
            deviceNumberLabel->setNum(deviceNumber);
            cameraResolutionLabel->setText(QString::number(sourceWidth)+QString("x")+QString::number(sourceHeight));
        }
        // Display error dialog if camera connection is unsuccessful
        else
        {
            QMessageBox::warning(this,"ERROR:","Could not connect to camera.");
            // Delete dialogs
            delete cameraConnectDialog;
            delete imageProcessingSettingsDialog;
        }
    }
} // connectToCamera()

void MainWindow::disconnectCamera()
{
    // Check if camera is connected
    if(controller->captureThread->isCameraConnected())
    {
        // Disconnect connections (5)
        disconnect(this->frameLabel,SIGNAL(newMouseData(struct MouseData)),this,SLOT(newMouseData(struct MouseData)));
        disconnect(controller->processingThread,SIGNAL(newFrame(QImage)),0,0);
        disconnect(this->imageProcessingSettingsDialog,SIGNAL(newImageProcessingSettings(struct ImageProcessingSettings)),controller->processingThread,SLOT(updateImageProcessingSettings(struct ImageProcessingSettings)));
        disconnect(this,SIGNAL(newImageProcessingFlags(struct ImageProcessingFlags)),controller->processingThread,SLOT(updateImageProcessingFlags(struct ImageProcessingFlags)));
        disconnect(this,SIGNAL(newTaskData(struct TaskData)),controller->processingThread,SLOT(updateTaskData(struct TaskData)));
        // Delete dialogs
        delete cameraConnectDialog;
        delete imageProcessingSettingsDialog;
        // Reset flag
        isCameraConnected=false;
        // Set GUI
        setInitialGUIState();
        // Disconnect camera
        controller->disconnectCamera();
    }
    // Display error dialog
    else
        QMessageBox::warning(this,"ERROR:","Camera already disconnected.");
} // disconnectCamera()

void MainWindow::about()
{
    QMessageBox::information(this,"About",QString("Created by Nick D'Ademo\n\nContact: nickdademo@gmail.com\nWebsite: www.nickdademo.com\n\nVersion: ")+appVersion);
} // about()

void MainWindow::clearImageBuffer()
{
    controller->clearImageBuffer();
} // clearImageBuffer()

void MainWindow::setGrayscale(bool input)
{
    // Not checked
    if(!input)
        imageProcessingFlags.grayscaleOn=false;
    // Checked
    else if(input)
        imageProcessingFlags.grayscaleOn=true;
    // Update image processing flags in processingThread
    emit newImageProcessingFlags(imageProcessingFlags);
} // setGrayscale()

void MainWindow::setSmooth(bool input)
{
    // Not checked
    if(!input)
        imageProcessingFlags.smoothOn=false;
    // Checked
    else if(input)
        imageProcessingFlags.smoothOn=true;
    // Update image processing flags in processingThread
    emit newImageProcessingFlags(imageProcessingFlags);
} // setSmooth()

void MainWindow::setDilate(bool input)
{
    // Not checked
    if(!input)
        imageProcessingFlags.dilateOn=false;
    // Checked
    else if(input)
        imageProcessingFlags.dilateOn=true;
    // Update image processing flags in processingThread
    emit newImageProcessingFlags(imageProcessingFlags);
} // setDilate()

void MainWindow::setErode(bool input)
{
    // Not checked
    if(!input)
        imageProcessingFlags.erodeOn=false;
    // Checked
    else if(input)
        imageProcessingFlags.erodeOn=true;
    // Update image processing flags in processingThread
    emit newImageProcessingFlags(imageProcessingFlags);
} // setErode()

void MainWindow::setFlip(bool input)
{
    // Not checked
    if(!input)
        imageProcessingFlags.flipOn=false;
    // Checked
    else if(input)
        imageProcessingFlags.flipOn=true;
    // Update image processing flags in processingThread
    emit newImageProcessingFlags(imageProcessingFlags);
} // setFlip()

void MainWindow::setCanny(bool input)
{
    // Not checked
    if(!input)
        imageProcessingFlags.cannyOn=false;
    // Checked
    else if(input)
        imageProcessingFlags.cannyOn=true;
    // Update image processing flags in processingThread
    emit newImageProcessingFlags(imageProcessingFlags);
} // setCanny()

void MainWindow::updateFrame(const QImage &frame)
{
    // Show [number of images in buffer / image buffer size] in imageBufferLabel in main window
    imageBufferLabel->setText(QString("[")+QString::number(controller->processingThread->getCurrentSizeOfBuffer())+
                              QString("/")+QString::number(imageBufferSize)+QString("]"));
    // Show percentage of image bufffer full in imageBufferBar in main window
    imageBufferBar->setValue(controller->processingThread->getCurrentSizeOfBuffer());
    // Show processing rate in captureRateLabel in main window
    captureRateLabel->setNum(controller->captureThread->getAvgFPS());
    captureRateLabel->setText(captureRateLabel->text()+" fps");
    // Show processing rate in processingRateLabel in main window
    processingRateLabel->setNum(controller->processingThread->getAvgFPS());
    processingRateLabel->setText(processingRateLabel->text()+" fps");
    // Show ROI information in roiLabel in main window
    roiLabel->setText(QString("(")+QString::number(controller->processingThread->getCurrentROI().x)+QString(",")+
                      QString::number(controller->processingThread->getCurrentROI().y)+QString(") ")+
                      QString::number(controller->processingThread->getCurrentROI().width)+
                      QString("x")+QString::number(controller->processingThread->getCurrentROI().height));
    // Display frame in main window
    frameLabel->setPixmap(QPixmap::fromImage(frame));
} // updateFrame()

void MainWindow::setImageProcessingSettings()
{
    // Prompt user:
    // If user presses OK button on dialog, update image processing settings
    if(imageProcessingSettingsDialog->exec()==1)
        imageProcessingSettingsDialog->updateStoredSettingsFromDialog();
    // Else, restore dialog state
    else
       imageProcessingSettingsDialog->updateDialogSettingsFromStored();
} // setImageProcessingSettings()

void MainWindow::updateMouseCursorPosLabel()
{
    // Update mouse cursor position in mouseCursorPosLabel in main window
    mouseCursorPosLabel->setText(QString("(")+QString::number(frameLabel->getMouseCursorPos().x())+
                                 QString(",")+QString::number(frameLabel->getMouseCursorPos().y())+
                                 QString(")"));
    // Show ROI-adjusted cursor position if camera is connected
    if(isCameraConnected)
        mouseCursorPosLabel->setText(mouseCursorPosLabel->text()+
                                     QString(" [")+QString::number(frameLabel->getMouseCursorPos().x()-(640-controller->processingThread->getCurrentROI().width)/2)+
                                     QString(",")+QString::number(frameLabel->getMouseCursorPos().y()-(480-controller->processingThread->getCurrentROI().height)/2)+
                                     QString("]"));
} // updateMouseCursorPosLabel()

void MainWindow::newMouseData(struct MouseData mouseData)
{
    // Local variables
    int x_temp, y_temp, width_temp, height_temp;
    // Set ROI
    if(mouseData.leftButtonRelease)
    {
        // Copy box dimensions from mouseData to taskData
        taskData.selectionBox.setX(mouseData.selectionBox.x()-((frameLabel->width()-controller->captureThread->getInputSourceWidth()))/2);
        taskData.selectionBox.setY(mouseData.selectionBox.y()-((frameLabel->height()-controller->captureThread->getInputSourceHeight()))/2);
        taskData.selectionBox.setWidth(mouseData.selectionBox.width());
        taskData.selectionBox.setHeight(mouseData.selectionBox.height());
        // Check if selection box has NON-ZERO dimensions
        if((taskData.selectionBox.width()!=0)&&((taskData.selectionBox.height())!=0))
        {
            // Selection box can also be drawn from bottom-right to top-left corner
            if(taskData.selectionBox.width()<0)
            {
                x_temp=taskData.selectionBox.x();
                width_temp=taskData.selectionBox.width();
                taskData.selectionBox.setX(x_temp+taskData.selectionBox.width());
                taskData.selectionBox.setWidth(width_temp*-1);
            }
            if(taskData.selectionBox.height()<0)
            {
                y_temp=taskData.selectionBox.y();
                height_temp=taskData.selectionBox.height();
                taskData.selectionBox.setY(y_temp+taskData.selectionBox.height());
                taskData.selectionBox.setHeight(height_temp*-1);
            }
            // Check if selection box is not outside window
            if((taskData.selectionBox.x()<0)||(taskData.selectionBox.y()<0)||
               ((taskData.selectionBox.x()+taskData.selectionBox.width())>sourceWidth)||
               ((taskData.selectionBox.y()+taskData.selectionBox.height())>sourceHeight))
            {
                // Display error message
                QMessageBox::warning(this,"ERROR:","Selection box outside range. Please try again.");
            }
            else
            {
                // Set setROIFlag to TRUE
                taskData.setROIFlag=true;
                // Update task data in processingThread
                emit newTaskData(taskData);
                // Set setROIFlag to FALSE
                taskData.setROIFlag=false;
            }
        }
    }
    // Reset ROI
    else if(mouseData.rightButtonRelease)
    {
        // Set resetROIFlag to TRUE
        taskData.resetROIFlag=true;
        // Update task data in processingThread
        emit newTaskData(taskData);
        // Set resetROIFlag to FALSE
        taskData.resetROIFlag=false;
    }
} // newMouseData()

void MainWindow::initializeImageProcessingFlagsStructure()
{
    imageProcessingFlags.grayscaleOn=false;
    imageProcessingFlags.smoothOn=false;
    imageProcessingFlags.dilateOn=false;
    imageProcessingFlags.erodeOn=false;
    imageProcessingFlags.flipOn=false;
    imageProcessingFlags.cannyOn=false;
} // initializeImageProcessingFlagsStructure()

void MainWindow::initializeTaskDataStructure()
{
    taskData.setROIFlag=false;
    taskData.resetROIFlag=false;
} // initializeTaskDataStructure()

void MainWindow::setInitialGUIState()
{
    //////////////////////////////////////////
    // Enable/disable appropriate GUI items //
    //////////////////////////////////////////
    // Menu
    connectToCameraAction->setDisabled(false);
    disconnectCameraAction->setDisabled(true);
    imageProcessingMenu->setDisabled(true);
    grayscaleAction->setChecked(false);
    smoothAction->setChecked(false);
    dilateAction->setChecked(false);
    erodeAction->setChecked(false);
    flipAction->setChecked(false);
    cannyAction->setChecked(false);
    // Window
    frameLabel->setText("No camera connected.");
    imageBufferBar->setValue(0);
    imageBufferLabel->setText("[000/000]");
    captureRateLabel->setText("");
    processingRateLabel->setText("");
    deviceNumberLabel->setText("");
    cameraResolutionLabel->setText("");
    roiLabel->setText("");
    mouseCursorPosLabel->setText("");
    clearImageBufferButton->setDisabled(true);
} // setInitialGUIState()

void MainWindow::signalSlotsInit()
{
    connect(connectToCameraAction, SIGNAL(triggered()), this, SLOT(connectToCamera()));
    connect(disconnectCameraAction, SIGNAL(triggered()), this, SLOT(disconnectCamera()));
    connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));
    connect(grayscaleAction, SIGNAL(toggled(bool)), this, SLOT(setGrayscale(bool)));
    connect(smoothAction, SIGNAL(toggled(bool)), this, SLOT(setSmooth(bool)));
    connect(dilateAction, SIGNAL(toggled(bool)), this, SLOT(setDilate(bool)));
    connect(erodeAction, SIGNAL(toggled(bool)), this, SLOT(setErode(bool)));
    connect(flipAction, SIGNAL(toggled(bool)), this, SLOT(setFlip(bool)));
    connect(cannyAction, SIGNAL(toggled(bool)), this, SLOT(setCanny(bool)));
    connect(imageProcessingSettingsAction, SIGNAL(triggered()), this, SLOT(setImageProcessingSettings()));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));
    connect(clearImageBufferButton, SIGNAL(released()), this, SLOT(clearImageBuffer()));
    connect(frameLabel, SIGNAL(onMouseMoveEvent()), this, SLOT(updateMouseCursorPosLabel()));
} // signalSlotsInit()
