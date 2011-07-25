QT       += core gui

TARGET = qt-opencv-multithreaded
TEMPLATE = app

VERSION = 1.21

DEFINES += APP_VERSION=$$VERSION

FORMS = CameraConnectDialog.ui MainWindow.ui ProcessingSettingsDialog.ui

SOURCES += main.cpp\
        MainWindow.cpp \
    CaptureThread.cpp \
    Controller.cpp \
    ImageBuffer.cpp \
    CameraConnectDialog.cpp \
    ProcessingThread.cpp \
    FrameLabel.cpp \
    ProcessingSettingsDialog.cpp \
    MatToQImage.cpp

HEADERS  += MainWindow.h \
    CaptureThread.h \
    Controller.h \
    ImageBuffer.h \
    CameraConnectDialog.h \
    ProcessingThread.h \
    FrameLabel.h \
    ProcessingSettingsDialog.h \
    Structures.h \
    Config.h \
    MatToQImage.h

LIBS += -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_ml -lopencv_video -lopencv_features2d -lopencv_calib3d -lopencv_objdetect -lopencv_contrib -lopencv_legacy -lopencv_flann
