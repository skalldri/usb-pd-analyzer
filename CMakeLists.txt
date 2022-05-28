cmake_minimum_required (VERSION 3.13)

project(USBPDAnalyzer)

add_definitions( -DLOGIC2 )

# enable generation of compile_commands.json, helpful for IDEs to locate include files.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# custom CMake Modules are located in the cmake directory.
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(ExternalAnalyzerSDK)

set(SOURCES 
src/crc32.cpp
src/USBPDAnalyzer.cpp
src/USBPDAnalyzer.h
src/USBPDAnalyzerResults.cpp
src/USBPDAnalyzerResults.h
src/USBPDAnalyzerSettings.cpp
src/USBPDAnalyzerSettings.h
src/USBPDSimulationDataGenerator.cpp
src/USBPDSimulationDataGenerator.h
)

add_analyzer_plugin(${PROJECT_NAME} SOURCES ${SOURCES})
