// SPDX-FileCopyrightText: 2026 Aaghash Murugesan <aaghashm@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import org.kde.kirigami as Kirigami

Kirigami.ApplicationWindow {
    id: root

    title: i18nc("@title:window", "Persistent Stopwatch")
    width: Kirigami.Units.gridUnit * 22
    height: Kirigami.Units.gridUnit * 28

    pageStack.initialPage: Kirigami.Page {
        title: i18nc("@title", "Stopwatch")
    }
}
