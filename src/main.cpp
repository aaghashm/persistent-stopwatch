// SPDX-FileCopyrightText: 2026 Aaghash Murugesan <aaghashm@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QApplication>
#include <QCommandLineParser>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQuickWindow>

#include <KAboutData>
#include <KCrash>
#include <KDBusService>
#include <KLocalizedQmlContext>
#include <KLocalizedString>
#include <KWindowSystem>

#include "commandline.h"
#include "stopwatchengine.h"

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
    KCrash::initialize();

    QCommandLineParser parser;
    CommandLine::addOptions(parser);
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    QGuiApplication::setWindowIcon(QIcon::fromTheme(u"io.github.aaghashm.persistentstopwatch"_s, QIcon::fromTheme(u"chronometer"_s)));
    QGuiApplication::setDesktopFileName(u"io.github.aaghashm.persistentstopwatch"_s);

    // Two windows would each hold their own copy of the state and overwrite
    // each other's saves. A second launch hands its arguments to the first
    // one and exits here. Without a session bus we simply run unguarded.
    KDBusService service(KDBusService::Unique | KDBusService::NoExitOnFailure);

    StopwatchEngine stopwatch;

    // Nothing else is running, so the state on disk is ours: apply the
    // actions and leave without ever showing a window.
    if (CommandLine::hasAction(parser)) {
        CommandLine::applyActions(parser, stopwatch);
        return 0;
    }

    // Without the org.kde.desktop style, QML controls fall back to the plain
    // Qt look and feel out of place on Plasma. Respect an explicit override.
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle(u"org.kde.desktop"_s);
    }

    QQmlApplicationEngine engine;
    KLocalization::setupLocalizedContext(&engine);
    engine.setInitialProperties({{u"stopwatch"_s, QVariant::fromValue(&stopwatch)}});
    engine.loadFromModule(u"io.github.aaghashm.persistentstopwatch"_s, u"Main"_s);
    if (engine.rootObjects().isEmpty()) {
        return 1;
    }

    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
    QObject::connect(&service, &KDBusService::activateRequested, window, [window, &stopwatch](const QStringList &arguments) {
        QCommandLineParser remoteParser;
        CommandLine::addOptions(remoteParser);
        if (remoteParser.parse(arguments) && CommandLine::hasAction(remoteParser)) {
            CommandLine::applyActions(remoteParser, stopwatch);
            return;
        }
        KWindowSystem::updateStartupId(window);
        KWindowSystem::activateWindow(window);
    });

    return app.exec();
}
