/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* CaptureThread.cpp                                                    */
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

// Qt header files
#include <QtGui>

// Header file containing default values
#include "DefaultValues.h"

CameraConnectDialog::CameraConnectDialog(QWidget *parent) : QDialog(parent)
{
    // Setup dialog
    setupUi(this);
    // On dialog creation, reset auto set flags to FALSE
    autoSetDeviceNumber=false;
    autoSetImageBufferSize=false;
    // deviceNumberEdit (device number) input string validation
    QRegExp rx1("[0-9]\\d{0,2}"); // Integers 0 to 999
    QRegExpValidator *validator1 = new QRegExpValidator(rx1, 0);
    deviceNumberEdit->setValidator(validator1);
    // imageBufferSizeEdit (image buffer size) input string validation
    QRegExp rx2("[0-9]\\d{0,2}"); // Integers 0 to 999
    QRegExpValidator *validator2 = new QRegExpValidator(rx2, 0);
    imageBufferSizeEdit->setValidator(validator2);
    // Set imageBufferSizeEdit to default value
    imageBufferSizeEdit->setText(QString::number(DEFAULT_IMAGE_BUFFER_SIZE));
} // CameraConnectDialog constructor

int CameraConnectDialog::getDeviceNumber()
{
    // Initially set device number to "any available camera".
    deviceNumber=-1;
    if(anyCameraButton->isChecked())
        return deviceNumber; // Attempt to connect to any available camera
    else // User specified camera
    {
        if(deviceNumberEdit->text().isEmpty()) // Set device number to default if field is blank
        {
            if(!autoSetDeviceNumber)
                QMessageBox::warning(this->parentWidget(), "WARNING:","Device Number field blank.\nAutomatically set to 'any available camera'.");
            autoSetDeviceNumber=true;
            return deviceNumber;
        } // if
        else
            return deviceNumber=deviceNumberEdit->text().toInt(); // Attempt to connect to camera specified by user
    } // else
} // getDeviceNumber()

int CameraConnectDialog::getImageBufferSize()
{
    // Initially set image buffer size to default value
    imageBufferSize=DEFAULT_IMAGE_BUFFER_SIZE;
    if(imageBufferSizeEdit->text().isEmpty()) // Set image buffer size to default if field is blank
    {
        if(!autoSetImageBufferSize)
            QMessageBox::warning(this->parentWidget(), "WARNING:","Image Buffer Size field blank.\nAutomatically set to default value.");
        autoSetImageBufferSize=true;
        return imageBufferSize;
    } // if
    else if(imageBufferSizeEdit->text().toInt()==0)
    {
        if(!autoSetImageBufferSize)
            QMessageBox::warning(this->parentWidget(), "WARNING:","Image Buffer Size cannot be zero.\nAutomatically set to default value.");
        autoSetImageBufferSize=true;
        return imageBufferSize;
    }
    else
        return imageBufferSize=imageBufferSizeEdit->text().toInt(); // Use image buffer size specified by user
} // getImageBufferSize()
