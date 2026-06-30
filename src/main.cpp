// SPDX-FileCopyrightText: 2026 Aaghash Murugesan <aaghashm@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>

#include <KLocalizedQmlContext>
#include <KLocalizedString>

using namespace Qt::Literals::StringLiterals;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    KLocalizedString::setApplicationDomain(QByteArrayLiteral("persistentstopwatch"));

    // Without the org.kde.desktop style, QML controls fall back to the plain
    // Qt look and feel out of place on Plasma. Respect an explicit override.
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle(u"org.kde.desktop"_s);
    }

    QQmlApplicationEngine engine;
    KLocalization::setupLocalizedContext(&engine);
    engine.loadFromModule(u"io.github.aaghashm.persistentstopwatch"_s, u"Main"_s);
    if (engine.rootObjects().isEmpty()) {
        return 1;
    }

    return app.exec();
}
