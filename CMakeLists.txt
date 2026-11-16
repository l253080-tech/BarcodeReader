cmake_minimum_required(VERSION 3.21)
project(BarcodeReader VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia MultimediaWidgets)

# ---------------------------------------------------------------------------
# Product module (member 1) - plain C++, no Qt
# ---------------------------------------------------------------------------
add_library(ProductModule STATIC
    src/Product.h
    src/Product.cpp
    src/ProductFileHandler.h
    src/ProductFileHandler.cpp
    src/ProductManager.h
    src/ProductManager.cpp
)
target_include_directories(ProductModule PUBLIC ${CMAKE_SOURCE_DIR}/src)

# ---------------------------------------------------------------------------
# Order module (member 3) - plain C++, no Qt, no dependency on ProductModule
# ---------------------------------------------------------------------------
add_library(OrderModule STATIC
    src/IProductCatalog.h
    src/OrderTypes.h
    src/OrderResult.h
    src/OrderItem.h
    src/OrderItem.cpp
    src/Order.h
    src/Order.cpp
    src/OrderIdGenerator.h
    src/OrderIdGenerator.cpp
    src/OrderManager.h
    src/OrderManager.cpp
)
target_include_directories(OrderModule PUBLIC ${CMAKE_SOURCE_DIR}/src)

# Adapter that connects the two modules (ProductManager -> IProductCatalog)
add_library(OrderProductAdapter STATIC
    src/ProductManagerCatalog.h
    src/ProductManagerCatalog.cpp
)
target_link_libraries(OrderProductAdapter PUBLIC OrderModule ProductModule)

# Copies Product.txt next to a target's executable so it can be opened at runtime
function(copy_product_data target)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/Product.txt"
            "$<TARGET_FILE_DIR:${target}>")
endfunction()

# ---------------------------------------------------------------------------
# Main application
# ---------------------------------------------------------------------------
qt_add_executable(BarcodeReader
    app/main.cpp
    app/mainwindow.h
    app/mainwindow.cpp
    app/generatetab.h
    app/generatetab.cpp
    app/readtab.h
    app/readtab.cpp
    app/cameradialog.h
    app/cameradialog.cpp

    # Core logic (member 2)
    src/Barcode_Encoder.cpp
    src/Barcode_Encoder.h
    src/Barcode_Decoder.cpp
    src/Barcode_Decoder.h
    src/Barcode_Tabels.h

    # Image scanner (member 4)
    src/ImageScanner.cpp
    src/ImageScanner.h
)

target_include_directories(BarcodeReader PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(BarcodeReader PRIVATE
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::MultimediaWidgets
    ProductModule
    OrderModule
    OrderProductAdapter
)
copy_product_data(BarcodeReader)

# ---------------------------------------------------------------------------
# Tests (no Qt needed) - run with ctest, or run each executable
# ---------------------------------------------------------------------------
enable_testing()

add_executable(ProductTest src/test_products.cpp)
target_link_libraries(ProductTest PRIVATE ProductModule)
copy_product_data(ProductTest)
add_test(NAME ProductTest COMMAND ProductTest)

add_executable(OrderManagerTest tests/OrderManagerTest.cpp)
target_link_libraries(OrderManagerTest PRIVATE OrderModule)
add_test(NAME OrderManagerTest COMMAND OrderManagerTest)

add_executable(OrderIntegrationTest tests/OrderIntegrationTest.cpp)
target_link_libraries(OrderIntegrationTest PRIVATE OrderProductAdapter)
copy_product_data(OrderIntegrationTest)
add_test(NAME OrderIntegrationTest COMMAND OrderIntegrationTest
         WORKING_DIRECTORY $<TARGET_FILE_DIR:OrderIntegrationTest>)
