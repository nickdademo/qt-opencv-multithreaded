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

#include "CaptureThread.h"
#include "ProcessingThread.h"
#include "ImageProcessingSettingsDialog.h"
#include "SharedImageBuffer.h"
#include "FrameLabel.h"

#include <QMessageBox>
#include <QDebug>
#include <QMenu>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QPushButton>
#include <QLabel>

CameraView::CameraView(int deviceNumber, SharedImageBuffer *sharedImageBuffer, QWidget *parent) :
    QWidget(parent),
    m_sharedImageBuffer(sharedImageBuffer),
    m_isCameraConnected(false),
    m_deviceNumber(deviceNumber)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;

    // Frame label
    m_frameLabel = new FrameLabel;
    m_frameLabel->setMouseTracking(true);
    m_frameLabel->setAlignment(Qt::AlignCenter);
    m_frameLabel->setText(tr("No camera connected."));
    m_frameLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    m_frameLabel->setAutoFillBackground(true);
    connect(m_frameLabel, &FrameLabel::onMouseMoveEvent, this, &CameraView::updateMouseCursorPosLabel);
    mainLayout->addWidget(m_frameLabel, 1);

    // Grid layout
    QGridLayout *gridLayout = new QGridLayout;

    // Bold font
    QFont boldFont;
    boldFont.setBold(true);

    // Device Number
    QLabel *deviceNumberLabel = new QLabel(tr("Device Number") + ":");
    deviceNumberLabel->setFont(boldFont);
    deviceNumberLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_deviceNumberLabel = new QLabel;
    m_deviceNumberLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    gridLayout->addWidget(deviceNumberLabel, 0, 0);
    gridLayout->addWidget(m_deviceNumberLabel, 0, 1);
    // Camera Resolution
    QLabel *cameraResolutionLabel = new QLabel(tr("Camera Resolution") + ":");
    cameraResolutionLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    cameraResolutionLabel->setFont(boldFont);
    m_cameraResolutionLabel = new QLabel;
    m_cameraResolutionLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    gridLayout->addWidget(cameraResolutionLabel, 1, 0);
    gridLayout->addWidget(m_cameraResolutionLabel, 1, 1);
    // Capture Rate
    QLabel *captureRateLabel = new QLabel(tr("Capture Rate") + ":");
    captureRateLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    captureRateLabel->setFont(boldFont);
    m_captureRateLabel = new QLabel;
    m_captureRateLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_nFramesCapturedLabel = new QLabel;
    m_nFramesCapturedLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    gridLayout->addWidget(captureRateLabel, 2, 0); 
    gridLayout->addWidget(m_captureRateLabel, 2, 1);
    gridLayout->addWidget(m_nFramesCapturedLabel, 2, 2);
    // Processing Rate
    QLabel *processingRateLabel = new QLabel(tr("Processing Rate") + ":");
    processingRateLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    processingRateLabel->setFont(boldFont);
    m_processingRateLabel = new QLabel;
    m_processingRateLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_nFramesProcessedLabel = new QLabel;
    m_nFramesProcessedLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    gridLayout->addWidget(processingRateLabel, 3, 0);
    gridLayout->addWidget(m_processingRateLabel, 3, 1);
    gridLayout->addWidget(m_nFramesProcessedLabel, 3, 2);
    // ROI
    QLabel *roiLabel = new QLabel(tr("ROI") + ":");
    roiLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    roiLabel->setFont(boldFont);
    m_roiLabel = new QLabel;
    m_roiLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    gridLayout->addWidget(roiLabel, 4, 0);
    gridLayout->addWidget(m_roiLabel, 4, 1);
    // Mouse Cursor Position
    QLabel *mouseCursorPosLabel = new QLabel(tr("Cursor") + ":");
    mouseCursorPosLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    mouseCursorPosLabel->setFont(boldFont);
    m_mouseCursorPosLabel = new QLabel;
    m_mouseCursorPosLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    gridLayout->addWidget(mouseCursorPosLabel, 5, 0);
    gridLayout->addWidget(m_mouseCursorPosLabel, 5, 1);
    // Image Buffer Status
    m_imageBufferStatusProgressBar = new QProgressBar;
    m_imageBufferStatusProgressBar->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_imageBufferStatusProgressBar->setValue(0);
    m_imageBufferStatusProgressBar->setFormat("%p% (%v/%m)");
    m_imageBufferStatusProgressBar->setAlignment(Qt::AlignCenter);
    // Clear Image Buffer
    QLabel *imageBufferLabel = new QLabel(tr("Image Buffer") + ":");
    imageBufferLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    imageBufferLabel->setFont(boldFont);
    m_clearImageBufferButton = new QPushButton(tr("Clear Buffer"));
    m_clearImageBufferButton->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_clearImageBufferButton->setEnabled(false);
    connect(m_clearImageBufferButton, &QPushButton::released, this, &CameraView::clearImageBuffer);
    gridLayout->addWidget(imageBufferLabel, 6, 0);
    gridLayout->addWidget(m_imageBufferStatusProgressBar, 6, 1, 1, 2);
    gridLayout->addWidget(m_clearImageBufferButton, 6, 3);

    mainLayout->addLayout(gridLayout);

    // Set layout
    setLayout(mainLayout);

    // Initialize ImageProcessingFlags structure
    m_imageProcessingFlags.grayscaleOn = false;
    m_imageProcessingFlags.smoothOn = false;
    m_imageProcessingFlags.dilateOn = false;
    m_imageProcessingFlags.erodeOn = false;
    m_imageProcessingFlags.flipOn = false;
    m_imageProcessingFlags.cannyOn = false;

    
    
    //connect(m_frameLabel->menu, &QMenu::triggered, this, &CameraView::handleContextMenuAction);
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
}

