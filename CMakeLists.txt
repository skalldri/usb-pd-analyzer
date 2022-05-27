cmake_minimum_required(VERSION 3.21.4)

project(usb-pd-analyzer)

# The Saleae Analyzer SDK library
add_library(saleae-analyzer-sdk SHARED IMPORTED)

set_target_properties(
  saleae-analyzer-sdk
  PROPERTIES
  IMPORTED_IMPLIB ${CMAKE_CURRENT_LIST_DIR}/SaleaeAnalyzerSdk-1.1.14/lib/Analyzer.lib
  IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/SaleaeAnalyzerSdk-1.1.14/lib/Analyzer.dll
)

add_library(USBPDAnalyzer SHARED)

target_sources(
    USBPDAnalyzer
    PUBLIC
      source/USBPDAnalyzer.cpp
      source/USBPDAnalyzerResults.cpp
      source/USBPDAnalyzerSettings.cpp
      source/USBPDSimulationDataGenerator.cpp
)

target_include_directories(
    USBPDAnalyzer
    PUBLIC
      source
      SaleaeAnalyzerSdk-1.1.14/include
)

target_link_libraries(
  USBPDAnalyzer
  PUBLIC
  saleae-analyzer-sdk
)