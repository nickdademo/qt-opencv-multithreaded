/************************************************************************/
/* qt-opencv-multithreaded:                                             */
/* A multithreaded OpenCV application using the Qt framework.           */
/*                                                                      */
/* CameraConnectDialog.cpp                                              */
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

#include "CameraConnectDialog.h"

#include "Config.h"

#include <QThread>
#include <QMessageBox>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QPushButton>

CameraConnectDialog::CameraConnectDialog(int nextDeviceNumber, QWidget *parent) :
    m_nextDeviceNumber(nextDeviceNumber),
    QDialog(parent)
{  
    // Dialog properties
    setWindowTitle(tr("Connect to Camera"));
    setStyleSheet("QGroupBox {font-weight: bold;};");

    // Layout
    QGridLayout *mainLayout = new QGridLayout;

    // Tab Name
    QLabel *tabNameLabel = new QLabel(tr("Tab Name") + ":");
    m_tabNameLineEdit = new QLineEdit;
    mainLayout->addWidget(tabNameLabel, 0, 0, 1, 2);
    mainLayout->addWidget(m_tabNameLineEdit, 0, 2, 1, 2);

    // Enable Frame Processing
    m_enableFrameProcessingCheckbox = new QCheckBox(tr("Enable frame processing"));
    mainLayout->addWidget(m_enableFrameProcessingCheckbox, 1, 0, 1, 4);

    // Stream Control
    QLabel *streamControlLabel = new QLabel(tr("Stream Control") + ":");
    m_streamControlComboBox = new QComboBox;
    m_streamControlComboBox->addItem(tr("Run"), (int)SharedImageBuffer::StreamControl::Run);
    m_streamControlComboBox->addItem(tr("Pause"), (int)SharedImageBuffer::StreamControl::Pause);
    mainLayout->addWidget(streamControlLabel, 2, 0);
    mainLayout->addWidget(m_streamControlComboBox, 2, 1, 1, 2);

    // Synchronize Stream
    m_syncStreamCheckbox = new QCheckBox(tr("Synchronize stream"));
    mainLayout->addWidget(m_syncStreamCheckbox, 2, 3);

    // Camera group box
    QGroupBox *cameraGroupBox = new QGroupBox(tr("Camera"));
    QGridLayout *cameraGridLayout = new QGridLayout;
    // Device Number
    QLabel *cameraDeviceNumberLabel = new QLabel(tr("Device Number") + ":");
    m_cameraDeviceNumberLineEdit = new QLineEdit;
    cameraGridLayout->addWidget(cameraDeviceNumberLabel, 0, 0);
    cameraGridLayout->addWidget(m_cameraDeviceNumberLineEdit, 0, 1);
    // Device Number Validator
    QRegExp rx1("^[0-9]{1,3}$"); // Integers 0 to 999
    QRegExpValidator *validator1 = new QRegExpValidator(rx1, 0);
    m_cameraDeviceNumberLineEdit->setValidator(validator1);
    // Resolution
    QLabel *cameraResolutionLabel = new QLabel(tr("Resolution (W x H)") + ":");
    m_cameraResolutionWidthLineEdit = new QLineEdit;
    m_cameraResolutionHeightLineEdit = new QLineEdit;
    cameraGridLayout->addWidget(cameraResolutionLabel, 1, 0);
    cameraGridLayout->addWidget(m_cameraResolutionWidthLineEdit, 1, 1);
    cameraGridLayout->addWidget(new QLabel("x"), 1, 2);
    cameraGridLayout->addWidget(m_cameraResolutionHeightLineEdit, 1, 3);
    cameraGridLayout->addWidget(new QLabel("(optional)"), 1, 4);
    // Resolution (Width) Validator
    QRegExp rx2("^[0-9]{1,4}$"); // Integers 0 to 9999
    QRegExpValidator *validator2 = new QRegExpValidator(rx2, 0);
    m_cameraResolutionWidthLineEdit->setValidator(validator2);
    // Resolution (Height) Validator
    QRegExp rx3("^[0-9]{1,4}$"); // Integers 0 to 9999
    QRegExpValidator *validator3 = new QRegExpValidator(rx3, 0);
    m_cameraResolutionHeightLineEdit->setValidator(validator3);

    cameraGroupBox->setLayout(cameraGridLayout);
    mainLayout->addWidget(cameraGroupBox, 3, 0, 1, 4);

    // Image Buffer group box
    QGroupBox *imageBufferGroupBox = new QGroupBox(tr("Image Buffer"));
    QGridLayout *imageBufferGridLayout = new QGridLayout;
    // Size
    QLabel *imageBufferSizeLabel = new QLabel(tr("Size") + ":");
    m_imageBufferSizeLineEdit = new QLineEdit;
    imageBufferGridLayout->addWidget(imageBufferSizeLabel, 0, 0);
    imageBufferGridLayout->addWidget(m_imageBufferSizeLineEdit, 0, 1);
    // Image Buffer Size Validator
    QRegExp rx4("^[0-9]{1,4}$"); // Integers 0 to 9999
    QRegExpValidator *validator4 = new QRegExpValidator(rx4, 0);
    m_imageBufferSizeLineEdit->setValidator(validator4);
    // Drop Frame if Buffer is Full
    m_dropFrameIfBufferFullCheckbox = new QCheckBox(tr("Drop frame if buffer full"));
    imageBufferGridLayout->addWidget(m_dropFrameIfBufferFullCheckbox, 1, 0, 1, 2);

    imageBufferGroupBox->setLayout(imageBufferGridLayout);
    mainLayout->addWidget(imageBufferGroupBox, 4, 0, 1, 4);

    // Thread Priorities group box
    QGroupBox *threadPrioritiesGroupBox = new QGroupBox(tr("Thread Priorities"));
    QGridLayout *threadPrioritiesGridLayout = new QGridLayout;
    // Capture Thread Priority
    QLabel *captureThreadPriorityLabel = new QLabel(tr("Capture Thread") + ":");
    m_captureThreadPriorityComboBox = new QComboBox;
    m_captureThreadPriorityComboBox->addItem(tr("Idle"), QThread::Priority::IdlePriority);
    m_captureThreadPriorityComboBox->addItem(tr("Lowest"), QThread::Priority::LowestPriority);
    m_captureThreadPriorityComboBox->addItem(tr("Low"), QThread::Priority::LowPriority);
    m_captureThreadPriorityComboBox->addItem(tr("Normal"), QThread::Priority::NormalPriority);
    m_captureThreadPriorityComboBox->addItem(tr("High"), QThread::Priority::HighPriority);
    m_captureThreadPriorityComboBox->addItem(tr("Highest"), QThread::Priority::HighestPriority);
    m_captureThreadPriorityComboBox->addItem(tr("Time Critical"), QThread::Priority::TimeCriticalPriority);
    threadPrioritiesGridLayout->addWidget(captureThreadPriorityLabel, 0, 0);
    threadPrioritiesGridLayout->addWidget(m_captureThreadPriorityComboBox, 0, 1);
    // Processing Thread Priority
    QLabel *processingThreadPriorityLabel = new QLabel(tr("Processing Thread") + ":");
    m_processingThreadPriorityComboBox = new QComboBox;
    m_processingThreadPriorityComboBox->addItem(tr("Idle"), QThread::Priority::IdlePriority);
    m_processingThreadPriorityComboBox->addItem(tr("Lowest"), QThread::Priority::LowestPriority);
    m_processingThreadPriorityComboBox->addItem(tr("Low"), QThread::Priority::LowPriority);
    m_processingThreadPriorityComboBox->addItem(tr("Normal"), QThread::Priority::NormalPriority);
    m_processingThreadPriorityComboBox->addItem(tr("High"), QThread::Priority::HighPriority);
    m_processingThreadPriorityComboBox->addItem(tr("Highest"), QThread::Priority::HighestPriority);
    m_processingThreadPriorityComboBox->addItem(tr("Time Critical"), QThread::Priority::TimeCriticalPriority);
    threadPrioritiesGridLayout->addWidget(processingThreadPriorityLabel, 1, 0);
    threadPrioritiesGridLayout->addWidget(m_processingThreadPriorityComboBox, 1, 1);

    threadPrioritiesGroupBox->setLayout(threadPrioritiesGridLayout);
    mainLayout->addWidget(threadPrioritiesGroupBox, 5, 0, 1, 4);

    // Horizontal line
    QFrame *horizontalLine = new QFrame;
    horizontalLine->setFrameShape(QFrame::HLine);
    horizontalLine->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(horizontalLine, 6, 0, 1, 4);

    // Reset to Defaults
    QPushButton *resetToDefaultsButton = new QPushButton(tr("Reset to Defaults"));
    connect(resetToDefaultsButton, &QPushButton::released, this, &CameraConnectDialog::resetToDefaults);
    mainLayout->addWidget(resetToDefaultsButton, 7, 0, 1, 4);

    // Dialog button
    QDialogButtonBox *dialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(dialogButtonBox->button(QDialogButtonBox::Ok), &QPushButton::released, this, &CameraConnectDialog::accept);
    connect(dialogButtonBox->button(QDialogButtonBox::Cancel), &QPushButton::released, this, &CameraConnectDialog::reject);
    mainLayout->addWidget(dialogButtonBox, 8, 0, 1, 4);

    // Update UI
    updateUi();

    // Set layout
    setLayout(mainLayout);
}

