cmake_minimum_required(VERSION 3.16)

project(UniversalChart VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Для всієї збірки ОПТИМІЗАЦІ
# add_definitions(-DQCUSTOMPLOT_USE_OPENGL)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets
  Positioning
  PrintSupport
  Qml
  QuickWidgets
  Widgets
  SerialPort
  SerialBus
  QuickControls2
)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h

)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(UniversalChart
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        Chart/ChartPanelWidget.cpp Chart/chartpanelwidget.h Chart/Fields_lookup.h Chart/Fields_parametrs.h Chart/FieldSpec.h Chart/FlightChart.cpp Chart/flightchart.h Chart/parametrs.h Chart/qcustomplot.cpp Chart/qcustomplot.h Chart/README.md
        Chart/resources.qrc
        Chart/resources.qrc

    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET UniversalChart APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(UniversalChart SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(UniversalChart
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(UniversalChart PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt6::Positioning Qt6::PrintSupport
  Qt6::Quick
  Qt6::QuickWidgets
  Qt6::Qml
  Qt${QT_VERSION_MAJOR}::Qml
  Qt${QT_VERSION_MAJOR}::Quick
  Qt${QT_VERSION_MAJOR}::QuickControls2
  Qt${QT_VERSION_MAJOR}::QuickWidgets
  Qt${QT_VERSION_MAJOR}::PrintSupport
)
# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.UniversalChart)
endif()
set_target_properties(UniversalChart PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS UniversalChart
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(UniversalChart)
endif()
