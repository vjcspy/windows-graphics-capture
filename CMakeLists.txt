cmake_minimum_required(VERSION 3.14)
project(ScreenCaptureApp CXX)

# Enable C++20 features for better coroutine support
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Common settings
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# Windows libraries needed for C++/WinRT and DirectX
set(COMMON_LIBRARIES
    windowsapp
    d3d11
    dxgi
    dwmapi
    user32
    gdi32
)

# Common compile definitions and options
set(COMMON_DEFINITIONS
    _WINRT_MESSAGES
    WINRT_LEAN_AND_MEAN
)

set(COMMON_OPTIONS
    "/await"
    "/EHsc"
)

# ========================================
# 1. Core Library (Static)
# ========================================
add_library(ScreenCaptureCore STATIC
    pch.cpp
    pch.h
    src/core/ScreenCaptureCore.h
    src/core/ScreenCaptureCore.cpp
)

target_link_libraries(ScreenCaptureCore PRIVATE ${COMMON_LIBRARIES})

if (MSVC)
    target_precompile_headers(ScreenCaptureCore PRIVATE pch.h)
    target_compile_definitions(ScreenCaptureCore PRIVATE ${COMMON_DEFINITIONS})
    target_compile_options(ScreenCaptureCore PRIVATE ${COMMON_OPTIONS})
endif()

target_include_directories(ScreenCaptureCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
)

# ========================================
# 2. Console Application
# ========================================
add_executable(ScreenCaptureApp
    src/console/main.cpp
)

target_link_libraries(ScreenCaptureApp PRIVATE 
    ScreenCaptureCore
    ${COMMON_LIBRARIES}
)

if (MSVC)
    target_precompile_headers(ScreenCaptureApp REUSE_FROM ScreenCaptureCore)
    target_compile_definitions(ScreenCaptureApp PRIVATE ${COMMON_DEFINITIONS})
    target_compile_options(ScreenCaptureApp PRIVATE ${COMMON_OPTIONS})
endif()

# ========================================
# 3. DLL Library
# ========================================
add_library(ScreenCaptureDLL SHARED
    src/dll/ScreenCaptureDLL.h
    src/dll/ScreenCaptureDLL.cpp
    src/dll/ScreenCaptureDLL.def
)

target_link_libraries(ScreenCaptureDLL PRIVATE 
    ScreenCaptureCore
    ${COMMON_LIBRARIES}
)

if (MSVC)
    target_precompile_headers(ScreenCaptureDLL REUSE_FROM ScreenCaptureCore)
    target_compile_definitions(ScreenCaptureDLL PRIVATE 
        ${COMMON_DEFINITIONS}
        SCREENCAPTUREDLL_EXPORTS
    )
    target_compile_options(ScreenCaptureDLL PRIVATE ${COMMON_OPTIONS})
endif()

# Set DLL export definition file
set_target_properties(ScreenCaptureDLL PROPERTIES
    LINK_FLAGS "/DEF:${CMAKE_CURRENT_SOURCE_DIR}/src/dll/ScreenCaptureDLL.def"
)

# ========================================
# Set output directories for all targets
# ========================================
set_target_properties(ScreenCaptureCore ScreenCaptureApp ScreenCaptureDLL PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)
