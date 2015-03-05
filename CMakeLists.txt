cmake_minimum_required(VERSION 2.8.11)

project(qt-opencv-multithreaded)

set(PROJECT_NAME "qt-opencv-multithreaded")
set(PROJECT_VERSION "1.4.0")

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

########
# Qt 5 #
########
find_package(Qt5Widgets REQUIRED)

##########
# OpenCV #
##########
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

###############
# Application #
###############
add_subdirectory(src)