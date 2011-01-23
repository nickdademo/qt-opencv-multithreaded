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

// OpenCV header files
#include <opencv/cv.h>
#include <opencv/highgui.h>

// Header file containing default values
#include "DefaultValues.h"

ProcessingSettingsDialog::ProcessingSettingsDialog(QWidget *parent) : QDialog(parent)
{
    // Setup dialog
    setupUi(this);
    // Set values to defaults
    resetToDefaults();
} // ProcessingSettingsDialog constructor

void ProcessingSettingsDialog::resetToDefaults()
{
    // Smooth
    if(SMOOTH_TYPE==CV_BLUR_NO_SCALE)
        smoothBlurNoScaleButton->setChecked(true);
    else if(SMOOTH_TYPE==CV_BLUR)
        smoothBlurButton->setChecked(true);
    else if(SMOOTH_TYPE==CV_GAUSSIAN)
        smoothGaussianButton->setChecked(true);
    else if(SMOOTH_TYPE==CV_MEDIAN)
        smoothMedianButton->setChecked(true);
    else if(SMOOTH_TYPE==CV_BILATERAL)
        smoothBilateralButton->setChecked(true);
    smoothParam1Edit->setText(QString::number(SMOOTH_PARAM_1));
    smoothParam2Edit->setText(QString::number(SMOOTH_PARAM_2));
    smoothParam3Edit->setText(QString::number(SMOOTH_PARAM_3));
    smoothParam4Edit->setText(QString::number(SMOOTH_PARAM_4));
    // Dilate
    dilateIterationsEdit->setText(QString::number(DILATE_ITERATIONS));
    // Erode
    erodeIterationsEdit->setText(QString::number(ERODE_ITERATIONS));
    // Flip
    if(FLIPMODE==0)
        flipXAxisButton->setChecked(true);
    else if(FLIPMODE==1)
        flipYAxisButton->setChecked(true);
    else if(FLIPMODE==-1)
        flipBothAxesButton->setChecked(true);
    // Canny
    cannyThresh1Edit->setText(QString::number(CANNY_THRESHOLD_1));
    cannyThresh2Edit->setText(QString::number(CANNY_THRESHOLD_2));
    cannyApertureSizeEdit->setText(QString::number(CANNY_APERTURE_SIZE));
} // resetToDefaults()
