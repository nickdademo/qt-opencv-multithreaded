/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* MainWindow.cpp                                                       */
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

#include "MainWindow.h"

#include "SharedImageBuffer.h"
#include "CameraView.h"
#include "CameraConnectDialog.h"
#include "Config.h"

#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QMenu>
#include <QAction>
#include <QTabWidget>
#include <QMenuBar>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    // Metatypes
    qRegisterMetaType<ThreadStatistics>("ThreadStatistics");
    // Create SharedImageBuffer instance
    m_sharedImageBuffer = new SharedImageBuffer();
    // Initialize UI
    initUi();
}

MainWindow::~MainWindow()
{
    delete m_sharedImageBuffer;
}

void MainWindow::initUi()
{
    // File menu
    m_menuFile = menuBar()->addMenu(tr("File"));
    m_actionQuit = new QAction(tr("Quit"), this);
    m_menuFile->addAction(m_actionQuit);
    connect(m_actionQuit, &QAction::triggered, this, &MainWindow::close);

    // Options menu
    m_menuOptions = menuBar()->addMenu(tr("Options"));
    m_actionSynchronizeStreams = new QAction(tr("Synchronize Streams"), this);
    m_actionSynchronizeStreams->setCheckable(true);
    m_menuOptions->addAction(m_actionSynchronizeStreams);

    // View menu
    m_menuView = menuBar()->addMenu(tr("View"));
    m_actionFullScreen = new QAction(tr("Full Screen"), this);
    m_actionFullScreen->setCheckable(true);
    m_menuView->addAction(m_actionFullScreen);
    connect(m_actionFullScreen, &QAction::toggled, this, &MainWindow::setFullScreen);

    // Help menu
    m_menuHelp = menuBar()->addMenu(tr("Help"));
    m_actionAbout = new QAction(tr("About"), this);
    m_menuHelp->addAction(m_actionAbout);
    connect(m_actionAbout, &QAction::triggered, this, &MainWindow::showAboutDialog);

    // Tab widget
    m_tabWidget = new QTabWidget(this);

    // Set initial tab
    m_initialTab = new QLabel;
    m_initialTab->setText(tr("No camera connected."));
    m_initialTab->setAlignment(Qt::AlignCenter);
    m_tabWidget->addTab(m_initialTab, "");
    m_tabWidget->setTabsClosable(false);
    // Add button to tab
    m_connectToCameraButton = new QPushButton();
    m_connectToCameraButton->setText(tr("Connect to Camera..."));
    m_tabWidget->setCornerWidget(m_connectToCameraButton, Qt::TopLeftCorner);
    connect(m_connectToCameraButton, &QPushButton::released, this, &MainWindow::connectToCamera);
    // Tab widget connections
    connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::disconnectCamera);
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &MainWindow::tabWidgetCurrentIndexChanged);
    // Set focus on button
    m_connectToCameraButton->setDefault(true);
    m_connectToCameraButton->setFocus();

    // Tab widget is central widget
    setCentralWidget(m_tabWidget);
}

