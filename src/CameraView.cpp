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
#include "SharedImageBuffer.h"
#include "FrameLabel.h"

#include <QMessageBox>
#include <QDebug>
#include <QMenu>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QPushButton>
#include <QLabel>

CameraView::CameraView(Settings settings, SharedImageBuffer *sharedImageBuffer, QWidget *parent) :
    QWidget(parent),
    m_settings(settings),
    m_sharedImageBuffer(sharedImageBuffer)
{
    // Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;

    // Frame label
    m_frameLabel = new FrameLabel;
    m_frameLabel->setMouseTracking(true);
    m_frameLabel->setAlignment(Qt::AlignCenter);
    m_frameLabel->setText(tr("No camera connected."));
    m_frameLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    m_frameLabel->setAutoFillBackground(true);
    connect(m_frameLabel, &FrameLabel::onMouseMoveEvent, this, &CameraView::updateMouseCursorPosLabel);
    connect(&m_frameLabel->menu(), &QMenu::triggered, this, &CameraView::onContextMenuAction);
    mainLayout->addWidget(m_frameLabel, 1);

    // Grid layout
    QGridLayout *gridLayout = new QGridLayout;

    // Fonts
    QFont boldFont;
    boldFont.setBold(true);
    QFont italicFont;
    italicFont.setItalic(true);

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
    m_captureRateLabel->setText("-");
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
    m_processingRateLabel->setText("-");
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
    m_roiLabel->setText("-");
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
    // Stream Control
    QLabel *streamControlLabel = new QLabel(tr("Stream Control") + ":");
    streamControlLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    streamControlLabel->setFont(boldFont);
    m_streamControlStatusLabel = new QLabel;
    m_streamControlStatusLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_streamControlStatusLabel->setFont(italicFont);
    m_streamControlRunButton = new QPushButton(tr("Run"));
    m_streamControlRunButton->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_streamControlRunButton->setEnabled(false);
    connect(m_streamControlRunButton, &QPushButton::released, this, &CameraView::runStream);
    m_streamControlPauseButton = new QPushButton(tr("Pause"));
    m_streamControlPauseButton->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_streamControlPauseButton->setEnabled(false);
    connect(m_streamControlPauseButton, &QPushButton::released, this, &CameraView::pauseStream);
    gridLayout->addWidget(streamControlLabel, 7, 0);
    gridLayout->addWidget(m_streamControlStatusLabel, 7, 1);
    gridLayout->addWidget(m_streamControlRunButton, 7, 2);
    gridLayout->addWidget(m_streamControlPauseButton, 7, 3);

    mainLayout->addLayout(gridLayout);

    // Set layout
    setLayout(mainLayout);

    // Connections related to stream state
    // We use Queued Connections here to prevent dead locks due to the SharedImageBuffer mutex
    connect(m_sharedImageBuffer, &SharedImageBuffer::streamRun, this, &CameraView::onStreamRun, Qt::QueuedConnection);
    connect(m_sharedImageBuffer, &SharedImageBuffer::streamPaused, this, &CameraView::onStreamPaused, Qt::QueuedConnection);
    connect(m_sharedImageBuffer, &SharedImageBuffer::syncStarted, this, &CameraView::onSyncStarted, Qt::QueuedConnection);
    connect(m_sharedImageBuffer, &SharedImageBuffer::syncStarted, this, &CameraView::onSyncStopped, Qt::QueuedConnection);
    // Create image buffer with user-defined size
    m_imageBuffer = new Buffer<cv::Mat>(settings.imageBufferSize);
}

CameraView::~CameraView()
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
    /*
    if (m_sharedImageBuffer->isSyncEnabled(m_settings.deviceNumber))
    {
        m_sharedImageBuffer->startSync();
    }
    */

    // Remove from shared buffer
    m_sharedImageBuffer->remove(m_settings.deviceNumber);
    // Disconnect camera
    if (m_captureThread->isCameraConnected())
    {
        m_captureThread->disconnectCamera();
        qDebug().noquote() << QString("[%1]: Camera successfully disconnected.").arg(m_settings.deviceNumber);
    }
    else
    {
        qWarning().noquote() << QString("[%1]: Camera already disconnected.").arg(m_settings.deviceNumber);
    }

    // Delete buffer
    delete m_imageBuffer;
    // Remove from shared buffer (if it exists)
    m_sharedImageBuffer->remove(m_settings.deviceNumber);
}

