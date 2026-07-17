// SPDX-FileCopyrightText: 2026 Aaghash Murugesan <aaghashm@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "commandline.h"

#include <QCommandLineParser>

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
}

void CommandLine::addOptions(QCommandLineParser &parser)
{
    parser.addOptions({
        {startOption, i18nc("@info:shell", "Start or resume the stopwatch.")},
        {pauseOption, i18nc("@info:shell", "Pause the stopwatch.")},
        {lapOption, i18nc("@info:shell", "Record a lap.")},
        {resetOption, i18nc("@info:shell", "Reset the stopwatch to zero and clear the laps.")},
        {statusOption, i18nc("@info:shell", "Print the state and elapsed time, then exit.")},
    });
}

bool CommandLine::hasAction(const QCommandLineParser &parser)
{
    return parser.isSet(startOption) || parser.isSet(pauseOption) || parser.isSet(lapOption) || parser.isSet(resetOption);
}

bool CommandLine::wantsStatus(const QCommandLineParser &parser)
{
    return parser.isSet(statusOption);
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
