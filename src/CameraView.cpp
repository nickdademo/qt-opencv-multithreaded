/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* CameraView.cpp                                                       */
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

#include "CameraView.h"
#include "ui_CameraView.h"

#include "CaptureThread.h"
#include "ProcessingThread.h"
#include "ImageProcessingSettingsDialog.h"
#include "SharedImageBuffer.h"

#include <QMessageBox>
#include <QDebug>
#include <QMenu>

CameraView::CameraView(int deviceNumber, SharedImageBuffer *sharedImageBuffer, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CameraView),
    m_sharedImageBuffer(sharedImageBuffer)
{
    // Setup UI
    ui->setupUi(this);
    // Save Device Number
    m_deviceNumber = deviceNumber;
    // Initialize internal flag
    m_isCameraConnected = false;
    // Set initial GUI state
    ui->frameLabel->setText(tr("No camera connected."));
    ui->imageBufferBar->setValue(0);
    ui->imageBufferLabel->setText("[000/000]");
    ui->captureRateLabel->setText("");
    ui->processingRateLabel->setText("");
    ui->deviceNumberLabel->setText("");
    ui->cameraResolutionLabel->setText("");
    ui->roiLabel->setText("");
    ui->mouseCursorPosLabel->setText("");
    ui->clearImageBufferButton->setDisabled(true);
    // Initialize ImageProcessingFlags structure
    m_imageProcessingFlags.grayscaleOn = false;
    m_imageProcessingFlags.smoothOn = false;
    m_imageProcessingFlags.dilateOn = false;
    m_imageProcessingFlags.erodeOn = false;
    m_imageProcessingFlags.flipOn = false;
    m_imageProcessingFlags.cannyOn = false;
    // Connect signals/slots
    connect(ui->frameLabel, &FrameLabel::onMouseMoveEvent, this, &CameraView::updateMouseCursorPosLabel);
    connect(ui->clearImageBufferButton, &QPushButton::released, this, &CameraView::clearImageBuffer);
    connect(ui->frameLabel->menu, &QMenu::triggered, this, &CameraView::handleContextMenuAction);
    // Register type
    qRegisterMetaType<ThreadStatisticsData>("ThreadStatisticsData");
}

CameraView::~CameraView()
{
    if(m_isCameraConnected)
    {
        // Stop processing thread
        if (m_processingThread->isRunning())
        {
            stopProcessingThread();
        }
        // Stop capture thread
        if (m_captureThread->isRunning())
        {
            stopCaptureThread();
        }

        // Automatically start frame processing for other streams if sync was enabled for camera we are disconnecting
        if (m_sharedImageBuffer->isSyncEnabled(m_deviceNumber))
        {
            m_sharedImageBuffer->setSyncStarted(true);
        }

        // Remove from shared buffer
        m_sharedImageBuffer->remove(m_deviceNumber);
        // Disconnect camera
        if (m_captureThread->isCameraConnected())
        {
            m_captureThread->disconnectCamera();
            qDebug().noquote() << QString("[%1]: Camera successfully disconnected.").arg(m_deviceNumber);
        }
        else
        {
            qWarning().noquote() << QString("[%1]: Camera already disconnected.").arg(m_deviceNumber);
        }
    }
    // Delete UI
    delete ui;
}

