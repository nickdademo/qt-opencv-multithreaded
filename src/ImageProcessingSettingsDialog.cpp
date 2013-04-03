/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* ImageProcessingSettingsDialog.cpp                                    */
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

#include "ImageProcessingSettingsDialog.h"
#include "ui_ImageProcessingSettingsDialog.h"

ImageProcessingSettingsDialog::ImageProcessingSettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImageProcessingSettingsDialog)
{
    // Setup dialog
    ui->setupUi(this);
    // Connect GUI signals and slots
    connect(ui->resetAllToDefaultsButton,SIGNAL(released()),SLOT(resetAllDialogToDefaults()));
    connect(ui->resetSmoothToDefaultsButton,SIGNAL(released()),SLOT(resetSmoothDialogToDefaults()));
    connect(ui->resetDilateToDefaultsButton,SIGNAL(released()),SLOT(resetDilateDialogToDefaults()));
    connect(ui->resetErodeToDefaultsButton,SIGNAL(released()),SLOT(resetErodeDialogToDefaults()));
    connect(ui->resetFlipToDefaultsButton,SIGNAL(released()),SLOT(resetFlipDialogToDefaults()));
    connect(ui->resetCannyToDefaultsButton,SIGNAL(released()),SLOT(resetCannyDialogToDefaults()));
    connect(ui->applyButton,SIGNAL(released()),SLOT(updateStoredSettingsFromDialog()));
    connect(ui->smoothTypeGroup,SIGNAL(buttonReleased(QAbstractButton*)),SLOT(smoothTypeChange(QAbstractButton*)));
    // dilateIterationsEdit input string validation
    QRegExp rx5("[1-9]\\d{0,1}"); // Integers 1 to 99
    QRegExpValidator *validator5 = new QRegExpValidator(rx5, 0);
    ui->dilateIterationsEdit->setValidator(validator5);
    // erodeIterationsEdit input string validation
    QRegExp rx6("[1-9]\\d{0,1}"); // Integers 1 to 99
    QRegExpValidator *validator6 = new QRegExpValidator(rx6, 0);
    ui->erodeIterationsEdit->setValidator(validator6);
    // cannyThresh1Edit input string validation
    QRegExp rx7("^[0-9]{1,3}$"); // Integers 0 to 999
    QRegExpValidator *validator7 = new QRegExpValidator(rx7, 0);
    ui->cannyThresh1Edit->setValidator(validator7);
    // cannyThresh2Edit input string validation
    QRegExp rx8("^[0-9]{1,3}$"); // Integers 0 to 999
    QRegExpValidator *validator8 = new QRegExpValidator(rx8, 0);
    ui->cannyThresh2Edit->setValidator(validator8);
    // cannyApertureSizeEdit input string validation
    QRegExp rx9("[3,5,7]"); // Integers 3,5,7
    QRegExpValidator *validator9 = new QRegExpValidator(rx9, 0);
    ui->cannyApertureSizeEdit->setValidator(validator9);
    // Set dialog values to defaults
    resetAllDialogToDefaults();
    // Update image processing settings in imageProcessingSettings structure and processingThread
    updateStoredSettingsFromDialog();
}

ImageProcessingSettingsDialog::~ImageProcessingSettingsDialog()
{
    delete ui;
}

