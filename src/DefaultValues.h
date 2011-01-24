/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* DefaultValues.h                                                      */
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

#ifndef DEFAULTVALUES_H
#define DEFAULTVALUES_H

// OpenCV header files
#include <opencv/cv.h>
#include <opencv/highgui.h>

// Image buffer size
#define DEFAULT_IMAGE_BUFFER_SIZE 1
// SMOOTH
#define DEFAULT_SMOOTH_TYPE CV_GAUSSIAN // Options: [CV_BLUR_NO_SCALE,CV_BLUR,CV_GAUSSIAN,CV_MEDIAN]
#define DEFAULT_SMOOTH_PARAM_1 3
#define DEFAULT_SMOOTH_PARAM_2 0
#define DEFAULT_SMOOTH_PARAM_3 0
#define DEFAULT_SMOOTH_PARAM_4 0
// DILATE
#define DEFAULT_DILATE_ITERATIONS 1
// ERODE
#define DEFAULT_ERODE_ITERATIONS 1
// FLIP
#define DEFAULT_FLIP_MODE 0 // Options: [x-axis=0,y-axis=1,both axes=-1]
// CANNY
#define DEFAULT_CANNY_THRESHOLD_1 10
#define DEFAULT_CANNY_THRESHOLD_2 100
#define DEFAULT_CANNY_APERTURE_SIZE 3

#endif // DEFAULTVALUES_H
