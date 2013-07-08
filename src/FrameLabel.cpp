/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* FrameLabel.cpp                                                       */
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

#include "FrameLabel.h"
// Qt
#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>

FrameLabel::FrameLabel(QWidget *parent) : QLabel(parent)
{
    startPoint.setX(0);
    startPoint.setY(0);
    mouseCursorPos.setX(0);
    mouseCursorPos.setY(0);
    drawBox=false;
    mouseData.leftButtonRelease=false;
    mouseData.rightButtonRelease=false;
    createContextMenu();
}

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
}

void FrameLabel::setMouseCursorPos(QPoint input)
{
    mouseCursorPos=input;
}

QPoint FrameLabel::getMouseCursorPos()
{
    return mouseCursorPos;
}

void FrameLabel::mouseReleaseEvent(QMouseEvent *ev)
{
    // Update cursor position
    setMouseCursorPos(ev->pos());
    // On left mouse button release
    if(ev->button()==Qt::LeftButton)
    {
        // Set leftButtonRelease flag to TRUE
        mouseData.leftButtonRelease=true;
        if(drawBox)
        {
            // Stop drawing box
            drawBox=false;
            // Save box dimensions
            mouseData.selectionBox.setX(box->left());
            mouseData.selectionBox.setY(box->top());
            mouseData.selectionBox.setWidth(box->width());
            mouseData.selectionBox.setHeight(box->height());
            // Set leftButtonRelease flag to TRUE
            mouseData.leftButtonRelease=true;
            // Inform main window of event
            emit newMouseData(mouseData);
        }
        // Set leftButtonRelease flag to FALSE
        mouseData.leftButtonRelease=false;
    }
    // On right mouse button release
    else if(ev->button()==Qt::RightButton)
    {
        // If user presses (and then releases) the right mouse button while drawing box, stop drawing box
        if(drawBox)
            drawBox=false;
        else
        {
            // Show context menu
            menu->exec(ev->globalPos());
        }
    }
}

void FrameLabel::mousePressEvent(QMouseEvent *ev)
{
    // Update cursor position
    setMouseCursorPos(ev->pos());;
    if(ev->button()==Qt::LeftButton)
    {
        // Start drawing box
        startPoint=ev->pos();
        box=new QRect(startPoint.x(),startPoint.y(),0,0);
        drawBox=true;
    }
}

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
}

void FrameLabel::createContextMenu()
{
    // Create top-level menu object
    menu = new QMenu(this);
    // Add actions
    QAction *action;
    action = new QAction(this);
    action->setText(tr("Reset ROI"));
    menu->addAction(action);
    action = new QAction(this);
    action->setText(tr("Scale to Fit Frame"));
    action->setCheckable(true);
    menu->addAction(action);
    menu->addSeparator();
    // Create image processing menu object
    QMenu* menu_imgProc = new QMenu(this);
    menu_imgProc->setTitle("Image Processing");
    menu->addMenu(menu_imgProc);
    // Add actions
    action = new QAction(this);
    action->setText(tr("Grayscale"));
    action->setCheckable(true);
    menu_imgProc->addAction(action);
    action = new QAction(this);
    action->setText(tr("Smooth"));
    action->setCheckable(true);
    menu_imgProc->addAction(action);
    action = new QAction(this);
    action->setText(tr("Dilate"));
    action->setCheckable(true);
    menu_imgProc->addAction(action);
    action = new QAction(this);
    action->setText(tr("Erode"));
    action->setCheckable(true);
    menu_imgProc->addAction(action);
    action = new QAction(this);
    action->setText(tr("Flip"));
    action->setCheckable(true);
    menu_imgProc->addAction(action);
    action = new QAction(this);
    action->setText(tr("Canny"));
    action->setCheckable(true);
    menu_imgProc->addAction(action);
    menu_imgProc->addSeparator();
    action = new QAction(this);
    action->setText(tr("Settings..."));
    menu_imgProc->addAction(action);
}
