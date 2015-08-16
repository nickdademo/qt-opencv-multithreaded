/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* MainWindow.cpp                                                       */
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
    m_actionSynchronizeStreams = new QAction(tr("Synchronize streams"), this);
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

    // Set start tab as blank
    QLabel *newTab = new QLabel(m_tabWidget);
    newTab->setText(tr("No camera connected."));
    newTab->setAlignment(Qt::AlignCenter);
    m_tabWidget->addTab(newTab, "");
    m_tabWidget->setTabsClosable(false);
    // Add button to tab
    m_connectToCameraButton = new QPushButton();
    m_connectToCameraButton->setText(tr("Connect to Camera..."));
    m_tabWidget->setCornerWidget(m_connectToCameraButton, Qt::TopLeftCorner);
    connect(m_connectToCameraButton, &QPushButton::released, this, &MainWindow::connectToCamera);
    connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::disconnectCamera);
    // Set focus on button
    m_connectToCameraButton->setDefault(true);
    m_connectToCameraButton->setFocus();

    // Tab widget is central widget
    setCentralWidget(m_tabWidget);
}

void MainWindow::connectToCamera()
{
    // We cannot connect to a camera if devices are already connected and stream synchronization is in progress
    if (m_actionSynchronizeStreams->isChecked() && (m_deviceNumberMap.size() > 0) && m_sharedImageBuffer->getSyncEnabled())
    {
        // Prompt user
        QMessageBox::warning(
                this,
                APP_NAME,
                QString("%1\n\n%2").arg(tr("Stream synchronization is in progress.")).arg(tr("Please close all currently open streams before attempting to open a new stream.")),
                QMessageBox::Ok
            );
    }
    // Attempt to connect to camera
    else
    {
        // Get next tab index
        int nextTabIndex = (m_deviceNumberMap.size() == 0) ? 0 : m_tabWidget->count();

        // Show dialog
        CameraConnectDialog *cameraConnectDialog = new CameraConnectDialog(this, m_actionSynchronizeStreams->isChecked());
        int ret = cameraConnectDialog->exec();
        if(ret == QDialog::Accepted)
        {
            // Save user-defined device number
            int deviceNumber = cameraConnectDialog->getDeviceNumber();

            // Check if this camera is already connected
            if (m_deviceNumberMap.contains(deviceNumber))
            {
                QMessageBox::warning(
                        this,
                        tr("Connect to Camera"),
                        tr("Could not connect to camera. Already connected.")
                    );
            }
            else
            {
                // Create ImageBuffer with user-defined size
                Buffer<cv::Mat> *imageBuffer = new Buffer<cv::Mat>(cameraConnectDialog->getImageBufferSize());
                // Add created ImageBuffer to SharedImageBuffer object
                m_sharedImageBuffer->add(deviceNumber, imageBuffer, m_actionSynchronizeStreams->isChecked());
                // Create CameraView
                m_cameraViewMap[deviceNumber] = new CameraView(deviceNumber, m_sharedImageBuffer, m_tabWidget);

                // Check if stream synchronization is enabled
                if (m_actionSynchronizeStreams->isChecked())
                {
                    // Prompt user
                    int ret = QMessageBox::question(
                            this,
                            APP_NAME,
                            QString("%1\n\n%2\n\n%3").arg(tr("Stream synchronization is enabled.")).arg(tr("Do you want to start processing?")).arg(tr("Choose 'No' if you would like to open additional streams.")),
                            QMessageBox::Yes | QMessageBox::No,
                            QMessageBox::Yes
                        );
                    // Start processing
                    if(ret == QMessageBox::Yes)
                    {
                        m_sharedImageBuffer->setSyncEnabled(true);
                    }
                    // Defer processing
                    else
                    {
                        m_sharedImageBuffer->setSyncEnabled(false);
                    }
                }

                // Attempt to connect to camera
                bool result = m_cameraViewMap[deviceNumber]->connectToCamera(cameraConnectDialog->getDropFrameCheckBoxState(),
                    cameraConnectDialog->getCaptureThreadPrio(),
                    cameraConnectDialog->getProcessingThreadPrio(),
                    cameraConnectDialog->getEnableFrameProcessingCheckBoxState(),
                    cameraConnectDialog->getResolutionWidth(),
                    cameraConnectDialog->getResolutionHeight());
                // Success
                if (result)
                {
                    // Add to map
                    m_deviceNumberMap[deviceNumber] = nextTabIndex;
                    // Save tab label
                    QString tabLabel = cameraConnectDialog->getTabLabel();
                    // Allow tabs to be closed
                    m_tabWidget->setTabsClosable(true);
                    // If start tab, remove
                    if(nextTabIndex == 0)
                    {
                        m_tabWidget->removeTab(0);
                    }
                    // Add tab
                    m_tabWidget->addTab(m_cameraViewMap[deviceNumber], QString("%1 [%2]").arg(tabLabel).arg(deviceNumber));
                    m_tabWidget->setCurrentWidget(m_cameraViewMap[deviceNumber]);
                    // Set tooltips
                    setTabCloseToolTips(m_tabWidget, tr("Disconnect Camera"));
                    // Prevent user from enabling/disabling stream synchronization after a camera has been connected
                    m_actionSynchronizeStreams->setEnabled(false);
                }
                // Could not connect to camera
                else
                {
                    // Display error message
                    QMessageBox::warning(
                            this,
                            tr("Connect to Camera"),
                            tr("Could not connect to camera. Please check device number.")
                        );
                    // Explicitly delete widget
                    delete m_cameraViewMap[deviceNumber];
                    m_cameraViewMap.remove(deviceNumber);
                    // Remove from shared buffer
                    m_sharedImageBuffer->removeByDeviceNumber(deviceNumber);
                    // Explicitly delete ImageBuffer object
                    delete imageBuffer;
                }
            }
        }

        // Delete dialog
        delete cameraConnectDialog;
    }
}

