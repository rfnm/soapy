#
# CMake
#

cmake_minimum_required(VERSION 3.22)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")



#
# Project
#

project(SoapyRFNMDriver
  LANGUAGES CXX
  VERSION 0.0.1
)

#select the release build type by default to get optimization flags
if(NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "RelWithDebInfo")
  message(STATUS "Build type not specified: defaulting to release.")
endif()

#enable PIC globally
set(CMAKE_POSITION_INDEPENDENT_CODE ON)


#
# Dependencies
#

find_package(Libusb REQUIRED)

include(cmake/cpm.cmake)
include(cmake/cpm-fmtlib.cmake)
include(cmake/cpm-spdlog.cmake)
include(cmake/cpm-soapysdr.cmake)



#
# Library
#

add_library(SoapyRFNMDriver SHARED)

# sources
target_sources(SoapyRFNMDriver PRIVATE
  "src/rfnm_conversion.cpp"
  "src/rfnm_construction.cpp"
  "src/rfnm_identification.cpp"
  "src/rfnm_stream.cpp"
  "src/rfnm_thread.cpp"
  "src/rfnm_types.cpp"
)

# definitions
if(MSVC)
  target_compile_definitions(SoapyRFNMDriver PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# features
target_compile_features(SoapyRFNMDriver PUBLIC cxx_std_23)

# libraries
target_link_libraries(SoapyRFNMDriver PRIVATE LIBUSB::LIBUSB)
target_link_libraries(SoapyRFNMDriver PRIVATE SoapySDR)
target_link_libraries(SoapyRFNMDriver PRIVATE fmt::fmt)
target_link_libraries(SoapyRFNMDriver PRIVATE spdlog)



#
# Installation
#

set(DEST_DIR ${CMAKE_INSTALL_LIBDIR}/SoapySDR/modules${SOAPY_SDR_ABI_VERSION}/)

install(
  FILES "$<TARGET_RUNTIME_DLLS:SoapyRFNMDriver>"
  DESTINATION ${DEST_DIR}
)

install(
  TARGETS SoapyRFNMDriver
  DESTINATION ${DEST_DIR}
)

if(MSVC)
  install(
    FILES "$<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:$<TARGET_PDB_FILE:SoapyRFNMDriver>>"
    DESTINATION "bin"
  )
endif()
