/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* ImageProcessingSettingsDialog.cpp                                    */
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

#include "ImageProcessingSettingsDialog.h"

// Qt header files
#include <QtGui>
// Configuration header file
#include "Config.h"

ImageProcessingSettingsDialog::ImageProcessingSettingsDialog(QWidget *parent) : QDialog(parent)
{
    // Setup dialog
    setupUi(this);
    // Connect GUI signals and slots
    connect(resetAllToDefaultsButton,SIGNAL(released()),SLOT(resetAllDialogToDefaults()));
    connect(resetSmoothToDefaultsButton,SIGNAL(released()),SLOT(resetSmoothDialogToDefaults()));
    connect(resetDilateToDefaultsButton,SIGNAL(released()),SLOT(resetDilateDialogToDefaults()));
    connect(resetErodeToDefaultsButton,SIGNAL(released()),SLOT(resetErodeDialogToDefaults()));
    connect(resetFlipToDefaultsButton,SIGNAL(released()),SLOT(resetFlipDialogToDefaults()));
    connect(resetCannyToDefaultsButton,SIGNAL(released()),SLOT(resetCannyDialogToDefaults()));
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
    QRegExp rx9("[3,5,7]\\d{0,0}"); // Integers 3,5,7
    QRegExpValidator *validator9 = new QRegExpValidator(rx9, 0);
    cannyApertureSizeEdit->setValidator(validator9);
    // Set dialog values to defaults
    resetAllDialogToDefaults();
    // Update image processing settings in imageProcessingSettings structure and processingThread
    updateStoredSettingsFromDialog();
} // ImageProcessingSettingsDialog constructor

void ImageProcessingSettingsDialog::updateStoredSettingsFromDialog()
{
    // Validate values in dialog before storing
    validateDialog();
    // Smooth
    if(smoothTypeGroup->checkedButton()==(QAbstractButton*)smoothBlurButton)
        imageProcessingSettings.smoothType=0;
    else if(smoothTypeGroup->checkedButton()==(QAbstractButton*)smoothGaussianButton)
        imageProcessingSettings.smoothType=1;
    else if(smoothTypeGroup->checkedButton()==(QAbstractButton*)smoothMedianButton)
        imageProcessingSettings.smoothType=2;
    imageProcessingSettings.smoothParam1=smoothParam1Edit->text().toInt();
    imageProcessingSettings.smoothParam2=smoothParam2Edit->text().toInt();
    imageProcessingSettings.smoothParam3=smoothParam3Edit->text().toDouble();
    imageProcessingSettings.smoothParam4=smoothParam4Edit->text().toDouble();
    // Dilate
    imageProcessingSettings.dilateNumberOfIterations=dilateIterationsEdit->text().toInt();
    // Erode
    imageProcessingSettings.erodeNumberOfIterations=erodeIterationsEdit->text().toInt();
    // Flip
    if(flipCodeGroup->checkedButton()==(QAbstractButton*)flipXAxisButton)
        imageProcessingSettings.flipCode=0;
    else if(flipCodeGroup->checkedButton()==(QAbstractButton*)flipYAxisButton)
        imageProcessingSettings.flipCode=1;
    else if(flipCodeGroup->checkedButton()==(QAbstractButton*)flipBothAxesButton)
        imageProcessingSettings.flipCode=-1;
    // Canny
    imageProcessingSettings.cannyThreshold1=cannyThresh1Edit->text().toDouble();
    imageProcessingSettings.cannyThreshold2=cannyThresh2Edit->text().toDouble();
    imageProcessingSettings.cannyApertureSize=cannyApertureSizeEdit->text().toInt();
    imageProcessingSettings.cannyL2gradient=cannyL2NormCheckBox->isChecked();
    // Update image processing flags in processingThread
    emit newImageProcessingSettings(imageProcessingSettings);
} // updateStoredSettingsFromDialog()

