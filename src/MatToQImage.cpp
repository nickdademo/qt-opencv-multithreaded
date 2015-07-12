/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* MatToQImage.cpp                                                      */
/*                                                                      */
/* Nick D'Ademo <nickdademo@gmail.com>                                  */
/*                                                                      */
/* Copyright (c) 2012-2015 Nick D'Ademo                                 */
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

#include "MatToQImage.h"

#include <QDebug>

QImage MatToQImage(const cv::Mat& mat)
{
    if(mat.type() == CV_8UC1)
    {
        // Make all channels (RGB) identical via a LUT
        QVector<QRgb> colorTable;
        for (int i = 0; i < 256; i++)
        {
            colorTable.push_back(qRgb(i, i, i));
        }

        // Create QImage from cv::Mat
        const unsigned char *qImageBuffer = (const unsigned char*)mat.data;
        QImage img(qImageBuffer, mat.cols, mat.rows, (int)mat.step, QImage::Format_Indexed8);
        img.setColorTable(colorTable);
        return img;
    }
    else if(mat.type() == CV_8UC3)
    {
        // Create QImage from cv::Mat
        const unsigned char *qImageBuffer = (const unsigned char*)mat.data;
        return QImage(qImageBuffer, mat.cols, mat.rows, (int)mat.step, QImage::Format_RGB888).rgbSwapped();
    }
    else
    {
        qCritical() << QString("Cannot convert cv::Mat to QImage. Unrecognized input image type: %1").arg(mat.type());
        return QImage();
    }
}
