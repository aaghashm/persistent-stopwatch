# SPDX-FileCopyrightText: 2026 Aaghash Murugesan <aaghashm@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

include(ECMAddTests)

ecm_add_test(
    stopwatchenginetest.cpp
    ../src/stopwatchengine.cpp
    ../src/lapsmodel.cpp
    TEST_NAME stopwatchenginetest
    LINK_LIBRARIES
        Qt6::Qml
        Qt6::Test
        KF6::ConfigCore
)
target_include_directories(stopwatchenginetest PRIVATE ../src)