void ImageProcessingSettingsDialog::updateDialogSettingsFromStored()
{
    // Smooth
    if(imageProcessingSettings.smoothType==0)
        smoothBlurButton->setChecked(true);
    else if(imageProcessingSettings.smoothType==1)
        smoothGaussianButton->setChecked(true);
    else if(imageProcessingSettings.smoothType==2)
        smoothMedianButton->setChecked(true);
    smoothParam1Edit->setText(QString::number(imageProcessingSettings.smoothParam1));
    smoothParam2Edit->setText(QString::number(imageProcessingSettings.smoothParam2));
    smoothParam3Edit->setText(QString::number(imageProcessingSettings.smoothParam3));
    smoothParam4Edit->setText(QString::number(imageProcessingSettings.smoothParam4));
    // Dilate
    dilateIterationsEdit->setText(QString::number(imageProcessingSettings.dilateNumberOfIterations));
    // Erode
    erodeIterationsEdit->setText(QString::number(imageProcessingSettings.erodeNumberOfIterations));
    // Flip
    if(imageProcessingSettings.flipCode==0)
        flipXAxisButton->setChecked(true);
    else if(imageProcessingSettings.flipCode==1)
        flipYAxisButton->setChecked(true);
    else if(imageProcessingSettings.flipCode==-1)
        flipBothAxesButton->setChecked(true);
    // Canny
    cannyThresh1Edit->setText(QString::number(imageProcessingSettings.cannyThreshold1));
    cannyThresh2Edit->setText(QString::number(imageProcessingSettings.cannyThreshold2));
    cannyApertureSizeEdit->setText(QString::number(imageProcessingSettings.cannyApertureSize));
    cannyL2NormCheckBox->setChecked(imageProcessingSettings.cannyL2gradient);
    // Enable/disable appropriate Smooth parameter inputs
    smoothTypeChange(smoothTypeGroup->checkedButton());
} // updateDialogSettingsFromStored()

void ImageProcessingSettingsDialog::resetAllDialogToDefaults()
{
    // Smooth
    resetSmoothDialogToDefaults();
    // Dilate
    resetDilateDialogToDefaults();
    // Erode
    resetErodeDialogToDefaults();
    // Flip
    resetFlipDialogToDefaults();
    // Canny
    resetCannyDialogToDefaults();
} // resetAllDialogToDefaults()

void ImageProcessingSettingsDialog::smoothTypeChange(QAbstractButton *input)
{
    if(input==(QAbstractButton*)smoothBlurButton)
    {
        // smoothParam1Edit input string validation
        QRegExp rx1("[1-9]\\d{0,1}"); // Integers 1 to 99
        QRegExpValidator *validator1 = new QRegExpValidator(rx1, 0);
        smoothParam1Edit->setValidator(validator1);
        // smoothParam2Edit input string validation
        QRegExp rx2("[1-9]\\d{0,1}"); // Integers 1 to 99
        QRegExpValidator *validator2 = new QRegExpValidator(rx2, 0);
        smoothParam2Edit->setValidator(validator2);
        // Enable/disable appropriate parameter inputs
        smoothParam1Edit->setEnabled(true);
        smoothParam2Edit->setEnabled(true);
        smoothParam3Edit->setEnabled(false);
        smoothParam4Edit->setEnabled(false);
        // Set parameter range labels
        smoothParam1RangeLabel->setText("[1-99]");
        smoothParam2RangeLabel->setText("[1-99]");
        smoothParam3RangeLabel->setText("");
        smoothParam4RangeLabel->setText("");
        // Set parameter labels
        smoothParam1Label->setText("Kernel Width");
        smoothParam2Label->setText("Kernel Height");
        smoothParam3Label->setText("");
        smoothParam4Label->setText("");
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
        QDoubleValidator *validator3 = new QDoubleValidator(0.0, 99.99, 2, this);
        validator3->setNotation(QDoubleValidator::StandardNotation);
        smoothParam3Edit->setValidator(validator3);
        // Enable/disable appropriate parameter inputs
        smoothParam1Edit->setEnabled(true);
        smoothParam2Edit->setEnabled(true);
        smoothParam3Edit->setEnabled(true);
        smoothParam4Edit->setEnabled(true);
        // Set parameter range labels
        smoothParam1RangeLabel->setText("[0-99]");
        smoothParam2RangeLabel->setText("[0-99]");
        smoothParam3RangeLabel->setText("[0.00-99.99]");
        smoothParam4RangeLabel->setText("[0.00-99.99]");
        // Set parameter labels
        smoothParam1Label->setText("Kernel Width");
        smoothParam2Label->setText("Kernel Height");
        smoothParam3Label->setText("Sigma X");
        smoothParam4Label->setText("Sigma Y");
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
        // Set parameter range labels
        smoothParam1RangeLabel->setText("[1-99]");
        smoothParam2RangeLabel->setText("");
        smoothParam3RangeLabel->setText("");
        smoothParam4RangeLabel->setText("");
        // Set parameter labels
        smoothParam1Label->setText("Kernel (Square)");
        smoothParam2Label->setText("");
        smoothParam3Label->setText("");
        smoothParam4Label->setText("");
    }
} // smoothTypeChange()

