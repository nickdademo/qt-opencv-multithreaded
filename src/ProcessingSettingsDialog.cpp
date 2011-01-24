/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* ProcessingSettingsDialog.cpp                                         */
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

#include "ProcessingSettingsDialog.h"

// Qt header files
#include <QtGui>

// Header file containing default values
#include "DefaultValues.h"

ProcessingSettingsDialog::ProcessingSettingsDialog(QWidget *parent) : QDialog(parent)
{
    // Setup dialog
    setupUi(this);
    // Connect signals and slots
    connect(resetToDefaultsButton,SIGNAL(released()),SLOT(resetDialogToDefaults()));
    // Set GUI values to defaults
    resetDialogToDefaults();
    // Update values in ProcessingSettings structure
    updateStoredSettingsFromDialog();
} // ProcessingSettingsDialog constructor

void ProcessingSettingsDialog::updateStoredSettingsFromDialog()
{
    // Smooth
    if(smoothTypeGroup->checkedButton()==(QAbstractButton*)smoothBlurNoScaleButton)
        processingSettings.smoothType=CV_BLUR_NO_SCALE;
    else if(smoothTypeGroup->checkedButton()==(QAbstractButton*)smoothBlurButton)
        processingSettings.smoothType=CV_BLUR;
    else if(smoothTypeGroup->checkedButton()==(QAbstractButton*)smoothGaussianButton)
        processingSettings.smoothType=CV_GAUSSIAN;
    else if(smoothTypeGroup->checkedButton()==(QAbstractButton*)smoothMedianButton)
        processingSettings.smoothType=CV_MEDIAN;
    else if(smoothTypeGroup->checkedButton()==(QAbstractButton*)smoothBilateralButton)
        processingSettings.smoothType=CV_BILATERAL;
    processingSettings.smoothParam1=smoothParam1Edit->text().toInt();
    processingSettings.smoothParam2=smoothParam2Edit->text().toInt();
    processingSettings.smoothParam3=smoothParam3Edit->text().toInt();
    processingSettings.smoothParam4=smoothParam4Edit->text().toInt();
    // Dilate
    processingSettings.dilateNumberOfIterations=dilateIterationsEdit->text().toInt();
    // Erode
    processingSettings.erodeNumberOfIterations=erodeIterationsEdit->text().toInt();
    // Flip
    if(flipModeGroup->checkedButton()==(QAbstractButton*)flipXAxisButton)
        processingSettings.flipMode=0;
    else if(flipModeGroup->checkedButton()==(QAbstractButton*)flipYAxisButton)
        processingSettings.flipMode=1;
    else if(flipModeGroup->checkedButton()==(QAbstractButton*)flipBothAxesButton)
        processingSettings.flipMode=-1;
    // Canny
    processingSettings.cannyThreshold1=cannyThresh1Edit->text().toDouble();
    processingSettings.cannyThreshold2=cannyThresh2Edit->text().toDouble();
    processingSettings.cannyApertureSize=cannyApertureSizeEdit->text().toInt();
} // updateStoredSettingsFromDialog()

void ProcessingSettingsDialog::updateDialogSettingsFromStored()
{
    // Smooth
    if(processingSettings.smoothType==CV_BLUR_NO_SCALE)
        smoothBlurNoScaleButton->setChecked(true);
    else if(processingSettings.smoothType==CV_BLUR)
        smoothBlurButton->setChecked(true);
    else if(processingSettings.smoothType==CV_GAUSSIAN)
        smoothGaussianButton->setChecked(true);
    else if(processingSettings.smoothType==CV_MEDIAN)
        smoothMedianButton->setChecked(true);
    else if(processingSettings.smoothType==CV_BILATERAL)
        smoothBilateralButton->setChecked(true);
    smoothParam1Edit->setText(QString::number(processingSettings.smoothParam1));
    smoothParam2Edit->setText(QString::number(processingSettings.smoothParam2));
    smoothParam3Edit->setText(QString::number(processingSettings.smoothParam3));
    smoothParam4Edit->setText(QString::number(processingSettings.smoothParam4));
    // Dilate
    dilateIterationsEdit->setText(QString::number(processingSettings.dilateNumberOfIterations));
    // Erode
    erodeIterationsEdit->setText(QString::number(processingSettings.erodeNumberOfIterations));
    // Flip
    if(processingSettings.flipMode==0)
        flipXAxisButton->setChecked(true);
    else if(processingSettings.flipMode==1)
        flipYAxisButton->setChecked(true);
    else if(processingSettings.flipMode==-1)
        flipBothAxesButton->setChecked(true);
    // Canny
    cannyThresh1Edit->setText(QString::number(processingSettings.cannyThreshold1));
    cannyThresh2Edit->setText(QString::number(processingSettings.cannyThreshold2));
    cannyApertureSizeEdit->setText(QString::number(processingSettings.cannyApertureSize));;
} // updateDialogSettingsFromStored()

void ProcessingSettingsDialog::resetDialogToDefaults()
{
    // Smooth
    if(DEFAULT_SMOOTH_TYPE==CV_BLUR_NO_SCALE)
        smoothBlurNoScaleButton->setChecked(true);
    else if(DEFAULT_SMOOTH_TYPE==CV_BLUR)
        smoothBlurButton->setChecked(true);
    else if(DEFAULT_SMOOTH_TYPE==CV_GAUSSIAN)
        smoothGaussianButton->setChecked(true);
    else if(DEFAULT_SMOOTH_TYPE==CV_MEDIAN)
        smoothMedianButton->setChecked(true);
    else if(DEFAULT_SMOOTH_TYPE==CV_BILATERAL)
        smoothBilateralButton->setChecked(true);
    smoothParam1Edit->setText(QString::number(DEFAULT_SMOOTH_PARAM_1));
    smoothParam2Edit->setText(QString::number(DEFAULT_SMOOTH_PARAM_2));
    smoothParam3Edit->setText(QString::number(DEFAULT_SMOOTH_PARAM_3));
    smoothParam4Edit->setText(QString::number(DEFAULT_SMOOTH_PARAM_4));
    // Dilate
    dilateIterationsEdit->setText(QString::number(DEFAULT_DILATE_ITERATIONS));
    // Erode
    erodeIterationsEdit->setText(QString::number(DEFAULT_ERODE_ITERATIONS));
    // Flip
    if(DEFAULT_FLIP_MODE==0)
        flipXAxisButton->setChecked(true);
    else if(DEFAULT_FLIP_MODE==1)
        flipYAxisButton->setChecked(true);
    else if(DEFAULT_FLIP_MODE==-1)
        flipBothAxesButton->setChecked(true);
    // Canny
    cannyThresh1Edit->setText(QString::number(DEFAULT_CANNY_THRESHOLD_1));
    cannyThresh2Edit->setText(QString::number(DEFAULT_CANNY_THRESHOLD_2));
    cannyApertureSizeEdit->setText(QString::number(DEFAULT_CANNY_APERTURE_SIZE));
} // resetDialogToDefaults()
