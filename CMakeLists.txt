# SPDX-FileCopyrightText: 2026 Aaghash Murugesan <aaghashm@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.20)

project(persistent-stopwatch VERSION 0.2.0 LANGUAGES CXX)

set(QT_MIN_VERSION "6.6.0")
set(KF_MIN_VERSION "6.0.0")

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMFindQmlModule)
include(ECMInstallIcons)
include(ECMSetupVersion)
include(KDEClangFormat)
include(KDEGitCommitHooks)

ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX PERSISTENTSTOPWATCH
    VERSION_HEADER ${CMAKE_CURRENT_BINARY_DIR}/src/version.h
)

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
    Crash
    DBusAddons
    I18n
    Kirigami
    WindowSystem
)
ecm_find_qmlmodule(org.kde.kirigami REQUIRED)
ecm_find_qmlmodule(org.kde.coreaddons REQUIRED)

add_subdirectory(src)
# Translations land in po/ once there are any; the template comes from src/Messages.sh.
if(EXISTS ${CMAKE_SOURCE_DIR}/po)
    ki18n_install(po)
endif()
if(BUILD_TESTING)
    add_subdirectory(autotests)
endif()

ecm_install_icons(ICONS icons/sc-apps-io.github.aaghashm.persistentstopwatch.svg DESTINATION ${KDE_INSTALL_ICONDIR})
install(FILES io.github.aaghashm.persistentstopwatch.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES io.github.aaghashm.persistentstopwatch.metainfo.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES doc/persistent-stopwatch.1 DESTINATION ${KDE_INSTALL_MANDIR}/man1)
install(FILES completions/persistent-stopwatch.bash
        DESTINATION ${KDE_INSTALL_DATADIR}/bash-completion/completions
        RENAME persistent-stopwatch)
install(FILES completions/_persistent-stopwatch DESTINATION ${KDE_INSTALL_ZSHAUTOCOMPLETEDIR})
install(FILES completions/persistent-stopwatch.fish DESTINATION ${KDE_INSTALL_DATADIR}/fish/vendor_completions.d)

file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES src/*.cpp src/*.h autotests/*.cpp)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

# A .deb for Debian 13, Ubuntu 25.04 and newer: `cpack -G DEB` in the build
# directory. Fedora uses the spec in packaging/ instead.
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VENDOR "Aaghash Murugesan")
set(CPACK_PACKAGE_CONTACT "Aaghash Murugesan <aaghashm@gmail.com>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Stopwatch that survives reboots")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/aaghashm/persistent-stopwatch")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_PACKAGE_SECTION utils)
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
# QML modules are loaded at runtime, so dpkg-shlibdeps can't find them.
set(CPACK_DEBIAN_PACKAGE_DEPENDS "qml6-module-org-kde-kirigami, qml6-module-org-kde-coreaddons, qml6-module-qtquick-controls, qml6-module-qtquick-layouts, qml6-module-qtquick-templates, qml6-module-qtquick-window, qml6-module-qtqml-workerscript")
set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "qml6-module-org-kde-desktop")
include(CPack)
