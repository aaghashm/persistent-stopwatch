# SPDX-FileCopyrightText: 2026 Aaghash Murugesan <aaghashm@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

include(ECMAddTests)

ecm_add_test(
    stopwatchenginetest.cpp
    ../src/stopwatchengine.cpp
    ../src/duration.cpp
    ../src/lapsmodel.cpp
    TEST_NAME stopwatchenginetest
    LINK_LIBRARIES
        Qt6::Qml
        Qt6::Test
        KF6::ConfigCore
)
target_include_directories(stopwatchenginetest PRIVATE ../src)

ecm_add_test(
    commandlinetest.cpp
    ../src/commandline.cpp
    ../src/stopwatchengine.cpp
    ../src/duration.cpp
    ../src/lapsmodel.cpp
    TEST_NAME commandlinetest
    LINK_LIBRARIES
        Qt6::Qml
        Qt6::Test
        KF6::ConfigCore
        KF6::I18n
)
target_include_directories(commandlinetest PRIVATE ../src)

ecm_add_test(
    lapsmodeltest.cpp
    ../src/lapsmodel.cpp
    TEST_NAME lapsmodeltest
    LINK_LIBRARIES
        # QAbstractItemModelTester probes the decoration role for QPixmap.
        Qt6::Gui
        Qt6::Qml
        Qt6::Test
)
target_include_directories(lapsmodeltest PRIVATE ../src)