bool CameraView::connectToCamera(bool dropFrameIfBufferFull, int capThreadPriority, int procThreadPriority, bool enableFrameProcessing, int width, int height)
{
    // Set frame label text
    if (m_sharedImageBuffer->isSyncEnabled(m_deviceNumber))
    {
        m_frameLabel->setText(tr("Camera connected. Waiting..."));
    }
    else
    {
        m_frameLabel->setText(tr("Connecting to camera..."));
    }

    // Create capture thread
    m_captureThread = new CaptureThread(m_sharedImageBuffer, m_deviceNumber, dropFrameIfBufferFull, width, height);

    // Attempt to connect to camera
    if (m_captureThread->connectToCamera())
    {
        // Set initial frame label size
        adjustSize();

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
            connect(m_frameLabel, &FrameLabel::newMouseData, this, &CameraView::newMouseData);
        }
        // Set initial data in processing thread
        emit setRoi(QRect(0, 0, m_captureThread->videoCapture().get(CV_CAP_PROP_FRAME_WIDTH), m_captureThread->videoCapture().get(CV_CAP_PROP_FRAME_HEIGHT)));
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
        m_imageBufferStatusProgressBar->setMinimum(0);
        m_imageBufferStatusProgressBar->setMaximum(m_sharedImageBuffer->get(m_deviceNumber)->maxSize());
        // Enable "Clear Image Buffer" push button
        m_clearImageBufferButton->setEnabled(true);
        // Set text in labels
        m_deviceNumberLabel->setNum(m_deviceNumber);
        m_cameraResolutionLabel->setText(QString::number(m_captureThread->videoCapture().get(CV_CAP_PROP_FRAME_WIDTH)) + QString("x") + QString::number(m_captureThread->videoCapture().get(CV_CAP_PROP_FRAME_HEIGHT)));
        // Set internal flag and return
        m_isCameraConnected = true;
        // Set frame label text
        if(!enableFrameProcessing)
        {
            m_frameLabel->setText(tr("Frame processing disabled."));
        }

        return true;
    }
    
    // Failed to connect to camera
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
    // Show percentage of image bufffer full in progress bar
    m_imageBufferStatusProgressBar->setValue(m_sharedImageBuffer->get(m_deviceNumber)->size());
    // Show processing rate in label
    m_captureRateLabel->setText(QString("%1 fps").arg(data.averageFps));
    // Show number of frames captured in label
    m_nFramesCapturedLabel->setText(QString("[%1]").arg(data.nFramesProcessed));
}

