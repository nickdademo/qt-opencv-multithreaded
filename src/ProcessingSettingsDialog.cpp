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
    // Connect GUI signals and slots
    connect(resetToDefaultsButton,SIGNAL(released()),SLOT(resetDialogToDefaults()));
    connect(applyButton,SIGNAL(released()),SLOT(updateStoredSettingsFromDialog()));
    connect(smoothTypeGroup,SIGNAL(buttonReleased(QAbstractButton*)),SLOT(smoothTypeChange(QAbstractButton*)));
    // dilateIterationsEdit input string validation
    QRegExp rx5("[1-9]\\d{0,1}"); // Integers 1 to 99
    QRegExpValidator *validator5 = new QRegExpValidator(rx5, 0);
    dilateIterationsEdit->setValidator(validator5);
    // erodeIterationsEdit input string validation
    QRegExp rx6("[1-9]\\d{0,1}"); // Integers 1 to 99
    QRegExpValidator *validator6 = new QRegExpValidator(rx6, 0);
    erodeIterationsEdit->setValidator(validator6);
    // cannyThresh1Edit input string validation
    QRegExp rx7("[0-9]\\d{0,2}"); // Integers 0 to 999
    QRegExpValidator *validator7 = new QRegExpValidator(rx7, 0);
    cannyThresh1Edit->setValidator(validator7);
    // cannyThresh2Edit input string validation
    QRegExp rx8("[0-9]\\d{0,2}"); // Integers 0 to 999
    QRegExpValidator *validator8 = new QRegExpValidator(rx8, 0);
    cannyThresh2Edit->setValidator(validator8);
    // cannyApertureSizeEdit input string validation
    QRegExp rx9("[0-9]\\d{0,2}"); // Integers 0 to 99
    QRegExpValidator *validator9 = new QRegExpValidator(rx9, 0);
    cannyApertureSizeEdit->setValidator(validator9);
    // Set dialog values to defaults
    resetDialogToDefaults();
    // Update values in ProcessingSettings structure
    updateStoredSettingsFromDialog();
} // ProcessingSettingsDialog constructor

void ProcessingSettingsDialog::updateStoredSettingsFromDialog()
{
    // Validate values in dialog before storing
    validateDialog();
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
    processingSettings.smoothParam3=smoothParam3Edit->text().toDouble();
    processingSettings.smoothParam4=smoothParam4Edit->text().toDouble();
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
    cannyApertureSizeEdit->setText(QString::number(processingSettings.cannyApertureSize));
    // Enable/disable appropriate Smooth parameter inputs
    smoothTypeChange(smoothTypeGroup->checkedButton());
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
    // Enable/disable appropriate Smooth parameter inputs
    smoothTypeChange(smoothTypeGroup->checkedButton());
} // resetDialogToDefaults()

void ProcessingSettingsDialog::smoothTypeChange(QAbstractButton *input)
{
    if(input==(QAbstractButton*)smoothBlurNoScaleButton)
    {
        // smoothParam1Edit input string validation
        QRegExp rx1("[1-9]\\d{0,1}"); // Integers 1 to 99
        QRegExpValidator *validator1 = new QRegExpValidator(rx1, 0);
        smoothParam1Edit->setValidator(validator1);
        // smoothParam2Edit input string validation
        QRegExp rx2("[0-9]\\d{0,1}"); // Integers 0 to 99
        QRegExpValidator *validator2 = new QRegExpValidator(rx2, 0);
        smoothParam2Edit->setValidator(validator2);
        // Enable/disable appropriate parameter inputs
        smoothParam1Edit->setEnabled(true);
        smoothParam2Edit->setEnabled(true);
        smoothParam3Edit->setEnabled(false);
        smoothParam4Edit->setEnabled(false);
    }
    else if(input==(QAbstractButton*)smoothBlurButton)
    {
        // smoothParam1Edit input string validation
        QRegExp rx1("[1-9]\\d{0,1}"); // Integers 1 to 99
        QRegExpValidator *validator1 = new QRegExpValidator(rx1, 0);
        smoothParam1Edit->setValidator(validator1);
        // smoothParam2Edit input string validation
        QRegExp rx2("[0-9]\\d{0,1}"); // Integers 0 to 99
        QRegExpValidator *validator2 = new QRegExpValidator(rx2, 0);
        smoothParam2Edit->setValidator(validator2);
        // Enable/disable appropriate parameter inputs
        smoothParam1Edit->setEnabled(true);
        smoothParam2Edit->setEnabled(true);
        smoothParam3Edit->setEnabled(false);
        smoothParam4Edit->setEnabled(false);
    }
    else if(input==(QAbstractButton*)smoothGaussianButton)
    {
        // smoothParam1Edit input string validation
        QRegExp rx1("[0-9]\\d{0,1}"); // Integers 0 to 99
        QRegExpValidator *validator1 = new QRegExpValidator(rx1, 0);
        smoothParam1Edit->setValidator(validator1);
        // smoothParam2Edit input string validation
        QRegExp rx2("[0-9]\\d{0,1}"); // Integers 0 to 99
        QRegExpValidator *validator2 = new QRegExpValidator(rx2, 0);
        smoothParam2Edit->setValidator(validator2);
        // smoothParam3Edit input string validation
        QDoubleValidator *validator3 = new QDoubleValidator(0.0, 99.0, 2, this);
        validator3->setNotation(QDoubleValidator::StandardNotation);
        smoothParam3Edit->setValidator(validator3);
        // Enable/disable appropriate parameter inputs
        smoothParam1Edit->setEnabled(true);
        smoothParam2Edit->setEnabled(true);
        smoothParam3Edit->setEnabled(true);
        smoothParam4Edit->setEnabled(false);
    }
    else if(input==(QAbstractButton*)smoothMedianButton)
    {
        // smoothParam1Edit input string validation
        QRegExp rx1("[1-9]\\d{0,1}"); // Integers 1 to 99
        QRegExpValidator *validator1 = new QRegExpValidator(rx1, 0);
        smoothParam1Edit->setValidator(validator1);
        // Enable/disable appropriate parameter inputs
        smoothParam1Edit->setEnabled(true);
        smoothParam2Edit->setEnabled(false);
        smoothParam3Edit->setEnabled(false);
        smoothParam4Edit->setEnabled(false);
    }
    else if(input==(QAbstractButton*)smoothBilateralButton)
    {
        // smoothParam1Edit input string validation
        QRegExp rx1("[0-9]\\d{0,1}"); // Integers 0 to 99
        QRegExpValidator *validator1 = new QRegExpValidator(rx1, 0);
        smoothParam1Edit->setValidator(validator1);
        // smoothParam3Edit input string validation
        QDoubleValidator *validator3 = new QDoubleValidator(0.0, 99.0, 2, this);
        validator3->setNotation(QDoubleValidator::StandardNotation);
        smoothParam3Edit->setValidator(validator3);
        // smoothParam4Edit input string validation
        QDoubleValidator *validator4 = new QDoubleValidator(0.0, 99.0, 2, this);
        validator4->setNotation(QDoubleValidator::StandardNotation);
        smoothParam4Edit->setValidator(validator4);
        // Enable/disable appropriate parameter inputs
        smoothParam1Edit->setEnabled(true);
        smoothParam2Edit->setEnabled(false);
        smoothParam3Edit->setEnabled(true);
        smoothParam4Edit->setEnabled(true);
    }
} // smoothTypeChange()

