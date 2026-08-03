// SPDX-FileCopyrightText: 2026 Aaghash Murugesan <aaghashm@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "commandline.h"

#include <QCommandLineParser>
#include <QSet>

#include <KLocalizedString>

#include "stopwatchengine.h"

using namespace Qt::Literals::StringLiterals;

namespace
{
const QString startOption = u"start"_s;
const QString pauseOption = u"pause"_s;
const QString lapOption = u"lap"_s;
const QString resetOption = u"reset"_s;
const QString statusOption = u"status"_s;
const QString lapsOption = u"laps"_s;
}

void CommandLine::addOptions(QCommandLineParser &parser)
{
    parser.addOptions({
        {startOption, i18nc("@info:shell", "Start or resume the stopwatch.")},
        {pauseOption, i18nc("@info:shell", "Pause the stopwatch.")},
        {lapOption, i18nc("@info:shell", "Record a lap.")},
        {resetOption, i18nc("@info:shell", "Reset the stopwatch to zero and clear the laps.")},
        {statusOption, i18nc("@info:shell", "Print the state and elapsed time, then exit.")},
        {lapsOption, i18nc("@info:shell", "Print the recorded laps, then exit.")},
    });
}

bool CommandLine::needsWindow(int argc, char *argv[])
{
    static const QSet<QByteArray> headless = {
        "--start",
        "--pause",
        "--lap",
        "--reset",
        "--status",
        "--laps",
        "-h",
        "--help",
        "-v",
        "--version",
        "--author",
        "--license",
    };
    for (int i = 1; i < argc; ++i) {
        if (headless.contains(QByteArray(argv[i]))) {
            return false;
        }
    }
    return true;
}

bool CommandLine::hasAction(const QCommandLineParser &parser)
{
    return parser.isSet(startOption) || parser.isSet(pauseOption) || parser.isSet(lapOption) || parser.isSet(resetOption);
}

bool CommandLine::wantsStatus(const QCommandLineParser &parser)
{
    return parser.isSet(statusOption);
}

bool CommandLine::wantsLaps(const QCommandLineParser &parser)
{
    return parser.isSet(lapsOption);
}

void CommandLine::applyActions(const QCommandLineParser &parser, StopwatchEngine &engine)
{
    if (parser.isSet(resetOption)) {
        engine.reset();
    }
    if (parser.isSet(startOption)) {
        engine.start();
    }
    if (parser.isSet(lapOption)) {
        engine.lap();
    }
    if (parser.isSet(pauseOption)) {
        engine.pause();
    }
}

QString CommandLine::statusLine(const StopwatchEngine &engine)
{
    QString state;
    switch (engine.state()) {
    case StopwatchEngine::Running:
        state = i18nc("@info:shell stopwatch state", "Running");
        break;
    case StopwatchEngine::Paused:
        state = i18nc("@info:shell stopwatch state", "Paused");
        break;
    case StopwatchEngine::Stopped:
        state = i18nc("@info:shell stopwatch state", "Stopped");
        break;
    }
    return state + u' ' + engine.formatDuration(engine.elapsedMs());
}

QString CommandLine::lapsTable(const StopwatchEngine &engine)
{
    const LapsModel *laps = engine.laps();
    QString table;
    // The model shows the newest lap first; a table reads better oldest first.
    for (int row = laps->rowCount() - 1; row >= 0; --row) {
        const QModelIndex index = laps->index(row, 0);
        table += QString::number(index.data(LapsModel::NumberRole).toInt()) + u'\t' + engine.formatDuration(index.data(LapsModel::SplitMsRole).toLongLong())
            + u'\t' + engine.formatDuration(index.data(LapsModel::TotalMsRole).toLongLong()) + u'\n';
    }
    return table;
}
