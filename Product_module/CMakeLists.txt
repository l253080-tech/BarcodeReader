cmake_minimum_required(VERSION 3.16)
project(ProductModule LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ProductTest
    ../src/test_products.cpp
    ../src/Product.cpp
    ../src/ProductFileHandler.cpp
    ../src/ProductManager.cpp
)

add_custom_command(TARGET ProductTest POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/Product.txt"
        "$<TARGET_FILE_DIR:ProductTest>")