void ProcessingSettingsDialog::validateDialog()
{
    // If value of Smooth parameter 1 is EVEN (and not zero), convert to ODD by adding 1
    if(((smoothParam1Edit->text().toInt()%2)==0)&&(smoothParam1Edit->text().toInt()!=0))
        smoothParam1Edit->setText(QString::number(smoothParam1Edit->text().toInt()+1));
    // If value of Smooth parameter 2 is EVEN (and not zero), convert to ODD by adding 1
    if(((smoothParam2Edit->text().toInt()%2)==0)&&(smoothParam2Edit->text().toInt()!=0))
        smoothParam2Edit->setText(QString::number(smoothParam2Edit->text().toInt()+1));

    // Check for empty inputs: if empty, set to default values
    if(smoothParam1Edit->text().isEmpty())
        smoothParam1Edit->setText(QString::number(DEFAULT_SMOOTH_PARAM_1));
    if(smoothParam2Edit->text().isEmpty())
        smoothParam2Edit->setText(QString::number(DEFAULT_SMOOTH_PARAM_2));
    if(smoothParam3Edit->text().isEmpty())
        smoothParam3Edit->setText(QString::number(DEFAULT_SMOOTH_PARAM_3));
    if(smoothParam4Edit->text().isEmpty())
        smoothParam4Edit->setText(QString::number(DEFAULT_SMOOTH_PARAM_4));
    if(dilateIterationsEdit->text().isEmpty())
        dilateIterationsEdit->setText(QString::number(DEFAULT_DILATE_ITERATIONS));
    if(erodeIterationsEdit->text().isEmpty())
        erodeIterationsEdit->setText(QString::number(DEFAULT_ERODE_ITERATIONS));
    if(cannyThresh1Edit->text().isEmpty())
        cannyThresh1Edit->setText(QString::number(DEFAULT_CANNY_THRESHOLD_1));
    if(cannyThresh2Edit->text().isEmpty())
        cannyThresh2Edit->setText(QString::number(DEFAULT_CANNY_THRESHOLD_2));
    if(cannyApertureSizeEdit->text().isEmpty())
        cannyApertureSizeEdit->setText(QString::number(DEFAULT_CANNY_APERTURE_SIZE));

    // Check for special Smooth parameter cases
    if((smoothTypeGroup->checkedButton()==(QAbstractButton*)smoothGaussianButton)&&
       (smoothParam1Edit->text().toInt()==0)&&(smoothParam3Edit->text().toInt()==0))
    {
        smoothParam1Edit->setText(QString::number(DEFAULT_SMOOTH_PARAM_1));
        smoothParam3Edit->setText(QString::number(DEFAULT_SMOOTH_PARAM_3));
        QMessageBox::warning(this->parentWidget(),"ERROR:","Parameters 1 and 3 cannot BOTH be zero when the smoothing type is GAUSSIAN.\n\nAutomatically set to default values.");
    }
    else if((smoothTypeGroup->checkedButton()==(QAbstractButton*)smoothBilateralButton)&&
            (smoothParam1Edit->text().toInt()==0)&&(smoothParam4Edit->text().toInt()==0))
    {
        smoothParam1Edit->setText(QString::number(DEFAULT_SMOOTH_PARAM_1));
        smoothParam4Edit->setText(QString::number(DEFAULT_SMOOTH_PARAM_4));
        QMessageBox::warning(this->parentWidget(),"ERROR:","Parameters 1 and 4 cannot BOTH be zero when the smoothing type is BILATERAL.\n\nAutomatically set to default values.");
    }
} // validateDialog()