void ImageProcessingSettingsDialog::updateStoredSettingsFromDialog()
{
    // Validate values in dialog before storing
    validateDialog();
    // Smooth
    if(ui->smoothTypeGroup->checkedButton()==(QAbstractButton*)ui->smoothBlurButton)
        imageProcessingSettings.smoothType=0;
    else if(ui->smoothTypeGroup->checkedButton()==(QAbstractButton*)ui->smoothGaussianButton)
        imageProcessingSettings.smoothType=1;
    else if(ui->smoothTypeGroup->checkedButton()==(QAbstractButton*)ui->smoothMedianButton)
        imageProcessingSettings.smoothType=2;
    imageProcessingSettings.smoothParam1=ui->smoothParam1Edit->text().toInt();
    imageProcessingSettings.smoothParam2=ui->smoothParam2Edit->text().toInt();
    imageProcessingSettings.smoothParam3=ui->smoothParam3Edit->text().toDouble();
    imageProcessingSettings.smoothParam4=ui->smoothParam4Edit->text().toDouble();
    // Dilate
    imageProcessingSettings.dilateNumberOfIterations=ui->dilateIterationsEdit->text().toInt();
    // Erode
    imageProcessingSettings.erodeNumberOfIterations=ui->erodeIterationsEdit->text().toInt();
    // Flip
    if(ui->flipCodeGroup->checkedButton()==(QAbstractButton*)ui->flipXAxisButton)
        imageProcessingSettings.flipCode=0;
    else if(ui->flipCodeGroup->checkedButton()==(QAbstractButton*)ui->flipYAxisButton)
        imageProcessingSettings.flipCode=1;
    else if(ui->flipCodeGroup->checkedButton()==(QAbstractButton*)ui->flipBothAxesButton)
        imageProcessingSettings.flipCode=-1;
    // Canny
    imageProcessingSettings.cannyThreshold1=ui->cannyThresh1Edit->text().toDouble();
    imageProcessingSettings.cannyThreshold2=ui->cannyThresh2Edit->text().toDouble();
    imageProcessingSettings.cannyApertureSize=ui->cannyApertureSizeEdit->text().toInt();
    imageProcessingSettings.cannyL2gradient=ui->cannyL2NormCheckBox->isChecked();
    // Update image processing flags in processingThread
    emit newImageProcessingSettings(imageProcessingSettings);
}

void ImageProcessingSettingsDialog::updateDialogSettingsFromStored()
{
    // Smooth
    if(imageProcessingSettings.smoothType==0)
        ui->smoothBlurButton->setChecked(true);
    else if(imageProcessingSettings.smoothType==1)
        ui->smoothGaussianButton->setChecked(true);
    else if(imageProcessingSettings.smoothType==2)
        ui->smoothMedianButton->setChecked(true);
    ui->smoothParam1Edit->setText(QString::number(imageProcessingSettings.smoothParam1));
    ui->smoothParam2Edit->setText(QString::number(imageProcessingSettings.smoothParam2));
    ui->smoothParam3Edit->setText(QString::number(imageProcessingSettings.smoothParam3));
    ui->smoothParam4Edit->setText(QString::number(imageProcessingSettings.smoothParam4));
    // Dilate
    ui->dilateIterationsEdit->setText(QString::number(imageProcessingSettings.dilateNumberOfIterations));
    // Erode
    ui->erodeIterationsEdit->setText(QString::number(imageProcessingSettings.erodeNumberOfIterations));
    // Flip
    if(imageProcessingSettings.flipCode==0)
        ui->flipXAxisButton->setChecked(true);
    else if(imageProcessingSettings.flipCode==1)
        ui->flipYAxisButton->setChecked(true);
    else if(imageProcessingSettings.flipCode==-1)
        ui->flipBothAxesButton->setChecked(true);
    // Canny
    ui->cannyThresh1Edit->setText(QString::number(imageProcessingSettings.cannyThreshold1));
    ui->cannyThresh2Edit->setText(QString::number(imageProcessingSettings.cannyThreshold2));
    ui->cannyApertureSizeEdit->setText(QString::number(imageProcessingSettings.cannyApertureSize));
    ui->cannyL2NormCheckBox->setChecked(imageProcessingSettings.cannyL2gradient);
    // Enable/disable appropriate Smooth parameter inputs
    smoothTypeChange(ui->smoothTypeGroup->checkedButton());
}

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
}

