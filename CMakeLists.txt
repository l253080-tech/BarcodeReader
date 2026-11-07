cmake_minimum_required(VERSION 3.21)
project(BarcodeReader VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia MultimediaWidgets)

qt_add_executable(BarcodeReader
    app/main.cpp
    app/mainwindow.h
    app/mainwindow.cpp
    app/generatetab.h
    app/generatetab.cpp
    app/readtab.h
    app/readtab.cpp

    # Core logic (member 2). Uncomment after his branch is merged into main:
     src/Barcode_Encoder.cpp
     src/Barcode_Decoder.cpp
)

target_include_directories(BarcodeReader PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(BarcodeReader PRIVATE
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::MultimediaWidgets
)
