QT       += core gui

TARGET = qt-opencv-multithreaded
TEMPLATE = app

VERSION = 1.00

DEFINES += APP_VERSION=$$VERSION

FORMS = CameraConnectDialog.ui MainWindow.ui

SOURCES += main.cpp\
        MainWindow.cpp \
    CaptureThread.cpp \
    Controller.cpp \
    ImageBuffer.cpp \
    CameraConnectDialog.cpp \
    ProcessingThread.cpp \
    ShowIplImage.cpp \
    QLabelWithMouseTracking.cpp

HEADERS  += MainWindow.h \
    CaptureThread.h \
    Controller.h \
    ImageBuffer.h \
    CameraConnectDialog.h \
    DefaultValues.h \
    ProcessingThread.h \
    ShowIplImage.h \
    QLabelWithMouseTracking.h

LIBS += -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_ml -lopencv_video -lopencv_features2d -lopencv_calib3d -lopencv_objdetect -lopencv_contrib -lopencv_legacy -lopencv_flann