void ImageProcessingSettingsDialog::smoothTypeChange(QAbstractButton *input)
{
    if(input==(QAbstractButton*)ui->smoothBlurButton)
    {
        // smoothParam1Edit input string validation
        QRegExp rx1("[1-9]\\d{0,1}"); // Integers 1 to 99
        QRegExpValidator *validator1 = new QRegExpValidator(rx1, 0);
        ui->smoothParam1Edit->setValidator(validator1);
        // smoothParam2Edit input string validation
        QRegExp rx2("[1-9]\\d{0,1}"); // Integers 1 to 99
        QRegExpValidator *validator2 = new QRegExpValidator(rx2, 0);
        ui->smoothParam2Edit->setValidator(validator2);
        // Enable/disable appropriate parameter inputs
        ui->smoothParam1Edit->setEnabled(true);
        ui->smoothParam2Edit->setEnabled(true);
        ui->smoothParam3Edit->setEnabled(false);
        ui->smoothParam4Edit->setEnabled(false);
        // Set parameter range labels
        ui->smoothParam1RangeLabel->setText("[1-99]");
        ui->smoothParam2RangeLabel->setText("[1-99]");
        ui->smoothParam3RangeLabel->setText("");
        ui->smoothParam4RangeLabel->setText("");
        // Set parameter labels
        ui->smoothParam1Label->setText("Kernel Width");
        ui->smoothParam2Label->setText("Kernel Height");
        ui->smoothParam3Label->setText("");
        ui->smoothParam4Label->setText("");
    }
    else if(input==(QAbstractButton*)ui->smoothGaussianButton)
    {
        // smoothParam1Edit input string validation
        QRegExp rx1("^[0-9]{1,2}$"); // Integers 0 to 99
        QRegExpValidator *validator1 = new QRegExpValidator(rx1, 0);
        ui->smoothParam1Edit->setValidator(validator1);
        // smoothParam2Edit input string validation
        QRegExp rx2("^[0-9]{1,2}$"); // Integers 0 to 99
        QRegExpValidator *validator2 = new QRegExpValidator(rx2, 0);
        ui->smoothParam2Edit->setValidator(validator2);
        // smoothParam3Edit input string validation
        QDoubleValidator *validator3 = new QDoubleValidator(0.0, 99.99, 2, this);
        validator3->setNotation(QDoubleValidator::StandardNotation);
        ui->smoothParam3Edit->setValidator(validator3);
        // Enable/disable appropriate parameter inputs
        ui->smoothParam1Edit->setEnabled(true);
        ui->smoothParam2Edit->setEnabled(true);
        ui->smoothParam3Edit->setEnabled(true);
        ui->smoothParam4Edit->setEnabled(true);
        // Set parameter range labels
        ui->smoothParam1RangeLabel->setText("[0-99]");
        ui->smoothParam2RangeLabel->setText("[0-99]");
        ui->smoothParam3RangeLabel->setText("[0.00-99.99]");
        ui->smoothParam4RangeLabel->setText("[0.00-99.99]");
        // Set parameter labels
        ui->smoothParam1Label->setText("Kernel Width");
        ui->smoothParam2Label->setText("Kernel Height");
        ui->smoothParam3Label->setText("Sigma X");
        ui->smoothParam4Label->setText("Sigma Y");
    }
    else if(input==(QAbstractButton*)ui->smoothMedianButton)
    {
        // smoothParam1Edit input string validation
        QRegExp rx1("[1-9]\\d{0,1}"); // Integers 1 to 99
        QRegExpValidator *validator1 = new QRegExpValidator(rx1, 0);
        ui->smoothParam1Edit->setValidator(validator1);
        // Enable/disable appropriate parameter inputs
        ui->smoothParam1Edit->setEnabled(true);
        ui->smoothParam2Edit->setEnabled(false);
        ui->smoothParam3Edit->setEnabled(false);
        ui->smoothParam4Edit->setEnabled(false);
        // Set parameter range labels
        ui->smoothParam1RangeLabel->setText("[1-99]");
        ui->smoothParam2RangeLabel->setText("");
        ui->smoothParam3RangeLabel->setText("");
        ui->smoothParam4RangeLabel->setText("");
        // Set parameter labels
        ui->smoothParam1Label->setText("Kernel (Square)");
        ui->smoothParam2Label->setText("");
        ui->smoothParam3Label->setText("");
        ui->smoothParam4Label->setText("");
    }
}

