QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qt-opencv-multithreaded
TEMPLATE = app

DEFINES += APP_VERSION=\\\"1.3.0\\\"

SOURCES += main.cpp \
    MainWindow.cpp \
    MatToQImage.cpp \
    FrameLabel.cpp \
    CameraView.cpp \
    ProcessingThread.cpp \
    ImageBuffer.cpp \
    CaptureThread.cpp \
    CameraConnectDialog.cpp \
    ImageProcessingSettingsDialog.cpp

HEADERS  += \
    MainWindow.h \
    Config.h \
    MatToQImage.h \
    FrameLabel.h \
    Structures.h \
    CameraView.h \
    ProcessingThread.h \
    ImageBuffer.h \
    CaptureThread.h \
    CameraConnectDialog.h \
    ImageProcessingSettingsDialog.h

FORMS    += \
    MainWindow.ui \
    CameraView.ui \
    CameraConnectDialog.ui \
    ImageProcessingSettingsDialog.ui

LIBS += `pkg-config opencv --cflags --libs`
