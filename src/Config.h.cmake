/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* Config.h                                                             */
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

#ifndef CONFIG_H
#define CONFIG_H

// An anonymous namespace restricts these variables to the scope of the compilation unit.
namespace {
    const char* APP_NAME                    = "@PROJECT_NAME@";
    const char* APP_VERSION                 = "@PROJECT_VERSION@";
    const char* APP_AUTHOR_NAME             = "@PROJECT_AUTHOR_NAME@";
    const char* APP_AUTHOR_EMAIL            = "@PROJECT_AUTHOR_EMAIL@";
    const char* APP_AUTHOR_WEBSITE          = "@PROJECT_AUTHOR_WEBSITE@";
}

// Initial Main Window size
#define INITIAL_MAINWINDOW_SIZE_W           640
#define INITIAL_MAINWINDOW_SIZE_H           480

// Capture statistics settings
#define CAPTURE__FPS_STAT_QUEUE_LENGTH      32
#define CAPTURE__EMIT_DATA_N_SAMPLES        1

// Processing statistics settings
#define PROCESSING__FPS_STAT_QUEUE_LENGTH   32
#define PROCESSING__EMIT_DATA_N_SAMPLES     1

// Image buffer size
#define DEFAULT_IMAGE_BUFFER_SIZE           1
// Drop frame if buffer is full
#define DEFAULT_DROP_FRAMES                 false
// Thread priorities
#define DEFAULT_CAP_THREAD_PRIO             QThread::NormalPriority
#define DEFAULT_PROC_THREAD_PRIO            QThread::HighPriority
// Enable Frame Processing
#define DEFAULT_ENABLE_FRAME_PROCESSING     true

// Image Processing
// Smooth
#define DEFAULT_SMOOTH_TYPE                 0 // Options: [Blur=0, Gaussian=1, Median=2]
#define DEFAULT_SMOOTH_PARAM_1              3
#define DEFAULT_SMOOTH_PARAM_2              3
#define DEFAULT_SMOOTH_PARAM_3              0
#define DEFAULT_SMOOTH_PARAM_4              0
// Dilate
#define DEFAULT_DILATE_ITERATIONS           1
// Erode
#define DEFAULT_ERODE_ITERATIONS            1
// Flip
#define DEFAULT_FLIP_CODE                   0 // Options: [X-axis=0, Y-axis=1, Both axes=-1]
// Canny
#define DEFAULT_CANNY_THRESHOLD_1           10
#define DEFAULT_CANNY_THRESHOLD_2           00
#define DEFAULT_CANNY_APERTURE_SIZE         3
#define DEFAULT_CANNY_L2GRADIENT            false

#endif // CONFIG_H
