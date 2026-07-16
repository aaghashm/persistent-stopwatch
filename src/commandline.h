// SPDX-FileCopyrightText: 2026 Aaghash Murugesan <aaghashm@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

class QCommandLineParser;
class StopwatchEngine;

/**
 * Command line actions, so the stopwatch can be driven from a terminal,
 * a script or a global shortcut without opening the window.
 */
namespace CommandLine
{
void addOptions(QCommandLineParser &parser);

bool hasAction(const QCommandLineParser &parser);

// Applies reset, start, lap and pause in that order, so "--reset --start"
// restarts from zero no matter how the flags were written.
void applyActions(const QCommandLineParser &parser, StopwatchEngine &engine);
}