bool CameraView::connectToCamera()
{
    m_frameLabel->setText(tr("Connecting to camera..."));

    // Create capture thread
    m_captureThread = new CaptureThread(m_sharedImageBuffer, m_settings.deviceNumber, m_settings.dropFrameIfBufferFull, m_settings.width, m_settings.height);

    // Attempt to connect to camera
    if (m_captureThread->connectToCamera())
    {
        // Create processing thread
        m_processingThread = new ProcessingThread(m_sharedImageBuffer, m_settings.deviceNumber);
        m_processingThread->setRoi(QRect(0, 0, m_captureThread->videoCapture().get(CV_CAP_PROP_FRAME_WIDTH), m_captureThread->videoCapture().get(CV_CAP_PROP_FRAME_HEIGHT)));

        // Add image buffer to SharedImageBuffer object
        m_sharedImageBuffer->add(m_settings.deviceNumber, m_imageBuffer, m_settings.streamControl, m_settings.synchronizeStream);

        // Start capturing frames from camera
        m_captureThread->start(m_settings.captureThreadPriority);
        // Start processing captured frames (if enabled)
        if (m_settings.enableFrameProcessing)
        {
            m_processingThread->start(m_settings.processingThreadPriority);
        }

        // Update UI
        m_frameLabel->setText(tr("Camera connected."));
        m_deviceNumberLabel->setNum(m_settings.deviceNumber);
        m_cameraResolutionLabel->setText(QString("%1x%2").arg(m_captureThread->videoCapture().get(CV_CAP_PROP_FRAME_WIDTH)).arg(m_captureThread->videoCapture().get(CV_CAP_PROP_FRAME_HEIGHT)));
        m_clearImageBufferButton->setEnabled(true);
        m_imageBufferStatusProgressBar->setMinimum(0);
        m_imageBufferStatusProgressBar->setMaximum(m_sharedImageBuffer->get(m_settings.deviceNumber)->maxSize());
        return true;
    }
    
    return false;
}

void CameraView::stopCaptureThread()
{
    qDebug().noquote() << QString("[%1]: About to stop capture thread...").arg(m_settings.deviceNumber);

    m_captureThread->stop();
    //m_sharedImageBuffer->wakeAll(); // This allows the thread to be stopped if it is in a wait-state

    // Take one frame off a FULL queue to allow the capture thread to finish
    if (m_sharedImageBuffer->get(m_settings.deviceNumber)->isFull())
    {
        m_sharedImageBuffer->get(m_settings.deviceNumber)->get();
    }
    m_captureThread->wait();

    qDebug().noquote() << QString("[%1]: Capture thread successfully stopped.").arg(m_settings.deviceNumber);
}

void CameraView::stopProcessingThread()
{
    qDebug().noquote() << QString("[%1]: About to stop processing thread...").arg(m_settings.deviceNumber);

    m_processingThread->stop();
    //m_sharedImageBuffer->wakeAll(); // This allows the thread to be stopped if it is in a wait-state
    m_processingThread->wait();

    qDebug().noquote() << QString("[%1]: Processing thread successfully stopped.").arg(m_settings.deviceNumber);
}

void CameraView::updateCaptureThreadStatistics(ThreadStatistics statistics)
{
    // Show processing rate in label
    m_captureRateLabel->setText(QString("%1 fps").arg(QString::number(statistics.averageFps, 'f', 2)));
    // Show number of frames captured in label
    m_nFramesCapturedLabel->setText(QString("[%1]").arg(statistics.frameCounter));
    // Show percentage of image bufffer full in progress bar
    m_imageBufferStatusProgressBar->setValue(m_sharedImageBuffer->get(m_settings.deviceNumber)->size());
}

void CameraView::updateProcessingThreadStatistics(ThreadStatistics statistics)
{
    // Show processing rate in label
    m_processingRateLabel->setText(QString("%1 fps").arg(QString::number(statistics.averageFps, 'f', 2)));
    // Show number of frames processed in label
    m_nFramesProcessedLabel->setText(QString("[%1]").arg(statistics.frameCounter));
    // Show ROI information in label   
    m_roiLabel->setText(QString("(%1, %2) %3x%4").arg(m_processingThread->currentRoi().x()).arg(m_processingThread->currentRoi().y()).arg(m_processingThread->currentRoi().width()).arg(m_processingThread->currentRoi().height()));
}

void CameraView::updateFrame(const QImage &frame)
{
    m_frameLabel->setPixmap(QPixmap::fromImage(frame).scaled(m_frameLabel->width(), m_frameLabel->height(), Qt::KeepAspectRatio));
}