void ImageProcessingSettingsDialog::validateDialog()
{
    // Local variables
    bool inputEmpty=false;

    // If value of Smooth parameter 1 is EVEN (and not zero), convert to ODD by adding 1
    if(((ui->smoothParam1Edit->text().toInt()%2)==0)&&(ui->smoothParam1Edit->text().toInt()!=0))
    {
        ui->smoothParam1Edit->setText(QString::number(ui->smoothParam1Edit->text().toInt()+1));
        QMessageBox::information(this->parentWidget(),"NOTE:","Smooth parameter 1 must be an ODD number.\n\nAutomatically set to (inputted value+1).");
    }
    // If value of Smooth parameter 2 is EVEN (and not zero), convert to ODD by adding 1
    if(((ui->smoothParam2Edit->text().toInt()%2)==0)&&(ui->smoothParam2Edit->text().toInt()!=0))
    {
        ui->smoothParam2Edit->setText(QString::number(ui->smoothParam2Edit->text().toInt()+1));
        QMessageBox::information(this->parentWidget(),"NOTE:","Smooth parameter 2 must be an ODD number (or zero).\n\nAutomatically set to (inputted value+1).");
    }

    // Check for empty inputs: if empty, set to default values
    if(ui->smoothParam1Edit->text().isEmpty())
    {
        ui->smoothParam1Edit->setText(QString::number(DEFAULT_SMOOTH_PARAM_1));
        inputEmpty=true;
    }
    if(ui->smoothParam2Edit->text().isEmpty())
    {
        ui->smoothParam2Edit->setText(QString::number(DEFAULT_SMOOTH_PARAM_2));
        inputEmpty=true;
    }
    if(ui->smoothParam3Edit->text().isEmpty())
    {
        ui->smoothParam3Edit->setText(QString::number(DEFAULT_SMOOTH_PARAM_3));
        inputEmpty=true;
    }
    if(ui->smoothParam4Edit->text().isEmpty())
    {
        ui->smoothParam4Edit->setText(QString::number(DEFAULT_SMOOTH_PARAM_4));
        inputEmpty=true;
    }
    if(ui->dilateIterationsEdit->text().isEmpty())
    {
        ui->dilateIterationsEdit->setText(QString::number(DEFAULT_DILATE_ITERATIONS));
        inputEmpty=true;
    }
    if(ui->erodeIterationsEdit->text().isEmpty())
    {
        ui->erodeIterationsEdit->setText(QString::number(DEFAULT_ERODE_ITERATIONS));
        inputEmpty=true;
    }
    if(ui->cannyThresh1Edit->text().isEmpty())
    {
        ui->cannyThresh1Edit->setText(QString::number(DEFAULT_CANNY_THRESHOLD_1));
        inputEmpty=true;
    }
    if(ui->cannyThresh2Edit->text().isEmpty())
    {
        ui->cannyThresh2Edit->setText(QString::number(DEFAULT_CANNY_THRESHOLD_2));
        inputEmpty=true;
    }
    if(ui->cannyApertureSizeEdit->text().isEmpty())
    {
        ui->cannyApertureSizeEdit->setText(QString::number(DEFAULT_CANNY_APERTURE_SIZE));
        inputEmpty=true;
    }
    // Check if any of the inputs were empty
    if(inputEmpty)
        QMessageBox::warning(this->parentWidget(),"WARNING:","One or more inputs empty.\n\nAutomatically set to default values.");

    // Check for special parameter cases when smoothing type is GAUSSIAN
    if((ui->smoothTypeGroup->checkedButton()==(QAbstractButton*)ui->smoothGaussianButton)&&
       (ui->smoothParam1Edit->text().toInt()==0)&&(ui->smoothParam3Edit->text().toDouble()==0.00))
    {
        ui->smoothParam1Edit->setText(QString::number(DEFAULT_SMOOTH_PARAM_1));
        ui->smoothParam3Edit->setText(QString::number(DEFAULT_SMOOTH_PARAM_3));
        QMessageBox::warning(this->parentWidget(),"ERROR:","Parameters 1 and 3 cannot BOTH be zero when the smoothing type is GAUSSIAN.\n\nAutomatically set to default values.");
    }
    if((ui->smoothTypeGroup->checkedButton()==(QAbstractButton*)ui->smoothGaussianButton)&&
       (ui->smoothParam2Edit->text().toInt()==0)&&(ui->smoothParam4Edit->text().toDouble()==0.00))
    {
        ui->smoothParam2Edit->setText(QString::number(DEFAULT_SMOOTH_PARAM_2));
        ui->smoothParam4Edit->setText(QString::number(DEFAULT_SMOOTH_PARAM_4));
        QMessageBox::warning(this->parentWidget(),"ERROR:","Parameters 2 and 4 cannot BOTH be zero when the smoothing type is GAUSSIAN.\n\nAutomatically set to default values.");
    }
    // Ensure neither smoothing parameters 1 or 2 are ZERO (except in the GAUSSIAN case)
    if((ui->smoothTypeGroup->checkedButton()!=(QAbstractButton*)ui->smoothGaussianButton)&&
       ((ui->smoothParam1Edit->text().toInt()==0)||(ui->smoothParam2Edit->text().toDouble()==0)))
    {
        ui->smoothParam1Edit->setText(QString::number(DEFAULT_SMOOTH_PARAM_1));
        ui->smoothParam2Edit->setText(QString::number(DEFAULT_SMOOTH_PARAM_2));
        QMessageBox::warning(this->parentWidget(),"ERROR:","Parameters 1 or 2 cannot be zero for the current smoothing type.\n\nAutomatically set to default values.");
    }
}

