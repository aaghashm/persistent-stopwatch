// SPDX-FileCopyrightText: 2026 Aaghash Murugesan <aaghashm@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import org.kde.coreaddons as KCoreAddons
import io.github.aaghashm.persistentstopwatch

Kirigami.ApplicationWindow {
    id: root

    title: i18nc("@title:window", "Persistent Stopwatch")
    width: Kirigami.Units.gridUnit * 22
    height: Kirigami.Units.gridUnit * 28
    minimumWidth: Kirigami.Units.gridUnit * 16
    minimumHeight: Kirigami.Units.gridUnit * 20

    required property StopwatchEngine stopwatch
    readonly property string startedAtText: stopwatch.startedAt.toLocaleString(Qt.locale(), Locale.ShortFormat)

    Shortcut {
        sequence: "Space"
        onActivated: stopwatch.state === StopwatchEngine.Running ? stopwatch.pause() : stopwatch.start()
    }
    Shortcut {
        sequence: "L"
        onActivated: stopwatch.lap()
    }
    Shortcut {
        sequence: "R"
        onActivated: stopwatch.reset()
    }

    globalDrawer: Kirigami.GlobalDrawer {
        isMenu: true
        actions: [
            Kirigami.Action {
                text: i18nc("@action:inmenu", "About Persistent Stopwatch")
                icon.name: "help-about"
                onTriggered: root.pageStack.pushDialogLayer(aboutPage)
            },
            Kirigami.Action {
                text: i18nc("@action:inmenu", "Quit")
                icon.name: "application-exit"
                shortcut: StandardKey.Quit
                onTriggered: Qt.quit()
            }
        ]
    }

    Component {
        id: aboutPage

        Kirigami.AboutPage {
            aboutData: KCoreAddons.AboutData
        }
    }

    pageStack.initialPage: Kirigami.Page {
        title: i18nc("@title", "Stopwatch")

        actions: [
            Kirigami.Action {
                text: stopwatch.state === StopwatchEngine.Running
                    ? i18nc("@action:button pause the stopwatch", "Pause")
                    : i18nc("@action:button start the stopwatch", "Start")
                icon.name: stopwatch.state === StopwatchEngine.Running ? "chronometer-pause" : "chronometer-start"
                onTriggered: stopwatch.state === StopwatchEngine.Running ? stopwatch.pause() : stopwatch.start()
            },
            Kirigami.Action {
                text: i18nc("@action:button record a lap", "Lap")
                icon.name: "chronometer-lap"
                enabled: stopwatch.state === StopwatchEngine.Running
                onTriggered: stopwatch.lap()
            },
            Kirigami.Action {
                text: i18nc("@action:button", "Copy Laps")
                icon.name: "edit-copy"
                displayHint: Kirigami.DisplayHint.AlwaysHide
                enabled: lapsView.count > 0
                onTriggered: {
                    stopwatch.laps.copyToClipboard()
                    root.showPassiveNotification(i18nc("@info:status", "Laps copied to the clipboard"))
                }
            },
            Kirigami.Action {
                text: i18nc("@action:button reset the stopwatch to zero", "Reset")
                icon.name: "chronometer-reset"
                enabled: stopwatch.state !== StopwatchEngine.Stopped
                onTriggered: stopwatch.reset()
            }
        ]

        ColumnLayout {
            anchors.fill: parent
            spacing: Kirigami.Units.largeSpacing

            Kirigami.Heading {
                Layout.alignment: Qt.AlignHCenter
                Layout.topMargin: Kirigami.Units.gridUnit
                text: stopwatch.formatDuration(stopwatch.elapsedMs)
                font.family: Kirigami.Theme.fixedWidthFont.family
                font.pointSize: Kirigami.Theme.defaultFont.pointSize * 4
            }

            Controls.Label {
                Layout.alignment: Qt.AlignHCenter
                text: stopwatch.state === StopwatchEngine.Paused
                    ? i18nc("@info:status %1 is a date and time", "Paused · started %1", root.startedAtText)
                    : i18nc("@info:status %1 is a date and time", "Started %1", root.startedAtText)
                opacity: stopwatch.state === StopwatchEngine.Stopped ? 0 : 0.7
            }

            ListView {
                id: lapsView

                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: stopwatch.laps

                delegate: Controls.ItemDelegate {
                    id: lapDelegate

                    required property int number
                    required property double splitMs
                    required property double totalMs
                    required property bool isFastest
                    required property bool isSlowest

                    width: ListView.view.width

                    contentItem: RowLayout {
                        spacing: Kirigami.Units.largeSpacing

                        Controls.Label {
                            text: i18nc("@item:intable numbered lap", "Lap %1", number)
                            opacity: 0.7
                        }
                        Item {
                            Layout.fillWidth: true
                        }
                        Controls.Label {
                            text: "+" + stopwatch.formatDuration(splitMs)
                            font.family: Kirigami.Theme.fixedWidthFont.family
                            color: lapDelegate.isFastest ? Kirigami.Theme.positiveTextColor
                                 : lapDelegate.isSlowest ? Kirigami.Theme.negativeTextColor
                                 : Kirigami.Theme.textColor
                        }
                        Controls.Label {
                            text: stopwatch.formatDuration(totalMs)
                            font.family: Kirigami.Theme.fixedWidthFont.family
                        }
                    }
                }

                Kirigami.PlaceholderMessage {
                    anchors.centerIn: parent
                    width: parent.width - Kirigami.Units.gridUnit * 4
                    visible: lapsView.count === 0
                    icon.name: "chronometer"
                    text: i18nc("@info:placeholder", "No laps yet")
                    explanation: i18nc("@info:placeholder", "Press Lap while the stopwatch is running")
                }
            }
        }
    }
}
