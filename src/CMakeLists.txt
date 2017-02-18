cmake_minimum_required(VERSION 3.6)
project(QRCode)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(OpenCV)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
find_package(Qt5Widgets)

set(SOURCE_FILES
        main.cpp
        ImageBinarization.hpp
        ImageBinarization.cpp
        FindPatern.hpp
        FindPatern.cpp
        FinderPaternModel.cpp
        FinderPaternModel.hpp
        Filesystem.cpp
        Filesystem.hpp
        Generate.cpp
        Generate.hpp)

add_executable(main ${SOURCE_FILES})
target_link_libraries( main ${OpenCV_LIBS} )
