// SPDX-FileCopyrightText: 2026 Aaghash Murugesan <aaghashm@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QObject>
#include <QTimer>
#include <qqmlregistration.h>

/**
 * The stopwatch state machine.
 *
 * Elapsed time is derived from the wall-clock timestamp of the last
 * (re)start plus the milliseconds accumulated before it. Nothing is ever
 * counted, so a slow or throttled timer cannot make the stopwatch drift.
 */
class StopwatchEngine : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(qint64 elapsedMs READ elapsedMs NOTIFY elapsedChanged)

public:
    enum State {
        Stopped,
        Running,
        Paused,
    };
    Q_ENUM(State)

    explicit StopwatchEngine(QObject *parent = nullptr);

    State state() const;
    qint64 elapsedMs() const;

    Q_INVOKABLE void start();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void reset();

    Q_INVOKABLE QString formatDuration(qint64 ms) const;

Q_SIGNALS:
    void stateChanged();
    void elapsedChanged();

private:
    void setState(State state);

    QTimer m_ticker;
    State m_state = Stopped;
    qint64 m_accumulatedMs = 0;
    qint64 m_startedAtMs = 0;
};
