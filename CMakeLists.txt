cmake_minimum_required(VERSION 3.21)

project(cl384_python_wrapper
    VERSION 0.1.0
    LANGUAGES CXX
)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# Library

add_library(cl384_python_wrapper SHARED
    main.cpp
)

add_library(cl384_python_wrapper::cl384_python_wrapper ALIAS cl384_python_wrapper)

# C standard

set_target_properties(cl384_python_wrapper PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    SUFFIX ".pyd"
)

# Headers

cmake_path(SET PYTHON_3_11_7_PATH_CMAKE "$ENV{PYTHON_3_11_7_PATH}")
cmake_path(SET PYBIND_11_PATH_CMAKE "$ENV{PYBIND_11_PATH}")

target_include_directories(cl384_python_wrapper
    PUBLIC
    $<BUILD_INTERFACE:${PYTHON_3_11_7_PATH_CMAKE}/include>
    $<BUILD_INTERFACE:${PYBIND_11_PATH_CMAKE}/include>
)

# Dependencies

cmake_path(SET E384COMMLIB_PATH_CMAKE "$ENV{E384COMMLIB_PATH}")
list(APPEND CMAKE_PREFIX_PATH "${E384COMMLIB_PATH_CMAKE}")

find_package(e384commlib REQUIRED CONFIG)

target_link_libraries(cl384_python_wrapper
    PUBLIC
    e384commlib::e384commlib
)

target_link_libraries(cl384_python_wrapper PUBLIC "${PYTHON_3_11_7_PATH_CMAKE}/libs/python311.lib")
if(MSVC)
    target_compile_options(cl384_python_wrapper PRIVATE /bigobj)
endif()