bool CameraView::connectToCamera(bool dropFrameIfBufferFull, int capThreadPriority, int procThreadPriority, bool enableFrameProcessing, int width, int height)
{
    // Set frame label text
    if (m_sharedImageBuffer->isSyncEnabled(m_deviceNumber))
    {
        ui->frameLabel->setText(tr("Camera connected. Waiting..."));
    }
    else
    {
        ui->frameLabel->setText(tr("Connecting to camera..."));
    }

    // Create capture thread
    m_captureThread = new CaptureThread(m_sharedImageBuffer, m_deviceNumber, dropFrameIfBufferFull, width, height);
    // Attempt to connect to camera
    if (m_captureThread->connectToCamera())
    {
        // Create processing thread
        m_processingThread = new ProcessingThread(m_sharedImageBuffer, m_deviceNumber);
        // Create image processing settings dialog
        m_imageProcessingSettingsDialog = new ImageProcessingSettingsDialog(this);
        // Setup signal/slot connections
        connect(m_processingThread, &ProcessingThread::newFrame, this, &CameraView::updateFrame);
        connect(m_processingThread, &ProcessingThread::newStatistics, this, &CameraView::updateProcessingThreadStatistics);
        connect(m_captureThread, &CaptureThread::newStatistics, this, &CameraView::updateCaptureThreadStatistics);
        connect(m_imageProcessingSettingsDialog, &ImageProcessingSettingsDialog::newImageProcessingSettings, m_processingThread, &ProcessingThread::updateImageProcessingSettings);
        connect(this, &CameraView::newImageProcessingFlags, m_processingThread, &ProcessingThread::updateImageProcessingFlags);
        connect(this, &CameraView::setRoi, m_processingThread, &ProcessingThread::setRoi);
        // Only enable ROI setting/resetting if frame processing is enabled
        if(enableFrameProcessing)
        {
            connect(ui->frameLabel, &FrameLabel::newMouseData, this, &CameraView::newMouseData);
        }
        // Set initial data in processing thread
        emit setRoi(QRect(0, 0, m_captureThread->getInputSourceWidth(), m_captureThread->getInputSourceHeight()));
        emit newImageProcessingFlags(m_imageProcessingFlags);
        m_imageProcessingSettingsDialog->updateStoredSettingsFromDialog();

        // Start capturing frames from camera
        m_captureThread->start((QThread::Priority)capThreadPriority);
        // Start processing captured frames (if enabled)
        if(enableFrameProcessing)
        {
            m_processingThread->start((QThread::Priority)procThreadPriority);
        }

        // Setup imageBufferBar with minimum and maximum values
        ui->imageBufferBar->setMinimum(0);
        ui->imageBufferBar->setMaximum(m_sharedImageBuffer->get(m_deviceNumber)->maxSize());
        // Enable "Clear Image Buffer" push button
        ui->clearImageBufferButton->setEnabled(true);
        // Set text in labels
        ui->deviceNumberLabel->setNum(m_deviceNumber);
        ui->cameraResolutionLabel->setText(QString::number(m_captureThread->getInputSourceWidth()) + QString("x") + QString::number(m_captureThread->getInputSourceHeight()));
        // Set internal flag and return
        m_isCameraConnected = true;
        // Set frame label text
        if(!enableFrameProcessing)
        {
            ui->frameLabel->setText(tr("Frame processing disabled."));
        }
        return true;
    }
    // Failed to connect to camera
    else
        return false;
}

void CameraView::stopCaptureThread()
{
    qDebug().noquote() << QString("[%1]: About to stop capture thread...").arg(m_deviceNumber);

    m_captureThread->stop();
    m_sharedImageBuffer->wakeAll(); // This allows the thread to be stopped if it is in a wait-state

    // Take one frame off a FULL queue to allow the capture thread to finish
    if (m_sharedImageBuffer->get(m_deviceNumber)->isFull())
    {
        m_sharedImageBuffer->get(m_deviceNumber)->get();
    }
    m_captureThread->wait();

    qDebug().noquote() << QString("[%1]: Capture thread successfully stopped.").arg(m_deviceNumber);
}

void CameraView::stopProcessingThread()
{
    qDebug().noquote() << QString("[%1]: About to stop processing thread...").arg(m_deviceNumber);

    m_processingThread->stop();
    m_sharedImageBuffer->wakeAll(); // This allows the thread to be stopped if it is in a wait-state
    m_processingThread->wait();

    qDebug().noquote() << QString("[%1]: Processing thread successfully stopped.").arg(m_deviceNumber);
}

void CameraView::updateCaptureThreadStatistics(ThreadStatisticsData data)
{
    // Show [number of images in buffer / image buffer size] in imageBufferLabel
    ui->imageBufferLabel->setText(QString("[%1/%2]").arg(m_sharedImageBuffer->get(m_deviceNumber)->size()).arg(m_sharedImageBuffer->get(m_deviceNumber)->maxSize()));
    // Show percentage of image bufffer full in imageBufferBar
    ui->imageBufferBar->setValue(m_sharedImageBuffer->get(m_deviceNumber)->size());

    // Show processing rate in captureRateLabel
    ui->captureRateLabel->setText(QString("%1 fps").arg(data.averageFPS));
    // Show number of frames captured in nFramesCapturedLabel
    ui->nFramesCapturedLabel->setText(QString("[%1]").arg(data.nFramesProcessed));
}

