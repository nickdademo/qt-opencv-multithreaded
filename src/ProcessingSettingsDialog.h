/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* ProcessingSettingsDialog.h                                           */
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

#ifndef PROCESSINGSETTINGSDIALOG_H
#define PROCESSINGSETTINGSDIALOG_H

#include "ui_ProcessingSettingsDialog.h"
#include "Structures.h"

class ProcessingSettingsDialog : public QDialog, private Ui::ProcessingSettingsDialog
{
    Q_OBJECT

public:
    ProcessingSettingsDialog(QWidget *parent = 0);
    void updateDialogSettingsFromStored();
private:
    ProcessingSettings processingSettings;
public slots:
    void resetAllDialogToDefaults();
    void updateStoredSettingsFromDialog();
private slots:
    void resetSmoothDialogToDefaults();
    void resetDilateDialogToDefaults();
    void resetErodeDialogToDefaults();
    void resetFlipDialogToDefaults();
    void resetCannyDialogToDefaults();
    void validateDialog();
    void smoothTypeChange(QAbstractButton*);
signals:
    void newProcessingSettings(struct ProcessingSettings p_settings);
};

#endif // PROCESSINGSETTINGSDIALOG_H
