// SPDX-FileCopyrightText: 2026 Aaghash Murugesan <aaghashm@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQuickStyle>

#include <KAboutData>
#include <KLocalizedQmlContext>
#include <KLocalizedString>

using namespace Qt::Literals::StringLiterals;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    KLocalizedString::setApplicationDomain(QByteArrayLiteral("persistentstopwatch"));

    KAboutData aboutData(u"persistentstopwatch"_s,
                         i18nc("@title application name", "Persistent Stopwatch"),
                         u"0.1.0"_s,
                         i18n("Stopwatch that keeps counting across restarts and reboots"),
                         KAboutLicense::GPL_V2,
                         i18n("© 2026 Aaghash Murugesan"));
    aboutData.addAuthor(i18nc("@info:credit", "Aaghash Murugesan"), i18nc("@info:credit role", "Maintainer"), u"aaghashm@gmail.com"_s);
    KAboutData::setApplicationData(aboutData);

    QGuiApplication::setWindowIcon(QIcon::fromTheme(u"io.github.aaghashm.persistentstopwatch"_s, QIcon::fromTheme(u"chronometer"_s)));
    QGuiApplication::setDesktopFileName(u"io.github.aaghashm.persistentstopwatch"_s);

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
