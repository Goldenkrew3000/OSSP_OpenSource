cmake_minimum_required(VERSION 3.16)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(OSSP_Launcher)

add_executable(OSSP_Launcher MACOSX_BUNDLE
    main.cpp
    macho.c
)

set_target_properties(OSSP_Launcher PROPERTIES
    MACOSX_BUNDLE_BUNDLE_NAME "OSSP"
    MACOSX_BUNDLE_INFO_STRING "OSSP (OpenSubsonicPlayer)"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "v0.4a"
    MACOSX_BUNDLE_LONG_VERSION_STRING "v0.4a"
    MACOSX_BUNDLE_GUI_IDENTIFIER "org.hojuix.ossp"
    MACOSX_BUNDLE_COPYRIGHT "Copyright (c) Goldenkrew3000/Hojuix 2025-2026 GPLv3"
    MACOSX_BUNDLE_ICON_FILE AppIcon
)
