/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* CameraConnectDialog.cpp                                              */
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

// Qt header files
#include <QtGui>
// Configuration header file
#include "Config.h"

CameraConnectDialog::CameraConnectDialog(QWidget *parent) : QDialog(parent)
{
    // Setup dialog
    setupUi(this);
    // deviceNumberEdit (device number) input string validation
    QRegExp rx1("[0-9]\\d{0,2}"); // Integers 0 to 999
    QRegExpValidator *validator1 = new QRegExpValidator(rx1, 0);
    deviceNumberEdit->setValidator(validator1);
    // imageBufferSizeEdit (image buffer size) input string validation
    QRegExp rx2("[0-9]\\d{0,2}"); // Integers 0 to 999
    QRegExpValidator *validator2 = new QRegExpValidator(rx2, 0);
    imageBufferSizeEdit->setValidator(validator2);
    // Setup combo boxes
    setupComboBoxes();
    // Set to defaults
    resetToDefaults();
    // Connect button to slot
    connect(resetToDefaultsPushButton,SIGNAL(released()),SLOT(resetToDefaults()));
} // CameraConnectDialog constructor

int CameraConnectDialog::getDeviceNumber()
{
    if(cameraButtonGroup->checkedButton()==(QAbstractButton*)anyCameraButton)
        return -1;
    else
    {
        // Set device number to default (any available camera) if field is blank
        if(deviceNumberEdit->text().isEmpty())
        {
            QMessageBox::warning(this->parentWidget(), "WARNING:","Device Number field blank.\nAutomatically set to 'any available camera'.");
            return -1;
        }
        return deviceNumberEdit->text().toInt();
    }
} // getDeviceNumber()

int CameraConnectDialog::getImageBufferSize()
{
    // Set image buffer size to default if field is blank
    if(imageBufferSizeEdit->text().isEmpty())
    {
        QMessageBox::warning(this->parentWidget(), "WARNING:","Image Buffer Size field blank.\nAutomatically set to default value.");
        return DEFAULT_IMAGE_BUFFER_SIZE;
    }
    // Set image buffer size to default if field is zero
    else if(imageBufferSizeEdit->text().toInt()==0)
    {
        QMessageBox::warning(this->parentWidget(), "WARNING:","Image Buffer Size cannot be zero.\nAutomatically set to default value.");
        return DEFAULT_IMAGE_BUFFER_SIZE;;
    }
    // Use image buffer size specified by user
    else
        return imageBufferSizeEdit->text().toInt();
} // getImageBufferSize()

bool CameraConnectDialog::getDropFrameCheckBoxState()
{
    return dropFrameCheckBox->isChecked();
} // getDropFrameCheckBoxState()

void CameraConnectDialog::setupComboBoxes()
{
    // Local variables
    QStringList threadPriorities;
    // Fill combo boxes
    threadPriorities<<"Idle"<<"Lowest"<<"Low"<<"Normal"<<"High"<<"Highest"<<"Time Critical"<<"Inherit";
    capturePrioComboBox->addItems(threadPriorities);
    processingPrioComboBox->addItems(threadPriorities);
    // Set to defaults

} // setupComboBoxes()

int CameraConnectDialog::getCaptureThreadPrio()
{
    return capturePrioComboBox->currentIndex();
} // getCaptureThreadPrio()

int CameraConnectDialog::getProcessingThreadPrio()
{
    return processingPrioComboBox->currentIndex();
} // getProcessingThreadPrio()

void CameraConnectDialog::resetToDefaults()
{
    // Default camera
    if(DEFAULT_CAMERA_DEV_NO!=-1)
    {
        deviceNumberButton->setChecked(true);
        deviceNumberEdit->setEnabled(true);
        deviceNumberEdit->setText(QString::number(DEFAULT_CAMERA_DEV_NO));
    }
    else
    {
        anyCameraButton->setChecked(true);
        deviceNumberEdit->setEnabled(false);
        deviceNumberEdit->clear();
    }
    // Image buffer size
    imageBufferSizeEdit->setText(QString::number(DEFAULT_IMAGE_BUFFER_SIZE));
    // Drop frames
    dropFrameCheckBox->setChecked(DEFAULT_DROP_FRAMES);
    // Capture thread
    if(DEFAULT_CAP_THREAD_PRIO==QThread::IdlePriority)
        capturePrioComboBox->setCurrentIndex(0);
    else if(DEFAULT_CAP_THREAD_PRIO==QThread::LowestPriority)
        capturePrioComboBox->setCurrentIndex(1);
    else if(DEFAULT_CAP_THREAD_PRIO==QThread::LowPriority)
        capturePrioComboBox->setCurrentIndex(2);
    else if(DEFAULT_CAP_THREAD_PRIO==QThread::NormalPriority)
        capturePrioComboBox->setCurrentIndex(3);
    else if(DEFAULT_CAP_THREAD_PRIO==QThread::HighPriority)
        capturePrioComboBox->setCurrentIndex(4);
    else if(DEFAULT_CAP_THREAD_PRIO==QThread::HighestPriority)
        capturePrioComboBox->setCurrentIndex(5);
    else if(DEFAULT_CAP_THREAD_PRIO==QThread::TimeCriticalPriority)
        capturePrioComboBox->setCurrentIndex(6);
    else if(DEFAULT_CAP_THREAD_PRIO==QThread::InheritPriority)
        capturePrioComboBox->setCurrentIndex(7);
    // Processing thread
    if(DEFAULT_PROC_THREAD_PRIO==QThread::IdlePriority)
        processingPrioComboBox->setCurrentIndex(0);
    else if(DEFAULT_PROC_THREAD_PRIO==QThread::LowestPriority)
        processingPrioComboBox->setCurrentIndex(1);
    else if(DEFAULT_PROC_THREAD_PRIO==QThread::LowPriority)
        processingPrioComboBox->setCurrentIndex(2);
    else if(DEFAULT_PROC_THREAD_PRIO==QThread::NormalPriority)
        processingPrioComboBox->setCurrentIndex(3);
    else if(DEFAULT_PROC_THREAD_PRIO==QThread::HighPriority)
        processingPrioComboBox->setCurrentIndex(4);
    else if(DEFAULT_PROC_THREAD_PRIO==QThread::HighestPriority)
        processingPrioComboBox->setCurrentIndex(5);
    else if(DEFAULT_PROC_THREAD_PRIO==QThread::TimeCriticalPriority)
        processingPrioComboBox->setCurrentIndex(6);
    else if(DEFAULT_PROC_THREAD_PRIO==QThread::InheritPriority)
        processingPrioComboBox->setCurrentIndex(7);
} // resetToDefaults()
