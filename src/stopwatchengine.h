// SPDX-FileCopyrightText: 2026 Aaghash Murugesan <aaghashm@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QDateTime>
#include <QObject>
#include <QTimer>
#include <qqmlregistration.h>

#include <functional>
#include <optional>

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
    Q_PROPERTY(QDateTime startedAt READ startedAt NOTIFY stateChanged)
    Q_PROPERTY(LapsModel *laps READ laps CONSTANT)
    Q_PROPERTY(bool canUndoReset READ canUndoReset NOTIFY canUndoResetChanged)

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
    // When the stopwatch was started from zero; invalid while stopped.
    // Pausing and resuming do not change it.
    QDateTime startedAt() const;
    LapsModel *laps() const;
    bool canUndoReset() const;

    Q_INVOKABLE void start();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void reset();
    Q_INVOKABLE void lap();
    // Brings back exactly what the last reset threw away, as if it never
    // happened. Only possible until the next start, pause or lap.
    Q_INVOKABLE void undoReset();

    Q_INVOKABLE QString formatDuration(qint64 ms) const;

    // Picks up a state written by another process, e.g. the command line
    // while the window is open. Does nothing if the saved state is the one
    // this engine wrote, so it's safe to call after our own saves.
    void reload();

Q_SIGNALS:
    void stateChanged();
    void elapsedChanged();
    void canUndoResetChanged();

private:
    struct Snapshot {
        State state;
        qint64 accumulatedMs;
        qint64 startedAtMs;
        qint64 firstStartedAtMs;
        QStringList laps;
    };

    void saveState() const;
    void dropUndo();
    void restoreState();
    void setState(State state);

    std::function<qint64()> m_now;
    LapsModel *const m_laps;
    QTimer m_ticker;
    State m_state = Stopped;
    qint64 m_accumulatedMs = 0;
    qint64 m_startedAtMs = 0;
    qint64 m_firstStartedAtMs = 0;
    std::optional<Snapshot> m_beforeReset;
};
