/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* ImageProcessingSettingsDialog.h                                      */
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

#ifndef IMAGEPROCESSINGSETTINGSDIALOG_H
#define IMAGEPROCESSINGSETTINGSDIALOG_H

// Qt
#include <QtWidgets>
// Local
#include "Structures.h"
#include "Config.h"

namespace Ui {
class ImageProcessingSettingsDialog;
}

class ImageProcessingSettingsDialog : public QDialog
{
    Q_OBJECT

    public:
        explicit ImageProcessingSettingsDialog(QWidget *parent = 0);
        ~ImageProcessingSettingsDialog();
        void updateDialogSettingsFromStored();

    private:
        Ui::ImageProcessingSettingsDialog *ui;
        ImageProcessingSettings imageProcessingSettings;

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
        void smoothTypeChange(QAbstractButton *);

    signals:
        void newImageProcessingSettings(struct ImageProcessingSettings p_settings);
};

#endif // IMAGEPROCESSINGSETTINGSDIALOG_H