void ImageProcessingSettingsDialog::validateDialog()
{
    // Local variables
    bool inputEmpty=false;

    // If value of Smooth parameter 1 is EVEN (and not zero), convert to ODD by adding 1
    if(((smoothParam1Edit->text().toInt()%2)==0)&&(smoothParam1Edit->text().toInt()!=0))
    {
        smoothParam1Edit->setText(QString::number(smoothParam1Edit->text().toInt()+1));
        QMessageBox::information(this->parentWidget(),"NOTE:","Smooth parameter 1 must be an ODD number.\n\nAutomatically set to (inputted value+1).");
    }
    // If value of Smooth parameter 2 is EVEN (and not zero), convert to ODD by adding 1
    if(((smoothParam2Edit->text().toInt()%2)==0)&&(smoothParam2Edit->text().toInt()!=0))
    {
        smoothParam2Edit->setText(QString::number(smoothParam2Edit->text().toInt()+1));
        QMessageBox::information(this->parentWidget(),"NOTE:","Smooth parameter 2 must be an ODD number (or zero).\n\nAutomatically set to (inputted value+1).");
    }

    // Check for empty inputs: if empty, set to default values
    if(smoothParam1Edit->text().isEmpty())
    {
        smoothParam1Edit->setText(QString::number(DEFAULT_SMOOTH_PARAM_1));
        inputEmpty=true;
    }
    if(smoothParam2Edit->text().isEmpty())
    {
        smoothParam2Edit->setText(QString::number(DEFAULT_SMOOTH_PARAM_2));
        inputEmpty=true;
    }
    if(smoothParam3Edit->text().isEmpty())
    {
        smoothParam3Edit->setText(QString::number(DEFAULT_SMOOTH_PARAM_3));
        inputEmpty=true;
    }
    if(smoothParam4Edit->text().isEmpty())
    {
        smoothParam4Edit->setText(QString::number(DEFAULT_SMOOTH_PARAM_4));
        inputEmpty=true;
    }
    if(dilateIterationsEdit->text().isEmpty())
    {
        dilateIterationsEdit->setText(QString::number(DEFAULT_DILATE_ITERATIONS));
        inputEmpty=true;
    }
    if(erodeIterationsEdit->text().isEmpty())
    {
        erodeIterationsEdit->setText(QString::number(DEFAULT_ERODE_ITERATIONS));
        inputEmpty=true;
    }
    if(cannyThresh1Edit->text().isEmpty())
    {
        cannyThresh1Edit->setText(QString::number(DEFAULT_CANNY_THRESHOLD_1));
        inputEmpty=true;
    }
    if(cannyThresh2Edit->text().isEmpty())
    {
        cannyThresh2Edit->setText(QString::number(DEFAULT_CANNY_THRESHOLD_2));
        inputEmpty=true;
    }
    if(cannyApertureSizeEdit->text().isEmpty())
    {
        cannyApertureSizeEdit->setText(QString::number(DEFAULT_CANNY_APERTURE_SIZE));
        inputEmpty=true;
    }
    // Check if any of the inputs were empty
    if(inputEmpty)
        QMessageBox::warning(this->parentWidget(),"WARNING:","One or more inputs empty.\n\nAutomatically set to default values.");

    // Check for special parameter cases when smoothing type is GAUSSIAN
    if((smoothTypeGroup->checkedButton()==(QAbstractButton*)smoothGaussianButton)&&
       (smoothParam1Edit->text().toInt()==0)&&(smoothParam3Edit->text().toDouble()==0.00))
    {
        smoothParam1Edit->setText(QString::number(DEFAULT_SMOOTH_PARAM_1));
        smoothParam3Edit->setText(QString::number(DEFAULT_SMOOTH_PARAM_3));
        QMessageBox::warning(this->parentWidget(),"ERROR:","Parameters 1 and 3 cannot BOTH be zero when the smoothing type is GAUSSIAN.\n\nAutomatically set to default values.");
    }
    if((smoothTypeGroup->checkedButton()==(QAbstractButton*)smoothGaussianButton)&&
       (smoothParam2Edit->text().toInt()==0)&&(smoothParam4Edit->text().toDouble()==0.00))
    {
        smoothParam2Edit->setText(QString::number(DEFAULT_SMOOTH_PARAM_2));
        smoothParam4Edit->setText(QString::number(DEFAULT_SMOOTH_PARAM_4));
        QMessageBox::warning(this->parentWidget(),"ERROR:","Parameters 2 and 4 cannot BOTH be zero when the smoothing type is GAUSSIAN.\n\nAutomatically set to default values.");
    }
    // Ensure neither smoothing parameters 1 or 2 are ZERO (except in the GAUSSIAN case)
    if((smoothTypeGroup->checkedButton()!=(QAbstractButton*)smoothGaussianButton)&&
       ((smoothParam1Edit->text().toInt()==0)||(smoothParam2Edit->text().toDouble()==0)))
    {
        smoothParam1Edit->setText(QString::number(DEFAULT_SMOOTH_PARAM_1));
        smoothParam2Edit->setText(QString::number(DEFAULT_SMOOTH_PARAM_2));
        QMessageBox::warning(this->parentWidget(),"ERROR:","Parameters 1 or 2 cannot be zero for the current smoothing type.\n\nAutomatically set to default values.");
    }
} // validateDialog()

