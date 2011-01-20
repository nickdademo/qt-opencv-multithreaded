/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* QLabelWithMouseTracking.cpp                                          */
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

#include "QLabelWithMouseTracking.h"

QLabelWithMouseTracking::QLabelWithMouseTracking(QWidget *parent) : QLabel(parent)
{
    // Initialize variables
    mouseXPos=0;
    mouseYPos=0;
    mouseLeftPressed=false;
    mouseRightPressed=false;
    mouseLeftReleased=true;
    mouseRightReleased=true;
} // QLabelWithMouseTracking constructor

void QLabelWithMouseTracking::mouseMoveEvent(QMouseEvent *ev)
{
    setMouseXPos(ev->pos().x());
    setMouseYPos(ev->pos().y());
} // mouseMoveEvent()

void QLabelWithMouseTracking::setMouseXPos(int x_in)
{
    mouseXPos=x_in;
} // setMouseXPos()

void QLabelWithMouseTracking::setMouseYPos(int y_in)
{
    mouseYPos=y_in;
} // setMouseYPos()

int QLabelWithMouseTracking::getMouseXPos()
{
    return mouseXPos;
} // getMouseXPos()

int QLabelWithMouseTracking::getMouseYPos()
{
    return mouseYPos;
} // getMouseYPos()

void QLabelWithMouseTracking::mouseReleaseEvent(QMouseEvent *ev)
{
    // Update mouse position on release event
    setMouseXPos(ev->pos().x());
    setMouseYPos(ev->pos().y());
    if(ev->button()==Qt::LeftButton)
        setLeftMouseButtonRelease(true);
    else if(ev->button()==Qt::RightButton)
        setRightMouseButtonRelease(true);
} // mouseReleaseEvent()

void QLabelWithMouseTracking::setLeftMouseButtonRelease(bool in)
{
    mouseLeftReleased=in;
} // setLeftMouseButtonRelease()

bool QLabelWithMouseTracking::getLeftMouseButtonRelease()
{
    return mouseLeftReleased;
} // getLeftMouseButtonRelease()

void QLabelWithMouseTracking::mousePressEvent(QMouseEvent *ev)
{
    // Update mouse position on press event
    setMouseXPos(ev->pos().x());
    setMouseYPos(ev->pos().y());
    if(ev->button()==Qt::LeftButton)
    {
        setLeftMouseButtonPress(true);
        setLeftMouseButtonRelease(false);
    }
    else if(ev->button()==Qt::RightButton)
    {
        setRightMouseButtonPress(true);
        setRightMouseButtonRelease(false);
    }
} // mousePressEvent()

void QLabelWithMouseTracking::setLeftMouseButtonPress(bool in)
{
    mouseLeftPressed=in;
} // setLeftMouseButtonPress()

bool QLabelWithMouseTracking::getLeftMouseButtonPress()
{
    return mouseLeftPressed;
} // getLeftMouseButtonPress()

void QLabelWithMouseTracking::setRightMouseButtonPress(bool in)
{
    mouseRightPressed=in;
} // setRightMouseButtonPress()

bool QLabelWithMouseTracking::getRightMouseButtonPress()
{
    return mouseRightPressed;
} // getRightMouseButtonPress()

void QLabelWithMouseTracking::setRightMouseButtonRelease(bool in)
{
    mouseRightReleased=in;
} // setRightMouseButtonRelease()

bool QLabelWithMouseTracking::getRightMouseButtonRelease()
{
    return mouseRightReleased;
} // getRightMouseButtonRelease()
