// SPDX-FileCopyrightText: 2026 Aaghash Murugesan <aaghashm@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QObject>
#include <QTimer>
#include <qqmlregistration.h>

#include <functional>

#include "lapsmodel.h"

/**
 * The stopwatch state machine.
 *
 * The only facts that are ever persisted are the wall-clock timestamp of the
 * last (re)start and the milliseconds accumulated before it. Elapsed time is
 * always derived from those, never counted, so killing the process, suspend,
 * or a reboot cannot lose time.
 */
class StopwatchEngine : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(qint64 elapsedMs READ elapsedMs NOTIFY elapsedChanged)
    Q_PROPERTY(LapsModel *laps READ laps CONSTANT)

public:
    enum State {
        Stopped,
        Running,
        Paused,
    };
    Q_ENUM(State)

    explicit StopwatchEngine(QObject *parent = nullptr);
    // Test seam: lets the test suite control "now" instead of the wall clock.
    explicit StopwatchEngine(std::function<qint64()> clock, QObject *parent = nullptr);

    State state() const;
    qint64 elapsedMs() const;
    LapsModel *laps() const;

    Q_INVOKABLE void start();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void reset();
    Q_INVOKABLE void lap();

    Q_INVOKABLE QString formatDuration(qint64 ms) const;

Q_SIGNALS:
    void stateChanged();
    void elapsedChanged();

private:
    void saveState() const;
    void restoreState();
    void setState(State state);

    std::function<qint64()> m_now;
    LapsModel *const m_laps;
    QTimer m_ticker;
    State m_state = Stopped;
    qint64 m_accumulatedMs = 0;
    qint64 m_startedAtMs = 0;
};