void ImageProcessingSettingsDialog::resetSmoothDialogToDefaults()
{
    if(DEFAULT_SMOOTH_TYPE==0)
        smoothBlurButton->setChecked(true);
    else if(DEFAULT_SMOOTH_TYPE==1)
        smoothGaussianButton->setChecked(true);
    else if(DEFAULT_SMOOTH_TYPE==2)
        smoothMedianButton->setChecked(true);
    smoothParam1Edit->setText(QString::number(DEFAULT_SMOOTH_PARAM_1));
    smoothParam2Edit->setText(QString::number(DEFAULT_SMOOTH_PARAM_2));
    smoothParam3Edit->setText(QString::number(DEFAULT_SMOOTH_PARAM_3));
    smoothParam4Edit->setText(QString::number(DEFAULT_SMOOTH_PARAM_4));
    // Enable/disable appropriate Smooth parameter inputs
    smoothTypeChange(smoothTypeGroup->checkedButton());
} // resetSmoothDialogToDefaults()

void ImageProcessingSettingsDialog::resetDilateDialogToDefaults()
{
    dilateIterationsEdit->setText(QString::number(DEFAULT_DILATE_ITERATIONS));
} // resetDilateDialogToDefaults()

void ImageProcessingSettingsDialog::resetErodeDialogToDefaults()
{
    erodeIterationsEdit->setText(QString::number(DEFAULT_ERODE_ITERATIONS));
} // resetErodeDialogToDefaults()

void ImageProcessingSettingsDialog::resetFlipDialogToDefaults()
{
    if(DEFAULT_FLIP_CODE==0)
        flipXAxisButton->setChecked(true);
    else if(DEFAULT_FLIP_CODE==1)
        flipYAxisButton->setChecked(true);
    else if(DEFAULT_FLIP_CODE==-1)
        flipBothAxesButton->setChecked(true);
} // resetFlipDialogToDefaults()

void ImageProcessingSettingsDialog::resetCannyDialogToDefaults()
{
    cannyThresh1Edit->setText(QString::number(DEFAULT_CANNY_THRESHOLD_1));
    cannyThresh2Edit->setText(QString::number(DEFAULT_CANNY_THRESHOLD_2));
    cannyApertureSizeEdit->setText(QString::number(DEFAULT_CANNY_APERTURE_SIZE));
    cannyL2NormCheckBox->setChecked(DEFAULT_CANNY_L2GRADIENT);
} // resetCannyDialogToDefaults()