void CameraView::updateProcessingThreadStatistics(ThreadStatisticsData data)
{
    // Show ROI information in roiLabel   
    ui->roiLabel->setText(QString("(%1, %2) %3x%4").arg(m_processingThread->currentRoi().x()).arg(m_processingThread->currentRoi().y()).arg(m_processingThread->currentRoi().width()).arg(m_processingThread->currentRoi().height()));
    
    // Show processing rate in processingRateLabel
    ui->processingRateLabel->setText(QString("%1 fps").arg(data.averageFPS));
    // Show number of frames processed in nFramesProcessedLabel
    ui->nFramesProcessedLabel->setText(QString("[%1]").arg(data.nFramesProcessed));
}

void CameraView::updateFrame(const QImage &frame)
{
    ui->frameLabel->setPixmap(QPixmap::fromImage(frame).scaled(ui->frameLabel->width(), ui->frameLabel->height(), Qt::KeepAspectRatio));
}

void CameraView::clearImageBuffer()
{
    if (m_sharedImageBuffer->get(m_deviceNumber)->clear())
    {
        qDebug().noquote() << QString("[%1]: Image buffer successfully cleared.").arg(m_deviceNumber);
    }
    else
    {
        qWarning().noquote() << QString("[%1]: Could not clear image buffer.").arg(m_deviceNumber);
    }
}

void CameraView::setImageProcessingSettings()
{
    // Prompt user:
    // If user presses OK button on dialog, update image processing settings
    if (m_imageProcessingSettingsDialog->exec() == QDialog::Accepted)
    {
        m_imageProcessingSettingsDialog->updateStoredSettingsFromDialog();
    }
    // Else, restore dialog state
    else
    {
        m_imageProcessingSettingsDialog->updateDialogSettingsFromStored();
    }
}

void CameraView::updateMouseCursorPosLabel()
{
    // Update mouse cursor position in mouseCursorPosLabel
    ui->mouseCursorPosLabel->setText(QString("(%1, %2)").arg(ui->frameLabel->getMouseCursorPos().x()).arg((ui->frameLabel->getMouseCursorPos().y())));

    // Also show pixel cursor position if camera is connected (frame is being shown)
    if(ui->frameLabel->pixmap() != 0)
    {
        // Scaling factor calculation depends on whether frame is scaled to fit label or not
        double xScalingFactor;
        double yScalingFactor;
        if(!ui->frameLabel->hasScaledContents())
        {
            xScalingFactor = ((double)ui->frameLabel->getMouseCursorPos().x() - ((ui->frameLabel->width() - ui->frameLabel->pixmap()->width()) / 2)) / (double)ui->frameLabel->pixmap()->width();
            yScalingFactor = ((double)ui->frameLabel->getMouseCursorPos().y() - ((ui->frameLabel->height() - ui->frameLabel->pixmap()->height()) / 2)) / (double)ui->frameLabel->pixmap()->height();
        }
        else
        {
            xScalingFactor = (double)ui->frameLabel->getMouseCursorPos().x() / (double)ui->frameLabel->width();
            yScalingFactor = (double)ui->frameLabel->getMouseCursorPos().y() / (double)ui->frameLabel->height();
        }

        ui->mouseCursorPosLabel->setText(QString("%1 [%2, %3]").arg(ui->mouseCursorPosLabel->text()).arg((int)(xScalingFactor*m_processingThread->currentRoi().width())).arg((int)(yScalingFactor*m_processingThread->currentRoi().height())));
    }
}

