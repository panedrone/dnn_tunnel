cmake_minimum_required(VERSION 3.0.0)
project(dnn_tunnel_test VERSION 0.1.0)

include(CTest)
enable_testing()

if (WIN32)
set(OpenCV_DIR "D:/opencv44/build")
set(OpenCV_INCLUDE_DIRS "D:/opencv44/build/opencv2")
endif (WIN32)

find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)

add_library(dnn_tunnel_core SHARED
pch.cpp pch.h dnn_tunnel_cv.h dnn_tunnel_cv.cpp CV_Utils.cpp CV_Utils.h CV_Rect.h 
CV_Item_Tracker.cpp CV_Item_Tracker.h 
CV_Tracking_Data.cpp CV_Tracking_Data.h
CV_Grouped_Boxes.cpp CV_Grouped_Boxes.h)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(OpenCV_INCLUDE_DIRS)
target_link_libraries(dnn_tunnel_core ${OpenCV_LIBS} ${CMAKE_THREAD_LIBS_INIT})