void CameraConnectDialog::updateUi()
{
    int index;
    m_cameraDeviceNumberLineEdit->setText(QString::number(m_nextDeviceNumber));
    m_cameraResolutionWidthLineEdit->setText("");
    m_cameraResolutionHeightLineEdit->setText("");
    m_imageBufferSizeLineEdit->setText(QString::number(DEFAULT_IMAGE_BUFFER_SIZE));
    m_dropFrameIfBufferFullCheckbox->setChecked(DEFAULT_DROP_FRAMES);
    setComboBoxByData(m_captureThreadPriorityComboBox, QThread::Priority::NormalPriority, index);
    setComboBoxByData(m_processingThreadPriorityComboBox, QThread::Priority::NormalPriority, index);
    m_tabNameLineEdit->setText("");
    m_enableFrameProcessingCheckbox->setChecked(DEFAULT_ENABLE_FRAME_PROCESSING);
    setComboBoxByData(m_streamControlComboBox, (int)SharedImageBuffer::StreamControl::Run, index);
    m_syncStreamCheckbox->setChecked(false);
}

CameraView::Settings CameraConnectDialog::settings()
{
    CameraView::Settings settings;
    settings.deviceNumber = m_cameraDeviceNumberLineEdit->text().toInt();
    settings.width = m_cameraResolutionHeightLineEdit->text().toInt();
    settings.height = m_cameraResolutionHeightLineEdit->text().toInt();
    settings.imageBufferSize = m_imageBufferSizeLineEdit->text().toInt();
    settings.dropFrameIfBufferFull = m_dropFrameIfBufferFullCheckbox->isChecked();
    settings.captureThreadPriority = (QThread::Priority)m_captureThreadPriorityComboBox->currentData().toInt();
    settings.processingThreadPriority = (QThread::Priority)m_processingThreadPriorityComboBox->currentData().toInt();
    settings.enableFrameProcessing = m_enableFrameProcessingCheckbox->isChecked();
    settings.streamControl = (SharedImageBuffer::StreamControl)m_streamControlComboBox->currentData().toInt();
    settings.synchronizeStream = m_syncStreamCheckbox->isChecked();
    return settings;
}

QString CameraConnectDialog::tabName()
{
    return m_tabNameLineEdit->text();
}

void CameraConnectDialog::resetToDefaults()
{
    updateUi();
}

bool CameraConnectDialog::setComboBoxByData(QComboBox *comboBox, QVariant data, int &index)
{
    for (int i = 0; i < comboBox->count(); i++)
    {
        if (data == comboBox->itemData(i))
        {
            index = i;
            comboBox->setCurrentIndex(index);
            return true;
        }
    }

    return false;
}
