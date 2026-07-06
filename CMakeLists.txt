# SPDX-FileCopyrightText: 2026 Aaghash Murugesan <aaghashm@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.20)

project(persistent-stopwatch VERSION 0.1.0 LANGUAGES CXX)

set(QT_MIN_VERSION "6.6.0")
set(KF_MIN_VERSION "6.0.0")

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMFindQmlModule)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS
    Core
    Gui
    Qml
    Quick
    QuickControls2
    Test
    Widgets
)
find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS
    Config
    CoreAddons
    I18n
    Kirigami
)
ecm_find_qmlmodule(org.kde.kirigami REQUIRED)

add_subdirectory(src)
if(BUILD_TESTING)
    add_subdirectory(autotests)
endif()

install(FILES io.github.aaghashm.persistentstopwatch.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES io.github.aaghashm.persistentstopwatch.metainfo.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