void MainWindow::connectToCamera()
{
    // We cannot connect to a camera if devices are already connected AND stream synchronization is in progress
    if ((m_cameraViewMap.size() > 0) && m_sharedImageBuffer->isSyncStarted())
    {
        QMessageBox::warning(
            this,
            tr("Stream Synchronization in Progress"),
            tr("Please close all currently open streams before attempting to open a new stream."),
            QMessageBox::Ok
        );
        return;
    }

    // Show dialog
    CameraConnectDialog dialog(this, m_actionSynchronizeStreams->isChecked());
    int ret = dialog.exec();
    if (ret == QDialog::Accepted)
    {
        // Save user-specified device number
        int deviceNumber = dialog.getDeviceNumber();

        // Check if this camera is already connected
        if (m_cameraViewMap.contains(deviceNumber))
        {
            QMessageBox::warning(
                this,
                tr("Connect to Camera"),
                QString("%1\n\n%2").arg(tr("Failed to connect to camera.")).arg(QString("%1: %2").arg(tr("Device already connected")).arg(deviceNumber))
            );
            return;
        }

        // Create image buffer with user-defined size
        Buffer<cv::Mat> *imageBuffer = new Buffer<cv::Mat>(dialog.getImageBufferSize());
        // Add created buffer to SharedImageBuffer object
        m_sharedImageBuffer->add(deviceNumber, imageBuffer, m_actionSynchronizeStreams->isChecked());
        // Create CameraView instance
        CameraView *cameraView = new CameraView(deviceNumber, m_sharedImageBuffer, m_tabWidget);

        // Check if stream synchronization is enabled
        if (m_actionSynchronizeStreams->isChecked())
        {
            // Prompt user
            int ret = QMessageBox::question(
                this,
                tr("Stream Synchronization Enabled"),
                QString("%1\n\n%2").arg(tr("Do you want to start processing?")).arg(tr("Choose 'No' if you would like to open additional streams.")),
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::Yes
            );
            // Start streaming if requested
            m_sharedImageBuffer->setSyncStarted(ret == QMessageBox::Yes);
        }

        // Attempt to connect to camera
        bool result = cameraView->connectToCamera(
            dialog.getDropFrameCheckBoxState(),
            dialog.getCaptureThreadPrio(),
            dialog.getProcessingThreadPrio(),
            dialog.getEnableFrameProcessingCheckBoxState(),
            dialog.getResolutionWidth(),
            dialog.getResolutionHeight()
        );

        // Success
        if (result)
        {
            // Save tab label
            QString tabLabel = dialog.getTabLabel();
            // Add tab
            m_tabWidget->addTab(cameraView, tabLabel.isEmpty() ? QString("[%1]").arg(deviceNumber) : QString("%1 [%2]").arg(tabLabel).arg(deviceNumber));
            m_tabWidget->setCurrentWidget(cameraView);
            // Insert into map
            m_cameraViewMap.insert(deviceNumber, cameraView);
            // Set tooltips
            setTabCloseToolTips(m_tabWidget, tr("Disconnect Camera"));
            // Prevent user from enabling/disabling stream synchronization after a camera has been connected
            m_actionSynchronizeStreams->setEnabled(false);
        }
        // Failure
        else
        {
            // Display error message
            QMessageBox::critical(
                this,
                tr("Connect to Camera"),
                QString("%1\n\n%2").arg(tr("Failed to connect to camera.")).arg(QString("%1: %2").arg(tr("Please check device number")).arg(deviceNumber))
            );

            // Delete widget
            delete cameraView;
            // Remove from shared buffer
            m_sharedImageBuffer->remove(deviceNumber);
            // Delete buffer
            delete imageBuffer;
        }
    }
}

void MainWindow::disconnectCamera(int index)
{
    bool doDisconnect = true;

    // Check if more than one camera is connected AND stream synchronization is in progress
    if ((m_cameraViewMap.size() > 1) && !m_sharedImageBuffer->isSyncStarted())
    {
        // Prompt user
        int ret = QMessageBox::question(
            this,
            tr("Stream Synchronization in Progress"),
            QString("%1\n\n%2").arg(tr("Disconnecting this camera will cause frame processing to begin on all other streams.")).arg(tr("Do you wish to proceed?")),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::Yes
        );
        doDisconnect = (ret == QMessageBox::Yes);
    }

    // Disconnect camera
    if(doDisconnect)
    {
        // Get widget at index
        CameraView *cameraView = (CameraView*)m_tabWidget->widget(index);
        // Close tab
        m_tabWidget->removeTab(index);
        // Get device number
        int deviceNumber = m_cameraViewMap.key(cameraView);
        // Remove widget from map
        m_cameraViewMap.remove(deviceNumber);
        // Delete widget
        delete cameraView;
    }
}

void MainWindow::showAboutDialog()
{
    QMessageBox::information(
        this,
        tr("About"),
        QString("%1 v%2\n\nCreated by %3\nEmail: %4\nWebsite: %5").arg(APP_NAME).arg(APP_VERSION).arg(APP_AUTHOR_NAME).arg(APP_AUTHOR_EMAIL).arg(APP_AUTHOR_WEBSITE)
    );
}

void MainWindow::setTabCloseToolTips(QTabWidget *tabs, QString tooltip)
{
    QList<QAbstractButton*> allPushButtons = tabs->findChildren<QAbstractButton*>();
    for (int ind = 0; ind < allPushButtons.size(); ind++)
    {
        QAbstractButton* item = allPushButtons.at(ind);
        if (item->inherits("CloseButton"))
        {
            item->setToolTip(tooltip);
        }
    }
}

void MainWindow::setFullScreen(bool checked)
{
    if (checked)
    {
        showFullScreen();
    }
    else
    {
        showNormal();
    }
}

void MainWindow::tabWidgetCurrentIndexChanged(int index)
{
    // Tabs present
    if (index != -1)
    {
        QWidget *widget = m_tabWidget->widget(index);

        // New tab is not initial tab
        if (widget != m_initialTab)
        {
            // Ensure initial tab is not present
            int initialTabIndex = m_tabWidget->indexOf(m_initialTab);
            if (initialTabIndex != -1)
            {
                m_tabWidget->removeTab(initialTabIndex);
            }
            m_tabWidget->setTabsClosable(true);
        }
    }
    // No tabs present
    else
    {
        // Add initial tab
        m_tabWidget->addTab(m_initialTab, "");
        m_tabWidget->setTabsClosable(false);

        // Allow stream synchronization to be enabled
        m_actionSynchronizeStreams->setEnabled(true);
    }
}