/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* MainWindow.cpp                                                       */
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

#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // Setup UI
    ui->setupUi(this);
    // Set start tab as blank
    QLabel *newTab = new QLabel(ui->tabWidget);
    newTab->setText("No camera connected.");
    newTab->setAlignment(Qt::AlignCenter);
    ui->tabWidget->addTab(newTab, "");
    ui->tabWidget->setTabsClosable(false);
    // Add "Connect to Camera" button to tab
    connectToCameraButton = new QPushButton();
    connectToCameraButton->setText("Connect to Camera...");
    ui->tabWidget->setCornerWidget(connectToCameraButton, Qt::TopLeftCorner);
    connect(connectToCameraButton,SIGNAL(released()),this, SLOT(connectToCamera()));
    connect(ui->tabWidget,SIGNAL(tabCloseRequested(int)),this, SLOT(disconnectCamera(int)));
    // Set focus on button
    connectToCameraButton->setFocus();
    // Connect other signals/slots
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(showAboutDialog()));
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::connectToCamera()
{
    // Get next camera ID
    int nextCameraID = getNextCameraID();
    // Get next tab index
    int nextTabIndex = nextCameraID==1 ? 0 : ui->tabWidget->count();
    // Show dialog
    CameraConnectDialog *cameraConnectDialog = new CameraConnectDialog();
    if(cameraConnectDialog->exec()==QDialog::Accepted)
    {
        // Save user-defined device number
        int deviceNumber = cameraConnectDialog->getDeviceNumber();
        // Check if this camera is already connected
        if(!deviceIDMap.contains(deviceNumber))
        {
            CameraView *cameraView = new CameraView(ui->tabWidget, nextCameraID);
            // Attempt to connect to camera
            if(cameraView->connectToCamera(deviceNumber,
                                           cameraConnectDialog->getImageBufferSize(),
                                           cameraConnectDialog->getDropFrameCheckBoxState(),
                                           cameraConnectDialog->getCaptureThreadPrio(),
                                           cameraConnectDialog->getProcessingThreadPrio()))
            {
                // Add to maps
                cameraIDMap[nextCameraID] = nextTabIndex;
                deviceIDMap[deviceNumber] = nextTabIndex;
                // Save tab label
                QString tabLabel = cameraConnectDialog->getTabLabel();
                // Allow tabs to be closed
                ui->tabWidget->setTabsClosable(true);
                // If start tab, remove
                if(nextCameraID==1)
                    ui->tabWidget->removeTab(0);
                // Add tab
                ui->tabWidget->addTab(cameraView, tabLabel + " [" + QString::number(nextCameraID) + "]");
                ui->tabWidget->setCurrentWidget(cameraView);
                // Set tooltips
                setTabCloseToolTips(ui->tabWidget, "Disconnect Camera");
            }
            // Could not connect to camera
            else
            {
                // Display error message
                QMessageBox::warning(this,"ERROR:","Could not connect to camera. Please check device number.");
                // Explicitly delete widget
                delete cameraView;
            }
        }
        else
        {
            // Display error message
            QMessageBox::warning(this,"ERROR:","Could not connect to camera. Already connected.");
        }
    }
    // Delete dialog
    delete cameraConnectDialog;
}

void MainWindow::disconnectCamera(int index)
{
    // Save number of tabs
    int nTabs = ui->tabWidget->count();
    // Delete widget (CameraView) contained in tab
    ui->tabWidget->setCurrentIndex(index);
    ui->tabWidget->currentWidget()->deleteLater();
    // Close tab
    ui->tabWidget->removeTab(index);
    // If start tab, set tab as blank
    if(nTabs==1)
    {
        QLabel *newTab = new QLabel(ui->tabWidget);
        newTab->setText("No camera connected.");
        newTab->setAlignment(Qt::AlignCenter);
        ui->tabWidget->addTab(newTab, "");
        ui->tabWidget->setTabsClosable(false);
    }
    // Remove from maps
    removeIDFromMap(cameraIDMap, index);
    removeIDFromMap(deviceIDMap, index);
}

void MainWindow::showAboutDialog()
{
    QMessageBox::information(this, "About", QString("Created by Nick D'Ademo\n\nContact: nickdademo@gmail.com\nWebsite: www.nickdademo.com\n\nVersion: %1").arg(APP_VERSION));
}

int MainWindow::getNextCameraID()
{
    for(int i=1; i<=cameraIDMap.size()+1; i++)
    {
        if(!cameraIDMap.contains(i))
            return i;
    }
    return 1;
}

bool MainWindow::removeIDFromMap(QMap<int, int>& map, int tabIndex)
{
    QMutableMapIterator<int, int> i(map);
    while (i.hasNext())
    {
         i.next();
         if(i.value()==tabIndex)
         {
             i.remove();
             return true;
         }
    }
    return false;
}

void MainWindow::setTabCloseToolTips(QTabWidget *tabs, QString tooltip)
{
    QList<QAbstractButton*> allPButtons = tabs->findChildren<QAbstractButton*>();
    for (int ind = 0; ind < allPButtons.size(); ind++)
    {
        QAbstractButton* item = allPButtons.at(ind);
        if (item->inherits("CloseButton"))
            item->setToolTip(tooltip);
    }
}
