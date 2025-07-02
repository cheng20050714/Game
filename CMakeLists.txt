cmake_minimum_required(VERSION 3.16)

project(Game VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加 Qt Multimedia 模块
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Multimedia)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Multimedia)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    animation.h
    camera.h
    bullet.h
    chicken.h
    atlas.h
    timer.h
    chicken_fast.h
    chicken_medium.h
    chicken_slow.h
    vector2.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Game
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        global_resources.h
        resources.qrc
    )
else()
    if(ANDROID)
        add_library(Game SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(Game
            ${PROJECT_SOURCES}
        )
    endif()
endif()

# 链接 Qt Multimedia 模块
target_link_libraries(Game PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Multimedia)

# 复制资源到构建目录
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/./resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

set_target_properties(Game PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS Game
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Game)
endif()