void CameraView::updateProcessingThreadStatistics(ThreadStatisticsData data)
{
    // Show ROI information in label   
    m_roiLabel->setText(QString("(%1, %2) %3x%4").arg(m_processingThread->currentRoi().x()).arg(m_processingThread->currentRoi().y()).arg(m_processingThread->currentRoi().width()).arg(m_processingThread->currentRoi().height()));
    // Show processing rate in label
    m_processingRateLabel->setText(QString("%1 fps").arg(data.averageFps));
    // Show number of frames processed in label
    m_nFramesProcessedLabel->setText(QString("[%1]").arg(data.nFramesProcessed));
}

void CameraView::updateFrame(const QImage &frame)
{
    m_frameLabel->setPixmap(QPixmap::fromImage(frame).scaled(m_frameLabel->width(), m_frameLabel->height(), Qt::KeepAspectRatio));
}

void CameraView::clearImageBuffer()
{
    int nCleared = m_sharedImageBuffer->get(m_deviceNumber)->clear();
    qDebug().noquote() << QString("[%1]: %2 frame(s) cleared from buffer.").arg(m_deviceNumber).arg(nCleared);
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
    // Update mouse cursor position in label
    m_mouseCursorPosLabel->setText(QString("(%1, %2)").arg(m_frameLabel->getMouseCursorPos().x()).arg((m_frameLabel->getMouseCursorPos().y())));

    // Also show pixel cursor position if camera is connected (frame is being shown)
    if(m_frameLabel->pixmap() != 0)
    {
        // Scaling factor calculation depends on whether frame is scaled to fit label or not
        double xScalingFactor;
        double yScalingFactor;
        if(!m_frameLabel->hasScaledContents())
        {
            xScalingFactor = ((double)m_frameLabel->getMouseCursorPos().x() - ((m_frameLabel->width() - m_frameLabel->pixmap()->width()) / 2)) / (double)m_frameLabel->pixmap()->width();
            yScalingFactor = ((double)m_frameLabel->getMouseCursorPos().y() - ((m_frameLabel->height() - m_frameLabel->pixmap()->height()) / 2)) / (double)m_frameLabel->pixmap()->height();
        }
        else
        {
            xScalingFactor = (double)m_frameLabel->getMouseCursorPos().x() / (double)m_frameLabel->width();
            yScalingFactor = (double)m_frameLabel->getMouseCursorPos().y() / (double)m_frameLabel->height();
        }

        // Append text to label
        m_mouseCursorPosLabel->setText(QString("%1 [%2, %3]").arg(m_mouseCursorPosLabel->text()).arg((int)(xScalingFactor*m_processingThread->currentRoi().width())).arg((int)(yScalingFactor*m_processingThread->currentRoi().height())));
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
        if(!m_frameLabel->hasScaledContents())
        {
            xScalingFactor = ((double)mouseData.selectionBox.x() - ((m_frameLabel->width() - m_frameLabel->pixmap()->width()) / 2)) / (double)m_frameLabel->pixmap()->width();
            yScalingFactor = ((double)mouseData.selectionBox.y() - ((m_frameLabel->height() - m_frameLabel->pixmap()->height()) / 2)) / (double)m_frameLabel->pixmap()->height();
            wScalingFactor = (double)m_processingThread->currentRoi().width() / (double)m_frameLabel->pixmap()->width();
            hScalingFactor = (double)m_processingThread->currentRoi().height() / (double)m_frameLabel->pixmap()->height();
        }
        else
        {
            xScalingFactor = (double)mouseData.selectionBox.x() / (double)m_frameLabel->width();
            yScalingFactor = (double)mouseData.selectionBox.y() / (double)m_frameLabel->height();
            wScalingFactor = (double)m_processingThread->currentRoi().width() / (double)m_frameLabel->width();
            hScalingFactor = (double)m_processingThread->currentRoi().height() / (double)m_frameLabel->height();
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
        emit setRoi(QRect(0, 0, m_captureThread->videoCapture().get(CV_CAP_PROP_FRAME_WIDTH), m_captureThread->videoCapture().get(CV_CAP_PROP_FRAME_HEIGHT)));
    }
    else if(action->text() == "Scale to Fit Frame")
    {
        m_frameLabel->setScaledContents(action->isChecked());
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