void CameraView::newMouseData(MouseData mouseData)
{
    int x_temp, y_temp, width_temp, height_temp;
    QRect selectionBox;

    // Set ROI
    if(mouseData.leftButtonRelease)
    {
        double xScalingFactor;
        double yScalingFactor;
        double wScalingFactor;
        double hScalingFactor;

        // Selection box calculation depends on whether frame is scaled to fit label or not
        if(!ui->frameLabel->hasScaledContents())
        {
            xScalingFactor = ((double)mouseData.selectionBox.x() - ((ui->frameLabel->width() - ui->frameLabel->pixmap()->width()) / 2)) / (double)ui->frameLabel->pixmap()->width();
            yScalingFactor = ((double)mouseData.selectionBox.y() - ((ui->frameLabel->height() - ui->frameLabel->pixmap()->height()) / 2)) / (double)ui->frameLabel->pixmap()->height();
            wScalingFactor = (double)m_processingThread->currentRoi().width() / (double)ui->frameLabel->pixmap()->width();
            hScalingFactor = (double)m_processingThread->currentRoi().height() / (double)ui->frameLabel->pixmap()->height();
        }
        else
        {
            xScalingFactor = (double)mouseData.selectionBox.x() / (double)ui->frameLabel->width();
            yScalingFactor = (double)mouseData.selectionBox.y() / (double)ui->frameLabel->height();
            wScalingFactor = (double)m_processingThread->currentRoi().width() / (double)ui->frameLabel->width();
            hScalingFactor = (double)m_processingThread->currentRoi().height() / (double)ui->frameLabel->height();
        }

        // Set selection box properties (new ROI)
        selectionBox.setX(xScalingFactor * m_processingThread->currentRoi().width() + m_processingThread->currentRoi().x());
        selectionBox.setY(yScalingFactor * m_processingThread->currentRoi().height() + m_processingThread->currentRoi().y());
        selectionBox.setWidth(wScalingFactor * mouseData.selectionBox.width());
        selectionBox.setHeight(hScalingFactor * mouseData.selectionBox.height());

        // Check if selection box has NON-ZERO dimensions
        if((selectionBox.width() != 0) && ((selectionBox.height()) != 0))
        {
            // Selection box can also be drawn from bottom-right to top-left corner
            if(selectionBox.width() < 0)
            {
                x_temp = selectionBox.x();
                width_temp = selectionBox.width();
                selectionBox.setX(x_temp + selectionBox.width());
                selectionBox.setWidth(width_temp * -1);
            }
            if(selectionBox.height() < 0)
            {
                y_temp = selectionBox.y();
                height_temp = selectionBox.height();
                selectionBox.setY(y_temp + selectionBox.height());
                selectionBox.setHeight(height_temp * -1);
            }

            // Check if selection box is not outside window
            if((selectionBox.x() < 0) || (selectionBox.y() < 0)||
                ((selectionBox.x() + selectionBox.width()) > (m_processingThread->currentRoi().x() + m_processingThread->currentRoi().width())) ||
                ((selectionBox.y() + selectionBox.height()) > (m_processingThread->currentRoi().y() + m_processingThread->currentRoi().height())) ||
                (selectionBox.x() < m_processingThread->currentRoi().x()) ||
                (selectionBox.y() < m_processingThread->currentRoi().y()))
            {
                // Display error message
                QMessageBox::critical(this, "Invalid Selection", tr("Selection box outside allowable range."));
            }
            // Set ROI
            else
            {
                emit setRoi(selectionBox);
            }
        }
    }
}

void CameraView::handleContextMenuAction(QAction *action)
{
    if(action->text() == "Reset ROI")
    {
        emit setRoi(QRect(0, 0, m_captureThread->getInputSourceWidth(), m_captureThread->getInputSourceHeight()));
    }
    else if(action->text() == "Scale to Fit Frame")
    {
        ui->frameLabel->setScaledContents(action->isChecked());
    }
    else if(action->text() == "Grayscale")
    {
        m_imageProcessingFlags.grayscaleOn = action->isChecked();
        emit newImageProcessingFlags(m_imageProcessingFlags);
    }
    else if(action->text() == "Smooth")
    {
        m_imageProcessingFlags.smoothOn = action->isChecked();
        emit newImageProcessingFlags(m_imageProcessingFlags);
    }
    else if(action->text() == "Dilate")
    {
        m_imageProcessingFlags.dilateOn = action->isChecked();
        emit newImageProcessingFlags(m_imageProcessingFlags);
    }
    else if(action->text() == "Erode")
    {
        m_imageProcessingFlags.erodeOn = action->isChecked();
        emit newImageProcessingFlags(m_imageProcessingFlags);
    }
    else if(action->text() == "Flip")
    {
        m_imageProcessingFlags.flipOn = action->isChecked();
        emit newImageProcessingFlags(m_imageProcessingFlags);
    }
    else if(action->text() == "Canny")
    {
        m_imageProcessingFlags.cannyOn = action->isChecked();
        emit newImageProcessingFlags(m_imageProcessingFlags);
    }
    else if(action->text() == "Settings...")
    {
        setImageProcessingSettings();
    }
}
