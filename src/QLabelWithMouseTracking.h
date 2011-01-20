/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* QLabelWithMouseTracking.h                                            */
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

#ifndef QLABELWITHMOUSETRACKING_H
#define QLABELWITHMOUSETRACKING_H

// Qt header files
#include <QtGui>

class QLabelWithMouseTracking : public QLabel
{
    Q_OBJECT

public:
    QLabelWithMouseTracking(QWidget *parent = 0);
    void setMouseXPos(int);
    void setMouseYPos(int);
    int getMouseXPos();
    int getMouseYPos();
    void setLeftMouseButtonPress(bool);
    bool getLeftMouseButtonPress();
    void setLeftMouseButtonRelease(bool);
    bool getLeftMouseButtonRelease();
    void setRightMouseButtonPress(bool);
    bool getRightMouseButtonPress();
    void setRightMouseButtonRelease(bool);
    bool getRightMouseButtonRelease();
private:
    int mouseXPos, mouseYPos;
    bool mouseLeftPressed, mouseRightPressed, mouseLeftReleased, mouseRightReleased;
protected:
    void mouseMoveEvent(QMouseEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
};

#endif // QLABELWITHMOUSETRACKING_H