void CameraView::clearImageBuffer()
{
    int nCleared = m_sharedImageBuffer->get(m_settings.deviceNumber)->clear();
    qDebug().noquote() << QString("[%1]: %2 frame(s) cleared from buffer.").arg(m_settings.deviceNumber).arg(nCleared);
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

void CameraView::onNewSelection(QRect box)
{
    int x_temp, y_temp, width_temp, height_temp;
    QRect selectionBox;

    // Set ROI
    double xScalingFactor;
    double yScalingFactor;
    double wScalingFactor;
    double hScalingFactor;

    // Selection box calculation depends on whether frame is scaled to fit label or not
    if(!m_frameLabel->hasScaledContents())
    {
        xScalingFactor = ((double)box.x() - ((m_frameLabel->width() - m_frameLabel->pixmap()->width()) / 2)) / (double)m_frameLabel->pixmap()->width();
        yScalingFactor = ((double)box.y() - ((m_frameLabel->height() - m_frameLabel->pixmap()->height()) / 2)) / (double)m_frameLabel->pixmap()->height();
        wScalingFactor = (double)m_processingThread->currentRoi().width() / (double)m_frameLabel->pixmap()->width();
        hScalingFactor = (double)m_processingThread->currentRoi().height() / (double)m_frameLabel->pixmap()->height();
    }
    else
    {
        xScalingFactor = (double)box.x() / (double)m_frameLabel->width();
        yScalingFactor = (double)box.y() / (double)m_frameLabel->height();
        wScalingFactor = (double)m_processingThread->currentRoi().width() / (double)m_frameLabel->width();
        hScalingFactor = (double)m_processingThread->currentRoi().height() / (double)m_frameLabel->height();
    }

    // Set selection box properties (new ROI)
    selectionBox.setX(xScalingFactor * m_processingThread->currentRoi().width() + m_processingThread->currentRoi().x());
    selectionBox.setY(yScalingFactor * m_processingThread->currentRoi().height() + m_processingThread->currentRoi().y());
    selectionBox.setWidth(wScalingFactor * box.width());
    selectionBox.setHeight(hScalingFactor * box.height());

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
            QMessageBox::critical(this, "Invalid Selection", tr("Selection box outside allowable range."));
        }
        // Set ROI
        else
        {
            m_processingThread->setRoi(selectionBox);
        }
    }
}

void CameraView::onContextMenuAction(QAction *action)
{
}

void CameraView::runStream()
{
    m_sharedImageBuffer->setStreamControl(m_settings.deviceNumber, SharedImageBuffer::StreamControl::Run);
}

void CameraView::pauseStream()
{
    m_sharedImageBuffer->setStreamControl(m_settings.deviceNumber, SharedImageBuffer::StreamControl::Pause);
}

void CameraView::onStreamRun(int deviceNumber)
{
    if (deviceNumber == m_settings.deviceNumber)
    {
        m_streamControlRunButton->setEnabled(false);
        m_streamControlPauseButton->setEnabled(true);
        if (m_sharedImageBuffer->isSyncEnabled(deviceNumber) && m_sharedImageBuffer->isSyncInProgress())
        {
            m_streamControlStatusLabel->setText(tr("Running (synchronized)"));
        }
        else
        {
            m_streamControlStatusLabel->setText(m_settings.synchronizeStream ? tr("Running (waiting for sync)") : tr("Running"));
        }
        
        // Only enable ROI setting if frame processing is enabled
        if (m_settings.enableFrameProcessing)
        {
            disconnect(m_newSelectionConnection);
            m_newSelectionConnection = connect(m_frameLabel, &FrameLabel::newSelection, this, &CameraView::onNewSelection);
        }
        disconnect(m_captureStatisticsConnection);
        m_captureStatisticsConnection = connect(m_captureThread, &CaptureThread::newStatistics, this, &CameraView::updateCaptureThreadStatistics);
        disconnect(m_processingStatisticsConnection);
        m_processingStatisticsConnection = connect(m_processingThread, &ProcessingThread::newStatistics, this, &CameraView::updateProcessingThreadStatistics);
        disconnect(m_newFrameConnection);
        m_newFrameConnection = connect(m_processingThread, &ProcessingThread::newFrame, this, &CameraView::updateFrame);
    }
}

void CameraView::onStreamPaused(int deviceNumber)
{
    if (deviceNumber == m_settings.deviceNumber)
    {
        m_streamControlRunButton->setEnabled(true);
        m_streamControlPauseButton->setEnabled(false);
        if (m_sharedImageBuffer->isSyncEnabled(deviceNumber) && m_sharedImageBuffer->isSyncInProgress())
        {
            m_streamControlStatusLabel->setText(tr("Paused (synchronized)"));
        }
        else
        {
            m_streamControlStatusLabel->setText(m_settings.synchronizeStream ? tr("Paused (waiting for sync)") : tr("Paused"));
        }
        disconnect(m_newSelectionConnection);
        disconnect(m_captureStatisticsConnection);
        disconnect(m_processingStatisticsConnection);
        disconnect(m_newFrameConnection);
        m_captureRateLabel->setText("-");
        m_processingRateLabel->setText("-");
        m_frameLabel->clear();
    }
}

void CameraView::onSyncStarted()
{
    if (m_sharedImageBuffer->isSyncEnabled(m_settings.deviceNumber))
    {
        SharedImageBuffer::StreamControl streamControl = m_sharedImageBuffer->getStreamControl(m_settings.deviceNumber);
        if (streamControl == SharedImageBuffer::StreamControl::Run)
        {
            m_streamControlStatusLabel->setText(tr("Running (synchronized)"));
        }
        else if (streamControl == SharedImageBuffer::StreamControl::Pause)
        {
            m_streamControlStatusLabel->setText(tr("Paused (synchronized)"));
        }
    }
}

void CameraView::onSyncStopped()
{
    // Do nothing
}