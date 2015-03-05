file(GLOB_RECURSE UI_FILES *.ui)
file(GLOB_RECURSE SOURCE_FILES *.cpp)
file(GLOB_RECURSE HEADER_FILES *.h)

qt5_wrap_ui(UI_HEADERS ${UI_FILES})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/Config.h
)

add_executable(${CMAKE_PROJECT_NAME}
  ${UI_HEADERS}
  ${SOURCE_FILES}
  ${HEADER_FILES}
)

target_link_libraries(${CMAKE_PROJECT_NAME}
  Qt5::Widgets
  ${OpenCV_LIBS}
)