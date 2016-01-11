/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* FrameLabel.cpp                                                       */
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

#include "FrameLabel.h"

#include <QPainter>
#include <QMouseEvent>
#include <QRect>
#include <QMenu>

FrameLabel::FrameLabel(QWidget *parent) : 
    QLabel(parent),
    m_drawBox(false)
{
    m_startPoint.setX(0);
    m_startPoint.setY(0);
    m_mouseCursorPos.setX(0);
    m_mouseCursorPos.setY(0);
    createContextMenu();
}

void FrameLabel::mouseMoveEvent(QMouseEvent *ev)
{
    // Save mouse cursor position
    setMouseCursorPos(ev->pos());
    // Update box width and height if box drawing is in progress
    if (m_drawBox)
    {
        m_box->setWidth(getMouseCursorPos().x() - m_startPoint.x());
        m_box->setHeight(getMouseCursorPos().y() - m_startPoint.y());
    }

    // Inform main window of mouse move event
    emit onMouseMoveEvent();
}

void FrameLabel::setMouseCursorPos(QPoint input)
{
    m_mouseCursorPos = input;
}

QPoint FrameLabel::getMouseCursorPos()
{
    return m_mouseCursorPos;
}

void FrameLabel::mouseReleaseEvent(QMouseEvent *ev)
{
    // Update cursor position
    setMouseCursorPos(ev->pos());
    // On left mouse button release
    if(ev->button() == Qt::LeftButton)
    {
        if (m_drawBox)
        {
            // Stop drawing box
            m_drawBox = false;
            // Save box dimensions
            QRect rect(m_box->left(), m_box->top(), m_box->width(), m_box->height());
            emit newSelection(rect);
        }
    }
    // On right mouse button release
    else if(ev->button() == Qt::RightButton)
    {
        // If user presses (and then releases) the right mouse button while drawing box, stop drawing box
        if (m_drawBox)
        {
            m_drawBox = false;
        }
        else
        {
            // Show context menu
            m_menu->exec(ev->globalPos());
        }
    }
}

void FrameLabel::mousePressEvent(QMouseEvent *ev)
{
    // Update cursor position
    setMouseCursorPos(ev->pos());;
    if(ev->button() == Qt::LeftButton)
    {
        // Start drawing box
        m_startPoint = ev->pos();
        m_box = new QRect(m_startPoint.x(), m_startPoint.y(), 0, 0);
        m_drawBox = true;
    }
}

void FrameLabel::paintEvent(QPaintEvent *ev)
{
    QLabel::paintEvent(ev);
    QPainter painter(this);

    // Draw box
    if (m_drawBox)
    {
        painter.setPen(Qt::blue);
        painter.drawRect(*m_box);
    }
}

void FrameLabel::createContextMenu()
{
    // Top level menu
    m_menu = new QMenu(this);

    // Add actions
    QAction *action;
    action = new QAction(this);
    action->setText(tr("Reset ROI"));
    action->setData((int)Actions::ResetRoi);
    m_menu->addAction(action);
    action = new QAction(this);
    action->setText(tr("Scale to Fit Frame"));
    action->setData((int)Actions::ScaleToFitFrame);
    action->setCheckable(true);
    m_menu->addAction(action);
    m_menu->addSeparator();

    // Create image processing menu
    QMenu* menuImageProcessing = new QMenu(this);
    menuImageProcessing->setTitle(tr("Image Processing"));
    m_menu->addMenu(menuImageProcessing);

    // Add actions
    action = new QAction(this);
    action->setText(tr("Grayscale"));
    action->setData((int)Actions::Grayscale);
    action->setCheckable(true);
    menuImageProcessing->addAction(action);
    action = new QAction(this);
    action->setText(tr("Smooth"));
    action->setData((int)Actions::Smooth);
    action->setCheckable(true);
    menuImageProcessing->addAction(action);
    action = new QAction(this);
    action->setText(tr("Dilate"));
    action->setData((int)Actions::Dilate);
    action->setCheckable(true);
    menuImageProcessing->addAction(action);
    action = new QAction(this);
    action->setText(tr("Erode"));
    action->setData((int)Actions::Erode);
    action->setCheckable(true);
    menuImageProcessing->addAction(action);
    action = new QAction(this);
    action->setText(tr("Flip"));
    action->setData((int)Actions::Flip);
    action->setCheckable(true);
    menuImageProcessing->addAction(action);
    action = new QAction(this);
    action->setText(tr("Canny"));
    action->setData((int)Actions::Canny);
    action->setCheckable(true);
    menuImageProcessing->addAction(action);
    menuImageProcessing->addSeparator();
    action = new QAction(this);
    action->setText(tr("Settings..."));
    action->setData((int)Actions::Settings);
    menuImageProcessing->addAction(action);
}