void MainWindow::disconnectCamera(int index)
{
    bool doDisconnect = true;

    // Check if stream synchronization is enabled, more than one camera connected, and frame processing is not in progress
    if (m_actionSynchronizeStreams->isChecked() && (m_cameraViewMap.size() > 1) && !m_sharedImageBuffer->getSyncEnabled())
    {
        // Prompt user
        int ret = QMessageBox::question(
                this,
                APP_NAME,
                QString("%1\n\n%2\n\n%3").arg(tr("Stream synchronization is enabled.")).arg(tr("Disconnecting this camera will cause frame processing to begin on other streams.")).arg(tr("Do you wish to proceed?")),
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::Yes
            );
        doDisconnect = (ret == QMessageBox::Yes);
    }

    // Disconnect camera
    if(doDisconnect)
    {
        // Save number of tabs
        int nTabs = m_tabWidget->count();
        // Close tab
        m_tabWidget->removeTab(index);

        // Delete widget (CameraView) contained in tab
        delete m_cameraViewMap[m_deviceNumberMap.key(index)];
        m_cameraViewMap.remove(m_deviceNumberMap.key(index));

        // Remove from map
        removeFromMapByTabIndex(m_deviceNumberMap, index);
        // Update map (if tab closed is not last)
        if(index != (nTabs - 1))
        {
            updateMapValues(m_deviceNumberMap, index);
        }

        // If start tab, set tab as blank
        if(nTabs == 1)
        {
            QLabel *newTab = new QLabel(m_tabWidget);
            newTab->setText(tr("No camera connected."));
            newTab->setAlignment(Qt::AlignCenter);
            m_tabWidget->addTab(newTab, "");
            m_tabWidget->setTabsClosable(false);
            m_actionSynchronizeStreams->setEnabled(true);
        }
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

bool MainWindow::removeFromMapByTabIndex(QMap<int, int> &map, int tabIndex)
{
    QMutableMapIterator<int, int> i(map);
    while (i.hasNext())
    {
         i.next();
         if(i.value() == tabIndex)
         {
             i.remove();
             return true;
         }
    }

    return false;
}

void MainWindow::updateMapValues(QMap<int, int> &map, int tabIndex)
{
    QMutableMapIterator<int, int> i(map);
    while (i.hasNext())
    {
        i.next();
        if(i.value() > tabIndex)
        {
            i.setValue(i.value()-1);
        }
    }
}

void MainWindow::setTabCloseToolTips(QTabWidget *tabs, QString tooltip)
{
    QList<QAbstractButton*> allPButtons = tabs->findChildren<QAbstractButton*>();
    for (int ind = 0; ind < allPButtons.size(); ind++)
    {
        QAbstractButton* item = allPButtons.at(ind);
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