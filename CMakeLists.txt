cmake_minimum_required(VERSION 3.6)
project(qrcode)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        src/FinderPaternModel.cpp
        src/FinderPaternModel.hpp
        src/FindPatern.cpp
        src/FindPatern.hpp
        src/ImageBinarization.cpp
        src/ImageBinarization.hpp
        src/ImageReader.cpp
        src/ImageReader.hpp
        src/main.cpp)

add_executable(qrcode ${SOURCE_FILES})