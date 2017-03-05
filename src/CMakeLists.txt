cmake_minimum_required(VERSION 3.5.1)
project(QRCode)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(OpenCV)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
find_package(Qt5Widgets)

set(SOURCE_FILES
        ./Source/main.cpp
        ./Header/ImageBinarization.hpp
        ./Source/ImageBinarization.cpp
        ./Header/FinderPatternModel.hpp
        ./Source/Filesystem.cpp
        ./Header/Filesystem.hpp
        ./Source/Generator.cpp
        ./Header/Generator.hpp
        ./Header/CodeFinder.hpp
        ./Source/CodeFinder.cpp)

add_executable(main ${SOURCE_FILES})
target_link_libraries( main ${OpenCV_LIBS} )
