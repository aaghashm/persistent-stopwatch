# SPDX-FileCopyrightText: 2026 Aaghash Murugesan <aaghashm@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

add_executable(persistent-stopwatch)

qt_policy(SET QTP0001 NEW)

ecm_add_qml_module(persistent-stopwatch
    URI io.github.aaghashm.persistentstopwatch
    QML_FILES
        Main.qml
)

target_sources(persistent-stopwatch PRIVATE
    lapsmodel.cpp
    lapsmodel.h
    main.cpp
    stopwatchengine.cpp
    stopwatchengine.h
)

target_link_libraries(persistent-stopwatch PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Widgets
    KF6::ConfigCore
    KF6::CoreAddons
    KF6::DBusAddons
    KF6::I18n
    KF6::I18nQml
    KF6::WindowSystem
)

install(TARGETS persistent-stopwatch ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