void ImageProcessingSettingsDialog::resetSmoothDialogToDefaults()
{
    if(DEFAULT_SMOOTH_TYPE==0)
        ui->smoothBlurButton->setChecked(true);
    else if(DEFAULT_SMOOTH_TYPE==1)
        ui->smoothGaussianButton->setChecked(true);
    else if(DEFAULT_SMOOTH_TYPE==2)
        ui->smoothMedianButton->setChecked(true);
    ui->smoothParam1Edit->setText(QString::number(DEFAULT_SMOOTH_PARAM_1));
    ui->smoothParam2Edit->setText(QString::number(DEFAULT_SMOOTH_PARAM_2));
    ui->smoothParam3Edit->setText(QString::number(DEFAULT_SMOOTH_PARAM_3));
    ui->smoothParam4Edit->setText(QString::number(DEFAULT_SMOOTH_PARAM_4));
    // Enable/disable appropriate Smooth parameter inputs
    smoothTypeChange(ui->smoothTypeGroup->checkedButton());
}

void ImageProcessingSettingsDialog::resetDilateDialogToDefaults()
{
    ui->dilateIterationsEdit->setText(QString::number(DEFAULT_DILATE_ITERATIONS));
}

void ImageProcessingSettingsDialog::resetErodeDialogToDefaults()
{
    ui->erodeIterationsEdit->setText(QString::number(DEFAULT_ERODE_ITERATIONS));
}

void ImageProcessingSettingsDialog::resetFlipDialogToDefaults()
{
    if(DEFAULT_FLIP_CODE==0)
        ui->flipXAxisButton->setChecked(true);
    else if(DEFAULT_FLIP_CODE==1)
        ui->flipYAxisButton->setChecked(true);
    else if(DEFAULT_FLIP_CODE==-1)
        ui->flipBothAxesButton->setChecked(true);
}

void ImageProcessingSettingsDialog::resetCannyDialogToDefaults()
{
    ui->cannyThresh1Edit->setText(QString::number(DEFAULT_CANNY_THRESHOLD_1));
    ui->cannyThresh2Edit->setText(QString::number(DEFAULT_CANNY_THRESHOLD_2));
    ui->cannyApertureSizeEdit->setText(QString::number(DEFAULT_CANNY_APERTURE_SIZE));
    ui->cannyL2NormCheckBox->setChecked(DEFAULT_CANNY_L2GRADIENT);
}
