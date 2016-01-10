/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* ImageProcessing.h                                                    */
/*                                                                      */
/* Nick D'Ademo <nickdademo@gmail.com>                                  */
/*                                                                      */
/* Copyright (c) 2012-2016 Nick D'Ademo                                 */
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

#ifndef IMAGEPROCESSING_H
#define IMAGEPROCESSING_H

#include "Config.h"

class ImageProcessing
{
    public:
        struct Grayscale
        {
            bool enabled;
        } grayscale;
        struct Smooth
        {
            bool enabled;
            int type;
            int parameter1;
            int parameter2;
            double parameter3;
            double parameter4;
        } smooth;
        struct Dilate
        {
            bool enabled;
            int nIterations;
        } dilate;
        struct Erode
        {
            bool enabled;
            int nIterations;
        } erode;
        struct Flip
        {
            bool enabled;
            int code;
        } flip;
        struct Canny
        {
            bool enabled;
            double threshold1;
            double threshold2;
            int apertureSize;
            bool l2gradient;
        } canny;

        void init()
        {
            // Grayscale
            grayscale.enabled = false;
            // Smooth
            smooth.enabled = false;
            smooth.type = DEFAULT_SMOOTH_TYPE;
            smooth.parameter1 = DEFAULT_SMOOTH_PARAM_1;
            smooth.parameter2 = DEFAULT_SMOOTH_PARAM_2;
            smooth.parameter3 = DEFAULT_SMOOTH_PARAM_3;
            smooth.parameter4 = DEFAULT_SMOOTH_PARAM_4;
            // Dilate
            dilate.enabled = false;
            dilate.nIterations = DEFAULT_DILATE_ITERATIONS;
            // Erode
            erode.enabled = false;
            erode.nIterations = DEFAULT_ERODE_ITERATIONS;
            // Flip
            flip.enabled = false;
            flip.code = DEFAULT_FLIP_CODE;
            // Canny
            canny.enabled = false;
            canny.threshold1 = DEFAULT_CANNY_THRESHOLD_1;
            canny.threshold2 = DEFAULT_CANNY_THRESHOLD_2;
            canny.apertureSize = DEFAULT_CANNY_APERTURE_SIZE;
            canny.l2gradient = DEFAULT_CANNY_L2GRADIENT;
        }
};

#endif // IMAGEPROCESSING_H