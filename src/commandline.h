// SPDX-FileCopyrightText: 2026 Aaghash Murugesan <aaghashm@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

class QCommandLineParser;
class QString;
class StopwatchEngine;

/**
 * Command line actions, so the stopwatch can be driven from a terminal,
 * a script or a global shortcut without opening the window.
 */
namespace CommandLine
{
void addOptions(QCommandLineParser &parser);

// Decided from the raw arguments, because it picks which kind of
// application object to create, before a QCommandLineParser can exist.
bool needsWindow(int argc, char *argv[]);

bool hasAction(const QCommandLineParser &parser);
bool wantsStatus(const QCommandLineParser &parser);
bool wantsLaps(const QCommandLineParser &parser);

// Applies reset, start, lap and pause in that order, so "--reset --start"
// restarts from zero no matter how the flags were written.
void applyActions(const QCommandLineParser &parser, StopwatchEngine &engine);

// One line, e.g. "Running 00:12:03.4", meant for a terminal or a status bar.
QString statusLine(const StopwatchEngine &engine);

// Oldest lap first, one per line: number, split and total, tab separated
// so it pastes straight into a spreadsheet or `cut -f`.
QString lapsTable(const StopwatchEngine &engine);
}
