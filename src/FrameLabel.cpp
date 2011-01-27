/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* FrameLabel.cpp                                                       */
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

#include "FrameLabel.h"

FrameLabel::FrameLabel(QWidget *parent) : QLabel(parent)
{
    // Initialize variables
    startPoint.setX(0);
    startPoint.setY(0);
    mouseCursorPos.setX(0);
    mouseCursorPos.setY(0);
    drawBox=false;
    taskData.setROIOn=false;
    taskData.resetROIOn=false;
} // FrameLabel constructor

void FrameLabel::mouseMoveEvent(QMouseEvent *ev)
{
    // Save mouse cursor position
    setMouseCursorPos(ev->pos());
    // Update box width and height if box drawing is in progress
    if(drawBox)
    {
        box->setWidth(getMouseCursorPos().x()-startPoint.x());
        box->setHeight(getMouseCursorPos().y()-startPoint.y());
    }
    // Inform main window of mouse move event
    emit onMouseMoveEvent();
} // mouseMoveEvent()

void FrameLabel::setMouseCursorPos(QPoint input)
{
    mouseCursorPos=input;
} // setMouseCursorPos()

QPoint FrameLabel::getMouseCursorPos()
{
    return mouseCursorPos;
} // getMouseXPos()

void FrameLabel::mouseReleaseEvent(QMouseEvent *ev)
{
    // Update cursor position
    setMouseCursorPos(ev->pos());
    // On left mouse button release
    if(ev->button()==Qt::LeftButton)
    {
        // Set ROI
        if(drawBox)
        {
            // Stop drawing box
            drawBox=false;
            // Set setROIOn flag to TRUE
            taskData.setROIOn=true;
            // Save box dimensions
            taskData.selectionBox.setLeft(box->left());
            taskData.selectionBox.setTop(box->top());
            taskData.selectionBox.setWidth(box->width());
            taskData.selectionBox.setHeight(box->height());
            // Update taskData in processingThread
            emit newTaskData(taskData);
            // Reset setROIOn flag to FALSE
            taskData.setROIOn=false;
        }
    }
    // On right mouse button release
    else if(ev->button()==Qt::RightButton)
    {
        // If user presses (and then releases) the right mouse button while drawing box, stop drawing box
        if(drawBox)
            drawBox=false;
        // Reset ROI
        else
        {
            // Set resetROIOn flag to FALSE
            taskData.resetROIOn=true;
            // Update taskData in processingThread
            emit newTaskData(taskData);
            // Reset resetROIOn flag to FALSE
            taskData.resetROIOn=false;
        }
    }
} // mouseReleaseEvent()

void FrameLabel::mousePressEvent(QMouseEvent *ev)
{
    // Update cursor position
    setMouseCursorPos(ev->pos());;
    // Start drawing box
    if(ev->button()==Qt::LeftButton)
    {
        startPoint=ev->pos();
        box=new QRect(startPoint.x(),startPoint.y(),0,0);
        drawBox=true;
    }
} // mousePressEvent()

void FrameLabel::paintEvent(QPaintEvent *ev)
{
    QLabel::paintEvent(ev);
    QPainter painter(this);
    // Draw box
    if(drawBox)
    {
        painter.setPen(Qt::blue);
        painter.drawRect(*box);
    }
} // paintEvent()
