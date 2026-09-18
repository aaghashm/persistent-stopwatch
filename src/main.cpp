// SPDX-FileCopyrightText: 2026 Aaghash Murugesan <aaghashm@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QApplication>
#include <QCommandLineParser>
#include <QFile>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QTextStream>

#include <KAboutData>
#include <KConfigGroup>
#include <KCrash>
#include <KDBusService>
#include <KLocalizedQmlContext>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KWindowConfig>
#include <KWindowSystem>

#include "commandline.h"
#include "stopwatchengine.h"
#include "version.h"

using namespace Qt::Literals::StringLiterals;

// The style ships separately (only recommended on Debian), and asking for
// a style that isn't installed stops the window from loading at all.
static bool hasDesktopStyle(const QQmlEngine &engine)
{
    const QStringList importPaths = engine.importPathList();
    return std::any_of(importPaths.cbegin(), importPaths.cend(), [](const QString &path) {
        return QFile::exists(path + u"/org/kde/desktop/qmldir"_s);
    });
}

static void printReport(const QCommandLineParser &parser, const StopwatchEngine &stopwatch)
{
    QTextStream out(stdout);
    if (CommandLine::wantsJson(parser)) {
        out << QString::fromUtf8(CommandLine::statusJson(stopwatch)) << Qt::endl;
        return;
    }
    if (CommandLine::wantsLaps(parser)) {
        out << stopwatch.laps()->toTable();
    }
    if (CommandLine::wantsStatus(parser)) {
        out << CommandLine::statusLine(stopwatch) << Qt::endl;
    }
}

int main(int argc, char *argv[])
{
    // Command line use has to work over SSH and on a text console, where a
    // QApplication would abort for lack of a display. Only the window gets one.
    const bool windowed = CommandLine::needsWindow(argc, argv);
    std::unique_ptr<QCoreApplication> app(windowed ? new QApplication(argc, argv) : new QCoreApplication(argc, argv));

    KLocalizedString::setApplicationDomain(QByteArrayLiteral("persistentstopwatch"));

    KAboutData aboutData(u"persistentstopwatch"_s,
                         i18nc("@title application name", "Persistent Stopwatch"),
                         QStringLiteral(PERSISTENTSTOPWATCH_VERSION_STRING),
                         i18n("Stopwatch that keeps counting across restarts and reboots"),
                         KAboutLicense::GPL_V2,
                         i18n("© 2026 Aaghash Murugesan"));
    aboutData.addAuthor(i18nc("@info:credit", "Aaghash Murugesan"), i18nc("@info:credit role", "Maintainer"), u"aaghashm@gmail.com"_s);
    KAboutData::setApplicationData(aboutData);
    KCrash::initialize();

    QCommandLineParser parser;
    CommandLine::addOptions(parser);
    aboutData.setupCommandLine(&parser);
    parser.process(*app);
    aboutData.processCommandLine(&parser);

    // Reading needs no coordination: whoever owns the state syncs it to disk
    // on every change, so the file is always current.
    const bool wantsReport = CommandLine::wantsStatus(parser) || CommandLine::wantsLaps(parser) || CommandLine::wantsJson(parser);
    if (wantsReport && !CommandLine::hasAction(parser)) {
        const StopwatchEngine stopwatch;
        printReport(parser, stopwatch);
        return 0;
    }

    // Two windows would each hold their own copy of the state and overwrite
    // each other's saves. A second launch hands its arguments to the first
    // one and exits here. Without a session bus we simply run unguarded.
    KDBusService service(KDBusService::Unique | KDBusService::NoExitOnFailure);

    StopwatchEngine stopwatch;

    // Nothing else is running, so the state on disk is ours: apply the
    // actions and leave without ever showing a window.
    if (CommandLine::hasAction(parser)) {
        CommandLine::applyActions(parser, stopwatch);
        printReport(parser, stopwatch);
        return 0;
    }

    QGuiApplication::setWindowIcon(QIcon::fromTheme(u"io.github.aaghashm.persistentstopwatch"_s, QIcon::fromTheme(u"chronometer"_s)));
    QGuiApplication::setDesktopFileName(u"io.github.aaghashm.persistentstopwatch"_s);

    QQmlApplicationEngine engine;

    // Without the org.kde.desktop style, QML controls fall back to the plain
    // Qt look and feel out of place on Plasma. Respect an explicit override.
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE") && hasDesktopStyle(engine)) {
        QQuickStyle::setStyle(u"org.kde.desktop"_s);
    }
    KLocalization::setupLocalizedContext(&engine);
    engine.setInitialProperties({{u"stopwatch"_s, QVariant::fromValue(&stopwatch)}});
    engine.loadFromModule(u"io.github.aaghashm.persistentstopwatch"_s, u"Main"_s);
    if (engine.rootObjects().isEmpty()) {
        return 1;
    }

    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());

    // Size only: on Wayland the compositor decides where windows go anyway.
    KConfigGroup windowState = KSharedConfig::openStateConfig()->group(u"Window"_s);
    KWindowConfig::restoreWindowSize(window, windowState);
    QObject::connect(app.get(), &QCoreApplication::aboutToQuit, window, [window, windowState]() mutable {
        KWindowConfig::saveWindowSize(window, windowState);
        windowState.sync();
    });
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

    return app->exec();
}
