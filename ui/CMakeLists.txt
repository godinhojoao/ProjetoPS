cmake_minimum_required(VERSION 3.16)

project(ui VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        resources.qrc
        ../src/cpu/cpu.h
        ../src/cpu/cpu.cpp
        ../src/instruction_set/decoder.cpp
        ../src/instruction_set/decoder.h
        ../src/instruction_set/instruction.h
        ../src/loader/binary_loader.cpp
        ../src/loader/binary_loader.h
        ../src/loader/object_loader.cpp
        ../src/loader/object_loader.h
        ../src/memory/memory.cpp
        ../src/memory/memory.h
        ../src/vm/vm.cpp
        ../src/vm/vm.h
        ../src/constants.h
        # ../src/main_cli.cpp
        ../src/macro_processor/macro_processor.cpp
        ../src/macro_processor/macro_processor.h
        ../src/shared/shared.cpp
        ../src/shared/shared.h
        ../src/assembler/assembler.cpp
        ../src/assembler/assembler.h
        ../src/linker/extref_resolver.cpp
        ../src/linker/extref_resolver.h
        ../src/linker/global_symbol_table.cpp
        ../src/linker/global_symbol_table.h
        ../src/linker/linker.cpp
        ../src/linker/linker.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(ui
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        Console.h Console.cpp
        Project.h Project.cpp
        vmstate.h
        btnrun_popup.h btnrun_popup.cpp btnrun_popup.ui
        btnbuild_popup.h btnbuild_popup.cpp btnbuild_popup.ui


    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET ui APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(ui SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(ui
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_include_directories(ui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)
target_include_directories(ui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../data)

target_link_libraries(ui PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.ui)
endif()
set_target_properties(ui PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS ui
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ui)
endif()
